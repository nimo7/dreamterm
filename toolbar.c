#include "global.h"
#include "loc_strings.h"

/*
char ver_toolbar[]="1.02 (Apr 16 1996)";
*/

#define EMPTY_WIDTH  8
#define EMPTY_HEIGHT 8
#define ID_SIZE      16

enum ID { ID_tb = TB_First,
          ID_name         ,ID_button       ,ID_curbut       ,ID_butleft      ,
          ID_butright     ,ID_addbut       ,ID_delbut       ,ID_serial       ,
          ID_command      ,ID_action       ,ID_add          ,ID_del          ,
          ID_loadset      ,ID_saveset      ,ID_appset       ,ID_savesel      ,
          ID_quit         ,

          ID_bar_base     };

static aptr bar_win = 0;
static bool win_act = FALSE;
static aptr win,lv_tb,st_name,st_serial,st_command,bt_add,bt_del,bi_group,
            bi_button,tx_format,sl_curbut,bt_addbut,bt_delbut,
            cy_action,bt_butleft,bt_butright;

static struct NewMenu menu[] =
{
  { NM_TITLE,(strptr)MSG_tb_toolbar_configuration,  0,0,0,0                },
  { NM_ITEM ,(strptr)MSG_load                    ,"o",0,0,(APTR)ID_loadset },
  { NM_ITEM ,(strptr)MSG_save                    ,"s",0,0,(APTR)ID_saveset },
  { NM_ITEM ,(strptr)MSG_append                  ,"a",0,0,(APTR)ID_appset  },
  { NM_ITEM ,(strptr)MSG_save_selected           ,"c",0,0,(APTR)ID_savesel },
  { NM_ITEM , NM_BARLABEL                        ,  0,0,0,0                },
  { NM_ITEM ,(strptr)MSG_close                   ,"q",0,0,(APTR)ID_quit    },
  { NM_END,0,0,0,0,0 }
};

static const ulong tb_tags[]=
{
  cmd_none          ,

  cmd_upload        ,cmd_uploadlist    ,cmd_download      ,cmd_send_conv     ,
  cmd_send_no_conv  ,cmd_abort_send    ,

  cmd_start_capture ,cmd_hangup        ,cmd_release_serial,

  cmd_open_current  ,cmd_open_profile  ,cmd_open_xp       ,cmd_open_cs       ,
  cmd_open_xm       ,cmd_open_tb       ,cmd_open_crypt    ,cmd_open_uplist   ,
  cmd_open_review   ,

  cmd_open_phonebook,cmd_dial_number   ,cmd_dial_next     ,cmd_send_password ,
  cmd_new_entry     ,cmd_load_phonebook,cmd_save_phonebook,

  cmd_load_current  ,cmd_load_default  ,cmd_save_current  ,cmd_save_default  ,
  cmd_titlebar      ,cmd_about         ,cmd_quit          ,
  
  0
};

static struct List      toolbar;
static struct Info     *curtb  =0;
static ulong            curnum =0;
static struct Info     *tb_show=0;        /* currently showing bar */
static struct Picture **tb_map =0;        /* used bitmaps          */
static ulong           *tb_obj =0;        /* objects               */
static strptr          *tb_list=0;        /* names of actions      */
static struct BitMap   *tb_bitmap;        /* empty bitmap          */
static bool             no_close;         /* flag for update_image */
static struct Picture  *curpic =0;        /* current picture       */
static char             tb_id  [ID_SIZE]; /* current format id     */
static char             tb_file[STRSIZE]; /* current file          */
static char             tb_name[STRSIZE];
static void (*orderfunc)(void);

/***----------------------------------------------------------------------***/
/*** support                                                              ***/
/***----------------------------------------------------------------------***/

