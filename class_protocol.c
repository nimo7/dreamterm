#include "global.h"
#include "loc_strings.h"

/*
char ver_protocol[]="1.15 (Mar 10 1996)";
*/

static struct MUI_CustomClass *mcc=0;

static char *t_type[] = { (strptr)MSG_xp_xpr, (strptr)MSG_xp_external, 0 };

enum id { ID_loadset = 1,
          ID_saveset      ,ID_appset       ,ID_savesel      ,ID_quit         ,
          ID_lv           ,ID_name         ,ID_type         ,ID_proto        ,
          ID_proto_req    ,ID_init         ,ID_init_req     ,ID_autoup1      ,
          ID_autodn1      ,ID_upcom        ,ID_upcom_req    ,ID_dncom        ,
          ID_dncom_req    ,ID_autoup2      ,ID_autodn2      ,ID_add          ,
          ID_del          };

static struct NewMenu menu[] =
{
  { NM_TITLE,(strptr)MSG_xp_protocol_configuration,  0,0,0,0                },
  { NM_ITEM ,(strptr)MSG_load                     ,"o",0,0,(APTR)ID_loadset },
  { NM_ITEM ,(strptr)MSG_save                     ,"s",0,0,(APTR)ID_saveset },
  { NM_ITEM ,(strptr)MSG_append                   ,"a",0,0,(APTR)ID_appset  },
  { NM_ITEM ,(strptr)MSG_save_selected            ,"c",0,0,(APTR)ID_savesel },
  { NM_ITEM , NM_BARLABEL                         ,  0,0,0,0                },
  { NM_ITEM ,(strptr)MSG_close                    ,"q",0,0,(APTR)ID_quit    },
  { NM_END,0,                                       0,0,0,0                 }
};

struct Data
{
  Object *wi_win         ,*lv_list        ,*st_name        ,*pg_type        ,
         *io_proto       ,*st_proto       ,*io_init        ,*st_init        ,
         *st_autoup1     ,*st_autodn1     ,*io_upcom       ,*st_upcom       ,
         *io_dncom       ,*st_dncom       ,*st_autoup2     ,*st_autodn2     ,
         *bt_add         ,*bt_del         ;
  struct List  info_list;
  struct Info *info;
  void (*cb)(void);
  ulong        cl_proto;
  ulong        cl_upcom;
  ulong        cl_dncom;
  ulong        cl_load;
  ulong        cl_save;
  ulong        cl_append;
  ulong        cl_select;
  char proto_file   [STRSIZE];
  char proto_drawer [STRSIZE];
  char proto_pattern[STRSIZE];
  char com_file     [STRSIZE];
  char com_drawer   [STRSIZE];
  char com_pattern  [STRSIZE];
  char gen_file     [STRSIZE];
  char gen_drawer   [STRSIZE];
  char gen_pattern  [STRSIZE];
};

/***----------------------------------------------------------------------***/
/*** support                                                              ***/
/***----------------------------------------------------------------------***/

/*
 *  Dummy routine for callback
 */
static void dummy(void)
{
}

/*
 * Return path+file from filerequester and copy path/file/pattern
 *
 * req    : asl filerequest where to get data (can be 0 for no-op)
 * file   : file save
 * drawer : drawer save
 * pattern: pattern save
 *
 * return : combined file/drawer string, valid only between calls
 *
 */
static strptr get_filereq(struct FileRequester *req, strptr file, strptr drawer, strptr pattern)
{
static char str[STRSIZE];

  if (req) {
    if (file)    strncpy(file   ,req->fr_File   ,STRSIZE);
    if (drawer)  strncpy(drawer ,req->fr_Drawer ,STRSIZE);
    if (pattern) strncpy(pattern,req->fr_Pattern,STRSIZE);

    strcpy(str,req->fr_Drawer);
    AddPart(str,req->fr_File,STRSIZE);
    return(str);
  }
  return(0);
}

/*
 * Add tags missing in 1.00 - 1.49 version of protocol list
 *
 * list  : list to be corrected
 *
 * return: -
 *
 */
static void fix_list(struct List *list)
{
struct Info *info;

  info = list->lh_Head;
  while (info->ln_Succ)
  {
    if (!FindTag_info(XP_Type ,info)) AddTag_info(XP_Type , 0,info);
    if (!FindTag_info(XP_UpCom,info)) AddTag_info(XP_UpCom,"",info);
    if (!FindTag_info(XP_DnCom,info)) AddTag_info(XP_DnCom,"",info);
    info = info->ln_Succ;
  }
}

/*
 * Update whole crap display
 *
 * data  : data instance
 * info  : info node where to get data
 *
 * return: -
 *
 */
