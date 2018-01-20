;--------------------------------------------------------
; File Created by SDCC : free open source ANSI-C Compiler
; Version 3.6.0 #9615 (MINGW64)
;--------------------------------------------------------
	.module opcs_core
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
	.globl _Print
	.globl _CanWriteAtAddress
	.globl _CanExecuteAtAddress
	.globl _MustTerminateServer
	.globl _DoEnvironmentStuff
	.globl _AsmCall
	.globl _strlen
	.globl _sprintf
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
;opcs_core.c:126: int StartOpcServer(void* transportInitData, bool _verbose)
;	---------------------------------
; Function StartOpcServer
; ---------------------------------
_StartOpcServer::
;opcs_core.c:130: verbose = _verbose;
	ld	hl, #4+0
	add	hl, sp
	ld	a, (hl)
	ld	(#_verbose + 0),a
;opcs_core.c:131: if(!InitializeTransport(transportInitData))
	pop	bc
	pop	hl
	push	hl
	push	bc
	push	hl
	call	_InitializeTransport
	pop	af
	ld	a,l
	or	a, a
	jr	NZ,00102$
;opcs_core.c:132: return 1;
	ld	hl,#0x0001
	ret
00102$:
;opcs_core.c:134: pendingCommand.state = PCMD_NONE;
	ld	hl,#_pendingCommand
	ld	(hl),#0x00
;opcs_core.c:136: while(!MustTerminateServer())
00107$:
	call	_MustTerminateServer
	ld	a,l
	or	a, a
	jr	NZ,00109$
;opcs_core.c:138: DoEnvironmentStuff();
	call	_DoEnvironmentStuff
;opcs_core.c:139: DoTransportStuff();
	call	_DoTransportStuff
;opcs_core.c:140: if(!HandleConnectionLifetime()) {
	call	_HandleConnectionLifetime
	ld	a,l
	or	a, a
	jr	NZ,00104$
;opcs_core.c:141: return 2;
	ld	hl,#0x0002
	ret
00104$:
;opcs_core.c:144: datum = GetByteFromConnection();
	call	_GetByteFromConnection
	ld	b,l
;opcs_core.c:145: if(datum != -1) {
	ld	a,b
	inc	a
	jr	NZ,00132$
	ld	a,h
	inc	a
	jr	Z,00107$
00132$:
;opcs_core.c:146: ProcessReceivedByte((byte)datum);
	push	bc
	inc	sp
	call	_ProcessReceivedByte
	inc	sp
	jr	00107$
00109$:
;opcs_core.c:150: ShutDownTransport();
	call	_ShutDownTransport
;opcs_core.c:151: return 0;
	ld	hl,#0x0000
	ret
;opcs_core.c:159: void ProcessReceivedByte(byte datum)
;	---------------------------------
; Function ProcessReceivedByte
; ---------------------------------
_ProcessReceivedByte::
	call	___sdcc_enter_ix
;opcs_core.c:161: switch(pendingCommand.state)
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
;opcs_core.c:163: case PCMD_NONE:
	jp	(hl)
00123$:
	jr	00101$
	jr	00104$
	jr	00105$
00101$:
;opcs_core.c:164: pendingCommand.state = ProcessFirstCommandByte(datum);
	push	bc
	ld	a,4 (ix)
	push	af
	inc	sp
	call	_ProcessFirstCommandByte
	inc	sp
	ld	a,l
	pop	bc
	ld	(bc),a
;opcs_core.c:165: if(pendingCommand.state == PCMD_PARTIAL) {
	dec	a
	jr	NZ,00106$
;opcs_core.c:166: pendingCommand.buffer[0] = datum;
	ld	hl,#(_pendingCommand + 0x000a)
	ld	a,4 (ix)
	ld	(hl),a
;opcs_core.c:167: pendingCommand.bufferWritePointer = (byte*)&(pendingCommand.buffer[1]);
	ld	hl,#(_pendingCommand + 0x000b)
	ld	((_pendingCommand + 0x0021)), hl
;opcs_core.c:169: break;
	jr	00106$
;opcs_core.c:170: case PCMD_PARTIAL:
00104$:
;opcs_core.c:171: pendingCommand.state = ProcessNextCommandByte(datum);
	push	bc
	ld	a,4 (ix)
	push	af
	inc	sp
	call	_ProcessNextCommandByte
	inc	sp
	ld	a,l
	pop	bc
	ld	(bc),a
;opcs_core.c:172: break;
	jr	00106$
;opcs_core.c:173: case PCMD_WRITING:
00105$:
;opcs_core.c:174: pendingCommand.state = ProcessNextByteToWrite(datum);
	push	bc
	ld	a,4 (ix)
	push	af
	inc	sp
	call	_ProcessNextByteToWrite
	inc	sp
	ld	a,l
	pop	bc
	ld	(bc),a
;opcs_core.c:175: };
00106$:
;opcs_core.c:177: if(pendingCommand.state == PCMD_FULL) {
	ld	a,(bc)
	sub	a, #0x03
	jr	NZ,00109$
;opcs_core.c:178: RunCompletedCommand();
	push	bc
	call	_RunCompletedCommand
	pop	bc
;opcs_core.c:179: pendingCommand.state = PCMD_NONE;
	xor	a, a
	ld	(bc),a
00109$:
	pop	ix
	ret
;opcs_core.c:183: byte ProcessFirstCommandByte(byte datum)
;	---------------------------------
; Function ProcessFirstCommandByte
; ---------------------------------
_ProcessFirstCommandByte::
	call	___sdcc_enter_ix
	dec	sp
;opcs_core.c:185: byte commandCode = datum & 0xF0;
	ld	a,4 (ix)
	and	a, #0xf0
;opcs_core.c:187: if(commandCode == OPC_PING) {
	ld	c,a
	or	a, a
	jr	NZ,00104$
;opcs_core.c:189: if(verbose) print("- Received PING command\r\n");
	ld	a,(#_verbose + 0)
	or	a, a
	jr	Z,00102$
	ld	hl,#___str_0
	push	hl
	call	_printf
	pop	af
00102$:
;opcs_core.c:190: SendByte(0, false);
	ld	hl,#0x0000
	push	hl
	call	_SendByte
;opcs_core.c:191: SendByte(datum, true);
	ld	h,#0x01
	ex	(sp),hl
	inc	sp
	ld	a,4 (ix)
	push	af
	inc	sp
	call	_SendByte
	pop	af
;opcs_core.c:192: return PCMD_NONE;
	ld	l,#0x00
	jp	00113$
00104$:
;opcs_core.c:195: if(commandCode == OPC_EXECUTE) {
	ld	a,c
	sub	a, #0x10
	jr	NZ,00106$
;opcs_core.c:196: pendingCommand.commandCode = commandCode;
	ld	hl,#(_pendingCommand + 0x0001)
	ld	(hl),c
;opcs_core.c:197: pendingCommand.remainingBytes = (int)executeCommandPayloadLengths[datum & 3];
	ld	de,#_executeCommandPayloadLengths+0
	ld	a,4 (ix)
	and	a, #0x03
	ld	l, a
	ld	h,#0x00
	add	hl,de
	ld	c,(hl)
	ld	b,#0x00
	ld	((_pendingCommand + 0x0002)), bc
;opcs_core.c:198: pendingCommand.stateData.registers.input = pendingCommand.remainingBytes;
	ld	hl,#_pendingCommand + 6
	ld	(hl),c
;opcs_core.c:199: pendingCommand.stateData.registers.output = (byte)executeCommandPayloadLengths[(datum >> 2) & 3];
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
;opcs_core.c:202: return PCMD_PARTIAL;
	ld	l,#0x01
	jr	00113$
00106$:
;opcs_core.c:205: if(commandCode == OPC_READ_MEM || commandCode == OPC_WRITE_MEM) {
	ld	a,c
	cp	a,#0x20
	jr	Z,00107$
	sub	a, #0x30
	jr	NZ,00108$
00107$:
;opcs_core.c:206: pendingCommand.commandCode = commandCode;
	ld	hl,#(_pendingCommand + 0x0001)
	ld	(hl),c
;opcs_core.c:207: pendingCommand.remainingBytes = ((datum & 0x0F) == 0 ? 4 : 2);
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
;opcs_core.c:211: return PCMD_PARTIAL;
	ld	l,#0x01
	jr	00113$
00108$:
;opcs_core.c:214: if(commandCode == OPC_READ_PORT || commandCode == OPC_WRITE_PORT) {
	ld	a,c
	cp	a,#0x40
	jr	Z,00110$
	sub	a, #0x50
	jr	NZ,00111$
00110$:
;opcs_core.c:215: pendingCommand.commandCode = commandCode;
	ld	hl,#(_pendingCommand + 0x0001)
	ld	(hl),c
;opcs_core.c:216: pendingCommand.remainingBytes = ((datum & 0x07) == 0 ? 3 : 1);
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
;opcs_core.c:220: return PCMD_PARTIAL;
	ld	l,#0x01
	jr	00113$
00111$:
;opcs_core.c:224: print("*** Unknown command received, disconnecting\r\n");
	ld	hl,#___str_1
	push	hl
	call	_printf
;opcs_core.c:225: SendErrorMessage("Unknown command");
	ld	hl, #___str_2
	ex	(sp),hl
	call	_SendErrorMessage
	pop	af
;opcs_core.c:226: DisconnectClient();
	call	_DisconnectClient
;opcs_core.c:227: return PCMD_NONE;
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
;opcs_core.c:230: byte ProcessNextCommandByte(byte datum)
;	---------------------------------
; Function ProcessNextCommandByte
; ---------------------------------
_ProcessNextCommandByte::
	call	___sdcc_enter_ix
	push	af
;opcs_core.c:235: *(pendingCommand.bufferWritePointer) = datum;
	ld	hl, (#(_pendingCommand + 0x0021) + 0)
	ld	a,4 (ix)
	ld	(hl),a
;opcs_core.c:236: pendingCommand.bufferWritePointer++;
	ld	bc, (#(_pendingCommand + 0x0021) + 0)
	inc	bc
	ld	((_pendingCommand + 0x0021)), bc
;opcs_core.c:237: pendingCommand.remainingBytes--;
	ld	bc, (#(_pendingCommand + 0x0002) + 0)
	dec	bc
	ld	((_pendingCommand + 0x0002)), bc
;opcs_core.c:239: if(pendingCommand.remainingBytes != 0)
	ld	a,b
	or	a,c
	jr	Z,00102$
;opcs_core.c:240: return PCMD_PARTIAL;
	ld	l,#0x01
	jp	00117$
00102$:
;opcs_core.c:242: if(pendingCommand.commandCode != OPC_WRITE_MEM && pendingCommand.commandCode != OPC_WRITE_PORT)
	ld	hl, #(_pendingCommand + 0x0001) + 0
	ld	b,(hl)
	ld	a,b
	sub	a, #0x30
	jr	NZ,00181$
	ld	a,#0x01
	jr	00182$
00181$:
	xor	a,a
00182$:
	ld	c,a
	bit	0,c
	jr	NZ,00104$
	ld	a,b
	sub	a, #0x50
	jr	Z,00104$
;opcs_core.c:243: return PCMD_FULL;
	ld	l,#0x03
	jp	00117$
00104$:
;opcs_core.c:245: length = pendingCommand.buffer[0] & 0x07;
	ld	a, (#(_pendingCommand + 0x000a) + 0)
	and	a, #0x07
	ld	e,a
	ld	d,#0x00
;opcs_core.c:246: if(length == 0) {
	ld	a,d
	or	a,e
	jr	NZ,00107$
;opcs_core.c:247: length = *((uint*)&(pendingCommand.buffer[pendingCommand.commandCode == OPC_WRITE_MEM ? 3 : 2]));
	bit	0,c
	jr	Z,00119$
	ld	c,#0x03
	jr	00120$
00119$:
	ld	c,#0x02
00120$:
	ld	hl,#(_pendingCommand + 0x000a)
	ld	b,#0x00
	add	hl, bc
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
00107$:
;opcs_core.c:249: if(length == 0) {
	ld	a,d
;opcs_core.c:250: return PCMD_NONE;
	or	a,e
	jr	NZ,00109$
	ld	l,a
	jp	00117$
00109$:
;opcs_core.c:253: pendingCommand.remainingBytes = length;
	ld	((_pendingCommand + 0x0002)), de
;opcs_core.c:254: if(pendingCommand.commandCode == OPC_WRITE_MEM) {
	ld	hl, #(_pendingCommand + 0x0001) + 0
	ld	c,(hl)
;opcs_core.c:255: address = *(byte**)&(pendingCommand.buffer[1]);
	ld	hl,#_pendingCommand + 11
;opcs_core.c:256: pendingCommand.stateData.memWrite.pointer = address;
;opcs_core.c:254: if(pendingCommand.commandCode == OPC_WRITE_MEM) {
	ld	a,c
	sub	a, #0x30
	jr	NZ,00115$
;opcs_core.c:255: address = *(byte**)&(pendingCommand.buffer[1]);
	ld	c,(hl)
	inc	hl
	ld	b,(hl)
;opcs_core.c:256: pendingCommand.stateData.memWrite.pointer = address;
	ld	((_pendingCommand + 0x0006)), bc
;opcs_core.c:257: pendingCommand.stateData.memWrite.isErrored = !CanWriteAtAddress(address);
	push	bc
	call	_CanWriteAtAddress
	pop	af
	ld	a, l
	sub	a,#0x01
	ld	a,#0x00
	rla
	ld	c,a
	ld	hl,#(_pendingCommand + 0x0008)
	ld	(hl),c
;opcs_core.c:258: pendingCommand.stateData.memWrite.lockAddress = (bool)((pendingCommand.buffer[0] & (1<<3)) != 0);
	ld	bc,#_pendingCommand + 9
	ld	a, (#(_pendingCommand + 0x000a) + 0)
	bit	3, a
	jr	Z,00121$
	ld	a,#0x01
	jr	00122$
00121$:
	ld	a,#0x00
00122$:
	ld	(bc),a
;opcs_core.c:262: if(verbose) 
	ld	a,(#_verbose + 0)
	or	a, a
	jp	Z,00116$
;opcs_core.c:263: Printf3("- Received WRITE MEMORY command for address 0x%x, length=%u%s\r\n", 
	ld	a,(bc)
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
	ld	bc, (#(_pendingCommand + 0x0002) + 0)
	ld	de, (#(_pendingCommand + 0x0006) + 0)
	pop	hl
	push	hl
	push	hl
	push	bc
	push	de
	ld	hl,#___str_3
	push	hl
	ld	hl,#_readPortBuffer
	push	hl
	call	_sprintf
	ld	hl,#10
	add	hl,sp
	ld	sp,hl
	ld	hl,#_readPortBuffer
	push	hl
	call	_Print
	pop	af
	jr	00116$
00115$:
;opcs_core.c:268: pendingCommand.stateData.portWrite.port = *(byte*)&(pendingCommand.buffer[1]);
	ld	c,(hl)
	ld	hl,#(_pendingCommand + 0x0006)
	ld	(hl),c
;opcs_core.c:269: pendingCommand.stateData.portWrite.increment = (bool)((pendingCommand.buffer[0] & (1<<3)) != 0);
	ld	bc,#_pendingCommand + 7
	ld	a, (#(_pendingCommand + 0x000a) + 0)
	bit	3, a
	jr	Z,00125$
	ld	a,#0x01
	jr	00126$
00125$:
	ld	a,#0x00
00126$:
	ld	(bc),a
;opcs_core.c:273: if(verbose) 
	ld	a,(#_verbose + 0)
	or	a, a
	jr	Z,00116$
;opcs_core.c:274: Printf3("- Received WRITE PORT command for port %u, length=%u, autoincrement=%s\r\n", 
	ld	a,(bc)
	or	a, a
	jr	Z,00127$
	ld	bc,#___str_7+0
	jr	00128$
00127$:
	ld	bc,#___str_8+0
00128$:
	ld	de, (#(_pendingCommand + 0x0002) + 0)
	ld	a, (#(_pendingCommand + 0x0006) + 0)
	ld	-2 (ix),a
	ld	-1 (ix),#0x00
	push	bc
	push	de
	ld	l,-2 (ix)
	ld	h,-1 (ix)
	push	hl
	ld	hl,#___str_6
	push	hl
	ld	hl,#_readPortBuffer
	push	hl
	call	_sprintf
	ld	hl,#10
	add	hl,sp
	ld	sp,hl
	ld	hl,#_readPortBuffer
	push	hl
	call	_Print
	pop	af
00116$:
;opcs_core.c:278: return PCMD_WRITING;
	ld	l,#0x02
00117$:
	pop	af
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
;opcs_core.c:281: byte ProcessNextByteToWrite(byte datum)
;	---------------------------------
; Function ProcessNextByteToWrite
; ---------------------------------
_ProcessNextByteToWrite::
	call	___sdcc_enter_ix
;opcs_core.c:283: if(pendingCommand.commandCode == OPC_WRITE_MEM) {
	ld	hl, #(_pendingCommand + 0x0001) + 0
	ld	c,(hl)
;opcs_core.c:284: if(!pendingCommand.stateData.memWrite.isErrored) {
;opcs_core.c:283: if(pendingCommand.commandCode == OPC_WRITE_MEM) {
	ld	a,c
	sub	a, #0x30
	jr	NZ,00108$
;opcs_core.c:284: if(!pendingCommand.stateData.memWrite.isErrored) {
	ld	a, (#(_pendingCommand + 0x0008) + 0)
	or	a, a
	jr	NZ,00109$
;opcs_core.c:285: *(pendingCommand.stateData.memWrite.pointer) = datum;
	ld	hl, (#(_pendingCommand + 0x0006) + 0)
	ld	a,4 (ix)
	ld	(hl),a
;opcs_core.c:286: if(!pendingCommand.stateData.memWrite.lockAddress) {
	ld	a, (#_pendingCommand + 9)
	or	a, a
	jr	NZ,00109$
;opcs_core.c:287: pendingCommand.stateData.memWrite.pointer++;
	ld	bc, (#(_pendingCommand + 0x0006) + 0)
	inc	bc
	ld	((_pendingCommand + 0x0006)), bc
	jr	00109$
00108$:
;opcs_core.c:292: WriteToPort(pendingCommand.stateData.portWrite.port, datum);
	ld	hl, #(_pendingCommand + 0x0006) + 0
	ld	b,(hl)
	ld	a,4 (ix)
	push	af
	inc	sp
	push	bc
	inc	sp
	call	_WriteToPort
	pop	af
;opcs_core.c:293: if(pendingCommand.stateData.portWrite.increment) {
	ld	a, (#_pendingCommand + 7)
	or	a, a
	jr	Z,00109$
;opcs_core.c:294: pendingCommand.stateData.portWrite.port++;
	ld	a, (#(_pendingCommand + 0x0006) + 0)
	inc	a
	ld	(#(_pendingCommand + 0x0006)),a
00109$:
;opcs_core.c:297: pendingCommand.remainingBytes--;
	ld	bc, (#(_pendingCommand + 0x0002) + 0)
	dec	bc
	ld	((_pendingCommand + 0x0002)), bc
;opcs_core.c:299: if(pendingCommand.remainingBytes == 0) {
	ld	a,b
	or	a,c
	jr	NZ,00115$
;opcs_core.c:301: if(pendingCommand.commandCode == OPC_WRITE_MEM && pendingCommand.stateData.memWrite.isErrored) {
	ld	a, (#(_pendingCommand + 0x0001) + 0)
	sub	a, #0x30
	jr	NZ,00111$
	ld	a, (#(_pendingCommand + 0x0008) + 0)
	or	a, a
	jr	Z,00111$
;opcs_core.c:302: SendErrorMessage("Can't write to this address, this space is used by the server");
	ld	hl,#___str_9
	push	hl
	call	_SendErrorMessage
	pop	af
	jr	00112$
00111$:
;opcs_core.c:305: SendByte(0, true);
	ld	hl,#0x0100
	push	hl
	call	_SendByte
	pop	af
00112$:
;opcs_core.c:307: return PCMD_NONE;
	ld	l,#0x00
	jr	00117$
00115$:
;opcs_core.c:310: return PCMD_WRITING;
	ld	l,#0x02
00117$:
	pop	ix
	ret
___str_9:
	.ascii "Can't write to this address, this space is used by the serve"
	.ascii "r"
	.db 0x00
;opcs_core.c:314: void RunCompletedCommand()
;	---------------------------------
; Function RunCompletedCommand
; ---------------------------------
_RunCompletedCommand::
	call	___sdcc_enter_ix
	push	af
	push	af
	push	af
	dec	sp
;opcs_core.c:324: if(pendingCommand.commandCode == OPC_EXECUTE) {
	ld	a,(#_pendingCommand + 1)
	ld	-1 (ix),a
;opcs_core.c:325: address = (byte*)*((int*)&(pendingCommand.buffer[1]));
	ld	hl, (#(_pendingCommand + 0x000b) + 0)
	ld	-3 (ix),l
	ld	-2 (ix),h
;opcs_core.c:324: if(pendingCommand.commandCode == OPC_EXECUTE) {
	ld	a,-1 (ix)
	sub	a, #0x10
	jp	NZ,00128$
;opcs_core.c:325: address = (byte*)*((int*)&(pendingCommand.buffer[1]));
	ld	c,-3 (ix)
	ld	b,-2 (ix)
;opcs_core.c:327: if(!CanExecuteAtAddress(address)) {
	push	bc
	push	bc
	call	_CanExecuteAtAddress
	pop	af
	pop	bc
	ld	a,l
	or	a, a
	jr	NZ,00112$
;opcs_core.c:329: if(verbose) Printf1("- Received EXECUTE command for address 0x%x, error: bad address\r\n", address);
	ld	a,(#_verbose + 0)
	or	a, a
	jr	Z,00102$
	push	bc
	ld	hl,#___str_10
	push	hl
	ld	hl,#_readPortBuffer
	push	hl
	call	_sprintf
	pop	af
	pop	af
	ld	hl, #_readPortBuffer
	ex	(sp),hl
	call	_Print
	pop	af
00102$:
;opcs_core.c:330: SendErrorMessage("Can't execute code at this address, this space is used by the server");            
	ld	hl,#___str_11
	push	hl
	call	_SendErrorMessage
	pop	af
	jp	00130$
00112$:
;opcs_core.c:332: else if(pendingCommand.stateData.registers.input == 22 || pendingCommand.stateData.registers.output == 22) {
	ld	a, (#(_pendingCommand + 0x0006) + 0)
	sub	a, #0x16
	jr	Z,00107$
	ld	a, (#(_pendingCommand + 0x0007) + 0)
	sub	a, #0x16
	jr	NZ,00108$
00107$:
;opcs_core.c:334: if(verbose) Printf1("- Received EXECUTE command for address 0x%x, error: alt regs not supported\r\n", address);
	ld	a,(#_verbose + 0)
	or	a, a
	jr	Z,00104$
	push	bc
	ld	hl,#___str_12
	push	hl
	ld	hl,#_readPortBuffer
	push	hl
	call	_sprintf
	pop	af
	pop	af
	ld	hl, #_readPortBuffer
	ex	(sp),hl
	call	_Print
	pop	af
00104$:
;opcs_core.c:335: SendErrorMessage("Setting alternate input/output registers is not supported by this server");
	ld	hl,#___str_13
	push	hl
	call	_SendErrorMessage
	pop	af
	jp	00130$
00108$:
;opcs_core.c:339: if(verbose) Printf1("- Received EXECUTE command for address 0x%x\r\n", address);
	ld	a,(#_verbose + 0)
	or	a, a
	jr	Z,00106$
	push	bc
	push	bc
	ld	hl,#___str_14
	push	hl
	ld	hl,#_readPortBuffer
	push	hl
	call	_sprintf
	pop	af
	pop	af
	ld	hl, #_readPortBuffer
	ex	(sp),hl
	call	_Print
	pop	af
	pop	bc
00106$:
;opcs_core.c:340: LoadRegistersBeforeExecutingCode(pendingCommand.stateData.registers.input-2);
	ld	hl, #(_pendingCommand + 0x0006) + 0
	ld	d,(hl)
	dec	d
	dec	d
	push	bc
	push	de
	inc	sp
	call	_LoadRegistersBeforeExecutingCode
	inc	sp
	pop	bc
;opcs_core.c:342: AsmCall((uint)address, &regs, REGS_ALL, REGS_ALL);
	ld	hl,#0x0303
	push	hl
	ld	hl,#_regs
	push	hl
	push	bc
	call	_AsmCall
	pop	af
	pop	af
	pop	af
;opcs_core.c:344: SendResponseAfterExecutingCode(pendingCommand.stateData.registers.output-2);
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
;opcs_core.c:348: address = (byte*)*((int*)&(pendingCommand.buffer[1]));
;opcs_core.c:349: flags.lockAddress = (pendingCommand.buffer[0] & (1 << 3)) != 0;
	ld	a, (#(_pendingCommand + 0x000a) + 0)
	and	a, #0x08
	ld	e,a
;opcs_core.c:347: else if(pendingCommand.commandCode == OPC_READ_MEM) {
	ld	a,-1 (ix)
	sub	a, #0x20
	jp	NZ,00125$
;opcs_core.c:348: address = (byte*)*((int*)&(pendingCommand.buffer[1]));
	ld	c,-3 (ix)
	ld	b,-2 (ix)
;opcs_core.c:349: flags.lockAddress = (pendingCommand.buffer[0] & (1 << 3)) != 0;
	ld	hl,#0x0001
	add	hl,sp
	ld	-3 (ix),l
	ld	-2 (ix),h
	ld	a,e
	or	a, a
	jr	Z,00132$
	ld	e,#0x01
	jr	00133$
00132$:
	ld	e,#0x00
00133$:
	ld	l,-3 (ix)
	ld	h,-2 (ix)
	ld	(hl),e
;opcs_core.c:350: length = pendingCommand.buffer[0] & 0x07;
	ld	a, (#(_pendingCommand + 0x000a) + 0)
	and	a, #0x07
	ld	-5 (ix),a
;opcs_core.c:351: if(length == 0) {
	ld	-4 (ix), #0x00
	ld	a, #0x00
	or	a,-5 (ix)
	jr	NZ,00115$
;opcs_core.c:352: length = *((uint*)&(pendingCommand.buffer[3]));
	ld	de,#_pendingCommand + 13
	ld	a,(de)
	ld	-5 (ix),a
	inc	de
	ld	a,(de)
	ld	-4 (ix),a
00115$:
;opcs_core.c:355: if(verbose) 
	ld	a,(#_verbose + 0)
	or	a, a
	jr	Z,00117$
;opcs_core.c:356: Printf3("- Received READ MEMORY command for address 0x%x, length=%u%s\r\n", address, length,
	ld	l,-3 (ix)
	ld	h,-2 (ix)
	ld	a,(hl)
	or	a, a
	jr	Z,00134$
	ld	de,#___str_16+0
	jr	00135$
00134$:
	ld	de,#___str_17+0
00135$:
	push	bc
	push	de
	ld	l,-5 (ix)
	ld	h,-4 (ix)
	push	hl
	push	bc
	ld	hl,#___str_15
	push	hl
	ld	hl,#_readPortBuffer
	push	hl
	call	_sprintf
	ld	hl,#10
	add	hl,sp
	ld	sp,hl
	ld	hl,#_readPortBuffer
	push	hl
	call	_Print
	pop	af
	pop	bc
00117$:
;opcs_core.c:358: SendByte(0, false);
	push	bc
	ld	hl,#0x0000
	push	hl
	call	_SendByte
	pop	af
	pop	bc
;opcs_core.c:359: SendMemoryBytes((byte*)address, length, flags.lockAddress);
	ld	l,-3 (ix)
	ld	h,-2 (ix)
	ld	d,(hl)
	push	de
	inc	sp
	ld	l,-5 (ix)
	ld	h,-4 (ix)
	push	hl
	push	bc
	call	_SendMemoryBytes
	pop	af
	pop	af
	inc	sp
	jp	00130$
00125$:
;opcs_core.c:361: else if(pendingCommand.commandCode == OPC_READ_PORT) {
	ld	a,-1 (ix)
	sub	a, #0x40
	jp	NZ,00130$
;opcs_core.c:362: port = *((byte*)&(pendingCommand.buffer[1]));
	ld	a,(#(_pendingCommand + 0x000b) + 0)
	ld	-7 (ix),a
;opcs_core.c:363: flags.incrementPort = (pendingCommand.buffer[0] & (1 << 3)) != 0;
	ld	hl,#0x0001
	add	hl,sp
	ld	-5 (ix),l
	ld	-4 (ix),h
	ld	a,e
	or	a, a
	jr	Z,00136$
	ld	c,#0x01
	jr	00137$
00136$:
	ld	c,#0x00
00137$:
	ld	l,-5 (ix)
	ld	h,-4 (ix)
	ld	(hl),c
;opcs_core.c:364: length = pendingCommand.buffer[0] & 0x07;
	ld	a, (#(_pendingCommand + 0x000a) + 0)
	and	a, #0x07
	ld	c,a
	ld	b,#0x00
;opcs_core.c:365: if(length == 0) {
	ld	a,b
	or	a,c
	jr	NZ,00119$
;opcs_core.c:366: length = *((uint*)&(pendingCommand.buffer[2]));
	ld	hl,#_pendingCommand + 12
	ld	c,(hl)
	inc	hl
	ld	b,(hl)
00119$:
;opcs_core.c:369: if(verbose) 
	ld	a,(#_verbose + 0)
	or	a, a
	jr	Z,00121$
;opcs_core.c:370: Printf3("- Received READ PORT command for port %u, length=%u, autoincrement=%s\r\n",
	ld	l,-5 (ix)
	ld	h,-4 (ix)
	ld	a,(hl)
	or	a, a
	jr	Z,00138$
	ld	de,#___str_19+0
	jr	00139$
00138$:
	ld	de,#___str_20+0
00139$:
	ld	a,-7 (ix)
	ld	-3 (ix),a
	ld	-2 (ix),#0x00
	push	bc
	push	de
	push	bc
	ld	l,-3 (ix)
	ld	h,-2 (ix)
	push	hl
	ld	hl,#___str_18
	push	hl
	ld	hl,#_readPortBuffer
	push	hl
	call	_sprintf
	ld	hl,#10
	add	hl,sp
	ld	sp,hl
	ld	hl,#_readPortBuffer
	push	hl
	call	_Print
	pop	af
	pop	bc
00121$:
;opcs_core.c:372: SendByte(0, false);
	push	bc
	ld	hl,#0x0000
	push	hl
	call	_SendByte
	pop	af
	pop	bc
;opcs_core.c:373: SendPortBytes(port, length, flags.incrementPort);
	ld	l,-5 (ix)
	ld	h,-4 (ix)
	ld	d,(hl)
	push	de
	inc	sp
	push	bc
	ld	a,-7 (ix)
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
;opcs_core.c:377: void SendPortBytes(byte port, uint length, bool increment)
;	---------------------------------
; Function SendPortBytes
; ---------------------------------
_SendPortBytes::
	call	___sdcc_enter_ix
;opcs_core.c:379: uint remaining = length;
	ld	c,5 (ix)
	ld	b,6 (ix)
;opcs_core.c:382: if(length == 0) return;
	ld	a,b
	or	a,c
;opcs_core.c:384: while(remaining > 0) {
	jr	Z,00106$
00103$:
	ld	a,b
	or	a,c
	jr	Z,00106$
;opcs_core.c:385: sendSize = remaining > SEND_CHUNK_SIZE ? SEND_CHUNK_SIZE : remaining;
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
;opcs_core.c:386: ReadFromPort(port, readPortBuffer, sendSize, increment);
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
	pop	af
	pop	af
	pop	af
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
;opcs_core.c:388: remaining -= sendSize;
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
;opcs_core.c:392: void SendMemoryBytes(byte* address, uint length, bool lockAddress)
;	---------------------------------
; Function SendMemoryBytes
; ---------------------------------
_SendMemoryBytes::
	call	___sdcc_enter_ix
	push	af
	push	af
;opcs_core.c:398: if(length == 0) return;
	ld	a,7 (ix)
	or	a,6 (ix)
	jp	Z,00112$
;opcs_core.c:400: if(!lockAddress) {
	ld	a,8 (ix)
	or	a, a
	jr	NZ,00104$
;opcs_core.c:401: SendBytes(address, length, true);
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
;opcs_core.c:402: return;
	jr	00112$
00104$:
;opcs_core.c:405: remaining = length;
	ld	c,6 (ix)
	ld	b,7 (ix)
;opcs_core.c:406: while(remaining > 0) {
	ld	a,4 (ix)
	ld	-2 (ix),a
	ld	a,5 (ix)
	ld	-1 (ix),a
00106$:
	ld	a,b
	or	a,c
	jr	Z,00112$
;opcs_core.c:407: sendSize = remaining > SEND_CHUNK_SIZE ? SEND_CHUNK_SIZE : remaining;
	xor	a, a
	cp	a, c
	ld	a,#0x02
	sbc	a, b
	jr	NC,00114$
	ld	de,#0x0200
	jr	00115$
00114$:
	ld	e, c
	ld	d, b
00115$:
	inc	sp
	inc	sp
	push	de
;opcs_core.c:408: for(i=0; i<sendSize; i++) {
	ld	de,#0x0000
00110$:
	ld	a,e
	sub	a, -4 (ix)
	ld	a,d
	sbc	a, -3 (ix)
	jr	NC,00105$
;opcs_core.c:409: readPortBuffer[i] = *address;
	ld	iy,#_readPortBuffer
	add	iy, de
	ld	l,-2 (ix)
	ld	h,-1 (ix)
	ld	l,(hl)
	ld	0 (iy), l
;opcs_core.c:408: for(i=0; i<sendSize; i++) {
	inc	de
	jr	00110$
00105$:
;opcs_core.c:411: SendBytes(readPortBuffer, sendSize, true);
	push	bc
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
	pop	bc
;opcs_core.c:412: remaining -= sendSize;
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
;opcs_core.c:416: void LoadRegistersBeforeExecutingCode(byte length)
;	---------------------------------
; Function LoadRegistersBeforeExecutingCode
; ---------------------------------
_LoadRegistersBeforeExecutingCode::
	call	___sdcc_enter_ix
;opcs_core.c:418: short* regsPointer = (short*)&(pendingCommand.buffer[3]);
	ld	bc,#_pendingCommand+13
;opcs_core.c:420: regs.Words.AF = regsPointer[0];
	ld	l, c
	ld	h, b
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	ld	(_regs), de
;opcs_core.c:422: length -=2;
	dec	4 (ix)
	dec	4 (ix)
;opcs_core.c:423: if(length == 0) return;
	ld	a,4 (ix)
	or	a, a
	jr	Z,00107$
;opcs_core.c:425: regs.Words.BC = regsPointer[1];
	ld	l, c
	ld	h, b
	inc	hl
	inc	hl
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	ld	((_regs + 0x0002)), de
;opcs_core.c:426: regs.Words.DE = regsPointer[2];
	ld	l, c
	ld	h, b
	ld	de, #0x0004
	add	hl, de
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	ld	((_regs + 0x0004)), de
;opcs_core.c:427: regs.Words.HL = regsPointer[3];
	ld	l, c
	ld	h, b
	ld	de, #0x0006
	add	hl, de
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	ld	((_regs + 0x0006)), de
;opcs_core.c:431: length -=6;
	ld	a,4 (ix)
	add	a,#0xfa
;opcs_core.c:432: if(length == 0) return;
	ld	4 (ix), a
	or	a, a
	jr	Z,00107$
;opcs_core.c:434: regs.Words.IX = regsPointer[4];
	ld	l, c
	ld	h, b
	ld	de, #0x0008
	add	hl, de
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	ld	((_regs + 0x0008)), de
;opcs_core.c:435: regs.Words.IY = regsPointer[5];
	ld	l, c
	ld	h, b
	ld	de, #0x000a
	add	hl, de
	ld	c,(hl)
	inc	hl
	ld	b,(hl)
	ld	((_regs + 0x000a)), bc
;opcs_core.c:438: length -=4;
	ld	a,4 (ix)
	add	a,#0xfc
;opcs_core.c:439: if(length == 0) return;
	ld	4 (ix), a
	or	a, a
	jr	NZ,00107$
00107$:
	pop	ix
	ret
;opcs_core.c:449: void SendResponseAfterExecutingCode(byte length)
;	---------------------------------
; Function SendResponseAfterExecutingCode
; ---------------------------------
_SendResponseAfterExecutingCode::
	call	___sdcc_enter_ix
;opcs_core.c:451: short* regsPointer = (short*)&(pendingCommand.buffer[3]);
	ld	bc,#_pendingCommand + 13
;opcs_core.c:452: pendingCommand.buffer[2] = 0; //First byte of "ok" response
	ld	hl,#(_pendingCommand + 0x000c)
	ld	(hl),#0x00
;opcs_core.c:454: regsPointer[0] = regs.Words.AF;
	ld	de, (#_regs + 0)
	ld	l, c
	ld	h, b
	ld	(hl),e
	inc	hl
	ld	(hl),d
;opcs_core.c:456: if(length > 2) {
	ld	a,#0x02
	sub	a, 4 (ix)
	jr	NC,00104$
;opcs_core.c:457: regsPointer[1] = regs.Words.BC;
	push	bc
	pop	iy
	inc	iy
	inc	iy
	ld	hl, (#_regs + 2)
	ld	0 (iy),l
	ld	1 (iy),h
;opcs_core.c:458: regsPointer[2] = regs.Words.DE;
	ld	iy,#0x0004
	add	iy, bc
	ld	hl, (#_regs + 4)
	ld	0 (iy),l
	ld	1 (iy),h
;opcs_core.c:459: regsPointer[3] = regs.Words.HL;
	ld	iy,#0x0006
	add	iy, bc
	ld	hl, (#_regs + 6)
	ld	0 (iy),l
	ld	1 (iy),h
;opcs_core.c:463: if(length > 8) {
	ld	a,#0x08
	sub	a, 4 (ix)
	jr	NC,00104$
;opcs_core.c:464: regsPointer[4] = regs.Words.IX;
	ld	iy,#0x0008
	add	iy, bc
	ld	hl, (#_regs + 8)
	ld	0 (iy),l
	ld	1 (iy),h
;opcs_core.c:465: regsPointer[5] = regs.Words.IY;
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
;opcs_core.c:468: if(length > 12) {
00104$:
;opcs_core.c:474: SendBytes((byte*)&(pendingCommand.buffer[2]), length+1, true);
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
;opcs_core.c:477: void SendErrorMessage(char* message)
;	---------------------------------
; Function SendErrorMessage
; ---------------------------------
_SendErrorMessage::
;opcs_core.c:479: byte length = strlen(message);
	pop	bc
	pop	hl
	push	hl
	push	bc
	push	hl
	call	_strlen
	pop	af
	ld	b,l
;opcs_core.c:480: SendByte(length, false);
	push	bc
	xor	a, a
	push	af
	inc	sp
	push	bc
	inc	sp
	call	_SendByte
	pop	af
	pop	bc
;opcs_core.c:481: SendBytes((byte*)message, length, true);
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
;opcs_core.c:484: void SendByte(byte datum, bool push)
;	---------------------------------
; Function SendByte
; ---------------------------------
_SendByte::
;opcs_core.c:486: SendBytes(&datum, 1, push);
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
;opcs_core.c:489: bool HandleConnectionLifetime()
;	---------------------------------
; Function HandleConnectionLifetime
; ---------------------------------
_HandleConnectionLifetime::
;opcs_core.c:491: bool wasPreviouslyConnected = clientIsConnected;
	ld	hl,#_clientIsConnected + 0
	ld	c, (hl)
;opcs_core.c:492: clientIsConnected = ClientIsConnected();
	push	bc
	call	_ClientIsConnected
	pop	bc
	ld	iy,#_clientIsConnected
	ld	0 (iy),l
;opcs_core.c:494: if(clientIsConnected && !wasPreviouslyConnected) {
	ld	a,0 (iy)
	or	a, a
	jr	Z,00102$
	ld	a,c
	or	a, a
	jr	NZ,00102$
;opcs_core.c:495: print("Client connected!\r\n");
	push	bc
	ld	hl,#___str_21
	push	hl
	call	_printf
	pop	af
	pop	bc
;opcs_core.c:496: getDataBuffer.dataPointer = &(getDataBuffer.data[0]);
	ld	hl,#_getDataBuffer
	ld	((_getDataBuffer + 0x0200)), hl
;opcs_core.c:497: getDataBuffer.remainingData = 0;
	ld	hl,#0x0000
	ld	((_getDataBuffer + 0x0202)), hl
00102$:
;opcs_core.c:500: if(!clientIsConnected) {
	ld	a,(#_clientIsConnected + 0)
;opcs_core.c:501: if(wasPreviouslyConnected) {
	or	a,a
	jr	NZ,00107$
	or	a,c
	jp	Z,_WaitForClientConnection
;opcs_core.c:502: print("Disconnected...\r\n");
	ld	hl,#___str_22
	push	hl
	call	_printf
	pop	af
;opcs_core.c:505: return WaitForClientConnection();
	jp  _WaitForClientConnection
00107$:
;opcs_core.c:508: return true;
	ld	l,#0x01
	ret
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
;opcs_core.c:511: void ReadFromPort(byte portNumber, byte* destinationAddress, uint size, bool autoIncrement) __naked
;	---------------------------------
; Function ReadFromPort
; ---------------------------------
_ReadFromPort::
;opcs_core.c:543: __endasm;
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
;opcs_core.c:546: void WriteToPort(byte portNumber, byte value)
;	---------------------------------
; Function WriteToPort
; ---------------------------------
_WriteToPort::
;opcs_core.c:559: __endasm;
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
