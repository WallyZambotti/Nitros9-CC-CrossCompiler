#ifndef CoCo
#include <ctype.h>
#include <sys/unistd.h>
#include <string.h>
#include <stdlib.h>
#else
#include <ctype.h>
#endif
#include "ansifront.h"

double atod();

#define FALSE (0)
#define TRUE (!FALSE)

#define NewExLst() ((ExpLst *)StkAlloc(sizeof(ExpLst)))

Value curvalue;
static int IsVoid(VDecl *);

int EIntVal(expr)
char *expr;
{
  return (int)((UnryExpr *)expr)->exu_t; /* WZ - changed (int) to *(int *) in line with next two functions */
}

long ELngVal(expr)
char *expr;
{
  return *(long *)((UnryExpr *)expr)->exu_t;
}

double ENumVal(expr)
char *expr;
{
  return *(double *)((UnryExpr *)expr)->exu_t;
}

EAsInt(expr,val)
char *expr;
int val;
{
  ((UnryExpr *)expr)->exu_t = (char *)val;
}

EAsLng(expr,val) /* WZ - removed return type long as nothing is returned */
char *expr;
long val;
{
  *(long *)((UnryExpr *)expr)->exu_t = val;
}

EAsNum(expr,val) /* WZ - removed return type double as nothing is returned */
char *expr;
double val;
{
  *(double *)((UnryExpr *)expr)->exu_t = val;
}

char *IntExpr(val)
int val;
{
  return (char *)NewUExpr(T_INTNUM,(char *)val);
}

char *LngExpr(val)
long val;
{
  long *lp = (long *)StkAlloc(sizeof(long));
  *lp = val;
  return (char *)NewUExpr(T_LNGNUM,(char *)lp);
}

UnryExpr *NewUExpr(op,term)
int op;
char *term;
{
  UnryExpr *ue = (UnryExpr *)StkAlloc(sizeof(UnryExpr));
  ue->exu_op = op;
  ue->exu_t = term;
  return ue;
}

BinExpr *NewBExpr(op,term1,term2)
int op;
char *term1;
char *term2;
{
  BinExpr *be = (BinExpr *)StkAlloc(sizeof(BinExpr));
  be->exb_op = op;
  be->exb_t1 = term1;
  be->exb_t2 = term2;
  return be;
}

#define NewTExpr() ((TerExpr *)StkAlloc(sizeof(TerExpr)))

#define curexpr  (curvalue.val_expr)
#define curislv  (curvalue.val_islv)
#define curtype  (curvalue.val_type.vd_type)
#define curtspec (curvalue.val_type.vd_tspec)

static int isodigit(c)
int c;
{
  return (c>='0' && c<='7');
}

static int hexval(c)
int c;
{
  return (c<='9') ? (c-'0') : (c<='F') ? (c-'A'+10) : (c-'a'+10);
}

int CharUVal(pp)
char **pp;
{
  char *p = *pp;
  ++*pp;
  if (*p!='\\')
    return *p;
  else {
    ++*pp;
    switch (*++p) {
      case 'a':
        return 7;
        break;
      case 'b':
        return 8;
        break;
      case 'f':
        return 12;
        break;
      case 'l':
        return 10;
        break;
      case 'n':
        return 13;
        break;
      case 'r':
        return 13;
        break;
      case 't':
        return 9;
        break;
      case 'v':
        return 11;
        break;
      case '\\':
        return '\\';
        break;
      case '\'':
        return '\'';
        break;
      case '"':
        return '"';
        break;
      case '?':
        return '?';
        break;
      case 'x':
        ++p;
        {
          int val = 0;
          while (isxdigit(*p)) {
            ++*pp;
            val = val*16 + hexval(*p);
            ++p;
          }
          return val;
        }
      default:
        {
          if (isodigit(*p)) {
            int val = *p-'0';
            ++p;
            if (isodigit(*p)) {
              ++*pp;
              val = val*8 + *p - '0';
              ++p;
              if (isodigit(*p)) {
                ++*pp;
                val = val*8 + *p - '0';
              }
            }
            return val;
          }
          else
            return *p;
        }
    }
  }
}

static int CharVal(p)
char *p;
{
  return CharUVal(&p);
}

