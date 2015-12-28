#include "global.h"
#include "loc_strings.h"

/***----------------------------------------------------------------------***/
/*** misc                                                                 ***/
/***----------------------------------------------------------------------***/

/* update every window, they disable/enable gadgets according gl_req etc. states */
void Update_windows(void)
{
  Charset  (DT_UPDATE,0);
  Crypt    (DT_UPDATE,0,0);
  Dialer   (DT_UPDATE,0);
  Drop     (DT_UPDATE,0);
  Entry    (DT_UPDATE,0);
  Fileid   (DT_UPDATE,0);
  Phone    (DT_UPDATE,0);
  Show     (DT_UPDATE,0);
  Toolbar  (DT_UPDATE,0);
  Transfer (DT_UPDATE,0);
  Xem      (DT_UPDATE,0);
  Menu_fresh();
}

/* show warning message to user */
void Warning(STRPTR msg)
{
  if ( gl_warning == TRUE AND MUIMasterBase != 0 ) {
    MUI_Request(gl_app,win_fake,0,LOC(MSG_warning),LOC(MSG_ok),msg,0);
  } else {
    TimedDisplayAlert(RECOVERY_ALERT,msg,64,600);
  }
}

/*
/***----------------------------------------------------------------------***/
/*** window opening and handle                                            ***/
/***----------------------------------------------------------------------***/

void hook_protocol(void)
{
}

ulong Protocol(ulong tag, ulong data)
{
static bool        ocall = FALSE;
static ulong       oc;

  switch ( tag )
  {
    case DT_CLOSE:
      set(gl_protocol,MUIA_Window_Open,FALSE);
      break;
    case DT_OPEN:
      set(gl_protocol,MUIA_Window_Screen  ,gl_conscreen);
      set(gl_protocol,MUIA_Window_Open    ,TRUE);
      set(gl_protocol,MUIA_Window_Activate,TRUE);
      break;
    case DT_OC:
      if (ocall) {
  			set(gl_protocol,MUIA_Window_Screen,gl_conscreen);
  			set(gl_protocol,MUIA_Window_Open,oc);
  			ocall = FALSE;
  		} else {
  			get(gl_protocol,MUIA_Window_Open,&oc);
  			set(gl_protocol,MUIA_Window_Open,FALSE);
  			set(gl_protocol,MUIA_Window_Screen,0);
  			ocall = TRUE;
  		}
			break;
		case DT_FIND:
      return(DoMethod(gl_protocol,MUIM_INF_Find,data));
			break;
		case DT_LOAD:
      return(DoMethod(gl_protocol,MUIM_INF_Load,data));
			break;
		case DT_SAVE:
      return(DoMethod(gl_protocol,MUIM_INF_Save,data));
			gl_protosafe = TRUE;
			break;
		case DT_GET:
      return(DoMethod(gl_protocol,MUIM_INF_Get,data));
			break;
		case DT_LOCK:
      DoMethod(gl_protocol,MUIM_INF_Lock,data);
			break;
		case DT_UNLOCK:
      DoMethod(gl_protocol,MUIM_INF_Unlock,data);
      break;
    case DT_COUNT:
      return(DoMethod(gl_protocol,MUIM_INF_Count,data));
      break;
  }
}
*/

/***----------------------------------------------------------------------***/
/*** MUI 'inside' class asl requester support                             ***/
/***                                                                      ***/
/*** These just spawn new task and use PushMethodID after requester is    ***/
/*** done or cancelled so there is no rule that req must be started from  ***/
/*** inside clas. However object must be exist and receiving object must  ***/
/*** also free request using cl_remreq()!                                 ***/
/***----------------------------------------------------------------------***/

struct CL_Msg 
{
  struct Message msg;
  struct TagItem tags[32];
  ulong          sub;
  ulong          type;
  ulong          result;
};

struct CL_Sub
{
  struct Process *cl_child; /* child process created */
  struct MsgPort *cl_port;  /* allocated by child    */
  struct MsgPort *cl_reply; /* allocated by main     */
  struct CL_Msg   cl_msg;   /* message               */
};

/*
 * Requester task
 *
 */
static __saveds __asm void task_class_req(void)
{
bool                  ret;
struct CL_Sub        *sub;
struct CL_Msg        *msg;
struct CL_Msg         par_msg;
struct Process       *proc = FindTask(0);
struct FileRequester *req;

  /* wait startup message */
  while( !(msg = GetMsg(&proc->pr_MsgPort)) ) 
  {
    WaitPort(&proc->pr_MsgPort);
  }

  /* copy msg data and initialize */
  sub     = msg->sub;
  par_msg = *msg;
  if (!(sub->cl_port = CreateMsgPort())) {
    msg->result = FALSE;
    ReplyMsg(msg);
    return;
  }

  /* startup asl request */
  if (req = AllocAslRequest(par_msg.type,0)) {
    msg->result = TRUE;
    ReplyMsg(msg);
		ret = AslRequest(req,&par_msg.tags[2]);
	} else {
    msg->result = FALSE;
    ReplyMsg(msg);
    return;
  }

  /* push result to object */
  if (ret) {
    DoMethod(gl_app,MUIM_Application_PushMethod,
             par_msg.tags[0].ti_Data, /* object */
             2,
             par_msg.tags[1].ti_Data, /* method */
             req);
  } else { /* cancel or request did not start */
    DoMethod(gl_app,MUIM_Application_PushMethod,
             par_msg.tags[0].ti_Data, /* object */
             2,
             par_msg.tags[1].ti_Data, /* method */
             0);
  }

  /* wait termination message */
  while( !(msg = GetMsg(sub->cl_port)) ) 
  {
    WaitPort(sub->cl_port);
  }
  Forbid();
  FreeAslRequest(req);
  DeleteMsgPort(sub->cl_port);
  ReplyMsg(msg);
}

