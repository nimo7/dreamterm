#include "global.h"
#include "loc_strings.h"

#ifdef DEBUG
char ver_dterm[]="$VER: DreamTerm 1.50_DEBUG ("__DATE__") "__TIME__"";
BPTR stdout;
#else
char ver_dterm[]="$VER: DreamTerm 1.50 ("__DATE__") "__TIME__"";
#endif

/*
__asm void __chkabort(void)     {}  /* Disable SAS CTRL-C checking. */
__asm void __autoopenfail(void) {}  /* Disable auto openlib msg     */
*/

/* extern struct MUI_Command mui_com; */

extern struct SerialInfo  SerialInfo; /* from serial header      */
extern struct WBStartup  *WBenchMsg;  /* from SAS startup module */

long  pen;
long  pencol[12*3];
long  pool_type = MEMF_ANY,
      pool_size = 16384,
      pool_tres = 8192;

static byte                wakesignum   =-1,    /* gl_wakesig number          */
                           old_pri        ;
static bool                timok        =FALSE; /* timer device open          */
static aptr                myabs        =0,     /* correct MUI bug            */
                           old_msgwin     ,     /* old window in task         */
                           win_obj        ;     /* fakewindow object          */
static ubyte               noconv[256]    ;     /* translate map              */
static ulong               timsig         ;
static struct MsgPort     *timport      =0;
static struct timerequest *timio        =0;
struct GfxBase            *GfxBase      =0;
struct IntuitionBase      *IntuitionBase=0;
struct Library            *AslBase      =0,
                          *AsyncIOBase  =0,
                          *DataTypesBase=0,
                          *DiskfontBase =0,
                          *GadToolsBase =0,
                          *KeymapBase   =0,
/*                          *LocaleBase   =0, */
                          *MUIMasterBase=0,
                          *UtilityBase  =0,
                          *TimerBase    =0,
                          *XEmulatorBase=0;

struct LocaleBase         *LocaleBase   =0; /* why change from Library :c */

/***----------------------------------------------------------------------***/
/*** object hooks                                                         ***/
/***----------------------------------------------------------------------***/

/*
 * Callback from protocol object when list changes
 *
 */
static void callback_protocol(void)
{
  Entry(DT_UPDATE,0);
  Menu_build();
}

void hook_cs(void)
{
  Entry(DT_UPDATE,0);
  Menu_build();
}

void hook_sl(void)
{
  Phone(DT_UPDATE,0);
}

void hook_tb(void)
{
  Entry(DT_UPDATE,0);
  Menu_build();
}

void hook_xm(void)
{
  Entry(DT_UPDATE,0);
  Menu_build();
}

void hook_timer(void)
{
}

/* xpr protocol run/stop */
void hook_xfer(void)
{
  Fileid(DT_UPDATE,0);
  Menu_fresh();
}

/***----------------------------------------------------------------------***/
/*** support                                                              ***/
/***----------------------------------------------------------------------***/

/*
 * Launch protocol driver task
 *
 * -     : -
 *
 * return: TRUE/FALSE
 *
 */
static ulong launch_driver(void)
{
  if (gl_proc_xp = AllocVec(sizeof(struct XP_Sub),MEMF_PUBLIC)) {
    gl_proc_xp->mp_port  = CreateMsgPort();
    gl_proc_xp->ch_reply = CreateMsgPort();
    if (gl_proc_xp->mp_port AND gl_proc_xp->ch_reply) {
      if (gl_proc_xp->child = CreateNewProcTags(NP_Entry   ,&driver_protocol,
                                                NP_Name    ,"dterm_driver_protocol",
                                                NP_Priority,4, TAG_DONE)) {
        /* send initial message */
        gl_proc_xp->ch_msg.mn_Length    = sizeof(struct XP_Msg);
        gl_proc_xp->ch_msg.arg          = gl_proc_xp;
        gl_proc_xp->ch_msg.mn_ReplyPort = gl_proc_xp->ch_reply;
        PutMsg  (&gl_proc_xp->child->pr_MsgPort,&gl_proc_xp->ch_msg);
        WaitPort(gl_proc_xp->ch_reply);
        GetMsg  (gl_proc_xp->ch_reply);
        if (gl_proc_xp->ch_msg.result) return(TRUE);
      }
    }
    DeleteMsgPort(gl_proc_xp->mp_port );
    DeleteMsgPort(gl_proc_xp->ch_reply);
    FreeVec(gl_proc_xp);
    gl_proc_xp = 0;
  }
  return(FALSE);
}

/*
 * Deallocate possible resourcers coz we're goint to sleep
 *
 * -     : -
 *
 * return: -
 *
 */
static void Sleep(void)
{
  gl_process->pr_WindowPtr = old_msgwin;
  EmRemove(&gl_curbbs->tags);

  if (gl_conwindow) {
    Menu_free();
    ScreenRemove();
    CloseWindow(gl_conwindow); gl_conwindow = 0;
  }
  if (gl_linwindow) {
    CloseWindow(gl_linwindow); gl_linwindow = 0;
  }
  if ( CUR(PB_Wb) AND !CUR(PB_HardScr) ) {
    UnlockPubScreen(0,gl_conscreen);
    gl_conscreen = 0;
  }
  gl_scrok = FALSE;
  if ( !gl_onlinepb AND !gl_dial AND !gl_xferon ) {
    RemoveSerial();
  }
  gl_sleep = TRUE;
}

/*
 * Try to allocate necessary resourcers and continue as we want to wake up
 *
 * -     : -
 *
 * return: -
 *
 */
static void Wake(void)
{
struct Info *pb;

  if ( (pb = CreateInfo()) ) {
    CopyTags(&gl_curbbs->tags,&pb->tags);
    SetUp(pb);
    DeleteInfo(pb);
  } else {
    EndProg(LOC(MSG_cannot_wake_up));
  }
  gl_sleep = FALSE;
}

/*
 * Write log entry (uses current settings)
 *
 * -     :
 *
 * return: -
 *
 */
static void WriteLog(void)
{
char            day [32],
                date[32],
                time[32];
long            txdsec,
                rxdsec,
                ontimestop;
bptr            fh;
struct DateTime dt;

  if (fh = Open(CUR(PB_LogName),MODE_READWRITE)) {
    Seek(fh,0,OFFSET_END);
    if ( (txdsec = SerialInfo.txdsec) == 0) txdsec = 1;
    if ( (rxdsec = SerialInfo.rxdsec) == 0) rxdsec = 1;

    dt.dat_Format = FORMAT_DOS;
    dt.dat_Flags  = 0;
    dt.dat_StrDay = day;
    dt.dat_StrDate= date;
    dt.dat_StrTime= time;

    dt.ds_Days    =  gl_onlinebase / (24*60*60);
    dt.ds_Minute  = (gl_onlinebase - (24*60*60*dt.ds_Days)) / 60;
    dt.ds_Tick    = (gl_onlinebase - (24*60*60*dt.ds_Days + dt.ds_Minute*60)) * TICKS_PER_SECOND;
    DateToStr(&dt); day[3] = 0x00;
    FPrintf(fh,"%s %s %s ",(LONG)day,date,time);
    ontimestop = GetSec();
    DateStamp(&dt.dat_Stamp);
    DateToStr(&dt); day[3] = 0x00;
    FPrintf(fh,"%s %s %s\n",(LONG)day,date,time);
    FPrintf(fh,"%s\n%s\n%-5ld %ld\n%-5ld %ld\n%-5ld %ld\n\n",(LONG)
            CUR(PB_Name),
            gl_onlinephone,
            SerialInfo.overrun,ontimestop-gl_onlinebase,
            (CUR(PB_HardSer)) ? SerialInfo.txdbytes/txdsec : 0,SerialInfo.txdbytes,
            (CUR(PB_HardSer)) ? SerialInfo.rxdbytes/rxdsec : 0,SerialInfo.rxdbytes);
    Close(fh);
  }
}

/*
 * Set current settings, every tag is examined and missing ones will be
 * get from profile (defbbs). After this current settings will always reflect
 * what settings really now are, not what was requested (those can fail 
 * sometimes!). This will of course udpate every gui object when needed.
 *
 * entry : Info node containing PB_xxx tags.
 *
 * return: -
 *
 */
