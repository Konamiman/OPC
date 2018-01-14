#ifndef __OPCS_SERVER_H
#define __OPCS_SERVER_H

/*
Starts the OPC server.

- It will run until MustTerminateServer() return true
- DoEnvironmentStuff() and DoTransportStuff() will be invoked continuously
*/
int StartOpcServer(void* transportInitData, bool _verbose);

#endif
