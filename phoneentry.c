#include "global.h"
#include "loc_strings.h"

/*
char ver_phoneentry[]="1.49 (Apr 04 1996)";
*/

enum ID		{ ID_ok = PE_First,
						ID_pick				,ID_clr					,ID_cancel			,
						/* bbs */
						ID_name				,ID_phonepre		,ID_phone				,ID_password		,
						ID_comment		,
						ID_costfirst	,ID_costrest		,
						ID_waitentry	,ID_waitdial		,ID_logname			,ID_logcall			,
						/* serial */
						ID_dte				,ID_databits		,ID_stopbits		,ID_parity			,
						ID_duplex			,ID_flow				,ID_deviceio		,ID_device			,
						ID_unit				,ID_serbufsize	,ID_hardser			,ID_fixrate			,
            ID_dtrhangup  ,ID_sershared   ,
						/* screen */
						ID_cs					,ID_hardscr			,ID_borblank		,ID_keepwb			,
						ID_aga				,ID_poprxd			,ID_rtsblit			,ID_pubscreen		,
						ID_wb					,ID_status			,ID_statusline  ,ID_columns			,
            ID_lines			,ID_hlines			,ID_modeid			,ID_forceconxy	,
						/* transfer */
						ID_xp					,ID_upath				,ID_dpath				,ID_dnfiles			,
						ID_autoxfer		,ID_quietxfer		,ID_double			,
						/* emulation */
						ID_emulation	,
						ID_wrap				,ID_killbs			,ID_stripbold		,ID_cls					,
						ID_emcr2crlf	,ID_ansipc			,ID_prescroll		,ID_ansipal			,
						ID_ansidef		,ID_ansibright	,ID_ansicopy		,ID_hexdef			,
						ID_xm					,
						/* keyboard */
						ID_func01			,ID_func02			,ID_func03			,ID_func04			,
						ID_func05			,ID_func06			,ID_func07			,ID_func08			,
						ID_func09			,ID_func10			,ID_cr2crlf			,ID_lf2crlf			,
						/* modem */
						ID_dialstring	,ID_dialsuffix	,ID_nocarrier		,ID_nodialtone	,
						ID_connect		,ID_ringing			,ID_error				,ID_busy				,
						ID_modemok		,ID_hangupcom		,ID_dialabort   };

static char	*t_pages[]			= { (strptr)MSG_bbs, (strptr)MSG_serial, (strptr)MSG_screen, (strptr)MSG_transfer, (strptr)MSG_emulation, (strptr)MSG_keyboard, (strptr)MSG_modem, 0 };
static char	*t_dte[]				= { (strptr)MSG_1200, (strptr)MSG_2400, (strptr)MSG_4800, (strptr)MSG_9600, (strptr)MSG_14400, (strptr)MSG_19200, (strptr)MSG_38400, (strptr)MSG_57600, (strptr)MSG_76800, (strptr)MSG_115200,0 };
static char	*t_databits[]		= { (strptr)MSG_7, (strptr)MSG_8, 0 };
static char	*t_stopbits[]		= { (strptr)MSG_1, (strptr)MSG_2, 0 };
static char	*t_parity[]			= { (strptr)MSG_none, (strptr)MSG_even, (strptr)MSG_odd, (strptr)MSG_mark ,(strptr)MSG_space, 0 };
static char	*t_duplex[]			= { (strptr)MSG_half, (strptr)MSG_full, 0 };
static char	*t_flow[]				= { (strptr)MSG_none, (strptr)MSG_xon_xoff, (strptr)MSG_rts_cts, 0 };
static char	*t_status[]			= { (strptr)MSG_nothing, (strptr)MSG_line, (strptr)MSG_window, (strptr)MSG_line_window, 0 };
static char	*t_emulation[]	= { (strptr)MSG_ansi_vt102, (strptr)MSG_hex, (strptr)MSG_external_xem, 0 };
/* Index data cyclejen handlaukseen */
static const ulong i_dte[]				= { 1200,2400,4800,9600,14400,19200,38400,57600,76800,115200,0 };
static const ulong i_databits[]	  = { 7,8 };
static const ulong i_stopbits[]	  = { 1,2 };
static const ulong i_parity[]		  =	{ PAR_NONE,PAR_EVEN,PAR_ODD,PAR_MARK,PAR_SPACE };
static const ulong i_duplex[]		  = { FALSE,TRUE };
static const ulong i_flow[]			  = { FLOW_NONE,FLOW_XONXOFF,FLOW_RTSCTS };
static const ulong i_status[]		  = { STAT_NONE,STAT_BAR,STAT_WINDOW,STAT_BAR|STAT_WINDOW };
static const ulong i_emulation[]	= { EM_ANSI, EM_HEX, EM_XEM };
static struct MUI_Palette_Entry ansientry[] =
{
  { 0,0x00000000,0x00000000,0x00000000,0	},
  { 0,0x00000000,0x00000000,0x00000000,1	},
  { 0,0x00000000,0x00000000,0x00000000,2	},
  { 0,0x00000000,0x00000000,0x00000000,3	},
	{ 0,0x00000000,0x00000000,0x00000000,4	},
	{ 0,0x00000000,0x00000000,0x00000000,5	},
	{ 0,0x00000000,0x00000000,0x00000000,6	},
	{ 0,0x00000000,0x00000000,0x00000000,7	},
	{ 0,0x00000000,0x00000000,0x00000000,8	},
	{ 0,0x00000000,0x00000000,0x00000000,9	},
	{ 0,0x00000000,0x00000000,0x00000000,10	},
	{ 0,0x00000000,0x00000000,0x00000000,11	},
	{ 0,0x00000000,0x00000000,0x00000000,12	},
	{ 0,0x00000000,0x00000000,0x00000000,13	},
	{ 0,0x00000000,0x00000000,0x00000000,14	},
	{ 0,0x00000000,0x00000000,0x00000000,15	},
	{ MUIV_Palette_Entry_End,0,0,0,0 },
};
static char *ansiname[] =
{
  (strptr)MSG_black_background,
  (strptr)MSG_red,
  (strptr)MSG_green,
  (strptr)MSG_yellow,
  (strptr)MSG_blue,
  (strptr)MSG_magenta,
  (strptr)MSG_cyan,
  (strptr)MSG_white,
  (strptr)MSG_black_bright,
  (strptr)MSG_red_bright,
  (strptr)MSG_green_bright,
  (strptr)MSG_yellow_bright,
  (strptr)MSG_blue_bright,
  (strptr)MSG_magenta_bright,
  (strptr)MSG_cyan_bright,
  (strptr)MSG_white_bright
};
static struct MUI_Palette_Entry hexentry[] =
{
  { 0,0x00000000,0x00000000,0x00000000,0	},
  { 0,0x00000000,0x00000000,0x00000000,1	},
  { MUIV_Palette_Entry_End,0,0,0,0 },
};
static char *hexname[] =
{
  (strptr)MSG_background,
  (strptr)MSG_text
};

static aptr win_act = FALSE;
static APTR win,rg_mypage;
static APTR bt_ok,tg_pick,tg_clr,bt_cancel;

static APTR	st_name,st_phonepre,st_phone,st_password,
						st_comment,it_costfirst,it_costrest,it_waitentry,it_waitdial,
						st_logname,cm_logcall;

static aptr cy_dte,cy_databits,cy_stopbits,cy_parity,cy_duplex,cy_flow,
            io_device,st_device,sl_unit,sl_serbufsize,cm_hardser,cm_fixrate,
            cm_dtrhangup,cm_sershared;

static aptr lv_cs,cy_status,st_statusline,sl_columns,sl_lines,sl_hlines,tx_scrsize,cm_borblank,
						cm_keepwb,cm_hardscr,cm_aga,cm_poprxd,cm_rtsblit,st_pubscreen,cm_wb,
						bt_modeid,cm_forceconxy;

static APTR	lv_xp,st_upath,st_dpath,st_dnfiles,cm_autoxfer,cm_quietxfer,cm_double;

static APTR	cy_emulation,empage,lv_xm,cm_wrap,cm_killbs,cm_stripbold,cm_cls,
						cm_emcr2crlf,cm_ansipc,sl_prescroll,
						pa_ansipal,bt_ansidef,bt_ansibright,
						bt_ansicopy,pa_hexpal,bt_hexdef;

static APTR	st_func01,st_func02,st_func03,st_func04,st_func05,
						st_func06,st_func07,st_func08,st_func09,st_func10,
						cm_cr2crlf,cm_lf2crlf;

static APTR	st_dialstring,st_dialsuffix,st_nocarrier,st_nodialtone,st_connect,
						st_ringing,st_error,st_busy,st_modemok,st_hangupcom,st_dialabort;

/* Macro eri gadgettien handlaukseen */
#define HANDLE(fname,myfunc,obj,tag)\
static void fname(void)\
{\
  myfunc(obj,tag);\
}

#define	QUIET			0	/* Gadgettien painelu ei muuta mitään */
#define	EDITIT		1	/* Editoi optioita */
#define	PICK			2	/* Settaa optio pick from table stateen */
#define	LOCAL			3	/* Settaa optio pick from local stateen	*/
#define	PICKPROF	4	/* Settaa optio default pick from table profile */
#define	LOCALPROF	5	/* Settaa optio default pick from local profile */

static char        curtitle[STRSIZE];
static ulong       nowstate=0,nowedit;
static struct Info *newpb,*oldpb,*expb;

/***----------------------------------------------------------------------***/
/*** support                                                              ***/
/***----------------------------------------------------------------------***/

/* Etsi datan index */
static ULONG GetIndex(ULONG data, ULONG index[])
{
long i;

	for(i=0;i <= 10;i++)
	{
		if (index[i] == data) return(i);
	}
	EndProg("IndexSearch()");
}

/* Printtaa ruudun dimensio */
static void Dimension(void)
{
char        my[STRSIZE];
struct Info *cs;

	if ( (cs = Charset(DT_FIND,GetEntryTag(PB_Charset,newpb))) == 0 ) cs = Charset(DT_FIND,STR_DEF(PB_Charset));
	if (cs == 0) { EndProg("Dimension()"); }
	if (GetEntryTag(PB_ForceConXY,newpb)) {
		sprintf(my,LOC(MSG_screen_dimension_x_x_pixels_x_colors),
						GetEntryTag(PB_ConX,newpb),
						GetEntryTag(PB_ConY,newpb),
						Pot2(GetEntryTag(PB_Depth,newpb)));
	} else {
		sprintf(my,LOC(MSG_screen_dimension_x_x_pixels_x_colors),
						GetInfoTag(CS_Xsize,cs)*GetEntryTag(PB_Columns,newpb),
						GetInfoTag(CS_Ysize,cs)*GetEntryTag(PB_Lines	,newpb),
						Pot2(GetEntryTag(PB_Depth,newpb)));
	}
	set(tx_scrsize,MUIA_Text_Contents,my);
}

/* Editoi, set optio pick table/local tai pick table profile */

