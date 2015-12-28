;*
**	Hardware screen / Copyright © By Pasi Ristioja
*/

		OPT	!
		NEAR	A4,-2
		MACHINE	68000

		INCDIR	"asminc:"
		INCLUDE	"exec/funcdef.i"
		INCLUDE	"exec/memory.i"
		INCLUDE	"exec/exec_lib.i"
		INCLUDE	"graphics/gfx.i"
		INCLUDE	"graphics/gfxbase.i"
		INCLUDE	"graphics/rastport.i"
		INCLUDE	"graphics/text.i"
		INCLUDE	"graphics/videocontrol.i"
		INCLUDE	"intuition/screens.i"
		INCLUDE	"mine/offseta4.i"

		INCLUDE	"work:dterm/global.i"
		INCLUDE	"work:dterm/routinesa.i"
		INCLUDE	"work:dterm/scr/scr_head.i"

		XREF	_LinkerDB
		NREF	_SysBase
		NREF	_GfxBase
		NREF	_IntuitionBase
		NREF	_DiskfontBase
		NREF	_ScrInfo
		NREF	_gl_conscreen
		NREF	_gl_cur_qv

		XREF	_LVOOpenFont
		XREF	_LVOOpenDiskFont
		XREF	_LVOCloseFont
		XREF	_LVOStripFont
		XREF	_LVOWaitBlit
		XREF	_LVOOwnBlitter
		XREF	_LVODisownBlitter
		XREF	_LVOLoadView
		XREF	_LVOWaitTOF
		XREF	_LVOInitRastPort
		XREF	_LVOInitBitMap
		XREF	_LVOVideoControl
		XREF	_LVOMakeScreen
		XREF	_LVORethinkDisplay
		XREF	_TempPrint
		XREF	_SetRTS
		XREF	_alloc_pool
		XREF	_free_pool

;		XDEF	_hardscrver
		XDEF	_HInit
		XDEF	_HRemove
		XDEF	_HChange
		XDEF	_HOn
		XDEF	_HOff
		XDEF	_HPrint
		XDEF	_HSPrint
		XDEF	_HDown
		XDEF	_HUp
		XDEF	_HClearInLine
		XDEF	_HCopyInLine
		XDEF	_HAreaClear
		XDEF	_HAreaCopy
		XDEF	_HCursorOn
		XDEF	_HCursorOff
		XDEF	_H_addcol
		XDEF	_H_remcol
		XDEF	_HScrollDown

WAITBLIT	MACRO
		movem.l	d0-d1/a0-a1/a6,-(sp)
		CALL	Gfx,WaitBlit
		movem.l	(sp)+,d0-d1/a0-a1/a6
		ENDM

MYBLIT		MACRO	; Blitteri omaan käyttöön
		movem.l	d0-d1/a0-a1/a6,-(sp)
		CALL	Gfx,OwnBlitter
		LIBC	WaitBlit
		move.w	#$0400,$dff096		; Nasty OFF
		movem.l	(sp)+,d0-d1/a0-a1/a6
		ENDM

ENDBLIT		MACRO
		movem.l	d0-d1/a0-a1/a6,-(sp)
		CALL	Gfx,WaitBlit
		LIBC	DisownBlitter
		movem.l	(sp)+,d0-d1/a0-a1/a6
		ENDM

		SECTION	code,code

_HInit:		; OUPUT	D0.L: TRUE/FALSE
		movem.l	d2-d7/a2-a6,-(sp)

; --> alloc memory (any)

		move.l	#256*8,d0
		bsr	_alloc_pool
		move.l	d0,RawFont
		beq	.fail
		move.l	#256*8,d0
		bsr	_alloc_pool
		move.l	d0,RawFontInv
		beq	.fail
		move.l	#256*8,d0
		bsr	_alloc_pool
		move.l	d0,RawFontBold
		beq	.fail
		move.l	#256*8,d0
		bsr	_alloc_pool
		move.l	d0,RawFontBoldInv
		beq	.fail
		move.l	#256*8,d0
		bsr	_alloc_pool
		move.l	d0,RawFontUnd
		beq	.fail
		move.l	#256*8,d0
		bsr	_alloc_pool
		move.l	d0,RawFontUndInv
		beq	.fail
		move.l	#bm_SIZEOF,d0
		bsr	_alloc_pool
		move.l	d0,TempBitmap
		beq	.fail
		move.l	#rp_SIZEOF,d0
		bsr	_alloc_pool
		move.l	d0,TempRast
		beq	.fail

		move.l	TempBitmap,a0
		move.l	#bm_SIZEOF-1,d0
.loop1:		clr.b	(a0)+
		dbf	d0,.loop1
		move.l	TempRast,a0
		move.l	#rp_SIZEOF-1,d0
.loop2:		clr.b	(a0)+
		dbf	d0,.loop2

; --> alloc memory (chip)

		move.l	#cop_end-cop_start,d0
		move.l	#MEMF_CHIP,d1
		CALL	Exec,AllocVec
		move.l	d0,Copper
		beq	.fail
		move.l	#sprite_end-sprite_start,d0
		move.l	#MEMF_CHIP,d1
		LIBC	AllocVec
		move.l	d0,Sprite_cursor
		beq	.fail
		move.l	#sprite_empty_end-sprite_empty_start,d0
		move.l	#MEMF_CHIP,d1
		LIBC	AllocVec
		move.l	d0,Sprite_empty
		beq	.fail
		move.l	#256*8,d0
		move.l	#MEMF_CHIP,d1
		LIBC	AllocVec
		move.l	d0,TempText
		beq	.fail
		move.l	#80*8,d0
		move.l	#MEMF_CHIP,d1
		LIBC	AllocVec
		move.l	d0,StatusMap
		beq	.fail

		lea	cop_start,a0
		move.l	Copper,a1
		move.l	#cop_end-cop_start,d0
		LIBC	CopyMem	
		lea	sprite_start,a0
		move.l	Sprite_cursor,a1
		move.l	#sprite_end-sprite_start,d0
		LIBC	CopyMem	
		lea	sprite_empty_start,a0
		move.l	Sprite_empty,a1
		move.l	#sprite_empty_end-sprite_empty_start,d0
		LIBC	CopyMem	

		lea	BlankInt,a1
		moveq	#5,d0
		LIBC	AddIntServer
		st	IntOn
		moveq	#TRUE,d0
		movem.l	(sp)+,d2-d7/a2-a6
		rts
.fail:		bsr	_HRemove
		moveq	#FALSE,d0
		movem.l	(sp)+,d2-d7/a2-a6
		rts

_HRemove:	; OUPUT	D0.L: TRUE/FALSE
		movem.l	d2-d7/a2-a6,-(sp)
		bsr	_HOff
		tst.b	IntOn
		beq	.noint
		lea	BlankInt,a1
		moveq	#5,d0
		CALL	Exec,RemIntServer
.noint:		tst.b	ScrOk
		beq	.noscr
		move.l	ScrData,d0
		beq	.noscr
		move.l	d0,a1
		CALL	Exec,FreeVec
		clr.l	ScrData
.noscr:

; --> free (any)

		move.l	RawFont,a0
		bsr	_free_pool
		move.l	RawFontInv,a0
		bsr	_free_pool
		move.l	RawFontBold,a0
		bsr	_free_pool
		move.l	RawFontBoldInv,a0
		bsr	_free_pool
		move.l	RawFontUnd,a0
		bsr	_free_pool
		move.l	RawFontUndInv,a0
		bsr	_free_pool
		move.l	TempBitmap,a0
		bsr	_free_pool
		move.l	TempRast,a0
		bsr	_free_pool
		clr.l	RawFont
		clr.l	RawFontInv
		clr.l	RawFontBold
		clr.l	RawFontBoldInv
		clr.l	RawFontUnd
		clr.l	RawFontUndInv
		clr.l	TempBitmap
		clr.l	TempRast

