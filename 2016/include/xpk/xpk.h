#ifndef XPK_XPK_H
#define XPK_XPK_H

/*
**	$VER: xpk/xpk.h 4.10 (05.04.97) by SDI
**
**	(C) Copyright 1991-1996 by 
**          Urban Dominik Mueller, Bryan Ford,
**          Christian Schneider, Christian von Roques,
**	    Dirk Stöcker
**	    All Rights Reserved
*/

#ifndef EXEC_LIBRARIES_H
  #include <exec/libraries.h>
#endif

#ifndef EXEC_LISTS_H
  #include <exec/lists.h>
#endif

#ifndef UTILITY_TAGITEM_H
  #include <utility/tagitem.h>
#endif

#ifndef UTILITY_HOOKS_H
  #include <utility/hooks.h>
#endif

#define XPKNAME "xpkmaster.library"

/***************************************************************************
 *
 *
 *      The packing/unpacking tags
 *
 * (TRUE) or (FALSE) mean the default value given by xpkmaster.library
 *
 */

#define XPK_TagBase	(TAG_USER + ('X'<<8) + 'P')
#define XTAG(a)		(XPK_TagBase+a)

/* Caller must supply ONE of these to tell Xpk#?ack where to get data from */
#define XPK_InName	  XTAG(0x01) /* Process an entire named file */
#define XPK_InFH	  XTAG(0x02) /* File handle - start from current position */
				     /* If packing partial file, must also supply InLen */
#define XPK_InBuf	  XTAG(0x03) /* Single unblocked memory buffer */
				     /* Must also supply InLen */
#define XPK_InHook	  XTAG(0x04) /* Call custom Hook to read data */
				     /* Must also supply InLen, when hook
				        cannot do! (not for XPK unpacking) */

/* Caller must supply ONE of these to tell Xpk#?ackFile where to send data to */
#define XPK_OutName	  XTAG(0x10) /* Write (or overwrite) this data file */
#define XPK_OutFH	  XTAG(0x11) /* File handle - write from current position on */
#define XPK_OutBuf	  XTAG(0x12) /* Unblocked buffer - must also supply OutBufLen */
#define XPK_GetOutBuf	  XTAG(0x13) /* Master allocates OutBuf - ti_Data points to buf ptr */
#define XPK_OutHook	  XTAG(0x14) /* Callback Hook to get output buffers */

/* Other tags for Pack/Unpack */
#define XPK_InLen	  XTAG(0x20) /* Length of data in input buffer  */
#define XPK_OutBufLen	  XTAG(0x21) /* Length of output buffer         */
#define XPK_GetOutLen	  XTAG(0x22) /* ti_Data points to long to receive OutLen    */
#define XPK_GetOutBufLen  XTAG(0x23) /* ti_Data points to long to receive OutBufLen */
#define XPK_Password	  XTAG(0x24) /* Password for de/encoding        */
#define XPK_GetError	  XTAG(0x25) /* ti_Data points to buffer for error message  */
#define XPK_OutMemType	  XTAG(0x26) /* Memory type for output buffer   */
#define XPK_PassThru	  XTAG(0x27) /* Bool: Pass through unrecognized formats on unpack */
#define XPK_StepDown	  XTAG(0x28) /* Bool: Step down pack method if necessary    */
#define XPK_ChunkHook	  XTAG(0x29) /* Call this Hook between chunks   */
#define XPK_PackMethod	  XTAG(0x2a) /* Do a FindMethod before packing  */
#define XPK_ChunkSize	  XTAG(0x2b) /* Chunk size to try to pack with  */
#define XPK_PackMode	  XTAG(0x2c) /* Packing mode for sublib to use  */
#define XPK_NoClobber	  XTAG(0x2d) /* Don't overwrite existing files  */
#define XPK_Ignore	  XTAG(0x2e) /* Skip this tag                   */
#define XPK_TaskPri	  XTAG(0x2f) /* Change priority for (un)packing */
#define XPK_FileName	  XTAG(0x30) /* File name for progress report   */
#define XPK_ShortError	  XTAG(0x31) /* !!! obsolete !!!                */
#define XPK_PackersQuery  XTAG(0x32) /* Query available packers         */
#define XPK_PackerQuery	  XTAG(0x33) /* Query properties of a packer    */
#define XPK_ModeQuery	  XTAG(0x34) /* Query properties of packmode    */
#define XPK_LossyOK	  XTAG(0x35) /* Lossy packing permitted? (FALSE)*/
#define XPK_NoCRC         XTAG(0x36) /* Ignore checksum                 */

/* preference depending tags added for version 4 - their default value
 may depend on preferences, see <xpk/xpkprefs.h> for more info */

