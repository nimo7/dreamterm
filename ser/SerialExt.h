#ifndef DTERM_SERIAL_H
#define DTERM_SERIAL_H

#include <exec/types.h>

#define SER_HARD	TRUE
#define SER_SOFT	FALSE

void				__asm SerialToUse	(register __d0 ULONG);
ULONG				__asm InitSerial	(void);
void				__asm RemoveSerial(void);
void				__asm ResetInfo		(void);
BOOL				__asm InsertTXD		(register __a0 UBYTE *, register __d0 LONG);
BOOL				__asm CheckTXD		(void);
BOOL				__asm WaitTXD			(void);
void				__asm BreakTXD		(void);
struct RXD *__asm GetRXD			(void);
BOOL				__asm SetPort			(void);
struct RXD *__asm GetTimedRXD	(register __d0 LONG, register __d1 LONG);
BOOL				__asm CheckCTS		(void);
BOOL				__asm CheckRTS		(void);
BOOL				__asm CheckCS			(void);
void				__asm SetRTS			(register __d0 BOOL);
void				__asm DropDTR			(void);
LONG				__asm SerQuery		(void);

struct SerialInfo
{
	WORD txdcps;          /* second slices    */
  WORD txdcps_peak;     /* vblank slices    */
  WORD txdcps_avr_peak; /* avr of current   */
  WORD txdcps_avr;      /* average cps      */
	LONG txdbytes;        /* bytes            */
	LONG txdsec;          /* seconds used     */
	WORD rxdcps;
	WORD rxdcps_peak;
	WORD rxdcps_avr_peak;
	WORD rxdcps_avr;
	LONG rxdbytes;
	LONG rxdsec;
	WORD overrun;
	LONG charbuf;	        /* chars in buffer  */
};

struct RXD
{
	UBYTE*	buffer;		/* Missä bufferi */
	LONG		size;			/* Monta merkkiä */
	UBYTE*	buffer2;	/* Vain TimedRXD (mahdollinen toinen osa) */
	LONG		size2;
};

#endif
