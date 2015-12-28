#include "global.h"

/*
char softscrver[]="1.43 (Sep 15 1994)";
*/

/** KLUDGE **/
extern LONG	pen;
extern LONG	pencol[12*3];
/** KLUDGE **/

extern struct ScreenInfo	ScrInfo;

static BOOL		fakealloc=FALSE,statalloc=FALSE,nocursor;
static WORD		fx,fy,wxfix,wyfix;
static WORD		x,y,fcol,bcol,fs;
static WORD		col [28]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
static WORD		fake[28]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
static WORD		statcol=-1;
static WORD		curx,cury;
static WORD		nl[12];
static ULONG	fsenable;
static struct RastPort		*rp;
static struct TextFont		*font=0;
static struct TextAttr		attr;

/* System screen */

void __asm SPrint(register __a0 STRPTR p, register __d0 WORD count)
{
	if (ScrInfo.xpos != x OR ScrInfo.ypos != y) { 
		x = ScrInfo.xpos; y = ScrInfo.ypos; Move(rp,wxfix+x*fx,wyfix+font->tf_Baseline+y*fy);
	}
	if (ScrInfo.inverse == 0) {
		if (ScrInfo.forecol != fcol) {
			fcol = ScrInfo.forecol; SetAPen(rp,col[fcol]);
		}
		if (ScrInfo.backcol != bcol) {
			bcol = ScrInfo.backcol; SetBPen(rp,col[bcol]);
		}
	} else {	/* Inverse video */
		if (ScrInfo.backcol != fcol) {
			fcol = ScrInfo.backcol; SetAPen(rp,col[fcol]);
		}
		if (ScrInfo.forecol != bcol) {
			bcol = ScrInfo.forecol; SetBPen(rp,col[bcol]);
		}
	}
	if (ScrInfo.fontstate != fs) {
		fs = ScrInfo.fontstate; SetSoftStyle(rp,fs,fsenable);
	}
	Text(rp,p,count);
	x += count;
}

void __asm SSPrint(register __a0 STRPTR p)
{
	if (gl_linwindow) {
		Move(gl_linwindow->RPort,0,font->tf_Baseline+1);
		Text(gl_linwindow->RPort,p,strlen(p));
	} else {
		if ( CUR(PB_Wb) AND gl_conscreen ) {
			SetWindowTitles(gl_conwindow,p,-1);
		}
	}
}

void __asm SDown(void)
{
	if (CUR(PB_RtsBlit)) SetRTS(FALSE);
	SetBPen(rp,col[0]);
	ScrollRaster(rp,0,fy,wxfix+0,wyfix+0,wxfix+ScrInfo.columns*fx-1,wyfix+ScrInfo.lines*fy-1);
	SetBPen(rp,col[bcol]);
	if (CUR(PB_RtsBlit)) SetRTS(TRUE);
}

void __asm SScrollDown(REG __d0 LONG count)
{
	if (CUR(PB_RtsBlit)) SetRTS(FALSE);
	SetBPen(rp,col[0]);
	ScrollRaster(rp,0,fy*count,wxfix+0,wyfix+0,wxfix+ScrInfo.columns*fx-1,wyfix+ScrInfo.lines*fy-1);
	SetBPen(rp,col[bcol]);
	if (CUR(PB_RtsBlit)) SetRTS(TRUE);
}

void __asm SUp(void)
{
	if (CUR(PB_RtsBlit)) SetRTS(FALSE);
	SetBPen(rp,col[0]);
	ScrollRaster(rp,0,-fy,wxfix+0,wyfix+0,wxfix+ScrInfo.columns*fx-1,wyfix+ScrInfo.lines*fy-1);
	SetBPen(rp,col[bcol]);
	if (CUR(PB_RtsBlit)) SetRTS(TRUE);
}

