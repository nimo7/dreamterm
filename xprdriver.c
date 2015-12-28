#include "global.h"
#include "loc_strings.h"

/*
char ver_class_xpr[]="1.00 (Apr 16 1996)";
*/

enum ID { ID_skip = FT_First,
          ID_abort  };

static bool win_act = FALSE;
static aptr win,bt_skip,bt_abort,upstat,dnstat,cp_up,cp_dn,
            tx_upname,go_upbytes,tx_upsize ,tx_upelap ,tx_upexp  ,go_upcps   ,
            tx_upcps ,tx_upmsg  ,tx_uperr  ,tx_upbytes,tx_upbsize,tx_upbcheck,
            tx_dnname,go_dnbytes,tx_dnsize ,tx_dnelap ,tx_dnexp  ,go_dncps   ,
            tx_dncps ,tx_dnmsg  ,tx_dnerr  ,tx_dnbytes,tx_dnbsize,tx_dnbcheck;

static char         disp_min[82],
                    up_dir[STRSIZE];
static ulong        disp_quiet;
                    disp_maxcps;
static strptr       fl1;
static struct Info *up_xp;

/***----------------------------------------------------------------------***/
/*** support                                                              ***/
/***----------------------------------------------------------------------***/

/*
 * Send message to child
 *
 * cmd   : command
 * arg   : possible args
 * arg2  : 
 * arg3  : 
 *
 * return: TRUE/FALSE
 *
 */
static ulong send_child(ulong cmd, ulong arg, ulong arg2, ulong arg3)
{
  gl_proc_xp->ch_msg.mn_Length    = sizeof(struct XP_Msg);
  gl_proc_xp->ch_msg.cmd          = cmd;
  gl_proc_xp->ch_msg.arg          = arg;
  gl_proc_xp->ch_msg.arg2         = arg2;
  gl_proc_xp->ch_msg.arg3         = arg3;
  gl_proc_xp->ch_msg.mn_ReplyPort = gl_proc_xp->ch_reply;
  PutMsg  (gl_proc_xp->ch_port,&gl_proc_xp->ch_msg);
  WaitPort(gl_proc_xp->ch_reply);
  GetMsg  (gl_proc_xp->ch_reply);
  return  (gl_proc_xp->ch_msg.result);
}

/*
 * Update window
 *
 * u     : XPR_UPDATE structure
 * which : 1 = upload, 0 = download
 *
 * return: -
 *
 */
