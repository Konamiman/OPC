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

enum TcpipUnapiFunctions {
    UNAPI_GET_INFO = 0,
    TCPIP_GET_CAPAB = 1,
    TCPIP_NET_STATE = 3,
    TCPIP_DNS_Q = 6,
    TCPIP_DNS_S = 7,
    TCPIP_TCP_OPEN = 13,
    TCPIP_TCP_CLOSE = 14,
    TCPIP_TCP_ABORT = 15,
    TCPIP_TCP_STATE = 16,
    TCPIP_TCP_SEND = 17,
    TCPIP_TCP_RCV = 18,
    TCPIP_WAIT = 29
};

enum TcpipErrorCodes {
    ERR_OK = 0,			    
    ERR_NOT_IMP,		
    ERR_NO_NETWORK,		
    ERR_NO_DATA,		
    ERR_INV_PARAM,		
    ERR_QUERY_EXISTS,	
    ERR_INV_IP,		    
    ERR_NO_DNS,		    
    ERR_DNS,		    
    ERR_NO_FREE_CONN,	
    ERR_CONN_EXISTS,	
    ERR_NO_CONN,		
    ERR_CONN_STATE,		
    ERR_BUFFER,		    
    ERR_LARGE_DGRAM,	
    ERR_INV_OPER
};


    /* MSX-DOS functions */

#define _TERM0 0
#define _TERM 0x62


    /* Strings */

const char* strTitle=
    "OPC server 1.0\r\n"
    "By Konamiman, 12/2017\r\n"
    "\r\n";
    
const char* strUsage=
    "Usage: opcs <local port>\r\n";
    
const char* strInvParam = "Invalid parameter";


    /* Variables */

char** arguments;
int argumentsCount;
Z80_registers regs;
unapi_code_block codeBlock;
int port;


    /* Some handy defines */

typedef unsigned char bool;
#define false (0)
#define true (!false)

#define print printf


    /* Function prototypes */

int NoParameters();
void PrintTitle();
void PrintUsageAndEnd();
void ParseParameters();
void Terminate(const char* errorMessage);
void InitializeTcpipUnapi();
bool EscIsPressed();


/**********************
 ***  MAIN is here  ***
 **********************/

int main(char** argv, int argc)
{
    arguments = argv;
    argumentsCount = argc;

    PrintTitle();
    if(NoParameters()) {
        PrintUsageAndEnd();
    }

    ParseParameters();
    InitializeTcpipUnapi();

    print("--- Press ESC at any time to exit\r\n\r\n");

    while(!EscIsPressed())
    {
        //Application main loop
    }

    Terminate(NULL);
    return 0;
}


/****************************
 ***  FUNCTIONS are here  ***
 ****************************/

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
    if(port == 0)
        Terminate("Invalid parameters");
}

void Terminate(const char* errorMessage)
{
    if(errorMessage != NULL) {
        printf("\r\x1BK*** %s\r\n", errorMessage);
    }
    
    regs.Bytes.B = (errorMessage == NULL ? 0 : 1);
    DosCall(_TERM, &regs, REGS_MAIN, REGS_NONE);
    DosCall(_TERM0, &regs, REGS_MAIN, REGS_NONE);
}

void InitializeTcpipUnapi()
{
    int i;

    i = UnapiGetCount("TCP/IP");
    if(i==0) {
        Terminate("No TCP/IP UNAPI implementations found");
    }
    UnapiBuildCodeBlock(NULL, 1, &codeBlock);
    
    regs.Bytes.B = 1;
    UnapiCall(&codeBlock, TCPIP_GET_CAPAB, &regs, REGS_MAIN, REGS_MAIN);
    if((regs.Bytes.L & (1 << 5)) == 0) {
        Terminate("This TCP/IP implementation does not support passive TCP connections.");
    }
    
    regs.Bytes.B = 0;
    UnapiCall(&codeBlock, TCPIP_TCP_ABORT, &regs, REGS_MAIN, REGS_NONE);
}

bool EscIsPressed()
{
    return (*((byte*)0xFBEC) & 4) == 0;
}