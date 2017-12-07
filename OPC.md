# Obsolete Procedure Call 1.0 

By Konamiman, 12/2017

## Introduction 

Obsolete Procedure Call (OPC) is a protocol intended for performing remote access to a machine that is controlled (or simulates being controlled) by a Z80 processor. It is designed to minimize the number of bytes transmitted over the communication channel, therefore command and responses are mostly plain binary data not suitable for direct human consumption. 

In this document we define the "server" as the machine that is being accessed, and the "client" as the machine that performs the remote access. 

## Communication channel 

The OPC protocol does not define the communication channel over with commands and responses are exchanged, it only requires it to be a one-to-one, bidirectional, reliable byte stream, that meaning that there are no packet boundaries (or they are not directly visible to the client and the server) and that data bytes are guaranteed to not be lost, to be delivered unaltered, and to reach their destination in the same order in which they were transmitted. A TCP stream is a perfect fit, a serial port with the appropriate error checking configuration in place is also a good candidate. 

OPC does not define either how the communication channel is established, or how clients discover the availability of a server. 

## General protocol design 

OPC is a simple command-response protocol. Given a proper communication channel, one of the ends is the client and the other one is the server (the protocol does not provide a means to reverse these roles). The client always initiates the communication by sending a command, then the server executes it and returns a response. 

Simultaneous commands are not supported: the server always finishes processing the current command before retrieving and processing the next one. However, the client may send a sequence of commands in one row instead of waiting for completion of each command before sending the next one; in this case, the server is expected to process the received commands in the same order in which they are received (and to send the responses in the proper order too, of course). Flow control is provided by the communication channel itself. 

The protocol is intended to be a very low-level way to control a Z80 processor, it provides just five operations that simulate what a local program would be able to do, namely: 

* Execute code 
* Read from memory 
* Write to memory 
* Read from a port 
* Write to a port 

Given this set of operations, a client can control the remote Z80 almost the same way as a local program would do (care must be taken not to destroy the server program itself, of course; the server must document what are the "forbidden" memory areas so that the client does not accesses them, but how this is done is outside of the scope of this specification). 

## Command and response format 

An OPC command consists of one byte whose high nibble contains the command code; the low nibble may contain a parameter for the command. Following this first byte there are zero or more command specific data bytes. The length of this additional data is either implicit by the first command byte or explicitly specified. 

       Command parameter 
             |  
    +-----------+  +-----------+  +-----------+ 
    |     |     |  |           |  |           |  ... 
    +-----------+  +-----------+  +-----------+ 
       |             Additional command data   
    Command code       

The response consists of one of the following: 

_If the command is completed successfully:_ a zero byte followed by zero or more command specific response data bytes. The length of this response data is either implicit by the executed command or explicitly specified. 

    +-----------+  +-----------+  +-----------+ 
    |     0     |  |           |  |           |  ... 
    +-----------+  +-----------+  +-----------+ 
                           Response data 

_If an error prevents the command from being completed:_ one byte containing the length in bytes of an ASCII encoded error message, followed by the error message bytes. 

    +-----------+  +-----------+  +-----------+  +-----------+  +-----------+ 
    |     4     |  |    'N'    |  |    'O'    |  |    'K'    |  |    '!'    | 
    +-----------+  +-----------+  +-----------+  +-----------+  +-----------+ 
    Error message length               Error message in ASCII 

This specification does not define the possible error conditions nor the error messages. Given the simplicity of the protocol, all the operations are likely to succeed; however, as mentioned before the server could forbid the client from executing code or writing memory if this would compromise the stability of the server (writing to a system work area, for example), and in this case it is appropriate to refuse the command and return an error message such as "Access forbidden". 

All two-byte values are sent in low-endian format (low byte first). 

# Commands description 

This section describes each available command in detail. The client must not send any command not listed here; upon reception of an unknown command the server must return an error message and if possible close the communication channel (since the length in bytes of such a command is not known and therefore client and server are out of sync from that point). 

For each command the following information is given: 

* The **command code** goes in the high nibble of the first command byte, and uniquely identifies the command being sent. 
* The **command parameter** goes in the low nibble of the first command byte. 
* The **command data** consists of all the command bytes that follow the first one. 
* The **response data** is all the data sent by the server in response to a command after successful command execution, not including the initial zero byte. 

An example transaction (command and response sequence) is given as well. All the numbers in the examples are hexadecimal values. 

Remember that all the commands may return an error message instead of response data. 

## Ping command 

**Command code:** 0 

**Command parameter:** Any value. 

**Command data:** None. 

**Response data:**

* One byte that consists of the following: 
  * The low nibble is a copy of the command parameter. 
  * The high nibble is the length of the rest of the response data (that is, the number of bytes that follow the first one). 
* Zero or more bytes whose meaning is currently undefined (see remarks). 

