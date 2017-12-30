;--------------------------------------------------------
; File Created by SDCC : free open source ANSI-C Compiler
; Version 3.6.0 #9615 (MINGW64)
;--------------------------------------------------------
	.module base
	.optsdcc -mz80
	
;--------------------------------------------------------
; Public variables in this module
;--------------------------------------------------------
	.globl _StartOpcServer
	.globl _GetByteFromConnection
	.globl _SendBytes
	.globl _DisconnectClient
	.globl _WaitForClientConnection
	.globl _ClientIsConnected
	.globl _DoTransportStuff
	.globl _ShutDownTransport
	.globl _InitializeTransport
	.globl _CanWriteAtAddress
	.globl _CanExecuteAtAddress
	.globl _MustTerminateServer
	.globl _DoEnvironmentStuff
	.globl _AsmCall
	.globl _strlen
	.globl _printf
	.globl _executeCommandPayloadLengths
	.globl _verbose
	.globl _clientIsConnected
	.globl _readPortBuffer
	.globl _getDataBuffer
	.globl _pendingCommand
	.globl _regs
	.globl _ProcessReceivedByte
	.globl _ProcessFirstCommandByte
	.globl _ProcessNextCommandByte
	.globl _ProcessNextByteToWrite
	.globl _RunCompletedCommand
	.globl _SendPortBytes
	.globl _SendMemoryBytes
	.globl _LoadRegistersBeforeExecutingCode
	.globl _SendResponseAfterExecutingCode
	.globl _SendErrorMessage
	.globl _SendByte
	.globl _HandleConnectionLifetime
	.globl _ReadFromPort
	.globl _WriteToPort
;--------------------------------------------------------
; special function registers
;--------------------------------------------------------
;--------------------------------------------------------
; ram data
;--------------------------------------------------------
	.area _DATA
_regs::
	.ds 12
_pendingCommand::
	.ds 35
_getDataBuffer::
	.ds 516
_readPortBuffer::
	.ds 512
;--------------------------------------------------------
; ram data
;--------------------------------------------------------
	.area _INITIALIZED
_clientIsConnected::
	.ds 1
_verbose::
	.ds 1
_executeCommandPayloadLengths::
	.ds 4
;--------------------------------------------------------
; absolute external ram data
;--------------------------------------------------------
	.area _DABS (ABS)
;--------------------------------------------------------
; global & static initialisations
;--------------------------------------------------------
	.area _HOME
	.area _GSINIT
	.area _GSFINAL
	.area _GSINIT
;--------------------------------------------------------
; Home
;--------------------------------------------------------
	.area _HOME
	.area _HOME
;--------------------------------------------------------
; code
;--------------------------------------------------------
	.area _CODE
