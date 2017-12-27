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
#define _DIRIO 0x06
#define _TERM 0x62
#define _DEFAB 0x63
#define _DEFER 0x64

#define _CTRLC 0x9E
#define _STOP 0x9F


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
#define ToLowerCase(c) ((c) | 32)

#define SEND_CHUNK_SIZE 512
#define SERVER_MAX_ADDRESS 0x2500

    /* Strings */

const char* strTitle=
    "OPC server 1.0\r\n"
    "By Konamiman, 12/2017\r\n"
    "\r\n";
    
const char* strUsage=
    "Usage: opcs <local port> [v]\r\n"
    "       v = verbose mode\r\n";
    
const char* strInvParam = "Invalid parameter";


    /* Variables */

char** arguments;
int argumentsCount;
Z80_registers regs;
unapi_code_block codeBlock;
int port;
bool connectionIsEstablished = false;
int connectionNumber = -1;
bool verbose = false;
struct {
    byte remoteIP[4];
    uint remotePort;
    uint localPort;
    int userTimeout;
    byte flags;
} tcpConnectionParameters;
byte executeCommandPayloadLengths[4] = { 4, 10, 14, 22 };

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
        struct {
            byte* pointer;
            bool isErrored;
            bool lockAddress;
        } memWrite;
        struct {
            byte port;
            bool increment;
        } portWrite;
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

struct {
    byte data[512];
    byte* dataPointer;
    int remainingData;
} getDataBuffer;

