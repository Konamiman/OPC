# Obsolete Procedure Call

Obsolete Procedure Call (OPC) is a protocol intended for performing remote access to a machine that is controlled (or simulates being controlled) by a Z80 processor. It can be useful for testing hardware without having to physically use the target computer (using a modern machine with modern tools instead), or to test Z80 code agains a real Z80-based system.

* **[Protocol specification](OPC.md)**

* **[Server written in C](Server)** - includes an implementation for MSX computers.

* **[Client library for .NET](Client/dotNet)** - includes a sample application that remotely retrieves some information from a MSX computer.

Coming "soon": the MSX-DOS server "desconstructed" in separate modules to ease the development of servers for different environments/transports.