/*
 * Remove any requester immediately, you must call this after you have
 * received method and used its data.
 *
 * tmp   : Value returned by cl_addreq
 *
 * -     : this will always succeed
 *
 */
void cl_remreq(ulong tmp)
{
struct CL_Sub *sub = tmp;

  sub->cl_msg.mn_Length    = sizeof(struct CL_Msg);
  sub->cl_msg.mn_ReplyPort = sub->cl_reply;
  PutMsg(sub->cl_port,&sub->cl_msg);
  WaitPort(sub->cl_reply);
  GetMsg(sub->cl_reply);
	DeleteMsgPort(sub->cl_reply);
	FreeVec(sub);
}

/*
 * Start any asl requester as child
 *
 * type  : type for AllocAslRequest
 * tags  : tag list
 *         CL_Object - Object to receive method when done
 *         CL_Method - Method (we always push ASL struct or 0!)
 *                     these must be in this order and always present!
 *         ... after this normal asl request tags must follow
 *
 * ret   : CL_Sub which is passed to cl_remreq when exiting
 */
ulong cl_addreq(ulong type, ...)
{
ulong           i=0;
va_list         ap;
struct CL_Sub  *sub;

  /* create requester process and pass values to it */
  if (sub = AllocVec(sizeof(struct CL_Sub),MEMF_PUBLIC)) {
    if (sub->cl_reply = CreateMsgPort()) {
      if (sub->cl_child = CreateNewProcTags(NP_Entry ,&task_class_req,
                                            NP_Name  ,"dterm_class_req",
                                            TAG_DONE)) {
        /* copy tags for child */
        sub->cl_msg.sub  = sub;
        sub->cl_msg.type = type;
      	va_start(ap, type);
        while( (sub->cl_msg.tags[i].ti_Tag = va_arg(ap,ULONG)) != TAG_END )
        {
          sub->cl_msg.tags[i].ti_Data = va_arg(ap,ULONG);
          i++;
        }
      	va_end(ap);
        /* send initial message */
        sub->cl_msg.mn_Length    = sizeof(struct CL_Msg);
        sub->cl_msg.mn_ReplyPort = sub->cl_reply;
        PutMsg(&sub->cl_child->pr_MsgPort,&sub->cl_msg);
        WaitPort(sub->cl_reply);
        GetMsg(sub->cl_reply);
        if (sub->cl_msg.result) return(sub);
      }
      DeleteMsgPort(sub->cl_reply);
    }
    FreeVec(sub);
  }
  return(0);
}

/***----------------------------------------------------------------------***/
/*** Req 'links' for changing tags easily                                 ***/
/*** NOTE: nämä luottaa siihen että vain yksi req kerrallaan activena!!!! ***/
/***       (eli muista korjata jos joskus parantelet systeemiä)           ***/
/***----------------------------------------------------------------------***/

/* vaihda string tagin tilaa infonodesta kysymällä userilta (dir req) */
void Ask_dir_tag(char *prompt, ulong tag, struct Info *info)
{
static char        str[STRSIZE];
static ulong       tagsave;
static struct Info *infosave;

  if (!gl_req) {
    tagsave  = tag;
    infosave = info;
		strcpy(str,(char *)GetInfoTag(tag,info));
		Ask_dir(gl_conwindow,&Ask_dir_tag,prompt,str);
	} else {
		if ( Ask_dir_done() == REQ_OK ) {
			AddInfoTag(tagsave,(ULONG)str,infosave);
		}
	}
}

/* vaihda string tagin tilaa infonodesta kysymällä userilta (str req) */
void Ask_str_tag(char *prompt, ulong tag, struct Info *info)
{
static char        str[STRSIZE];
static ulong       tagsave;
static struct Info *infosave;

	if (!gl_req) {
    tagsave  = tag;
    infosave = info;
		strcpy(str,GetInfoTag(tag,info));
		Ask_string(&Ask_str_tag,prompt,str);
	} else {
		if ( Ask_string_done() == REQ_OK ) {
			AddInfoTag(tagsave,(ULONG)str,infosave);
		}
	}
}

/* vaihda value tagin tilaa infonodesta kysymällä userilta (slider req) */
void Ask_val_tag(char *prompt,ulong min, ulong max, ulong tag, struct Info *info)
{
static ulong       val;
static ulong       tagsave;
static struct Info *infosave;

	if (!gl_req) {
    tagsave  = tag;
    infosave = info;
		val = GetInfoTag(tag,info);
		Ask_slider(&Ask_val_tag,prompt,min,max,&val);
	} else {
		if ( Ask_slider_done() == REQ_OK ) {
			AddInfoTag(tagsave,val,infosave);
		}
	}
}

/* vaihda string tagin tilaa setupin kautta (dir req) */
void Ask_dir_cur(char *prompt, ulong tag)
{
static char  str[STRSIZE];
static ulong tagsave;

	if (!gl_req) {
    tagsave  = tag;
		strcpy(str,(char *)CUR(tag));
		Ask_dir(gl_conwindow,&Ask_dir_cur,prompt,str);
	} else {
		if ( Ask_dir_done() == REQ_OK ) {
			SetUpTag(tagsave,(ULONG)str);
		}
	}
}

/* vaihda string tagin tilaa setupin kautta (str req) */
void Ask_str_cur(char *prompt, ulong tag)
{
static char  str[STRSIZE];
static ulong tagsave;

  if (!gl_req) {
    tagsave  = tag;
    strcpy(str,CUR(tag));
    Ask_string(&Ask_str_cur,prompt,str);
  } else {
    if ( Ask_string_done() == REQ_OK ) {
      SetUpTag(tagsave,(ULONG)str);
    }
  }
}

