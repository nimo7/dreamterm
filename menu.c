#include "global.h"
#include "loc_strings.h"

/*
char ver_menu[]="1.48 (Mar 27 1996)";
*/

extern char ver_dterm[];

enum mEnU
{
  ID_cs						,
  ID_tb           ,
  ID_xp						,
  ID_xm						,
  /* project */
  ID_confcurrent	,ID_confprofile	,ID_phoneprefix	,ID_confcs			,
  ID_confxp				,ID_confxm			,ID_conftb			,ID_loaddef			,
  ID_savedef			,ID_titlebar		,ID_tempdir			,ID_initstring	,
  ID_pri3m				,ID_pri0				,ID_pri3				,ID_cryptset		,
  ID_cryptmode		,ID_autosafe		,ID_dispose     ,ID_logno				,
  ID_logcall			,ID_about				,ID_quit				,
  /* phonebook */
  ID_directory		,ID_newentry		,ID_loadbook		,ID_savebook		,
  ID_dialnumber		,ID_dialnext		,ID_sendpassword,
  /* transfer */
	ID_upload				,ID_uploadid		,ID_dnload			,ID_sendconv		,
	ID_sendnoconv		,ID_abortsend		,ID_showid			,ID_uppath			,
	ID_dnpath				,ID_autoxfer		,ID_quietxfer		,ID_double			,
	ID_togglerev		,ID_revshow			,ID_revsize			,ID_captext			,
	ID_capraw				,ID_capopen			,ID_capclose		,
	/* serial */
	ID_dte300				,ID_dte1200			,ID_dte2400			,ID_dte4800			,
	ID_dte9600			,ID_dte14400		,ID_dte19200		,ID_dte38400		,
	ID_dte57600			,ID_dte76800		,ID_dte115200		,ID_dteenter		,
	ID_halfdup			,ID_fulldup			,ID_serdev			,ID_serunit			,
  ID_sershared    ,ID_hardser			,ID_dtrhang			,ID_hangup			,
  ID_relser			  ,
	/* screen */
	ID_scrmode			,ID_scrcolumns	,ID_scrlines		,ID_statnone		,
	ID_statline			,ID_statwindow	,ID_statusline  ,ID_pophard			,
  ID_borblank		  ,ID_keepwb			,ID_aga					,ID_hardscr			,
  ID_rxdpop			  ,ID_rtsblit			,ID_forceconxy	,ID_wb					,
  ID_pubscreen		,ID_cr2crlf			,ID_lf2crlf			,ID_barver      ,
  ID_barhor       ,ID_baron       ,
	/* emulation */
	ID_setansi			,ID_sethex			,ID_emulreset		,ID_ansipre			,
	ID_ansiwrap			,ID_ansikillbs	,ID_ansistrip		,ID_ansicls			,
	ID_ansicr				,ID_ansipc			,ID_xemchange		,			
	/* special */
	ID_direct				,ID_runcom			,ID_selftest    ,
};

#define	MENU_PROJECT		( SHIFTMENU(0) )
#define	MENU_PHONEBOOK	( SHIFTMENU(1) )
#define	MENU_TRANSFER		( SHIFTMENU(2) )
#define	MENU_SERIAL			( SHIFTMENU(3) )
#define	MENU_SCREEN			( SHIFTMENU(4) )
#define	MENU_EMULATION	( SHIFTMENU(5) )
#define	MENU_SPECIAL		( SHIFTMENU(6) )

#define	MENU_CS					( SHIFTMENU(4) | SHIFTITEM( 4) )
#define	MENU_TB					( SHIFTMENU(4) | SHIFTITEM(20) )
#define	MENU_XP					( SHIFTMENU(2) | SHIFTITEM( 9) )
#define	MENU_XM					( SHIFTMENU(5) | SHIFTITEM( 2) )

static struct Menu *menu_all = 0;

