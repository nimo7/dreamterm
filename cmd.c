/*
 * Most actions are gathered here, some are interactive. All interactive tag
 * changes should be called from here. Well, this needs more thinking before
 * whole concept is useful...
 *
 */

/*
char ver_cmd[]="1.01 (Mar 15 1996)";
*/

#include "global.h"
#include "loc_strings.h"

/*
extern char	ver_dterm[]     ,ver_droplist[]   ,ver_crypt[]    ,ver_xem[]	    ,
						ver_phonebook[]	,ver_phoneentry[]	,ver_protocol[]	,ver_charset[]  ,
						ver_routines[]	,softscrver[]			,hardscrver[]		,softserver[]	  ,
						hardserver[]		,ansiver[]				,ver_hex[]      ,ver_xprdriver[],
						ver_xemdriver[]	,ver_uplist[]     ,capturever[]		,ver_review[]	  ,
						ver_class_cps[]  ,ver_class_revarea[]		,ver_dialer[]		,ver_toolbar[]  ,
            ver_showlist[]  ,ver_menu[];
*/

static char namesc[STRSIZE];

static ulong do_none(void)
{
}

/*
 *  Open current settings window
 *
 *  Parameters
 *    none
 *
 *  Return value
 *    none
 */
static ulong do_open_current(void)
{
  Entry(PE_CURRENT,0);
}

/*
 *  Open profile settings window
 *
 *  Parameters
 *    none
 *
 *  Return value
 *    none
 */
static ulong do_open_profile(void)
{
  Entry(PE_PICK,0);
}

/*
 *  Open protocol settings window
 *
 *  Parameters
 *    none
 *
 *  Return value
 *    none
 */
static ulong do_open_xp(void)
{
  set(gl_protocol,MUIA_Window_Screen  ,gl_conscreen);
  set(gl_protocol,MUIA_Window_Open    ,TRUE);
  set(gl_protocol,MUIA_Window_Activate,TRUE);
}

/*
 *  Open charset settings window
 *
 *  Parameters
 *    none
 *
 *  Return value
 *    none
 */
static ulong do_open_cs(void)
{
  Charset(DT_OPEN,0);
}

/*
 *  Open emulation settings window
 *
 *  Parameters
 *    none
 *
 *  Return value
 *    none
 */
static ulong do_open_xm(void)
{
  Xem(DT_OPEN,0);
}

/*
 *  Open toolbar settings window
 *
 *  Parameters
 *    none
 *
 *  Return value
 *    none
 */
static ulong do_open_tb(void)
{
  Toolbar(DT_OPEN,0);
}

/*
 *  Load default settings
 *
 *  Parameters
 *    none
 *
 *  Return value
 *    none
 */
static ulong do_load_default(void)
{
  if ( (!gl_xemsafe)   OR
       (!gl_profsafe)  OR
       (!gl_charsafe)  OR
       (!gl_protosafe) OR
       (!gl_toolbarsafe) ) {
    if ( MUI_RequestA(gl_app,win_fake,0,
                      LOC(MSG_note),
                      LOC(MSG_ok_cancel),
                      LOC(MSG_current_configuration_is_not_saved),
                      0) == 0 ) return;
    com(cmd_load_profile,gl_def_gl);
    Charset (DT_LOAD,gl_def_cs);
    DoMethod(gl_protocol,MUIM_INF_Load,gl_def_xp);
    Toolbar (DT_LOAD,gl_def_tb);
    Xem     (DT_LOAD,gl_def_xm);
  }
}

/*
 *  Save default settings
 *
 *  Parameters
 *    none
 *
 *  Return value
 *    none
 */
static ulong do_save_default(void)
{
  Charset (DT_SAVE,gl_def_cs);
  DoMethod(gl_protocol,MUIM_INF_Save,gl_def_xp);
  Toolbar (DT_SAVE,gl_def_tb);
  Xem     (DT_SAVE,gl_def_xm);
  com(cmd_save_profile,gl_def_gl);
  com(cmd_save_current,gl_def_cu);
}

/*  Toggle titlebar on/off
 *
 *
 *  Parameters
 *    none
 *
 *  Return value
 *    none
 */
static ulong do_titlebar(void)
{
  if ( !CUR(PB_HardScr) AND gl_conscreen ) {
    ShowTitle(gl_conscreen,gl_title = TF(gl_title));
  }
}