void __asm SClearInLine(register __d1 WORD xs, register __d0 WORD count)
{
	SetAPen(rp,col[0]);
	RectFill(rp,wxfix+xs*fx,wyfix+ScrInfo.ypos*fy,wxfix+(xs+count)*fx-1,wyfix+(ScrInfo.ypos+1)*fy-1);
	SetAPen(rp,col[fcol]);
}

void __asm SCopyInLine(register __d1 WORD xs, register __d2 WORD xt, register __d0 WORD count)
{
	ClipBlit(rp,wxfix+xs*fx,wyfix+ScrInfo.ypos*fy,rp,wxfix+xt*fx,wyfix+ScrInfo.ypos*fy, count*fx,fy,0xc0);
}

void __asm SAreaClear(register __d0 WORD ys, register __d1 WORD count)
{
	SetAPen(rp,col[0]);
	RectFill(rp,wxfix+0,wyfix+ys*fy,wxfix+ScrInfo.columns*fx-1,wyfix+(ys+count)*fy-1);
	SetAPen(rp,col[fcol]);
}

void __asm SAreaCopy(register __d0 WORD ys, register __d2 WORD yt, register __d1 WORD count)
{
	if (CUR(PB_RtsBlit)) SetRTS(FALSE);
	ClipBlit(rp,wxfix+0,wyfix+ys*fy,rp,0,wxfix+yt*fy, wyfix+ScrInfo.columns*fx,count*fy,0xc0);
	if (CUR(PB_RtsBlit)) SetRTS(TRUE);
}

void __asm SCursorOn(void)
{
	if (nocursor == FALSE) return;
	nocursor = FALSE;
	curx = ScrInfo.xpos;
	cury = ScrInfo.ypos;
	ClipBlit(rp,wxfix+curx*fx,wyfix+cury*fy,rp,wxfix+curx*fx,wyfix+cury*fy, fx,fy, 0x50);
}

void __asm SCursorOff(void)
{
	if (nocursor) return;
	nocursor = TRUE;
	ClipBlit(rp,wxfix+curx*fx,wyfix+cury*fy,rp,wxfix+curx*fx,wyfix+cury*fy, fx,fy, 0x50);
}

void __asm S_remcol(void)
{
LONG	i;

	if (fakealloc)
	{
		for (i=0; i < 28; i++)
		{
			if (fake[i] != -1) ReleasePen(gl_conwindow->WScreen->ViewPort.ColorMap,col[i]);
			fake[i] = -1;
		}
	}
	if (statalloc) {
		ReleasePen(gl_conwindow->WScreen->ViewPort.ColorMap,statcol);
		statcol = -1;
	}
	fakealloc = statalloc = FALSE;
}

