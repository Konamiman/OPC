#ifndef __ENV_H
#define __ENV_H

#include "types.h"

void DoEnvironmentStuff();
bool MustTerminateServer();
bool CanExecuteAtAddress(uint address);

#endif