### Remarks 

This command can be used to test the communication channel. It should normally succeed, but an error can be returned if for some reason the server is unable to execute any command. 

This command is designed so that additional data can be returned by the server past the first response data byte. This is intended as an extension mechanism for future versions of the protocol; as far as the current version is concerned, the server should never send additional data, but the client should be prepared to receive this data (so that compatibility with future versions of the protocol is guaranteed) and just ignore it. 

### Example transaction 

The client sends: 

    +----+ 
    | 07 | 
    +----+ 

The server responds: 

    +----+ +----+ 
    | 00 | | 07 | 
    +----+ +----+ 

An example of a response with additional data ("xx" can be anything): 

    +----+ +----+ +----+ +----+ +----+ 
    | 00 | | 37 | | xx | | xx | | xx | 
    +----+ +----+ +----+ +----+ +----+ 

## Execute code commad 

**Command code:** 1 

**Command parameter:**

Bits 0 and 1 specify which Z80 registers must be set up by the server before executing the code: 

* 0: AF
* 1: AF, BC, DE, HL 
* 2: AF, BC, DE, HL, IX, IY 
* 3: AF, BC, DE, HL, IX, IY, AF', BC', DE', HL' 

Bits 2 and 3 specify which Z80 registers must be examined and their values returned in the response data after the code is executed. The meaning of these bits is the same as in the case of bits 0 and 1. 

**Command data:**

* Two bytes with the address of the code to be executed, followed by... 
* The values of the Z80 registers to be set by the server before executing the code. How many bytes are sent depends on the lower two bits of the command parameter: from 2 if the value is 0, to 20 if the value is 3. The register values must be sent in the same order as they are listed in the "command parameter" section. 

Note that the Z80 register pairs are considered two-byte values and are therefore sent in little-endian format, that is: F, A, C, B, E, D, L, H... 

**Response data:**

The values of the Z80 registers after the code has been executed. As in the case of the initial values for the registers, how many data bytes are sent depends on the value specified in the command parameter (will range from 2 to 20), the register values are sent in the same order as they are listed, and the register pair values are sent in little-endian format. 

### Remarks 

Upon complete reception of this command the server will load the appropriate Z80 registers with the received values, then it will execute the code located at the specified address with a CALL instruction or equivalent, then it will read back the values of the appropriate Z80 registers and send them as the response data.  

The code to be executed must either have been preloaded by the server (and the client must be aware of that), or may have been loaded by the client itself with a previously executed "Write to memory" command. 

Alternatively, if the server considers that executing code at the specified address would be dangerous (it would crash the system of corrupt data), it can refuse to execute the command and return an error message instead. 

### Example transaction 

The client wants to execute the code that starts at address 1234h with the following register values: A=56h, DE=789Ah, L=BCh. Then it wants to read back the values of the IX and IY registers. 

The client begins by sending the following: 

    +----+ +----+ +----+ +----+ +----+ +----+ +----+ +----+ +----+ +----+ +----+ 
    | 19 | | 34 | | 12 | | 00 | | 56 | | 00 | | 00 | | 9A | | 78 | | BC | | 00  
    +----+ +----+ +----+ +----+ +----+ +----+ +----+ +----+ +----+ +----+ +----+ 
        |   Code address    F      A      C      B      E      D      L      H  
        |    
    Bits 01=1 (send AF-HL) 
    Bits 23=2 (receive AF-IY) 

Note that the values of registers F, BC and H are "don't care" in this case. In this example we are sending these values as zero but the client could send anything. 

The server sets the Z80 registers with the supplied values and executes the code in the specified address until a RET instruction or equivalent is reached. The state of the registers after the code execution is as follows: AF=1122h, BC=3344h, DE=5566h, HL=7788h, IX=99AAh, IY=BBCCh. Then it sends the following response: 

    +----+ +----+ +----+ +----+ +----+ +----+ +----+ +----+ +----+ 
    | 00 | | 22 | | 11 | | 44 | | 33 | | 66 | | 55 | | 88 | | 77 | 
    +----+ +----+ +----+ +----+ +----+ +----+ +----+ +----+ +----+ 
              F      A      C      B      E      D      L      H   

    +----+ +----+ +----+ +----+ 
    | AA | | 99 | | CC | | BB | 
    +----+ +----+ +----+ +----+ 
      IXl    IXh    IYl    IYh 

## Read from memory commad 

**Command code:** 2

**Command parameter:** One of:

* 1 to 15: The number of bytes to read. 
* Zero: Indicates that the number of bytes to read will be specified as part of the command data. 

**Command data:**

* Two bytes with the first memory address that will be read, followed by... 
* Two bytes with the number of bytes to read, **only if the command parameter is zero**.

**Response data:**

