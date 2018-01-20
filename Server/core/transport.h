#ifndef __TRANSPORT_H
#define __TRANSPORT_H

#include "types.h"

#define SEND_CHUNK_SIZE 512

bool InitializeTransport(void* initializationData);
void ShutDownTransport();

void DoTransportStuff();
bool ClientIsConnected();
bool WaitForClientConnection();
void DisconnectClient();

int GetByteFromConnection();
void SendBytes(byte* data, uint length, bool push);

#endif