#include "global.h"
#include "loc_strings.h"

/*
char ver_charset[]="1.23 (Aug 28 1995)";
*/

enum ID { ID_cs = CS_First,
          ID_name     ,ID_font    ,ID_add       ,ID_del      ,
          ID_txd      ,ID_rxd     ,ID_cap       ,ID_loadset  ,
          ID_saveset  ,ID_appset  ,ID_savesel   ,ID_quit    };

static bool win_act = FALSE;
static aptr lv_cs   = 0;
static aptr win,st_name,io_font,tx_font,tx_xsize,tx_ysize;
static aptr bt_txd,bt_rxd,bt_cap,bt_add,bt_del;

static struct NewMenu menu[] =
{
  { NM_TITLE ,(strptr)MSG_charset_configuration ,  0,0,0,0                },
  { NM_ITEM  ,(strptr)MSG_load                  ,"o",0,0,(APTR)ID_loadset },
  { NM_ITEM  ,(strptr)MSG_save                  ,"s",0,0,(APTR)ID_saveset },
  { NM_ITEM  ,(strptr)MSG_append                ,"a",0,0,(APTR)ID_appset  },
  { NM_ITEM  ,(strptr)MSG_save_selected         ,"c",0,0,(APTR)ID_savesel },
  { NM_ITEM  ,NM_BARLABEL                       ,  0,0,0,0                },
  { NM_ITEM  ,(strptr)MSG_close                 ,"q",0,0,(APTR)ID_quit    },
  { NM_END,0,0,0,0,0 }
};

static struct List  charset;
static struct Info *curcs=0;
static ulong        curnum=0;
static char         namecs[STRSIZE],
                    namect[STRSIZE];
static void (*orderfunc)(void);

/***----------------------------------------------------------------------***/
/*** support                                                              ***/
/***----------------------------------------------------------------------***/

static void Update(void)
{
char t[12];

  if (win_act) {
    set(win,MUIA_Window_Sleep,gl_req);
    if (curcs) {
      nnset(st_name,MUIA_String_Contents,curcs->node.ln_Name);
      nnset(tx_font,MUIA_Text_Contents,GetInfoTag(CS_FontName,curcs));
      stcl_d(t,GetInfoTag(CS_Xsize,curcs)); nnset(tx_xsize,MUIA_Text_Contents,t);
      stcl_d(t,GetInfoTag(CS_Ysize,curcs)); nnset(tx_ysize,MUIA_Text_Contents,t);
      DoMethod(win, MUIM_MultiSet,MUIA_Disabled,(curcs->LOCK OR curcs->NODISK) ? TRUE : FALSE,
               st_name,io_font,bt_txd,bt_rxd,bt_cap,bt_del,NULL);
    } else {
      DoMethod(win,MUIM_MultiSet,MUIA_Disabled,TRUE,st_name,io_font,bt_txd,bt_rxd,bt_cap,bt_del,NULL);
    }
  }
}

static struct Info *CreateCharset(void)
{
ubyte       abc[260];
ulong       i;
struct Info *cs;

  if ( cs = CreateInfo() ) {
    for (i = 0; i < 256; i++) abc[i+4] = i;
    abc[0] = 0; abc[1] = 0; abc[2] = 1; abc[3] = 0; /* size: 256 */
    AddInfoTags(cs,
                CS_FontName ,CSFontName ,
                CS_Xsize    ,CSXsize    ,
                CS_Ysize    ,CSYsize    ,
                CS_Txd      ,&abc       ,
                CS_Rxd      ,&abc       ,
                CS_Cap      ,&abc       , TAG_END);
  }
  return(cs);
}

/***----------------------------------------------------------------------***/
/*** menut                                                                ***/
/***----------------------------------------------------------------------***/

static void loadset_menu(void)
{
  if (!gl_req) {
    Ask_file(gl_conwindow,&loadset_menu,LOC(MSG_load),"#?.cs",namecs);
  } else {
    if ( Ask_file_done() == REQ_OK ) {
      EmptyInfoList(&charset);
      Charset(DT_LOAD,(ULONG)namecs);
    }
  }
}

static void appset_menu(void)
{
  if (!gl_req) {
    Ask_file(gl_conwindow,&appset_menu,LOC(MSG_append),"#?.cs",namecs);
  } else {
    if ( Ask_file_done() == REQ_OK ) {
      Charset(DT_LOAD,(ULONG)namecs);
    }
  }
}

