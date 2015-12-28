		OPT	!
		NEAR	A4,-2
		MACHINE	68000

		INCDIR	"asminc:"
		INCLUDE	"devices/timer.i"
		INCLUDE	"exec/funcdef.i"
		INCLUDE	"exec/memory.i"
		INCLUDE	"exec/execbase.i"
		INCLUDE	"exec/exec_lib.i"
		INCLUDE "resources/misc.i"
		INCLUDE	"mine/offseta4.i"

		INCLUDE	"work:dterm/global.i"
		INCLUDE	"work:dterm/routinesa.i"
		INCLUDE	"work:dterm/ser/ser_head.i"

		XREF	_LinkerDB		; Interruptia varten
		XREF	_alloc_pool
		XREF	_free_pool

		XREF	_LVOGetSysTime
		XREF	_LVOSubTime
		XREF	_LVOAddTime

		NREF	_SysBase
		NREF	_GfxBase
		NREF	_TimerBase
		NREF	_SerialInfo
		NREF	_RXD
		NREF	_gl_process
		NREF	_gl_serok
		NREF	_gl_rxdsig
		NREF	_gl_txdsig

;		XDEF	_hardserver		; Id teksti (mainille)
		XDEF	_InitHSerial
		XDEF	_RemoveHSerial
		XDEF	_ResetHInfo
		XDEF	_InsertHTXD
		XDEF	_CheckTXD_h
		XDEF	_WaitHTXD
		XDEF	_BreakHTXD
		XDEF	_GetHRXD
		XDEF	_SetHPort
		XDEF	_GetHTimedRXD
		XDEF	_CheckHCTS
		XDEF	_CheckHRTS
		XDEF	_CheckHCS
		XDEF	_SetHRTS
		XDEF	_DropHDTR
		XDEF	_HQuery

		SECTION	code,code

_InitHSerial:	; Hardware serial p‰‰lle
		; OUPUT	D0.L: BOOL
		movem.l	d2-d7/a2-a6,-(sp)
		CUR_VAL	PB_SerBufSize,d0
		move.l	d0,RXDBufSize
		bsr	_alloc_pool
		move.l	d0,RXDBuf
		beq	nores
		CALL	Exec,CreateMsgPort	; Timer
		move.l	d0,timport
		beq	nores		
		move.l	d0,a0
		move.l	#IOTV_SIZE,d0
		LIBC	CreateIORequest
		move.l	d0,timio
		beq	nores
		lea	TimName,a0
		move.l	d0,a1
		move.l	#UNIT_VBLANK,d0
		moveq	#0,d1
		LIBC	OpenDevice
		tst.b	d0
		bne	nores
		st	timok

		moveq	#-1,d0			; Signals
		LIBC	AllocSignal
		move.b	d0,txdsignal
		bmi	nores
		moveq	#-1,d0
		LIBC	AllocSignal
		move.b	d0,rxdsignal
		bmi	nores
		moveq	#0,d0
		moveq	#0,d1
		move.b	txdsignal,d0
		bset	d0,d1
		move.l	d1,txdsignalbit
		moveq	#0,d1
		move.b	rxdsignal,d0
		bset	d0,d1
		move.l	d1,rxdsignalbit
		clr.b	rxdsignaldone
		clr.b	rxdsignalmake
		clr.b	rxdsignalwait
		clr.b	txdcanwait

		LIBC	Forbid			; Kick serial.device's ass
		lea	DeviceList(a6),a0
		lea	SerName(pc),a1
		LIBC	FindName
		tst.l	d0
		beq	.noserial
		move.l	d0,a1
		LIBC	RemDevice
.noserial:
		lea	DeviceList(a6),a0	; BaudBandit to heaven
		lea	SerName2(pc),a1
		LIBC	FindName
		tst.l	d0
		beq	.nobandit
		move.l	d0,a1
		LIBC	RemDevice
.nobandit:
		lea	DeviceList(a6),a0	; artser to hell
		lea	SerName3(pc),a1
		LIBC	FindName
		tst.l	d0
		beq	.noartser
		move.l	d0,a1
		LIBC	RemDevice
