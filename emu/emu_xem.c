#include "global.h"
#include "loc_strings.h"

/*
char ver_xemdriver[]="1.04 (Oct 14 1994)";
*/

#define	DMAX	1024

extern struct Library			*XEmulatorBase;
static const LONG 				c_table[] = { 1,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4 };
static BOOL								xemok = FALSE;
static BOOL								dodb	=TRUE;
static UBYTE							db1[DMAX];
static UBYTE							db2[DMAX];
static ULONG							window,font,console,signal,screendepth,tempsig;
static struct Window			*xemwin=0;
static struct XEmulatorHostData	hostdata;

/* Support */

static BOOL GetOptionMode(struct xem_option *Option)
{
	if(!stricmp(Option->xemo_value, "off"))
		return(0);

	if(!stricmp(Option->xemo_value, "false"))
		return(0);

	if(!stricmp(Option->xemo_value, "f"))
		return(0);

	if(!stricmp(Option->xemo_value, "no"))
		return(0);

	if(!stricmp(Option->xemo_value, "n"))
		return(0);


	if(!stricmp(Option->xemo_value, "on"))
		return(1);

	if(!stricmp(Option->xemo_value, "true"))
		return(1);

	if(!stricmp(Option->xemo_value, "t"))
		return(1);

	if(!stricmp(Option->xemo_value, "yes"))
		return(1);

	if(!stricmp(Option->xemo_value, "y"))
		return(1);

	return(0);
}

/* xem callbacks */

LONG __asm __saveds xem_sread(REG __a0 APTR buffer, REG __d0 LONG size,
															REG __d1 LONG timeout)
{
struct RXD	*rxd;

	rxd = GetTimedRXD(size,timeout);
	if (rxd->size	) CopyMem(rxd->buffer,buffer,rxd->size);
	if (rxd->size2) CopyMem(rxd->buffer2,(ULONG)buffer+rxd->size,rxd->size2);
	return(rxd->size+rxd->size2);
}

LONG __asm __saveds xem_swrite(REG __a0 APTR buffer, REG __d0 LONG size)
{
	if (size <= DMAX) {
		if (dodb) {
			dodb = FALSE;
			CopyMem(buffer,db1,size);
			if (InsertTXD(db1,size)) return(0);
			return(1);
		} else {
			dodb = TRUE;
			CopyMem(buffer,db2,size);
			if (InsertTXD(db2,size)) return(0);
			return(1);
		}
	} else {
		if (InsertTXD(buffer,size)) { WaitTXD(); return(0); }
		return(1);
	}
}

LONG __asm __saveds xem_sflush(void)
{
	GetRXD(); return(0);
}

LONG __asm __saveds xem_sbreak(void)
{
	return(0);
}

LONG __asm __saveds xem_squery(void)
{
	return(SerQuery());
}

void __saveds xem_sstart(VOID)
{
#ifdef DEBUG
	printf("xem_sstart()\n");
#endif
	gl_xferon = FALSE;
	Menu_fresh();
}

LONG __saveds xem_sstop(VOID)
{
#ifdef DEBUG
	printf("xem_sstop()\n");
#endif
	if (gl_xferon) return(1);	/* xpr already on */
	gl_xferon = TRUE;
	Menu_fresh();
	return(0);
}

VOID __saveds __asm xem_tbeep(REG __d0 ULONG ntimes, REG __d1 ULONG delay)
{
	while(ntimes--)
	{
		DisplayBeep(NULL);
		Delay(delay);
	}
}

LONG __asm __saveds xem_tgets(REG __a0 STRPTR prompt, REG __a1 STRPTR buffer)
{
	if ( Ask_string_sync(prompt,buffer) ) return(0);
	return(1);
}

void __saveds __asm ProcessMacroKeys(REG __a0 struct XEmulatorMacroKey *key)
{
	VOID (*routine)(VOID);
	UBYTE	c;

	if(routine = key->xmk_UserData) {
		(*routine)();
	}	else {
		c = key->xmk_Code - 0x50;
		Send_translate(&c,1);
	}
}