static void update_panel(struct Data *data, struct Info *info)
{
  if (info) {
    nnset(data->st_name   ,MUIA_String_Contents ,info->node.ln_Name);
    nnset(data->pg_type   ,MUIA_Group_ActivePage,GetInfoTag(XP_Type,info));
    nnset(data->st_proto  ,MUIA_String_Contents ,GetInfoTag(XP_XprName,info));
    nnset(data->st_init   ,MUIA_String_Contents ,GetInfoTag(XP_Init   ,info));
    nnset(data->st_autoup1,MUIA_String_Contents ,GetInfoTag(XP_Uauto  ,info));
    nnset(data->st_autodn1,MUIA_String_Contents ,GetInfoTag(XP_Dauto  ,info));

    nnset(data->st_upcom  ,MUIA_String_Contents ,GetInfoTag(XP_UpCom  ,info));
    nnset(data->st_dncom  ,MUIA_String_Contents ,GetInfoTag(XP_DnCom  ,info));
    nnset(data->st_autoup2,MUIA_String_Contents ,GetInfoTag(XP_Uauto  ,info));
    nnset(data->st_autodn2,MUIA_String_Contents ,GetInfoTag(XP_Dauto  ,info));
  } else {
    nnset(data->pg_type   ,MUIA_Group_ActivePage,0);
    nnmultiset(MUIA_String_Contents,
               data->st_name   ,"",
               data->st_proto  ,"",
               data->st_init   ,"",
               data->st_autoup1,"",
               data->st_autodn1,"",
               data->st_upcom  ,"",
               data->st_dncom  ,"",
               data->st_autoup2,"",
               data->st_autoup2,"", TAG_END);
  }
}

/***----------------------------------------------------------------------***/
/*** general methods                                                      ***/
/***----------------------------------------------------------------------***/