static void do_update(struct XPR_UPDATE *u, long which)
{
char  str_tmp[14];
ulong t,
      pros;

  if (u) {
    t = u->xpru_updatemask & 0x3fffffff;
    if (which == 1) {
      if (disp_quiet) {
        if ( (t & XPRU_FILENAME) ) strncpy(&disp_min[4],u->xpru_filename,12);
        if ( (t & XPRU_FILESIZE) ) FastDec8(u->xpru_filesize,&disp_min[17]);
        if ( (t & XPRU_DATARATE) ) FastDec4(u->xpru_datarate,&disp_min[26]);
        if ( (t & XPRU_BYTES) ) {
          pros = 0;
          if (u->xpru_bytes != 0) pros = u->xpru_bytes*100/u->xpru_filesize;
          FastDec3(pros,&disp_min[31]);
          disp_min[80] = 0x00;
        }
      } else if (win_act) {
        if ( (t & XPRU_PROTOCOL)    ) set(win        ,MUIA_Window_Title ,u->xpru_protocol);
        if ( (t & XPRU_FILENAME)    ) set(tx_upname  ,MUIA_Text_Contents,u->xpru_filename);
        if ( (t & XPRU_ELAPSEDTIME) ) set(tx_upelap  ,MUIA_Text_Contents,u->xpru_elapsedtime);
        if ( (t & XPRU_EXPECTTIME)  ) set(tx_upexp   ,MUIA_Text_Contents,u->xpru_expecttime);
        if ( (t & XPRU_MSG)         ) set(tx_upmsg   ,MUIA_Text_Contents,u->xpru_msg);
        if ( (t & XPRU_ERRORMSG)    ) set(tx_uperr   ,MUIA_Text_Contents,u->xpru_errormsg);
        if ( (t & XPRU_BLOCKCHECK)  ) set(tx_upbcheck,MUIA_Text_Contents,u->xpru_blockcheck);
        if ( (t & XPRU_FILESIZE) )  {
          stcl_d(str_tmp,u->xpru_filesize);
          set(tx_upsize ,MUIA_Text_Contents,str_tmp);
          set(go_upbytes,MUIA_Gauge_Max    ,u->xpru_filesize);
        }
        if ( (t & XPRU_DATARATE)  ) {
          if (disp_maxcps >= u->xpru_datarate) {
            stcl_d  (str_tmp ,u->xpru_datarate);
            set     (tx_upcps,MUIA_Text_Contents,str_tmp);
            DoMethod(cp_up   ,MUIM_CPS_Cps      ,u->xpru_datarate);
          } else {
            stcl_d  (str_tmp ,disp_maxcps);
            set     (tx_upcps,MUIA_Text_Contents,str_tmp);
            DoMethod(cp_up   ,MUIM_CPS_Cps      ,disp_maxcps);
          }
        }
        if ( (t & XPRU_BYTES) ) {
          stcl_d(str_tmp,u->xpru_bytes);
          set(tx_upbytes,MUIA_Text_Contents,str_tmp);
          if (u->xpru_filesize >= u->xpru_bytes) set(go_upbytes,MUIA_Gauge_Current,u->xpru_bytes);
        }
        if ( (t & XPRU_BLOCKSIZE) ) {
          stcl_d(str_tmp,u->xpru_blocksize);
          set(tx_upbsize,MUIA_Text_Contents,str_tmp);
        }
      }
    }
  
    if (which == 0) {
      if (disp_quiet) {
        if ( (t & XPRU_FILENAME) ) strncpy(&disp_min[40],u->xpru_filename,12);
        if ( (t & XPRU_FILESIZE) ) FastDec8(u->xpru_filesize,&disp_min[53]);
        if ( (t & XPRU_DATARATE) ) FastDec4(u->xpru_datarate,&disp_min[62]);
        if ( (t & XPRU_BYTES) ) {
          pros = 0;
          if (u->xpru_bytes) pros = u->xpru_bytes*100/u->xpru_filesize;
          FastDec3(pros,&disp_min[67]);
          disp_min[80] = 0x00;
        }
      } else if (win_act) {
        if ( (t & XPRU_PROTOCOL)    ) set(win        ,MUIA_Window_Title ,u->xpru_protocol);
        if ( (t & XPRU_FILENAME)    ) set(tx_dnname  ,MUIA_Text_Contents,u->xpru_filename);
        if ( (t & XPRU_ELAPSEDTIME) ) set(tx_dnelap  ,MUIA_Text_Contents,u->xpru_elapsedtime);
        if ( (t & XPRU_EXPECTTIME)  ) set(tx_dnexp   ,MUIA_Text_Contents,u->xpru_expecttime);
        if ( (t & XPRU_MSG)         ) set(tx_dnmsg   ,MUIA_Text_Contents,u->xpru_msg);
        if ( (t & XPRU_ERRORMSG)    ) set(tx_dnerr   ,MUIA_Text_Contents,u->xpru_errormsg);
        if ( (t & XPRU_BLOCKCHECK)  ) set(tx_dnbcheck,MUIA_Text_Contents,u->xpru_blockcheck);
        if ( (t & XPRU_FILESIZE) ) {
          stcl_d(str_tmp,u->xpru_filesize);
          set(tx_dnsize ,MUIA_Text_Contents,str_tmp);
          set(go_dnbytes,MUIA_Gauge_Max    ,u->xpru_filesize);
        }
        if ( (t & XPRU_DATARATE) ) {
          if (disp_maxcps >= u->xpru_datarate) {
            stcl_d  (str_tmp ,u->xpru_datarate);
            set     (tx_dncps,MUIA_Text_Contents,str_tmp);
            DoMethod(cp_dn   ,MUIM_CPS_Cps      ,u->xpru_datarate);
          } else {
            stcl_d  (str_tmp ,disp_maxcps);
            set     (tx_dncps,MUIA_Text_Contents,str_tmp);
            DoMethod(cp_dn   ,MUIM_CPS_Cps      ,disp_maxcps);
          }
        }
        if ( (t & XPRU_BYTES) ) {
          stcl_d(str_tmp,u->xpru_bytes);
          set(tx_dnbytes,MUIA_Text_Contents,str_tmp);
          if (u->xpru_filesize >= u->xpru_bytes) set(go_dnbytes,MUIA_Gauge_Current,u->xpru_bytes);
        }
        if ( (t & XPRU_BLOCKSIZE) ) {
          stcl_d(str_tmp,u->xpru_blocksize);
          set(tx_dnbsize,MUIA_Text_Contents,str_tmp);
        }
      }
    }
    if (disp_quiet) StatPrint(disp_min);
  }
}

