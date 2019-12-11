#ifndef CoCo
#include "/usr/include/ctype.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#define writeln write
#else
#include <ctype.h>
#endif
#include "ansifront.h"

static char outline[80];
static char outlnlen = 0;
static char mainstk[11*1024+512];
static char tmpstk[512];
static char *mainsp=mainstk;
static char *tmpsp=tmpstk;
static char exitstat = 0;
static VDecl rettype;
char fldname = 0;
StrctDef *tagscope=NULL;
VDecl *varscope=NULL;
char outstrm;
char warn;

static StrctDef *strctlst = NULL;

VDecl    *varlst   = NULL;
VDecl     intdecl;
int       curline = 1;
char      curfile[64] = {'n','o','n','a','m','e','.','c','\0'};
char      progfile[30] = {'n','o','n','a','m','e','_','c','\0'};
int       tempsize;

static int HexDigit(int);
static RdStRef(StrctRef*);
static RdCStmt();
extern int DumpLine();

int TokIsInt(t)
int t;
{
  return t==T_INTNUM || t==T_LNGNUM;
}

int TokIsNum(t)
int t;
{
  return t==T_NUMBER || TokIsInt(t);
}

WrLn()
{
  if (outstrm==1) {
    outline[outlnlen] = '\n';
    writeln(1,outline,outlnlen+1);
    outlnlen = 0;
  }
  else
    writeln(outstrm,"\n",1);
}

WrStr(s)
char *s;
{
  if (outstrm==1) {
    int len = strlen(s);
    if (s[len-1]=='\n')
      --len;
    if (outlnlen+len>=80)
      WrLn();
    memcpy(outline+outlnlen,s,len);
    outlnlen += len;
    if (s[len]=='\n')
      WrLn();
  }
  else
    writeln(outstrm,s,strlen(s));
}

WrReserv(n)
int n;
{
  if (outlnlen+n>=80)
    WrLn();
}

WrEnIden(prefix,addr)
char *prefix;
int addr;
{
  if (outlnlen+8>=80)
    WrLn();
  WrStr(prefix);
  Wr04x(addr);
}


WrDelStr(s,delim)
char *s;
char delim;
{
  if (outstrm==1) {
    int slen = strlen(s);
    if (outlnlen+slen+2>=80)
      WrLn();
    outline[outlnlen++] = delim;
    memcpy(&outline[outlnlen],s,slen);
    outlnlen += slen;
    outline[outlnlen++] = delim;
  }
  else {
    WrChar(delim);
    WrStr(s);
    WrChar(delim);
  }
}

WrInt(i)
int i;
{
  if (i<0) {
    WrChar('-');
    i = -i;
  }
  WrUInt(i);
}

WrDouble(v)
double v;
{
  int e = 0;
  int d = 13;
  int l = 14;
  int iv;
  double n;
  if (outlnlen+21>=80)
    WrLn();
  if (v!=0.0) {
    if (v<0.0) {
      v = -v;
      WrChar('-');
    }
    while (v>=10.0) {
      ++e;
      v /= 10.0;
    }
    while ((int)v==0) {
      --e;
      v *= 10.0;
    }
    v+=1e-14;
    if (v>=10.0) {
      ++e;
      v/=10.0;
    }
  }
  WrChar((iv=(int)v)+'0');
  WrChar('.');
  while (d>0 && v!=0.0) {
    v = (v - iv)*10.0;
    if ((iv=(int)v)>0) {
      for (;;) {
        --l;
        if (l==d)
          break;
        WrChar('0');
      }
      WrChar('0'+(int)iv);
    }
    --d;
  }
  WrChar('e');
  WrInt(e);
}

static SWrHexB(buf,i)
char *buf;
int i;
{
  buf[0] = HexDigit((i>>4) & 0xf);
  buf[1] = HexDigit(i & 0xf);
}

WrLong(i)
long i;
{
  char buf[12];
  buf[0]='0';
  buf[1]='x';
/*
  WZ - Assumes big endian - this wont work on little endian
  SWrHexB(buf+2,*(char *)&i);
  SWrHexB(buf+4,*((char *)&i + 1));
  SWrHexB(buf+6,*((char *)&i + 2));
  SWrHexB(buf+8,*((char *)&i + 3));
*/
  SWrHexB(buf+2,(int)(i>>23));
  SWrHexB(buf+4,(int)((i>>16)&0xff));
  SWrHexB(buf+6,(int)((i>>8)&0x0ff));
  SWrHexB(buf+8,(int)(i&0xff));
  buf[10] = 'L';
  buf[11] = '\0';
  WrStr(buf);
}

WrUInt(i)
unsigned i;
{
  char buf[6];
  char *bp = buf;
  unsigned place = 10000;
  while (place>1 && i<place)
    place /= 10;
  while (place>1) {
    int count = 0;
    while (i>=place) {
      i-=place;
      ++count;
    }
    *bp++ = count+'0';
    place /= 10;
  }
  *bp++ = i+'0';
  *bp = '\0';
  WrStr(buf);
}

