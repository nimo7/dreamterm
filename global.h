#ifndef GLOBAL_H
#define GLOBAL_H

#define __USE_SYSBASE

/* ansi */
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/* mui  */
#include <libraries/mui.h>
#include <proto/muimaster.h>

/* system */
#include <exec/lists.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <exec/nodes.h>
#include <exec/types.h>
#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <datatypes/pictureclass.h>
#include <devices/serial.h>
#include <devices/timer.h>
#include <dos/datetime.h>
#include <dos/dos.h>
#include <dos/stdio.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>
#include <graphics/displayinfo.h>
#include <graphics/videocontrol.h>
#include <intuition/intuition.h>
#include <libraries/asl.h>
#include <libraries/locale.h>
#include <libraries/xem.h>
#include <utility/tagitem.h>
#include <workbench/icon.h>
#include <workbench/startup.h>

/* protos */
#include <clib/alib_protos.h>
#include <clib/alib_stdio_protos.h>
#include <proto/asl.h>
#include <proto/asyncio.h>
#include <proto/datatypes.h>
#include <proto/diskfont.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/gadtools.h>
#include <proto/graphics.h>
#include <proto/icon.h>
#include <proto/intuition.h>
#include <proto/keymap.h>
#include <proto/locale.h>
#include <proto/utility.h>
#include <proto/xem_proto.h>
#include <proto/xpkmaster.h>
#include <libraries/xproto.h>

/* mine */
#include <mine/priv.h>
#include <mine/toolkit.h>

#define TAGSTART    8               /* Monta tagia allokoidaan ensin    */
#define TAGALLOC    16              /* Monta tagia allokoidaan kerralla */
#define STRTAG      TAG_USER+0      /* string tags    */
#define STRMIN      TAG_USER+0
#define STRMAX      TAG_USER+9999
#define VALTAG      TAG_USER+10000  /* value tags     */
#define VALMIN      TAG_USER+10000
#define VALMAX      TAG_USER+19999
#define DATTAG      TAG_USER+20000  /* data tags      */
#define DATMIN      TAG_USER+20000
#define DLOTAG      TAG_USER+30000  /* long list      */
#define DLOMIN      TAG_USER+30000
#define DLOMAX      TAG_USER+39999
#define DSTTAG      TAG_USER+40000  /* string list    */
#define DSTMIN      TAG_USER+40000
#define DSTMAX      TAG_USER+49999
#define DATMAX      TAG_USER+99999 
#define COMTAG      TAG_USER+100000 /* command tags   */
#define COMMIN      TAG_USER+100000
#define COMMAX      TAG_USER+109999

#define STRSIZE     256             /* Size for fixed strings         */
#define REQ_CANCEL  0               /* Return codes for ask(xx)_done  */
#define REQ_OK      1
#define REQ_RUN     2
#define CSID        0x44544353      /* DTCS */
#define CUID        0x44544355      /* DTCU */
#define DLID        0x4454444C      /* DTDL */
#define G1ID        0x44544731      /* DTG1 */
#define G2ID        0x44544732      /* DTG2 */
#define SLID        0x4454534C      /* DTSL */
#define PBID        0x44545042      /* DTPB */
#define TBID        0x44545442      /* DTTB */
#define XMID        0x4454584D      /* DTXM */
#define XPID        0x44545850      /* DTXP */

#define CUR(x)      ( (x <= STR_PB_LAST) ? gl_cur_qs[x-STR_PB_FIRST] : (x <= VAL_PB_LAST) ? gl_cur_qv[x-VAL_PB_FIRST] : gl_cur_qd[x-DAT_PB_FIRST] )
#define Cur(x)      CUR(x)
#define cur(x)      CUR(x)
#define STR_DEF(x)  gl_str_ultimate[x-(STRTAG)]
#define VAL_DEF(x)  gl_val_ultimate[x-(VALTAG)]
#define DAT_DEF(x)  gl_dat_ultimate[x-(DATTAG)]

/* Kaikille ikkunoille/informaatiolle yhteiset 'metodit' */

#define DT_INIT      (0)	/*				 init module		*/
#define DT_NOTIFY		 (1)	/*				 make notifys		*/
#define DT_DEL			 (2)	/*				 kill module		*/
#define DT_OPEN			 (3)	/*				 open window		*/
#define DT_HANDLE		 (4)	/* [id   ] handle events	*/
#define DT_CLOSE		 (5)	/*				 close window		*/
#define DT_LOAD			 (6)	/* [name ] load infolist	*/
#define DT_SAVE			 (7)	/* [name ] save infolist	*/
#define DT_LINK			 (8)	/* [func ] callback 			*/
#define	DT_FIND			 (9)	/* [name ] find info			*/
#define	DT_OC				(10)	/*				 open/close			*/
#define	DT_GET			(11)	/* [num  ] find info			*/
#define	DT_UPDATE		(12)	/*			   update window	*/
#define	DT_LOCK			(13)	/* [info ] lock info			*/
#define	DT_UNLOCK   (14)  /* [info ] unlock info		*/
#define	DT_CLEAR		(15)  /* 				 clear infos		*/
#define	DT_COUNT		(16)  /*         count infos		*/