/*
 *  Open crypting settings window
 *
 *  Parameters
 *    none
 *
 *  Return value
 *    none
 */
static ulong do_open_crypt(void)
{
  Crypt(DT_OPEN,0,0);
}

/*
 *  Show about requester
 *
 *  Parameters
 *    none
 *
 *  Return value
 *    none
 */
static ulong do_about(void)
{
extern char  ver_dterm[];
extern long  pool_type, /* temporary */
             pool_size,
             pool_tres;

  if ( MUI_RequestA(gl_app,win_fake,0,
                    LOC(MSG_this_will_change_your_life),
                    LOC(MSG_program_modules_show_no_more),
                    LOC(MSG_about1),0) == 1)
  {
    MUI_Request(gl_app,win_fake,0,
                LOC(MSG_this_will_change_your_life),
                LOC(MSG_who_needs_this_information),
                LOC(MSG_about2),
    &ver_dterm[6],
    gl_async_dos  , pool_type,
    gl_cap_bufsize, pool_size,
    gl_txdsize    , pool_tres);

/*
    &ver_dterm[6],
    ver_charset   ,ansiver      ,
    ver_crypt     ,ver_cmd      ,
    ver_dialer    ,ver_hex      ,
    ver_droplist  ,hardscrver   ,
    ver_phonebook ,hardserver   ,
    ver_phoneentry,ver_menu     ,
    ver_protocol  ,ver_class_cps ,
    ver_review    ,ver_class_revarea  ,
    ver_showlist  ,softscrver   ,
    ver_toolbar   ,softserver   ,
    ver_uplist    ,ver_xemdriver,
    ver_xem       ,ver_xprdriver);
*/
  }
}

/*
 *  Quit program
 *
 *  Parameters
 *    none
 *
 *  Return value
 *    none
 */
static ulong do_quit(void)
{
char str[STRSIZE];

  str[0] = 0x00;
  if ( gl_onlinepb   ) strcat(str,LOC(MSG_online));
  if (!gl_phonesafe  ) strcat(str,LOC(MSG_phonebook_directory));
  if (!gl_toolbarsafe) strcat(str,LOC(MSG_toolbar_configuration));
	if (!gl_protosafe  ) strcat(str,LOC(MSG_protocol_configuration));
	if (!gl_xemsafe	   ) strcat(str,LOC(MSG_xemulation_configuration));
	if (!gl_charsafe   ) strcat(str,LOC(MSG_character_set_configuration));

	if (!gl_charsafe OR !gl_protosafe OR !gl_phonesafe OR !gl_toolbarsafe OR 
      !gl_xemsafe  OR  gl_onlinepb) {
		if ( MUI_Request(gl_app,win_fake,0,
                     LOC(MSG_note),
                     LOC(MSG_ok_cancel),
                     LOC(MSG_are_you_sure_to_quit),
										 str) ) {
			gl_exit = TRUE;
      Signal(gl_process,gl_wakesig);
		}
	} else {
		gl_exit = TRUE;
    Signal(gl_process,gl_wakesig);
	}
}

/*
 *  Open phonebook window
 *
 *  Parameters
 *    none
 *
 *  Return value
 *    none
 */
static ulong do_open_phonebook(void)
{
  Phone(DT_OPEN,0);
}

/*
 *  Create new phonebook entry
 *
 *  Parameters
 *    none
 *
 *  Return value
 *    none
 */
static ulong do_new_entry(void)
{
  Entry(PE_NEW,0);
}

/*
 *  Load default phonebook
 *
 *  Parameters
 *    none
 *
 *  Return value
 *    none
 */
static ulong do_load_phonebook(void)
{
  if (!gl_phonesafe) {
    if ( MUI_RequestA(gl_app,win_fake,0,
                      LOC(MSG_note),
                      LOC(MSG_ok_cancel),
                      LOC(MSG_current_phonebook_is_not_saved),0) == 0 ) return;
  }
  Drop (DT_LOAD,gl_def_dl);
  Phone(DT_LOAD,gl_def_pb);
  Show (DT_LOAD,gl_def_sl);
}

/*
 *  Save default phonebook
 *
 *  Parameters
 *    none
 *
 *  Return value
 *    none
 */