static struct NewMenu m1[]=
{
{ NM_TITLE,(strptr)MSG_project                ,  0,      0,0,0											},
{ NM_ITEM	,(strptr)MSG_current_settings       ,  0,      0,0,(APTR)ID_confcurrent		},/* 0,00,00 */
{ NM_ITEM	,(strptr)MSG_profile                ,  0,      0,0,(APTR)ID_confprofile		},
{ NM_ITEM	,(strptr)MSG_profile_phone_prefix   ,  0,      0,0,(APTR)ID_phoneprefix		},/* 0,02,00 */
{ NM_ITEM	,(strptr)MSG_character_sets         ,  0,      0,0,(APTR)ID_confcs				},
{ NM_ITEM	,(strptr)MSG_protocols              ,  0,      0,0,(APTR)ID_confxp				},
{ NM_ITEM	,(strptr)MSG_external_emulations    ,  0,      0,0,(APTR)ID_confxm				},
{ NM_ITEM	,(strptr)MSG_toolbars               ,  0,      0,0,(APTR)ID_conftb				},
{ NM_ITEM	,(strptr)MSG_load_configuration     ,  0,      0,0,(APTR)ID_loaddef				},
{ NM_ITEM	,(strptr)MSG_save_configuration     ,  0,      0,0,(APTR)ID_savedef				},
{ NM_ITEM	,NM_BARLABEL                        ,  0,      0,0,0											},
{ NM_ITEM	,(strptr)MSG_titlebar               ,"b",      0,0,(APTR)ID_titlebar			},
{ NM_ITEM	,(strptr)MSG_temporary_directory    ,  0,      0,0,(APTR)ID_tempdir				},/* 0,11,00 */
{ NM_ITEM	,(strptr)MSG_initstring             ,"i",      0,0,(APTR)ID_initstring		},/* 0,12,00 */
{ NM_ITEM	,(strptr)MSG_priority               ,  0,      0,0,0											},/* 0,13,00 */
{ NM_SUB	,(strptr)MSG_neg_3                  ,  0,CHECKIT,0,(APTR)ID_pri3m		      },
{ NM_SUB	,(strptr)MSG_0                      ,  0,CHECKIT,0,(APTR)ID_pri0			    },
{ NM_SUB	,(strptr)MSG_3                      ,  0,CHECKIT,0,(APTR)ID_pri3			    },
{ NM_ITEM	,(strptr)MSG_crypt_settings         ,  0,      0,0,(APTR)ID_cryptset			},/* 0,14,00 */
{ NM_ITEM	,(strptr)MSG_phonebook_crypting     ,  0,CHECKIT,0,(APTR)ID_cryptmode     },/* 0,15,00 */
{ NM_ITEM	,(strptr)MSG_auto_phonebook_save    ,  0,CHECKIT,0,(APTR)ID_autosafe	    },/* 0,16,00 */
{ NM_ITEM	,(strptr)MSG_dispose_window_objects ,  0,CHECKIT,0,(APTR)ID_dispose       },/* 0,17,00 */
{ NM_ITEM	,NM_BARLABEL                        ,  0,      0,0,0											},/* 0,18,00 */
{ NM_ITEM	,(strptr)MSG_logfile                ,  0,      0,0,0											},/* 0,19,00 */
{ NM_SUB	,(strptr)MSG_not_active             ,  0,CHECKIT,0,(APTR)ID_logno		      },
{ NM_SUB	,(strptr)MSG_calls                  ,  0,CHECKIT,0,(APTR)ID_logcall	      },
{ NM_ITEM	,(strptr)MSG_about                  ,"?",      0,0,(APTR)ID_about					},/* 0,20,00 */
{ NM_ITEM	,NM_BARLABEL                        ,  0,      0,0,0											},
{ NM_ITEM	,(strptr)MSG_quit                   ,"q",      0,0,(APTR)ID_quit					},/* 0,22,00 */
{ NM_TITLE,(strptr)MSG_phonebook              ,  0,      0,0,0											},
{ NM_ITEM	,(strptr)MSG_directory              ,"e",      0,0,(APTR)ID_directory			},/* 1,00,00 */
{ NM_ITEM	,(strptr)MSG_create_entry           ,  0,      0,0,(APTR)ID_newentry			},
{ NM_ITEM	,(strptr)MSG_load_default_phonebook ,  0,      0,0,(APTR)ID_loadbook			},/* 1,02,00 */
{ NM_ITEM	,(strptr)MSG_save_default_phonebook ,  0,      0,0,(APTR)ID_savebook			},
{ NM_ITEM	,NM_BARLABEL                        ,  0,      0,0,0											},
{ NM_ITEM	,(strptr)MSG_dial_number            ,  0,      0,0,(APTR)ID_dialnumber		},/* 1,05,00 */
{ NM_ITEM	,(strptr)MSG_dial_next_selected     ,"c",      0,0,(APTR)ID_dialnext			},/* 1,06,00 */
{ NM_ITEM	,(strptr)MSG_send_current_password  ,"p",      0,0,(APTR)ID_sendpassword	},
{ NM_TITLE,(strptr)MSG_transfer               ,  0,      0,0,0											},
{ NM_ITEM	,(strptr)MSG_upload                 ,"u",      0,0,(APTR)ID_upload				},/* 2,00,00 */
{ NM_ITEM	,(strptr)MSG_upload_using_list      ,"l",      0,0,(APTR)ID_uploadid			},/* 2,01,00 */
{ NM_ITEM	,(strptr)MSG_download               ,"d",      0,0,(APTR)ID_dnload				},/* 2,02,00 */
{ NM_ITEM	,(strptr)MSG_send_ascii             ,"s",      0,0,(APTR)ID_sendconv			},/* 2,03,00 */
{ NM_ITEM	,(strptr)MSG_send_ascii_no_conv     ,  0,      0,0,(APTR)ID_sendnoconv		},/* 2,04,00 */
{ NM_ITEM	,(strptr)MSG_abort_send             ,"a",      0,0,(APTR)ID_abortsend			},/* 2,05,00 */
{ NM_ITEM	,(strptr)MSG_edit_upload_list       ,  0,      0,0,(APTR)ID_showid				},/* 2,06,00 */
{ NM_ITEM	,(strptr)MSG_upload_path            ,  0,      0,0,(APTR)ID_uppath				},/* 2,07,00 */
{ NM_ITEM	,(strptr)MSG_download_path          ,  0,      0,0,(APTR)ID_dnpath				},/* 2,08,00 */
{ NM_ITEM	,(strptr)MSG_protocol               ,  0,      0,0,0											} /* 2,09,00 */
};
static struct NewMenu m2[] =
{
{ NM_ITEM ,(strptr)MSG_auto_transfer          ,  0,CHECKIT,0,(APTR)ID_autoxfer	    },/* 2,10,00 */
{ NM_ITEM ,(strptr)MSG_minimal_transfer_status,  0,CHECKIT,0,(APTR)ID_quietxfer     },/* 2,11,00 */
{ NM_ITEM ,(strptr)MSG_double_buffer          ,  0,CHECKIT,0,(APTR)ID_double		    },/* 2,12,00 */
{ NM_ITEM	,NM_BARLABEL                        ,  0,      0,0,0											},
{ NM_ITEM	,(strptr)MSG_review_buffer          ,  0,      0,0,0											},
{ NM_SUB	,(strptr)MSG_active                 ,  0,CHECKIT,0,(APTR)ID_togglerev     },/* 2,14,00 */
{ NM_SUB	,(strptr)MSG_view                   ,"r",      0,0,(APTR)ID_revshow				},
{ NM_SUB	,(strptr)MSG_size                   ,  0,      0,0,(APTR)ID_revsize				},/* 2,14,02 */
{ NM_ITEM	,(strptr)MSG_capture                ,  0,      0,0,0											},
{ NM_SUB	,(strptr)MSG_text                   ,  0,CHECKIT,0,(APTR)ID_captext	      },/* 2,15,00 */
{ NM_SUB	,(strptr)MSG_raw                    ,  0,CHECKIT,0,(APTR)ID_capraw		    },/* 2,15,01 */
{ NM_SUB	,(strptr)MSG_open_capture           ,  0,      0,0,(APTR)ID_capopen				},/* 2,15,02 */
{ NM_SUB	,(strptr)MSG_close_capture          ,  0,      0,0,(APTR)ID_capclose			},/* 2,15,03 */
{ NM_TITLE,(strptr)MSG_serial                 ,  0,      0,0,0											},
{ NM_ITEM	,(strptr)MSG_dte_rate               ,  0,      0,0,0											},
{ NM_SUB	,(strptr)MSG_300                    ,  0,CHECKIT,0,(APTR)ID_dte300		    },/* 3,00,00 */
{ NM_SUB	,(strptr)MSG_1200                   ,  0,CHECKIT,0,(APTR)ID_dte1200	      },
{ NM_SUB	,(strptr)MSG_2400                   ,  0,CHECKIT,0,(APTR)ID_dte2400	      },
{ NM_SUB	,(strptr)MSG_4800                   ,  0,CHECKIT,0,(APTR)ID_dte4800	      },
{ NM_SUB	,(strptr)MSG_9600                   ,  0,CHECKIT,0,(APTR)ID_dte9600	      },
{ NM_SUB	,(strptr)MSG_14400                  ,  0,CHECKIT,0,(APTR)ID_dte14400	    },
{ NM_SUB	,(strptr)MSG_19200                  ,  0,CHECKIT,0,(APTR)ID_dte19200	    },
{ NM_SUB	,(strptr)MSG_38400                  ,  0,CHECKIT,0,(APTR)ID_dte38400	    },
{ NM_SUB	,(strptr)MSG_57600                  ,  0,CHECKIT,0,(APTR)ID_dte57600	    },
{ NM_SUB	,(strptr)MSG_76800                  ,  0,CHECKIT,0,(APTR)ID_dte76800	    },
{ NM_SUB	,(strptr)MSG_115200                 ,  0,CHECKIT,0,(APTR)ID_dte115200     },
{ NM_SUB	,(strptr)MSG_enter                  ,  0,      0,0,(APTR)ID_dteenter			},/* 3,11,00 */
{ NM_ITEM	,(strptr)MSG_databits               ,  0,      0,0,0											},
{ NM_SUB	,(strptr)MSG_7                      ,  0,CHECKIT,0,0									    },/* 3,01,00 */
{ NM_SUB	,(strptr)MSG_8                      ,  0,CHECKIT,0,0									    },
{ NM_ITEM	,(strptr)MSG_parity                 ,  0,      0,0,0											},/* 3,02,00 */
{ NM_SUB	,(strptr)MSG_none                   ,  0,CHECKIT,0,0									    },
{ NM_SUB	,(strptr)MSG_even                   ,  0,CHECKIT,0,0									    },
{ NM_SUB	,(strptr)MSG_odd                    ,  0,CHECKIT,0,0									    },
{ NM_SUB	,(strptr)MSG_mark                   ,  0,CHECKIT,0,0									    },
{ NM_SUB	,(strptr)MSG_space                  ,  0,CHECKIT,0,0									    },
{ NM_ITEM	,(strptr)MSG_stopbits               ,  0,      0,0,0											},
{ NM_SUB	,(strptr)MSG_1                      ,  0,CHECKIT,0,0									    },/* 3,03,00 */
{ NM_SUB	,(strptr)MSG_2                      ,  0,CHECKIT,0,0									    },
{ NM_ITEM	,(strptr)MSG_flow_control           ,  0,      0,0,0											},
{ NM_SUB	,(strptr)MSG_none                   ,  0,CHECKIT,0,0									    },/* 3,04,00 */
{ NM_SUB	,(strptr)MSG_xon_xoff               ,  0,CHECKIT,0,0									    },
{ NM_SUB	,(strptr)MSG_rts_cts                ,  0,CHECKIT,0,0									    },
{ NM_ITEM	,(strptr)MSG_duplex                 ,  0,      0,0,0											},
{ NM_SUB	,(strptr)MSG_half                   ,  0,CHECKIT,0,(APTR)ID_halfdup	      },/* 3,05,00 */
{ NM_SUB	,(strptr)MSG_full                   ,  0,CHECKIT,0,(APTR)ID_fulldup	      },
{ NM_ITEM	,(strptr)MSG_serial_device          ,  0,      0,0,(APTR)ID_serdev				},/* 3,06,00 */
{ NM_ITEM	,(strptr)MSG_serial_unit            ,  0,      0,0,(APTR)ID_serunit				},/* 3,07,00 */
{ NM_ITEM	,(strptr)MSG_serial_shared          ,  0,CHECKIT,0,(APTR)ID_sershared     },/* 3,08,00 */
{ NM_ITEM	,(strptr)MSG_fixed_dte_rate         ,  0,CHECKIT,0,0								    	},/* 3,09,00 */
{ NM_ITEM	,(strptr)MSG_hardware_serial        ,  0,CHECKIT,0,(APTR)ID_hardser	      },/* 3,10,00 */
{ NM_ITEM	,(strptr)MSG_drop_dtr_to_hangup     ,  0,CHECKIT,0,(APTR)ID_dtrhang	      },/* 3,11,00 */
{ NM_ITEM	,(strptr)MSG_serial_active          ,  0,CHECKIT,0,(APTR)ID_relser	    	},/* 3,12,00 */
{ NM_ITEM	,(strptr)MSG_hangup                 ,"h",      0,0,(APTR)ID_hangup				},/* 3,13,00 */
{ NM_TITLE,(strptr)MSG_screen                 ,  0,      0,0,0											},
{ NM_ITEM	,(strptr)MSG_screen_mode            ,  0,      0,0,(APTR)ID_scrmode				},/* 4,00,00 */
{ NM_ITEM	,(strptr)MSG_columns                ,  0,      0,0,(APTR)ID_scrcolumns		},/* 4,01,00 */
{ NM_ITEM	,(strptr)MSG_lines                  ,  0,      0,0,(APTR)ID_scrlines			},/* 4,02,00 */
{ NM_ITEM	,(strptr)MSG_status                 ,  0,      0,0,0											},
{ NM_SUB	,(strptr)MSG_nothing                ,  0,CHECKIT,0,(APTR)ID_statnone	    },/* 4,03,00 */
{ NM_SUB	,(strptr)MSG_line                   ,  0,CHECKIT,0,(APTR)ID_statline	    },
{ NM_SUB	,(strptr)MSG_window                 ,  0,CHECKIT,0,(APTR)ID_statwindow    },
{ NM_SUB	,(strptr)MSG_status_line            ,  0,      0,0,(APTR)ID_statusline    },
{ NM_ITEM	,(strptr)MSG_charset                ,  0,      0,0,0											} /* 4,04,00 */
};
static struct NewMenu m3[] =
{
{ NM_ITEM	,(strptr)MSG_show_hardware_screen   ,"t",      0,0,(APTR)ID_pophard				},/* 4,05,00 */
{ NM_ITEM	,(strptr)MSG_borderblank            ,  0,CHECKIT,0,(APTR)ID_borblank	    },/* 4,06,00 */
{ NM_ITEM	,(strptr)MSG_keep_workbench_colors  ,  0,CHECKIT,0,(APTR)ID_keepwb		    },/* 4,07,00 */
{ NM_ITEM	,(strptr)MSG_enable_aga_bandwith    ,  0,CHECKIT,0,(APTR)ID_aga		  	    },/* 4,08,00 */
{ NM_ITEM	,(strptr)MSG_hardware_screen        ,  0,CHECKIT,0,(APTR)ID_hardscr   	  },/* 4,09,00 */
{ NM_ITEM	,(strptr)MSG_auto_rxd_popup         ,  0,CHECKIT,0,(APTR)ID_rxdpop		    },/* 4,10,00 */
{ NM_ITEM	,(strptr)MSG_rts_blit               ,  0,CHECKIT,0,(APTR)ID_rtsblit	      },/* 4,11,00 */
{ NM_ITEM	,(strptr)MSG_force_screenreqs_size  ,  0,CHECKIT,0,(APTR)ID_forceconxy    },/* 4,12,00 */
{ NM_ITEM	,(strptr)MSG_use_public_screen      ,  0,CHECKIT,0,(APTR)ID_wb				    },/* 4,13,00 */
{ NM_ITEM	,(strptr)MSG_public_screen_name     ,  0,      0,0,(APTR)ID_pubscreen			},/* 4,14,00 */
{ NM_ITEM	,NM_BARLABEL                        ,  0,      0,0,0											},
{ NM_ITEM	,(strptr)MSG_txd_crlf_conversion    ,  0,      0,0,0											},
{ NM_SUB	,(strptr)MSG_cr_to_crlf             ,  0,CHECKIT,0,(APTR)ID_cr2crlf	      },/* 4,16,00 */
{ NM_SUB	,(strptr)MSG_lf_to_crlf             ,  0,CHECKIT,0,(APTR)ID_lf2crlf	      },/* 4,16,01 */
{ NM_ITEM	,NM_BARLABEL                        ,  0,      0,0,0											},
{ NM_ITEM	,(strptr)MSG_active                 ,  0,CHECKIT,0,(APTR)ID_baron         },/* 4,18,00 */
{ NM_ITEM	,(strptr)MSG_layout                 ,  0,      0,0,0                   		},
{ NM_SUB	,(strptr)MSG_vertical               ,  0,CHECKIT,0,(APTR)ID_barver  	    },/* 4,19,00 */
{ NM_SUB	,(strptr)MSG_horizontal             ,  0,CHECKIT,0,(APTR)ID_barhor  	    },/* 4,19,01 */
{ NM_ITEM	,(strptr)MSG_toolbar                ,  0,      0,0,0											} /* 4,20,00 */
};
static struct NewMenu m4[] =
{
{ NM_TITLE,(strptr)MSG_emulation              ,  0,      0,0,0											},
{ NM_ITEM	,(strptr)MSG_ansi_vt102             ,  0,CHECKIT,0,(APTR)ID_setansi       },/* 5,00,00 */
{ NM_ITEM	,(strptr)MSG_hex                    ,  0,CHECKIT,0,(APTR)ID_sethex		    },/* 5,01,00 */
{ NM_ITEM	,(strptr)MSG_xem                    ,  0,      0,0,0											} /* 5,02,00 */
};
static struct NewMenu m5[] =
{
{ NM_ITEM	,NM_BARLABEL                        ,  0,      0,0,0											},
{ NM_ITEM	,(strptr)MSG_reset                  ,  0,      0,0,(APTR)ID_emulreset			},
{ NM_ITEM	,(strptr)MSG_ansi_setup             ,  0,      0,0,0											},
{ NM_SUB	,(strptr)MSG_prescroll              ,  0,      0,0,(APTR)ID_ansipre				},/* 5,05,00 */
{ NM_SUB	,(strptr)MSG_line_wrap              ,  0,CHECKIT,0,(APTR)ID_ansiwrap	    },/* 5,05,01 */
{ NM_SUB	,(strptr)MSG_destructive_backspace  ,  0,CHECKIT,0,(APTR)ID_ansikillbs    },/*5,05,02 */
{ NM_SUB	,(strptr)MSG_strip_bold             ,  0,CHECKIT,0,(APTR)ID_ansistrip     },/* 5,05,03 */
{ NM_SUB	,(strptr)MSG_esc2j_homes_cursor     ,  0,CHECKIT,0,(APTR)ID_ansicls	      },/* 5,05,04 */
{ NM_SUB	,(strptr)MSG_cr_lf_to_cr_lf         ,  0,CHECKIT,0,(APTR)ID_ansicr		    },/* 5,05,05 */
{ NM_SUB	,(strptr)MSG_pc_ansi                ,  0,CHECKIT,0,(APTR)ID_ansipc		    },/* 5,05,06 */
{ NM_ITEM	,(strptr)MSG_xem_setup              ,  0,      0,0,0											},/* 5,06,00 */
{ NM_SUB	,(strptr)MSG_change_options         ,  0,      0,0,(APTR)ID_xemchange			},
{ NM_TITLE,(strptr)MSG_special                ,  0,      0,0,0											},
{ NM_ITEM	,(strptr)MSG_direct_print           ,  0,      0,0,(APTR)ID_direct				},/* 6,00,00 */
{ NM_ITEM	,(strptr)MSG_run_command            ,  0,      0,0,(APTR)ID_runcom				},/* 6,01,00 */
#ifdef DEBUG
{ NM_ITEM	,"Self analysis",  					0,0,0,(APTR)ID_selftest				},/* 6,02,00 */
#endif
{ NM_END,0,0,0,0,0}
};

