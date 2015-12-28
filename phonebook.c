#include "global.h"
#include "loc_strings.h"

/*
char ver_phonebook[]="1.46 (Dec 07 1995)";
*/

enum ID { ID_pb = PB_First,
          ID_pbsel    ,ID_dial     ,ID_edit     ,ID_addentry ,ID_addclone ,
          ID_clear    ,ID_delete   ,ID_drop     ,ID_reset    ,ID_freeze   ,
          ID_list     ,ID_showlist ,ID_sort     ,ID_sortmode ,
          ID_loadset  ,ID_saveset  ,ID_appset   ,ID_savesel  ,ID_droplist ,
          ID_showedit ,ID_quit     };

static bool win_act = FALSE;
static aptr lv_pb   = 0;
static aptr win,lv_format,bt_showlist,cy_group,cy_sort,cy_sortmode,bt_dial,
            bt_addentry,bt_addclone,bt_edit,bt_clear,bt_delete,bt_drop,bt_freeze,
            bt_reset;

static struct NewMenu menu[] =
{
  { NM_TITLE ,(strptr)MSG_phonebook       ,  0,0,0,0                 },
  { NM_ITEM  ,(strptr)MSG_load            ,"o",0,0,(APTR)ID_loadset  },
  { NM_ITEM  ,(strptr)MSG_save            ,"s",0,0,(APTR)ID_saveset  },
  { NM_ITEM  ,(strptr)MSG_append          ,"a",0,0,(APTR)ID_appset   },
  { NM_ITEM  ,(strptr)MSG_save_selected   ,"c",0,0,(APTR)ID_savesel  },
  { NM_ITEM  ,(strptr)MSG_pb_edit_droplist,"e",0,0,(APTR)ID_droplist },
  { NM_ITEM  ,(strptr)MSG_pb_edit_showlist,"d",0,0,(APTR)ID_showedit },
  { NM_ITEM  , NM_BARLABEL                ,  0,0,0,0                 },
  { NM_ITEM  ,(strptr)MSG_close           ,"q",0,0,(APTR)ID_quit     },
  { NM_END,0,0,0,0,0 }
};

static struct Info *curpb       =0;    /* selected entry        */
static ulong        curnum      =0;    /* selected number       */
static long         selcount    =0;    /* how many selected     */
static struct Info *cursl       =0;    /* current showlist      */
static strptr       format      =0;    /* current format string */
static ulong        sort        =1;    /* current sort          */
static ulong        sortmode    =0;    /* sort mode             */
static strptr       arrays[64];        /* array strings         */
static strptr      *t_sort      =0;    /* Sort by strings       */
static char        *t_sortmode[]= { (strptr)MSG_pb_ascending,
                                    (strptr)MSG_pb_descending,
                                    0 };
static char         namepb[STRSIZE];
static char         namecr[STRSIZE];
static char         key   [STRSIZE];

/***----------------------------------------------------------------------***/
/*** support                                                              ***/
/***----------------------------------------------------------------------***/

static SAVEDS ASM long func2(REG __a0 struct Hook *hook,
                             REG __a2 struct Info *pb2,
                             REG __a1 struct Info *pb1)
{
long  sort_val,p1,p2;
ulong tag;

  if (cursl) {
    if (sort) {
      tag      = GetDat(FindTag_info(SL_tag,cursl),sort-1);
      sort_val = CmpEntryTag(tag,GetEntryTag(tag,pb1),pb2);
    } else {
      if (pb1->SELECT OR pb2->SELECT) {
        p1 = (pb1->SELECT) ? pb1->store : (sortmode) ? 0 : 0x7fffffff;
        p2 = (pb2->SELECT) ? pb2->store : (sortmode) ? 0 : 0x7fffffff;
        if (p1 <  p2) sort_val = -1;
        if (p1 == p2) sort_val =  0;
        if (p1 >  p2) sort_val =  1;
      } else {
        sort_val = strcmp(GetEntryTag(PB_Name,pb1),GetEntryTag(PB_Name,pb2));
      }
    }
  } else {
    switch (sort)
    {
      case 0:
        if ( (pb1->SELECT) OR (pb2->SELECT) ) {
          p2 = (pb1->SELECT) ? pb1->store : 0xffffffff;
          p1 = (pb2->SELECT) ? pb2->store : 0xffffffff;
          if (p1 >  p2) sort_val = -1;
          if (p1 == p2) sort_val =  0;
          if (p1 <  p2) sort_val =  1;
          break;
        }
      case 1:
        sort_val = strcmp(GetEntryTag(PB_Name,pb1),GetEntryTag(PB_Name,pb2));
        break;
    }
  }
  if (sortmode == 0) {
    if (sort_val <   0) return(-1);
    if (sort_val ==  0) return( 0);
    if (sort_val >   0) return( 1);
  } else {
    if (sort_val <   0) return( 1);
    if (sort_val ==  0) return( 0);
    if (sort_val >   0) return(-1);
  }
}

