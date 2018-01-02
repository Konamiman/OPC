//extern char getchar(void);
//extern void putchar(char);
//extern void printf(char*, char*);
#include <stdio.h>

unsigned char j = '!';
int i = 34;
static int st = 89;
const int imconst = 333;
int imnotinit;
int cien[100] = {0};

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
    printf("Mola %s %i %i\r\n", "mazo", i, st);
}