static ulong do_save_phonebook(void)
{
  Drop (DT_SAVE,gl_def_dl);
  Phone(DT_SAVE,gl_def_pb);
  Show (DT_SAVE,gl_def_sl);
}

/*
 *  Dial number
 *
 *  Parameters
 *    str: phonenumbers, if 0 then will ask numbers
 *
 *  Return value
 *    none
 */
static ulong do_dial_number(char *sup_str)
{
static char str[STRSIZE];

  if (gl_dial == 0) {
    if (!gl_req) {
      if (sup_str == 0) {
        Ask_string(&do_dial_number,LOC(MSG_enter_numbers_to_dial),str);
      } else {
        Dialer(DI_DIALNUM,sup_str);
      }
    } else {
      if ( Ask_string_done() == REQ_OK ) {
        Dialer(DI_DIALNUM,str);
      }
    }
  }
}

/*
 *  Start to dial selected entries in phonebook list
 *
 *  Parameters
 *    none
 *
 *  Return value
 *    none
 */
static ulong do_dial_next(void)
{
  Dialer(DI_DIAL,0);
}

/*
 *  Send current password
 *
 *  Parameters
 *    none
 *
 *  Return value
 *    none
 */
static ulong do_send_password(void)
{
   Send_string(CUR(PB_PassWord));
}

/*
 *  Start uploading, currently selected protocol is used
 *
 *  Parameters
 *    none
 *
 *  Return value
 *    none
 */
static ulong do_upload(void)
{
  Transfer(FT_UPLOADRQ,gl_curxp);
}

/*
 *  Start uploading from list, currently selected protocol is used
 *
 *  Parameters
 *    none
 *
 *  Return value
 *    none
 */
static ulong do_uploadlist(void)
{
  Transfer(FT_UPLOADID,gl_curxp);
}

/*
 *  Start downloading, currently selected protocol is used
 *
 *  Parameters
 *    none
 *
 *  Return value
 *    none
 */
static ulong do_download(void)
{
  Transfer(FT_DNLOAD,gl_curxp);
}

/*
 *  Send ascii file converting characters (this will ask user the file)
 *
 *  Parameters
 *    none
 *
 *  Return value
 *    none
 */
static ulong do_send_conv(void)
{
  if (gl_asc_fh == 0) {
    if (!gl_req) {
      Ask_file(gl_conwindow,&do_send_conv,LOC(MSG_select_file),"#?",namesc);
    } else {
      if ( Ask_file_done() == REQ_OK ) {
        gl_asc_fh   = dt_open(namesc,MODE_READ,8192);
        gl_asc_mode = 0;
        Menu_fresh();
      }
    }
  }
}

/*
 *  Send ascii file without converting characters (this will ask user the file)
 *
 *  Parameters
 *    none
 *
 *  Return value
 *    none
 */
static ulong do_send_no_conv(void)
{
  if (gl_asc_fh == 0) {
  	if (!gl_req) {
	  	Ask_file(gl_conwindow,&do_send_no_conv,LOC(MSG_select_file_no_conversion),"#?",namesc);
  	} else {
	  	if ( Ask_file_done() == REQ_OK ) {
		  	gl_asc_fh		= dt_open(namesc,MODE_READ,8192);
			  gl_asc_mode	= 1;
  			Menu_fresh();
	  	}
  	}
  }
}

/*
 *  Abort ascii send or upload/download
 *
 *  Parameters
 *    none
 *
 *  Return value
 *    none
 */
static ulong do_abort_send(void)
{
	if (gl_asc_fh) {
		dt_close(gl_asc_fh);
    gl_asc_fh = 0;
    Clear_buffer();
		Menu_fresh();
	}
	Transfer(FT_ABORT,0);
}

/*
 *  Open upload list window
 *
 *  Parameters
 *    none
 *
 *  Return value
 *    none
 */
static ulong do_open_uplist(void)
{
  Fileid(DT_OPEN,0);
}

/*
 *  Open review window
 *
 *  Parameters
 *    none
 *
 *  Return value
 *    none
 */
static ulong do_open_review(void)
{
  set(gl_review,MUIA_Window_Screen  ,gl_conscreen);
  set(gl_review,MUIA_Window_Open    ,TRUE);
  set(gl_review,MUIA_Window_Activate,TRUE);
}