/* Eri osien 'metodeita' */

#define DI_DIAL 		(64)	/*         check success  */
#define DI_DIALNUM  (65)	/* [phone] check success  */
#define DI_CHECK		(66)	/*         check success  */
#define	FT_ADDFILE	(64)	/* [file ] add file       */
#define	FT_UPLOADRQ	(65)	/* [proto] upload & req   */
#define	FT_UPLOADID	(66)	/* [proto] upload list    */
#define	FT_UPLOAD		(67)	/* [proto] upload queue   */
#define	FT_DNLOAD		(68)	/* [proto] download       */
#define	FT_ABORT		(69)  /*         abort xfer     */
#define	FT_SETUP		(70)  /*         show setup     */
#define	PB_DESELECT	(64)	/* [entry]	*/
#define	PB_ADDENTRY	(65)	/* [entry]	*/
#define	PB_REDRAW		(66)	/* [entry]	*/
#define	PB_SELECT	  (67)  /* [name ]  */
#define PE_EDIT			(64)	/* [entry]	*/
#define	PE_NEW			(65)
#define	PE_CLONE		(66)	/* [entry]	*/
#define	PE_PICK			(67)
#define	PE_CURRENT	(68)
#define	DL_ADD			(64)	/* [phone]	*/
#define DL_CMP			(65)	/* [phone]	*/
#define	DL_ADDREM		(66)	/* [entry]	*/
#define IM_START		(64)
#define TB_ENABLE   (64)  /* [bar  ]  */
#define TB_DISABLE  (65)  /* [bar  ]  */

#define CR_CRYPT    (64)  /* [name,name] crypt file   */
#define CR_DECRYPT  (65)  /* [name,name] decrypt file */
#define CR_DELETE   (66)  /* [name]      delete file  */

/*** Stringien etsintään RXD-flowista (PrepFlow initoi) ***/

struct FlowString
{
  strptr  string;     /* Tähän stringi (user) */
  bool    hit;        /* Löytynyt?            */
  word    hitlen;     /* Paljon löydetty      */
  /* MUISTA YKSI TYHJÄ ENTRY! */
};

/*** structure for loaded picture ***/

struct Picture
{
  struct BitMapHeader bmhd;  /* format and infos */
  struct BitMap *bmap;       /* bitmap */
  ULONG *palette;            /* color table in LoadRGB32() format */
  LONG  palette_size;        /* mem usage */
  LONG  palette_entries;     /* number of colors */
  ULONG display_ID;          /* video mode */
  UBYTE *author;             /* author info */
  UBYTE *copyright;          /* copyright info */
  UBYTE *annotation;         /* other info */
/*
  LONG author_size;          /* mem usage (unused) */
  LONG copyright_size;       /* mem usage (unused) */
  LONG annotation_size;      /* mem usage (unused) */
*/
};

/*** taglist base ***/

struct TagBase
{
  struct TagItem *tagdata;
  uword           tagfree;
  uword           tagmax;
};

/***  standard information node ***/

struct Info
{
  struct Node     node;  /* name                */
  struct TagBase  tags;  /* tags                */
  ulong           store; /* quick store         */
  unsigned LOCK   : 1;   /* locked from editing */
  unsigned EDIT   : 1;   /* user editing        */
  unsigned USED   : 1;   /* used in current     */
  unsigned SELECT : 1;   /* selected            */
  unsigned NODISK : 1;   /* don't write to disk */
};

/*** save/load tag structures ***/

#define TAGID     0x54414721  /* TAG!       */
#define TAGVER    2           /* Version    */
#define INFOVER   16          /* Info node  */

/* Levyllä olevan infolistan global header */
struct Header
{
  ulong id;     /* list id        */
  word  ver;    /* version        */
  word  flags;  /* unused         */
  long  size;   /* unused         */
  long  count;  /* infonode count */
};

/* info node header
  ulong node name size
  ...   node name
  ulong extension data size (currently 0)
  tagdata
*/

/* Levyllä olevan taglistan header */
struct TagEntry
{
  ulong id;         /* TAG!      */
  word  version;    /* version   */
  word  flags;      /* unused    */
  long  size;       /* unused    */
  /* tags: 0000 id (TAG_END terminates, remember to supply data too!)
           0004 if VALTAG then data
                if STRTAG then str size (without null byte which is _not_ saved!)
                if DATTAG then size
           .... possible str/dat data
  */
};

/*
struct SubStr
{
  ulong tag;
  ulong tagsize;
};
*/

/*** phonebook entry tags ***/

/* Tag defination limits */

#define STR_PB_FIRST    (STRTAG+00)
#define VAL_PB_FIRST    (VALTAG+00)
#define DAT_PB_FIRST    (DATTAG+00)

/* Some value tag definations */

#define PAR_NONE        0x01
#define PAR_EVEN        0x02
#define PAR_ODD         0x04
#define PAR_MARK        0x08
#define PAR_SPACE       0x10

#define FLOW_NONE       0x01
#define FLOW_XONXOFF    0x02
#define FLOW_RTSCTS     0x04

#define STAT_NONE       0x01
#define STAT_BAR        0x02
#define STAT_WINDOW     0x04