static void HandleCheck(APTR obj, ULONG tag)
{
ULONG	b;

	switch (nowedit)
	{
		case EDITIT:
			get(obj,MUIA_Selected,&b);
			AddInfoTag(tag,b,newpb);
			return;
		case PICK:
			DelInfoTag(tag,newpb);
			nnset(obj,MUIA_Selected,GetInfoTag(tag,gl_defbbs));
			break;
		case LOCAL:
			b = GetInfoTag(tag,gl_defbbs);
			AddInfoTag(tag,b,newpb);
			nnset(obj,MUIA_Selected,b);
			break;
		case PICKPROF:
			AddInfoTag(tag,(ULONG)"",gl_fromdef);
			nnset(obj,MUIA_Selected,GetInfoTag(tag,newpb));
			break;
		case LOCALPROF:
			DelInfoTag(tag,gl_fromdef);
			nnset(obj,MUIA_Selected,GetInfoTag(tag,newpb));
			break;
	}
	set(obj,MUIA_Disabled,TRUE);
}

static void HandleCycle(APTR obj, ULONG tag, ULONG index[])
{
ULONG	i,j;

	switch (nowedit)
	{
		case EDITIT:
			get(obj,MUIA_Cycle_Active,&i);
			AddInfoTag(tag,index[i],newpb);
			return;
		case PICK:
			DelInfoTag(tag,newpb);
			nnset(obj,MUIA_Cycle_Active,GetIndex(GetInfoTag(tag,gl_defbbs),index));
			break;
		case LOCAL:
			j = GetInfoTag(tag,gl_defbbs);
			AddInfoTag(tag,j,newpb);
			nnset(obj,MUIA_Cycle_Active,GetIndex(j,index));
			break;
		case PICKPROF:
			AddInfoTag(tag,(ULONG)"",gl_fromdef);
			nnset(obj,MUIA_Cycle_Active,GetIndex(GetInfoTag(tag,newpb),index));
			break;
		case LOCALPROF:
			DelInfoTag(tag,gl_fromdef);
			nnset(obj,MUIA_Cycle_Active,GetIndex(GetInfoTag(tag,newpb),index));
			break;
	}
	set(obj,MUIA_Disabled,TRUE);
}

static void HandleInt(APTR obj, ULONG tag)
{
ULONG	val;

	switch (nowedit)
	{
		case EDITIT:
			get(obj,MUIA_String_Integer,&val);
			AddInfoTag(tag,val,newpb);
			return;
		case PICK:
			DelInfoTag(tag,newpb);
			nnset(obj,MUIA_String_Integer,GetInfoTag(tag,gl_defbbs));
			break;
		case LOCAL:
			val = GetInfoTag(tag,gl_defbbs);
			AddInfoTag(tag,val,newpb);
			nnset(obj,MUIA_String_Integer,val);
			break;
		case PICKPROF:
			AddInfoTag(tag,(ULONG)"",gl_fromdef);
			nnset(obj,MUIA_String_Integer,GetInfoTag(tag,newpb));
			break;
		case LOCALPROF:
			DelInfoTag(tag,gl_fromdef);
			nnset(obj,MUIA_String_Integer,GetInfoTag(tag,newpb));
			break;
	}
	set(obj,MUIA_Disabled,TRUE);
}

static void HandleList(APTR obj, ULONG tag)
{
ULONG	num;
struct Node	*nd;

	switch (nowedit)
	{
		case EDITIT:
			get(obj,MUIA_List_Active,&num);
			DoMethod(obj,MUIM_List_GetEntry,num,&nd);
			if (nd) AddInfoTag(tag,(ULONG)nd->ln_Name,newpb);
			return;
		case PICK:
			DelInfoTag(tag,newpb);
			MUI_nnselname(obj,(char *)GetInfoTag(tag,gl_defbbs));
			break;
		case LOCAL:
			AddInfoTag(tag,GetInfoTag(tag,gl_defbbs),newpb);
			MUI_nnselname(obj,(char *)GetInfoTag(tag,gl_defbbs));
			break;
		case PICKPROF:
			AddInfoTag(tag,(ULONG)"",gl_fromdef);
			MUI_nnselname(obj,(char *)GetInfoTag(tag,newpb));
			break;
		case LOCALPROF:
			DelInfoTag(tag,gl_fromdef);
			MUI_nnselname(obj,(char *)GetInfoTag(tag,newpb));
			break;
	}
	set(obj,MUIA_Disabled,TRUE);
}

static void HandleSlider(APTR obj, ULONG tag)
{
ULONG	val;

	switch (nowedit)
	{
		case EDITIT:
			get(obj,MUIA_Slider_Level,&val);
			AddInfoTag(tag,val,newpb);
			return;
		case PICK:
			DelInfoTag(tag,newpb);
			nnset(obj,MUIA_Slider_Level,GetInfoTag(tag,gl_defbbs));
			break;
		case LOCAL:
			AddInfoTag(tag,GetInfoTag(tag,gl_defbbs),newpb);
			nnset(obj,MUIA_Slider_Level,GetInfoTag(tag,gl_defbbs));
			break;
		case PICKPROF:
			AddInfoTag(tag,(ULONG)"",gl_fromdef);
			nnset(obj,MUIA_Slider_Level,GetInfoTag(tag,newpb));
			break;
		case LOCALPROF:
			DelInfoTag(tag,gl_fromdef);
			nnset(obj,MUIA_Slider_Level,GetInfoTag(tag,newpb));
			break;
	}
	set(obj,MUIA_Disabled,TRUE);
}

static void HandleString(APTR obj, ULONG tag)
{
ULONG	str;

	switch (nowedit)
	{
		case EDITIT:
			get(obj,MUIA_String_Contents,&str);
			AddInfoTag(tag,str,newpb);
			return;
		case PICK:
			DelInfoTag(tag,newpb);
			nnset(obj,MUIA_String_Contents,GetInfoTag(tag,gl_defbbs));
			break;
		case LOCAL:
			str = GetInfoTag(tag,gl_defbbs);
			AddInfoTag(tag,str,newpb);
			nnset(obj,MUIA_String_Contents,str);
			break;
		case PICKPROF:
			AddInfoTag(tag,(ULONG)"",gl_fromdef);
			nnset(obj,MUIA_String_Contents,GetInfoTag(tag,newpb));
			break;
		case LOCALPROF:
			DelInfoTag(tag,gl_fromdef);
			nnset(obj,MUIA_String_Contents,GetInfoTag(tag,newpb));
			break;
	}
	set(obj,MUIA_Disabled,TRUE);
}

BOOL kludge_val;

static ULONG K_TF(BOOL bah)
{
	if (kludge_val) {
		return(bah);
	} else {
		return(TF(bah));
	}
}

static void kludge(struct TagBase *ti)
{
	multiset(MUIA_Disabled,
					st_name				,K_TF(LookTag(PB_Name,ti)),
					st_phonepre		,K_TF(LookTag(PB_PhonePre,ti)),
					st_phone			,K_TF(LookTag(PB_Phone,ti)),
					st_password		,K_TF(LookTag(PB_PassWord,ti)),
					st_comment		,K_TF(LookTag(PB_Comment,ti)),
					st_dialstring	,K_TF(LookTag(PB_DialString,ti)),
					st_logname		,K_TF(LookTag(PB_LogName,ti)),
					it_costfirst	,K_TF(LookTag(PB_CostFirst,ti)),
					it_costrest		,K_TF(LookTag(PB_CostRest,ti)),
					it_waitentry	,K_TF(LookTag(PB_WaitEntry,ti)),
					it_waitdial		,K_TF(LookTag(PB_WaitDial,ti)),
					cm_logcall		,K_TF(LookTag(PB_LogCall,ti)),

					cy_dte				,K_TF(LookTag(PB_DteRate,ti)),
					cy_databits		,K_TF(LookTag(PB_DataBits,ti)),
					cy_stopbits		,K_TF(LookTag(PB_StopBits,ti)),
					cy_parity			,K_TF(LookTag(PB_Parity,ti)),
					cy_duplex			,K_TF(LookTag(PB_FullDuplex,ti)),
					cy_flow				,K_TF(LookTag(PB_FlowControl,ti)),
					io_device			,K_TF(LookTag(PB_SerName,ti)),
					st_device			,K_TF(LookTag(PB_SerName,ti)),
					sl_unit				,K_TF(LookTag(PB_SerUnit,ti)),
					sl_serbufsize	,K_TF(LookTag(PB_SerBufSize,ti)),
					cm_hardser		,K_TF(LookTag(PB_HardSer,ti)),
					cm_fixrate		,K_TF(LookTag(PB_FixRate,ti)),
					cm_dtrhangup	,K_TF(LookTag(PB_DTRHangup,ti)),
          cm_sershared  ,K_TF(LookTag(PB_SerShared,ti)),

					lv_cs					,K_TF(LookTag(PB_Charset,ti)),
					cm_borblank		,K_TF(LookTag(PB_BorderBlank,ti)),
					cm_hardscr		,K_TF(LookTag(PB_HardScr,ti)),
					cm_keepwb			,K_TF(LookTag(PB_KeepWb,ti)),
					cm_aga				,K_TF(LookTag(PB_Aga,ti)),
					cm_poprxd			,K_TF(LookTag(PB_RxdPopup,ti)),
					cm_rtsblit		,K_TF(LookTag(PB_RtsBlit,ti)),
					st_pubscreen	,K_TF(LookTag(PB_PubScreen,ti)),
					cm_wb					,K_TF(LookTag(PB_Wb,ti)),
					cy_status			,K_TF(LookTag(PB_Status,ti)),
          st_statusline ,K_TF(LookTag(PB_StatusLine,ti)),
					sl_columns		,K_TF(LookTag(PB_Columns,ti)),
					sl_lines			,K_TF(LookTag(PB_Lines,ti)),
					sl_hlines			,K_TF(LookTag(PB_HLines,ti)),
					bt_modeid			,K_TF(LookTag(PB_ModeId,ti)),
					cm_forceconxy	,K_TF(LookTag(PB_ForceConXY,ti)),

					lv_xp					,K_TF(LookTag(PB_Protocol,ti)),
					st_upath			,K_TF(LookTag(PB_UpPath,ti)),
					st_dpath			,K_TF(LookTag(PB_DnPath,ti)),
					st_dnfiles		,K_TF(LookTag(PB_DnFiles,ti)),
					cm_autoxfer		,K_TF(LookTag(PB_AutoXfer,ti)),
					cm_quietxfer	,K_TF(LookTag(PB_QuietXfer,ti)),
					cm_double			,K_TF(LookTag(PB_Double,ti)),

					st_func01			,K_TF(LookTag(PB_Func1,ti)),
					st_func02			,K_TF(LookTag(PB_Func2,ti)),
					st_func03			,K_TF(LookTag(PB_Func3,ti)),
					st_func04			,K_TF(LookTag(PB_Func4,ti)),
					st_func05			,K_TF(LookTag(PB_Func5,ti)),
					st_func06			,K_TF(LookTag(PB_Func6,ti)),
					st_func07			,K_TF(LookTag(PB_Func7,ti)),
					st_func08			,K_TF(LookTag(PB_Func8,ti)),
					st_func09			,K_TF(LookTag(PB_Func9,ti)),
					st_func10			,K_TF(LookTag(PB_Func10,ti)),
					cm_cr2crlf		,K_TF(LookTag(PB_cr2crlf,ti)),
					cm_lf2crlf		,K_TF(LookTag(PB_lf2crlf,ti)),

					cy_emulation	,K_TF(LookTag(PB_Emulation,ti)),
					cm_wrap				,K_TF(LookTag(PB_ANSIWrap,ti)),
					cm_killbs			,K_TF(LookTag(PB_ANSIKillBs,ti)),
					cm_stripbold	,K_TF(LookTag(PB_ANSIStrip,ti)),
					cm_cls				,K_TF(LookTag(PB_ANSICls,ti)),
					cm_emcr2crlf	,K_TF(LookTag(PB_ANSICr2crlf,ti)),
					cm_ansipc			,K_TF(LookTag(PB_ANSIPc,ti)),
					sl_prescroll	,K_TF(LookTag(PB_PreScroll,ti)),
					pa_ansipal		,K_TF(LookTag(PB_ANSICol,ti)),
					bt_ansidef		,K_TF(LookTag(PB_ANSICol,ti)),
					bt_ansibright	,K_TF(LookTag(PB_ANSICol,ti)),
					bt_ansicopy		,K_TF(LookTag(PB_ANSICol,ti)),
					pa_hexpal			,K_TF(LookTag(PB_HEXCol,ti)),
					bt_hexdef			,K_TF(LookTag(PB_HEXCol,ti)),
					lv_xm					,K_TF(LookTag(PB_Xem,ti)),

					st_dialsuffix	,K_TF(LookTag(PB_DialSuffix,ti)),
					st_nocarrier	,K_TF(LookTag(PB_NoCarrier,ti)),
					st_nodialtone	,K_TF(LookTag(PB_NoDialTone,ti)),
					st_connect		,K_TF(LookTag(PB_Connect,ti)),
					st_ringing		,K_TF(LookTag(PB_Ringing,ti)),
					st_error			,K_TF(LookTag(PB_Error,ti)),
					st_busy				,K_TF(LookTag(PB_Busy,ti)),
					st_modemok		,K_TF(LookTag(PB_Ok,ti)),
					st_hangupcom	,K_TF(LookTag(PB_Hangup,ti)),
					st_dialabort	,K_TF(LookTag(PB_DialAbort,ti)), TAG_END);
}