LONG __saveds __asm xem_options(REG __d0 LONG numopts,REG __a0 struct xem_option *opts[])
{
struct xem_option	*option;
APTR	win,grp,bt_ok,obj1,obj2;
LONG	optnow=0,comid=-1;
APTR	obj[32];
ULONG	sigs,id,data;
BOOL	rungo=TRUE;

enum ID		{ ID_quit = XO_First, ID_opt };

	/* Create window */
/*
	set(gl_app,MUIA_Application_Sleep,TRUE);
*/
	win = WindowObject,
	MUIA_Window_ID		,MAKE_ID('X','E','M','O'),
	MUIA_Window_Title	,LOC(MSG_options),
	MUIA_HelpNode			,"xemopt",
	MUIA_Window_Width	,MUIV_Window_Width_Visible(50),
		WindowContents, VGroup,
			Child, grp = ColGroup( (numopts < 16) ? 2 : 4 ),
				GroupFrame,
			End,
			Child, bt_ok = MakeButton(LOC(MSG_key_ok)),
		End,
	End;
	if (win == 0) return(-1);

	while (numopts > optnow)
	{
		option = opts[optnow];
		switch (option->xemo_type)
		{
			case XEMO_BOOLEAN:
				obj1 = Label1L(option->xemo_description);
				obj2 =
					HGroup,
						Child, obj[optnow] = ImageObject,
							ImageButtonFrame,
							MUIA_InputMode			,MUIV_InputMode_Toggle,
							MUIA_Image_Spec			,MUII_CheckMark,
							MUIA_Background			,MUII_ButtonBack,
							MUIA_Image_FontMatch,TRUE,
							MUIA_ShowSelState		,FALSE,
							MUIA_Selected       ,GetOptionMode(option),
						End,
						Child, RectangleObject, MUIA_Weight,1, End,
					End;
			break;
			case XEMO_LONG:
			case XEMO_STRING:
			case XEMO_COMMPAR:
				obj1 = Label2L(option->xemo_description);
				obj2 = obj[optnow] =
					 StringObject,
							StringFrame,
							MUIA_String_MaxLen	,option->xemo_length,
							MUIA_String_Contents,option->xemo_value,
						End;
			break;
			case XEMO_HEADER:
				obj1 =
					TextObject,
						MUIA_Text_PreParse,"\33c",
						MUIA_Text_Contents,option->xemo_description,
						MUIA_Weight				,0,
						MUIA_InnerLeft		,0,
						MUIA_InnerRight		,0,
					End;
				obj2 = RectangleObject, MUIA_Weight,0, End;
			break;
			case XEMO_COMMAND:
				obj1 = obj[optnow] = MakeButton(option->xemo_description);
				obj2 = RectangleObject, MUIA_Weight,0, End;
			break;
			default: goto fail;
		}
		if (obj1 == 0 OR obj2 == 0) goto fail;
		DoMethod(grp,OM_ADDMEMBER,obj1);
		DoMethod(grp,OM_ADDMEMBER,obj2);
		optnow++;
	}
	if (numopts >= 16) {
		if ( (numopts % 2) == 1) {
				obj1 = RectangleObject, MUIA_Weight,0, End;
				obj2 = RectangleObject, MUIA_Weight,0, End;
			if (obj1 == 0 OR obj2 == 0) goto fail;
			DoMethod(grp,OM_ADDMEMBER,obj1);
			DoMethod(grp,OM_ADDMEMBER,obj2);
		}
	}

	DoMethod(gl_app,OM_ADDMEMBER,win);

	/* Notifyt */
	DoMethod(win	,MUIM_Notify,MUIA_Window_CloseRequest,TRUE, gl_app,2,MUIM_Application_ReturnID,ID_quit);
	DoMethod(bt_ok,MUIM_Notify,MUIA_Pressed,FALSE, gl_app,2,MUIM_Application_ReturnID,ID_quit);
	optnow = 0;
	while (numopts > optnow)
	{
		if (opts[optnow]->xemo_type == XEMO_COMMAND) {
			DoMethod(obj[optnow],MUIM_Notify,MUIA_Pressed,FALSE,gl_app,2,MUIM_Application_ReturnID,ID_opt+optnow);
		}
		if (opts[optnow]->xemo_type == XEMO_COMMPAR) {
			DoMethod(obj[optnow],MUIM_Notify,MUIA_String_Acknowledge,MUIV_EveryTime,gl_app,2,MUIM_Application_ReturnID,ID_opt+optnow);
		}
		optnow++;
	}

	set(win,MUIA_Window_Screen,gl_conscreen);
	set(win,MUIA_Window_Open,TRUE);
	
	/* Wait window */
	while (rungo)
	{
		id = DoMethod(gl_app,MUIM_Application_Input,&sigs);
		if (id == MUIV_Application_ReturnID_Quit) { gl_exit = TRUE; rungo = FALSE; }
		if (id == ID_quit) rungo = FALSE;
		if (id >= ID_opt AND id <= ID_opt+31) {
			comid = id-ID_opt;
			if (opts[comid]->xemo_type == XEMO_COMMPAR) { /* Get command string */
				get(obj[comid],MUIA_String_Contents,&data);
				if (opts[comid]->xemo_value != 0) {
					strcpy(opts[comid]->xemo_value,data);
				}
			}
			rungo = FALSE;
		}
		if (sigs) Wait(sigs);
	}

	if (comid == -1) {
		/* Collect new data */
		comid = optnow = 0;
		while (numopts > optnow)
		{
			option = opts[optnow];
			switch (option->xemo_type)
			{
				case XEMO_BOOLEAN:
					get(obj[optnow],MUIA_Selected,&data);
					if (option->xemo_value != 0) {
						if (data) {
							strcpy(option->xemo_value,"yes");
						} else {
							strcpy(option->xemo_value,"no");
						}
					}
				break;
				case XEMO_LONG:
				case XEMO_STRING:
					get(obj[optnow],MUIA_String_Contents,&data);
					if (option->xemo_value != 0) {
						strcpy(option->xemo_value,data);
					}
				break;
			}
			comid |= (1 << optnow);
			optnow++;
		}
	} else {
		comid = (1 << comid);
	}

	set(win,MUIA_Window_Open,FALSE);
	DoMethod(gl_app,OM_REMMEMBER,win);
	MUI_DisposeObject(win);
/*
	set(gl_app,MUIA_Application_Sleep,FALSE);
*/
	return(comid);

fail2:
	DoMethod(gl_app,OM_REMMEMBER,win);
fail:
	MUI_DisposeObject(win);
	set(gl_app,MUIA_Application_Sleep,FALSE);
	return(-1);
}