static ulong met_new(struct IClass *cl,Object *obj,struct opSet *msg)
{
Object *strip     ,*lv_list   ,*st_name   ,*io_proto,*st_proto,*io_init ,
       *st_init   ,*st_autoup1,*st_autodn1,*bt_add  ,*bt_del  ,*pg_type ,
       *st_autoup2,*st_autodn2,*io_upcom  ,*st_upcom,*io_dncom,*st_dncom;

  if (obj = (Object *)DoSuperNew(cl,obj,
      MUIA_Window_ID       ,MAKE_ID('X','P','W','I'),
      MUIA_Window_Title    ,LOC(MSG_xp_protocol_configuration),
      MUIA_Window_Menustrip,strip = MUI_MakeObject(MUIO_MenustripNM,menu,0),
      MUIA_HelpNode        ,"help_xp",
      MUIA_Window_Width    ,MUIV_Window_Width_Visible(50),
      WindowContents, VGroup,
        Child, VGroup,
          GroupSpacing(0),
          Child, lv_list = ListView(),
          Child, st_name = String(STRSIZE),
        End,
        Child, pg_type = RegisterGroup(t_type),
          Child, ColGroup(2),
            Child, Label2L(LOC(MSG_xp_xpr_library)),
            Child, HGroup,
              GroupSpacing(0),
              Child, io_proto = PopFile(),
              Child, st_proto = String(STRSIZE),
            End,
            Child, Label2L(LOC(MSG_xp_init_string)),
            Child, HGroup,
              GroupSpacing(0),
              Child, io_init = ImageObject,
                ButtonFrame,
                MUIA_Image_Spec     ,MUII_PopUp,
                MUIA_InputMode      ,MUIV_InputMode_RelVerify,
                MUIA_Background     ,MUII_ButtonBack,
                MUIA_Image_FontMatch,TRUE,
              End,
              Child, st_init = String(STRSIZE),
            End,
            Child, Label2L(LOC(MSG_xp_auto_upload))  , Child, st_autoup1 = String(STRSIZE),
            Child, Label2L(LOC(MSG_xp_auto_download)), Child, st_autodn1 = String(STRSIZE),
          End,
          Child, ColGroup(2),
            Child, Label2L(LOC(MSG_xp_upload_dos)),
            Child, HGroup,
              GroupSpacing(0),
              Child, io_upcom = PopFile(),
              Child, st_upcom = String(STRSIZE),
            End,
            Child, Label2L(LOC(MSG_xp_download_dos)),
            Child, HGroup,
              GroupSpacing(0),
              Child, io_dncom = PopFile(),
              Child, st_dncom = String(STRSIZE),
            End,
            Child, Label2L(LOC(MSG_xp_auto_upload))  , Child, st_autoup2 = String(STRSIZE),
            Child, Label2L(LOC(MSG_xp_auto_download)), Child, st_autodn2 = String(STRSIZE),
          End,
        End,
        Child, HGroup,
          GroupSpacing(4),
          Child, bt_add = MakeButton(LOC(MSG_key_add)),
          Child, bt_del = MakeButton(LOC(MSG_key_delete)),
        End,
      End,
      TAG_MORE,msg->ops_AttrList))
  {
    struct Data *data = INST_DATA(cl,obj);

    DoMethod((Object *)DoMethod(strip,MUIM_FindUData,ID_loadset),MUIM_Notify,MUIA_Menuitem_Trigger,MUIV_EveryTime,obj,2,MUIM_INF_Update,ID_loadset);
    DoMethod((Object *)DoMethod(strip,MUIM_FindUData,ID_saveset),MUIM_Notify,MUIA_Menuitem_Trigger,MUIV_EveryTime,obj,2,MUIM_INF_Update,ID_saveset);
    DoMethod((Object *)DoMethod(strip,MUIM_FindUData,ID_appset ),MUIM_Notify,MUIA_Menuitem_Trigger,MUIV_EveryTime,obj,2,MUIM_INF_Update,ID_appset);
    DoMethod((Object *)DoMethod(strip,MUIM_FindUData,ID_savesel),MUIM_Notify,MUIA_Menuitem_Trigger,MUIV_EveryTime,obj,2,MUIM_INF_Update,ID_savesel);
    DoMethod((Object *)DoMethod(strip,MUIM_FindUData,ID_quit   ),MUIM_Notify,MUIA_Menuitem_Trigger,MUIV_EveryTime,obj,3,MUIM_Set,MUIA_Window_Open,FALSE);

    DoMethod(obj       ,MUIM_Notify,MUIA_Window_CloseRequest ,TRUE          , obj       ,3,MUIM_Set,MUIA_Window_Open        ,FALSE);
    DoMethod(lv_list   ,MUIM_Notify,MUIA_List_Active         ,MUIV_EveryTime, obj       ,2,MUIM_INF_Update                  ,ID_lv);
    DoMethod(lv_list   ,MUIM_Notify,MUIA_Listview_DoubleClick,TRUE          , obj       ,3,MUIM_Set,MUIA_Window_ActiveObject,st_name);
    DoMethod(st_name   ,MUIM_Notify,MUIA_String_Acknowledge  ,MUIV_EveryTime, obj       ,2,MUIM_INF_Update                  ,ID_name);
    DoMethod(pg_type   ,MUIM_Notify,MUIA_Group_ActivePage    ,MUIV_EveryTime, obj       ,2,MUIM_INF_Update                  ,ID_type);
    DoMethod(st_proto  ,MUIM_Notify,MUIA_String_Contents     ,MUIV_EveryTime, obj       ,2,MUIM_INF_Update                  ,ID_proto);
    DoMethod(io_proto  ,MUIM_Notify,MUIA_Pressed             ,FALSE         , obj       ,2,MUIM_INF_Update                  ,ID_proto_req);
    DoMethod(st_init   ,MUIM_Notify,MUIA_String_Contents     ,MUIV_EveryTime, obj       ,2,MUIM_INF_Update                  ,ID_init);
    DoMethod(io_init   ,MUIM_Notify,MUIA_Pressed             ,FALSE         , obj       ,2,MUIM_INF_Update                  ,ID_init_req);
    DoMethod(st_autoup1,MUIM_Notify,MUIA_String_Contents     ,MUIV_EveryTime, obj       ,2,MUIM_INF_Update                  ,ID_autoup1);
    DoMethod(st_autodn1,MUIM_Notify,MUIA_String_Contents     ,MUIV_EveryTime, obj       ,2,MUIM_INF_Update                  ,ID_autodn1);
    DoMethod(st_upcom  ,MUIM_Notify,MUIA_String_Contents     ,MUIV_EveryTime, obj       ,2,MUIM_INF_Update                  ,ID_upcom);
    DoMethod(io_upcom  ,MUIM_Notify,MUIA_Pressed             ,FALSE         , obj       ,2,MUIM_INF_Update                  ,ID_upcom_req);
    DoMethod(st_dncom  ,MUIM_Notify,MUIA_String_Contents     ,MUIV_EveryTime, obj       ,2,MUIM_INF_Update                  ,ID_dncom);
    DoMethod(io_dncom  ,MUIM_Notify,MUIA_Pressed             ,FALSE         , obj       ,2,MUIM_INF_Update                  ,ID_dncom_req);
    DoMethod(st_autoup2,MUIM_Notify,MUIA_String_Contents     ,MUIV_EveryTime, obj       ,2,MUIM_INF_Update                  ,ID_autoup2);
    DoMethod(st_autodn2,MUIM_Notify,MUIA_String_Contents     ,MUIV_EveryTime, obj       ,2,MUIM_INF_Update                  ,ID_autodn2);
    DoMethod(bt_add    ,MUIM_Notify,MUIA_Pressed             ,FALSE         , obj       ,2,MUIM_INF_Update                  ,ID_add);
    DoMethod(bt_del    ,MUIM_Notify,MUIA_Pressed             ,FALSE         , obj       ,2,MUIM_INF_Update                  ,ID_del);

    DoMethod(obj,MUIM_MultiSet,MUIA_CycleChain,1,
             lv_list   , st_name   , io_proto, st_proto, io_init , st_init ,
             st_autoup1, st_autodn1, io_upcom, st_upcom, io_dncom, st_dncom,
             st_autoup2, st_autodn2, bt_add  , bt_del  , 0);

    NewList(&data->info_list);
    data->info           = 0;
    data->cb             = &dummy;
    data->cl_proto       = 0;
    data->cl_upcom       = 0;
    data->cl_dncom       = 0;
    data->cl_load        = 0;
    data->cl_save        = 0;
    data->cl_append      = 0;
    data->cl_select      = 0;
    data->proto_file[0]  = 0;
    data->com_file[0]    = 0;
    data->com_drawer[0]  = 0;
    data->com_pattern[0] = 0;
    data->gen_file[0]    = 0;
    data->gen_drawer[0]  = 0;
    strcpy(data->proto_drawer ,"LIBS:");
    strcpy(data->proto_pattern,"xpr#?.library");
    strcpy(data->gen_pattern  ,"#?.xp");

    data->wi_win    = obj;
    data->lv_list   = lv_list;
    data->st_name   = st_name;
    data->pg_type   = pg_type;
    data->io_proto  = io_proto;
    data->st_proto  = st_proto;
    data->io_init   = io_init;
    data->st_init   = st_init;
    data->st_autoup1= st_autoup1;
    data->st_autodn1= st_autodn1;
    data->io_upcom  = io_upcom;
    data->st_upcom  = st_upcom;
    data->io_dncom  = io_dncom;
    data->st_dncom  = st_dncom;
    data->st_autoup2= st_autoup2;
    data->st_autodn2= st_autodn2;
    data->bt_add    = bt_add;
    data->bt_del    = bt_del;

    set(data->pg_type,MUIA_Disabled,TRUE);
    set(data->bt_del ,MUIA_Disabled,TRUE);
  }
  return((ulong) obj);
}

