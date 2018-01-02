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

//#define DEBUG

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "asm.h"
#include "debug.h"
#include "env.h"
#include "transport.h"


    /* MSX-DOS functions */

#define _TERM0 0
#define _DIRIO 0x06
#define _TERM 0x62
#define _DEFAB 0x63
#define _DEFER 0x64

#define _CTRLC 0x9E
#define _STOP 0x9F


    /* Some handy defines */

#define print printf
#define ToLowerCase(c) ((c) | 32)

#define SERVER_MAX_ADDRESS 0x2500
#define IsProhibitedAddress(address) ((bool)(address >= (byte*)100 && address <= (byte*)SERVER_MAX_ADDRESS))

int StartOpcServer(void* transportInitData, bool _verbose);


    /* Strings */

const char* strTitle=
    "OPC server for TCP v1.0\r\n"
    "By Konamiman, 1/2018\r\n"
    "\r\n";
    
const char* strUsage=
    "Usage: opcs <local port> [v]\r\n"
    "       v = verbose mode\r\n";
    
const char* strInvParam = "Invalid parameter";


    /* Variables */

static char** arguments;
static int argumentsCount;
static Z80_registers regs;
static int port;
static bool verbose = false;
static bool serverTerminated = false;

static byte readPortBuffer[SEND_CHUNK_SIZE];
#define errorMessageBuffer readPortBuffer


    /* Local function prototypes */

int NoParameters();
void PrintTitle();
void PrintUsageAndEnd();
void ParseParameters();
void TerminateWithErrorCode(byte errorCode);
bool EscIsPressed();
void SetAutoAbortOnDiskError();
void DisableProgramTerminationOnDiskErrorAbort();
void RestoreDefaultDiskErrorRoutine();
void RestoreDefaultAbortRoutine();
void TerminateWithCtrlCOrCtrlStop();
void CheckKeyPressAvailable();


/**********************
 ***  MAIN is here  ***
 **********************/

int main(char** argv, int argc)
{
    int errorCode;

    arguments = argv;
    argumentsCount = argc;

    PrintTitle();
    if(NoParameters()) {
        PrintUsageAndEnd();
    }

    ParseParameters();
    
    SetAutoAbortOnDiskError();
    DisableProgramTerminationOnDiskErrorAbort();
    print("--- Press ESC at any time to exit\r\n\r\n");

    errorCode = StartOpcServer((void*)port, verbose);

    RestoreDefaultAbortRoutine();
    RestoreDefaultDiskErrorRoutine();
    TerminateWithErrorCode(errorCode);
    return 0;
}


/****************************
 ***  FUNCTIONS are here  ***
 ****************************/

/* Functions from env.h */

void DoEnvironmentStuff()
{
    CheckKeyPressAvailable();
}

bool MustTerminateServer()
{
    return EscIsPressed() || serverTerminated;
}

bool CanExecuteAtAddress(byte* address)
{
    return !IsProhibitedAddress(address);
}

bool CanWriteAtAddress(byte* address)
{
    return !IsProhibitedAddress(address);
}


/* Local functions */

int NoParameters()
{
    return (argumentsCount == 0);
}


void PrintTitle()
{
    print(strTitle);
}


void PrintUsageAndEnd()
{
    print(strUsage);
    DosCall(0, &regs, REGS_MAIN, REGS_NONE);
}

void ParseParameters()
{
    port = atoi(arguments[0]);
    if(port == 0) {
        printf("*** Invalid parameters\r\n");
        TerminateWithErrorCode(1);
    }
    
    if(argumentsCount >= 2) {
        if(ToLowerCase(arguments[1][0]) != 'v') {
            printf("*** Invalid parameters\r\n");
            TerminateWithErrorCode(1);
        }

        verbose = true;
    }
}

void TerminateWithErrorCode(byte errorCode)
{
    regs.Bytes.B = errorCode;
    DosCall(_TERM, &regs, REGS_MAIN, REGS_NONE);
    DosCall(_TERM0, &regs, REGS_MAIN, REGS_NONE);
}

bool EscIsPressed()
{
    return (*((byte*)0xFBEC) & 4) == 0;
}


/*
Prevent a "Disk error - Abort, Retry, Ignore?" prompt
to be thrown at the console if a disk access related
MSX-DOS function call is invoked and fails;
instead, the operation is simply auto-aborted
(and DisableProgramTerminationOnDiskErrorAbort prevents
that from terminating the program).

Unfortunately this does not work on MSX-DOS 1.
*/

void SetAutoAbortOnDiskError() __naked
{
    __asm
    
    push    ix
    ld  de,#DISKERR_CODE
    ld  c,#_DEFER
    call    #5
    pop ix
    ret

DISKERR_CODE:
    ld a,#1
    ret

    __endasm;
}

void RestoreDefaultDiskErrorRoutine()
{
    regs.Words.DE = 0;
    DosCall(_DEFER, &regs, REGS_MAIN, REGS_NONE);
}


/*
Prevent the program from terminating abruptely
after a disk error has been auto-aborted,
or when the user presses Ctrl-C or Ctrl-STOP.

In case of disk error, the program simply continues.
In case of Ctrl-C or Ctrl-STOP being pressed,
the program terminates gracefully.

Unfortunately this does not work on MSX-DOS 1.
*/

void DisableProgramTerminationOnDiskErrorAbort() __naked
{
    __asm
    
    push    ix
    ld  de,#ABORT_CODE
    ld  c,#_DEFAB
    call    #5
    pop ix
    ret

    ;Input:  A = Primary error code
    ;        B = Secondary error code, 0 if none
    ;Output: A = Actual error code to use
ABORT_CODE:
    cp  #_CTRLC
    jp  z,_TerminateWithCtrlCOrCtrlStop
    cp  #_STOP
    jp  z,_TerminateWithCtrlCOrCtrlStop

    pop hl  ;This causes the program to continue instead of terminating
    
    ld  c,a ;Return the secondary error code if present,
    ld  a,b ;instead of the generic "Operation aborted" error
    or  a
    ret nz
    ld  a,c
    ret

    __endasm;
}

void RestoreDefaultAbortRoutine()
{
    regs.Words.DE = 0;
    DosCall(_DEFAB, &regs, REGS_MAIN, REGS_NONE);
}

void TerminateWithCtrlCOrCtrlStop()
{
    printf("*** Server manually terminated\r\n");
    serverTerminated = true;
}

/*
Implemented in ASM for performance
(this is called continuosly in a loop)
*/

void CheckKeyPressAvailable() __naked
{
    __asm
    push ix
    ld c,#_DIRIO
    ld e,#0xFF
    call #5
    pop ix
    ret
    __endasm;
}