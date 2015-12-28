#include "global.h"
#include "loc_strings.h"

/*
char ver_showlist[]="1.00 (Oct 31 1995)";
*/

enum ID { ID_sl = SL_First,
          ID_tl     ,ID_name   ,ID_pos    ,ID_add    ,ID_del    ,
          ID_addtl  ,ID_deltl  ,ID_up     ,ID_dn     ,
          ID_loadset,ID_saveset,ID_appset,ID_savesel,ID_quit };

static bool win_act = FALSE;
static aptr lv_sl   = 0;
static aptr win,lv_tl,bt_add,bt_del,bt_addtl,bt_deltl,bt_up,bt_dn,st_pos,st_name;

static struct NewMenu menu[] =
{
  { NM_TITLE ,(strptr)MSG_showlist_configuration,  0,0,0,0                },
  { NM_ITEM  ,(strptr)MSG_load                  ,"o",0,0,(APTR)ID_loadset },
  { NM_ITEM  ,(strptr)MSG_save                  ,"s",0,0,(APTR)ID_saveset },
  { NM_ITEM  ,(strptr)MSG_append                ,"a",0,0,(APTR)ID_appset  },
  { NM_ITEM  ,(strptr)MSG_save_selected         ,"c",0,0,(APTR)ID_savesel },
  { NM_ITEM  , NM_BARLABEL                      ,  0,0,0,0                },
  { NM_ITEM  ,(strptr)MSG_close                 ,"q",0,0,(APTR)ID_quit    },
  { NM_END,0,0,0,0,0 }
};

static const ulong pop_tags[] =
{
  PB_Name       ,PB_PhonePre   ,PB_Phone      ,PB_Comment    ,
  PB_Emulation  ,
  ~0            ,
  PB_CallLast   ,PB_CallTime   ,PB_Freeze     ,PB_CallCost   ,
  PB_CallCount  ,PB_CallTxd    ,PB_CallRxd    ,
  ~0            ,
  PB_HardScr    ,PB_Columns    ,PB_Lines      ,PB_HLines     ,
  PB_ForceConXY ,PB_ConX       ,PB_ConY       ,PB_Depth      ,
  PB_ModeId     ,PB_Overscan   ,PB_AutoScroll ,PB_Wb         ,
  PB_PubScreen  ,PB_RxdPopup   ,PB_BorderBlank,PB_Aga        ,
  PB_KeepWb     ,PB_RtsBlit    ,PB_Status     ,
  ~0            ,
  PB_HardSer    ,PB_SerName    ,PB_SerUnit    ,PB_SerBufSize ,
  PB_DteRate    ,PB_DataBits   ,PB_Parity     ,PB_StopBits   ,
  PB_FlowControl,PB_FullDuplex ,PB_FixRate    ,PB_DTRHangup  ,
  PB_NoCarrier  ,PB_NoDialTone ,PB_Connect    ,PB_Ringing    ,
  PB_Busy       ,PB_Ok         ,PB_Error      ,PB_DialSuffix ,
  PB_DialAbort  ,PB_Hangup     ,
  ~0            ,
  PB_PassWord   ,PB_DialString ,PB_LogName    ,PB_LogCall    ,
  PB_Charset    ,PB_Protocol   ,PB_Xem        ,PB_UpPath     ,
  PB_DnPath     ,PB_DnFiles    ,PB_AutoXfer   ,PB_QuietXfer  ,
  PB_Double     ,PB_CostFirst  ,PB_CostRest   ,PB_Baron      ,
  PB_Toolbar    ,PB_cr2crlf    ,PB_lf2crlf    ,PB_WaitEntry  ,
  PB_WaitDial   ,
  0
};

static struct List showlist;
static struct Info *cursl  =0;  /* selected entry  */
static ulong       curnum  =0;  /* selected number */
static char        namesl[STRSIZE];
static void (*orderfunc)(void);

/***----------------------------------------------------------------------***/
/*** support                                                              ***/
/***----------------------------------------------------------------------***/

