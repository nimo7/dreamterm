#include "global.h"
#include "loc_strings.h"

/*
char ver_xem[]="1.05 (Aug 30 1995)";
*/

enum ID	{ ID_xm	= XM_First,
					ID_name			,ID_libpop		,ID_lib			,ID_pref		,
					ID_reset		,ID_add				,ID_del			,
					ID_loadset	,ID_saveset		,ID_appset	,ID_savesel	,
					ID_quit			};

static bool win_act = FALSE;
static aptr lv_xm   = 0;
static APTR	win,st_name,io_lib,st_lib,sl_colors,bt_pref,bt_reset,pa_col;
static APTR	bt_add,bt_del;

static struct MUI_Palette_Entry xementry[] =
{
	{ 0,0x00000000,0x00000000,0x00000000, 0	},
	{ 0,0x00000000,0x00000000,0x00000000, 1	},
	{ 0,0x00000000,0x00000000,0x00000000, 2	},
	{ 0,0x00000000,0x00000000,0x00000000, 3	},
	{ 0,0x00000000,0x00000000,0x00000000, 4	},
	{ 0,0x00000000,0x00000000,0x00000000, 5	},
	{ 0,0x00000000,0x00000000,0x00000000, 6	},
	{ 0,0x00000000,0x00000000,0x00000000, 7	},
	{ 0,0x00000000,0x00000000,0x00000000, 8	},
	{ 0,0x00000000,0x00000000,0x00000000, 9	},
	{ 0,0x00000000,0x00000000,0x00000000,10	},
	{ 0,0x00000000,0x00000000,0x00000000,11	},
	{ 0,0x00000000,0x00000000,0x00000000,12	},
	{ 0,0x00000000,0x00000000,0x00000000,13	},
	{ 0,0x00000000,0x00000000,0x00000000,14	},
	{ 0,0x00000000,0x00000000,0x00000000,15	},
	{ MUIV_Palette_Entry_End,0,0,0,0 },
};

static struct NewMenu menu[] =
{
	{ NM_TITLE,(strptr)MSG_xem_configuration,  0,0,0,0								},
	{ NM_ITEM	,(strptr)MSG_load			  	    ,"o",0,0,(APTR)ID_loadset	},
	{ NM_ITEM	,(strptr)MSG_save		  		    ,"s",0,0,(APTR)ID_saveset	},
	{ NM_ITEM	,(strptr)MSG_append				    ,"a",0,0,(APTR)ID_appset	},
	{ NM_ITEM	,(strptr)MSG_save_selected    ,"c",0,0,(APTR)ID_savesel	},
	{ NM_ITEM	, NM_BARLABEL						      ,  0,0,0,0								},
	{ NM_ITEM	,(strptr)MSG_close            ,"q",0,0,(APTR)ID_quit		},
	{ NM_END,0,												      0,0,0,0								    }
};

extern struct Library	*XEmulatorBase;
static struct List		xem;
static struct Library	*xemsave;
static struct Info		*curxm=0;
static ULONG					curnum=0;
static char						namexm[STRSIZE];
static void (*orderfunc)(void);

/*--------------------------------------------------------------------------*/

static BOOL OpenXemLib(char *temp)
{
	xemsave = XEmulatorBase;
	XemPrefMode();
	if ( (XEmulatorBase = OpenLibrary(GetInfoTag(XM_Lib,curxm),0)) ) {
		if (XEmulatorSetup(&gl_xem_io)) {
			if (temp) {
				XEmulatorPreferences(&gl_xem_io,temp,XEM_PREFS_LOAD);
			} else {
				XEmulatorPreferences(&gl_xem_io,	 0,XEM_PREFS_RESET);
			}
			if (XEmulatorOpenConsole(&gl_xem_io)) {
				return(TRUE);
			}
			XEmulatorCleanup(&gl_xem_io);
		}
		CloseLibrary(XEmulatorBase);
	}
	XemUserMode();
	XEmulatorBase = xemsave;
	return(FALSE);
}

static void CloseXemLib(void)
{
	XEmulatorCloseConsole(&gl_xem_io);
	XEmulatorCleanup(&gl_xem_io);
	CloseLibrary(XEmulatorBase);
	XemUserMode();
	XEmulatorBase = xemsave;
}

static void LoadXemPref(strptr str)
{
long   size;
bptr   fh;
ulong *p;

	if ((size = FileSize(str)) != -1) {
		if (fh = dt_open(str,MODE_READ,8192)) {
			if (p = alloc_pool(size+4)) {
				p[0] = size;
				dt_read(fh,&p[1],size);
				AddInfoTag(XM_Pref,p,curxm);
				free_pool(p);
			}
			dt_close(fh);
			DeleteFile(str);
		}
	}
}