/***----------------------------------------------------------------------***/
/*** class public methods                                                 ***/
/***----------------------------------------------------------------------***/

static ulong met_callback(struct IClass *cl,Object *obj,struct MUIP_INF_Callback *msg)
{
struct Data *data = INST_DATA(cl,obj);

  data->cb = msg->callback;
  return(0);
}

static ulong met_load(struct IClass *cl,Object *obj,struct MUIP_INF_Load *msg)
{
struct Data *data = INST_DATA(cl,obj);

  LoadInfoList (&data->info_list,msg->name,XPID);
  fix_list(&data->info_list);
  SortList     (&data->info_list);
  InfoListToMui(&data->info_list,data->lv_list,MUIV_List_Insert_Sorted);
  data->cb();
  return(0);
}

static ulong met_save(struct IClass *cl,Object *obj,struct MUIP_INF_Save *msg)
{
struct Data *data = INST_DATA(cl,obj);

  SaveInfoList(&data->info_list,msg->name,XPID);
  return(0);
}

static ulong met_find(struct IClass *cl,Object *obj,struct MUIP_INF_Find *msg)
{
struct Data *data = INST_DATA(cl,obj);

  return(FindName(&data->info_list,msg->name));
}

static ulong met_get(struct IClass *cl,Object *obj,struct MUIP_INF_Get *msg)
{
struct Data *data = INST_DATA(cl,obj);

  return(FindNode_num(&data->info_list,msg->num));
}

static ulong met_count(struct IClass *cl,Object *obj,Msg msg)
{
struct Data *data = INST_DATA(cl,obj);

  return(NodeCount(&data->info_list));
}

static ulong met_lock(struct IClass *cl,Object *obj,struct MUIP_INF_Lock *msg)
{
struct Data *data = INST_DATA(cl,obj);

  msg->info->LOCK = TRUE;
  if (data->info) {
    set(data->st_name,MUIA_Disabled,data->info->LOCK);
    set(data->bt_del ,MUIA_Disabled,data->info->LOCK);
  }
  return(0);
}

static ulong met_unlock(struct IClass *cl,Object *obj,struct MUIP_INF_Unlock *msg)
{
struct Data *data = INST_DATA(cl,obj);

  msg->info->LOCK = FALSE;
  if (data->info) {
    set(data->st_name,MUIA_Disabled,data->info->LOCK);
    set(data->bt_del ,MUIA_Disabled,data->info->LOCK);
  }
  return(0);
}

