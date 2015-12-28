#include "global.h"
#include "loc_strings.h"

extern struct SerialInfo SerialInfo;

/***----------------------------------------------------------------------***/
/*** memory handling                                                      ***/
/***----------------------------------------------------------------------***/

/* allokoi muistia ja looppaa kunnes se saadaan */
aptr __asm alloc_force(register __d0 ulong memsize)
{
aptr mem;
bool clear=FALSE;

  while ( (mem = alloc_pool(memsize)) == 0 )
  {
    if (clear) {
      Warning(LOC(MSG_alloc_force_error));
      Delay(50*5);
    } else {
      DoMethod(gl_review,MUIM_REV_Clear);
      clear = TRUE;
    }
  }
#ifdef DEBUG
  gl_call_force++;
#endif
  return(mem);
}

/* allokoi muistia, 0 = jos ei onnistu */
aptr __asm alloc_pool(register __d0 ulong memsize)
{
aptr p;

  if (p = AllocVecPooled(gl_pool,memsize)) {
#ifdef DEBUG
    gl_call_alloc++;
    gl_mem_now += memsize;
    gl_mem_add += memsize;
#endif
  }
  return(p);
}

/* vapauta varattu muisti alue, 0 = no operation */
void __asm free_pool(register __a0 aptr mem)
{
  if (mem) {
#ifdef DEBUG
ulong *tmp = (ulong *) mem;

    gl_call_free++;
    gl_mem_del += (tmp[-1])-4;
    gl_mem_now -= (tmp[-1])-4;
#endif
    FreeVecPooled(gl_pool,mem);
  }
}

/***----------------------------------------------------------------------***/
/*** file operations (all use asyncio.library modes and return codes!)    ***/
/***----------------------------------------------------------------------***/

/*
 * Close file
 *
 * fh    : handle
 *
 * return: result (>= 0 for success)
 *
 */
long dt_close(bptr fh)
{
  if (gl_async_dos) {
    return(CloseAsync(fh));
  } else {
    return(!Close(fh));
  }
}

/*
 * Open file
 *
 * name  : filename
 * mode  : mode
 * size  : buffersize (only used if asyncio.library is used)
 *
 * return: handle
 *
 */
bptr dt_open(strptr name, long mode, long size)
{
  if (gl_async_dos) {
    return(OpenAsync(name,mode,size));
  } else {
  bptr fh;

    switch (mode)
    {
      case MODE_READ  : mode = MODE_OLDFILE  ; break;
      case MODE_WRITE : mode = MODE_NEWFILE  ; break;
      case MODE_APPEND: mode = MODE_READWRITE; break;
    }
    if (fh = Open(name,mode)) {
      if (mode == MODE_READWRITE) dt_seek(fh,0,MODE_END);
    }
    return(fh);
  }
}

/*
 * Read file
 *
 * fh    : handle
 * buffer: where to read
 * size  : bytes to read
 *
 * return: result (-1 = error, 0 = EOF, other = bytes read)
 *
 */
long dt_read(bptr fh, ubyte *buffer, long size)
{
  if (gl_async_dos) {
    return(ReadAsync(fh,buffer,size));
  } else {
    return(Read(fh,buffer,size));
  }
}

/*
 * Read one char from file
 *
 * fh    : handle
 *
 * return: byte (-1 for error or EOF, note that result is LONG!)
 *
 */
long dt_readchar(bptr fh)
{
  if (gl_async_dos) {
    return(ReadCharAsync(fh));
  } else {
  ubyte c;
  long  result = Read(fh,&c,1);

    if ( result == -1 OR result == 0 ) {
      return(-1);
    } else {
      return(c);
    }
  }
}

/*
 * Set file position
 *
 * fh    : handle
 * pos   : position
 * mode  : mode
 *
 * return: old position (-1 for error)
 *
 */
bptr dt_seek(bptr fh, long pos, long mode)
{
  if (gl_async_dos) {
    return(SeekAsync(fh,pos,mode));
  } else {
    switch (mode)
    {
      case MODE_START  : mode = OFFSET_BEGINNING; break;
      case MODE_CURRENT: mode = OFFSET_CURRENT  ; break;
      case MODE_END    : mode = OFFSET_END      ; break;
    }
    return(Seek(fh,pos,mode));
  }
}

/*
 * Write file
 *
 * fh    : handle
 * buffer: where to write
 * size  : bytes to write (0 is legal and always succeeds with result of 0!)
 *
 * return: result (-1 = error, other = bytes written)
 *
 */
long dt_write(bptr fh, ubyte *buffer, long size)
{
  if (size) {
    if (gl_async_dos) {
      return(WriteAsync(fh,buffer,size));
    } else {
      return(Write(fh,buffer,size));
    }
  }
  return(0);
}

/*
 * Write one char to file
 *
 * fh    : handle
 * c     : byte
 *
 * return: result (-1 for error, 1 for success)
 *
 */
long dt_writechar(bptr fh, ubyte c)
{
  if (gl_async_dos) {
    return(WriteCharAsync(fh,c));
  } else {
    return(Write(fh,&c,1));
  }
}

/***----------------------------------------------------------------------***/
/*** datatypes handling                                                   ***/
/***----------------------------------------------------------------------***/

/*
 * Allocate picture structure
 *
 * return: structure or null for failure
 *
 */
struct Picture *AllocPicture(void)
{
struct Picture *pic;

  if (pic = alloc_pool(sizeof(struct Picture))) {
    memset(pic, 0, sizeof(struct Picture));
  }
  return(pic);
}

/*
 * Free picture
 *
 * pic   : picture structure with resources to free (or null for no-op)
 *
 * return: -
 *
 */
void FreePicture(struct Picture *pic)
{
  if (pic) {
    if (pic->bmap) FreeBitMap(pic->bmap);
    free_pool(pic->palette);
    free_pool(pic->author);
    free_pool(pic->copyright);
    free_pool(pic->annotation);
  }
}

/*
 * Check if file can be handled by some datatype
 *
 * file_name : name of the file to inspect
 * name_buff : (optional) buffer to store the file format name
 * nbuff_size: size of name_buff
 *
 * return    : TRUE if valid picture file, FALSE otherwise
 *
 */
bool IsDataTypes(ubyte *file_name, ubyte *name_buff, long nbuff_size)
{
  bool it_is;
  BPTR lock;
  struct DataType       *dtn;
  struct DataTypeHeader *dth;

   it_is = FALSE;
   if (lock = Lock(file_name, ACCESS_READ))
   {
      /* inspect file */
      if (dtn = ObtainDataTypeA(DTST_FILE, (APTR)lock, NULL))
      {
         dth = dtn->dtn_Header;
         if (dth->dth_GroupID == GID_PICTURE)   /* is it a picture? */
         {
            it_is = TRUE;
            if (name_buff)
            {
               strncpy(name_buff, dth->dth_Name, nbuff_size);
               *(name_buff + nbuff_size - 1) = 0;  /* safe strncpy() termination */
            }
         }
         ReleaseDataType(dtn);
      }
      UnLock(lock);
   }
   return(it_is);
}

/*
 * Load picture file to memory
 *
 * file_name : name of the file to load
 * pic       : picture structure
 * bmap_flags: AllocBitMap() flags (BMF_DISPLAYABLE, BMF_INTERLEAVED etc.)
 *
 * return    : 0 if successful (picture info and data in "pic" structure) or
 *             error code as from dos.library IoErr() function
 */
long LoadPicture(ubyte *file_name, struct Picture *pic, ulong bmap_flags)
{
  ubyte *str;
  long  ncol,crsize,err;
  ulong *creg,*ctab;
  Object              *obj;
  struct BitMap       *bmap;
  struct gpLayout     layout;
  struct BitMapHeader *bmh;

   err = 0;

   if (obj = NewDTObject(file_name,
                         DTA_SourceType, DTST_FILE  ,
                         DTA_GroupID   , GID_PICTURE,
                         PDTA_Remap    , FALSE      ,
                         TAG_DONE))    /* get the picture object */
   {
      if (GetDTAttrs(obj,
                     PDTA_ModeID      , &pic->display_ID,
                     PDTA_BitMapHeader, &bmh            ,
                     TAG_DONE) == 2)   /* get the bitmap_header and mode_id */
      {
         pic->bmhd = *bmh;

         /*
            query the object about its author, copyright and annotation
         */
         if (GetDTAttrs(obj, DTA_ObjAuthor, &str, TAG_DONE) == 1)
         {
            if (str)
            {
              if (pic->author = alloc_pool(strlen(str)+1)) strcpy(pic->author, str);
            }
         }
         if (GetDTAttrs(obj, DTA_ObjCopyright, &str, TAG_DONE) == 1)
         {
            if (str)
            {
              if (pic->copyright = alloc_pool(strlen(str)+1)) strcpy(pic->copyright, str);
            }
         }
         if (GetDTAttrs(obj, DTA_ObjAnnotation, &str, TAG_DONE) == 1)
         {
            if (str)
            {
              if (pic->annotation = alloc_pool(strlen(str)+1)) strcpy(pic->annotation, str);
            }
         }

         layout.MethodID    = DTM_PROCLAYOUT;   /* render the object */
         layout.gpl_GInfo   = NULL;
         layout.gpl_Initial = TRUE;

         if (DoDTMethodA(obj, NULL, NULL, (Msg)&layout))
         {
            if (GetDTAttrs(obj,
                           PDTA_BitMap   , &bmap,
                           PDTA_CRegs    , &creg,
                           PDTA_NumColors, &ncol,
                           TAG_DONE) == 3)   /* get the bitmap and its colors */
            {
               if (bmap != NULL && creg != NULL && ncol != 0)
               {
                  crsize = (ncol * 3) * 4;
                  pic->palette_entries = ncol;
                  pic->palette_size = crsize + (2 * 4);  /* LoadRGB32() table requirements */

                  if (pic->palette = alloc_pool(pic->palette_size))
                  {
                     ctab = pic->palette;
                     *ctab++ = (ncol << 16) | 0;   /* number of colors and first color to load */
                     memcpy(ctab, creg, crsize);
                     *(ctab + (crsize / 4)) = 0;   /* terminator */
                  }
                  else err = ERROR_NO_FREE_STORE;

                  if (pic->bmap = AllocBitMap(pic->bmhd.bmh_Width, pic->bmhd.bmh_Height, pic->bmhd.bmh_Depth, bmap_flags, bmap))
                  {
                     BltBitMap(bmap, 0,0, pic->bmap, 0,0, pic->bmhd.bmh_Width, pic->bmhd.bmh_Height, 0xC0, 0xFF, NULL);
                     WaitBlit();
                  }
                  else err = ERROR_NO_FREE_STORE;
               }
               else err = ERROR_REQUIRED_ARG_MISSING;
            }
            else err = IoErr();
         }
         else err = IoErr();
      }
      else err = ERROR_REQUIRED_ARG_MISSING;

      DisposeDTObject(obj);   /* free the object */
   }
   else err = IoErr();

   if (err)
      FreePicture(pic);

   return(err);
}

/***----------------------------------------------------------------------***/
/*** localization support                                                 ***/
/***----------------------------------------------------------------------***/

/*
 * Localize id
 *
 * loc_id: string id
 *
 * return: string, or pointer to **string missing if not found
 *
 */
strptr LOC(ulong loc_id)
{
  static const char no_str[]="**string missing";

  return(GetCatalogStr(gl_catalog,loc_id,no_str));
}

/*
 * Localize menu structure
 *
 * menu  : pointer to first NewMenu
 * count : maximum items to localize, NM_END in menu stops always
 *
 * return: -
 *
 */
void LocalizeMenu(struct NewMenu *menu, long count)
{
  while(menu->nm_Type != NM_END AND count-- > 0)
  {
    if (menu->nm_Label != NM_BARLABEL) {
      menu->nm_Label = LOC(menu->nm_Label);
    }
    menu++;
  }
}