static SAVEDS ASM long func1(REG __a0 struct Hook *hook,
                             REG __a2 char        **array,
														 REG __a1 struct Info *pb)
{
ulong          i,count,freeze;
strptr         str;
struct TagItem *ti;
static char sel[14];

  for (i = 0; i < 64; i++) { free_pool(arrays[i]); arrays[i] = 0; }
  freeze = ( GetEntryTag(PB_Freeze,pb) < GetSec() ) ? 0 : GetEntryTag(PB_Freeze,pb);
  sel[0] = 0x00;
  if (pb->store) stcl_d(sel,pb->store);
  *array++ = (freeze) ? "F" : sel;

  if (cursl) {
    ti    = FindTag_info(SL_tag,cursl);
    count = CountDat(ti);
    for (i = 0; i < count; i++)
    {
      str = TagData2Str(GetDat(ti,i),GetEntryTag(GetDat(ti,i),pb));
      if (arrays[i] = alloc_pool(strlen(str)+1)) {
        strcpy(arrays[i],str);
        *array++ = arrays[i];
      } else {
        *array++ = "no mem";
      }
    }
  } else {
    *array++ = (char *)GetEntryTag(PB_Name,pb);
  }
  return(0);
}

static const struct Hook pbDisplay  = { {NULL, NULL}, (void *)func1, NULL, NULL };
static const struct Hook pbCompare  = { {NULL, NULL}, (void *)func2, NULL, NULL };

/* p‰ivit‰ listan SELECT numerot oikein */
static void FixNumber(LONG num)
{
struct Info *pb=GetHead(&gl_phonebook);

  while (pb)
  {
    if (pb->store > num) pb->store--;
    pb = GetSucc(pb);
  }
}

/* update normal things */
static void Update(void)
{
  if (win_act) {
    set(win     ,MUIA_Window_Sleep,gl_req);
    set(bt_dial ,MUIA_Disabled    ,gl_onlinepb);
    if (curpb) {
      DoMethod(win,MUIM_MultiSet,MUIA_Disabled,curpb->LOCK ,bt_edit,bt_delete,bt_drop,bt_freeze,bt_reset,NULL);
    }
  }
}

static ulong str_do(char *p2, char *p1)
{
char *save=p2;
long val,size;

  while (*p1 != 0)
  {
    if (*p1 == '\\') {
      p1++;
      size = stcd_l(p1,&val);
      if (size) {
        p1 += size;
        *p2 = val;
        p2++;
      }
    } else {
      *p2 = *p1;
      p1++;
      p2++;
    }
  }
  return(p2-save);
}

