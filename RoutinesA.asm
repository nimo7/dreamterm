		OPT	!
		NEAR	A4,-2
		MACHINE	68000

		INCDIR	"asminc:"
		INCLUDE	"global.i"
		INCLUDE	"mine/offsetA4.i"

		NREF	_SysBase
		NREF	_DOSBase
		XREF	@EndProg
		XREF	_LVOPutStr
		XREF	_LVOAllocMem
		XREF	_LVOFreeMem
		XREF	_LVOAllocPooled
		XREF	_LVOFreePooled
		XDEF	_GetTag
;		XDEF	_MergePF
		XDEF	_RawTranslate
		XDEF	_PrepFlow
		XDEF	_HuntFlow
		XDEF	_CmpPhone
		XDEF	_DatCmp
		XDEF	_AllocVecPooled
		XDEF	_FreeVecPooled
;		XDEF	_ResetRev
;		XDEF	_ConvToRev
;		XDEF	_RevLine_find
;		XDEF	_RevLine_prev
;		XDEF	_RevLine_next
;		XDEF	_RevLine_num
		XDEF	_RetIndex

; --> Amiga lib new names

		XDEF	_TempTimeDelay
		XREF	TimeDelay
		XDEF	_TempDoSuperMethodA
		XREF	_DoSuperMethodA
		XDEF	_TempDoMethodA
		XREF	_DoMethodA

		SECTION	code,code

_TempTimeDelay:
		jmp	TimeDelay

_TempDoSuperMethodA:
		movem.l	a0-a2,-(sp)
		jsr	_DoSuperMethodA
		add.w	#12,sp
		rts

_TempDoMethodA:
		movem.l	a0-a1,-(sp)
		jsr	_DoMethodA
		add.w	#8,sp
		rts

; --> Some general (and less) routines for DreamTerm

_GetTag:	; Etsi tag listasta
		; INPUT	D0.L: tagid
		;	A0.L: tagbase
		; OUPUT	D0.L: tagdata
		move.l	(a0),d1
		beq	.err1
		move.l	d1,a0
.notag:		move.l	(a0),d1
		beq	.err2
		add.w	#8,a0
		cmp.l	d1,d0
		bne	.notag
		move.l	-(a0),d0
		rts
.err1:		lea	err1,a0
		jmp	@EndProg
.err2:		lea	err2,a0
		jmp	@EndProg

err1: 		dc.b	'GetTag_1()',0
err2: 		dc.b	'GetTag_2()',0
		EVEN

_RetIndex:	; Return ulong list index
		; INPUT A0.L: null ending list
		;       D0.L: value to find
		move.l	d2,a1
		moveq	#-1,d1
.loop:		addq.l	#1,d1
		move.l	(a0)+,d2
		beq	.error
		cmp.l	d0,d2
		bne	.loop
		move.l	d1,d0
		move.l	a1,d2
		rts
.error:		moveq	#-1,d0
		move.l	a1,d2
		rts

_PrepFlow:	; Init flowstring structure
		; INPUT	A0.L: FlowString structure
.clear:		clr.l	4(a0)
		add.w	#8,a0
		tst.l	(a0)
		bne	.clear
		rts

_HuntFlow:	; Find string
		; INPUT	A0.L: FlowString structure
		;	A1.L: Adr
		;	D0.L: Size
		movem.l	a2/d2-d4,-(sp)
.hflow:		movem.l	a0-a1/d0,-(sp)

		add.l	a1,d0
		move.l	(a0),a2
		move.l	a2,d3			; Save first adr
		move.w	6(a0),d1
		beq	.first
		add.w	d1,a2
		bra	.news
.first:		move.b	(a2)+,d2		; Search 1st char
.huntf:		cmp.l	a1,d0
		beq	.nosub
		cmp.b	(a1)+,d2
		bne	.huntf
.news:		move.l	a1,d4			; Save next to it
.huntr:		move.b	(a2)+,d2		; Check rest
		beq	.hit
		cmp.l	a1,d0
		beq	.sarea
		cmp.b	(a1)+,d2
		beq	.huntr
		move.l	d3,a2
		move.l	d4,a1
		bra	.first
.sarea:		sub.l	d3,a2			; How much found
		sub.w	#1,a2
		move.w	a2,6(a0)
		bra	.nosub
.hit:		move.w	#TRUE,4(a0)
.nosub:
		movem.l	(sp)+,a0-a1/d0
		add.w	#8,a0
		tst.l	(a0)
		bne	.hflow
		movem.l	(sp)+,a2/d2-d4
		rts