The bytes read from the server's Z80 memory. The length of the response data is always equal to the specified number of bytes to read. 

### Remarks 

This command reads a block of data from the server's Z80 memory address space, starting at a certain address. The server should just read the visible memory and output it to the communication channel, without performing any further processing. 

If the size of the block to be read is 15 bytes or less, the size can be specified either in the command parameter or in the command data. For 16 bytes to be read or more, the size must be specified in the command data. 

The client should not specify a block size of zero, but the server should be prepared to receive such a command and in that case it should do nothing and just return an empty response (with just the single zero byte indicating success). 

### Example transaction 

The client wants to read 5 bytes of the server's memory starting at address 1234h. It then sends one of the following: 

_Option 1:_

    +----+ +----+ +----+ 
    | 25 | | 34 | | 12 | 
    +----+ +----+ +----+ 
        |   Data address    
        |    
    Block length 

_Option 2:_

    +----+ +----+ +----+ +----+ +----+ 
    | 20 | | 34 | | 12 | | 05 | | 00 | 
    +----+ +----+ +----+ +----+ +----+ 
            Data address  Block length 

The server reads its own memory starting at the specified address, whose contents is: 11h, 22h, 33h, 44h, 55h. It then sends the following response: 

    +----+ +----+ +----+ +----+ +----+ +----+ 
    | 00 | | 11 | | 22 | | 33 | | 44 | | 55 | 
    +----+ +----+ +----+ +----+ +----+ +----+ 
             Memory contents 

## Write to memory commad 

**Command code:** 3

**Command parameter:** One of:

* 1 to 15: The number of bytes to be written. 
* Zero: Indicates that the number of bytes to be written will be specified as part of the command data. 

**Command data:**

* Two bytes with the first memory address that will be written, followed by... 
* Two bytes with the number of bytes to be written, **only if the command parameter is zero**; followed by... 
* The bytes to be written. 

**Response data:**

None. 

### Remarks 

This command writes a block of data to the server's Z80 memory address space, starting at a certain address. The server should just write the supplied bytes to the visible memory, without performing any further processing. 

The server may refuse to write to the specified address, if it knows beforehand that it is part of a sensible memory area (such as a system work area) and performing the write would crash the system or corrupt data. In that case, an error message must be returned and no memory must be written at all; partial writes are not supported. 

It may be the case that the specified memory address is part of a ROM area or equivalent, and writing to it has no actual effect. In that case the server must anyway perform the write operation and NOT return an error. Put it another way, the server's only obligation upon receiving this command is to try to write the specified data starting at the specified address; whether the memory contents actually change or not is not the server's concern, it is the client's responsibility to check back the memory contents (by using a "Read from memory" command) if it desires to do so. 

If the size of the block to be written is 15 bytes or less, the size can be specified either in the command parameter or in the command data. For 16 bytes to be written or more, the size must be specified in the command data. 

The client should not specify a block size of zero, but the server should be prepared to receive such a command and in that case it should do nothing and just return an empty response (with just the single zero byte indicating success). 

### Example transaction 

The client wants to write the data block 11h, 22h, 33h, 44h, 55h to the server's memory starting at address 1234h. It then sends one of the following: 

_Option 1:_

    +----+ +----+ +----+ +----+ +----+ +----+ +----+ +----+ 
    | 35 | | 34 | | 12 | | 11 | | 22 | | 33 | | 44 | | 55 | 
    +----+ +----+ +----+ +----+ +----+ +----+ +----+ +----+ 
        |   Data address     Data to be written 
        |    
    Block length 

_Option 2:_

    +----+ +----+ +----+ +----+ +----+ +----+ +----+ +----+ +----+ +----+ 
    | 30 | | 34 | | 12 | | 05 | | 00 | | 11 | | 22 | | 33 | | 44 | | 55 | 
    +----+ +----+ +----+ +----+ +----+ +----+ +----+ +----+ +----+ +----+ 
            Data address  Block length     Data to be written 

The server then writes the specified bytes to its own memory starting at the specified address, and returns: 

    +----+ 
    | 00 | 
    +----+ 

## Read from port(s) commad 

**Command code:** 4

**Command parameter:**

* Bits 0-2: one of:
  * 1 to 7: The number of bytes to read. 
  * Zero: Indicates that the number of bytes to read will be specified as part of the command data. 
* Bit 3: Set to increment the port number after reading each byte, reset to read all bytes from the same port. 

**Command data:**

* One byte with the first (or only) port number that will be read, followed by... 
* Two bytes with the number of bytes to read, **only if bits 0-2 of the command parameter are zero**.

**Response data:**

The bytes read from the server's Z80 port or ports. The length of the response data is always equal to the specified number of bytes to read. 

### Remarks 