#define CAP_NONE        0x00
#define CAP_TEXT        0x01
#define CAP_RAW         0x02

#define BAR_HOR         0x00
#define BAR_VER         0x01

/*** tags ***/

#define PB_Name					(STRTAG+0)
#define PB_Phone				(STRTAG+1)
#define PB_PassWord			(STRTAG+2)
#define PB_InitString		(STRTAG+3)
#define PB_Comment			(STRTAG+4)
#define PB_DialString		(STRTAG+5)
#define PB_Func1				(STRTAG+6)
#define PB_Func2				(STRTAG+7)
#define PB_Func3				(STRTAG+8)
#define PB_Func4				(STRTAG+9)
#define PB_Func5				(STRTAG+10)
#define PB_Func6				(STRTAG+11)
#define PB_Func7				(STRTAG+12)
#define PB_Func8				(STRTAG+13)
#define PB_Func9				(STRTAG+14)
#define PB_Func10				(STRTAG+15)
#define PB_Charset			(STRTAG+16)
#define PB_Protocol			(STRTAG+17)
#define PB_SerName			(STRTAG+18)
#define PB_LogName			(STRTAG+19)
#define PB_UpPath				(STRTAG+20)
#define PB_DnPath				(STRTAG+21)
#define PB_PhonePre			(STRTAG+22)
#define	PB_Xem					(STRTAG+23)
#define	PB_CryptLib			(STRTAG+24)
#define	PB_PackLib			(STRTAG+25)
#define	PB_CryptKey			(STRTAG+26)
#define	PB_TempDir			(STRTAG+27)
#define	PB_DnFiles			(STRTAG+28)
#define	PB_Hangup				(STRTAG+29)
#define	PB_NoCarrier		(STRTAG+30)
#define	PB_NoDialTone		(STRTAG+31)
#define	PB_Connect			(STRTAG+32)
#define	PB_Ringing			(STRTAG+33)
#define	PB_Busy					(STRTAG+34)
#define	PB_Ok						(STRTAG+35)
#define	PB_Error				(STRTAG+36)
#define	PB_DialSuffix		(STRTAG+37)
#define	PB_DialAbort		(STRTAG+38)
#define	PB_PubScreen		(STRTAG+39)
#define PB_obsolete2    (STRTAG+40) /* (obsolete)                 */
#define PB_Toolbar      (STRTAG+41)
#define PB_StatusLine   (STRTAG+42)
#define STR_PB_LAST     (STRTAG+42)
#define PB_DteRate      (VALTAG+0)  /* value: 1-292000            */
#define PB_FlowControl	(VALTAG+1)	/* bits : FLOW_NONE/RTSCTS    */
#define PB_DataBits			(VALTAG+2)	/* value: 7-9							    */
#define PB_Parity				(VALTAG+3)	/* bits : PAR_NONE/ODD/EVEN   */
#define PB_StopBits			(VALTAG+4)	/* value: 1-2		*/
#define PB_FixRate			(VALTAG+5)	/* bool					*/
#define PB_LogCall			(VALTAG+6)	/* bool					*/
#define	PB_Status				(VALTAG+7)	/* bits	: STAT_NONE/BAR/WIN   */
#define	PB_Aga					(VALTAG+8)	/* bool					*/
#define PB_CostFirst		(VALTAG+9)	/* value				*/
#define PB_CostRest			(VALTAG+10)	/* value				*/
#define PB_HardScr			(VALTAG+11)	/* bool					*/
#define PB_HardSer			(VALTAG+12)	/* bool					*/
#define PB_ModeId				(VALTAG+13)	/* value: (graphics/modeid.h)	*/
#define PB_Depth				(VALTAG+14)	/* value: 1-4		*/
#define PB_Columns			(VALTAG+15)	/* value: 80-256*/
#define PB_Lines				(VALTAG+16)	/* value: 24-256*/
#define PB_Wb						(VALTAG+17)	/* bool					*/
#define PB_SerUnit			(VALTAG+18)	/* value				*/
#define	PB_Capture			(VALTAG+19)	/* value: CAP_NONE/TEXT/RAW		*/
#define	PB_SerBufSize   (VALTAG+20)	/* value				*/
#define	PB_RxdPopup     (VALTAG+21)	/* bool					*/
#define	PB_FullDuplex   (VALTAG+22)	/* bool					*/
#define	PB_KeepWb       (VALTAG+23)	/* bool					*/
#define	PB_Priority     (VALTAG+24)	/* value: -128 to 127					*/
#define	PB_cr2crlf      (VALTAG+25) /* bool					*/
#define	PB_lf2crlf			(VALTAG+26) /* bool					*/
#define	PB_RtsBlit			(VALTAG+27)	/* bool					*/
#define	PB_HLines				(VALTAG+28)	/* value 24-32	*/
#define	PB_AutoXfer			(VALTAG+29)	/* bool					*/
#define	PB_QuietXfer		(VALTAG+30)	/* bool					*/
#define	PB_WaitEntry		(VALTAG+31)	/* value				*/
#define	PB_WaitDial			(VALTAG+32)	/* value				*/
#define PB_Emulation		(VALTAG+33)	/* value: (emulation/emuext.h)*/
#define	PB_ANSIWrap			(VALTAG+34)	/* bool					*/
#define	PB_ANSIKillBs		(VALTAG+35)	/* bool					*/
#define	PB_ANSIStrip		(VALTAG+36)	/* bool         */
#define	PB_ANSICls			(VALTAG+37)	/* bool					*/
#define	PB_ANSICr2crlf	(VALTAG+38)	/* bool					*/
#define	PB_Double				(VALTAG+39)	/* bool					*/
#define	PB_obsolete3		(VALTAG+40)	/* (obsolete)		*/
#define	PB_Safe					(VALTAG+41)	/* bool					*/
#define PB_CallLast			(VALTAG+42)	/* value				*/
#define PB_CallCost			(VALTAG+43)	/* value				*/
#define PB_CallTime			(VALTAG+44)	/* value				*/
#define PB_CallCount		(VALTAG+45)	/* value				*/
#define PB_DTRHangup		(VALTAG+46)	/* bool					*/
#define	PB_ANSIPc				(VALTAG+47)	/* bool					*/
#define	PB_Crypt				(VALTAG+48)	/* bool					*/
#define	PB_obsolete1		(VALTAG+49)	/* (obsolete)		*/
#define	PB_Review				(VALTAG+50)	/* bool					*/
#define	PB_ReviewSize		(VALTAG+51)	/* value				*/
#define	PB_CallTxd			(VALTAG+52)	/* value				*/
#define	PB_CallRxd			(VALTAG+53)	/* value				*/
#define	PB_PreScroll		(VALTAG+54)	/* value				*/
#define	PB_BorderBlank	(VALTAG+55)	/* bool					*/
#define	PB_ConX					(VALTAG+56)	/* value				*/
#define	PB_ConY					(VALTAG+57)	/* value				*/
#define	PB_ForceConXY		(VALTAG+58)	/* bool					*/
#define	PB_Overscan			(VALTAG+59)	/* value:	intuition/screens.h	*/
#define	PB_AutoScroll		(VALTAG+60)	/* bool					*/
#define	PB_Freeze				(VALTAG+61)	/* value				*/
#define	PB_Dispose 			(VALTAG+62)	/* bool 				*/
#define	PB_Baron  			(VALTAG+63)	/* bool 				*/
#define	PB_Barpos       (VALTAG+64) /* value: BAR_HOR/VER */
#define	PB_SerShared    (VALTAG+65) /* bool         */
#define	VAL_PB_LAST			(VALTAG+65)
#define PB_ANSICol			(DATTAG+00) /* 32bit colors	*/
#define	PB_HEXCol				(DATTAG+01) /* 32bit colors	*/
#define	DAT_PB_LAST			(DATTAG+01)