static void Update_image(struct Picture *pic)
{
char str[STRSIZE];

  if (win_act) {
    DoMethod(bi_group,MUIM_Group_InitChange);
    DoMethod(bi_group,OM_REMMEMBER,bi_button);
    FreePicture(curpic);
    if (pic) {
      bi_button = BitmapObject,
        ButtonFrame,
        MUIA_InputMode          ,MUIV_InputMode_RelVerify,
        MUIA_Bitmap_Bitmap      ,pic->bmap,
        MUIA_Bitmap_SourceColors,&pic->palette[1],
        MUIA_Bitmap_Width       ,pic->bmhd.bmh_Width,
        MUIA_Bitmap_Height      ,pic->bmhd.bmh_Height,
        MUIA_Bitmap_UseFriend   ,TRUE,
        MUIA_Bitmap_Precision   ,PRECISION_IMAGE,
      End;
      sprintf(str,"%ldx%ldx%ld %s (%s)",pic->bmhd.bmh_Width,
                                        pic->bmhd.bmh_Height,
                                        pic->bmhd.bmh_Depth,
                                        tb_id,
                                        FilePart(tb_file));
      curpic = pic;
    } else {
      bi_button = BitmapObject,
        ButtonFrame,
        MUIA_InputMode          ,MUIV_InputMode_RelVerify,
        MUIA_Bitmap_Bitmap      ,tb_bitmap,
        MUIA_Bitmap_Width       ,EMPTY_WIDTH,
        MUIA_Bitmap_Height      ,EMPTY_HEIGHT,
        MUIA_Bitmap_UseFriend   ,TRUE,
        MUIA_Bitmap_Precision   ,PRECISION_IMAGE,
      End;
      strcpy(str,LOC(MSG_internal_empty));
      tb_file[0] = 0;
      curpic = 0;
    }
    nnset   (tx_format,MUIA_Text_Contents,LOC(MSG_mui_is_remapping_image));
    DoMethod(bi_group ,OM_ADDMEMBER,bi_button);
    DoMethod(bi_group ,MUIM_Group_ExitChange);
    DoMethod(bi_button,MUIM_Notify,MUIA_Pressed,FALSE,gl_app,2,MUIM_Application_ReturnID,ID_button);
    nnset   (bi_button,MUIA_CycleChain,1);
    nnset   (tx_format,MUIA_Text_Contents,str);
  }
}

void Update(void)
{
ulong           buttons,
                curbut;
strptr          tb_new;
struct Picture *pic=0;

  if (win_act) {

    /* update objects */
    if (curtb) {
      nnset(st_name,MUIA_String_Contents,curtb->node.ln_Name);
      if (buttons = CountDat(FindTag_info(TB_Button,curtb))) {
          get(sl_curbut ,MUIA_Slider_Level,&curbut);
#ifdef DEBUG
        if (curbut > (buttons-1)) printf("Toolbar: update_slider range error\n");
#endif
        nnset(st_serial ,MUIA_String_Contents,GetDat(FindTag_info(TB_Serial ,curtb),curbut));
        nnset(st_command,MUIA_String_Contents,GetDat(FindTag_info(TB_Command,curtb),curbut));
        nnset(cy_action ,MUIA_Cycle_Active   ,RetIndex(tb_tags,GetDat(FindTag_info(TB_Action ,curtb),curbut)));
        nnset(sl_curbut ,MUIA_Slider_Min  ,0);
        nnset(sl_curbut ,MUIA_Slider_Max  ,buttons-1);
        if (strcmp(tb_file,tb_new = GetDat(FindTag_info(TB_Button,curtb),curbut)) != 0) {
          if (IsDataTypes(tb_new,tb_id,ID_SIZE)) {
            if (pic = AllocPicture()) {
              nnset(tx_format,MUIA_Text_Contents,LOC(MSG_loading_image));
              if (LoadPicture(tb_new,pic,0) == 0) {
                strcpy(tb_file,tb_new);
              } else {
                FreePicture(pic);
                pic = 0;
              }
            }
          }
          Update_image(pic);
        }
      } else {
        nnset(st_serial ,MUIA_String_Contents,"");
        nnset(st_command,MUIA_String_Contents,"");
        nnset(cy_action ,MUIA_Cycle_Active   , 0);
        nnset(sl_curbut ,MUIA_Slider_Level   , 0);
        nnset(sl_curbut ,MUIA_Slider_Min     , 0);
        nnset(sl_curbut ,MUIA_Slider_Max     , 0);
        Update_image(0);
      }
    } else {
      nnset(st_name   ,MUIA_String_Contents,"");
      nnset(st_serial ,MUIA_String_Contents,"");
      nnset(st_command,MUIA_String_Contents,"");
      nnset(cy_action ,MUIA_Cycle_Active   , 0);
      nnset(sl_curbut ,MUIA_Slider_Level   , 0);
      nnset(sl_curbut ,MUIA_Slider_Min     , 0);
      nnset(sl_curbut ,MUIA_Slider_Max     , 0);
      Update_image(0);
    }

    /* update disable/enable */
    set(win,MUIA_Window_Sleep,gl_req);
    if (curtb) {
      if (!curtb->LOCK) {
        if (CountDat(FindTag_info(TB_Button,curtb))) {
          DoMethod(win,MUIM_MultiSet,MUIA_Disabled,FALSE,
                   st_name,bi_button,tx_format,st_serial,st_command,cy_action,
                   sl_curbut,bt_butleft,bt_butright,bt_addbut,bt_delbut,bt_del, NULL);
        } else {
          DoMethod(win,MUIM_MultiSet,MUIA_Disabled,FALSE,
                   st_name,bt_addbut,bt_del, NULL);
          DoMethod(win,MUIM_MultiSet,MUIA_Disabled,TRUE,
                   bi_button,tx_format,st_serial,st_command,cy_action,sl_curbut,
                   bt_delbut, NULL);
        }
        return;
      }
    }
    DoMethod(win,MUIM_MultiSet,MUIA_Disabled,TRUE,
             st_name,bi_button,tx_format,st_serial,st_command,cy_action,
             sl_curbut,bt_addbut,bt_delbut,bt_butleft,bt_butright,bt_del, NULL);
  }
}

