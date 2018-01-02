@echo off
@copy ..\OPC.c

echo --- Building...
sdcc --code-loc 0x180 --data-loc 0 -mz80 --disable-warning 196 --disable-warning 85 --no-std-crt0 crt0msx_msxdos_advanced.rel putchar_msxdos.rel printf.rel asm.lib OPC.c
if errorlevel 1 goto :end
hex2bin -e com OPC.ihx

echo --- Mounting disk image file...
call mount.bat
if errorlevel 1 goto :end

echo --- Copying file...
copy OPC.com Y:

echo --- Unmounting disk image file...
ping 1.1.1.1 -n 1 -w 1500 >NUL
call unmount.bat

:end