MakUndef(ident)
char *ident;
{
  curvalue.val_type.vd_tspec = NewTSpec();
  curvalue.val_type.vd_tspec->ts_type = -1;
  curvalue.val_type.vd_tspec->ts_sclas = TS_NONE;
  curvalue.val_type.vd_tspec->ts_const = 0;
  curvalue.val_type.vd_tspec->ts_direc = 0;
  curvalue.val_type.vd_tspec->ts_xdat.ts_iden = ident;
  curvalue.val_isce = 0;
  curexpr = (char *)NewUExpr(-1,ident);
}

RdPriExp()
{
  curvalue.val_type.vd_type = NULL;
  for (;;) {
    if (curtok==T_IDENT) {
      {
        VDecl *curvar = SrchDecl(curident,NULL);
        if (curvar!=NULL) {
          if (curvar->vd_tspec->ts_sclas==T_TYPEDE-256)
            Error("type name used as value");
          curvalue.val_type.vd_tspec = curvar->vd_tspec;
          if (curvar->vd_type!=NULL && curvar->vd_type->tl_op=='#') {
            curvalue.val_type.vd_type = NULL;
            curvalue.val_expr = 
              IntExpr(curvar->vd_type->tl_dat.tld_num);
            curvalue.val_isce = 1;
            curvalue.val_islv = 0;
          }
          else {
            curvalue.val_type.vd_type = curvar->vd_type;
            curvalue.val_expr = (char *)NewUExpr(T_IDENT,(char *)curvar);
            curvalue.val_isce = 0;
            curvalue.val_islv = 1;
          }
        }
        else {
          MakUndef(KeepIden());
        }
      }
      SkipTok();
    }
    else if (curtok==T_NUMBER) {
      double *dp = (double *)StkAlloc(sizeof(double));
      *dp = atod(curident);
      curtspec = &flttspec;
      curexpr = (char *)NewUExpr(T_NUMBER,(char *)dp);
      curislv = 0;
      curvalue.val_isce = 1;
      SkipTok();
    }
    else if (curtok==T_INTNUM || curtok==T_LNGNUM) {
      long intval = 0;
      if (curident[0]=='0') {
        char *cip = curident;
        if (curident[1]=='x') {
          cip += 2;
          while (*cip!='\0') {
            intval = intval*16 + hexval(*cip);
            ++cip;
          }
        }
        else {
          while (*cip!='\0') {
            intval = intval*8 + *cip-'0';
            ++cip;
          }
        }
      }
      else
        intval = atol(curident);
      curislv = 0;
      curvalue.val_isce = 1;
      if (curtok==T_LNGNUM || intval>32767 || intval<-32768) {
        curexpr = LngExpr(intval);
        curtspec = &lngtspec;
      }
      else {
        curexpr = IntExpr((int)intval);
        curtspec = &inttspec;
      }
      SkipTok();
    }
    else if (curtok==T_STRING) {
      curtspec = &chrtspec;
      curtype = NewTQ();
      curtype->tl_op = '[';
      curtype->tl_dat.tld_num = StrLen(curident)+1;
      curtype->tl_next = NULL;
      curexpr = (char *)NewUExpr(T_STRING,KeepIden());
      curislv = 1;
      curvalue.val_isce = 0;
      SkipTok();
    }
    else if (curtok==T_CHRLIT) {
      int val;
      curexpr = IntExpr(CharVal(curident));
      curtspec = &inttspec;
      curislv = 0;
      curvalue.val_isce = 1;
      SkipTok();
    }
    else if (curtok=='(') {
      SkipTok();
      RdExpr();
      Skip(')');
    }
    else {
      BError();
      AError("Unexpected '");
      PrintTok(curtok);
      WrChar('\'');
      EError();
      if (curtok==';' || curtok=='}' || curtok==-1) {
        MakUndef("");
        break;
      }
      SkipTok();
      continue;
    }
    break;
  }
}

