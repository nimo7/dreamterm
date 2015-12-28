#include "global.h"
#include "loc_strings.h"

/*
char ver_dialer[]="1.46 (Apr 16 1996)";
*/

enum ID { ID_add = DI_First,
          ID_skip         ,
          ID_abort        ,
          ID_force        };

static bool win_act = FALSE;
static aptr win,tx_name,tx_phone,tx_comment,tx_modem,tx_time,tx_try;
static aptr bt_add,bt_skip,bt_abort,bt_force;

static char         tphone    [STRSIZE];
static char         res_str[6][STRSIZE];
static bool         dialing,
                    force;
static long         dialindex,
                    pindex,
                    wfix,
                    wfix2;
static ulong        tbase,
                    wbase,
                    tries;
static struct Info *dialentry;
static struct FlowString dialreply[] =
{
  { 0,0,0 },
  { 0,0,0 },
  { 0,0,0 },
  { 0,0,0 },
  { 0,0,0 },
  { 0,0,0 },
  { 0,0,0 }
};

/***----------------------------------------------------------------------***/
/*** support                                                              ***/
/***----------------------------------------------------------------------***/

/* Hae selectoitu entry numeron avulla */
static struct Info *GetSelect(LONG num)
{
struct Info *p = (struct Info *)gl_phonebook.lh_Head;

  while (p->node.ln_Succ != 0)
  {
    if (p->SELECT == TRUE AND p->store == num) return(p);
    p = (struct Info *)p->node.ln_Succ;
  }
  return(0);
}

static void AbortDial(void)
{
  Dialer(DT_CLOSE,0);
  if (!gl_serfreeze) Send_string((char *)GetEntryTag(PB_DialAbort,dialentry));
}

static void ShowEntry(void)
{
long i=0;

	if (pindex == 0) {
		if (!gl_dialnum) {
			if ( (dialentry = GetSelect(dialindex)) == 0 ) {
				dialindex = 1;
				if ( (dialentry = GetSelect(dialindex)) == 0) {
          AbortDial();
          return;
				}
			}
			dialindex++;
		} else {
			dialentry = gl_curbbs;
		}
		strcpy(tphone,(char *)GetEntryTag(PB_Phone,dialentry));
	}

  gl_onlinephone[0] = 0x00;
	while (tphone[pindex] != 0x00 AND tphone[pindex] != '|')
	{
		gl_onlinephone[i] = tphone[pindex];
		i++; pindex++;
	}
	gl_onlinephone[i] = 0x00;
	if (tphone[pindex] == 0x00) {
		pindex = 0;
	} else {
		pindex++;
	}
	if (tphone[pindex] == 0x00) pindex = 0;
	if (!gl_dialnum) strins(gl_onlinephone,(char *)GetEntryTag(PB_PhonePre,dialentry));

	set(tx_name	,MUIA_Text_Contents,GetEntryTag(PB_Name,dialentry));
	set(tx_phone,MUIA_Text_Contents,gl_onlinephone);
	if (!gl_dialnum) {
		set(tx_comment,MUIA_Text_Contents,GetEntryTag(PB_Comment,dialentry));
	} else {
		set(tx_comment,MUIA_Text_Contents,LOC(MSG_dialing_manually));
	}
}

