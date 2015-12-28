#ifndef  CLIB_XPKMASTER_PROTOS_H
#define  CLIB_XPKMASTER_PROTOS_H

/*
**	$VER: clib/xpkmaster_protos.h 4.0 (27.12.96) by SDI
**
**
**
**	(C) Copyright 1991-1996 by 
**          Urban Dominik Mueller, Bryan Ford,
**          Christian Schneider, Christian von Roques,
**	    Dirk Stöcker
**	    All Rights Reserved
*/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef XPK_XPK_H
#include <xpk/xpk.h>
#endif

LONG  XpkExamine	(struct XpkFib *fib, struct TagItem *tags);
LONG  XpkExamineTags	(struct XpkFib *fib, ULONG tag1Type, ...);
LONG  XpkPack		(struct TagItem *tags);
LONG  XpkPackTags	(ULONG tag1Type, ...);
LONG  XpkUnpack		(struct TagItem *tags);
LONG  XpkUnpackTags	(ULONG tag1Type, ...);
LONG  XpkOpen		(struct XpkFib **xfh, struct TagItem *tags);
LONG  XpkOpenTags	(struct XpkFib **xfh, ULONG tag1Type, ...);
LONG  XpkRead		(struct XpkFib *xfh, STRPTR buf, ULONG len);
LONG  XpkWrite		(struct XpkFib *xfh, STRPTR buf, LONG ulen);
LONG  XpkSeek		(struct XpkFib *xfh, LONG dist, ULONG mode);
LONG  XpkClose		(struct XpkFib *xfh);
LONG  XpkQuery		(struct TagItem *tags);
LONG  XpkQueryTags	(ULONG tag1Type, ...);

/* here start version 4 functions */

APTR  XpkAllocObject	(ULONG type, struct TagItem *tags);
APTR  XpkAllocObjectTags(ULONG type, ULONG tag1Type, ...);
void  XpkFreeObject	(ULONG type, APTR object);
BOOL  XpkPrintFault	(LONG code, STRPTR header);
ULONG XpkFault		(LONG code, STRPTR header, STRPTR buffer, ULONG size);
LONG  XpkPassRequest	(struct TagItem *tags);
LONG  XpkPassRequestTags(ULONG tag1Type, ...);

#endif	/* CLIB_XPK_PROTOS_H */
