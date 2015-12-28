/*
**  Global data, accesable for every program part (try to limit it anyway)
*/

#include "global.h"

static const ulong  ansi_ult[]= { DANSICol };
static const ulong  hex_ult[] = { DHEXCol  };
static const char  *str_ult[] = { STR_Ultimate     };
static const ulong  val_ult[] = { VAL_Ultimate     };
static const ulong *dat_ult[] = { ansi_ult,hex_ult };

aptr              gl_app              =0;     /* MUI application object         */
aptr              gl_review           =0;     /* Review window object           */
aptr              gl_charset          =0;     /* Charset window object          */
aptr              gl_protocol         =0;     /* Protocol window object         */
struct IClass    *gl_class_cps        =0;
struct IClass    *gl_class_revarea    =0;
struct IClass    *gl_class_review     =0;
struct IClass    *gl_class_protocol   =0;
struct Catalog   *gl_catalog          =0;     /* Catalog in use                 */
struct Locale    *gl_locale           =0;     /* Locale structure               */
aptr              win_fake            =0;			/* MUI_request() needs this				*/
void            (*gl_mui_id)(ulong)   =0;			/* MUI reqs ID handle							*/
void            (*gl_mui_oc)(void)    =0;			/* MUI reqs open/close						*/
void            (*gl_req)(void)       =0;			/* Requester handle callback			*/
aptr              gl_pool             =0;			/* Memory pool                    */
struct Screen    *gl_conscreen        =0;			/* Screen where console is open   */
aptr              gl_vi               =0;			/* Screen's visual info           */
struct Window    *gl_conwindow				=0;			/* Console window                 */
struct Window    *gl_linwindow				=0;			/* Status line window             */
struct Process   *gl_process            ;     /* DreamTerm process              */
struct XP_Sub    *gl_proc_xp          =0;     /* Protocol driver process        */
ulong             gl_tmp_abort        =0;
char              gl_upstring[STRSIZE]  ;     /* Automatic upload string        */
char              gl_dnstring[STRSIZE]  ;     /* Automatic dnload string        */
struct FlowString gl_autoxfer[]       ={ { gl_upstring,0,0 },{ gl_dnstring,0,0 },{ 0,0,0 } };
bptr              gl_asc_fh           =0;     /* Ascii data send fh							*/
long              gl_asc_mode           ;     /* Ascii data send mode						*/
bptr              gl_cap_fh           =0;     /* Capture fh											*/
ubyte            *gl_cap_buf          =0;     /* Capture buffer									*/
ubyte            *gl_cap_now          =0;     /* Capture write									*/
long              gl_cap_bufsize      =8192;
ubyte            *gl_txdbuffer        =0;     /* Serial send buffer             */
long              gl_txdsize          =4096;  /* Serial send buffer size        */
long              gl_txdleft            ;     /* Serial send buffer size left   */
ULONG							gl_xemsig							;			/* Signal: xem event						  */
ULONG							gl_wakesig						;			/* Signal: no wait in mainidcmp   */
ULONG							gl_rxdsig							;			/* Signal: received data          */
ULONG							gl_txdsig							;			/* Signal: data sent              */
struct Info			 *gl_onlinepb					=0;			/* Online entry (0 if not online) */
ulong             gl_onlinecost       =0;     /* Online cost (calc in mainloop) */
ulong             gl_onlinetime       =0;     /* Online time (calc in mainloop) */
ulong             gl_onlinebase         ;			/* Online start time              */
char              gl_onlinephone[STRSIZE];    /* Online phonenumber             */
char              gl_def_all[64];             /* Config file names              */
char              gl_def_cs [64];
char              gl_def_cu [64];
char              gl_def_dl [64];
char              gl_def_gl [64];
char              gl_def_pb [64];
char              gl_def_sl [64];
char              gl_def_tb [64];
char              gl_def_xm [64];
char              gl_def_xp [64];
struct List       gl_phonebook					;			/* Phonebook											*/
struct List       gl_fileid							;			/* Upload list										*/
struct Info      *gl_defbbs						=0;			/* Pickup table (default setup)		*/
struct Info      *gl_fromdef					=0;			/* Tagit defbbs:stä in create	pb	*/
struct Info      *gl_curbbs						=0;			/* Tämänhetkinen setup						*/
char						 *gl_cur_qs[STR_PB_LAST-STR_PB_FIRST+1];
ULONG							gl_cur_qv[VAL_PB_LAST-VAL_PB_FIRST+1];
ULONG						 *gl_cur_qd[DAT_PB_LAST-DAT_PB_FIRST+1];
char						**gl_str_ultimate			=str_ult;
ULONG						 *gl_val_ultimate			=val_ult;
ULONG						**gl_dat_ultimate			=dat_ult;
/*
struct XPR_IO			gl_xpr_io							;			/* XPr structure									*/
*/
struct XEM_IO			gl_xem_io							;			/* XEm structure									*/
struct List				gl_xem_key						;			/* XEm macro keys									*/

