#ifndef __OPCS_SERVER_H
#define __OPCS_SERVER_H

/*
Starts the OPC server.

- Returns the reason why the server stopped:
  0: MustTerminateServer() returned true
  1: InitializeTransport() returned false
  2: WaitForClientConnection() returned false
  
- DoEnvironmentStuff() and DoTransportStuff() will be invoked continuously
*/
int StartOpcServer(void* transportInitData, bool _verbose);

#endif
