#include <stdio.h>

//void printf_small (const char *fmt, va_list ap);
void printf_small(const char *fmt, ...);
void sprintf_small(const char* buf, const char* fmt, ...);

//#include "printf_small.c"

char kk[100];

int main()
{
    printf_small("%d %d %ld %hd %x %x %lx %hx %o %o %lo %ho %c %s", 
        1234, -1234, 123456, 'A', 1234, -1234, 0xaabbccdd, 'B', 1234, -1234, 123456, 'C', 'X', "blah");
    //printf("%d %d %ld %hd %x %x %lx %hx %o %o %lo %ho %c %s", 
    //    1234, -1234, 123456, 'A', 1234, -1234, 0xaabbccdd, 'B', 1234, -1234, 123456, 'C', 'X', "blah");
    printf_small("\r\n\r\n");
    printf_small("eh: %d %u ! %ld . %lu eso\r\n", 0x8000, 0x8000, 0x80000000, 0x80000000);

    sprintf_small(&kk[0], "\r\nHola %s", "mundo");
    printf_small(&kk[0]);

    //printf      ("%u %ul\r\n", 0x8000, 0x80000000);
    return 0;
}