/***----------------------------------------------------------------------***/
/*** menut                                                                ***/
/***----------------------------------------------------------------------***/

static void loadset_menu(void)
{
  if (!gl_req) {
    Ask_file(gl_conwindow,&loadset_menu,LOC(MSG_load),"#?.tb",tb_name);
  } else {
    if ( Ask_file_done() == REQ_OK ) {
      EmptyInfoList(&toolbar);
      Toolbar(DT_LOAD,(ULONG)tb_name);
    }
  }
}

static void appset_menu(void)
{
  if (!gl_req) {
    Ask_file(gl_conwindow,&appset_menu,LOC(MSG_append),"#?.tb",tb_name);
  } else {
    if ( Ask_file_done() == REQ_OK ) {
      Toolbar(DT_LOAD,(ULONG)tb_name);
    }
  }
}

static void saveset_menu(void)
{
  if (!gl_req) {
    Ask_file(gl_conwindow,&saveset_menu,LOC(MSG_save),"#?.tb",tb_name);
  } else {
    if ( Ask_file_done() == REQ_OK ) {
      Toolbar(DT_SAVE,(ULONG)tb_name);
    }
  }
}

static void savesel_menu(void)
{
static struct Info  *tb;

  if (!gl_req) {
    if (tb = curtb) {
      if (Ask_file(gl_conwindow,&savesel_menu,LOC(MSG_save_selected),"#?.tb",tb_name)) {
        Toolbar(DT_LOCK,tb);
      }
    }
  } else {
    switch ( Ask_file_done() )
    {
      case REQ_OK:
        SaveInfo(tb,tb_name,TBID);
      case REQ_CANCEL:
        Toolbar(DT_UNLOCK,tb);
    }
  }
}

/***----------------------------------------------------------------------***/
/*** gadgetit                                                             ***/
/***----------------------------------------------------------------------***/

static void tb_click(void)
{
struct Info *newtb;

  get(lv_tb,MUIA_List_Active,&curnum);
  DoMethod(lv_tb,MUIM_List_GetEntry,curnum,&newtb);
  if (newtb != curtb) {
    curtb = newtb;
    nnset(sl_curbut,MUIA_Slider_Level,0);
    Toolbar(DT_UPDATE,0);
  }
}

static void name_click(void)
{
char *name;

  if (curtb) {
    get(st_name,MUIA_String_Contents,&name);
    if ( FindName(&toolbar,name) != curtb ) {
      if ( FindName(&toolbar,name) == 0 ) {
        StrRep(&curtb->node.ln_Name,name);
        gl_toolbarsafe = FALSE;
      }
      DoMethod(lv_tb,MUIM_List_Redraw,curnum);
      DoMethod(lv_tb,MUIM_List_Sort,0);
      MUI_selnode(lv_tb,curtb); /* tb_click() */
      SortList(&toolbar);
      orderfunc();
    }
  }
}

static void button_click(void)
{
ulong num;
static char name[STRSIZE]="Buttons";

  if (!gl_req) {
    Ask_file(gl_conwindow,&button_click,LOC(MSG_select_picture),"#?",name);
  } else {
    if ( Ask_file_done() == REQ_OK ) {
      if (curtb) {
        get(sl_curbut,MUIA_Slider_Level,&num);
        SetDat(FindTag_info(TB_Button,curtb),name,num);
        Toolbar(DT_UPDATE,0);
        gl_toolbarsafe = FALSE;
      }
    }
  }
}

