@echo off
setlocal enabledelayedexpansion
set COMMON_ARGS=-mz80 --disable-warning 196 --disable-warning 85

for %%i in ("opcs_core.c") do set ATTRIBS=%%~ai
set ARCHIVE=!ATTRIBS:~2,1! 
if %ARCHIVE% neq a goto :coreok
echo --- Building server core...
sdcc -c %COMMON_ARGS% opcs_core.c
if errorlevel 1 goto :end
attrib -a opcs_core.c
:coreok

for %%i in ("transport_tcp-unapi.c") do set ATTRIBS=%%~ai
set ARCHIVE=!ATTRIBS:~2,1! 
if %ARCHIVE% neq a goto :transportok
echo --- Building transport module...
sdcc -c %COMMON_ARGS%  transport_tcp-unapi.c
if errorlevel 1 goto :end
attrib -a transport_tcp-unapi.c
:transportok

echo --- Building server app...
sdcc -o opcs.ihx --code-loc 0x180 --data-loc 0 %COMMON_ARGS% --no-std-crt0 crt0msx_msxdos_advanced.rel putchar_msx-dos.rel printf.rel asm.lib opcs_core.rel transport_tcp-unapi.rel opcs_msx-dos.c
if errorlevel 1 goto :end
hex2bin -e com opcs.ihx

echo --- Mounting disk image file...
call mount.bat
if errorlevel 1 goto :end

echo --- Copying file...
copy opcs.com Y:

echo --- Unmounting disk image file...
ping 1.1.1.1 -n 1 -w 1500 >NUL
call unmount.bat

:end
endlocal

