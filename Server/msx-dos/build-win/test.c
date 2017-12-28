//extern char getchar(void);
//extern void putchar(char);
//extern void printf(char*, char*);
#include <stdio.h>

unsigned char j = '!';

char getthechar() __naked
{
    __asm
    call 0x9F;
    ld l,a
    ret
    __endasm;
}

void main()
{
    unsigned char x;
    x = getthechar();
    putchar(x);
    putchar(j);
    putchar('\r');
    putchar('\n');
    printf("Mola %s\r\n", "mazo");
}
