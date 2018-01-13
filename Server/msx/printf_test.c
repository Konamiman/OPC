#include <stdio.h>

//void printf_small (const char *fmt, va_list ap);
//void printf_small(const char *fmt, ...);
//void sprintf_small(const char* buf, const char* fmt, ...);

//#include "printf_small.c"

#define printf_small printf
#define sprintf_small sprintf

char kk[100];

int main()
{
    printf_small("%d %i %x %x %c %% %s", 
        1234, -1234, 1234, -1234, 'X', "blah");

    //printf("Hello!");
    printf_small("%d %i %li %x %x %lx %c %% %s", 
        1234, -1234, 123456, 1234, -1234, 0xaabbccdd, 'X', "blah");
     printf_small("\r\n%d %i %li %x %lx %c %% %s", 
        0xFF00, 0xFFF0, 0xFFFF0000, 0xFF00, 0xFFFF0000, 'X', "blah");
    //printf("%d %d %ld %hd %x %x %lx %hx %o %o %lo %ho %c %s", 
    //    1234, -1234, 123456, 'A', 1234, -1234, 0xaabbccdd, 'B', 1234, -1234, 123456, 'C', 'X', "blah");
    puts("\r\n\r\n");
    printf_small("eh: %d %u ! %ld . %lu eso\r\n", 0x8000, 0x8000, 0x80000000, 0x80000000);

    sprintf_small(&kk[0], "\r\nHola %s", "mundo");
    printf_small(&kk[0]);

    //printf("%c", getchar());

    //printf      ("%u %ul\r\n", 0x8000, 0x80000000);
    return 0;
}