/* vaihda value tagin tilaa setupin kautta (slider req) */
void Ask_val_cur(char *prompt,ulong min, ulong max, ulong tag)
{
static ulong val;
static ulong tagsave;

  if (!gl_req) {
    tagsave  = tag;
    val = CUR(tag);
    Ask_slider(&Ask_val_cur,prompt,min,max,&val);
  } else {
    if ( Ask_slider_done() == REQ_OK ) {
      SetUpTag(tagsave,val);
    }
  }
}

/***----------------------------------------------------------------------***/
/*** Requesters                                                           ***/
/***----------------------------------------------------------------------***/

enum lv_ID { ID_lv_lv = TE_First, ID_lv_ok, ID_lv_cancel };

static aptr  lv_win,lv_lv,lv_bt_ok,lv_bt_cancel;
static long  lv_stat;
static ulong *lv_old;

/* mainidcmp calls when requester should close its window temporaly (toggle) */
void Ask_list_oc(void)
{
static BOOL ocall=FALSE;

  if (ocall) {
    set(lv_win,MUIA_Window_Screen,gl_conscreen);
    set(lv_win,MUIA_Window_Open,TRUE);
    set(lv_win,MUIA_Window_Activate,TRUE);
    ocall = FALSE;
  } else {
    set(lv_win,MUIA_Window_Open,FALSE);
    set(lv_win,MUIA_Window_Screen,0);
    ocall = TRUE;
  }
}

/* mainidcmp calls this using gl_muireq pointer when id arrives */
void Ask_list_id(ULONG id)
{
  switch (id)
  {
    case ID_lv_lv:
    case ID_lv_ok:
      get(lv_lv,MUIA_List_Active,lv_old);
      if (*lv_old != MUIV_List_Active_Off) {
        lv_stat = REQ_OK;
      } else {
        lv_stat = REQ_CANCEL;
      }
      break;
    case ID_lv_cancel:
      lv_stat = REQ_CANCEL;
      break;
  }
  set(lv_win,MUIA_Window_Open,FALSE);
  DoMethod(gl_app,OM_REMMEMBER,lv_win);
  MUI_DisposeObject(lv_win);
  Signal(gl_process,gl_wakesig);
}

/* start async listview requester, mainidcmp will call gl_req after this */
BOOL Ask_list(void (*handle)(void), strptr title, strptr *list, ulong *old)
{
  if (gl_req) return(FALSE);

  lv_win = WindowObject,
  MUIA_Window_ID         ,MAKE_ID('R','L','I','S'),
  MUIA_Window_Title      ,title,
  MUIA_Window_Width      ,MUIV_Window_Width_Visible(50),
  MUIA_Window_CloseGadget,FALSE,
    WindowContents, VGroup,
      Child, lv_lv = ListviewObject,
        MUIA_Listview_List, ListObject,
          InputListFrame,
          MUIA_List_ConstructHook, MUIV_List_ConstructHook_String,
          MUIA_List_DestructHook , MUIV_List_DestructHook_String,
        End,
      End,
      Child, HGroup,
        Child, lv_bt_ok     = MakeButton(LOC(MSG_key_ok)),
        Child, lv_bt_cancel = MakeButton(LOC(MSG_key_cancel)),
      End,
    End,
  End;
  if (lv_win) {
    DoMethod(lv_lv,MUIM_List_Insert,list,-1,MUIV_List_Insert_Bottom);
    DoMethod(gl_app,OM_ADDMEMBER,lv_win);
    DoMethod(lv_win,MUIM_Window_SetCycleChain,lv_lv,lv_bt_ok,lv_bt_cancel, NULL);
    DoMethod(lv_lv ,MUIM_Notify,MUIA_Listview_DoubleClick,TRUE ,gl_app,2,MUIM_Application_ReturnID,ID_lv_lv);
    MultiIDNotify(MUIA_Pressed,FALSE,
                  lv_bt_ok      ,ID_lv_ok,
                  lv_bt_cancel  ,ID_lv_cancel, TAG_END);
    set(lv_win,MUIA_Window_ActiveObject,lv_lv);
    set(lv_win,MUIA_Window_Screen,gl_conscreen);
    set(lv_win,MUIA_Window_Open,TRUE);
    set(lv_win,MUIA_Window_Activate,TRUE);
    lv_stat   = REQ_RUN;
    lv_old    = old;
    gl_mui_id = &Ask_list_id;
    gl_mui_oc = &Ask_list_oc;
    gl_req    = handle;
    Update_windows();
    return(TRUE);
  }
  return(FALSE);
}

/* Check if listview requester is done, you should call this from handle routine
   which was passed to Ask_list
   REQ_RUN    = requester still running
   REQ_CANCEL = requester cancelled
   REQ_OK     = requester ok
*/
long Ask_list_done(void)
{
  if ( lv_stat != REQ_RUN ) {
    gl_mui_id = 0;
    gl_mui_oc = 0;
    gl_req    = 0;
    Update_windows();
  }
  return(lv_stat);
}

/***----------------------------------------------------------------------***/

enum ID	{ ID_str = TE_First, ID_ok, ID_cancel };

static APTR   str_win,st_str,bt_ok,bt_cancel;
static LONG   str_stat;
static STRPTR str_old;

/* Mainidcmp calls when requester should close its window temporaly (toggle) */
void Ask_string_oc(void)
{
static BOOL ocall=FALSE;

	if (ocall) {
		set(str_win,MUIA_Window_Screen,gl_conscreen);
		set(str_win,MUIA_Window_Open,TRUE);
		set(str_win,MUIA_Window_Activate,TRUE);
		ocall = FALSE;
	} else {
		set(str_win,MUIA_Window_Open,FALSE);
		set(str_win,MUIA_Window_Screen,0);
		ocall = TRUE;
	}
}

