#include "global.h"

/*
char ver_driver_protocol[]="1.00 (Apr 16 1996)";
*/

static struct XP_Msg  *main_loop(ulong);

static char            init     [STRSIZE];
static char            dnlog    [STRSIZE];
static char            dnpath   [STRSIZE];
static char            bbsname  [STRSIZE];
static char            xprname  [STRSIZE];
static char            upcom_seg[STRSIZE];
static char            dncom_seg[STRSIZE];
static char           *upcom_arg;
static char           *dncom_arg;
static ulong           type;
static ulong           dte;
static struct List     filelist;
static struct XP_Sub  *sub;
static struct XPR_IO   xpr_io;
static struct Library *XProtocolBase;

/***----------------------------------------------------------------------***/
/*** support                                                              ***/
/***----------------------------------------------------------------------***/

/*
 * Send message to main program, it must reply _after_ it has done what is
 * requested.
 *
 * cmd   : command
 * arg   : possible args
 * arg2  : 
 * arg3  : 
 *
 * return: TRUE/FALSE
 *
 */
static ulong send_main(ulong cmd, ulong arg, ulong arg2, ulong arg3)
{
  sub->mp_msg.mn_Length    = sizeof(struct XP_Msg);
  sub->mp_msg.cmd          = cmd;
  sub->mp_msg.arg          = arg;
  sub->mp_msg.arg2         = arg2;
  sub->mp_msg.arg3         = arg3;
  sub->mp_msg.mn_ReplyPort = sub->mp_reply;
  PutMsg  (sub->mp_port,&sub->mp_msg);
  WaitPort(sub->mp_reply);
  GetMsg  (sub->mp_reply);
  return  (sub->mp_msg.result);
}

/*
 * Do normal DOS open using xpr_fopen parameters
 *
 * name  : filename
 * mode  : mode
 *
 * return: fh
 *
 */
static BPTR fopen_xpr(strptr name, strptr mode)
{
BPTR  fh;
ulong dosmode;

  if (mode[0] == 'r') {
    if (mode[1] == '+') {
      dosmode = MODE_READWRITE;
    } else {
      dosmode = MODE_OLDFILE;
    }
  }
  if (mode[0] == 'w') dosmode = MODE_NEWFILE;
  if (mode[0] == 'a') dosmode = MODE_READWRITE;
  fh = Open(name,dosmode);
  if (mode[0] == 'a' AND fh) Seek(fh,0,OFFSET_END);
  return(fh);
}

/***----------------------------------------------------------------------***/
/*** xpr callbacks                                                        ***/
/***----------------------------------------------------------------------***/

static char         list_str[25];
static ulong        list_time;
static ulong        list_size;
static bptr         list_fh;
static bptr         list_fhdn;
static long         xpr_dir;
static long         xpr_abort;
static long         xpr_norecreq;
static long         xpr_nosndreq;
static long         xpr_xpr2001;
static long         xpr_double;
static long         xpr_noupdate;
static struct Node *id_first;
static struct Node *id_next;

static BPTR __asm __saveds xpr_fopen(register __a0 strptr name, register __a1 strptr mode)
{
char str[STRSIZE];

  if ( (xpr_xpr2001 AND mode[0] == 'r') OR (!xpr_xpr2001 AND xpr_dir) ) {
    return(fopen_xpr(name,mode));
  } else {
    strncpy(list_str,name,24);
    list_time = GetSec(); /* eep, shared lib base here! */
    list_size = 0;
    strcpy(str,dnpath);
    AddPart(str,FilePart(name),STRSIZE-1);
    return(list_fh = fopen_xpr(str,mode));
  }
}

static void __asm __saveds xpr_fclose(register __a0 BPTR fh)
{
char str[STRSIZE+64];

  Close(fh);

  if (fh == list_fh AND list_size != 0 AND list_fhdn != 0) {
    list_time = GetSec() - list_time;
    if (list_time == 0) list_time = 1;
    sprintf(str,"%-24s %7ld %5ld ",list_str,list_size,list_size/list_time);
    strncat(str,bbsname,38);
    strcat(str,"\n");
    Write(list_fhdn,str,strlen(str));
    list_fh = 0;
  }
}

static long __asm __saveds xpr_fread(REG __a0 APTR buffer, REG __d0 LONG size,
                                     REG __d1 LONG count,  REG __a1 BPTR fh)
{
  return(FRead(fh,buffer,size,count));
}