void SetUp(struct Info *entry)
{
static ulong scr_o =0, /* screen open   */
             scr_x =0, /* screen x size */
             scr_y =0, /* screen y size */
             scr_d =0, /* screen depth  */
             scr_m =0, /* screen mode   */
             win_wx=0, /* window x size */
             win_wy=0; /* window y size */
long           emul_reset=FALSE,
               scr_reset =FALSE,
               win_reset =FALSE,
               mui_reopen=FALSE,
               mui_refake=FALSE;
ulong          new_o,
               new_scrx,
               new_scry,
               new_d,
               new_m,
               new_wx,
               new_wy,
               new_minwx,
               new_minwy,
               new_maxwx,
               new_maxwy,
               new_realwx,
               new_realwy;
ulong          hardscr,columns,lines,depth,modeid,charset,wb,keepwb,aga,status,
               hlines,emulation,ansipc,xem,hardser,sername,serunit,serbufsize,
               protocol,pubscreen,pubname,conx,cony,forceconxy,overscan,
               autoscroll;
ulong          oc_review,
               oc_protocol;
ulong         *ansicol,*hexcol;
struct Info   *cs,
              *xm;
struct Screen *pub;

word  nl[13] = { ~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0,~0 };
long  i,j,numpen,emcol,realcol,lim;
aptr  cm;
ulong *val;
ulong col[]  = {0xaaaaaaaa,0xaaaaaaaa,0xaaaaaaaa,
                0x00000000,0x00000000,0x00000000,
                0x00000000,0x00000000,0x00000000,
                0xffffffff,0xffffffff,0xffffffff,
                0x00000000,0x00000000,0x00000000,
                0x66666666,0x88888888,0xbbbbbbbb,
                0xffffffff,0xffffffff,0xffffffff,
                0xaaaaaaaa,0xaaaaaaaa,0xaaaaaaaa,
                0xffffffff,0xffffffff,0xffffffff,
                0x00000000,0x00000000,0x00000000,
                0xffffffff,0xffffffff,0xffffffff,
                0x00000000,0x00000000,0x00000000 };
struct DrawInfo *di;

  /*** 'passive' tags (name, phone, f-keys jne...) ***/

  CopyEntryTags(entry,gl_curbbs,
	  						PB_Name				,PB_Phone				,PB_PassWord	,PB_Comment   ,
		  					PB_DialString	,PB_Func1				,PB_Func2			,PB_Func3     ,
			  				PB_Func4			,PB_Func5				,PB_Func6			,PB_Func7     ,
				  			PB_Func8			,PB_Func9				,PB_Func10		,PB_LogName   ,
					  		PB_UpPath			,PB_DnPath			,PB_PhonePre	,PB_DnFiles   ,
						  	PB_Hangup     ,PB_NoCarrier		,PB_NoDialTone,PB_Connect   ,
							  PB_Ringing    ,PB_Busy				,PB_Ok				,PB_Error     ,
  							PB_DialSuffix ,PB_DialAbort		,PB_StatusLine,
	  						PB_LogCall    ,PB_cr2crlf			,PB_lf2crlf		,PB_RtsBlit   ,
		  					PB_CostFirst  ,PB_CostRest		,PB_RxdPopup	,PB_FullDuplex,
			  				PB_AutoXfer   ,PB_QuietXfer		,PB_WaitEntry	,PB_WaitDial  ,
				  			PB_Double     ,PB_ANSIWrap		,PB_ANSIKillBs,PB_ANSIStrip ,
					  		PB_ANSICls    ,PB_ANSICr2crlf	,PB_DTRHangup	,PB_PreScroll ,
						  	PB_BorderBlank, TAG_END);

  /*** screen ***/

  pubscreen	= GetEntryTag(PB_PubScreen	,entry); /* quicker cmp etc. */
  hardscr		= GetEntryTag(PB_HardScr		,entry);
	columns		= GetEntryTag(PB_Columns		,entry);
	lines			= GetEntryTag(PB_Lines			,entry);
	depth			= GetEntryTag(PB_Depth			,entry);
	modeid		= GetEntryTag(PB_ModeId			,entry);
	wb				= GetEntryTag(PB_Wb					,entry);
	charset		= GetEntryTag(PB_Charset		,entry);
	keepwb		= GetEntryTag(PB_KeepWb			,entry);
	aga				= GetEntryTag(PB_Aga				,entry);
	status		= GetEntryTag(PB_Status			,entry);
	hlines		= GetEntryTag(PB_HLines			,entry);
	ansipc		= GetEntryTag(PB_ANSIPc			,entry);
	emulation = GetEntryTag(PB_Emulation	,entry);
	conx			= GetEntryTag(PB_ConX				,entry);
	cony			= GetEntryTag(PB_ConY				,entry);
	forceconxy= GetEntryTag(PB_ForceConXY	,entry);
	overscan	= GetEntryTag(PB_Overscan		,entry);
	autoscroll= GetEntryTag(PB_AutoScroll	,entry);
	ansicol		= GetEntryTag(PB_ANSICol		,entry);
	hexcol		= GetEntryTag(PB_HEXCol			,entry);
	xem				= GetEntryTag(PB_Xem				,entry);

  if ( (cs = Charset(DT_FIND,charset)) == 0 ) cs = Charset(DT_FIND,charset = STR_DEF(PB_Charset));
  if ( (forceconxy AND !wb) OR (forceconxy AND hardscr) ) {
    if ( (conx/GetInfoTag(CS_Xsize,cs)) <   1) conx = GetInfoTag(CS_Xsize,cs) *   1;
    if ( (conx/GetInfoTag(CS_Xsize,cs)) > 256) conx = GetInfoTag(CS_Xsize,cs) * 256;
    if ( (cony/GetInfoTag(CS_Ysize,cs)) <   1) cony = GetInfoTag(CS_Ysize,cs) *   1;
    if ( (cony/GetInfoTag(CS_Ysize,cs)) > 256) cony = GetInfoTag(CS_Ysize,cs) * 256;
    new_wx  = conx;
    new_wy  = cony - ((status == STAT_BAR AND hardscr == FALSE AND wb == FALSE) ? GetInfoTag(CS_Ysize,cs)+1 : 0);
    columns = new_wx / GetInfoTag(CS_Xsize,cs);
    lines   = new_wy / GetInfoTag(CS_Ysize,cs);
  } else {
    if (columns <   1) columns =   1;
    if (columns > 256) columns = 256;
    if (lines   <   1) lines   =   1;
    if (lines   > 256) lines   = 256;
    new_wx  = GetInfoTag(CS_Xsize,cs) * columns;
    new_wy  = GetInfoTag(CS_Ysize,cs) * lines;
  }
  new_o = (!wb OR hardscr) ? TRUE : FALSE;
  new_scrx = new_wx;
  new_scry = new_wy + ((status == STAT_BAR AND hardscr == FALSE) ? GetInfoTag(CS_Ysize,cs)+1 : 0);
  new_d = depth;
  new_m = modeid;
  if (gl_conwindow) { /* this is for user sizeable window */
    new_realwx = gl_conwindow->Width  - gl_conwindow->BorderLeft - gl_conwindow->BorderRight;
    new_realwy = gl_conwindow->Height - gl_conwindow->BorderTop  - gl_conwindow->BorderBottom;
  } else {
    new_realwx = win_wx;
    new_realwy = win_wy;
  }
#ifdef DEBUG
  printf("new_o: %ld\nnew_scrx: %ld new_wx: %ld\nnew_scry: %ld new_wy: %ld\n",(ULONG)new_o,new_scrx,new_wx,new_scry,new_wy);
#endif

  if (gl_sleep OR
      CmpInfoTag(PB_Emulation,emulation,gl_curbbs) OR
      (emulation == EM_ANSI AND CmpInfoTag(PB_ANSICol,ansicol,gl_curbbs) ) OR
      (emulation == EM_ANSI AND CmpInfoTag(PB_ANSIPc ,ansipc ,gl_curbbs) ) OR
      (emulation == EM_HEX  AND CmpInfoTag(PB_HEXCol ,hexcol ,gl_curbbs) ) OR
      (emulation == EM_XEM  AND CmpInfoTag(PB_Xem    ,xem    ,gl_curbbs) ) ) {
    emul_reset = TRUE;
  }
  if (gl_sleep OR
       scr_o != new_o OR
      (emul_reset        AND new_o) OR
			(scr_x != new_scrx AND new_o) OR
			(scr_y != new_scry AND new_o) OR
			(scr_d != new_d    AND new_o) OR
      (scr_m != new_m    AND new_o) OR
			(CmpInfoTag(PB_HardScr   ,hardscr   ,gl_curbbs) AND new_o ) OR
			(CmpInfoTag(PB_Overscan  ,overscan  ,gl_curbbs) AND new_o	) OR
			(CmpInfoTag(PB_AutoScroll,autoscroll,gl_curbbs) AND new_o	) OR
			(CmpInfoTag(PB_KeepWb    ,keepwb    ,gl_curbbs) AND new_o	) OR
			(CmpInfoTag(PB_PubScreen ,pubscreen ,gl_curbbs) AND !new_o) ) {
		emul_reset = scr_reset = TRUE;
	}
	if (gl_sleep  OR
			scr_reset OR
			win_wx != new_wx     OR
			win_wy != new_wy     OR
      win_wx != new_realwx OR
      win_wy != new_realwy) {
		emul_reset = win_reset = TRUE;
	}

	AddInfoTags(gl_curbbs,
							PB_PubScreen	,pubscreen	,
							PB_HardScr		,hardscr		,
  						PB_Columns		,columns		,
	  					PB_Lines			,lines			,
		  				PB_Depth			,depth			,
			  			PB_ModeId			,modeid			,
				  		PB_Wb					,wb					,
					  	PB_Charset		,charset		,
						  PB_KeepWb			,keepwb			,
  						PB_Aga				,aga				,
	  					PB_Status			,status			,
		  				PB_HLines			,hlines			,
			  			PB_ANSIPc			,ansipc			,
				  		PB_Emulation	,emulation	,
					  	PB_ConX				,conx				,
						  PB_ConY				,cony				,
							PB_ForceConXY	,forceconxy	,
							PB_Overscan		,overscan		,
              PB_AutoScroll	,autoscroll	,
							PB_ANSICol		,ansicol		,
							PB_HEXCol			,hexcol			,
							PB_Xem				,xem				, TAG_END);
	if (gl_curcs) Charset(DT_UNLOCK,gl_curcs);
	Charset(DT_LOCK,gl_curcs = cs);
  gl_curtxd = GetInfoTag(CS_Txd,gl_curcs)+4;
  gl_currxd = GetInfoTag(CS_Rxd,gl_curcs)+4;
  gl_curcap = (LookInfoTag(CS_Cap,gl_curcs)) ? GetInfoTag(CS_Cap,gl_curcs)+4 : &noconv;

  if (scr_reset) { /* screen close/open */
    mui_reopen = TRUE;
    gl_process->pr_WindowPtr = old_msgwin;
    if (gl_mui_oc) gl_mui_oc();
    Charset (DT_OC,0); Crypt  (DT_OC,0,0); Dialer  (DT_OC,0); Drop (DT_OC,0);
    Entry   (DT_OC,0); Fileid (DT_OC,0)  ; Toolbar (DT_OC,0); Phone(DT_OC,0);
    Show   (DT_OC,0)  ; Transfer(DT_OC,0);
    Xem     (DT_OC,0);

    oc_review   = xget(gl_review  ,MUIA_Window_Open);
    oc_protocol = xget(gl_protocol,MUIA_Window_Open);

    set(gl_review  ,MUIA_Window_Open,FALSE);
    set(gl_protocol,MUIA_Window_Open,FALSE);

    EmRemove(&gl_curbbs->tags);
    if (gl_conwindow) { Menu_free(); ScreenRemove(); CloseWindow(gl_conwindow); gl_conwindow = 0; }
    if (gl_linwindow) { CloseWindow(gl_linwindow); gl_linwindow = 0; }
    set(win_obj,MUIA_Window_Open,FALSE);
    if (gl_conscreen AND scr_o) {
      while ( !CloseScreen(gl_conscreen) )
      {
        set(win_obj,MUIA_Window_Open,TRUE);
        Warning(LOC(MSG_closing_screen_you_must_close_all_windows));
        set(win_obj,MUIA_Window_Open,FALSE);
      }
      myabs = AllocAbs(4,gl_conscreen); /* fix mui bug */
      gl_conscreen = 0;
    }
    FreeVisualInfo(gl_vi); gl_vi = 0;
    set(win_obj,MUIA_Window_Open,FALSE); win_fake = 0;

    if (new_o) {
      /*--- SCREEN COLOR KLUDGE ---*/
      if (!hardscr AND !keepwb) {
				/* Lue wb:n penien värit */
				numpen = 12;
				if ( (pub = LockPubScreen("Workbench")) ) {
					if (di = GetScreenDrawInfo(pub) ) {
						numpen = (di->dri_NumPens <= 12) ? numpen : 12;
						for (i=0; i < numpen; i++)
						{
							GetRGB32(pub->ViewPort.ColorMap,di->dri_Pens[i],1,&col[i*3]);
						}
						nl[0] = di->dri_Pens[0];
						nl[1] = di->dri_Pens[1];
						FreeScreenDrawInfo(pub,di);
					}
					UnlockPubScreen(0,pub);
				}

				/* lue emulaation värit + monta tarvitsee */
				switch (emulation)
				{
					case EM_HEX:
						val = &hexcol[1];
						emcol = 2;
						break;
					case EM_XEM:
						if ( (xm = Xem(DT_FIND,(char *)xem)) ) {
							val = GetInfoTag(XM_Col,xm);
							emcol = *val / 12;
							val++;
							break;
						}
						emulation = EM_ANSI;
					case EM_ANSI:
						val = &ansicol[1];
						emcol = 8;
						if (ansipc) emcol = 16;
						break;
				}

				/* Laita systeemin värit taulukkoon */
				pen = 0;
				for (i=0; i < numpen; i++)
				{
					for(j=0; j < pen; j++)
					{
						if (	(pencol[j*3+0] == col[i*3+0]) AND
									(pencol[j*3+1] == col[i*3+1]) AND
									(pencol[j*3+2] == col[i*3+2]) ) break;
					}
					if (j == pen) {
						pencol[pen*3+0] = col[i*3+0];
						pencol[pen*3+1] = col[i*3+1];
						pencol[pen*3+2] = col[i*3+2];
						pen++;
					}
				}

				/* Laita colormappiin mahdollisimman monta väriä */
				realcol = Pot2(depth);
				if ( (cm = GetColorMap(realcol)) ) {
					for(i=0; i < emcol; i++)	/* emulation colors */
					{
						if (realcol == i) break;
						SetRGB32CM(cm,i,val[i*3+0],val[i*3+1],val[i*3+2]);
						lim = i;
					}
					j = 0;
					for(   ; i < realcol; i++)	/* system colors */
					{
						if (j >= pen) break;
						SetRGB32CM(cm,i,pencol[j*3+0],pencol[j*3+1],pencol[j*3+2]);
						j++; lim = i;
					}
					/* Hae sopivat penit systeemille */
					nl[1] =   FindColor(cm,col[ 1*3+0],col[ 1*3+1],col[ 1*3+2],lim);
					nl[0] = MyFindColor(cm,col[ 0*3+0],col[ 0*3+1],col[ 0*3+2],lim,nl[1]);
					nl[3] =   FindColor(cm,col[ 3*3+0],col[ 3*3+1],col[ 3*3+2],lim);
					nl[4] =   FindColor(cm,col[ 4*3+0],col[ 4*3+1],col[ 4*3+2],lim);
					nl[5] =   FindColor(cm,col[ 5*3+0],col[ 5*3+1],col[ 5*3+2],lim);
					nl[6] = MyFindColor(cm,col[ 6*3+0],col[ 6*3+1],col[ 6*3+2],lim,nl[5]);
					nl[7] =   FindColor(cm,col[ 7*3+0],col[ 7*3+1],col[ 7*3+2],lim);
					nl[2] = MyFindColor(cm,col[ 2*3+0],col[ 2*3+1],col[ 2*3+2],lim,nl[7]);
					nl[8] = MyFindColor(cm,col[ 8*3+0],col[ 8*3+1],col[ 8*3+2],lim,nl[7]);
					nl[10]=   FindColor(cm,col[10*3+0],col[10*3+1],col[10*3+2],lim);
					nl[9] = MyFindColor(cm,col[ 9*3+0],col[ 9*3+1],col[ 9*3+2],lim,nl[10]);
					nl[11]=   FindColor(cm,col[11*3+0],col[11*3+1],col[11*3+2],lim);
					FreeColorMap(cm);
				}
			}
			/*--- SCREEN COLOR KLUDGE ---*/

			get(gl_app,MUIA_Application_Base,&pubname);
			if ( gl_conscreen = OpenScreenTags(NULL,
													SA_Overscan		,overscan,
													SA_AutoScroll	,autoscroll,
													SA_Width			,new_scrx,
													SA_Height			,new_scry,
													SA_Depth			,new_d,
													SA_DisplayID	,new_m,
													SA_Title			,LOC(MSG_dreamterm_scr_title),
                          SA_ShowTitle	,((hardscr) ? TRUE : gl_title),
                          SA_PubName		,pubname,
                          SA_SysFont    ,1,
													SA_Pens       ,nl,
													SA_DetailPen  ,nl[0],
                          SA_BlockPen   ,nl[1],
													SA_Interleaved,TRUE,
													SA_FullPalette,TRUE,
													SA_SharePens  ,TRUE,
/*
                          SA_BackFill   ,LAYERS_NOBACKFILL, /* lemme test */
*/
                          TAG_DONE) ) {
				mui_refake = TRUE;
				gl_vi = GetVisualInfo(gl_conscreen,NULL);
			} else {
				Warning(LOC(MSG_cannot_open_requested_screen_trying_workbench));
				AddInfoTags(gl_curbbs,
										PB_Wb					,wb				= TRUE,
										PB_HardScr		,hardscr	= FALSE,
										PB_PubScreen	,pubscreen= "Workbench", TAG_END);
				new_o = FALSE;
			}
		}
	}

  if (win_reset) { /* window close/change */
    if (!gl_conwindow) {
      if ( wb AND !hardscr ) {
        if ( !(gl_conscreen = LockPubScreen(pubscreen)) ) {
          Warning(LOC(MSG_cannot_find_public_screen_trying_workbench));
          AddInfoTag(PB_PubScreen,pubscreen = "Workbench",gl_curbbs);
          if ( !(gl_conscreen = LockPubScreen(pubscreen)) ) {
            EndProg(LOC(MSG_cannot_find_public_screen));
          }
        }
        gl_vi = GetVisualInfoA(gl_conscreen,0);
        /* read window limits in new screen
           note: 14 is window right border size, don't know where to read it */
        new_minwx = gl_conscreen->WBorLeft + gl_conscreen->WBorRight + 14 +
                    GetInfoTag(CS_Xsize,cs);
        new_minwy = gl_conscreen->WBorTop + gl_conscreen->WBorBottom + 1 +
                    gl_conscreen->RastPort.TxHeight + GetInfoTag(CS_Ysize,cs);
        new_maxwx = gl_conscreen->Width - gl_conscreen->WBorLeft -
                    gl_conscreen->WBorRight - 14;
        new_maxwy = gl_conscreen->Height - gl_conscreen->WBorTop -
                    gl_conscreen->WBorBottom - 1 -
                    gl_conscreen->RastPort.TxHeight;
        if (new_wx > new_maxwx) {
          AddInfoTag(PB_Columns,columns = new_maxwx/GetInfoTag(CS_Xsize,cs),gl_curbbs);
          new_wx = GetInfoTag(CS_Xsize,cs) * columns;
        }
        if (new_wy > new_maxwy) {
          AddInfoTag(PB_Lines,lines = new_maxwy/GetInfoTag(CS_Ysize,cs),gl_curbbs);
          new_wy = GetInfoTag(CS_Ysize,cs) * lines;
        }

        gl_conwindow=OpenWindowTags(NULL,
                      WA_Top          ,0,
                      WA_InnerWidth	  ,new_wx,
                      WA_InnerHeight  ,new_wy,
                      WA_MinWidth     ,new_minwx,
                      WA_MinHeight    ,new_minwy,
                      WA_MaxWidth     ,gl_conscreen->Width ,
                      WA_MaxHeight    ,gl_conscreen->Height,
											WA_Title				,LOC(MSG_dreamterm_win_title),
											WA_SizeGadget		,TRUE,
											WA_DragBar			,TRUE,
											WA_DepthGadget	,TRUE,
											WA_CloseGadget	,TRUE,
											WA_SmartRefresh	,TRUE,
											WA_Activate			,TRUE,
											WA_NewLookMenus	,TRUE,
											WA_PubScreen		,gl_conscreen,
											WA_IDCMP				,IDCMP_MENUPICK | IDCMP_RAWKEY |
                                       IDCMP_NEWSIZE  | IDCMP_CLOSEWINDOW,
                      TAG_DONE);
#ifdef DEBUG
  printf("WA_InnerWidth : %ld  ",new_wx);
  printf("WA_InnerHeight: %ld\n",new_wy);
  printf("WA_MinWidth   : %ld  ",new_minwx);
  printf("WA_MinHeight  : %ld\n",new_minwy);
  printf("WA_MaxWidth   : %ld  ",new_maxwx);
  printf("WA_MaxHeight  : %ld\n",new_maxwy);
#endif
			} else {
				gl_conwindow=OpenWindowTags(NULL,
											WA_Top					,0,
											WA_Width				,new_wx,
											WA_Height				,new_wy,
											WA_CustomScreen	,gl_conscreen,
											WA_SizeGadget		,FALSE,
											WA_DragBar			,FALSE,
											WA_DepthGadget	,FALSE,
											WA_CloseGadget	,FALSE,
											WA_SmartRefresh	,TRUE,
											WA_Backdrop			,TRUE,
											WA_Activate			,TRUE,
											WA_NewLookMenus	,TRUE,
											WA_Borderless		,TRUE,
											WA_IDCMP				,IDCMP_MENUPICK | IDCMP_RAWKEY |
                                       IDCMP_NEWSIZE | ((hardscr) ? IDCMP_REQVERIFY|IDCMP_MENUVERIFY|IDCMP_ACTIVEWINDOW|IDCMP_INACTIVEWINDOW : 0),
											TAG_DONE);
			}
			if (gl_conwindow == 0) EndProg(LOC(MSG_cannot_open_console));
			gl_process->pr_WindowPtr = gl_conwindow;
			ScreenToUse(hardscr);
			if ( !ScreenInit() ) EndProg(LOC(MSG_cannot_init_console));
			Menu_build();
		} else {
			bool  b=TRUE;
			long  i=0;
			struct IntuiMessage	*msg;

			if (new_o) {
				ChangeWindowBox(gl_conwindow,gl_conwindow->LeftEdge,
																		 gl_conwindow->TopEdge,
																		 new_wx,
																		 new_wy);
			} else {
        long  pos_x=gl_conwindow->LeftEdge,
              pos_y=gl_conwindow->TopEdge;

        /* read window limits in new screen
           note: 14 is window right border size, don't know where to read it */
        new_minwx = gl_conscreen->WBorLeft + gl_conscreen->WBorRight + 14 +
                    GetInfoTag(CS_Xsize,cs);
        new_minwy = gl_conscreen->WBorTop + gl_conscreen->WBorBottom + 1 +
                    gl_conscreen->RastPort.TxHeight + GetInfoTag(CS_Ysize,cs);
        new_maxwx = gl_conscreen->Width - gl_conscreen->WBorLeft -
                    gl_conscreen->WBorRight - 14;
        new_maxwy = gl_conscreen->Height - gl_conscreen->WBorTop -
                    gl_conscreen->WBorBottom - 1 -
                    gl_conscreen->RastPort.TxHeight;
        if (new_wx > new_maxwx) {
          AddInfoTag(PB_Columns,columns = new_maxwx/GetInfoTag(CS_Xsize,cs),gl_curbbs);
          new_wx = GetInfoTag(CS_Xsize,cs) * columns;
        }
        if (new_wy > new_maxwy) {
          AddInfoTag(PB_Lines,lines = new_maxwy/GetInfoTag(CS_Ysize,cs),gl_curbbs);
          new_wy = GetInfoTag(CS_Ysize,cs) * lines;
        }
        if ((new_wx + pos_x) > new_maxwx ) pos_x = new_maxwx-new_wx;
        if ((new_wy + pos_y) > new_maxwy ) pos_y = new_maxwy-new_wy;

        WindowLimits(gl_conwindow,new_minwx,new_minwy,
                                  gl_conscreen->Width,gl_conscreen->Height);
        ChangeWindowBox(gl_conwindow,pos_x,
																		 pos_y,
																		 new_wx + gl_conwindow->BorderRight + gl_conwindow->BorderLeft,
																		 new_wy + gl_conwindow->BorderBottom+ gl_conwindow->BorderTop );
#ifdef DEBUG
  printf("WA_InnerWidth : %ld  ",new_wx);
  printf("WA_InnerHeight: %ld\n",new_wy);
  printf("WA_MinWidth   : %ld  ",new_minwx);
  printf("WA_MinHeight  : %ld\n",new_minwy);
  printf("WA_MaxWidth   : %ld  ",new_maxwx);
  printf("WA_MaxHeight  : %ld\n",new_maxwy);
#endif
			}
      while ( (msg = GT_GetIMsg(gl_conwindow->UserPort)) OR b )
			{
				if (msg) {
					if (msg->Class == IDCMP_NEWSIZE) b = FALSE;
					GT_ReplyIMsg(msg);
				} else {
					TempTimeDelay(UNIT_VBLANK,0,20000);
					if (i++ > 100) 
          {
#ifdef DEBUG
	printf("Window size change was not ack by system!\n");
#endif
            break; /* while */
          }
        }
      }
      WindowLimits(gl_conwindow,new_minwx,new_minwy,
                                gl_conscreen->Width,gl_conscreen->Height);
    }
    if ( !hardscr AND status == STAT_BAR AND !wb ) { /* stat window */
      if (gl_linwindow) { CloseWindow(gl_linwindow); gl_linwindow = 0; }
			gl_linwindow=OpenWindowTags(NULL,
										WA_Top					,new_wx,
										WA_Width				,new_wx,
										WA_Height				,GetInfoTag(CS_Ysize,cs)+1,
										WA_CustomScreen	,gl_conscreen,
										WA_SizeGadget		,FALSE,
										WA_DragBar			,FALSE,
										WA_DepthGadget	,FALSE,
										WA_CloseGadget	,FALSE,
										WA_Backdrop			,TRUE,
										WA_Borderless		,TRUE,
										WA_IDCMP				,IDCMP_ACTIVEWINDOW, TAG_DONE);
		} else {
			if (gl_linwindow) { CloseWindow(gl_linwindow); gl_linwindow = 0; }
		}
		gl_scrok = TRUE;
	}
	if ( !ScreenChange(&gl_curbbs->tags) ) EndProg(LOC(MSG_cannot_modify_console));

  if (emul_reset) {
    EmRemove(&gl_curbbs->tags);
    EmToUse (emulation);
    EmInit  (&gl_curbbs->tags);
    EmReset (&gl_curbbs->tags);
  }

  if (emulation != EM_XEM) {
    EmChange(&gl_curbbs->tags);
  }

  if (mui_refake) {
    PubScreenStatus(gl_conscreen,0);
    set(win_obj,MUIA_Window_Screen,gl_conscreen);
    set(win_obj,MUIA_Window_Open,TRUE);
    DoMethod(win_obj,MUIM_Window_ToBack,0);
    win_fake = win_obj;
  }

  if (!wb OR hardscr) {
  ulong nil;

    VideoControlTags(gl_conscreen->ViewPort.ColorMap,
										 ((CUR(PB_BorderBlank) ? VTAG_BORDERBLANK_SET : VTAG_BORDERBLANK_CLR)),0,
										 VTAG_IMMEDIATE, &nil, TAG_END);
	}
  if (status != STAT_BAR) {
    SetWindowTitles(gl_conwindow,LOC(MSG_dreamterm_win_title),-1);
  }

	scr_o	 = new_o;
	scr_x  = new_scrx;
	scr_y  = new_scry;
	scr_d  = new_d;
	scr_m  = new_m;
	win_wx = new_wx;
  win_wy = new_wy;

	/*** serial ***/

	hardser		= GetEntryTag(PB_HardSer	 ,entry);
	sername		= GetEntryTag(PB_SerName	 ,entry);
	serunit		= GetEntryTag(PB_SerUnit	 ,entry);
	serbufsize= GetEntryTag(PB_SerBufSize,entry);
	if ( !gl_serfreeze AND
		  (!gl_serok OR
				CmpInfoTags(gl_curbbs,
										PB_HardSer	 ,hardser	  ,
										PB_SerName	 ,sername	  ,
										PB_SerUnit	 ,serunit	  ,
										PB_SerBufSize,serbufsize, TAG_END)) )
	{
		AddInfoTags(gl_curbbs,
								PB_HardSer		,hardser		,
								PB_SerName		,sername		,
								PB_SerUnit		,serunit		,
								PB_SerBufSize	,serbufsize	, TAG_END);
		RemoveSerial();
		SerialToUse(hardser);
		if ( !InitSerial() ) {
			Warning(LOC(MSG_cannot_init_serial));
		}
	}
	CopyEntryTags(entry,gl_curbbs,
								PB_DteRate	,PB_FlowControl	,PB_DataBits		,
								PB_Parity		,PB_StopBits		,PB_FixRate			,
                PB_SerShared, TAG_END);
	if (gl_serok) SetPort();

  /* protocol */

  protocol = GetEntryTag(PB_Protocol,entry);
  if (CmpInfoTag(PB_Protocol,protocol,gl_curbbs)) {
    AddInfoTag(PB_Protocol,protocol,gl_curbbs);
    if (gl_curxp) DoMethod(gl_protocol,MUIM_INF_Unlock,gl_curxp);
    gl_curxp = DoMethod(gl_protocol,MUIM_INF_Find,protocol);
    if (gl_curxp) {
      DoMethod(gl_protocol,MUIM_INF_Lock,gl_curxp);
      dprintf_user(gl_upstring,GetInfoTag(XP_Uauto,gl_curxp));
      dprintf_user(gl_dnstring,GetInfoTag(XP_Dauto,gl_curxp));
      PrepFlow(gl_autoxfer);
    }
  }

	if (mui_reopen) {
		Charset	(DT_OC,0); Crypt	(DT_OC,0,0); Dialer	 (DT_OC,0);	Drop (DT_OC,0);
		Entry		(DT_OC,0); Fileid	(DT_OC,0)	 ; Toolbar (DT_OC,0);	Phone(DT_OC,0);
		Show   (DT_OC,0)  ; Transfer(DT_OC,0);
    Xem     (DT_OC,0);
    if (gl_mui_oc) gl_mui_oc();

    set(gl_review  ,MUIA_Window_Screen  ,gl_conscreen);
    set(gl_protocol,MUIA_Window_Screen  ,gl_conscreen);

    set(gl_review  ,MUIA_Window_Open    ,oc_review);
    set(gl_protocol,MUIA_Window_Open    ,oc_protocol);

	}
	if (myabs) { FreeMem(myabs,4); myabs = 0; }
	Menu_fresh();
}

