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

enum OpcCommands {
    OPC_PING = 0,
    OPC_EXECUTE = 0x10,
    OPC_READ_MEM = 0x20,
    OPC_WRITE_MEM = 0x30,
    OPC_READ_PORT = 0x40,
    OPC_WRITE_PORT = 0x50
};

const byte TCP_ESTABLISHED = 4;

    /* MSX-DOS functions */

#define _TERM0 0
#define _TERM 0x62


    /* Some handy defines */

typedef unsigned char bool;
#define false (0)
#define true (!false)

#define print printf
#define LetTcpipBreathe() UnapiCall(&codeBlock, TCPIP_WAIT, &regs, REGS_NONE, REGS_NONE)
#define AbortTcpConnection() CloseTcpConnection(true)


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
bool connectionIsEstablished = false;
int connectionNumber = -1;
struct {
    byte remoteIP[4];
    uint remotePort;
    uint localPort;
    int userTimeout;
    byte flags;
} tcpConnectionParameters;


    /* Function prototypes */

int NoParameters();
void PrintTitle();
void PrintUsageAndEnd();
void ParseParameters();
void Terminate(const char* errorMessage);
void TerminateWithErrorCode(byte errorCode);
void InitializeTcpipUnapi();
bool EscIsPressed();
void OpenPassiveTcpConnection();
void CloseTcpConnection(bool abort);
void HandleConnectionLifetime();
int GetByteFromConnection();
void ProcessReceivedByte(byte datum);
void SendErrorMessage(const char* message);
void SendByte(byte datum, bool push);
void SendBytes(byte* data, int length, bool push);


/**********************
 ***  MAIN is here  ***
 **********************/

int main(char** argv, int argc)
{
    int datum;

    arguments = argv;
    argumentsCount = argc;

    PrintTitle();
    if(NoParameters()) {
        PrintUsageAndEnd();
    }

    ParseParameters();
    InitializeTcpipUnapi();

    OpenPassiveTcpConnection();
    print("--- Press ESC at any time to exit\r\n\r\n");

    while(!EscIsPressed())
    {
        LetTcpipBreathe();
        HandleConnectionLifetime();

        datum = GetByteFromConnection();
        if(datum != -1) {
            ProcessReceivedByte((byte)datum);
        }
    }

    TerminateWithErrorCode(0);
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

    TerminateWithErrorCode(1);
}

void TerminateWithErrorCode(byte errorCode)
{
    AbortTcpConnection();

    regs.Bytes.B = errorCode;
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

    tcpConnectionParameters.remoteIP[0] = 0;
    tcpConnectionParameters.remoteIP[1] = 0;
    tcpConnectionParameters.remoteIP[2] = 0;
    tcpConnectionParameters.remoteIP[3] = 0;
    tcpConnectionParameters.localPort = port;
    tcpConnectionParameters.userTimeout = 0;
    tcpConnectionParameters.flags = 1; //passive connection
}

bool EscIsPressed()
{
    return (*((byte*)0xFBEC) & 4) == 0;
}

void OpenPassiveTcpConnection()
{
    byte error;

    regs.Words.HL = (int)&tcpConnectionParameters;
    UnapiCall(&codeBlock, TCPIP_TCP_OPEN, &regs, REGS_MAIN, REGS_MAIN);
    error = (byte)regs.Bytes.A;
    if(error == (byte)ERR_NO_FREE_CONN) {
        Terminate("No free TCP connections available");
    }
    else if(error == (byte)ERR_NO_NETWORK) {
        Terminate("No network connection available");
    }
    else if(error != 0) {
        printf("Unexpected error when opening TCP connection: %d", regs.Bytes.A);
        Terminate(NULL);
    }

    connectionNumber = regs.Bytes.B;
    connectionIsEstablished = false;
}

void CloseTcpConnection(bool abort)
{
    if(connectionNumber == -1) return;

    regs.Bytes.B = connectionNumber;
    UnapiCall(&codeBlock, abort ? TCPIP_TCP_ABORT : TCPIP_TCP_CLOSE, &regs, REGS_MAIN, REGS_NONE);
    connectionIsEstablished = false;
    connectionNumber = -1;
}

void HandleConnectionLifetime()
{
    byte tcpState;
    bool wasPreviouslyEstablished = connectionIsEstablished;
    bool previouslyConnectionExisted = (connectionNumber != -1);
    bool currentlyNotEstablished = false;

    regs.Bytes.B = connectionNumber;
    regs.Words.HL = 0;
    UnapiCall(&codeBlock, TCPIP_TCP_STATE, &regs, REGS_MAIN, REGS_MAIN);

    if(regs.Bytes.A == 0) {
        tcpState = regs.Bytes.B;

        if(tcpState == TCP_ESTABLISHED) {
            connectionIsEstablished = true;
        }
        else if(tcpState > TCP_ESTABLISHED) {
            AbortTcpConnection();
        }
    }
    else {
        connectionNumber = -1;
    }

    if(connectionIsEstablished && !wasPreviouslyEstablished) {
        print("Client connected!\r\n");
    }

    if(connectionNumber == -1) {
        if(previouslyConnectionExisted) {
            print("Disconnected...\r\n");
        }

        OpenPassiveTcpConnection();
    }
}

int GetByteFromConnection()
{
    byte datum;

    if(!connectionIsEstablished) {
        return -1;
    }

    regs.Bytes.B = connectionNumber;
    regs.Words.DE = (short)&datum;
    regs.Words.HL = 1;
    UnapiCall(&codeBlock, TCPIP_TCP_RCV, &regs, REGS_MAIN, REGS_MAIN);

    if(regs.Bytes.A != 0 || regs.Words.BC == 0) {
        return -1;
    } 
    else {
        return datum;
    }
}

void ProcessReceivedByte(byte datum)
{
    byte commandCode = datum & 0xF0;

    if(commandCode == OPC_PING) {
        print("--- Received PING\r\n");
        SendByte(0, false);
        SendByte(datum, true);
    }
    else {
        print("*** Unknown command received, disconnecting\r\n");
        SendErrorMessage("Unknown command");
        LetTcpipBreathe();
        AbortTcpConnection();
    }
}

void SendErrorMessage(const char* message)
{
    byte length = strlen(message);
    SendByte(length, false);
    SendBytes((byte*)message, length, true);
}

void SendByte(byte datum, bool push)
{
    SendBytes(&datum, 1, push);
}

void SendBytes(byte* data, int length, bool push)
{
    if(connectionNumber == -1) return;

    regs.Bytes.B = connectionNumber;
    regs.Words.DE = (short)data;
    regs.Words.HL = (short)length;
    regs.Bytes.C = push ? 1 : 0;

    while(true) {
        UnapiCall(&codeBlock, TCPIP_TCP_SEND, &regs, REGS_MAIN, REGS_AF);
        if(regs.Bytes.A != ERR_BUFFER) break;
        LetTcpipBreathe();
    }
}