;*
**	ANSI/VT102 Emulation / Copyright © By Pasi Ristioja
*/

; V1.0	-  05.12.1993 - Initial, removed from DTerm.
; V1.1	-  18.12.1993 - Removed input conversion
; V1.2	-  06.02.1994 - Improved color handling
; V1.3	-  16.02.1994 - Fixed bug in cursor save
; V1.4	-  10.04.1994 - Bug in tab
; V1.41	-  17.07.1994 - PreScroll

		OPT	!
		NEAR	A4,-2
		MACHINE	68000

		INCDIR	"asminc:"
		INCLUDE	"global.i"
		INCLUDE	"work:dterm/routinesa.i"
		INCLUDE	"work:dterm/emu/emu_ansi.i"
		INCLUDE	"work:dterm/scr/scr_head.i"
		INCLUDE	"work:dterm/ser/ser_head.i"
		INCLUDE	"mine/offsetA4.i"

		NREF	_ScrInfo
		NREF	_gl_cur_qv
		NREF	_IntuitionBase
		NREF	_GetPreScroll
		XREF	_LVODisplayBeep

		XREF	_ScreenPrint
		XREF	_ScreenDown
		XREF	_ScreenUp
		XREF	_ClearInLine
		XREF	_CopyInLine
		XREF	_AreaClear
		XREF	_AreaCopy
		XREF	_CursorOn
		XREF	_CursorOff
		XREF	_CursorPos
		XREF	_Screen_addcol
		XREF	_Screen_remcol
		XREF	_InsertTXD
		XREF	_ScrollDown

;		XDEF	_ansiver
		XDEF	_InitANSI
		XDEF	_RemoveANSI
		XDEF	_ResetANSI
		XDEF	_ChangeANSI
		XDEF	_PrintANSI
		XDEF	_StripANSI

		SECTION	code,code

Base:
B:	; Jump base word offseteille

_InitANSI:	; INPUT	A0.L: Taglist
		movem.l	d2-d7/a2-a6,-(sp)
		move.l	a0,a2
		GETTAG	#PB_ANSICol,(a2)
		move.l	d0,a3
		GETTAG	#PB_ANSIPc,(a2)
		tst.w	d0
		bne	.ansipc
		lea	Col8Mod,a0
		moveq	#8,d0
		sf	IBMBold
		bra	.setcol
.ansipc:	lea	Col16Mod,a0
		moveq	#16,d0
		st	IBMBold
.setcol:	move.l	a0,CurColMod
		move.w	7*2(a0),_ScrInfo+forecol
		move.w	0*2(a0),_ScrInfo+backcol
		lea	4(a3),a0
		jsr	_Screen_addcol
		jsr	_CursorOff
		jsr	_CursorOn
		movem.l	(sp)+,d2-d7/a2-a6
		moveq	#TRUE,d0
		rts

_RemoveANSI:	; INPUT	A0.L: Taglist
		movem.l	d2-d7/a2-a6,-(sp)
		bsr	_Screen_remcol
		movem.l	(sp)+,d2-d7/a2-a6
		moveq	#TRUE,d0
		rts

_ResetANSI:	; INPUT	A0.L: Taglist
		movem.l	d2-d7/a2-a6,-(sp)
		jsr	_CursorOff

		sf	ESCOk			; Some flags
		sf	StringControl
		sf	IBMHigh
		sf	FullFont
		sf	stripcsi
		clr.w	_ScrInfo+xpos		; Screen (external)
		clr.w	_ScrInfo+ypos
		clr.w	_ScrInfo+fontstate
		sf	_ScrInfo+inverse
		move.l	CurColMod,a0
		move.w	7*2(a0),_ScrInfo+forecol
		move.w	0*2(a0),_ScrInfo+backcol
		bsr	CLR

		jsr	_CursorOn
		movem.l	(sp)+,d2-d7/a2-a6
		moveq	#TRUE,d0
		rts

_ChangeANSI:	; INPUT	A0.L: Taglist
		movem.l	d2-d7/a2-a6,-(sp)
		move.l	a0,a2
		GETTAG	#PB_ANSIWrap,(a2)
		move.b	d0,Wrap
		GETTAG	#PB_ANSIKillBs,(a2)
		move.b	d0,KillBS
		GETTAG	#PB_ANSIStrip,(a2)
		move.b	d0,StripBold
		GETTAG	#PB_ANSICls,(a2)
		move.b	d0,CLRHome
		GETTAG	#PB_ANSICr2crlf,(a2)
		move.b	d0,CRLF
; check limits (will allow window resizing without reset)
		move.w	_ScrInfo+xpos,d0
		cmp.w	_ScrInfo+columnsdbf,d0
		bls	.ok1
		move.w	_ScrInfo+columnsdbf,_ScrInfo+xpos
.ok1:		move.w	_ScrInfo+ypos,d0
		cmp.w	_ScrInfo+linesdbf,d0
		bls	.ok2
		move.w	_ScrInfo+linesdbf,_ScrInfo+ypos
.ok2:		movem.l	(sp)+,d2-d7/a2-a6
		moveq	#TRUE,d0
		rts