; --> free (chip)

		move.l	$4.w,a6
		move.l	Copper,d0
		beq	.no1
		move.l	d0,a1
		LIBC	FreeVec
.no1:		move.l	Sprite_cursor,d0
		beq	.no2
		move.l	d0,a1
		LIBC	FreeVec
.no2:		move.l	Sprite_empty,d0
		beq	.no3
		move.l	d0,a1
		LIBC	FreeVec
.no3:		move.l	TempText,d0
		beq	.no4
		move.l	d0,a1
		LIBC	FreeVec
.no4:		move.l	StatusMap,d0
		beq	.no5
		move.l	d0,a1
		LIBC	FreeVec
.no5:		clr.l	Copper
		clr.l	Sprite_cursor
		clr.l	Sprite_empty
		clr.l	TempText
		clr.l	StatusMap

		moveq	#TRUE,d0
		sf	IntOn
		sf	ScrOk
		movem.l	(sp)+,d2-d7/a2-a6
		rts

_HChange:	; Vaihda resoluutiota yms.
		; INPUT	A0.L: Osoite bbs tageihin
		movem.l	d2-d7/a2-a6,-(sp)
		move.l	a0,-(sp)
		bsr	_HOff
		move.l	(sp)+,a2
		clr.w	_ScrInfo+xpos
		clr.w	_ScrInfo+ypos
		move.w	#80,d0
		move.w	d0,_ScrInfo+columns
		subq.w	#1,d0
		move.w	d0,_ScrInfo+columnsdbf
		CUR_VAL	PB_HLines,d0
		cmp.w	#32,d0
		bls	.maxlim
		moveq	#32,d0
.maxlim:	cmp.w	#24,d0
		bhs	.minlim
		moveq	#24,d0
.minlim:	move.w	d0,_ScrInfo+lines
		subq.w	#1,d0
		move.w	d0,_ScrInfo+linesdbf
		bsr	MakeFont
		tst.w	d0
		beq	.fail
		CALL	Exec,Forbid
		move.l	ScrData,d0
		beq	.noscr
		move.l	d0,a1
		LIBC	FreeVec
		clr.l	ScrData
;		CLRFREE	ScrData
.noscr:		CUR_VAL	PB_Depth,d0
		cmp.w	#4,d0
		bls	.depthlim
		moveq	#4,d0
.depthlim:	move.w	d0,TermDepth
		subq.w	#1,d0
		move.w	d0,TermDBF
		move.w	TermDepth,d0
		mulu.w	#80*8,d0
		move.l	d0,OneRow
		move.w	_ScrInfo+lines,d1
		addq.w	#8,d1
		mulu.w	d1,d0
		add.l	#32,d0
;		MYALLOC	MEMF_CHIP+MEMF_CLEAR
		move.l	#MEMF_CHIP+MEMF_CLEAR,d1
		CALL	Exec,AllocVec
		move.l	d0,ScrData
		beq	.fail
		addq.l	#8,d0
		and.l	#$fffffff8,d0		; 8 byte boundarie / required for burst
		move.l	d0,TermPoint
		move.l	d0,TermOneMap
		move.l	d0,TermTwoMap
		move.w	_ScrInfo+lines,TermOne
		addq.w	#8,TermOne
		clr.w	TermTwo
;		lea	SC,a3
		move.l	Copper,a3
		move.w	ScreenOffset,d0		; Fix UpBitmap place
		subq.w	#1,d0
		lsl.w	#3,d0
		lsl.w	#8,d0
		addq.w	#1,d0
		move.w	d0,UpScrPlace-SC(a3)
		move.l	TermTwoMap,d0		; Fix DnBitmap
		moveq	#80,d1
		moveq	#4-1,d2
;		lea	DnBitmap-SC+6(a3),a0
		move.l	a3,a0
		add.l	#DnBitmap-SC+6,a0
.loop4:		move.w	d0,(a0)
		swap	d0
		move.w	d0,-4(a0)
		swap	d0
		add.l	d1,d0
		addq.w	#8,a0
		dbf	d2,.loop4
;		move.l	#StatusMap,d0		; Status
		move.l	StatusMap,d0
		move.w	d0,StatusBitmap-SC+6(a3)
		swap	d0
		move.w	d0,StatusBitmap-SC+2(a3)
		move.w	#$01a0,d1		; AGA burst?
		move.w	#$003c,DataFetchStart-SC+2(a3)
		move.w	#$00d4,DataFetchStop-SC+2(a3)
		CUR_VAL	PB_Aga,d0
		tst.l	d0
		beq	.noaga
		move.w	#$01fc,d1
		move.w	#$0038,DataFetchStart-SC+2(a3)	; *** ? ***
		move.w	#$00d0,DataFetchStop-SC+2(a3)
.noaga:		move.w	d1,AGABurst-SC(a3)
;		move.l	#SpriteData,d0		; Sprite
		move.l	Sprite_cursor,d0
		move.w	d0,Sprite-SC+6(a3)
		swap	d0
		move.w	d0,Sprite-SC+2(a3)
;		move.l	#EmptySprite,d0		; Sprites 1-7
		move.l	Sprite_empty,d0
		move.w	d0,Sprite1-SC+6(a3)
		move.w	d0,Sprite2-SC+6(a3)
		move.w	d0,Sprite3-SC+6(a3)
		move.w	d0,Sprite4-SC+6(a3)
		move.w	d0,Sprite5-SC+6(a3)
		move.w	d0,Sprite6-SC+6(a3)
		move.w	d0,Sprite7-SC+6(a3)
		swap	d0
		move.w	d0,Sprite1-SC+2(a3)
		move.w	d0,Sprite2-SC+2(a3)
		move.w	d0,Sprite3-SC+2(a3)
		move.w	d0,Sprite4-SC+2(a3)
		move.w	d0,Sprite5-SC+2(a3)
		move.w	d0,Sprite6-SC+2(a3)
		move.w	d0,Sprite7-SC+2(a3)
		move.w	ScreenOffset,d0		; Screen dimension
		move.w	d0,d1
		lsl.w	#8,d0
		or.w	#$0081,d0
		move.w	d0,DiwStart-SC+2(a3)
		move.w	_ScrInfo+lines,d0
		lsl.w	#3,d0
		add.w	d0,d1
		add.w	#9,d1			; Statuslineä varten
		lsl.w	#8,d1
		or.w	#$00c1,d1
		move.w	d1,DiwStop-SC+2(a3)
		move.w	TermDBF,d0		; Modules
		mulu.w	#80,d0
		move.w	d0,ModOdd-SC+2(a3)
		move.w	d0,ModEven-SC+2(a3)
		move.w	TermDepth,d0		; Plane control -> depth
		ror.w	#4,d0
		or.w	#$8300,d0
		move.w	d0,Control-SC+2(a3)
		move.w	_ScrInfo+lines,d0	; Status screen place
		lsl.w	#3,d0
		add.w	ScreenOffset,d0
		cmp.w	#$100,d0
		bhs	.pal
		move.l	#$01a00000,StatePAL-SC(a3)	; Interrupt will handle PALSKIP
.pal:		lsl.w	#8,d0
		or.w	#$0001,d0
		move.w	d0,StatePlace-SC(a3)
		add.w	#$0100,d0
		move.w	d0,StatePlace2-SC(a3)
		CALL	Exec,Permit
		st	ScrOk
		moveq	#TRUE,d0
		movem.l	(sp)+,d2-d7/a2-a6
		rts
.fail:		moveq	#FALSE,d0
		movem.l	(sp)+,d2-d7/a2-a6
		rts

