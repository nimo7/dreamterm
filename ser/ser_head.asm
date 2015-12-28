*************************************************
*						*
*		 Serial FrontEd			*
*						*
*	  Copyright © By Pasi Ristioja		*
*						*
*************************************************

		OPT	!
		NEAR	A4,-2
		MACHINE	68000

		INCDIR	"asminc:"
		INCLUDE	"work:dterm/ser/ser_head.i"

* Hardware serial

		XREF	_InitHSerial
		XREF	_RemoveHSerial
		XREF	_ResetHInfo
		XREF	_InsertHTXD
		XREF	_CheckTXD_h
		XREF	_WaitHTXD
		XREF	_BreakHTXD
		XREF	_GetHRXD
		XREF	_SetHPort
		XREF	_GetHTimedRXD
		XREF	_CheckHCTS
		XREF	_CheckHRTS
		XREF	_CheckHCS
		XREF	_SetHRTS
		XREF	_DropHDTR
		XREF	_HQuery

* Software serial

		XREF	_InitSSerial
		XREF	_RemoveSSerial
		XREF	_ResetSInfo
		XREF	_InsertSTXD
		XREF	_CheckTXD_s
		XREF	_WaitSTXD
		XREF	_BreakSTXD
		XREF	_GetSRXD
		XREF	_SetSPort
		XREF	_GetSTimedRXD
		XREF	_CheckSCTS
		XREF	_CheckSRTS
		XREF	_CheckSCS
		XREF	_SetSRTS
		XREF	_DropSDTR
		XREF	_SQuery

* Ulosp‰in n‰kyv‰t

		XDEF	_SerialInfo
		XDEF	_RXD

		XDEF	_SerialToUse
		XDEF	_InitSerial
		XDEF	_RemoveSerial
		XDEF	_ResetInfo
		XDEF	_InsertTXD
		XDEF	_CheckTXD
		XDEF	_WaitTXD
		XDEF	_BreakTXD
		XDEF	_GetRXD
		XDEF	_SetPort
		XDEF	_GetTimedRXD
		XDEF	_CheckCTS
		XDEF	_CheckRTS
		XDEF	_CheckCS
		XDEF	_SetRTS
		XDEF	_DropDTR
		XDEF	_SerQuery

		SECTION	code,code

_SerialToUse:	; Kumpaa serialia k‰ytet‰‰n
		; INPUT	D0.B:	1 = SERIAL_HARDWARE (TRUE)
		;		0 = SERIAL_SOFTWARE (FALSE)
		move.b	d0,Using
		rts

_InitSerial:	; Initoi serial module
		; OUPUT	D0.L: BOOL
		tst.b	Using
		beq	_InitSSerial
		bne	_InitHSerial

_RemoveSerial:	; Poista serial module
		tst.b	Using
		beq	_RemoveSSerial
		bne	_RemoveHSerial

_ResetInfo:	; Resetoi info
		tst.b	Using
		beq	_ResetSInfo
		bne	_ResetHInfo

_InsertTXD:	; L‰het‰ merkkej‰
		; INPUT	A0.L: Mist‰
		;	D0.L: Paljon
		tst.b	Using
		beq	_InsertSTXD
		bne	_InsertHTXD

_CheckTXD:	; Kysy onko l‰hetys valmis
		; OUPUT	D0.L: BOOL
		tst.b	Using
		beq	_CheckTXD_s
		bne	_CheckTXD_h

_WaitTXD:	; Odota l‰hetyksen loppumista
		; OUPUT	D0.L: TRUE/FALSE
		tst.b	Using
		beq	_WaitSTXD
		bne	_WaitHTXD

_BreakTXD:	; Lopeta sendi
		tst.b	Using
		beq	_BreakSTXD
		bne	_BreakHTXD

_GetRXD:	; Hae tulleet merkit
		; OUPUT	D0.L: RXD structure
		tst.b	Using
		beq	_GetSRXD
		bne	_GetHRXD

_SetPort:	; Aseta serial portin parametrit
		; OUPUT D0.L: TRUE/FALSE
		tst.b	Using
		beq	_SetSPort
		bne	_SetHPort

_GetTimedRXD:	; Hae merkkek‰ timeoutin kanssa
		; INPUT	D0.L: Monta merkki‰ maksimissaan
		;	D1.L: Timeout (microseconds)
		; OUPUT	D0.L: RXD structure
		tst.b	Using
		beq	_GetSTimedRXD
		bne	_GetHTimedRXD

_CheckCTS:	; CTS signal p‰‰ll‰?
		; OUPUT	D0.L: BOOL
		tst.b	Using
		beq	_CheckSCTS
		bne	_CheckHCTS

_CheckRTS:	; RTS signal p‰‰ll‰?
		; OUPUT	D0.L: BOOL
		tst.b	Using
		beq	_CheckSRTS
		bne	_CheckHRTS

_CheckCS:	; Carrier signal p‰‰ll‰?
		; OUPUT	D0.L: BOOL
		tst.b	Using
		beq	_CheckSCS
		bne	_CheckHCS

_SetRTS:	; Settaa RTS
		; INPUT	D0.L: RTSon = TRUE, RTSoff = FALSE
		tst.b	Using
		beq	_SetSRTS
		bne	_SetHRTS

_DropDTR:	; Tipauta DTR
		tst.b	Using
		beq	_DropSDTR
		bne	_DropHDTR

_SerQuery:	; Monta merkki‰ bufferissa
		; OUPUT	D0.L: size
		tst.b	Using
		beq	_SQuery
		bne	_HQuery

		SECTION	__MERGED,bss

_SerialInfo:	ds.b	SERIALINFO_SIZE
_RXD:		ds.b	RXD_SIZE
Using:		ds.b	1
		EVEN

		END