/*-------------------------------------------------------------------------*/

/*	Support
 */

/* disable/enable item */
static Item_dis(ULONG item, ULONG b)
{
	if (b) {
			IA(item)->Flags &= ~ITEMENABLED;
	} else {
			IA(item)->Flags |= ITEMENABLED;
	}
}

/* set checked for item and clear others */
static Item_set(ULONG item, ULONG j, ULONG val, ... )
{
ULONG	i;
va_list	ap;
struct MenuItem	*mi;

	va_start(ap, val);
	for (i=0 ; i < j ; i++ )
	{
		if ( val == va_arg(ap,ULONG) ) {
			if ( mi = IA(item + SHIFTITEM(i)) ) mi->Flags |= CHECKED;
		} else {
			if ( mi = IA(item + SHIFTITEM(i)) ) mi->Flags &= ~CHECKED;
		}
	}
	va_end(ap);
}

/* set checked for subitem and clear others */
static Sub_set(ULONG item, ULONG j, ULONG val, ... )
{
ULONG	i;
va_list	ap;
struct MenuItem	*mi;

	va_start(ap, val);
	for (i=0 ; i < j ; i++ )
	{
		if ( val == va_arg(ap,ULONG) ) {
			if ( mi = IA(item + SHIFTSUB(i)) ) mi->Flags |= CHECKED;
		} else {
			if ( mi = IA(item + SHIFTSUB(i)) ) mi->Flags &= ~CHECKED;
		}
	}
	va_end(ap);
}

/*	Console menu handling
 */

/* get menuitem using menunumber */
struct MenuItem *IA(ULONG number)
{
	return(ItemAddress(menu_all,number));
}

