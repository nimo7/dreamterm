/*** mega routine for handling commands (actions, methods whatever) ***/
ulong com(ulong, ...);

/*** main routines (some of them should be in somewhere else...) ***/
void  MainIDCMP(void);
void  SetUp(struct Info *);
void  SetUpTag(ulong, ulong);
void  SetUpTags(long, ...);
void  LoadPick(void);
void  SavePick(void);
void  EndProg(strptr);

/*** menu routines ***/
void Menu_build(void);
void Menu_free(void);
void Menu_fresh(void);
void Menu_handle(uword *);
struct MenuItem	*IA(ulong);

/*** 'object kind' interface for windows ***/
ulong Charset (ulong,ulong);
ulong Crypt   (ulong,ulong,ulong);
ulong Dialer  (ulong,ulong);
ulong Drop    (ulong,ulong);
ulong Entry   (ulong,ulong);
ulong Fileid  (ulong,ulong);
ulong Macro   (ulong,ulong);
ulong Phone   (ulong,ulong);
ulong Review  (ulong,ulong);
ulong Show    (ulong,ulong);
ulong Toolbar (ulong,ulong);
ulong Transfer(ulong,ulong);
ulong Xem     (ulong,ulong);

bool XemPrefMode(void);
void XemUserMode(void);

/*** own classes ***/
struct IClass *AddClass_cps     (void);
struct IClass *AddClass_protocol(void);
struct IClass *AddClass_revarea (void);
struct IClass *AddClass_review  (void);
void           DelClass_cps     (void);
void           DelClass_protocol(void);
void           DelClass_revarea (void);
void           DelClass_review  (void);

/*** async request & other 'gui' routines ***/
void  Warning        (strptr);
void  Update_windows (void);
void  Ask_dir_tag    (strptr, ulong, struct Info *);
void  Ask_str_tag    (strptr, ulong, struct Info *);
void  Ask_val_tag    (strptr, ulong, ulong, ulong, struct Info *);
void  Ask_dir_cur    (strptr, ulong);
void  Ask_str_cur    (strptr, ulong);
void  Ask_val_cur    (strptr, ulong, ulong, ulong);
bool  Ask_list       (void (*)(void), strptr, strptr *,ulong *);
long  Ask_list_done  (void);
bool  Ask_string     (void (*)(void), strptr, strptr );
bool  Ask_string_sync(strptr, strptr );
long  Ask_string_done(void);
bool  Ask_slider     (void (*)(void), strptr, long, long, long *);
long  Ask_slider_done(void);
bool  Ask_file       (struct Window *, void (*)(void), strptr, strptr, strptr );
bool  Ask_files      (struct Window *, void (*)(void), strptr, strptr, strptr, strptr * );
bool  Ask_dir        (struct Window *, void (*)(void), strptr, strptr );
long  Ask_file_done  (void);
long  Ask_files_done (void);
long  Ask_dir_done   (void);
bool  Ask_font       (struct Window *, void (*)(void), strptr, strptr );
long  Ask_font_done  (void);
bool  Ask_screenmode (struct Window *, void (*)(void), struct ScreenModeRequester * );
long  Ask_screenmode_done(void);

/*** mui routines ***/
ulong __stdargs DoSuperNew(struct IClass *, Object *, ulong,...);
ulong xget         (Object *, ulong);
long  MUI_nodenum  (aptr, struct Node *);
bool  MUI_selnode  (aptr, struct Node *);
bool  MUI_nnselnode(aptr, struct Node *);
bool  MUI_selname  (aptr, char *);
bool  MUI_nnselname(aptr, char *);
bool  InfoListToMui(struct List *, aptr, ulong);
void  multiset     (ulong , ...);
void  nnmultiset   (ulong , ...);
void  MultiIDNotify(ulong , ulong, ...);
extern struct Hook NodeDisplay;
extern struct Hook NodeCompare;
extern struct Hook NodeConstruct;
extern struct Hook NodeDestruct;

/*** pooled memory allocation ***/
aptr __asm alloc_pool (register __d0 ulong);
aptr __asm alloc_force(register __d0 ulong);
void __asm free_pool  (register __a0 aptr);
#define AllocMyPooled(a) alloc_pool(a)
#define FreeMyPooled(a)  free_pool(a)

/*** file operations ***/
long dt_close    (bptr);
bptr dt_open     (strptr, long   , long);
long dt_read     (bptr  , ubyte *, long);
long dt_readchar (bptr  );
bptr dt_seek     (bptr  , long   , long);
long dt_write    (bptr  , ubyte *, long);
long dt_writechar(bptr  , ubyte  );

/*** locale ***/
strptr LOC(ulong);
void   LocalizeMenu(struct NewMenu *, long);
void   LocalizeList(strptr *, long);

/*** datatype picture loading ***/
struct Picture *AllocPicture(void);
void            FreePicture (struct Picture *);
bool            IsDataTypes (ubyte *, ubyte *, long);
long            LoadPicture (ubyte *, struct Picture *, ulong);

