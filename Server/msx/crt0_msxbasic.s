	.module crt0
	.globl	_main

    .globl  l__INITIALIZER
    .globl  s__INITIALIZED
    .globl  s__INITIALIZER

	.area _HEADER (ABS)

	.org    0xA000 
    .db 	0xfe
    .dw 	init
    .dw		end
    .dw 	init

	;--- Initialize globals and jump to "main"

init:   call gsinit
	;push de
	;ld de,#_HEAP_start
	;ld (_heap_top),de
	;pop de

	jp    _main

	;--- Program code and data (global vars) start here

	;; Ordering of segments for the linker.
	.area	_HOME
	.area	_CODE
	.area	_INITIALIZER
	.area   _GSINIT
	.area   _GSFINAL

	.area	_DATA
	.area	_INITIALIZED
	.area	_BSEG
	.area   _BSS
	.area   _HEAP

_heap_top::
	.dw 0

	.area	_CODE
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

	.area _INITIALIZER
end:        

	;* These doesn't seem to be necessary... (?)

	;.area  _OVERLAY
	;.area	_HOME
	;.area  _BSS

	.area	_HEAP

_HEAP_start::
