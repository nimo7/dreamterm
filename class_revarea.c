#include "global.h"

/*
char ver_class_revarea[]="1.01 (Mar 08 1996)";
*/

#define MAXLINE 512       /* maximum line size */

struct Node_line          /* this will waste memory... who cares :) */
{
  struct Node_line *succ; /* actually minimal node */
  struct Node_line *pred;
  ulong             size; /* linesize              */
                          /* line starts here      */
};

struct Data
{
  long   lines;         /* lines in buffer             */
  long   displines;     /* lines in display (if on!)   */
  long   dispcols;      /* columns in display (if on!) */
  long   memoryused;    /* memory used                 */
  long   showline;      /* first line to display       */
  long   showcol;       /* first column to display     */ /* not ready */
  long   colorfg;       /* font fg color               */
  long   colorbg;       /* font bg color               */
  long   maxlines;      /* lines to buffer             */
  strptr virtual;       /* virtual disk file           */ /* not ready */

  struct List linebuf;  /* line data                   */
  strptr tmp;           /* one line buffer             */
  bool   tmpsize;
  bool   notify;        /* dummy set                   */

  /* data for draw method (update only) */
  long   scroll;        /* scroll amount               */
  long   offset;        /* which line to start print   */
};

static struct MUI_CustomClass *mcc=0;

/***----------------------------------------------------------------------***/
/*** support (offers way to change store method easily)                   ***/
/***----------------------------------------------------------------------***/

/*
 * Delete first line
 *
 * data  : class data
 *
 * return: true/false
 *
 */
static bool del_line(struct Data *data)
{
struct Node_line *node;

  if (node = RemHead(&data->linebuf)) {
    data->memoryused -= sizeof(struct Node_line) + node->size;
    FreeMem(node,sizeof(struct Node_line) + node->size);
    data->lines--;
    return(TRUE);
  }
  return(FALSE);
}

/*
 * Add new line to end of list
 *
 * data  : class data
 * line  : line to add (don't assume null end)
 * size  : line size
 *
 * return: true/false
 *
 */
static bool add_line(struct Data *data, strptr line, long size)
{
struct Node_line *node;

  if (node = AllocMem(sizeof(struct Node_line) + size,MEMF_ANY)) {
    strncpy(&node[1],line,size);
    node->size = size;
    AddTail(&data->linebuf,node);
    data->memoryused += node->size + sizeof(struct Node_line);
    data->lines++;
    return(TRUE);
  }
  return(FALSE);
}

/*
 * Find node for line number (this could use some hashing)
 *
 * data  : class data
 * num   : line number 0-n
 *
 * return: node, 0 for not finding
 *
 */
static __inline struct Node_line *get_line(struct Data *data, ulong num)
{
  return(FindNode_num(&data->linebuf,num));
}

/*
 * Find next line
 *
 * data  : class data
 * node  : node
 *
 * return: node, 0 for not finding
 *
 */
static __inline struct Node_line *next_line(struct Data *data, struct Node_line *node)
{
  return(GetSucc(node));
}

/*
 * Find previous line
 *
 * data  : class data
 * node  : node
 *
 * return: node, 0 for not finding
 *
 */
static __inline struct Node_line *prev_line(struct Data *data, struct Node_line *node)
{
  return(GetPred(node));
}

/*
 * Fake set values using SetAttribute after changing data->xx in various
 * methods. This will allow notifys to work.
 *
 * data  : class data
 * obj   : this object
 *
 * return: -
 *
 */
static void set_notify(struct Data *data,Object *obj)
{
  data->notify = TRUE;
  set(obj,MUIA_REV_Lines     ,data->lines);
  set(obj,MUIA_REV_DispLines ,data->displines);
  set(obj,MUIA_REV_DispCols  ,data->dispcols);
  set(obj,MUIA_REV_ShowLine  ,data->showline);
  set(obj,MUIA_REV_ShowCol   ,data->showcol);
  set(obj,MUIA_REV_MemoryUsed,data->memoryused);
  data->notify = FALSE;
}

/***----------------------------------------------------------------------***/
/*** general methods                                                      ***/
/***----------------------------------------------------------------------***/

