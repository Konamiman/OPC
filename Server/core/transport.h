#ifndef __TRANSPORT_H
#define __TRANSPORT_H

#include "types.h"

#define SEND_CHUNK_SIZE 512

// Initialize the transport layer, initializationData can be anything
// (implementation dependant), called once when the server starts;
// return true if ok, false on error
bool InitializeTransport(void* initializationData);

// Deinitialize the transport layer, called once when the server terminates
// (unless InitializeTransport returned false)
void ShutDownTransport();

// This method is called continuously by the server,
// here the transport layer can do whatever maintenance tasks it needs
void DoTransportStuff();

// Is a client currently connected to the server?
bool ClientIsConnected();

// Wait (synchronously or not) for a client connection, after that
// ClientIsConnected() must return the appropriate value;
// return false if there is a "fatal error" (the server will be terminated)
// e.g. network connection is unavailable
bool WaitForClientConnection();

// Disconnect the currently connected client
void DisconnectClient();

// Read one byte from the current client connection,
// return -1 if no data is available to read
int GetByteFromConnection();

// Send the specified bytes to the current client connection,
// "push" means send immediately (might be dummy depending on the implementation)
void SendBytes(byte* data, uint length, bool push);

#endif