_PrintANSI:	; INPUT	A0.L: Missä merkit
		;	D0.L: Paljon merkkejä
		tst.w	d0
		beq	.pass
		movem.l	d2-d7/a2-a6,-(sp)
		movem.l	a0/d0,-(sp)
		jsr	_CursorOff

		CUR_VAL	PB_HardScr,d6
		bne	.nopre
		CUR_VAL	PB_PreScroll,d6
		beq	.nopre
		move.w	_ScrInfo+ypos,d1
		cmp.w	_ScrInfo+linesdbf,d1
		bne	.nopre
		movem.l	(sp),a1/d7
		cmp.w	d7,d6
		bhs	.limitok
		move.w	d6,d7
.limitok:	subq.w	#1,d7
		moveq	#$a,d2
		moveq	#0,d0
.look:		cmp.b	(a1)+,d2
		dbeq	d7,.look
		bne	.counted
		addq.w	#1,d0
		cmp.w	d1,d0
		bhs	.counted
		subq.w	#1,d7
		bpl	.look
.counted:	cmp.w	#1,d0
		bls	.nopre
		sub.w	d0,_ScrInfo+ypos
		bsr	_ScrollDown

.nopre:		movem.l	(sp)+,a1/d7

.gochar:	tst.b	StringControl		; escape in progress?
		bne	.escape
.reset:		move.l	a1,a0
		lea	ControlChar,a2
		moveq	#0,d0
		move.w	_ScrInfo+columns,d1
		sub.w	_ScrInfo+xpos,d1	; left in line
.more:		moveq	#0,d2
		move.b	(a1)+,d2
		add.w	d2,d2
		move.w	(a2,d2.w),d3
		bne	.nochar
.deny:		addq.w	#1,d0
		subq.w	#1,d1
		beq	.flush1
		subq.w	#1,d7
		bne	.more
		addq.w	#1,d7
.flush1:	movem.l	d0/d1/d7/a1,-(sp)	; Printtaa merkkejä
		jsr	_ScreenPrint
		movem.l	(sp)+,d0/d1/d7/a1
		add.w	_ScrInfo+xpos,d0
		tst.w	d1
		bne	.wrap1
		subq.w	#1,d0
		tst.b	Wrap
		beq	.wrap1
		movem.l	d7/a1,-(sp)
		jsr	LF
		movem.l	(sp)+,d7/a1
		moveq	#0,d0
.wrap1:		move.w	d0,_ScrInfo+xpos
		subq.w	#1,d7
		bne	.reset
		bra	.done
.nochar:	tst.w	d0			; Käsittele koodi
		beq	.flush2
		movem.l	d0/d1/d3/d7/a1,-(sp)	; Printtaa merkkejä
		jsr	_ScreenPrint
		movem.l	(sp)+,d0/d1/d3/d7/a1
		add.w	_ScrInfo+xpos,d0
		tst.w	d1
		bne	.wrap2
		subq.w	#1,d0
		tst.b	Wrap
		beq	.wrap2
		movem.l	d3/d7/a1,-(sp)
		jsr	LF
		movem.l	(sp)+,d3/d7/a1
		moveq	#0,d0
.wrap2:		move.w	d0,_ScrInfo+xpos
.flush2:	movem.l	d7/a1,-(sp)
		lea	Base(pc),a1
		jsr	(a1,d3.w)
		movem.l	(sp)+,d7/a1
		subq.w	#1,d7
		bne	.gochar
		bra	.done
.escape:	moveq	#0,d0
		move.b	(a1)+,d0
		movem.l	d7/a1,-(sp)
		bsr	EscapeProc
		movem.l	(sp)+,d7/a1
		subq.w	#1,d7
		bne	.gochar
.done:		jsr	_CursorOn
		movem.l	(sp)+,d2-d7/a2-a6
.pass:		rts

_StripANSI:	; INPUT	A0.L: Mistä
		;	A1.L: Minne
		;	D0.L: Paljon
		; OUPUT	D0.L: Outputted chars
		move.w	d0,d1
		beq	.done
		movem.l	d2-d4,-(sp)
		subq.w	#1,d1
		moveq	#0,d0
		move.b	stripcsi,d3
		move.l	a1,d4
.loop:		move.b	(a0)+,d2
		tst.b	d3
		beq	.nocsi
		cmp.b	#';',d2			; skippaa escape
		bls	.stillcsi
		cmp.b	#'[',d2
		beq	.stillcsi
		moveq	#0,d3
		bra	.stillcsi
.nocsi:		cmp.b	#$1b,d2
		beq.b	.csifound
		cmp.b	#$9b,d2
		bne	.normal
.csifound:	moveq	#-1,d3
		dbf	d1,.loop
		bra	.stripped
.normal:	move.b	d2,(a1)+
.stillcsi:	dbf	d1,.loop

.stripped:	sub.l	d4,a1
		move.l	a1,d0
		move.b	d3,stripcsi
		movem.l	(sp)+,d2-d4
.done:		rts

; --> Erikoismerkkien handlaus