/*
 * Localize string pointer list
 *
 * list  : pointer to strptr list
 * count : how many pointers, 0 in list stops always
 *
 * return: -
 *
 */
void LocalizeList(strptr *list, long count)
{
  while (*list != 0 AND count-- > 0)
  {
    *list = LOC(*list);
    list++;
  }
}

/***----------------------------------------------------------------------***/
/*** misc routines                                                        ***/
/***----------------------------------------------------------------------***/

/*
 * Replace string pointer with new string
 * 
 * p1: ptr to string ptr (string to replace)
 * p2: new string
 *
 * return: TRUE if succesfull (old string is freed), FALSE otherwise
 *
 */
bool StrRep(strptr *p1,strptr p2)
{
strptr pnew;

  if ( pnew = alloc_pool(strlen(p2)+1) ) {
    strcpy(pnew,p2);
    free_pool(*p1);
    *p1 = pnew;
    return(TRUE);
  }
  return(FALSE);
}

/* etsi lähin väri joka ei ole 'exclude pen' ja allokoi se */
long MyObtainBestPen(struct ColorMap *cm,ulong r2,ulong g2, ulong b2, long ex)
{
bool  zigzag=FALSE;
long  j,pen;
ulong r,g,b;

  for(j=0; j < 512; j++)
  {
    if (zigzag) {
      zigzag = FALSE;
      r = r2 + (j/2) * 16777216;
      g = g2 + (j/2) * 16777216;
      b = b2 + (j/2) * 16777216;
      if (r < r2) r = ~0;
      if (g < g2) g = ~0;
      if (b < b2) b = ~0;
    } else {
      zigzag = TRUE;
      r = r2 - (j/2) * 16777216;
      g = g2 - (j/2) * 16777216;
      b = b2 - (j/2) * 16777216;
      if (r > r2) r = 0;
      if (g > g2) g = 0;
      if (b > b2) b = 0;
    }
    pen=ObtainBestPen(cm,r,g,b,OBP_Precision,PRECISION_EXACT,TAG_END);
    if (pen != ex) return(pen);
    ReleasePen(cm,pen);
    pen = -1;
  }
  return(pen);
}

/* etsi lähin väri joka ei ole 'exclude pen' */
long MyFindColor(struct ColorMap *cm,ulong r2,ulong g2, ulong b2, ulong lim, long ex)
{
bool  zigzag=FALSE;
long  j,pen;
ulong r,g,b;

  for(j=0; j < 512; j++)
  {
    if (zigzag) {
      zigzag = FALSE;
      r = r2 + (j/2) * 16777216;
      g = g2 + (j/2) * 16777216;
      b = b2 + (j/2) * 16777216;
      if (r < r2) r = ~0;
      if (g < g2) g = ~0;
      if (b < b2) b = ~0;
    } else {
      zigzag = TRUE;
      r = r2 - (j/2) * 16777216;
      g = g2 - (j/2) * 16777216;
      b = b2 - (j/2) * 16777216;
      if (r > r2) r = 0;
      if (g > g2) g = 0;
      if (b > b2) b = 0;
    }
    pen = FindColor(cm,r,g,b,lim);
    if (pen != ex) return(pen);
  }
  return(pen);
}

/*
 * Create temporary name (with full path)
 *
 * str   : string to be filled with temporary name (at least STRSIZE)
 *
 * return: -
 *
 */
void TempName(strptr str)
{
  sprintf(str,"%s__dterm.%lx(%lx).tmp",CUR(PB_TempDir),gl_process,GetUniqueID());
}

/*
 * Examine file size
 *
 * str   : filename
 *
 * return: size or -1 for error
 *
 */
long FileSize(strptr str)
{
long                  ret = -1;
struct Lock          *lock;
struct FileInfoBlock *fib;

  if (fib = AllocDosObjectTags(DOS_FIB,TAG_END)) {
    if (lock = Lock(str,ACCESS_READ)) {
      if(Examine(lock,fib)) ret = fib->fib_Size;
      UnLock(lock);
    }
    FreeDosObject(DOS_FIB,fib);
  }
  return(ret);
}

/*
 * Copy file
 *
 * from  : file to copy
 * to    : new file
 *
 * return: success (TRUE/FALSE)
 *
 */
long CloneFile(strptr from, strptr to)
{
long   result = TRUE,
       size_file,
       size_read,
       size_buffer;
bptr   fh1,fh2;
ubyte *buffer;

  if ( (size_buffer = size_file = FileSize(from)) != -1 ) {
    while ( (buffer = alloc_pool(size_buffer)) == 0 )
    {
      if ( (size_buffer /= 2) < 8 ) return(FALSE);
    }
    if (fh1 = dt_open(from,MODE_READ,size_buffer)) {
      if (fh2 = dt_open(to,MODE_WRITE,size_buffer)) {
        while (size_file)
        {
          size_file -= ( size_read = ( size_file > size_buffer ) ? size_buffer : size_file );
          if (dt_read (fh1,buffer,size_read) != size_read) { result = FALSE; break; }
          if (dt_write(fh2,buffer,size_read) != size_read) { result = FALSE; break; }
        }
        dt_close(fh2);
      }
      dt_close(fh1);
    }
    free_pool(buffer);
    if (!result) DeleteFile(to);
    return(result);
  }
  return(FALSE);
}

/***----------------------------------------------------------------------***/
/*** string conversion and txd send                                       ***/
/***----------------------------------------------------------------------***/

/*
 * Make B/KB/MB string from value (support for dprintf_user)
 *
 * str   : destination, (must 7 bytes of space, string is always 6 bytes)
 * value : value
 *
 * return: -
 *
 */
static void str_mb(strptr str, long value)
{
  if (value < 100000) { /* xxxxxB */
    sprintf(str,"%05ldB",value);
  } else if (value < (1024 * 1024)) { /* xxxxKB */
    sprintf(str,"%04ldkB",value/1024);
  } else if (value < (1024*1024 * 10)) { /* x.xxMB */
    sprintf(str,"%01ld.%02ldMB",value/1048576,(value % 1048576) / 10486);
  } else if (value < (1024*1024 * 100)) { /* xx.xMB */
    sprintf(str,"%02ld.%01ldMB",value/1048576,(value % 1048576) / 104858);
  } else if (value < 2147483647) { /* xxxxMB */
    sprintf(str,"%04ldMB",value/1048576);
  } else {
    strcpy(str,"> 2GB!");
  }
}

/*
 * Copy string to some field (support for dprintf_user)
 *
 * to    : destination
 * from  : formatstring
 * width : field size (0 == no field!)
 * size  : destination size (must be >= width)
 * left  : non zero to fill from left
 * pad1  : start pad
 * pad2  : end pad
 *
 * return: characters written (0 for target overflow!)
 *
 */
static long subcpy(strptr to, strptr from, long width, long size, long left, ubyte pad1, ubyte pad2)
{
long j   = 0;
long len = strlen(from);

  if (left == 0) {
    while ((width - len) > 0)
    {
      *to++ = pad1;
      j++;
      size--;
      width--;
    }
  }
  while (len)
  {
    if (size == 0) return(0);
    *to++ = *from++;
    j++;
    len--;
    size--;
    width--;
  }
  while (width > 0)
  {
    *to++ = pad2;
    j++;
    width--;
  }
  return(j);
}

/*
 * Converte DreamTerm string codes to string.
 *
 * str   : destination, maximum of STRSIZE is written (includes 0 byte)!
 * format: formatstring
 *
 * Note: ~ is not translated as is it used only when string is sent to serial.
 *
 *         \n  = linefeed  (0x0a)  (char )
 *         \r  = return    (0x0d)  (char )
 *         \t  = tab       (0x09)  (char )
 *         \b  = backspace (0x08)  (char )
 *         \e  = escape    (0x1b)  (char )
 *         \\  = \                 (char )
 *         ^x  = control           (char )
 *         ^^  = ^                 (char )
 *
 * These are special which just copy from current settings, %vx is treated
 * as numerical and %rx as string.
 *
 *         %va = current rxd speed      
 *         %vb = current txd speed      
 *         %vc = current cost
 *         %vd = current dte            
 *         %vf = current overruns       
 *         %vk = current columns        
 *         %vl = current lines          
 *         %vo = current online time
 *         %vr = current rxd bytes
 *         %vt = current txd bytes
 *         %vu = current serial unit    
 *         %vy = current serial unit+1  
 *
 *         \p  = current password        only for compatibility!
 *         %rb = current bbs name       
 *         %rc = current cost           (xxx.xx  )
 *         %rd = current download dir   
 *         %re = current emulation      (Ansi/VT,Hex,XEm 2.0)
 *         %rf = current port flags     (8N1     )
 *         %rh = current cts/rts/online (CRO     )
 *         %ri = current font name      
 *         %rl = current protocol       
 *         %rm = current phone          
 *         %ro = current online time    (xx:xx:xx)
 *         %rp = current password       
 *         %rr = current rxd bytes      (xxxxxB,xxxxkB,xxxxMB)
 *         %rs = current system time    (00:00:00)
 *         %rt = current txd bytes      (xxxxxB,xxxxkB,xxxxMB)
 *         %ru = current upload dir     
 *         %rv = current serial device  
 *         %rx = current pubscreen name (always used screen!)
 *
 * return: -
 *
 */