static int HexDigit(n)
int n;
{
  return n+((n<10) ? '0' : 'a'-10);
}

SWr04x(buf,i)
char *buf;
int i;
{
  buf[3] = HexDigit(i & 0xf);
  buf[2] = HexDigit((i>>=4) & 0xf);
  buf[1] = HexDigit((i>>=4) & 0xf);
  buf[0] = HexDigit((i>>4) & 0xf);
  buf[4] = '\0';
}

Wr04x(i)
int i;
{
  char buf[6];
  SWr04x(buf,i);
  WrStr(buf);
}

Wr03x(i)
int i;
{
  char buf[5];
  buf[2] = HexDigit(i & 0xf);
  buf[1] = HexDigit((i>>=4) & 0xf);
  buf[0] = HexDigit((i>>4) & 0xf);
  buf[3] = '\0';
  WrStr(buf);
}

WrChar(c)
char c;
{
  if (outstrm==1) {
    if (c=='\n')
      WrLn();
    else {
      if (outlnlen+1>=80)
        WrLn();
      outline[outlnlen++] = c;
    }
  }
  else 
    writeln(outstrm,&c,1);
}

char *StkAlloc(n)
int n;
{
  if (n>255 || n<1) {
    Error("Bad main allocation");
    exit(1);
  }
  if (mainsp+n>mainstk+sizeof(mainstk))
    Error("Out of stack space");
  {
    char *p = mainsp;
    mainsp += n;
    return p;
  }
}

char *TSAlloc(n)
int n;
{
  if (n>255 || n<1) {
    Error("Bad temp allocation");
    exit(1);
  }
  if (tmpsp+n>tmpstk+sizeof(tmpstk))
    Error("Out of stack space");
  {
    char *p = tmpsp;
    tmpsp += n;
    return p;
  }
}

TSReset()
{
  tmpsp = tmpstk;
}

char *KeepIden()
{
  int len = strlen(curident);
  return (char *)memcpy(StkAlloc(len+1),curident,len+1);
}

char *KeepTI()
{
  int len = strlen(curident);
  return (char *)memcpy(TSAlloc(len+1),curident,len+1);
}

static WrErr(errtype)
char *errtype;
{
  outstrm = 2;
  WrStr(errtype);
  WrStr(" in ");
  WrStr(curfile);
  WrStr(" line ");
  WrInt(curline);
  WrStr(": ");
}

BWarning()
{
  WrErr("Warning");
}

BError()
{
  exitstat = 1;
  WrErr("Error");
}

AError(x)
char *x;
{
  WrStr(x);
}

ATSError(tspec)
TSpec *tspec;
{
  char sclas = tspec->ts_sclas;
  tspec->ts_sclas = TS_NONE;
  WrTSpec(tspec);
  tspec->ts_sclas = sclas;
}

ATError(decl)
VDecl *decl;
{
  char sclas = decl->vd_tspec->ts_sclas;
  WrChar('(');
  decl->vd_tspec->ts_sclas = TS_NONE;
  WrTDecl(decl);
  decl->vd_tspec->ts_sclas = sclas;
  WrChar(')');
}

EError()
{
  WrLn();
  outstrm = 1;
}

Error(x)
char *x;
{
  BError();
  AError(x);
  EError();
}

PrintTok(x)
int x;
{
  if (x<256) {
    if (x==T_IDENT || TokIsNum(x)) {
      WrStr(curident);
      WrChar(' ');
    }
    else if (x==T_CHRLIT)
      WrDelStr(curident,'\'');
    else if (x=='S')
      WrDelStr(curident,'"');
    else if (x>=32)
      WrChar(x);
    else
      WrStr("[[Invalid Token]]");
  }
  else {
    WrStr(toknames[x-256]);
    WrChar(' ');
  }
}

static int lasttok=0;

static WrTokTyp(c)
int c;
{
  if (c<256) {
    if (c==T_STRING)
      WrStr("string");
    else if (TokIsNum(c))
      WrStr("number");
    else if (c==T_IDENT)
      WrStr("identifier");
    else if (c==T_CHAR)
      WrStr("character");
    else if (c==-1)
      WrStr("end of file");
    else {
      WrChar('\'');
      WrChar(c);
      WrChar('\'');
    }
  }
  else
    WrStr(toknames[c-256]);
}

static ExpError(c)
int c;
{
  BError();
  AError("Expecting ");
  WrTokTyp(c);
  WrStr(" found ");
  WrTokTyp(curtok);
  EError();
  exit(1);
}

Skip(c)
int c;
{
  if (curtok!=c)
    ExpError(c);
  else
    SkipTok();
}

Expect(c)
int c;
{
  if (curtok!=c)
    ExpError(c);
  else {
    PrintTok(curtok);
    SkipTok();
  }
}

InvldOp(vdecl1,optok,vdecl2)
VDecl *vdecl1;
int optok;
VDecl *vdecl2;
{
  BError();
  AError("Invalid operation: ");
  ATError(vdecl1);
  WrChar(' ');
  PrintTok(optok);
  WrChar(' ');
  ATError(vdecl2);
  EError();
}

static Type **RdLIDecl();

VDecl *TDName(ident)
char *ident;
{
  VDecl *typep = varlst;
  while (typep!=NULL) {
    if (strcmp(typep->vd_var,ident)==0)
      break;
    typep = typep->vd_prev;
  }
  if (typep!=NULL && typep->vd_tspec->ts_sclas!=T_TYPEDE-256)
    return NULL;
  return typep;
}

int IsATYPE(tok,ident)
int tok;
char *ident;
{
  return ((tok>=TB_ATYPE && tok<TE_ATYPE) ||
          (tok==T_IDENT && TDName(ident)!=NULL));
}

ErrMSpec(specstr)
char *specstr;
{
  BError();
  WrStr("multiple ");
  WrStr(specstr);
  WrStr(" specifiers");
  EError();
}

RdTSpec(ts)
TSpec *ts;
{
  int nextt;
  char sign    = TS_NONE;
  char len     = TS_NONE;
  char type    = TS_NONE;
  char sclass  = TS_NONE;
  char isconst = 0;
  char isdirec = 0;
  for (;;) {
    if (IsTYPE(curtok)) {
      nextt = curtok;
      SkipTok();
      if (nextt==T_UNSIGN || nextt==T_SIGNED) {
        if (sign!=TS_NONE)
          ErrMSpec("sign");
        else
          sign = nextt-256;
      }
      else if (nextt==T_LONG || nextt==T_SHORT) {
        if (len!=TS_NONE)
          ErrMSpec("length");
        else
          len = nextt-256;
      }
      else if (nextt>=TB_BTYPE && nextt<TE_BTYPE) {
        if (type!=TS_NONE)
          ErrMSpec("type");
        else
          type = nextt-256;
      }
      else if (IsSCLASS(nextt)) {
        if (sclass!=TS_NONE)
          ErrMSpec("storage class");
        else
          sclass = nextt-256;
      }
      else if (nextt==T_CONST) {
        if (isconst)
          ErrMSpec("const");
        isconst = 1;
      }
      else if (nextt==T_DIRECT) {
        if (isdirec)
          ErrMSpec("direct");
        isdirec = 1;
      }
      else
        Error("Bug: RdTSpec: Unknown type");
    }
    else if (curtok==T_STRUCT || curtok==T_UNION || curtok==T_ENUM) {
      if (type!=TS_NONE) {
        ErrMSpec("type");
        SkipTok();
      }
      else {
        ts->ts_xdat.ts_sref = NewSRef();
        type=curtok-256;
        RdStRef(ts->ts_xdat.ts_sref);
      }
    }
    else if (curtok==T_IDENT) {
      VDecl *typep = TDName(curident);
      if (typep==NULL || type!=TS_NONE)
        break;
      ts->ts_xdat.ts_tdef = typep;
      type=T_TDNAME-256;
      SkipTok();
    }
    else
      break;
  }
  if (type!=TS_NONE && type!=T_INT-256) {
    char *errtyp = NULL;
    if (len!=TS_NONE)
      errtyp = "length";
    else if (type!=T_CHAR-256 && sign!=TS_NONE)
      errtyp = "sign";
    if (errtyp!=NULL) {
      BError();
      AError(errtyp);
      AError(" specification with ");
      if (type==T_VOID-256)
        AError("'void'");
      else if (type==T_FLOAT-256)
        AError("'float'");
      else if (type==T_DOUBLE-256)
        AError("'double'");
      else if (type==T_STRUCT-256 || type==T_UNION-256)
        AError("'struct' or 'union'");
      else if (type==T_ENUM-256)
        AError("'enum'");
      else if (type==T_TDNAME-256)
        AError("typedef name");
      EError();
    }
  }
  if (type==TS_NONE || type==T_INT-256 || type==T_CHAR-256) {
    ts->ts_xdat.ts_siln.ts_sign = sign;
    ts->ts_xdat.ts_siln.ts_len  = len;
  }
  if (type==TS_NONE)
    type=T_INT-256;
  ts->ts_type  = type;
  ts->ts_sclas = sclass;
  ts->ts_const = isconst;
  ts->ts_direc = isdirec;
}

WrTQual(ts)
TSpec *ts;
{
  if (ts->ts_sclas!=TS_NONE && ts->ts_sclas!=TS_PARAM && 
      ts->ts_sclas!=T_AUTO-256)
    PrintTok(ts->ts_sclas+256);
  if (ts->ts_direc)
    PrintTok(T_DIRECT);
}

WrTSpec(ts)
TSpec *ts;
{
  WrTQual(ts);
  if (outstrm==2 && ts->ts_const)
    PrintTok(T_CONST);
  ts->ts_type = ts->ts_type;
  if (ts->ts_type==T_STRUCT-256 || ts->ts_type==T_UNION-256 ||
      ts->ts_type==T_ENUM-256)
    WrStRef(ts->ts_xdat.ts_sref,ts->ts_type);
  else if (ts->ts_type==T_TDNAME-256) {
    VDecl tmpvdecl;
    tmpvdecl.vd_type = NULL;
    tmpvdecl.vd_tspec = ts;
    ExpsType(&tmpvdecl,&tmpvdecl);
/* Expand typedef'ed struct refs since we weren't able to write the typedef */
    if (IsSURef(&tmpvdecl))
      WrStRef(tmpvdecl.vd_tspec->ts_xdat.ts_sref,tmpvdecl.vd_tspec->ts_type);
    else {
      WrEncode(ts->ts_xdat.ts_tdef->vd_var);
      WrChar(' ');
    }
  }
  else if (ts->ts_type==-1) {
    WrStr("undefined ");
    if (ts->ts_xdat.ts_iden!=NULL)
      WrStr(ts->ts_xdat.ts_iden);
  }
  else {
    if (outstrm!=1) {
      if (ts->ts_xdat.ts_siln.ts_sign==T_UNSIGN-256)
        WrStr("unsigned ");
      else if (ts->ts_xdat.ts_siln.ts_sign==T_SIGNED-256)
        WrStr("signed ");
      if (ts->ts_xdat.ts_siln.ts_len==T_LONG-256)
        WrStr("long ");
      else if (ts->ts_xdat.ts_siln.ts_len==T_SHORT-256)
        WrStr("short ");
      if (ts->ts_type!=TS_NONE)
        PrintTok(ts->ts_type+256);
    }
    else {
      if (ts->ts_type==T_INT-256) {
        if (ts->ts_xdat.ts_siln.ts_len==T_LONG-256)
          PrintTok(T_LONG);
        else {
          if (ts->ts_xdat.ts_siln.ts_sign==T_UNSIGN-256)
            PrintTok(T_UNSIGN);
          else
            PrintTok(T_INT);
        }
      }
      else if (ts->ts_type==T_VOID-256)
        WrStr("int ");
      else
        PrintTok(ts->ts_type+256);
    }
  }
}

RWTSpec(ts)
TSpec *ts;
{
  RdTSpec(ts);
  WrTSpec(ts);
}

static RdCkExpr()
{
  RdExpr();
  ChkDef(&curvalue.val_type);
}

static RdPExpr()
{
  Skip('(');
  RdCkExpr();
  Skip(')');
}

StoreStt(state)
State *state;
{
  state->s_varlst = varlst;
  state->s_stlst = strctlst;
  state->s_mainsp = mainsp;
  state->s_varscp = varscope;
  state->s_tagscp = tagscope;
  varscope = varlst;
  tagscope = strctlst;
}

RetrvStt(state)
State *state;
{
  varlst = state->s_varlst;
  strctlst = state->s_stlst;
  mainsp = state->s_mainsp;
  varscope = state->s_varscp;
  tagscope = state->s_tagscp;
}

WrTemp()
{
  if (tempsize>0) {
    WrStr("{\nchar _tvstrct[");
    WrInt(tempsize);
    WrStr("];\n");
  }
}

static ChkCond(vdecl)
VDecl *vdecl;
{
  VDecl tmpvdecl;
  ExpsType(vdecl,&tmpvdecl);
  if (tmpvdecl.vd_type!=NULL)
    return;
  {
    char type = tmpvdecl.vd_tspec->ts_type;
    if (type==T_VOID-256 || type==T_STRUCT-256 || type==T_UNION-256) {
      BError();
      ATError(&tmpvdecl);
      WrStr(" expression used for condition");
      EError();
    }
  }
}

static RdStmt()
{
  State state;
  StoreStt(&state);
  if (curtok=='{') {
    RdCStmt();
  }
  else if (curtok==T_IF) {
    int tsize;
    SkipTok();
    tempsize = 0;
    RdPExpr();
    ChkCond(&curvalue.val_type);
    tsize = tempsize;
    WrTemp();
    WrStr("if (");
    WrExpr(curvalue.val_expr);
    WrStr(" )\n");
    RdStmt();
    if (curtok==T_ELSE) {
      PrintTok(curtok);
      SkipTok();
      RdStmt();
    }
    if (tsize>0)
      WrStr("}\n");
  }
  else if (curtok==T_WHILE) {
    int tsize;
    SkipTok();
    tempsize = 0;
    RdPExpr();
    ChkCond(&curvalue.val_type);
    tsize = tempsize;
    WrTemp();
    WrStr("while (");
    WrExpr(curvalue.val_expr);
    WrStr(")\n");
    RdStmt();
    if (tsize>0)
      WrStr("}\n");
  }
  else if (curtok==T_DO) {
    SkipTok();
    WrStr("for (;;) {\n");
    RdStmt();
    if (curtok!=T_WHILE)
      Error("'do' whithout 'while'");
    else
      SkipTok();
    tempsize = 0;
    RdPExpr();
    ChkCond(&curvalue.val_type);
    WrTemp();
    WrStr("if (!(");
    WrExpr(curvalue.val_expr);
    WrStr("))\n");
    WrStr("break;\n");
    if (tempsize>0)
      WrStr("}\n");
    WrStr("}\n");
    Skip(';');
  }
  else if (curtok==T_FOR) {
    char *expr1=NULL,*expr2=NULL,*expr3=NULL;
    int tsize;
    tempsize = 0;
    SkipTok();
    Skip('(');
    if (curtok!=';') {
      RdCkExpr();
      expr1 = curvalue.val_expr;
    }
    Skip(';');
    if (curtok!=';') {
      RdCkExpr();
      ChkCond(&curvalue.val_type);
      expr2 = curvalue.val_expr;
    }
    Skip(';');
    if (curtok!=')') {
      RdCkExpr();
      expr3 = curvalue.val_expr;
    }
    Skip(')');
    tsize = tempsize;
    WrTemp();
    WrStr("for (");
    if (expr1!=0)
      WrExpr(expr1);
    WrChar(';');
    if (expr2!=0)
      WrExpr(expr2);
    WrChar(';');
    if (expr3!=0)
      WrExpr(expr3);
    WrChar(')');
    RdStmt();
    if (tsize>0)
      WrStr("}\n");
  }
  else if (curtok==T_RETURN) {
    int tsize = 0;
    SkipTok();
    if (curtok!=';') {
      tempsize = 0;
      RdCkExpr();
      MakRVal(&curvalue);
      if (rettype.vd_type==NULL && rettype.vd_tspec->ts_type==T_VOID-256)
        Error("expression returned from void function");
      else if (!CanInit(&rettype,&curvalue.val_type,curvalue.val_expr)) {
        BError();
        AError("wrong return type - ");
        ATError(&curvalue.val_type);
        AError(" instead of ");
        ATError(&rettype);
        EError();
      }
      tsize = tempsize;
      WrTemp();
      PrintTok(T_RETURN);
      if (IsSorU(curvalue.val_type.vd_type,curvalue.val_type.vd_tspec)) {
        WrChar('(');
        WrTSpec(curvalue.val_type.vd_tspec);
        WrStr("*)memcpy(_tvret,&(");
        WrExpr(curvalue.val_expr);
        WrStr("),");
        WrInt(SizeOf(&curvalue.val_type));
        WrChar(')');
      }
      else
        WrExpr(curvalue.val_expr);
    }
    else
      PrintTok(T_RETURN);
    Expect(';');
    if (tsize>0)
      WrChar('}');
    WrLn();
  }
  else if (curtok==T_BREAK || curtok==T_CONTIN) {
    PrintTok(curtok);
    SkipTok();
    Expect(';');
    WrLn();
  }
  else if (curtok==T_SWITCH) {
    int tsize;
    SkipTok();
    tempsize = 0;
    RdPExpr();
    tsize = tempsize;
    WrTemp();
    WrStr("switch (");
    WrExpr(curvalue.val_expr);
    WrStr(")\n");
    RdStmt();
    if (tsize>0)
      WrStr("}\n");
  }
  else if (curtok==T_CASE) {
    PrintTok(curtok);
    SkipTok();
    RWExpr();
    Expect(':');
    WrLn();
    RdStmt();
  }
  else if (curtok==T_DEFAUL) {
    PrintTok(curtok);
    SkipTok();
    Expect(':');
    WrLn();
    RdStmt();
  }
  else if (curtok==T_GOTO) {
    PrintTok(curtok);
    SkipTok();
    Expect(T_IDENT);
    Expect(';');
    WrLn();
  }
  else if (curtok==';') {
    PrintTok(curtok);
    SkipTok();
    WrLn();
  }
  else if (curtok==T_IDENT && PeekTok()==':') {
    PrintTok(curtok);
    SkipTok();
    PrintTok(curtok);
    SkipTok();
    RdStmt();
  }
  else {
    tempsize = 0;
    RdCkExpr();
    WrTemp();
    WrExpr(curvalue.val_expr);
    Expect(';');
    if (tempsize!=0)
      WrStr("}\n");
    WrLn();
  }
  RetrvStt(&state);
}

static RdCStmt()
{
  int scpdepth = 0;
  PrintTok(curtok);
  SkipTok();
  WrLn();
  if (IsATYPE(curtok,curident))
    scpdepth = RdDeclLs();
  while (curtok!='}') {
    if (IsATYPE(curtok,curident)) {
      Error("Declaration after statement");
      RdDeclLs();
    }
    else
      RdStmt();
  }
  while (scpdepth>0) {
    WrChar('}');
    scpdepth--;
  }
  Expect('}');
  WrLn();
}

RdOSPLst(plst)
PLst *plst;
{
  TLLst *ptypes;
  {
    TLLst **tllp = &ptypes;
    ILst *pnames = plst->pl_ident;
    while (pnames!=NULL) {
      TLLst *newtype = NewTLLst();
      newtype->tll_type = NULL;
      newtype->tll_tspec.ts_sclas = T_EXTERN-256;
      newtype->tll_next = NULL;
      tllp = &(*tllp = newtype)->tll_next;
      pnames = pnames->il_next;
    }
    *tllp = NULL;
  }
  while (IsATYPE(curtok,curident)) {
    TSpec tspec;
    RdTSpec(&tspec);
    if (curtok!=';') {
      for (;;) {
        if (curtok==';') {
          SkipTok();
          break;
        }
        {
          VDecl vdecl;
          vdecl.vd_tspec = &tspec;
          RdIDecl(&vdecl);
          if (vdecl.vd_type!=NULL && vdecl.vd_type->tl_op=='(')
            Error("Parameter declared as function\n");
          if (vdecl.vd_tspec->ts_sclas!=TS_NONE &&
              vdecl.vd_tspec->ts_sclas!=TS_PARAM &&
              vdecl.vd_tspec->ts_sclas!=T_REGIST-256) {
            Error("Invalid storage class for parameter");
            vdecl.vd_tspec->ts_sclas=TS_NONE;
          }
          if (vdecl.vd_tspec->ts_sclas==TS_NONE)
            vdecl.vd_tspec->ts_sclas = TS_PARAM;
          {
            TLLst *tlst = ptypes;
            ILst *ilst = plst->pl_ident;
            while (ilst!=NULL) {
              if (strcmp(ilst->il_ident,vdecl.vd_var)==0)
                break;
              tlst = tlst->tll_next;
              ilst = ilst->il_next;
            }
            if (tlst==NULL) {
              BError();
              WrStr(vdecl.vd_var);
              WrStr(" is not a parameter");
              EError();
            }
            else {
              if (tlst->tll_tspec.ts_sclas!=T_EXTERN-256)
                Error("Redeclared parameter");
              else {
                tlst->tll_type = vdecl.vd_type;
                memcpy(&tlst->tll_tspec,vdecl.vd_tspec,sizeof(TSpec));
              }
            }
          }
          if (curtok==',')
            SkipTok();
          else if (curtok==';') {
            SkipTok();
            break;
          }
          else {
            Error("Expecting ',' or ';'");
            SkipTok();
            break;
          }
        }
      }
    }
  }
  {
    TLLst *tlst = ptypes;
    while (tlst!=NULL) {
      if (tlst->tll_tspec.ts_sclas==T_EXTERN-256) {
        tlst->tll_type=NULL;
        tlst->tll_tspec.ts_sclas=TS_PARAM;
        tlst->tll_tspec.ts_type=T_INT-256;
        tlst->tll_tspec.ts_xdat.ts_siln.ts_sign=TS_NONE;
        tlst->tll_tspec.ts_xdat.ts_siln.ts_len=TS_NONE;
        tlst->tll_tspec.ts_const = 0;
        tlst->tll_tspec.ts_direc = 0;
      }
      tlst = tlst->tll_next;
    }
  }
  plst->pl_type = ptypes;
}

int IsProto(plst)
PLst *plst;
{
  return (int)(plst->pl_type)!=1;
}

RdFDef(fdecl)
VDecl *fdecl;
{
  State oldstate;
  int isproto = IsProto(fdecl->vd_type->tl_dat.tld_plst);
  int scopedep = 0;
  StoreStt(&oldstate);
  if (!isproto) {
    RdOSPLst(fdecl->vd_type->tl_dat.tld_plst);
    WrOSPLst(fdecl->vd_type->tl_dat.tld_plst,fdecl);
  }
  {
    {
      PLst *plst = fdecl->vd_type->tl_dat.tld_plst;
      ILst *pnames = plst->pl_ident;
      TLLst *ptypes = plst->pl_type;
      int paramnum = 0;
      while (ptypes!=NULL && ptypes!=(TLLst *)1) {
        VDecl *vdecl = NewVDecl();
        vdecl->vd_type = ptypes->tll_type;
        vdecl->vd_var = pnames->il_ident;
        vdecl->vd_tspec = &ptypes->tll_tspec;
        vdecl->vd_prev = varlst;
        if (vdecl->vd_var==NULL) {
          BError();
          WrStr("no name for parameter ");
          WrInt(paramnum+1);
          EError();
        }
        if (IsSorU(ptypes->tll_type,&ptypes->tll_tspec)) {
          WrStr("{\n");
          ++scopedep;
          WrVDecl(vdecl);
          WrStr(";\n");
          WrStr("memcpy(&");
          WrStr(vdecl->vd_var);
          WrChar(',');
          WrEnIden("_tvp",ptypes);
          WrChar(',');
          WrInt(SizeOf(vdecl));
          WrStr(");\n");
        }
        varlst = vdecl;
        ++paramnum;
        ptypes = ptypes->tll_next;
        pnames = pnames->il_next;
      }
    }
  }
  if (!isproto)
    fdecl->vd_type->tl_dat.tld_plst->pl_type=(TLLst *)1;
  fdecl->vd_type->tl_dat.tld_plst->pl_ident=(ILst *)0;
  if (curtok!='{')
    ExpError('{');
  else {
    rettype.vd_tspec = fdecl->vd_tspec;
    rettype.vd_type = fdecl->vd_type->tl_next;
    RdCStmt();
    WrLn();
  }
  while (scopedep>0) {
    WrStr("}\n");
    --scopedep;
  }
  RetrvStt(&oldstate);
}

TSpec flttspec;
TSpec chrtspec;
TSpec inttspec;
TSpec lngtspec;
TSpec undtspec;
TSpec cvdtspec;

typedef struct enumdef {
  VDecl *ed_start;
  VDecl *ed_end;
} EnumDef;

static EnumDef *RdEnDef()
{
  EnumDef *edef = (EnumDef *)StkAlloc(sizeof(EnumDef));
  TSpec *tspec = NewTSpec();
  int nextval = 1;
  tspec->ts_type = T_ENUM-256;
  edef->ed_start = NULL;
  SkipTok();
  for (;;) {
    VDecl *vdecl = NewVDecl();
    if (edef->ed_start==NULL)
      edef->ed_start = vdecl;
    vdecl->vd_tspec = tspec;
    if (curtok!=T_IDENT) {
      Error("Expecting identifier");
      vdecl->vd_var = NULL;
    }
    else {
      vdecl->vd_var = KeepIden();
      if (SrchDecl(vdecl->vd_var,varscope)!=NULL) {
        BError();
        WrStr("Redefinition of ");
        WrStr(vdecl->vd_var);
        EError();
      }
      vdecl->vd_type = NewTQ();
      vdecl->vd_type->tl_op='#';
      vdecl->vd_type->tl_next=NULL;
      SkipTok();
    }
    if (curtok==T_EQUAL) {
      SkipTok();
      RdAsExpr();
      if (!curvalue.val_isce)
        Error("Initializer must be constant");
      else if (*(int *)curvalue.val_expr!=T_INTNUM)
        Error("Initializer must be an int");
      else
        nextval = vdecl->vd_type->tl_dat.tld_num = EIntVal(curvalue.val_expr);
    }
    else
      vdecl->vd_type->tl_dat.tld_num = nextval;
    ++nextval;
    vdecl->vd_prev = varlst;
    varlst = vdecl;
    if (curtok==',')
      SkipTok();
    else if (curtok=='}') {
      SkipTok();
      break;
    }
    else {
      Error("Expecting ',' or '}'");
      break;
    }
  }
  edef->ed_end = varlst;
  return edef;
}

static VDecl *RdStDef()
{
  VDecl *prev = NULL;
  VDecl **prevp = &prev;
  SkipTok();
  for (;;) {
    if (curtok=='}') {
      SkipTok();
      break;
    }
    {
      TSpec *tspec = NewTSpec();
      RdTSpec(tspec);
      if (HasDef(tspec)) {
        WrStRef(tspec->ts_xdat.ts_sref,tspec->ts_type);
        WrStr(";\n");
      }
      if (curtok==';')
        SkipTok();
      else {
        for (;;) {
          VDecl *vdecl = NewVDecl();
          RdIDecl(vdecl);
          vdecl->vd_tspec = tspec;
          {
            VDecl **vdpp = &prev;
            while (vdpp!=prevp) {
              if (strcmp((*vdpp)->vd_var,vdecl->vd_var)==0) {
                BError();
                WrChar('\'');
                WrStr(vdecl->vd_var);
                WrStr("' declared twice in struct/union");
                EError();
              }
              vdpp = &(*vdpp)->vd_prev;
            }
          }
          *prevp = vdecl;
          prevp = &vdecl->vd_prev;
          if (curtok==',')
            SkipTok();
          else if (curtok==';') {
            SkipTok();
            break;
          }
          else {
            Error("Expecting ';'");
            DumpLine();
            break;
          }
        }
      }
    }
  }
  *prevp = NULL;
  return prev;
}

/* Read structure definition */