MakeFont:	; OUPUT	D0.L: TRUE/FALSE
		movem.l	d2-d7/a2-a6,-(sp)
		move.l	_GfxBase,a6
;		lea	TempBitmap,a0
		move.l	TempBitmap,a0
		moveq	#1,d0
		move.l	#8*256,d1
		move.l	#8,d2
		LIBC	InitBitMap
;		move.l	#TempText,TempBitmap+bm_Planes
		move.l	TempBitmap,a0
		move.l	TempText,bm_Planes(a0)
;		lea	TempRast,a1
		move.l	TempRast,a1
		LIBC	InitRastPort
;		move.l	#TempBitmap,TempRast+rp_BitMap	; RastPort inited
		move.l	TempRast,a0
		move.l	TempBitmap,rp_BitMap(a0)	; RastPort inited
;		lea	TempRast,a0
		bsr	_TempPrint
		tst.w	d0
		beq	.nogo
;		lea	TempText,a0
;		lea	RawFont,a1
;		lea	RawFontInv,a2
;		lea	RawFontBold,a3
;		lea	RawFontBoldInv,a4
;		lea	RawFontUnd,a5
;		lea	RawFontUndInv,a6
		move.l	TempText,a0
		move.l	RawFont,a1
		move.l	RawFontInv,a2
		move.l	RawFontBold,a3
		move.l	RawFontUnd,a5
		move.l	RawFontUndInv,a6
		move.l	a4,-(sp)
		move.l	RawFontBoldInv,a4	; ! Last A4 reference here !
		move.w	#256-1,d0
.loop:		moveq	#8-1,d1
.loop2:		move.b	(a0),d2
		move.b	d2,(a1)+		; RawFont
		move.b	d2,d3
		not.b	d3
		move.b	d3,(a2)+		; RawFontInv
		move.b	d2,d3
		lsr.b	#1,d3
		or.b	d2,d3
		move.b	d3,(a3)+		; RawFontBold
		not.b	d3
		move.b	d3,(a4)+		; RawFontBoldInv
		move.b	d2,(a5)+		; RawFontUnd
		not.b	d2
		move.b	d2,(a6)+		; RawFontUndInv
		add.w	#256,a0
		dbf	d1,.loop2
		sub.w	#[256*8]-1,a0
		dbf	d0,.loop
;		lea	RawFontUnd+7,a5
;		lea	RawFontUndInv+7,a6
		move.l	(sp)+,a4
		move.l	RawFontUnd,a5
		move.l	RawFontUndInv,a6
		add.l	#7,a5
		add.l	#7,a6
		moveq	#-1,d1
		move.w	#256-1,d0
.loop3:		move.b	d1,(a5)
		clr.b	(a6)
		addq.w	#8,a5
		addq.w	#8,a6
		dbf	d0,.loop3
		moveq	#TRUE,d0
		movem.l	(sp)+,d2-d7/a2-a6
		rts
.nogo:		moveq	#FALSE,d0
		movem.l	(sp)+,d2-d7/a2-a6
		rts

_HOn:		; OUPUT	D0.L: TRUE/FALSE
		movem.l	d2-d7/a2-a6,-(sp)
		tst.b	ScreenState
		bne	.on
		tst.b	ScrOk
		beq	.on

		move.l	_GfxBase,a6
		move.l	_gl_conscreen,d7
		move.l	#VTAG_SPRITERESN_GET,taglist
		move.l	d7,a0
		move.l	sc_ViewPort+vp_ColorMap(a0),a0
		lea	taglist,a1
		LIBC	VideoControl
		move.l	resolution,oldres
		move.l	#SPRITERESN_140NS,resolution
		move.l	#VTAG_SPRITERESN_SET,taglist
		move.l	d7,a0
		move.l	sc_ViewPort+vp_ColorMap(a0),a0
		lea	taglist,a1
		LIBC	VideoControl
		move.l	d7,a0
		move.l	_IntuitionBase,a6
		LIBC	MakeScreen
		LIBC	RethinkDisplay

		move.l	_GfxBase,a6
		move.l	gb_ActiView(a6),SaveView
		sub.l	a1,a1
		LIBC	LoadView
		LIBC	WaitTOF
		LIBC	WaitTOF
;		move.l	#SC,$dff080
		move.l	Copper,$dff080
		move.w	#$81a0,$dff096
		st	ScreenState

.on:		moveq	#TRUE,d0
		movem.l	(sp)+,d2-d7/a2-a6
		rts

_HOff:		; Screeni pois
		; OUPUT	D0.L: TRUE/FALSE
		movem.l	d2-d7/a2-a6,-(sp)
		tst.b	ScreenState
		beq	.off
		tst.b	ScrOk
		beq	.off

		move.l	_GfxBase,a6
		move.l	_gl_conscreen,a0
		move.l	oldres,resolution
		move.l	#VTAG_SPRITERESN_SET,taglist
		lea	taglist,a1
		move.l	sc_ViewPort+vp_ColorMap(a0),a0
		jsr	_LVOVideoControl(a6)
		move.l	_IntuitionBase,a6
		move.l	_gl_conscreen,a0
		LIBC	MakeScreen

		move.l	_GfxBase,a6
		move.l	SaveView,a1
		LIBC	LoadView
		LIBC	WaitTOF
		LIBC	WaitTOF
		move.l	gb_copinit(a6),$dff080
		sf	ScreenState

.off:		moveq	#TRUE,d0
		movem.l	(sp)+,d2-d7/a2-a6
		rts

; --> Ruudun käsittely rutiinit

_HPrint:	; Merkkit näyttöön
		; INPUT	A0.L: Missä
		;	D0.W: Paljon
		move.w	d0,d3
		beq	stop
		subq.w	#1,d3
		move.l	TermPoint,a3
		add.w	_ScrInfo+xpos,a3
		move.w	_ScrInfo+ypos,d0
		bsr	CalcYmap
		add.l	d0,a3			; bitmap
		move.w	_ScrInfo+fontstate,d1
		beq	norm
		btst	#1,d1
		bne	bold
		btst	#0,d1
		beq	norm
;		lea	RawFontUnd,a5
;		lea	RawFontUndInv,a6
		move.l	RawFontUnd,a5
		move.l	RawFontUndInv,a6
		bra	go
bold:	;	lea	RawFontBold,a5
	;	lea	RawFontInv,a6
		move.l	RawFontBold,a5
		move.l	RawFontInv,a6
		bra	go
norm:	;	lea	RawFont,a5
	;	lea	RawFontInv,a6
		move.l	RawFont,a5
		move.l	RawFontInv,a6
go:		tst.b	_ScrInfo+inverse
		beq	noinv
		exg	a5,a6
noinv:		move.w	d2,d0
		subq.w	#1,d0
		moveq	#-1,d4
		moveq	#80,d6
more:		moveq	#0,d1
		move.b	(a0)+,d1
		lsl.w	#3,d1
		lea	(a5,d1.w),a1
		lea	(a6,d1.w),a2
		move.w	TermDBF,d7
		move.w	_ScrInfo+forecol,d1
		move.w	_ScrInfo+backcol,d2
		cmp.w	#3,d7
		beq.w	CharPlane4
		cmp.w	#2,d7
		beq.w	CharPlane3
		cmp.w	#1,d7
		beq.w	CharPlane2
; --> 1 bitplane
		lsr.w	#1,d1
		bcs	ForeWrite1
		lsr.w	#1,d2
		bcs	BackWrite1
		clr.b	80*0(a3)		; Tyhjennä bitplane
		clr.b	80*1(a3)
		clr.b	80*2(a3)
		clr.b	80*3(a3)
		clr.b	80*4(a3)
		clr.b	80*5(a3)
		clr.b	80*6(a3)
		clr.b	80*7(a3)
		bra	NextPlace
