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

#define DEBUG

#ifdef DEBUG
#define debug(x) {print("--- ");print(x);print("\r\n");}
#define debug2(x,y) {print("--- ");printf(x,y);print("\r\n");}
#define debug3(x,y,z) {print("--- ");printf(x,y,z);print("\r\n");}
#define debug4(x,y,z,a) {print("--- ");printf(x,y,z,a);print("\r\n");}
#define debug5(x,y,z,a,b) {print("--- ");printf(x,y,z,a,b);print("\r\n");}
#else
#define debug(x)
#define debug2(x,y)
#define debug3(x,y,z)
#define debug4(x,y,z,a)
#define debug5(x,y,z,a,b)
#endif

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

enum PendingCommandState {
    PCMD_NONE = 0,
    PCMD_PARTIAL = 1,
    PCMD_WRITING = 2,
    PCMD_FULL = 3
};

const byte TCP_ESTABLISHED = 4;

    /* MSX-DOS functions */

#define _TERM0 0
#define _TERM 0x62


    /* Some handy defines */

typedef unsigned char bool;
#define false (0)
#define true (!false)

#ifndef ushort
typedef unsigned short ushort;
#endif

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
byte* executeCommandPayloadLengths[4] = { 4, 10, 14, 22 };

struct {
    //from PendingCommandState
    byte state;

    //First byte of command with low nibble stripped out
    byte commandCode;

    //How many bytes left to have received a completed command?
    //(in state "Writing": how many bytes left to write to mem/port?)
    int remainingBytes;

    //For execution, read or write
    int address;

    union {
        byte* writePointer; //for "Write"
        struct {
            byte input;
            byte output;
        } registers; //for "Execute"
    } stateData;

    //Storage for incomplete command.
    //For "Write" commands this does not include the bytes to write.
    //Max length is for "Execute": command + address + 10 register pairs
    byte buffer[23];

    byte* bufferWritePointer;
} pendingCommand;


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
byte ProcessFirstCommandByte(byte datum);
byte ProcessNextCommandByte(byte datum);
byte ProcessNextByteToWrite(byte datum);
void RunCompletedCommand();
void LoadRegistersBeforeExecutingCode(byte length);
void SendResponseAfterExecutingCode(byte length);
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
    pendingCommand.state = PCMD_NONE;

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
    switch(pendingCommand.state)
    {
        case PCMD_NONE:
            pendingCommand.state = ProcessFirstCommandByte(datum);
            if(pendingCommand.state == PCMD_PARTIAL) {
                pendingCommand.buffer[0] = datum;
                pendingCommand.bufferWritePointer = (byte*)&(pendingCommand.buffer[1]);
            }
            break;
        case PCMD_PARTIAL:
            pendingCommand.state = ProcessNextCommandByte(datum);
            break;
        case PCMD_WRITING:
            pendingCommand.state = ProcessNextByteToWrite(datum);
    };

    if(pendingCommand.state == PCMD_FULL) {
        RunCompletedCommand();
        pendingCommand.state = PCMD_NONE;
    }
}

byte ProcessFirstCommandByte(byte datum)
{
    byte commandCode = datum & 0xF0;

    if(commandCode == OPC_PING) {
        debug("Received PING\r\n");
        SendByte(0, false);
        SendByte(datum, true);
        return PCMD_NONE;
    }

    if(commandCode == OPC_EXECUTE) {
        pendingCommand.commandCode = commandCode;
        pendingCommand.remainingBytes = (int)executeCommandPayloadLengths[datum & 3];
        pendingCommand.stateData.registers.input = pendingCommand.remainingBytes;
        pendingCommand.stateData.registers.output = (byte)executeCommandPayloadLengths[(datum >> 2) & 3];
        debug4("Received start of EXECUTE, rem bytes=%d, input regs=%d, output regs=%d\r\n", 
            pendingCommand.remainingBytes, pendingCommand.stateData.registers.input, pendingCommand.stateData.registers.output);
        return PCMD_PARTIAL;
    }

    if(commandCode == OPC_READ_MEM || commandCode == OPC_WRITE_MEM) {
        pendingCommand.commandCode = commandCode;
        pendingCommand.remainingBytes = ((datum & 0x0F) == 0 ? 4 : 2);
        debug3("Received start of %s MEM, rem bytes=%d",
            commandCode == OPC_READ_MEM ? "READ" : "WRITE",
            pendingCommand.remainingBytes);
        return PCMD_PARTIAL;
    }

    debug2("Unknown command: 0x%x", datum);
    print("*** Unknown command received, disconnecting\r\n");
    SendErrorMessage("Unknown command");
    LetTcpipBreathe();
    AbortTcpConnection();
    return PCMD_NONE;
}

