#include "global.h"

/*
char softserver[]="1.42 (Feb 16 1996)";
*/

extern struct SerialInfo SerialInfo;
extern struct RXD        RXD;

static BOOL		 unitok  = FALSE;
static BOOL		 timok   = FALSE;
static long    bufsize = 0;
static ubyte  *bufmem  = 0;
static ubyte   bufone;
static struct MsgPort			*timport=0;
static struct MsgPort			*rport	=0;
static struct MsgPort			*wport	=0;
static struct MsgPort			*iport	=0;
static struct IOExtSer		*readio	=0;
static struct IOExtSer		*writeio=0;
static struct IOExtSer		*infoio	=0;
static struct timerequest	*timio	=0;

void __asm RemoveSSerial(void);
void __asm ResetSInfo(void);
void __asm BreakSTXD(void);
struct RXD *__asm GetSRXD(void);

static bool Wait_io(struct IOExtSer *io)
{
  if ( CheckIO(io) == FALSE ) {
    WaitIO((struct IORequest *)io);
  }
  SetSignal(0,Bset(io->mn_ReplyPort->mp_SigBit));
}

bool __asm InitSSerial(void)
{
	if ( timport = CreateMsgPort() ) {
		if ( timio = CreateIORequest(timport,sizeof(struct timerequest)) ) {
			if ( OpenDevice(TIMERNAME,UNIT_VBLANK,timio,0) == 0 ) {
				timok = TRUE;
				if ( (bufmem	= AllocVec(CUR(PB_SerBufSize),MEMF_PUBLIC)) AND
						 (bufsize	= CUR(PB_SerBufSize)) AND
						 (rport		= CreateMsgPort()) AND
						 (wport		= CreateMsgPort()) AND
						 (iport   = CreateMsgPort()) ) {
					if ( (readio  = CreateIORequest(rport,sizeof(struct IOExtSer))) AND
							 (writeio = CreateIORequest(wport,sizeof(struct IOExtSer))) AND
							 (infoio  = CreateIORequest(iport,sizeof(struct IOExtSer))) ) {
						readio->io_SerFlags = SERF_7WIRE;
            if ( OpenDevice(CUR(PB_SerName),CUR(PB_SerUnit),readio,0) == 0 ) {
              unitok = TRUE;
              CopyMem(readio,writeio,sizeof(struct IOExtSer));
              CopyMem(readio,infoio ,sizeof(struct IOExtSer));
							readio ->io_Message.mn_ReplyPort = rport;
							writeio->io_Message.mn_ReplyPort = wport;
							infoio ->io_Message.mn_ReplyPort = iport;
							readio ->io_Command = CMD_CLEAR;
							writeio->io_Command = CMD_CLEAR;
							infoio ->io_Command = CMD_CLEAR;
							DoIO((struct IORequest *)readio);
							DoIO((struct IORequest *)writeio);
							DoIO((struct IORequest *)infoio);
							gl_rxdsig = Bset(rport->mp_SigBit);
							gl_txdsig = Bset(wport->mp_SigBit);

              /* remove cps value */

              SerialInfo.txdcps   = 0;
              SerialInfo.rxdcps   = 0;

              /* clear signals */
              SetSignal(0,Bset( readio->mn_ReplyPort->mp_SigBit));
              SetSignal(0,Bset(writeio->mn_ReplyPort->mp_SigBit));
              SetSignal(0,Bset( infoio->mn_ReplyPort->mp_SigBit));

              /* for rxd signal */
              readio->io_Data     = &bufone;
              readio->io_Length   = 1;
              readio->io_Command  = CMD_READ;
              SendIO((struct IORequest *)readio);

              return(gl_serok = TRUE);
            }
					}
				}
			}
		}
	}
  RemoveSerial();
  return(FALSE);
}

void __asm RemoveSSerial(void)
{
  if (timok  ) {
    AbortIO((struct IORequest *)timio);
    Wait_io(timio);
    CloseDevice((struct IORequest *)timio);
  }
  if (timio  ) DeleteIORequest((struct IORequest *)timio);
  if (timport) DeleteMsgPort(timport);

  if (unitok) {
    AbortIO((struct IORequest *)readio);
    AbortIO((struct IORequest *)writeio);
    Wait_io(readio);
    Wait_io(writeio);
    CloseDevice((struct IORequest *)readio);
	}
	if (readio ) DeleteIORequest((struct IORequest *)readio);
	if (writeio) DeleteIORequest((struct IORequest *)writeio);
	if (infoio ) DeleteIORequest((struct IORequest *)infoio);
	if (rport	 ) DeleteMsgPort(rport);
	if (wport	 ) DeleteMsgPort(wport);
	if (iport	 ) DeleteMsgPort(iport);
	if (bufmem ) FreeVec(bufmem);
	timok	= unitok = FALSE;
	timio = readio = writeio = infoio = 0;
	timport = rport = wport = iport = 0;
	bufmem = 0;
	gl_rxdsig = 0;
	gl_txdsig = 0;
	gl_serok = FALSE;
}

