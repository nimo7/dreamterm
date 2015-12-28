	INCLUDE	"exec/types.i"

; --> Kumpaa screeni‰ k‰ytet‰‰n  (ScreenToUse)

SCR_HARD	EQU 1
SCR_SOFT	EQU 0

    STRUCTURE	SCREENINFO,0
	WORD	xpos
	WORD	ypos
	WORD	columns
	WORD	columnsdbf
	WORD	lines
	WORD	linesdbf
	WORD	forecol
	WORD	backcol
	WORD	fontstate	; graphics/text.i
	UBYTE	inverse
	UBYTE	nil
    LABEL	SCREENINFO_SIZE