WrPriExp(e)
char *e;
{
  UnryExpr *ue = (UnryExpr *)e;
  switch (ue->exu_op) {
    case T_IDENT:
      {
        VDecl *vd = (VDecl *)ue->exu_t;
        WrVarNam(vd->vd_var,vd->vd_tspec->ts_sclas);
        WrChar(' ');
      }
      break;
    case -1:
      WrHash((char *)ue->exu_t);
      WrChar(' ');
      break;
    case T_NUMBER:
      WrDouble(*(double *)ue->exu_t);
      break;
    case T_INTNUM:
      WrInt(EIntVal(ue));
      break;
    case T_LNGNUM:
      WrLong(ELngVal(ue));
      break;
    case T_STRING:
      WrDelStr((char *)ue->exu_t,'"');
      break;
    case T_CHRLIT:
      WrDelStr((char *)ue->exu_t,'\'');
      break;
    default:
      PrintTok('(');
      WrExpr(e);
      PrintTok(')');
  }
}

ExpsType(vdecl,nvdecl)
VDecl *vdecl;
VDecl *nvdecl;
{
  nvdecl->vd_tspec = vdecl->vd_tspec;
  nvdecl->vd_type = vdecl->vd_type;
  while (nvdecl->vd_type==NULL && nvdecl->vd_tspec->ts_type==T_TDNAME-256) {
    VDecl *vd = nvdecl->vd_tspec->ts_xdat.ts_tdef;
    nvdecl->vd_tspec = vd->vd_tspec;
    nvdecl->vd_type = vd->vd_type;
  }
}

Type *DeclType(vdecl)
VDecl *vdecl;
{
  VDecl xvdecl;
  ExpsType(vdecl,&xvdecl);
  return xvdecl.vd_type;
}

int IsPtr(ovdecl)
VDecl *ovdecl;
{
  Type *type = DeclType(ovdecl);
  return (type!=NULL && (type->tl_op=='*' || type->tl_op=='['));
}

int IsNum(vdecl)
VDecl *vdecl;
{
  VDecl xvdecl;
  ExpsType(vdecl,&xvdecl);
  if (xvdecl.vd_type!=NULL)
    return FALSE;
  return IsNumTyp(xvdecl.vd_tspec->ts_type);
}

static int IsChar(vdecl)
VDecl *vdecl;
{
  return (vdecl->vd_tspec->ts_type==T_CHAR-256);
}

static UnryExpr c255expr = {T_INTNUM,(char *)255};

static Type *PtrTo(type)
Type *type;
{
  Type *newnode = NewTQ();
  newnode->tl_op = '*';
  newnode->tl_dat.tld_cons = 0;
  newnode->tl_next = type;
  return newnode;
}

MakRVal(value)
Value *value;
{
  VDecl *valdecl = &value->val_type;
  Type *valtype = valdecl->vd_type;
  ExpsType(valdecl,valdecl);
  if (valtype==NULL) {
    if (valdecl->vd_tspec->ts_type==T_CHAR-256) {
      if (valdecl->vd_tspec->ts_xdat.ts_siln.ts_sign==T_UNSIGN-256) {
        value->val_expr = 
          (char *)NewBExpr('&',value->val_expr,(char *)&c255expr);
      }
      valdecl->vd_tspec = &inttspec;
    }
  }
  else {
    int typeop = valtype->tl_op;
    if (typeop=='[') {
      valdecl->vd_type = PtrTo(valtype->tl_next);
    }
    else if (typeop=='(') {
      valdecl->vd_type = PtrTo(valtype);
    }
  }
  value->val_islv = FALSE;
}

CpyValue(val1p,val2p)
Value *val1p;
Value *val2p;
{
  memcpy(val1p,val2p,sizeof(Value));
}

int IsSorU(type,tspec)
Type *type;
TSpec *tspec;
{
  VDecl vdecl;
  vdecl.vd_type = type;
  vdecl.vd_tspec = tspec;
  ExpsType(&vdecl,&vdecl);
  if (vdecl.vd_type!=NULL)
    return 0;
  if (vdecl.vd_tspec->ts_type!=T_STRUCT-256 &&
      vdecl.vd_tspec->ts_type!=T_UNION-256)
    return 0;
  return !0;
}

static VDecl *GetFlds(vdecl)
VDecl *vdecl;
{
  StrctRef *sref = vdecl->vd_tspec->ts_xdat.ts_sref;
  if (sref->sr_dorr==SR_DEF)
    return sref->sr_drdat.sr_def;
  else
    return sref->sr_drdat.sr_ref->sd_flst;
}