static void saveset_menu(void)
{
  if (!gl_req) {
    Ask_file(gl_conwindow,&saveset_menu,LOC(MSG_save),"#?.cs",namecs);
  } else {
    if ( Ask_file_done() == REQ_OK ) {
      Charset(DT_SAVE,(ULONG)namecs);
    }
  }
}

static void savesel_menu(void)
{
static struct Info  *cs;

  if (!gl_req) {
    if (cs = curcs) {
      if (Ask_file(gl_conwindow,&savesel_menu,LOC(MSG_save_selected),"#?.cs",namecs)) {
        Charset(DT_LOCK,cs);
      }
    }
  } else {
    switch ( Ask_file_done() )
    {
      case REQ_OK:
        SaveInfo(cs,namecs,CSID);
      case REQ_CANCEL:
        Charset(DT_UNLOCK,cs);
    }
  }
}

/***----------------------------------------------------------------------***/
/*** Gadgetit                                                             ***/
/***----------------------------------------------------------------------***/

static void cs_click(void)
{
  get(lv_cs,MUIA_List_Active,&curnum);
  DoMethod(lv_cs,MUIM_List_GetEntry,curnum,&curcs);
  Charset(DT_UPDATE,0);
}

static void name_click(void)
{
char *name;

  if (curcs) {
    get(st_name,MUIA_String_Contents,&name);
    if ( FindName(&charset,name) != curcs ) {
      if ( FindName(&charset,name) == 0 ) {
        StrRep(&curcs->node.ln_Name,name);
         gl_charsafe = FALSE;
      }
      DoMethod(lv_cs,MUIM_List_Redraw,curnum);
      DoMethod(lv_cs,MUIM_List_Sort,0);
      MUI_selnode(lv_cs,curcs);
      SortList(&charset);
      orderfunc();
    }
  }
}

static void font_click(void)
{
static char          name[STRSIZE];
static struct Info  *cs;
struct TextFont *font;
struct TextAttr  attr;

  if (!gl_req) {
    if (cs = curcs) {
      sprintf(name,"%s/%ld",GetInfoTag(CS_FontName,cs),GetInfoTag(CS_Ysize,cs));
      if (Ask_font(gl_conwindow,&font_click,LOC(MSG_select_font),name)) {
        Charset(DT_LOCK,cs);
      }
    }
  } else {
    switch ( Ask_font_done() )
    {
      case REQ_OK:
        attr.ta_Name  = name;
        attr.ta_YSize = atol(FilePart(name));
        attr.ta_Style = FS_NORMAL; 
        attr.ta_Flags = 0;
        *(PathPart(name)) = 0x00;

        if ( (font = OpenFont(&attr)) == 0 ) {
          if ( (font = OpenDiskFont(&attr)) == 0 ) {
            Warning(LOC(MSG_cannot_open_font));
            Charset(DT_UNLOCK,cs);
            return;
          }
        }
        AddInfoTags(cs,
                    CS_FontName ,name,
                    CS_Xsize    ,font->tf_XSize,
                    CS_Ysize    ,font->tf_YSize, TAG_END);          
        CloseFont(font);
        gl_charsafe = FALSE;
      case REQ_CANCEL:
        Charset(DT_UNLOCK,cs);
    }
  }
}

static void add_click(void)
{
struct Info *cs;

  if ( FindName(&charset,LOC(MSG_new_entry)) == 0 ) {
    if ( cs = CreateCharset() ) {
      if ( StrRep(&cs->node.ln_Name,LOC(MSG_new_entry)) ) {
        AddNode_sort(&charset,(struct Node *) cs);
        DoMethod(lv_cs,MUIM_List_InsertSingle,cs,MUIV_List_Insert_Sorted);
        MUI_selnode(lv_cs,cs);
        orderfunc();
        gl_charsafe = FALSE;
      } else {
        DeleteInfo(cs);
      }
    }
  }
}

static void del_click(void)
{
  if (curcs) {
    if (!curcs->LOCK) {
      RemoveInfo(curcs); curcs = 0;
      DoMethod(lv_cs,MUIM_List_Remove,curnum);
      orderfunc();
      gl_charsafe = FALSE;
    }
  }
}