/* functions */

BOOL __asm InitXEM(REG __a0 struct TagBase *tagbase)
{
LONG	colors,depth;
char	temp[STRSIZE];

	if ( (gl_curxm = (struct Info *)Xem(DT_FIND,CUR(PB_Xem))) == 0) return(FALSE);

	colors = *((ULONG *)GetInfoTag(XM_Col,gl_curxm)) / 12;
	depth	 = c_table[colors-1];
#ifdef DEBUG
	printf("XEm colors/depth: %ld/%ld\n",colors,depth);
#endif

	gl_xem_io.xem_window			= gl_conwindow;
	gl_xem_io.xem_font 				= gl_conwindow->RPort->Font;
	gl_xem_io.xem_signal			= &gl_xemsig;
	gl_xem_io.xem_screendepth	= (CUR(PB_Depth) <= depth) ? CUR(PB_Depth) : depth;
/*
	gl_xem_io.xem_screendepth	= (CUR(PB_Depth) <= 3) ? CUR(PB_Depth) : 3;
*/
	gl_xem_io.xem_swrite			= (APTR)xem_swrite;
	gl_xem_io.xem_sread				= (APTR)xem_sread;
	gl_xem_io.xem_sbreak			= (APTR)xem_sbreak;
	gl_xem_io.xem_sflush			= (APTR)xem_sflush;
	gl_xem_io.xem_sstart			= (APTR)xem_sstart;
	gl_xem_io.xem_sstop				= (APTR)xem_sstop;
	gl_xem_io.xem_tbeep				= (APTR)xem_tbeep;
	gl_xem_io.xem_tgets				= (APTR)xem_tgets;
	gl_xem_io.xem_toptions		= (APTR)xem_options;
	gl_xem_io.xem_process_macrokeys = (APTR)ProcessMacroKeys;

	hostdata.InESC = hostdata.InCSI = FALSE;

	if (! CUR(PB_Wb)) Screen_addcol(GetInfoTag(XM_Col,gl_curxm)+4,colors);

	if ( (XEmulatorBase = OpenLibrary(GetInfoTag(XM_Lib,gl_curxm),0)) ) {
		if (XEmulatorSetup(&gl_xem_io)) {
			TempName(temp);
			SaveDatTag(XM_Pref,&gl_curxm->tags,temp);
			XEmulatorPreferences(&gl_xem_io,temp,XEM_PREFS_LOAD);
			DeleteFile(temp);
			if (XEmulatorOpenConsole(&gl_xem_io)) {
				Xem(DT_LOCK,gl_curxm);
				return(TRUE);
			}
			XEmulatorCleanup(&gl_xem_io);
		}
		CloseLibrary(XEmulatorBase);
	}
	gl_curxm = 0;
	return(FALSE);
}