/*
 *  Open capture file (this will ask user the file)
 *
 *  Parameters
 *    none
 *
 *  Return value
 *    none
 */
static ulong do_start_capture(void)
{
BPTR	lock;
ulong ans;
static char	name_ca[STRSIZE];

	if (gl_cap_fh == 0) {
		if (!gl_req) {
			Ask_file(gl_conwindow,&do_start_capture,LOC(MSG_open_capture),"#?",name_ca);
		} else {
			if ( Ask_file_done() == REQ_OK ) {
				if ( (gl_cap_buf = alloc_pool(gl_cap_bufsize)) ) {
					gl_cap_now = gl_cap_buf;
					lock = Lock(name_ca,ACCESS_WRITE); UnLock(lock);
					if (lock) {
						ans = MUI_Request(gl_app,win_fake,0,
                              LOC(MSG_note),
                              LOC(MSG_overwrite_append_cancel),
															LOC(MSG_file_x_already_exist),
                              name_ca);
						if (ans == 0) return;
						if (ans == 2) {
              gl_cap_fh = dt_open(name_ca,MODE_APPEND,8192);
							Menu_fresh();
							return;
						}
					}
					gl_cap_fh = dt_open(name_ca,MODE_WRITE,8192);
				}
				Menu_fresh();
			}
		}
	}
}

/*
 *  Hangup modem
 *
 *  Parameters
 *    none
 *
 *  Return value
 *    none
 */
static ulong do_hangup(void)
{
	if (CUR(PB_DTRHangup)) {
		DropDTR();
	} else {
		Send_string(CUR(PB_Hangup));
	}
}

/*
 *  Release serial device
 *
 *  Parameters
 *    none
 *
 *  Return value
 *    none
 */
static ulong do_release_serial(void)
{
  if (!gl_xferon) { /* don't play this during file transfer! */
  	if (gl_serok) {
      if (gl_serfreeze) {
    		gl_serfreeze = FALSE;
      } else {
        if (!CUR(PB_SerShared) OR CUR(PB_HardSer)) {
      		RemoveSerial();
        }
  	  	gl_serfreeze = TRUE;
      }
  	} else {
	  	InitSerial();
  		SetPort();
		  gl_serfreeze = FALSE;
	  }
  }
  Menu_fresh();
}

/*
 * Load and set current settings
 *
 * str   : file to load
 *
 * return: TRUE/FALSE
 *
 */
static ulong do_load_current(strptr str)
{
struct Info *pb;

  if ( (pb = CreateInfo()) ) {
    if (LoadInfo(pb,str,CUID)) {

      /* Global options (copy to current settings) */
      CopyEntryTags(pb,gl_curbbs, ENV_TAGS, TAG_END);

      /* set entry(changeable) settings */
      SetUp(pb);
      DeleteInfo(pb);
      return(TRUE);
    }
  }
  return(FALSE);
}

/*
 * Save current settings
 *
 * str   : file to save
 *
 * return: -
 *
 */
static ulong do_save_current(strptr str)
{
struct Info *pb;

  if ( (pb = CreateInfo()) == 0 ) {
    Warning(LOC(MSG_cannot_save_current_settings));
    return(FALSE);
  }
  if (CUR(PB_Crypt)) {
    CopyTags(&gl_curbbs->tags,&pb->tags);
    DelInfoTags(pb,
                PB_Name   ,PB_Phone   ,PB_PassWord	,PB_Comment	,
                PB_Func1  ,PB_Func2   ,PB_Func3			,PB_Func4		,
                PB_Func5  ,PB_Func6   ,PB_Func7			,PB_Func8		,
                PB_Func9  ,PB_Func10  ,PB_CryptKey	,	TAG_END);
    if (! SaveInfo(pb,str,CUID)) Warning(LOC(MSG_cannot_save_current_settings));
  } else {
    CopyTags(&gl_curbbs->tags,&pb->tags);
    DelInfoTag(PB_CryptKey,pb);
    if (! SaveInfo(pb,str,CUID)) Warning(LOC(MSG_cannot_save_current_settings));
  }
  DeleteInfo(pb);
}

/*
 * Load profile
 *
 * str   : base string for name
 *
 * return: -
 *
 */
