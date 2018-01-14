/* Obsolete Procedure Call (OPC) server core
   By Konamiman 1/2018 - www.konamiman.com

   Compilation command line:
   
   sdcc -mz80 --disable-warning 196 --disable-warning 85 
        --max-allocs-per-node 100000 --allow-unsafe-read --opt-code-size
        -c opcs_core.c

   Needs to be linked with modules that implement the functions in env.h and transport.h
   and with asm.lib
*/

//#define DEBUG

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../lib/asm.h"
#include "types.h"
#include "env.h"
#include "transport.h"
#include "debug.h"

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

#define SEND_CHUNK_SIZE 512


    /* Variables */

Z80_registers regs;
bool clientIsConnected = false;
bool verbose = false;
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
void SendErrorMessage(char* message);
void SendByte(byte datum, bool push);
bool HandleConnectionLifetime();
void ReadFromPort(byte portNumber, byte* destinationAddress, uint size, bool autoIncrement);
void WriteToPort(byte portNumber, byte value);

#define Printf1(msg, param) {sprintf(errorMessageBuffer, msg, param); Print(errorMessageBuffer);}
#define Printf2(msg, param1, param2) {sprintf(errorMessageBuffer, msg, param1, param2); Print(errorMessageBuffer);}
#define Printf3(msg, param1, param2, param3) {sprintf(errorMessageBuffer, msg, param1, param2, param3); Print(errorMessageBuffer);}


/**********************
 ***  MAIN is here  ***
 **********************/

int StartOpcServer(void* transportInitData, bool _verbose)
{
    int datum;

    verbose = _verbose;
    if(!InitializeTransport(transportInitData))
        return 1;

    pendingCommand.state = PCMD_NONE;

    while(!MustTerminateServer())
    {
        DoEnvironmentStuff();
        DoTransportStuff();
        if(!HandleConnectionLifetime()) {
            return 2;
        }

        datum = GetByteFromConnection();
        if(datum != -1) {
            ProcessReceivedByte((byte)datum);
        }
    }

    ShutDownTransport();
    return 0;
}


/****************************
 ***  FUNCTIONS are here  ***
 ****************************/

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
    DisconnectClient();
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
        pendingCommand.stateData.memWrite.isErrored = !CanWriteAtAddress(address);
        pendingCommand.stateData.memWrite.lockAddress = (bool)((pendingCommand.buffer[0] & (1<<3)) != 0);
        debug4("Write mem: address=0x%x, length=%u, errored: %u",
            pendingCommand.stateData.memWrite.pointer, pendingCommand.remainingBytes,
            pendingCommand.stateData.memWrite.isErrored);
        if(verbose) 
            Printf3("- Received WRITE MEMORY command for address 0x%x, length=%u%s\r\n", 
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
            Printf3("- Received WRITE PORT command for port %u, length=%u, autoincrement=%s\r\n", 
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
            SendErrorMessage("Can't write to this address, this space is used by the server");
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
    byte* address;
    byte port;
    uint length;
    union {
        bool incrementPort;
        bool lockAddress;
    } flags;

    if(pendingCommand.commandCode == OPC_EXECUTE) {
        address = (byte*)*((int*)&(pendingCommand.buffer[1]));

        if(!CanExecuteAtAddress(address)) {
            debug2("Execute: address=0x%x, ERROR! Server code", address);
            if(verbose) Printf1("- Received EXECUTE command for address 0x%x, error: bad address\r\n", address);
            SendErrorMessage("Can't execute code at this address, this space is used by the server");            
        }
        else if(pendingCommand.stateData.registers.input == 22 || pendingCommand.stateData.registers.output == 22) {
            debug2("Execute: address=0x%x, ERROR! Using alternate regs", address);
            if(verbose) Printf1("- Received EXECUTE command for address 0x%x, error: alt regs not supported\r\n", address);
            SendErrorMessage("Setting alternate input/output registers is not supported by this server");
        }
        else {
            debug2("Execute: address=0x%x", address);
            if(verbose) Printf1("- Received EXECUTE command for address 0x%x\r\n", address);
            LoadRegistersBeforeExecutingCode(pendingCommand.stateData.registers.input-2);

            AsmCall((uint)address, &regs, REGS_ALL, REGS_ALL);

            SendResponseAfterExecutingCode(pendingCommand.stateData.registers.output-2);
        }
    }
    else if(pendingCommand.commandCode == OPC_READ_MEM) {
        address = (byte*)*((int*)&(pendingCommand.buffer[1]));
        flags.lockAddress = (pendingCommand.buffer[0] & (1 << 3)) != 0;
        length = pendingCommand.buffer[0] & 0x07;
        if(length == 0) {
            length = *((uint*)&(pendingCommand.buffer[3]));
        }
        debug3("Read mem: address=0x%x, length=%u", address, length);
        if(verbose) 
            Printf3("- Received READ MEMORY command for address 0x%x, length=%u%s\r\n", address, length,
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
            Printf3("- Received READ PORT command for port %u, length=%u, autoincrement=%s\r\n",
            port, length, flags.incrementPort ? "yes" : "no");
        SendByte(0, false);
        SendPortBytes(port, length, flags.incrementPort);
    }
}

void SendPortBytes(byte port, uint length, bool increment)
{
    uint remaining = length;
    uint sendSize;

    if(length == 0) return;

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

    if(length == 0) return;

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

void SendErrorMessage(char* message)
{
    byte length = strlen(message);
    SendByte(length, false);
    SendBytes((byte*)message, length, true);
}

void SendByte(byte datum, bool push)
{
    SendBytes(&datum, 1, push);
}

bool HandleConnectionLifetime()
{
    bool wasPreviouslyConnected = clientIsConnected;
    clientIsConnected = ClientIsConnected();

    if(clientIsConnected && !wasPreviouslyConnected) {
        print("Client connected!\r\n");
        getDataBuffer.dataPointer = &(getDataBuffer.data[0]);
        getDataBuffer.remainingData = 0;
    }

    if(!clientIsConnected) {
        if(wasPreviouslyConnected) {
            print("Disconnected...\r\n");
        }

        return WaitForClientConnection();
    }

    return true;
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