static void serial_click(void)
{
char  *str;
ulong  curbut;

  if (curtb) {
    get(st_serial,MUIA_String_Contents,&str);
    get(sl_curbut,MUIA_Slider_Level   ,&curbut);
    SetDat(FindTag_info(TB_Serial,curtb),str,curbut);
    gl_toolbarsafe = FALSE;
  }
}

static void command_click(void)
{
char  *str;
ulong  curbut;

  if (curtb) {
    get(st_command,MUIA_String_Contents,&str);
    get(sl_curbut ,MUIA_Slider_Level,&curbut);
    SetDat(FindTag_info(TB_Command,curtb),str,curbut);
    gl_toolbarsafe = FALSE;
  }
}

static void action_click(void)
{
ulong curbut,num;

  if (curtb) {
    get(cy_action,MUIA_Cycle_Active,&num);
    get(sl_curbut,MUIA_Slider_Level,&curbut);
    SetDat(FindTag_info(TB_Action,curtb),tb_tags[num],curbut);
    gl_toolbarsafe = FALSE;
  }
}

static void curbut_click(void)
{
  Toolbar(DT_UPDATE,0);
}

static void butleft_click(void)
{
char  oldpos[STRSIZE];
ulong num,oldtag;
struct TagItem *ti;

  get(sl_curbut,MUIA_Slider_Level,&num);
  if (num > 0 AND curtb) {

    ti = FindTag_info(TB_Button,curtb);
    strcpy(oldpos,GetDat(ti,num-1));
    SetDat(ti,GetDat(ti,num),num-1);
    SetDat(ti,oldpos,num);

    ti = FindTag_info(TB_Serial,curtb);
    strcpy(oldpos,GetDat(ti,num-1));
    SetDat(ti,GetDat(ti,num),num-1);
    SetDat(ti,oldpos,num);

    ti = FindTag_info(TB_Command,curtb);
    strcpy(oldpos,GetDat(ti,num-1));
    SetDat(ti,GetDat(ti,num),num-1);
    SetDat(ti,oldpos,num);

    ti = FindTag_info(TB_Action,curtb);
    oldtag = GetDat(ti,num-1);
    SetDat(ti,GetDat(ti,num),num-1);
    SetDat(ti,oldtag,num);

    set(sl_curbut,MUIA_Slider_Level,num-1); /* curbut_click() */
    gl_toolbarsafe = FALSE;
  }
}

static void butright_click(void)
{
char  oldpos[STRSIZE];
ulong num,max,oldtag;
struct TagItem *ti;

  get(sl_curbut,MUIA_Slider_Level,&num);
  get(sl_curbut,MUIA_Slider_Max  ,&max);
  if (num < max AND curtb) {

    ti = FindTag_info(TB_Button,curtb);
    strcpy(oldpos,GetDat(ti,num+1));
    SetDat(ti,GetDat(ti,num),num+1);
    SetDat(ti,oldpos,num);

    ti = FindTag_info(TB_Serial,curtb);
    strcpy(oldpos,GetDat(ti,num+1));
    SetDat(ti,GetDat(ti,num),num+1);
    SetDat(ti,oldpos,num);

    ti = FindTag_info(TB_Command,curtb);
    strcpy(oldpos,GetDat(ti,num+1));
    SetDat(ti,GetDat(ti,num),num+1);
    SetDat(ti,oldpos,num);

    ti = FindTag_info(TB_Action,curtb);
    oldtag = GetDat(ti,num+1);
    SetDat(ti,GetDat(ti,num),num+1);
    SetDat(ti,oldtag,num);

    set(sl_curbut,MUIA_Slider_Level,num+1); /* curbut_click() */
    gl_toolbarsafe = FALSE;
  }
}

static void addbut_click(void)
{
ulong buttons;

  if (curtb) {
    buttons = CountDat(FindTag_info(TB_Button,curtb));
    if (buttons < (TB_Last - ID_bar_base)) {
      SetDat(FindTag_info(TB_Button ,curtb),""      ,buttons);
      SetDat(FindTag_info(TB_Serial ,curtb),""      ,buttons);
      SetDat(FindTag_info(TB_Command,curtb),""      ,buttons);
      SetDat(FindTag_info(TB_Action ,curtb),cmd_none,buttons);
      nnset(sl_curbut ,MUIA_Slider_Min     ,0);
      nnset(sl_curbut ,MUIA_Slider_Max     ,buttons);
      nnset(sl_curbut ,MUIA_Slider_Level   ,buttons);
      Toolbar(DT_UPDATE,0);
      gl_toolbarsafe = FALSE;
    }
  }
}