;base.c:143: void StartOpcServer(void* transportInitData, bool _verbose)
;	---------------------------------
; Function StartOpcServer
; ---------------------------------
_StartOpcServer::
;base.c:147: verbose = _verbose;
	ld	hl, #4+0
	add	hl, sp
	ld	a, (hl)
	ld	(#_verbose + 0),a
;base.c:148: if(!InitializeTransport(transportInitData))
	pop	bc
	pop	hl
	push	hl
	push	bc
	push	hl
	call	_InitializeTransport
	pop	af
	ld	a,l
	or	a, a
;base.c:149: return;
;base.c:151: while(!MustTerminateServer())
	ret	Z
00105$:
	call	_MustTerminateServer
	ld	a,l
	or	a, a
	jp	NZ,_ShutDownTransport
;base.c:153: DoEnvironmentStuff();
	call	_DoEnvironmentStuff
;base.c:154: DoTransportStuff();
	call	_DoTransportStuff
;base.c:155: HandleConnectionLifetime();
	call	_HandleConnectionLifetime
;base.c:157: datum = GetByteFromConnection();
	call	_GetByteFromConnection
	ld	b,l
;base.c:158: if(datum != -1) {
	ld	a,b
	inc	a
	jr	NZ,00126$
	ld	a,h
	inc	a
	jr	Z,00105$
00126$:
;base.c:159: ProcessReceivedByte((byte)datum);
	push	bc
	inc	sp
	call	_ProcessReceivedByte
	inc	sp
	jr	00105$
;base.c:163: ShutDownTransport();
	jp  _ShutDownTransport
;base.c:171: void ProcessReceivedByte(byte datum)
;	---------------------------------
; Function ProcessReceivedByte
; ---------------------------------
_ProcessReceivedByte::
	push	ix
	ld	ix,#0
	add	ix,sp
;base.c:173: switch(pendingCommand.state)
	ld	bc,#_pendingCommand+0
	ld	a,(bc)
	ld	e,a
	ld	a,#0x02
	sub	a, e
	jr	C,00106$
	ld	d,#0x00
	ld	hl,#00123$
	add	hl,de
	add	hl,de
;base.c:175: case PCMD_NONE:
	jp	(hl)
00123$:
	jr	00101$
	jr	00104$
	jr	00105$
00101$:
;base.c:176: pendingCommand.state = ProcessFirstCommandByte(datum);
	push	bc
	ld	a,4 (ix)
	push	af
	inc	sp
	call	_ProcessFirstCommandByte
	inc	sp
	ld	a,l
	pop	bc
	ld	(bc),a
;base.c:177: if(pendingCommand.state == PCMD_PARTIAL) {
	dec	a
	jr	NZ,00106$
;base.c:178: pendingCommand.buffer[0] = datum;
	ld	hl,#(_pendingCommand + 0x000a)
	ld	a,4 (ix)
	ld	(hl),a
;base.c:179: pendingCommand.bufferWritePointer = (byte*)&(pendingCommand.buffer[1]);
	ld	hl,#(_pendingCommand + 0x000b)
	ld	((_pendingCommand + 0x0021)), hl
;base.c:181: break;
	jr	00106$
;base.c:182: case PCMD_PARTIAL:
00104$:
;base.c:183: pendingCommand.state = ProcessNextCommandByte(datum);
	push	bc
	ld	a,4 (ix)
	push	af
	inc	sp
	call	_ProcessNextCommandByte
	inc	sp
	ld	a,l
	pop	bc
	ld	(bc),a
;base.c:184: break;
	jr	00106$
;base.c:185: case PCMD_WRITING:
00105$:
;base.c:186: pendingCommand.state = ProcessNextByteToWrite(datum);
	push	bc
	ld	a,4 (ix)
	push	af
	inc	sp
	call	_ProcessNextByteToWrite
	inc	sp
	ld	a,l
	pop	bc
	ld	(bc),a
;base.c:187: };
00106$:
;base.c:189: if(pendingCommand.state == PCMD_FULL) {
	ld	a,(bc)
	sub	a, #0x03
	jr	NZ,00109$
;base.c:190: RunCompletedCommand();
	push	bc
	call	_RunCompletedCommand
	pop	bc
;base.c:191: pendingCommand.state = PCMD_NONE;
	xor	a, a
	ld	(bc),a
00109$:
	pop	ix
	ret
;base.c:195: byte ProcessFirstCommandByte(byte datum)
;	---------------------------------
; Function ProcessFirstCommandByte
; ---------------------------------
_ProcessFirstCommandByte::
	push	ix
	ld	ix,#0
	add	ix,sp
	dec	sp
;base.c:197: byte commandCode = datum & 0xF0;
	ld	a,4 (ix)
	and	a, #0xf0
;base.c:199: if(commandCode == OPC_PING) {
	ld	c,a
	or	a, a
	jr	NZ,00104$
;base.c:201: if(verbose) print("- Received PING command\r\n");
	ld	a,(#_verbose + 0)
	or	a, a
	jr	Z,00102$
	ld	hl,#___str_0
	push	hl
	call	_printf
	pop	af
00102$:
;base.c:202: SendByte(0, false);
	ld	hl,#0x0000
	push	hl
	call	_SendByte
;base.c:203: SendByte(datum, true);
	ld	h,#0x01
	ex	(sp),hl
	inc	sp
	ld	a,4 (ix)
	push	af
	inc	sp
	call	_SendByte
	pop	af
;base.c:204: return PCMD_NONE;
	ld	l,#0x00
	jp	00113$
00104$:
;base.c:207: if(commandCode == OPC_EXECUTE) {
	ld	a,c
	sub	a, #0x10
	jr	NZ,00106$
;base.c:208: pendingCommand.commandCode = commandCode;
	ld	hl,#(_pendingCommand + 0x0001)
	ld	(hl),c
;base.c:209: pendingCommand.remainingBytes = (int)executeCommandPayloadLengths[datum & 3];
	ld	de,#_executeCommandPayloadLengths+0
	ld	a,4 (ix)
	and	a, #0x03
	ld	l, a
	ld	h,#0x00
	add	hl,de
	ld	c,(hl)
	ld	b,#0x00
	ld	((_pendingCommand + 0x0002)), bc
;base.c:210: pendingCommand.stateData.registers.input = pendingCommand.remainingBytes;
	ld	hl,#_pendingCommand + 6
	ld	(hl),c
;base.c:211: pendingCommand.stateData.registers.output = (byte)executeCommandPayloadLengths[(datum >> 2) & 3];
	ld	bc,#_pendingCommand + 7
	ld	a,4 (ix)
	rrca
	rrca
	and	a,#0x3f
	and	a, #0x03
	ld	l,a
	ld	h,#0x00
	add	hl,de
	ld	a,(hl)
	ld	(bc),a
;base.c:214: return PCMD_PARTIAL;
	ld	l,#0x01
	jr	00113$
00106$:
;base.c:217: if(commandCode == OPC_READ_MEM || commandCode == OPC_WRITE_MEM) {
	ld	a,c
	cp	a,#0x20
	jr	Z,00107$
	sub	a, #0x30
	jr	NZ,00108$
00107$:
;base.c:218: pendingCommand.commandCode = commandCode;
	ld	hl,#(_pendingCommand + 0x0001)
	ld	(hl),c
;base.c:219: pendingCommand.remainingBytes = ((datum & 0x0F) == 0 ? 4 : 2);
	ld	a,4 (ix)
	and	a, #0x0f
	jr	NZ,00115$
	ld	c,#0x04
	jr	00116$
00115$:
	ld	c,#0x02
00116$:
	ld	b,#0x00
	ld	((_pendingCommand + 0x0002)), bc
;base.c:223: return PCMD_PARTIAL;
	ld	l,#0x01
	jr	00113$
00108$:
;base.c:226: if(commandCode == OPC_READ_PORT || commandCode == OPC_WRITE_PORT) {
	ld	a,c
	cp	a,#0x40
	jr	Z,00110$
	sub	a, #0x50
	jr	NZ,00111$
00110$:
;base.c:227: pendingCommand.commandCode = commandCode;
	ld	hl,#(_pendingCommand + 0x0001)
	ld	(hl),c
;base.c:228: pendingCommand.remainingBytes = ((datum & 0x07) == 0 ? 3 : 1);
	ld	a,4 (ix)
	and	a, #0x07
	jr	NZ,00117$
	ld	c,#0x03
	jr	00118$
00117$:
	ld	c,#0x01
00118$:
	ld	b,#0x00
	ld	((_pendingCommand + 0x0002)), bc
;base.c:232: return PCMD_PARTIAL;
	ld	l,#0x01
	jr	00113$
00111$:
;base.c:236: print("*** Unknown command received, disconnecting\r\n");
	ld	hl,#___str_1
	push	hl
	call	_printf
;base.c:237: SendErrorMessage("Unknown command");
	ld	hl, #___str_2
	ex	(sp),hl
	call	_SendErrorMessage
	pop	af
;base.c:238: DisconnectClient();
	call	_DisconnectClient
;base.c:239: return PCMD_NONE;
	ld	l,#0x00
00113$:
	inc	sp
	pop	ix
	ret
___str_0:
	.ascii "- Received PING command"
	.db 0x0d
	.db 0x0a
	.db 0x00
___str_1:
	.ascii "*** Unknown command received, disconnecting"
	.db 0x0d
	.db 0x0a
	.db 0x00
___str_2:
	.ascii "Unknown command"
	.db 0x00
;base.c:242: byte ProcessNextCommandByte(byte datum)
;	---------------------------------
; Function ProcessNextCommandByte
; ---------------------------------
_ProcessNextCommandByte::
	push	ix
	ld	ix,#0
	add	ix,sp
	push	af
	dec	sp
;base.c:247: *(pendingCommand.bufferWritePointer) = datum;
	ld	bc,#_pendingCommand+0
	ld	hl, (#(_pendingCommand + 0x0021) + 0)
	ld	a,4 (ix)
	ld	(hl),a
;base.c:248: pendingCommand.bufferWritePointer++;
	ld	de, (#(_pendingCommand + 0x0021) + 0)
	inc	de
	ld	((_pendingCommand + 0x0021)), de
;base.c:249: pendingCommand.remainingBytes--;
	ld	hl, (#(_pendingCommand + 0x0002) + 0)
	ld	a,l
	add	a,#0xff
	ld	d,a
	ld	a,h
	adc	a,#0xff
	ld	e,a
	ld	hl,#(_pendingCommand + 0x0002)
	ld	(hl),d
	inc	hl
	ld	(hl),e
;base.c:251: if(pendingCommand.remainingBytes != 0)
	ld	a,e
	or	a,d
	jr	Z,00102$
;base.c:252: return PCMD_PARTIAL;
	ld	l,#0x01
	jp	00117$
00102$:
;base.c:254: if(pendingCommand.commandCode != OPC_WRITE_MEM && pendingCommand.commandCode != OPC_WRITE_PORT)
	ld	hl, #(_pendingCommand + 0x0001) + 0
	ld	e,(hl)
	ld	a,e
	sub	a, #0x30
	jr	NZ,00181$
	ld	a,#0x01
	jr	00182$
00181$:
	xor	a,a
00182$:
	ld	-1 (ix),a
	bit	0,-1 (ix)
	jr	NZ,00104$
	ld	a,e
	sub	a, #0x50
	jr	Z,00104$
;base.c:255: return PCMD_FULL;
	ld	l,#0x03
	jp	00117$
00104$:
;base.c:257: length = pendingCommand.buffer[0] & 0x07;
	ld	a, (#(_pendingCommand + 0x000a) + 0)
	and	a, #0x07
	ld	e,a
	ld	d,#0x00
;base.c:258: if(length == 0) {
	ld	a,d
	or	a,e
	jr	NZ,00107$
;base.c:259: length = *((uint*)&(pendingCommand.buffer[pendingCommand.commandCode == OPC_WRITE_MEM ? 3 : 2]));
	bit	0,-1 (ix)
	jr	Z,00119$
	ld	e,#0x03
	jr	00120$
00119$:
	ld	e,#0x02
00120$:
	ld	hl,#(_pendingCommand + 0x000a)
	ld	d,#0x00
	add	hl, de
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
00107$:
;base.c:261: if(length == 0) {
	ld	a,d
;base.c:262: return PCMD_NONE;
	or	a,e
	jr	NZ,00109$
	ld	l,a
	jp	00117$
00109$:
;base.c:265: pendingCommand.remainingBytes = length;
	ld	((_pendingCommand + 0x0002)), de
;base.c:266: if(pendingCommand.commandCode == OPC_WRITE_MEM) {
	ld	hl, #(_pendingCommand + 0x0001) + 0
	ld	e,(hl)
;base.c:267: address = *(byte**)&(pendingCommand.buffer[1]);
;base.c:268: pendingCommand.stateData.memWrite.pointer = address;
;base.c:266: if(pendingCommand.commandCode == OPC_WRITE_MEM) {
	ld	a,e
	sub	a, #0x30
	jr	NZ,00115$
;base.c:267: address = *(byte**)&(pendingCommand.buffer[1]);
	ld	de, (#(_pendingCommand + 0x000b) + 0)
;base.c:268: pendingCommand.stateData.memWrite.pointer = address;
	ld	((_pendingCommand + 0x0006)), de
;base.c:269: pendingCommand.stateData.memWrite.isErrored = !CanWriteAtAddress(address);
	push	bc
	push	de
	call	_CanWriteAtAddress
	pop	af
	ld	e,l
	pop	bc
	ld	a,e
	sub	a,#0x01
	ld	a,#0x00
	rla
	ld	e,a
	ld	hl,#(_pendingCommand + 0x0008)
	ld	(hl),e
;base.c:270: pendingCommand.stateData.memWrite.lockAddress = (bool)((pendingCommand.buffer[0] & (1<<3)) != 0);
	ld	a, (#(_pendingCommand + 0x000a) + 0)
	bit	3, a
	jr	Z,00121$
	ld	e,#0x01
	jr	00122$
00121$:
	ld	e,#0x00
00122$:
	ld	hl,#(_pendingCommand + 0x0009)
	ld	(hl),e
;base.c:274: if(verbose) 
	ld	a,(#_verbose + 0)
	or	a, a
	jr	Z,00116$
;base.c:277: pendingCommand.stateData.memWrite.lockAddress ? ", lock address" : "");
	ld	a, (#(_pendingCommand + 0x0009) + 0)
	or	a, a
	jr	Z,00123$
	ld	bc,#___str_4+0
	jr	00124$
00123$:
	ld	bc,#___str_5+0
00124$:
	inc	sp
	inc	sp
	push	bc
;base.c:276: pendingCommand.stateData.memWrite.pointer, pendingCommand.remainingBytes,
	ld	bc, (#(_pendingCommand + 0x0002) + 0)
	ld	de, (#(_pendingCommand + 0x0006) + 0)
;base.c:275: printf("- Received WRITE MEMORY command for address 0x%x, length=%u%s\r\n", 
	pop	hl
	push	hl
	push	hl
	push	bc
	push	de
	ld	hl,#___str_3
	push	hl
	call	_printf
	ld	hl,#8
	add	hl,sp
	ld	sp,hl
	jr	00116$
00115$:
;base.c:280: pendingCommand.stateData.portWrite.port = *(byte*)&(pendingCommand.buffer[1]);
	ld	a, (#(_pendingCommand + 0x000b) + 0)
	ld	(#(_pendingCommand + 0x0006)),a
;base.c:281: pendingCommand.stateData.portWrite.increment = (bool)((pendingCommand.buffer[0] & (1<<3)) != 0);
	ld	hl,#0x0007
	add	hl,bc
	ld	c,l
	ld	b,h
	ld	a, (#(_pendingCommand + 0x000a) + 0)
	bit	3, a
	jr	Z,00125$
	ld	a,#0x01
	jr	00126$
00125$:
	ld	a,#0x00
00126$:
	ld	(bc),a
;base.c:285: if(verbose) 
	ld	a,(#_verbose + 0)
	or	a, a
	jr	Z,00116$
;base.c:288: pendingCommand.stateData.portWrite.increment ? "yes" : "no");
	ld	a,(bc)
	or	a, a
	jr	Z,00127$
	ld	bc,#___str_7+0
	jr	00128$
00127$:
	ld	bc,#___str_8+0
00128$:
;base.c:287: pendingCommand.stateData.portWrite.port, pendingCommand.remainingBytes,
	ld	de, (#(_pendingCommand + 0x0002) + 0)
	ld	a, (#(_pendingCommand + 0x0006) + 0)
	ld	-3 (ix),a
	ld	-2 (ix),#0x00
;base.c:286: printf("- Received WRITE PORT command for port %u, length=%u, autoincrement=%s\r\n", 
	push	bc
	push	de
	ld	l,-3 (ix)
	ld	h,-2 (ix)
	push	hl
	ld	hl,#___str_6
	push	hl
	call	_printf
	ld	hl,#8
	add	hl,sp
	ld	sp,hl
00116$:
;base.c:290: return PCMD_WRITING;
	ld	l,#0x02
00117$:
	ld	sp, ix
	pop	ix
	ret
___str_3:
	.ascii "- Received WRITE MEMORY command for address 0x%x, length=%u%"
	.ascii "s"
	.db 0x0d
	.db 0x0a
	.db 0x00
___str_4:
	.ascii ", lock address"
	.db 0x00
___str_5:
	.db 0x00
___str_6:
	.ascii "- Received WRITE PORT command for port %u, length=%u, autoin"
	.ascii "crement=%s"
	.db 0x0d
	.db 0x0a
	.db 0x00
___str_7:
	.ascii "yes"
	.db 0x00
___str_8:
	.ascii "no"
	.db 0x00
;base.c:293: byte ProcessNextByteToWrite(byte datum)
;	---------------------------------
; Function ProcessNextByteToWrite
; ---------------------------------
_ProcessNextByteToWrite::
	push	ix
	ld	ix,#0
	add	ix,sp
;base.c:295: if(pendingCommand.commandCode == OPC_WRITE_MEM) {
	ld	hl, #(_pendingCommand + 0x0001) + 0
	ld	e,(hl)
;base.c:296: if(!pendingCommand.stateData.memWrite.isErrored) {
	ld	bc,#_pendingCommand + 6
;base.c:295: if(pendingCommand.commandCode == OPC_WRITE_MEM) {
	ld	a,e
	sub	a, #0x30
	jr	NZ,00108$
;base.c:296: if(!pendingCommand.stateData.memWrite.isErrored) {
	ld	a, (#(_pendingCommand + 0x0008) + 0)
	or	a, a
	jr	NZ,00109$
;base.c:297: *(pendingCommand.stateData.memWrite.pointer) = datum;
	ld	l, c
	ld	h, b
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	ld	a,4 (ix)
	ld	(de),a
;base.c:298: if(!pendingCommand.stateData.memWrite.lockAddress) {
	ld	a, (#_pendingCommand + 9)
	or	a, a
	jr	NZ,00109$
;base.c:299: pendingCommand.stateData.memWrite.pointer++;
	ld	l, c
	ld	h, b
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	inc	de
	ld	a,e
	ld	(bc),a
	inc	bc
	ld	a,d
	ld	(bc),a
	jr	00109$
00108$:
;base.c:304: WriteToPort(pendingCommand.stateData.portWrite.port, datum);
	ld	a,(bc)
	ld	d,a
	push	bc
	ld	a,4 (ix)
	push	af
	inc	sp
	push	de
	inc	sp
	call	_WriteToPort
	pop	af
	pop	bc
;base.c:305: if(pendingCommand.stateData.portWrite.increment) {
	ld	a, (#_pendingCommand + 7)
	or	a, a
	jr	Z,00109$
;base.c:306: pendingCommand.stateData.portWrite.port++;
	ld	a,(bc)
	inc	a
	ld	(bc),a
00109$:
;base.c:309: pendingCommand.remainingBytes--;
	ld	bc, (#(_pendingCommand + 0x0002) + 0)
	dec	bc
	ld	((_pendingCommand + 0x0002)), bc
;base.c:311: if(pendingCommand.remainingBytes == 0) {
	ld	a,b
	or	a,c
	jr	NZ,00115$
;base.c:313: if(pendingCommand.commandCode == OPC_WRITE_MEM && pendingCommand.stateData.memWrite.isErrored) {
	ld	a, (#(_pendingCommand + 0x0001) + 0)
	sub	a, #0x30
	jr	NZ,00111$
	ld	a, (#(_pendingCommand + 0x0008) + 0)
	or	a, a
	jr	Z,00111$
;base.c:314: SendErrorMessage("Can't write to this address, this space is used by the server");
	ld	hl,#___str_9
	push	hl
	call	_SendErrorMessage
	pop	af
	jr	00112$
00111$:
;base.c:317: SendByte(0, true);
	ld	hl,#0x0100
	push	hl
	call	_SendByte
	pop	af
00112$:
;base.c:319: return PCMD_NONE;
	ld	l,#0x00
	jr	00117$
00115$:
;base.c:322: return PCMD_WRITING;
	ld	l,#0x02
00117$:
	pop	ix
	ret
___str_9:
	.ascii "Can't write to this address, this space is used by the serve"
	.ascii "r"
	.db 0x00
;base.c:326: void RunCompletedCommand()
;	---------------------------------
; Function RunCompletedCommand
; ---------------------------------
_RunCompletedCommand::
	push	ix
	ld	ix,#0
	add	ix,sp
	ld	hl,#-14
	add	hl,sp
	ld	sp,hl
;base.c:336: if(pendingCommand.commandCode == OPC_EXECUTE) {
	ld	a,(#_pendingCommand + 1)
;base.c:337: address = (byte*)*((int*)&(pendingCommand.buffer[1]));
;base.c:336: if(pendingCommand.commandCode == OPC_EXECUTE) {
	ld	-1 (ix), a
	sub	a, #0x10
	jp	NZ,00128$
;base.c:337: address = (byte*)*((int*)&(pendingCommand.buffer[1]));
	ld	bc, (#(_pendingCommand + 0x000b) + 0)
;base.c:339: if(!CanExecuteAtAddress(address)) {
	push	bc
	push	bc
	call	_CanExecuteAtAddress
	pop	af
	pop	bc
	ld	a,l
	or	a, a
	jr	NZ,00112$
;base.c:341: if(verbose) printf("- Received EXECUTE command for address 0x%x, error: bad address\r\n", address);
	ld	a,(#_verbose + 0)
	or	a, a
	jr	Z,00102$
	push	bc
	ld	hl,#___str_10
	push	hl
	call	_printf
	pop	af
	pop	af
00102$:
;base.c:342: SendErrorMessage("Can't execute code at this address, this space is used by the server");            
	ld	hl,#___str_11
	push	hl
	call	_SendErrorMessage
	pop	af
	jp	00130$
00112$:
;base.c:344: else if(pendingCommand.stateData.registers.input == 22 || pendingCommand.stateData.registers.output == 22) {
	ld	a, (#(_pendingCommand + 0x0006) + 0)
	sub	a, #0x16
	jr	Z,00107$
	ld	a, (#(_pendingCommand + 0x0007) + 0)
	sub	a, #0x16
	jr	NZ,00108$
00107$:
;base.c:346: if(verbose) printf("- Received EXECUTE command for address 0x%x, error: alt regs not supported\r\n", address);
	ld	a,(#_verbose + 0)
	or	a, a
	jr	Z,00104$
	push	bc
	ld	hl,#___str_12
	push	hl
	call	_printf
	pop	af
	pop	af
00104$:
;base.c:347: SendErrorMessage("Setting alternate input/output registers is not supported by this server");
	ld	hl,#___str_13
	push	hl
	call	_SendErrorMessage
	pop	af
	jp	00130$
00108$:
;base.c:351: if(verbose) printf("- Received EXECUTE command for address 0x%x\r\n", address);
	ld	a,(#_verbose + 0)
	or	a, a
	jr	Z,00106$
	push	bc
	push	bc
	ld	hl,#___str_14
	push	hl
	call	_printf
	pop	af
	pop	af
	pop	bc
00106$:
;base.c:352: LoadRegistersBeforeExecutingCode(pendingCommand.stateData.registers.input-2);
	ld	a, (#(_pendingCommand + 0x0006) + 0)
	ld	d,a
	dec	d
	dec	d
	push	bc
	push	de
	inc	sp
	call	_LoadRegistersBeforeExecutingCode
	inc	sp
	pop	bc
;base.c:354: AsmCall((uint)address, &regs, REGS_ALL, REGS_ALL);
	ld	hl,#0x0303
	push	hl
	ld	hl,#_regs
	push	hl
	push	bc
	call	_AsmCall
	ld	hl,#6
	add	hl,sp
	ld	sp,hl
;base.c:356: SendResponseAfterExecutingCode(pendingCommand.stateData.registers.output-2);
	ld	hl, #(_pendingCommand + 0x0007) + 0
	ld	b,(hl)
	dec	b
	dec	b
	push	bc
	inc	sp
	call	_SendResponseAfterExecutingCode
	inc	sp
	jp	00130$
00128$:
;base.c:360: address = (byte*)*((int*)&(pendingCommand.buffer[1]));
;base.c:359: else if(pendingCommand.commandCode == OPC_READ_MEM) {
	ld	a,-1 (ix)
	sub	a, #0x20
	jp	NZ,00125$
;base.c:360: address = (byte*)*((int*)&(pendingCommand.buffer[1]));
	ld	hl, #(_pendingCommand + 0x000b) + 0
	ld	a,(hl)
	ld	-3 (ix),a
	inc	hl
	ld	a,(hl)
	ld	-2 (ix),a
	ld	a,-3 (ix)
	ld	-11 (ix),a
	ld	a,-2 (ix)
	ld	-10 (ix),a
;base.c:361: flags.lockAddress = (pendingCommand.buffer[0] & (1 << 3)) != 0;
	ld	hl,#0x0000
	add	hl,sp
	ld	-3 (ix),l
	ld	-2 (ix),h
	ld	a, (#(_pendingCommand + 0x000a) + 0)
	bit	3, a
	jr	Z,00132$
	ld	-6 (ix),#0x01
	jr	00133$
00132$:
	ld	-6 (ix),#0x00
00133$:
	ld	l,-3 (ix)
	ld	h,-2 (ix)
	ld	a,-6 (ix)
	ld	(hl),a
;base.c:362: length = pendingCommand.buffer[0] & 0x07;
	ld	a, (#(_pendingCommand + 0x000a) + 0)
	and	a, #0x07
	ld	-5 (ix),a
;base.c:363: if(length == 0) {
	ld	-4 (ix), #0x00
	ld	a, #0x00
	or	a,-5 (ix)
	jr	NZ,00115$
;base.c:364: length = *((uint*)&(pendingCommand.buffer[3]));
	ld	-8 (ix),#<((_pendingCommand + 0x000d))
	ld	-7 (ix),#>((_pendingCommand + 0x000d))
	ld	l,-8 (ix)
	ld	h,-7 (ix)
	ld	a,(hl)
	ld	-5 (ix),a
	inc	hl
	ld	a,(hl)
	ld	-4 (ix),a
00115$:
;base.c:367: if(verbose) 
	ld	a,(#_verbose + 0)
	or	a, a
	jr	Z,00117$
;base.c:369: flags.lockAddress ? ", lock address" : "");
	ld	l,-3 (ix)
	ld	h,-2 (ix)
	ld	a,(hl)
	or	a, a
	jr	Z,00134$
	ld	bc,#___str_16+0
	jr	00135$
00134$:
	ld	bc,#___str_17+0
00135$:
;base.c:368: printf("- Received READ MEMORY command for address 0x%x, length=%u%s\r\n", address, length,
	push	bc
	ld	l,-5 (ix)
	ld	h,-4 (ix)
	push	hl
	ld	l,-11 (ix)
	ld	h,-10 (ix)
	push	hl
	ld	hl,#___str_15
	push	hl
	call	_printf
	ld	hl,#8
	add	hl,sp
	ld	sp,hl
00117$:
;base.c:370: SendByte(0, false);
	ld	hl,#0x0000
	push	hl
	call	_SendByte
	pop	af
;base.c:371: SendMemoryBytes((byte*)address, length, flags.lockAddress);
	ld	l,-3 (ix)
	ld	h,-2 (ix)
	ld	b,(hl)
	push	bc
	inc	sp
	ld	l,-5 (ix)
	ld	h,-4 (ix)
	push	hl
	ld	l,-11 (ix)
	ld	h,-10 (ix)
	push	hl
	call	_SendMemoryBytes
	pop	af
	pop	af
	inc	sp
	jp	00130$
00125$:
;base.c:373: else if(pendingCommand.commandCode == OPC_READ_PORT) {
	ld	a,-1 (ix)
	sub	a, #0x40
	jp	NZ,00130$
;base.c:374: port = *((byte*)&(pendingCommand.buffer[1]));
	ld	a,(#(_pendingCommand + 0x000b) + 0)
	ld	-9 (ix),a
;base.c:375: flags.incrementPort = (pendingCommand.buffer[0] & (1 << 3)) != 0;
	ld	hl,#0x0000
	add	hl,sp
	ld	-8 (ix),l
	ld	-7 (ix),h
	ld	a, (#(_pendingCommand + 0x000a) + 0)
	bit	3, a
	jr	Z,00136$
	ld	-5 (ix),#0x01
	jr	00137$
00136$:
	ld	-5 (ix),#0x00
00137$:
	ld	l,-8 (ix)
	ld	h,-7 (ix)
	ld	a,-5 (ix)
	ld	(hl),a
;base.c:376: length = pendingCommand.buffer[0] & 0x07;
	ld	a,(#(_pendingCommand + 0x000a) + 0)
	ld	-5 (ix), a
	and	a, #0x07
	ld	-5 (ix), a
	ld	-13 (ix),a
;base.c:377: if(length == 0) {
	ld	-12 (ix), #0x00
	ld	a, #0x00
	or	a,-13 (ix)
	jr	NZ,00119$
;base.c:378: length = *((uint*)&(pendingCommand.buffer[2]));
	ld	-5 (ix),#<((_pendingCommand + 0x000c))
	ld	-4 (ix),#>((_pendingCommand + 0x000c))
	ld	l,-5 (ix)
	ld	h,-4 (ix)
	ld	a,(hl)
	ld	-13 (ix),a
	inc	hl
	ld	a,(hl)
	ld	-12 (ix),a
00119$:
;base.c:381: if(verbose) 
	ld	a,(#_verbose + 0)
	or	a, a
	jr	Z,00121$
;base.c:383: port, length, flags.incrementPort ? "yes" : "no");
	ld	l,-8 (ix)
	ld	h,-7 (ix)
	ld	a,(hl)
	or	a, a
	jr	Z,00138$
	ld	hl,#___str_19+0
	ld	-5 (ix),l
	ld	-4 (ix),h
	jr	00139$
00138$:
	ld	hl,#___str_20+0
	ld	-5 (ix),l
	ld	-4 (ix),h
00139$:
	ld	a,-9 (ix)
	ld	-3 (ix),a
	ld	-2 (ix),#0x00
;base.c:382: printf("- Received READ PORT command for port %u, length=%u, autoincrement=%s\r\n",
	ld	l,-5 (ix)
	ld	h,-4 (ix)
	push	hl
	ld	l,-13 (ix)
	ld	h,-12 (ix)
	push	hl
	ld	l,-3 (ix)
	ld	h,-2 (ix)
	push	hl
	ld	hl,#___str_18
	push	hl
	call	_printf
	ld	hl,#8
	add	hl,sp
	ld	sp,hl
00121$:
;base.c:384: SendByte(0, false);
	ld	hl,#0x0000
	push	hl
	call	_SendByte
	pop	af
;base.c:385: SendPortBytes(port, length, flags.incrementPort);
	ld	l,-8 (ix)
	ld	h,-7 (ix)
	ld	b,(hl)
	push	bc
	inc	sp
	pop	bc
	pop	hl
	push	hl
	push	bc
	push	hl
	ld	a,-9 (ix)
	push	af
	inc	sp
	call	_SendPortBytes
	pop	af
	pop	af
00130$:
	ld	sp, ix
	pop	ix
	ret
___str_10:
	.ascii "- Received EXECUTE command for address 0x%x, error: bad addr"
	.ascii "ess"
	.db 0x0d
	.db 0x0a
	.db 0x00
___str_11:
	.ascii "Can't execute code at this address, this space is used by th"
	.ascii "e server"
	.db 0x00
___str_12:
	.ascii "- Received EXECUTE command for address 0x%x, error: alt regs"
	.ascii " not supported"
	.db 0x0d
	.db 0x0a
	.db 0x00
___str_13:
	.ascii "Setting alternate input/output registers is not supported by"
	.ascii " this server"
	.db 0x00
___str_14:
	.ascii "- Received EXECUTE command for address 0x%x"
	.db 0x0d
	.db 0x0a
	.db 0x00
___str_15:
	.ascii "- Received READ MEMORY command for address 0x%x, length=%u%s"
	.db 0x0d
	.db 0x0a
	.db 0x00
___str_16:
	.ascii ", lock address"
	.db 0x00
___str_17:
	.db 0x00
___str_18:
	.ascii "- Received READ PORT command for port %u, length=%u, autoinc"
	.ascii "rement=%s"
	.db 0x0d
	.db 0x0a
	.db 0x00
___str_19:
	.ascii "yes"
	.db 0x00
___str_20:
	.ascii "no"
	.db 0x00
;base.c:389: void SendPortBytes(byte port, uint length, bool increment)
;	---------------------------------
; Function SendPortBytes
; ---------------------------------
_SendPortBytes::
	push	ix
	ld	ix,#0
	add	ix,sp
;base.c:391: uint remaining = length;
	ld	c,5 (ix)
	ld	b,6 (ix)
;base.c:394: if(length == 0) return;
	ld	a,b
	or	a,c
;base.c:396: while(remaining > 0) {
	jr	Z,00106$
00103$:
	ld	a,b
	or	a,c
	jr	Z,00106$
;base.c:397: sendSize = remaining > SEND_CHUNK_SIZE ? SEND_CHUNK_SIZE : remaining;
	xor	a, a
	cp	a, c
	ld	a,#0x02
	sbc	a, b
	jr	NC,00108$
	ld	de,#0x0200
	jr	00109$
00108$:
	ld	e, c
	ld	d, b
00109$:
;base.c:398: ReadFromPort(port, readPortBuffer, sendSize, increment);
	push	bc
	push	de
	ld	a,7 (ix)
	push	af
	inc	sp
	push	de
	ld	hl,#_readPortBuffer
	push	hl
	ld	a,4 (ix)
	push	af
	inc	sp
	call	_ReadFromPort
	ld	hl,#6
	add	hl,sp
	ld	sp,hl
	pop	de
	push	de
	ld	a,#0x01
	push	af
	inc	sp
	push	de
	ld	hl,#_readPortBuffer
	push	hl
	call	_SendBytes
	pop	af
	pop	af
	inc	sp
	pop	de
	pop	bc
;base.c:400: remaining -= sendSize;
	ld	a,c
	sub	a, e
	ld	c,a
	ld	a,b
	sbc	a, d
	ld	b,a
	jr	00103$
00106$:
	pop	ix
	ret
;base.c:404: void SendMemoryBytes(byte* address, uint length, bool lockAddress)
;	---------------------------------
; Function SendMemoryBytes
; ---------------------------------
_SendMemoryBytes::
	push	ix
	ld	ix,#0
	add	ix,sp
	ld	hl,#-6
	add	hl,sp
	ld	sp,hl
;base.c:410: if(length == 0) return;
	ld	a,7 (ix)
	or	a,6 (ix)
	jp	Z,00112$
;base.c:412: if(!lockAddress) {
	ld	a,8 (ix)
	or	a, a
	jr	NZ,00104$
;base.c:413: SendBytes(address, length, true);
	ld	a,#0x01
	push	af
	inc	sp
	ld	l,6 (ix)
	ld	h,7 (ix)
	push	hl
	ld	l,4 (ix)
	ld	h,5 (ix)
	push	hl
	call	_SendBytes
	pop	af
	pop	af
	inc	sp
;base.c:414: return;
	jp	00112$
00104$:
;base.c:417: remaining = length;
	ld	c,6 (ix)
	ld	b,7 (ix)
;base.c:418: while(remaining > 0) {
	ld	de,#_readPortBuffer+0
	ld	a,4 (ix)
	ld	-2 (ix),a
	ld	a,5 (ix)
	ld	-1 (ix),a
00106$:
	ld	a,b
	or	a,c
	jr	Z,00112$
;base.c:419: sendSize = remaining > SEND_CHUNK_SIZE ? SEND_CHUNK_SIZE : remaining;
	xor	a, a
	cp	a, c
	ld	a,#0x02
	sbc	a, b
	jr	NC,00114$
	ld	hl,#0x0200
	jr	00115$
00114$:
	ld	l, c
	ld	h, b
00115$:
	ld	-4 (ix),l
	ld	-3 (ix),h
;base.c:420: for(i=0; i<sendSize; i++) {
	ld	hl,#0x0000
	ex	(sp), hl
00110$:
	ld	a,-6 (ix)
	sub	a, -4 (ix)
	ld	a,-5 (ix)
	sbc	a, -3 (ix)
	jr	NC,00105$
;base.c:421: readPortBuffer[i] = *address;
	pop	iy
	push	iy
	add	iy, de
	ld	l,-2 (ix)
	ld	h,-1 (ix)
	ld	l,(hl)
	ld	0 (iy), l
;base.c:420: for(i=0; i<sendSize; i++) {
	inc	-6 (ix)
	jr	NZ,00110$
	inc	-5 (ix)
	jr	00110$
00105$:
;base.c:423: SendBytes(readPortBuffer, sendSize, true);
	push	bc
	push	de
	ld	a,#0x01
	push	af
	inc	sp
	ld	l,-4 (ix)
	ld	h,-3 (ix)
	push	hl
	ld	hl,#_readPortBuffer
	push	hl
	call	_SendBytes
	pop	af
	pop	af
	inc	sp
	pop	de
	pop	bc
;base.c:424: remaining -= sendSize;
	ld	a,c
	sub	a, -4 (ix)
	ld	c,a
	ld	a,b
	sbc	a, -3 (ix)
	ld	b,a
	jr	00106$
00112$:
	ld	sp, ix
	pop	ix
	ret
;base.c:428: void LoadRegistersBeforeExecutingCode(byte length)
;	---------------------------------
; Function LoadRegistersBeforeExecutingCode
; ---------------------------------
_LoadRegistersBeforeExecutingCode::
	push	ix
	ld	ix,#0
	add	ix,sp
;base.c:430: short* regsPointer = (short*)&(pendingCommand.buffer[3]);
	ld	bc,#_pendingCommand+13
;base.c:432: regs.Words.AF = regsPointer[0];
	ld	l, c
	ld	h, b
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	ld	(_regs), de
;base.c:434: length -=2;
	dec	4 (ix)
	dec	4 (ix)
;base.c:435: if(length == 0) return;
	ld	a,4 (ix)
	or	a, a
	jr	Z,00107$
;base.c:437: regs.Words.BC = regsPointer[1];
	ld	l, c
	ld	h, b
	inc	hl
	inc	hl
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	ld	((_regs + 0x0002)), de
;base.c:438: regs.Words.DE = regsPointer[2];
	ld	l, c
	ld	h, b
	ld	de, #0x0004
	add	hl, de
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	ld	((_regs + 0x0004)), de
;base.c:439: regs.Words.HL = regsPointer[3];
	ld	l, c
	ld	h, b
	ld	de, #0x0006
	add	hl, de
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	ld	((_regs + 0x0006)), de
;base.c:443: length -=6;
	ld	a,4 (ix)
	add	a,#0xfa
;base.c:444: if(length == 0) return;
	ld	4 (ix), a
	or	a, a
	jr	Z,00107$
;base.c:446: regs.Words.IX = regsPointer[4];
	ld	l, c
	ld	h, b
	ld	de, #0x0008
	add	hl, de
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	ld	((_regs + 0x0008)), de
;base.c:447: regs.Words.IY = regsPointer[5];
	ld	l, c
	ld	h, b
	ld	de, #0x000a
	add	hl, de
	ld	c,(hl)
	inc	hl
	ld	b,(hl)
	ld	((_regs + 0x000a)), bc
;base.c:450: length -=4;
	ld	a,4 (ix)
	add	a,#0xfc
;base.c:451: if(length == 0) return;
	ld	4 (ix), a
	or	a, a
	jr	NZ,00107$
00107$:
	pop	ix
	ret
;base.c:461: void SendResponseAfterExecutingCode(byte length)
;	---------------------------------
; Function SendResponseAfterExecutingCode
; ---------------------------------
_SendResponseAfterExecutingCode::
	push	ix
	ld	ix,#0
	add	ix,sp
;base.c:463: short* regsPointer = (short*)&(pendingCommand.buffer[3]);
	ld	bc,#_pendingCommand + 13
;base.c:464: pendingCommand.buffer[2] = 0; //First byte of "ok" response
	ld	hl,#(_pendingCommand + 0x000c)
	ld	(hl),#0x00
;base.c:466: regsPointer[0] = regs.Words.AF;
	ld	de, (#_regs + 0)
	ld	l, c
	ld	h, b
	ld	(hl),e
	inc	hl
	ld	(hl),d
;base.c:468: if(length > 2) {
	ld	a,#0x02
	sub	a, 4 (ix)
	jr	NC,00104$
;base.c:469: regsPointer[1] = regs.Words.BC;
	push	bc
	pop	iy
	inc	iy
	inc	iy
	ld	hl, (#_regs + 2)
	ld	0 (iy),l
	ld	1 (iy),h
;base.c:470: regsPointer[2] = regs.Words.DE;
	ld	iy,#0x0004
	add	iy, bc
	ld	hl, (#_regs + 4)
	ld	0 (iy),l
	ld	1 (iy),h
;base.c:471: regsPointer[3] = regs.Words.HL;
	ld	iy,#0x0006
	add	iy, bc
	ld	hl, (#_regs + 6)
	ld	0 (iy),l
	ld	1 (iy),h
;base.c:475: if(length > 8) {
	ld	a,#0x08
	sub	a, 4 (ix)
	jr	NC,00104$
;base.c:476: regsPointer[4] = regs.Words.IX;
	ld	iy,#0x0008
	add	iy, bc
	ld	hl, (#_regs + 8)
	ld	0 (iy),l
	ld	1 (iy),h
;base.c:477: regsPointer[5] = regs.Words.IY;
	ld	hl,#0x000a
	add	hl,bc
	ld	c,l
	ld	b,h
	ld	de, (#_regs + 10)
	ld	a,e
	ld	(bc),a
	inc	bc
	ld	a,d
	ld	(bc),a
;base.c:480: if(length > 12) {
00104$:
;base.c:486: SendBytes((byte*)&(pendingCommand.buffer[2]), length+1, true);
	ld	c,4 (ix)
	ld	b,#0x00
	inc	bc
	ld	a,#0x01
	push	af
	inc	sp
	push	bc
	ld	hl,#(_pendingCommand + 0x000c)
	push	hl
	call	_SendBytes
	pop	af
	pop	af
	inc	sp
	pop	ix
	ret
;base.c:489: void SendErrorMessage(char* message)
;	---------------------------------
; Function SendErrorMessage
; ---------------------------------
_SendErrorMessage::
;base.c:491: byte length = strlen(message);
	pop	bc
	pop	hl
	push	hl
	push	bc
	push	hl
	call	_strlen
	pop	af
	ld	b,l
;base.c:492: SendByte(length, false);
	push	bc
	xor	a, a
	push	af
	inc	sp
	push	bc
	inc	sp
	call	_SendByte
	pop	af
	pop	bc
;base.c:493: SendBytes((byte*)message, length, true);
	ld	c,b
	ld	b,#0x00
	ld	a,#0x01
	push	af
	inc	sp
	push	bc
	ld	hl, #5
	add	hl, sp
	ld	c, (hl)
	inc	hl
	ld	b, (hl)
	push	bc
	call	_SendBytes
	pop	af
	pop	af
	inc	sp
	ret
;base.c:496: void SendByte(byte datum, bool push)
;	---------------------------------
; Function SendByte
; ---------------------------------
_SendByte::
;base.c:498: SendBytes(&datum, 1, push);
	ld	hl,#0x0002
	add	hl,sp
	ld	c,l
	ld	b,h
	ld	hl, #3+0
	add	hl, sp
	ld	a, (hl)
	push	af
	inc	sp
	ld	hl,#0x0001
	push	hl
	push	bc
	call	_SendBytes
	pop	af
	pop	af
	inc	sp
	ret
;base.c:501: void HandleConnectionLifetime()
;	---------------------------------
; Function HandleConnectionLifetime
; ---------------------------------
_HandleConnectionLifetime::
;base.c:503: bool wasPreviouslyConnected = clientIsConnected;
	ld	hl,#_clientIsConnected + 0
	ld	c, (hl)
;base.c:504: clientIsConnected = ClientIsConnected();
	push	bc
	call	_ClientIsConnected
	pop	bc
	ld	iy,#_clientIsConnected
	ld	0 (iy),l
;base.c:506: if(clientIsConnected && !wasPreviouslyConnected) {
	ld	a,0 (iy)
	or	a, a
	jr	Z,00102$
	ld	a,c
	or	a, a
	jr	NZ,00102$
;base.c:507: print("Client connected!\r\n");
	push	bc
	ld	hl,#___str_21
	push	hl
	call	_printf
	pop	af
	pop	bc
;base.c:508: getDataBuffer.dataPointer = &(getDataBuffer.data[0]);
	ld	hl,#_getDataBuffer
	ld	((_getDataBuffer + 0x0200)), hl
;base.c:509: getDataBuffer.remainingData = 0;
	ld	hl,#0x0000
	ld	((_getDataBuffer + 0x0202)), hl
00102$:
;base.c:512: if(!clientIsConnected) {
	ld	a,(#_clientIsConnected + 0)
;base.c:513: if(wasPreviouslyConnected) {
	or	a,a
	ret	NZ
	or	a,c
	jp	Z,_WaitForClientConnection
;base.c:514: print("Disconnected...\r\n");
	ld	hl,#___str_22
	push	hl
	call	_printf
	pop	af
;base.c:517: WaitForClientConnection();
	jp  _WaitForClientConnection
___str_21:
	.ascii "Client connected!"
	.db 0x0d
	.db 0x0a
	.db 0x00
___str_22:
	.ascii "Disconnected..."
	.db 0x0d
	.db 0x0a
	.db 0x00
;base.c:523: void ReadFromPort(byte portNumber, byte* destinationAddress, uint size, bool autoIncrement) __naked
;	---------------------------------
; Function ReadFromPort
; ---------------------------------
_ReadFromPort::
;base.c:555: __endasm;
	push	ix
	ld	ix,#4
	add	ix,sp
	ld	c,(ix) ;C=(first) port number
	ld	l,1(ix)
	ld	h,2(ix) ;HL=dest address
	ld	e,3(ix)
	ld	d,4(ix) ;DE=size
	ld	a,5(ix) ;A=autoIncrement
	or	a,a
	jr	z,RFP_DO
	ld	a,#0x0C ;INC C
	RFP_DO:
	ld	(RFP_INCC),a
	RFP_LOOP:
	ld	a,(hl)
	ini
	RFP_INCC:
	nop	;INC C on autoincrement, NOP otherwise
	dec	de
	ld	a,d
	or	e
	jr	nz,RFP_LOOP
	pop	ix
	ret
;base.c:558: void WriteToPort(byte portNumber, byte value)
;	---------------------------------
; Function WriteToPort
; ---------------------------------
_WriteToPort::
;base.c:571: __endasm;
	push	ix
	ld	ix,#4
	add	ix,sp
	ld	c,(ix) ;C=port number
	ld	a,1(ix) ;A=value
	out	(c),a
	pop	ix
	ret
	ret
	.area _CODE
	.area _INITIALIZER
__xinit__clientIsConnected:
	.db #0x00	; 0
__xinit__verbose:
	.db #0x00	; 0
__xinit__executeCommandPayloadLengths:
	.db #0x04	; 4
	.db #0x0a	; 10
	.db #0x0e	; 14
	.db #0x16	; 22
	.area _CABS (ABS)