static void Update(void)
{
  if (win_act) {
    set(win,MUIA_Window_Sleep,gl_req);
    if (cursl) {
      nnset(st_name,MUIA_String_Contents,cursl->node.ln_Name);
      set(lv_tl,MUIA_Disabled,FALSE);
      DoMethod(win,MUIM_MultiSet,MUIA_Disabled,cursl->LOCK | cursl->NODISK,
               st_name,st_pos,bt_up,bt_dn,bt_addtl,bt_deltl,bt_del,NULL);
    } else {
      DoMethod(win,MUIM_MultiSet,MUIA_Disabled,TRUE,
               st_name,lv_tl,st_pos,bt_up,bt_dn,bt_addtl,bt_deltl,bt_del,NULL);
    }
  }
}

static void Update_tl(void)
{
long i;
struct TagItem *ti;

  if (win_act AND cursl) {
    nnset   (lv_tl,MUIA_List_Quiet,TRUE);
    DoMethod(lv_tl,MUIM_List_Clear);
    ti = FindTag_info(SL_tag,cursl);
    for (i=0; i < CountDat(ti); i++)
    {
      DoMethod(lv_tl,MUIM_List_InsertSingle,
               TagName(GetDat(ti,i)),MUIV_List_Insert_Bottom);
    }
    nnset(lv_tl,MUIA_List_Quiet,FALSE);
  }
}

/***----------------------------------------------------------------------***/
/*** menut                                                                ***/
/***----------------------------------------------------------------------***/

static void loadset_menu(void)
{
  if (!gl_req) {
    Ask_file(gl_conwindow,&loadset_menu,LOC(MSG_load),"#?.sl",namesl);
  } else {
    if ( Ask_file_done() == REQ_OK ) {
      EmptyInfoList(&showlist);
      Show(DT_LOAD,(ULONG)namesl);
    }
  }
}

static void appset_menu(void)
{
  if (!gl_req) {
    Ask_file(gl_conwindow,&appset_menu,LOC(MSG_append),"#?.sl",namesl);
  } else {
    if ( Ask_file_done() == REQ_OK ) {
      Show(DT_LOAD,(ULONG)namesl);
    }
  }
}

static void saveset_menu(void)
{
  if (!gl_req) {
    Ask_file(gl_conwindow,&saveset_menu,LOC(MSG_save),"#?.sl",namesl);
  } else {
    if ( Ask_file_done() == REQ_OK ) {
      Show(DT_SAVE,(ULONG)namesl);
    }
  }
}

static void savesel_menu(void)
{
static struct Info  *sl;

  if (!gl_req) {
    if (sl = cursl) {
      if (Ask_file(gl_conwindow,&savesel_menu,LOC(MSG_save_selected),"#?.sl",namesl)) {
        Show(DT_LOCK,sl);
      }
    }
  } else {
    switch ( Ask_file_done() )
    {
      case REQ_OK:
        SaveInfo(sl,namesl,SLID);
      case REQ_CANCEL:
        Show(DT_UNLOCK,sl);
    }
  }
}

/***----------------------------------------------------------------------***/
/*** Gadgetit                                                             ***/
/***----------------------------------------------------------------------***/

static void sl_click(void)
{
  get(lv_sl,MUIA_List_Active,&curnum);
  DoMethod(lv_sl,MUIM_List_GetEntry,curnum,&cursl);
  Update();
  Update_tl();
  set(lv_tl,MUIA_List_Active,0);
}

static void name_click(void)
{
char *name;

  if (cursl) {
    get(st_name,MUIA_String_Contents,&name);
    if ( FindName(&showlist,name) != cursl ) {
      if ( FindName(&showlist,name) == 0 ) {
        StrRep(&cursl->node.ln_Name,name);
      }
      DoMethod(lv_sl,MUIM_List_Redraw,curnum);
      DoMethod(lv_sl,MUIM_List_Sort,0);
      MUI_selnode(lv_sl,cursl);
      SortList(&showlist);
      orderfunc();
    }
  }
}