byte ProcessNextCommandByte(byte datum)
{
    uint length;

    *(pendingCommand.bufferWritePointer) = datum;
    pendingCommand.bufferWritePointer++;
    pendingCommand.remainingBytes--;

    if(pendingCommand.remainingBytes != 0)
        return PCMD_PARTIAL;
    
    if(pendingCommand.commandCode != OPC_WRITE_MEM)
        return PCMD_FULL;

    length = pendingCommand.buffer[0] & 0x0F;
    if(length == 0) {
        length = *((uint*)&(pendingCommand.buffer[3]));
    }
    if(length == 0) {
        return PCMD_NONE;
    }

    pendingCommand.remainingBytes = length;
    pendingCommand.stateData.writePointer = *(byte**)&(pendingCommand.buffer[1]);
    debug3("Write mem: address=0x%x, length=%u", pendingCommand.stateData.writePointer, pendingCommand.remainingBytes);
    return PCMD_WRITING;
}

byte ProcessNextByteToWrite(byte datum)
{
    *(pendingCommand.stateData.writePointer) = datum;
    pendingCommand.stateData.writePointer++;
    pendingCommand.remainingBytes--;

    if(pendingCommand.remainingBytes == 0) {
        debug("Write mem: completed");
        SendByte(0, true);
        return PCMD_NONE;
    }
    else {
        return PCMD_WRITING;
    }
}

void RunCompletedCommand()
{
    int address;
    uint length;

    if(pendingCommand.commandCode == OPC_EXECUTE) {
        address = *((int*)&(pendingCommand.buffer[1]));
        debug2("Execute: address=0x%x", address);
        LoadRegistersBeforeExecutingCode(pendingCommand.stateData.registers.input-2);

        AsmCall(address, &regs, REGS_ALL, REGS_ALL);

        SendResponseAfterExecutingCode(pendingCommand.stateData.registers.output-2);
    }
    else if(pendingCommand.commandCode == OPC_READ_MEM) {
        address = *((int*)&(pendingCommand.buffer[1]));
        length = pendingCommand.buffer[0] & 0x0F;
        if(length == 0) {
            length = *((uint*)&(pendingCommand.buffer[3]));
        }
        debug3("Read mem: address=0x%x, length=%u", address, length);
        SendByte(0, false);
        if(length > 0) {
            SendBytes((byte*)address, length, true);
        }
    }
}

void LoadRegistersBeforeExecutingCode(byte length)
{
    short* regsPointer = (short*)&(pendingCommand.buffer[3]);

    regs.Words.AF = regsPointer[0];
    debug2("Execute: in AF=0x%x", regs.Words.AF);
    length -=2;
    if(length == 0) return;

    regs.Words.BC = regsPointer[1];
    regs.Words.DE = regsPointer[2];
    regs.Words.HL = regsPointer[3];
    debug2("Execute: in BC=0x%x", regs.Words.BC);
    debug2("Execute: in DE=0x%x", regs.Words.DE);
    debug2("Execute: in HL=0x%x", regs.Words.HL);
    length -=6;
    if(length == 0) return;

    regs.Words.IX = regsPointer[4];
    regs.Words.IY = regsPointer[5];
    debug2("Execute: in IX=0x%x", regs.Words.IX);
    debug2("Execute: in IY=0x%x", regs.Words.IY);
    length -=4;
    if(length == 0) return;

    //TODO: Set alternate registers
}

void SendResponseAfterExecutingCode(byte length)
{
    short* regsPointer = (short*)&(pendingCommand.buffer[3]);
    pendingCommand.buffer[2] = 0; //First byte of "ok" response

    regsPointer[0] = regs.Words.AF;
    debug2("Execute: out AF=0x%x", regs.Words.AF);
    if(length > 2) {
        regsPointer[1] = regs.Words.BC;
        regsPointer[2] = regs.Words.DE;
        regsPointer[3] = regs.Words.HL;
        debug2("Execute: out BC=0x%x", regs.Words.BC);
        debug2("Execute: out DE=0x%x", regs.Words.DE);
        debug2("Execute: out HL=0x%x", regs.Words.HL);
        if(length > 8) {
            regsPointer[4] = regs.Words.IX;
            regsPointer[5] = regs.Words.IY;
            debug2("Execute: out IX=0x%x", regs.Words.IX);
            debug2("Execute: out IY=0x%x", regs.Words.IY);
            if(length > 12) {
                //TODO: Set alternate registers
            }
        }
    }

    SendBytes((byte*)&(pendingCommand.buffer[2]), length+1, true);
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