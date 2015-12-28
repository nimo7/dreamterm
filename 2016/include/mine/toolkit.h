#ifndef TOOLKIT_H
#define TOOLKIT_H

/*
**  stuff which I use/need often
**
*/

#include <dos/dos.h>
#include <exec/types.h>
#include <exec/nodes.h>

/* for some reason I wanna use lowercase names */

typedef  BOOL   bool;
typedef  BPTR   bptr;
typedef  APTR   aptr;
typedef  BYTE   byte;
typedef UBYTE  ubyte;
typedef  WORD   word;
typedef UWORD  uword;
typedef ULONG  ulong;
typedef STRPTR strptr;

#undef  ASM
#undef  REG
#undef  SAVEDS
#define AND     &&
#define OR      ||
#define NOT     !
#define ASM     __asm
#define REG     register
#define SAVEDS  __saveds

#define GetPred(node) (((struct Node *)node)->ln_Pred->ln_Pred ? ((struct Node *)node)->ln_Pred : NULL)
#define GetSucc(node) (((struct Node *)node)->ln_Succ->ln_Succ ? ((struct Node *)node)->ln_Succ : NULL)
#define GetHead(list) GetSucc(list)
#define TF(x) ((x) ? FALSE : TRUE)

#ifndef IFF_IFFPARSE_H
#define MAKE_ID(a,b,c,d) ((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))
#endif

#endif
