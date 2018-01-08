/* Obsolete Procedure Call (OPC) server for MSX-DOS v1.0
   By Konamiman 12/2017

   Compilation command line:
   
   sdcc --code-loc 0x180 --data-loc 0 -mz80 --disable-warning 196
        --no-std-crt0 crt0msx_msxdos_advanced.rel msxchar.lib asm.lib opc.c
   hex2bin -e com opc.ihx
   
   msxchar.lib, asm.lib, asm.h and crt0msx_msxdos_advanced.rel
   are available at www.konamiman.com
   
   (You don't need MSXCHAR.LIB if you manage to put proper PUTCHAR.REL,
   GETCHAR.REL and PRINTF.REL in the standard Z80.LIB... I couldn't manage to
   do it, I get a "Library not created with SDCCLIB" error)
   
   Comments are welcome: konamiman@konamiman.com
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "asm.h"
#include "env.h"
#include "opcs_core.h"

#define DefaultPort 3434

    /* BIOS and BASIC functions */

#define PTRGET 0x5EA4


    /* Some handy defines */

#define ToLowerCase(c) ((c) | 32)

#define SERVER_MAX_ADDRESS 0x2800
#define IsProhibitedAddress(address) ((bool)(address >= (byte*)100 && address <= (byte*)SERVER_MAX_ADDRESS))


    /* Strings */

const char* strTitle=
    "OPC server for TCP v1.0\r\n"
    "By Konamiman, 1/2018\r\n"
    "\r\n";


    /* Variables */

static Z80_registers regs;
static int port;
static bool verbose = false;
static bool serverTerminated = false;


    /* Local function prototypes */

int ReadBasicIntVariable(char* variableName);
#define ClearKeyboardBuffer() __asm__ ("call #0x0156") //KILBUF


/**********************
 ***  MAIN is here  ***
 **********************/

void main()
{
    int port;
    bool verboseMode;

    Print(strTitle);

    port = ReadBasicIntVariable("P%");
    if(port == 0) {
        port = DefaultPort;
    }
    
    printf("--- Listening on port %u\r\n", port);
    if(port == DefaultPort) {
        printf("    To use a different port do P%c=<port>\r\n",'%');
    }

    verboseMode = ReadBasicIntVariable("V%") != 0;
    if(!verboseMode) {
        printf("    To enable verbose mode do V%c=1\r\n",'%');
    }

    Print("\r\n--- Press any key to exit\r\n\r\n");

    StartOpcServer((void*)port, verboseMode);

    ClearKeyboardBuffer();
}


/****************************
 ***  FUNCTIONS are here  ***
 ****************************/

/* Functions from env.h */

void DoEnvironmentStuff()
{
}

#include "key_is_pressed.c"

bool MustTerminateServer() 
{
    return AnyKeyIsPressed();
}

bool CanExecuteAtAddress(byte* address)
{
    return !IsProhibitedAddress(address);
}

bool CanWriteAtAddress(byte* address)
{
    return !IsProhibitedAddress(address);
}

void Print(char* text)
{
    printf(text);
}


/* Local functions */

int ReadBasicIntVariable(char* variableName)
{
    regs.Words.HL = (int)variableName;
    AsmCall(PTRGET, &regs, REGS_MAIN, REGS_MAIN);
    return *((int*)regs.Words.DE);
}