/* show currrent settings in menus */
void Menu_fresh(void)
{
ulong           i,j,tmp;
struct Info     *in;
struct MenuItem *mi;

	if ( !gl_menufreeze AND gl_conwindow ) {

		/* checkit states */

		/*** project ***/
		Sub_set	(MENU_PROJECT|SHIFTITEM(13)|SHIFTSUB(0),3,CUR(PB_Priority	),-3,0,3);
		Item_set(MENU_PROJECT|SHIFTITEM(15)|SHIFTSUB(0),1,CUR(PB_Crypt		),TRUE);
		Item_set(MENU_PROJECT|SHIFTITEM(16)|SHIFTSUB(0),1,CUR(PB_Safe			),TRUE);
		Item_set(MENU_PROJECT|SHIFTITEM(17)|SHIFTSUB(0),1,CUR(PB_Dispose	),TRUE);
		Sub_set	(MENU_PROJECT|SHIFTITEM(19)|SHIFTSUB(0),2,CUR(PB_LogCall	),FALSE,TRUE);

		/*** transfer ***/
		j = DoMethod(gl_protocol,MUIM_INF_Count);
		for (i = 0; i < j; i++)
		{
			if ( mi = IA(MENU_XP|SHIFTSUB(i)) ) {
				mi->Flags &= ~CHECKED;
				if (gl_curxp) {
					if ( ((struct IntuiText *)(mi->ItemFill))->IText == gl_curxp->ln_Name ) {
						mi->Flags |= CHECKED;
					}
				}
			}
		}
		Item_set(MENU_TRANSFER|SHIFTITEM(10)|SHIFTSUB(0),1,CUR(PB_AutoXfer	),TRUE);
		Item_set(MENU_TRANSFER|SHIFTITEM(11)|SHIFTSUB(0),1,CUR(PB_QuietXfer	),TRUE);
		Item_set(MENU_TRANSFER|SHIFTITEM(12)|SHIFTSUB(0),1,CUR(PB_Double		),TRUE);
		Sub_set	(MENU_TRANSFER|SHIFTITEM(14)|SHIFTSUB(0),1,CUR(PB_Review		),TRUE);
		Sub_set	(MENU_TRANSFER|SHIFTITEM(15)|SHIFTSUB(0),2,CUR(PB_Capture		),CAP_TEXT,CAP_RAW);

		/*** serial ***/
		Sub_set	(MENU_SERIAL|SHIFTITEM( 0)|SHIFTSUB(0),11,CUR(PB_DteRate		),300,1200,2400,4800,9600,14400,19200,38400,57600,76800,115200);
		Sub_set	(MENU_SERIAL|SHIFTITEM( 1)|SHIFTSUB(0), 2,CUR(PB_DataBits		),7,8);
		Sub_set	(MENU_SERIAL|SHIFTITEM( 2)|SHIFTSUB(0), 5,CUR(PB_Parity			),PAR_NONE,PAR_EVEN,PAR_ODD,PAR_MARK,PAR_SPACE);
		Sub_set	(MENU_SERIAL|SHIFTITEM( 3)|SHIFTSUB(0), 2,CUR(PB_StopBits		),1,2);
		Sub_set	(MENU_SERIAL|SHIFTITEM( 4)|SHIFTSUB(0), 3,CUR(PB_FlowControl),FLOW_NONE,FLOW_XONXOFF,FLOW_RTSCTS);
		Sub_set	(MENU_SERIAL|SHIFTITEM( 5)|SHIFTSUB(0), 2,CUR(PB_FullDuplex	),FALSE,TRUE);
		Item_set(MENU_SERIAL|SHIFTITEM( 8)|SHIFTSUB(0), 1,CUR(PB_SerShared  ),TRUE);
		Item_set(MENU_SERIAL|SHIFTITEM( 9)|SHIFTSUB(0), 1,CUR(PB_FixRate		),TRUE);
		Item_set(MENU_SERIAL|SHIFTITEM(10)|SHIFTSUB(0), 1,CUR(PB_HardSer		),TRUE);
		Item_set(MENU_SERIAL|SHIFTITEM(11)|SHIFTSUB(0), 1,CUR(PB_DTRHangup	),TRUE);
		Item_set(MENU_SERIAL|SHIFTITEM(12)|SHIFTSUB(0), 1,((gl_serok == TRUE AND gl_serfreeze == FALSE) ? 1 : 0),1);

		/*** screen ***/
		Sub_set	(MENU_SCREEN|SHIFTITEM( 3)|SHIFTSUB(0),3,CUR(PB_Status			),STAT_NONE,STAT_BAR,STAT_WINDOW);
		j = Charset(DT_COUNT,0);
		for (i = 0; i < j; i++)
		{
			if ( mi = IA(MENU_CS|SHIFTSUB(i)) ) {
				mi->Flags &= ~CHECKED;
				if ( in = Charset(DT_GET,i) ) {
					if ( CUR(PB_HardScr) AND (GetInfoTag(CS_Xsize,in) != 8 OR GetInfoTag(CS_Ysize,in) != 8) ) {
						mi->Flags &= ~ITEMENABLED;
					} else {
						mi->Flags |= ITEMENABLED;
					}
				}
				if (gl_curcs) {
					if ( ((struct IntuiText *)(mi->ItemFill))->IText == gl_curcs->ln_Name ) {
						mi->Flags |= CHECKED;
					}
				}
			}
		}
		Item_set(MENU_SCREEN|SHIFTITEM( 6)|SHIFTSUB(0),1,CUR(PB_BorderBlank	),TRUE);
		Item_set(MENU_SCREEN|SHIFTITEM( 7)|SHIFTSUB(0),1,CUR(PB_KeepWb			),TRUE);
		Item_set(MENU_SCREEN|SHIFTITEM( 8)|SHIFTSUB(0),1,CUR(PB_Aga					),TRUE);
		Item_set(MENU_SCREEN|SHIFTITEM( 9)|SHIFTSUB(0),1,CUR(PB_HardScr			),TRUE);
		Item_set(MENU_SCREEN|SHIFTITEM(10)|SHIFTSUB(0),1,CUR(PB_RxdPopup		),TRUE);
		Item_set(MENU_SCREEN|SHIFTITEM(11)|SHIFTSUB(0),1,CUR(PB_RtsBlit			),TRUE);
		Item_set(MENU_SCREEN|SHIFTITEM(12)|SHIFTSUB(0),1,CUR(PB_ForceConXY	),TRUE);
		Item_set(MENU_SCREEN|SHIFTITEM(13)|SHIFTSUB(0),1,CUR(PB_Wb					),TRUE);
		Sub_set	(MENU_SCREEN|SHIFTITEM(16)|SHIFTSUB(0),1,CUR(PB_cr2crlf			),TRUE);
		Sub_set	(MENU_SCREEN|SHIFTITEM(16)|SHIFTSUB(1),1,CUR(PB_lf2crlf			),TRUE);
    Item_set(MENU_SCREEN|SHIFTITEM(18)|SHIFTSUB(0),1,CUR(PB_Baron       ),TRUE);
    Sub_set (MENU_SCREEN|SHIFTITEM(19)|SHIFTSUB(0),2,CUR(PB_Barpos      ),BAR_VER,BAR_HOR);
    j = Toolbar(DT_COUNT,0);
    for (i = 0; i < j; i++)
    {
      if ( mi = IA(MENU_TB|SHIFTSUB(i)) ) {
				mi->Flags &= ~CHECKED;
				if (gl_curtb) {
					if ( ((struct IntuiText *)(mi->ItemFill))->IText == gl_curtb->ln_Name ) {
						mi->Flags |= CHECKED;
					}
				}
			}
		}

		/*** emulation ***/
		Item_set(MENU_EMULATION|SHIFTITEM( 0)|SHIFTSUB(0),2,CUR(PB_Emulation	),EM_ANSI,EM_HEX);
		j = Xem(DT_COUNT,0);
		for (i = 0; i < j; i++)
		{
			if ( mi = IA(MENU_XM|SHIFTSUB(i)) ) {
				if (gl_curxm) {
					if ( ((struct IntuiText *)(mi->ItemFill))->IText == gl_curxm->ln_Name ) {
						mi->Flags |= CHECKED;
					} else {
						mi->Flags &= ~CHECKED;
					}
				} else {
					mi->Flags &= ~CHECKED;
				}
			}
		}
		Sub_set(MENU_EMULATION|SHIFTITEM( 5)|SHIFTSUB(1),1,CUR(PB_ANSIWrap		),TRUE);
		Sub_set(MENU_EMULATION|SHIFTITEM( 5)|SHIFTSUB(2),1,CUR(PB_ANSIKillBs	),TRUE);
		Sub_set(MENU_EMULATION|SHIFTITEM( 5)|SHIFTSUB(3),1,CUR(PB_ANSIStrip		),TRUE);
		Sub_set(MENU_EMULATION|SHIFTITEM( 5)|SHIFTSUB(4),1,CUR(PB_ANSICls			),TRUE);
		Sub_set(MENU_EMULATION|SHIFTITEM( 5)|SHIFTSUB(5),1,CUR(PB_ANSICr2crlf	),TRUE);
		Sub_set(MENU_EMULATION|SHIFTITEM( 5)|SHIFTSUB(6),1,CUR(PB_ANSIPc			),TRUE);

		/* Disable menus which aren't available */

		/*** project ***/
		Item_dis(MENU_PROJECT|SHIFTITEM( 2)|SHIFTSUB(0),(ULONG)gl_req);
		Item_dis(MENU_PROJECT|SHIFTITEM(11)|SHIFTSUB(0),(ULONG)gl_req);
		Item_dis(MENU_PROJECT|SHIFTITEM(12)|SHIFTSUB(0),(ULONG)gl_req);
		Item_dis(MENU_PROJECT|SHIFTITEM(20)|SHIFTSUB(0),gl_xferon);
		Item_dis(MENU_PROJECT|SHIFTITEM(22)|SHIFTSUB(0),gl_xferon|gl_dial|(ULONG)gl_req);

		/*** phonebook ***/
		Item_dis(MENU_PHONEBOOK|SHIFTITEM( 0)|SHIFTSUB(0),gl_dial);
		Item_dis(MENU_PHONEBOOK|SHIFTITEM( 2)|SHIFTSUB(0),(ULONG)gl_req);
		Item_dis(MENU_PHONEBOOK|SHIFTITEM( 5)|SHIFTSUB(0),gl_dial|(ULONG)gl_onlinepb|(ULONG)gl_req);
		Item_dis(MENU_PHONEBOOK|SHIFTITEM( 6)|SHIFTSUB(0),gl_dial|(ULONG)gl_onlinepb);

		/*** transfer ***/
		Item_dis(MENU_TRANSFER|SHIFTITEM( 0)|SHIFTSUB(0),(ULONG)gl_req);
		Item_dis(MENU_TRANSFER|SHIFTITEM( 2)|SHIFTSUB(0),gl_xferon|(ULONG)gl_req);
		Item_dis(MENU_TRANSFER|SHIFTITEM( 3)|SHIFTSUB(0),gl_xferon|(ULONG)gl_req|(ULONG)gl_asc_fh);
		Item_dis(MENU_TRANSFER|SHIFTITEM( 4)|SHIFTSUB(0),gl_xferon|(ULONG)gl_req|(ULONG)gl_asc_fh);
		Item_dis(MENU_TRANSFER|SHIFTITEM( 5)|SHIFTSUB(0),((gl_xferon|(ULONG)gl_asc_fh) ? FALSE : TRUE));
		Item_dis(MENU_TRANSFER|SHIFTITEM( 7)|SHIFTSUB(0),gl_xferon|(ULONG)gl_req);
		Item_dis(MENU_TRANSFER|SHIFTITEM( 8)|SHIFTSUB(0),gl_xferon|(ULONG)gl_req);
		Item_dis(MENU_TRANSFER|SHIFTITEM( 9)|SHIFTSUB(NOSUB),gl_xferon);
		Item_dis(MENU_TRANSFER|SHIFTITEM(10)|SHIFTSUB(0),gl_xferon);
		Item_dis(MENU_TRANSFER|SHIFTITEM(11)|SHIFTSUB(0),gl_xferon);
		Item_dis(MENU_TRANSFER|SHIFTITEM(12)|SHIFTSUB(0),gl_xferon);
		Item_dis(MENU_TRANSFER|SHIFTITEM(14)|SHIFTSUB(2),gl_req);
		Item_dis(MENU_TRANSFER|SHIFTITEM(15)|SHIFTSUB(2),((((ULONG)gl_cap_fh|(ULONG)gl_req)) ? TRUE : FALSE));
		Item_dis(MENU_TRANSFER|SHIFTITEM(15)|SHIFTSUB(3),((gl_req) ? TRUE : (gl_cap_fh) ? FALSE : TRUE));

		/*** serial ***/
		Item_dis(MENU_SERIAL|SHIFTITEM( 0)|SHIFTSUB(11),gl_req);
		Item_dis(MENU_SERIAL|SHIFTITEM( 1)|SHIFTSUB(NOSUB),TRUE);
		Item_dis(MENU_SERIAL|SHIFTITEM( 2)|SHIFTSUB(NOSUB),TRUE);
		Item_dis(MENU_SERIAL|SHIFTITEM( 3)|SHIFTSUB(NOSUB),TRUE);
		Item_dis(MENU_SERIAL|SHIFTITEM( 4)|SHIFTSUB(NOSUB),TRUE);
		Item_dis(MENU_SERIAL|SHIFTITEM( 6)|SHIFTSUB(0),gl_req);
		Item_dis(MENU_SERIAL|SHIFTITEM( 7)|SHIFTSUB(0),gl_req);
		Item_dis(MENU_SERIAL|SHIFTITEM( 8)|SHIFTSUB(NOSUB),CUR(PB_HardSer));
		Item_dis(MENU_SERIAL|SHIFTITEM( 9)|SHIFTSUB(0),TRUE);
		Item_dis(MENU_SERIAL|SHIFTITEM(12)|SHIFTSUB(0),((gl_xferon) ? (CUR(PB_DTRHangup) ? TRUE : FALSE) : FALSE));

		/*** screen ***/
		Item_dis(MENU_SCREEN|SHIFTITEM( 0)|SHIFTSUB(0),gl_req);
		Item_dis(MENU_SCREEN|SHIFTITEM( 1)|SHIFTSUB(0),gl_req);
		Item_dis(MENU_SCREEN|SHIFTITEM( 2)|SHIFTSUB(0),gl_req);
    Item_dis(MENU_SCREEN|SHIFTITEM( 3)|SHIFTSUB(3),gl_req);
		Item_dis(MENU_SCREEN|SHIFTITEM( 5)|SHIFTSUB(0),(CUR(PB_HardScr) ? FALSE : TRUE));
		Item_dis(MENU_SCREEN|SHIFTITEM( 9)|SHIFTSUB(0),((CUR(PB_Emulation) == EM_XEM OR GetInfoTag(CS_Xsize,gl_curcs) != 8 OR GetInfoTag(CS_Ysize,gl_curcs) != 8) ? TRUE : FALSE));
		Item_dis(MENU_SCREEN|SHIFTITEM(14)|SHIFTSUB(0),gl_req);


		/*** emulation ***/
		Item_dis(MENU_EMULATION|SHIFTITEM( 2)|SHIFTSUB(NOSUB),((CUR(PB_HardScr)) ? TRUE : FALSE));
		Item_dis(MENU_EMULATION|SHIFTITEM( 5)|SHIFTSUB(NOSUB),((CUR(PB_Emulation) == EM_ANSI) ? FALSE : TRUE));
		Item_dis(MENU_EMULATION|SHIFTITEM( 5)|SHIFTSUB(0),gl_req);
		Item_dis(MENU_EMULATION|SHIFTITEM( 6)|SHIFTSUB(NOSUB),((CUR(PB_Emulation) == EM_XEM) ? FALSE : TRUE));

		/*** special ***/
		Item_dis(MENU_SPECIAL|SHIFTITEM( 0)|SHIFTSUB(0),gl_req);
		Item_dis(MENU_SPECIAL|SHIFTITEM( 1)|SHIFTSUB(0),gl_req);

		ResetMenuStrip(gl_conwindow,menu_all);
	}
}