/* Mainidcmp calls this using gl_muireq pointer when id arrives */
void Ask_string_id(ULONG id)
{
STRPTR	data;

	switch (id)
	{
		case ID_str:
		case ID_ok:
			get(st_str,MUIA_String_Contents,&data);
			strcpy(str_old,data);
			str_stat = REQ_OK;
			break;
		case ID_cancel:
			str_stat = REQ_CANCEL;
			break;
	}
	set(str_win,MUIA_Window_Open,FALSE);
	DoMethod(gl_app,OM_REMMEMBER,str_win);
	MUI_DisposeObject(str_win);
	Signal(gl_process,gl_wakesig);
}

/* Start async string requester, mainidcmp will call gl_req after this */
BOOL Ask_string(void (*handle)(void), STRPTR title, STRPTR old)
{
	if (gl_req) return(FALSE);

	str_win = WindowObject,
	MUIA_Window_ID					,MAKE_ID('R','S','T','R'),
	MUIA_Window_CloseGadget	,FALSE,
	MUIA_Window_Title				,title,
	MUIA_Window_Width				,MUIV_Window_Width_Visible(50),
		WindowContents, VGroup,
			Child, st_str = String(STRSIZE),
			Child, HGroup,
				Child, bt_ok     = MakeButton(LOC(MSG_key_ok)),
				Child, bt_cancel = MakeButton(LOC(MSG_key_cancel)),
			End,
		End,
	End;
	if (str_win) {
		DoMethod(gl_app,OM_ADDMEMBER,str_win);
		set(st_str	,MUIA_String_Contents,old);
		set(str_win	,MUIA_Window_ActiveObject,st_str);
		DoMethod(str_win,MUIM_Window_SetCycleChain,st_str,bt_ok,bt_cancel, NULL);
		DoMethod(st_str		,MUIM_Notify,MUIA_String_Acknowledge,MUIV_EveryTime, gl_app,2,MUIM_Application_ReturnID,ID_str);
		MultiIDNotify(MUIA_Pressed,FALSE,
									bt_ok			,ID_ok,
									bt_cancel	,ID_cancel, TAG_END);
		set(str_win,MUIA_Window_Screen,gl_conscreen);
		set(str_win,MUIA_Window_Open,TRUE);
		set(str_win,MUIA_Window_Activate,TRUE);
		str_stat	= REQ_RUN;
		str_old		= old;
		gl_mui_id = &Ask_string_id;
		gl_mui_oc	= &Ask_string_oc;
		gl_req		= handle;
		Update_windows();
		return(TRUE);
	}
	return(FALSE);
}

BOOL Ask_string_sync(STRPTR title, STRPTR old)
{
BOOL	rungo=TRUE;
BOOL	ret;
ULONG	sigs,id;

	if (gl_req) return(FALSE);

	if ( Ask_string(&Ask_string_sync,title,old) ) {
/*
		set(gl_app,MUIA_Application_Sleep,TRUE);
*/
		while (rungo)
		{
			id = DoMethod(gl_app,MUIM_Application_Input,&sigs);
			if (id >= TE_First AND id <= TE_Last)	if (gl_mui_id) gl_mui_id(id);
			if (sigs) Wait(sigs);
			switch ( Ask_string_done() )
			{
				case REQ_OK:
					ret   = TRUE;
				case REQ_CANCEL:
					ret   = FALSE;
					rungo = FALSE;
			}
		}
/*
		set(gl_app,MUIA_Application_Sleep,FALSE);
*/
		return(ret);
	}
	return(FALSE);
}

/* Check if string requester is done, you should call this from handle routine
   which was passed to Ask_string
	 REQ_RUN    = requester still running
   REQ_CANCEL = requester cancelled
   REQ_OK     = requester ok
*/
LONG Ask_string_done(void)
{
	if ( str_stat != REQ_RUN ) {
		gl_mui_id	= 0;
		gl_mui_oc	= 0;
		gl_req		= 0;
		Update_windows();
	}
	return(str_stat);
}

/***----------------------------------------------------------------------***/

enum sl_ID { ID_sl_ok = TE_First, ID_sl_cancel };

static APTR	sl_win,sl_val,sl_bt_ok,sl_bt_cancel;
static LONG	sl_stat;
static LONG	*sl_old;

/* Mainidcmp calls when requester should close its window temporaly (toggle) */
void Ask_slider_oc(void)
{
static BOOL ocall=FALSE;

	if (ocall) {
		set(sl_win,MUIA_Window_Screen,gl_conscreen);
		set(sl_win,MUIA_Window_Open,TRUE);
		set(sl_win,MUIA_Window_Activate,TRUE);
		ocall = FALSE;
	} else {
		set(sl_win,MUIA_Window_Open,FALSE);
		set(sl_win,MUIA_Window_Screen,0);
		ocall = TRUE;
	}
}

/* Mainidcmp calls this using gl_muireq pointer when id arrives */
void Ask_slider_id(ULONG id)
{
	switch (id)
	{
		case ID_sl_ok:
			get(sl_val,MUIA_Slider_Level,sl_old);
			sl_stat = REQ_OK;
			break;
		case ID_sl_cancel:
			sl_stat = REQ_CANCEL;
			break;
	}
	set(sl_win,MUIA_Window_Open,FALSE);
	DoMethod(gl_app,OM_REMMEMBER,sl_win);
	MUI_DisposeObject(sl_win);
	Signal(gl_process,gl_wakesig);
}

