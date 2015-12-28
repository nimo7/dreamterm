#ifndef XPK_XPKOBSOLETE_H
#define XPK_XPKOBSOLETE_H

/*
**	$VER: xpk/xpkobsolete.h 4.10 (05.04.97) by SDI
**
**	(C) Copyright 1996 by Dirk Stöcker
**	    All Rights Reserved
**
**	Made for all the old definitions!
**	Has to be included before the first other xpk include !!
**	Do NOT use this in newer code!
**
*/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif
#include <dos/dos.h>

typedef struct TagItem		TAGS;
typedef struct Hook *		HOOK;

typedef struct XpkIOMsg 	XIOMSG;
typedef struct XpkProgress	XPROG;
typedef struct XpkFib		XFIB;
typedef struct XpkPackerInfo	XPINFO;
typedef struct XpkMode		XMINFO;
typedef struct XpkPackerList	XPLIST;
typedef struct XpkFib		XFH;
#define XpkFH XpkFib

typedef struct XpkInfo		XINFO;
typedef struct XpkSubParams	XPARAMS;

#define XPK_FindMethod XPK_PackMethod

/* This gives the ability to use the old Structure member names */
#define xiom_Type		Type
#define xiom_Ptr		Ptr
#define xiom_Size		Size
#define xiom_IOError		IOError
#define xiom_Reserved		Reserved
#define xiom_Private1		Private1
#define xiom_Private2		Private2
#define xiom_Private3		Private3
#define xiom_Private4		Private4
#define xp_Type 		Type
#define xp_PackerName		PackerName
#define xp_PackerLongName	PackerLongName
#define xp_Activity		Activity
#define xp_FileName		FileName
#define xp_CCur 		CCur
#define xp_UCur 		UCur
#define xp_ULen 		ULen
#define xp_CF			CF
#define xp_Done 		Done
#define xp_Speed		Speed
#define xp_Reserved		Reserved
#define xf_Type 		Type
#define xf_ULen 		ULen
#define xf_CLen 		CLen
#define xf_NLen 		NLen
#define xf_UCur 		UCur
#define xf_CCur 		CCur
#define xf_ID			ID
#define xf_Packer		Packer
#define xf_SubVersion		SubVersion
#define xf_MasVersion		MasVersion
#define xf_Flags		Flags
#define xf_Head 		Head
#define xf_Ratio		Ratio
#define xf_Reserved		Reserved
#define xpi_Name		Name
#define xpi_LongName		LongName
#define xpi_Description 	Description
#define xpi_Flags		Flags
#define xpi_MaxChunk		MaxChunk
#define xpi_DefChunk		DefChunk
#define xpi_DefMode		DefMode
#define xm_Next 		Next
#define xm_Upto 		Upto
#define xm_Flags		Flags
#define xm_PackMemory		PackMemory
#define xm_UnpackMemory 	UnpackMemory
#define xm_PackSpeed		PackSpeed
#define xm_UnpackSpeed		UnpackSpeed
#define xm_Ratio		Ratio
#define xm_ChunkSize		ChunkSize
#define xm_Description		Description
#define xpl_NumPackers		NumPackers
#define xpl_Packer		Packer
#define xi_XpkInfoVersion	XpkInfoVersion
#define xi_LibVersion		LibVersion
#define xi_MasterVersion	MasterVersion
#define xi_ModesVersion 	ModesVersion
#define xi_Name 		Name
#define xi_LongName		LongName
#define xi_Description		Description
#define xi_ID			ID
#define xi_Flags		Flags
#define xi_MaxPkInChunk 	MaxPkInChunk
#define xi_MinPkInChunk 	MinPkInChunk
#define xi_DefPkInChunk 	DefPkInChunk
#define xi_PackMsg		PackMsg
#define xi_UnpackMsg		UnpackMsg
#define xi_PackedMsg		PackedMsg
#define xi_UnpackedMsg		UnpackedMsg
#define xi_DefMode		DefMode
/* #define xi_Pad		Pad */
#define xi_ModeDesc		ModeDesc
#define xi_Reserved		Reserved
#define xsp_InBuf		InBuf
#define xsp_InLen		InLen
#define xsp_OutBuf		OutBuf
#define xsp_OutBufLen		OutBufLen
#define xsp_OutLen		OutLen
#define xsp_Flags		Flags
#define xsp_Number		Number
#define xsp_Mode		Mode
#define xsp_Password		Password
#define xsp_LibVersion		LibVersion
/* #define xsp_Pad 		Pad */
#define xsp_Arg 		Arg
#define xsp_Sub 		Sub

#endif /* XPK_XPKOBSOLETE_H */