void dprintf_user(strptr str, strptr format)
{
static char str_tmp[STRSIZE];
long   i       = 0;
long   type    = 0;
long   type_go = 0;
long   jus_left;
long   pad_zero;
long   width;
ubyte  c;
strptr ori_form;

  while ((c = *format++) != 0 AND i < (STRSIZE-1))
  {
    switch (type)
    {
      case 0: /* direct copy */
        switch (c)
        {
          case '^' : type = 1;     break;
          case '\\': type = 2;     break;
          case '%' : type = 3;     break;
          default  : str[i++] = c; break;
        }
        break;
      case 1: /* ctrl code   */
        if (c != '^') {
          if (c >= 'A' AND c <= 'Z') {
            str[i++] = (c + 1) - 'A';
          } else if (c >= 'a' AND c <= 'z') {
            str[i++] = (c + 1) - 'a';
          }
        } else {
          str[i++] = '^';
        }
        type = 0;
        break;
      case 2: /* \ code      */
        switch (c)
        {
          case 'n' : str[i++] = 0x0a; break;
          case 'r' : str[i++] = 0x0d; break;
          case 't' : str[i++] = 0x09; break;
          case 'b' : str[i++] = 0x08; break;
          case 'e' : str[i++] = 0x1b; break;
          case '\\': str[i++] = '\\'; break;
          case 'p' : /* for compatibility! */
          {
          long len = strlen(CUR(PB_PassWord));

            if ( (i+len) < (STRSIZE-1) ) {
              strcpy(&str[i],CUR(PB_PassWord));
              i += len;
            } else {
              str[i] = 0;
              i = STRSIZE-1;
            }
          }
          break;
        }
        type = 0;
        break;
      case 3: /* %rx code    */
        switch (type_go)
        {
          case 0: /* justification */
            pad_zero = 0; /* no zero padding     */
            jus_left = 0; /* right justification */
            width    = 0; /* no width            */
            ori_form = format-1;
            if (c == '-') {
              jus_left = 1;
              break;
            }
            type_go  = 1;
          case 1: /* width and limit */
            if ( c >= '0' AND c <= '9' ) {
              if (c == '0') pad_zero = 1;
              format--;
              format += stcd_l(format,&width);
              type_go = 2;
              break;
            }
            type_go  = 2;
          case 2: /* lenght (ignored!) */
            type_go = 3;            
            if (c == 'l') break;
          case 3: /* type (pass others than %rx/%vx!) */
            if (c == 'v') {
              type_go = 4;
            } else if (c == 'r') {
              type_go = 5;
            } else {
              format   = ori_form;
              str[i++] = '%';
              type     = 0;
              type_go  = 0;
            }
            break;
          case 4: /* %vx type */
            {
            long len,value;

              switch (c)
              {
                case 'a': value = SerialInfo.rxdcps;   break; /* rxd speed     */
                case 'b': value = SerialInfo.txdcps;   break; /* txd speed     */
                case 'c': value = gl_onlinecost;       break; /* cost          */
                case 'd': value = CUR(PB_DteRate);     break; /* dte           */           
                case 'f': value = SerialInfo.overrun;  break; /* overruns      */
                case 'k': value = CUR(PB_Columns);     break; /* columns       */
                case 'l': value = CUR(PB_Lines);       break; /* lines         */
                case 'o': value = gl_onlinetime;       break; /* online time   */
                case 'r': value = SerialInfo.rxdbytes; break; /* rxd bytes     */
                case 't': value = SerialInfo.txdbytes; break; /* txd bytes     */
                case 'u': value = CUR(PB_SerUnit);     break; /* serial unit   */
                case 'y': value = CUR(PB_SerUnit)+1;   break; /* serial unit+1 */
                default : value = 0; break;
              }

              if ( (i+width) < (STRSIZE-1) ) {
                stcl_d(str_tmp,value);
                if (len = subcpy(&str[i],str_tmp,width,STRSIZE-i-1,jus_left,(pad_zero) ? '0' : ' ',' ')) {
                  i += len;
                } else {
                  str[i] = 0;
                  i = STRSIZE-1;
                }
              } else {
                str[i] = 0;
                i = STRSIZE-1;
              }

              type    = 0;
              type_go = 0;
            }
            break;
          case 5: /* %rx type */
            {
            long   len;
            strptr str_ptr = str_tmp;

              switch (c)
              {
                case 'b': str_ptr = CUR(PB_Name);      break; /* bbs name                  */
                case 'c':                                     /* cost           (xxx.xx  ) */
                  FastDec3(gl_onlinecost/100,&str_tmp[0]);
                  str_tmp[3] = '.';
                  FastDec2(gl_onlinecost%100,&str_tmp[4]);
                  str_tmp[6] = 0;
                  break;
                case 'd': str_ptr = CUR(PB_DnPath);    break; /* download dir              */
                case 'e':                                     /* emulation      (Ansi/XEm) */
                  if (CUR(PB_Emulation) == EM_ANSI) str_ptr = "Ansi/VT";
                  if (CUR(PB_Emulation) == EM_HEX)  str_ptr = "Hex";
                  if (CUR(PB_Emulation) == EM_XEM)  str_ptr = "XEm 2.0";
                  break;
                case 'f':                                     /* port flags     (8n1 etc ) */
                  FastDec1(CUR(PB_DataBits),&str_tmp[0]);
                  if (CUR(PB_Parity) == PAR_NONE)  str_tmp[1] = 'N';
                  if (CUR(PB_Parity) == PAR_EVEN)  str_tmp[1] = 'E';
                  if (CUR(PB_Parity) == PAR_ODD)   str_tmp[1] = 'O';
                  if (CUR(PB_Parity) == PAR_MARK)  str_tmp[1] = 'M';
                  if (CUR(PB_Parity) == PAR_SPACE) str_tmp[1] = 'S';
                  FastDec1(CUR(PB_StopBits),&str_tmp[2]);
                  str_tmp[3] = 0;
                  break;
                case 'h':                                     /* cts/rts/online (CRO etc ) */
                  str_tmp[0] = '-';
                  str_tmp[1] = '-';
                  str_tmp[2] = '-';
                  if (gl_onlinepb) str_tmp[2] = 'O';
                  if (gl_serok AND !gl_serfreeze) {
                    if (CheckCTS()) str_tmp[0] = 'C';
                    if (CheckRTS()) str_tmp[1] = 'R';
                  }
                  str_tmp[3] = 0;
                  break;
                case 'i': str_ptr = CUR(PB_Charset);   break; /* font name                 */
                case 'l': str_ptr = CUR(PB_Protocol);  break; /* protocol                  */
                case 'm': str_ptr = CUR(PB_Phone);     break; /* phone                     */
                case 'o':                                     /* online time    (00:00:00) */
                  TimeText(gl_onlinetime,str_tmp);
                  str_tmp[8] = 0;
                  break;
                case 'p': str_ptr = CUR(PB_PassWord);  break; /* password                  */
                case 'r':                                     /* rxd bytes      (Kb/Mb/Gb) */
                  str_mb(str_tmp,SerialInfo.rxdbytes);
                  break;
                case 's':                                     /* system time    (00:00:00) */
                  TimeText(GetSec() % (24 * 60 * 60),str_tmp);
                  str_tmp[8] = 0;
                  break;
                case 't':                                     /* txd bytes      (Kb/Mb/Gb) */
                  str_mb(str_tmp,SerialInfo.txdbytes);
                  break;
                case 'u': str_ptr = CUR(PB_UpPath);    break; /* upload dir                */
                case 'v': str_ptr = CUR(PB_SerName);   break; /* serial device             */
                case 'x':                                     /* pubscreen name            */
                  if (CUR(PB_Wb)) {
                    str_ptr = CUR(PB_PubScreen);
                  } else {
                    str_ptr = xget(gl_app,MUIA_Application_Base);
                  }
                  break;
                default : str_ptr = ""; break;
              }

              if ( (i+width) < (STRSIZE-1) ) {
                if (len = subcpy(&str[i],str_ptr,width,STRSIZE-i-1,jus_left,' ',' ')) {
                  i += len;
                } else {
                  str[i] = 0;
                  i = STRSIZE-1;
                }
              } else {
                str[i] = 0;
                i = STRSIZE-1;
              }

              type    = 0;
              type_go = 0;
            }
            break;
        }
        break;
    }
  }
  str[i] = 0;
}

/* NOT YET IMPLEMENTED, MUST THINK ABOUT THIS!
/*
 * Own sprintf with loads of extra features, handles all normal %x, \x and
 * ^x conversions + dreamterm own %ax, %sx, %vx.
 *
 * str   : destination  (remember that this don't limit size as dprintf_user!)
 * format: formatstring (mainly same as dprintf_user)
 *
 *         %am = Kb/Mb/Gb          (ulong) value is bytes
 *         %at = Time (xx:xx:xx)   (ulong) value is seconds
 *
 *         Remember that dprintf_user codes do no need value!
 *
 * args  : values (note all must be longwords)
 *
 * return: -
 *
 */
void dprintf(strptr str, strptr format, ...)
{
}
*/

#define add_pos(a)  txd_pos  = ((txd_pos+1)  < gl_txdsize) ? txd_pos  + 1 : 0;
#define add_rpos(a) txd_rpos = ((txd_rpos+1) < gl_txdsize) ? txd_rpos + 1 : 0;

static long txd_rpos =0;  /* next send index  */
            txd_wpos =0;  /* next write index */
            txd_wait =0;  /* blocks in buffer */
            txd_used =0;  /* buffer used      */

/*
 * Insert dreamterm string to serial buffer and start sending it, however
 * don't use this routine directly but Send_string, Send_translate and
 * Send_raw macros. If serial/program condition is such that data can't be
 * send then it is simply flushed. Routine also relies heavily in buffer
 * size, in theory buffer can be overflown if many strings has waits (~).
 *
 * This routine will start to wait current send when buffer is too full!
 *
 * str   : string to insert
 * size  : string size, pass 0 for no-op and 2048 is MAXIMUM!
 * type  : 0 = do every conversion, resulting string is maximum of STRSIZE
 *         1 = do only charset conversion
 *         2 = do nothing
 *
 * return: sets global variable gl_txdleft and also sets signal so main_loop
 *         will immediately call send_buffer().
 *
 */
void Send_insert(strptr str, long size, long type)
{
static char str_tmp[STRSIZE];
long   make_type = 0;
long   make_head = 1;
long   make_crlf = 0;
long   make_wait = 0;
long   make_deli    ;
long   make_left = gl_txdsize - txd_used;
long   make_size = 0;
long   txd_pos   = txd_wpos;
long   old_wpos  = txd_wpos;
long   old_wait  = txd_wait;
long   old_used  = txd_used;
ubyte  c,
       nil;
ubyte *make_ptr1 = &nil;
ubyte *make_ptr2 = &nil;

  if (gl_serok AND !gl_serfreeze) {
    if (type == 0) {
      dprintf_user(str_tmp,str);
      str  = str_tmp;
      size = strlen(str_tmp);
    }
    if (size == 0) return;

    while (size--)
    {
      if (make_crlf == 0) {
        c = *str++;
        switch (type)
        {
          case 0: /* pause conversion + type 1 */
            if (make_wait) {
              if (c == '~') {
                c         = '~';
                make_deli = 0;
              } else {
                c         = '\\';
                make_deli = 1;
              }
              make_wait = 2;
            } else {
              if (c == '\\') {
                make_wait = 1;
                break;
              } else if (c == '~') {
                make_type = 1; /* pause block */
                make_head = 1;
                break;
              }
            }
          case 1: /* charset conversion */
            c = gl_curtxd[c];
            if (c == 0x0a AND CUR(PB_lf2crlf)) {
              c         = 0x0d;
              make_crlf = 1;
            } else if (c == 0x0d AND CUR(PB_cr2crlf)) {
              c         = 0x0d;
              make_crlf = 1;
            }
        }
      }
  
      if (make_head) { /* make block header */
        if ((make_left -= 3) >= 0) {
          gl_txdbuffer[txd_pos] = make_type; add_pos(1);
          make_ptr1[0] = (ubyte)((make_size>>8) & 0xff);
          make_ptr2[0] = (ubyte)( make_size     & 0xff);
          make_ptr1    = &gl_txdbuffer[txd_pos]; add_pos(1);
          make_ptr2    = &gl_txdbuffer[txd_pos]; add_pos(1);
          make_head    = 0;
          make_size    = 0;
          txd_wait    += 1;
        } else {
          txd_wpos = old_wpos;
          txd_wait = old_wait;
          txd_used = old_used;
          return;
        }
      }

      if (make_type == 0) {
        switch (make_wait)
        {
          case 0:
            if (c != 0 OR type == 2) {
              if ((make_left -= 1) >= 0) {
                gl_txdbuffer[txd_pos] = c; add_pos(1);
                make_size += 1;
              } else {
                txd_wpos = old_wpos;
                txd_wait = old_wait;
                txd_used = old_used;
                return;
              }
            }
            break;
          case 2:
            if ((make_left -= 1) >= 0) {
              gl_txdbuffer[txd_pos] = c; add_pos(1);
              make_size += 1;
              if (make_deli == '\\') {
                str--;
                size++;
              }
            } else {
              txd_wpos = old_wpos;
              txd_wait = old_wait;
              txd_used = old_used;
              return;
            }
            make_wait = 0;
            break;
        }
        if (make_crlf == 1) {
          size++;
          c         = 0x0a;
          make_crlf = 2;
        } else if (make_crlf == 2) {
          make_crlf = 0;
        }
        if (txd_pos == 0) { /* buffer wrap */
          make_type  = 0;
          make_head  = 1;
        }
      } else { /* we did pause block, now do another */
        make_type  = 0;
        make_head  = 1;
      }

    }
    txd_used += ((gl_txdsize - txd_used) - make_left);
    txd_wpos  = txd_pos;
    make_ptr1[0] = (ubyte)((make_size>>8) & 0xff);
    make_ptr2[0] = (ubyte)( make_size     & 0xff);
    Signal(gl_process,gl_wakesig);
    gl_txdleft = (gl_txdsize - txd_used) / 2;
  }
#ifdef DEBUG
  printf("Send_insert\n");
  printf(" txd_rpos  : %4ld  ", txd_rpos);
  printf(" txd_wpos  : %4ld\n", txd_wpos);
  printf(" txd_wait  : %4ld  ", txd_wait);
  printf(" txd_used  : %4ld\n", txd_used);
  printf(" type      : %4ld  ", type);
  printf(" size      : %4ld\n", size);
#endif
}