/* Disabloi optiot jotka löytyvät */
static void DisableLocal(struct TagBase *ti)
{
	kludge_val = TRUE;
	kludge(ti);
}

/* Disabloi optiot joita ei löydy */
static void DisablePick(struct TagBase *ti)
{
	kludge_val = FALSE;
	kludge(ti);
}

/* Kopio Charset lista guihin */
static void CsToGui(void)
{
BOOL	b;
LONG	i=0;
struct Info	*cs;

	b = GetEntryTag(PB_HardScr,newpb);

	set(lv_cs,MUIA_List_Quiet,TRUE);
	nnset(lv_cs,MUIA_List_Active,MUIV_List_Active_Off); /* prevent notify */
	DoMethod(lv_cs,MUIM_List_Clear);
	while ( (cs = Charset(DT_GET,i++)) )
	{
		if (b == FALSE OR ( GetInfoTag(CS_Xsize,cs) == 8 AND GetInfoTag(CS_Ysize,cs) == 8 ) ) {
			DoMethod(lv_cs,MUIM_List_InsertSingle,cs,MUIV_List_Insert_Sorted);
		}
	}
	set(lv_cs,MUIA_List_Quiet,FALSE);
	if ( MUI_nnselname(lv_cs,(char *)GetEntryTag(PB_Charset,newpb)) == FALSE ) {
		MUI_nnselname(lv_cs,(char *)STR_DEF(PB_Charset));
	}
	Dimension();
}

/* Kopio Protocol lista guihin */
static void XpToGui(void)
{
struct Info	*xp;
LONG				i=0;

	set(lv_xp,MUIA_List_Quiet,TRUE);
	nnset(lv_xp,MUIA_List_Active,MUIV_List_Active_Off); /* prevent notify */
	DoMethod(lv_xp,MUIM_List_Clear);
	while ( xp = DoMethod(gl_protocol,MUIM_INF_Get,i++) )
	{
		DoMethod(lv_xp,MUIM_List_InsertSingle,xp,MUIV_List_Insert_Sorted);
	}
	set(lv_xp,MUIA_List_Quiet,FALSE);
	MUI_nnselname(lv_xp,(char *)GetEntryTag(PB_Protocol,newpb));
}

/* Kopio emulation lista guihin */
static void XmToGui(void)
{
struct Info	*xm;
LONG				i=0;

	set(lv_xm,MUIA_List_Quiet,TRUE);
	nnset(lv_xm,MUIA_List_Active,MUIV_List_Active_Off); /* prevent notify */
	DoMethod(lv_xm,MUIM_List_Clear);
	while ( (xm = Xem(DT_GET,i++)) )
	{
		DoMethod(lv_xm,MUIM_List_InsertSingle,xm,MUIV_List_Insert_Sorted);
	}
	set(lv_xm,MUIA_List_Quiet,FALSE);
	MUI_nnselname(lv_xm,(char *)GetEntryTag(PB_Xem,newpb));
}

/* Kopio entry GUIhin */
static void EntryToGui(void)
{
char  str_cf[14],str_cr[14],str_we[14],str_wd[14];
long  i;
ulong *col;

	stcl_d(str_cf,GetEntryTag(PB_CostFirst,newpb));
	stcl_d(str_cr,GetEntryTag(PB_CostRest ,newpb));
	stcl_d(str_we,GetEntryTag(PB_WaitEntry,newpb));
	stcl_d(str_wd,GetEntryTag(PB_WaitDial ,newpb));
	nnmultiset(MUIA_String_Contents,
						st_name				,GetEntryTag(PB_Name,newpb),
						st_phonepre		,GetEntryTag(PB_PhonePre,newpb),
						st_phone			,GetEntryTag(PB_Phone,newpb),
						st_password		,GetEntryTag(PB_PassWord,newpb),
						st_comment		,GetEntryTag(PB_Comment,newpb),
						st_dialstring	,GetEntryTag(PB_DialString,newpb),
						st_logname		,GetEntryTag(PB_LogName,newpb),
						it_costfirst	,str_cf,
						it_costrest		,str_cr,
						it_waitentry	,str_we,
						it_waitdial		,str_wd, TAG_END);
/* MUI generates MUIA_String_Contents notify from this and
	 MUIA_String_Integer notify do not work...
	nnmultiset(MUIA_String_Integer,
						it_costfirst	,GetEntryTag(PB_CostFirst,newpb),
						it_costrest		,GetEntryTag(PB_CostRest,newpb),
						it_waitentry	,GetEntryTag(PB_WaitEntry,newpb),
						it_waitdial		,GetEntryTag(PB_WaitDial,newpb), TAG_END);
*/
	nnset(cm_logcall		,MUIA_Selected,GetEntryTag(PB_LogCall,newpb));

	nnmultiset(MUIA_Cycle_Active,
						cy_dte				,GetIndex(GetEntryTag(PB_DteRate,newpb),i_dte),
						cy_databits		,GetIndex(GetEntryTag(PB_DataBits,newpb),i_databits),
						cy_stopbits		,GetIndex(GetEntryTag(PB_StopBits,newpb),i_stopbits),
						cy_parity			,GetIndex(GetEntryTag(PB_Parity,newpb),i_parity),
						cy_duplex			,GetIndex(GetEntryTag(PB_FullDuplex,newpb),i_duplex),
						cy_flow				,GetIndex(GetEntryTag(PB_FlowControl,newpb),i_flow), TAG_END);
	nnset(st_device			,MUIA_String_Contents,GetEntryTag(PB_SerName,newpb));
	nnset(sl_unit				,MUIA_Slider_Level,GetEntryTag(PB_SerUnit,newpb));
	nnset(sl_serbufsize	,MUIA_Slider_Level,GetEntryTag(PB_SerBufSize,newpb));
	nnmultiset(MUIA_Selected,
						cm_hardser		,GetEntryTag(PB_HardSer,newpb),
						cm_fixrate		,GetEntryTag(PB_FixRate,newpb),
						cm_dtrhangup	,GetEntryTag(PB_DTRHangup,newpb),
						cm_sershared  ,GetEntryTag(PB_SerShared,newpb), TAG_END);

	nnmultiset(MUIA_Selected,
						cm_wb					,GetEntryTag(PB_Wb,newpb),
						cm_borblank		,GetEntryTag(PB_BorderBlank,newpb),
						cm_hardscr		,GetEntryTag(PB_HardScr,newpb),
						cm_keepwb			,GetEntryTag(PB_KeepWb,newpb),
						cm_aga				,GetEntryTag(PB_Aga,newpb),
						cm_poprxd			,GetEntryTag(PB_RxdPopup,newpb),
						cm_rtsblit		,GetEntryTag(PB_RtsBlit,newpb), TAG_END);
  nnset(st_pubscreen	,MUIA_String_Contents,GetEntryTag(PB_PubScreen,newpb));
  nnset(cy_status			,MUIA_Cycle_Active   ,GetIndex(GetEntryTag(PB_Status,newpb),i_status));
  nnset(st_statusline ,MUIA_String_Contents,GetEntryTag(PB_StatusLine,newpb));

  nnset(cm_forceconxy	,MUIA_Selected,GetEntryTag(PB_ForceConXY,newpb));
	nnmultiset(MUIA_Slider_Level,
						sl_columns		,GetEntryTag(PB_Columns,newpb),
						sl_lines			,GetEntryTag(PB_Lines,newpb),
						sl_hlines			,GetEntryTag(PB_HLines,newpb), TAG_END);

	nnmultiset(MUIA_String_Contents,
						st_upath			,GetEntryTag(PB_UpPath,newpb),
						st_dpath			,GetEntryTag(PB_DnPath,newpb),
						st_dnfiles		,GetEntryTag(PB_DnFiles,newpb), TAG_END);
	nnmultiset(MUIA_Selected,
						cm_autoxfer		,GetEntryTag(PB_AutoXfer,newpb),
						cm_quietxfer	,GetEntryTag(PB_QuietXfer,newpb),
						cm_double			,GetEntryTag(PB_Double,newpb), TAG_END);

	nnmultiset(MUIA_String_Contents,
						st_func01			,GetEntryTag(PB_Func1,newpb),
						st_func02			,GetEntryTag(PB_Func2,newpb),
						st_func03			,GetEntryTag(PB_Func3,newpb),
						st_func04			,GetEntryTag(PB_Func4,newpb),
						st_func05			,GetEntryTag(PB_Func5,newpb),
						st_func06			,GetEntryTag(PB_Func6,newpb),
						st_func07			,GetEntryTag(PB_Func7,newpb),
						st_func08			,GetEntryTag(PB_Func8,newpb),
						st_func09			,GetEntryTag(PB_Func9,newpb),
						st_func10			,GetEntryTag(PB_Func10,newpb), TAG_END);
	nnmultiset(MUIA_Selected,
						cm_cr2crlf		,GetEntryTag(PB_cr2crlf,newpb),
						cm_lf2crlf		,GetEntryTag(PB_lf2crlf,newpb), TAG_END);

	nnset(cy_emulation	,MUIA_Cycle_Active,GetIndex(GetEntryTag(PB_Emulation,newpb),i_emulation));
	nnmultiset(MUIA_Selected,
						cm_wrap				,GetEntryTag(PB_ANSIWrap,newpb),
						cm_killbs			,GetEntryTag(PB_ANSIKillBs,newpb),
						cm_stripbold	,GetEntryTag(PB_ANSIStrip,newpb),
						cm_cls				,GetEntryTag(PB_ANSICls,newpb),
						cm_emcr2crlf	,GetEntryTag(PB_ANSICr2crlf,newpb),
						cm_ansipc			,GetEntryTag(PB_ANSIPc,newpb), TAG_END);
	nnset(sl_prescroll	,MUIA_Slider_Level,GetEntryTag(PB_PreScroll,newpb));

	nnset(empage,MUIA_Group_ActivePage,GetIndex(GetEntryTag(PB_Emulation,newpb),i_emulation));
	col = (ULONG *)GetEntryTag(PB_ANSICol,newpb)+1;
	for (i=0; i<16; i++)
	{
		ansientry[i].mpe_Red  = *col++;
		ansientry[i].mpe_Green= *col++;
		ansientry[i].mpe_Blue = *col++;
	}
	col = (ULONG *)GetEntryTag(PB_HEXCol,newpb)+1;
	for (i=0; i<2; i++)
	{
		hexentry[i].mpe_Red   = *col++;
		hexentry[i].mpe_Green = *col++;
		hexentry[i].mpe_Blue  = *col++;
	}

	nnmultiset(MUIA_String_Contents,
						st_dialsuffix ,GetEntryTag(PB_DialSuffix,newpb),
						st_nocarrier  ,GetEntryTag(PB_NoCarrier,newpb),
						st_nodialtone ,GetEntryTag(PB_NoDialTone,newpb),
						st_connect    ,GetEntryTag(PB_Connect,newpb),
						st_ringing    ,GetEntryTag(PB_Ringing,newpb),
						st_error      ,GetEntryTag(PB_Error,newpb),
						st_busy       ,GetEntryTag(PB_Busy,newpb),
						st_modemok    ,GetEntryTag(PB_Ok,newpb),
						st_hangupcom  ,GetEntryTag(PB_Hangup,newpb),
						st_dialabort  ,GetEntryTag(PB_DialAbort,newpb), TAG_END);

  CsToGui();
  XpToGui();
  XmToGui();
}