void __asm ResetSInfo(void)
{
  SerialInfo.txdcps   = 0;
  SerialInfo.txdbytes = 0;
  SerialInfo.txdsec   = 0;
  SerialInfo.rxdcps   = 0;
  SerialInfo.rxdbytes = 0;
  SerialInfo.rxdsec   = 0;
  SerialInfo.overrun  = 0;
  SerialInfo.charbuf  = 0;
}

bool __asm InsertSTXD(REG __a0 ubyte *buf, REG __d0 long size)
{
  if (unitok AND size) {
    Wait_io(writeio);
    writeio->io_Data    = buf;
    writeio->io_Length  = size;
    writeio->io_Command = CMD_WRITE;
    SendIO((struct IORequest *)writeio);
    if (writeio->io_Error) return(FALSE);
    SerialInfo.txdbytes += size;
  }
  return(TRUE);
}

bool __asm CheckTXD_s(void)
{
  if (unitok) {
    return( CheckIO((struct IORequest *)writeio) );
  }
}

bool __asm WaitSTXD(void)
{
  if (unitok) {
    Wait_io(writeio);
    if (writeio->io_Error) return(FALSE);
  }
  return(TRUE);
}

void __asm BreakSTXD(void)
{
  if (unitok) {
    AbortIO((struct IORequest *)writeio);
    Wait_io(writeio);
  }
}

struct RXD *__asm GetSRXD(void)
{
long abytes;

  if (unitok) {
    if ( CheckIO((struct IORequest *)readio) ) {
      Wait_io(readio);
      bufmem[0] = bufone;

			readio->io_Command = SDCMD_QUERY;
			DoIO((struct IORequest *)readio);
			abytes = readio->io_Actual;
			if (abytes != 0) {
				if (abytes >= bufsize) abytes = bufsize-1;
				readio->io_Data			= bufmem+1;
				readio->io_Length		= abytes;
				readio->io_Command	= CMD_READ;
				DoIO((struct IORequest *)readio);
				RXD.buffer	= bufmem;
				RXD.size		= readio->io_Actual+1;
				RXD.size2		= 0;
			} else {
				RXD.buffer	= bufmem;
				RXD.size		= 1;
				RXD.size2		= 0;
			}
	
			readio->io_Data			= &bufone;	/* for signal */
			readio->io_Length		= 1;
			readio->io_Command	= CMD_READ;
			SendIO((struct IORequest *)readio);
		} else {
      RXD.size	= 0;
      RXD.size2	= 0;
    }
  } else {
    RXD.size  = 0;
    RXD.size2 = 0;
  }
  SerialInfo.rxdbytes	+= RXD.size;
  return(&RXD);
}

bool __asm SetSPort(void)
{
ulong tmp = CUR(PB_SerShared) ? SERF_SHARED : 0;

  if (unitok) {
    AbortIO((struct IORequest *)readio);
    AbortIO((struct IORequest *)writeio);
    Wait_io(readio);
    Wait_io(writeio);
		readio->io_RBufLen	= CUR(PB_SerBufSize);
		readio->io_Baud			= CUR(PB_DteRate	 );
		readio->io_ReadLen	= CUR(PB_DataBits  );
		readio->io_WriteLen	= CUR(PB_DataBits  );
		readio->io_StopBits	= CUR(PB_StopBits  );
		readio->io_SerFlags	= SERF_XDISABLED|SERF_RAD_BOOGIE|SERF_7WIRE|tmp;
		readio->io_Command	= SDCMD_SETPARAMS;
		DoIO((struct IORequest *)readio);
		writeio->io_RBufLen	= CUR(PB_SerBufSize);
		writeio->io_Baud		= CUR(PB_DteRate	 );
		writeio->io_ReadLen	= CUR(PB_DataBits  );
		writeio->io_WriteLen= CUR(PB_DataBits  );
		writeio->io_StopBits= CUR(PB_StopBits  );
		writeio->io_SerFlags= SERF_XDISABLED|SERF_RAD_BOOGIE|SERF_7WIRE|tmp;
		writeio->io_Command	= SDCMD_SETPARAMS;
		DoIO((struct IORequest *)writeio);
		infoio->io_RBufLen	= CUR(PB_SerBufSize);
		infoio->io_Baud			= CUR(PB_DteRate	 );
		infoio->io_ReadLen	= CUR(PB_DataBits  );
		infoio->io_WriteLen	= CUR(PB_DataBits  );
		infoio->io_StopBits	= CUR(PB_StopBits  );
		infoio->io_SerFlags	= SERF_XDISABLED|SERF_RAD_BOOGIE|SERF_7WIRE|tmp;
		infoio->io_Command	= SDCMD_SETPARAMS;
		DoIO((struct IORequest *)infoio);

		readio->io_Data			= &bufone;	/* for signal */
		readio->io_Length		= 1;
		readio->io_Command	= CMD_READ;
		SendIO((struct IORequest *)readio);
	}
	return(TRUE);
}