NOC:		; Controlli ilman koodia
		rts

BEL:		; Ring Bell
		move.l	_IntuitionBase,a6
		sub.l	a0,a0
		LIBC	DisplayBeep
		rts

DEL:		; Delete
		subq.w	#1,_ScrInfo+xpos
		bpl	delok
		clr.w	_ScrInfo+xpos
delok:		tst.b	KillBS
		beq	nokb
		lea	Space,a0
		moveq	#1,d0
		jsr	_ScreenPrint
nokb:		rts

TAB:		; Tabulator
		move.w	_ScrInfo+xpos,d0
		lea	TabL,a0
		add.w	d0,d0
		move.w	(a0,d0.w),_ScrInfo+xpos
		rts

LF:		tst.b	CRLF		; LineFeed
		beq	.nocrlf
		clr.w	_ScrInfo+xpos
.nocrlf:	move.w	_ScrInfo+ypos,d0
		addq.w	#1,d0
		cmp.w	_ScrInfo+linesdbf,d0
		bls	.nextlf
		jsr	_ScreenDown
		move.w	_ScrInfo+linesdbf,d0
.nextlf:	move.w	d0,_ScrInfo+ypos
		rts

VT:		; Vertical TAB / yksi rivi ylös
		move.w	_ScrInfo+ypos,d0
		subq.w	#1,d0
		bpl	.goup
		moveq	#0,d0
.goup:		move.w	d0,_ScrInfo+ypos
		rts

CLR:		; Clear screen
		moveq	#0,d0
		move.w	_ScrInfo+lines,d1
		jsr	_AreaClear
		clr.w	_ScrInfo+xpos		; Home cursor
		clr.w	_ScrInfo+ypos
		rts

RET:		; Return
		clr.w	_ScrInfo+xpos
		rts

SIN:		; Shift In (Raw merkit -> kaikki printataan)
		st	FullFont
		rts
	
SOU:		; Shift Out (ANSI merkitys)
		sf	FullFont
		rts