BackWrite1:
		move.b	(a2)+,80*0(a3)		; Taustan image bitplaneen
		move.b	(a2)+,80*1(a3)
		move.b	(a2)+,80*2(a3)
		move.b	(a2)+,80*3(a3)
		move.b	(a2)+,80*4(a3)
		move.b	(a2)+,80*5(a3)
		move.b	(a2)+,80*6(a3)
		move.b	(a2)+,80*7(a3)
		bra	NextPlace
ForeWrite1:
		lsr.w	#1,d2
		bcs	FillPlane1
		move.b	(a1)+,80*0(a3)		; Fontin image bitplaneen
		move.b	(a1)+,80*1(a3)
		move.b	(a1)+,80*2(a3)
		move.b	(a1)+,80*3(a3)
		move.b	(a1)+,80*4(a3)
		move.b	(a1)+,80*5(a3)
		move.b	(a1)+,80*6(a3)
		move.b	(a1)+,80*7(a3)
		bra	NextPlace
FillPlane1:
		move.b	d4,80*0(a3)		; Fillaa plane
		move.b	d4,80*1(a3)
		move.b	d4,80*2(a3)
		move.b	d4,80*3(a3)
		move.b	d4,80*4(a3)
		move.b	d4,80*5(a3)
		move.b	d4,80*6(a3)
		move.b	d4,80*7(a3)
		bra	NextPlace
; --> 2 bitplanea
CharPlane2:
		lsr.w	#1,d1
		bcs	ForeWrite2
		lsr.w	#1,d2
		bcs	BackWrite2
		clr.b	80*0*2(a3)		; Tyhjennä bitplane
		clr.b	80*1*2(a3)
		clr.b	80*2*2(a3)
		clr.b	80*3*2(a3)
		clr.b	80*4*2(a3)
		clr.b	80*5*2(a3)
		clr.b	80*6*2(a3)
		clr.b	80*7*2(a3)
		add.w	d6,a3
		dbf	d7,CharPlane2
		sub.w	#80*2,a3
		bra	NextPlace
BackWrite2:
		move.b	(a2)+,80*0*2(a3)	; Taustan image bitplaneen
		move.b	(a2)+,80*1*2(a3)
		move.b	(a2)+,80*2*2(a3)
		move.b	(a2)+,80*3*2(a3)
		move.b	(a2)+,80*4*2(a3)
		move.b	(a2)+,80*5*2(a3)
		move.b	(a2)+,80*6*2(a3)
		move.b	(a2)+,80*7*2(a3)
		subq.w	#8,a2
		add.w	d6,a3
		dbf	d7,CharPlane2
		sub.w	#80*2,a3
		bra	NextPlace
ForeWrite2:
		lsr.w	#1,d2
		bcs	FillPlane2
		move.b	(a1)+,80*0*2(a3)	; Fontin image bitplaneen
		move.b	(a1)+,80*1*2(a3)
		move.b	(a1)+,80*2*2(a3)
		move.b	(a1)+,80*3*2(a3)
		move.b	(a1)+,80*4*2(a3)
		move.b	(a1)+,80*5*2(a3)
		move.b	(a1)+,80*6*2(a3)
		move.b	(a1)+,80*7*2(a3)
		subq.w	#8,a1
		add.w	d6,a3
		dbf	d7,CharPlane2
		sub.w	#80*2,a3
		bra	NextPlace
FillPlane2:
		move.b	d4,80*0*2(a3)		; Fillaa plane
		move.b	d4,80*1*2(a3)
		move.b	d4,80*2*2(a3)
		move.b	d4,80*3*2(a3)
		move.b	d4,80*4*2(a3)
		move.b	d4,80*5*2(a3)
		move.b	d4,80*6*2(a3)
		move.b	d4,80*7*2(a3)
		add.w	d6,a3
		dbf	d7,CharPlane2
		sub.w	#80*2,a3
		bra	NextPlace
; --> 3 bitplanea
CharPlane3:
		lsr.w	#1,d1
		bcs	ForeWrite3
		lsr.w	#1,d2
		bcs	BackWrite3
		clr.b	80*0*3(a3)		; Tyhjennä bitplane
		clr.b	80*1*3(a3)
		clr.b	80*2*3(a3)
		clr.b	80*3*3(a3)
		clr.b	80*4*3(a3)
		clr.b	80*5*3(a3)
		clr.b	80*6*3(a3)
		clr.b	80*7*3(a3)
		add.w	d6,a3
		dbf	d7,CharPlane3
		sub.w	#80*3,a3
		bra	NextPlace
BackWrite3:
		move.b	(a2)+,80*0*3(a3)	; Taustan image bitplaneen
		move.b	(a2)+,80*1*3(a3)
		move.b	(a2)+,80*2*3(a3)
		move.b	(a2)+,80*3*3(a3)
		move.b	(a2)+,80*4*3(a3)
		move.b	(a2)+,80*5*3(a3)
		move.b	(a2)+,80*6*3(a3)
		move.b	(a2)+,80*7*3(a3)
		subq.w	#8,a2
		add.w	d6,a3
		dbf	d7,CharPlane3
		sub.w	#80*3,a3
		bra	NextPlace
ForeWrite3:
		lsr.w	#1,d2
		bcs	FillPlane3
		move.b	(a1)+,80*0*3(a3)	; Fontin image bitplaneen
		move.b	(a1)+,80*1*3(a3)
		move.b	(a1)+,80*2*3(a3)
		move.b	(a1)+,80*3*3(a3)
		move.b	(a1)+,80*4*3(a3)
		move.b	(a1)+,80*5*3(a3)
		move.b	(a1)+,80*6*3(a3)
		move.b	(a1)+,80*7*3(a3)
		subq.w	#8,a1
		add.w	d6,a3
		dbf	d7,CharPlane3
		sub.w	#80*3,a3
		bra	NextPlace
FillPlane3:
		move.b	d4,80*0*3(a3)		; Fillaa plane
		move.b	d4,80*1*3(a3)
		move.b	d4,80*2*3(a3)
		move.b	d4,80*3*3(a3)
		move.b	d4,80*4*3(a3)
		move.b	d4,80*5*3(a3)
		move.b	d4,80*6*3(a3)
		move.b	d4,80*7*3(a3)
		add.w	d6,a3
		dbf	d7,CharPlane3
		sub.w	#80*3,a3
		bra	NextPlace
; --> 4 bitplanea
CharPlane4:
		lsr.w	#1,d1
		bcs	ForeWrite4
		lsr.w	#1,d2
		bcs	BackWrite4
		clr.b	80*0*4(a3)		; Tyhjennä bitplane
		clr.b	80*1*4(a3)
		clr.b	80*2*4(a3)
		clr.b	80*3*4(a3)
		clr.b	80*4*4(a3)
		clr.b	80*5*4(a3)
		clr.b	80*6*4(a3)
		clr.b	80*7*4(a3)
		add.w	d6,a3
		dbf	d7,CharPlane4
		sub.w	#80*4,a3
		bra	NextPlace
BackWrite4:
		move.b	(a2)+,80*0*4(a3)	; Taustan image bitplaneen
		move.b	(a2)+,80*1*4(a3)
		move.b	(a2)+,80*2*4(a3)
		move.b	(a2)+,80*3*4(a3)
		move.b	(a2)+,80*4*4(a3)
		move.b	(a2)+,80*5*4(a3)
		move.b	(a2)+,80*6*4(a3)
		move.b	(a2)+,80*7*4(a3)
		subq.w	#8,a2
		add.w	d6,a3
		dbf	d7,CharPlane4
		sub.w	#80*4,a3
		bra	NextPlace
