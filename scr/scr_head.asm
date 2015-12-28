*************************************************
*						*
*		 Screen FrontEd			*
*						*
*	  Copyright © By Pasi Ristioja		*
*						*
*************************************************

		OPT	!
		NEAR	A4,-2
		MACHINE	68000

		INCDIR	"asminc:"
		INCLUDE	"work:dterm/scr/scr_head.i"

		NREF	_gl_currxd

* Hardware screenin rutiinit

		XREF	_HInit
		XREF	_HRemove
		XREF	_HChange
		XREF	_HOn
		XREF	_HOff
		XREF	_HPrint
		XREF	_HSPrint
		XREF	_HDown
		XREF	_HUp
		XREF	_HClearInLine
		XREF	_HCopyInLine
		XREF	_HAreaClear
		XREF	_HAreaCopy
		XREF	_HCursorOn
		XREF	_HCursorOff
		XREF	_H_addcol
		XREF	_H_remcol
		XREF	_HScrollDown

* Software screenin rutiinit

		XREF	_SInit
		XREF	_SRemove
		XREF	_SChange
		XREF	_SOn
		XREF	_SOff
		XREF	_SPrint
		XREF	_SSPrint
		XREF	_SDown
		XREF	_SUp
		XREF	_SClearInLine
		XREF	_SCopyInLine
		XREF	_SAreaClear
		XREF	_SAreaCopy
		XREF	_SCursorOn
		XREF	_SCursorOff
		XREF	_S_addcol
		XREF	_S_remcol
		XREF	_SScrollDown

* P‰‰koodille n‰kyvat rutiinit

		XDEF	_ScrInfo

		XDEF	_ScreenToUse
		XDEF	_ScreenInit
		XDEF	_ScreenRemove
		XDEF	_ScreenChange
		XDEF	_ScreenOn
		XDEF	_ScreenOff
		XDEF	_ScreenPrint		; Ei talleta rekistereit‰
		XDEF	_StatPrint
		XDEF	_ScreenDown		; Ei talleta rekistereit‰
		XDEF	_ScreenUp		; Ei talleta rekistereit‰
		XDEF	_ClearInLine		; Ei talleta rekistereit‰
		XDEF	_CopyInLine		; Ei talleta rekistereit‰
		XDEF	_AreaClear		; Ei talleta rekistereit‰
		XDEF	_AreaCopy		; Ei talleta rekistereit‰
		XDEF	_CursorOn		; Ei talleta rekistereit‰
		XDEF	_CursorOff		; Ei talleta rekistereit‰
		XDEF	_Screen_addcol		; Ei talleta rekistereit‰
		XDEF	_Screen_remcol		; Ei talleta rekistereit‰
		XDEF	_ScrollDown		; Ei talleta rekistereit‰

		XREF	_RawTranslate

		SECTION	code,code

_ScreenToUse:	; Kumpaa screeni‰ k‰ytet‰‰n
		; INPUT	D0.B:	SCR_HARD (TRUE)
		;		SCR_SOFT (FALSE)
		move.b	d0,Using
		rts

_ScreenInit:	; OUPUT	D0.L: TRUE/FALSE
		tst.b	Using
		beq	_SInit
		bne	_HInit

_ScreenRemove:	; OUPUT	D0.L: TRUE/FALSE
		tst.b	Using
		beq	_SRemove
		bne	_HRemove

_ScreenChange:	; INPUT	A0.L: Osoite tageihin
		; OUPUT	D0.L: TRUE/FALSE
		tst.b	Using
		beq	_SChange
		bne	_HChange

_ScreenOn:	; Aktivoi screeni
		; OUPUT	D0.L: TRUE/FALSE
		tst.b	Using
		beq	_SOn
		bne	_HOn

_ScreenOff:	; Deaktivoi screeni
		; OUPUT	D0.L: TRUE/FALSE
		tst.b	Using
		beq	_SOff
		bne	_HOff

_ScreenPrint:	; 1. Input conversion l‰pi
		; 2. Printtaa ruudulle
		; INPUT	A0.L: Miss‰ (p‰‰ttyy nollaan!)
		;	D0.W: Paljon
		lea	Raw,a1
		move.l	_gl_currxd,a2
		jsr	_RawTranslate
		lea	Raw,a0
		tst.b	Using
		beq	_SPrint
		bne	_HPrint

_StatPrint:	; Printtaa statusriville (jos on...)
		;	A0.L: String (null end)
		tst.b	Using
		beq	_SSPrint
		bne	_HSPrint

_ScreenDown:	; Scrollaa alas
		tst.b	Using
		beq	_SDown
		bne	_HDown

_ScreenUp:	; Scrollaa ylˆs
		tst.b	Using
		beq	_SUp
		bne	_HUp

_ClearInLine:	; Clearaa rivill‰
		; INPUT	D1.W: start row
		;	D0.W: char count
		tst.b	Using
		beq	_SClearInLine
		bne	_HClearInLine

_CopyInLine:	; Kopioi merkkej‰ rivill‰
		; INPUT	D1.W: from row
		;	D2.W: to row
		;	D0.W: char count
		tst.b	Using
		beq	_SCopyInLine
		bne	_HCopyInLine

_AreaClear:	; Clearaa alue
		; INPUT	D0.W: start line
		;	D1.W: line count
		tst.b	Using
		beq	_SAreaClear
		bne	_HAreaClear

_AreaCopy:	; Kopioi alue
		; INPUT	D0.W: from line
		;	D2.W: to line
		;	D1.W: line count
		tst.b	Using
		beq	_SAreaCopy
		bne	_HAreaCopy

_CursorOn:	; Kursori esiin
		tst.b	Using
		beq	_SCursorOn
		bne	_HCursorOn

_CursorOff:	; Kursori piiloon
		tst.b	Using
		beq	_SCursorOff
		bne	_HCursorOff

_Screen_addcol:	; Uudet v‰rit
		; INPUT	A0.L: Osoite UWORD v‰reihin
		;	D0.W: Monta
		tst.b	Using
		beq	_S_addcol
		bne	_H_addcol

_Screen_remcol:	; Poista v‰rit
		tst.b	Using
		beq	_S_remcol
		bne	_H_remcol

_ScrollDown:	; Scrollaa
		; INPUT	D0.L: Monta rivi‰
		tst.b	Using
		beq	_SScrollDown
		bne	_HScrollDown

		SECTION	__MERGED,bss

_ScrInfo:	ds.b	SCREENINFO_SIZE
Raw:		ds.b	256
Using:		ds.b	1
		EVEN
		END