void __asm S_addcol(REG __a0 ULONG *val, REG __d0 WORD count)
{
LONG	i,j;
LONG	realcol,tempcol,lim,maxpen;

	S_remcol();

	if ( CUR(PB_Wb) OR CUR(PB_KeepWb) ) { /* WB-scr tai KeepWb */
		col[0] = ObtainBestPen(gl_conwindow->WScreen->ViewPort.ColorMap,
													 *val++,*val++,*val++,
													 OBP_Precision,	PRECISION_EXACT,TAG_END);
		maxpen = fake[0] = col[0];
		for (i=1; i < count; i++)
		{
			col[i] = fake[i] = MyObtainBestPen(gl_conwindow->WScreen->ViewPort.ColorMap,
																				 *val++,*val++,*val++, col[0]);
			if (col[i] > maxpen) maxpen = col[i];
		}
		statcol = MyObtainBestPen(gl_conwindow->WScreen->ViewPort.ColorMap,
													  	0xffffffff,0xffffffff,0xffffffff,col[0]);
		if (statcol > maxpen) maxpen = statcol;
		SetMaxPen(rp,maxpen);
		fakealloc = statalloc = TRUE;
	} else {	/* Custom screen */
		realcol = Pot2(CUR(PB_Depth));
		if ( (tempcol = count) > realcol ) tempcol = realcol;
		maxpen = tempcol-1;
		if ( (tempcol = count+pen) > realcol ) tempcol = realcol;
		for (i=0; i < tempcol; i++)
		{
			fake[i]	= ObtainPen(gl_conwindow->WScreen->ViewPort.ColorMap,i,
															0,0,0,0);
		}
		if ( (tempcol = count) > realcol ) tempcol = realcol;
		for (i=0; i < tempcol; i++)
		{
			SetRGB32(&gl_conwindow->WScreen->ViewPort,i,
									 val[i*3+0],val[i*3+1],val[i*3+2]);
		}

/** KLUDGE **/
		if ( (tempcol = count+pen) > realcol ) tempcol = realcol;
		j = 0;
		for (  ; i < tempcol; i++)
		{
			if (pen <= j) break;
			SetRGB32(&gl_conwindow->WScreen->ViewPort,i,
									 pencol[j*3+0],pencol[j*3+1],pencol[j*3+2]);
			j++;
		}
/** KLUDGE **/

		col[0] = 0; lim = i-1;
		for (i=1; i < count; i++)
		{
			col[i] = MyFindColor(gl_conwindow->WScreen->ViewPort.ColorMap,
													 val[i*3+0],val[i*3+1],val[i*3+2],maxpen,col[0]);
		}
		statcol = MyFindColor(gl_conwindow->WScreen->ViewPort.ColorMap,
													0xffffffff,0xffffffff,0xffffffff,maxpen,col[0]);
		SetMaxPen(rp,maxpen);
		fakealloc	= TRUE; statalloc = FALSE;
	}

	SetAPen(rp,col[0]);
	RectFill(rp,wxfix+0,wyfix+0,wxfix+ScrInfo.columns*fx-1,wyfix+ScrInfo.lines*fy-1);
	if (gl_linwindow) {
		SetRast(gl_linwindow->RPort,col[0]);
		SetAPen(gl_linwindow->RPort,statcol);
		SetBPen(gl_linwindow->RPort,col[0]);
	}
	nocursor = TRUE;
	fcol = bcol = ~0;
#ifdef DEBUG
	printf("SetMaxPen: %ld\n",maxpen);
#endif
}

/* Init & change */

BOOL __asm SInit(void)
{
	nocursor = TRUE;
	return(TRUE);
}

BOOL __asm SRemove(void)
{
	if (font) { CloseFont(font); font = 0; }
	return(TRUE);
}

BOOL __asm SChange(REG __a0 struct TagBase *tagbase)
{
	SRemove();
	rp = gl_conwindow->RPort;
	ScrInfo.columns    = CUR(PB_Columns);
	ScrInfo.columnsdbf = ScrInfo.columns-1;
	ScrInfo.lines      = CUR(PB_Lines);
	ScrInfo.linesdbf   = ScrInfo.lines-1;
	attr.ta_Name  = GetInfoTag(CS_FontName,gl_curcs);
	attr.ta_YSize = GetInfoTag(CS_Ysize		,gl_curcs);
	attr.ta_Style = 0;
	attr.ta_Flags = 0;
	if ( (font = OpenDiskFont(&attr)) == 0 ) {
		Warning("Cannot open font for screen!"); return(FALSE);
	}
	SetFont(rp,font);
	if (gl_linwindow) SetFont(gl_linwindow->RPort,font);
	fsenable = AskSoftStyle(rp);
	fx = GetInfoTag(CS_Xsize,gl_curcs);
	fy = GetInfoTag(CS_Ysize,gl_curcs);
	x = y = fcol = bcol = fs = ~0;
	wxfix	= (CUR(PB_Wb)) ? gl_conscreen->WBorLeft : 0;
	wyfix	= (CUR(PB_Wb)) ? gl_conscreen->WBorTop + gl_conscreen->Font->ta_YSize + 1 : 0;
	return(TRUE);
}

BOOL __asm SOn(void)
{
	return(TRUE);
}

BOOL __asm SOff(void)
{
	return(TRUE);
}