#define XPK_UseXfdMaster  XTAG(0x40) /* Use xfdmaster.library (FALSE)   */
#define XPK_UseExternals  XTAG(0x41) /* Use packers in extern dir (TRUE)*/
#define XPK_PassRequest   XTAG(0x42) /* automatic password req.? (FALSE)*/
#define XPK_Preferences   XTAG(0x43) /* use prefs semaphore ? (TRUE)    */
#define XPK_ChunkReport	  XTAG(0x44) /* automatic chunk report ? (FALSE)*/

/* tags XTAG(0x50) to XTAG(0x69) are for XpkPassRequest -- see below */

#define XPK_MARGIN	256	/* Safety margin for output buffer	*/

/***************************************************************************
 *
 *
 *     The hook function interface
 *
 */

/* Message passed to InHook and OutHook as the ParamPacket */
struct XpkIOMsg {
	ULONG xiom_Type		; /* Read/Write/Alloc/Free/Abort	*/
	APTR  xiom_Ptr		; /* The mem area to read from/write to */
	LONG  xiom_Size		; /* The size of the read/write		*/
	ULONG xiom_IOError	; /* The IoErr() that occurred		*/
	ULONG xiom_Reserved	; /* Reserved for future use		*/
	ULONG xiom_Private1	; /* Hook specific, will be set to 0 by */
	ULONG xiom_Private2	; /* master library before first use	*/
	ULONG xiom_Private3	;
	ULONG xiom_Private4	;
};

/* The values for XpkIoMsg->Type */
#define XIO_READ    1
#define XIO_WRITE   2
#define XIO_FREE    3
#define XIO_ABORT   4
#define XIO_GETBUF  5
#define XIO_SEEK    6
#define XIO_TOTSIZE 7

/***************************************************************************
 *
 *
 *      The progress report interface
 *
 */

/* Passed to ChunkHook as the ParamPacket */
struct XpkProgress {
  ULONG	 xp_Type;	    /* Type of report: start/cont/end/abort	  */
  STRPTR xp_PackerName;     /* Brief name of packer being used 		  */
  STRPTR xp_PackerLongName; /* Descriptive name of packer being used 	  */
  STRPTR xp_Activity;       /* Packing/unpacking message		  */
  STRPTR xp_FileName;       /* Name of file being processed, if available */
  ULONG	 xp_CCur;           /* Amount of packed data already processed	  */
  ULONG	 xp_UCur;           /* Amount of unpacked data already processed  */
  ULONG	 xp_ULen;	    /* Amount of unpacked data in file		  */
  LONG	 xp_CF;		    /* Compression factor so far		  */
  ULONG	 xp_Done;           /* Percentage done already			  */
  ULONG	 xp_Speed;          /* Bytes per second, from beginning of stream */
  ULONG	 xp_Reserved[8];    /* For future use				  */
};
#define XPKPROG_START	1
#define XPKPROG_MID	2
#define XPKPROG_END	3

/***************************************************************************
 *
 *
 *       The file info block
 *
 */

struct XpkFib {
	ULONG	xf_Type		; /* Unpacked, packed, archive?   */
	ULONG	xf_ULen		; /* Uncompressed length          */
	ULONG	xf_CLen		; /* Compressed length            */
	ULONG	xf_NLen		; /* Next chunk len               */
	ULONG	xf_UCur		; /* Uncompressed bytes so far    */
	ULONG	xf_CCur		; /* Compressed bytes so far      */
	ULONG	xf_ID		; /* 4 letter ID of packer        */
	UBYTE	xf_Packer[6]	; /* 4 letter name of packer      */
	UWORD	xf_SubVersion	; /* Required sublib version      */
	UWORD	xf_MasVersion	; /* Required masterlib version   */
	ULONG	xf_Flags	; /* Password?                    */
	UBYTE	xf_Head[16]	; /* First 16 bytes of orig. file */
	LONG	xf_Ratio	; /* Compression ratio            */
	ULONG	xf_Reserved[8]	; /* For future use               */
};

#define XPKTYPE_UNPACKED 0        /* Not packed                   */
#define XPKTYPE_PACKED   1        /* Packed file                  */
#define XPKTYPE_ARCHIVE  2        /* Archive                      */

#define XPKFLAGS_PASSWORD 1       /* Password needed              */
#define XPKFLAGS_NOSEEK   2       /* Chunks are dependent         */
#define XPKFLAGS_NONSTD   4       /* Nonstandard file format      */

/***************************************************************************
 *
 *
 *       The error messages
 *
 */