.noartser:	LIBC	Permit

		clr.w	MiscId
		moveq	#0,d0
		lea	MiscText(pc),a1
		LIBC	OpenResource
		move.l	d0,MiscBase
		beq	nores
		move.l	d0,a6
		lea	MiscUser(pc),a1
		moveq	#MR_SERIALPORT,d0
		jsr	MR_ALLOCMISCRESOURCE(a6)
		tst.l	d0
		bne	nores
		move.w	#1,MiscId
		lea	MiscUser(pc),a1
		moveq	#MR_SERIALBITS,d0
		jsr	MR_ALLOCMISCRESOURCE(a6)
		tst.l	d0
		bne	nores
		move.w	#2,MiscId
		or.b	#$c0,$bfd200		; DTR/RTS -> output
		and.b	#$c7,$bfd200		; CD/CTS/DSR -> input

		move.l	_SysBase,a6
		clr.b	TXDCTS			; CTS off stateen
		move.l	RXDBuf,d0
		move.l	d0,RXDRead		; RXDbuffer read oikein
		move.l	d0,RXDWrite		; RXDbuffer write oikein
		add.l	RXDBufSize,d0
		move.l	d0,RXDBufEnd
	
		LIBC	Disable
		lea	BlankInt,a1
		moveq	#5,d0
		LIBC	AddIntServer
		lea	RXDInt,a1
		moveq	#11,d0
		LIBC	SetIntVector
		move.l	d0,OldRXDVector
		moveq	#0,d0
		lea	TXDInt,a1
		LIBC	SetIntVector
		move.l	d0,OldTXDVector

		lea	time_txd_base,a0
		move.l	_TimerBase,a6
		LIBC	GetSysTime
		lea	time_rxd_base,a0
		move.l	_TimerBase,a6
		LIBC	GetSysTime
		move.l	_SerialInfo+txdsec,time_txd_total
		move.l	#0,time_txd_total+4
		move.l	_SerialInfo+rxdsec,time_rxd_total
		move.l	#0,time_rxd_total+4
		clr.w	time_txd
		clr.w	time_rxd

; --> Poista mahdolliset odottavat keskeytykset + lis‰‰ txd signal
		move.w	#$0801,$dff09c		; Clear InterReg(TBE/TBF)
		CALL	Exec,Enable
		move.w	#$c801,$dff09a
		bclr	#6,$bfd000		; RTS on (logical)
		bclr	#7,$bfd000		; DTR on

		move.l	txdsignalbit,_gl_txdsig
		move.l	rxdsignalbit,_gl_rxdsig
		move.l	#1,_gl_serok

		moveq	#TRUE,d0
		movem.l	(sp)+,d2-d7/a2-a6
		rts
nores:		bsr.b	_RemoveHSerial
		moveq	#FALSE,d0
		movem.l	(sp)+,d2-d7/a2-a6
		rts

_RemoveHSerial:	; Hardware serial pois
		movem.l	d2-d7/a2-a6,-(sp)
		move.l	MiscBase,d0
		beq	nomisc
		move.l	d0,a6
		move.w	MiscId,d1
		beq	nomisc
		cmp.w	#1,d1
		beq	freef
		bset	#6,$bfd000		; RTS off (logical)
		bset	#7,$bfd000		; DTR off
		moveq	#MR_SERIALBITS,d0
		jsr	MR_FREEMISCRESOURCE(a6)
freef:		moveq	#MR_SERIALPORT,d0
		jsr	MR_FREEMISCRESOURCE(a6)
nomisc:		cmp.w	#2,MiscId
		bne	.noint
		CALL	Exec,Disable
		move.w	#$0801,$dff09a
		lea	BlankInt,a1
		moveq	#5,d0
		LIBC	RemIntServer
		move.l	OldRXDVector,a1
		moveq	#11,d0
		LIBC	SetIntVector
		move.L	OldTXDVector,a1
		moveq	#0,d0
		LIBC	SetIntVector
		LIBC	Enable
.noint:		clr.w	MiscId
		clr.l	MiscBase
		move.b	txdsignal,d0
		CALL	Exec,FreeSignal
		move.b	rxdsignal,d0
		LIBC	FreeSignal
		move.b	#-1,rxdsignal
		move.b	#-1,txdsignal
		tst.b	timok
		beq	.nodev
		clr.b	timok
		move.l	timio,a1
		LIBC	CloseDevice
.nodev:		move.l	timio,a0
		LIBC	DeleteIORequest
		move.l	timport,a0
		LIBC	DeleteMsgPort
		clr.l	timio
		clr.l	timport
		move.l	RXDBuf,a0
		bsr	_free_pool
		clr.l	RXDBuf
		move.l	#0,_gl_rxdsig
		move.l	#0,_gl_txdsig
		move.l	#0,_gl_serok
		movem.l	(sp)+,d2-d7/a2-a6
		rts

