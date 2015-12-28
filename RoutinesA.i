	XREF	_FastDec1
	XREF	_FastDec2
	XREF	_FastDec3
	XREF	_FastDec4
	XREF	_FastDec5
	XREF	_FastDec6
	XREF	_FastDec7
	XREF	_FastDec8
	XREF	_MyAllocR
	XREF	_CLRFreeR
	XREF	_DoAscii
	XREF	_HuntBIT
	XREF	_SetBIT
	XREF	_GetTag

GETTAG:	MACRO	; tagid,ptr taglist
	move.l	\1,d0
	lea	\2,a0
	jsr	_GetTag
	ENDM