#define STR_Ultimate \
  "","","","","","ATDT ","","","","","","","","","","","ISO",\
  "ZModem","serial.device","DT_Logfile","","","","","IDEA.100",\
  "LZSS.100","","T:","","+++~~~ATH\\r","NO CARRIER",\
  "NO DIALTONE","CONNECT","RINGING","BUSY","OK","ERROR",\
  "\\r","\\r~","Workbench","","",\
  "%ro %rc$ %06vd %rf %rh %04vf TXD/RXD: %04vb/%04va %6rt/%6rr %7re"

#define VAL_Ultimate \
  38400,FLOW_RTSCTS,8,PAR_NONE,1,TRUE,TRUE,STAT_BAR,TRUE,0,0,FALSE,\
  FALSE,PAL_MONITOR_ID|HIRES_KEY,3,80,24,TRUE,0,CAP_TEXT,16384,TRUE,\
  TRUE,FALSE,0,FALSE,FALSE,FALSE,32,TRUE,FALSE,3,90,EM_ANSI,TRUE,\
  FALSE,TRUE,TRUE,FALSE,FALSE,FALSE,FALSE,0,0,0,0,FALSE,FALSE,FALSE,\
  FALSE,FALSE,32768,0,0,0,FALSE,640,256,FALSE,OSCAN_TEXT,TRUE,0,\
  TRUE,FALSE,BAR_HOR,FALSE

#define DANSICol			192,0x00000000,0x00000000,0x00000000,0xcccccccc,0x00000000,0x00000000,\
													0x00000000,0xbbbbbbbb,0x00000000,0xbbbbbbbb,0xaaaaaaaa,0x00000000,\
													0x00000000,0x00000000,0xcccccccc,0xbbbbbbbb,0x00000000,0xbbbbbbbb,\
													0x00000000,0xdddddddd,0xdddddddd,0xdddddddd,0xdddddddd,0xdddddddd,\
													0x66666666,0x66666666,0x66666666,0xffffffff,0x00000000,0x00000000,\
                          0x00000000,0xffffffff,0x00000000,0xffffffff,0xffffffff,0x00000000,\
													0x00000000,0x00000000,0xffffffff,0xffffffff,0x00000000,0xffffffff,\
													0x00000000,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff
#define	DHEXCol				24, 0,0,0, -1,-1,-1