/*
 * Clear TXD buffer, this may hang to wait current send finish.
 *
 * -     : -
 *
 * return: -
 *
 */
void Clear_buffer(void)
{
  if (!CheckTXD()) {
    WaitTXD();
  }
  txd_rpos   = 0;
  txd_wpos   = 0;
  txd_wait   = 0;
  txd_used   = 0;
  gl_txdleft = (gl_txdsize - txd_used) / 2;
}

/*
 * Send next part from TXD buffer, if data is present in buffer but it can't
 * be sent then it will stay until send succeeds.
 *
 * -     : -
 *
 * return: 0 = do nothing
 *         1 = don't call again before waiting 0.5 second
 *         (also sets global variable gl_txdleft)
 *
 */
long Send_buffer(void)
{
long block_size;

  if (gl_serok AND !gl_serfreeze AND txd_wait) {
    if (CheckTXD()) {

      switch (gl_txdbuffer[txd_rpos])
      {
        case 0: /* data to send     */
          add_rpos(1);
          block_size  = gl_txdbuffer[txd_rpos]<<8; add_rpos(1);
          block_size += gl_txdbuffer[txd_rpos];    add_rpos(1);
          InsertTXD(&gl_txdbuffer[txd_rpos],block_size);
          txd_wait   -= 1;
          txd_used   -= (block_size+3);
          txd_rpos   +=  block_size;
          if (txd_rpos >= gl_txdsize) txd_rpos = 0;
#ifdef DEBUG
  printf("Send_buffer (send)\n");
  printf(" txd_rpos  : %4ld  ", txd_rpos);
  printf(" txd_wpos  : %4ld\n", txd_wpos);
  printf(" txd_wait  : %4ld  ", txd_wait);
  printf(" txd_used  : %4ld\n", txd_used);
  printf(" block_size: %4ld\n", block_size);
#endif
          gl_txdleft = (gl_txdsize - txd_used) / 2;
          return(0);
        case 1: /* 0.5 second pause */
          txd_wait -= 1;
          txd_used -= 3;
          add_rpos(1);
          add_rpos(1);
          add_rpos(1);
#ifdef DEBUG
  printf("Send_buffer (pause)\n");
  printf(" txd_rpos  : %4ld  ", txd_rpos);
  printf(" txd_wpos  : %4ld\n", txd_wpos);
  printf(" txd_wait  : %4ld  ", txd_wait);
  printf(" txd_used  : %4ld\n", txd_used);
  printf(" block_size: n/a\n");
#endif
          gl_txdleft = (gl_txdsize - txd_used) / 2;
          return(1);
      }

    }
  }
  return(0);
}

/***----------------------------------------------------------------------***/
/*** Tag routines (high level information from them)                      ***/
/***----------------------------------------------------------------------***/

/*
 * Returns name for tag, if undefined (~0 is always) then emptry string
 *
 * tag   : tag
 *
 * return: string
 *
 */
strptr TagName(ulong tag)
{
strptr tagstr;

  switch (tag)
  {
    /*** phonebook tags ***/

    /* str */
    case PB_Name       : tagstr = LOC(MSG_name)                    ; break;
    case PB_Phone      : tagstr = LOC(MSG_phone)                   ; break;
    case PB_PassWord   : tagstr = LOC(MSG_password)                ; break;
    case PB_InitString : tagstr = LOC(MSG_initstring)              ; break;
    case PB_Comment    : tagstr = LOC(MSG_comment)                 ; break;
    case PB_DialString : tagstr = LOC(MSG_dial_prefix)             ; break;
    case PB_Func1      : tagstr = LOC(MSG_f1)                      ; break;
    case PB_Func2      : tagstr = LOC(MSG_f2)                      ; break;
    case PB_Func3      : tagstr = LOC(MSG_f3)                      ; break;
    case PB_Func4      : tagstr = LOC(MSG_f4)                      ; break;
    case PB_Func5      : tagstr = LOC(MSG_f5)                      ; break;
    case PB_Func6      : tagstr = LOC(MSG_f6)                      ; break;
    case PB_Func7      : tagstr = LOC(MSG_f7)                      ; break;
    case PB_Func8      : tagstr = LOC(MSG_f8)                      ; break;
    case PB_Func9      : tagstr = LOC(MSG_f9)                      ; break;
    case PB_Func10     : tagstr = LOC(MSG_f10)                     ; break;
    case PB_Charset    : tagstr = LOC(MSG_charset)                 ; break;
    case PB_Protocol   : tagstr = LOC(MSG_protocol)                ; break;
    case PB_SerName    : tagstr = LOC(MSG_serial_device)           ; break;
    case PB_LogName    : tagstr = LOC(MSG_logfile)                 ; break;
    case PB_UpPath     : tagstr = LOC(MSG_upload_path)             ; break;
    case PB_DnPath     : tagstr = LOC(MSG_download_path)           ; break;
    case PB_PhonePre   : tagstr = LOC(MSG_phone_prefix)            ; break;
    case PB_Xem        : tagstr = LOC(MSG_external_xem)            ; break;
    case PB_CryptLib   : tagstr = LOC(MSG_crypting_library)        ; break;
    case PB_PackLib    : tagstr = LOC(MSG_packing_library)         ; break;
    case PB_CryptKey   : tagstr = LOC(MSG_crypting_phrase)         ; break;
    case PB_TempDir    : tagstr = LOC(MSG_temporary_directory)     ; break;
    case PB_DnFiles    : tagstr = LOC(MSG_download_list)           ; break;
    case PB_Hangup     : tagstr = LOC(MSG_hangup_string)           ; break;
    case PB_NoCarrier  : tagstr = LOC(MSG_no_carrier)              ; break;
    case PB_NoDialTone : tagstr = LOC(MSG_no_dialtone)             ; break;
    case PB_Connect    : tagstr = LOC(MSG_connect)                 ; break;
    case PB_Ringing    : tagstr = LOC(MSG_ringing)                 ; break;
    case PB_Busy       : tagstr = LOC(MSG_busy)                    ; break;
    case PB_Ok         : tagstr = LOC(MSG_ok_string)               ; break;
    case PB_Error      : tagstr = LOC(MSG_error)                   ; break;
    case PB_DialSuffix : tagstr = LOC(MSG_dial_suffix)             ; break;
    case PB_DialAbort  : tagstr = LOC(MSG_dial_abort)              ; break;
    case PB_PubScreen  : tagstr = LOC(MSG_public_screen_name)      ; break;
    case PB_obsolete2  : tagstr = LOC(MSG_obsolete)                ; break;
    case PB_Toolbar    : tagstr = LOC(MSG_toolbar)                 ; break;
    case PB_StatusLine : tagstr = LOC(MSG_status_line)             ; break;
    /* val */ 
    case PB_DteRate    : tagstr = LOC(MSG_dte_rate)                ; break;
    case PB_FlowControl: tagstr = LOC(MSG_flow_control)            ; break;
    case PB_DataBits   : tagstr = LOC(MSG_databits)                ; break;
    case PB_Parity     : tagstr = LOC(MSG_parity)                  ; break;
    case PB_StopBits   : tagstr = LOC(MSG_stopbits)                ; break;
    case PB_FixRate    : tagstr = LOC(MSG_fixed_dte_rate)          ; break;
    case PB_LogCall    : tagstr = LOC(MSG_log_calls)               ; break;
    case PB_Status     : tagstr = LOC(MSG_status)                  ; break;
    case PB_Aga        : tagstr = LOC(MSG_enable_aga_bandwith)     ; break;
    case PB_CostFirst  : tagstr = LOC(MSG_call_cost)               ; break;
    case PB_CostRest   : tagstr = LOC(MSG_minute_cost)             ; break;
    case PB_HardScr    : tagstr = LOC(MSG_hardware_screen)         ; break;
    case PB_HardSer    : tagstr = LOC(MSG_hardware_serial)         ; break;
    case PB_ModeId     : tagstr = LOC(MSG_screen_mode)             ; break;
    case PB_Depth      : tagstr = LOC(MSG_depth)                   ; break;
    case PB_Columns    : tagstr = LOC(MSG_columns)                 ; break;
    case PB_Lines      : tagstr = LOC(MSG_lines)                   ; break;
    case PB_Wb         : tagstr = LOC(MSG_use_public_screen)       ; break;
    case PB_SerUnit    : tagstr = LOC(MSG_serial_unit)             ; break;
    case PB_Capture    : tagstr = LOC(MSG_capture)                 ; break;
    case PB_SerBufSize : tagstr = LOC(MSG_serial_buffer)           ; break;
    case PB_RxdPopup   : tagstr = LOC(MSG_auto_rxd_popup)          ; break;
    case PB_FullDuplex : tagstr = LOC(MSG_duplex)                  ; break;
    case PB_KeepWb     : tagstr = LOC(MSG_keep_workbench_colors)   ; break;
    case PB_Priority   : tagstr = LOC(MSG_priority)                ; break;
    case PB_cr2crlf    : tagstr = LOC(MSG_cr_to_crlf)              ; break;
    case PB_lf2crlf    : tagstr = LOC(MSG_lf_to_crlf)              ; break;
    case PB_RtsBlit    : tagstr = LOC(MSG_rts_blit)                ; break; 
    case PB_HLines     : tagstr = LOC(MSG_hardware_lines)          ; break;
    case PB_AutoXfer   : tagstr = LOC(MSG_auto_transfer)           ; break;
    case PB_QuietXfer  : tagstr = LOC(MSG_minimal_transfer_status) ; break;
    case PB_WaitEntry  : tagstr = LOC(MSG_wait_between_dials)      ; break;
    case PB_WaitDial   : tagstr = LOC(MSG_wait_dialing)            ; break;
    case PB_Emulation  : tagstr = LOC(MSG_emulation)               ; break;
    case PB_ANSIWrap   : tagstr = LOC(MSG_line_wrap)               ; break;
    case PB_ANSIKillBs : tagstr = LOC(MSG_destructive_backspace)   ; break;
    case PB_ANSIStrip  : tagstr = LOC(MSG_strip_bold)              ; break;
    case PB_ANSICls    : tagstr = LOC(MSG_esc2j_homes_cursor)      ; break;
    case PB_ANSICr2crlf: tagstr = LOC(MSG_cr_lf_to_cr_lf)          ; break;
    case PB_Double     : tagstr = LOC(MSG_double_buffer)           ; break;
    case PB_obsolete3  : tagstr = LOC(MSG_obsolete)                ; break;
    case PB_Safe       : tagstr = LOC(MSG_auto_phonebook_save)     ; break;
    case PB_CallLast   : tagstr = LOC(MSG_last_called)             ; break;
    case PB_CallCost   : tagstr = LOC(MSG_total_cost)              ; break;
    case PB_CallTime   : tagstr = LOC(MSG_total_time)              ; break;
    case PB_CallCount  : tagstr = LOC(MSG_total_calls)             ; break;
    case PB_DTRHangup  : tagstr = LOC(MSG_drop_dtr_to_hangup)      ; break;
    case PB_ANSIPc     : tagstr = LOC(MSG_pc_ansi)                 ; break;
    case PB_Crypt      : tagstr = LOC(MSG_phonebook_crypting)      ; break;
    case PB_obsolete1  : tagstr = LOC(MSG_obsolete)                ; break;
    case PB_Review     : tagstr = LOC(MSG_review)                  ; break;
    case PB_ReviewSize : tagstr = LOC(MSG_review_lines)            ; break;
    case PB_CallTxd    : tagstr = LOC(MSG_sent_bytes)              ; break;
    case PB_CallRxd    : tagstr = LOC(MSG_received_bytes)          ; break;
    case PB_PreScroll  : tagstr = LOC(MSG_prescroll)               ; break;
    case PB_BorderBlank: tagstr = LOC(MSG_borderblank)             ; break;
    case PB_ConX       : tagstr = LOC(MSG_console_x)               ; break; 
    case PB_ConY       : tagstr = LOC(MSG_console_y)               ; break;
    case PB_ForceConXY : tagstr = LOC(MSG_force_screenreqs_size)   ; break;
    case PB_Overscan   : tagstr = LOC(MSG_overscan)                ; break;
    case PB_AutoScroll : tagstr = LOC(MSG_autoscroll)              ; break;
    case PB_Freeze     : tagstr = LOC(MSG_freeze)                  ; break;
    case PB_Dispose    : tagstr = LOC(MSG_dispose_window_objects)  ; break;
    case PB_Baron      : tagstr = LOC(MSG_toolbar_active)          ; break;
    case PB_Barpos     : tagstr = LOC(MSG_toolbar_layout)          ; break;
    case PB_SerShared  : tagstr = LOC(MSG_serial_shared)           ; break;
    /* dat */
    case PB_ANSICol    : tagstr = ""                        ; break;
    case PB_HEXCol     : tagstr = ""                        ; break;

    /*** action tags ***/

    case cmd_none          : tagstr = LOC(MSG_no_command)            ; break;
    case cmd_open_current  : tagstr = LOC(MSG_edit_current_settings) ; break;
    case cmd_open_profile  : tagstr = LOC(MSG_edit_profile)          ; break;
    case cmd_open_xp       : tagstr = LOC(MSG_edit_protocols)        ; break;
    case cmd_open_cs       : tagstr = LOC(MSG_edit_character_sets)   ; break;
    case cmd_open_xm       : tagstr = LOC(MSG_edit_emulations)       ; break;
    case cmd_open_tb       : tagstr = LOC(MSG_edit_toolbar)          ; break;
    case cmd_load_default  : tagstr = LOC(MSG_load_configuration)    ; break;
    case cmd_save_default  : tagstr = LOC(MSG_save_configuration)    ; break;
    case cmd_titlebar      : tagstr = LOC(MSG_titlebar)              ; break;
    case cmd_open_crypt    : tagstr = LOC(MSG_edit_crypt_settings)   ; break;
    case cmd_about         : tagstr = LOC(MSG_about)                 ; break;
    case cmd_quit          : tagstr = LOC(MSG_quit)                  ; break;
    case cmd_open_phonebook: tagstr = LOC(MSG_edit_phonebook)        ; break;
    case cmd_new_entry     : tagstr = LOC(MSG_create_entry)          ; break;
    case cmd_load_phonebook: tagstr = LOC(MSG_load_default_phonebook); break;
    case cmd_save_phonebook: tagstr = LOC(MSG_save_default_phonebook); break;
    case cmd_dial_number   : tagstr = LOC(MSG_dial_number)           ; break;
    case cmd_dial_next     : tagstr = LOC(MSG_dial_next_selected)    ; break;
    case cmd_send_password : tagstr = LOC(MSG_send_current_password) ; break;
    case cmd_upload        : tagstr = LOC(MSG_upload)                ; break;
    case cmd_uploadlist    : tagstr = LOC(MSG_upload_using_list)     ; break;
    case cmd_download      : tagstr = LOC(MSG_download)              ; break;
    case cmd_send_conv     : tagstr = LOC(MSG_send_ascii)            ; break;
    case cmd_send_no_conv  : tagstr = LOC(MSG_send_ascii_no_conv)    ; break;
    case cmd_abort_send    : tagstr = LOC(MSG_abort_send)            ; break;
    case cmd_open_uplist   : tagstr = LOC(MSG_edit_upload_list)      ; break;
    case cmd_open_review   : tagstr = LOC(MSG_view_review)           ; break;
    case cmd_start_capture : tagstr = LOC(MSG_open_capture)          ; break;
    case cmd_hangup        : tagstr = LOC(MSG_hangup)                ; break;
    case cmd_release_serial: tagstr = LOC(MSG_release_serial)        ; break;
    case cmd_load_current  : tagstr = LOC(MSG_load_configuration)    ; break;
    case cmd_save_current  : tagstr = LOC(MSG_save_configuration)    ; break;
    case cmd_load_profile  : tagstr = LOC(MSG_load_profile)          ; break;
    case cmd_save_profile  : tagstr = LOC(MSG_save_profile)          ; break;

    default: tagstr = "";
  }
  return(tagstr);
}