/***----------------------------------------------------------------------***/
/*** class private methods                                                ***/
/***----------------------------------------------------------------------***/

static ulong met_load_req(struct IClass *cl,Object *obj,struct MUIP_INF_File_req *msg)
{
struct Data *data = INST_DATA(cl,obj);
strptr       str;

  if (data->cl_load) {
    if (str = get_filereq(msg->req,data->gen_file,data->gen_drawer,data->gen_pattern)) {
      EmptyInfoList(&data->info_list);
      LoadInfoList (&data->info_list,str,XPID);
      fix_list(&data->info_list);
      SortList     (&data->info_list);
      InfoListToMui(&data->info_list,data->lv_list,MUIV_List_Insert_Sorted);
      data->cb();
    }
    cl_remreq(data->cl_load);
    data->cl_load = 0;
    set(data->wi_win,MUIA_Window_Sleep,FALSE);
  }
  return(0);
}

static ulong met_save_req(struct IClass *cl,Object *obj,struct MUIP_INF_File_req *msg)
{
struct Data *data = INST_DATA(cl,obj);
strptr       str;

  if (data->cl_save) {
    if (str = get_filereq(msg->req,data->gen_file,data->gen_drawer,data->gen_pattern)) {
      SaveInfoList(&data->info_list,str,XPID);
    }
    cl_remreq(data->cl_save);
    data->cl_save = 0;
    set(data->wi_win,MUIA_Window_Sleep,FALSE);
  }
  return(0);
}

static ulong met_append_req(struct IClass *cl,Object *obj,struct MUIP_INF_File_req *msg)
{
struct Data *data = INST_DATA(cl,obj);
strptr       str;

  if (data->cl_append) {
    if (str = get_filereq(msg->req,data->gen_file,data->gen_drawer,data->gen_pattern)) {
      LoadInfoList (&data->info_list,str,XPID);
      fix_list(&data->info_list);
      SortList     (&data->info_list);
      InfoListToMui(&data->info_list,data->lv_list,MUIV_List_Insert_Sorted);
      data->cb();
    }
    cl_remreq(data->cl_append);
    data->cl_append = 0;
    set(data->wi_win,MUIA_Window_Sleep,FALSE);
  }
  return(0);
}

static ulong met_select_req(struct IClass *cl,Object *obj,struct MUIP_INF_File_req *msg)
{
struct Data *data = INST_DATA(cl,obj);
strptr       str;

  if (data->cl_select) {
    if (str = get_filereq(msg->req,data->gen_file,data->gen_drawer,data->gen_pattern)) {
      if (data->info) SaveInfo(data->info,str,XPID);
    }
    cl_remreq(data->cl_select);
    data->cl_select = 0;
    set(data->wi_win,MUIA_Window_Sleep,FALSE);
  }
  return(0);
}

static ulong met_proto_req(struct IClass *cl,Object *obj,struct MUIP_INF_File_req *msg)
{
struct Data *data = INST_DATA(cl,obj);
strptr       str;

  if (data->cl_proto) {
    if (str = get_filereq(msg->req,data->proto_file,data->proto_drawer,data->proto_pattern)) {
      AddTag_info(XP_XprName,str,data->info);
      nnset(data->st_proto,MUIA_String_Contents,str);
    }
    cl_remreq(data->cl_proto);
    data->cl_proto = 0;
    set(data->wi_win,MUIA_Window_Sleep,FALSE);
  }
  return(0);
}

static ulong met_upcom_req(struct IClass *cl,Object *obj,struct MUIP_INF_File_req *msg)
{
struct Data *data = INST_DATA(cl,obj);
strptr       str;

  if (data->cl_upcom) {
    if (str = get_filereq(msg->req,data->com_file,data->com_drawer,data->com_pattern)) {
      AddTag_info(XP_UpCom,str,data->info);
      nnset(data->st_upcom,MUIA_String_Contents,str);
    }
    cl_remreq(data->cl_upcom);
    data->cl_upcom = 0;
    set(data->wi_win,MUIA_Window_Sleep,FALSE);
  }
  return(0);
}

static ulong met_dncom_req(struct IClass *cl,Object *obj,struct MUIP_INF_File_req *msg)
{
struct Data *data = INST_DATA(cl,obj);
strptr       str;

  if (data->cl_dncom) {
    if (str = get_filereq(msg->req,data->com_file,data->com_drawer,data->com_pattern)) {
      AddTag_info(XP_DnCom,str,data->info);
      nnset(data->st_dncom,MUIA_String_Contents,str);
    }
    cl_remreq(data->cl_dncom);
    data->cl_dncom = 0;
    set(data->wi_win,MUIA_Window_Sleep,FALSE);
  }
  return(0);
}