#define XPKERR_OK	  0
#define XPKERR_NOFUNC	   -1	/* This function not implemented	*/
#define XPKERR_NOFILES	   -2	/* No files allowed for this function	*/
#define XPKERR_IOERRIN	   -3	/* Input error happened, look at Result2*/
#define XPKERR_IOERROUT	   -4	/* Output error happened,look at Result2*/
#define XPKERR_CHECKSUM	   -5	/* Check sum test failed		*/
#define XPKERR_VERSION	   -6	/* Packed file's version newer than lib */
#define XPKERR_NOMEM	   -7	/* Out of memory			*/
#define XPKERR_LIBINUSE	   -8	/* For not-reentrant libraries		*/
#define XPKERR_WRONGFORM   -9	/* Was not packed with this library	*/
#define XPKERR_SMALLBUF	   -10	/* Output buffer too small		*/
#define XPKERR_LARGEBUF	   -11	/* Input buffer too large		*/
#define XPKERR_WRONGMODE   -12	/* This packing mode not supported	*/
#define XPKERR_NEEDPASSWD  -13	/* Password needed for decoding		*/
#define XPKERR_CORRUPTPKD  -14	/* Packed file is corrupt		*/
#define XPKERR_MISSINGLIB  -15	/* Required library is missing		*/
#define XPKERR_BADPARAMS   -16	/* Caller's TagList was screwed up	*/
#define XPKERR_EXPANSION   -17	/* Would have caused data expansion	*/
#define XPKERR_NOMETHOD    -18	/* Cannot find requested method		*/
#define XPKERR_ABORTED     -19	/* Operation aborted by user		*/
#define XPKERR_TRUNCATED   -20	/* Input file is truncated		*/
#define XPKERR_WRONGCPU    -21	/* Better CPU required for this library	*/
#define XPKERR_PACKED      -22	/* Data are already XPacked		*/
#define XPKERR_NOTPACKED   -23	/* Data not packed			*/
#define XPKERR_FILEEXISTS  -24	/* File already exists			*/
#define XPKERR_OLDMASTLIB  -25	/* Master library too old		*/
#define XPKERR_OLDSUBLIB   -26	/* Sub library too old			*/
#define XPKERR_NOCRYPT     -27	/* Cannot encrypt			*/
#define XPKERR_NOINFO      -28	/* Can't get info on that packer	*/
#define XPKERR_LOSSY       -29	/* This compression method is lossy	*/
#define XPKERR_NOHARDWARE  -30	/* Compression hardware required	*/
#define XPKERR_BADHARDWARE -31	/* Compression hardware failed		*/
#define XPKERR_WRONGPW     -32	/* Password was wrong			*/
#define XPKERR_UNKNOWN	   -33	/* unknown error cause			*/

#define XPKERRMSGSIZE	80	/* Maximum size of an error message	*/

/***************************************************************************
 *
 *
 *     The XpkQuery() call
 *
 */

struct XpkPackerInfo {
	UBYTE	xpi_Name[24]       ; /* Brief name of the packer          */
	UBYTE	xpi_LongName[32]   ; /* Full name of the packer           */
	UBYTE	xpi_Description[80]; /* One line description of packer    */
	ULONG	xpi_Flags          ; /* Defined below                     */
	ULONG	xpi_MaxChunk       ; /* Max input chunk size for packing  */
	ULONG	xpi_DefChunk       ; /* Default packing chunk size        */
	UWORD	xpi_DefMode        ; /* Default mode on 0..100 scale      */
};

/* Defines for Flags */
#define XPKIF_PK_CHUNK   0x00001 /* Library supplies chunk packing       */
#define XPKIF_PK_STREAM  0x00002 /* Library supplies stream packing      */
#define XPKIF_PK_ARCHIVE 0x00004 /* Library supplies archive packing     */
#define XPKIF_UP_CHUNK   0x00008 /* Library supplies chunk unpacking     */
#define XPKIF_UP_STREAM  0x00010 /* Library supplies stream unpacking    */
#define XPKIF_UP_ARCHIVE 0x00020 /* Library supplies archive unpacking   */
#define XPKIF_HOOKIO     0x00080 /* Uses full Hook I/O                   */
#define XPKIF_CHECKING   0x00400 /* Does its own data checking           */
#define XPKIF_PREREADHDR 0x00800 /* Unpacker pre-reads the next chunkhdr */
#define XPKIF_ENCRYPTION 0x02000 /* Sub library supports encryption      */
#define XPKIF_NEEDPASSWD 0x04000 /* Sub library requires encryption      */
#define XPKIF_MODES      0x08000 /* Sub library has different modes      */
#define XPKIF_LOSSY      0x10000 /* Sub library does lossy compression   */

struct XpkMode {
  struct XpkMode *xm_Next;   /* Chain to next descriptor for ModeDesc list*/
  ULONG   xm_Upto;	     /* Maximum efficiency handled by this mode   */
  ULONG   xm_Flags;	     /* Defined below                             */
  ULONG   xm_PackMemory;     /* Extra memory required during packing      */
  ULONG   xm_UnpackMemory;   /* Extra memory during unpacking             */
  ULONG   xm_PackSpeed;      /* Approx packing speed in K per second      */
  ULONG   xm_UnpackSpeed;    /* Approx unpacking speed in K per second    */
  UWORD   xm_Ratio;	     /* CF in 0.1% for AmigaVision executable     */
  UWORD   xm_ChunkSize;	     /* Desired chunk size in K (!!) for this mode*/
  UBYTE   xm_Description[10];/* 7 character mode description              */
};