int SizeOf(ovdecl)
VDecl *ovdecl;
{
  VDecl vdecl;
  ExpsType(ovdecl,&vdecl);
  if (vdecl.vd_type==NULL) {
    switch (vdecl.vd_tspec->ts_type) {
      case T_DOUBLE-256:
        return sizeof(double);
      case T_FLOAT-256:
        return sizeof(float);
      case T_CHAR-256:
        return sizeof(char);
      case T_INT-256:
        if (vdecl.vd_tspec->ts_xdat.ts_siln.ts_len==T_LONG-256)
          return 4 ; /* return sizeof(long); cant assume is same as CoCo */
        else
          /* Cannot assume size of int of cross compiliation system is the same as OS9/C int 
             Probably it will be different
          return sizeof(int);
          */
         return 2; /* A CoCo C int is always 2 */
      case T_STRUCT-256:
        {
          VDecl *fields = GetFlds(&vdecl);
          if (fields==NULL)
            return 1;
          {
            int size = 0;
            while (fields!=NULL) {
              size += SizeOf(fields);
              fields = fields->vd_prev;
            }
            return size;
          }
        }
      case T_UNION-256:
        {
          VDecl *fields = GetFlds(&vdecl);
          int size = 1;
          while (fields!=NULL) {
            int fsize = SizeOf(fields);
            if (fsize>size)
              size = fsize;
            fields = fields->vd_prev;
          }
          return size;
        }
      default:
        BError();
        WrStr("SizeOf: Unknown type ");
        WrInt(vdecl.vd_tspec->ts_type);
        EError();
    }
  }
  else if (vdecl.vd_type->tl_op=='*')
    return sizeof (char *);
  else if (vdecl.vd_type->tl_op=='[') {
    if (vdecl.vd_type->tl_dat.tld_num==0) {
      Error("Cannot evaluate size of array of unspecified length");
      return 1;
    }
    else {
      VDecl nvdecl;
      nvdecl.vd_type = vdecl.vd_type->tl_next;
      nvdecl.vd_tspec = vdecl.vd_tspec;
      return vdecl.vd_type->tl_dat.tld_num * SizeOf(&nvdecl);
    }
  }
  else {
    BError();
    WrStr("SizeOf: Unknown op ");
    WrInt(vdecl.vd_type->tl_op);
    EError();
  }
}

AddPreOp(op)
int op;
{
  curvalue.val_expr = (char *)NewBExpr(op,NULL,curvalue.val_expr);
}