static void delbut_click(void)
{
ulong buttons,num;

  if (curtb) {
    buttons = CountDat(FindTag_info(TB_Button,curtb));
    get(sl_curbut,MUIA_Slider_Level,&num);
    if ( num < buttons ) {
      DelDat(FindTag_info(TB_Button ,curtb),num);
      DelDat(FindTag_info(TB_Serial ,curtb),num);
      DelDat(FindTag_info(TB_Command,curtb),num);
      DelDat(FindTag_info(TB_Action ,curtb),num);
      if (buttons != 1) {
        nnset(sl_curbut ,MUIA_Slider_Min     ,0);
        nnset(sl_curbut ,MUIA_Slider_Max     ,buttons-2);
      } else {
        nnset(sl_curbut ,MUIA_Slider_Min     ,0);
        nnset(sl_curbut ,MUIA_Slider_Max     ,0);
      }
      Toolbar(DT_UPDATE,0);
      gl_toolbarsafe = FALSE;
    }
  }
}

static void add_click(void)
{
struct Info *tb;

  if ( FindName(&toolbar,LOC(MSG_new_entry)) == 0 ) {
    if ( tb = CreateInfo() ) {
      if ( StrRep(&tb->node.ln_Name,LOC(MSG_new_entry)) ) {
        AddInfoTag(TB_Button ,CreateDat(TB_Button) ,tb);
        AddInfoTag(TB_Serial ,CreateDat(TB_Serial) ,tb);
        AddInfoTag(TB_Command,CreateDat(TB_Command),tb);
        AddInfoTag(TB_Action ,CreateDat(TB_Action) ,tb);
        AddNode_sort(&toolbar,(struct Node *) tb);
        DoMethod(lv_tb,MUIM_List_InsertSingle,tb,MUIV_List_Insert_Sorted);
        MUI_selnode(lv_tb,tb); /* tb_click() */
        gl_toolbarsafe = FALSE;
        orderfunc();
      } else {
        DeleteInfo(tb);
      }
    }
  }
}

static void del_click(void)
{
  if (curtb) {
    if (!curtb->LOCK AND !curtb->NODISK) {
      RemoveInfo(curtb);
      curtb = 0;
      DoMethod(lv_tb,MUIM_List_Remove,curnum);
      gl_toolbarsafe = FALSE;
      orderfunc();
    }
  }
}

/*
 *  Interface pääohjelmaan
 *
 *  TB_ENABLE  [tb] - Open toolbar
 *  TB_DISABLE [tb] - Close toolbar
 * 
 */