/*
 * Same as SetUp but you pass taglist.
 *
 * nil   : this value is ignored, only used because args limitations.
 * ...   : normal taglist containing PB_xxx tags.
 *
 * return: -
 *
 */
void SetUpTags(LONG nil, ... )
{
struct Info	*pb;
ulong  tag;
va_list	ap;

  va_start(ap, nil);
  if ( (pb = CreateInfo()) == 0 ) { va_end(ap); EndProg(LOC(MSG_out_of_memory)); }
  CopyTags(&gl_curbbs->tags,&pb->tags);
  while( (tag = va_arg(ap,ULONG)) != TAG_END )
  {
    AddInfoTag(tag,va_arg(ap,ULONG),pb);
  }
  SetUp(pb);
  DeleteInfo(pb);
  va_end(ap);
}

/*
 * Same as SetUp but you pass one tag.
 *
 * tag   : PB_xxx tag
 * data  : tag data
 *
 * return: -
 *
 */
void SetUpTag(ULONG tag, ULONG data)
{
  SetUpTags(0, tag,data, TAG_END);
}

/***----------------------------------------------------------------------***/
/*** Program initialize, terminate and handle                             ***/
/***----------------------------------------------------------------------***/

/*
 * Main loop, handles everything. (CALLED ONCE!)
 *
 * -     : -
 *
 * return: -
 *
 */