static void XemToCol(void)
{
ULONG	*col;
LONG	i,j;

	if (curxm) {
		col = GetInfoTag(XM_Col,curxm);
		j = *col / 12;
		col++;
		for (i=0; i<j; i++)
		{
			xementry[i].mpe_Red		= *col++;
			xementry[i].mpe_Green	= *col++;
			xementry[i].mpe_Blue	= *col++;
		}
	}
}

static void ColToXem(void)
{
ULONG	hexcol[1+16*3];
LONG	i,j;
ULONG	*col=&hexcol[1];

	if (curxm) {
		get(sl_colors,MUIA_Slider_Level,&j);
		hexcol[0] = j * 12;
		for (i=0; i<j; i++)
		{
			*col++ = xementry[i].mpe_Red;
			*col++ = xementry[i].mpe_Green;
			*col++ = xementry[i].mpe_Blue;
		}
		AddInfoTag(XM_Col,hexcol,curxm);
	}
}

static void Update(void)
{
  if (win_act) {
  	set(win,MUIA_Window_Sleep,gl_req);
	  if (curxm) {
		  nnset(st_name		,MUIA_String_Contents,curxm->node.ln_Name);
  		nnset(st_lib		,MUIA_String_Contents,GetInfoTag(XM_Lib,curxm));
	  	nnset(sl_colors	,MUIA_Slider_Level,*((ULONG *)GetInfoTag(XM_Col,curxm)) / 12);
		  DoMethod(win, MUIM_MultiSet,MUIA_Disabled,curxm->LOCK,
  						 st_name,io_lib,st_lib,sl_colors,bt_pref,bt_reset,pa_col,bt_del, NULL);
	  	XemToCol();
  	} else {
	  	DoMethod(win,MUIM_MultiSet,MUIA_Disabled,TRUE,st_name,io_lib,st_lib,pa_col,sl_colors,bt_pref,bt_reset,bt_del,NULL);
  	}
  }
}

/*	Menut
 */

static void loadset_menu(void)
{
	if (!gl_req) {
		Ask_file(gl_conwindow,&loadset_menu,LOC(MSG_load),"#?.xm",namexm);
	} else {
		if ( Ask_file_done() == REQ_OK ) {
			EmptyInfoList(&xem);
			Xem(DT_LOAD,(ULONG)namexm);
		}
	}
}

static void appset_menu(void)
{
	if (!gl_req) {
		Ask_file(gl_conwindow,&appset_menu,LOC(MSG_append),"#?.xm",namexm);
	} else {
		if ( Ask_file_done() == REQ_OK ) {
			Xem(DT_LOAD,(ULONG)namexm);
		}
	}
}

static void saveset_menu(void)
{
	if (!gl_req) {
		Ask_file(gl_conwindow,&saveset_menu,LOC(MSG_save),"#?.xm",namexm);
	} else {
		if ( Ask_file_done() == REQ_OK ) {
			Xem(DT_SAVE,(ULONG)namexm);
		}
	}
}

static void savesel_menu(void)
{
static struct Info	*xm;

	if (!gl_req) {
		if (xm = curxm) {
			if (Ask_file(gl_conwindow,&savesel_menu,LOC(MSG_save_selected),"#?.xm",namexm)) {
				Xem(DT_LOCK,xm);
			}
		}
	} else {
		switch ( Ask_file_done() )
		{
			case REQ_OK:
				SaveInfo(xm,namexm,XMID);
			case REQ_CANCEL:
				Xem(DT_UNLOCK,xm);
		}
	}
}

/*	Gadgetit
 */

static void xm_click(void)
{
	ColToXem();
	get(lv_xm,MUIA_List_Active,&curnum);
	DoMethod(lv_xm,MUIM_List_GetEntry,curnum,&curxm);
	Update();
}

static void name_click(void)
{
char *name;

	if (curxm) {
		get(st_name,MUIA_String_Contents,&name);
		if ( FindName(&xem,name) != curxm ) {
			if ( FindName(&xem,name) == 0 ) {
				StrRep(&curxm->node.ln_Name,name);
	 			gl_xemsafe = FALSE;
			}
			DoMethod(lv_xm,MUIM_List_Redraw,curnum);
			DoMethod(lv_xm,MUIM_List_Sort,0);
			MUI_selnode(lv_xm,curxm);
      SortList(&xem);
			orderfunc();
		}
	}
}