/*  Interface pääohjelmaan
 *
 *  Commands:
 *
 *  FT_ADDFILE  [name ] - Lisää filejä listaan
 *  FT_UPLOADRQ [proto] - Kysy lisää filejä & aloita upload jos ei käynnissä
 *  FT_UPLOADID [proto] - Lisää upload list & aloita upload jos ei käynnissä
 *  FT_UPLOAD   [proto] - Aloita upload
 *  FT_DNLOAD   [proto] - Aloita dnload
 *  FT_SETUP    [proto] - Setup using options (ret: initstring)
 *  FT_ABORT            - Abortoi xfer
 *
 *  UPLOADRQ, FT_DNLOAD palauttavat falsen jos async req käynnistyi...
 */

static void up_req(void)
{
char   s[STRSIZE];
strptr fl2;

  switch ( Ask_files_done() )
  {
    case REQ_OK:
      AddInfoTag(PB_UpPath,up_dir,gl_curbbs);
      if (fl2 = fl1) {
        while (*fl2 != 0)
        {
          strcpy(s,up_dir);
          AddPart(s,fl2,STRSIZE-1);
          Transfer(FT_ADDFILE,s);
          fl2 += (strlen(fl2) + 1);
        }
        FreeVec(fl1);
      }
    case REQ_CANCEL:
      Transfer(FT_UPLOAD,up_xp); /* hm... up_xp should be locked */
  }
}