ForeWrite4:
		lsr.w	#1,d2
		bcs	FillPlane4
		move.b	(a1)+,80*0*4(a3)	; Fontin image bitplaneen
		move.b	(a1)+,80*1*4(a3)
		move.b	(a1)+,80*2*4(a3)
		move.b	(a1)+,80*3*4(a3)
		move.b	(a1)+,80*4*4(a3)
		move.b	(a1)+,80*5*4(a3)
		move.b	(a1)+,80*6*4(a3)
		move.b	(a1)+,80*7*4(a3)
		subq.w	#8,a1
		add.w	d6,a3
		dbf	d7,CharPlane4
		sub.w	#80*4,a3
		bra	NextPlace
FillPlane4:
		move.b	d4,80*0*4(a3)		; Fillaa plane
		move.b	d4,80*1*4(a3)
		move.b	d4,80*2*4(a3)
		move.b	d4,80*3*4(a3)
		move.b	d4,80*4*4(a3)
		move.b	d4,80*5*4(a3)
		move.b	d4,80*6*4(a3)
		move.b	d4,80*7*4(a3)
		add.w	d6,a3
		dbf	d7,CharPlane4
		sub.w	#80*4,a3
NextPlace:
		addq.w	#1,a3
		dbf	d3,more
stop:		rts

_HSPrint:	; Printtaa statusriville
		; INPUT	A0.L: Missä
		movem.l	a2-a3,-(sp)
;		lea	StatusMap,a1
;		lea	RawFont,a2
		move.l	StatusMap,a1
		move.l	RawFont,a2
		move.w	#80-1,d1
.more:		moveq	#0,d0
		move.b	(a0)+,d0
		lsl.w	#3,d0
		lea	(a2,d0.w),a3
		move.b	(a3)+,80*0(a1)		; Fontin image bitplaneen
		move.b	(a3)+,80*1(a1)
		move.b	(a3)+,80*2(a1)
		move.b	(a3)+,80*3(a1)
		move.b	(a3)+,80*4(a1)
		move.b	(a3)+,80*5(a1)
		move.b	(a3)+,80*6(a1)
		move.b	(a3)+,80*7(a1)
		addq.w	#1,a1
		dbf	d1,.more
		movem.l	(sp)+,a2-a3
		rts

_HDown:		; Siirrä ruutua yksi line alas
		lea	$dff000,a5
		move.w	#$4000,$9a(a5)
		subq.w	#1,TermOne
		beq	top
		move.l	OneRow,d0
		add.l	d0,TermOneMap
		addq.w	#1,TermTwo
		bra	clrbot
top:		move.l	TermPoint,d0
		move.l	d0,TermOneMap
		move.w	_ScrInfo+lines,TermOne
		addq.w	#8,TermOne
		clr.w	TermTwo
clrbot:		move.w	#$c000,$9a(a5)

		move.w	TermOne,d0
		cmp.w	_ScrInfo+lines,d0
		bhs	.first
		move.w	_ScrInfo+lines,d0
		sub.w	TermOne,d0
		mulu.w	OneRow+2,d0
		add.l	TermTwoMap,d0
		bra	.last
.first:		move.w	_ScrInfo+lines,d0
		mulu.w	OneRow+2,d0
		add.l	TermOneMap,d0
.last:
		move.l	a4,-(sp)
		move.l	d0,a0
		move.w	TermDBF,d0
		moveq	#0,d1
		moveq	#0,d2
		moveq	#0,d3
		moveq	#0,d4
		moveq	#0,d5
		moveq	#0,d6
		moveq	#0,d7
		sub.l	a1,a1
		sub.l	a2,a2
		sub.l	a3,a3
		sub.l	a4,a4
		sub.l	a5,a5
		sub.l	a6,a6
.loop:		movem.l	d1-d7/a1-a6,-(a0)	; clear 640 bytes at once
		movem.l	d1-d7/a1-a6,-(a0)
		movem.l	d1-d7/a1-a6,-(a0)
		movem.l	d1-d7/a1-a6,-(a0)
		movem.l	d1-d7/a1-a6,-(a0)
		movem.l	d1-d7/a1-a6,-(a0)
		movem.l	d1-d7/a1-a6,-(a0)
		movem.l	d1-d7/a1-a6,-(a0)
		movem.l	d1-d7/a1-a6,-(a0)
		movem.l	d1-d7/a1-a6,-(a0)
		movem.l	d1-d7/a1-a6,-(a0)
		movem.l	d1-d7/a1-a6,-(a0)
		movem.l	d1/d2/d3/d4,-(a0)
		dbf	d0,.loop
		move.l	(sp)+,a4
		rts

_HUp:		; Siirrä ruutua yksi line ylös
		lea	$dff000,a5
		move.w	#$4000,$9a(a5)
		addq.w	#1,TermOne
		move.w	TermOne,d0
		move.w	_ScrInfo+lines,d1
		addq.w	#8,d1
		cmp.w	d1,d0
		bhi	top2
		move.l	OneRow,d0
		sub.l	d0,TermOneMap
		subq.w	#1,TermTwo
		bra	clrtop
top2:		move.l	TermPoint,d0
		move.w	_ScrInfo+linesdbf,d1
		addq.w	#8,d1
		mulu.w	OneRow+2,d1
		add.l	d1,d0
		move.l	d0,TermOneMap
		move.w	#1,TermOne
		move.w	_ScrInfo+linesdbf,TermTwo
		addq.w	#8,TermTwo
clrtop:		move.w	#$c000,$9a(a5)

		move.l	a4,-(sp)
		move.l	TermOneMap,a0
		add.l	OneRow,a0
		move.w	TermDBF,d0
		moveq	#0,d1
		moveq	#0,d2
		moveq	#0,d3
		moveq	#0,d4
		moveq	#0,d5
		moveq	#0,d6
		moveq	#0,d7
		sub.l	a1,a1
		sub.l	a2,a2
		sub.l	a3,a3
		sub.l	a4,a4
		sub.l	a5,a5
		sub.l	a6,a6
.loop:		movem.l	d1-d7/a1-a6,-(a0)	; clear 640 bytes at once
		movem.l	d1-d7/a1-a6,-(a0)
		movem.l	d1-d7/a1-a6,-(a0)
		movem.l	d1-d7/a1-a6,-(a0)
		movem.l	d1-d7/a1-a6,-(a0)
		movem.l	d1-d7/a1-a6,-(a0)
		movem.l	d1-d7/a1-a6,-(a0)
		movem.l	d1-d7/a1-a6,-(a0)
		movem.l	d1-d7/a1-a6,-(a0)
		movem.l	d1-d7/a1-a6,-(a0)
		movem.l	d1-d7/a1-a6,-(a0)
		movem.l	d1-d7/a1-a6,-(a0)
		movem.l	d1/d2/d3/d4,-(a0)
		dbf	d0,.loop
		move.l	(sp)+,a4
		rts

_HClearInLine:	; Poista merkkejä (Current) rivistä
		; INPUT	D0.W: Monta merkkiä
		;	D1.W: x-kohta
		MYBLIT
		lea	$dff000,a5
		moveq	#0,d5
		moveq	#0,d6
		moveq	#0,d7
		lsr.w	#1,d1			; Kohta
		bcc	SyncWord
		addq.w	#1,d1
		moveq	#-1,d6			; Ensimmäinen byte prossulla
		subq.w	#1,d0
SyncWord:
		move.l	TermPoint,a1
		move.l	d0,-(sp)
		move.w	_ScrInfo+ypos,d0
		bsr	CalcYmap
		add.l	d0,a1
		move.l	(sp)+,d0

		add.w	d1,a1
		add.w	d1,a1

		lsr.w	#1,d0			; Monta wordia
		bcc	SyncWord2
		moveq	#-1,d7			; Viimeinen byte prossulla
