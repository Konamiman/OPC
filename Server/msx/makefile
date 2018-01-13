FLAGS = -mz80 --disable-warning 85 --disable-warning 196 --allow-unsafe-read --opt-code-size

all: printf_test.com printf_test.bin

printf_test.com: printf_small.rel printf_test.c
	sdcc -mz80 --code-loc 0x180 --data-loc 0 --no-std-crt0 .\crt0msx_msxdos.rel putchar_msxdos.rel getchar_msxdos.rel printf_small.rel .\printf_test.c
	hex2bin -e com printf_test.ihx

printf_test.bin: printf_small.rel printf_test.c crt0_msxbasic.rel
	sdcc -mz80 --code-loc 0x9820 --data-loc 0 --no-std-crt0 .\crt0_msxbasic.rel putchar_msxbasic.rel getchar_msxbasic.rel printf_small.rel .\printf_test.c
	hex2bin -e bin printf_test.ihx

crt0_msxbasic.rel: crt0_msxbasic.s
	sdasz80 -o crt0_msxbasic.rel crt0_msxbasic.s	

printf_small.rel: printf_small.c
	sdcc -c $(FLAGS) --max-allocs-per-node 100000 printf_small.c
	sdcclib a char_msxdos.lib printf_small.rel

copy:
	echo --- Mounting disk image file...
	call .\mount.bat
	if errorlevel 1 goto :end

	echo --- Copying files...
	copy printf_test.com Y:pt.com
	copy printf_test.bin Y:pt.bin

	echo --- Unmounting disk image file...
	rem ping 1.1.1.1 -n 1 -w 1500 >NUL
	call .\unmount.bat

del:
	del printf_test.com
	del printf_test.bin
	