_ResetHInfo:	; Resetoi TXD/RXD ratet yms.
		clr.w	_SerialInfo+txdcps
		clr.w	_SerialInfo+txdcps_peak
		clr.w	_SerialInfo+txdcps_avr_peak
		clr.w	_SerialInfo+txdcps_avr
		clr.l	_SerialInfo+txdbytes
		clr.l	_SerialInfo+txdsec
		clr.w	_SerialInfo+rxdcps
		clr.w	_SerialInfo+rxdcps_peak
		clr.w	_SerialInfo+rxdcps_avr_peak
		clr.w	_SerialInfo+rxdcps_avr
		clr.l	_SerialInfo+rxdbytes
		clr.l	_SerialInfo+rxdsec
		clr.w	_SerialInfo+overrun
		clr.l	_SerialInfo+charbuf
		rts

_InsertHTXD:	; Insertoi datan l‰hetys (doublebuffered)
		; INPUT	A0.L: Miss‰ data
		;	D0.L: Paljon
		;	D0.L: TRUE/FALSE
		tst.l	d0
		beq	.serok
		movem.l	d0/a0,-(sp)
		bsr	_WaitHTXD
		move.l	d0,d1
		movem.l	(sp)+,d0/a0
		tst.l	d1
		beq	.sererr
		move.l	a0,TXDPtr
		move.l	d0,TXDSize
		move.l	_SysBase,a0
		moveq	#0,d0
		move.b	VBlankFrequency(a0),d0	; ~60 seconds
		mulu.w	#60,d0
		move.w	d0,txdover
		move.w	#$8001,$dff09c
.serok:		moveq	#TRUE,d0
		rts
.sererr:	moveq	#FALSE,d0
		rts

_CheckTXD_h:	; Kysy on l‰hetys viel‰ k‰ynniss‰
		; OUPUT	D0.L: BOOL
		tst.l	TXDSize
		bmi	.txdgone
		moveq	#FALSE,d0
		rts
.txdgone:	moveq	#TRUE,d0
		rts

_WaitHTXD:	; Odota TXD:t‰
		; OUPUT	D0.L: TRUE/FALSE
		move.l	a6,-(sp)
		move.l	#0,d0
		move.l	txdsignalbit,d1
		CALL	Exec,SetSignal
		tst.l	TXDSize
		bmi	.nosignal
		move.l	txdsignalbit,d0
		LIBC	Wait
.nosignal:	move.l	(sp)+,a6
		moveq	#TRUE,d0
		rts

_BreakHTXD:	; Lopeta l‰hetys
		clr.l	TXDSize
		rts

_GetHRXD:	; Hae tulleet merkit
		; OUPUT	D0.L: RXD structure
		sf	rxdsignaldone
		lea	_RXD,a0
		move.l	RXDWrite,d0
		move.l	RXDRead,d1
		cmp.l	d1,d0
		bhs	.simple
		move.l	RXDBufEnd,d0		; Vain bufferin loppuun
.simple:	sub.l	d1,d0
		move.l	d1,buffer(a0)
		move.l	d0,size(a0)
		add.l	RXDRead,d0		; P‰ivit‰ read
		cmp.l	RXDBufEnd,d0
		blo	.justgo
		move.l	RXDBuf,d0
.justgo:	move.l	d0,RXDRead
		move.l	a0,d0
		rts

_GetHTimedRXD:	; Hae tulleet merkit (vain xpr k‰ytt‰‰)
		; INPUT	D0.L: Monta merkki‰
		;	D1.L: Timeout (microseconds)
		; OUPUT	D0.L: RXD structure (expanded)
		movem.l	d2-d3/a6,-(sp)
		st	rxdsignaldone
		move.l	d0,rxdwaitsize
		move.l	d1,d2

		move.l	RXDWrite,d0		; check if enough already
		move.l	RXDRead,d1
		cmp.l	d1,d0
		bhs	.bufsimple
		sub.l	RXDBuf,d0
		add.l	RXDBufEnd,d0