SyncWord2:
		tst.w	d0
		beq	NoBlitter
		moveq	#80,d2
		sub.w	d0,d2
		sub.w	d0,d2			; Modulo
		move.l	a1,$54(a5)
		move.l	#$01000000,$40(a5)
		move.w	d2,$66(a5)
		move.w	TermDepth,d3
		lsl.w	#3,d3
		lsl.w	#6,d3
		or.w	d0,d3
		move.w	d3,$58(a5)
NoBlitter:
		tst.w	d6
		beq	nfirst
		move.l	a1,a0
		subq.w	#1,a0			; Tyhjennä ensimmäinen merkki
		move.w	TermDepth,d4
		subq.w	#1,d4
.loop1:		clr.b	80*0(a0)
		clr.b	80*1(a0)
		clr.b	80*2(a0)
		clr.b	80*3(a0)
		clr.b	80*4(a0)
		clr.b	80*5(a0)
		clr.b	80*6(a0)
		clr.b	80*7(a0)
		add.w	#80*8,a0
		dbf	d4,.loop1
nfirst:		tst.w	d7
		beq	nolast
		add.w	d0,a1
		add.w	d0,a1
		move.w	TermDepth,d4
		subq.w	#1,d4
.loop2:		clr.b	80*0(a1)
		clr.b	80*1(a1)
		clr.b	80*2(a1)
		clr.b	80*3(a1)
		clr.b	80*4(a1)
		clr.b	80*5(a1)
		clr.b	80*6(a1)
		clr.b	80*7(a1)
		add.w	#80*8,a1
		dbf	d4,.loop2
nolast:		ENDBLIT
		rts

_HCopyInLine:	; INPUT	D0.W: Monta merkkiä
		;	D1.W: x-kohta (0-79) source
		;	D2.W: x-kohta (0-79) target
		cmp.w	d1,d2
		bhi	fromend
		move.w	d0,-(sp)		; from start
		move.l	TermPoint,a0
		move.w	_ScrInfo+ypos,d0
		bsr	CalcYmap
		add.l	d0,a0
		move.w	(sp)+,d0
		move.l	a0,a1
		move.l	a0,a2
		add.w	d1,a0
		add.w	d2,a1			; Minne
		move.w	TermDBF,d5
		move.w	#80*8,d6
		move.w	d0,d4
		subq.w	#1,d4
Loop93:		move.l	a0,a5
		move.l	a1,a6
		move.w	d5,d3
Loop92:		move.b	80*0(a5),80*0(a6)
		move.b	80*1(a5),80*1(a6)
		move.b	80*2(a5),80*2(a6)
		move.b	80*3(a5),80*3(a6)
		move.b	80*4(a5),80*4(a6)
		move.b	80*5(a5),80*5(a6)
		move.b	80*6(a5),80*6(a6)
		move.b	80*7(a5),80*7(a6)
		add.w	d6,a5
		add.w	d6,a6
		dbf	d3,Loop92
		addq.w	#1,a0
		addq.w	#1,a1
		dbf	d4,Loop93
		rts
fromend		move.w	d0,-(sp)
		move.l	TermPoint,a0
		move.w	_ScrInfo+ypos,d0
		bsr	CalcYmap
		add.l	d0,a0
		move.w	(sp)+,d0
		move.l	a0,a1
		move.l	a0,a2
		add.w	d1,a0
		add.w	d2,a1			; Minne
		add.w	d0,a0
		add.w	d0,a1
		subq.w	#1,a0
		subq.w	#1,a1
		move.w	TermDBF,d5
		move.w	#80*8,d6
		move.w	d0,d4
		subq.w	#1,d4
Loop91:		move.l	a0,a5
		move.l	a1,a6
		move.w	d5,d3
Loop90:		move.b	80*0(a5),80*0(a6)
		move.b	80*1(a5),80*1(a6)
		move.b	80*2(a5),80*2(a6)
		move.b	80*3(a5),80*3(a6)
		move.b	80*4(a5),80*4(a6)
		move.b	80*5(a5),80*5(a6)
		move.b	80*6(a5),80*6(a6)
		move.b	80*7(a5),80*7(a6)
		add.w	d6,a5
		add.w	d6,a6
		dbf	d3,Loop90
		subq.w	#1,a0
		subq.w	#1,a1
		dbf	d4,Loop91
		rts

_HAreaClear:	; Tyhjennä rivejä
		; INPUT	D0.W: Mistä rivistä lähtien (0-x)
		;	D1.W: Monta riviä
		MYBLIT
		lea	$dff000,a5
		move.w	#0,$66(a5)		; Modulo
		move.l	#$01000000,$40(a5)
		move.w	TermOne,d2
		sub.w	d0,d2
		beq	botonly
		bmi	botonly
		move.w	d1,d3
		sub.w	d2,d1
		bmi	LineEnough
		beq	LineEnough
		move.w	d2,d3
LineEnough:
		move.l	TermOneMap,d2
		move.w	d0,d4
		add.w	d3,d0
		mulu.w	OneRow+2,d4
		add.l	d4,d2
		move.l	d2,$54(a5)
		mulu.w	OneRow+2,d3
		divu.w	#80,d3
		lsl.w	#6,d3
		or.w	#80/2,d3
		move.w	d3,$58(a5)
botonly		tst.w	d1
		beq	lok
		bmi	lok
		move.l	TermTwoMap,d2
		sub.w	TermOne,d0
		mulu.w	OneRow+2,d0
		add.l	d0,d2
		mulu.w	OneRow+2,d1
		divu.w	#80,d1
		lsl.w	#6,d1
		or.w	#80/2,d1
		WAITBLIT
		move.l	d2,$54(a5)
		move.w	d1,$58(a5)
lok:		ENDBLIT
		rts

_HAreaCopy:	; Kopioi rivejä
		; INPUT	D0.W: Mistä rivistä lähtien (0-x)
		;	D1.W: Monta riviä
		;	D2.W: Mille riville (0-x)
		movem.l	d0-d2,-(sp)
		tst.l	_gl_cur_qv+[[PB_RtsBlit-VAL_PB_FIRST]*4]
		beq	nohalt
		moveq	#FALSE,d0
		jsr	_SetRTS
nohalt:		movem.l	(sp)+,d0-d2
		MYBLIT
		lea	$dff000,a5
		move.l	#$ffffffff,$44(a5)
		move.w	#0,$64(a5)
		move.w	#0,$66(a5)
		cmp.w	d0,d2
		bhs.w	UpToDown

		move.l	#$09f00000,$40(a5)
		move.w	TermOne,d3
		sub.w	d0,d3			; Monta riviä 1. osalla
		beq	PosClip
		bmi	PosClip
		move.w	d1,d4			; Monta riviä käsitellään
		sub.w	d3,d1			; Paljon jää jäljelle
		bmi	LineEnough2
		beq	LineEnough2
		move.w	d3,d4			; Kaikki rivit 1. alueella
LineEnough2:
		move.l	TermOneMap,d5
		move.l	d5,d6
		move.w	d0,d7
		mulu.w	OneRow+2,d7
		add.l	d7,d5
		move.w	d2,d7
		mulu.w	OneRow+2,d7
		add.l	d7,d6
		add.w	d4,d0
		add.w	d4,d2
		mulu.w	OneRow+2,d4
		divu.w	#80,d4
		lsl.w	#6,d4
		or.w	#80/2,d4
		move.l	d5,$50(a5)		; Mistä
		move.l	d6,$54(a5)		; Minne
		move.w	d4,$58(a5)

PosClip:	; Blittaa alueelta 2->1
		tst.w	d1
		beq.w	ACok
		bmi.w	ACok
		move.w	TermOne,d3
		sub.w	d2,d3			; Monta riviä 1. osalla
		beq	LastClip
		bmi	LastClip
		move.w	d1,d4			; Monta riviä käsitellään
		sub.w	d3,d1			; Paljon jää jäljelle
		bmi	LineEnough3
		beq	LineEnough3
		move.w	d3,d4			; Kaikki rivit 1. alueella