static ulong do_load_profile(strptr str)
{
char str_tmp[STRSIZE];
long ret1,
     ret2;

  strcpy(str_tmp,str); strcat(str_tmp,"1");
	ret1 = LoadInfo(gl_defbbs ,str_tmp,G1ID);
  strcpy(str_tmp,str); strcat(str_tmp,"2");
  ret2 = LoadInfo(gl_fromdef,str_tmp,G2ID);
	if (!ret1 OR !ret2) {
		Warning(LOC(MSG_cannot_read_profile));
	}
	gl_profsafe	= TRUE;
}

/*
 * Save profile
 *
 * str   : base string for name
 *
 * return: -
 *
 */
static ulong do_save_profile(strptr str)
{
char str_tmp[STRSIZE];

  strcpy(str_tmp,str); strcat(str_tmp,"1");
	if (! SaveInfo(gl_defbbs	,str_tmp,G1ID) ) {
    Warning(LOC(MSG_cannot_save_profile));
	}
  strcpy(str_tmp,str); strcat(str_tmp,"2");
	if (! SaveInfo(gl_fromdef	,str_tmp,G2ID) ) {
    Warning(LOC(MSG_cannot_save_default_profile));
	}
	gl_profsafe	= TRUE;
}

/*
 * Start some action (NOTE: action do not necessarily stop after return!)
 *
 * id    : command tag
 * ...   : additional data for command
 *
 * return: depends of command (mostly nothing)
 *
 */
ulong com(ulong id, ...)
{
va_list ap;
ulong   ret;

  va_start(ap, id);

  switch (id)
  {
    /* mostly simple actions */
    case cmd_none          : ret = do_none()                       ; break;
    case cmd_open_current  : ret = do_open_current()               ; break;
    case cmd_open_profile  : ret = do_open_profile()               ; break;
    case cmd_open_xp       : ret = do_open_xp()                    ; break;
    case cmd_open_cs       : ret = do_open_cs()                    ; break;
    case cmd_open_xm       : ret = do_open_xm()                    ; break;
    case cmd_open_tb       : ret = do_open_tb()                    ; break;  
    case cmd_load_default  : ret = do_load_default()               ; break;  
    case cmd_save_default  : ret = do_save_default()               ; break;  
    case cmd_titlebar      : ret = do_titlebar()                   ; break;  
    case cmd_open_crypt    : ret = do_open_crypt()                 ; break;  
    case cmd_about         : ret = do_about()                      ; break;  
    case cmd_quit          : ret = do_quit()                       ; break;  
    case cmd_open_phonebook: ret = do_open_phonebook()             ; break;
    case cmd_new_entry     : ret = do_new_entry()                  ; break;
    case cmd_load_phonebook: ret = do_load_phonebook()             ; break;
    case cmd_save_phonebook: ret = do_save_phonebook()             ; break;
    case cmd_dial_number   : ret = do_dial_number(va_arg(ap,ulong)); break;
    case cmd_dial_next     : ret = do_dial_next()                  ; break;
    case cmd_send_password : ret = do_send_password()              ; break;
    case cmd_upload        : ret = do_upload()                     ; break;
    case cmd_uploadlist    : ret = do_uploadlist()                 ; break;
    case cmd_download      : ret = do_download()                   ; break;
    case cmd_send_conv     : ret = do_send_conv()                  ; break;
    case cmd_send_no_conv  : ret = do_send_no_conv()               ; break;
    case cmd_abort_send    : ret = do_abort_send()                 ; break;
    case cmd_open_uplist   : ret = do_open_uplist()                ; break;
    case cmd_open_review   : ret = do_open_review()                ; break;
    case cmd_start_capture : ret = do_start_capture()              ; break;
    case cmd_hangup        : ret = do_hangup()                     ; break;
    case cmd_release_serial: ret = do_release_serial()             ; break;
    case cmd_load_current  : ret = do_load_current(va_arg(ap,ulong)); break;
    case cmd_save_current  : ret = do_save_current(va_arg(ap,ulong)); break;
    case cmd_load_profile  : ret = do_load_profile(va_arg(ap,ulong)); break;
    case cmd_save_profile  : ret = do_save_profile(va_arg(ap,ulong)); break;

    /* these are for changing tags by requester, routines will pick up
       right requester to deal with */
/*
    case cmd_change_cur
    case cmd_change_def
    case cmd_change_info
*/
  }

  va_end(ap);
  return(ret);
}