static ulong met_update_pri(struct IClass *cl,Object *obj,struct MUIP_INF_Update *msg)
{
struct Data *data = INST_DATA(cl,obj);
strptr      *name;

  switch (msg->id)
  {
    case ID_loadset:
      if (data->cl_load = cl_addreq(ASL_FileRequest,
                             CL_Object           ,data->wi_win,
                             CL_Method           ,MUIM_XP_Load_req,
                             ASLFR_Window        ,xget(data->wi_win,MUIA_Window_Window),
                             ASLFR_PrivateIDCMP  ,TRUE,
                             ASLFR_TitleText     ,LOC(MSG_load),
                             ASLFR_InitialFile   ,data->gen_file,
                             ASLFR_InitialDrawer ,data->gen_drawer,
                             ASLFR_InitialPattern,data->gen_pattern,
                             ASLFR_DoMultiSelect ,FALSE,
                             ASLFR_DrawersOnly   ,FALSE,
                             ASLFR_DoPatterns    ,TRUE,
                             TAG_END)) {
        set(data->wi_win,MUIA_Window_Sleep,TRUE);
      }
      break;
    case ID_saveset:
      if (data->cl_save = cl_addreq(ASL_FileRequest,
                             CL_Object           ,data->wi_win,
                             CL_Method           ,MUIM_XP_Save_req,
                             ASLFR_Window        ,xget(data->wi_win,MUIA_Window_Window),
                             ASLFR_PrivateIDCMP  ,TRUE,
                             ASLFR_TitleText     ,LOC(MSG_save),
                             ASLFR_InitialFile   ,data->gen_file,
                             ASLFR_InitialDrawer ,data->gen_drawer,
                             ASLFR_InitialPattern,data->gen_pattern,
                             ASLFR_DoMultiSelect ,FALSE,
                             ASLFR_DrawersOnly   ,FALSE,
                             ASLFR_DoPatterns    ,TRUE,
                             ASLFR_DoSaveMode    ,TRUE,
                             TAG_END)) {
        set(data->wi_win,MUIA_Window_Sleep,TRUE);
      }
      break;
    case ID_appset:
      if (data->cl_append = cl_addreq(ASL_FileRequest,
                               CL_Object           ,data->wi_win,
                               CL_Method           ,MUIM_XP_Append_req,
                               ASLFR_Window        ,xget(data->wi_win,MUIA_Window_Window),
                               ASLFR_PrivateIDCMP  ,TRUE,
                               ASLFR_TitleText     ,LOC(MSG_append),
                               ASLFR_InitialFile   ,data->gen_file,
                               ASLFR_InitialDrawer ,data->gen_drawer,
                               ASLFR_InitialPattern,data->gen_pattern,
                               ASLFR_DoMultiSelect ,FALSE,
                               ASLFR_DrawersOnly   ,FALSE,
                               ASLFR_DoPatterns    ,TRUE,
                               TAG_END)) {
        set(data->wi_win,MUIA_Window_Sleep,TRUE);
      }
      break;
    case ID_savesel:
      if (data->info) {
        if (data->cl_select = cl_addreq(ASL_FileRequest,
                                 CL_Object           ,data->wi_win,
                                 CL_Method           ,MUIM_XP_Select_req,
                                 ASLFR_Window        ,xget(data->wi_win,MUIA_Window_Window),
                                 ASLFR_PrivateIDCMP  ,TRUE,
                                 ASLFR_TitleText     ,LOC(MSG_save_selected),
                                 ASLFR_InitialFile   ,data->gen_file,
                                 ASLFR_InitialDrawer ,data->gen_drawer,
                                 ASLFR_InitialPattern,data->gen_pattern,
                                 ASLFR_DoMultiSelect ,FALSE,
                                 ASLFR_DrawersOnly   ,FALSE,
                                 ASLFR_DoPatterns    ,TRUE,
                                 ASLFR_DoSaveMode    ,TRUE,
                                 TAG_END)) {
          set(data->wi_win,MUIA_Window_Sleep,TRUE);
        }
      }
      break;
    case ID_lv:
      if (data->info) data->info->EDIT = FALSE;
      DoMethod(data->lv_list,MUIM_List_GetEntry,xget(data->lv_list,MUIA_List_Active),&data->info);
      if (data->info) {
        set(data->st_name,MUIA_Disabled,data->info->LOCK);
        set(data->pg_type,MUIA_Disabled,FALSE);
        set(data->bt_del ,MUIA_Disabled,data->info->LOCK);
        data->info->EDIT = TRUE;
      } else {
        set(data->st_name,MUIA_Disabled,TRUE);
        set(data->pg_type,MUIA_Disabled,TRUE);
        set(data->bt_del ,MUIA_Disabled,TRUE);
      }
      update_panel(data,data->info);
      break;
    case ID_name:
      if (data->info) {
        get(data->st_name,MUIA_String_Contents,&name);
        if ( FindName(&data->info_list,name) != data->info ) {
          if ( FindName(&data->info_list,name) == 0 ) {
            StrRep(&data->info->node.ln_Name,name);
          }
          SortList(&data->info_list);
          DoMethod(data->lv_list,MUIM_List_Redraw,MUIV_List_Redraw_Active);
          DoMethod(data->lv_list,MUIM_List_Sort,0);
          MUI_selnode(data->lv_list,data->info);
          data->cb();
        }
      }
      break;
    case ID_type:
      if (data->info) {
        AddTag_info(XP_Type,xget(data->pg_type,MUIA_Group_ActivePage),data->info);
      }
      break;
    case ID_proto:
      if (data->info) {
        AddTag_info(XP_XprName,xget(data->st_proto,MUIA_String_Contents),data->info);
      }
      break;
    case ID_proto_req:
      if (data->info) {
        if (data->cl_proto = cl_addreq(ASL_FileRequest,
                                CL_Object           ,data->wi_win,
                                CL_Method           ,MUIM_XP_Proto_req,
                                ASLFR_Window        ,xget(data->wi_win,MUIA_Window_Window),
                                ASLFR_PrivateIDCMP  ,TRUE,
                                ASLFR_TitleText     ,LOC(MSG_xp_select_xpr_protocol_library),
                                ASLFR_InitialFile   ,data->proto_file,
                                ASLFR_InitialDrawer ,data->proto_drawer,
                                ASLFR_InitialPattern,data->proto_pattern,
                                ASLFR_DoMultiSelect ,FALSE,
                                ASLFR_DrawersOnly   ,FALSE,
                                ASLFR_DoPatterns    ,TRUE,
                                TAG_END)) {
          set(data->wi_win,MUIA_Window_Sleep,TRUE);
        }
      }
      break;
    case ID_init:
      if (data->info) {
        AddTag_info(XP_Init,xget(data->st_init,MUIA_String_Contents),data->info);
      }
      break;
    case ID_init_req:
      if (data->info) {
      ulong p;

        set(data->wi_win,MUIA_Window_Sleep,TRUE);
        if (p = Transfer(FT_SETUP,data->info)) { /* this is sucky way */
          AddTag_info(XP_Init,p,data->info);
          nnset(data->st_init,MUIA_String_Contents,p);
        }
        set(data->wi_win,MUIA_Window_Sleep,FALSE);
      }
      break;
    case ID_autoup1:
      if (data->info) {
        AddTag_info(XP_Uauto,xget(data->st_autoup1,MUIA_String_Contents),data->info);
      }
      break;
    case ID_autodn1:
      if (data->info) {
        AddTag_info(XP_Dauto,xget(data->st_autodn1,MUIA_String_Contents),data->info);
      }
      break;
    case ID_upcom:
      if (data->info) {
        AddTag_info(XP_UpCom,xget(data->st_upcom,MUIA_String_Contents),data->info);
      }
      break;
    case ID_upcom_req:
      if (data->info) {
        if (data->cl_upcom = cl_addreq(ASL_FileRequest,
                                CL_Object           ,data->wi_win,
                                CL_Method           ,MUIM_XP_Upcom_req,
                                ASLFR_Window        ,xget(data->wi_win,MUIA_Window_Window),
                                ASLFR_PrivateIDCMP  ,TRUE,
                                ASLFR_TitleText     ,LOC(MSG_xp_upload_dos),
                                ASLFR_InitialFile   ,data->com_file,
                                ASLFR_InitialDrawer ,data->com_drawer,
                                ASLFR_InitialPattern,data->com_pattern,
                                ASLFR_DoMultiSelect ,FALSE,
                                ASLFR_DrawersOnly   ,FALSE,
                                ASLFR_DoPatterns    ,TRUE,
                                TAG_END)) {
          set(data->wi_win,MUIA_Window_Sleep,TRUE);
        }
      }
      break;
    case ID_dncom:
      if (data->info) {
        AddTag_info(XP_DnCom,xget(data->st_dncom,MUIA_String_Contents),data->info);
      }
      break;
    case ID_dncom_req:
      if (data->info) {
        if (data->cl_dncom = cl_addreq(ASL_FileRequest,
                                CL_Object           ,data->wi_win,
                                CL_Method           ,MUIM_XP_Dncom_req,
                                ASLFR_Window        ,xget(data->wi_win,MUIA_Window_Window),
                                ASLFR_PrivateIDCMP  ,TRUE,
                                ASLFR_TitleText     ,LOC(MSG_xp_download_dos),
                                ASLFR_InitialFile   ,data->com_file,
                                ASLFR_InitialDrawer ,data->com_drawer,
                                ASLFR_InitialPattern,data->com_pattern,
                                ASLFR_DoMultiSelect ,FALSE,
                                ASLFR_DrawersOnly   ,FALSE,
                                ASLFR_DoPatterns    ,TRUE,
                                TAG_END)) {
          set(data->wi_win,MUIA_Window_Sleep,TRUE);
        }
      }
      break;
    case ID_autoup2:
      if (data->info) {
        AddTag_info(XP_Uauto,xget(data->st_autoup2,MUIA_String_Contents),data->info);
      }
      break;
    case ID_autodn2:
      if (data->info) {
        AddTag_info(XP_Dauto,xget(data->st_autodn2,MUIA_String_Contents),data->info);
      }
      break;
    case ID_add:
      if ( FindName(&data->info_list,LOC(MSG_new_entry)) == 0 ) {
      struct Info *xp;

        if ( xp = CreateInfo() ) {
          if ( StrRep(&xp->node.ln_Name,LOC(MSG_new_entry)) ) {
            AddInfoTags(xp,
                        XP_XprName,XPXprName,
                        XP_Init   ,XPInit   ,
                        XP_Uauto  ,XPUauto  ,
                        XP_Dauto  ,XPDauto  ,
                        XP_Type   ,0        ,
                        XP_UpCom  ,""       ,
                        XP_DnCom  ,""       , TAG_END);
            AddNode_sort(&data->info_list,(struct Node *) xp);
            DoMethod(data->lv_list,MUIM_List_InsertSingle,xp,MUIV_List_Insert_Sorted);
            MUI_selnode(data->lv_list,xp);
            data->cb();
          } else {
            DeleteInfo(xp);
          }
        }
      }
      break;
    case ID_del:
      if (data->info) {
        if (!data->info->LOCK) {
          RemoveInfo(data->info);
          data->info = 0;
          DoMethod(data->lv_list,MUIM_List_Remove,xget(data->lv_list,MUIA_List_Active));
          data->cb();
        }
      }
      break;
  }
  return(0);
}