_RawTranslate:	; Simple translate chars
		; INPUT	A0.L: Mistä
		;	A1.L: Minne
		;	A2.L: Table
		;	D0.L: Paljon
		; OUPUT	D0.L: size
		move.w	d0,d1
		beq	.skip
		movem.l	d2/a2,-(sp)
		subq.w	#1,d1
		move.l	a1,d0
		moveq	#0,d2
.copy:		move.b	(a0)+,d2
		move.b	(a2,d2.w),(a1)+
		bne	.nostrip
		subq.w	#1,a1
.nostrip:	dbf	d1,.copy
		exg	a1,d0
		sub.l	a1,d0
		movem.l	(sp)+,d2/a2
.skip:		rts

_CmpPhone:	; Compare phone against group
		; INPUT	A0.L: Phonenumber
		;	A1.L: Phonestring (xxx|xxx)
		; OUPUT	D0.W: TRUE/FALSE
		move.l	d2,-(sp)
		move.l	a0,d2
.skipit:	move.b	(a1)+,d1
		beq	.nomatch
		cmp.b	#'|',d1
		beq	.reset
		cmp.b	#'0',d1
		blo	.skipit
		cmp.b	#'9',d1
		bhi	.skipit
.try:		move.b	(a0)+,d0
		beq	.match
		cmp.b	#'0',d0
		blo	.try
		cmp.b	#'9',d0
		bhi	.try
		cmp.b	d0,d1
		bne	.skiprest
		tst.b	(a0)
		beq	.match
		bra	.skipit
.skiprest:	move.b	(a1)+,d1
		beq	.nomatch
		cmp.b	#'|',d1
		bne	.skiprest
.reset:		move.l	d2,a0
		bra	.skipit
.match:		move.l	(sp)+,d2
		moveq	#TRUE,d0
		rts
.nomatch:	move.l	(sp)+,d2
		moveq	#FALSE,d0
		rts

_DatCmp:	; Compare DAT tags
		; INPUT	A0.L: tagdata 1
		; INPUT	A1.L: tagdata 2
		; OUPUT	D0.W: 0 = equal, -1/1 not equal
		moveq	#1,d0
		cmp.l	(a0)+,(a1)+
		bne	.noequal
		move.l	-4(a0),d1
		beq	.equal
.loop:		cmpm.b	(a0)+,(a1)+
		bne	.noequal
		sub.l	#1,d1
		bne	.loop
.equal:		moveq	#0,d0
.noequal:	rts

_AllocVecPooled:
;	      IFD DEBUG
;		move.l	a6,-(sp)
;		add.l	#4+8,d0
;		move.l	d0,-(sp)
;		CALL	Exec,AllocPooled
;		move.l	(sp)+,d1
;		tst.l	d0
;		beq	.nomem
;		move.l	d0,a0
;		move.l	d1,(a0)+
;		move.l	#$01020304,(a0)+	; start ID
;		move.l	#$04030201,-12(a0,d1.l)	; end ID
;		move.l	a0,d0
;.nomem:		move.l	(sp)+,a6
;		rts
;	      ELSE
		move.l	a6,-(sp)
		addq.l	#4,d0
		move.l	d0,-(sp)
		CALL	Exec,AllocPooled
		move.l	(sp)+,d1
		tst.l	d0
		beq	.nomem
		move.l	d0,a0
		move.l	d1,(a0)+
		move.l	a0,d0
.nomem:		move.l	(sp)+,a6
		rts
;	      ENDIF

_FreeVecPooled:
;	      IFD DEBUG
;		movem.l	d0-d7/a0-a6,-(sp)
;		move.l	_DOSBase,a6
;		move.l	a1,a2
;		cmp.l	#$01020304,-4(a2)
;		beq	.ok1
;		move.l	#id_1,d1
;		LIBC	PutStr
;.ok1:
;		move.l	-8(a2),d0
;		cmp.l	#$04030201,-12(a2,d0.l)
;		beq	.ok2
;		move.l	#id_2,d1
;		LIBC	PutStr
;.ok2:
;		movem.l	(sp)+,d0-d7/a0-a6
;
;		sub.l	#4,a1
;		move.l	a6,-(sp)
;		move.l	-(a1),d0
;		CALL	Exec,FreePooled
;		move.l	(sp)+,a6
;		rts
;id_1:		dc.b	'start\n',0
;id_2:		dc.b	'.end.\n',0
;		EVEN
;	      ELSE
		move.l	a6,-(sp)
		move.l	-(a1),d0
		CALL	Exec,FreePooled
		move.l	(sp)+,a6
		rts
;	      ENDIF
