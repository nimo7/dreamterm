	IFND	Mine_OffsetA4_I
Mine_OffsetA4_I SET 1

*************************************************
*						*
*	V1.0 -- 13.06.1992			*
*	V1.1 -- 31.08.1993			*
*	V1.2 -- 08.10.1993 PhxAsm (only)	*
*						*
*************************************************

TRUE		EQU	1
FALSE		EQU	0
NULL		EQU	0

;		NREF	_SysBase

; --> Macros

OPENLIB		MACRO	; open lib (name, version)
		move.l	_SysBase,a6
		lea	\1(pc),a1
		moveq	#\2,d0
		jsr	_LVOOpenLibrary(a6)
		ENDM

CALL		MACRO
		; \1 = lib name / base is added: Exec[Base]
		; \2 = func name
	IFC	'\1','Exec'
		move.l	_SysBase,a6
	ELSE
		move.l	_\1Base,a6
	ENDIF
		jsr	_LVO\2(a6)
		ENDM

LIBC		MACRO	; call func
		jsr	_LVO\1(a6)
		ENDM

	ENDC	; Mine_OffsetA4_I