#define ENV_TAGS \
  PB_Priority   ,PB_Safe       ,PB_InitString ,PB_TempDir    ,\
  PB_CryptLib   ,PB_PackLib    ,PB_Crypt      ,PB_Review     ,\
  PB_ReviewSize ,PB_Capture    ,PB_Dispose    ,PB_Toolbar    ,\
  PB_Baron      ,PB_Barpos

/*** action 'methods' tags ***/

#define cmd_none            (COMTAG+0)  /* 1.49 */
#define cmd_open_current    (COMTAG+1)
#define cmd_open_profile    (COMTAG+2)
#define cmd_open_xp         (COMTAG+3)
#define cmd_open_cs         (COMTAG+4)
#define cmd_open_xm         (COMTAG+5)
#define cmd_open_tb         (COMTAG+6)
#define cmd_load_default    (COMTAG+7)
#define cmd_save_default    (COMTAG+8)
#define cmd_titlebar        (COMTAG+9)
#define cmd_open_crypt      (COMTAG+10)
#define cmd_about           (COMTAG+11)
#define cmd_quit            (COMTAG+12)
#define cmd_open_phonebook  (COMTAG+13)
#define cmd_new_entry       (COMTAG+14)
#define cmd_load_phonebook  (COMTAG+15)
#define cmd_save_phonebook  (COMTAG+16)
#define cmd_dial_number     (COMTAG+17)
#define cmd_dial_next       (COMTAG+18)
#define cmd_send_password   (COMTAG+19)
#define cmd_upload          (COMTAG+20)
#define cmd_uploadlist      (COMTAG+21)
#define cmd_download        (COMTAG+22)
#define cmd_send_conv       (COMTAG+23)
#define cmd_send_no_conv    (COMTAG+24)
#define cmd_abort_send      (COMTAG+25)
#define cmd_open_uplist     (COMTAG+26)
#define cmd_open_review     (COMTAG+27)
#define cmd_start_capture   (COMTAG+28)
#define cmd_hangup          (COMTAG+29)
#define cmd_release_serial  (COMTAG+30)
#define cmd_load_current    (COMTAG+31) /* 1.50 */
#define cmd_save_current    (COMTAG+32)
#define cmd_load_profile    (COMTAG+33)
#define cmd_save_profile    (COMTAG+34)

/*** charset ***/

#define CS_FontName (STRTAG+1000+0)
#define CS_Xsize    (VALTAG+1000+1)
#define CS_Ysize    (VALTAG+1000+2)
#define CS_Txd      (DATTAG+1000+3)
#define CS_Rxd      (DATTAG+1000+4)
#define CS_Cap      (DATTAG+1000+5)

#define CSFontName	(ULONG)"topaz.font"
#define CSXsize			8
#define CSYsize			8
/* txd/rxd/cap not defined */

/*** protocol ***/

#define XP_XprName	(STRTAG+1050+0)
#define XP_Init			(STRTAG+1050+1)
#define XP_Uauto		(STRTAG+1050+2)
#define XP_Dauto		(STRTAG+1050+3)
#define XP_UpCom    (STRTAG+1050+4)
#define XP_DnCom    (STRTAG+1050+5)
#define XP_Type     (VALTAG+1050+0)

#define XPXprName		(ULONG)""
#define XPInit			(ULONG)""
#define XPUauto			(ULONG)""
#define XPDauto			(ULONG)""
#define XPType      (ULONG)0

/*** upload list ***/

#define FI_Size     (VALTAG+1100+0)

/*** xem ***/

#define XM_Lib      (STRTAG+1150+0)
#define XM_Col      (DATTAG+1150+1)
#define XM_Pref     (DATTAG+1150+2)

#define XMLib       (ULONG)""
#define XMCol       192,0x00000000,0x00000000,0x00000000,0xcccccccc,0x00000000,0x00000000,\
                        0x00000000,0xbbbbbbbb,0x00000000,0xbbbbbbbb,0xaaaaaaaa,0x00000000,\
                        0x00000000,0x00000000,0xcccccccc,0xbbbbbbbb,0x00000000,0xbbbbbbbb,\
                        0x00000000,0xdddddddd,0xdddddddd,0xdddddddd,0xdddddddd,0xdddddddd,\
                        0x66666666,0x66666666,0x66666666,0xffffffff,0x00000000,0x00000000,\
                        0x00000000,0xffffffff,0x00000000,0xffffffff,0xffffffff,0x00000000,\
                        0x00000000,0x00000000,0xffffffff,0xffffffff,0x00000000,0xffffffff,\
                        0x00000000,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff
#define XMPref      0

/*** toolbar ***/

#define TB_STR      (STRTAG+1250+0)
#define TB_VAL      (VALTAG+1250+0)
#define TB_DAT      (DATTAG+1250+0)
#define TB_Action   (DLOTAG+1250+0)
#define TB_Button   (DSTTAG+1250+0)
#define TB_Serial   (DSTTAG+1250+1)
#define TB_Command  (DSTTAG+1250+2)

#define TBIcons     0

/*** showlist ***/

#define SL_STR      (STRTAG+1300+0)
#define SL_VAL      (VALTAG+1300+0)
#define SL_DAT      (DATTAG+1300+0)
#define SL_tag      (DLOTAG+1300+0)
#define SL_pos      (DSTTAG+1300+0)