static void tl_click(void)
{
ulong num;

  get(lv_tl,MUIA_List_Active,&num);
  if (num != MUIV_List_Active_Off AND cursl) {
    nnset(st_pos,MUIA_String_Contents,GetDat(FindInfoTag(SL_pos,cursl),num));
  }
}

static void pos_click(void)
{
ulong  num;
strptr name;

  get(lv_tl,MUIA_List_Active,&num);
  get(st_pos,MUIA_String_Contents,&name);
  if (num != MUIV_List_Active_Off AND cursl) {
    SetDat(FindInfoTag(SL_pos,cursl),name,num);
  }
}

static void up_click(void)
{
struct TagItem *ti;
ulong  num,oldtag;
char   oldpos[STRSIZE];

  get(lv_tl,MUIA_List_Active,&num);
  if (num != MUIV_List_Active_Off AND num != 0 AND cursl) {

    ti = FindTag_info(SL_tag,cursl);
    oldtag = GetDat(ti,num-1);
    SetDat(ti,GetDat(ti,num),num-1);
    SetDat(ti,oldtag,num);

    ti = FindTag_info(SL_pos,cursl);
    strcpy(oldpos,GetDat(ti,num-1));
    SetDat(ti,GetDat(ti,num),num-1);
    SetDat(ti,oldpos,num);

    Update_tl();
    set(lv_tl,MUIA_List_Active,num-1);
    orderfunc();
  }
}

static void dn_click(void)
{
struct TagItem *ti;
ulong  num,oldtag;
char   oldpos[STRSIZE];

  get(lv_tl,MUIA_List_Active,&num);
  if (num != MUIV_List_Active_Off AND cursl) {
    ti = FindTag_info(SL_tag,cursl);
    if (CountDat(ti) > (num+1)) {

      oldtag = GetDat(ti,num+1);
      SetDat(ti,GetDat(ti,num),num+1);
      SetDat(ti,oldtag,num);
  
      ti = FindTag_info(SL_pos,cursl);
      strcpy(oldpos,GetDat(ti,num+1));
      SetDat(ti,GetDat(ti,num),num+1);
      SetDat(ti,oldpos,num);
  
      Update_tl();
      set(lv_tl,MUIA_List_Active,num+1);
      orderfunc();
    }
  }
}

static void addtl_click(void)
{
struct TagItem *ti1,*ti2;
static long   num;
static strptr *list;

  if (!gl_req) {
    if (cursl) {
      if (CountDat(FindTag_info(SL_tag,cursl)) < 64) {
        if (list = MakeNameList(pop_tags)) {
          if (!Ask_list(&addtl_click,LOC(MSG_select_tag),list,&num)) {
            FreeNameList(list);
          }
        }
      }
    }
  } else {
    switch ( Ask_list_done() )
    {
      case REQ_OK:
        if (pop_tags[num] != ~0) {
          ti1 = FindTag_info(SL_tag,cursl);
          ti2 = FindTag_info(SL_pos,cursl);
          SetDat(ti1,pop_tags[num],CountDat(ti1));
          SetDat(ti2,""           ,CountDat(ti2));
          Update_tl();
          set(lv_tl,MUIA_List_Active,CountDat(ti2));
        }
      case REQ_CANCEL:
        FreeNameList(list);
    }
  }
}

static void deltl_click(void)
{
ulong num;

  get(lv_tl,MUIA_List_Active,&num);
  if (num != MUIV_List_Active_Off AND cursl) {
    DelDat(FindTag_info(SL_tag,cursl),num);
    DelDat(FindTag_info(SL_pos,cursl),num);
    Update_tl();
    if (num) {
      set(lv_tl,MUIA_List_Active,num-1);
    } else {
      set(lv_tl,MUIA_List_Active,0);
    }
    orderfunc();
  }
}

