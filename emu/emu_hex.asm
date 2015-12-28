;*
**	Hex output 'emulation' / Copyright © By Pasi Ristioja
*/

; V1.00	- 02.12.1993
; V1.10	- 06.02.1994
; V1.11 - 03.11.1995

		OPT	!
		NEAR	A4,-2
		MACHINE	68000

		INCDIR	"asminc:"
		INCLUDE "global.i"
		INCLUDE	"work:dterm/routinesa.i"
		INCLUDE	"work:dterm/scr/scr_head.i"
		INCLUDE	"mine/offseta4.i"

		NREF	_ScrInfo

		XREF	_ScreenPrint
		XREF	_ScreenDown
		XREF	_CursorOn
		XREF	_CursorOff
		XREF	_Screen_addcol
		XREF	_Screen_remcol

;		XDEF	_ver_hex
		XDEF	_InitHEX
		XDEF	_RemoveHEX
		XDEF	_ResetHEX
		XDEF	_ChangeHEX
		XDEF	_PrintHEX
		XDEF	_StripHEX

		SECTION	code,code

_InitHEX:	; INPUT	A0.L: Tags
		movem.l	d2-d7/a2-a6,-(sp)
		GETTAG	#PB_HEXCol,(a0)
		move.l	d0,a0
		addq.w	#4,a0
		moveq	#2,d0
		jsr	_Screen_addcol
		jsr	_CursorOff
		jsr	_CursorOn
		movem.l	(sp)+,d2-d7/a2-a6
		moveq	#TRUE,d0
		rts

_RemoveHEX:	; INPUT	A0.L: Tags
		movem.l	d2-d7/a2-a6,-(sp)
		bsr	_Screen_remcol	
		movem.l	(sp)+,d2-d7/a2-a6
		moveq	#TRUE,d0
		rts

_ResetHEX:	; INPUT	A0.L: Tags
		movem.l	d2-d7/a2-a6,-(sp)
		move.w	_ScrInfo+columns,d0
		lsr.w	#2,d0
		mulu.w	#3,d0
		move.w	d0,char_pos
		move.w	#0,hex_cur
		move.w	d0,char_cur
		clr.w	_ScrInfo+xpos		; Screen (external)
		clr.w	_ScrInfo+ypos
		move.w	#1,_ScrInfo+forecol
		clr.w	_ScrInfo+backcol
		clr.w	_ScrInfo+fontstate
		sf	_ScrInfo+inverse
		jsr	_CursorOff
		jsr	_CursorOn
		movem.l	(sp)+,d2-d7/a2-a6
		moveq	#TRUE,d0
		rts

_ChangeHEX:	; INPUT	A0.L: Tags
		move.w	_ScrInfo+columns,d0
		lsr.w	#2,d0
		mulu.w	#3,d0
		move.w	d0,char_pos
		move.w	#0,hex_cur
		move.w	d0,char_cur
		clr.w	_ScrInfo+xpos		; Screen (external)
		clr.w	_ScrInfo+ypos
		moveq	#TRUE,d0
		rts

_PrintHEX:	; INPUT	A0.L: Missä merkit
		;	D0.L: Paljon merkkejä
		tst.w	d0
		beq	.pass
		cmp.w	#2,_ScrInfo+columns
		blo	.pass
		movem.l	d2-d7/a2-a6,-(sp)
		movem.l	a0/d0,-(sp)
		jsr	_CursorOff
		movem.l	(sp)+,a2/d7

		subq.w	#1,d7
.more:		moveq	#0,d0
		move.b	(a2)+,d0
		move.b	d0,rawchar
		move.w	d0,d1
		lsr.w	#4,d0
		and.w	#$0f,d1
		lea	HexIndex(pc),a0
		move.b	(a0,d0.w),rawhex
		move.b	(a0,d1.w),rawhex+1
		movem.l	a2/d7,-(sp)
		move.w	hex_cur,_ScrInfo+xpos
		lea	rawhex,a0
		moveq	#2,d0
		jsr	_ScreenPrint
		cmp.w	#4,_ScrInfo+columns
		blo	.nochar
		move.w	char_cur,_ScrInfo+xpos
		lea	rawchar,a0
		moveq	#1,d0
		jsr	_ScreenPrint
.nochar:	addq.w	#3,hex_cur
		addq.w	#1,char_cur
		move.w	char_pos,d0
		cmp.w	hex_cur,d0
		bhi	.nowrap
		move.w	#0,hex_cur
		move.w	d0,char_cur
		move.w	_ScrInfo+ypos,d0
		addq.w	#1,d0
		cmp.w	_ScrInfo+lines,d0
		beq	.scroll
		move.w	d0,_ScrInfo+ypos
		bra	.nowrap
.scroll:	jsr	_ScreenDown
.nowrap:	movem.l	(sp)+,a2/d7
		dbf	d7,.more
		jsr	_CursorOn
		movem.l	(sp)+,d2-d7/a2-a6
.pass:		rts

_StripHEX:	; INPUT	A0.L: Mistä
		;	A1.L: Minne
		;	D0.L: Paljon
		; OUPUT	D0.L: Outputted chars
		move.l	d0,d1
.loop:		move.b	(a0)+,(a1)+
		subq.l	#1,d1
		bne	.loop
		rts

HexIndex:	dc.b	'0123456789ABCDEF'

		SECTION	__MERGED,data

char_pos:	dc.w	0
char_cur:	dc.w	0
hex_cur:	dc.w	0
rawhex:		dc.b	0,0,0
rawchar:	dc.b	0,0
;_ver_hex:	dc.b	'1.11 (Nov 03 1995)',0
		EVEN

		END
