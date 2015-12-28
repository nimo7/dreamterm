#include "global.h"
#include "loc_strings.h"

/*
char ver_uplist[]="1.22 (Aug 28 1995)";
*/

enum ID { ID_add = FI_First,
          ID_cps    ,
          ID_cpsack ,
          ID_del    ,
          ID_delall ,
          ID_upload ,
          ID_quit     };

static bool win_act = FALSE;
static aptr win,lv_fi,tx_combsize,it_cps,bt_add,bt_del,bt_delall,bt_upload;

static char str_size[12],str_time[10];

/***----------------------------------------------------------------------***/
/*** support                                                              ***/
/***----------------------------------------------------------------------***/

/* Hooks for phonebook listview */

static __saveds __asm long func3(REG __a0 struct Hook	*hook,
													    	 REG __a2 APTR				nil,
														     REG __a1 struct Info	*fi)
{
	return(fi);
}

static SAVEDS ASM LONG func2(REG __a0 struct Hook	*hook,
														 REG __a2 struct Info	*fi2,
														 REG __a1 struct Info	*fi1)
{
	return(stricmp(fi1->ln_Name,fi2->ln_Name));
}

static SAVEDS ASM LONG func1(REG __a0 struct Hook	*hook,
														 REG __a2 char				**array,
														 REG __a1 struct Info	*fi)
{
ULONG	size,cps;

	get(it_cps,MUIA_String_Integer,&cps);
	if (cps < 30) cps = 30;
	stci_d(str_size,size=GetInfoTag(FI_Size,fi));
	TimeText(size/cps,str_time); str_time[8] = 0;
	*array++ = fi->ln_Name;
	*array++ = "                                                                 ";
	*array++ = str_size;
	*array	 = str_time;
	return(0);
}

static const struct Hook fiDisplay  = { {NULL, NULL}, (void *)func1, NULL, NULL };
static const struct Hook fiCompare  = { {NULL, NULL}, (void *)func2, NULL, NULL };
static const struct Hook fiConstruct= { {NULL, NULL}, (void *)func3, NULL, NULL};

static void Update(void)
{
LONG	i,cps,combsize=0;
char	s[64];
struct Info	*fi = GetHead(&gl_fileid);

  if (win_act) {
  	while (fi)
	  {
		  combsize += GetInfoTag(FI_Size,fi);
  		fi = GetSucc(fi);
	  }
  	get(it_cps,MUIA_String_Integer,&cps);
	  if (cps < 30) cps = 30;
    sprintf(s,LOC(MSG_x_x_mb_time),combsize/1048576, (combsize % 1048576) / 10486);
	  TimeText(combsize/cps,&s[i = strlen(s)]);
  	s[i+8] = 0x00;
	  set(tx_combsize,MUIA_Text_Contents,s);
  }
}

/*	Gadgetit
 */

static void add_click(void)
{
static char		dir[STRSIZE];
static STRPTR	fl;
char		s[STRSIZE];
LONG		size;
STRPTR	fl2;
struct Info	*fi;

	if (!gl_req) {
		Ask_files(gl_conwindow,&add_click,LOC(MSG_select_files_to_add),"#?",dir,&fl);
	} else {
		if ( Ask_files_done() == REQ_OK ) {
			if ( (fl2 = fl) ) {
				while ( *fl2 != 0x00 )
				{
					strcpy(s,dir);
					AddPart(s,fl2,STRSIZE-1);
					if ( FindName(&gl_fileid,s) == 0 ) {
						if ( (size = FileSize(s)) != -1 ) {
							if (fi = CreateInfo()) {
								StrRep(&fi->ln_Name,s);
								AddInfoTag(FI_Size,size,fi);
								AddTail(&gl_fileid,(struct Node *)fi);
							}
						}
					}
					fl2 += (strlen(fl2) + 1);
				}
				InfoListToMui(&gl_fileid,lv_fi,MUIV_List_Insert_Bottom);
				FreeVec(fl);
				Update();
			}
		}
	}
}

static void del_click(void)
{
struct Info	*fi;
ULONG				num;

	if (!gl_xferon) {
		get(lv_fi,MUIA_List_Active,&num);
		DoMethod(lv_fi,MUIM_List_GetEntry,num,&fi);
		if (fi != 0) {
			RemoveInfo(fi);
			DoMethod(lv_fi,MUIM_List_Remove,num);
		}
		Update();
	}
}