/*
 * Converts tag data to general string, returned string will be overwritted
 * after this routine is called next time. 
 *
 * tag   : tag
 * data  : tag data  
 *
 * return: temporary string (private area provided by this function)
 *
 */
strptr TagData2Str(ulong tag, ulong data)
{
static char str[STRSIZE];
struct DateTime dt;

  if (tag >= STRMIN AND tag <= STRMAX) {
    strcpy(str,data);
  }
  if (tag >= VALMIN AND tag <= VALMAX) {
    switch (tag)
    {
      /* special tags */
      case PB_FlowControl:
        switch (data)
        {
          case FLOW_NONE   : strcpy(str,LOC(MSG_none))     ; break;
          case FLOW_XONXOFF: strcpy(str,LOC(MSG_xon_xoff)) ; break;
          case FLOW_RTSCTS : strcpy(str,LOC(MSG_rts_cts))  ; break;
        }
        break;
      case PB_Parity:
        switch (data)
        {
          case PAR_NONE : strcpy(str,LOC(MSG_none)) ; break;
          case PAR_EVEN : strcpy(str,LOC(MSG_even)) ; break;
          case PAR_ODD  : strcpy(str,LOC(MSG_odd))  ; break;
          case PAR_MARK : strcpy(str,LOC(MSG_mark)) ; break;
          case PAR_SPACE: strcpy(str,LOC(MSG_space)); break;
        }
        break;
      case PB_Status:
        switch (data)
        {
          case STAT_NONE  : strcpy(str,LOC(MSG_none))  ; break;
          case STAT_BAR   : strcpy(str,LOC(MSG_line))  ; break;
          case STAT_WINDOW: strcpy(str,LOC(MSG_window)); break;
        }
        break;
      case PB_Capture:
        switch (data)
        {
          case CAP_NONE: strcpy(str,LOC(MSG_none)); break;
          case CAP_TEXT: strcpy(str,LOC(MSG_text)); break;
          case CAP_RAW : strcpy(str,LOC(MSG_raw)) ; break;
        }
        break;
      case PB_Emulation:
        switch (data)
        {
          case EM_ANSI: strcpy(str,LOC(MSG_ansi_vt102)); break;
          case EM_HEX : strcpy(str,LOC(MSG_hex)) ; break;
          case EM_XEM : strcpy(str,LOC(MSG_xem)) ; break;
        }
        break;
      case PB_Freeze:
        if (data) {
          dt.dat_Flags   = 0;
          dt.ds_Days     = data / (24*60*60);
          dt.ds_Minute   = 0;
          dt.ds_Tick     = 0;
          dt.dat_Format  = FORMAT_DOS;
          dt.dat_StrDay  = 0;
          dt.dat_StrDate = str;
          dt.dat_StrTime = 0;
          DateToStr(&dt);
        } else {
          str[0] = 0;
        }
        break;
      case PB_CallLast:
        if (data) {
          dt.dat_Flags   = DTF_SUBST;
          dt.ds_Days     = data / (24*60*60);
          dt.ds_Minute   = 0;
          dt.ds_Tick     = 0;
          dt.dat_Format  = FORMAT_DOS;
          dt.dat_StrDay  = 0;
          dt.dat_StrDate = str;
          dt.dat_StrTime = 0;
          DateToStr(&dt);
        } else {
          str[0] = 0;
        }
        break;
      case PB_CallCost:
        if (gl_locale->loc_MonPositiveCSPos) {
          stcl_d(str,data/100);
          strcat(str,gl_locale->loc_MonCS);
        } else {
          strcpy(str,gl_locale->loc_MonCS);
          stcl_d(str+strlen(str),data/100);
        }
        break;
      case PB_CallTime:
        TimeText(data,str);
        str[8] = 0;
        break;
      case PB_CallTxd:
      case PB_CallRxd:
        sprintf(str,LOC(MSG_xx_mb),data/1048576, (data % 1048576) / 10486);
        break;
      /* boolean tags */
      case PB_FixRate     :
      case PB_LogCall     :
      case PB_Aga         :
      case PB_HardScr     :
      case PB_HardSer     :
      case PB_Wb          :
      case PB_RxdPopup    :
      case PB_FullDuplex  :
      case PB_KeepWb      :
      case PB_cr2crlf     :
      case PB_lf2crlf     :
      case PB_RtsBlit     :
      case PB_AutoXfer    :
      case PB_QuietXfer   :
      case PB_ANSIWrap    :
      case PB_ANSIKillBs  :
      case PB_ANSIStrip   :
      case PB_ANSICls     :
      case PB_ANSICr2crlf :
      case PB_Double      :
      case PB_Safe        :
      case PB_DTRHangup   :
      case PB_ANSIPc      :
      case PB_Crypt       :
      case PB_Review      :
      case PB_BorderBlank :
      case PB_ForceConXY  :
      case PB_AutoScroll  :
      case PB_Dispose     :
      case PB_Baron       :
      case PB_SerShared   :
        strcpy(str,(data) ? LOC(MSG_on) : LOC(MSG_off) );
        break;
      /* simple value tags */
      default:
        stcl_d(str,data);
        break;
    }
  }
  if (tag >= DATMIN AND tag <= DATMAX) {
    str[0] = 0;
  }
  return(str);
}

/*
 * Free name list created with MakeNameList()
 *
 * list  : name list created with MakeNameList(), can also be 0
 *
 * return: -
 *
 */
void FreeNameList(strptr *list)
{
  free_pool(list);
}

/*
 * Creates namelist from tags
 *
 * tags  : tag array (only tags, not tagitems!)
 *         -1 = empty str ("")
 *          0 = enf of list
 *
 * return: 0 or strptr list (end of list is marked with NULL ptr)
 */
strptr *MakeNameList(ulong *tags)
{
ulong   i,
        count = 0;
strptr *list  = 0;

  while (tags[count++]) 
  {
  }
  if (list = alloc_pool(count*4)) {
    for (i=0; i < (count-1); i++)
    {
      list[i] = TagName(tags[i]);
    }
    list[i] = 0;
  }
  return(list);
}

/***----------------------------------------------------------------------***/
/*** Base tag handling (lowest level dealing with tagbase, should not be  ***/
/*** used outside!)                                                       ***/
/***----------------------------------------------------------------------***/

/*
 * Find tagitem from list
 *
 * tag    : tag
 * tagbase: tagbase
 *
 * return : tagitem or 0 for not founding
 *
 */
struct TagItem *FindTag(ulong tag, struct TagBase *tagbase)
{
struct TagItem *ti;

  if (ti = tagbase->tagdata) {
    while (ti->ti_Tag != TAG_END)
    {
      if (ti->ti_Tag == tag) return(ti);
      ti++;
    }
  }
  return(0);
}

/*
 * Check if tagitem is in taglist (obsolete, use FindTag insted!)
 *
 * tag    : tag
 * tagbase: tagbase
 *
 * return : TRUE/FALSE
 *
 */
long LookTag(ulong tag, struct TagBase *tagbase)
{
  return(FindTag(tag,tagbase));
}

/*
 * Remove tagitem from list, just marks tag as TAG_IGNORE and frees data
 *
 * tag    : tag
 * tagbase: tagbase
 *
 * return : -
 *
 */