/* Start async slider requester, mainidcmp will call gl_req after this */
BOOL Ask_slider(void (*handle)(void), STRPTR title, LONG min, LONG max, LONG *old)
{
	if (gl_req) return(FALSE);

	sl_win = WindowObject,
	MUIA_Window_ID					,MAKE_ID('R','S','L','I'),
	MUIA_Window_CloseGadget	,FALSE,
	MUIA_Window_Title				,title,
	MUIA_Window_Width				,MUIV_Window_Width_Visible(50),
		WindowContents, VGroup,
			Child, sl_val = Slider(min,max),
			Child, HGroup,
				Child, sl_bt_ok     = MakeButton(LOC(MSG_key_ok)),
				Child, sl_bt_cancel = MakeButton(LOC(MSG_key_cancel)),
			End,
		End,
	End;
	if (sl_win) {
		DoMethod(gl_app,OM_ADDMEMBER,sl_win);
		set(sl_val	,MUIA_Slider_Level,*old);
		set(sl_win	,MUIA_Window_ActiveObject,sl_val);
		DoMethod(sl_win,MUIM_Window_SetCycleChain,sl_val,sl_bt_ok,sl_bt_cancel, NULL);
		MultiIDNotify(MUIA_Pressed,FALSE,
									sl_bt_ok			,ID_sl_ok,
									sl_bt_cancel	,ID_sl_cancel, TAG_END);
		set(sl_win,MUIA_Window_Screen,gl_conscreen);
		set(sl_win,MUIA_Window_Open,TRUE);
		set(sl_win,MUIA_Window_Activate,TRUE);
		sl_stat		= REQ_RUN;
		sl_old		= old;
		gl_mui_id = &Ask_slider_id;
		gl_mui_oc	= &Ask_slider_oc;
		gl_req		= handle;
		Update_windows();
		return(TRUE);
	}
	return(FALSE);
}

/* Check if slider requester is done, you should call this from handle routine
   which was passed to Ask_slider
	 REQ_RUN    = requester still running
   REQ_CANCEL = requester cancelled
   REQ_OK     = requester ok
*/
LONG Ask_slider_done(void)
{
	if ( sl_stat != REQ_RUN ) {
		gl_mui_id	= 0;
		gl_mui_oc	= 0;
		gl_req		= 0;
		Update_windows();
	}
	return(sl_stat);
}

/***----------------------------------------------------------------------***/

static LONG		file_stat;
static STRPTR	file_old;
static char		file_store[STRSIZE];
static STRPTR	*multi;
static ULONG	file_tags[]=
{
	ASLFR_Window				,0,			/*  1 */
	ASLFR_TitleText			,0,			/*  3 */
	ASLFR_InitialFile		,0,			/*  5 */
	ASLFR_InitialDrawer	,0,			/*  7 */
	ASLFR_InitialPattern,0,			/*  9 */
	ASLFR_DoMultiSelect	,TRUE,	/* 10 */
	ASLFR_DrawersOnly		,TRUE,	/* 12 */
	ASLFR_PrivateIDCMP	,TRUE,
	ASLFR_DoPatterns		,TRUE,
	TAG_END
};

/* sub process for askfile(s) */
static __saveds __asm void Ask_file_proc(void)
{
LONG		ret=REQ_CANCEL,i,size=0;
STRPTR	p;
struct FileRequester	*req;

	if ( (req = AllocAslRequest(ASL_FileRequest,0)) ) {
		if ( AslRequest(req,&file_tags) ) {
			if (file_tags[10] == ASLFR_DoMultiSelect) {
				strncpy(file_old,req->fr_Drawer,STRSIZE-1);
				if (req->fr_NumArgs) {
					for (i = 0; i < req->fr_NumArgs; i++)
					{
						size += ( strlen(req->fr_ArgList[i].wa_Name) +1 );
					}
					if ( (p = *multi = AllocVec(size+1,MEMF_PUBLIC)) ) {
						for (i = 0; i < req->fr_NumArgs; i++)
						{
							strcpy(p,req->fr_ArgList[i].wa_Name);
							p += (strlen(p) + 1);
						}
						*p = 0x00; /* Empty string = END */
					}
				} else {
					*multi = 0;
				}
			} else {
				strncpy(file_old,req->fr_Drawer,STRSIZE-1);
				AddPart(file_old,req->fr_File,STRSIZE-1);
			}
			ret=REQ_OK;
		}
		FreeAslRequest(req);
	}
	Signal(gl_process,gl_wakesig);
	file_stat = ret;
}

/* Start async filerequester, mainidcmp will call gl_req after this */
BOOL Ask_file(struct Window *win, void (*handle)(void), STRPTR title, STRPTR pat, STRPTR old)
{
	if (gl_req) return(FALSE);

	file_stat = REQ_RUN;
	file_old	= old;
	strcpy(file_store,old);
	file_tags[ 1] = win;
	file_tags[ 3] = title;
	file_tags[ 5] = FilePart(file_store);
	file_tags[ 7] = file_store;
	file_tags[ 9] = pat;
	file_tags[10] = TAG_IGNORE;
	file_tags[12] = TAG_IGNORE;
	*(char *)PathPart(file_store) = 0x00;
	if ( CreateNewProcTags(NP_Entry	,&Ask_file_proc,
												 NP_Name	,"DTerm.asl",
												 TAG_END) ) {
		gl_req = handle;
		Update_windows();
		return(TRUE);
	}
	return(FALSE);
}

/* Start async filerequester, mainidcmp will call gl_req after this (multiselect) 
 *
 * new is ptr to pointer, after request it will point to string area where
 * selected files are, you MUST use FreeVec to deallocate area after use.
 * (Note that new is 0 if no files were selected!)
*/
BOOL Ask_files(struct Window *win, void (*handle)(void), STRPTR title, STRPTR pat, STRPTR old, STRPTR *new)
{
	if (gl_req) return(FALSE);

	file_stat = REQ_RUN;
	file_old	= old;
	multi		= new;
	strcpy(file_store,old);
	file_tags[ 1] = win;
	file_tags[ 3] = title;
	file_tags[ 5] = "";
	file_tags[ 7] = file_store;
	file_tags[ 9] = pat;
	file_tags[10] = ASLFR_DoMultiSelect;
	file_tags[12] = TAG_IGNORE;
	if ( CreateNewProcTags(NP_Entry	,&Ask_file_proc,
												 NP_Name	,"DTerm.asl",
												 TAG_END) ) {
		gl_req = handle;
		Update_windows();
		return(TRUE);
	}
	return(FALSE);
}