static void txd_click(void)
{
static struct Info *cs;
ULONG abc[65];
BPTR  fh;

  if (!gl_req) {
    if (cs = curcs) {
      if (Ask_file(gl_conwindow,&txd_click,LOC(MSG_load_txd_conversion_table),"#?.ct",namect)) {
        Charset(DT_LOCK,cs);
      }
    }
  } else {
    switch ( Ask_file_done() )
    {
      case REQ_OK:
        if ( (fh = Open(namect,MODE_OLDFILE)) ) {
          if (Read(fh,&abc[1],256) == 256 ) {
            abc[0] = 256;
            AddInfoTag(CS_Txd,&abc,cs);
            gl_charsafe = FALSE;
          } else {
            Warning(LOC(MSG_loaded_conversion_table_incomplete_current_not_changed));
          }
          Close(fh);
        } else {
          Warning(LOC(MSG_cannot_open_conversion_table));
        }
      case REQ_CANCEL:
        Charset(DT_UNLOCK,cs);
    }
  }
}

static void rxd_click(void)
{
static struct Info *cs;
bptr  fh;
ulong abc[65];

  if (!gl_req) {
    if (cs = curcs) {
      if (Ask_file(gl_conwindow,&rxd_click,LOC(MSG_load_rxd_conversion_table),"#?.ct",namect)) {
        Charset(DT_LOCK,cs);
      }
    }
  } else {
    switch ( Ask_file_done() )
    {
      case REQ_OK:
        if ( (fh = Open(namect,MODE_OLDFILE)) ) {
          if (Read(fh,&abc[1],256) == 256 ) {
            abc[0] = 256;
            AddInfoTag(CS_Rxd,&abc,cs);
            gl_charsafe = FALSE;
          } else {
            Warning(LOC(MSG_loaded_conversion_table_incomplete_current_not_changed));
          }
          Close(fh);
        } else {
          Warning(LOC(MSG_cannot_open_conversion_table));
        }
      case REQ_CANCEL:
        Charset(DT_UNLOCK,cs);
    }
  }
}

static void cap_click(void)
{
static struct Info *cs;
bptr  fh;
ulong abc[65];

  if (!gl_req) {
    if (cs = curcs) {
      if (Ask_file(gl_conwindow,&cap_click,LOC(MSG_load_capture_conversion_table),"#?.ct",namect)) {
        Charset(DT_LOCK,cs);
      }
    }
  } else {
    switch ( Ask_file_done() )
    {
      case REQ_OK:
        if ( (fh = Open(namect,MODE_OLDFILE)) ) {
          if (Read(fh,&abc[1],256) == 256 ) {
            abc[0] = 256;
            AddInfoTag(CS_Cap,&abc,cs);
            gl_charsafe = FALSE;
          } else {
            Warning(LOC(MSG_loaded_conversion_table_incomplete_current_not_changed));
          }
          Close(fh);
        } else {
          Warning(LOC(MSG_cannot_open_conversion_table));
        }
      case REQ_CANCEL:
        Charset(DT_UNLOCK,cs);
    }
  }
}

/*  Interface pääohjelmaan
 *
 */