LineEnough3:
		move.l	TermTwoMap,d5
		move.l	TermOneMap,d6
		move.w	d0,d7
		sub.w	TermOne,d7
		mulu.w	OneRow+2,d7
		add.l	d7,d6
		move.w	d2,d7
		mulu.w	OneRow+2,d7
		add.l	d7,d6
		add.w	d4,d0
		add.w	d4,d2
		mulu.w	OneRow+2,d4
		divu.w	#80,d4
		lsl.w	#6,d4
		or.w	#80/2,d4
		WAITBLIT
		move.l	d5,$50(a5)		; Mistä
		move.l	d6,$54(a5)		; Minne
		move.w	d4,$58(a5)

LastClip:
		tst.w	d1
		beq.w	ACok
		bmi.w	ACok
		sub.w	TermOne,d0
		sub.w	TermOne,d2
		move.l	TermTwoMap,d5
		move.l	d5,d6
		mulu.w	OneRow+2,d0
		add.l	d0,d5
		mulu.w	OneRow+2,d2
		add.l	d2,d6
		mulu.w	OneRow+2,d1
		divu.w	#80,d1
		lsl.w	#6,d1
		or.w	#80/2,d1
		WAITBLIT
		move.l	d5,$50(a5)
		move.l	d6,$54(a5)
		move.w	d1,$58(a5)
		bra	ACok
UpToDown:
		; INPUT	D0.W: Mistä rivistä lähtien (0-x)
		;	D1.W: Monta riviä
		;	D2.W: Mille riville (0-x)
		add.w	d1,d0
		add.w	d1,d2
		move.l	#$09f00002,$40(a5)
		move.w	d0,d3
		sub.w	TermOne,d3		; Monta kopioitavaa riviä 2. osassa
		beq	PosClip2
		bmi	PosClip2
		move.w	d1,d4
		sub.w	d3,d1
		bmi	LineEnough4
		beq	LineEnough4
		move.w	d3,d4			; Kaikki rivit 2. alueella
LineEnough4:
		move.l	TermTwoMap,d5
		subq.l	#2,d5			; Coz descending blit
		move.l	d5,d6
		move.w	d0,d7
		sub.w	TermOne,d7
		mulu.w	OneRow+2,d7
		add.l	d7,d5
		move.w	d2,d7
		sub.w	TermOne,d7
		mulu.w	OneRow+2,d7
		add.l	d7,d6
		sub.w	d4,d0
		sub.w	d4,d2
		mulu.w	OneRow+2,d4
		divu.w	#80,d4
		lsl.w	#6,d4
		or.w	#80/2,d4
		move.l	d5,$50(a5)		; Mistä
		move.l	d6,$54(a5)		; Minne
		move.w	d4,$58(a5)

PosClip2:	; Blittaa alueelta 1->2
		tst.w	d1
		beq.w	ACok
		bmi.w	ACok
		move.w	d2,d3
		sub.w	TermOne,d3		; Monta riviä 2. osassa jäljellä
		beq	LastClip2
		bmi	LastClip2
		move.w	d1,d4
		sub.w	d3,d1
		bmi	LineEnough5
		beq	LineEnough5
		move.w	d3,d4			; Kaikki rivit 2. alueella
LineEnough5:
		move.l	TermOneMap,d5
		move.l	TermTwoMap,d6
		subq.l	#2,d5			; Coz descending blit
		subq.l	#2,d6
		move.w	d0,d7
		mulu.w	OneRow+2,d7
		add.l	d7,d5
		move.w	d2,d7
		sub.w	TermOne,d7
		mulu.w	OneRow+2,d7
		add.l	d7,d6
		sub.w	d4,d0
		sub.w	d4,d2
		mulu.w	OneRow+2,d4
		divu	#80,d4
		lsl.w	#6,d4
		or.w	#80/2,d4
		WAITBLIT
		move.l	d5,$50(a5)		; Mistä
		move.l	d6,$54(a5)		; Minne
		move.w	d4,$58(a5)

LastClip2:
		tst.w	d1
		beq	ACok
		bmi	ACok
		move.l	TermOneMap,d5
		move.l	TermOneMap,d6
		subq.l	#2,d5			; Coz descending blit
		subq.l	#2,d6
		move.w	d0,d7
		mulu.w	OneRow+2,d7
		add.l	d7,d5
		move.w	d2,d7
		mulu.w	OneRow+2,d7
		add.l	d7,d6
		mulu.w	OneRow+2,d1
		divu.w	#80,d1
		lsl.w	#6,d1
		or.w	#80/2,d1
		WAITBLIT
		move.l	d5,$50(a5)		; Mistä
		move.l	d6,$54(a5)		; Minne
		move.w	d1,$58(a5)
ACok:		ENDBLIT
		tst.l	_gl_cur_qv+[[PB_RtsBlit-VAL_PB_FIRST]*4]
		beq	geton
		moveq	#TRUE,d0
		jsr	_SetRTS
geton:		rts

_HCursorOn:
;		lea	SC,a3
;		move.l	#SpriteData,d0
		move.l	Copper,a3
		move.l	Sprite_cursor,d0
		move.w	d0,Sprite-SC+6(a3)
		swap	d0
		move.w	d0,Sprite-SC+2(a3)
		rts

_HCursorOff:
;		lea	SC,a3
;		move.l	#EmptySprite,d0
		move.l	Copper,a3
		move.l	Sprite_empty,d0
		move.w	d0,Sprite-SC+6(a3)
		swap	d0
		move.w	d0,Sprite-SC+2(a3)
		rts

_H_addcol:	; INPUT	A0.L: ULONG värit
		;	D0.W: Monta
		movem.l	d2-d4,-(sp)
		move.l	#$f0000000,d4
		subq.w	#1,d0
;		lea	Colors+2,a1
		move.l	Copper,a1
		add.l	#Colors-SC+2,a1
		moveq	#16-1,d0
loop1:		movem.l	(a0)+,d1-d3
		and.l	d4,d1
		and.l	d4,d2
		and.l	d4,d3
		rol.l	#8,d1
		rol.l	#4,d1
		rol.l	#8,d2
		rol.l	#4,d3
		or.w	d1,d3
		or.w	d2,d3
		move.w	d3,(a1)
		addq.w	#4,a1
		dbf	d0,loop1
		movem.l	(sp)+,d2-d4
		rts

_H_remcol:
		rts

_HScrollDown:	; INPUT	D0.L: Monta riviä
		move.l	d0,-(sp)
.loop:		bsr	_HDown
		subq.l	#1,(sp)
		bne	.loop
		addq.w	#4,sp
		rts

CalcYmap:	; Laske missä kohtaa bitmappia rivi y on
		; INPUT	D0.W: ypos
		; OUPUT	D0.L: y-offset in bitmap
		movem.l	d1/d2,-(sp)
		move.l	TermOneMap,d2
		move.w	TermOne,d1
		subq.w	#1,d1
		sub.w	d0,d1
		bpl	uphalf
		move.l	TermTwoMap,d2
		neg.w	d1
		subq.w	#1,d1
		move.w	d1,d0
uphalf:		mulu.w	OneRow+2,d0
		add.l	d2,d0
		sub.l	TermPoint,d0
		movem.l	(sp)+,d1/d2
		rts

; --> ScrolliKeskeytys

Blank:		movem.l	a4/d2,-(sp)
		lea     _LinkerDB,a4