bool open_window(void)
{
  if (!win_act) {
    win = WindowObject,
    MUIA_Window_ID          ,MAKE_ID('F','T','W','I'),
    MUIA_Window_CloseGadget ,FALSE,
    MUIA_Window_Width       ,MUIV_Window_Width_Visible(100),
      WindowContents, VGroup,
        Child, upstat = VGroup,
          GroupFrameT(LOC(MSG_upload)),
          Child, VGroup,
            Child, HGroup,
              Child, ColGroup(3),
                Child, LabelL(LOC(MSG_xw_name)), Child, LabelL(":"), Child, tx_upname = MyText(),
                Child, LabelL(LOC(MSG_xw_size)), Child, LabelL(":"), Child, tx_upsize = MyText(),
                Child, LabelL(LOC(MSG_xw_time)), Child, LabelL(":"), Child, tx_upelap = MyText(),
                Child, LabelL("")              , Child, LabelL(":"), Child, tx_upexp  = MyText(),
              End,
              Child, ColGroup(3),
                Child, LabelL(LOC(MSG_xw_cps))      , Child, LabelL(":"), Child, tx_upcps   = MyText(),
                Child, LabelL(LOC(MSG_xw_bytes))    , Child, LabelL(":"), Child, tx_upbytes = MyText(),
                Child, LabelL(LOC(MSG_xw_blocksize)), Child, LabelL(":"), Child, tx_upbsize = MyText(),
                Child, LabelL(LOC(MSG_xw_block_crc)), Child, LabelL(":"), Child, tx_upbcheck= MyText(),
              End,

              Child, cp_up = NewObject(gl_class_cps,NULL,
                TextFrame,
                MUIA_Background, MUII_BACKGROUND,
              TAG_DONE),

            End,
            Child, ColGroup(3),
              Child, LabelL(LOC(MSG_xw_msg))  , Child, LabelL(":"), Child, tx_upmsg = MyText(),
              Child, LabelL(LOC(MSG_xw_error)), Child, LabelL(":"), Child, tx_uperr = MyText(),
            End,
          End,
          Child, go_upbytes = GaugeObject,GaugeFrame, MUIA_Gauge_Current,0, MUIA_Gauge_Horiz,TRUE, MUIA_Gauge_InfoText,"%ld", MUIA_VertWeight,8,End,
          Child, ScaleObject, End,
        End,
        Child, dnstat = VGroup,
          GroupFrameT(LOC(MSG_download)),
          Child, VGroup,
            Child, HGroup,
              Child, ColGroup(3),
                Child, LabelL(LOC(MSG_xw_name)), Child, LabelL(":"), Child, tx_dnname = MyText(),
                Child, LabelL(LOC(MSG_xw_size)), Child, LabelL(":"), Child, tx_dnsize = MyText(),
                Child, LabelL(LOC(MSG_xw_time)), Child, LabelL(":"), Child, tx_dnelap = MyText(),
                Child, LabelL("")              , Child, LabelL(":"), Child, tx_dnexp  = MyText(),
              End,
              Child, ColGroup(3),
                Child, LabelL(LOC(MSG_xw_cps))      , Child, LabelL(":"), Child, tx_dncps    = MyText(),
                Child, LabelL(LOC(MSG_xw_bytes))    , Child, LabelL(":"), Child, tx_dnbytes  = MyText(),
                Child, LabelL(LOC(MSG_xw_blocksize)), Child, LabelL(":"), Child, tx_dnbsize  = MyText(),
                Child, LabelL(LOC(MSG_xw_block_crc)), Child, LabelL(":"), Child, tx_dnbcheck = MyText(),
              End,

              Child, cp_dn = NewObject(gl_class_cps,NULL,
                TextFrame,
                MUIA_Background, MUII_BACKGROUND,
              TAG_DONE),

            End,
            Child, ColGroup(3),
              Child, LabelL(LOC(MSG_xw_msg))  , Child, LabelL(":"), Child, tx_dnmsg = MyText(),
              Child, LabelL(LOC(MSG_xw_error)), Child, LabelL(":"), Child, tx_dnerr = MyText(),
            End,
          End,
          Child, go_dnbytes = GaugeObject,GaugeFrame, MUIA_Gauge_Current,0, MUIA_Gauge_Horiz,TRUE, MUIA_Gauge_InfoText,"%ld", MUIA_VertWeight,8,End,
          Child, ScaleObject, End,
        End,
        Child, HGroup,
          Child, bt_skip  = MakeButton(LOC(MSG_xw_skip)),
          Child, bt_abort = MakeButton(LOC(MSG_xw_abort)),
        End,
      End,
    End;
    if (win) {
      DoMethod(gl_app,OM_ADDMEMBER,win);
      DoMethod(win,MUIM_MultiSet,MUIA_CycleChain,1, bt_skip,bt_abort, NULL);

      MultiIDNotify(MUIA_Pressed,FALSE,
                    bt_skip ,ID_skip ,
                    bt_abort,ID_abort, TAG_END);
      win_act = TRUE;
    }
  }
  return(win_act);
}