byte readPortBuffer[SEND_CHUNK_SIZE];
#define errorMessageBuffer readPortBuffer


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
void SendPortBytes(byte port, uint length, bool increment);
void SendMemoryBytes(byte* address, uint length, bool lockAddress);
void LoadRegistersBeforeExecutingCode(byte length);
void SendResponseAfterExecutingCode(byte length);
void ProcessReceivedByte(byte datum);
void SendErrorMessage(const char* message);
void SendByte(byte datum, bool push);
void SendBytes(byte* data, uint length, bool push);
void ReadFromPort(byte portNumber, byte* destinationAddress, uint size, bool autoIncrement);
void WriteToPort(byte portNumber, byte value);
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
    int datum;

    arguments = argv;
    argumentsCount = argc;

    PrintTitle();
    if(NoParameters()) {
        PrintUsageAndEnd();
    }

    ParseParameters();
    InitializeTcpipUnapi();
    
    SetAutoAbortOnDiskError();
    DisableProgramTerminationOnDiskErrorAbort();

    OpenPassiveTcpConnection();
    print("--- Press ESC at any time to exit\r\n\r\n");
    pendingCommand.state = PCMD_NONE;

    while(!EscIsPressed())
    {
        /*We need this to give MSX-DOS a chance of detecting
          if CTRL-STOP has been pressed*/
        CheckKeyPressAvailable();

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
    
    if(argumentsCount >= 2) {
        if(ToLowerCase(arguments[1][0]) != 'v')
            Terminate("Invalid parameters");

        verbose = true;
    }
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
    RestoreDefaultAbortRoutine();
    RestoreDefaultDiskErrorRoutine();

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
        getDataBuffer.dataPointer = &(getDataBuffer.data[0]);
        getDataBuffer.remainingData = 0;
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

    if(getDataBuffer.remainingData > 0) {
        datum = *(getDataBuffer.dataPointer);
        getDataBuffer.dataPointer++;
        getDataBuffer.remainingData--;
        return datum;
    }

    regs.Bytes.B = connectionNumber;
    regs.Words.DE = (short)&(getDataBuffer.data[0]);
    regs.Words.HL = sizeof(getDataBuffer.data);
    UnapiCall(&codeBlock, TCPIP_TCP_RCV, &regs, REGS_MAIN, REGS_MAIN);

    if(regs.Bytes.A != 0 || regs.Words.BC == 0) {
        return -1;
    } 
    else {
        debug2("Get data: got %u bytes", regs.Words.BC);
        getDataBuffer.dataPointer = &(getDataBuffer.data[1]); //dummy if got just 1 byte, but doesn't matter
        getDataBuffer.remainingData = regs.Words.BC - 1;
        return getDataBuffer.data[0];
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
        if(verbose) print("- Received PING command\r\n");
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

    if(commandCode == OPC_READ_PORT || commandCode == OPC_WRITE_PORT) {
        pendingCommand.commandCode = commandCode;
        pendingCommand.remainingBytes = ((datum & 0x07) == 0 ? 3 : 1);
        debug3("Received start of %s PORT, rem bytes=%d",
            commandCode == OPC_READ_PORT ? "READ" : "WRITE",
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
    byte* address;

    *(pendingCommand.bufferWritePointer) = datum;
    pendingCommand.bufferWritePointer++;
    pendingCommand.remainingBytes--;

    if(pendingCommand.remainingBytes != 0)
        return PCMD_PARTIAL;

    if(pendingCommand.commandCode != OPC_WRITE_MEM && pendingCommand.commandCode != OPC_WRITE_PORT)
        return PCMD_FULL;

    length = pendingCommand.buffer[0] & 0x07;
    if(length == 0) {
        length = *((uint*)&(pendingCommand.buffer[pendingCommand.commandCode == OPC_WRITE_MEM ? 3 : 2]));
    }
    if(length == 0) {
        return PCMD_NONE;
    }

    pendingCommand.remainingBytes = length;
    if(pendingCommand.commandCode == OPC_WRITE_MEM) {
        address = *(byte**)&(pendingCommand.buffer[1]);
        pendingCommand.stateData.memWrite.pointer = address;
        pendingCommand.stateData.memWrite.isErrored = ((uint)address >= 0x100 && (uint)address <= SERVER_MAX_ADDRESS);
        pendingCommand.stateData.memWrite.lockAddress = (bool)((pendingCommand.buffer[0] & (1<<3)) != 0);
        debug4("Write mem: address=0x%x, length=%u, errored: %u",
            pendingCommand.stateData.memWrite.pointer, pendingCommand.remainingBytes,
            pendingCommand.stateData.memWrite.isErrored);
        if(verbose) 
            printf("- Received WRITE MEMORY command for address 0x%x, length=%u%s\r\n", 
            pendingCommand.stateData.memWrite.pointer, pendingCommand.remainingBytes,
            pendingCommand.stateData.memWrite.lockAddress ? ", lock address" : "");
    }
    else {
        pendingCommand.stateData.portWrite.port = *(byte*)&(pendingCommand.buffer[1]);
        pendingCommand.stateData.portWrite.increment = (bool)((pendingCommand.buffer[0] & (1<<3)) != 0);
        debug4("Write port: port=%u, length=%u, incr=%u", 
            pendingCommand.stateData.portWrite.port, pendingCommand.remainingBytes,
            pendingCommand.stateData.portWrite.increment);
        if(verbose) 
            printf("- Received WRITE PORT command for port %u, length=%u, autoincrement=%s\r\n", 
            pendingCommand.stateData.portWrite.port, pendingCommand.remainingBytes,
            pendingCommand.stateData.portWrite.increment ? "yes" : "no");
    }
    return PCMD_WRITING;
}

byte ProcessNextByteToWrite(byte datum)
{
    if(pendingCommand.commandCode == OPC_WRITE_MEM) {
        if(!pendingCommand.stateData.memWrite.isErrored) {
            *(pendingCommand.stateData.memWrite.pointer) = datum;
            if(!pendingCommand.stateData.memWrite.lockAddress) {
                pendingCommand.stateData.memWrite.pointer++;
            }
        }
    }
    else {
        WriteToPort(pendingCommand.stateData.portWrite.port, datum);
        if(pendingCommand.stateData.portWrite.increment) {
            pendingCommand.stateData.portWrite.port++;
        }
    }
    pendingCommand.remainingBytes--;

    if(pendingCommand.remainingBytes == 0) {
        debug2("Write %s: completed", pendingCommand.commandCode == OPC_WRITE_MEM ? "mem" : "port");
        if(pendingCommand.commandCode == OPC_WRITE_MEM && pendingCommand.stateData.memWrite.isErrored) {
            sprintf(errorMessageBuffer, "Can't write to 0x100-0x%x, this space is used by the server", SERVER_MAX_ADDRESS);
            SendErrorMessage(errorMessageBuffer);
        }
        else {
            SendByte(0, true);
        }
        return PCMD_NONE;
    }
    else {
        return PCMD_WRITING;
    }
}

void RunCompletedCommand()
{
    int address;
    byte port;
    uint length;
    union {
        bool incrementPort;
        bool lockAddress;
    } flags;

    if(pendingCommand.commandCode == OPC_EXECUTE) {
        address = *((int*)&(pendingCommand.buffer[1]));

        if(address >= 0x100 && address <= SERVER_MAX_ADDRESS) {
            debug2("Execute: address=0x%x, ERROR! Server code", address);
            if(verbose) printf("- Received EXECUTE command for address 0x%x, error: bad address\r\n", address);
            sprintf(errorMessageBuffer, "Can't execute at 0x100-0x%x, this space is used by the server", SERVER_MAX_ADDRESS);
            SendErrorMessage(errorMessageBuffer);            
        }
        else if(pendingCommand.stateData.registers.input == 22 || pendingCommand.stateData.registers.output == 22) {
            debug2("Execute: address=0x%x, ERROR! Using alternate regs", address);
            if(verbose) printf("- Received EXECUTE command for address 0x%x, error: alt regs not supported\r\n", address);
            SendErrorMessage("Setting alternate input/output registers is not supported by this server");
        }
        else {
            debug2("Execute: address=0x%x", address);
            if(verbose) printf("- Received EXECUTE command for address 0x%x\r\n", address);
            LoadRegistersBeforeExecutingCode(pendingCommand.stateData.registers.input-2);

            AsmCall(address, &regs, REGS_ALL, REGS_ALL);

            SendResponseAfterExecutingCode(pendingCommand.stateData.registers.output-2);
        }
    }
    else if(pendingCommand.commandCode == OPC_READ_MEM) {
        address = *((int*)&(pendingCommand.buffer[1]));
        flags.lockAddress = (pendingCommand.buffer[0] & (1 << 3)) != 0;
        length = pendingCommand.buffer[0] & 0x07;
        if(length == 0) {
            length = *((uint*)&(pendingCommand.buffer[3]));
        }
        debug3("Read mem: address=0x%x, length=%u", address, length);
        if(verbose) 
            printf("- Received READ MEMORY command for address 0x%x, length=%u%s\r\n", address, length,
            flags.lockAddress ? ", lock address" : "");
        SendByte(0, false);
        SendMemoryBytes((byte*)address, length, flags.lockAddress);
    }
    else if(pendingCommand.commandCode == OPC_READ_PORT) {
        port = *((byte*)&(pendingCommand.buffer[1]));
        flags.incrementPort = (pendingCommand.buffer[0] & (1 << 3)) != 0;
        length = pendingCommand.buffer[0] & 0x07;
        if(length == 0) {
            length = *((uint*)&(pendingCommand.buffer[2]));
        }
        debug4("Read port: port=0x%x, length=%u, incr=%u", port, length, flags.incrementPort);
        if(verbose) 
            printf("- Received READ PORT command for port %u, length=%u, autoincrement=%s\r\n",
            port, length, flags.incrementPort ? "yes" : "no");
        SendByte(0, false);
        SendPortBytes(port, length, flags.incrementPort);
    }
}

void SendPortBytes(byte port, uint length, bool increment)
{
    uint remaining = length;
    uint sendSize;

    if(connectionNumber == -1 || length == 0) return;

    while(remaining > 0) {
        sendSize = remaining > SEND_CHUNK_SIZE ? SEND_CHUNK_SIZE : remaining;
        ReadFromPort(port, readPortBuffer, sendSize, increment);
        SendBytes(readPortBuffer, sendSize, true);
        remaining -= sendSize;
    }
}

void SendMemoryBytes(byte* address, uint length, bool lockAddress)
{
    uint remaining;
    uint sendSize;
    uint i;

    if(connectionNumber == -1 || length == 0) return;

    if(!lockAddress) {
        SendBytes(address, length, true);
        return;
    }

    remaining = length;
    while(remaining > 0) {
        sendSize = remaining > SEND_CHUNK_SIZE ? SEND_CHUNK_SIZE : remaining;
        for(i=0; i<sendSize; i++) {
            readPortBuffer[i] = *address;
        }
        SendBytes(readPortBuffer, sendSize, true);
        remaining -= sendSize;
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

    debug2("Execute: in AF'=0x%x", regsPointer[6]);
    debug2("Execute: in BC'=0x%x", regsPointer[7]);
    debug2("Execute: in DE'=0x%x", regsPointer[8]);
    debug2("Execute: in HL'=0x%x", regsPointer[9]);

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

void SendBytes(byte* data, uint length, bool push)
{
    uint remaining = length;
    uint sendSize;

    if(connectionNumber == -1) return;

    while(remaining > 0) {
        sendSize = remaining > SEND_CHUNK_SIZE ? SEND_CHUNK_SIZE : remaining;

        regs.Bytes.B = connectionNumber;
        regs.Words.DE = (short)data;
        regs.Words.HL = (short)sendSize;
        regs.Bytes.C = push ? 1 : 0;

        while(true) {
            UnapiCall(&codeBlock, TCPIP_TCP_SEND, &regs, REGS_MAIN, REGS_AF);
            if(regs.Bytes.A == 0) break;
            if(regs.Bytes.A != ERR_BUFFER) return;
            LetTcpipBreathe();
        }

        remaining -= sendSize;
    }
}

void ReadFromPort(byte portNumber, byte* destinationAddress, uint size, bool autoIncrement) __naked
{
    __asm

    push    ix
    ld      ix,#4
    add     ix,sp
    ld      c,(ix)  ;C=(first) port number
    ld      l,1(ix)
    ld      h,2(ix) ;HL=dest address
    ld      e,3(ix)
    ld      d,4(ix) ;DE=size
    ld      a,5(ix) ;A=autoIncrement
    or      a,a
    jr      z,RFP_DO
    ld      a,#0x0C ;INC C
RFP_DO:
    ld      (RFP_INCC),a

RFP_LOOP:
    ld  a,(hl)
    ini
RFP_INCC:
    nop ;INC C on autoincrement, NOP otherwise
    dec de
    ld a,d
    or e
    jr nz,RFP_LOOP

    pop ix
    ret

    __endasm;
}

void WriteToPort(byte portNumber, byte value)
{
    __asm

    push    ix
    ld      ix,#4
    add     ix,sp
    ld      c,(ix)  ;C=port number
    ld      a,1(ix) ;A=value
    out     (c),a
    pop     ix
    ret

    __endasm;
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
    Terminate("Server manually terminated");
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