/* Start async dirrequester, mainidcmp will call gl_req after this */
BOOL Ask_dir(struct Window *win, void (*handle)(void), STRPTR title, STRPTR old)
{
	if (gl_req) return(FALSE);

	file_stat = REQ_RUN;
	file_old	= old;
	strcpy(file_store,old);
	file_tags[ 1] = win;
	file_tags[ 3] = title;
	file_tags[ 5] = "";
	file_tags[ 7] = file_store;
	file_tags[ 9] = "#?";
	file_tags[10] = TAG_IGNORE;
	file_tags[12] = ASLFR_DrawersOnly;
	if ( CreateNewProcTags(NP_Entry	,&Ask_file_proc,
												 NP_Name	,"DTerm.asl",
												 TAG_END) ) {
		gl_req = handle;
		Update_windows();
		return(TRUE);
	}
	return(FALSE);
}

/* Check if filerequester is done, you should call this from handle routine
   which was passed to AskFile
	 REQ_RUN    = requester still running
   REQ_CANCEL = requester cancelled
   REQ_OK     = requester ok
*/
LONG Ask_file_done(void)
{
	if ( file_stat != REQ_RUN ) {
		gl_req = 0;
		Update_windows();
	}
	return(file_stat);
}

LONG Ask_files_done(void)
{
	return(Ask_file_done());
}

LONG Ask_dir_done(void)
{
	return(Ask_file_done());
}

/***----------------------------------------------------------------------***/

static LONG		font_stat;
static STRPTR	font_old;
static char		font_store[STRSIZE];
static ULONG	font_tags[]=
{
	ASLFO_Window					,0,			/*  1 */
	ASLFO_TitleText				,0,			/*  3 */
	ASLFO_InitialName			,0,			/*	5 */
	ASLFO_InitialSize			,0,			/*  7 */
	ASLFO_InitialFrontPen	,0,			/*	9 */
	ASLFO_InitialBackPen	,0,			/* 11 */
	ASLFO_PrivateIDCMP		,TRUE,
	ASLFO_FixedWidthOnly	,TRUE,
	ASLFO_MinHeight				,1,
	ASLFO_MaxHeight				,0xffff,
	TAG_END
};

/* sub process for askfont */
static __saveds __asm void Ask_font_proc(void)
{
LONG		ret=REQ_CANCEL;
struct FontRequester	*req;

	if ( req = AllocAslRequest(ASL_FontRequest,0) ) {
		if ( AslRequest(req,&font_tags) ) {
			sprintf(font_old,"%s/%ld",req->fo_Attr.ta_Name,(LONG)req->fo_Attr.ta_YSize);
			ret=REQ_OK;
		}
		FreeAslRequest(req);
	}
	Signal(gl_process,gl_wakesig);
	font_stat = ret;
}

/* Start async fontrequester, mainidcmp will call gl_req after this 
 * Font name is in format name/size ( topaz.font/8 )
 */
BOOL Ask_font(struct Window *win, void (*handle)(void), STRPTR title, STRPTR old)
{
struct DrawInfo	*di;

	if (gl_req) return(FALSE);

	font_stat = REQ_RUN;
	font_old	= old;
	strcpy(font_store,old);
	font_tags[1] = win;
	font_tags[3] = title;
	font_tags[5] = font_store;
	font_tags[7] = atol(FilePart(font_store));
	*(char *)PathPart(font_store) = 0x00;
	if (di = GetScreenDrawInfo(gl_conscreen)) {
		font_tags[ 9] = di->dri_Pens[TEXTPEN];
		font_tags[11] = di->dri_Pens[BACKGROUNDPEN];
		FreeScreenDrawInfo(gl_conscreen,di);
	}
	if ( CreateNewProcTags(NP_Entry	,&Ask_font_proc,
												 NP_Name	,"DTerm.asl",
												 TAG_END) ) {
		gl_req = handle;
		Update_windows();
		return(TRUE);
	}
	return(FALSE);
}

/* Check if fontrequester is done, you should call this from handle routine
   which was passed to AskFont
	 REQ_RUN    = requester still running
   REQ_CANCEL = requester cancelled
   REQ_OK     = requester ok
*/
LONG Ask_font_done(void)
{
	if ( font_stat != REQ_RUN ) {
		gl_req = 0;
		Update_windows();
	}
	return(font_stat);
}

/***----------------------------------------------------------------------***/

static LONG		mode_stat;
static struct ScreenModeRequester	*mode_old;
static ULONG	mode_tags[]=
{
	ASLSM_Window							,0,			/*  1 */
	ASLSM_InitialDisplayID		,0,			/*  3 */
	ASLSM_InitialDisplayDepth	,0,			/*  5 */
	ASLSM_InitialDisplayWidth	,0,			/*  7 */
	ASLSM_InitialDisplayHeight,0,			/*  9 */
	ASLSM_InitialOverscanType	,0,			/* 11 */
	ASLSM_InitialAutoScroll		,0,			/* 13	*/
	ASLSM_PrivateIDCMP				,TRUE,
	ASLSM_DoDepth							,TRUE,
	ASLSM_DoWidth							,TRUE,
	ASLSM_DoHeight						,TRUE,
	ASLSM_DoOverscanType			,TRUE,
	ASLSM_DoAutoScroll				,TRUE,
	TAG_END
};

/* sub process for askscreenmode */
static __saveds __asm void Ask_screenmode_proc(void)
{
LONG	ret=REQ_CANCEL;
struct ScreenModeRequester	*req;

	if ( req = AllocAslRequest(ASL_ScreenModeRequest,0) ) {
		if ( AslRequest(req,&mode_tags) ) {
			CopyMem(req,mode_old,sizeof(struct ScreenModeRequester));
			ret=REQ_OK;
		}
		FreeAslRequest(req);
	}
	Signal(gl_process,gl_wakesig);
	mode_stat = ret;
}