static void add_click(void)
{
struct Info *sl;

  if ( FindName(&showlist,LOC(MSG_new_entry)) == 0 ) {
    if ( sl = CreateInfo() ) {
      if ( StrRep(&sl->node.ln_Name,LOC(MSG_new_entry)) ) {
        AddInfoTag(SL_tag,CreateDat(SL_tag),sl);
        AddInfoTag(SL_pos,CreateDat(SL_pos),sl);
        AddNode_sort(&showlist,(struct Node *) sl);
        DoMethod(lv_sl,MUIM_List_InsertSingle,sl,MUIV_List_Insert_Sorted);
        MUI_selnode(lv_sl,sl);
        orderfunc();
      } else {
        DeleteInfo(sl);
      }
    }
  }
}

static void del_click(void)
{
  if (cursl) {
    if (!cursl->LOCK AND !cursl->NODISK) {
      RemoveInfo(cursl);
      cursl = 0;
      DoMethod(lv_sl,MUIM_List_Remove,curnum);
      orderfunc();
    }
  }
}

/*  Interface pääohjelmaan
 *
 */

ulong Show(ulong tag, ulong data)
{
static BOOL ocall = FALSE; static ULONG oc;
struct Info *sl;

  switch (tag)
  {
    case DT_INIT:
      NewList(&showlist);
      LocalizeMenu(menu,sizeof(menu)/sizeof(struct NewMenu));
      return(TRUE);
      break;
    case DT_CLOSE:
      if (win_act) {
        set(win,MUIA_Window_Open,FALSE);
        if (CUR(PB_Dispose)) {
          DoMethod(gl_app,OM_REMMEMBER,win);
          MUI_DisposeObject(win);
          win_act = FALSE;
          cursl   = 0;
        }
      }
      break;
    case DT_OPEN:
      if (!win_act) {
        win = WindowObject,
        MUIA_Window_ID    ,MAKE_ID('S','L','W','I'),
        MUIA_Window_Title ,LOC(MSG_showlist_configuration),
        MUIA_Window_Menu  ,&menu,
        MUIA_HelpNode     ,"help_sl",
        MUIA_Window_Width ,MUIV_Window_Width_Visible(50),
          WindowContents, VGroup,
            Child, HGroup,
              Child, VGroup,
                GroupSpacing(0),
                Child, lv_sl   = ListView(),
                Child, st_name = String(STRSIZE),
              End,
              Child, VGroup,
                GroupFrame,
      					Child, lv_tl = ListviewObject,
      						MUIA_Listview_List, ListObject,
      							InputListFrame,
                    MUIA_List_ConstructHook, MUIV_List_ConstructHook_String,
                    MUIA_List_DestructHook , MUIV_List_DestructHook_String,
      						End,
      					End,
                Child, ColGroup(2),
                  Child, Label2L(LOC(MSG_control)), Child, st_pos = String(STRSIZE),
                End,
                Child, HGroup,
                  Child, bt_up = MakeButton(LOC(MSG_move_up)),
                  Child, bt_dn = MakeButton(LOC(MSG_move_down)),
                End,
                Child, HGroup,
                  Child, bt_addtl = MakeButton(LOC(MSG_add_tag)),
                  Child, bt_deltl = MakeButton(LOC(MSG_delete_tag)),
                End,
              End,
            End,
            Child, HGroup,
              Child, bt_add = MakeButton(LOC(MSG_key_add)),
              Child, bt_del = MakeButton(LOC(MSG_key_delete)),
            End,
          End,
        End;
        if (win) {
          DoMethod(gl_app,OM_ADDMEMBER,win);

          DoMethod(win,MUIM_Window_SetCycleChain,
                   lv_sl,st_name,lv_tl,st_pos,bt_up,bt_dn,bt_addtl,bt_deltl,bt_add,bt_del, NULL);

          DoMethod(win    ,MUIM_Notify,MUIA_Window_CloseRequest,TRUE  , gl_app,2,MUIM_Application_ReturnID,ID_quit);
          DoMethod(lv_sl  ,MUIM_Notify,MUIA_List_Active,MUIV_EveryTime, gl_app,2,MUIM_Application_ReturnID,ID_sl);
          DoMethod(lv_tl  ,MUIM_Notify,MUIA_List_Active,MUIV_EveryTime, gl_app,2,MUIM_Application_ReturnID,ID_tl);
          DoMethod(lv_sl  ,MUIM_Notify,MUIA_Listview_DoubleClick,TRUE , win,3,MUIM_Set,MUIA_Window_ActiveObject,st_name);
          DoMethod(lv_tl  ,MUIM_Notify,MUIA_Listview_DoubleClick,TRUE , win,3,MUIM_Set,MUIA_Window_ActiveObject,st_pos);
          DoMethod(st_name,MUIM_Notify,MUIA_String_Acknowledge,MUIV_EveryTime,gl_app,2,MUIM_Application_ReturnID,ID_name);
          DoMethod(st_pos ,MUIM_Notify,MUIA_String_Contents   ,MUIV_EveryTime,gl_app,2,MUIM_Application_ReturnID,ID_pos);
          MultiIDNotify(MUIA_Pressed,FALSE,
                      bt_addtl,ID_addtl,
                      bt_deltl,ID_deltl,
                      bt_up   ,ID_up   ,
                      bt_dn   ,ID_dn   ,
                      bt_add  ,ID_add  ,
                      bt_del  ,ID_del  , TAG_END);
          InfoListToMui(&showlist,lv_sl,MUIV_List_Insert_Sorted);
          win_act = TRUE;
        }
      }
      if (win_act) {
        Update();
        set(win,MUIA_Window_Screen  ,gl_conscreen);
        set(win,MUIA_Window_Open    ,TRUE);
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
    case DT_LINK:
      orderfunc = data;
      break;
    case DT_FIND:
      return(FindName(&showlist,(char *)data));
      break;
    case DT_LOAD:
      LoadInfoList (&showlist,data,SLID);
      InfoListToMui(&showlist,lv_sl,MUIV_List_Insert_Sorted);
      SortList(&showlist);
      cursl = 0;
      Show(DT_UPDATE,0);
      orderfunc();
      break;
    case DT_SAVE:
      SaveInfoList(&showlist,data,SLID);
      break;
    case DT_HANDLE:
      if (win_act) {
        switch (data)
        {
          case ID_sl:       sl_click();     break;
          case ID_tl:       tl_click();     break;
          case ID_name:     name_click();   break;
          case ID_pos:      pos_click();    break;
          case ID_add:      add_click();    break;
          case ID_del:      del_click();    break;
          case ID_up:       up_click();     break;
          case ID_dn:       dn_click();     break;
          case ID_addtl:    addtl_click();  break;
          case ID_deltl:    deltl_click();  break;
          case ID_loadset:  loadset_menu(); break;
          case ID_saveset:  saveset_menu(); break;
          case ID_appset:   appset_menu();  break;
          case ID_savesel:  savesel_menu(); break;
          case ID_quit:     Show(DT_CLOSE,0); break;
        }
        break;
      }
#ifdef DEBUG
  printf("showlist: unexpected MUI id\n");
#endif
      break;
    case DT_UPDATE:
      if (win_act) {
        ulong b;

        get(win,MUIA_Window_Open,&b);
        if (b) Update();
      }
      break;
    case DT_GET:
      return(FindNode_num(&showlist,data));
      break;
    case DT_LOCK:
      sl = data;
      sl->LOCK = TRUE; 
      if (sl == cursl) Show(DT_UPDATE,0);
      break;
    case DT_UNLOCK:
      sl = data;
      sl->LOCK = FALSE;
      if (sl == cursl) Show(DT_UPDATE,0);
      break;
    case DT_COUNT:
      return(NodeCount(&showlist));
      break;
  }
}