/* free menus */
void Menu_free(void)
{
	if (gl_conwindow) ClearMenuStrip(gl_conwindow);
	if (menu_all) FreeMenus(menu_all);
	menu_all = 0;
}

/* build complete menu tree */
void Menu_build(void)
{
static bool loc_done = FALSE;
ulong i,count;
struct NewMenu *menu,*mn;

  if ( !loc_done ) { /* localize menu texts in first call */
    LocalizeMenu(m1,sizeof(m1)/sizeof(struct NewMenu));
    LocalizeMenu(m2,sizeof(m2)/sizeof(struct NewMenu));
    LocalizeMenu(m3,sizeof(m3)/sizeof(struct NewMenu));
    LocalizeMenu(m4,sizeof(m4)/sizeof(struct NewMenu));
    LocalizeMenu(m5,sizeof(m5)/sizeof(struct NewMenu));
    loc_done = TRUE;
  }
	if ( gl_conwindow ) {
		Menu_free();
		count = Charset (DT_COUNT,0)+
            DoMethod(gl_protocol,MUIM_INF_Count)+
            Toolbar (DT_COUNT,0)+
            Xem     (DT_COUNT,0);
		if ( menu = mn = alloc_pool(count * sizeof(struct NewMenu)+
										 sizeof(m1)+
                     sizeof(m2)+
                     sizeof(m3)+
                     sizeof(m4)+
                     sizeof(m5)) ) {
			CopyMem(m1,mn,sizeof(m1)); mn = (UBYTE *)mn + sizeof(m1);
			if ( count = DoMethod(gl_protocol,MUIM_INF_Count) ) {
				for (i=0; i < count; i++)
				{
					mn->nm_Type					= NM_SUB;
          mn->nm_Label        = ((struct Info *)DoMethod(gl_protocol,MUIM_INF_Get,i))->ln_Name;
					mn->nm_CommKey			= 0;
					mn->nm_Flags				= CHECKIT;
					mn->nm_MutualExclude= 0;
					mn->nm_UserData			= ID_xp;
					mn++;
				}
			}
			CopyMem(m2,mn,sizeof(m2)); mn = (UBYTE *)mn + sizeof(m2);
			if ( count = Charset(DT_COUNT,0) ) {
				for (i=0; i < count; i++)
				{
					mn->nm_Type					= NM_SUB;
					mn->nm_Label				= ((struct Info *)Charset(DT_GET,i))->ln_Name;
					mn->nm_CommKey			= 0;
					mn->nm_Flags				= CHECKIT;
					mn->nm_MutualExclude= 0;
					mn->nm_UserData			= ID_cs;
					mn++;
				}
			}
			CopyMem(m3,mn,sizeof(m3)); mn = (UBYTE *)mn + sizeof(m3);
			if ( count = Toolbar(DT_COUNT,0) ) {
				for (i=0; i < count; i++)
				{
					mn->nm_Type					= NM_SUB;
					mn->nm_Label				= ((struct Info *)Toolbar(DT_GET,i))->ln_Name;
					mn->nm_CommKey			= 0;
					mn->nm_Flags				= CHECKIT;
					mn->nm_MutualExclude= 0;
					mn->nm_UserData			= ID_tb;
					mn++;
				}
			}
			CopyMem(m4,mn,sizeof(m4)); mn = (UBYTE *)mn + sizeof(m4);
			if ( count = Xem(DT_COUNT,0) ) {
				for (i=0; i < count; i++)
				{
					mn->nm_Type					= NM_SUB;
					mn->nm_Label				= ((struct Info *)Xem(DT_GET,i))->ln_Name;
					mn->nm_CommKey			= 0;
					mn->nm_Flags				= CHECKIT;
					mn->nm_MutualExclude= 0;
					mn->nm_UserData			= ID_xm;
					mn++;
				}
			}
			CopyMem(m5,mn,sizeof(m5)); mn = (UBYTE *)mn + sizeof(m5);
		}
		if ( menu_all = CreateMenus(menu,NULL) ) {
			if ( LayoutMenus(menu_all,gl_vi,GTMN_NewLookMenus,TRUE,TAG_END) ) {
				SetMenuStrip(gl_conwindow,menu_all);
				Menu_fresh();
			} else {
				FreeMenus(menu_all); menu_all = 0;
				EndProg("Cannot layout menus!"); 
			}
		} else {
			EndProg("Cannot create menus!");
		}
		FreeMyPooled(menu);
	}
}