void main_loop(void)
{
char                 xkey[32];
long                 userx,
                     usery,
                     xreal,
                     wait_half = FALSE,
                     tick_done = TRUE;
ulong                sigs      = 0,
                     id;
struct RXD          *rxd;
struct Info         *pb;
struct InputEvent    ie;
struct IntuiMessage  msg,
                    *new_msg;
struct Info         *temp_pb;
struct XP_Msg       *xp_msg;
ulong                xp_double = 0;
ubyte               *dp_ptr    = 0;
ulong                dp_size   = 0;

  while (!gl_exit)
  {
    /*** status window kludge ***/
    if (gl_linwindow) {
      while ( (new_msg = GT_GetIMsg(gl_linwindow->UserPort)) )
      {
        if (new_msg->Class == IDCMP_ACTIVEWINDOW) ActivateWindow(gl_conwindow);
        GT_ReplyIMsg(new_msg);
      }
    }
    /*** Console (keys/menus) ***/
    if (gl_conwindow ) {
      while ( (new_msg = GT_GetIMsg(gl_conwindow->UserPort)) )
      {
        msg = *new_msg;
        GT_ReplyIMsg(new_msg);
        switch (msg.Class)
        {
          case NULL:
            break;
          case IDCMP_CLOSEWINDOW:
            com(cmd_quit);
            break;
          case IDCMP_NEWSIZE:
            if (gl_conwindow) {
              userx = (gl_conwindow->Width - gl_conwindow->BorderLeft - gl_conwindow->BorderRight); 
              usery = (gl_conwindow->Height- gl_conwindow->BorderTop  - gl_conwindow->BorderBottom);
              userx = (userx < 0) ? 0 : userx / GetInfoTag(CS_Xsize,gl_curcs);
              usery = (usery < 0) ? 0 : usery / GetInfoTag(CS_Ysize,gl_curcs);
#ifdef DEBUG
    printf("IDCMP_NEWSIZE: x: %ld, y: %ld\n",userx,usery);
#endif
              SetUpTags(0,
                        PB_Columns, userx,
                        PB_Lines  , usery, TAG_END);
            } else {
#ifdef DEBUG
    printf("Unexpected IDCMP_NEWSIZE\n");
#endif
            }
            break;
          case IDCMP_REFRESHWINDOW:
            GT_BeginRefresh(gl_conwindow);
            GT_EndRefresh  (gl_conwindow,TRUE);
  					break;
  				case IDCMP_INACTIVEWINDOW:
  					ScreenOff();
  					break;
  				case IDCMP_ACTIVEWINDOW:
  					if (gl_hardstat) ScreenOn();
  					break;
  				case IDCMP_REQVERIFY:
  					ScreenOff();
  					break;
  				case IDCMP_MENUVERIFY:
  					ScreenOff();
  					break;
  				case IDCMP_MENUPICK:
  					if ( msg.Code != MENUNULL ) {
  						long   i = 2,
                     j;
              uword *menu_id,
                     code = msg.Code;
  
  						while ( (code = (IA(code)->NextSelect)) != MENUNULL ) { i++; }
  						if (menu_id = alloc_pool(i*2)) {
  							menu_id[0] = msg.Code;
  							for (j=1; j < i; j++)
  							{
  								menu_id[j] = (IA(menu_id[j-1])->NextSelect);
  							}
  							Menu_handle(menu_id);
                free_pool(menu_id);
  						}
  					}
  					break;
  				case IDCMP_RAWKEY:
            if ( gl_serok AND !gl_serfreeze AND !gl_xferon AND !gl_dial) {
    					if (CUR(PB_Emulation) == EM_XEM) {
    						if ( (xreal = XEmulatorUserMon(&gl_xem_io,xkey,32,&msg)) != -1 ) {
    							Send_raw(xkey,xreal);
    						}
    					} else {
    						if ( (msg.Qualifier & IEQUALIFIER_LSHIFT) OR
    								 (msg.Qualifier & IEQUALIFIER_RSHIFT) ) {
    							switch(msg.Code)
    							{
    								case 76: Send_raw("\033[T",3); break;
    								case 77: Send_raw("\033[S",3); break;
    								case 78: Send_raw("\033[ @",4);break;
    								case 79: Send_raw("\033[ A",4);break;
    								default:
    									ie.ie_Class			= IECLASS_RAWKEY;
    									ie.ie_SubClass	= 0;
    									ie.ie_Code			= msg.Code;
    									ie.ie_Qualifier	= msg.Qualifier;
    									ie.ie_position.ie_addr = *((APTR *)msg.IAddress);
    									if ( (xreal = MapRawKey(&ie, xkey, 32, NULL)) != -1 ) {
    										Send_translate(xkey,xreal);
    									}
    							}
    						} else {
    							switch(msg.Code)
    							{
    								case 76: Send_raw("\033[A",3); break;
    								case 77: Send_raw("\033[B",3); break;
    								case 78: Send_raw("\033[C",3); break;
    								case 79: Send_raw("\033[D",3); break;
    								case 80: Send_string(CUR(PB_Func1)); break;
    								case 81: Send_string(CUR(PB_Func2)); break;
    								case 82: Send_string(CUR(PB_Func3)); break;
    								case 83: Send_string(CUR(PB_Func4)); break;
    								case 84: Send_string(CUR(PB_Func5)); break;
    								case 85: Send_string(CUR(PB_Func6)); break;
    								case 86: Send_string(CUR(PB_Func7)); break;
    								case 87: Send_string(CUR(PB_Func8)); break;
    								case 88: Send_string(CUR(PB_Func9)); break;
    								case 89: Send_string(CUR(PB_Func10));break;
    								default:
    									ie.ie_Class			= IECLASS_RAWKEY;
    									ie.ie_SubClass	= 0;
    									ie.ie_Code			= msg.Code;
    									ie.ie_Qualifier	= msg.Qualifier;
    									ie.ie_position.ie_addr = *((APTR *)msg.IAddress);
    									if ( (xreal = MapRawKey(&ie, xkey, 32, NULL)) != -1) {
    										Send_translate(xkey,xreal);
    									}
    							}
    						}
    					}
            }
  					break;
  				default:
  					if (gl_hardstat) ScreenOn();
  					break;
  			}
  		}
  	}
  	/*** Windows (MUI) ***/
  	id = DoMethod(gl_app,MUIM_Application_NewInput,&sigs);
#ifdef DEBUG
  	if (id) printf("MUI ID: %ld\n");
#endif
  	if (id == MUIV_Application_ReturnID_Quit) gl_exit = TRUE;
  	if (id == SLEEP) Sleep();
  	if (id == WAKE ) Wake();
  	if (id == FAKE ) ActivateWindow(gl_conwindow);
  	if (id >= CS_First AND id <= CS_Last) { Charset	(DT_HANDLE,id); }
  	if (id >= CR_First AND id <= CR_Last) { Crypt		(DT_HANDLE,id,0); }
  	if (id >= DI_First AND id <= DI_Last) { Dialer	(DT_HANDLE,id); }
  	if (id >= DL_First AND id <= DL_Last) { Drop		(DT_HANDLE,id); }
  	if (id >= PE_First AND id <= PE_Last) { Entry		(DT_HANDLE,id); }
  	if (id >= FI_First AND id <= FI_Last) { Fileid	(DT_HANDLE,id); }
  	if (id >= PB_First AND id <= PB_Last) { Phone		(DT_HANDLE,id); }
  	if (id >= TB_First AND id <= TB_Last) { Toolbar (DT_HANDLE,id); }
  	if (id >= FT_First AND id <= FT_Last) { Transfer(DT_HANDLE,id); }
  	if (id >= SL_First AND id <= SL_Last) { Show    (DT_HANDLE,id); }
  	if (id >= XM_First AND id <= XM_Last) { Xem			(DT_HANDLE,id); }
  	if (id >= TE_First AND id <= TE_Last)	if (gl_mui_id) gl_mui_id(id);

    /*** handle possible protocol task messages (don't send messages to child
         during transfer as it will jam both tasks!) ***/
    if (gl_proc_xp) {
      while (xp_msg = GetMsg(gl_proc_xp->mp_port))
      {
        xp_msg->result = FALSE;
        switch (xp_msg->cmd)
        {
          case XP_sread     :
            rxd = GetTimedRXD(xp_msg->arg2,xp_msg->arg3);
            if (rxd->size ) CopyMem(rxd->buffer ,xp_msg->arg,rxd->size);
            if (rxd->size2) CopyMem(rxd->buffer2,xp_msg->arg+rxd->size,rxd->size2);
            xp_msg->result = rxd->size+rxd->size2;
            break;
          case XP_swrite    :
            if (CUR(PB_Double)) {
              if (xp_double) {
                InsertTXD(xp_msg->arg,xp_msg->arg2);
              } else {
                if (xp_msg->arg2 > dp_size) {
                  WaitTXD();
                  FreeVec(dp_ptr);
                  dp_ptr  = AllocVec(xp_msg->arg2,MEMF_PUBLIC);
                  dp_size = xp_msg->arg2;
                }
                if (dp_ptr) {
                  WaitTXD();
                  CopyMem(xp_msg->arg,dp_ptr,xp_msg->arg2);
                  InsertTXD(dp_ptr,xp_msg->arg2);
                } else {
                  InsertTXD(xp_msg->arg,xp_msg->arg2);
                  WaitTXD();
                  dp_size = 0;
                }
              }
              xp_msg->result = 0;
            } else {
              WaitTXD(); /* so we can change mode doublebuffer during send :) */
              if (InsertTXD(xp_msg->arg,xp_msg->arg2)) {
                WaitTXD();
                xp_msg->result = 0;
              } else {
                xp_msg->result = 1;
              }
            }
            break;
          case XP_sflush    :
            GetRXD();
            xp_msg->result = 0;
            break;
          case XP_squery    :
            xp_msg->result = SerQuery();
            break;
          case XP_chkabort  :
            xp_msg->result = gl_tmp_abort;
            gl_tmp_abort   = 0;
            break;
          case XP_double    :
            xp_double = xp_msg->arg;
            xp_msg->result = TRUE;
            break;
          case XP_finished  :
            FreeVec(dp_ptr);
            dp_ptr    = 0;
            dp_size   = 0;
            gl_xferon = FALSE;
            Menu_fresh();
            xp_msg->result = TRUE;
            break;
          case XP_openwindow:
            Transfer(DT_OPEN,xp_msg->arg);
            xp_msg->result = TRUE;
            break;
          case XP_closewindow:
            Transfer(DT_CLOSE,0);
            xp_msg->result = TRUE;
            break;
          case XP_update    :
            Transfer(DT_UPDATE,xp_msg);
            xp_msg->result = TRUE;
            break;
          case XP_seroff    :
            if (!CUR(PB_SerShared) OR CUR(PB_HardSer)) {
              RemoveSerial();
            }
            gl_serfreeze = TRUE;
            Menu_fresh();
            xp_msg->result = TRUE;
            break;
          case XP_seron     :
            if (gl_serok) {
              gl_serfreeze = FALSE;
            } else {
              InitSerial();
              SetPort();
              gl_serfreeze = FALSE;
            }
            Menu_fresh();
            xp_msg->result = TRUE;
            break;
        }
        ReplyMsg(xp_msg);
        if (CheckIO(timio)) break; /* make sure that main_loop gets time too */
      }
    }

    /*** send possible characters from txd buffer ***/
    if ( gl_serok AND !gl_serfreeze) {
      if (wait_half == FALSE) {
        if (wait_half = Send_buffer()) {
          AbortIO(timio);
          WaitIO (timio);
          SetSignal(0,timsig);
          timio->io_Command       = TR_ADDREQUEST;
          timio->tr_time.tv_secs  = 0;
          timio->tr_time.tv_micro = 500000;
          SendIO(timio);
          tick_done = TRUE;
        }
      }
    }

    /*** handle serial activity ***/
    if ( gl_serok AND !gl_serfreeze AND !gl_xferon AND !gl_dial) {
      /* ascii send */
      if (gl_asc_fh AND gl_txdleft > 512) {
        static char str[512];
        long cur;
  
        if ( (cur = dt_read(gl_asc_fh,str,512-1)) > 0 ) {
          str[cur] = 0x00;
          if (gl_asc_mode == 0) Send_translate(str,cur);
          if (gl_asc_mode == 1) Send_raw      (str,cur);
          if (gl_asc_mode == 2) EmPrint       (str,cur);
        } else {
          dt_close(gl_asc_fh);
          gl_asc_fh = 0;
          Menu_fresh();
        }
      }
      /*** read serial input & process ***/
      rxd = GetRXD();
      if (rxd->size) {
  			if ( CUR(PB_AutoXfer) AND gl_curxp ) {
  				HuntFlow(gl_autoxfer,rxd->buffer,rxd->size);
  				if (gl_autoxfer[0].hit AND gl_upstring[0] != 0) { Transfer(FT_UPLOADRQ,gl_curxp); }
  				if (gl_autoxfer[1].hit AND gl_dnstring[0] != 0) { Transfer(FT_DNLOAD	,gl_curxp); }
  			}
  			if (gl_scrok)	{
  				if (CUR(PB_RxdPopup)) { ScreenOn(); gl_hardstat = TRUE; }
  				EmPrint(rxd->buffer,rxd->size);
  			}
  			/*** capture ***/
  			if (gl_cap_fh) {
  				if ( (gl_cap_buf + gl_cap_bufsize - gl_cap_now) < rxd->size ) {
  					if ((gl_cap_now-gl_cap_buf) != 0) dt_write(gl_cap_fh,gl_cap_buf,gl_cap_now-gl_cap_buf);
  					gl_cap_now = gl_cap_buf;
  				}
  				if ( (gl_cap_buf + gl_cap_bufsize - gl_cap_now) > rxd->size ) {
  					if (CUR(PB_Capture) == CAP_TEXT) {
  						gl_cap_now += RawTranslate(gl_cap_now,gl_cap_now,gl_curcap,EmStrip(rxd->buffer,gl_cap_now,rxd->size));
  					}
  					if (CUR(PB_Capture) == CAP_RAW) {
  						CopyMem(rxd->buffer,gl_cap_now,rxd->size);
  						gl_cap_now += rxd->size;
  					}
          }
        }
        /*** review ***/
        if (CUR(PB_Review)) {
          static char buffer[512];
          ubyte *ptr = rxd->buffer;
          long   i   = rxd->size,
                 j;
  
          while ( i )
          {
            i -= ( j = (i > 512) ? 512 : i );
            DoMethod(gl_review,MUIM_REV_FeedData,
                     buffer,
                     RawTranslate(buffer,buffer,gl_curcap,EmStrip(ptr,buffer,j)));
            ptr += j;
          }
        }
      }
    }
    /*** dialer ***/
    if (gl_dial) {
      if ( (pb = Dialer(DI_CHECK,0)) ) {
        Dialer(DT_CLOSE,0);
        gl_onlinepb   = pb;
        gl_onlinebase = GetSec();
        ResetInfo();
        if (! gl_dialnum) {
          Phone(PB_DESELECT,gl_onlinepb);
          Phone(DT_LOCK    ,gl_onlinepb);
          SetUp(gl_onlinepb);
        }
      }
    }
    /*** wait no carrier ***/
    if (gl_onlinepb AND !gl_serfreeze) {
      if ( !CheckCS() ) {
        if (!gl_dialnum) {
          AddInfoTags(gl_onlinepb,
                      PB_CallLast ,GetSec(),
                      PB_CallCost ,GetEntryTag(PB_CallCost ,gl_onlinepb)+((GetSec()-gl_onlinebase)/60)*CUR(PB_CostRest)+CUR(PB_CostFirst),
                      PB_CallTime ,GetEntryTag(PB_CallTime ,gl_onlinepb)+GetSec()-gl_onlinebase,
                      PB_CallCount,GetEntryTag(PB_CallCount,gl_onlinepb)+1,
                      PB_CallTxd  ,GetEntryTag(PB_CallTxd  ,gl_onlinepb)+SerialInfo.txdbytes,
                      PB_CallRxd  ,GetEntryTag(PB_CallRxd  ,gl_onlinepb)+SerialInfo.rxdbytes,
                      TAG_END);
          if (CUR(PB_Safe)) {
            Drop (DT_SAVE,gl_def_dl);
            Phone(DT_SAVE,gl_def_pb);
          }
        }
        if ( CUR(PB_LogCall) ) WriteLog();
        temp_pb     = gl_onlinepb;
        gl_onlinepb = 0;
        Phone(DT_UNLOCK,temp_pb);
        Phone(PB_REDRAW,temp_pb);
        Menu_fresh();
      }
    }

    /*** handle request callback ***/
    if (gl_req) gl_req();

    /*** events updated once per tick (note: tick can be _very_ variable!) ***/
    if (tick_done) {
    ulong cur_time = GetSec();

      tick_done = FALSE;

      /* update some values which don't need to be 100% accurate */
      if (gl_onlinepb) { /* _only_ when online */
        gl_onlinecost = ((cur_time-gl_onlinebase)/60)*CUR(PB_CostRest)+CUR(PB_CostFirst);
        gl_onlinetime = cur_time-gl_onlinebase;
      }

      /* update status line */
      if ( CUR(PB_Status) == STAT_BAR OR CUR(PB_HardScr) ) {
        if ( !(gl_xferon AND CUR(PB_QuietXfer)) ) {
        static char str[STRSIZE];

          dprintf_user(str,CUR(PB_StatusLine));
          StatPrint(str);
        }
      }
    }

    /*** signal handling ***/
    if (sigs) {
      sigs = Wait(sigs | gl_wakesig | gl_xemsig | gl_rxdsig | gl_txdsig | timsig |
                  Bset(gl_conwindow->UserPort->mp_SigBit) | 
                  Bset(gl_proc_xp->mp_port->mp_SigBit) | 
                  ( (gl_linwindow) ? Bset(gl_linwindow->UserPort->mp_SigBit) : 0 ));
    }
    if ( (sigs & gl_xemsig) AND CUR(PB_Emulation) == EM_XEM ) {
      if (! XEmulatorSignal(&gl_xem_io,sigs)) SetUpTag(PB_Emulation,EM_ANSI);
    }

    /*** set maximum wait ***/
    if ( CheckIO(timio) ) {
      WaitIO(timio);
      SetSignal(0,timsig);
      timio->io_Command       = TR_ADDREQUEST;
      timio->tr_time.tv_secs  = 1;
      timio->tr_time.tv_micro = 0;
      SendIO(timio);
      tick_done = TRUE;
      wait_half = FALSE;
    }
  }
}