static void libpop_click(void)
{
static char					name[STRSIZE];
static struct Info	*xm;

	if (!gl_req) {
		if (xm = curxm) {
			strcpy(name,(char *)GetInfoTag(XM_Lib,xm));
			if (Ask_file(gl_conwindow,&libpop_click,LOC(MSG_select_xem_library),"xem#?.library",name)) {
				Xem(DT_LOCK,xm);
			}
		}
	} else {
		switch ( Ask_file_done() )
		{
			case REQ_OK:
				AddInfoTag(XM_Lib,(ULONG)name,xm);
				gl_xemsafe = FALSE;
			case REQ_CANCEL:
				Xem(DT_UNLOCK,xm);
		}
	}
}

static void lib_click(void)
{
char	*name;

	if (curxm) {
		get(st_lib,MUIA_String_Contents,&name);
		AddInfoTag(XM_Lib,(ULONG)name,curxm);
		gl_xemsafe = FALSE;
	}
}

static void pref_click(void)
{
char	temp[STRSIZE];

	if (curxm) {
		TempName(temp);
		SaveDatTag(XM_Pref,&curxm->tags,temp);
		if (OpenXemLib(temp)) {
			XEmulatorOptions(&gl_xem_io);
			XEmulatorPreferences(&gl_xem_io,temp,XEM_PREFS_SAVE);
			LoadXemPref(temp);
			CloseXemLib();
		}
	}
}

static void reset_click(void)
{
char	temp[STRSIZE];

	if (curxm) {
		if (OpenXemLib(0)) {
			XEmulatorPreferences(&gl_xem_io,temp,XEM_PREFS_SAVE);
			LoadXemPref(temp);
			CloseXemLib();
		}
	}
}

static void add_click(void)
{
ULONG	xmcol[]	= { XMCol };
ULONG	xmpref	= 0;
struct Info	*xm;

	if ( FindName(&xem,LOC(MSG_new_entry)) == 0 ) {
		if ( xm = CreateInfo() ) {
			if ( StrRep(&xm->node.ln_Name,LOC(MSG_new_entry)) ) {
				AddInfoTags(xm,
										XM_Lib  ,XMLib  ,
										XM_Col  ,&xmcol ,
										XM_Pref ,&xmpref, TAG_END);
        AddNode_sort(&xem,(struct Node *) xm);
				DoMethod(lv_xm,MUIM_List_Insert,&xm,1,MUIV_List_Insert_Sorted);
				MUI_selnode(lv_xm,xm);
				gl_xemsafe = FALSE;
				orderfunc();
			} else {
				DeleteInfo(xm);
			}
		}
	}
}

static void del_click(void)
{
	if (curxm) {
		if (!curxm->LOCK) {
			RemoveInfo(curxm); curxm = 0;
			DoMethod(lv_xm,MUIM_List_Remove,curnum);
			gl_xemsafe = FALSE;
			orderfunc();
		}
	}
}

/*	Interface pääohjelmaan
 *
 */