/* Start async moderequester, mainidcmp will call gl_req after this 
 * Mode id/depth must be in old
 */
BOOL Ask_screenmode(struct Window *win, void (*handle)(void), struct ScreenModeRequester *old)
{
	if (gl_req) return(FALSE);

	mode_stat = REQ_RUN;
	mode_old	= old;
	mode_tags[ 1] = win;
	mode_tags[ 3] = old->sm_DisplayID;
	mode_tags[ 5] = old->sm_DisplayDepth;
	mode_tags[ 7] = old->sm_DisplayWidth;
	mode_tags[ 9] = old->sm_DisplayHeight;
	mode_tags[11] = old->sm_OverscanType;
	mode_tags[13] = old->sm_AutoScroll;
	if ( CreateNewProcTags(NP_Entry	,&Ask_screenmode_proc,
												 NP_Name	,"DTerm.asl",
												 TAG_END) ) {
		gl_req = handle;
		Update_windows();
		return(TRUE);
	}
	return(FALSE);
}

/* Check if moderequester is done, you should call this from handle routine
   which was passed to AskFont
	 REQ_RUN    = requester still running
   REQ_CANCEL = requester cancelled
   REQ_OK     = requester ok
*/
LONG Ask_screenmode_done(void)
{
	if ( mode_stat != REQ_RUN ) {
		gl_req = 0;
		Update_windows();
	}
	return(mode_stat);
}

/***----------------------------------------------------------------------***/
/*** Some MUI support routines                                            ***/
/***----------------------------------------------------------------------***/

ulong xget(Object *obj, ulong attribute)
{
ulong x;

  get(obj,attribute,&x);
  return(x);
}

ulong __stdargs DoSuperNew(struct IClass *cl,Object *obj,ULONG tag1,...)
{
  return(DoSuperMethod(cl,obj,OM_NEW,&tag1,NULL));
}

LONG MUI_nodenum(APTR obj, struct Node *nd)
{
struct Node	*node;
LONG				i;

	for (i=0;;i++)
	{
		DoMethod(obj,MUIM_List_GetEntry,i,&node);
		if (node == nd) return( i);
		if (node == 0 ) return(-1);
	}
}

/* Selectoi entry jos sama node */
BOOL MUI_selnode(APTR obj, struct Node *nd)
{
struct Node	*node;
LONG				i;

	for (i=0;;i++)
	{
		DoMethod(obj,MUIM_List_GetEntry,i,&node);
		if (node == nd) { set(obj,MUIA_List_Active,i); return(TRUE); }
		if (node == 0) return(FALSE);
	}
}

/* Selectoi entry jos sama node */
BOOL MUI_nnselnode(APTR obj, struct Node *nd)
{
struct Node	*node;
LONG				i;

	for (i=0;;i++)
	{
		DoMethod(obj,MUIM_List_GetEntry,i,&node);
		if (node == nd) { nnset(obj,MUIA_List_Active,i); return(TRUE); }
		if (node == 0) return(FALSE);
	}
}

/* Selectoi entry jos sama nimi (Käytetään noden nimeä) */
BOOL MUI_selname(APTR obj, char *name)
{
struct Node	*node;
LONG				i;

	for (i=0;;i++)
	{
		DoMethod(obj,MUIM_List_GetEntry,i,&node);
		if (strcmp(node->ln_Name,name) == 0) { set(obj,MUIA_List_Active,i); return(TRUE); }
		if (node == 0) return(FALSE);
	}
}

/* Selectoi entry jos sama nimi (Käytetään noden nimeä) */
BOOL MUI_nnselname(APTR obj, char *name)
{
long         i;
struct Node *node;

  for (i=0;;i++)
  {
    DoMethod(obj,MUIM_List_GetEntry,i,&node);
    if (strcmp(node->ln_Name,name) == 0) { nnset(obj,MUIA_List_Active,i); return(TRUE); }
    if (node == 0) return(FALSE);
  }
}

/* Infolist mui listviewiin (no add if no object) */
BOOL InfoListToMui(struct List *list, APTR obj, ULONG met)
{
struct Info *info = GetHead(list);

  if (obj) {
    nnset(obj,MUIA_List_Quiet,TRUE);
    DoMethod(obj,MUIM_List_Clear);
    while (info)
    {
      DoMethod(obj,MUIM_List_Insert,&info,1,met);
      info = GetSucc(info);
    }
    nnset(obj,MUIA_List_Quiet,FALSE);
  }
}

/* normal node name to MUI list name (hooks) */

static __saveds __asm long func4(void)
{
  return(0);
}

static __saveds __asm long func3(REG __a1 struct Node *node)
{
  return(node);
}

static __saveds __asm long func2(REG __a2 struct Node *node1,
                                 REG __a1 struct Node *node2)
{
  return(stricmp(node2->ln_Name,node1->ln_Name));
}

static __saveds __asm long func1(REG __a2 char        **array,
                                 REG __a1 struct Node *node)
{
  *array = node->ln_Name;
  return(0);
}

const struct Hook NodeDisplay   = { {0,0}, (void *)func1, 0,0 };
const struct Hook NodeCompare   = { {0,0}, (void *)func2, 0,0 };
const struct Hook NodeConstruct = { {0,0}, (void *)func3, 0,0 };
const struct Hook NodeDestruct  = { {0,0}, (void *)func4, 0,0 };

/***----------------------------------------------------------------------***/
/*** MUI_MakeObject calls as "synonyme" routines                          ***/
/***----------------------------------------------------------------------***/

