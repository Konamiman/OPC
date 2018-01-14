#ifndef __ENV_H
#define __ENV_H

#include "types.h"

void DoEnvironmentStuff();
bool MustTerminateServer();
bool CanExecuteAtAddress(byte* address);
bool CanWriteAtAddress(byte* address);
void Print(char* text);

#endif