/* BBS */

/* Serial */

static void device_click(void)
{
static char name[STRSIZE]="DEVS:";

	if (!gl_req) {
		if (LookInfoTag(PB_SerName,newpb)) {
			Ask_file(gl_conwindow,&device_click,LOC(MSG_serial_device),"#?.device",name);
		}
	} else {
		if ( Ask_file_done() == REQ_OK ) {
			set(st_device,MUIA_String_Contents,name);
		}
	}
}

/* Screen */

static void modeid(void)
{
static struct ScreenModeRequester smr;

  if (!gl_req) {
    switch ( nowedit )
    {
			case EDITIT:
				smr.sm_DisplayID		= GetEntryTag(PB_ModeId			,newpb);
				smr.sm_DisplayWidth	= GetEntryTag(PB_ConX				,newpb);
				smr.sm_DisplayHeight= GetEntryTag(PB_ConY				,newpb);
				smr.sm_DisplayDepth = GetEntryTag(PB_Depth			,newpb);
				smr.sm_OverscanType	= GetEntryTag(PB_Overscan		,newpb);
				smr.sm_AutoScroll		= GetEntryTag(PB_AutoScroll	,newpb);
				Ask_screenmode(gl_conwindow,&modeid,&smr);
				return;
			case PICK:
				DelInfoTags(newpb,
										PB_ModeId		,PB_Depth				,PB_ConX	,PB_ConY	, 
										PB_Overscan	,PB_AutoScroll	, TAG_END);
				break;
			case LOCAL:
				AddInfoTags(newpb,
										PB_ModeId			,GetInfoTag(PB_ModeId			,gl_defbbs),
										PB_ConX				,GetInfoTag(PB_ConX				,gl_defbbs),
										PB_ConY				,GetInfoTag(PB_ConY				,gl_defbbs),
										PB_Depth			,GetInfoTag(PB_Depth			,gl_defbbs),
										PB_Overscan		,GetInfoTag(PB_Overscan		,gl_defbbs),
										PB_AutoScroll	,GetInfoTag(PB_AutoScroll	,gl_defbbs), TAG_END);
				break;
			case PICKPROF:
				AddInfoTags(gl_fromdef,
										PB_ModeId			,0,
										PB_ConX				,0,
										PB_ConY				,0,
										PB_Depth			,0,
										PB_Overscan		,0,
										PB_AutoScroll	,0, TAG_END);
				break;
			case LOCALPROF:
				DelInfoTags(gl_fromdef,
										PB_ModeId		,PB_Depth				,PB_ConX	,PB_ConY	, 
										PB_Overscan	,PB_AutoScroll	, TAG_END);
				break;
		}
		set(bt_modeid,MUIA_Disabled,TRUE);
		Dimension();
	} else {
		if ( Ask_screenmode_done() == REQ_OK ) {
			AddInfoTags(newpb,
									PB_ModeId			,smr.sm_DisplayID			,
									PB_ConX				,smr.sm_DisplayWidth	,
									PB_ConY				,smr.sm_DisplayHeight	,
									PB_Depth			,smr.sm_DisplayDepth	,
									PB_Overscan		,smr.sm_OverscanType	,
									PB_AutoScroll	,smr.sm_AutoScroll		, TAG_END);
			Dimension();
		}
	}
}

/* Transfer */

/* Keys */

/* Emulation */

static handle_ansicol(void)
{
const ulong nil=0;

	switch (nowedit)
	{
		case PICK:
			DelInfoTag(PB_ANSICol,newpb);
			break;
		case LOCAL:
			AddInfoTag(PB_ANSICol,GetInfoTag(PB_ANSICol,gl_defbbs),newpb);
			break;
		case PICKPROF:
			AddInfoTag(PB_ANSICol,&nil,gl_fromdef);
			break;
		case LOCALPROF:
			DelInfoTag(PB_ANSICol,gl_fromdef);
			break;
	}
	DoMethod(win,MUIM_MultiSet,MUIA_Disabled,TRUE,
					 pa_ansipal	,bt_ansidef	,bt_ansibright	,bt_ansicopy, NULL);
}

static handle_hexcol(void)
{
const ulong nil=0;

	switch (nowedit)
	{
		case PICK:
			DelInfoTag(PB_HEXCol,newpb);
			break;
		case LOCAL:
			AddInfoTag(PB_HEXCol,GetInfoTag(PB_HEXCol,gl_defbbs),newpb);
			break;
		case PICKPROF:
			AddInfoTag(PB_HEXCol,&nil,gl_fromdef);
      break;
    case LOCALPROF:
      DelInfoTag(PB_HEXCol,gl_fromdef);
      break;
  }
  set(pa_hexpal	,MUIA_Disabled,TRUE);
  set(bt_hexdef	,MUIA_Disabled,TRUE);
}

static void emulation(void)
{
	HandleCycle(cy_emulation,PB_Emulation,i_emulation);
	nnset(empage,MUIA_Group_ActivePage,GetIndex(GetEntryTag(PB_Emulation,newpb),i_emulation));
}

static void ansidef(void)
{
ULONG	*col;
LONG	i;

	if (nowedit == EDITIT) {
		col = DAT_DEF(PB_ANSICol); col++;
		for (i=0; i<16; i++)
		{
			ansientry[i].mpe_Red	= *col++;
			ansientry[i].mpe_Green= *col++;
			ansientry[i].mpe_Blue	= *col++;
		}
	} else {
		handle_ansicol();
	}
}

static void ansibright(void)
{
LONG	i;
ULONG	r,g,b;

	if (nowedit == EDITIT) {
		for (i=0; i<8; i++)
		{
			r = ansientry[i+8].mpe_Red	+16777216*4;
			g = ansientry[i+8].mpe_Green+16777216*4;
			b = ansientry[i+8].mpe_Blue	+16777216*4;
			if (r < ansientry[i+8].mpe_Red  ) r = 0xffffffff;
			if (g < ansientry[i+8].mpe_Green) g = 0xffffffff;
			if (b < ansientry[i+8].mpe_Blue ) b = 0xffffffff;
			ansientry[i+8].mpe_Red	= r;
			ansientry[i+8].mpe_Green= g;
			ansientry[i+8].mpe_Blue	= b;
		}
	} else {
		handle_ansicol();
	}
}

static void ansicopy(void)
{
long i;

	if (nowedit == EDITIT) {
		for (i=0; i<8; i++)
		{
			ansientry[i+8].mpe_Red	= ansientry[i].mpe_Red;
			ansientry[i+8].mpe_Green= ansientry[i].mpe_Green;
			ansientry[i+8].mpe_Blue	= ansientry[i].mpe_Blue; 
		}
	} else {
		handle_ansicol();
	}
}

static void hexdef(void)
{
LONG	i;
ULONG	*col;

	if (nowedit == EDITIT) {
		col = DAT_DEF(PB_HEXCol); col++;
		for (i=0; i<2; i++)
		{
			hexentry[i].mpe_Red		= *col++;
			hexentry[i].mpe_Green	= *col++;
			hexentry[i].mpe_Blue	= *col++;
		}
	} else {
		handle_hexcol();
	}
}

/* action */

static void pick_click(void)
{
ULONG	b;

	get(tg_pick,MUIA_Selected,&b);
	if (b) {
		nnset(tg_clr,MUIA_Selected,FALSE);
		if (nowstate == PE_NEW OR nowstate == PE_EDIT OR nowstate == PE_CLONE) {
			nowedit = PICK;
			nnset(win,MUIA_Window_Title,LOC(MSG_disable_option_to_pick_it_from_profile));
      DisablePick(&newpb->tags);
    }
    if (nowstate == PE_PICK) {
      nowedit = PICKPROF;
      nnset(win,MUIA_Window_Title,LOC(MSG_disable_option_to_make_it_default_pick_from_profile));
			DisableLocal(&gl_fromdef->tags);
		}
	} else {
		nowedit = EDITIT;
		nnset(win,MUIA_Window_Title,curtitle);
		DisablePick(&newpb->tags);
	}
}