;		lea	SC,a0
		move.l	Copper,a0

		move.l	TermOneMap,d0		; Fix UpBitmap
		moveq	#80,d1
		move.w	d0,UpBitmap-SC+6(a0)
		swap	d0
		move.w	d0,UpBitmap-SC+2(a0)
		swap	d0
		add.w	d1,d0
		move.w	d0,UpBitmap-SC+14(a0)
		swap	d0
		move.w	d0,UpBitmap-SC+10(a0)
		swap	d0
		add.w	d1,d0
		move.w	d0,UpBitmap-SC+22(a0)
		swap	d0
		move.w	d0,UpBitmap-SC+18(a0)
		swap	d0
		add.w	d1,d0
		move.w	d0,UpBitmap-SC+30(a0)
		swap	d0
		move.w	d0,UpBitmap-SC+26(a0)

		move.w	_ScrInfo+lines,d0	; Fix DnBitmap place
		lsl.w	#3,d0
		move.w	TermTwo,d1
		subq.w	#8,d1
		bpl	termok
		moveq	#0,d1
termok:		lsl.w	#3,d1
		sub.w	d1,d0
		add.w	ScreenOffset,d0		; Missä vaihdetaan bitmappeja
		move.w	d0,d1
		lsl.w	#8,d0
		addq.w	#1,d0
		move.w	d0,DnScrPlace-SC(a0)
		cmp.w	#$100,d1
		blo	NoPAL
		move.l	#$ffdffffe,DnScrPAL-SC(a0)
		move.l	#$01a00000,StatePAL-SC(a0)
		bra	SpriteHandle
NoPAL:		move.l	#$01a00000,DnScrPAL-SC(a0)
		move.w	_ScrInfo+lines,d0
		lsl.w	#3,d0
		add.w	ScreenOffset,d0
		cmp.w	#$100,d0
		blo	SpriteHandle
		move.l	#$ffdffffe,StatePAL-SC(a0)

SpriteHandle:	;lea	SpriteData,a0		; Kursorin paikka
		move.l	Sprite_cursor,a0	; Kursorin paikka
		moveq	#0,d2
		move.w	_ScrInfo+xpos,d0
		lsl.w	#2,d0
		add.w	#128,d0
		bclr	#0,d2
		lsr.w	#1,d0
		bcc	lclr
		bset	#0,d2
lclr:		move.b	d0,1(a0)
		move.w	_ScrInfo+ypos,d0
		lsl.w	#3,d0
		add.w	ScreenOffset,d0
		move.w	d0,d1
		addq.w	#8,d1
		move.b	d0,(a0)
		move.b	d1,2(a0)
		bclr	#2,d2
		btst	#8,d0
		beq	hclr
		bset	#2,d2
hclr:		bclr	#1,d2
		btst	#8,d1
		beq	hclr2
		bset	#1,d2
hclr2:		move.b	d2,3(a0)

		movem.l	(sp)+,a4/d2
		moveq	#0,d0			; set Z-flag
		rts

BlankName:	dc.b	"DTerm.VB-SCR",0
csfontname:	dc.b	"topaz.font",0	; CSFontName
		EVEN

cop_start
SC:
AGABurst:	dc.w	$0182,$0001	; $01fc / max burst under AGA
Sprite:		dc.w	$0120,$0000,$0122,$0000
Sprite1:	dc.w	$0124,$0000,$0126,$0000
Sprite2:	dc.w	$0128,$0000,$012a,$0000
Sprite3:	dc.w	$012c,$0000,$012e,$0000
Sprite4:	dc.w	$0130,$0000,$0132,$0000
Sprite5:	dc.w	$0134,$0000,$0136,$0000
Sprite6:	dc.w	$0138,$0000,$013a,$0000
Sprite7:	dc.w	$013c,$0000,$013e,$0000
		dc.w	$0100,$0000
DiwStart:	dc.w	$008e,$0000
DiwStop:	dc.w	$0090,$0000
DataFetchStart:	dc.w	$0092,$0000
DataFetchStop:	dc.w	$0094,$0000
		dc.w	$0102,$0000
		dc.w	$0104,$0000
ModOdd:		dc.w	$0108,$0000
ModEven:	dc.w	$010a,$0000
Colors:		dc.w	$0180,$0000,$0182,$0000,$0184,$0000,$186,$0000
		dc.w	$0188,$0000,$018a,$0000,$018c,$0000,$18e,$0000
		dc.w	$0190,$0000,$0192,$0000,$0194,$0000,$196,$0000
		dc.w	$0198,$0000,$019a,$0000,$019c,$0000,$19e,$0000
		dc.w	$01a6,$0fff	; Cursor color

UpScrPlace:	dc.w	$0000,$fffe	; Wait
UpBitmap:	dc.w	$00e0,$0000,$00e2,$0000
		dc.w	$00e4,$0000,$00e6,$0000
		dc.w	$00e8,$0000,$00ea,$0000
		dc.w	$00ec,$0000,$00ee,$0000
Control:	dc.w	$0100,$0000

DnScrPAL:	dc.w	$0000,$fffe	; PAL-WAIT
DnScrPlace:	dc.w	$0000,$fffe	; Wait
DnBitmap:	dc.w	$00e0,$0000,$00e2,$0000
		dc.w	$00e4,$0000,$00e6,$0000
		dc.w	$00e8,$0000,$00ea,$0000
		dc.w	$00ec,$0000,$00ee,$0000

StatePAL:	dc.w	$0000,$fffe
StatePlace:	dc.w	$0000,$fffe
		dc.w	$0180,$0000,$182,$0fff,$0100,$8300
StatusBitmap:	dc.w	$00e0,$0000,$00e2,$0000
		dc.w	$0108,$0000,$010a,$0000
StatePlace2:	dc.w	$0000,$fffe
		dc.w	$0100,$9300
		dc.w	$ffff,$fffe
		dc.w	$ffff,$fffe
cop_end:

sprite_start:
		dc.w	0,0
		incbin	"work:dterm/scr/cursor.bin"
		dc.w	0,0
sprite_end:

sprite_empty_start:
		dc.w	0,0,0,0,0,0
sprite_empty_end:

		SECTION	__MERGED,data

ScrData:	dc.l	0		; Osoite varattuihin bitmappeihin
SaveView:	dc.l	0
TermPoint:	dc.l	0		; Osoite bitmappien alkuun
ScreenOffset:	dc.w	$24
ScreenState:	dc.b	0		; -1 = Screeni päällä
IntOn:		dc.b	0		; Interrupt päällä
ScrOk:		dc.b	0
		EVEN
TermOneMap:	dc.l	0		; Ylä BITMAP
TermOne:	dc.w	0		; Monta riviä
TermTwoMap:	dc.l	0		; Ala BITMAP
TermTwo:	dc.w	0		; Monta riviä
TermDBF:	dc.w	0		; Monta bitmappia päällä-1
TermDepth:	dc.w	0		; Monta bitmappia päällä
OneRow:		dc.l	0		; Yhden rivin koko

oldres          dc.l  0
taglist         dc.l  VTAG_SPRITERESN_GET
resolution      dc.l  SPRITERESN_ECS
                dc.l  TAG_DONE,0

BlankInt:	dc.l	0
		dc.l	0
		dc.b	NT_INTERRUPT
		dc.b	0
		dc.l	BlankName
		dc.l	BlankName
		dc.l	Blank
;_hardscrver:	dc.b	'1.44 (Dec 07 1995)',0
		EVEN

; --> any

RawFont:	dc.l	0
RawFontInv:	dc.l	0
RawFontBold:	dc.l	0
RawFontBoldInv:	dc.l	0
RawFontUnd:	dc.l	0
RawFontUndInv:	dc.l	0
TempBitmap:	dc.l	0
TempRast:	dc.l	0

; --> chip

Copper:		dc.l	0
Sprite_cursor:	dc.l	0
Sprite_empty:	dc.l	0
TempText:	dc.l	0
StatusMap:	dc.l	0

		END
