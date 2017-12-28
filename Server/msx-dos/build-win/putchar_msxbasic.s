	.area _CODE
_putchar::       
        ld      hl,#2
        add     hl,sp
        
		ld		a,(hl)
		jp      0x00A2
