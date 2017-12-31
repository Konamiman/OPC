@echo off
@copy ..\OPCS.c

echo --- Building...
sdcc --code-loc 0x180 --data-loc 0 -mz80 --disable-warning 196 --disable-warning 85 --max-allocs-per-node 10000 --allow-unsafe-read --opt-code-speed --no-std-crt0 crt0msx_msxdos_advanced.rel msxchar.lib asm.lib OPCS.c
if errorlevel 1 goto :end
hex2bin -e com OPCS.ihx

echo --- Mounting disk image file...
call mount.bat
if errorlevel 1 goto :end

echo --- Copying file...
copy OPCS.com Y:

echo --- Unmounting disk image file...
ping 1.1.1.1 -n 1 -w 1500 >NUL
call unmount.bat

:end
