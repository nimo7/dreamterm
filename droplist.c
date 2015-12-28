#include "global.h"
#include "loc_strings.h"

/*
char ver_droplist[]="1.33 (Aug 28 1994)";
*/

enum ID { ID_dl = DL_First,
          ID_adddrop,
          ID_deldrop  };

static bool win_act = FALSE;
static aptr lv_dl   = 0;
static aptr win,bt_adddrop,bt_deldrop;

static struct List droplist;
static struct Info *curdl=0; /* selected entry  */
static ulong       dlnum =0; /* selected number */

/* Support
 */

__inline static void Update(void)
{
}

/*  Gadgetit
 */

static void adddrop_click(void)
{
static char name[STRSIZE];
struct Info *dl;

  if (!gl_req) {
    Ask_string(&adddrop_click,LOC(MSG_enter_new_number),name);
  } else {
    if ( Ask_string_done() == REQ_OK ) {
      if ( dl = CreateInfo() ) {
        if ( StrRep(&dl->ln_Name,name) ) {
          AddTail(&droplist,dl);
          DoMethod(lv_dl,MUIM_List_Insert,&dl,1,MUIV_List_Insert_Sorted);
          MUI_selnode(lv_dl,dl);
        } else {
          DeleteInfo(dl);
        }
      }
    }
  }
}

static void deldrop_click(void)
{
  get(lv_dl,MUIA_List_Active,&dlnum);
  DoMethod(lv_dl,MUIM_List_GetEntry,dlnum,&curdl);
  if (curdl) {
    RemoveInfo(curdl);
    DoMethod(lv_dl,MUIM_List_Remove,dlnum);
  }
}

/*  Interface pääohjelmaan
 *
 *  DL_ADD [phone] - Lisää phonet droplistaan
 *  DL_CMP [phone] - Vertaile phonea
 *
 */

ulong Drop(ULONG tag, ULONG data)
{
static BOOL ocall = FALSE; static ULONG oc;
char        t[STRSIZE];
char        *p;
LONG        b,i;
struct Node *dl;

  switch (tag)
  {
    case DT_CLOSE:
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
        MUIA_Window_ID   ,MAKE_ID('D','L','W','I'),
        MUIA_Window_Title,LOC(MSG_droplist),
        MUIA_HelpNode    ,"help_dl",
        MUIA_Window_Width,MUIV_Window_Width_Visible(15),
          WindowContents, VGroup,
            Child, lv_dl = ListView(),
            Child, HGroup,
              Child, bt_adddrop = MakeButton(LOC(MSG_key_add)),
              Child, bt_deldrop = MakeButton(LOC(MSG_key_delete)),
            End,
          End,
        End;
        if (win) {
          DoMethod(gl_app,OM_ADDMEMBER,win);

          DoMethod(win,MUIM_Window_SetCycleChain, lv_dl,bt_adddrop,bt_deldrop, NULL);
          DoMethod(win,MUIM_Notify,MUIA_Window_CloseRequest,TRUE, win,3,MUIM_Set,MUIA_Window_Open,FALSE);
          MultiIDNotify(MUIA_Pressed,FALSE,
                      bt_adddrop  ,ID_adddrop  ,
                      bt_deldrop  ,ID_deldrop  , TAG_END);
          InfoListToMui(&droplist,lv_dl,MUIV_List_Insert_Sorted);
          win_act = TRUE;
        }
      }
      if (win_act) {
        Update();
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
    case DT_LOAD:
      EmptyInfoList(&droplist);
      LoadInfoList(&droplist,data,DLID);
      InfoListToMui(&droplist,lv_dl,MUIV_List_Insert_Sorted);
      break;
    case DT_SAVE:
      SaveInfoList(&droplist,data,DLID);
      break;
    case DT_INIT:
      NewList(&droplist);
      return(TRUE);
      break;
    case DT_HANDLE:
      switch (data)
      {
        case ID_adddrop: adddrop_click(); break;
        case ID_deldrop: deldrop_click(); break;
      }
      break;
    case DT_UPDATE:
      if (win_act) {
        get(win,MUIA_Window_Open,&b);
        if (b) Update();
      }
      break;
    case DL_ADD:  /* Lisää droplistaan */
      p = (char *)data;
      while (TRUE)
      {
        i = 0; t[0] = 0x00;
        while (*p != 0x00 AND *p != '|')
        {
          t[i] = *p; p++; i++;
        }
        if (t[0] != 0x00) {
          t[i] = 0x00;
          if ( (dl = CreateInfo()) ) {
            if ( StrRep(&dl->ln_Name,t) ) {
              AddTail(&droplist,dl);
              if (win_act) DoMethod(lv_dl,MUIM_List_Insert,&dl,1,MUIV_List_Insert_Sorted);
            } else {
              DeleteInfo(dl);
            }
            if (*p == 0x00) return(0);
            p++;
          }
        } else {
          return(0);
        }
      }
      break;
    case DL_CMP:  /* Vertaile droplistaan */
      dl = GetHead(&droplist);
      while (dl)
      {
        if (CmpPhone(dl->ln_Name,(char *)data)) return(TRUE);
        dl = GetSucc(dl);
      }
      return(FALSE);
      break;
  }
}
