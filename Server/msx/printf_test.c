#include <stdio.h>

void printf_small (const char *fmt, ...);

int main()
{
    printf_small("%d %d %ld %hd %x %x %lx %hx %o %o %lo %ho %c %s", 
        1234, -1234, 123456, 'A', 1234, -1234, 0xaabbccdd, 'B', 1234, -1234, 123456, 'C', 'X', "blah");
    //printf("%d %d %ld %hd %x %x %lx %hx %o %o %lo %ho %c %s", 
    //    1234, -1234, 123456, 'A', 1234, -1234, 0xaabbccdd, 'B', 1234, -1234, 123456, 'C', 'X', "blah");
    printf("\r\n\r\n");
    printf_small("eh: %d %u ! %ld . %lu eso\r\n", 0x8000, 0x8000, 0x80000000, 0x80000000);
    //printf      ("%u %ul\r\n", 0x8000, 0x80000000);
    return 0;
}