void DelTag(ulong tag, struct TagBase *tagbase)
{
struct TagItem *ti;

  if (ti = tagbase->tagdata) {
    while (ti->ti_Tag != TAG_END)
    {
      if (ti->ti_Tag == tag) {
        if ( tag >= STRMIN AND tag <= STRMAX ) {
          free_pool(ti->ti_Data);
        }
        if ( tag >= DATMIN AND tag <= DATMAX ) {
          free_pool(ti->ti_Data);
        }
        ti->ti_Tag = TAG_IGNORE;
        tagbase->tagfree++;
#ifdef DEBUG
        gl_tags_now--;
        gl_tags_ign++;
        gl_tags_del++;
#endif
        return;
      }
      ti++;
    }
  }
}

/*
 * Replace or add tagitem to list, always succeeds (problematic in low memory...)
 *
 * tagid  : tag
 * tagdata: data (pointer if STR or DAT tag)
 * tagbase: tagbase
 *
 * return : -
 *
 */
void AddTag(ulong tagid, ulong tagdata, struct TagBase *tagbase)
{
ulong           *tp;
strptr           p;
struct TagItem  *tags,*oldtags,*oldtemp;
  
  if (tagid == TAG_IGNORE) return;

  if ((tags = tagbase->tagdata) == 0) { /* allocate first time */
    tagbase->tagdata = tags = alloc_force((TAGSTART+1)*8);
    tagbase->tagfree = TAGSTART;
    tagbase->tagmax  = TAGSTART;
    tags->ti_Tag     = TAG_END;
  }

  DelTag(tagid,tagbase);

  if ( tagbase->tagfree == 0 ) { /* allocate more tagspace */
    tags = alloc_force( tagbase->tagmax*8 + (TAGALLOC+1)*8 );
    oldtags = oldtemp = tagbase->tagdata;
    tagbase->tagdata  = tags;
    tagbase->tagfree += TAGALLOC;
    tagbase->tagmax  += TAGALLOC;
    while (oldtags->ti_Tag != TAG_END)
    {
      tags->ti_Tag  = oldtags->ti_Tag;
      tags->ti_Data = oldtags->ti_Data;
      tags++; oldtags++;
    }
    tags->ti_Tag = TAG_END;
    free_pool(oldtemp);
  }
  tags = tagbase->tagdata;
  tagbase->tagfree--;

  while (tags->ti_Tag != TAG_END AND tags->ti_Tag != TAG_IGNORE) tags++;
#ifdef DEBUG
  if (tags->ti_Tag == TAG_IGNORE) gl_tags_ign--;
#endif
  if (tags->ti_Tag == TAG_END) {
    tags++; tags->ti_Tag = TAG_END; tags--;
  }

  if ( (tagid >= STRMIN) AND (tagid <= STRMAX) ) {
    p = alloc_force(strlen((char *)tagdata)+1);
    strcpy(p,(char *)tagdata);
    tags->ti_Tag  = tagid;
    tags->ti_Data = (ulong)p;
    /*--- curbbs kludge ---*/
    if ( &gl_curbbs->tags == tagbase ) gl_cur_qs[tagid - STR_PB_FIRST] = p;
    /*--- curbbs kludge ---*/
#ifdef DEBUG
    gl_tags_now++;
    gl_tags_add++;
#endif
    return;
  }
  if ( (tagid >= VALMIN) AND (tagid <= VALMAX) ) {
    tags->ti_Tag	= tagid;
    tags->ti_Data	= tagdata;
    /*--- curbbs kludge ---*/
    if ( &gl_curbbs->tags == tagbase ) gl_cur_qv[tagid - VAL_PB_FIRST] = tagdata;
    /*--- curbbs kludge ---*/
#ifdef DEBUG
    gl_tags_now++;
    gl_tags_add++;
#endif
    return;
  }
  if ( tagid >= DATMIN AND tagid <= DATMAX ) {
    tp = (ulong *)tagdata;
    p = alloc_force(tp[0]+4);
    CopyMem(tagdata,p,tp[0]+4);
    tags->ti_Tag  = tagid;
    tags->ti_Data = (ulong)p;
    /*--- curbbs kludge ---*/
    if ( &gl_curbbs->tags == tagbase ) gl_cur_qd[tagid - DAT_PB_FIRST] = p;
    /*--- curbbs kludge ---*/
#ifdef DEBUG
    gl_tags_now++;
    gl_tags_add++;
#endif
    return;
  }
  EndProg("AddTag()");
}

/*
 * Remove every tagitem from list and deallocate list
 *
 * tagbase: tagbase
 *
 * return : -
 *
 */
void DelTagList(struct TagBase *tagbase)
{
struct TagItem *ti;

  if (ti = tagbase->tagdata) {
    while (ti->ti_Tag != TAG_DONE)
    {
      if ( (ti->ti_Tag >= STRMIN) AND (ti->ti_Tag <= STRMAX) ) {
        free_pool(ti->ti_Data);
#ifdef DEBUG
        gl_tags_now--;
        gl_tags_del++;
#endif
      }
      if ( (ti->ti_Tag >= DATMIN) AND (ti->ti_Tag <= DATMAX) ) {
        free_pool(ti->ti_Data);
#ifdef DEBUG
        gl_tags_now--;
        gl_tags_del++;
#endif
      }
#ifdef DEBUG
      if ( (ti->ti_Tag >= VALMIN) AND (ti->ti_Tag <= VALMAX) ) {
        gl_tags_now--;
        gl_tags_del++;
      }
      if (ti->ti_Tag == TAG_IGNORE) {
        gl_tags_ign--;
      }
#endif
      ti++;
    }
    free_pool(tagbase->tagdata);
    tagbase->tagdata = 0;
    tagbase->tagfree = 0;
  }
}

/*
 * Compare tagitem against taglist (ie. same tag in list)
 *
 * tag    : tag
 * tagdata: data    (data to be compared)
 * tagbase: tagbase (list to have same tag)
 *
 * return : negative = first is below the second (bigger )
 *              zero = equal
 *          positive = first is above the second (smaller) 
 *           (first = tag, second = tag in taglist)
 *           if taglist does not contain tag then result != zero
 */
long CmpTag(ulong tag, ulong tagdata, struct TagBase *tagbase)
{
struct TagItem *ti;

  if (ti = FindTag(tag,tagbase)) {
    if ( tag >= VALMIN AND tag <= VALMAX ) {
      if (tagdata  < ti->ti_Data) return(-1);
      if (tagdata == ti->ti_Data) return( 0);
      if (tagdata  > ti->ti_Data) return( 1);
    }
    if ( tag >= STRMIN AND tag <= STRMAX ) {
      return(strcmp((char *)tagdata,(char *)ti->ti_Data));
    }
    if ( tag >= DATMIN AND tag <= DATMAX ) {
      return(DatCmp(ti->ti_Data,tagdata));
    }
  }
  return(1);
}

/*
 * Copy tags to another taglist (both remain accessable!)
 *
 * from  : source tagbase
 * to    : target tagbase
 *
 * return: -
 *
 */
void CopyTags(struct TagBase *from, struct TagBase *to)
{
struct TagItem *ti;

  if (ti = from->tagdata) {
    while (ti->ti_Tag != TAG_DONE)
    {
      AddTag(ti->ti_Tag,ti->ti_Data,to);
      ti++;
    }
  }
}

/*
 * Load taglist to memory taglist, version 2
 *
 * fh     : file handle
 * tagbase: tagbase
 *
 * return : result (TRUE/FALSE)
 *
 */
static long load_taglist_ver2(bptr fh, struct TagBase *tagbase)
{
char  str[STRSIZE];
ulong file_dat[2];

  do 
  {
    if ( dt_read(fh,file_dat,8) == 8 ) {

      if ( file_dat[0] >= VALMIN AND file_dat[0] <= VALMAX ) {
        AddTag(file_dat[0],file_dat[1],tagbase);
      }

      if ( file_dat[0] >= STRMIN AND file_dat[0] <= STRMAX ) {
        if (file_dat[1] == 0) {
          str[0] = 0;
          AddTag(file_dat[0],str,tagbase);
        } else {
          if (file_dat[1] < STRSIZE ) {
            if ( dt_read(fh,str,file_dat[1]) == file_dat[1] ) {
              str[file_dat[1]] = 0;
              AddTag(file_dat[0],str,tagbase);
            } else {
              return(FALSE);
            }
          } else {
            return(FALSE);
          }
        }
      }

      if ( file_dat[0] >= DATMIN AND file_dat[0] <= DATMAX ) {
      ulong dat;

        if (dat = alloc_pool(file_dat[1])) {
          if ( dt_read(fh,dat,file_dat[1]) == file_dat[1] ) {
            AddTag(file_dat[0],dat,tagbase);
            free_pool(dat);
          } else {
            free_pool(dat);
            return(FALSE);
          }
        } else {
          return(FALSE);
        }
      }

    } else {
      return(FALSE);
    }
  } while (file_dat[0] != TAG_END);

  return(TRUE);
}

/*
 * Load 'tags_chunk' to memory taglist
 *
 * fh     : file handle
 * tagbase: tagbase
 *
 * return : result (TRUE/FALSE)
 *
 */
long LoadTags(bptr fh, struct TagBase *tagbase)
{
struct TagEntry tag_entry;

  if ( dt_read(fh,&tag_entry,sizeof(struct TagEntry)) == sizeof(struct TagEntry) ) {
    if (tag_entry.id == TAGID) {
      switch (tag_entry.version)
      {
        case TAGVER: /* we handle only current version (2) */
          return(load_taglist_ver2(fh,tagbase));
        default: 
          return(FALSE);
      }
    }
  }
  return(FALSE);
/*
char             tchar[258];
ulong           *p;
struct SubStr    sub;
struct TagEntry  e;

  if ( dt_read(fh,&e,sizeof(struct TagEntry)) != sizeof(struct TagEntry) ) return(FALSE);
  if (e.id != TAGID OR e.version > TAGVER) return(FALSE);
  if ( FRead(fh,&sub,sizeof(struct SubStr),1) != 1 ) return(FALSE);
  while (sub.tag != TAG_END)
  {
		if ( (sub.tag >= VALMIN) AND (sub.tag <= VALMAX) ) {
			AddTag(sub.tag,sub.tagsize,tagbase);
		}
		if ( (sub.tag >= STRMIN) AND (sub.tag <= STRMAX) ) {
			if (sub.tagsize == 0) {
				tchar[0] = 0x00;
				AddTag(sub.tag,(ulong)&tchar,tagbase);
			} else {
				if (sub.tagsize > 256) return(FALSE);
				if ( FRead(fh,&tchar,sub.tagsize,1) != 1 ) return(FALSE);
				tchar[sub.tagsize] = 0x00;
				AddTag(sub.tag,(ulong)&tchar,tagbase);
			}
		}
		if ( sub.tag >= DATMIN AND sub.tag <= DATMAX ) {
			if ( (p = AllocMyPooled(sub.tagsize)) == 0) return(FALSE);
			if ( FRead(fh,p,sub.tagsize,1) != 1 ) { FreeMyPooled(p); return(FALSE); }
			AddTag(sub.tag,p,tagbase);
			FreeMyPooled(p);
		}
		if ( FRead(fh,&sub,sizeof(struct SubStr),1) != 1 ) return(FALSE);
	}
	return(TRUE);
*/
}

/*
 * Save 'tags_chunk' from memory taglist
 *
 * tagbase: tagbase
 * fh     : file handle
 *
 * return : result (TRUE/FALSE)
 *
 */
