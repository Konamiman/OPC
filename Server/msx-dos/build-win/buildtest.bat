@echo off
@copy ..\OPC.c

echo --- Building...
sdasz80 -o .\crt0_msxbasic.rel .\crt0_msxbasic.s
sdcc --code-loc 0x9020 --data-loc 0 -mz80 --disable-warning 196 --disable-warning 85 --no-std-crt0 crt0_msxbasic.rel char_msxbasic.lib printf.rel test.c
if errorlevel 1 goto :end
hex2bin -e bin test.ihx

echo --- Mounting disk image file...
call mount.bat
if errorlevel 1 goto :end

echo --- Copying file...
copy test.bin Y:

echo --- Unmounting disk image file...
ping 1.1.1.1 -n 1 -w 1500 >NUL
call unmount.bat

:end
