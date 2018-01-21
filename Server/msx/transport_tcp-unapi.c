#include <stdio.h>
#include "../lib/asm.h"
#include "../core/types.h"
#include "../core/debug.h"

#include "../core/transport.h"

enum TcpipUnapiFunctions {
    UNAPI_GET_INFO = 0,
    TCPIP_GET_CAPAB = 1,
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

const byte TCP_ESTABLISHED = 4;

#define LetTcpipBreathe() UnapiCall(&codeBlock, TCPIP_WAIT, &regs, REGS_NONE, REGS_NONE)
#define AbortTcpConnection() CloseTcpConnection(true)

    /* Variables */

static Z80_registers regs;
static unapi_code_block codeBlock;
static bool connectionIsEstablished = false;
static int connectionNumber = -1;
static struct {
    byte remoteIP[4];
    uint remotePort;
    uint localPort;
    int userTimeout;
    byte flags;
} tcpConnectionParameters;

static struct {
    byte data[512];
    byte* dataPointer;
    int remainingData;
} getDataBuffer;

#define errorMessageBuffer ((char*)&(getDataBuffer.data))


static void PrintError(char* errorMEssage);
static void CloseTcpConnection(bool abort);

bool InitializeTransport(void* initializationData)
{
    int i;

    i = UnapiGetCount("TCP/IP");
    if(i==0) {
        PrintError("No TCP/IP UNAPI implementations found");
        return false;
    }
    UnapiBuildCodeBlock(NULL, 1, &codeBlock);
    
    regs.Bytes.B = 1;
    UnapiCall(&codeBlock, TCPIP_GET_CAPAB, &regs, REGS_MAIN, REGS_MAIN);
    if((regs.Bytes.L & (1 << 5)) == 0) {
        PrintError("This TCP/IP implementation does not support passive TCP connections.");
        return false;
    }

    regs.Bytes.B = 0;
    UnapiCall(&codeBlock, TCPIP_TCP_ABORT, &regs, REGS_MAIN, REGS_NONE);

    tcpConnectionParameters.remoteIP[0] = 0;
    tcpConnectionParameters.remoteIP[1] = 0;
    tcpConnectionParameters.remoteIP[2] = 0;
    tcpConnectionParameters.remoteIP[3] = 0;
    tcpConnectionParameters.localPort = (uint)initializationData;
    tcpConnectionParameters.userTimeout = 0;
    tcpConnectionParameters.flags = 1; //passive connection

    return true;
}

void ShutDownTransport()
{
    AbortTcpConnection();
}

void DoTransportStuff()
{
    byte tcpState;

    LetTcpipBreathe();
    if(connectionNumber == -1)
        return;

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
            connectionNumber = -1;
        }
    }
    else {
        connectionNumber = -1;
    }

    if (connectionNumber == -1) connectionIsEstablished = false;
}

bool ClientIsConnected()
{
    return connectionIsEstablished;
}

bool WaitForClientConnection()
{
    byte error;

    if(connectionNumber != -1)
        return true; //Already listening

    regs.Words.HL = (int)&tcpConnectionParameters;
    UnapiCall(&codeBlock, TCPIP_TCP_OPEN, &regs, REGS_MAIN, REGS_MAIN);
    error = (byte)regs.Bytes.A;
    if(error == (byte)ERR_NO_FREE_CONN) {
        PrintError("No free TCP connections available");
        return false;
    }
    else if(error == (byte)ERR_NO_NETWORK) {
        PrintError("No network connection available");
        return false;
    }
    else if(error != 0) {
        sprintf(errorMessageBuffer, "Unexpected error when opening TCP connection: %d", regs.Bytes.A);
        PrintError(errorMessageBuffer);
        return false;
    }

    connectionNumber = regs.Bytes.B;
    connectionIsEstablished = false;

    return true;
}

void DisconnectClient()
{
    CloseTcpConnection(false);
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
        getDataBuffer.dataPointer = &(getDataBuffer.data[1]); //dummy if got just 1 byte, but doesn't matter
        getDataBuffer.remainingData = regs.Words.BC - 1;
        return getDataBuffer.data[0];
    }
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

static void CloseTcpConnection(bool abort)
{
    if(connectionNumber == -1) return;

    regs.Bytes.B = connectionNumber;
    UnapiCall(&codeBlock, abort ? TCPIP_TCP_ABORT : TCPIP_TCP_CLOSE, &regs, REGS_MAIN, REGS_NONE);
    connectionIsEstablished = false;
    connectionNumber = -1;
}

static void PrintError(char* errorMessage)
{
    printf("*** %s\r\n", errorMessage);
}