/* menu action subroutines for async reqs... argh
 */

/*** project ***/

void Phoneprefix(void)
{
static char str[STRSIZE];

  if (!gl_req) {
    strcpy(str,GetTag_info(PB_PhonePre,gl_defbbs));
    Ask_string(&Phoneprefix,LOC(MSG_change_profile_phone_prefix),str);
  } else {
    if ( Ask_string_done() == REQ_OK ) {
      AddInfoTag(PB_PhonePre,(ulong)str,gl_defbbs);
      Phone(PB_REDRAW,0);
    }
  }
}

/*** phonebook ***/

/*** transfer ***/

static void RevSize(void)
{
static long val;

  if (!gl_req) {
    val = CUR(PB_ReviewSize);
    Ask_slider(&RevSize,"Review lines",100,100000,&val);
  } else {
    if ( Ask_slider_done() == REQ_OK ) {
      AddInfoTag(PB_ReviewSize,val,gl_curbbs);
      set(gl_review,MUIA_REV_MaxLines,val);
    }
  }
}

/*** serial ***/

/*** screen ***/

static void ScrMode(void)
{
static struct ScreenModeRequester	smr;

	if (!gl_req) {
		smr.sm_DisplayID		= CUR(PB_ModeId			);
		smr.sm_DisplayWidth	= CUR(PB_ConX				);
		smr.sm_DisplayHeight= CUR(PB_ConY				);
		smr.sm_DisplayDepth = CUR(PB_Depth			);
		smr.sm_OverscanType	= CUR(PB_Overscan		);
		smr.sm_AutoScroll		= CUR(PB_AutoScroll	);
		Ask_screenmode(gl_conwindow,&ScrMode,&smr);
	} else {
		if ( Ask_screenmode_done() == REQ_OK ) {
			SetUpTags(0,
								PB_ModeId			,smr.sm_DisplayID			,
								PB_ConX				,smr.sm_DisplayWidth	,
								PB_ConY				,smr.sm_DisplayHeight	,
								PB_Depth			,smr.sm_DisplayDepth	,
								PB_Overscan		,smr.sm_OverscanType	,
								PB_AutoScroll	,smr.sm_AutoScroll		, TAG_END);
		}
	}
}

static void ScrLines(void)
{
static LONG	val;

	if (!gl_req) {
		if ( CUR(PB_HardScr) ) {
			val = CUR(PB_HLines);
			Ask_slider(&ScrLines,LOC(MSG_hardware_lines),24,32,&val);
		} else {
			val = CUR(PB_Lines);
			Ask_slider(&ScrLines,LOC(MSG_lines),1,256,&val);
		}
	} else {
		if ( Ask_slider_done() == REQ_OK ) {
			if ( CUR(PB_HardScr) ) {
				SetUpTag(PB_HLines,val);
			} else {
				SetUpTag(PB_Lines,val);
			}
		}
	}
}

/*** emulation ***/

/*** special ***/

static void direct(void)
{
static char name_dir[STRSIZE];

	if (!gl_req) {
		Ask_file(gl_conwindow,&direct,LOC(MSG_raw_direct_to_emulation),"#?",name_dir);
	} else {
		if ( Ask_file_done() == REQ_OK ) {
			gl_asc_fh		= dt_open(name_dir,MODE_READ,8192);
			gl_asc_mode = 2;
			Menu_fresh();
		}
	}
}

static void runcom(void)
{
static char	com[STRSIZE];
BPTR	doscommand;
char	*comarg;

	if (!gl_req) {
		Ask_string(&runcom,LOC(MSG_enter_command_arguments),com);
	} else {
		if ( Ask_string_done() == REQ_OK ) {
			if ( (comarg = strchr(com,' ')) ) {
				*comarg = 0x00;
				comarg++;
				strcat(comarg,"\n");
			}
			if ( (doscommand = LoadSeg(com)) ) {
				CreateNewProcTags(NP_Seglist			,doscommand		,
													NP_Name					,com					,
													NP_CommandName	,FilePart(com),
													NP_Cli					,TRUE					,
													NP_FreeSeglist	,TRUE					,
													NP_Arguments		,comarg				,	TAG_END);
			}
			if (comarg) {
				comarg--; *comarg = ' ';
				*(strchr(comarg,0x0a)) = 0x00;
			}
		}
	}
}

#ifdef DEBUG

#include <clib/timer_protos.h>
#include <pragmas/timer_pragmas.h>
extern struct Library *TimerBase; /* in dterm.c */