static long __asm __saveds xpr_fwrite(REG __a0 APTR buffer, REG __d0 LONG size,
                                      REG __d1 LONG count,  REG __a1 BPTR fh)
{
  if (fh == list_fh) list_size += size*count;
  return(FWrite(fh,buffer,size,count));
}

static long __asm __saveds xpr_sread(REG __a0 APTR buffer, REG __d0 LONG size,
                                     REG __d1 LONG timeout)
{
  return(send_main(XP_sread,buffer,size,timeout)); /* ret -1 for error, other is size */
}

static long __asm __saveds xpr_swrite(REG __a0 APTR buffer, REG __d0 LONG size)
{
  return(send_main(XP_swrite,buffer,size,0)); /* 0 for succes, other is failure */
}

static long __asm __saveds xpr_sflush(void)
{
  send_main(XP_sflush,0,0,0);
  return(0);
}

static void __asm __saveds xpr_update(REG __a0 struct XPR_UPDATE *update)
{
  if (xpr_xpr2001) {
    send_main(XP_update,update,(update->xpru_updatemask & XPRU_UPLOAD) ? 1 :0,0);
  } else {
    send_main(XP_update,update,(xpr_dir) ? 1 : 0,0);
  }
}

static long __asm __saveds xpr_chkabort(void)
{
  return(send_main(XP_chkabort,0,0,0));
}

static void __asm __saveds xpr_chkmisc(void)
{
}

static long __asm __saveds xpr_setserial(REG __d0 long status)
{
  if (dte <=    110) return(0x000094);
  if (dte <=    300) return(0x010094);
  if (dte <=   1200) return(0x020094);
  if (dte <=   2400) return(0x030094);
  if (dte <=   4800) return(0x040094);
  if (dte <=   9600) return(0x050094);
  if (dte <=  19200) return(0x060094);
  if (dte <=  38400) return(0x080094);
  if (dte <=  57600) return(0x090094);
  if (dte <=  76800) return(0x0A0094);
  if (dte <= 115200) return(0x0B0094);
  return(0x0B0094);
}

static long __asm __saveds xpr_ffirst(REG __a0 STRPTR buffer, REG __a1 STRPTR pattern)
{
  strncpy(buffer,id_first->ln_Name,255);
  id_next = id_first->ln_Succ;
  return(1);
}

static long __asm __saveds xpr_fnext(REG __d0 state, REG __a0 STRPTR buffer, REG __a1 STRPTR pattern)
{
  if (id_next->ln_Succ) {
    strncpy(buffer,id_next->ln_Name,255);
    id_next = id_next->ln_Succ;
    return(1);
  }
  return(0);
}

static long __asm __saveds xpr_finfo(REG __a0 STRPTR name, REG __d0 LONG itype)
{
char str[STRSIZE];
long size;

  if ( (xpr_xpr2001 AND (itype & 0x80000000)) OR (!xpr_xpr2001 AND xpr_dir) ) {
    strncpy(str,name,STRSIZE-1);
  } else {
    strcpy(str,dnpath);
    AddPart(str,FilePart(name),STRSIZE-1);
  }
  if ( (itype & 0xf) == 1) {
    if ( (size = FileSize(str)) == -1 ) return(0); /* eep, shares DOSBASE! */
    return(size);
  }
  if ( (itype & 0xf) == 2) {
    return(1); /* always binary */
  }
  return(0);
}

static long __asm __saveds xpr_fseek(REG __a0 BPTR fh, REG __d0 LONG offset,
                              REG __d1 LONG origin)
{
long r;

  if (origin == 0) r = Seek(fh,offset,OFFSET_BEGINNING);
  if (origin == 1) r = Seek(fh,offset,OFFSET_CURRENT);
  if (origin == 2) r = Seek(fh,offset,OFFSET_END);
  if (r == -1) return(-1);
  return(0);
}

static aptr __asm __saveds xpr_unlink(REG __a0 STRPTR name)
{
  return(0);
}

static long __asm __saveds xpr_squery(void)
{
  return(send_main(XP_squery,0,0,0));
}

static long __asm __saveds xpr_getptr(REG __d0 LONG itype)
{
  if (itype == 1) return(gl_conscreen);
  return(-1);
}

/***----------------------------------------------------------------------***/
/*** driver task                                                          ***/
/***----------------------------------------------------------------------***/

/*
 * Close xpr library and deallocate everything
 *
 * -     : -
 *
 * return: TRUE/FALSE
 *
 */