/*
 * Terminate program and deallocate resources
 *
 * msg   : possible error message for user
 *
 * return: -
 *
 */
void EndProg(strptr msg)
{
char str[STRSIZE];

  if (msg) {
    if (gl_catalog) {
      strcpy(str,LOC(MSG_unable_to_continue));
    } else {
      strcpy(str,"*** UNABLE TO CONTINUE ***\n\n");
    }
    strncat(str,msg,STRSIZE-1);
    Warning(str);
  }

  /* remove driver_protocol task */
  if (gl_proc_xp) {
    gl_proc_xp->ch_msg.cmd          = XP_Remove;
    gl_proc_xp->ch_msg.mn_Length    = sizeof(struct XP_Msg);
    gl_proc_xp->ch_msg.mn_ReplyPort = gl_proc_xp->ch_reply;
    PutMsg(gl_proc_xp->ch_port,&gl_proc_xp->ch_msg);
    WaitPort(gl_proc_xp->ch_reply);
    GetMsg(gl_proc_xp->ch_reply);
    DeleteMsgPort(gl_proc_xp->mp_port );
    DeleteMsgPort(gl_proc_xp->ch_reply);
    FreeVec(gl_proc_xp);
  }

  /* free wakeup signal */
  FreeSignal(wakesignum);

  /* remove timer */
  if (timok  ) { WaitIO((struct IORequest *)timio); CloseDevice((struct IORequest *)timio); }
  if (timio  ) DeleteIORequest((struct IORequest *)timio);
  if (timport) DeleteMsgPort(timport);

  /* set process windowptr and remove emulation */
  gl_process->pr_WindowPtr = old_msgwin;
  EmToUse(EM_ANSI); EmRemove(&gl_curbbs->tags);
  EmToUse(EM_HEX);  EmRemove(&gl_curbbs->tags);
  EmToUse(EM_XEM);  EmRemove(&gl_curbbs->tags);

  /* close subwindows */
  Charset (DT_CLOSE,0); Crypt (DT_CLOSE,0,0); Dialer  (DT_CLOSE,0); Drop    (DT_CLOSE,0);
  Entry   (DT_CLOSE,0); Fileid(DT_CLOSE,0)  ; Toolbar (DT_CLOSE,0);
  Phone   (DT_CLOSE,0); Show  (DT_CLOSE,0)  ; Transfer(DT_CLOSE,0);
  Xem     (DT_CLOSE,0);

  if (gl_review)   set(gl_review  ,MUIA_Window_Open,FALSE);
  if (gl_protocol) set(gl_protocol,MUIA_Window_Open,FALSE);

  /* deallocate sub window stuff */
  Charset(DT_DEL,0); Crypt   (DT_DEL,0,0); Dialer  (DT_DEL,0); 
  Drop   (DT_DEL,0); Entry   (DT_DEL,0)  ; Fileid  (DT_DEL,0);
  Toolbar(DT_DEL,0); Phone   (DT_DEL,0);
  Show   (DT_DEL,0); Transfer(DT_DEL,0);
  Xem    (DT_DEL,0);

  /* remove mui objects and application root */
  if (gl_app) MUI_DisposeObject(gl_app);

  /* remove private classes */
  DelClass_cps();
  DelClass_protocol();
  DelClass_revarea();
  DelClass_review();

  /* deallocate screen stuff & close windows */
  ScreenToUse(SCR_HARD); ScreenRemove();
  ScreenToUse(SCR_SOFT); ScreenRemove();
  if (gl_conwindow) {
    Menu_free();
    CloseWindow(gl_conwindow);
  }
  if (gl_linwindow) {
    CloseWindow(gl_linwindow);
  }
  if ( gl_conscreen != 0 AND (CUR(PB_Wb)) == TRUE ) {
    if ( CUR(PB_HardScr) ) {
      CloseScreen(gl_conscreen); gl_conscreen = 0;
    } else {
      UnlockPubScreen(0,gl_conscreen); gl_conscreen = 0;
    }
  }
  if (gl_conscreen != 0 AND CUR(PB_Wb) == FALSE) {
    CloseScreen(gl_conscreen);
  }
  FreeVisualInfo(gl_vi);

  /* close serial & deallocate kaikki muu */
  SerialToUse(SER_HARD); RemoveSerial();
  SerialToUse(SER_SOFT); RemoveSerial();
  if (gl_asc_fh) Close(gl_asc_fh);
  if (gl_cap_fh) {
    if ((gl_cap_now-gl_cap_buf) != 0) dt_write(gl_cap_fh,gl_cap_buf,gl_cap_now-gl_cap_buf);
    Close(gl_cap_fh);
  }
  if (gl_defbbs ) DeleteInfo(gl_defbbs);
  if (gl_curbbs ) DeleteInfo(gl_curbbs);
  if (gl_fromdef) DeleteInfo(gl_fromdef);

  /* free locale stuff */
  if (gl_locale ) CloseLocale (gl_locale);
  if (gl_catalog) CloseCatalog(gl_catalog);

  /* free libraries */
  if (UtilityBase   ) CloseLibrary(UtilityBase);
  if (MUIMasterBase ) CloseLibrary(MUIMasterBase);
  if (LocaleBase    ) CloseLibrary(LocaleBase);
  if (KeymapBase    ) CloseLibrary(KeymapBase);
  if (IntuitionBase ) CloseLibrary(IntuitionBase);
  if (GfxBase       ) CloseLibrary(GfxBase);
  if (GadToolsBase  ) CloseLibrary(GadToolsBase);
  if (DiskfontBase  ) CloseLibrary(DiskfontBase);
  if (DataTypesBase ) CloseLibrary(DataTypesBase);
  if (AsyncIOBase   ) CloseLibrary(AsyncIOBase);
  if (AslBase       ) CloseLibrary(AslBase);

  /* delete non pooled memory allocations */
  if (myabs       ) FreeMem(myabs,4);
  if (gl_txdbuffer) FreeVec(gl_txdbuffer);

  /* delete memory pool */
  if (gl_pool) DeletePool(gl_pool);

  /* set old task priority for shell startup */
  SetTaskPri(FindTask(0),old_pri);

  if (msg == 0) {
    _exit(0);
  } else {
    _exit(20);
  }
}

