#include "global.h"
#include "loc_strings.h"

/*
char ver_review[]="1.01 (Mar 08 1996)";
*/

enum ID { ID_search = 1, ID_save };

static struct MUI_CustomClass *mcc=0;

static struct NewMenu menu[] =
{
  { NM_TITLE ,(strptr)MSG_review  ,  0,0,0,0               },
/*
  { NM_ITEM  ,(strptr)MSG_search  ,"h",0,0,(APTR)ID_search },
*/
  { NM_ITEM  ,(strptr)MSG_save    ,"s",0,0,(APTR)ID_save   },
  { NM_END,0,0,0,0,0 }
};

struct Data
{
  Object *wi_window;
  Object *rv_revarea;
  Object *sb_bar;
/*
  Object *mi_search;
*/
  Object *mi_save;
  ulong   cl_save;
  char file   [STRSIZE];
  char drawer [STRSIZE];
  char pattern[STRSIZE];
};

/***----------------------------------------------------------------------***/
/*** general methods                                                      ***/
/***----------------------------------------------------------------------***/

static ulong met_new(struct IClass *cl,Object *obj,struct opSet *msg)
{
Object *rv_revarea,*sb_bar,*strip;

  if (obj = (Object *)DoSuperNew(cl,obj,
      MUIA_Window_ID       , MAKE_ID('R','V','W','I'),
      MUIA_Window_Menustrip, strip = MUI_MakeObject(MUIO_MenustripNM,menu,0),
      MUIA_HelpNode        , "help_rv",
      WindowContents, VGroup,
        Child, HGroup,
          GroupSpacing(0),
          Child, rv_revarea = NewObject(gl_class_revarea,NULL,
            TextFrame,
            MUIA_Background, MUII_BACKGROUND,
          TAG_DONE),
          Child, sb_bar = ScrollbarObject, End,
        End,
      End,
      TAG_MORE,msg->ops_AttrList))
  {
    struct Data *data = INST_DATA(cl,obj);

    DoMethod(obj       ,MUIM_Notify,MUIA_Window_CloseRequest,TRUE          , obj       ,3,MUIM_Set,MUIA_Window_Open ,FALSE);
    DoMethod(rv_revarea,MUIM_Notify,MUIA_REV_Lines          ,MUIV_EveryTime, sb_bar    ,3,MUIM_Set,MUIA_Prop_Entries,MUIV_TriggerValue);
    DoMethod(rv_revarea,MUIM_Notify,MUIA_REV_ShowLine       ,MUIV_EveryTime, sb_bar    ,3,MUIM_Set,MUIA_Prop_First  ,MUIV_TriggerValue);
    DoMethod(rv_revarea,MUIM_Notify,MUIA_REV_DispLines      ,MUIV_EveryTime, sb_bar    ,3,MUIM_Set,MUIA_Prop_Visible,MUIV_TriggerValue);
    DoMethod(sb_bar    ,MUIM_Notify,MUIA_Prop_First         ,MUIV_EveryTime, rv_revarea,3,MUIM_Set,MUIA_REV_ShowLine,MUIV_TriggerValue);
    DoMethod(rv_revarea,MUIM_Notify,MUIA_REV_Lines          ,MUIV_EveryTime, obj       ,1,MUIM_RVI_Update);
    DoMethod(rv_revarea,MUIM_Notify,MUIA_REV_ShowLine       ,MUIV_EveryTime, obj       ,1,MUIM_RVI_Update);
    DoMethod(rv_revarea,MUIM_Notify,MUIA_REV_MaxLines       ,MUIV_EveryTime, obj       ,1,MUIM_RVI_Update);

/*
    DoMethod(data->mi_search = (Object *)DoMethod(strip,MUIM_FindUData,ID_search),MUIM_Notify,MUIA_Menuitem_Trigger,MUIV_EveryTime,obj,1,MUIM_RVI_Search);
*/
    DoMethod(data->mi_save   = (Object *)DoMethod(strip,MUIM_FindUData,ID_save  ),MUIM_Notify,MUIA_Menuitem_Trigger,MUIV_EveryTime,obj,1,MUIM_RVI_Save);

    data->wi_window  = obj;
    data->rv_revarea = rv_revarea;
    data->sb_bar     = sb_bar;

    data->file[0]    = 0;
    data->drawer[0]  = 0;
    data->pattern[0] = 0;

    set(data->rv_revarea,MUIA_CycleChain,1);
    set(obj,MUIA_Window_ActiveObject,data->rv_revarea);
  }
  return((ulong) obj);
}

/***----------------------------------------------------------------------***/
/*** class private methods                                                ***/
/***----------------------------------------------------------------------***/