static void delall_click(void)
{
	if (!gl_xferon) {
		EmptyInfoList(&gl_fileid);
		DoMethod(lv_fi,MUIM_List_Clear);
		Update();
	}
}

static void upload_click(void)
{
	Transfer(FT_UPLOADID,(ULONG)gl_curxp);
}

/*	Interface p‰‰ohjelmaan
 *
 */

ulong Fileid(ULONG tag, ULONG data)
{
static BOOL ocall = FALSE; static ULONG oc;

  switch (tag)
  {
    case  DT_CLOSE:
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
      if (!win_act) {
        win = WindowObject,
        MUIA_Window_ID		,MAKE_ID('F','I','W','I'),
        MUIA_Window_Title	,LOC(MSG_upload_list),
        MUIA_HelpNode			,"help_fi",
        MUIA_Window_Width	,MUIV_Window_Width_Visible(25),
          WindowContents, VGroup,
  					Child, lv_fi = ListviewObject,
  						MUIA_Listview_List, ListObject,
  							InputListFrame,
  							MUIA_List_Format				,"P=\33l,MIW=1,P=\33r,p=\33l",
  							MUIA_List_ConstructHook	,&fiConstruct,
  							MUIA_List_CompareHook		,&fiCompare,
  							MUIA_List_DisplayHook		,&fiDisplay,
  						End,
  					End,
  				Child, ColGroup(2),
  						GroupFrame,
  						Child, LabelL (LOC(MSG_combined_size))	,Child, tx_combsize = MyText(),
  						Child, Label2L(LOC(MSG_estimated_cps))	,Child, it_cps			= Int(),
  					End,
  					Child, HGroup,
  						GroupSpacing(4),
  						Child, bt_add			= MakeButton(LOC(MSG_key_add)),
  						Child, bt_del			= MakeButton(LOC(MSG_key_delete)),
  						Child, bt_delall	= MakeButton(LOC(MSG_delete_all)),
  						Child, bt_upload	= MakeButton(LOC(MSG_fi_upload)),
  					End,
  				End,
  			End;
        if (win) {
          DoMethod(gl_app,OM_ADDMEMBER,win);

    			DoMethod(win		,MUIM_Window_SetCycleChain,	lv_fi,it_cps,bt_add,bt_del,bt_delall,bt_upload,NULL);
          DoMethod(win    ,MUIM_Notify,MUIA_Window_CloseRequest,TRUE, gl_app,2,MUIM_Application_ReturnID,ID_quit);
    			DoMethod(it_cps	,MUIM_Notify,MUIA_String_Contents		,MUIV_EveryTime, gl_app,2,MUIM_Application_ReturnID,ID_cps);
    			DoMethod(it_cps	,MUIM_Notify,MUIA_String_Acknowledge,MUIV_EveryTime, gl_app,2,MUIM_Application_ReturnID,ID_cpsack);
    			MultiIDNotify(MUIA_Pressed,FALSE,
    									bt_add		,ID_add			,
    									bt_del		,ID_del			,
    									bt_delall	,ID_delall	,
    									bt_upload	,ID_upload	, TAG_END);
  				InfoListToMui(&gl_fileid,lv_fi,MUIV_List_Insert_Bottom);
          win_act = TRUE;
        }
      }
      if (win_act) {
  			set(bt_add,MUIA_Disabled,gl_req);
  			set(win,MUIA_Window_Screen,gl_conscreen);
  			set(win,MUIA_Window_Open,TRUE);
  			set(win,MUIA_Window_Activate,TRUE);
      }
			break;
		case DT_OC:
      if (win_act) {
  			if (ocall) {
			  	set(win,MUIA_Window_Screen,gl_conscreen);
		  		set(win,MUIA_Window_Open,oc);
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
      return(TRUE);
			break;
		case DT_HANDLE:
      if (win_act) {
  			switch (data)
	  		{
		  		case ID_cps:    Update();       break;
			  	case ID_cpsack: DoMethod(lv_fi,MUIM_List_Redraw,MUIV_List_Redraw_All); break;
  				case ID_add:		add_click();		break;
	  			case ID_del:		del_click();		break;
		  		case ID_delall:	delall_click();	break;
          case ID_upload: upload_click();	break;
          case ID_quit:   Fileid(DT_CLOSE,0); break;
  			}
        break;
      }
#ifdef DEBUG
  printf("Uplist: unexpected MUI ID\n");
#endif
      break;
    case DT_UPDATE:
      if (win_act) {
        set(bt_add,MUIA_Disabled,gl_req);
      }
      break;
  }
}