/*** tag routines (high level) ***/
strptr          TagName      (ulong);
strptr          TagData2Str  (ulong, ulong);
void            FreeNameList (strptr *);
strptr *        MakeNameList (ulong *);
/*** tag routines (base) ***/
void            AddTag       (ulong, ulong, struct TagBase *);
long            CmpTag       (ulong, ulong, struct TagBase *);
void            CopyTags     (struct TagBase *, struct TagBase *);
void            DelTag       (ulong, struct TagBase *);
void            DelTagList   (struct TagBase *);
struct TagItem *FindTag      (ulong, struct TagBase *);
long            LookTag      (ulong, struct TagBase *);
long            LoadTags     (bptr, struct TagBase *);
long            SaveTags     (struct TagBase *, BPTR);
long            SaveDatTag   (ulong, struct TagBase *, char *);
ulong *         CreateDat    (ulong tagid);
ulong           CountDat     (struct TagItem *);
void            DelDat       (struct TagItem *, ulong);
ulong           GetDat       (struct TagItem *, ulong);
void            SetDat       (struct TagItem *, ulong, ulong);
/*** tag routines (phonebook entry) ***/
long            CmpEntryTag  (ulong, ulong, struct Info *);
ulong	          GetEntryTag  (ulong, struct Info *);
void	          CopyEntryTags(struct Info *, struct Info *, ...);

/*** misc routines ***/
bool  StrRep(strptr *, strptr);
long  MyObtainBestPen(aptr,ulong,ulong,ulong,long);
long  MyFindColor(aptr,ulong,ulong,ulong,ulong,long);
void  TempName(strptr);
long  FileSize(strptr);
long  CloneFile(strptr,strptr);

/*** serial handling & string conversion ***/
void dprintf     (strptr, strptr, ...);
void dprintf_user(strptr, strptr);
void Send_insert (strptr, long, long);
long Send_buffer (void);
void Clear_buffer(void);
#define Send_string(a)      Send_insert(a,strlen(a),0)
#define Send_translate(a,b) Send_insert(a,b,1)
#define Send_raw(a,b)       Send_insert(a,b,2)
/*
void Send_handle  (void);
void Conv_mystring(strptr, strptr);
#define Str_string(a,b)     Conv_mystring(a,b)
*/

/*** info node routines ***/
struct Info *   CreateInfo   (void);
void            DeleteInfo   (struct Info *);
long            RemoveInfo   (struct Info *);
long            EmptyInfoList(struct List *);
long            LoadInfo     (struct Info *, strptr, ulong);
long            SaveInfo     (struct Info *, strptr, ulong);
long            LoadInfoList (struct List *, strptr, ulong);
long            SaveInfoList (struct List *, strptr, ulong);
void	          AddInfoTag   (ulong, ulong, struct Info *);
struct TagItem *FindInfoTag  (ulong, struct Info *);
long            CmpInfoTag   (ulong, ulong, struct Info *);
void	          DelInfoTag   (ulong, struct Info *);
ulong	          GetInfoTag   (ulong, struct Info *);
long            LookInfoTag  (ulong, struct Info *);
void	          AddInfoTags  (struct Info *, ... );
long            CmpInfoTags  (struct Info *, ... );
void	          DelInfoTags  (struct Info *, ... );
void	          CopyInfoTags (struct Info *, struct Info *, ...);
/*** new names ***/
#define AddTag_info(a,b,c)  AddInfoTag(a,b,c)
#define AddTags_info(a,b)   AddInfoTags(a,b)
#define DelTag_info(a,b)    DelInfoTag(a,b)
#define DelTags_info(a,b)   DelInfoTags(a,b)
#define FindTag_info(a,b)   FindInfoTag(a,b)
#define GetTag_info(a,b)    GetInfoTag(a,b)
#define LookTag_info(a,b)   LookInfoTag(a,b)

/*** asm routines ***/
ulong	__asm GetTag      (REG __d0 ulong, REG __a0 struct TagBase *);
void	__asm MergePF     (REG __a0 strptr, REG __a1 strptr, REG __a2 strptr, REG __d0 WORD);
long	__asm Translate   (REG __a0 strptr, REG __a1 strptr, REG __a2 aptr);
long	__asm RawTranslate(REG __a0 strptr, REG __a1 strptr, REG __a2 aptr, REG __d0 WORD);
long	__asm Expand      (REG __a0 strptr, REG __a1 strptr, REG __a2 aptr, REG __d0 WORD, REG __d1 UWORD);
long	__asm FindPause   (REG __a0 strptr);
void	__asm PrepFlow    (REG __a0 struct FlowString *);
void	__asm HuntFlow    (REG __a0 struct FlowString *, REG __a1 strptr, REG __d0 ulong);
bool	__asm	CmpPhone    (REG __a0 aptr, REG __a1 aptr);
long  __asm	DatCmp      (REG __a0 aptr, REG __a1 aptr);
aptr	__asm AllocVecPooled(REG __a0 aptr, REG __d0 ulong);
void	__asm FreeVecPooled(REG __a0 aptr, REG __a1 aptr);

/*
void	  __asm ResetRev(REG __a1 struct RevData *);
void	  __asm ConvToRev(REG __a0 ubyte *, REG __a1 struct RevData *, REG __d0 long);
ubyte * __asm RevLine_find(REG __d0 long   , REG __a1 struct RevData *);
ubyte * __asm RevLine_next(REG __a0 ubyte *, REG __a1 struct RevData *);
ubyte * __asm RevLine_prev(REG __a0 ubyte *, REG __a1 struct RevData *);
long	  __asm RevLine_num (REG __a0 ubyte *, REG __a1 struct RevData *);
*/
ulong   __asm RetIndex    (REG __a0 ulong *, REG __d0 ulong);

ulong   __asm TempDoSuperMethodA(REG __a0 aptr ,REG __a1 aptr , REG __a2 aptr);
ulong   __asm TempDoMethodA     (REG __a0 aptr, REG __a1 aptr);
void	  __asm TempTimeDelay     (REG __d0 ulong,REG __d1 ulong, REG __d2 ulong);