static bool close_xpr(void)
{
struct Node *node;

  if (XProtocolBase) {
    XProtocolCleanup(&xpr_io);
    CloseLibrary(XProtocolBase);
    if (list_fhdn) Close(list_fhdn);
    while (node = RemHead(&filelist))
    {
      FreeVec(node->ln_Name);
      FreeVec(node);
    }
    XProtocolBase = 0;
    list_fhdn     = 0;
  }
}

/*
 * Open xpr library and prepare structures
 *
 * -     : -
 *
 * return: TRUE/FALSE
 *
 */
static bool open_xpr(void)
{
ulong tmp;

  xpr_io.xpr_filename = init;
  if (XProtocolBase = OpenLibrary(xprname,0)) {
    tmp = XProtocolSetup(&xpr_io);
    if ( (tmp & (XPRS_HOSTMON|XPRS_USERMON|XPRS_HOSTNOWAIT)) == 0) {
      if ( (tmp & XPRS_SUCCESS) ) {
        xpr_abort    = 0;
        xpr_norecreq = (tmp & XPRS_NORECREQ) ? TRUE : FALSE;
        xpr_nosndreq = (tmp & XPRS_NOSNDREQ) ? TRUE : FALSE;
        xpr_xpr2001  = (tmp & XPRS_XPR2001 ) ? TRUE : FALSE;
        xpr_double   = (tmp & XPRS_DOUBLE  ) ? TRUE : FALSE;
        xpr_noupdate = (tmp & XPRS_NOUPDATE) ? TRUE : FALSE;
        if (list_fhdn = Open(dnlog,MODE_READWRITE)) Seek(list_fhdn,0,OFFSET_END);
        return(TRUE);
      }
    }
  }
  close_xpr();
  return(FALSE);
}

/*
 * Handle input coming from main
 *
 * do_wait: 
 *
 * return : termination msg
 *
 */
static struct XP_Msg *main_loop(ulong do_wait)
{
ulong           running = TRUE;
strptr         *name;
struct Node    *node;
struct XP_Msg  *msg;

  while (running) 
  {
    while (running AND (msg = GetMsg(sub->ch_port)))
    {
      /* note: When doing transfer you MUST not send msg's to here as it will
       *       freeze both process.
       *
       *       Remember also that we _must_ update XP_double before using
       *       XP_swrite!
       */
      msg->result = FALSE;
      switch (msg->cmd)
      {
        case PB_DteRate   : dte  = msg->arg; break;
        case PB_DnPath    : strncpy (dnpath ,msg->arg,STRSIZE); break;
        case PB_Name      : strncpy (bbsname,msg->arg,STRSIZE); break;
        case PB_DnFiles   : strncpy (dnlog  ,msg->arg,STRSIZE); break;
        case XP_Init      : strncpy (init   ,msg->arg,STRSIZE); break;
        case XP_XprName   : strncpy (xprname,msg->arg,STRSIZE); break;
        case XP_Type      : type = msg->arg; break;
        case XP_DnCom     :
          strncpy(dncom_seg,msg->arg,STRSIZE-1);
          break;
        case XP_UpCom     :
          strncpy(upcom_seg,msg->arg,STRSIZE-1);
          break;

        case XP_Remove    : /* don't call before transfer is finished */
          while (node = RemHead(&filelist))
          {
            FreeVec(node->ln_Name);
            FreeVec(node);
          }
          running     = FALSE;
          msg->result = TRUE;
          break;
        case XP_Upload    :
          if (type == 0) {
            if (open_xpr()) {
              xpr_dir  = TRUE;
              id_first = filelist.lh_Head;
              id_next  = filelist.lh_Head;
              if (id_first->ln_Succ) {
                msg->result = TRUE;
                ReplyMsg(msg);
                msg = 0;
                send_main(XP_openwindow,(xpr_xpr2001) ? 2 : 1,0,0);
                send_main(XP_double    ,xpr_double,0,0);
                xpr_io.xpr_filename = id_first->ln_Name;
                XProtocolSend(&xpr_io);
                send_main(XP_closewindow,0,0,0);
              }
              close_xpr();
              if (msg == 0) send_main(XP_finished,0,0,0);
            }
          } else {
            msg->result = TRUE;
            ReplyMsg(msg);
            msg = 0;
            send_main(XP_seroff,0,0,0);
            Execute(upcom_seg,0,0);
            send_main(XP_seron,0,0,0);
            send_main(XP_finished,0,0,0);
          }
          break;
        case XP_Download  :
          if (type == 0) {
            if (open_xpr()) {
              xpr_dir = FALSE;
              if (xpr_norecreq) { /* cannot use req in download anymore! */
                msg->result = TRUE;
                ReplyMsg(msg);
                msg = 0;
                send_main(XP_openwindow,(xpr_xpr2001) ? 2 : 0,0,0);
                send_main(XP_double    ,xpr_double,0,0);
                XProtocolReceive(&xpr_io);
                send_main(XP_closewindow,0,0,0);
              }
              close_xpr();
              if (msg == 0) send_main(XP_finished,0,0,0);
            }
          } else {
            msg->result = TRUE;
            ReplyMsg(msg);
            msg = 0;
            send_main(XP_seroff,0,0,0);
            Execute(dncom_seg,0,0);
            send_main(XP_seron,0,0,0);
            send_main(XP_finished,0,0,0);
          }
          break;
        case XP_Abort     :
          if (type == 0) {
            xpr_abort   = msg->arg;
            msg->result = TRUE;
          } else {
          }
          break;
        case XP_Addfile   :
          if (!FindName(&filelist,msg->arg)) {
            if (node = AllocVec(sizeof(struct Node),MEMF_ANY)) {
              if (name = AllocVec(strlen(msg->arg)+1,MEMF_ANY)) {
                strcpy(name,msg->arg);
                node->ln_Name = name;
                AddTail(&filelist,node);
                msg->result = TRUE;
              } else {
                FreeVec(node);
              }
            }
          }
          break;
        case XP_Clearfiles:
          while (node = RemHead(&filelist))
          {
            FreeVec(node->ln_Name);
            FreeVec(node);
          }
          msg->result = TRUE;
          break;
      }
      if (running AND msg) ReplyMsg(msg);
    }
    if (running AND do_wait) WaitPort(sub->ch_port);
    if (!do_wait) running = FALSE;
  }
  return(msg);
}