.bufsimple:	sub.l	d1,d0
		cmp.l	rxdwaitsize,d0
		bhs	.nowait

		moveq	#0,d0
		move.l	rxdsignalbit,d1
		CALL	Exec,SetSignal
		move.l	timio,a1		; set timer
		move.l	d2,d3
		divu.w	#62500,d2
		and.l	#$ffff,d2
		divu.w	#16,d2
		and.l	#$ffff,d2
		move.l	d2,IOTV_TIME+TV_SECS(a1)
		mulu.w	#16,d2
		mulu.w	#62500,d2
		sub.l	d2,d3
		move.l	d3,IOTV_TIME+TV_MICRO(a1)
		move.w	#TR_ADDREQUEST,IO_COMMAND(a1)
		LIBC	SendIO
		st	rxdsignalwait
		moveq	#0,d0
		moveq	#0,d1
		move.l	timport,a1
		move.b	MP_SIGBIT(a1),d1
		bset	d1,d0
		or.l	rxdsignalbit,d0
		LIBC	Wait
		sf	rxdsignalwait
		move.l	timio,a1
		LIBC	AbortIO
		move.l	timio,a1
		LIBC	WaitIO
		moveq	#0,d0
		moveq	#0,d1
		moveq	#0,d2
		move.l	timport,a1
		move.b	MP_SIGBIT(a1),d2
		bset	d2,d1
		LIBC	SetSignal

.nowait:	move.l	RXDWrite,d0		; check if bytes left (signal)
		move.l	RXDRead,d1
		moveq	#0,d2
		cmp.l	d1,d0
		bhs	.simple
		sub.l	RXDBuf,d0
		move.l	d0,d2
		add.l	RXDBufEnd,d0
.simple:	sub.l	d1,d0
		move.l	d0,_RXD+size
		sub.l	d2,_RXD+size
		move.l	d2,_RXD+size2
		cmp.l	rxdwaitsize,d0
		bls	.noneedsig
		st	rxdsignalmake
.noneedsig:	sf	rxdsignaldone

		move.l	RXDRead,_RXD+buffer	; output bytes
		move.l	RXDBuf,_RXD+buffer2
		move.l	rxdwaitsize,d0
		move.l	d0,d1
		sub.l	_RXD+size,d1
		bmi	.fix2
		beq	.fix2
		move.l	d1,d0
		sub.l	_RXD+size2,d1
		bmi	.fix1
		beq	.fix1
		bra	.fixr
.fix1:		move.l	d0,_RXD+size2
		bra	.fixr
.fix2:		move.l	d0,_RXD+size
		clr.l	_RXD+buffer2
		clr.l	_RXD+size2
.fixr:		move.l	RXDRead,d0
		add.l	_RXD+size,d0
		cmp.l	RXDBufEnd,d0
		blo	.nover
		move.l	RXDBuf,d0
.nover:		add.l	_RXD+size2,d0
		move.l	d0,RXDRead
		lea	_RXD,a0
		move.l	a0,d0
		movem.l	(sp)+,d2-d3/a6
		rts

_SetHPort:	; DTE, flow control, databits and parity yms.
		CUR_VAL	PB_DteRate,d0
		move.l	d0,CurDTE
		move.l	d0,d1
		lsl.l	#3,d0
		sub.l	d1,d0
		move.l	_SysBase,a0
		move.l	#$017d7840,d1
		cmp.b	#50,531(a0)
		bne	.NTSC
		move.l	#$0179ff40,D1
.NTSC:		cmp.l	#$ffff,d0
		ble	.dodivu
		lsr.l	#5,d0
		divu	d0,d1
		and.l	#$ffff,d1
		lsr.l	#5,d1
		bra	.makedte
.dodivu:	divu	d0,d1
.makedte:	move.w	d1,$dff032
		rts

_CheckHCTS:	; Logical CTS state (p‰‰ll‰ = TRUE)
		moveq	#TRUE,d0
		btst	#4,$bfd000
		beq	.cton
		moveq	#FALSE,d0
.cton:		rts

_CheckHRTS:	; Logical RTS state (p‰‰ll‰ = TRUE)
		moveq	#TRUE,d0
		btst	#6,$bfd000
		beq	.rton
		moveq	#FALSE,d0
.rton:		rts

_CheckHCS:	; Logical carrier state (p‰‰ll‰ = TRUE)
		moveq	#TRUE,d0
		btst	#5,$bfd000
		beq	.cson
		moveq	#FALSE,d0
.cson:		rts

_SetHRTS:	; Set RTS
		; INPUT	D0.W: BOOL
		tst.w	d0
		bne	.rtson
		bset	#6,$bfd000		; RTS off (logical)
		rts
.rtson:		bclr	#6,$bfd000		; RTS on (logical)
		rts

_DropHDTR:	; Tiputa DTR
		bset	#7,$bfd000		; DTR off
		move.l	timio,a1
		move.l	#0,IOTV_TIME+TV_SECS(a1)
		move.l	#20000,IOTV_TIME+TV_MICRO(a1)
		move.w	#TR_ADDREQUEST,TIMEREQUEST+IO_COMMAND(a1)
		LIBC	DoIO
		bclr	#7,$bfd000		; DTR on
		rts

