#ifndef __TRANSPORT_H
#define __TRANSPORT_H

#include "types.h"
#include "asm.h"

bool InitializeTransport();
void ShutDownTransport();

void DoTransportStuff();
bool ClientIsConnected();
void WaitForClientConnection();
void DisconnectClient();

int GetByteFromConnection();
void SendBytes(byte* data, uint length, bool push);

#endif