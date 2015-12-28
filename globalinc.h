#ifndef GLOBALINC_H
#define GLOBALINC_H

/*--------------------------------------------------------------------------*/

/* ansi */
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/* mui  */
#include <libraries/mui.h>
#include <proto/muimaster.h>
#include "muimacro.h"

/* system */
#include <exec/lists.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <exec/nodes.h>
#include <exec/types.h>
#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <datatypes/pictureclass.h>
#include <devices/serial.h>
#include <devices/timer.h>
#include <dos/datetime.h>
#include <dos/dos.h>
#include <dos/stdio.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>
#include <graphics/displayinfo.h>
#include <graphics/videocontrol.h>
#include <intuition/intuition.h>
#include <libraries/asl.h>
#include <libraries/locale.h>
#include <libraries/xem.h>
#include <libraries/xpk.h>
#include <utility/tagitem.h>

/* protos */
#include <clib/alib_protos.h>
#include <clib/alib_stdio_protos.h>
#include <libraries/xproto.h>
#include <proto/asl.h>
#include <proto/datatypes.h>
#include <proto/diskfont.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/gadtools.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/keymap.h>
#include <proto/locale.h>
#include <proto/utility.h>
#include <proto/xem_proto.h>

/* mine */
#include <mine/priv.h>
#include <mine/toolkit.h>

/* this project */
#include "globalext.h"
#include "globalvar.h"
#include "emulation/emuext.h"
#include "muiclass/class.h"	
#include "screen/screenext.h"	
#include "serial/serialext.h"

#endif