RdPrExpr()
{
  for (;;) {
    int iscast = 0;
    if (curtok=='(') {
      int nexttok = PeekTok();
      if (IsATYPE(nexttok,nexident))
        iscast = 1;
    }
    if (iscast) {
      VDecl *vdecl = NewVDecl();
      SkipTok();
      RdVDecl(vdecl);
      if (vdecl->vd_var!=NULL)
        Error("variable in cast type");
      Skip(')');
      RdPrExpr();
      if (IsSorU(curtype,curtspec))
        Error("cast of struct or union");
      curvalue.val_isce = 0;
      curtspec = vdecl->vd_tspec;
      curtype = vdecl->vd_type;
      curexpr = (char *)NewBExpr('(',vdecl,curexpr);
      curislv = 0;
    }
    else if (curtok==T_SIZEOF) {
      VDecl *sizetype;
      SkipTok();
      if (curtok=='(') {
        SkipTok();
        if (IsATYPE(curtok,curident)) {
          VDecl *vdecl = NewVDecl();
          RdVDecl(vdecl);
          if (vdecl->vd_var!=NULL)
            Error("non-abstract declarator used with 'sizeof'");
          sizetype = vdecl;
        }
        else {
          RdExpr();
          ChkDef(&curvalue.val_type);
          sizetype = &curvalue.val_type;
        }
        Skip(')');
      }
      else {
        RdPrExpr();
        sizetype = &curvalue.val_type;
      }
      curexpr = IntExpr(SizeOf(sizetype));
      curtype = NULL;
      curtspec = &inttspec;
      curislv = 0;
      curvalue.val_isce = 1;
    }
    else if (curtok=='*') {
      SkipTok();
      RdPrExpr();
      MakRVal(&curvalue);
      if (IsSUPtr(&curvalue.val_type))
        CurrCast();
      AddPreOp('*');
      if (curvalue.val_type.vd_type==NULL)
        Error("Attempt to indirect non-pointer");
      else {
        curvalue.val_type.vd_type = curvalue.val_type.vd_type->tl_next;
        IndCast();
      }
      curvalue.val_islv = 1;
    }
    else if (curtok=='&') {
      Type *oldtype;
      SkipTok();
      RdPrExpr();
      if (!curislv)
        Error("Address of rvalue");
      oldtype = curvalue.val_type.vd_type;
      curvalue.val_type.vd_type = PtrTo(oldtype);
      if (oldtype==NULL || oldtype->tl_op!='[')
        AddPreOp('&');
      else {
        VDecl *vdecl = NewVDecl();
        vdecl->vd_type = curvalue.val_type.vd_type;
        vdecl->vd_tspec = curvalue.val_type.vd_tspec;
        vdecl->vd_var = NULL;
        curvalue.val_expr = (char *)NewBExpr('(',vdecl,curvalue.val_expr);
      }
      curislv = 0;
    }
    else if (IsPREOP(curtok)) {
      {
        int op = curtok;
        SkipTok();
        RdPrExpr();
        if (curvalue.val_isce) {
          switch (*(int *)curexpr) {
            case T_INTNUM:
              {
                int val = EIntVal(curvalue.val_expr);
                if (op=='!')
                  val = !val;
                else if (op=='~')
                  val = ~val;
                else if (op=='-')
                  val = -val;
                else
                  exit(1);
                EAsInt(curvalue.val_expr,val);
              }
              break;
            case T_LNGNUM:
              {
                long val = ELngVal(curvalue.val_expr);
                if (op=='!') {
                  curvalue.val_expr = IntExpr(!val);
                  break;
                }
                else if (op=='-')
                  val = -val;
                else if (op=='~')
                  val = ~val;
                else
                  exit(1);
                EAsLng(curvalue.val_expr,val);
              }
              break;
            case T_NUMBER:
              {
                double *valp = (double *)((UnryExpr *)curexpr)->exu_t;
                if (op=='!')
                  curexpr = IntExpr(!*valp);
                else if (op=='-')
                  *valp = -*valp;
                else if (op=='~')
                  InvldOp(NULL,'~',&curtype);
                else
                  exit(1);
              }
              break;
            default:
              Error("RdPrExpr: Unknown constant type");
              exit(1);
          }
        }
        else {
          if (op==T_PLPL || op==T_MIMI) {
            if (IsSUPtr(&curvalue.val_type))
              CPtrCast();
          }
          AddPreOp(op);
        }
      }
      curislv = 0;
    }
    else {
      RdPoExpr();
      break;
    }
    break;
  }
}

WrPrExpr(e)
char *e;
{
  int op = *(int *)e;
  if (op=='(' || op=='L') {
    BinExpr *be = (BinExpr *)e;
    PrintTok('(');
    if (op=='(')
      WrXDecl((VDecl *)be->exb_t1,0);
    else
      WrLXDecl((VDecl *)be->exb_t1);
    PrintTok(')');
    WrPrExpr(be->exb_t2);
  }
  else if (op==T_SIZEOF)
    Error("Bug... sizeof got through");
  else if ((op=='*' || op=='&' || IsPREOP(op)) &&
           ((BinExpr *)e)->exb_t1==NULL) {
    BinExpr *be = (BinExpr *)e;
    PrintTok(op);
    WrPrExpr(be->exb_t2);
  }
  else
    WrPoExpr(e);
}

int SamUQTyp(decl1,decl2)
VDecl *decl1,*decl2;
{
  int lconst = IsConst(decl1);
  int rconst = IsConst(decl2);
  int issame;
  MakConst(decl1,FALSE);
  MakConst(decl2,FALSE);
  issame = IsSamTyp(decl1,decl2);
  MakConst(decl1,lconst);
  MakConst(decl2,rconst);
  return issame;
}

static VDecl *UnfyPtr(decl1,decl2)
VDecl *decl1,*decl2;
{
  VDecl ldecl;
  VDecl rdecl;
  ldecl.vd_tspec = decl1->vd_tspec;
  ldecl.vd_type = decl1->vd_type->tl_next;
  rdecl.vd_tspec = decl2->vd_tspec;
  rdecl.vd_type = decl2->vd_type->tl_next;
  if (!SamUQTyp(&ldecl,&rdecl)) {
    if (IsVoid(&ldecl)) {
      if (IsConst(&ldecl) || !IsConst(&rdecl))
        return decl1;
    }
    else if (IsVoid(&rdecl)) {
      if (IsConst(&rdecl) || !IsConst(&ldecl))
        return decl2;
    }
    else
      return NULL;
    return &cvpdecl;
  }
  if (IsConst(&ldecl))
    return decl1;
  return decl2;
}

