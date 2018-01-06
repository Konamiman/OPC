	.module crt0
	.globl	_main

    .globl  l__INITIALIZER
    .globl  s__INITIALIZED
    .globl  s__INITIALIZER

	.area _HEADER (ABS)

	.org    0xA000 
    .db 	0xFE
    .dw 	init
    .dw		end
    .dw 	init

	;--- Initialize globals and jump to "main"

init:
    ld	bc, #l__INITIALIZER
	ld	a, b
	or	a, c
	ret	z
	ld	de, #s__INITIALIZED
	ld	hl, #s__INITIALIZER
	ldir

	jp    _main

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

end:
