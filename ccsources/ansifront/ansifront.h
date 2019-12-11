extern char *toknames[];

#define NULL 0

#define T_IDENT  'A'
#define T_NUMBER 'N'
#define T_STRING 'S'
#define T_INTNUM 'I'
#define T_CHRLIT 'C'

#define T_LNGNUM 'O'

#define TB_ATYPE 257
#define T_STRUCT 257
#define T_UNION  258
#define T_ENUM   259
#define TB_TYPE  260
#define TB_BTYPE 260
#define T_INT    260
#define T_CHAR   261
#define T_FLOAT  262
#define T_DOUBLE 263
#define T_VOID   264
#define TE_BTYPE 265
#define T_UNSIGN 265
#define T_SIGNED 266
#define T_SHORT  267
#define T_LONG   268
#define TB_SCLAS 269
#define T_STATIC 269
#define T_TYPEDE 270
#define T_EXTERN 271
#define T_AUTO   272
#define T_REGIST 273
#define TE_SCLAS 274
#define T_CONST  274
#define T_DIRECT 275
#define TE_TYPE  276
#define T_TDNAME 276
#define TE_ATYPE 277
#define T_IF     277
#define T_ELSE   278
#define T_DO     279
#define T_FOR    280
#define T_WHILE  281
#define T_SWITCH 282
#define T_BREAK  283
#define T_CONTIN 284
#define T_RETURN 285
#define T_CASE   286
#define T_DEFAUL 287
#define T_GOTO   288
#define TB_ASNOP 289
#define T_EQUAL  289
#define T_PLSEQ  290
#define T_MINEQ  291
#define T_STAREQ 292
#define T_DIVEQ  293
#define T_MODEQ  294
#define T_SHREQ  295
#define T_SHLEQ  296
#define T_OREQ   297
#define T_ANDEQ  298
#define T_XOREQ  299
#define TE_ASNOP 300
#define T_OROR   300
#define T_ANDAND 301
#define T_EQEQ   302
#define T_NOTEQ  303
#define TB_COMP  304
#define T_LT     304
#define T_LTEQ   305
#define T_GTEQ   306
#define T_GT     307
#define TE_COMP  308
#define T_SHIFTL 308
#define T_SHIFTR 309
#define T_PLPL   310
#define T_MIMI   311
#define T_SIZEOF 312
#define T_DASHGT 313
#define T_ELIPSE 314

#define TS_NONE   0
#define TS_PARAM -1

#define IsTYPE(x)   ((x)>=TB_TYPE &&  (x)<TE_TYPE)
#define IsSCLASS(x) ((x)>=TB_SCLAS && (x)<TE_SCLAS)
#define IsASNOP(x)  ((x)>=TB_ASNOP && (x)<TE_ASNOP)
#define IsCOMP(x)   ((x)>=TB_COMP  && (x)<TE_COMP)
#define IsPREOP(x)  ((x)=='!' || (x)=='~' || (x)==T_PLPL || /*
*/ (x)==T_MIMI || (x)=='-' || (x)=='*' || (x)=='&' || (x)==T_SIZEOF)

#define NewTQ()    ((Type *)StkAlloc(sizeof(Type)))
#define NewTSpec() ((TSpec *)StkAlloc(sizeof(TSpec)))
#define NewPLst()  ((PLst *)StkAlloc(sizeof(PLst)))
#define NewTLLst() ((TLLst *)StkAlloc(sizeof(TLLst)))
#define NewILst()  ((ILst *)StkAlloc(sizeof(ILst)))
#define NewVDecl() ((VDecl *)StkAlloc(sizeof(VDecl)))
#define NewSDef()  ((StrctDef *)StkAlloc(sizeof(StrctDef)))
#define NewSRef()  ((StrctRef *)StkAlloc(sizeof(StrctRef)))

#define SR_REF   1
#define SR_DEF   2
#define SR_RANDD 3

#define SD_STRUC 1
#define SD_UNION 2

typedef struct type_s {
  char           tl_op;
  union {
    unsigned       tld_num;
    struct plst_s *tld_plst;
    char           tld_cons;
  } tl_dat;
  struct type_s *tl_next;
} Type;

typedef struct {
  union {
    struct {
      char ts_sign;
      char ts_len;
    } ts_siln;
    struct strctref *ts_sref;
    struct vdecl_s  *ts_tdef;
    char            *ts_iden;
  } ts_xdat;
  char ts_type;
  char ts_sclas;
  char ts_const;
  char ts_direc;
} TSpec;

typedef struct vdecl_s {
  char           *vd_var;
  Type           *vd_type;
  TSpec          *vd_tspec;
  struct vdecl_s *vd_prev;
} VDecl;

typedef struct strctdef {
  char            *sd_name;
  VDecl           *sd_flst;
  char             sd_type;
  struct strctdef *sd_prev;
} StrctDef;

typedef struct strctref {
  char    sr_dorr;
  union {
    VDecl    *sr_def;
    StrctDef *sr_ref;
  } sr_drdat;
} StrctRef;

typedef struct {
  int exb_op;
  char *exb_t1;
  char *exb_t2;
} BinExpr;

typedef struct {
  int exu_op;
  char *exu_t;
} UnryExpr;

typedef struct {
  int ext_op;
  char *ext_t1;
  char *ext_t2;
  char *ext_t3;
} TerExpr;

