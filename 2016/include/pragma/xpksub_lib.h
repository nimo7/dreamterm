#ifndef _INCLUDE_PRAGMA_XPKSUB_LIB_H
#define _INCLUDE_PRAGMA_XPKSUB_LIB_H

#ifndef CLIB_XPKSUB_PROTOS_H
#include <clib/xpksub_protos.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(AZTEC_C) || defined(__MAXON__) || defined(__STORM__)
#pragma amicall(XpkSubBase,0x01E,XpksPackerInfo())
#pragma amicall(XpkSubBase,0x024,XpksPackChunk(a0))
#pragma amicall(XpkSubBase,0x02A,XpksPackFree(a0))
#pragma amicall(XpkSubBase,0x030,XpksPackReset(a0))
#pragma amicall(XpkSubBase,0x036,XpksUnpackChunk(a0))
#pragma amicall(XpkSubBase,0x03C,XpksUnpackFree(a0))
#endif
#if defined(_DCC) || defined(__SASC)
#pragma libcall XpkSubBase XpksPackerInfo       01E 00
#pragma libcall XpkSubBase XpksPackChunk        024 801
#pragma libcall XpkSubBase XpksPackFree         02A 801
#pragma libcall XpkSubBase XpksPackReset        030 801
#pragma libcall XpkSubBase XpksUnpackChunk      036 801
#pragma libcall XpkSubBase XpksUnpackFree       03C 801
#endif

#ifdef __cplusplus
}
#endif

#endif	/*  _INCLUDE_PRAGMA_XPKSUB_LIB_H  */