long SaveTags(struct TagBase *tagbase, bptr fh)
{
ulong            file_dat[2];
struct TagItem  *ti;
struct TagEntry  tag_entry;

  tag_entry.id      = TAGID;
  tag_entry.version = TAGVER; /* save _always_ handles only current type */
  tag_entry.flags   = 0;
  tag_entry.size    = 0;

  if ( dt_write(fh,&tag_entry,sizeof(struct TagEntry)) == sizeof(struct TagEntry) ) {
    if ( ti = tagbase->tagdata ) {

      while (ti->ti_Tag != TAG_END)
      {

        if ( ti->ti_Tag >= VALMIN AND ti->ti_Tag <= VALMAX ) {
          if ( dt_write(fh,&ti->ti_Tag,8) != 8 ) return(FALSE);
        }

        if ( ti->ti_Tag >= STRMIN AND ti->ti_Tag <= STRMAX ) {
          file_dat[0] = ti->ti_Tag;
          file_dat[1] = strlen(ti->ti_Data);
          if ( dt_write(fh,file_dat   ,8          ) != 8           ) return(FALSE);
          if ( dt_write(fh,ti->ti_Data,file_dat[1]) != file_dat[1] ) return(FALSE);
        }

        if ( ti->ti_Tag >= DATMIN AND ti->ti_Tag <= DATMAX ) {
          file_dat[0] = ti->ti_Tag;
          file_dat[1] = ((ulong *)ti->ti_Data)[0] + 4;
          if ( dt_write(fh,file_dat   ,8          ) != 8           ) return(FALSE);
          if ( dt_write(fh,ti->ti_Data,file_dat[1]) != file_dat[1] ) return(FALSE);
        }

        ti++;
      }

    }
    file_dat[0] = TAG_END;
    file_dat[1] = TAG_END; /* can be anything but let's use TAG_END */
    if ( dt_write(fh,file_dat,8) == 8 ) {
      return(TRUE);
    }
  }
  return(FALSE);
/*
ulong           *tp;
struct SubStr    sub;
struct TagItem  *tags;
struct TagEntry  tag_entry;

  tag_entry.id      = TAGID;
  tag_entry.version = TAGVER; /* save _always_ handles only current type */
  tag_entry.flags   = 0;
  tag_entry.size    = 0;
	if ( FWrite(fh,&e,sizeof(struct TagEntry),1) != 1 ) return(FALSE);
	if ( (tags = tagbase->tagdata) != 0) 
	{
		while (tags->ti_Tag != TAG_END)
		{
			if ( (tags->ti_Tag >= VALMIN) AND (tags->ti_Tag <= VALMAX) ) {
				if ( FWrite(fh,&tags->ti_Tag,8,1) != 1 ) return(FALSE);
			}
			if ( (tags->ti_Tag >= STRMIN) AND (tags->ti_Tag <= STRMAX) ) {
				sub.tag			= tags->ti_Tag;
				sub.tagsize	= strlen((char *)tags->ti_Data);
				if (sub.tagsize == 0) {
					if ( FWrite(fh,&sub,8,1) != 1 ) return(FALSE);
				} else {
					if (sub.tagsize > 512) sub.tagsize = 512;
					if ( FWrite(fh,&sub,8,1) != 1 ) return(FALSE);
					if ( FWrite(fh,(void *)tags->ti_Data,sub.tagsize,1) != 1 ) return(FALSE);
				}
			}
			if ( tags->ti_Tag >= DATMIN AND tags->ti_Tag <= DATMAX ) {
				tp = (ulong *)tags->ti_Data;
				sub.tag			= tags->ti_Tag;
				sub.tagsize	= tp[0] + 4;
				if ( FWrite(fh,&sub,8,1) != 1 ) return(FALSE);
				if ( FWrite(fh,tp,sub.tagsize,1) != 1 ) return(FALSE);
			}
			tags++;
		}
	}
	sub.tag			= TAG_END;
	sub.tagsize	= TAG_END;
	if  ( FWrite(fh,&sub,8,1) != 1 ) return(FALSE);
	return(TRUE);
*/
}

/*
 * Save data from dat tag to file
 *
 * tag    : tag
 * tagbase: tagbase
 * file   : file
 *
 * return : result (TRUE/FALSE)
 *
 */
long SaveDatTag(ulong tag, struct TagBase *tagbase, char *file)
{
bptr   fh;
ulong *p = GetTag(tag,tagbase);

  if (p[0]) {
    if (fh = dt_open(file,MODE_WRITE,8192)) {
      if (dt_write(fh,&p[1],p[0]) != p[0]) { dt_close(fh); return(FALSE); }
      dt_close(fh);
      return(TRUE);
    }
  } else {
    DeleteFile(file);
    return(TRUE);
  }
}

/*** routines to handle specially formatted dat tag data ***/

/* these are 'kludges' to handle formatted data in dat tag, after everything
 * is done you simply add data back to taglist using AddTag().
 * notes:
 *  CreateDat will create empty data structure
 *  CountDat  will return index count
 *  DelDat    will delete data from desired index
 *  GetDat    will get data from desired index (index must exist!)
 *  SetDat    will set data and replace data if needed in desired index
 *            (DelDat and SetDat will always return new structure address!)
 *  index is 0-n
 * 
 * Currently only three special dat formats exits
 *
 * undefined dat: (can't be handled with these routines)
 *
 *  [0]   = normal dat tag size
 *  [...] = whatever
 *
 * long dat:
 *  [0]    = normal dat tag size
 *  [1..n] = longwords
 *
 * str dat:
 *  [0]    = normal dat tag size
 *  [1]    = strings stored
 *  [2]    = string area size
 *  [3..n] = offsets to strings
 *  [....] = strings
 *
 */

ulong *CreateDat(ulong tagid)
{
ulong *dat;

  if ( (tagid >= DLOMIN) AND (tagid <= DLOMAX) ) {
    dat    = alloc_force(4);
    dat[0] = 0;
    return(dat);
  }
  if ( (tagid >= DSTMIN) AND (tagid <= DSTMAX) ) {
    dat    = alloc_force(12);
    dat[0] = 8;
    dat[1] = 0;
    dat[2] = 0;
    return(dat);
  }
}

ulong CountDat(struct TagItem *tag)
{
ulong *dat = tag->ti_Data;

  if ( (tag->ti_Tag >= DLOMIN) AND (tag->ti_Tag <= DLOMAX) ) {
    return(dat[0]/4);
  }
  if ( (tag->ti_Tag >= DSTMIN) AND (tag->ti_Tag <= DSTMAX) ) {
    return(dat[1]);
  }
}

void DelDat(struct TagItem *tag, ulong index)
{
ulong  *dat=tag->ti_Data,*dat2,size,i,j=1;
strptr p;

  if ( (tag->ti_Tag >= DLOMIN) AND (tag->ti_Tag <= DLOMAX) ) {
    dat2    = alloc_force(dat[0]);
    dat2[0] = dat[0]-4;
    for (i = 1; i <= dat2[0]/4; i++)
    {
      if ( (index+1) == j ) j++;
      dat2[i] = dat[j++];
    }
  }
  if ( (tag->ti_Tag >= DSTMIN) AND (tag->ti_Tag <= DSTMAX) ) {
    size = 12 + (dat[1]-1)*4 + dat[2] - 
           strlen( (ulong) &dat[0] + dat[3+index] )-1;
    dat2 = alloc_force(size);
    p = &dat2[3+dat[1]-1];
    dat2[0] = size-4;
    dat2[1] = dat[1]-1;
    dat2[2] = size-12-(dat[1]-1)*4;
    for (i=1; i <= dat2[1]; i++)
    {
      if ( (index+1) == j) j++;
      dat2[3+i-1] = p-(strptr) &dat2[0];
      strcpy(p,(ulong) &dat[0] + dat[3+j-1]);
      p += strlen(p)+1;
      j++;
    }
  }
  free_pool(dat);
  tag->ti_Data = dat2;
}

ulong GetDat(struct TagItem *tag, ulong index)
{
ulong *dat=tag->ti_Data;

  if ( (tag->ti_Tag >= DLOMIN) AND (tag->ti_Tag <= DLOMAX) ) {
    return(dat[1+index]);
  }
  if ( (tag->ti_Tag >= DSTMIN) AND (tag->ti_Tag <= DSTMAX) ) {
    return( (ulong) dat + dat[3+index] );
  }
}

void SetDat(struct TagItem *tag, ulong new, ulong index)
{
ulong  *dat=tag->ti_Data,*dat2,size,i,str_size;
strptr p;

  if ( (tag->ti_Tag >= DLOMIN) AND (tag->ti_Tag <= DLOMAX) ) {
    index++;
    if (dat[0] >= (index*4)) {
      dat[index] = new;
    } else {
      dat2          = alloc_force((index+1)*4);
      dat2[0]       = index*4;
      dat2[index]   = new;
      CopyMem(&dat[1],&dat2[1],dat[0]);
      free_pool(dat);
      tag->ti_Data = dat2;
    }
  }
  if ( (tag->ti_Tag >= DSTMIN) AND (tag->ti_Tag <= DSTMAX) ) {
    index++;
    str_size = strlen(new);
    if (dat[1] >= index ) { /* tag existed and index entry already on */
      size = 12 + dat[1]*4 + dat[2] + str_size+1 - 
             strlen( (ulong) &dat[0] + dat[3+index-1] )-1;
      dat2 = alloc_force(size);
      p = &dat2[3+dat[1]];
      dat2[0] = size-4;
      dat2[1] = dat[1];
      dat2[2] = size-12-dat[1]*4;
      for (i=1; i <= dat[1]; i++)
      {
        dat2[3+i-1] = p-(strptr) &dat2[0];
        if (i == index) {
          strcpy(p,new);
          p += str_size+1;
        } else {
          strcpy(p,(ulong) &dat[0] + dat[3+i-1]);
          p += strlen(p)+1;
        }
      }
    } else { /* tag existed but not index entry */
      size = 12 + index*4 + dat[2] + str_size + index-dat[1];
      dat2 = alloc_force(size);
      p = &dat2[3+index];
      dat2[0] = size-4;
      dat2[1] = index;
      dat2[2] = size-12-index*4;
      for (i=1; i <= dat[1]; i++)
      {
        dat2[3+i-1] = p-(strptr) &dat2[0];
        strcpy(p,(ulong) &dat[0] + dat[3+i-1]);
        p += strlen(p)+1;
      }
      for (i=dat[1]+1; i <= index; i++)
      {
        dat2[3+i-1] = p-(strptr) &dat2[0];
        if (i == index) {
          strcpy(p,new);
          p += str_size+1;
        } else {
          *p = 0x00;
          p++;
        }
      }
    }
    free_pool(dat);
    tag->ti_Data = dat2;
  }
}

/***----------------------------------------------------------------------***/
/*** special phoneentry tag routines                                      ***/
/***----------------------------------------------------------------------***/

/*
 * Compare tagitem against phonebook entry, if tagitem is not found from entry
 * then profile tagitem is used. (profile = gl_defbbs)
 *
 * tagid  : tag
 * tagdata: data            (data to be compared)
 * entry  : phonebook entry (list to have same tag)
 *
 * return : negative = first is below the second (bigger )
 *              zero = equal
 *          positive = first is above the second (smaller) 
 *          (first = tag, second = tag in taglist)
 */
long CmpEntryTag(ulong tagid, ulong tagdata, struct Info *entry)
{
ulong           td;
struct TagItem *tags=FindTag_info(tagid,entry);

  if (tags) {
    td = tags->ti_Data;
  } else {
    td = GetTag(tagid,&gl_defbbs->tags);
  }
  if ( (tagid >= VALMIN) AND (tagid <= VALMAX) ) {
    if (tagdata  < td) return(-1);
    if (tagdata == td) return( 0);
    if (tagdata  > td) return( 1);
  }
  if ( (tagid >= STRMIN) AND (tagid <= STRMAX) ) {
    return(strcmp((char *)tagdata,(char *)td));
  }
  if ( (tagid >= DATMIN) AND (tagid <= DATMAX) ) {
    return(DatCmp(td,tagdata));
  }
}

/*
 * Get tagitem data from phonebook entry, if tagitem is not found in entry 
 * then get data from profile. (profile = gl_defbbs)
 *
 * tag   : tag
 * entry : phonebook entry (list to have same tag)
 *
 * return: tag data
 *
 */