static VDecl *UnfyType(lval,rval)
Value *lval,*rval;
{
  VDecl ldecl;
  VDecl rdecl;
  ExpsType(&lval->val_type,&ldecl);
  ExpsType(&rval->val_type,&rdecl);
  if (ldecl.vd_type!=NULL && IsZero(rval->val_expr))
    return &lval->val_type;
  if (rdecl.vd_type!=NULL && IsZero(lval->val_expr))
    return &rval->val_type;
  if ((ldecl.vd_type!=NULL) != (rdecl.vd_type!=NULL))
    return NULL;
  if (ldecl.vd_type==NULL) {
    if (IsNumTyp(ldecl.vd_tspec->ts_type) && 
        IsNumTyp(rdecl.vd_tspec->ts_type))
      return UnfyNTyp(&lval->val_type,&rval->val_type);
    else if (IsSamTyp(&ldecl,&rdecl))
      return &lval->val_type;
  }
  else
    return UnfyPtr(&ldecl,&rdecl);
}

static RdQCExpr()
{
  RdOOExpr();
  if (curtok=='?') {
    TerExpr *te = NewTExpr();
    te->ext_op = curtok;
    te->ext_t1 = curexpr;
    SkipTok();
    RdExpr();
    {
      Value lval;
      MakRVal(&curvalue);
      CpyValue(&lval,&curvalue);
      te->ext_t2 = curexpr;
      Skip(':');
      RdQCExpr();
      MakRVal(&curvalue);
      {
        VDecl *newtype = UnfyType(&lval,&curvalue);
        if (newtype==NULL)
          InvldOp(&lval.val_type,'?',&curvalue.val_type);
        else {
          curvalue.val_type.vd_type = newtype->vd_type;
          curvalue.val_type.vd_tspec = newtype->vd_tspec;
          curvalue.val_type.vd_var = NULL;
          te->ext_t3 = curexpr;
          curexpr = (char *)te;
          curislv = 0;
          curvalue.val_isce = 0;
        }
      }
    }
  }
}

static WrQCExpr(e)
char *e;
{
#define op (*(int *)e)
  if (op=='?') {
    TerExpr *te = (TerExpr *)e;
    WrOOExpr(te->ext_t1);
    PrintTok('?');
    WrAsExpr(te->ext_t2);
    PrintTok(':');
    WrQCExpr(te->ext_t3);
  }
  else
    WrOOExpr(e);
#undef op
}