static ulong met_new(struct IClass *cl,Object *obj,Msg msg)
{
struct Data *data;
strptr p;

  if (obj = (Object *) TempDoSuperMethodA(cl,obj,msg)) {
    data = INST_DATA(cl,obj);

    NewList(&data->linebuf);
    data->lines     = 0;
    data->displines = 0;
    data->dispcols  = 0;
    data->memoryused= 0;
    data->showline  = GetTagData(MUIA_REV_ShowLine,0            , ((struct opSet *)msg)->ops_AttrList);
    data->showcol   = GetTagData(MUIA_REV_ShowCol ,0            , ((struct opSet *)msg)->ops_AttrList);
    data->colorfg   = GetTagData(MUIA_REV_ColorFG ,TEXTPEN      , ((struct opSet *)msg)->ops_AttrList);
    data->colorbg   = GetTagData(MUIA_REV_ColorBG ,BACKGROUNDPEN, ((struct opSet *)msg)->ops_AttrList);
    data->maxlines  = GetTagData(MUIA_REV_MaxLines,1000         , ((struct opSet *)msg)->ops_AttrList);
    p               = GetTagData(MUIA_REV_Virtual ,""           , ((struct opSet *)msg)->ops_AttrList);
    if (strlen(p)) {
      if (data->virtual = AllocVec(strlen(p)+1,MEMF_ANY)) strcpy(data->virtual,p);
    } else {
      data->virtual = 0;
    }
    if ((data->tmp = AllocVec(MAXLINE,MEMF_ANY)) == 0) {
      CoerceMethod(cl,obj,OM_DISPOSE);
      return(0);
    }
    data->tmpsize = 0;
  }
  return((ulong) obj);
}

static ulong met_dispose(struct IClass *cl,Object *obj,Msg msg)
{
struct Data *data = INST_DATA(cl,obj);

  while (del_line(data)) {}
  if (data->virtual) FreeVec(data->virtual);
  if (data->tmp    ) FreeVec(data->tmp);

  return(TempDoSuperMethodA(cl,obj,msg));
}

static ulong met_set(struct IClass *cl,Object *obj,Msg msg)
{
struct Data    *data = INST_DATA(cl,obj);
struct TagItem *tags,*tag;

  if (!data->notify) {
    for (tags=((struct opSet *)msg)->ops_AttrList;tag=NextTagItem(&tags);)
    {
      switch (tag->ti_Tag) /* remember: display isn't always on, so don't crash */
      {                    /*           if disp values are out of range         */
        case MUIA_REV_ShowLine:
          if (data->lines > tag->ti_Data) {
            data->scroll   = tag->ti_Data - data->showline;
            data->offset   = (data->scroll > 0) ? data->displines - abs(data->scroll) : 0;
            data->showline = tag->ti_Data;
            if (abs(data->scroll) < data->displines) {
              MUI_Redraw(obj,MADF_DRAWUPDATE);
            } else if (data->scroll) {
              MUI_Redraw(obj,MADF_DRAWOBJECT);
            }
          }
          break;
        case MUIA_REV_ShowCol:
          if (tag->ti_Data > 0 AND tag->ti_Data < 256) {
            data->showcol = tag->ti_Data;
            MUI_Redraw(obj,MADF_DRAWOBJECT);
          }
          break;
        case MUIA_REV_ColorFG:
          data->colorfg = tag->ti_Data;
          MUI_Redraw(obj,MADF_DRAWOBJECT);
          break;
        case MUIA_REV_ColorBG:
          data->colorbg = tag->ti_Data;
          MUI_Redraw(obj,MADF_DRAWOBJECT);
          break;
        case MUIA_REV_MaxLines:
          data->maxlines = tag->ti_Data;
          for (;data->lines > data->maxlines;)
          {
            if (del_line(data)) data->showline--;
          }
          if (data->showline < 0) {
            data->scroll   = abs(data->showline);
            data->offset   = data->displines - abs(data->scroll);
            data->showline = 0;
            if (abs(data->scroll) < data->displines) {
              MUI_Redraw(obj,MADF_DRAWUPDATE);
            } else {
              MUI_Redraw(obj,MADF_DRAWOBJECT);
            }
          }
          break;
        case MUIA_REV_Virtual:
          /* not ready */
          break;
      }
    }
  }
  return(TempDoSuperMethodA(cl,obj,msg));
}