ulong GetEntryTag(ulong tag, struct Info *entry)
{
struct TagItem *ti=FindTag_info(tag,entry);

  if (ti) {
    return(ti->ti_Data);
  } else {
    return(GetTag(tag,&gl_defbbs->tags));
  }
}

/*
 * Copy phonebook entry tags (selected) to another entry, if tagitem is not
 * found in entry then copy from profile. (profile = gl_defbbs)
 *
 * entry : source entry
 * dest  : target entry
 * ...   : tags to be copied
 *
 * return: -
 *
 */
void CopyEntryTags(struct Info *entry, struct Info *dest, ...)
{
ulong   tag;
va_list ap;

  va_start(ap, dest);
  while( (tag = va_arg(ap,ulong)) != TAG_END )
  {
    AddTag(tag,GetEntryTag(tag,entry),&dest->tags);
  }
  va_end(ap);
}

/***----------------------------------------------------------------------***/
/*** info node routines, most are just 'links' to base tag routines :)    ***/
/***----------------------------------------------------------------------***/

/*
 * Save info list header
 *
 * name  : file name
 * count : how many nodes
 * id    : list id (ie. charset/protocol etc. list...)
 *
 * return: file handle (0 = for failure -> file may exist!)
 *
 */
static bptr SaveHead(strptr name, long count, ulong id)
{
bptr          fh;
struct Header header;

  if (fh = dt_open(name,MODE_WRITE,8192)) {
    header.id    = id;
    header.ver   = INFOVER; /* save _always_ handles only current type */
    header.flags = 0;
    header.size  = 0;
    header.count = count;
    if ( dt_write(fh,&header,sizeof(struct Header)) == sizeof(struct Header) ) {
      return(fh);
    }
    dt_close(fh);
  }
  return(0);
}

/*
 * Save one info node
 *
 * info  : info node
 * fh    : file handle
 *
 * return: TRUE/FALSE
 *
 */
static long SaveOne(struct Info *info, bptr fh)
{
ulong str_size = (info->ln_Name) ? strlen(info->ln_Name) : 0;

  if (dt_write(fh,&str_size,4) == 4) {
    if (str_size) {
      if (dt_write(fh,info->ln_Name,str_size) != str_size) return(FALSE);
    }
    str_size = 0;
    if (dt_write(fh,&str_size,4) != 4) return(FALSE); /* future expansions */
    return(SaveTags(&info->tags,fh));
  }
  return(FALSE);
}

/*
 * Load one info node
 *
 * fh    : file handle
 * info  : info node (MUST BE JUST INITIALIZED!)
 *
 * return: TRUE/FALSE
 *
 */
static long LoadOne(bptr fh, struct Info *info)
{
ulong str_size;

  if (dt_read(fh,&str_size,4) == 4) {
    if (str_size) {
      if (info->ln_Name = alloc_pool(str_size+1)) {
        info->ln_Name[str_size] = 0;
        if (dt_read(fh,info->ln_Name,str_size) != str_size) return(FALSE);
      } else {
        return(FALSE);
      }
    }
    if (dt_read(fh,&str_size,4) == 4) {
      if (str_size) { /* skip possible extension */
        if (Seek(fh,str_size,MODE_CURRENT) == -1) return(FALSE);
      }
      return(LoadTags(fh,&info->tags));
    }
  }
  return(FALSE);
}

/*
 * Create info node
 *
 * -     :
 *
 * return: info node (0 = failure)
 *
 */
struct Info *CreateInfo(void)
{
struct Info *info;

  if (info = alloc_pool(sizeof(struct Info))) {
    info->node.ln_Succ = 0;
    info->node.ln_Pred = 0;
    info->node.ln_Type = NT_USER;
    info->node.ln_Pri  = 0;
    info->node.ln_Name = 0;
    info->tags.tagdata = 0;
    info->tags.tagfree = 0;
    info->store        = 0;
    info->LOCK         = FALSE;
    info->EDIT         = FALSE;
    info->USED         = FALSE;
    info->SELECT       = FALSE;
    info->NODISK       = FALSE;
  }
  return(info);
}

/*
 * Delete info node
 *
 * info  : info node
 *
 * return: -
 *
 */
void DeleteInfo(struct Info *info)
{
  DelTagList(&info->tags);
  free_pool(info->node.ln_Name);
  free_pool(info);
}

/*
 * Remove info node from list and delete it
 *
 * info  : info node
 *
 * return: TRUE/FALSE
 *
 */
long RemoveInfo(struct Info *info)
{
  if (!info->LOCK) { /* we can't remove LOCKED nodes! */
    Remove(info);
    DeleteInfo(info);
    return(TRUE);
  }
  return(FALSE);
}

/*
 * Remove all info nodes from list (and delete them)
 *
 * list  : list
 *
 * return: TRUE/FALSE (TRUE if all nodes were deleted!)
 *
 */
long EmptyInfoList(struct List *list)
{
long        i,
            result=TRUE;
struct Info *info;

  for (i = NodeCount(list)-1; i >= 0; i--)
  {
    info = FindNode_num(list,i);
    if (!RemoveInfo(info)) result = FALSE;
  }
  return(result);
}

/*
 * Load one info node (from file!)
 *
 * info  : info node (MUST BE JUST INITIALIZED!)
 * name  : file name
 * id    : id
 *
 * return: TRUE/FALSE
 *
 */
long LoadInfo(struct Info *info, strptr name, ulong id)
{
bptr          fh;
struct Header header;

  if (fh = dt_open(name,MODE_READ,8192)) {
    if (dt_read(fh,&header,sizeof(struct Header)) == sizeof(struct Header)) {
      if (header.id == id AND header.ver == INFOVER) {
        if (LoadOne(fh,info)) {
          dt_close(fh);
          return(TRUE);
        }
      }
    }
    dt_close(fh);
  }
  return(FALSE);
/*
bptr          fh;
struct Header head;

  if ( (fh = Open(name,MODE_OLDFILE)) == 0 ) return(FALSE);
  SetVBuf(fh,0,BUF_FULL,8192);
  if (FRead(fh,&head,sizeof(struct Header),1) != 1) goto fail;
  if (head.id != id OR head.ver != INFOVER) goto fail;
  if (! LoadOne(fh,info)) goto fail;
  Close(fh);
  return(TRUE);

fail: Close(fh); return(FALSE);
*/
}

/*
 * Save one info node (to file!)
 *
 * info  : info node
 * name  : file name
 * id    : id
 *
 * return: TRUE/FALSE
 *
 */
long SaveInfo(struct Info *info, strptr name, ulong id)
{
bptr fh;

  if (fh = SaveHead(name,1,id)) {
    if (SaveOne(info,fh)) {
      dt_close(fh);
      return(TRUE);
    } else {
      dt_close(fh);
      DeleteFile(name);
    }
  }
  return(FALSE);
/*
bptr fh;

  if ( (fh = SaveHead(name,1,id)) == 0 ) return(FALSE);
  if (SaveOne(info,fh) == FALSE) {
    Close(fh); DeleteFile(name); return(FALSE);
  }
  Close(fh);
  return(TRUE);
*/
}

/*
 * Load complete info list
 *
 * list  : list
 * name  : file name
 * id    : id
 *
 * return: TRUE/FALSE
 *
 */
long LoadInfoList(struct List *list, strptr name, ulong id)
{
bptr           fh;
struct Info   *info;
struct Header  header;

  if (fh = dt_open(name,MODE_READ,8192)) {
    if (dt_read(fh,&header,sizeof(struct Header)) == sizeof(struct Header)) {
      if (header.id == id AND header.ver == INFOVER) {

        while ( --header.count >= 0 )
        {
          if (info = CreateInfo()) {
            if (LoadOne(fh,info)) {
              if ( info->ln_Name ) {
                if (FindName(list,info->ln_Name) == 0) {
                  AddTail(list,info);
                } else {
                  DeleteInfo(info);
                }
              } else {
                AddTail(list,info);
              }
            } else {
              DeleteInfo(info);
              dt_close(fh);
              return(FALSE);
            }
          } else {
            dt_close(fh);
            return(FALSE);
          }
        }

        dt_close(fh);
        return(TRUE);
      }
    }
    dt_close(fh);
  }
  return(FALSE);
/*
bptr           fh;
struct Info   *info=0;
struct Header  header;

  if ( (fh = dt_pen(fname,MODE_OLDFILE)) == 0 ) goto fail;
	if (FRead(fh,&head,sizeof(struct Header),1) != 1) goto fail;
	if (head.id != id OR head.ver != INFOVER) goto fail;

	while ( --head.count >= 0 )
	{
		if ( (info = CreateInfo()) == 0 ) goto fail;
		if (! LoadOne(fh,info)) goto fail;
		if ( info->ln_Name ) {
			if (FindName(list,info->ln_Name) == 0) {
				AddTail(list,info);
			} else {
				DeleteInfo(info);
			}
		} else {
			AddTail(list,info);
		}
	}
	Close(fh);
	return(TRUE);

fail:

	if (info)	DeleteInfo(info);
	if (fh)		Close(fh);
	return(FALSE);
*/
}

/*
 * Save complete info list
 *
 * list  : list
 * name  : file name
 * id    : id
 *
 * return: TRUE/FALSE
 *
 */
long SaveInfoList(struct List *list, strptr name, ulong id)
{
bptr         fh;
long         i=0;
struct Info *info;

  /* count nodes */
  if (!IsListEmpty(list)) {
    info = list->lh_Head;
    while (info)
    {
      if (!info->NODISK) i++;
      info = GetSucc(info);
    }
  }
  /* save nodes */
  if (fh = SaveHead(name,i,id)) {
    if (!IsListEmpty(list)) {
      info = list->lh_Head;
      while (info)
      {
        if (!info->NODISK) {
          if (!SaveOne(info,fh)) {
            dt_close(fh);
            return(FALSE);
          }
        }
        info = GetSucc(info);
      }
    }
    dt_close(fh);
    return(TRUE);
  }
  return(FALSE);
}

void AddInfoTag(ulong tag, ulong data, struct Info *info)
{
  AddTag(tag,data,&info->tags);
}

long CmpInfoTag(ulong tag, ulong data, struct Info *info)
{
  return(CmpTag(tag,data,&info->tags));
}

void DelInfoTag(ulong tag, struct Info *info)
{
  DelTag(tag,&info->tags);
}

struct TagItem *FindInfoTag(ulong tagid, struct Info *info)
{
  return(FindTag(tagid,&info->tags));
}

ulong GetInfoTag(ulong tag, struct Info *info)
{
  return(GetTag(tag,&info->tags));
}

long LookInfoTag(ulong tag, struct Info *info)
{
  return(LookTag(tag,&info->tags));
}

void AddInfoTags(struct Info *info, ... )
{
va_list	ap;
ulong		tag;

	va_start(ap, info);
	while( (tag = va_arg(ap,ulong)) != TAG_END )
	{
		AddInfoTag(tag,va_arg(ap,ulong),info);
	}
	va_end(ap);
}

long CmpInfoTags(struct Info *info, ... )
{
va_list	ap;
ulong		tag;

	va_start(ap, info);
	while( (tag = va_arg(ap,ulong)) != TAG_END )
	{
		if ( CmpInfoTag(tag,va_arg(ap,ulong),info) ) {
			va_end(ap);
			return(1);
		}
	}
	va_end(ap);
	return(0);
}

void DelInfoTags(struct Info *info, ... )
{
va_list ap;
ulong   tag;

  va_start(ap, info);
  while( (tag = va_arg(ap,ulong)) != TAG_END )
  {
    DelInfoTag(tag,info);
  }
  va_end(ap);
}

void CopyInfoTags(struct Info *source, struct Info *dest, ...)
{
va_list ap;
ulong   tag;

  va_start(ap, dest);
  while( (tag = va_arg(ap,ulong)) != TAG_END )
  {
    AddInfoTag(tag,GetInfoTag(tag,source),dest);
  }
  va_end(ap);
}