static void selftest(void)
{
char  str[STRSIZE];
long  old_call_alloc = gl_call_alloc,
      old_call_free  = gl_call_free,
      old_call_force = gl_call_force,
      old_tags_add   = gl_tags_add,
      old_tags_del   = gl_tags_del,
      old_mem_add    = gl_mem_add,
      old_mem_del    = gl_mem_del,
      i,j;
struct Info    *info;
struct timeval c1,c2;

  sprintf(str,"\33[2J\33[37m%s self analysis...\r\n\r\n",&ver_dterm[6]);EmPrint(str,strlen(str));
  sprintf(str,"alloc_pool  calls   : %ld\r\n",gl_call_alloc); EmPrint(str,strlen(str));
  sprintf(str,"alloc_free  calls   : %ld\r\n",gl_call_free);  EmPrint(str,strlen(str));
  sprintf(str,"alloc_force calls   : %ld\r\n",gl_call_force); EmPrint(str,strlen(str));
  sprintf(str,"memory allocated    : %ld\r\n",gl_mem_add);    EmPrint(str,strlen(str));
  sprintf(str,"memory deallocated  : %ld\r\n",gl_mem_del);    EmPrint(str,strlen(str));
  sprintf(str,"memory allocated now: %ld\r\n",gl_mem_now);    EmPrint(str,strlen(str));

  strcpy(str,"\r\n"); EmPrint(str,strlen(str));

  sprintf(str,"tags added    : %ld\r\n",gl_tags_add); EmPrint(str,strlen(str));
  sprintf(str,"tags deleted  : %ld\r\n",gl_tags_del); EmPrint(str,strlen(str));
  sprintf(str,"tags now      : %ld\r\n",gl_tags_now); EmPrint(str,strlen(str));
  sprintf(str,"tags now (ign): %ld\r\n",gl_tags_ign); EmPrint(str,strlen(str));

  strcpy(str,"\r\n"); EmPrint(str,strlen(str));

  sprintf(str,"String size          : %ld\r\n",STRSIZE);        EmPrint(str,strlen(str));
  sprintf(str,"Txd buffer size      : %ld\r\n",gl_txdsize);     EmPrint(str,strlen(str));
  sprintf(str,"Capture internal size: %ld\r\n",gl_cap_bufsize); EmPrint(str,strlen(str));

  strcpy(str,"\r\nExecuting speed test...\r\n\r\n"); EmPrint(str,strlen(str));

  /*** test 1 ***/
  GetSysTime(&c1);
  for (i=0; i < 1000; i++)
  {
    if (info = CreateInfo()) DeleteInfo(info);
  }
  GetSysTime(&c2);
  SubTime(&c2,&c1);
  sprintf(str,"CreateInfo/DeleteInfo pair (empty)      : %ld.%06ld (%ld per one)\r\n",
          c2.tv_secs,c2.tv_micro,((c2.tv_secs * 1000000)+c2.tv_micro)/1000);
  EmPrint(str,strlen(str));

  /*** test 2 ***/
  if (info = CreateInfo()) {
    GetSysTime(&c1);
    for (i=0; i < 1000; i++)
    {
      for (j = 0; j < 32; j++) AddInfoTag(VALTAG+j,0,info);
      for (j = 0; j < 32; j++) DelInfoTag(VALTAG+j,info);
    }
    GetSysTime(&c2);
    SubTime(&c2,&c1);
    sprintf(str,"Adding/deleting 32 value tags from info : %ld.%06ld (%ld per one)\r\n",
            c2.tv_secs,c2.tv_micro,((c2.tv_secs * 1000000)+c2.tv_micro)/1000);
    EmPrint(str,strlen(str));
    DeleteInfo(info);
  }

  /*** test 3 ***/
  if (info = CreateInfo()) {
    GetSysTime(&c1);
    for (i=0; i < 1000; i++)
    {
      for (j = 0; j < 32; j++) AddInfoTag(STRTAG+j,"0123456789012345",info);
      for (j = 0; j < 32; j++) DelInfoTag(STRTAG+j,info);
    }
    GetSysTime(&c2);
    SubTime(&c2,&c1);
    sprintf(str,"Adding/deleting 32 string tags from info: %ld.%06ld (%ld per one)\r\n",
            c2.tv_secs,c2.tv_micro,((c2.tv_secs * 1000000)+c2.tv_micro)/1000);
    EmPrint(str,strlen(str));
    DeleteInfo(info);
  }

  /*** test 4 ***/
  GetSysTime(&c1);
  for (i=0; i < 1000; i++)
  {
    if (info = alloc_pool(128)) free_pool(info);
  }
  GetSysTime(&c2);
  SubTime(&c2,&c1);
  sprintf(str,"Allocating/deallocating 128 bytes       : %ld.%06ld (%ld per one)\r\n",
          c2.tv_secs,c2.tv_micro,((c2.tv_secs * 1000000)+c2.tv_micro)/1000);
  EmPrint(str,strlen(str));

  /*** test 5 ***/
  if (info = CreateInfo()) {
    for (j = 0; j < 32; j++) AddInfoTag(VALTAG+j,0,info);
    GetSysTime(&c1);
    for (i=0; i < 1000; i++)
    {
      for (j = 0; j < 32; j++) GetInfoTag(VALTAG+j,info);
    }
    GetSysTime(&c2);
    SubTime(&c2,&c1);
    sprintf(str,"Getting 32 tags from info               : %ld.%06ld (%ld per one)\r\n",
            c2.tv_secs,c2.tv_micro,((c2.tv_secs * 1000000)+c2.tv_micro)/1000);
    EmPrint(str,strlen(str));
    DeleteInfo(info);
  }

  strcpy(str,"\r\nDone.\r\n"); EmPrint(str,strlen(str));

  gl_call_alloc = old_call_alloc;
  gl_call_free  = old_call_free;
  gl_call_force = old_call_force;
  gl_tags_add   = old_tags_add;
  gl_tags_del   = old_tags_del;
  gl_mem_add    = old_mem_add;
  gl_mem_del    = old_mem_del;
}
#endif

