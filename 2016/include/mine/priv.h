#ifndef PRIVLIB_H
#define PRIVLIB_H

/*
**  protos for priv.lib
**
*/

#include <exec/types.h>
#include <exec/lists.h>
#include <exec/nodes.h>
#include <mine/toolkit.h>

/* c routines */

void         AddNode_sort(struct List *, struct Node *);
long         CalcNode_num(struct Node *);
struct Node *FindNode_num(struct List *, long);
struct Node *RemNode     (struct Node *);
struct Node *ReplaceNode (struct Node *, struct Node *);
bool         SortList    (struct List *);

/* asm routines */

ulong __asm GetSec   (void);
void  __asm TimeText (register __d0 ulong, register __a0 STRPTR);
ulong __asm Pot2     (register __d0 word);
ulong __asm Bset     (register __d1 byte);
void  __asm FastDec1 (register __d0 ulong, register __a0 STRPTR);
void  __asm FastDec2 (register __d0 ulong, register __a0 STRPTR);
void  __asm FastDec3 (register __d0 ulong, register __a0 STRPTR);
void  __asm FastDec4 (register __d0 ulong, register __a0 STRPTR);
void  __asm FastDec5 (register __d0 ulong, register __a0 STRPTR);
void  __asm FastDec6 (register __d0 ulong, register __a0 STRPTR);
void  __asm FastDec7 (register __d0 ulong, register __a0 STRPTR);
void  __asm FastDec8 (register __d0 ulong, register __a0 STRPTR);
long  __asm DoAscii  (register __d0 ulong, register __a0 STRPTR);
long  __asm NodeCount(register __a0 struct List *);

#endif