/*** droplist ***/

#define DL_STR      (STRTAG+1350+0)
#define DL_VAL      (VALTAG+1350+0)
#define DL_DAT      (DATTAG+1350+0)

/*** MUI interface ID base's ***/

#define CS_First     1  /*** charset     ***/
#define CS_Last    999
#define CR_First  1000  /*** crypt       ***/
#define CR_Last   1999
#define DI_First  2000  /*** dialer      ***/
#define DI_Last   2999
#define DL_First  3000  /*** droplist    ***/
#define DL_Last   3999
#define FI_First  4000  /*** upload list ***/
#define FI_Last   4999
#define FT_First  5000  /*** xfer status ***/
#define FT_Last   5999
#define PB_First  6000  /*** phonebook   ***/
#define PB_Last   6999
#define PE_First  7000  /*** entry       ***/
#define PE_Last   7999
#define SL_First  9000  /*** showlist    ***/
#define SL_Last   9999
#define TE_First 10000  /*** temporary   ***/
#define TE_Last  10999
#define TB_First 11000  /*** toolbar     ***/
#define TB_Last  11999
#define XM_First 13000  /*** xem         ***/
#define XM_Last  13999
#define XO_First 14000  /*** xem opts    ***/
#define XO_Last  14999
#define SLEEP    32000  /*** iconify     ***/
#define WAKE     32001
#define FAKE     32002  /*** reqfake     ***/

/*
 * Async driver_protocol task
 *
 */

enum XPT { /* to child (also normal tags used by info can be used) */
           XP_Remove =1    ,XP_Upload       ,XP_Download     ,XP_Abort        ,
           XP_Addfile      ,XP_Clearfiles   ,

           /* to main */
           XP_sread        ,XP_swrite       ,XP_sflush       ,XP_squery       ,
           XP_chkabort     ,XP_finished     ,XP_openwindow   ,XP_closewindow  ,
           XP_update       ,XP_seroff       ,XP_seron        ,XP_double       ,
           XP_fopen        ,XP_fread        ,XP_fwrite       ,XP_fclose       };

struct XP_Msg 
{
  struct Message msg;
  ulong          cmd;
  ulong          arg;
  ulong          arg2;
  ulong          arg3;
  ulong          result;
};

struct XP_Sub
{
  struct Process *child;    /* child process created               */
  struct MsgPort *ch_port;  /* allocated by child (msg to child)   */
  struct MsgPort *ch_reply; /* allocated by main  (rep from child) */
  struct MsgPort *mp_port;  /* allocated by main  (msg to main)    */
  struct MsgPort *mp_reply; /* allocated by child (rep from main)  */
  struct XP_Msg   ch_msg;   /* message to child                    */
  struct XP_Msg   mp_msg;   /* message to main                     */
};

void __asm driver_protocol(void);

/*
 * Async asl requesters for inside classes (or whatever)
 *
 */

enum CL_STUFF { CL_Object = 1, CL_Method };

void  cl_remreq(ulong);
ulong cl_addreq(ulong, ...);

/*
 * Emulations
 *
 */

#define EM_ANSI 0 /* emulation type */
#define EM_HEX  1
#define EM_XEM  2

void  __asm EmToUse (register __d0 ULONG);
BOOL  __asm EmInit  (register __a0 APTR);
BOOL  __asm EmRemove(register __a0 APTR);
BOOL  __asm EmReset (register __a0 APTR);
BOOL  __asm EmChange(register __a0 APTR);
void  __asm EmPrint (register __a0 APTR, register __d0 ULONG);
ulong __asm EmStrip (register __a0 APTR, register __a1 APTR, register __d0 ULONG);

/* emu_ansi.asm */

#define ANSI_INIT       0x001
#define ANSI_WRAP       0x002
#define ANSI_KILLBS     0x004
#define ANSI_CRLF       0x008
#define ANSI_STRIPBOLD  0x010
#define ANSI_CLRHOME    0x040

/* emu_hex.asm  */

#define HEX_INIT        0x001

/*
 * Screen stuff
 *
 */

#define SCR_HARD  TRUE
#define SCR_SOFT  FALSE

void __asm ScreenToUse      (register __d0 ulong);
bool __asm ScreenInit       (void);
bool __asm ScreenRemove     (void);
bool __asm ScreenChange     (register __a0 aptr);
bool __asm ScreenOn         (void);
bool __asm ScreenOff        (void);
void __asm ScreenPrint      (register __a0 aptr, register __d0 WORD);
void __asm StatPrint        (register __a0 aptr);
void __asm ScreenDown       (void);
void __asm ScreenUp         (void);
void __asm ScreenClearInLine(register __d1 word,register __d0 word);
void __asm ScreenCopyInLine (register __d1 word,register __d2 word,register __d0 word);
void __asm ScreenAreaClear  (register __d0 word,register __d1 word);
void __asm ScreenAreaCopy   (register __d0 word,register __d2 word,register __d1 word);
void __asm CursorOn         (void);
void __asm CursorOff        (void);
void __asm Screen_addcol    (register __a0 ulong *, register __d0 word);
void __asm Screen_remcol    (void);
void __asm ScrollDown       (register __d0 long);