int IsSamTyp(otype1,otype2)
VDecl *otype1;
VDecl *otype2;
{
  VDecl type1;
  VDecl type2;
  type1.vd_type = otype1->vd_type;
  type1.vd_tspec = otype1->vd_tspec;
  type2.vd_type = otype2->vd_type;
  type2.vd_tspec = otype2->vd_tspec;
  for (;;) {
    ExpsType(&type1,&type1);
    ExpsType(&type2,&type2);
    if ((type1.vd_type==NULL) != (type2.vd_type==NULL))
      return FALSE;
    if (type1.vd_type==NULL)
      break;
    if (type1.vd_type->tl_op!=type2.vd_type->tl_op)
      return FALSE;
    if (type1.vd_type->tl_op=='[') {
      if (type1.vd_type->tl_dat.tld_num!=type2.vd_type->tl_dat.tld_num)
        return FALSE;
    }
    else if (type1.vd_type->tl_op=='(') {
      TLLst *ptypes1 = type1.vd_type->tl_dat.tld_plst->pl_type;
      TLLst *ptypes2 = type2.vd_type->tl_dat.tld_plst->pl_type;
      for (;;) {
        VDecl pdecl1;
        VDecl pdecl2;

        if ((ptypes1==NULL) != (ptypes2==NULL))
          return FALSE;
        if ((ptypes1==(TLLst *)1) != (ptypes2==(TLLst *)1))
          return FALSE;
        if (ptypes1==NULL || ptypes1==(TLLst *)1)
          break;
        pdecl1.vd_type = ptypes1->tll_type;
        pdecl1.vd_tspec = &ptypes1->tll_tspec;
        pdecl2.vd_type = ptypes2->tll_type;
        pdecl2.vd_tspec = &ptypes2->tll_tspec;
        if (!IsSamTyp(&pdecl1,&pdecl2))
          return FALSE;
        ptypes1 = ptypes1->tll_next;
        ptypes2 = ptypes2->tll_next;
      }
    }
    type1.vd_type = type1.vd_type->tl_next;
    type2.vd_type = type2.vd_type->tl_next;
  }
  {
    TSpec *tspec1 = type1.vd_tspec;
    TSpec *tspec2 = type2.vd_tspec;
    char t1 = tspec1->ts_type;
    char t2 = tspec2->ts_type;
    if (t1!=t2)
      return FALSE;
    if (tspec1->ts_const != tspec2->ts_const)
      return FALSE;
    if (t1==T_STRUCT-256 || t1==T_UNION-256 || t1==T_ENUM-256) {
      StrctRef *sref1 = tspec1->ts_xdat.ts_sref;
      StrctRef *sref2 = tspec2->ts_xdat.ts_sref;
      VDecl *vdecl1;
      VDecl *vdecl2;
      if (sref1->sr_dorr==SR_DEF)
        vdecl1 = sref1->sr_drdat.sr_def;
      else
        vdecl1 = sref1->sr_drdat.sr_ref->sd_flst;
      if (sref2->sr_dorr==SR_DEF)
        vdecl2 = sref2->sr_drdat.sr_def;
      else
        vdecl2 = sref2->sr_drdat.sr_ref->sd_flst;
      if (vdecl1!=vdecl2)
        return FALSE;
    }
    else {
      if (t1==T_CHAR-256 || t1==T_INT-256) {
        if (tspec1->ts_xdat.ts_siln.ts_sign!=
            tspec2->ts_xdat.ts_siln.ts_sign)
          return FALSE;
        if (t1==T_INT-256) {
          if (tspec1->ts_xdat.ts_siln.ts_len!=
              tspec2->ts_xdat.ts_siln.ts_len)
            return FALSE;
        }
      }
    }
  }
  return TRUE;
}

int IsConst(decl)
VDecl *decl;
{
  if (decl->vd_type==NULL)
    return decl->vd_tspec->ts_const;
  if (decl->vd_type->tl_op=='*' && decl->vd_type->tl_dat.tld_cons)
    return 1;
  if (decl->vd_type->tl_op=='[' || decl->vd_type->tl_op=='(')
    return 1;
  return 0;
}

MakConst(decl,val)
VDecl *decl;
int val;
{
  if (decl->vd_type==NULL)
    decl->vd_tspec->ts_const = val;
  else if (decl->vd_type->tl_op=='*')
    decl->vd_type->tl_dat.tld_cons = val;
}

static int IsVoid(decl)
VDecl *decl;
{
  return (decl->vd_type==NULL && decl->vd_tspec->ts_type==T_VOID-256);
}

int CanAssgn(oltype,ortype,oexpr)
VDecl *oltype;
VDecl *ortype;
char *oexpr;
{
  VDecl ltype,rtype;
  ExpsType(oltype,&ltype);
  ExpsType(ortype,&rtype);
  if (IsConst(&ltype))
    return FALSE;
  if (ltype.vd_type!=NULL && IsZero(oexpr))
    return TRUE;
  if ((ltype.vd_type!=NULL) != (rtype.vd_type!=NULL))
    return FALSE;
  if (ltype.vd_type==NULL) {
    if (IsNumTyp(ltype.vd_tspec->ts_type) && 
        IsNumTyp(rtype.vd_tspec->ts_type))
      return UnfyNTyp(&ltype,&rtype)!=NULL;
    else if (IsSamTyp(&ltype,&rtype))
      return TRUE;
  }
  else
    return UnfyPtr(&ltype,&rtype)!=NULL;
}

int CanInit(oltype,ortype,oexpr)
VDecl *oltype;
VDecl *ortype;
char *oexpr;
{
  int lconst = IsConst(oltype);
  int caninit;
  MakConst(oltype,FALSE);
  caninit = CanAssgn(oltype,ortype,oexpr);
  MakConst(oltype,lconst);
  return caninit;
}