/* update sort/sortmode, only when starting or when showlist changes */
static void Update_show(void)
{
char *p,*new_format=0;
long i,count,size;
struct TagItem *ti,*ti2;
strptr *new_t_sort=0;

  if (cursl) {
    ti    = FindTag_info(SL_pos,cursl);
    ti2   = FindTag_info(SL_tag,cursl);
    count = CountDat(ti);
    size  = 0;
    if (count) {
      for (i = 0; i < count; i++)
      {
        size += strlen(GetDat(ti,i))+1;
      }
      new_t_sort     = alloc_pool((count+2)*4);
      new_format = p = alloc_pool(size+strlen("P=\33r,"));
      if (new_format AND new_t_sort) {
        strcpy(p,"P=\33r,");
        p += strlen("P=\33r,");
        new_t_sort[0] = LOC(MSG_pb_selected);
        for (i = 0; i < count; i++)
        {
          p    += str_do(p,GetDat(ti,i));
          *p++  = ',';
          new_t_sort[i+1] = TagName(GetDat(ti2,i));
        }
        *--p = 0;
        new_t_sort[i+1] = 0;
        goto disp;
      }
    }
  }
  /* no showlist, make internal */
  if (cursl) {
    Show(DT_UNLOCK,cursl);
    cursl = 0;
  }
  free_pool(new_format);
  free_pool(new_t_sort);
  new_format = alloc_force(strlen("P=\33r,")+1);
  new_t_sort = alloc_force(12);
  count = 1;
  strcpy(new_format,"P=\33r,");
  new_t_sort[0] = LOC(MSG_pb_selected);
  new_t_sort[1] = LOC(MSG_name);
  new_t_sort[2] = 0;

disp:

  free_pool(format); /* make sure no same adr for string */
  free_pool(t_sort);
  format = new_format;
  t_sort = new_t_sort;
  if (sort > count) sort = 1;
  nnset(lv_format  ,MUIA_List_Format ,format);

  DoMethod(cy_group,MUIM_Group_InitChange);
  DoMethod(cy_group,OM_REMMEMBER,cy_sort);
  if (cy_sort = Cycle(t_sort)) {
    nnset   (cy_sort,MUIA_Cycle_Active,sort);
    DoMethod(cy_group,OM_ADDMEMBER,cy_sort);
    DoMethod(cy_group,MUIM_Group_ExitChange);
    DoMethod(cy_sort,MUIM_Notify,MUIA_Cycle_Active,MUIV_EveryTime, gl_app,2,MUIM_Application_ReturnID,ID_sort);
    nnset   (cy_sort,MUIA_CycleChain,1);
  }
}

/***----------------------------------------------------------------------***/
/*** menut                                                                ***/
/***----------------------------------------------------------------------***/

static void loadset_menu(void)
{
  if (!gl_req) {
    if (!gl_phonesafe) {
      if (! MUI_Request(gl_app,win_fake, 0,
                        LOC(MSG_note),
                        LOC(MSG_mui_ok_cancel),
                        LOC(MSG_current_phonebook_is_not_saved) ,0) ) return;
		}
		Ask_file(gl_conwindow,&loadset_menu,LOC(MSG_pb_load_new_phonebook),"#?.pb",namepb);
	} else {
		if ( Ask_file_done() == REQ_OK ) {
			selcount = 0;
			EmptyInfoList(&gl_phonebook);
			Phone(DT_LOAD,(ULONG)namepb);
		}
	}
}

static void appset_menu(void)
{
	if (!gl_req) {
		Ask_file(gl_conwindow,&appset_menu,LOC(MSG_pb_append_new_phonebook),"#?.pb",namepb);
	} else {
		if ( Ask_file_done() == REQ_OK ) {
			Phone(DT_LOAD,namepb);
		}
	}
}

static void saveset_menu(void)
{
	if (!gl_req) {
		Ask_file(gl_conwindow,&saveset_menu,LOC(MSG_pb_save_phonebook),"#?.pb",namepb);
	} else {
		if ( Ask_file_done() == REQ_OK ) {
			Phone(DT_SAVE,namepb);
		}
	}
}

static void savesel_menu(void)
{
static struct Info	*pb;

	if (!gl_req) {
		if (pb = curpb) {
			if (Ask_file(gl_conwindow,&savesel_menu,LOC(MSG_save_selected),"#?.pb",namepb)) {
				Phone(DT_LOCK,pb);
			}
		}
	} else {
		switch ( Ask_file_done() )
		{
			case REQ_OK:
				SaveInfo(pb,namepb,PBID);
			case REQ_CANCEL:
				Phone(DT_UNLOCK,pb);
		}
	}
}

static void droplist_menu(void)
{
  Drop(DT_OPEN,0);
}

static void showedit_menu(void)
{
  Show(DT_OPEN,0);
}

/***----------------------------------------------------------------------***/
/*** gadgetit                                                             ***/
/***----------------------------------------------------------------------***/

static void pb_click(void)
{
  get(lv_pb,MUIA_List_Active,&curnum);
  DoMethod(lv_pb,MUIM_List_GetEntry,curnum,&curpb);
  Update();
}