ulong Xem(ULONG tag, ULONG data)
{
static BOOL ocall = FALSE; static ULONG oc;
LONG	b;
struct Info	*xm;

	switch ( tag )
	{
		case DT_CLOSE:
			if (win_act) {
        set(win,MUIA_Window_Open,FALSE);
        if (CUR(PB_Dispose)) {
          DoMethod(gl_app,OM_REMMEMBER,win);
          MUI_DisposeObject(win);
          win_act = FALSE;
          curxm   = 0;
        }
      }
			break;
		case DT_OPEN:
      if (!win_act) {
  			win = WindowObject,
  			MUIA_Window_ID		,MAKE_ID('X','M','W','I'),
  			MUIA_Window_Title	,LOC(MSG_xem_configuration),
  			MUIA_Window_Menu	,&menu,
  			MUIA_HelpNode			,"help_xm",
  			MUIA_Window_Width	,MUIV_Window_Width_Visible(100),
  				WindowContents, VGroup,
  					Child,HGroup,
  						Child, VGroup,
  							Child, VGroup,
  								GroupSpacing(0),
  								Child, lv_xm = ListView(),
  								Child, st_name = String(STRSIZE),
  							End,
  							Child, HGroup,
  								Child, Label2L(LOC(MSG_xem_library)),
    							Child, HGroup,
    								GroupSpacing(0),
  	  							Child, io_lib = PopFile(),
  		  						Child, st_lib = String(STRSIZE),
                  End,
  							End,
  						End,
  						Child, VGroup,
  							GroupFrameT(LOC(MSG_settings)),
  							Child, pa_col = PaletteObject,
  								MUIA_Palette_Entries,xementry,
  							End,
  							Child, HGroup,
  								Child, Label1L(LOC(MSG_colors_to_use)), Child, sl_colors = Slider(1,16),
  							End,
  							Child, HGroup,
  								Child, bt_pref	= MakeButton(LOC(MSG_xm_change_options)),
  								Child, bt_reset	= MakeButton(LOC(MSG_xm_reset)),
  							End,
  						End,
  					End,
  	
  					Child, HGroup,
  						GroupSpacing(4),
  						Child, bt_add = MakeButton(LOC(MSG_key_add)),
  						Child, bt_del = MakeButton(LOC(MSG_key_delete)),
  					End,
  				End,
  			End;
        if (win) {
          DoMethod(gl_app,OM_ADDMEMBER,win);
    			DoMethod(win,MUIM_Window_SetCycleChain,
    							lv_xm,st_name,io_lib,st_lib,pa_col,bt_pref,bt_reset,
    							bt_add,bt_del,NULL);
    			DoMethod(win			,MUIM_Notify,MUIA_Window_CloseRequest,TRUE, gl_app,2,MUIM_Application_ReturnID,ID_quit);
    			DoMethod(lv_xm		,MUIM_Notify,MUIA_List_Active,MUIV_EveryTime, gl_app,2,MUIM_Application_ReturnID,ID_xm);
    			DoMethod(lv_xm		,MUIM_Notify,MUIA_Listview_DoubleClick,TRUE, win,3,MUIM_Set,MUIA_Window_ActiveObject,st_name);
    			DoMethod(st_name	,MUIM_Notify,MUIA_String_Acknowledge,MUIV_EveryTime, gl_app,2,MUIM_Application_ReturnID,ID_name);
    			DoMethod(io_lib		,MUIM_Notify,MUIA_Pressed,FALSE, gl_app,2,MUIM_Application_ReturnID,ID_libpop);
    			DoMethod(st_lib		,MUIM_Notify,MUIA_String_Contents,MUIV_EveryTime, gl_app,2,MUIM_Application_ReturnID,ID_lib);
    			MultiIDNotify(MUIA_Pressed,FALSE,
    										bt_pref	,ID_pref	,
    										bt_reset,ID_reset	,
    										bt_add	,ID_add		,
    										bt_del	,ID_del		, TAG_END);
    			InfoListToMui(&xem,lv_xm,MUIV_List_Insert_Sorted);
          win_act = TRUE;
        }
      }

      if (win_act) {
  			Update();
  			set(win,MUIA_Window_Screen,gl_conscreen);
	  		set(win,MUIA_Window_Open,TRUE);
		  	set(win,MUIA_Window_Sleep,gl_req);
			  set(win,MUIA_Window_Activate,TRUE);
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
		case DT_LINK:
			orderfunc = data;
			break;
		case DT_FIND:
			return(FindName(&xem,(char *)data));
			break;
		case DT_LOAD:
			LoadInfoList(&xem,(char *)data,XMID);
			InfoListToMui(&xem,lv_xm,MUIV_List_Insert_Sorted);
      SortList(&xem);
			curxm      = 0;
      gl_xemsafe = TRUE;
			Xem(DT_UPDATE,0);
			orderfunc();
			break;
		case DT_SAVE:
			SaveInfoList(&xem,(char *)data,XMID);
			gl_xemsafe = TRUE;
			break;
		case DT_INIT:
			NewList(&xem);
      LocalizeMenu(menu,sizeof(menu)/sizeof(struct NewMenu));
      return(TRUE);
			break;
		case DT_HANDLE:
      if (win_act) {
  			switch (data)
	  		{
		  		case ID_xm:				xm_click();				break;
			  	case ID_name:			name_click();			break;
  				case ID_libpop:		libpop_click();		break;
	  			case ID_lib:			lib_click();			break;
		  		case ID_pref:			pref_click();			break;
			  	case ID_reset:		reset_click();		break;
  				case ID_add:			add_click();			break;
	  			case ID_del:			del_click();			break;
		  		case ID_loadset:	loadset_menu();		break;
			  	case ID_saveset:	saveset_menu();		break;
  				case ID_appset:		appset_menu();		break;
	  			case ID_savesel:	savesel_menu();		break;
		  		case ID_quit:			ColToXem(); Xem(DT_CLOSE,0); break;
			  }
  			break;
      }
#ifdef DEBUG
  printf("Xem: unexpected MUI ID\n");
#endif
      break;
		case DT_UPDATE:
      if (win_act) {
  			get(win,MUIA_Window_Open,&b);
	  		if (b) Update();
      }
			break;
		case DT_GET:
  		return(FindNode_num(&xem,data));
			break;
		case DT_LOCK:
			((struct Info *)data)->LOCK = TRUE; 
			if ((struct Info *)data == curxm) Xem(DT_UPDATE,0);
			break;
		case DT_UNLOCK:
			((struct Info *)data)->LOCK = FALSE; 
			if ((struct Info *)data == curxm) Xem(DT_UPDATE,0);
			break;
		case DT_COUNT:
      return(NodeCount(&xem));
			break;
	}
}