ulong Charset(ULONG tag, ULONG data)
{
static bool ocall = FALSE; static ulong oc;
struct Info *cs;

  switch (tag)
  {
    case DT_INIT:
      LocalizeMenu(menu,sizeof(menu)/sizeof(struct NewMenu));
      NewList(&charset);
      if (cs = CreateCharset()) {
        StrRep(&cs->ln_Name,STR_DEF(PB_Charset));
        AddHead(&charset,(struct Node *) cs);
        cs->LOCK = cs->NODISK = TRUE;
        return(TRUE);
      } else {
        return(FALSE);
      }
      break;
    case DT_CLOSE:
      if (win_act) {
        set(win,MUIA_Window_Open,FALSE);
        if (CUR(PB_Dispose)) {
          DoMethod(gl_app,OM_REMMEMBER,win);
          MUI_DisposeObject(win);
          win_act = FALSE;
          curcs   = 0;
        }
      }
      break;
    case DT_OPEN:
      if (!win_act) {
        win = WindowObject,
        MUIA_Window_ID    ,MAKE_ID('C','S','W','I'),
        MUIA_Window_Title ,LOC(MSG_charset_configuration),
        MUIA_Window_Menu  ,&menu,
        MUIA_HelpNode     ,"help_cs",
        MUIA_Window_Width ,MUIV_Window_Width_Visible(50),
          WindowContents, VGroup,
            Child, VGroup,
              Child, VGroup,
                GroupSpacing(0),
                Child, lv_cs   = ListView(),
                Child, st_name = String(STRSIZE),
              End,
              Child, HGroup,
                Child, HGroup,
                  GroupSpacing(0),
                  Child, io_font = PopFile(),
                  Child, tx_font = TextObject, TextFrame, MUIA_Background,MUII_TextBack, End,
                End,
                Child, Label2L(LOC(MSG_width)),
                Child, tx_xsize = TextObject, TextFrame, MUIA_Weight,30, MUIA_Background,MUII_TextBack, End,
                Child, Label2L(LOC(MSG_heigth)),
                Child, tx_ysize = TextObject, TextFrame, MUIA_Weight,30, MUIA_Background,MUII_TextBack, End,
              End,
              Child, bt_txd = MakeButton(LOC(MSG_load_txd_conversion_table)),
              Child, bt_rxd = MakeButton(LOC(MSG_load_rxd_conversion_table)),
              Child, bt_cap = MakeButton(LOC(MSG_load_capture_conversion_table)),
            End,
            Child, HGroup,
              Child, bt_add = MakeButton(LOC(MSG_key_add)),
              Child, bt_del = MakeButton(LOC(MSG_key_delete)),
            End,
          End,
        End;
        if (win) {
          DoMethod(gl_app,OM_ADDMEMBER,win);

          DoMethod(win,MUIM_MultiSet,MUIA_CycleChain,1,
                   lv_cs,st_name,io_font,bt_txd,bt_rxd,bt_cap,bt_add,bt_del,NULL);
    
          DoMethod(win    ,MUIM_Notify,MUIA_Window_CloseRequest,TRUE, gl_app,2,MUIM_Application_ReturnID,ID_quit);
          DoMethod(lv_cs  ,MUIM_Notify,MUIA_List_Active,MUIV_EveryTime, gl_app,2,MUIM_Application_ReturnID,ID_cs);
          DoMethod(lv_cs  ,MUIM_Notify,MUIA_Listview_DoubleClick,TRUE, win,3,MUIM_Set,MUIA_Window_ActiveObject,st_name);
          DoMethod(st_name,MUIM_Notify,MUIA_String_Acknowledge,MUIV_EveryTime, gl_app,2,MUIM_Application_ReturnID,ID_name);
          MultiIDNotify(MUIA_Pressed,FALSE,
                        io_font ,ID_font,
                        bt_add  ,ID_add,
                        bt_del  ,ID_del,
                        bt_txd  ,ID_txd,
                        bt_rxd  ,ID_rxd,
                        bt_cap  ,ID_cap, TAG_END);
          InfoListToMui(&charset,lv_cs,MUIV_List_Insert_Sorted);
          win_act = TRUE;
        }
      }
      if (win_act) {
        Update();
        set(win,MUIA_Window_Screen,gl_conscreen);
        set(win,MUIA_Window_Open,TRUE);
        set(win,MUIA_Window_Activate,TRUE);
        set(win,MUIA_Window_Sleep,gl_req);
      }
      break;
    case DT_OC:
      if (win_act) {
        if (ocall) {
          Update();
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
      return(FindName(&charset,(char *)data));
      break;
    case DT_LOAD:
      LoadInfoList(&charset,data,CSID);
      InfoListToMui(&charset,lv_cs,MUIV_List_Insert_Sorted);
      SortList(&charset);
      curcs       = 0;
      gl_charsafe = TRUE;
      Charset(DT_UPDATE,0);
      orderfunc();
      break;
    case DT_SAVE:
      SaveInfoList(&charset,data,CSID);
      gl_charsafe = TRUE;
      break;
    case DT_HANDLE:
      if (win_act) {
        switch (data)
        {
          case ID_cs:       cs_click();     break;
          case ID_name:     name_click();   break;
          case ID_font:     font_click();   break;
          case ID_add:      add_click();    break;
          case ID_del:      del_click();    break;
          case ID_txd:      txd_click();    break;
          case ID_rxd:      rxd_click();    break;
          case ID_cap:      cap_click();    break;
          case ID_loadset:  loadset_menu(); break;
          case ID_saveset:  saveset_menu(); break;
          case ID_appset:   appset_menu();  break;
          case ID_savesel:  savesel_menu(); break;
          case ID_quit:     Charset(DT_CLOSE,0); break;
        }
        break;
      }
#ifdef DEBUG
  printf("Charset: unexpected MUI ID\n");
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
      return(FindNode_num(&charset,data));
      break;
    case DT_LOCK:
      cs = data;
      cs->LOCK = TRUE; 
      if (cs == curcs) Charset(DT_UPDATE,0);
      break;
    case DT_UNLOCK:
      cs = data;
      cs->LOCK = FALSE;
      if (cs == curcs) Charset(DT_UPDATE,0);
      break;
    case DT_COUNT:
      return(NodeCount(&charset));
      break;
  }
}
