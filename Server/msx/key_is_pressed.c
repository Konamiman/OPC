#include "../core/types.h"

bool AnyKeyIsPressed() __naked
{
    __asm

    push ix
    ld ix,#0xFBE5 ;Keyboard matrix status, bit set to 0 = key pressed
    ld a,(ix)
    and a,1(ix)
    and a,2(ix)
    and a,3(ix)
    and a,4(ix)
    and a,5(ix)
    and a,6(ix)
    and a,7(ix)
    and a,8(ix)
    and a,9(ix)
    and a,10(ix)
    pop ix
    cpl
    ld l,a
    ret

    __endasm;
}