BOOL __asm RemoveXEM(REG __a0 struct TagBase *tagbase)
{
	if (gl_curxm) {
		Screen_remcol();
		XEmulatorCloseConsole(&gl_xem_io);
		XEmulatorCleanup(&gl_xem_io);
		CloseLibrary(XEmulatorBase);
		Xem(DT_UNLOCK,gl_curxm);
		gl_curxm	= 0;
		gl_xemsig	= 0;
	}
	return(TRUE);
}

BOOL __asm ResetXEM(REG __a0 struct TagBase *tagbase)
{
	XEmulatorResetConsole(&gl_xem_io);
	return(TRUE);
}

BOOL __asm ChangeXEM(REG __a0 ULONG *tag)
{
char	temp[STRSIZE];

	TempName(temp);
	SaveDatTag(XM_Pref,&gl_curxm->tags,temp);
	XEmulatorPreferences(&gl_xem_io,temp,XEM_PREFS_LOAD);
	XEmulatorOptions(&gl_xem_io);
	DeleteFile(temp);
	return(TRUE);
}

void __asm PrintXEM(REG __a0 APTR data, REG __d0 ULONG size)
{
	XEmulatorWrite(&gl_xem_io,data,size);
}

ULONG __asm StripXEM(REG __a0 APTR from, REG __a1 APTR to, REG __d0 ULONG size)
{
	hostdata.Source = from; hostdata.Destination = to;
	return(XEmulatorHostMon(&gl_xem_io,&hostdata,size));
}

/* Some handle */

BOOL XemPrefMode(void)
{
	xemwin=OpenWindowTags(NULL,
												WA_Top					,0,
												WA_Width				,gl_conwindow->Width,
												WA_Height				,gl_conwindow->Height,
												WA_Backdrop			,TRUE,
												WA_Borderless		,TRUE,
												WA_SizeGadget		,FALSE,
												WA_DragBar			,FALSE,
												WA_DepthGadget	,FALSE,
												WA_CloseGadget	,FALSE,
												WA_CustomScreen	,gl_conscreen,
												TAG_DONE);
	window			= (ULONG)gl_xem_io.xem_window;
	font				= (ULONG)gl_xem_io.xem_font;
	console			= (ULONG)gl_xem_io.xem_console;
	signal			= (ULONG)gl_xem_io.xem_signal;
	screendepth	= (ULONG)gl_xem_io.xem_screendepth;
	gl_xem_io.xem_window			= (xemwin) ? xemwin : gl_conwindow;
	gl_xem_io.xem_font 				= gl_conwindow->RPort->Font; /* guru is close... */
	gl_xem_io.xem_signal			= &tempsig;
	gl_xem_io.xem_screendepth	= (CUR(PB_Depth) <= 4) ? CUR(PB_Depth) : 4;
	gl_xem_io.xem_swrite			= (APTR)xem_swrite;
	gl_xem_io.xem_sread				= (APTR)xem_sread;
	gl_xem_io.xem_sbreak			= (APTR)xem_sbreak;
	gl_xem_io.xem_sflush			= (APTR)xem_sflush;
	gl_xem_io.xem_sstart			= (APTR)xem_sstart;
	gl_xem_io.xem_sstop				= (APTR)xem_sstop;
	gl_xem_io.xem_tbeep				= (APTR)xem_tbeep;
	gl_xem_io.xem_tgets				= (APTR)xem_tgets;
	gl_xem_io.xem_toptions		= (APTR)xem_options;
	gl_xem_io.xem_process_macrokeys = (APTR)ProcessMacroKeys;
	return(TRUE);
}

void XemUserMode(void)
{
	gl_xem_io.xem_window			= (struct Window *)window;
	gl_xem_io.xem_font				= (struct TextFont *)font;
	gl_xem_io.xem_console			= (APTR)console;
	gl_xem_io.xem_signal			= (ULONG *)signal;
	gl_xem_io.xem_screendepth	= screendepth;
	if (xemwin) { CloseWindow(xemwin); xemwin = 0; }
}