/*
 * Startup program
 *
 * -     : -
 *
 * return: -
 *
 */
long main(long agrc, strptr argv)
{
static char  str_loc [64],
             str_dial[64];
ulong        do_dial = FALSE,
             do_loc  = FALSE;
ulong        i;

#ifdef DEBUG
  stdout = Output();
#endif

  /* read some values needed later on */
  gl_process = FindTask(0);              /* this task                */
  old_msgwin = gl_process->pr_WindowPtr; /* old intuition msg window */

  /* get task pri */
  Forbid();
  old_pri = SetTaskPri(gl_process,0);
            SetTaskPri(gl_process,old_pri);
  Permit();

  /* initialize critical dreamterm global structures */
  NewList(&gl_fileid);
  NewList(&gl_phonebook);
  gl_fileid.lh_Type    = NT_USER;
  gl_phonebook.lh_Type = NT_USER;

  for (i = 0; i < 256; i++) noconv[i] = i;

  /* open libraries */
  if (!(AslBase       = OpenLibrary("asl.library"       ,39))) EndProg("Failed to open asl.library (39)");
        AsyncIOBase   = OpenLibrary("asyncio.library"   ,37);
  if (!(DiskfontBase  = OpenLibrary("diskfont.library"  ,39))) EndProg("Failed to open diskfont.library (39)");
  if (!(DataTypesBase = OpenLibrary("datatypes.library", 39))) EndProg("Failed to open datatypes.library (39)");
  if (!(GadToolsBase  = OpenLibrary("gadtools.library"  ,39))) EndProg("Failed to open gadtools.library (39)");
  if (!(GfxBase       = OpenLibrary("graphics.library"  ,39))) EndProg("Failed to open graphics.library (39)");
  if (!(IntuitionBase = OpenLibrary("intuition.library" ,39))) EndProg("Failed to open intuition.library (39)");
  if (!(KeymapBase    = OpenLibrary("keymap.library"    ,37))) EndProg("Failed to open keymap.library (37)");
  if (!(LocaleBase    = OpenLibrary("locale.library"    ,38))) EndProg("Failed to open locale.library (38)");
  if (!(MUIMasterBase = OpenLibrary("muimaster.library" ,12))) EndProg("Failed to open muimaster.library (12)");
  if (!(UtilityBase   = OpenLibrary("utility.library"   ,39))) EndProg("Failed to open utility.library (39)");

  /* prepare config file names */
  strcpy(gl_def_all,"DT_config");
  strcpy(gl_def_cs,"");
  strcpy(gl_def_cu,"");
  strcpy(gl_def_dl,"");
  strcpy(gl_def_gl,"");
  strcpy(gl_def_pb,"");
  strcpy(gl_def_sl,"");
  strcpy(gl_def_tb,"");
  strcpy(gl_def_xm,"");
  strcpy(gl_def_xp,""); 

  /* read tooltypes and set values */
  if (WBenchMsg) {
  strptr             str;
  struct Library    *IconBase;
  struct DiskObject *disk_obj;
    
    if (IconBase = OpenLibrary("icon.library",39)) {
      if (disk_obj = GetDiskObject(WBenchMsg->sm_ArgList->wa_Name)) {

        if (str = FindToolType(disk_obj->do_ToolTypes,"DEF_ALL")) { strncpy(gl_def_all,str,63); gl_def_all[60] = 0; }
        if (str = FindToolType(disk_obj->do_ToolTypes,"DEF_CS"))  { strncpy(gl_def_cs ,str,63); gl_def_cs [60] = 0; }
        if (str = FindToolType(disk_obj->do_ToolTypes,"DEF_CU"))  { strncpy(gl_def_cu ,str,63); gl_def_cu [60] = 0; }
        if (str = FindToolType(disk_obj->do_ToolTypes,"DEF_GL"))  { strncpy(gl_def_gl ,str,63); gl_def_gl [60] = 0; }
        if (str = FindToolType(disk_obj->do_ToolTypes,"DEF_PB"))  {
         strncpy(gl_def_dl,str,63); gl_def_dl[60] = 0;
         strncpy(gl_def_pb,str,63); gl_def_pb[60] = 0;
        }
        if (str = FindToolType(disk_obj->do_ToolTypes,"DEF_SL"))  { strncpy(gl_def_sl ,str,63); gl_def_sl [60] = 0; }
        if (str = FindToolType(disk_obj->do_ToolTypes,"DEF_TB"))  { strncpy(gl_def_tb ,str,63); gl_def_tb [60] = 0; }
        if (str = FindToolType(disk_obj->do_ToolTypes,"DEF_XM"))  { strncpy(gl_def_xm ,str,63); gl_def_xm [60] = 0; }
        if (str = FindToolType(disk_obj->do_ToolTypes,"DEF_XP"))  { strncpy(gl_def_xp ,str,63); gl_def_xp [60] = 0; }

        if (      FindToolType(disk_obj->do_ToolTypes,"ASYNCIO"))   gl_async_dos   = TRUE;
        if (str = FindToolType(disk_obj->do_ToolTypes,"CAPSIZE"))   gl_cap_bufsize = atol(str);
        if (str = FindToolType(disk_obj->do_ToolTypes,"TXDSIZE"))   gl_txdsize     = atol(str);
        if (str = FindToolType(disk_obj->do_ToolTypes,"POOL_TYPE")) pool_type      = atol(str);
        if (str = FindToolType(disk_obj->do_ToolTypes,"POOL_SIZE")) pool_size      = atol(str);
        if (str = FindToolType(disk_obj->do_ToolTypes,"POOL_TRES")) pool_tres      = atol(str);
        if (str = FindToolType(disk_obj->do_ToolTypes,"LOCALE" )) { strncpy(str_loc ,str,63); str_loc [63] = 0; do_loc  = TRUE; }
        if (str = FindToolType(disk_obj->do_ToolTypes,"DIAL"))    { strncpy(str_dial,str,63); str_dial[63] = 0; do_dial = TRUE; }

        /* do some sanity checking for values */
        if (AsyncIOBase    == 0        ) gl_async_dos   = FALSE;
        if (gl_cap_bufsize <= 128      ) gl_cap_bufsize = 128;
        if (gl_txdsize     <= 550      ) gl_txdsize     = 550;
        if (pool_size      <= 128      ) pool_size      = 128;
        if (pool_tres      >= pool_size) pool_tres      = pool_size;

        FreeDiskObject(disk_obj);
      }
      CloseLibrary(IconBase);
    }
  }
  /* set up gl_txdleft from gl_txdsize */
  gl_txdleft = gl_txdsize / 2;

  /* finalize config file names */
  if (gl_def_cs[0] == 0) strcpy(gl_def_cs,gl_def_all);
  if (gl_def_cu[0] == 0) strcpy(gl_def_cu,gl_def_all);
  if (gl_def_dl[0] == 0) strcpy(gl_def_dl,gl_def_all);
  if (gl_def_gl[0] == 0) strcpy(gl_def_gl,gl_def_all);
  if (gl_def_pb[0] == 0) strcpy(gl_def_pb,gl_def_all);
  if (gl_def_sl[0] == 0) strcpy(gl_def_sl,gl_def_all);
  if (gl_def_tb[0] == 0) strcpy(gl_def_tb,gl_def_all);
  if (gl_def_xm[0] == 0) strcpy(gl_def_xm,gl_def_all);
  if (gl_def_xp[0] == 0) strcpy(gl_def_xp,gl_def_all);
  strcat(gl_def_cs,".cs");
  strcat(gl_def_cu,".cu");
  strcat(gl_def_dl,".dl");
  strcat(gl_def_gl,".g");
  strcat(gl_def_pb,".pb");
  strcat(gl_def_sl,".sl");
  strcat(gl_def_tb,".tb");
  strcat(gl_def_xm,".xm");
  strcat(gl_def_xp,".xp");

  /* launch driver_protocol task */
  if (!launch_driver()) EndProg("Failed to create driver protocol");

  /* open locale stuff */
  gl_locale = OpenLocale(0);
  if ((gl_catalog = OpenCatalog(0,
                                "dreamterm.catalog",
                                OC_BuiltInLanguage,0,
                                OC_BuiltInCodeSet ,0,
               (do_loc) ? OC_Language : TAG_IGNORE,str_loc,
                                OC_Version        ,2, TAG_END)) == 0) {
    if ((gl_catalog = OpenCatalog(0,
                                  "dreamterm.catalog",
                                  OC_BuiltInLanguage,0,
                                  OC_BuiltInCodeSet ,0,
                                  OC_Language       ,"english",
                                  OC_Version        ,2, TAG_END)) == 0) {
      EndProg("Cannot open any catalog!");
    }
  }

  /* create memory pool and allocate some critical memory */
  if ( (gl_pool      = CreatePool(pool_type,pool_size,pool_tres)) == 0 ) EndProg("Cannot allocate memory");
  if ( (gl_txdbuffer = AllocVec(gl_txdsize,MEMF_PUBLIC))          == 0 ) EndProg("Cannot allocate memory");

  gl_wakesig = Bset(wakesignum = AllocSignal(-1));
  if (wakesignum == -1) EndProg("Cannot allocate signal");

  ResetInfo();
  if ((gl_class_cps     = AddClass_cps())      == 0) EndProg("Class creation failed");
  if ((gl_class_protocol= AddClass_protocol()) == 0) EndProg("Class creation failed");
  if ((gl_class_revarea = AddClass_revarea())  == 0) EndProg("Class creation failed");
  if ((gl_class_review  = AddClass_review())   == 0) EndProg("Class creation failed");

  /* init sub window stuff */
  if (!Charset (DT_INIT,0)  ) EndProg("Sub window allocation failed");
  if (!Crypt   (DT_INIT,0,0)) EndProg("Sub window allocation failed");
  if (!Dialer  (DT_INIT,0)  ) EndProg("Sub window allocation failed");
  if (!Drop    (DT_INIT,0)  ) EndProg("Sub window allocation failed");
  if (!Entry   (DT_INIT,0)  ) EndProg("Sub window allocation failed");
  if (!Fileid  (DT_INIT,0)  ) EndProg("Sub window allocation failed");
  if (!Phone   (DT_INIT,0)  ) EndProg("Sub window allocation failed");
  if (!Show    (DT_INIT,0)  ) EndProg("Sub window allocation failed");
  if (!Toolbar (DT_INIT,0)  ) EndProg("Sub window allocation failed");
  if (!Transfer(DT_INIT,0)  ) EndProg("Sub window allocation failed");
  if (!Xem     (DT_INIT,0)  ) EndProg("Sub window allocation failed");  

  /* link sub window stuff (for updating) */
  Charset (DT_LINK,(ULONG)&hook_cs);
  Show    (DT_LINK,(ULONG)&hook_sl);
  Toolbar (DT_LINK,(ULONG)&hook_tb);
  Transfer(DT_LINK,(ULONG)&hook_xfer);
  Xem     (DT_LINK,(ULONG)&hook_xm);

  /* create MUI */
  gl_app = ApplicationObject,
    MUIA_Application_Title      ,"DreamTerm",
    MUIA_Application_Version    ,&ver_dterm,
    MUIA_Application_Copyright  ,"© 1992-96 by Pasi Ristioja",
    MUIA_Application_Author     ,"Pasi Ristioja",
    MUIA_Application_Description,"Simple ANSI/XEm2.0 terminal",
    MUIA_Application_Base       ,"DTERM",
    MUIA_Application_HelpFile   ,"DreamTerm.guide",
    MUIA_HelpNode               ,"help_base",
    SubWindow, win_obj = WindowObject,
      MUIA_Window_Backdrop    ,TRUE,
      MUIA_Window_Borderless  ,TRUE,
      MUIA_Window_CloseGadget ,FALSE,
      MUIA_Window_DepthGadget ,FALSE,
      MUIA_Window_DragBar     ,FALSE,
      MUIA_Window_SizeGadget  ,FALSE,
      MUIA_Window_Width       ,0,
      MUIA_Window_Height      ,0,
      MUIA_Window_TopEdge     ,MUIV_Window_TopEdge_Centered,
      MUIA_Window_LeftEdge    ,MUIV_Window_LeftEdge_Centered,
      MUIA_Window_Activate    ,FALSE,
      WindowContents, VGroup,
      End,
    End,
  End;

  if (!gl_app) EndProg("Cannot create Magic User Interface");

  /* add window objects (waste memory as it should be:^) */
  if (gl_review   = NewObject(gl_class_review  ,NULL,TAG_DONE)) DoMethod(gl_app,OM_ADDMEMBER,gl_review  );
  if (gl_protocol = NewObject(gl_class_protocol,NULL,TAG_DONE)) DoMethod(gl_app,OM_ADDMEMBER,gl_protocol);

  if (!gl_review OR !gl_protocol) EndProg("Cannot add objects");

  /* do methods for info windows */
  DoMethod(gl_protocol,MUIM_INF_Callback,&callback_protocol);

  /* do application control */
  DoMethod(gl_app ,MUIM_Notify,MUIA_Application_Iconified,TRUE  ,gl_app,2,MUIM_Application_ReturnID,SLEEP);
  DoMethod(gl_app ,MUIM_Notify,MUIA_Application_Iconified,FALSE ,gl_app,2,MUIM_Application_ReturnID,WAKE);
  DoMethod(win_obj,MUIM_Notify,MUIA_Window_Activate,TRUE        ,gl_app,2,MUIM_Application_ReturnID,FAKE);

  if ( (timport = CreateMsgPort()) == 0) {
    EndProg("Cannot create timer");
  }
  if ( (timio = (struct timerequest *)CreateIORequest(timport,sizeof(struct timerequest))) == 0) {
    EndProg("Cannot create timer");
  }
  if (OpenDevice(TIMERNAME,UNIT_VBLANK,(struct IORequest *)timio,0) != 0) {
    EndProg("Cannot create timer");
  }
  timok = TRUE;
  timsig = Bset(timport->mp_SigBit);
  timio->io_Command = TR_ADDREQUEST;
  timio->tr_time.tv_secs	= 1;
  timio->tr_time.tv_micro	= 0;
  SendIO((struct IORequest *)timio);
  TimerBase = timio->io_Device; /* I wonder if this is legal */

  if ( (gl_defbbs = CreateInfo()) == 0 OR
       (gl_curbbs = CreateInfo()) == 0 OR
       (gl_fromdef= CreateInfo()) == 0 ) EndProg("Cannot allocate critical data");

  /* add ALL hardcoded tags (defaults) to defbbs */
  for (i = STR_PB_FIRST; i <= STR_PB_LAST; i++) 
  {
    AddInfoTag(i,STR_DEF(i),gl_defbbs);
#ifdef DEBUG
    printf("STR_DEF (%-24s) %-8lx: %s\n",TagName(i),i,TagData2Str(i,STR_DEF(i)));
#endif
  }
  for (i = VAL_PB_FIRST; i <= VAL_PB_LAST; i++)
  {
    AddInfoTag(i,VAL_DEF(i),gl_defbbs);
#ifdef DEBUG
    printf("VAL_DEF (%-24s) %-8lx: %s\n",TagName(i),i,TagData2Str(i,VAL_DEF(i)));
#endif
  }
  for (i = DAT_PB_FIRST; i <= DAT_PB_LAST; i++)
  {
    AddInfoTag(i,DAT_DEF(i),gl_defbbs);
#ifdef DEBUG
    printf("DAT_DEF (%-24s) %-8lx: %s\n",TagName(i),i,TagData2Str(i,DAT_DEF(i)));
#endif
  }

  /* load configuration */
  DoMethod(gl_protocol,MUIM_INF_Load,gl_def_xp);
  Charset (DT_LOAD,gl_def_cs);
  Toolbar (DT_LOAD,gl_def_tb);
  Xem     (DT_LOAD,gl_def_xm);
  com     (cmd_load_profile,gl_def_gl);

  AddInfoTag(PB_CryptKey,STR_DEF(PB_CryptKey),gl_curbbs);
  
  if (!com(cmd_load_current,gl_def_cu)) {
    Warning(LOC(MSG_cannot_load_use_current_settings));
    CopyInfoTags(gl_defbbs,gl_curbbs, ENV_TAGS, TAG_END);
    SetUp(gl_defbbs);
  }
/*
  if (!LoadCurrent()) {
    Warning(LOC(MSG_cannot_load_use_current_settings));
    CopyInfoTags(gl_defbbs,gl_curbbs, ENV_TAGS, TAG_END);
    SetUp(gl_defbbs);
  }
*/

  SetTaskPri((struct Task *)gl_process,CUR(PB_Priority));

  /* start review */
  if (CUR(PB_Review)) {
    set(gl_review,MUIA_REV_MaxLines,CUR(PB_ReviewSize));
/*
    if ((gl_revtemp = alloc_pool(gl_revtempsize)) == 0) {
      AddInfoTag(PB_Review,FALSE,gl_curbbs);
    }
*/
  }

  /* send initstring */
  Send_string((char *)CUR(PB_InitString));

  /* set toolbar */
  if (gl_curtb = Toolbar(DT_FIND,CUR(PB_Toolbar))) {
    if (CUR(PB_Baron)) Toolbar(TB_ENABLE,gl_curtb);
  }

  if ( CUR(PB_Safe) ) {
  char str_a[64+8],
       str_b[64+8];

    strcpy(str_a,gl_def_pb); strcat(str_a,".old");
    strcpy(str_b,gl_def_pb); strcat(str_b,".oldest");
    CloneFile(str_a    ,str_b);
    CloneFile(gl_def_pb,str_a);
    strcpy(str_a,gl_def_dl); strcat(str_a,".old");
    strcpy(str_b,gl_def_dl); strcat(str_b,".oldest");
    CloneFile(str_a    ,str_b);
    CloneFile(gl_def_dl,str_a);
    strcpy(str_a,gl_def_sl); strcat(str_a,".old");
    strcpy(str_b,gl_def_sl); strcat(str_b,".oldest");
    CloneFile(str_a    ,str_b);
    CloneFile(gl_def_sl,str_a);
/*    
    CloneFile("DT_config.pb.old","DT_config.pb.oldest");
    CloneFile("DT_config.pb","DT_config.pb.old");
    CloneFile("DT_config.dl","DT_config.dl.old");
    CloneFile("DT_config.dl.old","DT_config.dl.oldest");
    CloneFile("DT_config.sl.old","DT_config.sl.oldest");
    CloneFile("DT_config.sl","DT_config.sl.old");
*/
  }
  Drop (DT_LOAD,gl_def_dl);
  Phone(DT_LOAD,gl_def_pb);
  Show (DT_LOAD,gl_def_sl);

  gl_menufreeze = FALSE;
  Menu_fresh();

  /* prepare possible auto dialing */
  if (do_dial) {
    Phone (PB_SELECT,str_dial);
    Dialer(DI_DIAL  ,0);
  }

  /* all is done, now just go to wait what user wants */
  main_loop();

  EndProg(0);
}