static void clr_click(void)
{
ulong b;

  get(tg_clr,MUIA_Selected,&b);
  if (b) {
    nnset(tg_pick,MUIA_Selected,FALSE);
    if (nowstate == PE_NEW OR nowstate == PE_EDIT OR nowstate == PE_CLONE) {
      nowedit = LOCAL;
      nnset(win,MUIA_Window_Title,LOC(MSG_disable_option_to_pick_it_from_local_here));
      DisableLocal(&newpb->tags);
    }
    if (nowstate == PE_PICK) {
      nowedit = LOCALPROF;
      nnset(win,MUIA_Window_Title,LOC(MSG_disable_option_to_make_it_default_pick_from_local_here));
      DisablePick(&gl_fromdef->tags);
    }
  } else {
    nowedit = EDITIT;
    nnset(win,MUIA_Window_Title,curtitle);
    DisablePick(&newpb->tags);
  }
}

static void ok_click(void)
{
long  i;
ulong ansicol[16*3+1];
ulong hexcol[2*3+1];
ulong *col;

  if (gl_req) return; /* fix better */

	ansicol[0]= 16*3*4;
	if (LookInfoTag(PB_ANSICol,newpb)) {
		col = ansicol; col++;
		for (i=0; i<16; i++)
		{
			*col++ = ansientry[i].mpe_Red;
			*col++ = ansientry[i].mpe_Green;
			*col++ = ansientry[i].mpe_Blue;
		}
		AddInfoTag(PB_ANSICol,ansicol,newpb);
	}
	hexcol[0]	=  2*3*4;
	if (LookInfoTag(PB_HEXCol,newpb)) {
		col = hexcol; col++;
		for (i=0; i<2; i++)
		{
			*col++ = hexentry[i].mpe_Red;
			*col++ = hexentry[i].mpe_Green;
			*col++ = hexentry[i].mpe_Blue;
		}
		AddInfoTag(PB_HEXCol,hexcol,newpb);
	}

	if (nowstate == PE_NEW OR nowstate == PE_CLONE) {
		if (Drop(DL_CMP,GetEntryTag(PB_Phone,newpb))) {
			if ( MUI_Request(gl_app,win_fake,0,
                       LOC(MSG_note),
                       LOC(MSG_yes_discard),
                       LOC(MSG_phonenumber_is_in_droplist),0) == 0 ) {
				DeleteInfo(newpb);
				nowstate = 0;
        Entry(DT_CLOSE,0);
				return;
			}
		}
		Phone(PB_ADDENTRY,newpb);
		nowstate     = 0;
		gl_phonesafe = FALSE;
    Entry(DT_CLOSE,0);
	}
	if (nowstate == PE_EDIT) {
		DelTagList(&oldpb->tags);
		CopyTags(&newpb->tags,&oldpb->tags);
		Phone(DT_UNLOCK,oldpb);
		Phone(PB_REDRAW,oldpb);
		DeleteInfo(newpb);
		nowstate     = 0;
		gl_phonesafe = FALSE;
    Entry(DT_CLOSE,0);
	}
	if (nowstate == PE_PICK) {
		CopyTags(&newpb->tags,&gl_defbbs->tags);
		Phone(PB_REDRAW,0);
		DeleteInfo(newpb);
		nowstate = 0;
    Entry(DT_CLOSE,0);
	}
	if (nowstate == PE_CURRENT) {
    Entry(DT_CLOSE,0);
		SetUp(newpb);
		DeleteInfo(newpb);
		nowstate = 0;
	}
}

static void cancel_click(void)
{
	if (gl_req) return;	/* fix better */

	if (nowstate == PE_EDIT) Phone(DT_UNLOCK,oldpb);
	DeleteInfo(newpb);
	nowstate = 0;
  Entry(DT_CLOSE,0);
}