static void pbsel_click(void)
{
  get(lv_pb,MUIA_List_Active,&curnum);
  DoMethod(lv_pb,MUIM_List_GetEntry,curnum,&curpb);
  if (curpb) {
    if ( GetEntryTag(PB_Freeze,curpb) < GetSec() ) {
      if (curpb->SELECT == TRUE ) {
        selcount--; curpb->SELECT = FALSE; 
        FixNumber(curpb->store); curpb->store = 0;
      } else {
        selcount++; curpb->SELECT = TRUE; curpb->store = selcount;
      }
      if (sort == 0) { /* to speed up things */
      	nnset   (lv_pb,MUIA_List_Quiet,TRUE);
        DoMethod(lv_pb,MUIM_List_Sort,0);
        DoMethod(lv_pb,MUIM_List_Redraw,MUIV_List_Redraw_All);
        MUI_selnode(lv_pb,curpb);
      	nnset   (lv_pb,MUIA_List_Quiet,FALSE);
      } else {
        DoMethod(lv_pb,MUIM_List_Redraw,MUIV_List_Redraw_All);
      }
    }
  }
}

static void dial_click(void)
{
  if ( (selcount) AND (!gl_onlinepb) ) {
    Dialer(DI_DIAL,0);
  }
}

static void edit_click(void)
{
  if (curpb) {
    if (!curpb->LOCK) {
      Entry(PE_EDIT,(ULONG)curpb);
    }
  }
}

static void addentry_click(void)
{
  Entry(PE_NEW,0);
}

static void addclone_click(void)
{
  if (curpb) {
    Entry(PE_CLONE,(ulong)curpb);
  }
}

static void clear_click(void)
{
struct Info *pb=GetHead(&gl_phonebook);

  while(pb)
  {
    if (pb->SELECT) {
      selcount--;
      pb->SELECT = FALSE; 
      FixNumber(pb->store);
      pb->store = 0;
    }
    pb = GetSucc(pb);
  }
	nnset   (lv_pb,MUIA_List_Quiet,TRUE);
  DoMethod(lv_pb,MUIM_List_Sort,0);
  DoMethod(lv_pb,MUIM_List_Redraw,MUIV_List_Redraw_All);
	nnset   (lv_pb,MUIA_List_Quiet,FALSE);
}

static void delete_click(void)
{
  if (curpb) {
    if (!curpb->LOCK) {
      if (curpb->SELECT) {
        if (curpb->store == selcount) {
          selcount--;
        } else {
          selcount--; FixNumber(curpb->store);
        }
      }
      RemoveInfo(curpb); curpb = 0;
      DoMethod(lv_pb,MUIM_List_Remove,curnum);
      gl_phonesafe = FALSE;
    }
  }
}

static void drop_click(void)
{
  if (curpb) {
    if (!curpb->LOCK) {
      if (curpb->SELECT) {
        if (curpb->store == selcount) {
          selcount--;
        } else {
          selcount--; FixNumber(curpb->store);
        }
      }
      Drop(DL_ADD,GetEntryTag(PB_Phone,curpb));
      RemoveInfo(curpb); curpb = 0;
      DoMethod(lv_pb,MUIM_List_Remove,curnum);
      gl_phonesafe = FALSE;
    }
  }
}

static void reset_click(void)
{
  if (curpb) {
  	if (!curpb->LOCK) {
    	DelInfoTags(curpb,
    							PB_CallLast		,PB_CallCost	,PB_CallTime	,
    							PB_CallCount	,PB_CallTxd		,PB_CallRxd		, TAG_END);
    	DoMethod(lv_pb,MUIM_List_Redraw,MUIV_List_Redraw_All);
    	gl_phonesafe = FALSE;
    }
  }
}

static void freeze_click(void)
{
static ULONG	val;
static struct Info	*pb;

	if (!gl_req) {
		if (pb = curpb) {
			if (!pb->LOCK) {
				if (Ask_slider(&freeze_click,LOC(MSG_pb_how_many_hours_to_freeze),1,720,&val)) {
					Phone(DT_LOCK,pb);
				}
			}
		}
	} else {
		switch ( Ask_slider_done() )
		{
			case REQ_OK:
				if ( GetEntryTag(PB_Freeze,curpb) < (val*60*60+GetSec()) ) {
					AddInfoTag(PB_Freeze,val*60*60+GetSec(),pb);
				}
				if (pb->SELECT) {
					selcount--;
					pb->SELECT = FALSE; 
					FixNumber(pb->store);
					pb->store = 0;
				}
				DoMethod(lv_pb,MUIM_List_Redraw,MUIV_List_Redraw_All);
				gl_phonesafe = FALSE;
			case REQ_CANCEL:
				Phone(DT_UNLOCK,pb);
		}
	}
}