static RdStRef(sref)
StrctRef *sref;
{
  char tagtype = curtok-256;
  VDecl **flstptr = NULL;
  SkipTok();
  if (curtok!=T_IDENT && curtok!=T_DIRECT && curtok!='{') {
    Error("identifier or '{' expected");
    sref->sr_dorr = SR_DEF;
    sref->sr_drdat.sr_def = (VDecl *)1;
    return;
  }
  if (curtok==T_IDENT || curtok==T_DIRECT) {
    StrctDef *sd = strctlst;
    sref->sr_dorr = SR_REF;
    while (sd!=tagscope) {
      if (strcmp(sd->sd_name,curident)==0)
        break;
      sd=sd->sd_prev;
    }
    if (sd==tagscope) {
      if (PeekTok()=='{')
        sd = NULL;
      else {
        while (sd!=NULL) {
          if (strcmp(sd->sd_name,curident)==0)
            break;
          sd = sd->sd_prev;
        }
      }
    }
    if (sd==NULL) {
      sd = NewSDef();
      sd->sd_name = KeepIden();
      SkipTok();
      sd->sd_prev = strctlst;
      sd->sd_flst = (VDecl *)1;
      sd->sd_type = tagtype;
      sref->sr_drdat.sr_ref = strctlst = sd;
      flstptr = &sd->sd_flst;
    }
    else {
      if (sd->sd_type!=tagtype) {
        BError();
        AError(sd->sd_name);
        AError(" is a ");
        PrintTok(sd->sd_type+256);
        AError(" tag");
        EError();
      }
      sref->sr_drdat.sr_ref = sd;
      SkipTok();
      if (curtok=='{') {
        if (sd->sd_flst!=(VDecl *)1) {
          BError();
          WrStr("Redefinition of tag ");
          WrStr(sd->sd_name);
          EError();
        }
        flstptr = &sd->sd_flst;
      }
    }
  }
  else {
    sref->sr_dorr = SR_DEF;
    flstptr = &sref->sr_drdat.sr_def;
  }
  if (curtok=='{') {
    if (sref->sr_dorr==SR_REF)
      sref->sr_dorr = SR_RANDD;
    if (tagtype==T_ENUM-256)
      *flstptr = (VDecl *)RdEnDef();
    else
      *flstptr = RdStDef();
  }
}

static WrFiLst(vdecl)
VDecl *vdecl;
{
  fldname = 1;
  if (vdecl==(VDecl *)1)
    WrStr("undefined\n");
  else {
    while (vdecl!=NULL) {
      WrXDecl(vdecl,0);
#ifdef IGNORE
      WrTSpec(vdecl->vd_tspec);
      WrLIDecl(vdecl->vd_type);
      WrEnIden("_tvf",vdecl->vd_var);
      WrRIDecl(vdecl->vd_type,0,vdecl);
      WrStr(";\n");
#endif /* IGNORE */
      WrStr(";\n");
      vdecl = vdecl->vd_prev;
    }
  }
  fldname = 0;
}

static WrStDef(sdef)
StrctDef *sdef;
{
  WrStr("struct ");
  if (sdef->sd_name!=NULL) {
    WrEncode(sdef->sd_name);
    WrChar(' ');
  }
  WrChar('{');
  WrFiLst(sdef->sd_flst);
  WrChar('}');
}

WrStTag(sref)
StrctRef *sref;
{
  if (sref->sr_dorr==SR_DEF) {
    if (outstrm==1) {
      WrStr("_tvt");
      Wr04x(sref);
    }
  }
  else
    WrEncode(sref->sr_drdat.sr_ref->sd_name);
}

WrStRef(sref,soru)
StrctRef *sref;
int soru;
{
  if (outstrm==1 && soru==T_ENUM-256) {
    PrintTok(T_INT);
    return;
  }
  WrStr(toknames[soru]);
  WrChar(' ');
  WrStTag(sref);
  WrChar(' ');
  {
    char dorr = sref->sr_dorr;
    if (outstrm==1 && (dorr==SR_DEF || dorr==SR_RANDD)) {
      VDecl *fields;
      if (dorr==SR_DEF)
        fields = sref->sr_drdat.sr_def;
      else
        fields = sref->sr_drdat.sr_ref->sd_flst;
      WrStr(" {\n");
      WrFiLst(fields);
      WrChar('}');
    }
  }
}

VDecl cvpdecl;
static Type ptrtype;

main(argc,argv)
int argc;
char **argv;
{
  if (argc>1 && strcmp(argv[1],"-w")==0) {
    warn = 1;
  }
  flttspec.ts_type = T_DOUBLE-256;
  flttspec.ts_sclas = T_STATIC-256;
  cvdtspec.ts_type = T_VOID-256;
  cvdtspec.ts_sclas = TS_NONE;
  cvdtspec.ts_const = 1;
  chrtspec.ts_type = T_CHAR-256;
  chrtspec.ts_sclas = T_STATIC-256;
  inttspec.ts_type = T_INT-256;
  inttspec.ts_sclas = T_EXTERN-256;
  lngtspec.ts_type = T_INT-256;
  lngtspec.ts_sclas = TS_NONE;
  lngtspec.ts_xdat.ts_siln.ts_len = T_LONG-256;
  intdecl.vd_type = NULL;
  intdecl.vd_tspec = &inttspec;
  undtspec.ts_type = -1;
  undtspec.ts_xdat.ts_iden = NULL;
  ptrtype.tl_op='*';
  ptrtype.tl_next = NULL;
  cvpdecl.vd_type = &ptrtype;
  cvpdecl.vd_tspec = &cvdtspec;
  outstrm = 1;
  InitRdTk();
  while (curtok!=-1) {
    RdGDecl();
    WrLn();
  }
  outstrm = 2;
  WrInt(mainsp-mainstk);
  WrChar('/');
  WrInt(sizeof(mainstk));
  WrStr(" stack\n");
  exit(exitstat);
}