aptr MakeLabel        (strptr label, ulong flags)         { return(MUI_MakeObject(MUIO_Label,label,flags)); }
aptr MakeButton       (strptr label)                      { return(MUI_MakeObject(MUIO_Button,label)); }
aptr MakeCheckmark    (strptr label)                      { return(MUI_MakeObject(MUIO_Checkmark,label)); }
aptr MakeCycle        (strptr label, strptr *entries)     { return(MUI_MakeObject(MUIO_Cycle,label,entries)); }
aptr MakeRadio        (strptr label, strptr *entries)     { return(MUI_MakeObject(MUIO_Radio,label,entries)); }
aptr MakeSlider       (strptr label, long min , long max) { return(MUI_MakeObject(MUIO_Slider,label,min,max)); }
aptr MakeString       (strptr label, long max)            { return(MUI_MakeObject(MUIO_String,label,max)); }
aptr MakePopButton    (strptr label)                      { return(MUI_MakeObject(MUIO_PopButton,label)); }
aptr MakeHSpace       (long space)                        { return(MUI_MakeObject(MUIO_HSpace,space)); }
aptr MakeVSpace       (long space)                        { return(MUI_MakeObject(MUIO_VSpace,space)); }
aptr MakeHBar         (long space)                        { return(MUI_MakeObject(MUIO_HBar,space)); }
aptr MakeVBar         (long space)                        { return(MUI_MakeObject(MUIO_VBar,space)); }
aptr MakeMenustripNM  (struct NewMenu *nm, ulong flags)   { return(MUI_MakeObject(MUIO_MenustripNM,nm,flags)); }
aptr MakeMenuitem     (strptr label, strptr shortcut, ulong flags, ulong data) { return(MUI_MakeObject(MUIO_Menuitem,label,shortcut,flags,data)); }
aptr MakeBarTitle     (strptr label)                      { return(MUI_MakeObject(MUIO_BarTitle,label)); }
aptr MakeNumericButton(strptr label, long min, long max, strptr format) { return(MUI_MakeObject(MUIO_NumericButton,label,min,max,format)); }

APTR PopFile(void)
{
	return(PopButton(MUII_PopFile));
}

APTR SimpleToggle(char *name)
{
	return(TextObject,
		ButtonFrame,
		MUIA_Text_Contents,name,
		MUIA_Text_PreParse,"\33c",
		MUIA_InputMode    ,MUIV_InputMode_Toggle,
		MUIA_Background   ,MUII_ButtonBack,
		End);
}

APTR LabelL(char *label)
{
	return(LLabel(label));
}

APTR Label1L(char *label)
{
	return(LLabel1(label));
}

APTR Label2L(char *label)
{
	return(LLabel2(label));
}

APTR Check(void)
{
	return(ImageObject,
		ImageButtonFrame,
		MUIA_InputMode						,MUIV_InputMode_Toggle,
		MUIA_Image_Spec						,MUII_CheckMark,
		MUIA_Background						,MUII_ButtonBack,
		MUIA_Image_FontMatchHeight,TRUE,
		MUIA_ShowSelState					,FALSE,
	End);
}

APTR String(ULONG size)
{
	return(StringObject,
		StringFrame,
		MUIA_String_MaxLen	,size,
		MUIA_String_Contents,"",
	End);
}

APTR Int(void)
{
	return(StringObject,
		StringFrame,
		MUIA_String_MaxLen	, 11,
		MUIA_String_Accept	,	"0123456879",
		MUIA_String_Integer	,	0,
	End);
}

APTR Cycle(char *entries[])
{
	return(CycleObject,
		MUIA_Cycle_Entries, entries,
	End);
}

APTR Slider(ULONG min, ULONG max)
{
	return(SliderObject,
		MUIA_Slider_Min		,min,
		MUIA_Slider_Max		,max,
		MUIA_Slider_Level	,min,
	End);
}

APTR KeyButton(char *name, char key)
{
  return(TextObject,
    ButtonFrame,
		MUIA_Text_Contents,name,
		MUIA_Text_PreParse,"\33c",
		MUIA_Text_HiChar	,key,
		MUIA_ControlChar	,key,
		MUIA_InputMode		,MUIV_InputMode_RelVerify,
		MUIA_Background		,MUII_ButtonBack,
	End);
}

APTR SimpleButton(char *name)
{
	return(MUI_MakeObject(MUIO_Button,name));
}

APTR MyText(void)
{
	return(TextObject, 
		MUIA_Frame, MUIV_Frame_None,
	End);
}

APTR ListView(void)
{
	return(ListviewObject,
		MUIA_Listview_List, ListObject,
			InputListFrame,
			MUIA_List_ConstructHook	,&NodeConstruct,
			MUIA_List_CompareHook		,&NodeCompare,
			MUIA_List_DisplayHook		,&NodeDisplay,
		End,
	End);
}

aptr Rectangle(void)
{
  return(RectangleObject,/*MUIA_Weight,1,*/End);
}

ulong set(APTR obj,ULONG attr, ULONG value)
{
  return(SetAttrs(obj,attr,value,TAG_DONE));
}

ulong nnset(APTR obj,ULONG attr, ULONG value)
{
  return(SetAttrs(obj,MUIA_NoNotify,TRUE,attr,value,TAG_DONE));
}

void multiset(ULONG attr, ...)
{
ULONG		obj;
va_list ap;

	va_start(ap, attr);
  while( (obj = va_arg(ap,ULONG)) != TAG_END )
  {
		set(obj,attr,va_arg(ap,ULONG));
  }
	va_end(ap);
}

void nnmultiset(ULONG attr, ...)
{
ULONG		obj;
va_list ap;

	va_start(ap, attr);
  while( (obj = va_arg(ap,ULONG)) != TAG_END )
  {
		nnset(obj,attr,va_arg(ap,ULONG));
  }
	va_end(ap);
}

void MultiIDNotify(ULONG attr, ULONG qual, ...)
{
ULONG		obj;
va_list ap;

	va_start(ap, qual);
  while( (obj = va_arg(ap,ULONG)) != TAG_END )
  {
		DoMethod(obj,MUIM_Notify,attr,qual, gl_app,2,MUIM_Application_ReturnID,va_arg(ap,ULONG));
  }
	va_end(ap);
}