static void showlist_click(void)
{
ulong       i,count;
struct Info *sl;
static long   num;
static strptr *list;

  if (!gl_req) {
    if (count = Show(DT_COUNT,0)) {
      if (list = alloc_pool((count+1)*4)) {
        for (i=0; i < count; i++)
        {
          sl = Show(DT_GET,i);
          list[i] = sl->ln_Name;
        }
        list[i] = 0;
        if (!Ask_list(&showlist_click,LOC(MSG_pb_select_showlist),list,&num)) {
          free_pool(list);
        }
      }
    }
  } else {
    switch ( Ask_list_done() )
    {
      case REQ_OK:
        nnset   (lv_pb,MUIA_List_Quiet,TRUE);
        if (cursl) Show(DT_UNLOCK,cursl);
        cursl = Show(DT_GET,num);
        if (cursl) Show(DT_LOCK,cursl);
        Update_show();
        DoMethod(lv_pb,MUIM_List_Sort,0);
        nnset   (lv_pb,MUIA_List_Quiet,FALSE);
      case REQ_CANCEL:
        free_pool(list);
    }
  }
}

static void sort_click(void)
{
  get(cy_sort,MUIA_Cycle_Active,&sort);
  DoMethod(lv_pb,MUIM_List_Sort,0);
}

static void sortmode_click(void)
{
  get(cy_sortmode,MUIA_Cycle_Active,&sortmode);
  DoMethod(lv_pb,MUIM_List_Sort,0);
}

/*  Interface p‰‰ohjelmaan
 *
 *  Commands:
 *
 *  PB_DESELECT [entry] - Poista select
 *  PB_ADDENTRY [entry] - Lis‰‰ listaan
 *  PB_REDRAW   [entry] - P‰ivit‰ entry (0 = p‰ivit‰ kaikki)
 */

static void req_load(void)
{
char name[STRSIZE];

  if ( Ask_string_done() == REQ_OK ) {
    if (strlen(CUR(PB_CryptKey)) == 0) AddInfoTag(PB_CryptKey,key,gl_curbbs);
		TempName(name);
		if ( Crypt(CR_DECRYPT,namecr,name) ) {
			if ( LoadInfoList(&gl_phonebook,name,PBID) ) {
				gl_phonesafe = TRUE;
			} else {
				Warning(LOC(MSG_pb_cannot_load_complete_phonebook));
			}
			InfoListToMui(&gl_phonebook,lv_pb,MUIV_List_Insert_Sorted);
		} else {
			Warning(LOC(MSG_pb_cannot_decrypt_phonebook));
		}
		Crypt(CR_DELETE,name,0);
	}
}