/***----------------------------------------------------------------------***/
/*** dispatcher                                                           ***/
/***----------------------------------------------------------------------***/

static __saveds __asm ulong Dispatcher(REG __a0 struct IClass *cl,REG __a2 Object *obj,REG __a1 Msg msg)
{
  switch (msg->MethodID)
  {
    case OM_NEW           : return(met_new       (cl,obj,(APTR)msg));

    case MUIM_INF_Callback: return(met_callback(cl,obj,(APTR)msg));
    case MUIM_INF_Load    : return(met_load      (cl,obj,(APTR)msg));
    case MUIM_INF_Save    : return(met_save      (cl,obj,(APTR)msg));
    case MUIM_INF_Find    : return(met_find      (cl,obj,(APTR)msg));
    case MUIM_INF_Get     : return(met_get       (cl,obj,(APTR)msg));
    case MUIM_INF_Count   : return(met_count     (cl,obj,(APTR)msg));
    case MUIM_INF_Lock    : return(met_lock      (cl,obj,(APTR)msg));
    case MUIM_INF_Unlock  : return(met_unlock    (cl,obj,(APTR)msg));
    case MUIM_INF_Update  : return(met_update_pri(cl,obj,(APTR)msg));

    case MUIM_XP_Proto_req : return(met_proto_req (cl,obj,(APTR)msg));
    case MUIM_XP_Upcom_req : return(met_upcom_req (cl,obj,(APTR)msg));
    case MUIM_XP_Dncom_req : return(met_dncom_req (cl,obj,(APTR)msg));
    case MUIM_XP_Load_req  : return(met_load_req  (cl,obj,(APTR)msg));
    case MUIM_XP_Save_req  : return(met_save_req  (cl,obj,(APTR)msg));
    case MUIM_XP_Append_req: return(met_append_req(cl,obj,(APTR)msg));
    case MUIM_XP_Select_req: return(met_select_req(cl,obj,(APTR)msg));
  }
  return(TempDoSuperMethodA(cl,obj,msg));
}

/***----------------------------------------------------------------------***/
/*** init/delete                                                          ***/
/***----------------------------------------------------------------------***/

void DelClass_protocol(void)
{
  if (mcc) {
    MUI_DeleteCustomClass(mcc);
    mcc = 0;
  }
}

struct IClass *AddClass_protocol(void)
{
  if (mcc = MUI_CreateCustomClass(NULL,MUIC_Window,NULL,sizeof(struct Data),Dispatcher)) {
    LocalizeList(t_type,sizeof(t_type)/4);
    LocalizeMenu(menu  ,sizeof(menu)/sizeof(struct NewMenu));
    return(mcc->mcc_Class);
  } else {
    return(0);
  }
}
