#include "global.h"
#include "loc_strings.h"

/* Printtaa ABC.. teksti hardware-screeniä varten */
bool __asm TempPrint(REG __a0 struct RastPort *rp)
{
struct TextFont *font;
struct TextAttr attr;
ubyte t[256];
ulong i;

  attr.ta_Name  = GetInfoTag(CS_FontName,gl_curcs);
  attr.ta_YSize = GetInfoTag(CS_Ysize   ,gl_curcs);
  attr.ta_Style = 0;
  attr.ta_Flags = 0;
  if ( attr.ta_YSize != 8 OR GetInfoTag(CS_Xsize,gl_curcs) != 8 ) {
    EndProg("TempPrint()");
  }
  if ( (font = OpenDiskFont(&attr)) == 0 ) {
    Warning(LOC(MSG_cannot_open_font_for_hardware_screen));
  }
  SetFont(rp,font);
  for (i = 0; i < 256; i++) t[i] = i;
  Move(rp,0,font->tf_Baseline);
  Text(rp,&t[000],128);
  Text(rp,&t[128],128);
  CloseFont(font);
  return(TRUE);
}
