#ifndef __ENV_H
#define __ENV_H

#include "types.h"

// This method is called continuously by the server,
// here the program can do whatever maintenance tasks it needs
void DoEnvironmentStuff();

// The server will terminate itself when this method returns true
bool MustTerminateServer();

// Is the specified address safe to execute (for OPC "Execute code" command)?
bool CanExecuteAtAddress(byte* address);

// Is the specified address safe to write (for OPC "Write to memory" command?)
bool CanWriteAtAddress(byte* address);

// Display a message to the user
void Print(char* text);

#endif