ulong Transfer(ULONG tag, ULONG data)
{
static bool   ocall = FALSE;
static ulong  oc;
static char   str_dncom[STRSIZE];
static char   str_upcom[STRSIZE];
struct Info *fi;

  switch ( tag )
  {
    case DT_UPDATE:
      do_update(((struct XP_Msg *)data)->arg,((struct XP_Msg *)data)->arg2);
      break;
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
    case DT_OPEN:
      strcpy(disp_min,"Up:              00000000 0000 000% "
                      "Dn:              00000000 0000 000%  a=abort");
      if (!disp_quiet) {
        open_window();
        if (win_act) {
          set(win,MUIA_Window_Screen,gl_conscreen);
          if (CUR(PB_HardScr)) {
            set(win,MUIA_Window_Activate,TRUE);
          } else {
            set(win,MUIA_Window_Activate,FALSE);
          }  
          set(win       ,MUIA_Window_Title,"Xpr 2.001");
          set(cp_up     ,MUIA_CPS_Max,disp_maxcps);
          set(cp_dn     ,MUIA_CPS_Max,disp_maxcps);
          set(go_upbytes,MUIA_Gauge_Current,0);
          set(go_dnbytes,MUIA_Gauge_Current,0);
          DoMethod(win, MUIM_MultiSet,MUIA_Text_Contents,"",
                   tx_upname,tx_upelap,tx_upexp,tx_upmsg,tx_uperr,tx_upbcheck,
                   tx_upsize,tx_upcps,tx_upbytes,tx_upbsize,NULL);
          DoMethod(win, MUIM_MultiSet,MUIA_Text_Contents,"",
                   tx_dnname,tx_dnelap,tx_dnexp,tx_dnmsg,tx_dnerr,tx_dnbcheck,
                   tx_dnsize,tx_dncps,tx_dnbytes,tx_dnbsize,NULL);
          set(upstat,MUIA_ShowMe,TRUE);
          set(dnstat,MUIA_ShowMe,TRUE);
          if (data == 1) set(dnstat,MUIA_ShowMe,FALSE);
          if (data == 0) set(upstat,MUIA_ShowMe,FALSE);
          set(win,MUIA_Window_Open,TRUE);
        }
      }
      break;
    case DT_OC:
      if (win_act) {
        if (ocall) {
          set(win,MUIA_Window_Screen,gl_conscreen);
          set(win,MUIA_Window_Open,oc); ocall = FALSE;
        } else {
          get(win,MUIA_Window_Open,&oc); set(win,MUIA_Window_Open,FALSE); ocall = TRUE;
        }
      }
      break;
    case DT_INIT:
      return(TRUE);
      break;
    case DT_HANDLE:
      switch (data)
      {
        case ID_skip:  gl_tmp_abort =  1; break;
        case ID_abort: gl_tmp_abort = -1; break;
      }
      break;
    case FT_ADDFILE:
      if (!gl_xferon) {
        send_child(XP_Addfile,data,0,0);
      }
      break;
    case FT_UPLOADRQ:
      PrepFlow(gl_autoxfer);
      if (data) {
        if (GetTag_info(XP_Type,data) == 0) {
          strcpy(up_dir,CUR(PB_UpPath));
          if (Ask_files(gl_conwindow,&up_req,LOC(MSG_select_files_to_upload),"#?",up_dir,&fl1)) {
            up_xp      = data;
            gl_xferend = FALSE; /* arexx kludge */
            return(FALSE);
          }
          return(TRUE);
        } else {
          Transfer(FT_UPLOAD,data);
          return(TRUE);
        }
      }
      break;
    case FT_UPLOADID:
      if (!gl_xferon) {
        fi=GetHead(&gl_fileid);
        while (fi)
        {
          Transfer(FT_ADDFILE,fi->ln_Name);
          fi = GetSucc(fi);
        }
      }
    case FT_UPLOAD:
      if (data AND !gl_xferon AND !gl_serfreeze AND gl_serok) {
        PrepFlow(gl_autoxfer);
        disp_quiet  = CUR(PB_QuietXfer);
        disp_maxcps = CUR(PB_DteRate)/10;

        dprintf_user(str_dncom,GetTag_info(XP_DnCom,data));
        dprintf_user(str_upcom,GetTag_info(XP_UpCom,data));
        send_child(PB_DteRate,CUR(PB_DteRate)             ,0,0);
        send_child(PB_DnPath ,CUR(PB_DnPath)              ,0,0);
        send_child(PB_Name   ,CUR(PB_Name)                ,0,0);
        send_child(PB_DnFiles,CUR(PB_DnFiles)             ,0,0);
        send_child(XP_XprName,GetTag_info(XP_XprName,data),0,0);
        send_child(XP_Init   ,GetTag_info(XP_Init   ,data),0,0);
        send_child(XP_Type   ,GetTag_info(XP_Type   ,data),0,0);
        send_child(XP_DnCom  ,str_dncom                   ,0,0);
        send_child(XP_UpCom  ,str_upcom                   ,0,0);

        if (send_child(XP_Upload,0,0,0)) {
          gl_xferon = TRUE;
          Menu_fresh();
        } else {
          send_child(XP_Clearfiles,0,0,0);
        }
      }
      break;
    case FT_DNLOAD:
      if (data AND !gl_xferon AND !gl_serfreeze AND gl_serok) {
        PrepFlow(gl_autoxfer);
        disp_quiet  = CUR(PB_QuietXfer);
        disp_maxcps = CUR(PB_DteRate)/10;

        dprintf_user(str_dncom,GetTag_info(XP_DnCom,data));
        dprintf_user(str_upcom,GetTag_info(XP_UpCom,data));
        send_child(PB_DteRate,CUR(PB_DteRate)             ,0,0);
        send_child(PB_DnPath ,CUR(PB_DnPath)              ,0,0);
        send_child(PB_Name   ,CUR(PB_Name)                ,0,0);
        send_child(PB_DnFiles,CUR(PB_DnFiles)             ,0,0);
        send_child(XP_XprName,GetTag_info(XP_XprName,data),0,0);
        send_child(XP_Init   ,GetTag_info(XP_Init   ,data),0,0);
        send_child(XP_Type   ,GetTag_info(XP_Type   ,data),0,0);
        send_child(XP_DnCom  ,str_dncom                   ,0,0);
        send_child(XP_UpCom  ,str_upcom                   ,0,0);

        if (send_child(XP_Download,0,0,0)) {
          gl_xferon = TRUE;
          Menu_fresh();
        } else {
          send_child(XP_Clearfiles,0,0,0);
        }
      }
      break;
    case FT_ABORT:
      gl_tmp_abort = -1;
      break;
    case FT_SETUP:

/* to do later :)

LONG __asm __saveds xpr_gets(REG __a0 STRPTR prompt, REG __a1 STRPTR buffer)
{
  if ( Ask_string_sync(prompt,buffer) ) return(0);
  return(1);
}

LONG __asm __saveds xpr_stealgets(REG __a0 STRPTR prompt, REG __a1 STRPTR buffer)
{
  strncpy(stealstr,buffer,STRSIZE-1);
  steal = TRUE;
  return(1);
}

      steal = FALSE;
      if ( (XProtocolBase = OpenLibrary((char *)GetInfoTag(XP_XprName,data),0)) != 0) {
        gl_xpr_io.xpr_filename = (char *)GetInfoTag(XP_Init,data);
        XProtocolSetup(&gl_xpr_io);
        gl_xpr_io.xpr_filename = 0;
        if (XProtocolSetup(&gl_xpr_io) != 0) {
  
          /* Kludge to steal options */
          gl_xpr_io.xpr_filename  = 0;
          gl_xpr_io.xpr_gets      = (long (* )())&xpr_stealgets;
          gl_xpr_io.xpr_extension = 0;
          gl_xpr_io.xpr_options   = 0;
  
          XProtocolSetup(&gl_xpr_io);
  
          gl_xpr_io.xpr_gets      = (long (* )())&xpr_gets;
          gl_xpr_io.xpr_extension = 4;
          gl_xpr_io.xpr_options   = (long (* )())&xem_options;
  
          XProtocolCleanup(&gl_xpr_io);
        }
      }
      CloseLibrary(XProtocolBase); XProtocolBase = 0;
      if (steal) return(stealstr);
      return(0);
*/
      return(0);
      break;
  }
}
