#include "global.h"
#include "loc_strings.h"

/*
char ver_crypt[]="1.01 (Aug 28 1995)";
*/

enum ID { ID_lib = CR_First,
          ID_password,
          ID_quit      };

static bool win_act = FALSE;
static aptr win,st_lib,st_password;

struct Library *XpkBase;

/***----------------------------------------------------------------------***/
/*** gadgetit                                                             ***/
/***----------------------------------------------------------------------***/

static void lib_click(void)
{
strptr p;

  get(st_lib,MUIA_String_Contents,&p);
  AddInfoTag(PB_CryptLib,p,gl_curbbs);
}

static void password_click(void)
{
strptr p;

  get(st_password,MUIA_String_Contents,&p);
  AddInfoTag(PB_CryptKey,p,gl_curbbs);
}

/*  Interface pääohjelmaan
 *
 *  CR_CRYPT    [source,target] crypt file
 *  CR_DECRYPPT [source,target] decrypt file
 *  CR_DELETE   [target]        delete file safely
 *
 */

ulong Crypt(ULONG tag, ULONG data, ULONG data2)
{
static BOOL ocall = FALSE; static ULONG oc;
BPTR fh;
long size,r;
char nil[512];

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
        MUIA_Window_ID    ,MAKE_ID('C','R','W','I'),
        MUIA_Window_Title ,LOC(MSG_crypting_options),
  			MUIA_HelpNode     ,"help_cr",
  			MUIA_Window_Width ,MUIV_Window_Width_Visible(50),
  				WindowContents, VGroup,
  					Child, ColGroup(2),
  						Child, Label2L(LOC(MSG_library)) , Child, st_lib      = String(STRSIZE),
  						Child, Label2L(LOC(MSG_password)), Child, st_password = String(STRSIZE),
  					End,
  				End,
  			End;
        if (win) {
      		DoMethod(gl_app,OM_ADDMEMBER,win);

    			DoMethod(win,MUIM_Window_SetCycleChain, st_lib,st_password ,NULL);
    			DoMethod(win,MUIM_Notify,MUIA_Window_CloseRequest,TRUE, gl_app,2,MUIM_Application_ReturnID,ID_quit);
    			MultiIDNotify(MUIA_String_Contents,MUIV_EveryTime,
    									st_lib			,ID_lib,
    									st_password	,ID_password, TAG_END);
          win_act = TRUE;
        }
      }
      if (win_act) {
        nnset(st_lib      ,MUIA_String_Contents,CUR(PB_CryptLib));
        nnset(st_password ,MUIA_String_Contents,CUR(PB_CryptKey));
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
		case DT_INIT:
      return(TRUE);
			break;
		case DT_HANDLE:
      if (win_act) {
  			switch (data)
	  		{
          case ID_lib:      lib_click();        break;
          case ID_password: password_click();   break;
          case ID_quit:     Crypt(DT_CLOSE,0,0);break;
  			}
	  		break;
      }
#ifdef DEBUG
  printf("Crypt: unexpected MUI ID\n");
#endif 
      break;
		case CR_CRYPT:
			if ( XpkBase = OpenLibrary(XPKNAME,2) ) {
				r =	XpkPackTags(XPK_InName		,data,
												XPK_OutName		,data2,
												XPK_StepDown	,TRUE,
												XPK_PackMethod,CUR(PB_CryptLib),
												XPK_Password	,CUR(PB_CryptKey),
												TAG_END);
				CloseLibrary(XpkBase);
				if (r == 0) return(1);
				return(0);
			}
			break;
		case CR_DECRYPT:
			if ( XpkBase = OpenLibrary(XPKNAME,2) ) {
				r =	XpkUnpackTags(XPK_InName		,data,
													XPK_OutName		,data2,
													XPK_NoClobber	,FALSE,
													XPK_Password	,CUR(PB_CryptKey),
													TAG_END);
				CloseLibrary(XpkBase);
				if (r == 0) return(1);
				return(0);
			}
			break;
		case CR_DELETE:
			memset(nil,0,512);
			if ((size = FileSize(data)) != -1) {
				if (fh = Open(data,MODE_READWRITE)) {
					while (size > 0)
					{
						if (Write(fh,nil,512) != 512) break;
						size -= 512;
					}
					Close(fh);
				}
			}
			DeleteFile(data);
			break;
	}
}
