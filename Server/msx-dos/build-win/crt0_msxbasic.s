	.globl	_main
    .globl  l__INITIALIZER
    .globl  s__INITIALIZED
    .globl  s__INITIALIZER

	.area _HEADER (ABS)

	.org    0x9000 
    .db 	0xfe
    .dw 	init
    .dw		end
    .dw 	init

	;--- Initialize globals and jump to "main"

init:   call gsinit
	push de
	ld de,#_HEAP_start
	ld (_heap_top),de
	pop de

	jp    _main

	;--- Program code and data (global vars) start here

	;* Place data after program code, and data init code after data

	.area	_CODE
	.area	_DATA

_heap_top::
	.dw 0

        .area   _GSINIT

        ;--- Globals initialization code
        ;    (needed for SDCC >=3.3.0)

gsinit:
        ld	bc, #l__INITIALIZER
	ld	a, b
	or	a, c
	ret	z
	ld	de, #s__INITIALIZED
	ld	hl, #s__INITIALIZER
	ldir
	ret

	.area   _GSFINAL
	ret
end:        

	;* These doesn't seem to be necessary... (?)

	;.area  _OVERLAY
	;.area	_HOME
	;.area  _BSS

	.area	_HEAP

_HEAP_start::