/*** curbbs:stä purettuja tietoja ***/
struct Info      *gl_curcs						=0;		/* Tämän hetkinen charset					*/
struct Info      *gl_curtb						=0;		/* Tämän hetkinen toolbar         */
struct Info      *gl_curxp						=0;		/* Tämän hetkinen protocol				*/
struct Info			 *gl_curxm						=0;		/* Tämän hetkinen xem							*/
UBYTE						 *gl_curtxd							;		/* Conversion tablet							*/
UBYTE						 *gl_currxd							;
UBYTE						 *gl_curcap							;

/*** run time flags ***/
long  gl_exit       =FALSE;		/* Quit requested           */
LONG  gl_sleep      =FALSE;	  /* Iconifyed on/off         */
LONG  gl_warning    =TRUE;		/* Ok to print warning      */
LONG  gl_title			=FALSE;	  /* Screen title on/off      */
LONG  gl_hardstat		=FALSE;	  /* Hardware display on/off  */
LONG  gl_serok			=FALSE;		/* Serial working           */
LONG  gl_serfreeze	=FALSE;		/* Serial freezed           */
LONG  gl_menufreeze	=TRUE;		/* No menu refresh          */
LONG  gl_scrok			=FALSE;		/* Console working          */
LONG  gl_dial				=FALSE;		/* Dial in process          */
LONG  gl_dialnum		=FALSE;		/* Dial number in process   */
LONG  gl_xferon			=FALSE;		/* Transfer running         */
LONG  gl_xferend		=TRUE;		/* Transfer ended someway   */
long  gl_async_dos  =FALSE;   /* Use asyncio.library      */

LONG	gl_charsafe		=TRUE;		/* ei muutoksia							*/
LONG	gl_macrosafe	=TRUE;		/* ei muutoksia							*/
LONG	gl_profsafe		=TRUE;		/* ei muutoksia							*/
LONG	gl_phonesafe	=TRUE;		/* ei muutoksia							*/
LONG	gl_protosafe	=TRUE;		/* ei muutoksia							*/
long  gl_showsafe   =TRUE;
LONG	gl_xemsafe		=TRUE;		/* ei muutoksia							*/
LONG	gl_toolbarsafe=TRUE;		/* ei muutoksia							*/

#ifdef DEBUG
/*** statistic ***/
long  gl_call_alloc =0;       /* alloc_pool               */
long  gl_call_free  =0;       /* free_pool                */
long  gl_call_force =0;       /* alloc_force              */

long  gl_tags_now   =0;       /* monta tagia listoissa    */
long  gl_tags_ign   =0;       /* monta ignorea listoissa  */
long  gl_tags_add   =0;       /* monta tagia lisätty      */
long  gl_tags_del   =0;       /* monta tagia dellattu     */

long  gl_mem_now    =0;       /* paljon varattu           */
long  gl_mem_add    =0;       /* paljon pyydetty          */
long  gl_mem_del    =0;       /* paljon vapautettu        */
#endif