ulong Toolbar(ulong tag, ulong data)
{
static bool ocall  = FALSE; static ulong oc ;
static bool ocall2 = FALSE; static ulong oc2;
static char str_run[STRSIZE],*runarg;
BPTR         dos;
aptr         bar_base;
ulong        i,count;
struct Info *tb;

  switch (tag)
  {
    case DT_INIT:
      NewList(&toolbar);
      LocalizeMenu(menu,sizeof(menu)/sizeof(struct NewMenu));
      if ( (tb_bitmap = AllocBitMap(8,8,1,BMF_CLEAR,0)) == 0) return(FALSE);
      if ( (tb_list   = MakeNameList(tb_tags))          == 0) return(FALSE);
      return(TRUE);
      break;
    case DT_DEL:
      Toolbar(TB_DISABLE,0);
      FreePicture(curpic);
      FreeBitMap(tb_bitmap);
      if (tb_list) FreeNameList(tb_list);
      break;
    case DT_CLOSE:
      if (win_act) {
        set(win,MUIA_Window_Open,FALSE);
        if (CUR(PB_Dispose)) {
          DoMethod(gl_app,OM_REMMEMBER,win);
          MUI_DisposeObject(win);
          FreePicture(curpic);
          curpic  = 0;
          win_act = FALSE;
          curtb   = 0;
        }
      }
      break;
    case DT_OPEN:
      if (!win_act) {
        win = WindowObject,
        MUIA_Window_ID    ,MAKE_ID('T','B','W','I'),
        MUIA_Window_Title ,LOC(MSG_tb_toolbar_configuration),
        MUIA_Window_Menu  ,&menu,
        MUIA_HelpNode     ,"help_tb",
          WindowContents, VGroup,
            Child, HGroup,
              Child, VGroup,
                GroupSpacing(0),
                MUIA_Weight,20,
                Child, lv_tb   = ListView(),
                Child, st_name = String(STRSIZE),
              End,
              Child, VGroup,
                Child, VGroup,
                  GroupFrame,
                  Child, bi_group = HGroup,
                    Child, bi_button = BitmapObject,
                      ButtonFrame,
                      MUIA_InputMode     ,MUIV_InputMode_RelVerify,
                      MUIA_Bitmap_Width  ,EMPTY_WIDTH,
                      MUIA_Bitmap_Height ,EMPTY_HEIGHT,
                      MUIA_Bitmap_Bitmap ,tb_bitmap,
                    End,
                  End,
                  Child, ColGroup(2),
                    GroupSpacing(0),
                    Child, Label2L(LOC(MSG_format)), Child, tx_format  = TextObject, TextFrame, MUIA_Background,MUII_TextBack, End,
                    Child, Label2L(LOC(MSG_serial)), Child, st_serial  = String(STRSIZE),
                    Child, Label2L(LOC(MSG_dos))   , Child, st_command = String(STRSIZE),
                    Child, Label2L(LOC(MSG_action)), Child, cy_action  = Cycle(tb_list),
                  End,
                End,
                Child, HGroup,
                  Child, sl_curbut = Slider(0,0),
                  Child, bt_butleft = ImageObject,
                    ButtonFrame,
                    MUIA_InputMode      ,MUIV_InputMode_RelVerify,
                    MUIA_Image_Spec     ,MUII_ArrowLeft,
                  End,
                  Child, bt_butright = ImageObject,
                    ButtonFrame,
                    MUIA_InputMode      ,MUIV_InputMode_RelVerify,
                    MUIA_Image_Spec     ,MUII_ArrowRight,
                  End,
                End,
                Child, HGroup,
                  Child, bt_addbut = MakeButton(LOC(MSG_tb_add)),
                  Child, bt_delbut = MakeButton(LOC(MSG_tb_delete)),
                End,
                Child, RectangleObject,MUIA_Rectangle_HBar,TRUE,MUIA_FixHeight,8,End,
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

          DoMethod(win,MUIM_MultiSet,MUIA_CycleChain,1,
                   lv_tb,st_name,bi_button,st_serial,st_command,cy_action,
                   sl_curbut,bt_butleft,bt_butright,bt_addbut,bt_delbut,bt_add,
                   bt_del,NULL);

          DoMethod(win       ,MUIM_Notify,MUIA_Window_CloseRequest ,TRUE          ,gl_app,2,MUIM_Application_ReturnID        ,ID_quit   );
          DoMethod(lv_tb     ,MUIM_Notify,MUIA_List_Active         ,MUIV_EveryTime,gl_app,2,MUIM_Application_ReturnID        ,ID_tb     );
          DoMethod(lv_tb     ,MUIM_Notify,MUIA_Listview_DoubleClick,TRUE          ,win   ,3,MUIM_Set,MUIA_Window_ActiveObject,st_name   );
          DoMethod(st_name   ,MUIM_Notify,MUIA_String_Acknowledge  ,MUIV_EveryTime,gl_app,2,MUIM_Application_ReturnID        ,ID_name   );
          DoMethod(sl_curbut ,MUIM_Notify,MUIA_Slider_Level        ,MUIV_EveryTime,gl_app,2,MUIM_Application_ReturnID        ,ID_curbut );
          DoMethod(st_serial ,MUIM_Notify,MUIA_String_Contents     ,MUIV_EveryTime,gl_app,2,MUIM_Application_ReturnID        ,ID_serial );
          DoMethod(st_command,MUIM_Notify,MUIA_String_Contents     ,MUIV_EveryTime,gl_app,2,MUIM_Application_ReturnID        ,ID_command);
          DoMethod(cy_action ,MUIM_Notify,MUIA_Cycle_Active        ,MUIV_EveryTime,gl_app,2,MUIM_Application_ReturnID        ,ID_action );

          MultiIDNotify(MUIA_Pressed,FALSE,
                        bi_button   ,ID_button  ,
                        bt_butleft  ,ID_butleft ,
                        bt_butright ,ID_butright,
                        bt_addbut   ,ID_addbut  ,
                        bt_delbut   ,ID_delbut  ,
                        bt_add      ,ID_add     ,
                        bt_del      ,ID_del     , TAG_END);

          InfoListToMui(&toolbar,lv_tb,MUIV_List_Insert_Sorted);
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
      if (bar_win) {
        if (ocall2) {
          set(bar_win,MUIA_Window_Screen,gl_conscreen);
          set(bar_win,MUIA_Window_Open,oc2);
          set(bar_win,MUIA_Window_Sleep,gl_req);
          ocall2 = FALSE;
        } else {
          get(bar_win,MUIA_Window_Open,&oc2);
          set(bar_win,MUIA_Window_Open,FALSE);
          set(bar_win,MUIA_Window_Screen,0);
          ocall2 = TRUE;
        }
      }
      break;
    case DT_LINK:
      orderfunc = data;
      break;
    case DT_FIND:
      return(FindName(&toolbar,(char *)data));
      break;
    case DT_LOAD:
      LoadInfoList(&toolbar,data,TBID);
      InfoListToMui(&toolbar,lv_tb,MUIV_List_Insert_Sorted);
      SortList(&toolbar);
      curtb          = 0;
      gl_toolbarsafe = TRUE;
      orderfunc();
      Toolbar(DT_UPDATE,0);
      break;
    case DT_SAVE:
      SaveInfoList(&toolbar,data,TBID);
      gl_toolbarsafe = TRUE;
      break;
    case DT_HANDLE:
      if (bar_win) { /* button bar */
        if (data >= ID_bar_base AND data <= TB_Last) {
          /* dos */
          dprintf_user(str_run,GetDat(FindTag_info(TB_Command,tb_show),data - ID_bar_base));
          if (str_run[0] != 0) {
            if (runarg = strchr(str_run,' ')) {
              *runarg = 0x00;
              runarg++;
              strcat(runarg,"\n");
            }
            if ( dos = LoadSeg(str_run) ) {
              CreateNewProcTags(NP_Seglist    ,dos             ,
                                NP_Name       ,str_run         ,
                                NP_CommandName,FilePart(str_run),
                                NP_Cli        ,TRUE            ,
                                NP_FreeSeglist,TRUE            ,
                                NP_Arguments  ,runarg          , TAG_END);
            }
          }
          /* serial */
          Send_string(GetDat(FindTag_info(TB_Serial ,tb_show),data - ID_bar_base));
          /* action */
          if (gl_req == 0) { /* too dangerous otherwise, extra check btw! */
            com(GetDat(FindTag_info(TB_Action,tb_show),data - ID_bar_base),0); /* 0 added due DIALNUM */
          }
          break;
        }
      }
      if (win_act) {
        switch (data)
        {
          case ID_tb:       tb_click();      break;
          case ID_name:     name_click();    break;
          case ID_button:   button_click();  break;
          case ID_butleft:  butleft_click(); break;
          case ID_butright: butright_click();break;
          case ID_curbut:   curbut_click();  break;
          case ID_serial:   serial_click();  break;
          case ID_command:  command_click(); break;
          case ID_action:   action_click();  break;
          case ID_addbut:   addbut_click();  break;
          case ID_delbut:   delbut_click();  break;
          case ID_add:      add_click();     break;
          case ID_del:      del_click();     break;
          case ID_loadset:  loadset_menu();  break;
          case ID_saveset:  saveset_menu();  break;
          case ID_appset:   appset_menu();   break;
          case ID_savesel:  savesel_menu();  break;
          case ID_quit:     Toolbar(DT_CLOSE,0); break;
        }
        break;
      }
#ifdef DEBUG
      printf("toolbar: unexpected MUI id\n");
#endif
      break;
    case DT_UPDATE:
      if (win_act) {
      ulong b;

        get(win,MUIA_Window_Open,&b);
        if (b) Update();
      }
      if (bar_win) {
        set(bar_win,MUIA_Window_Sleep,gl_req);
      }
      break;
    case DT_GET:
      return(FindNode_num(&toolbar,data));
      break;
    case DT_LOCK:
      tb = data;
      tb->LOCK = TRUE; 
      if (tb == curtb) Toolbar(DT_UPDATE,0);
      break;
    case DT_UNLOCK:
      tb = data;
      tb->LOCK = FALSE;
      if (tb == curtb) Toolbar(DT_UPDATE,0);
      break;
    case DT_COUNT:
      return(NodeCount(&toolbar));
      break;
    case TB_ENABLE:
      Toolbar(TB_DISABLE,0);
      if (tb_show = data) {
        Toolbar(DT_LOCK,tb_show);
        if (count = CountDat(FindTag_info(TB_Button,tb_show))) {
          if (tb_map = alloc_pool(count*4)) {
            if (tb_obj = alloc_pool(count*4)) {
              for (i = 0; i < count; i++)
              {
                if (tb_map[i] = AllocPicture()) {
                  if (LoadPicture(GetDat(FindTag_info(TB_Button,tb_show),i),tb_map[i],0) != 0) {
                    FreePicture(tb_map[i]);
                    tb_map[i] = 0;
                  }
                }
              }
              if (CUR(PB_Barpos) == BAR_HOR) {
                bar_win = WindowObject,
                MUIA_Window_ID         ,MAKE_ID('B','A','W','I'),
                MUIA_Window_Title      ,tb_show->ln_Name,
                MUIA_HelpNode          ,"help_base",
                MUIA_Window_Width      ,MUIV_Window_Width_Visible(100),
                MUIA_Window_CloseGadget,FALSE,
                MUIA_Window_Activate   ,FALSE,
                  WindowContents, bar_base = HGroup,
                  End,
                End;
              } else {
                bar_win = WindowObject,
                MUIA_Window_ID         ,MAKE_ID('B','A','W','I'),
                MUIA_Window_Title      ,tb_show->ln_Name,
                MUIA_HelpNode          ,"help_base",
                MUIA_Window_Width      ,MUIV_Window_Width_Visible(100),
                MUIA_Window_CloseGadget,FALSE,
                MUIA_Window_Activate   ,FALSE,
                  WindowContents, bar_base = VGroup,
                  End,
                End;
              }
              if (bar_win) {
                for (i = 0; i < count; i++)
                {
                  if (tb_map[i]) {
                    tb_obj[i] = BitmapObject,
                      ButtonFrame,
                      MUIA_InputMode          ,MUIV_InputMode_RelVerify,
                      MUIA_Bitmap_Width       ,tb_map[i]->bmhd.bmh_Width,
                      MUIA_Bitmap_Height      ,tb_map[i]->bmhd.bmh_Height,
                      MUIA_Bitmap_Bitmap      ,tb_map[i]->bmap,
                      MUIA_Bitmap_SourceColors,&tb_map[i]->palette[1],
                      MUIA_FixWidth           ,tb_map[i]->bmhd.bmh_Width,
                      MUIA_FixHeight          ,tb_map[i]->bmhd.bmh_Height,
                      MUIA_Bitmap_UseFriend   ,TRUE,
                      MUIA_Bitmap_Precision   ,PRECISION_IMAGE,
                    End;
                  } else {
                    tb_obj[i] = BitmapObject,
                      ButtonFrame,
                      MUIA_InputMode          ,MUIV_InputMode_RelVerify,
                      MUIA_Bitmap_Width       ,EMPTY_WIDTH,
                      MUIA_Bitmap_Height      ,EMPTY_HEIGHT,
                      MUIA_Bitmap_Bitmap      ,tb_bitmap,
                      MUIA_FixWidth           ,EMPTY_WIDTH,
                      MUIA_FixHeight          ,EMPTY_HEIGHT,
                      MUIA_Bitmap_UseFriend   ,TRUE,
                      MUIA_Bitmap_Precision   ,PRECISION_IMAGE,
                    End;
                  }
                  if (tb_obj[i]) {
                    DoMethod(bar_base,OM_ADDMEMBER,tb_obj[i]);
                    set(tb_obj[i],MUIA_CycleChain,1);
                  }
                }
                for (i = 0; i < count; i++)
                {
                  if (tb_obj[i]) {
                    DoMethod(tb_obj[i],MUIM_Notify,MUIA_Pressed,FALSE,gl_app,2,MUIM_Application_ReturnID,ID_bar_base+i);
                  }
                }
                DoMethod(gl_app,OM_ADDMEMBER,bar_win);
                set(bar_win,MUIA_Window_Screen  ,gl_conscreen);
                set(bar_win,MUIA_Window_Open    ,TRUE);
                set(bar_win,MUIA_Window_Sleep   ,gl_req);
                set(bar_win,MUIA_Window_Activate,FALSE);
                break;
              }
            }
          }
        }
        Toolbar(TB_DISABLE,0);
      }
      break;
    case TB_DISABLE:
      if (tb_show) {
        if (bar_win) {
          set(bar_win,MUIA_Window_Open,FALSE);
          DoMethod(gl_app,OM_REMMEMBER,bar_win);
          MUI_DisposeObject(bar_win);
        }
        if (tb_map) {
          count = CountDat(FindTag_info(TB_Button,tb_show));
          for (i = 0; i < count ; i++)
          {
            FreePicture(tb_map[i]);
          }
        }
        free_pool(tb_map);
        free_pool(tb_obj);
        Toolbar(DT_UNLOCK,tb_show);
        tb_show = 0;
        tb_map  = 0;
        tb_obj  = 0;
        bar_win = 0;
      }
      break;
  }
}
