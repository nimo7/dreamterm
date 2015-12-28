	INCLUDE "exec/types.i"

; --> Mikä serialunit (SerialToUse)

SER_HARD	EQU 1
SER_SOFT	EQU 0

   STRUCTURE SERIALINFO,0
	WORD	txdcps
	WORD	txdcps_peak
	WORD	txdcps_avr_peak
	WORD	txdcps_avr
	LONG	txdbytes
	LONG	txdsec
	WORD	rxdcps
	WORD	rxdcps_peak
	WORD	rxdcps_avr_peak
	WORD	rxdcps_avr
	LONG	rxdbytes
	LONG	rxdsec
	WORD	overrun
	LONG	charbuf
	LABEL	SERIALINFO_SIZE

    STRUCTURE RXD,0
	APTR	buffer
	LONG	size
	APTR	buffer2
	LONG	size2
	LABEL	RXD_SIZE