struct RXD *__asm GetSTimedRXD(REG __d0 long size, REG __d1 long timeout)
{
long  abytes=0;
ulong newsig;

  if (unitok) {
    if (size >= bufsize) size = bufsize-1;

    timio->io_Command       = TR_ADDREQUEST;
    timio->tr_time.tv_secs  = timeout/1000000;
    timio->tr_time.tv_micro = timeout - (timio->tr_time.tv_secs * 1000000);
    SendIO((struct IORequest *)timio);

    if ( CheckIO((struct IORequest *)readio) == 0 ) {
      newsig = Wait(Bset(timport->mp_SigBit)|Bset(rport->mp_SigBit));
    } else {
      newsig = Bset(rport->mp_SigBit);
    }
    if ( (newsig & Bset(rport->mp_SigBit)) ) {
      Wait_io(readio);
      bufmem[0] = bufone; abytes = 1;
			if (size > 1) {
				if ( (newsig & Bset(timport->mp_SigBit)) == 0 ) {
					readio->io_Data			= bufmem+1;
					readio->io_Length		= size-1;
					readio->io_Command	= CMD_READ;
					SendIO((struct IORequest *)readio);
          Wait(Bset(timport->mp_SigBit)|Bset(rport->mp_SigBit));
					AbortIO((struct IORequest *)readio);
					Wait_io(readio);
					abytes += readio->io_Actual;
				} else {
					readio->io_Command = SDCMD_QUERY;
					DoIO((struct IORequest *)readio);
					if ( (readio->io_Length = readio->io_Actual) > size-1 ) readio->io_Length = size-1;
					if ( readio->io_Length != 0) {
						readio->io_Data    = bufmem+1;
						readio->io_Command = CMD_READ;
						DoIO((struct IORequest *)readio);
						abytes += readio->io_Actual;
					}
				}
			}
      readio->io_Data    = &bufone; /* for signal */
      readio->io_Length  = 1;
      readio->io_Command = CMD_READ;
      SendIO((struct IORequest *)readio);
    }
    AbortIO(timio);
    Wait_io(timio);
  }
  RXD.buffer  = bufmem;
  RXD.size    = abytes;
  RXD.size2   = 0;
  SerialInfo.rxdbytes += RXD.size;
  return(&RXD);
}

BOOL __asm CheckSCTS(void)
{
	if (unitok) {
		infoio->io_Command = SDCMD_QUERY;
		DoIO((struct IORequest *)infoio);
		if ( (infoio->io_Status & 0x10) ) return(FALSE);
		return(TRUE);
	}
	return(FALSE);
}

BOOL __asm CheckSRTS(void)
{
	if (unitok) {
		infoio->io_Command = SDCMD_QUERY;
		DoIO((struct IORequest *)infoio);
		if ( (infoio->io_Status & 0x40) ) return(FALSE);
		return(TRUE);
	}
	return(FALSE);
}

BOOL __asm CheckSCS(void)
{
	if (unitok) {
		infoio->io_Command = SDCMD_QUERY;
		DoIO((struct IORequest *)infoio);
		if ( (infoio->io_Status & 0x20) ) return(FALSE);
		return(TRUE);
	}
	return(FALSE);
}

void __asm SetSRTS(REG __d0 BOOL stat)
{
}

void __asm DropSDTR(void)
{
  RemoveSSerial();
  TempTimeDelay(UNIT_VBLANK,0,20000);
  if ( !InitSSerial() ) {
    Warning("Cannot reinitalize serial, no IO!");
    return;
  }
  SetSPort();
}

long __asm SQuery(void)
{
	if (unitok) {
		infoio->io_Command = SDCMD_QUERY;
		DoIO((struct IORequest *)infoio);
    return(infoio->io_Actual);
  }
  return(0);
}
