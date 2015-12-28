#include <exec/types.h>

#include "emulation/ansiext.h"
#include "emulation/hexext.h"

#define EM_ANSI		0	/* Mikä emulaatio */
#define EM_HEX		1
#define EM_XEM		2

void	__asm EmToUse	(register __d0 ULONG);
BOOL	__asm	EmInit	(register	__a0 APTR);
BOOL	__asm	EmRemove(register	__a0 APTR);
BOOL	__asm EmReset	(register __a0 APTR);
BOOL	__asm EmChange(register __a0 APTR);
void	__asm EmPrint	(register __a0 APTR, register __d0 ULONG);
ULONG	__asm EmStrip (register __a0 APTR, register __a1 APTR, register __d0 ULONG);
