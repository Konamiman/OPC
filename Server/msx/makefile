ALLOCS = 100000 # Higher value = smaller generated file but slower compilation. Recommended for release: 100000
COMMON_FLAGS = -mz80 --disable-warning 196 --disable-warning 85 --max-allocs-per-node $(ALLOCS) --allow-unsafe-read --opt-code-size
SERVER_FLAGS = $(COMMON_FLAGS) -o opcs.ihx --data-loc 0 --no-std-crt0
SERVER_LIBS =  ../lib/asm.lib ../lib/printf_simple.rel opcs_core.rel transport_tcp-unapi.rel

ifeq ($(OS),Windows_NT)
	RM = del
endif

all: com bin

com: opcs.com

bin: opcs.bin

opcs.com: opcs_msxdos.c opcs_core.rel transport_tcp-unapi.rel
	sdcc $(SERVER_FLAGS) -o opcs.ihx --code-loc 0x180 lib/crt0msx_msxdos_advanced.rel $(SERVER_LIBS) lib/putchar_msxdos.rel opcs_msxdos.c
	hex2bin -e com opcs.ihx

opcs.bin: opcs_msxbasic.c opcs_core.rel transport_tcp-unapi.rel crt0_msxbasic.rel
	sdcc $(SERVER_FLAGS) -o opcs.ihx --code-loc 0x9820  crt0_msxbasic.rel $(SERVER_LIBS) lib/putchar_msxbasic.rel opcs_msxbasic.c
	hex2bin -e bin opcs.ihx

crt0_msxbasic.rel: crt0_msxbasic.asm
	sdasz80 -o crt0_msxbasic.rel crt0_msxbasic.asm

opcs_core.rel: ../core/opcs_core.c
	sdcc -c $(COMMON_FLAGS) ../core/opcs_core.c

transport_tcp-unapi.rel: transport_tcp-unapi.c
	sdcc -c $(COMMON_FLAGS) transport_tcp-unapi.c

clean:
	$(RM) *.ihx
	$(RM) *.map
	$(RM) *.sym
	$(RM) *.lst
	$(RM) *.lk
	$(RM) *.noi
	$(RM) opcs.asm
	$(RM) opcs.rel
	$(RM) opcs_core.*
	$(RM) transport_tcp-unapi.asm
	$(RM) transport_tcp-unapi.rel
	$(RM) crt0_msxbasic.rel

copy:
	echo --- Mounting disk image file...
	call ..\sandbox\mount.bat

	echo --- Copying files...
	copy opcs.com Y:
	copy opcs.bin Y:

	echo --- Unmounting disk image file...
	rem ping 1.1.1.1 -n 1 -w 1500 >NUL
	call ..\sandbox\unmount.bat