static void DialEntry(void) /* Soita */
{
	if (gl_dial) {
		Send_string((char *)GetEntryTag(PB_DialAbort,dialentry));

		if (!gl_dialnum) {
			SetUpTags(0,
								PB_DteRate		,GetEntryTag(PB_DteRate			,dialentry),
								PB_FlowControl,GetEntryTag(PB_FlowControl	,dialentry),
								PB_DataBits		,GetEntryTag(PB_DataBits		,dialentry),
								PB_Parity			,GetEntryTag(PB_Parity			,dialentry),
								PB_StopBits		,GetEntryTag(PB_StopBits		,dialentry),
								PB_HardSer		,GetEntryTag(PB_HardSer			,dialentry),
								PB_SerName		,GetEntryTag(PB_SerName			,dialentry),
								PB_SerUnit		,GetEntryTag(PB_SerUnit			,dialentry),
								PB_SerBufSize	,GetEntryTag(PB_SerBufSize	,dialentry),
								PB_WaitEntry	,GetEntryTag(PB_WaitEntry		,dialentry),
								PB_WaitDial		,GetEntryTag(PB_WaitDial		,dialentry),
								TAG_END);
		}

    dialreply[0].string = &res_str[0][0];
    dialreply[1].string = &res_str[1][0];
    dialreply[2].string = &res_str[2][0];
    dialreply[3].string = &res_str[3][0];
    dialreply[4].string = &res_str[4][0];
    dialreply[5].string = &res_str[5][0];
    dprintf_user(&res_str[0][0],GetEntryTag(PB_NoDialTone,dialentry));
    dprintf_user(&res_str[1][0],GetEntryTag(PB_NoCarrier ,dialentry));
    dprintf_user(&res_str[2][0],GetEntryTag(PB_Busy      ,dialentry));
    dprintf_user(&res_str[3][0],GetEntryTag(PB_Ok        ,dialentry));
    dprintf_user(&res_str[4][0],GetEntryTag(PB_Ringing   ,dialentry));
    dprintf_user(&res_str[5][0],GetEntryTag(PB_Connect   ,dialentry));
    PrepFlow(dialreply);
/*
		dialreply[0].string = strcpy(&res_str[0][0],GetEntryTag(PB_NoDialTone	,dialentry));
		dialreply[1].string = strcpy(&res_str[1][0],GetEntryTag(PB_NoCarrier	,dialentry));
		dialreply[2].string = strcpy(&res_str[2][0],GetEntryTag(PB_Busy				,dialentry));
		dialreply[3].string = strcpy(&res_str[3][0],GetEntryTag(PB_Ok					,dialentry));
		dialreply[4].string = strcpy(&res_str[4][0],GetEntryTag(PB_Ringing		,dialentry));
		dialreply[5].string = strcpy(&res_str[5][0],GetEntryTag(PB_Connect		,dialentry));
		PrepFlow(dialreply);
*/

		Send_string((char *)GetEntryTag(PB_DialString,dialentry));
		Send_string(gl_onlinephone);
		Send_string((char *)GetEntryTag(PB_DialSuffix,dialentry));
		tbase = GetSec();
		wfix = wfix2 = 0;
		dialing = TRUE;
		tries++;
	}
}

/*  Interface pääohjelmaan
 *
 *  DI_DIAL    []        - Soita phonebookin listaa
 *  DI_DIALNUM [numbers] - Soita numeroihin
 *  DI_CHECK   []        - Tutki onnistuiko dialaus + jatka dialausta
 *                         Return: Entry joka onnistui
 *                                 0 = continue, -1 = abort
*/