static ulong met_get(struct IClass *cl,Object *obj,Msg msg)
{
struct Data *data  = INST_DATA(cl,obj);
ulong       *store = ((struct opGet *)msg)->opg_Storage;

  switch (((struct opGet *)msg)->opg_AttrID)
  {
    case MUIA_REV_Lines     : *store = data->lines;      return(TRUE);
    case MUIA_REV_DispLines : *store = data->displines;  return(TRUE);
    case MUIA_REV_DispCols  : *store = data->dispcols;   return(TRUE);
    case MUIA_REV_MemoryUsed: *store = data->memoryused; return(TRUE);
    case MUIA_REV_ShowLine  : *store = data->showline;   return(TRUE);
    case MUIA_REV_ShowCol   : *store = data->showcol;    return(TRUE);
    case MUIA_REV_ColorFG   : *store = data->colorfg;    return(TRUE);
    case MUIA_REV_ColorBG   : *store = data->colorbg;    return(TRUE);
    case MUIA_REV_MaxLines  : *store = data->maxlines;   return(TRUE);
  }
  return(TempDoSuperMethodA(cl,obj,msg));
}

static ulong met_askminmax(struct IClass *cl,Object *obj,struct MUIP_AskMinMax *msg)
{
  TempDoSuperMethodA(cl,obj,msg);

  msg->MinMaxInfo->MinWidth  += _font(obj)->tf_XSize * 1;
  msg->MinMaxInfo->DefWidth  += _font(obj)->tf_XSize * 80;
  msg->MinMaxInfo->MaxWidth  += MUI_MAXMAX;

  msg->MinMaxInfo->MinHeight += _font(obj)->tf_YSize;
  msg->MinMaxInfo->DefHeight += _font(obj)->tf_YSize * 25;
  msg->MinMaxInfo->MaxHeight += MUI_MAXMAX;

  return(0);
}

static ulong met_draw(struct IClass *cl,Object *obj,struct MUIP_Draw *msg)
{
struct Data       *data = INST_DATA(cl,obj);
struct Node_line  *node;
struct TextExtent te;
long              len;
ulong             wx,wy,wh,ww,fy,i;

  TempDoSuperMethodA(cl,obj,msg);

  /* Actually autodocs states that _dri(obj) & _font(obj) are valid only
   * in setup/cleanup method _but_ as mui examples use it in draw it should
   * be ok me to use it here too. Huah, another fine example how sucky MUI
   * docs are.
   */

  if ((msg->flags & MADF_DRAWOBJECT) OR (msg->flags & MADF_DRAWUPDATE)) {
    /* calculate sizes */
    data->displines = _mheight(obj) / _font(obj)->tf_YSize;
    data->dispcols  = _mwidth(obj)  / _font(obj)->tf_XSize;
    fy = _font(obj)->tf_YSize;
    wx = _mleft(obj);
    wy = _mtop(obj);
    wh = _mheight(obj);
    ww = _mwidth(obj);
    SetAPen(_rp(obj),_dri(obj)->dri_Pens[data->colorfg]);
    SetBPen(_rp(obj),_dri(obj)->dri_Pens[data->colorbg]);

    if (msg->flags & MADF_DRAWOBJECT) {
      /* total redraw */
      data->scroll = data->displines;
      data->offset = 0;
    } else {
      /* partial redraw */
      if (data->scroll > 0) {
        /* scrolling up */
        ScrollRaster(_rp(obj),0,  abs(data->scroll)*fy ,wx,wy,wx+ww-1,(wy+data->displines*fy)-1);
      } else if (data->scroll < 0) { 
        /* scrolling down */
        ScrollRaster(_rp(obj),0,-(abs(data->scroll)*fy),wx,wy,wx+ww-1,(wy+data->displines*fy)-1);
      } else { 
        /* updating without scroll (ie. adding lines to end) */
        data->scroll = data->displines - data->offset;
      }
    }

    /* print lines */
    if (node = get_line(data,data->showline+data->offset)) {
      wy += _font(obj)->tf_Baseline + fy * data->offset;
      for (i=0;i < abs(data->scroll);i++)
      {
        len = node->size - data->showcol;
        if (len > 0) {
          Move(_rp(obj),wx,wy);
          Text(_rp(obj),&((strptr)&node[1])[data->showcol],TextFit(_rp(obj),&((strptr)&node[1])[data->showcol],len,&te,NULL,1,ww,fy));
        }
        wy += fy;
        if (!(node = next_line(data,node))) break;
      }
    }
    set_notify(data,obj);
  }

  return(0);
}