/* updated by emulation and passed to scr_hw/sw routines */
struct ScreenInfo
{
  word  xpos;       /* current x position (0-n) */
  word  ypos;       /* current y position (0-n) */
  word  columns;    /* current columns          */
  word  columnsdbf; /* current columns - 1      */
  word  lines;      /* current lines            */
  word  linesdbf;   /* current lines -1         */
  word  forecol;    /* current foreground color */
  word  backcol;    /* current background color */
  word  fontstate;  /* graphics/text.h          */
  UBYTE inverse;    /* inverse video            */
  UBYTE nil;
};

/*
 * Serial stuff
 *
 */

#define SER_HARD  TRUE
#define SER_SOFT  FALSE

void        __asm SerialToUse	(register __d0 ulong);
ulong       __asm InitSerial	(void);
void        __asm RemoveSerial(void);
void				__asm ResetInfo		(void);
BOOL				__asm InsertTXD		(register __a0 UBYTE *, register __d0 LONG);
BOOL				__asm CheckTXD		(void);
BOOL				__asm WaitTXD			(void);
void				__asm BreakTXD		(void);
struct RXD *__asm GetRXD			(void);
BOOL				__asm SetPort			(void);
struct RXD *__asm GetTimedRXD	(register __d0 LONG, register __d1 LONG);
BOOL				__asm CheckCTS		(void);
BOOL				__asm CheckRTS		(void);
BOOL				__asm CheckCS			(void);
void				__asm SetRTS			(register __d0 BOOL);
void				__asm DropDTR			(void);
LONG				__asm SerQuery		(void);

struct SerialInfo
{
  word txdcps;          /* second slices    */
  word txdcps_peak;     /* vblank slices    */
  word txdcps_avr_peak; /* avr of current   */
  word txdcps_avr;      /* average cps      */
  LONG txdbytes;        /* bytes            */
  LONG txdsec;          /* seconds used     */
  WORD rxdcps;
	WORD rxdcps_peak;
	WORD rxdcps_avr_peak;
	WORD rxdcps_avr;
	LONG rxdbytes;
	LONG rxdsec;
	WORD overrun;
	LONG charbuf;	        /* chars in buffer  */
};

struct RXD
{
  ubyte*  buffer;       /* read bytes       */
  long    size;         /* read size        */
  ubyte*  buffer2;      /* only timedrxd!   */
  long    size2;        /* only timedrxd!   */
};

/*
 * MUI private class definations
 *
 * note: many methods are private and use within class internal update only!
 *
 */

#define BASE  0xffff0000

/* these are available for all classes but not necessarily used */

#define MUIM_GLO_Update     (BASE+   0) /* forces total update */

/*
 * methods available for all 'info node/list' classes, some really are
 * attributes but keeping them as methods due ..eh..
 * 
 * All these user editing windows use MUIC_Window for their superclass.
 *
 * MUIM_INF_Callback : callback routine when list changes!
 * MUIM_INF_Change   : ptr to ulong which is change to 1 when data is changed
 * MUIM_INF_Load     : Load current list/node
 * MUIM_INF_Save     : Save current list/node
 * MUIM_INF_Find     : Find node using name
 * MUIM_INF_Get      : Find node using number (0-n)
 * MUIM_INF_Count    : Count how many nodes
 * MUIM_INF_Lock     : Lock node from editing/deleting etc.
 * MUIM_INF_Unlock   : Unlock node
 * MUIM_INF_Update   : Used by class for updating (DO NOT CALL OUTSIDE!)
 *
 */

#define MUIM_INF_Node       (BASE+1000) /* obsolete */
#define MUIM_INF_List       (BASE+1001) /* obsolete */
#define MUIM_INF_Callback   (BASE+1002)
#define MUIM_INF_Change     (BASE+1003)
#define MUIM_INF_Load       (BASE+1004)
#define MUIM_INF_Save       (BASE+1005)
#define MUIM_INF_Find       (BASE+1006)
#define MUIM_INF_Get        (BASE+1007)
#define MUIM_INF_Count      (BASE+1008)
#define MUIM_INF_Lock       (BASE+1009)
#define MUIM_INF_Unlock     (BASE+1010)
#define MUIM_INF_Update     (BASE+1011) /* PRI */

struct  MUIP_INF_Node       { ulong MethodID; struct Info *info;   };
struct  MUIP_INF_List       { ulong MethodID; struct List *list;   };
struct  MUIP_INF_Callback   { ulong MethodID; void (*callback)(void); };
struct  MUIP_INF_Change     { ulong MethodID; ulong       *mem;    };
struct  MUIP_INF_Load       { ulong MethodID; strptr       name;   };
struct  MUIP_INF_Save       { ulong MethodID; strptr       name;   };
struct  MUIP_INF_Find       { ulong MethodID; strptr       name;   };
struct  MUIP_INF_Get        { ulong MethodID; ulong        num;    };
struct  MUIP_INF_Lock       { ulong MethodID; struct Info *info;   };
struct  MUIP_INF_Unlock     { ulong MethodID; struct Info *info;   };
struct  MUIP_INF_Update     { ulong MethodID; ulong        id;     };

