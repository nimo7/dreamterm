#ifndef XPK_XPKPREFS_H
#define XPK_XPKPREFS_H

/*
**	$VER: xpk/xpkprefs.h 4.10 (05.04.97) by SDI
**
**	(C) Copyright 1996-1997 by Dirk Stöcker
**	    All Rights Reserved
*/

#include <exec/semaphores.h>
#include <libraries/iffparse.h>

#define ID_XPKT  MAKE_ID('X','P','K','T')
#define ID_XPKM  MAKE_ID('X','P','K','M')

/***************************************************************************
 *
 *
 *     XpkTypeData structure
 *
 */

#define XTD_NoPack		(1<<0)	/* filetype should not be crunched */
#define XTD_ReturnError		(1<<1)	/* return XPKERR_BADPARAMS -- this
					   equals version 3 handling */
/* These two cannot be set same time! */

struct XpkTypeData {
  ULONG  xtd_Flags;	   /* see above XTD flags */
  ULONG  xtd_StdID;	   /* holding the ID --> 'NUKE' */
  ULONG  xtd_ChunkSize;    /* maybe useless with external crunchers */
  UWORD  xtd_Mode;	   /* PackMode */
  UWORD  xtd_Version;	   /* structure version --> 0 at the moment */
  STRPTR xtd_Password;	   /* not used at the moment */
  STRPTR xtd_Memory;	   /* memory pointer - when should be freed by */
  ULONG  xtd_MemorySize;   /* memory size    - receiver (xpkmaster) */
};

/***************************************************************************
 *
 *
 *     XpkTypePrefs structure
 *
 */

#define XPKT_NamePattern	(1<<0)	/* File Pattern is given */
#define XPKT_FilePattern	(1<<1)  /* Name Pattern is given */
/* These can both be set (in loading this means File AND Name Pattern have
to match), but one is needed */

struct XpkTypePrefs {
  ULONG 		xtp_Flags;	 /* See above XPKT Flags */
  STRPTR 		xtp_TypeName;	 /* Name of this file type (for prefs program) */
  STRPTR 		xtp_NamePattern; /* Pointer to NamePattern */
  STRPTR 		xtp_FilePattern; /* Pointer to FilePattern */
  struct XpkTypeData *	xtp_PackerData;
};

/***************************************************************************
 *
 *
 *     XpkMainPrefs structure
 *
 */

#define XPKM_UseXFD		(1<<0)	/* Use xfdmaster.library for unpacking */
#define XPKM_UseExternals	(1<<1)	/* Use xex libraries */
#define XPKM_AutoPassword	(1<<2)	/* Use the automatic password requester */

struct XpkMainPrefs {
  ULONG			xmp_Version;	 /* version of structure ==> 0 */
  ULONG			xmp_Flags;	 /* above defined XPKM flags */
  struct XpkTypeData *	xmp_DefaultType; /* sets the mode used as default */
  UWORD			xmp_Timeout;     /* Timeout for password requester
				 given in seconds, zero means no timeout */
};

/* The library internal defaults are:
  XPKM_UseXFD			FALSE
  XPKM_AutoPassword		FALSE
  XPKM_UseExternals		TRUE
  XTD_ReturnError		defined as default
  xmp_TimeOut			set to 120	(two minutes)

These defaults are used, when no preferences file is given.
*/

/***************************************************************************
 *
 *
 *     XpkMasterPrefs Semaphore structure
 *
 *  find with FindSemaphore(XPKPREFSSEMNAME);
 *
 *  obtain with ObtainSemaphoreShared(),
 *  programs WRITING into the structure fields must know:
 *    - use ObtainSemaphore() instead of ObtainSemaphoreShared()
 *    - free memory of elements you remove
 *    - xb_MainPrefsSize is the length of memory allocated for xb_MainPrefs
 *    - all other nodes are freed, when XpkMasterPrefs program finishes, do
 *	not do it your own, but you have to allocate memory for new ones!
 *  Generally there should be no need to write to these fields !!!!
 */

#define XPKPREFSSEMNAME		"« XpkMasterPrefs »"

/* Defines used for xps_PrefsType. These help to find out, which preferences
 * type is used. */

#define XPREFSTYPE_STANDARD	0x58504B4D	/* 'XPKM' */
#define XPREFSTYPE_CYB		0x20435942	/* ' CYB' */

struct XpkPrefsSemaphore {
  struct SignalSemaphore 	xps_Semaphore;
  ULONG				xps_Version;	   /* at the moment 0 */
  ULONG				xps_PrefsType;	   /* preferences type */
  APTR				xps_PrefsData;	   /* preferences data */
  struct XpkMainPrefs *		xps_MainPrefs;     /* defined defaults */
  ULONG				xps_RecogSize;	   /* needed size of Recogbuffer */
  struct XpkTypeData * ( *	xps_RecogFunc) (); /* Recog function */
  struct Hook *			xps_ProgressHook;  /* hook function */
  struct Task *			xps_MasterTask;	   /* Creater's task */
};

/* Use Signal(sem->xps_MasterTask, SIGBREAKF_CTRL_C); to get the installer
   program to remove the semaphore. */

/* prototype/typedef of RecogFunc:
typedef struct XpkTypeData * __asm (*RecogFunc)
	(register __a0 STRPTR buffer,
	 register __a1 STRPTR filename,
	 register __a2 STRPTR chunkname,
	 register __d0 ULONG  buffersize,
	 register __d1 ULONG  fullsize);
*/

#endif /* XPK_XPKPREFS_H */