RdAsExpr()
{
  RdQCExpr();
  if (IsASNOP(curtok)) {
    BinExpr *be = (BinExpr *)StkAlloc(sizeof(BinExpr));
    if (curtok==T_PLSEQ || curtok==T_MINEQ) {
      if (IsSUPtr(&curvalue.val_type))
        CPtrCast();
    }
    be->exb_op = curtok;
    SkipTok();
    be->exb_t1 = curvalue.val_expr;
    if (!curislv)
      Error("assignment to rvalue");
    {
      VDecl ltype,rtype;
      int valid = 0;
      ChkDef(&curvalue.val_type);
      ExpsType(&curvalue.val_type,&ltype);
      RdAsExpr();
      ChkDef(&curvalue.val_type);
      MakRVal(&curvalue);
      rtype.vd_type = curvalue.val_type.vd_type;
      rtype.vd_tspec = curvalue.val_type.vd_tspec;
      if (be->exb_op==T_PLSEQ || be->exb_op==T_MINEQ) {
        VDecl *newtype = CanAdd(&ltype,&rtype);
        if (newtype!=NULL) {
          if (CanAssgn(&ltype,newtype,NULL))
            valid = 1;
        }
      }
      else if (CanAssgn(&ltype,&rtype,curvalue.val_expr)) {
        valid = 1;
        if (IsSorU(rtype.vd_type,rtype.vd_tspec)) {
          Type *strctptr = PtrTo(curvalue.val_type.vd_type);
          ExpLst *mcparm = NewExLst();
          ExpLst *mcparm2 = NewExLst();
          ExpLst *mcparm3 = NewExLst();
          mcparm->el_exp = (char *)NewBExpr('&',NULL,be->exb_t1);
          mcparm2->el_exp = (char *)NewBExpr('&',NULL,curvalue.val_expr);
          mcparm3->el_exp = IntExpr(SizeOf(&curvalue.val_type));
          mcparm->el_next = mcparm2;
          mcparm2->el_next = mcparm3;
          mcparm3->el_next = NULL;
          curvalue.val_type.vd_type = strctptr;
          curvalue.val_expr = 
            (char *)NewBExpr(')',NewUExpr(-1,"memcpy"),mcparm);
          CurrCast();
          be->exb_op = '*';
          be->exb_t1 = NULL;
          be->exb_t2 = curvalue.val_expr;
          curvalue.val_expr = NULL;
        }
      }
      if (!valid)
        InvldOp(&ltype,be->exb_op,&rtype);
      curvalue.val_type.vd_type = ltype.vd_type;
      curvalue.val_type.vd_tspec = ltype.vd_tspec;
    }
    if (curvalue.val_expr!=NULL)
      be->exb_t2 = curvalue.val_expr;
    curvalue.val_expr = (char *)be;
    curvalue.val_islv = 0;
    curvalue.val_isce = 0;
  }
}

WrAsExpr(e)
char *e;
{
#define op (*(int *)e)
  if (IsASNOP(op)) {
    BinExpr *be = (BinExpr *)e;
    WrQCExpr(be->exb_t1);
    PrintTok(op);
    WrAsExpr(be->exb_t2);
  }
  else
    WrQCExpr(e);
#undef op
}

RWAsExpr()
{
  RdAsExpr();
  WrAsExpr(curexpr);
}

static RdCExpr()
{
  RdAsExpr();
  while (curtok==',')
    RdBExpr(RdAsExpr);
}

RdExpr()
{
  RdCExpr();
}

static WrCExpr(e)
char *e;
{
#define op (*(int *)e)
  if (op==',') {
    BinExpr *be = (BinExpr *)e;
    WrCExpr(be->exb_t1);
    PrintTok(op);
    e = be->exb_t2;
  }
  WrAsExpr(e);
#undef op
}

WrExpr(e)
char *e;
{
  static char *last = 0;
  if (e==(char *)0)
    return;
  if (e==last) {
    Error("Recursive op");
    exit(1);
  }
  last = e;
  WrCExpr(e);
  last = 0;
}

RWExpr()
{
  RdExpr();
  WrExpr(curexpr);
}