typedef struct explst_s {
  char *el_exp;
  struct explst_s *el_next;
} ExpLst;

typedef struct {
  VDecl val_type;
  char *val_expr;
  char  val_islv;
  char  val_isce;
} Value;

typedef struct tllst_s {
  Type  *tll_type;
  TSpec  tll_tspec;
  struct tllst_s *tll_next;
} TLLst;

typedef struct ilst_s {
  char *il_ident;
  struct ilst_s *il_next;
} ILst;

typedef struct plst_s {
  TLLst *pl_type;
  ILst  *pl_ident;
} PLst;
  
typedef struct {
  VDecl    *s_varlst;
  StrctDef *s_stlst;
  char     *s_mainsp;
  VDecl    *s_varscp; /* variable scope */
  StrctDef *s_tagscp; /* tag scope */
} State;

extern char  *curident;
extern char  *nexident;
extern int    nexttok;
extern int    curtok;
extern VDecl *varlst;
extern VDecl  intdecl;
extern TSpec  flttspec;
extern TSpec  chrtspec;
extern TSpec  inttspec;
extern TSpec  lngtspec;
extern TSpec  undtspec;
extern Type   strtype;
extern char   arrysiz[6];
extern int    curline;
extern char   curfile[];
extern char   progfile[];
extern int    tempsize;
extern VDecl *varscope;
extern TSpec  cvdtspec;
extern VDecl  cvpdecl;
extern VDecl *UnfyNTyp();

extern int   IsATYPE();
extern int   RdToken();
extern       InitRdTk();
extern       Error();
extern int   PeekTok();
extern       PrintTok();
extern       WrCurTyp();
extern       Expect();
extern       RdExpr();
extern       WrExpr();
extern       WrCurExp();
extern       Error();
extern char *KeepIden();
extern char  outstrm;
extern char  warn;
extern Value curvalue;
extern       RdPLst();
static WrPLst(PLst*,VDecl*);
static RdInit(Type*,TSpec*,char);
extern       RdFDef();
extern       RWTSpec();
extern int   IsPtr();
extern int   IsNum();
extern       WrPrExpr();
extern       CpyValue();
extern       WrExpr();
extern       RdPrExpr();
extern           RdBExpr();
extern           MakRVal();
extern           ExpsType();
extern           RdGDecl();
extern           RdDeclLs();
extern BinExpr  *NewBExpr();
extern           RdVDecl();
extern VDecl    *SrchDecl();
extern VDecl    *CanAdd();
extern UnryExpr *NewUExpr(int,char*);
extern long   ELngVal();
extern double ENumVal();
extern int CanInit(VDecl*,VDecl*,char *);
extern int IsSorU(Type*,TSpec*);
extern int SizeOf(VDecl*);
extern RdIDecl(VDecl*);
extern WrOSPLst(PLst*,VDecl*);
extern WrVDecl(VDecl*);
extern int EIntVal(char*);
extern int HasDef(TSpec*);
extern WrXDecl(VDecl*,int);
extern Wr04x(int);
extern WrChar(char);
extern WrUInt(unsigned);
extern WrTSpec(TSpec*);
extern WrTDecl(VDecl*);
extern WrStRef(StrctRef*,int);
extern int IsSURef(TSpec*);
extern WrEncode(char*);
extern hkDef(VDecl*);
extern char *StkAlloc(int);
extern Skip(int);
extern AError(char*);
extern WrVarNam(char*,int);
extern WrHash(char*);
extern WrDouble(double);
extern WrInt(int);
extern int EIntVal(char*);
extern WrLong(long);
extern WrDelStr(char*,char);
extern int IsNumTyp(int);
extern WrStr(char*);
extern ChkDef(VDecl*);
extern int IsSUPtr(VDecl*);
extern InvldOp(VDecl*,int,VDecl*);
extern WrLXDecl(VDecl*);
extern WrPoExpr(char*);
extern int IsConst(VDecl*);
extern MakConst(VDecl*,int);
extern int IsSamTyp(VDecl*,VDecl*);
extern int IsZero(char*);
extern WrAsExpr(char*);
extern int StrLen(char*);
extern WrEnIden(char*,int);
extern char *TSAlloc(int);
extern RdTSpec(TSpec*);
extern WrIDecl(VDecl *,int);
extern StoreNam(VDecl*);
extern int CharUVal(char**);
extern StoreStt(State*);
extern RetrvStt(State*);
extern WrTQual(TSpec *);
extern WrStTag(StrctRef*);
extern WrXLIDec(VDecl*);
extern WrXRIDec(VDecl*,int,VDecl*);
extern ATSError(TSpec*);
extern SWr04x(char*,int);
extern int IsProto(PLst*);
extern WrReserv(int);
extern Wr03x(int);
extern WrEnIden(char*,int);
extern int TokIsInt(int);
extern int TokIsNum(int);
extern ATSError(TSpec*);
extern SWr04x(char*,int);
extern EAsInt(char*,int);
extern EAsLng(char*,long);
extern int IsSamPtr(VDecl*,VDecl*);
extern int SamUQTyp(VDecl*,VDecl*);
extern int NeedSize(VDecl*);

#ifdef CoCo
extern int atoi(char*);
extern int read(int, char*, int);
extern long atol(char*);
#endif