/* Defines for XpkMode.Flags */
#define XPKMF_A3000SPEED 0x00000001 /* Timings on A3000/25                */
#define XPKMF_PK_NOCPU   0x00000002 /* Packing not heavily CPU dependent  */
#define XPKMF_UP_NOCPU   0x00000004 /* Unpacking... (i.e. hardware modes) */

#define MAXPACKERS 100

struct XpkPackerList {
	ULONG	xpl_NumPackers;
	UBYTE	xpl_Packer[MAXPACKERS][6];
};

/***************************************************************************
 *
 *
 *     The XpkOpen() type calls
 *
 */

#define XPKLEN_ONECHUNK 0x7fffffff

/***************************************************************************
 *
 *
 *     The XpkPassRequest() call (library version 4)
 *
 */

#define XPK_PassChars	XTAG(0x50) /* which chars should be used */
#define XPK_PasswordBuf XTAG(0x51) /* buffer to write password to */
#define XPK_PassBufSize XTAG(0x52) /* size of password buffer */
#define XPK_Key16BitPtr	XTAG(0x53) /* pointer to UWORD var for key data */
#define XPK_Key32BitPtr	XTAG(0x54) /* pointer to ULONG var for key data */
#define XPK_PubScreen	XTAG(0x55) /* pointer to struct Screen */
#define XPK_PassTitle	XTAG(0x56) /* Text shown in Screen title */
#define XPK_TimeOut	XTAG(0x57) /* Timeout time of requester in seconds */

/* XPKPASSFF defines for XPK_PassChars. Do not use. Use XPKPASSFLG defines */

#define XPKPASSFF_30x39		(1 << 0)	/* all numbers		*/
#define XPKPASSFF_41x46		(1 << 1)	/* chars 'A' to 'F'	*/
#define XPKPASSFF_61x66		(1 << 2)	/* chars 'a' to 'f'	*/
#define XPKPASSFF_47x5A		(1 << 3)	/* chars 'G' to 'Z'	*/
#define XPKPASSFF_67x7A		(1 << 4)	/* chars 'g' to 'z'	*/
#define XPKPASSFF_20		(1 << 5)	/* space character	*/
#define XPKPASSFF_SPECIAL7BIT	(1 << 6)
 /* all chars 0x20 to 0x7F without above defined */
#define XPKPASSFF_C0xDE		(1 << 7)	/* upper special chars	*/
#define XPKPASSFF_DFxFF		(1 << 8)	/* lower special chars	*/
#define XPKPASSFF_SPECIAL8BIT	(1 << 9)	/* special 8Bit chars	*/

/* flags for XPK_PassChars, XPKPASSFLG_PRINTABLE is default */

/*
NUMERIC		: numbers
HEXADECIMAL	: hex numbers
ALPHANUMERIC	: numbers and letters
INTALPHANUM	: numbers and international letters
ASCII7		: 7 Bit ASCII
PRINTABLE	: all characters
*/

#define XPKPASSFLG_NUMERIC	XPKPASSFF_30x39
#define XPKPASSFLG_HEXADECIMAL	(XPKPASSFF_30x39|XPKPASSFF_41x46|XPKPASSFF_61x66)
#define XPKPASSFLG_ALPHANUMERIC	(XPKPASSFLG_HEXADECIMAL|XPKPASSFF_47x5A|XPKPASSFF_67x7A)
#define XPKPASSFLG_INTALPHANUM	(XPKPASSFLG_ALPHANUMERIC|XPKPASSFF_C0xDE|XPKPASSFF_DFxFF)
#define XPKPASSFLG_ASCII7	(XPKPASSFLG_ALPHANUMERIC|XPKPASSFF_SPECIAL7BIT)
#define XPKPASSFLG_PRINTABLE	(XPKPASSFLG_INTALPHANUM|XPKPASSFF_SPECIAL7BIT|XPKPASSFF_SPECIAL8BIT|XPKPASSFF_20)

/***************************************************************************
 *
 *
 *     The XpkAllocObject() call (library version 4)
 *
 * use this always with library version >= 4, do NO longer allocate the
 * structures yourself
 *
 */

#define XPKOBJ_FIB		0	/* XpkFib structure */
#define XPKOBJ_PACKERINFO	1	/* XpkPackerInfo structure */
#define XPKOBJ_MODE		2	/* XpkMode structure */
#define XPKOBJ_PACKERLIST	3	/* XpkPackerList structure */

#endif /* XPK_XPK_H */