/*
 * private methods for various info windows, they are here only to avoid
 * using same values for different windows. (which time being don't mean
 * anything but may make future enchantments easier)
 *
 */

#define MUIM_XP_Proto_req   (BASE+1500) /* PRI */
#define MUIM_XP_Upcom_req   (BASE+1501) /* PRI */
#define MUIM_XP_Dncom_req   (BASE+1502) /* PRI */
#define MUIM_XP_Load_req    (BASE+1503) /* PRI */
#define MUIM_XP_Save_req    (BASE+1504) /* PRI */
#define MUIM_XP_Append_req  (BASE+1505) /* PRI */
#define MUIM_XP_Select_req  (BASE+1506) /* PRI */

struct  MUIP_INF_Dir_req    { ulong MethodID; struct FileRequester       *req; };
struct  MUIP_INF_File_req   { ulong MethodID; struct FileRequester       *req; };
struct  MUIP_INF_Font_req   { ulong MethodID; struct FontRequester       *req; };
struct  MUIP_INF_Screen_req { ulong MethodID; struct ScreenModeRequester *req; };

/*
 * following classes are standalone and do not have anything to do with
 * those editing window classes!
 *
 */

/* review textarea class (shows review text in area) */

#define MUIA_REV_Lines      (BASE+2000) /* --G */
#define MUIA_REV_DispLines  (BASE+2001) /* --G */
#define MUIA_REV_DispCols   (BASE+2002) /* --G */
#define MUIA_REV_MemoryUsed (BASE+2003) /* --G */
#define MUIA_REV_ShowLine   (BASE+2004) /* ISG */
#define MUIA_REV_ShowCol    (BASE+2005) /* ISG */
#define MUIA_REV_ColorFG    (BASE+2006) /* ISG */
#define MUIA_REV_ColorBG    (BASE+2007) /* ISG */
#define MUIA_REV_MaxLines   (BASE+2008) /* ISG */
#define MUIA_REV_Virtual    (BASE+2009) /* I-- */
#define MUIM_REV_FeedData   (BASE+2010)
#define MUIM_REV_Clear      (BASE+2011)
#define MUIM_REV_Save       (BASE+2012)
#define MUIM_REV_Search     (BASE+2013)

struct  MUIP_REV_FeedData { ulong MethodID; strptr data; ulong size; };
struct  MUIP_REV_Clear    { ulong MethodID; };
struct  MUIP_REV_Save     { ulong MethodID; strptr name;  };
struct  MUIP_REV_Search   { ulong MethodID; strptr text;  };

/* review window class */

#define MUIM_RVI_Search     (BASE+3000) /* PRI */
#define MUIM_RVI_Search_req (BASE+3001) /* PRI */
#define MUIM_RVI_Save       (BASE+3002) /* PRI */
#define MUIM_RVI_Save_req   (BASE+3003) /* PRI */
#define MUIM_RVI_Update     (BASE+3004) /* PRI */

struct  MUIP_RVI_Save_req { ulong MethodID; struct FileRequester *req; };

/* cps class */

#define MUIA_CPS_Max        (BASE+4000) /* IS- */
#define MUIM_CPS_Cps        (BASE+4001)

struct  MUIP_CPS_Cps { ulong MethodID; ulong cps; };

/*
 * Override some mui macros, allowing them to be subroutines
 *
 */

aptr MakeLabel        (strptr, ulong);
aptr MakeButton       (strptr);
aptr MakeCheckmark    (strptr);
aptr MakeCycle        (strptr, strptr *);
aptr MakeRadio        (strptr, strptr *);
aptr MakeSlider       (strptr, long, long);
aptr MakeString       (strptr, long);
aptr MakePopButton    (strptr);
aptr MakeHSpace       (long);
aptr MakeVSpace       (long);
aptr MakeHBar         (long);
aptr MakeVBar         (long);
aptr MakeMenustripNM  (struct NewMenu *, ulong);
aptr MakeMenuitem     (strptr, strptr, ulong, ulong);
aptr MakeBarTitle     (strptr);
aptr MakeNumericButton(strptr, long, long, strptr);

#undef   set
#undef nnset
ulong    set(aptr obj, ulong attr, ulong value);
ulong  nnset(aptr obj, ulong attr, ulong value);

#undef Cycle
#undef KeyButton
#undef SimpleButton
#undef Slider
#undef String

APTR PopFile(void);
APTR SimpleToggle(char *name);
APTR LabelL(char *label);
APTR Label1L(char *label);
APTR Label2L(char *label);

APTR Check(void);
APTR Cycle(char *entries[]);
APTR Int(void);
APTR KeyButton(char *name,char key);
APTR ListView(void);
APTR Rectangle(void);
APTR SimpleButton(char *name);
APTR Slider(ULONG min, ULONG max);
APTR String(ULONG size);
APTR MyText(void);

/* some includes not in this file */
#include "globalext.h"
#include "globalvar.h"
/*
#include "emulation/emuext.h"
#include "screen/screenext.h"	
#include "serial/serialext.h"
*/

#endif