/* menu action maingod */
static void Menu_act(struct MenuItem *mi)
{
  switch ( (ULONG)GTMENUITEM_USERDATA(mi) )
  {
    case ID_cs:
      SetUpTag(PB_Charset,((struct IntuiText *)mi->ItemFill)->IText);
      break;
    case ID_xp:
      SetUpTag(PB_Protocol,((struct IntuiText *)mi->ItemFill)->IText);
      break;
    case ID_tb:
      AddInfoTag(PB_Toolbar,((struct IntuiText *)mi->ItemFill)->IText,gl_curbbs);
      if (gl_curtb = Toolbar(DT_FIND,((struct IntuiText *)mi->ItemFill)->IText)) {
        if (CUR(PB_Baron)) Toolbar(TB_ENABLE,gl_curtb);
      }
      Menu_fresh();
      break;
    case ID_xm:
      SetUpTags(0,
								PB_Emulation,EM_XEM				,
								PB_Xem			,((struct IntuiText *)mi->ItemFill)->IText, TAG_END);
			break;

		/*** project ***/
		case ID_confcurrent	: com(cmd_open_current); break;
		case ID_confprofile	: com(cmd_open_profile); break;
    case ID_phoneprefix	: Phoneprefix()        ; break;
    case ID_confxp			: com(cmd_open_xp)     ; break;
		case ID_confcs			: com(cmd_open_cs)     ; break;
		case ID_confxm			: com(cmd_open_xm)     ; break;
		case ID_conftb			: com(cmd_open_tb)     ; break;
		case ID_loaddef			: com(cmd_load_default); break;
		case ID_savedef			: com(cmd_save_default); break;
		case ID_titlebar		: com(cmd_titlebar)    ; break;
		case ID_tempdir			:
      Ask_dir_tag(LOC(MSG_select_tempdir),PB_TempDir,gl_curbbs);
      break;
		case ID_initstring	:
      Ask_str_tag(LOC(MSG_startup_initstring),PB_InitString,gl_curbbs);
      break;
		case ID_pri3m				:
			AddInfoTag(PB_Priority,-3,gl_curbbs);
			SetTaskPri((struct Task *)gl_process,-3);
			Menu_fresh();
			break;
		case ID_pri0				:
			AddInfoTag(PB_Priority,0,gl_curbbs);
			SetTaskPri((struct Task *)gl_process,0);
			Menu_fresh();
			break;
		case ID_pri3				:
			AddInfoTag(PB_Priority,3,gl_curbbs);
			SetTaskPri((struct Task *)gl_process,3);
			Menu_fresh();
			break;
		case ID_cryptmode		:
			AddInfoTag(PB_Crypt,TF(CUR(PB_Crypt)),gl_curbbs);
			Menu_fresh();
			break;
		case ID_autosafe		:
			AddInfoTag(PB_Safe,TF(CUR(PB_Safe)),gl_curbbs);
			Menu_fresh();
			break;
    case ID_dispose     :
			AddInfoTag(PB_Dispose,TF(CUR(PB_Dispose)),gl_curbbs);
			Menu_fresh();
			break;
		case ID_logno				: SetUpTag(PB_LogCall,FALSE); break;
		case ID_logcall			: SetUpTag(PB_LogCall,TRUE) ;	break;
		case ID_cryptset		: com(cmd_open_crypt)       ; break;
		case ID_about				: com(cmd_about)            ; break;
		case ID_quit			  : com(cmd_quit)             ; break;

    /*** phonebook ***/
    case ID_directory   : com(cmd_open_phonebook); break;
    case ID_newentry    : com(cmd_new_entry)     ; break;
    case ID_loadbook    : com(cmd_load_phonebook); break;
    case ID_savebook    : com(cmd_save_phonebook); break;
    case ID_dialnumber  : com(cmd_dial_number,0) ; break;
    case ID_dialnext    : com(cmd_dial_next)     ; break;
    case ID_sendpassword: com(cmd_send_password) ; break;

    /*** transfer ***/
		case ID_upload		: com(cmd_upload)      ; break;
		case ID_uploadid	: com(cmd_uploadlist)  ; break;
		case ID_dnload		: com(cmd_download)    ; break; 
		case ID_sendconv	: com(cmd_send_conv)   ; break;
		case ID_sendnoconv: com(cmd_send_no_conv); break;
		case ID_abortsend	: com(cmd_abort_send)  ; break;
		case ID_showid		: com(cmd_open_uplist) ; break;
		case ID_uppath		:
      Ask_dir_cur(LOC(MSG_select_upload_path),PB_UpPath);
      break;
		case ID_dnpath		:
      Ask_dir_cur(LOC(MSG_select_download_path),PB_DnPath);
      break;
		case ID_autoxfer	: SetUpTag(PB_AutoXfer	,TF(CUR(PB_AutoXfer)));		break;
    case ID_quietxfer	: SetUpTag(PB_QuietXfer	,TF(CUR(PB_QuietXfer)));	break;
    case ID_double    : SetUpTag(PB_Double		,TF(CUR(PB_Double)));			break;
    case ID_togglerev :
      AddInfoTag(PB_Review,TF(CUR(PB_Review)),gl_curbbs);
      if (CUR(PB_Review)) {
        set(gl_review,MUIA_REV_MaxLines,CUR(PB_ReviewSize));
/*
        if ((gl_revtemp = alloc_pool(gl_revtempsize)) == 0) {
          AddInfoTag(PB_Review,FALSE,gl_curbbs);
        }
*/
      } else {
/*
        if (gl_revtemp) {
*/
          DoMethod(gl_review,MUIM_REV_Clear);
/*
          free_pool(gl_revtemp);
          gl_revtemp = 0;
        }
*/
      }
			Menu_fresh();
			break;
		case ID_revshow		: com(cmd_open_review); break;
		case ID_revsize		: RevSize(); break;
		case ID_captext		:
			AddInfoTag(PB_Capture,CAP_TEXT,gl_curbbs);
			Menu_fresh();
			break;
		case ID_capraw		:
			AddInfoTag(PB_Capture,CAP_RAW,gl_curbbs);
			Menu_fresh();
			break;
		case ID_capopen		: com(cmd_start_capture); break;
		case ID_capclose	:
			if (gl_cap_fh) {
				if ( (gl_cap_now-gl_cap_buf) ) dt_write(gl_cap_fh,gl_cap_buf,gl_cap_now-gl_cap_buf);
				dt_close(gl_cap_fh); gl_cap_fh = 0;
			}
			Menu_fresh();
			break;

		/*** serial ***/
		case ID_dte300		: SetUpTag(PB_DteRate,300);		break;
		case ID_dte1200		: SetUpTag(PB_DteRate,1200);	break;
		case ID_dte2400		: SetUpTag(PB_DteRate,2400);	break;
		case ID_dte4800		: SetUpTag(PB_DteRate,4800);	break;
		case ID_dte9600		: SetUpTag(PB_DteRate,9600);	break;
		case ID_dte14400	: SetUpTag(PB_DteRate,14400);	break;
		case ID_dte19200	: SetUpTag(PB_DteRate,19200);	break;
		case ID_dte38400	: SetUpTag(PB_DteRate,38400);	break;
		case ID_dte57600	: SetUpTag(PB_DteRate,57600);	break;
		case ID_dte76800	: SetUpTag(PB_DteRate,76800);	break;
		case ID_dte115200	: SetUpTag(PB_DteRate,115200);break;
		case ID_dteenter	:
      Ask_val_cur(LOC(MSG_dte_rate),2,292000,PB_DteRate);
      break;
		case ID_halfdup		: SetUpTag(PB_FullDuplex,FALSE);break;
		case ID_fulldup		: SetUpTag(PB_FullDuplex,TRUE);	break;
		case ID_serdev		:
      Ask_str_cur(LOC(MSG_serial_device),PB_SerName);
      break;
		case ID_serunit		:
      Ask_val_cur(LOC(MSG_serial_unit),0,255,PB_SerUnit);
      break;
		case ID_sershared : SetUpTag(PB_SerShared ,TF(CUR(PB_SerShared))); break;
		case ID_hardser		: SetUpTag(PB_HardSer		,TF(CUR(PB_HardSer)));	 break;
		case ID_dtrhang		: SetUpTag(PB_DTRHangup	,TF(CUR(PB_DTRHangup))); break;
		case ID_hangup		: com(cmd_hangup)        ; break;
		case ID_relser		: com(cmd_release_serial); break;

		/*** screen ***/
		case ID_scrcolumns:
      Ask_val_cur(LOC(MSG_columns),1,256,PB_Columns);
      break;
		case ID_scrmode		: ScrMode();		break;
		case ID_scrlines	: ScrLines();		break;
		case ID_statnone	: SetUpTag(PB_Status,STAT_NONE);	break;
		case ID_statline	: SetUpTag(PB_Status,STAT_BAR);		break;
		case ID_statwindow: SetUpTag(PB_Status,STAT_WINDOW);break;
    case ID_statusline:
      Ask_str_cur(LOC(MSG_status_line),PB_StatusLine);
      break;
		case ID_pophard		:
			if (gl_scrok) gl_hardstat = TF(gl_hardstat);
			break;
		case ID_borblank	: SetUpTag(PB_BorderBlank	,TF(CUR(PB_BorderBlank)));break;
		case ID_keepwb		: SetUpTag(PB_KeepWb			,TF(CUR(PB_KeepWb)));			break;
		case ID_aga				: SetUpTag(PB_Aga					,TF(CUR(PB_Aga)));				break;
		case ID_hardscr		: SetUpTag(PB_HardScr			,TF(CUR(PB_HardScr)));		break;
		case ID_rxdpop		: SetUpTag(PB_RxdPopup		,TF(CUR(PB_RxdPopup))); 	break;
		case ID_rtsblit		: SetUpTag(PB_RtsBlit			,TF(CUR(PB_RtsBlit)));		break;
		case ID_wb				: SetUpTag(PB_Wb					,TF(CUR(PB_Wb)));					break;
		case ID_forceconxy: SetUpTag(PB_ForceConXY	,TF(CUR(PB_ForceConXY)));	break;
		case ID_pubscreen	:
      Ask_str_cur(LOC(MSG_public_screen_name),PB_PubScreen);
      break;
		case ID_cr2crlf		: SetUpTag(PB_cr2crlf	,TF(CUR(PB_cr2crlf)));	break;
		case ID_lf2crlf		: SetUpTag(PB_lf2crlf	,TF(CUR(PB_lf2crlf)));	break;
    case ID_barver    :
      AddInfoTag(PB_Barpos,BAR_VER,gl_curbbs);
      if (CUR(PB_Baron)) Toolbar(TB_ENABLE ,gl_curtb);
      Menu_fresh();
      break;
    case ID_barhor    :
      AddInfoTag(PB_Barpos,BAR_HOR,gl_curbbs);
      if (CUR(PB_Baron)) Toolbar(TB_ENABLE ,gl_curtb);
      Menu_fresh();
      break;
    case ID_baron     :
      AddInfoTag(PB_Baron,TF(CUR(PB_Baron)),gl_curbbs);
      if (CUR(PB_Baron)) {
        Toolbar(TB_ENABLE ,gl_curtb);
      } else {
        Toolbar(TB_DISABLE,0);
      }
      Menu_fresh();
      break;

    /*** emulation ***/
    case ID_emulreset	: EmReset	(&gl_curbbs->tags);			break;
    case ID_setansi		: SetUpTag(PB_Emulation,EM_ANSI);	break;
    case ID_sethex		: SetUpTag(PB_Emulation,EM_HEX);	break;
    case ID_ansipre		:
      Ask_val_cur(LOC(MSG_prescroll),0,8192,PB_PreScroll);
      break;
    case ID_ansiwrap	: SetUpTag(PB_ANSIWrap    ,TF(CUR(PB_ANSIWrap))); break;
    case ID_ansikillbs: SetUpTag(PB_ANSIKillBs  ,TF(CUR(PB_ANSIKillBs))); break;
    case ID_ansistrip	: SetUpTag(PB_ANSIStrip   ,TF(CUR(PB_ANSIStrip))); break;
    case ID_ansicls		: SetUpTag(PB_ANSICls     ,TF(CUR(PB_ANSICls))); break;
    case ID_ansicr		: SetUpTag(PB_ANSICr2crlf ,TF(CUR(PB_ANSICr2crlf))); break;
    case ID_ansipc		: SetUpTag(PB_ANSIPc      ,TF(CUR(PB_ANSIPc))); break;
    case ID_xemchange	: EmChange(&gl_curbbs->tags); break;

		/*** special ***/
		case ID_direct		: direct(); break;
		case ID_runcom		: runcom(); break;
#ifdef DEBUG
		case ID_selftest  : selftest(); break;
#endif

		default:
			EndProg("Menu_act()");
	}
	if (gl_hardstat) ScreenOn();	/* hm... */
}

/* handle menuitem messages */
void Menu_handle(UWORD *code)
{
struct MenuItem	*mi;

	while (*code != MENUNULL)
	{
		if (mi = IA(*code)) {
			if ( (mi->Flags & ITEMENABLED) ) {
#ifdef DEBUG
	printf("menu start: %lx\n",(ULONG)*code);
#endif
				Menu_act(mi);
#ifdef DEBUG
	printf("menu end  : %lx\n",(ULONG)*code);
#endif
			}
		}
		code++;
	}
}