static ulong met_save_req(struct IClass *cl,Object *obj,struct MUIP_RVI_Save_req *msg)
{
struct Data *data = INST_DATA(cl,obj);
char str[STRSIZE];

  if (data->cl_save) {
    if (msg->req) {
      strncpy(data->file   ,msg->req->fr_File   ,STRSIZE);
      strncpy(data->drawer ,msg->req->fr_Drawer ,STRSIZE);
      strncpy(data->pattern,msg->req->fr_Pattern,STRSIZE);

      strcpy(str,data->drawer);
      AddPart(str,data->file,STRSIZE);
      DoMethod(data->rv_revarea,MUIM_REV_Save,str);
    }
    cl_remreq(data->cl_save);
    set(data->mi_save,MUIA_Menuitem_Enabled,TRUE);
    data->cl_save = 0;
  }
  return(0);
}

static ulong met_save(struct IClass *cl,Object *obj,Msg msg)
{
struct Data *data = INST_DATA(cl,obj);

  if (data->cl_save = cl_addreq(ASL_FileRequest,
                                CL_Object           ,data->wi_window,
                                CL_Method           ,MUIM_RVI_Save_req,
                                ASLFR_Window        ,xget(data->wi_window,MUIA_Window_Window),
                                ASLFR_PrivateIDCMP  ,TRUE,
                                ASLFR_TitleText     ,LOC(MSG_save),
                                ASLFR_InitialFile   ,data->file,
                                ASLFR_InitialDrawer ,data->drawer,
                                ASLFR_InitialPattern,data->pattern,
                                ASLFR_DoMultiSelect ,FALSE,
                                ASLFR_DrawersOnly   ,FALSE,
                                ASLFR_DoPatterns    ,TRUE,
                                ASLFR_DoSaveMode    ,TRUE,
                                TAG_END)) {
    set(data->mi_save,MUIA_Menuitem_Enabled,FALSE);
  }
  return(0);
}

/*
static ulong met_search_req(struct IClass *cl,Object *obj,Msg msg)
{
struct Data *data = INST_DATA(cl,obj);

/*
  DoMethod(data->rv_revarea,MUIM_REV_Search,xget(data->st_search,MUIA_String_Contents));
*/
  return(0);
}

static ulong met_search(struct IClass *cl,Object *obj,Msg msg)
{
struct Data *data = INST_DATA(cl,obj);

  return(0);
}
*/

static ulong met_update(struct IClass *cl,Object *obj,Msg msg)
{
struct Data *data = INST_DATA(cl,obj);
static char title[STRSIZE];

  sprintf(title,"%ld/%ld of %ld (col: %ld , mem: %ld)",
          xget(data->rv_revarea,MUIA_REV_ShowLine)+1,
          xget(data->rv_revarea,MUIA_REV_Lines),
          xget(data->rv_revarea,MUIA_REV_MaxLines),
          xget(data->rv_revarea,MUIA_REV_ShowCol),
          xget(data->rv_revarea,MUIA_REV_MemoryUsed));
  set(data->wi_window,MUIA_Window_Title,title);
  return(0);
}

/***----------------------------------------------------------------------***/
/*** dispatcher                                                           ***/
/***----------------------------------------------------------------------***/

static __saveds __asm ulong Dispatcher(REG __a0 struct IClass *cl,REG __a2 Object *obj,REG __a1 Msg msg)
{
  switch (msg->MethodID)
  {
    case OM_NEW             : return(met_new       (cl,obj,(APTR)msg));

/*
    case MUIM_RVI_Search    : return(met_search    (cl,obj,(APTR)msg));
    case MUIM_RVI_Search_req: return(met_search_req(cl,obj,(APTR)msg));
*/
    case MUIM_RVI_Save      : return(met_save      (cl,obj,(APTR)msg));
    case MUIM_RVI_Save_req  : return(met_save_req  (cl,obj,(APTR)msg));
    case MUIM_RVI_Update    : return(met_update    (cl,obj,(APTR)msg));

    /* these are just passed to revarea to allow treating review as
     * revarea class from outside.
     */
    case OM_SET: /* I wonder if this is legal */
    {
    struct Data *data = INST_DATA(cl,obj);

      TempDoMethodA(data->rv_revarea,msg);
    }
    break;
    case MUIM_REV_FeedData:
    case MUIM_REV_Clear   :
    {
    struct Data *data = INST_DATA(cl,obj);

      return(TempDoMethodA(data->rv_revarea,msg));
    }
  }
  return(TempDoSuperMethodA(cl,obj,msg));
}

/***----------------------------------------------------------------------***/
/*** init/delete                                                          ***/
/***----------------------------------------------------------------------***/

void DelClass_review(void)
{
  if (mcc) {
    MUI_DeleteCustomClass(mcc);
    mcc = 0;
  }
}

struct IClass *AddClass_review(void)
{
  if (mcc = MUI_CreateCustomClass(NULL,MUIC_Window,NULL,sizeof(struct Data),Dispatcher)) {
    LocalizeMenu(menu,sizeof(menu)/sizeof(struct NewMenu));
    return(mcc->mcc_Class);
  } else {
    return(0);
  }
}