_HQuery:	; Monta byte‰ bufferissa
		; OUPUT	D0.L: bytes
		move.l	RXDWrite,d0		; timed read
		move.l	RXDRead,d1
		cmp.l	d1,d0
		bhs	.simple
		sub.l	RXDBuf,d0
		add.l	RXDBufEnd,d0
.simple:	sub.l	d1,d0
		rts

; --> Keskeytykset

TXDVector:
		move.l	a4,a5
		lea	_LinkerDB,a4
		move.w	#$0001,$9c(a0)		; Clear InterReq(TBE)
		btst	#4,$bfd000
		beq	.ctsok
		st	TXDCTS			; Odota modeemia
		bra	.txdend
.ctsok:		subq.l	#1,TXDSize
		bpl	.sendit
		move.l	_SysBase,a6
		move.l	_gl_process,a1
		move.l	txdsignalbit,d0
		LIBC	Signal
		bra	.txdend
.sendit:	move.l	TXDPtr,a1
		move.w	#$100,d0		; 8N1
		move.b	(a1)+,d0		; Data joka l‰htee
		move.l	a1,TXDPtr		; Kohta muistiin
		move.w	d0,$30(a0)		; Data rekisteriin
		addq.w	#1,time_txd
.txdend:	move.l	a5,a4
		rts

RXDVector:
		move.l	a4,a5
		lea	_LinkerDB,a4
		move.w	$18(a0),d0
		bpl	.notoverrun
		addq.w	#1,_SerialInfo+overrun
.notoverrun:	move.w	#$0800,$9c(a0)		; Clear InterReq(TBF)
		move.l	RXDWrite,a1
		move.b	d0,(a1)+		; Merkki bufferiin
		cmp.l	RXDBufEnd,a1
		blo	.nores
		move.l	RXDBuf,a1
.nores:		move.l	a1,RXDWrite
		addq.w	#1,time_rxd
		st	rxdsignalmake		; VBlankille tieto
		move.l	a5,a4
		rts

Blank:		; Handlaa CTS / CPS
		move.l	a4,a5

		lea	_LinkerDB,a4
		move.l	_SysBase,a6

		tst.l	TXDSize			; Prevent txd freeze
		bmi	.nocheck
		subq.w	#1,txdover
		bne	.nocheck
		sf	TXDCTS
		move.l	#-1,TXDSize
		move.l	_gl_process,a1
		move.l	txdsignalbit,d0
		LIBC	Signal
.nocheck:

		tst.b	rxdsignalwait
		beq	.informget
		move.l	RXDWrite,d0		; timed read
		move.l	RXDRead,d1
		cmp.l	d1,d0
		bhs	.simple
		sub.l	RXDBuf,d0
		add.l	RXDBufEnd,d0
.simple:	sub.l	d1,d0
		cmp.l	rxdwaitsize,d0
		bhs	.sigwait

.informget:	tst.b	rxdsignalmake		; rxd signal needed?
		beq	.nosignal
		clr.b	rxdsignalmake
		tst.b	rxdsignaldone
		bne	.nosignal
.sigwait:	move.l	_gl_process,a1
		move.l	rxdsignalbit,d0
		LIBC	Signal
		st	rxdsignaldone
.nosignal:

		tst.b	TXDCTS			; CTS handling
		beq	.calccps
		btst	#4,$bfd000
		bne	.calccps
		sf	TXDCTS
		move.w	#$8001,$dff09c
.calccps:
		move.l	_TimerBase,a6

		tst.w	time_txd
		beq	.txdnil
		lea	time_txd_now,a0
		LIBC	GetSysTime
		lea	time_txd_now,a0
		lea	time_txd_base,a1
		LIBC	SubTime
		tst.l	time_txd_now		; ~1 second
		beq	.txdend
		moveq	#0,d0
		move.w	time_txd,d0
		clr.w	time_txd
		add.l	d0,_SerialInfo+txdbytes
		move.w	d0,_SerialInfo+txdcps
		lea	time_txd_total,a0
		lea	time_txd_now,a1
		LIBC	AddTime
		move.l	time_txd_total,_SerialInfo+txdsec
		bra	.txdbase