ulong Dialer(ulong tag, ulong data)
{
static BOOL  ocall = FALSE; static ULONG oc;
static ulong tnow;
char         temp[80];
long         i;
ulong        ttmp;
struct RXD	*rxd;

	switch (tag)
	{
		case DT_CLOSE:
      if (win_act) {
        gl_dial = FALSE;
        set(win,MUIA_Window_Open,FALSE);
        Menu_fresh();
        if (CUR(PB_Dispose)) {
          DoMethod(gl_app,OM_REMMEMBER,win);
          MUI_DisposeObject(win);
          win_act = FALSE;
        }
      }
			break;
		case DT_OPEN:
 			if ( (GetSelect(1) == 0 AND gl_dialnum == FALSE) OR gl_dial OR gl_serfreeze) return(0);
      if (!win_act) {
  			win = WindowObject,
  			MUIA_Window_ID		,MAKE_ID('D','I','W','I'),
  			MUIA_Window_Title	,LOC(MSG_dialing),
  			MUIA_HelpNode			,"help_di",
  			MUIA_Window_Width	,MUIV_Window_Width_Visible(50),
  				WindowContents, VGroup,
  					Child, ColGroup(3),
  						GroupFrame,
  						Child, LabelL(LOC(MSG_name))   , Child, LabelL(":"), Child, tx_name	  = MyText(),
  						Child, LabelL(LOC(MSG_phone))  , Child, LabelL(":"), Child, tx_phone	= MyText(),
  						Child, LabelL(LOC(MSG_comment)), Child, LabelL(":"), Child, tx_comment= MyText(),
  					End,
  					Child, ColGroup(3),
  						Child, LabelL(LOC(MSG_modem)), Child, LabelL(":"), Child, tx_modem= MyText(),
  						Child, LabelL(LOC(MSG_note)) , Child, LabelL(":"), Child, tx_time	= MyText(),
  						Child, LabelL("")			       , Child, LabelL(":"), Child, tx_try	= MyText(),
  					End,
  					Child, HGroup,
  						Child, bt_add		= MakeButton(LOC(MSG_add_time)),
  						Child, bt_skip	= MakeButton(LOC(MSG_skip)),
  						Child, bt_abort	= MakeButton(LOC(MSG_abort)),
  						Child, bt_force	= MakeButton(LOC(MSG_force_connect)),
  					End,
  				End,
  			End;
        if (win) {
          DoMethod(gl_app,OM_ADDMEMBER,win);

    			DoMethod(win,MUIM_Window_SetCycleChain, bt_add,bt_skip,bt_abort,NULL);
    			DoMethod(win,MUIM_Notify,MUIA_Window_CloseRequest,TRUE, gl_app,2,MUIM_Application_ReturnID,ID_abort);
    			MultiIDNotify(MUIA_Pressed,FALSE,
    										bt_add		,ID_add		,
    										bt_skip		,ID_skip	,
    										bt_abort	,ID_abort	,
                        bt_force  ,ID_force , TAG_END);
          win_act = TRUE;
        }
      }

      if (win_act) {
  			Phone(DT_CLOSE,0);
  			DoMethod(win, MUIM_MultiSet,MUIA_Text_Contents,"",
  							 tx_name,tx_phone,tx_comment,tx_modem,tx_time,tx_try,NULL);
  			set(win,MUIA_Window_Screen,gl_conscreen);
  			set(win,MUIA_Window_Open,TRUE);
  			set(win,MUIA_Window_Activate,TRUE);
  			gl_dial		= TRUE;
        force     = FALSE;
  			dialindex	= 1;
  			pindex		= 0;
  			tries			= 0;
  			wbase			= GetSec();
        tnow      = wbase-1;
  			ShowEntry();
  			DialEntry();
  			Menu_fresh();
        return(TRUE);
      }
			break;
		case  DT_OC:
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
			switch (data)
			{
				case ID_add:		wfix += CUR(PB_WaitDial); wfix2 += CUR(PB_WaitEntry); break;
				case ID_skip:
					if (dialing) {
						Send_string((char *)GetEntryTag(PB_DialAbort,dialentry));
					} else {
						DialEntry();
					}
					break;
        case ID_abort:  AbortDial(); break;
        case ID_force:  force=TRUE;  break;
			}
			break;
		case DI_CHECK:
      if (!win_act) return(0);  /* too lazy to change internals */
      if (!gl_serok OR gl_serfreeze) { AbortDial(); return(0); }
      rxd = GetRXD();
      if (rxd->size) {
        EmPrint(rxd->buffer,rxd->size);
        HuntFlow(dialreply,rxd->buffer,rxd->size);
				for (i=0; i <= 4; i++)
				{
					if (dialreply[i].hit) {
						set(tx_modem,MUIA_Text_Contents,dialreply[i].string);
						PrepFlow(dialreply);
						if (i != 4) {
							Send_string((char *)GetEntryTag(PB_DialAbort,dialentry));
							tbase = GetSec();
							dialing = FALSE;
							ShowEntry();
						}
					}
				}
				if (dialreply[5].hit) { /* connect */
					return(dialentry);
				}
			}
      if (force) return(dialentry); /* forced connect */

      ttmp = tnow;
			tnow = GetSec();
			if (dialing) { /* dialing */
				if ( (tnow - tbase) > (CUR(PB_WaitDial) + wfix) ) {
					Send_string((char *)GetEntryTag(PB_DialAbort,dialentry));
					tbase = GetSec();
					dialing = FALSE;
					ShowEntry();
				} else {
          if (tnow != ttmp) {
  					sprintf(temp,LOC(MSG_x_seconds_left_for_succesful_connection),CUR(PB_WaitDial)+wfix - (tnow-tbase));
	  				set(tx_time,MUIA_Text_Contents,temp);
          }
				}
			} else { /* delaying between dials */
				if ( (tnow - tbase) > (CUR(PB_WaitEntry) + wfix2) ) {
					DialEntry();
				} else {
          if (tnow != ttmp) {
            sprintf(temp,LOC(MSG_x_seconds_left_for_waiting),CUR(PB_WaitEntry)+wfix2 - (tnow-tbase));
            set(tx_time,MUIA_Text_Contents,temp);
          }
        }
      }
      if (tnow != ttmp) {
      char str[10];

        TimeText(tnow-wbase,str);
        str[8] = 0;
        sprintf(temp,LOC(MSG_x_tries_x_seconds_used),tries,str);
        set(tx_try,MUIA_Text_Contents,temp);
      }
      return(0);
      break;
    case DI_DIAL:
      if (!gl_dial) {
        gl_dialnum = FALSE;
        Dialer(DT_OPEN,0);
      }
      break;
    case DI_DIALNUM:
      if (!gl_dial) {
        AddInfoTags(gl_curbbs,
                    PB_Name   ,LOC(MSG_no_name),
                    PB_Phone  ,data         , TAG_END);
        gl_dialnum = TRUE;
        Dialer(DT_OPEN,0);
      }
       break;
  }
}