__saveds __asm void driver_protocol(void)
{
struct XP_Msg  *msg;
struct Process *proc = FindTask(0);

  while( !(msg = GetMsg(&proc->pr_MsgPort)) )
  {
    WaitPort(&proc->pr_MsgPort);
  }
  sub           = msg->arg;
  sub->ch_port  = CreateMsgPort();
  sub->mp_reply = CreateMsgPort();
  NewList(&filelist);
  XProtocolBase        = 0;
  xpr_io.xpr_fopen     = (long (* )())&xpr_fopen;
  xpr_io.xpr_fclose    = (long (* )())&xpr_fclose;
  xpr_io.xpr_fread     = (long (* )())&xpr_fread;
  xpr_io.xpr_fwrite    = (long (* )())&xpr_fwrite;
  xpr_io.xpr_sread     = (long (* )())&xpr_sread;
  xpr_io.xpr_swrite    = (long (* )())&xpr_swrite;
  xpr_io.xpr_sflush    = (long (* )())&xpr_sflush;
  xpr_io.xpr_update    = (long (* )())&xpr_update;
  xpr_io.xpr_chkabort  = (long (* )())&xpr_chkabort;
  xpr_io.xpr_chkmisc   = (long (* )())&xpr_chkmisc;
  xpr_io.xpr_gets      = 0;
  xpr_io.xpr_setserial = (long (* )())&xpr_setserial;
  xpr_io.xpr_ffirst    = (long (* )())&xpr_ffirst;
  xpr_io.xpr_fnext     = (long (* )())&xpr_fnext;
  xpr_io.xpr_finfo     = (long (* )())&xpr_finfo;
  xpr_io.xpr_fseek     = (long (* )())&xpr_fseek;
  xpr_io.xpr_extension = 4;
  xpr_io.xpr_data      = 0;
  xpr_io.xpr_options   = 0;
  xpr_io.xpr_unlink    = (long (* )())&xpr_unlink;
  xpr_io.xpr_squery    = (long (* )())&xpr_squery;
  xpr_io.xpr_getptr    = (long (* )())&xpr_getptr;

  if (sub->ch_port == 0 OR sub->mp_reply == 0) {
    Forbid();
    DeleteMsgPort(sub->ch_port);
    DeleteMsgPort(sub->mp_reply);
    msg->result = FALSE;
    ReplyMsg(msg);
    return;
  }

  msg->result = TRUE;
  ReplyMsg(msg);

  msg = main_loop(TRUE);

  Forbid();
  DeleteMsgPort(sub->ch_port);
  DeleteMsgPort(sub->mp_reply);
  ReplyMsg(msg);
}