ESC:		; Escape sequence ID check. ELI: ESC[ + data
		st	StringControl
		rts

DEL2:		; Deletoi 1 merkki rivistä
		rts

ShortESC:	; Escape sequence ID check(varmistettu). ELI: $9b + data
		st	StringControl	; Aloitetaan ANSI-Escape tulkkaus
		st	ESCOk		; Kumpikin merkki saatu
		clr.w	ESCCount
		rts

; --> ANSI rutiinit

LowSpeed:	; Yli 2 numeroa / todennäköisesti tarpeeton
		lea	NumData,a0
		lea	MuluTable,a1
		moveq	#0,d3
.loop:		moveq	#0,d2
		move.b	(a0)+,d2
		move.w	d1,d4
		add.w	d4,d4
		mulu.w	(a1,d4.w),d2
;		mulu.w	(a1,d1.w*2),d2
		add.w	d2,d3
		subq.w	#1,d1
		bne	.loop
		lea	ESCNum,a0
		add.w	ESCCount,a0
		move.w	d3,(a0)
		addq.w	#2,ESCCount
		clr.w	NumCount
		rts
MedSpeed:	; 2 numeroa
		lea	QuickTwo,a1
		move.w	NumData,d1
		lsl.b	#4,d1
		lea	ESCNum,a0
		add.w	ESCCount,a0
		lsr.w	#3,d1
		move.w	(a1,d1.w),(a0)
		addq.w	#2,ESCCount
		clr.w	NumCount
		rts
ASCtoNum:	; Muuta NumDatassa olevat ASCII-$30 luvuksi ja laita se
		; ESCNum. (D0 ei saa sotkea!)
		cmp.w	#32,ESCCount		; 16 enintään
		bhs	.nonum
		move.w	NumCount,d1
		beq	.nonum
		cmp.w	#2,d1
		beq	MedSpeed
		bhi	LowSpeed
; --> Yksi numero
		move.b	NumData,d1
		lea	ESCNum,a0
		add.w	ESCCount,a0
		move.w	d1,(a0)
		addq.w	#2,ESCCount
		clr.w	NumCount
.nonum:		rts

EscapeProc:	; Processoi ANSI ESCAPE SEQUENCEA
		; INPUT	D0.W: Merkki
		tst.b	ESCOk
		bne	DataProc
		cmp.b	#'[',d0
		bne	ESCFailed
		st	ESCOk
		clr.w	ESCCount
		rts
ESCFailed:	; Escape ID oli väärin
		sf	StringControl
		sf	ESCOk
		rts
DataProc:	cmp.b	#'9',d0
		bhi	DoSeq
		cmp.b	#'0',d0
		blo	DoSeq
		cmp.w	#5,NumCount		; 5-lukua maksimissaan
		bhs	.numexp
		sub.b	#'0',d0
		lea	NumData,a0
		add.w	NumCount,a0
		move.b	d0,(a0)
		addq.w	#1,NumCount
.numexp:	rts
DoSeq:		; Tutki mitä ei-numerot ovat / käynnistä sequence jos ei ;
		cmp.b	#';',d0
		beq	ASCtoNum		; (RTS)
		bsr	ASCtoNum
		sf	StringControl
		sf	ESCOk
		lea	ESCRoutines,a0
		add.w	d0,d0
		move.w	(a0,d0.w),d0
;		move.w	(a0,d0.w*2),d0
		beq	.noesc
		lea	Base(pc),a0
		jmp	(a0,d0.w)
;		jmp	Base+2(pc,d0.w)			; +2 = phxasm bug!
.noesc:		rts

; --> Varsinaiset ESCAPE ANSI rutiinit

;--> ***
Code_h2:	; Select mode
		cmp.w	#2,ESCCount
		bne	Code_h2_Error
		move.w	ESCNum,d0
		cmp.w	#20,d0
		bne	Code_h2_Error
		st	CRLF		; LineFeed on LF+RET
Code_h2_Error:	rts
***

;--> ***
Code_l2:	; Reset mode
		cmp.w	#2,ESCCount
		bne	Code_l2_Error
		move.w	ESCNum,d0
		cmp.w	#20,d0
		bne	Code_l2_Error
		sf	CRLF		; LineFeed on LF
Code_l2_Error:	rts
***

;--> ***
Code_m2:	; Fontin moodi + väri
		lea	ESCNum,a1
		move.w	ESCCount,d0
		lsr.w	#1,d0
		subq.w	#1,d0
		bpl	Loop50
		moveq	#0,d0
		bra	ResetStyle
Loop50:		move.w	(a1)+,d1
		cmp.w	#8,d1			; Fontin moodi
		bhi	ColorDef
		tst.w	d1
		bne	Code_m2_ModeDo
ResetStyle:	clr.w	_ScrInfo+fontstate
		sf	_ScrInfo+inverse
		sf	IBMHigh
		move.l	CurColMod,a0
		move.w	7*2(a0),_ScrInfo+forecol
		move.w	0*2(a0),_ScrInfo+backcol
		bra	More_m2
Code_m2_ModeDo:	cmp.w	#1,d1
		bne	Code_m2_1
		tst.b	IBMBold
		bne	.highintensity
		tst.b	StripBold
		bne	More_m2
		st	IBMHigh	; ***CHECK!
		or.w	#2,_ScrInfo+fontstate
		bra	More_m2
.highintensity:	st	IBMHigh			; Kirkas väri päälle
		move.w	_ScrInfo+forecol,d1
		cmp.w	#7,d1
		bhi	More_m2
		addq.w	#8,d1
		move.l	CurColMod,a0
		add.w	d1,d1
		move.w	(a0,d1.w),_ScrInfo+forecol
;		move.w	([CurColMod],d1.w*2),_ScrInfo+forecol
		bra	More_m2
Code_m2_1:	cmp.w	#4,d1
		bne	Code_m2_2
		or.w	#1,_ScrInfo+fontstate
		bra	More_m2
Code_m2_2:	cmp.w	#7,d1
		bne	More_m2
		st	_ScrInfo+inverse
		bra	More_m2

ColorDef:	cmp.w	#37,d1
		bhi	BackGround
		sub.w	#30,d1
		tst.b	IBMHigh			; Lisää kirkkautta väriin
		beq	nohigh
		addq.w	#8,d1
nohigh:		move.l	CurColMod,a0
		add.w	d1,d1
		move.w	(a0,d1.w),_ScrInfo+forecol
;		move.w	([CurColMod],d1.w*2),_ScrInfo+forecol
		bra	More_m2
BackGround:	cmp.w	#47,d1			; Liian suuri
		bhi	More_m2
		sub.w	#40,d1
		move.l	CurColMod,a0
		add.w	d1,d1
		move.w	(a0,d1.w),_ScrInfo+backcol
;		move.w	([CurColMod],d1.w*2),_ScrInfo+backcol
More_m2:	dbf	d0,Loop50
Code_m2_Error:	rts
***

;--> ***
Code_n2:	; Device Status Report (VT220)
		cmp.w	#2,ESCCount
		bne	Code_n2_Error
		lea	Report1,a2
		not.b	CurDir
		beq	.dir
		lea	Report2,a2
.dir:		cmp.w	#5,ESCNum
		beq	.state
		cmp.w	#6,ESCNum
		beq	.currep
		bra	Code_n2_Error
.state:		move.l	a2,a0			; Operating status (ok)
		move.l	#$1b5b306e,(a0)		; Esc [ 0 n
		moveq	#4,d0
		jmp	_InsertTXD
.currep:	move.l	a2,a3			; Cursor report
		move.w	#$1b5b,(a2)+		; Esc [
		moveq	#0,d0
		move.w	_ScrInfo+ypos,d0
		addq.w	#1,d0
		move.l	a2,a0
		jsr	_DoAscii
		add.w	d0,a2
		move.b	#';',(a2)+
		moveq	#0,d0
		move.w	_ScrInfo+xpos,d0
		addq.w	#1,d0
		move.l	a2,a0
		jsr	_DoAscii
		add.w	d0,a2
		move.b	#'R',(a2)+
		sub.l	a3,a2
		move.l	a3,a0
		move.l	a2,d0
		jmp	_InsertTXD
Code_n2_Error:	rts
***

; --> ***
Code_Hf:	; Kursorin kohta (Line,Column) / (y/x)
		lea	ESCNum,a1
		move.w	ESCCount,d1
		bne	Code_Hf_NoDef
		moveq	#1,d0			; Def value
Code_Hf_HalfDef	moveq	#1,d1
		bra	Code_Hf_do
Code_Hf_NoDef:	cmp.w	#4,d1
		bhi	Code_Hf_Error
		move.w	(a1)+,d0
		beq	Code_Hf_Error
		cmp.w	#2,d1
		beq	Code_Hf_HalfDef
		move.w	(a1)+,d1
		beq	Code_Hf_Error
Code_Hf_do:	cmp.w	_ScrInfo+lines,d0
		bls	Limit_Hf_Ok
		move.w	_ScrInfo+lines,d0
Limit_Hf_Ok:	cmp.w	_ScrInfo+columns,d1
		bls	Limit2_Hf_ok
		move.w	_ScrInfo+columns,d1
Limit2_Hf_ok:	subq.w	#1,d0
		subq.w	#1,d1
		move.w	d0,_ScrInfo+ypos
		move.w	d1,_ScrInfo+xpos	
Code_Hf_Error:	rts
***

;--> ***
Code_s2:	; Talleta kursorin paikka
		tst.w	ESCCount
		bne	Code_s2_Error
		move.w	_ScrInfo+xpos,SaveX
		move.w	_ScrInfo+ypos,SaveY
Code_s2_Error:	rts
***

;--> ***
Code_u2:	; Talletetun kursorin paikka takaisin
		tst.w	ESCCount
		bne	Code_u2_Error
		move.w	SaveX,d0
		move.w	SaveY,d1
		cmp.w	_ScrInfo+columnsdbf,d0
		bls	.okx
		move.w	_ScrInfo+columnsdbf,d0
.okx:		cmp.w	_ScrInfo+linesdbf,d1
		bls	.oky
		move.w	_ScrInfo+linesdbf,d1
.oky:		move.w	d0,_ScrInfo+xpos
		move.w	d1,_ScrInfo+ypos
Code_u2_Error:	rts
***

;--> ***
Code_64:	; Insertaa spaceja riviin
		move.w	ESCCount,d0
		bne	Code_64_NoDef
		moveq	#1,d0			; DEF Arvo
		bra	Code_64_Do
Code_64_NoDef:	cmp.w	#2,d0
		bne	Code_64_Error
		move.w	ESCNum,d0
Code_64_Do:	cmp.w	_ScrInfo+columns,d0
		bhi	Code_64_Error
		move.w	_ScrInfo+xpos,d1
		add.w	d0,d1
		cmp.w	_ScrInfo+columnsdbf,d1
		blo	NoMax
		move.w	_ScrInfo+columns,d0
		sub.w	_ScrInfo+xpos,d0
		bra	FastInsert
NoMax:		; --> Siirrä riviä
		move.w	d0,-(sp)
		move.w	_ScrInfo+columns,d0
		sub.w	d1,d0			; Monta merkkiä
		move.w	d1,d2			; Minne
		move.w	_ScrInfo+xpos,d1	; Mistä
		jsr	_CopyInLine
		move.w	(sp)+,d0
FastInsert:	; --> Tyhjennä insert alue / D0.W = Monta merkkiä
		move.w	_ScrInfo+xpos,d1
		jsr	_ClearInLine
Code_64_Error:	rts
***

; --> ***
Code_A:		; Siirrä kursoria ylös
		move.w	ESCCount,d0
		bne	Code_A_NoDef
		moveq	#1,d0			; DEF Arvo
		bra	Code_A_Do
Code_A_NoDef:	cmp.w	#2,d0
		bne	Code_A_Error
		move.w	ESCNum,d0
Code_A_Do:	move.w	_ScrInfo+ypos,d1
		sub.w	d0,d1
		bpl	Code_A_Ok
		moveq	#0,d1
Code_A_Ok:	move.w	d1,_ScrInfo+ypos
Code_A_Error:	rts
***

; --> ***
Code_B:		; Siirrä kursoria alas
		move.w	ESCCount,d0
		bne	Code_B_NoDef
		moveq	#1,d0			; DEF Arvo
		bra	Code_B_Do
Code_B_NoDef:	cmp.w	#2,d0
		bne	Code_B_Error
		move.w	ESCNum,d0
Code_B_Do:	move.w	_ScrInfo+ypos,d1
		add.w	d0,d1
		cmp.w	_ScrInfo+linesdbf,d1
		bls	Code_B_Ok
		move.w	_ScrInfo+linesdbf,d1
Code_B_Ok:	move.w	d1,_ScrInfo+ypos
Code_B_Error:	rts
***

; --> ***
Code_C:		; Siirrä kursoria eteenpäin
		move.w	ESCCount,d0
		bne	Code_C_NoDef
		moveq	#1,d0			; DEF Arvo
		bra	Code_C_Do
Code_C_NoDef:	cmp.w	#2,d0
		bne	Code_C_Error
		move.w	ESCNum,d0
Code_C_Do:	move.w	_ScrInfo+xpos,d1
		add.w	d0,d1
		cmp.w	_ScrInfo+columnsdbf,d1
		bls	Code_C_Ok
		move.w	_ScrInfo+columnsdbf,d1
Code_C_Ok:	move.w	d1,_ScrInfo+xpos
Code_C_Error:	rts
***

; --> ***
Code_D:		; Siirrä kursoria taaksepäin
		move.w	ESCCount,d0
		bne	Code_D_NoDef
		moveq	#1,d0			; DEF Arvo
		bra	Code_D_Do
Code_D_NoDef:	cmp.w	#2,d0
		bne	Code_D_Error
		move.w	ESCNum,d0
Code_D_Do:	move.w	_ScrInfo+xpos,d1
		sub.w	d0,d1
		bpl	Code_D_Ok
		moveq	#0,d1
Code_D_Ok:	move.w	d1,_ScrInfo+xpos
Code_D_Error:	rts
***

; --> ***
Code_E:		; Siirrä kursoria alas + ensimmäiseen columniin
		move.w	ESCCount,d0
		bne	Code_E_NoDef
		moveq	#1,d0			; DEF Arvo
		bra	Code_E_Do
Code_E_NoDef:	cmp.w	#2,d0
		bne	Code_E_Error
		move.w	ESCNum,d0
Code_E_Do:	move.w	_ScrInfo+ypos,d1
		add.w	d0,d1
		cmp.w	_ScrInfo+linesdbf,d1
		bls	Code_E_Ok
		move.w	_ScrInfo+linesdbf,d1
Code_E_Ok:	clr.w	_ScrInfo+xpos
		move.w	d1,_ScrInfo+ypos
Code_E_Error:	rts
***

; --> ***
Code_F:		; Siirrä kursoria ylös + ensimmäiseen columniin
		move.w	ESCCount,d0
		bne	Code_F_NoDef
		moveq	#1,d0			; DEF Arvo
		bra	Code_F_Do
Code_F_NoDef:	cmp.w	#2,d0
		bne	Code_F_Error
		move.w	ESCNum,d0
Code_F_Do:	move.w	_ScrInfo+ypos,d1
		sub.w	d0,d1
		bpl	Code_F_Ok
		moveq	#0,d1
Code_F_Ok:	clr.w	_ScrInfo+xpos
		move.w	d1,_ScrInfo+ypos
Code_F_Error:	rts
***

; --> ***
Code_J:		; Tyhjennä näyttö eri tavoilla
		move.w	ESCCount,d0
		bne	Code_J_NoDef
		moveq	#0,d0			; DEF Arvo
		bra	Code_J_Do
Code_J_NoDef:	cmp.w	#2,d0
		bne	Code_J_Error
		move.w	ESCNum,d0
Code_J_Do:	tst.w	d0
		beq	Code_J_0
		cmp.w	#1,d0
		beq	Code_J_1
		moveq	#0,d0			; Clear screen
		move.w	_ScrInfo+lines,d1
		jsr	_AreaClear
		tst.b	CLRHome
		beq	.right
		clr.w	_ScrInfo+xpos	; Home cursor
		clr.w	_ScrInfo+ypos
.right:		rts
Code_J_0:	; From cursor to end of screen
		bsr	Code_K_0
		move.w	_ScrInfo+linesdbf,d1
		sub.w	_ScrInfo+ypos,d1	; Monta riviä
		beq	Code_J_Go
		move.w	_ScrInfo+ypos,d0	; Mikä rivi
		addq.w	#1,d0
		jsr	_AreaClear
Code_J_Go:	rts
Code_J_1:	; From start of screen to cursor
		bsr	Code_K_1
		move.w	_ScrInfo+ypos,d1	; Monta riviä
		beq	Code_J_Error
		moveq	#0,d0			; Mikä rivi
		jsr	_AreaClear
Code_J_Error:	rts
***

;--> ***
Code_K:		; Tyhjennä eri tavoilla
		move.w	ESCCount,d0
		bne	Code_K_NoDef
		moveq	#0,d0			; DEF Arvo
		bra	Code_K_Do
Code_K_NoDef:	cmp.w	#2,d0
		bne	Code_K_Error
		move.w	ESCNum,d0
Code_K_Do:	tst.w	d0
		beq	Code_K_0
		cmp.w	#1,d0
		beq	Code_K_1
; --> erase line containing cursor
		move.w	_ScrInfo+columns,d0
		move.w	#0,d1
		jsr	_ClearInLine
		rts
Code_K_0:	; Erase from cursor to end of line
		move.w	_ScrInfo+xpos,d1
		move.w	_ScrInfo+columns,d0
		sub.w	d1,d0
		jsr	_ClearInLine
		rts
Code_K_1:	; Erase from beginning of line to cursor
		move.w	_ScrInfo+xpos,d0
		addq.w	#1,d0
		move.w	#0,d1
		jsr	_ClearInLine
Code_K_Error:	rts
***

;--> ***
Code_L:		; Insertoi line
		move.w	ESCCount,d0
		bne	Code_L_NoDef
		moveq	#1,d1			; DEF Arvo
		bra	Code_L_Do
Code_L_NoDef:	move.w	ESCNum,d1
Code_L_Do:	move.w	_ScrInfo+lines,d0
		sub.w	_ScrInfo+ypos,d0
		sub.w	d1,d0
		beq	OnlyClear
		bpl	CopyArea
		add.w	d1,d0
		move.w	d0,d1
		bra	OnlyClear
CopyArea:	move.w	d1,-(sp)
		move.w	_ScrInfo+ypos,d2
		add.w	d1,d2			; Minne
		move.w	d0,d1			; Monta riviä
		move.w	_ScrInfo+ypos,d0	; Mistä
		jsr	_AreaCopy
		move.w	(sp)+,d1
OnlyClear:	; D1.W: Monta riviä tyhjennetään
		move.w	_ScrInfo+ypos,d0
		jsr	_AreaClear
Code_L_Error:	rts
***

;--> ***
Code_M:		; Delete line
		move.w	ESCCount,d0
		bne	Code_M_NoDef
		moveq	#1,d1			; DEF Arvo
		bra	Code_M_Do
Code_M_NoDef:	move.w	ESCNum,d1
Code_M_Do:	move.w	_ScrInfo+lines,d0
		sub.w	_ScrInfo+ypos,d0
		sub.w	d1,d0
		beq	OnlyClear2
		bpl	CopyArea2
		add.w	d1,d0
		move.w	d0,d1
		bra	OnlyClear2
CopyArea2:	move.w	d1,-(sp)
		move.w	d0,d4
		move.w	_ScrInfo+ypos,d2	; Minne
		move.w	d2,d0
		add.w	d1,d0			; Mistä
		move.w	d4,d1			; Monta riviä
		jsr	_AreaCopy
		move.w	(sp)+,d1
OnlyClear2:	; INPUT	D1.W: Monta riviä tyhjennetään
		move.w	_ScrInfo+lines,d0
		sub.w	d1,d0
		jsr	_AreaClear
Code_M_Error:	rts
***

;--> ***
Code_P:		; Deletoi merkkejä rivistä
		move.w	ESCCount,d0
		bne	Code_P_NoDef
		moveq	#1,d0			; DEF Arvo
		bra	Code_P_Do
Code_P_NoDef:	cmp.w	#2,d0
		bne	Code_P_Error
		move.w	ESCNum,d0
Code_P_Do:	cmp.w	_ScrInfo+columns,d0
		bhi	.max			; if del > column we just del rest
		move.w	_ScrInfo+xpos,d1
		add.w	d0,d1
		cmp.w	_ScrInfo+columnsdbf,d1
		blo	.nomax
.max:		move.w	_ScrInfo+columns,d0
		sub.w	_ScrInfo+xpos,d0
		bra	.fastdelete
.nomax:		; Siirrä riviä
		move.w	d0,-(sp)
		move.w	_ScrInfo+columns,d0
		sub.w	d1,d0			; Monta merkkiä
		move.w	_ScrInfo+xpos,d2	; Minne / D1 = Mistä
		jsr	_CopyInLine
		move.w	(sp)+,d0
.fastdelete:	; Tyhjennä liikutetut merkit / D0.W = Monta merkkiä
		move.w	_ScrInfo+columns,d1
		sub.w	d0,d1
		jsr	_ClearInLine
Code_P_Error:	rts
***

; --> ***
Code_S:		; Scrollaa ylöspäin
		move.w	ESCCount,d0
		bne	Code_S_NoDef
		moveq	#1,d0			; DEF Arvo
		bra	Code_S_Do
Code_S_NoDef:	cmp.w	#2,d0
		bne	Code_S_Error
		move.w	ESCNum,d0
Code_S_Do:	subq.w	#1,d0
		bmi	Code_S_Error
		cmp.w	_ScrInfo+lines,d0
		blo	LoopUp
		move.w	_ScrInfo+lines,d0
LoopUp:		move.l	d0,-(sp)
		jsr	_ScreenUp
		move.l	(sp)+,d0
		dbf	d0,LoopUp
Code_S_Error:	rts
***

; --> ***
Code_T:		; Scrollaa alaspäin
		move.w	ESCCount,d0
		bne	Code_T_NoDef
		moveq	#1,d0			; DEF Arvo
		bra	Code_T_Do
Code_T_NoDef:	cmp.w	#2,d0
		bne	Code_T_Error
		move.w	ESCNum,d0
Code_T_Do:	subq.w	#1,d0
		bmi	Code_T_Error
		cmp.w	_ScrInfo+lines,d0
		blo	LoopDown
		move.w	_ScrInfo+lines,d0
LoopDown:	move.l	d0,-(sp)
		jsr	_ScreenDown
		move.l	(sp)+,d0
		dbf	d0,LoopDown
Code_T_Error:	rts
***

ControlChar:	dc.w	0,0,0,0,0,0,0,BEL-B,DEL-B
		dc.w	TAB-B,LF-B,VT-B,CLR-B,RET-B,SIN-B,SOU-B,0,0
		dc.w	0,0,0,0,0,0,0,0,0
		dc.w	ESC-B,0,0,0,0
		blk.w	95,0
		dc.w	DEL2-B
		blk.w	27,0
		dc.w	ShortESC-B
		blk.w	100,0

ESCRoutines:	blk.w	64,0
		dc.w	Code_64-B	; 64 @
		dc.w	Code_A-B	; 65 A
		dc.w	Code_B-B	; 66 B
		dc.w	Code_C-B	; 67 C
		dc.w	Code_D-B	; 68 D
		dc.w	Code_E-B	; 69 E
		dc.w	Code_F-B	; 70 F
		dc.w	0
		dc.w	Code_Hf-B	; 72 H (f)
		dc.w	0
		dc.w	Code_J-B	; 74 J
		dc.w	Code_K-B	; 75 K
		dc.w	Code_L-B	; 76 L
		dc.w	Code_M-B	; 77 M
		dc.w	0
		dc.w	0
		dc.w	Code_P-B	; 80 P
		dc.w	0
		dc.w	0
		dc.w	Code_S-B	; 83 S
		dc.w	Code_T-B	; 84 T
		blk.w	17,0
		dc.w	Code_Hf-B	; 102 f (H)
		dc.w	0
		dc.w	Code_h2-B	; 104 h
		dc.w	0
		dc.w	0
		dc.w	0
		dc.w	Code_l2-B	; 108 l
		dc.w	Code_m2-B	; 109 m
		dc.w	Code_n2-B	; 110 n
		dc.w	0
		dc.w	0
		dc.w	0
		dc.w	0
		dc.w	Code_s2-B	; 115 s
		dc.w	0
		dc.w	Code_u2-B	; 117 u
		blk.w	147,0

MuluTable:	dc.w	0,1,10,100,1000,10000
QuickTwo:	dc.w	00,01,02,03,04,05,06,07,08,09,00,00,00,00,00,00
		dc.w	10,11,12,13,14,15,16,17,18,19,00,00,00,00,00,00
		dc.w	20,21,22,23,24,25,26,27,28,29,00,00,00,00,00,00
		dc.w	30,31,32,33,34,35,36,37,38,39,00,00,00,00,00,00
		dc.w	40,41,42,43,44,45,46,47,48,49,00,00,00,00,00,00
		dc.w	50,51,52,53,54,55,56,57,58,59,00,00,00,00,00,00
		dc.w	60,61,62,63,64,65,66,67,68,69,00,00,00,00,00,00
		dc.w	70,71,72,73,74,75,76,77,78,79,00,00,00,00,00,00
		dc.w	80,81,82,83,84,85,86,87,88,89,00,00,00,00,00,00
		dc.w	90,91,92,93,94,95,96,97,98,99,00,00,00,00,00,00

Col8Mod:	dc.w	00,01,02,03,04,05,06,07,07,01,02,03,04,05,06,07
Col16Mod:	dc.w	00,01,02,03,04,05,06,07,08,09,10,11,12,13,14,15

Space:		dc.b	' '
		EVEN

		SECTION	__MERGED,data

TabL:		dc.w	08,08,08,08,08,08,08,08,16,16,16,16,16,16,16,16
		dc.w	24,24,24,24,24,24,24,24,32,32,32,32,32,32,32,32
		dc.w	40,40,40,40,40,40,40,40,48,48,48,48,48,48,48,48
		dc.w	56,56,56,56,56,56,56,56,64,64,64,64,64,64,64,64
		dc.w	72,72,72,72,72,72,72,72,79,79,79,79,79,79,79,79
		blk.w	256-80,79

ESCOk:		ds.b	1	; -1 = Escape ID tunnistettu
StringControl:	ds.b	1	; -1 = Processoidaan ESC-kontrollia
NumCount:	ds.w	1	; Monta numeroa
ESCCount:	ds.w	1	; Monta numeroa otettu
ESCNum:		ds.w	16	; Numeroarvot
NumData:	ds.b	5	; Luvun osat (ASCII muotoista -$30 )
stripcsi:	ds.b	1
; --> Emulation parameters
Wrap:		ds.b	1	; Linewrap
KillBS:		ds.b	1	; Killer BS
CRLF:		ds.b	1	; CR -> CRLF
StripBold:	ds.b	1	; Ei boldia
IBMBold:	ds.b	1	; 16 color screen
CLRHome:	ds.b	1	; CLR homes also cursor
FullFont:	ds.b	1	; -1 = Kaikki merkit printataan
IBMHigh:	ds.b	1	; -1 = high intensity on
		EVEN
CurColMod:	dc.l	0	; Mikä ansicol -> realcol table
Report1:	ds.b	10
Report2:	ds.b	10
CurDir:		dc.b	0
		EVEN
SaveX:		ds.w	1
SaveY:		ds.w	1

;_ansiver:	dc.b	'1.41 (Jul 17 1994)',0
		EVEN

		END
