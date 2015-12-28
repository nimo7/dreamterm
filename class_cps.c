#include "global.h"

/*
char ver_class_cps[]="1.02 (Jan 06 1996)";
*/

#define SPEED 2

static struct MUI_CustomClass *mcc=0;

struct Data
{
  ulong max,var_last,var_new;
};

/***----------------------------------------------------------------------***/
/*** general methods                                                      ***/
/***----------------------------------------------------------------------***/

static ulong met_new(struct IClass *cl,Object *obj,Msg msg)
{
struct Data *data;

  if (!(obj = (Object *)TempDoSuperMethodA(cl,obj,msg))) return(0);
  data = INST_DATA(cl,obj);
  data->max      = GetTagData(MUIA_CPS_Max,1,((struct opSet *)msg)->ops_AttrList);
  data->var_last = 0;
  data->var_new  = 0;
  return((ulong)obj);
}

static ulong met_set(struct IClass *cl,Object *obj,Msg msg)
{
struct Data *data = INST_DATA(cl,obj);
struct TagItem *tags,*tag;

  for (tags=((struct opSet *)msg)->ops_AttrList;tag=NextTagItem(&tags);)
  {
    switch (tag->ti_Tag)
    {
      case MUIA_CPS_Max:
        data->max = tag->ti_Data;
        if (data->max < data->var_last) data->var_last = data->max;
        MUI_Redraw(obj,MADF_DRAWOBJECT);
        break;
    }
  }

  return(TempDoSuperMethodA(cl,obj,msg));
}

static ulong met_askminmax(struct IClass *cl,Object *obj,struct MUIP_AskMinMax *msg)
{
  TempDoSuperMethodA(cl,obj,msg);

  msg->MinMaxInfo->MinWidth  += SPEED;
  msg->MinMaxInfo->DefWidth  += 512;
  msg->MinMaxInfo->MaxWidth  += MUI_MAXMAX;

  msg->MinMaxInfo->MinHeight += SPEED;
  msg->MinMaxInfo->DefHeight += 512;
  msg->MinMaxInfo->MaxHeight += MUI_MAXMAX;

  return(0);
}

static ulong met_draw(struct IClass *cl,Object *obj,struct MUIP_Draw *msg)
{
struct Data *data = INST_DATA(cl,obj);
long wx,wy,wh,ww;

  TempDoSuperMethodA(cl,obj,msg);

  if (msg->flags & MADF_DRAWUPDATE)
  {
    wx = _mleft(obj);
    wy = _mtop(obj);
    ww = _mwidth(obj);
    wh = _mheight(obj);

    SetAPen(_rp(obj),_dri(obj)->dri_Pens[TEXTPEN]);
    ScrollRaster(_rp(obj),SPEED,0,wx,wy,wx+ww,wy+wh);
    Move(_rp(obj),wx+ww-1-SPEED,wy + wh - (((data->var_last*wh)/data->max)));
    Draw(_rp(obj),wx+ww-1      ,wy + wh - (((data->var_new *wh)/data->max)));
  }

/* total redraw using memory
  if (msg->flags & MADF_DRAWOBJECT) {
  }
*/

  return(0);
}

/***----------------------------------------------------------------------***/
/*** class public methods                                                 ***/
/***----------------------------------------------------------------------***/

static ulong met_cps(struct IClass *cl,Object *obj,struct MUIP_CPS_Cps *msg)
{
struct Data *data = INST_DATA(cl,obj);

  data->var_last = data->var_new;
  data->var_new  = (msg->cps <= data->max) ? msg->cps : data->max;
  MUI_Redraw(obj,MADF_DRAWUPDATE);
  return(0);
}

/***----------------------------------------------------------------------***/
/*** dispatcher                                                           ***/
/***----------------------------------------------------------------------***/

static __saveds __asm ulong Dispatcher(REG __a0 struct IClass *cl,REG __a2 Object *obj,REG __a1 Msg msg)
{
  switch (msg->MethodID)
  {
    case OM_NEW        : return(met_new      (cl,obj,(APTR)msg));
    case OM_SET        : return(met_set      (cl,obj,(APTR)msg));
    case MUIM_AskMinMax: return(met_askminmax(cl,obj,(APTR)msg));
    case MUIM_Draw     : return(met_draw     (cl,obj,(APTR)msg));

    case MUIM_CPS_Cps  : return(met_cps      (cl,obj,(APTR)msg));
  }
  return(TempDoSuperMethodA(cl,obj,msg));
}

/***----------------------------------------------------------------------***/
/*** init/delete                                                          ***/
/***----------------------------------------------------------------------***/

void DelClass_cps(void)
{
  if (mcc) {
    MUI_DeleteCustomClass(mcc);
    mcc = 0;
  }
}

struct IClass *AddClass_cps(void)
{
  if (mcc = MUI_CreateCustomClass(NULL,MUIC_Area,NULL,sizeof(struct Data),Dispatcher)) {
    return(mcc->mcc_Class);
  } else {
    return(0);
  }
}
