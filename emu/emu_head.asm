	*************************************************
	*						*
	*	        Emulation FrontEd		*
	*						*
	*	  Copyright © By Pasi Ristioja		*
	*						*
	*************************************************

		TTL	emuhead
		OPT	!
		NEAR	A4,-2
		MACHINE	68000

		INCDIR	"asminc:"

		XREF	_InitANSI	; Ansi
		XREF	_RemoveANSI
		XREF	_ResetANSI
		XREF	_ChangeANSI
		XREF	_PrintANSI
		XREF	_StripANSI
		XREF	_InitHEX	; Hex
		XREF	_RemoveHEX
		XREF	_ResetHEX
		XREF	_ChangeHEX
		XREF	_PrintHEX
		XREF	_StripHEX
		XREF	_InitXEM	; Xem
		XREF	_RemoveXEM
		XREF	_ResetXEM
		XREF	_ChangeXEM
		XREF	_PrintXEM
		XREF	_StripXEM
	
		* P‰‰koodille n‰kyvat rutiinit

		XDEF	_EmToUse
		XDEF	_EmInit
		XDEF	_EmRemove
		XDEF	_EmReset
		XDEF	_EmChange
		XDEF	_EmPrint
		XDEF	_EmStrip

		SECTION	FrontEdCode,code

_EmToUse:	; Mit‰ emulaatiota k‰ytet‰‰n
		; INPUT	D0.B:	0 / ANSI EMULATION
		;		1 / HEX
		;		2 / XEM
		move.b	d0,Using
		rts

_EmInit:	; Initoi emulaatio (v‰rit etc.)
		; INPUT	A0.L: Tags
		tst.b	Using
		beq	_InitANSI
		cmp.b	#1,Using
		beq	_InitHEX
		bne	_InitXEM
	
_EmRemove:	; Poista emulaatio
		; INPUT	A0.L: Tags
		tst.b	Using
		beq	_RemoveANSI
		cmp.b	#1,Using
		beq	_RemoveHEX
		bne	_RemoveXEM

_EmReset:	; Resetoi emulaatio
		; INPUT	A0.L: Tags
		tst.b	Using
		beq	_ResetANSI
		cmp.b	#1,Using
		beq	_ResetHEX
		bne	_ResetXEM

_EmChange:	; Vaihda emulaation arvoja
		; INPUT	A0.L: Tags
		tst.b	Using
		beq	_ChangeANSI
		cmp.b	#1,Using
		beq	_ChangeHEX
		bne	_ChangeXEM

_EmPrint:	; Printtaa
		; INPUT	A0.L: Miss‰
		;	D0.L: Paljon
		tst.b	Using
		beq	_PrintANSI
		cmp.b	#1,Using
		beq	_PrintHEX
		bne	_PrintXEM

_EmStrip:	; Strip chars
		; INPUT	A0.L: Mist‰
		;	A1.L: Minne
		;	D0.L: Paljon
		; OUPUT	D0.L: Outputted chars
		tst.b	Using
		beq	_StripANSI
		cmp.b	#1,Using
		beq	_StripHEX
		bne	_StripXEM

		SECTION	__MERGED,bss

Using:		ds.b	1
		EVEN

		END
