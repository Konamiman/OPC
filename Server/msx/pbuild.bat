@echo off

echo ---printf_small
sdcc -mz80 -c --max-allocs-per-node 100000 --allow-unsafe-read --opt-code-size .\printf_small.c
if errorlevel 1 goto :end

echo ---test
sdcc -mz80 --code-loc 0x180 --data-loc 0 --no-std-crt0 .\crt0msx_msxdos.rel .\char_msxdos.lib .\printf_test.c
if errorlevel 1 goto :end

hex2bin -e com printf_test.ihx

echo --- Mounting disk image file...
call mount.bat
if errorlevel 1 goto :end

echo --- Copying files...
copy printf_test.com Y:pt.com

echo --- Unmounting disk image file...
ping 1.1.1.1 -n 1 -w 1500 >NUL
call unmount.bat

:end