static ulong met_setup(struct IClass *cl,Object *obj,struct MUIP_HandleInput *msg)
{
  if (TempDoSuperMethodA(cl,obj,msg)) {
    MUI_RequestIDCMP(obj,IDCMP_RAWKEY);
    return(TRUE);
  }
  return(FALSE);
}


static ulong met_cleanup(struct IClass *cl,Object *obj,struct MUIP_HandleInput *msg)
{
  MUI_RejectIDCMP(obj,IDCMP_RAWKEY);
  return(TempDoSuperMethodA(cl,obj,msg));
}

static ulong met_handleinput(struct IClass *cl,Object *obj,struct MUIP_HandleInput *msg)
{
struct Data *data = INST_DATA(cl,obj);

  if (msg->muikey)
  {
    switch (msg->muikey)
    {
      case MUIKEY_UP      :
        if (data->showline) {
          data->scroll = -1;
          data->offset = 0;
          data->showline--;
          MUI_Redraw(obj,MADF_DRAWUPDATE);
        }
        break;
      case MUIKEY_PAGEUP  :
        if (data->showline) {
          data->showline = (data->showline > data->displines) ? data->showline - data->displines : 0;
          MUI_Redraw(obj,MADF_DRAWOBJECT);
        }
        break;
      case MUIKEY_DOWN    :
        if ((data->showline + data->displines) < data->lines) {
          data->scroll = 1;
          data->offset = data->displines - 1;
          data->showline++;
          MUI_Redraw(obj,MADF_DRAWUPDATE);
        }
        break;
      case MUIKEY_PAGEDOWN:
        if ((data->showline + data->displines) < data->lines) {
          data->showline = ( (data->showline + data->displines * 2) < data->lines ) ? data->showline + data->displines : data->lines - data->displines;
          MUI_Redraw(obj,MADF_DRAWOBJECT);
        }
        break;
      case MUIKEY_LEFT   :
        if (data->showcol) {
          data->showcol--;
          MUI_Redraw(obj,MADF_DRAWOBJECT);
        }        
        break;
      case MUIKEY_RIGHT   :
        if (data->showcol < 256) {
          data->showcol++;
          MUI_Redraw(obj,MADF_DRAWOBJECT);
        }        
        break;
    }
  }
  return(TempDoSuperMethodA(cl,obj,msg));
}

/***----------------------------------------------------------------------***/
/*** class public methods                                                 ***/
/***----------------------------------------------------------------------***/

static ulong met_feeddata(struct IClass *cl,Object *obj,struct MUIP_REV_FeedData *msg)
{
struct Data *data = INST_DATA(cl,obj);
long   size,s,dif;
strptr ansi,p;

  /* note: This method just adds data as it gets it, so ansi strip and
   *       conversion to iso is _NOT_ done here. Incomplete lines are
   *       joined in next calls, however review uses linebuffer to store
   *       incomplete lines and will discard lines which are longer than it.
   *       Also 0x0a is used as line end mark, 0x0d should be stripped before
   *       feeding them to here.
   *       (huah, this n times slower compared to previous review)
   */

  dif  = data->lines - data->showline;
  ansi = msg->data;
  size = msg->size;
  p    = ansi;
  s    = 0;
  while (size-- > 0)
  {
    if (*ansi++ == 0x0a) {
      if (data->tmpsize) {
        strncpy(&data->tmp[data->tmpsize],p,((data->tmpsize+s) < MAXLINE) ? s : MAXLINE - data->tmpsize);
        p = data->tmp;
        s = ((data->tmpsize+s) < MAXLINE) ? data->tmpsize+s : MAXLINE;
        data->tmpsize = 0;
      }
      if (data->maxlines <= data->lines) {
        if (del_line(data)) data->showline--;
      }
      while (add_line(data,p,s) == FALSE)
      {
        if (del_line(data)) {
          data->showline--;
        } else {
          break;
        }
      }
      p = ansi;
      s = 0;
    } else {
      s++;
    }
  }
  /* store incomplete line */
  if ( (data->tmpsize + s) < MAXLINE ) {
    strncpy(&data->tmp[data->tmpsize],p,s);
    data->tmpsize += s;
  } else {
    data->tmpsize = 0;
  }
  /* redraw if needed */
  if (data->showline < 0) {
    data->scroll   = abs(data->showline);
    data->offset   = data->displines - abs(data->scroll);
    data->showline = 0;
    if (abs(data->scroll) < data->displines) {
      MUI_Redraw(obj,MADF_DRAWUPDATE);
    } else {
      MUI_Redraw(obj,MADF_DRAWOBJECT);
    }
  } else if (dif < data->displines) {
    data->scroll = 0;
    data->offset = dif;
    MUI_Redraw(obj,MADF_DRAWUPDATE);
  }
  set_notify(data,obj);
  return(0);
}