This command reads a block of data from the server's Z80 ports space. If bit 3 of the command parameter is set, the bytes are read from consecutive ports, starting at the one with the specified number; otherwise, the same port is read repeatedly. The server should just read the port(s) and output the received data to the communication channel, without performing any further processing. 

If the size of the block to be read is 7 bytes or less, the size can be specified either in the command parameter or in the command data. For 8 bytes to be read or more, the size must be specified in the command data. 

If the port number autoincrement bit is set and the port number overflows (port FFh is read and there are more bytes left to read), then the process continues at port 0. 

The client should not specify a block size of zero, but the server should be prepared to receive such a command and in that case it should do nothing and just return an empty response (with just the single zero byte indicating success). 

### Example transaction 

The client wants to read 5 bytes from the server's ports 10h, 11h, 12h, 13h and 14h. It then sends one of the following: 

_Option 1:_

    +----+ +----+ 
    | 4D | | 10 | 
    +----+ +----+ 
        |  Port # 
        |    
    Bits 0-2: Block length 
    Bit 3 set (autoincrement port number) 

_Option 2:_

    +----+ +----+ +----+ +----+ 
    | 48 | | 10 | | 05 | | 00 | 
    +----+ +----+ +----+ +----+ 
           Port #  Block length 

(If the client wanted to read all five bytes from port 10h instead, the first command byte would be 45h in option 1 and 40h in option 2) 

The server reads its own ports space starting at the specified port number, obtaining these values: 11h, 22h, 33h, 44h, 55h. It then sends the following response: 

    +----+ +----+ +----+ +----+ +----+ +----+ 
    | 00 | | 11 | | 22 | | 33 | | 44 | | 55 | 
    +----+ +----+ +----+ +----+ +----+ +----+ 
             Ports contents 

## Write to port(s) commad 

**Command code:** 5 

**Command parameter:**

* Bits 0-2: one of: 
  * 1 to 7: The number of bytes to write. 
  * Zero: Indicates that the number of bytes to write will be specified as part of the command data. 
* Bit 3: Set to increment the port number after writing each byte, reset to write all bytes to the same port. 

**Command data:**

* One byte with the first (or only) port number that will be written, followed by... 
* Two bytes with the number of bytes to be written, **only if bits 0-2 of the command parameter are zero**; followed by... 
* The bytes to be written. 

**Response data:**

None. 

### Remarks 

This command writes a block of data to the server's Z80 ports space. If bit 3 of the command parameter is set, the bytes are written to consecutive ports, starting at the one with the specified number; otherwise, the same port is written to repeatedly. The server should just write the port(s) without performing any further processing. 

The server may refuse to write to the specified port(s), if it knows beforehand that performing the write would crash the system or corrupt data. In that case, an error message must be returned and no ports must be written at all; partial writes are not supported. 

It may be the case that the specified port(s) is/are not connected to any hardware or an equivalent situation, and writing to them has no actual effect. In that case the server must anyway perform the write operation and NOT return an error. Put it another way, the server's only obligation upon receiving this command is to try to write the specified data at the specified port(s); whether the write operation has actually any effect or not is not the server's concern, it is the client's responsibility to check back the ports (perhaps by using a "Read from ports" command) if it desires to do so. 

If the size of the block to be written is 7 bytes or less, the size can be specified either in the command parameter or in the command data. For 8 bytes to be written or more, the size must be specified in the command data. 

If the port number autoincrement bit is set and the port number overflows (port FFh is written to and there are more bytes left to write), then the process continues at port 0. 

The client should not specify a block size of zero, but the server should be prepared to receive such a command and in that case it should do nothing and just return an empty response (with just the single zero byte indicating success). 

### Example transaction 

The client wants to write the data block 11h, 22h, 33h, 44h, 55h to the server's ports 10h, 11h, 12h, 13h and 14h, respectively. It then sends one of the following: 

_Option 1:_

    +----+ +----+ +----+ +----+ +----+ +----+ +----+ 
    | 5D | | 10 | | 11 | | 22 | | 33 | | 44 | | 55 | 
    +----+ +----+ +----+ +----+ +----+ +----+ +----+ 
        |  Port #    Data to be written 
        |    
    Bits 0-2: Block length 
    Bit 3 set (autoincrement port number) 

_Option 2:_

    +----+ +----+ +----+ +----+ +----+ +----+ +----+ +----+ +----+ 
    | 58 | | 10 | | 05 | | 00 | | 11 | | 22 | | 33 | | 44 | | 55 | 
    +----+ +----+ +----+ +----+ +----+ +----+ +----+ +----+ +----+ 
           Port #  Block length     Data to be written 

(If the client wanted to write all five bytes to port 10h instead, the first command byte would be 55h in option 1 and 50h in option 2) 

The server then writes the specified bytes to its own ports space starting at the specified port number, and returns: 

    +----+ 
    | 00 | 
    +----+ 


 