static bool open_window(void)
{
  if (!win_act) {
    win = WindowObject,
    MUIA_Window_ID    ,MAKE_ID('P','E','W','I'),
    MUIA_HelpNode     ,"help_entry",
    MUIA_Window_Width ,MUIV_Window_Width_Visible(100),
      WindowContents, VGroup,
        Child, rg_mypage = RegisterGroup(t_pages),
          Child, VGroup, /*** bbs ***/
            GroupFrame,
            Child, ColGroup(2),
              Child, Label2L(LOC(MSG_name)) , Child, st_name = String(STRSIZE),
  						Child, Label2L(LOC(MSG_phone)),
  						Child, HGroup,
  							GroupSpacing(0),
  							Child, st_phonepre	= String(STRSIZE),
  							Child, st_phone			= StringObject,StringFrame, MUIA_String_MaxLen,STRSIZE, MUIA_String_Contents,"", MUIA_HorizWeight,1000, End,
  						End,
  						Child, Label2L(LOC(MSG_password)), Child, st_password = String(STRSIZE),
  						Child, Label2L(LOC(MSG_comment)) , Child, st_comment  = String(STRSIZE),
  						Child, Label2L(LOC(MSG_logfile)) , Child, st_logname  = String(STRSIZE),
  					End,
  					Child, ColGroup(4),
  						Child, Label2L(LOC(MSG_call_cost))         , Child, it_costfirst = Int(),
  						Child, Label2L(LOC(MSG_wait_between_dials)), Child, it_waitentry = Int(),
  						Child, Label2L(LOC(MSG_minute_cost))       , Child, it_costrest  = Int(),
  						Child, Label2L(LOC(MSG_wait_dialing))      , Child, it_waitdial  = Int(),
  					End,
  					Child, HGroup,
  						Child, cm_logcall = Check(), Child, Label1L(LOC(MSG_log_calls)),
  						Child, Rectangle(),
  					End,
  					Child, Rectangle(),
  				End,
  				Child, VGroup, /*** serial ***/
  					GroupFrame,
  					Child, ColGroup(2),
  						Child, Label1L(LOC(MSG_dte_rate))    , Child, cy_dte			= Cycle(t_dte),
  						Child, Label1L(LOC(MSG_databits))    , Child, cy_databits	=	Cycle(t_databits),
  						Child, Label1L(LOC(MSG_stopbits))    , Child, cy_stopbits	=	Cycle(t_stopbits),
  						Child, Label1L(LOC(MSG_parity))      , Child, cy_parity		=	Cycle(t_parity),
  						Child, Label1L(LOC(MSG_duplex))      , Child, cy_duplex		=	Cycle(t_duplex),
  						Child, Label1L(LOC(MSG_flow_control)), Child, cy_flow			=	Cycle(t_flow),
  					End,
  
  					Child, RectangleObject,MUIA_Rectangle_HBar,TRUE,MUIA_FixHeight,8,End,
  
  					Child, HGroup,
  						Child, ColGroup(2),
  							Child, Label1L(LOC(MSG_serial_device)),
  							Child, HGroup,
  								GroupSpacing(0),
  								Child, io_device = PopFile(),
  								Child, st_device = String(STRSIZE),
  							End,
  							Child, Label1L(LOC(MSG_serial_unit))  , Child, sl_unit      = Slider(0,255),
  							Child, Label1L(LOC(MSG_serial_buffer)), Child, sl_serbufsize= Slider(16384,256*1024),
  						End,
  						Child, ColGroup(2),
  							GroupFrame,
  							Child, cm_hardser   = Check(), Child, Label1L(LOC(MSG_hardware_serial)),
  							Child, cm_fixrate   = Check(), Child, Label1L(LOC(MSG_fixed_dte_rate)),
  							Child, cm_dtrhangup = Check(), Child, Label1L(LOC(MSG_drop_dtr_to_hangup)),
                Child, cm_sershared = Check(), Child, Label1L(LOC(MSG_serial_shared)),
  							Child, Rectangle()           , Child, RectangleObject,MUIA_HorizWeight,0,End,
  						End,
  					End,
  					Child, Rectangle(),
  				End,
  				Child, VGroup, /*** screen ***/
  					Child, HGroup,
  						Child, lv_cs = ListView(),
  						Child, VGroup,
  							GroupFrame,
  							Child, HGroup,
  								Child, ColGroup(2),
  									Child, cm_borblank  = Check(), Child, Label1L(LOC(MSG_borderblank)),
  									Child, cm_keepwb    = Check(), Child, Label1L(LOC(MSG_keep_workbench_colors)),
  									Child, cm_rtsblit   = Check(), Child, Label1L(LOC(MSG_rts_blit)),
  								End,
  								Child, ColGroup(2),
  									Child, cm_hardscr = Check(), Child, Label1L(LOC(MSG_hardware_screen)),
  									Child, cm_aga     = Check(), Child, Label1L(LOC(MSG_enable_aga_bandwith)),
  									Child, cm_poprxd  = Check(), Child, Label1L(LOC(MSG_auto_rxd_popup)),
  								End,
  							End,
  							Child, ColGroup(2),
  								Child, Label1L(LOC(MSG_status))        , Child, cy_status     = Cycle(t_status),
                  Child, Label1L(LOC(MSG_pe_status_line)), Child, st_statusline = String(STRSIZE),
  								Child, Label2L(LOC(MSG_pe_public_screen_name)), 
  								Child, HGroup,
  									Child, st_pubscreen = String(STRSIZE),
  									Child, RectangleObject,MUIA_Rectangle_VBar,TRUE,MUIA_FixWidth,8,End,
  									Child, cm_wb        = Check(), Child, Label1L(LOC(MSG_pe_use_public_screen)),
  								End,
  							End,
                Child, Rectangle(),
  						End,
  					End,
  					Child, VGroup,
  						GroupFrame,
  						Child, HGroup,
  							Child, bt_modeid     = MakeButton(LOC(MSG_screen_mode)),
  							Child, cm_forceconxy = Check(), Child, Label1L(LOC(MSG_pe_force_screenreqs_size)),
  						End,
  						Child, ColGroup(2),
  							Child, Label1L(LOC(MSG_columns))          , Child, sl_columns = Slider( 1,256),
  							Child, Label1L(LOC(MSG_lines))            , Child, sl_lines   = Slider( 1,256),
  							Child, Label1L(LOC(MSG_pe_hardware_lines)), Child, sl_hlines  = Slider(24, 32),
  						End,
  						Child, tx_scrsize = MyText(),
  					End,
  				End,
  				Child, VGroup,	/*** transfer ***/
  					GroupFrame,
            Child, lv_xp = ListView(),
            Child, HGroup,
  						Child, ColGroup(2),
  							Child, Label2L(LOC(MSG_upload_path))  , Child, st_upath  = String(STRSIZE),
  							Child, Label2L(LOC(MSG_download_path)), Child, st_dpath  = String(STRSIZE),
  							Child, Label2L(LOC(MSG_download_list)), Child, st_dnfiles= String(STRSIZE),
  						End,
  						Child, ColGroup(2),
  							GroupFrame,
  							Child, cm_autoxfer  = Check(), Child, Label1L(LOC(MSG_auto_transfer)),
  							Child, cm_quietxfer = Check(), Child, Label1L(LOC(MSG_minimal_transfer_status)),
  							Child, cm_double    = Check(), Child, Label1L(LOC(MSG_double_buffer)),
  						End,
  					End,
  				End,
  				Child, VGroup, /*** emulation ***/
  					GroupFrame,
  					Child, cy_emulation = Cycle(t_emulation),
  					Child, empage = PageGroup,
  						Child, HGroup,
  							Child, VGroup,
  								Child, pa_ansipal = PaletteObject,
  									MUIA_Palette_Entries,ansientry,
  									MUIA_Palette_Names  ,ansiname,
  								End,
  								Child, HGroup,
  									GroupSpacing(4),
  									Child, bt_ansidef		= MakeButton(LOC(MSG_default)),
  									Child, bt_ansibright= MakeButton(LOC(MSG_brighter)),
  									Child, bt_ansicopy	= MakeButton(LOC(MSG_copy_8)),
  								End,
  							End,
  							Child, VGroup,
  								GroupFrame,
  								Child, Rectangle(),
  								Child, ColGroup(2),
  									Child, cm_wrap			= Check(), Child, Label1L(LOC(MSG_line_wrap)),
  									Child, cm_killbs		= Check(), Child, Label1L(LOC(MSG_destructive_backspace)),
  									Child, cm_stripbold	= Check(), Child, Label1L(LOC(MSG_strip_bold)),
  									Child, cm_cls				= Check(), Child, Label1L(LOC(MSG_esc2j_homes_cursor)),
  									Child, cm_emcr2crlf	= Check(), Child, Label1L(LOC(MSG_cr_lf_to_cr_lf)),
  									Child, cm_ansipc		= Check(), Child, Label1L(LOC(MSG_pc_ansi)),
  								End,
  								Child, Label1L(LOC(MSG_pe_prescroll)), Child, sl_prescroll = Slider(0,8192),
                  Child, Rectangle(),
  							End,
  						End,
  						Child, VGroup,
  							Child, pa_hexpal = PaletteObject,
  								MUIA_Palette_Entries,hexentry,
  								MUIA_Palette_Names  ,hexname,
  							End,
  							Child, bt_hexdef = SimpleButton(LOC(MSG_default)),
  						End,
  						Child, VGroup,
  							Child, lv_xm = ListView(),
  						End,
  					End,
  				End,
  				Child, HGroup,
  					Child, VGroup, /*** keyboard ***/
  						GroupFrame,
  						Child, ColGroup(4),
  							Child, Label2L(LOC(MSG_f1)), Child, st_func01 = String(STRSIZE), Child, Label2L(LOC(MSG_f6)) , Child, st_func06 = String(STRSIZE),
  							Child, Label2L(LOC(MSG_f2)), Child, st_func02 = String(STRSIZE), Child, Label2L(LOC(MSG_f7)) , Child, st_func07 = String(STRSIZE),
  							Child, Label2L(LOC(MSG_f3)), Child, st_func03 = String(STRSIZE), Child, Label2L(LOC(MSG_f8)) , Child, st_func08 = String(STRSIZE),
  							Child, Label2L(LOC(MSG_f4)), Child, st_func04 = String(STRSIZE), Child, Label2L(LOC(MSG_f9)) , Child, st_func09 = String(STRSIZE),
  							Child, Label2L(LOC(MSG_f5)), Child, st_func05 = String(STRSIZE), Child, Label2L(LOC(MSG_f10)), Child, st_func10 = String(STRSIZE),
  						End,
  						Child, HGroup,
  							Child, ColGroup(2),
  								Child, cm_cr2crlf = Check(), Child, Label1L(LOC(MSG_cr_to_crlf)),
  								Child, cm_lf2crlf = Check(), Child, Label1L(LOC(MSG_lf_to_crlf)),
  							End,
                Child, Rectangle(),
              End,
              Child, Rectangle(),
            End,
          End,
  
  				Child, VGroup, /*** modem ***/
  					Child, VGroup,
  						GroupFrame,
  						Child, ColGroup(4),
  							Child, Label2L(LOC(MSG_dial_prefix)), Child, st_dialstring= String(STRSIZE), Child, Label2L(LOC(MSG_dial_suffix))  , Child, st_dialsuffix = String(STRSIZE),
  							Child, Label2L(LOC(MSG_no_carrier)) , Child, st_nocarrier	= String(STRSIZE), Child, Label2L(LOC(MSG_no_dialtone))  , Child, st_nodialtone = String(STRSIZE),
  							Child, Label2L(LOC(MSG_connect))		, Child, st_connect		= String(STRSIZE), Child, Label2L(LOC(MSG_ringing))      , Child, st_ringing		= String(STRSIZE),
  							Child, Label2L(LOC(MSG_error))			, Child, st_error			= String(STRSIZE), Child, Label2L(LOC(MSG_busy))         , Child, st_busy			  = String(STRSIZE),
  							Child, Label2L(LOC(MSG_ok_string))	, Child, st_modemok		= String(STRSIZE), Child, Label2L(LOC(MSG_hangup_string)), Child, st_hangupcom	= String(STRSIZE),
  							Child, Label2L(LOC(MSG_dial_abort)) , Child, st_dialabort	= String(STRSIZE), Child, Rectangle()						         , Child, Rectangle(),
  						End,
  						Child, Rectangle(),
  					End,
  				End,
  			End,
  			Child, HGroup,
  				Child, bt_ok		= MakeButton  (LOC(MSG_key_ok)),
  				Child, tg_pick	= SimpleToggle(LOC(MSG_pe_set_pick)),
  				Child, tg_clr		= SimpleToggle(LOC(MSG_pe_clear_pick)),
  				Child, bt_cancel= MakeButton  (LOC(MSG_key_cancel)),
  			End,
  		End,
  	End;

    if (win) {
      DoMethod(gl_app,OM_ADDMEMBER,win);

			DoMethod(win,MUIM_MultiSet,MUIA_CycleChain,1,
							 /*** bbs ***/
							 st_name,st_phonepre,st_phone,st_password,
							 st_comment,st_logname,it_costfirst,it_costrest,
							 it_waitentry,it_waitdial,cm_logcall,
							 /*** ser ***/
							 cy_dte,cy_databits,cy_stopbits,cy_parity,cy_duplex,cy_flow,
							 cm_hardser,cm_fixrate,cm_dtrhangup,cm_sershared,io_device,
               st_device,sl_unit,sl_serbufsize,
							 /*** scr ***/
							 lv_cs,cm_borblank,cm_keepwb,cm_rtsblit,cm_hardscr,cm_aga,
							 cm_poprxd,cy_status,st_statusline,st_pubscreen,cm_wb,bt_modeid,
               cm_forceconxy,sl_columns,sl_lines,sl_hlines,
							 /*** xpr ***/
							 lv_xp,st_upath,st_dpath,st_dnfiles,cm_autoxfer,cm_quietxfer,
							 cm_double,
							 /*** key ***/
							 st_func01,st_func02,st_func03,st_func04,st_func05,st_func06,
							 st_func07,st_func08,st_func09,st_func10,cm_cr2crlf,cm_lf2crlf,
							 /*** emul ***/
							 cy_emulation,pa_ansipal,bt_ansidef,bt_ansibright,bt_ansicopy,
							 cm_wrap,cm_killbs,cm_stripbold,cm_cls,cm_emcr2crlf,cm_ansipc,
							 sl_prescroll,pa_hexpal,bt_hexdef,lv_xm,
							 /*** mod  ***/
							 st_dialstring,st_dialsuffix,st_nocarrier,st_nodialtone,
							 st_connect,st_ringing,st_error,st_busy,st_modemok,
							 st_hangupcom,st_dialabort,
	
							 bt_ok,tg_pick,tg_clr,bt_cancel,NULL);
	
			DoMethod(win				,MUIM_Notify,MUIA_Window_CloseRequest,TRUE, gl_app,2,MUIM_Application_ReturnID,ID_cancel);
	
			MultiIDNotify(MUIA_String_Contents,MUIV_EveryTime,
									st_name				,ID_name,
									st_phonepre		,ID_phonepre,
									st_phone			,ID_phone,
									st_password		,ID_password,
									st_comment		,ID_comment,
									st_logname		,ID_logname,
									it_costfirst	,ID_costfirst,
									it_costrest		,ID_costrest,
									it_waitentry	,ID_waitentry,
									it_waitdial		,ID_waitdial, TAG_END);
/* FUCK! MUI don't notify this...
			MultiIDNotify(MUIA_String_Integer,MUIV_EveryTime,
									it_costfirst	,ID_costfirst,
									it_costrest		,ID_costrest,
									it_waitentry	,ID_waitentry,
									it_waitdial		,ID_waitdial, TAG_END);
*/
			DoMethod(cm_logcall		,MUIM_Notify,MUIA_Selected,MUIV_EveryTime, gl_app,2,MUIM_Application_ReturnID,ID_logcall);
	
			MultiIDNotify(MUIA_Cycle_Active,MUIV_EveryTime,
									cy_dte				,ID_dte,
									cy_databits		,ID_databits,
									cy_stopbits		,ID_stopbits,
									cy_parity			,ID_parity,
									cy_duplex			,ID_duplex,
									cy_flow				,ID_flow, TAG_END);  
			DoMethod(io_device		,MUIM_Notify,MUIA_Pressed,FALSE, gl_app,2,MUIM_Application_ReturnID,ID_deviceio);
			DoMethod(st_device		,MUIM_Notify,MUIA_String_Contents,MUIV_EveryTime, gl_app,2,MUIM_Application_ReturnID,ID_device);
			DoMethod(sl_unit			,MUIM_Notify,MUIA_Slider_Level,MUIV_EveryTime, gl_app,2,MUIM_Application_ReturnID,ID_unit);
			DoMethod(sl_serbufsize,MUIM_Notify,MUIA_Slider_Level,MUIV_EveryTime, gl_app,2,MUIM_Application_ReturnID,ID_serbufsize);
			MultiIDNotify(MUIA_Selected,MUIV_EveryTime,
									cm_hardser		,ID_hardser,
									cm_fixrate		,ID_fixrate,
                  cm_dtrhangup  ,ID_dtrhangup,
                  cm_sershared  ,ID_sershared, TAG_END);

			DoMethod(lv_cs				,MUIM_Notify,MUIA_List_Active,MUIV_EveryTime, gl_app,2,MUIM_Application_ReturnID,ID_cs);
			MultiIDNotify(MUIA_Selected,MUIV_EveryTime,
									cm_hardscr		,ID_hardscr,
									cm_borblank		,ID_borblank,
									cm_wb					,ID_wb,
									cm_keepwb			,ID_keepwb,
									cm_aga				,ID_aga,
									cm_poprxd			,ID_poprxd,
									cm_rtsblit		,ID_rtsblit, TAG_END);
			DoMethod(st_pubscreen	,MUIM_Notify,MUIA_String_Contents,MUIV_EveryTime, gl_app,2,MUIM_Application_ReturnID,ID_pubscreen);
			DoMethod(cy_status    ,MUIM_Notify,MUIA_Cycle_Active   ,MUIV_EveryTime, gl_app,2,MUIM_Application_ReturnID,ID_status);
      DoMethod(st_statusline,MUIM_Notify,MUIA_String_Contents,MUIV_EveryTime, gl_app,2,MUIM_Application_ReturnID,ID_statusline);
			DoMethod(cm_forceconxy,MUIM_Notify,MUIA_Selected,MUIV_EveryTime, gl_app,2,MUIM_Application_ReturnID,ID_forceconxy);
			MultiIDNotify(MUIA_Slider_Level,MUIV_EveryTime,
									sl_columns		,ID_columns,
									sl_lines			,ID_lines,
									sl_hlines			,ID_hlines, TAG_END);
			DoMethod(bt_modeid		,MUIM_Notify,MUIA_Pressed,FALSE,	gl_app,2,MUIM_Application_ReturnID,ID_modeid);
	
			DoMethod(lv_xp				,MUIM_Notify,MUIA_List_Active,MUIV_EveryTime, gl_app,2,MUIM_Application_ReturnID,ID_xp);
			MultiIDNotify(MUIA_String_Contents,MUIV_EveryTime,
									st_upath			,ID_upath,
									st_dpath			,ID_dpath,
									st_dnfiles		,ID_dnfiles, TAG_END);
			MultiIDNotify(MUIA_Selected,MUIV_EveryTime,
									cm_autoxfer		,ID_autoxfer,
									cm_quietxfer	,ID_quietxfer,
									cm_double			,ID_double, TAG_END);
	
			MultiIDNotify(MUIA_String_Contents,MUIV_EveryTime,
									st_func01			,ID_func01,
									st_func02			,ID_func02,
									st_func03			,ID_func03,
									st_func04			,ID_func04,
									st_func05			,ID_func05,
									st_func06			,ID_func06,
									st_func07			,ID_func07,
									st_func08			,ID_func08,
									st_func09			,ID_func09,
									st_func10			,ID_func10, TAG_END);
			MultiIDNotify(MUIA_Selected,MUIV_EveryTime,
									cm_cr2crlf		,ID_cr2crlf,
									cm_lf2crlf		,ID_lf2crlf, TAG_END);
	
			DoMethod(cy_emulation	,MUIM_Notify,MUIA_Cycle_Active,MUIV_EveryTime, gl_app,2,MUIM_Application_ReturnID,ID_emulation);
			MultiIDNotify(MUIA_Selected,MUIV_EveryTime,
									cm_wrap				,ID_wrap,
									cm_killbs			,ID_killbs,
									cm_stripbold	,ID_stripbold,
									cm_cls				,ID_cls,
									cm_emcr2crlf	,ID_emcr2crlf,
									cm_ansipc			,ID_ansipc, TAG_END);
			DoMethod(sl_prescroll	,MUIM_Notify,MUIA_Slider_Level,MUIV_EveryTime, gl_app,2,MUIM_Application_ReturnID,ID_prescroll);
			MultiIDNotify(MUIA_Pressed,FALSE,
									bt_ansidef		,ID_ansidef,
									bt_ansibright	,ID_ansibright,
									bt_ansicopy		,ID_ansicopy, TAG_END);
	
			DoMethod(bt_hexdef		,MUIM_Notify,MUIA_Pressed,FALSE,	gl_app,2,MUIM_Application_ReturnID,ID_hexdef);
			DoMethod(lv_xm				,MUIM_Notify,MUIA_List_Active,MUIV_EveryTime, gl_app,2,MUIM_Application_ReturnID,ID_xm);
	
			MultiIDNotify(MUIA_String_Contents,MUIV_EveryTime,
									st_dialstring	,ID_dialstring,
									st_dialsuffix	,ID_dialsuffix,
									st_nocarrier	,ID_nocarrier,
									st_nodialtone	,ID_nodialtone,
									st_connect		,ID_connect,
									st_ringing		,ID_ringing,
									st_error			,ID_error,
									st_busy				,ID_busy,
									st_modemok		,ID_modemok,
									st_hangupcom	,ID_hangupcom,
									st_dialabort	,ID_dialabort, TAG_END);
	
			MultiIDNotify(MUIA_Pressed,FALSE,
									bt_ok					,ID_ok,
									bt_cancel			,ID_cancel, TAG_END);
			MultiIDNotify(MUIA_Selected,MUIV_EveryTime,
									tg_pick				,ID_pick,
									tg_clr				,ID_clr, TAG_END);
      win_act = TRUE;
    }
  }
  return(win_act);
}

