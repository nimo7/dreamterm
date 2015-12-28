#ifndef DTERM_SCREEN_H
#define DTERM_SCREEN_H

#include <exec/types.h>

/* Declare Screen routines */

void	__asm ScreenToUse(register __d0 ULONG);
BOOL	__asm ScreenInit(void);
BOOL	__asm ScreenRemove(void);
BOOL	__asm ScreenChange(register __a0 APTR);
BOOL	__asm ScreenOn(void);
BOOL	__asm ScreenOff(void);
void	__asm ScreenPrint(register __a0 APTR, register __d0 WORD);
void	__asm StatPrint(register __a0 APTR);
void	__asm ScreenDown(void);
void	__asm ScreenUp(void);
void	__asm ScreenClearInLine(register __d1 WORD,register __d0 WORD);
void	__asm ScreenCopyInLine(register __d1 WORD,register __d2 WORD,register __d0 WORD);
void	__asm ScreenAreaClear(register __d0 WORD,register __d1 WORD);
void	__asm ScreenAreaCopy(register __d0 WORD,register __d2 WORD,register __d1 WORD);
void	__asm CursorOn(void);
void	__asm CursorOff(void);
void	__asm Screen_addcol(register __a0 ULONG *, register __d0 WORD);
void	__asm Screen_remcol(void);
void	__asm ScrollDown(register __d0 LONG);

/* Kumpaa screeniä käytetään  (ScreenToUse) */

#define SCR_HARD	TRUE
#define SCR_SOFT	FALSE

/* Screeninfo josta hard/soft screeni lukee xposition yms. ja kirjoittaa
   oman infonsa -> ver,rev,id */

struct ScreenInfo
{
	WORD	xpos;
	WORD	ypos;
	WORD	columns;
	WORD	columnsdbf;
	WORD	lines;
	WORD	linesdbf;
	WORD	forecol;
	WORD	backcol;
	WORD	fontstate;	/* graphics/text.h */
	UBYTE	inverse;		/* Inverse video */
	UBYTE	nil;
};

#endif