.txdnil:	tst.w	_SerialInfo+txdcps
		beq	.txdbase
		lea	time_txd_now,a0
		LIBC	GetSysTime
		lea	time_txd_now,a0
		lea	time_txd_base,a1
		LIBC	SubTime
		tst.l	time_txd_now		; ~1 second
		beq	.txdend
		clr.w	_SerialInfo+txdcps
.txdbase:	lea	time_txd_base,a0
		LIBC	GetSysTime
.txdend:
		tst.w	time_rxd
		beq	.rxdnil
		lea	time_rxd_now,a0
		LIBC	GetSysTime
		lea	time_rxd_now,a0
		lea	time_rxd_base,a1
		LIBC	SubTime
		tst.l	time_rxd_now		; ~1 second
		beq	.rxdend
		moveq	#0,d0
		move.w	time_rxd,d0
		clr.w	time_rxd
		add.l	d0,_SerialInfo+rxdbytes
		move.w	d0,_SerialInfo+rxdcps
		lea	time_rxd_total,a0
		lea	time_rxd_now,a1
		LIBC	AddTime
		move.l	time_rxd_total,_SerialInfo+rxdsec
		bra	.rxdbase
.rxdnil:	tst.w	_SerialInfo+rxdcps
		beq	.rxdbase
		lea	time_rxd_now,a0
		LIBC	GetSysTime
		lea	time_rxd_now,a0
		lea	time_rxd_base,a1
		LIBC	SubTime
		tst.l	time_rxd_now		; ~1 second
		beq	.rxdend
		clr.w	_SerialInfo+rxdcps
.rxdbase:	lea	time_rxd_base,a0
		LIBC	GetSysTime
.rxdend:
		move.l	a5,a4
		moveq	#0,d0		; Set Z
		rts

TXDName:	dc.b	'DTerm.TX-SER',0
RXDName:	dc.b	'DTerm.RX-SER',0
BlankName:	dc.b	'DTerm.VB-SER',0
MiscUser:	dc.b	'DTerm.misc',0
MiscText:	dc.b	'misc.resource',0
SerName:	dc.b	'serial.device',0
SerName2:	dc.b	'BaudBandit.device',0
SerName3:	dc.b	'artser.device',0
TimName:	dc.b	'timer.device',0

		SECTION   __MERGED,data

TXDPtr:		dc.l	0		; Mist‰ l‰hetet‰‰n
TXDSize:	dc.l	-1		; Paljon l‰hetet‰‰n
OldTXDVector:	dc.l	0
txdsignalbit:	dc.l	0		; signal bit
txdover:	dc.w	0		; timer
TXDCTS:		dc.b	0		; 1 = CTS meni p‰‰lle
txdsignal:	dc.b	-1		; signal number
txdcanwait:	dc.b	0
		EVEN
RXDBuf:		dc.l	0		; Miss‰ RXD bufferi
RXDBufEnd:	dc.l	0
RXDRead:	dc.l	0
RXDWrite:	dc.l	0
RXDBufSize:	dc.l	0
CurDTE:		dc.l	0
OldRXDVector:	dc.l	0
rxdwaitsize:	dc.l	0
rxdsignalbit:	dc.l	0
rxdsignal:	dc.b	-1
rxdsignaldone:	dc.b	0		; signal l‰hetetty
rxdsignalmake:	dc.b	0		; signal tarvitaan
rxdsignalwait:	dc.b	0		; signal when read done (special)
		EVEN
timport:	dc.l	0
timio:		dc.l	0
timok:		dc.b	0
		EVEN
MiscBase:	dc.l	0
MiscId:		dc.w	0
time_txd:	dc.w	0
time_txd_now:	dc.l	0,0
time_txd_base:	dc.l	0,0
time_txd_total:	dc.l	0,0
time_rxd:	dc.w	0
time_rxd_now:	dc.l	0,0
time_rxd_base:	dc.l	0,0
time_rxd_total:	dc.l	0,0
		EVEN
TXDInt:		dc.l	0
		dc.l	0
		dc.b	NT_INTERRUPT
		dc.b	0
		dc.l	TXDName
		dc.l	TXDName
		dc.l	TXDVector
RXDInt:		dc.l	0
		dc.l	0
		dc.b	NT_INTERRUPT
		dc.b	0
		dc.l	RXDName
		dc.l	RXDName
		dc.l	RXDVector
BlankInt:	dc.l	0
		dc.l	0
		dc.b	NT_INTERRUPT
		dc.b	0
		dc.l	BlankName
		dc.l	BlankName
		dc.l	Blank

;_hardserver:	dc.b	'1.42 (Jan 14 1996)',0
		EVEN

		END