ulong Phone(ULONG tag, ULONG data)
{
static bool ocall = FALSE; static ulong oc;
char        name[STRSIZE];
BPTR        fh;
ulong       i,id;
struct Info *pb;

  switch (tag)
  {
    case DT_INIT:
      for (i = 0; i < 64; i++)
      {
        arrays[i] = 0;
      }
      LocalizeMenu(menu,sizeof(menu)/sizeof(struct NewMenu));
      LocalizeList(t_sortmode,2);
      return(TRUE);
      break;
    case DT_CLOSE:
      Drop(DT_CLOSE,0);
      Show(DT_CLOSE,0);
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
      if (gl_dial) return(0);
      if (!win_act) {
        win = WindowObject,
        MUIA_Window_ID    ,MAKE_ID('P','B','W','I'),
        MUIA_Window_Title ,LOC(MSG_phonebook),
        MUIA_Window_Menu  ,&menu,
        MUIA_HelpNode     ,"help_pb",
        MUIA_Window_Width ,MUIV_Window_Width_Visible(80),
        MUIA_Window_Height,MUIV_Window_Height_Visible(100),
          WindowContents, VGroup,
            Child, lv_pb = ListviewObject,
              MUIA_Listview_List, lv_format = ListObject,
                InputListFrame,
  							MUIA_List_CompareHook ,&pbCompare,
  							MUIA_List_DisplayHook ,&pbDisplay,
  						End,
  					End,
            Child, HGroup,
              Child, bt_showlist = MakeButton(LOC(MSG_pb_showlist)),
              Child, cy_group = HGroup,
                Child, cy_sort = Cycle(t_sortmode), /* just temporary */
              End,
              Child, cy_sortmode = Cycle(t_sortmode),
            End,
  					Child, HGroup,
              Child, bt_dial    = MakeButton(LOC(MSG_pb_dial)),
              Child, bt_clear   = MakeButton(LOC(MSG_pb_unselect)),
              Child, HSpace(8),
              Child, bt_addentry= MakeButton(LOC(MSG_pb_add)),
              Child, bt_addclone= MakeButton(LOC(MSG_pb_clone)),
              Child, bt_edit    = MakeButton(LOC(MSG_pb_edit)),
              Child, bt_delete  = MakeButton(LOC(MSG_pb_delete)),
              Child, bt_drop    = MakeButton(LOC(MSG_pb_drop_away)),
              Child, bt_freeze  = MakeButton(LOC(MSG_pb_freeze)),
              Child, bt_reset   = MakeButton(LOC(MSG_pb_reset)),
            End,
          End,
        End;

        if (win) {
          DoMethod(gl_app,OM_ADDMEMBER,win);

          DoMethod(win,MUIM_MultiSet,MUIA_CycleChain,1,
                   lv_pb,bt_showlist,cy_sort,cy_sortmode,bt_dial,bt_clear,
                   bt_addentry,bt_addclone,bt_edit,bt_delete,bt_drop,bt_freeze,
                   bt_reset,
                   NULL);
/*
          DoMethod(win,MUIM_Window_SetCycleChain,
                   lv_pb,bt_showlist,cy_sort,cy_sortmode,bt_dial,bt_clear,
                   bt_addentry,bt_addclone,bt_edit,bt_delete,bt_drop,bt_freeze,
                   bt_reset,
                   NULL);
*/
    
          DoMethod(win				,MUIM_Notify,MUIA_Window_CloseRequest ,TRUE          , gl_app,2,MUIM_Application_ReturnID,ID_quit);
          DoMethod(lv_pb			,MUIM_Notify,MUIA_List_Active         ,MUIV_EveryTime, gl_app,2,MUIM_Application_ReturnID,ID_pb);
          DoMethod(lv_pb			,MUIM_Notify,MUIA_Listview_DoubleClick,TRUE          , gl_app,2,MUIM_Application_ReturnID,ID_pbsel);
          DoMethod(cy_sortmode,MUIM_Notify,MUIA_Cycle_Active        ,MUIV_EveryTime, gl_app,2,MUIM_Application_ReturnID,ID_sortmode);
          MultiIDNotify(MUIA_Pressed,FALSE,
                        bt_showlist ,ID_showlist,
    										bt_addentry	,ID_addentry,
    										bt_addclone	,ID_addclone,
    										bt_edit			,ID_edit		,
    										bt_delete		,ID_delete	,
    										bt_drop			,ID_drop		,
    										bt_freeze		,ID_freeze	,
    										bt_reset		,ID_reset		,
    										bt_dial			,ID_dial		,
    										bt_clear		,ID_clear		, TAG_END);
          InfoListToMui(&gl_phonebook,lv_pb,MUIV_List_Insert_Sorted);
          nnset(cy_sortmode,MUIA_Cycle_Active,sortmode);
          win_act = TRUE;
        }
      }

      if (win_act) {
        if (cursl == 0) {
          if (cursl = Show(DT_GET,0)) Show(DT_LOCK,cursl);
        }
        Update();
        Update_show();
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
		case DT_LOAD:
      curpb = 0;
      if (fh = Open(data,MODE_OLDFILE)) {
        Read(fh,&id,4);
				Close(fh);
				if (id != PBID) {
					strcpy(key   ,CUR(PB_CryptKey));
					strcpy(namecr,data);
					if (Ask_string(&req_load,LOC(MSG_pb_enter_password),key)) return;
				} else {
					if ( LoadInfoList(&gl_phonebook,data,PBID) ) {
						gl_phonesafe = TRUE;
            InfoListToMui(&gl_phonebook,lv_pb,MUIV_List_Insert_Sorted);
						return;
					}
				}
			}
			InfoListToMui(&gl_phonebook,lv_pb,MUIV_List_Insert_Sorted);
      Warning(LOC(MSG_pb_cannot_load_complete_phonebook));
      break;
    case DT_SAVE:
      if ( CUR(PB_Crypt) ) {
        TempName(name);
			} else {
				strcpy(name,data);
			}
			if ( SaveInfoList(&gl_phonebook,name,PBID) ) {
				if ( CUR(PB_Crypt) ) {
					if (Crypt(CR_CRYPT,name,data) == 0) {
						Crypt(CR_DELETE,data,0);
						Warning(LOC(MSG_pb_cannot_crypt_phonebook));
						return(0);
					}
					Crypt(CR_DELETE,name,0);
					gl_phonesafe = TRUE;
				} else {
					gl_phonesafe = TRUE;
				}
			} else {
        Warning(LOC(MSG_pb_cannot_save_phonebook));
      }
      break;
    case DT_HANDLE:
      if (win_act) {
        switch (data)
        {
  				case ID_pb:				pb_click();				break;
  				case ID_pbsel:		pbsel_click();		break;
  				case ID_dial:			dial_click();			break;
  				case ID_addentry:	addentry_click();	break;
  				case ID_addclone:	addclone_click();	break;
  				case ID_edit:			edit_click();			break;
  				case ID_clear:		clear_click();		break;
  				case ID_delete:		delete_click();		break;
  				case ID_drop:			drop_click();			break;
  				case ID_freeze:		freeze_click();		break;
  				case ID_reset:		reset_click();		break;
          case ID_showlist: showlist_click(); break;
          case ID_sort:			sort_click();			break;
          case ID_sortmode:	sortmode_click(); break;
          case ID_loadset:	loadset_menu();		break;
          case ID_saveset:	saveset_menu();		break;
          case ID_appset:		appset_menu();		break;
          case ID_savesel:	savesel_menu();		break;
          case ID_droplist:	droplist_menu();	break;
          case ID_showedit:	showedit_menu();	break;
          case ID_quit:			Phone(DT_CLOSE,0);break;
        }
        break;
      }
#ifdef DEBUG
  printf("phonebook: unexpected MUI id\n");
#endif
			break;
    case PB_DESELECT:
      pb = (struct Info *)data;
      if (selcount > 0 AND pb->SELECT) {
        selcount--; pb->SELECT = FALSE; 
        FixNumber(pb->store); pb->store = 0;
        Phone(DT_UPDATE,0);
			}
			break;
    case PB_SELECT: /* only from startup! */
      pb = GetHead(&gl_phonebook);
      if (data) {
        while (pb)
        {
          if (stricmp(GetEntryTag(PB_Name,pb),data) == 0) break;
          pb = GetSucc(pb);
        }
      }
      if (pb) {
        if ( GetEntryTag(PB_Freeze,pb) < GetSec() ) {
          if (pb->SELECT == TRUE ) {
            selcount--;
            pb->SELECT = FALSE; 
            FixNumber(pb->store); 
            pb->store = 0;
          } else {
            selcount++;
            pb->SELECT = TRUE;
            pb->store  = selcount;
          }
        }
      }
      break;
		case PB_ADDENTRY:
			pb = (struct Info *)data;
			AddTail(&gl_phonebook,(struct Node *)pb);
      if (win_act) {
        DoMethod(lv_pb,MUIM_List_Insert,&pb,1,MUIV_List_Insert_Sorted);
        MUI_selnode(lv_pb,pb);
      }
      break;
    case PB_REDRAW:
      if (win_act) {
        pb = (struct Info *)data;
        if (pb) {
          DoMethod(lv_pb,MUIM_List_Redraw,MUI_nodenum(lv_pb,pb));
        } else {
          DoMethod(lv_pb,MUIM_List_Redraw,MUIV_List_Redraw_All);
        }
        Update();
      }
      break;
    case DT_UPDATE:
      if (win_act) {
      ulong b;

        get(win,MUIA_Window_Open,&b);
        if (b) Update();
      }
      break;
    case DT_LOCK:
      ((struct Info *)data)->LOCK = TRUE; 
      if ((struct Info *)data == curpb) Phone(DT_UPDATE,0);
      break;
    case DT_UNLOCK:
      ((struct Info *)data)->LOCK = FALSE; 
      if ((struct Info *)data == curpb) Phone(DT_UPDATE,0);
      break;
  }
}