/*  Interface pääohjelmaan
 *
 *  Commands:
 *
 *  PE_EDIT			[entry]	- Editoi entryä -> laita listaan jos OK
 *  PE_NEW							- Tee uusi entry -> laita listaan jos OK
 *  PE_CLONE						- Kopio entry -> laita listaan jos OK
 *  PE_PICK							- Editoi pick tablea (gl_defbbs)
 *  PE_CURRENT					- Editoi current settingsejä (gl_defbbs)
 */

ulong Entry(ULONG tag, ULONG data)
{
static BOOL ocall = FALSE; static ULONG oc;
ulong i;

  switch (tag)
  {
    case DT_CLOSE:
      if (win_act) {
        set(win,MUIA_Window_Open,FALSE);
        if (CUR(PB_Dispose)) {
          DoMethod(gl_app,OM_REMMEMBER,win);
          MUI_DisposeObject(win);
          win_act = FALSE;
        }
      }
      break;
    case DT_OC:
      if (win_act) {
        if (ocall) {
          set(win,MUIA_Window_Screen,gl_conscreen);
          set(win,MUIA_Window_Open,oc);
          set(win,MUIA_Window_Sleep,gl_req);
          ocall = FALSE;
        } else {
          get(win,MUIA_Window_Open,&oc);
          set(win,MUIA_Window_Open,FALSE);
          set(win,MUIA_Window_Screen,0);
          ocall = TRUE;
        }
      }
      break;
    case DT_INIT:
      LocalizeList(t_pages    ,sizeof(t_pages    )/4);
      LocalizeList(t_dte      ,sizeof(t_dte      )/4);
      LocalizeList(t_databits ,sizeof(t_databits )/4);
      LocalizeList(t_stopbits ,sizeof(t_stopbits )/4);
      LocalizeList(t_parity   ,sizeof(t_parity   )/4);
      LocalizeList(t_duplex   ,sizeof(t_duplex   )/4);
      LocalizeList(t_flow     ,sizeof(t_flow     )/4);
      LocalizeList(t_status   ,sizeof(t_status   )/4);
      LocalizeList(t_emulation,sizeof(t_emulation)/4);
      LocalizeList(ansiname   ,sizeof(ansiname   )/4);
      LocalizeList(hexname    ,sizeof(hexname    )/4);
      return(TRUE);
      break;
    case DT_HANDLE:
      switch (data)
      {
				/*** bbs ***/
				case ID_name:				HandleString(st_name			,PB_Name);			break;
				case ID_phonepre:		HandleString(st_phonepre	,PB_PhonePre);	break;
				case ID_phone:			HandleString(st_phone			,PB_Phone);			break;
				case ID_password:		HandleString(st_password	,PB_PassWord);	break;
				case ID_comment:		HandleString(st_comment		,PB_Comment);		break;
				case ID_costfirst:	HandleInt		(it_costfirst	,PB_CostFirst);	break;
				case ID_costrest:		HandleInt		(it_costrest	,PB_CostRest);	break;
				case ID_waitentry:	HandleInt		(it_waitentry	,PB_WaitEntry);	break;
				case ID_waitdial:		HandleInt		(it_waitdial	,PB_WaitDial);	break;
				case ID_logname:		HandleString(st_logname		,PB_LogName);		break;
				case ID_logcall:		HandleCheck	(cm_logcall		,PB_LogCall);		break;
				/*** serial ***/
				case ID_dte:				HandleCycle	(cy_dte				,PB_DteRate			,i_dte);			break;
				case ID_databits:		HandleCycle	(cy_databits	,PB_DataBits		,i_databits);	break;
				case ID_stopbits:		HandleCycle	(cy_stopbits	,PB_StopBits		,i_stopbits);	break;
				case ID_parity:			HandleCycle	(cy_parity		,PB_Parity			,i_parity);		break;
				case ID_duplex:			HandleCycle	(cy_duplex		,PB_FullDuplex	,i_duplex);		break;
				case ID_flow:				HandleCycle	(cy_flow			,PB_FlowControl	,i_flow);    	break;
				case ID_deviceio:		device_click(); break;
				case ID_device:			HandleString(st_device		,PB_SerName);		break;
				case ID_unit:				HandleSlider(sl_unit			,PB_SerUnit);		break;
				case ID_serbufsize:	HandleSlider(sl_serbufsize,PB_SerBufSize);break;
				case ID_hardser:		HandleCheck	(cm_hardser		,PB_HardSer);		break;
				case ID_fixrate:		HandleCheck	(cm_fixrate		,PB_FixRate);		break;
        case ID_dtrhangup:  HandleCheck (cm_dtrhangup ,PB_DTRHangup); break;
        case ID_sershared:  HandleCheck (cm_sershared ,PB_SerShared); break;
				/*** screen ***/
				case ID_cs:					HandleList	(lv_cs				,PB_Charset); Dimension();	break;
				case ID_hardscr:		HandleCheck	(cm_hardscr		,PB_HardScr); CsToGui();		break;
				case ID_borblank:		HandleCheck	(cm_borblank	,PB_BorderBlank);break;
				case ID_keepwb:			HandleCheck	(cm_keepwb		,PB_KeepWb);		break;
				case ID_aga:				HandleCheck	(cm_aga				,PB_Aga);				break;
				case ID_poprxd:			HandleCheck	(cm_poprxd		,PB_RxdPopup);	break;
				case ID_rtsblit:		HandleCheck	(cm_rtsblit		,PB_RtsBlit);		break;
				case ID_pubscreen:	HandleString(st_pubscreen	,PB_PubScreen);	break;
				case ID_wb:					HandleCheck	(cm_wb				,PB_Wb);				break;
				case ID_status:			HandleCycle	(cy_status		,PB_Status	,i_status);			break;
        case ID_statusline: HandleString(st_statusline,PB_StatusLine);break;
				case ID_modeid:			modeid(); break;
				case ID_forceconxy:	HandleCheck	(cm_forceconxy,PB_ForceConXY);Dimension();	break;
				case ID_columns:		HandleSlider(sl_columns		,PB_Columns); 	Dimension();	break;
				case ID_lines:			HandleSlider(sl_lines			,PB_Lines);			Dimension();	break;
				case ID_hlines:			HandleSlider(sl_hlines		,PB_HLines);		break;
				/* Transfer */
				case ID_xp:					HandleList	(lv_xp				,PB_Protocol);	break;
				case ID_upath:			HandleString(st_upath			,PB_UpPath);		break;
				case ID_dpath:			HandleString(st_dpath			,PB_DnPath);		break;
				case ID_dnfiles:		HandleString(st_dnfiles		,PB_DnFiles);		break;
				case ID_autoxfer:		HandleCheck	(cm_autoxfer	,PB_AutoXfer);	break;
				case ID_quietxfer:	HandleCheck	(cm_quietxfer	,PB_QuietXfer);	break;
				case ID_double:			HandleCheck	(cm_double		,PB_Double);		break;
				/* Keyboard */
				case ID_func01:			HandleString(st_func01		,PB_Func1);			break;
				case ID_func02:			HandleString(st_func02		,PB_Func2);			break;
				case ID_func03:			HandleString(st_func03		,PB_Func3);			break;
				case ID_func04:			HandleString(st_func04		,PB_Func4);			break;
				case ID_func05:			HandleString(st_func05		,PB_Func5);			break;
				case ID_func06:			HandleString(st_func06		,PB_Func6);			break;
				case ID_func07:			HandleString(st_func07		,PB_Func7);			break;
				case ID_func08:			HandleString(st_func08		,PB_Func8);			break;
				case ID_func09:			HandleString(st_func09		,PB_Func9);			break;
				case ID_func10:			HandleString(st_func10		,PB_Func10);		break;
				case ID_cr2crlf:		HandleCheck	(cm_cr2crlf		,PB_cr2crlf);		break;
				case ID_lf2crlf:		HandleCheck	(cm_lf2crlf		,PB_lf2crlf);		break;
				/* Emulation */
				case ID_emulation:	emulation();	break;
				case ID_wrap:				HandleCheck	(cm_wrap			,PB_ANSIWrap);	break;
				case ID_killbs:			HandleCheck	(cm_killbs		,PB_ANSIKillBs);break;
				case ID_stripbold:	HandleCheck	(cm_stripbold	,PB_ANSIStrip);	break;
				case ID_cls:				HandleCheck	(cm_cls				,PB_ANSICls);		break;
				case ID_emcr2crlf:	HandleCheck	(cm_emcr2crlf	,PB_ANSICr2crlf);break;
				case ID_ansipc:			HandleCheck	(cm_ansipc		,PB_ANSIPc);		break;
				case ID_prescroll:	HandleSlider(sl_prescroll	,PB_PreScroll); break;
				case ID_ansidef:		ansidef();		break;
				case ID_ansibright:	ansibright();	break;
				case ID_ansicopy:		ansicopy();		break;
				case ID_hexdef:			hexdef();			break;
				case ID_xm:					HandleList	(lv_xm				,PB_Xem);				break;
        /* Modem */
        case ID_dialstring: HandleString(st_dialstring,PB_DialString);break;
        case ID_dialsuffix: HandleString(st_dialsuffix,PB_DialSuffix);break;
        case ID_nocarrier:  HandleString(st_nocarrier ,PB_NoCarrier); break;
        case ID_nodialtone: HandleString(st_nodialtone,PB_NoDialTone);break;
        case ID_connect:    HandleString(st_connect   ,PB_Connect);   break;
        case ID_ringing:    HandleString(st_ringing   ,PB_Ringing);   break;
        case ID_error:      HandleString(st_error     ,PB_Error);     break;
        case ID_busy:       HandleString(st_busy      ,PB_Busy);      break;
        case ID_modemok:    HandleString(st_modemok   ,PB_Ok);        break;
        case ID_hangupcom:  HandleString(st_hangupcom ,PB_Hangup);    break;
        case ID_dialabort:  HandleString(st_dialabort ,PB_DialAbort); break;

        case ID_ok:     ok_click();    break;
        case ID_pick:   pick_click();  break;
        case ID_clr:    clr_click();   break;
        case ID_cancel: cancel_click();break;
      }
      break;
    case DT_UPDATE:
      if (win_act) {
        if (nowstate) {
          CsToGui();
          XpToGui();
          XmToGui();
          set(win,MUIA_Window_Sleep,gl_req);
        }
      }
      break;
    case PE_EDIT:
      if (nowstate == 0) {
        if (open_window()) {
          if (newpb = CreateInfo()) {
            oldpb = (struct Info *)data;
            CopyTags(&oldpb->tags,&newpb->tags);
            Phone(DT_LOCK,oldpb);
            nowedit  = EDITIT;
            nowstate = tag;
            DisablePick(&newpb->tags);
            EntryToGui();
            strncpy(curtitle,LOC(MSG_pe_editing_phonebook_entry),STRSIZE);
            nnset(tg_pick ,MUIA_Selected,FALSE);
            nnset(tg_clr  ,MUIA_Selected,FALSE);
            set(tg_pick   ,MUIA_Disabled,FALSE);
            set(tg_clr    ,MUIA_Disabled,FALSE);
            set(win       ,MUIA_Window_Title,curtitle);
            set(win       ,MUIA_Window_Screen,gl_conscreen);
            set(win       ,MUIA_Window_Open,TRUE);
            set(win       ,MUIA_Window_Sleep,gl_req); /* fuck mui */
            set(win       ,MUIA_Window_Activate,TRUE);
          } else {
            Entry(DT_CLOSE,0);
            Warning(LOC(MSG_no_memory_to_edit_entry));
          }
        }
      }
      break;
    case PE_NEW:
      if (nowstate == 0) {
        if (open_window()) {
          if (newpb = CreateInfo()) {
            for (i = STR_PB_FIRST; i <= STR_PB_LAST; i++) if (! LookInfoTag(i,gl_fromdef) ) AddInfoTag(i,GetInfoTag(i,gl_defbbs),newpb);
            for (i = VAL_PB_FIRST; i <= VAL_PB_LAST; i++) if (! LookInfoTag(i,gl_fromdef) ) AddInfoTag(i,GetInfoTag(i,gl_defbbs),newpb);
            for (i = DAT_PB_FIRST; i <= DAT_PB_LAST; i++) if (! LookInfoTag(i,gl_fromdef) ) AddInfoTag(i,GetInfoTag(i,gl_defbbs),newpb);
            DelInfoTags(newpb,
                        ENV_TAGS     ,
                        PB_CryptKey  ,
                        PB_obsolete1 ,PB_obsolete2 ,PB_obsolete3, /* obsolete */
                        TAG_END);
            AddInfoTags(newpb,
                        PB_CallLast ,0,
                        PB_CallCost ,0,
                        PB_CallTime ,0,
                        PB_CallCount,0,
                        PB_CallTxd  ,0,
                        PB_CallRxd  ,0, TAG_END);
            nowedit  = EDITIT;
            nowstate = tag;
            DisablePick(&newpb->tags);
            EntryToGui();
            strncpy(curtitle,LOC(MSG_creating_new_phonebook_entry),STRSIZE);
            nnset(tg_pick,MUIA_Selected,FALSE);
            nnset(tg_clr ,MUIA_Selected,FALSE);
            set(tg_pick  ,MUIA_Disabled,FALSE);
            set(tg_clr   ,MUIA_Disabled,FALSE);
            set(win      ,MUIA_Window_Title,curtitle);
            set(win      ,MUIA_Window_Screen,gl_conscreen);
            set(win      ,MUIA_Window_Open,TRUE);
            set(win      ,MUIA_Window_Sleep,gl_req);  /* fuck mui */
            set(win      ,MUIA_Window_Activate,TRUE);
          } else {
            Entry(DT_CLOSE,0);
            Warning(LOC(MSG_cannot_create_entry));
          }
        }
      }
      break;
    case PE_CLONE:
      if (nowstate == 0) {
        if (open_window()) {
          if (newpb = CreateInfo()) {
            oldpb = (struct Info *)data;
            CopyTags(&oldpb->tags,&newpb->tags);
            DelInfoTag (PB_Freeze,newpb);
            AddInfoTags(newpb,
                        PB_CallLast ,0,
                        PB_CallCost ,0,
                        PB_CallTime ,0,
                        PB_CallCount,0,
                        PB_CallTxd  ,0,
                        PB_CallRxd  ,0, TAG_END);
            nowedit  = EDITIT;
            nowstate = tag;
            DisablePick(&newpb->tags);
            EntryToGui();
            strncpy(curtitle,LOC(MSG_creating_new_phonebook_entry),STRSIZE);
            nnset(tg_pick,MUIA_Selected,FALSE);
            nnset(tg_clr ,MUIA_Selected,FALSE);
            set(tg_pick  ,MUIA_Disabled,FALSE);
            set(tg_clr   ,MUIA_Disabled,FALSE);
            set(win      ,MUIA_Window_Title,curtitle);
            set(win      ,MUIA_Window_Screen,gl_conscreen);
            set(win      ,MUIA_Window_Open,TRUE);
            set(win      ,MUIA_Window_Sleep,gl_req); /* fuck mui */
            set(win      ,MUIA_Window_Activate,TRUE);
          } else {
            Entry(DT_CLOSE,0);
            Warning(LOC(MSG_cannot_create_entry));
          }
        }
      }
      break;
    case PE_PICK:
      if (nowstate == 0) {
        if (open_window()) {
          if (newpb = CreateInfo()) {
            CopyTags(&gl_defbbs->tags,&newpb->tags);
            nowedit  = EDITIT;
            nowstate = tag;
            DisablePick(&newpb->tags);
            EntryToGui();
            strncpy(curtitle,LOC(MSG_editing_profile),STRSIZE);
            nnset(tg_pick,MUIA_Selected,FALSE);
            nnset(tg_clr ,MUIA_Selected,FALSE);
            set(tg_pick  ,MUIA_Disabled,FALSE);
            set(tg_clr   ,MUIA_Disabled,FALSE);
            set(win      ,MUIA_Window_Title,curtitle);
            set(win      ,MUIA_Window_Screen,gl_conscreen);
            set(win      ,MUIA_Window_Open,TRUE);
            set(win      ,MUIA_Window_Sleep,gl_req); /* fuck mui */
            set(win      ,MUIA_Window_Activate,TRUE);
          } else {
            Entry(DT_CLOSE,0);
            Warning(LOC(MSG_cannot_create_entry));
          }
        }
      }
      break;
    case PE_CURRENT:
      if (nowstate == 0) {
        if (open_window()) {
          if (newpb = CreateInfo()) {
            CopyTags(&gl_curbbs->tags,&newpb->tags);
            nowedit  = EDITIT;
            nowstate = tag;
            DisablePick(&newpb->tags);
            EntryToGui();
            strncpy(curtitle,LOC(MSG_editing_current_settings),STRSIZE);
            nnset(tg_pick,MUIA_Selected,FALSE);
            nnset(tg_clr ,MUIA_Selected,FALSE);
            set(tg_pick  ,MUIA_Disabled,TRUE);
            set(tg_clr   ,MUIA_Disabled,TRUE);
            set(win      ,MUIA_Window_Title,curtitle);
            set(win      ,MUIA_Window_Screen,gl_conscreen);
            set(win      ,MUIA_Window_Open,TRUE);
            set(win      ,MUIA_Window_Sleep,gl_req); /* fuck mui */
            set(win      ,MUIA_Window_Activate,TRUE);
          } else {
            Entry(DT_CLOSE,0);
            Warning(LOC(MSG_cannot_create_entry));
          }
        }
      }
      break;
  }
}