static ulong met_clear(struct IClass *cl,Object *obj,struct MUIP_REV_Clear *msg)
{
struct Data *data = INST_DATA(cl,obj);

  while (del_line(data)) {}
  data->showline = 0;
  data->showcol  = 0;
  data->tmpsize  = 0;
  MUI_Redraw(obj,MADF_DRAWOBJECT);
  set_notify(data,obj);
  return(0);
}

static ulong met_save(struct IClass *cl,Object *obj,struct MUIP_REV_Save *msg)
{
struct Data      *data = INST_DATA(cl,obj);
struct Node_line *node;
BPTR              fh;
ubyte             eol = 0x0a;

  if (fh = dt_open(msg->name,MODE_WRITE,8192)) {
    if (node = get_line(data,0)) {
      dt_write(fh,&node[1],node->size);
      dt_write(fh,&eol    ,1);
      while (node = next_line(data,node))
      {
        if (dt_write(fh,&node[1],node->size) != node->size) break;
        if (dt_write(fh,&eol    ,1         ) != 1         ) break;
      }
    }
    dt_close(fh);
  }
  return(0);
}

static ulong met_search(struct IClass *cl,Object *obj,struct MUIP_REV_Search *msg)
{
struct Data      *data = INST_DATA(cl,obj);
struct Node_line *node;

  return(0);
}

/***----------------------------------------------------------------------***/
/*** class private methods                                                ***/
/***----------------------------------------------------------------------***/

/***----------------------------------------------------------------------***/
/*** dispatcher                                                           ***/
/***----------------------------------------------------------------------***/

static __saveds __asm ulong Dispatcher(REG __a0 struct IClass *cl,REG __a2 Object *obj,REG __a1 Msg msg)
{
  switch (msg->MethodID)
  {
    case OM_NEW           : return(met_new        (cl,obj,(APTR)msg));
    case OM_DISPOSE       : return(met_dispose    (cl,obj,(APTR)msg));
    case OM_SET           : return(met_set        (cl,obj,(APTR)msg));
    case OM_GET           : return(met_get        (cl,obj,(APTR)msg));
    case MUIM_AskMinMax   : return(met_askminmax  (cl,obj,(APTR)msg));
    case MUIM_Draw        : return(met_draw       (cl,obj,(APTR)msg));
    case MUIM_HandleInput : return(met_handleinput(cl,obj,(APTR)msg));
    case MUIM_Setup       : return(met_setup      (cl,obj,(APTR)msg));
    case MUIM_Cleanup     : return(met_cleanup    (cl,obj,(APTR)msg));

    case MUIM_REV_FeedData: return(met_feeddata   (cl,obj,(APTR)msg));
    case MUIM_REV_Clear   : return(met_clear      (cl,obj,(APTR)msg));
    case MUIM_REV_Save    : return(met_save       (cl,obj,(APTR)msg));
    case MUIM_REV_Search  : return(met_search     (cl,obj,(APTR)msg));
  }
  return(TempDoSuperMethodA(cl,obj,msg));
}

/***----------------------------------------------------------------------***/
/*** init/delete                                                          ***/
/***----------------------------------------------------------------------***/

void DelClass_revarea(void)
{
  if (mcc) {
    MUI_DeleteCustomClass(mcc);
    mcc = 0;
  }
}

struct IClass *AddClass_revarea(void)
{
  if (mcc = MUI_CreateCustomClass(NULL,MUIC_Area,NULL,sizeof(struct Data),Dispatcher)) {
    return(mcc->mcc_Class);
  } else {
    return(0);
  }
}
