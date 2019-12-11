#ifndef CoCo
#include <stdlib.h>
#endif
#include "ansifront.h"

VDecl *UnfyNTyp();
static VDecl *UnfyITyp();

static double EToNum(expr)
char *expr;
{
  switch (*(int *)expr) {
    case T_INTNUM:
      return (double)EIntVal(expr);
      break;
    case T_LNGNUM:
      return (double)ELngVal(expr);
      break;
    case T_NUMBER:
      return ENumVal(expr);
      break;
    default:
      Error("Bad ltype");
      exit(1);
  }
}

static long EToLng(expr)
char *expr;
{
  switch (*(int *)expr) {
    case T_INTNUM:
      return (long)EIntVal(expr);
      break;
    case T_LNGNUM:
      return ELngVal(expr);
      break;
    default:
      Error("EToLng: bad type");
      exit(1);
  }
}

DoCBExpr(lval,op,rval)
Value *lval;
int op;
Value *rval;
{
  int ltype = *(int *)lval->val_expr;
  int rtype = *(int *)rval->val_expr;
  if (ltype==T_INTNUM && rtype==T_INTNUM) {
    int leftnum = EIntVal(lval->val_expr);
    int rightnum = EIntVal(rval->val_expr);
    switch (op) {
      case '+':
        rightnum = leftnum+rightnum;
        break;
      case '-':
        rightnum = leftnum-rightnum;
        break;
      case '*':
        rightnum = leftnum*rightnum;
        break;
      case '/':
        rightnum = leftnum/rightnum;
        break;
      case '%':
        rightnum = leftnum%rightnum;
        break;
      case '&':
        rightnum = leftnum&rightnum;
        break;
      case '|':
        rightnum = leftnum|rightnum;
        break;
      case '^':
        rightnum = leftnum^rightnum;
        break;
      case T_SHIFTR:
        rightnum = leftnum>>rightnum;
        break;
      case T_SHIFTL:
        rightnum = leftnum<<rightnum;
        break;
      case T_LT:
        rightnum = leftnum < rightnum;
        break;
      case T_GT:
        rightnum = leftnum > rightnum;
        break;
      case T_LTEQ:
        rightnum = leftnum <= rightnum;
        break;
      case T_GTEQ:
        rightnum = leftnum >= rightnum;
        break;
      case T_EQEQ:
        rightnum = leftnum == rightnum;
        break;
      case T_NOTEQ:
        rightnum = leftnum != rightnum;
        break;
      default:
        Error("DoCBExpr: Unknown operator");
        exit(1);
    }
    EAsInt(rval->val_expr,rightnum);
  }
  else if (ltype==T_NUMBER || rtype==T_NUMBER) {
    double leftnum;
    double rightnum;
    char *lexpr = lval->val_expr;
    char *rexpr = rval->val_expr;
    leftnum = EToNum(lexpr);
    rightnum = EToNum(rexpr);
    switch (op) {
      case '+':
        rightnum = leftnum+rightnum;
        break;
      case '-':
        rightnum = leftnum-rightnum;
        break;
      case '*':
        rightnum = leftnum*rightnum;
        break;
      case '/':
        rightnum = leftnum/rightnum;
        break;
      default:
        Error("DoCBExpr: Unknown operator");
        exit(1);
    }
    {
      char *rt;
      if (rtype!=T_NUMBER)
        ((UnryExpr *)rval->val_expr)->exu_t = rt = ((UnryExpr *)lexpr)->exu_t;
      else
        rt = ((UnryExpr *)rexpr)->exu_t;
      *(int *)rexpr = T_NUMBER;
      *(double *)rt = rightnum;
    }
  }
  else if (ltype==T_LNGNUM || rtype==T_LNGNUM) {
    long leftnum = EToLng(lval->val_expr);
    long rightnum = EToLng(rval->val_expr);
    switch (op) {
      case '+':
        rightnum = leftnum+rightnum;
        break;
      case '-':
        rightnum = leftnum-rightnum;
        break;
      case '*':
        rightnum = leftnum*rightnum;
        break;
      case '/':
        rightnum = leftnum/rightnum;
        break;
      case '%':
        rightnum = leftnum%rightnum;
        break;
      case '&':
        rightnum = leftnum&rightnum;
        break;
      case '|':
        rightnum = leftnum|rightnum;
        break;
      case '^':
        rightnum = leftnum^rightnum;
        break;
      case T_SHIFTR:
        rightnum = leftnum>>rightnum;
        break;
      case T_SHIFTL:
        rightnum = leftnum<<rightnum;
        break;
      default:
        Error("DoCBExpr: Unknown operator");
        exit(1);
    }
    {
      UnryExpr *rexpr = (UnryExpr *)rval->val_expr;
      if (rtype!=T_LNGNUM) {
        rexpr->exu_t = ((UnryExpr *)lval->val_expr)->exu_t;
        rexpr->exu_op = T_LNGNUM;
      }
      EAsLng((char *)rexpr,rightnum);
    }
  }
  else {
    Error("Unknown constant expr");
    exit(1);
  }
}

VDecl *CanIOp(ldecl,rdecl)
VDecl *ldecl;
VDecl *rdecl;
{
  if (ldecl->vd_type!=NULL || rdecl->vd_type!=NULL)
    return NULL;
  if (ldecl->vd_tspec->ts_type!=T_INT-256 ||
      rdecl->vd_tspec->ts_type!=T_INT-256)
    return NULL;
  return UnfyITyp(ldecl,rdecl);
}

VDecl *CanArith(ldecl,rdecl)
VDecl *ldecl;
VDecl *rdecl;
{
  if (ldecl->vd_type!=NULL || rdecl->vd_type!=NULL)
    return NULL;
  if (!IsNumTyp(ldecl->vd_tspec->ts_type) ||
      !IsNumTyp(rdecl->vd_tspec->ts_type))
    return NULL;
  return UnfyNTyp(ldecl,rdecl);
}

VDecl *CanAdd(ldecl,rdecl)
VDecl *ldecl;
VDecl *rdecl;
{
  if (ldecl->vd_type!=NULL && ldecl->vd_type->tl_op=='*') {
    if (rdecl->vd_type==NULL && rdecl->vd_tspec->ts_type==T_INT-256)
      return ldecl;
  }
  if (rdecl->vd_type!=NULL && rdecl->vd_type->tl_op=='*') {
    if (ldecl->vd_type==NULL && ldecl->vd_tspec->ts_type==T_INT-256)
      return rdecl;
  }
  return CanArith(ldecl,rdecl);
}

VDecl *CanSub(ldecl,rdecl)
VDecl *ldecl;
VDecl *rdecl;
{
  int lisptr = IsPtr(ldecl);
  int risptr = IsPtr(rdecl);
  if (lisptr) {
    if (risptr && IsSamPtr(ldecl,rdecl))
      return &intdecl;
    if (rdecl->vd_type==NULL && rdecl->vd_tspec->ts_type==T_INT-256)
      return ldecl;
    return NULL;
  }
  return CanArith(ldecl,rdecl);
}

int IsNumTyp(t)
int t;
{
  return (t==T_CHAR-256 || t==T_INT-256 || t==T_FLOAT-256 || 
          t==T_DOUBLE-256 || t==T_ENUM-256);
}

int IsZero(expr)
char *expr;
{
  if (expr==NULL)
    return 0;
  switch (*(int *)expr) {
    case T_INTNUM:
      return EIntVal(expr)==0;
    case T_LNGNUM:
      return ELngVal(expr)==0;
  }
  return 0;
}

int IsSamPtr(ldecl,rdecl)
VDecl *ldecl;
VDecl *rdecl;
{
  VDecl nldecl;
  VDecl nrdecl;
  nldecl.vd_type = ldecl->vd_type->tl_next;
  nldecl.vd_tspec = ldecl->vd_tspec;
  nrdecl.vd_type = rdecl->vd_type->tl_next;
  nrdecl.vd_tspec = rdecl->vd_tspec;
  return IsSamTyp(&nldecl,&nrdecl);
}

int CanCmp(lval,rval)
Value *lval;
Value *rval;
{
  VDecl ldecl;
  VDecl rdecl;
  ExpsType(&lval->val_type,&ldecl);
  ExpsType(&rval->val_type,&rdecl);
  if (ldecl.vd_type!=NULL && IsZero(rval->val_expr))
    return 1;
  if (rdecl.vd_type!=NULL && IsZero(lval->val_expr))
    return 1;
  if ((ldecl.vd_type==NULL) != (rdecl.vd_type==NULL))
    return 0;
  if (ldecl.vd_type!=NULL) {
    VDecl lndecl;
    VDecl rndecl;
    lndecl.vd_type = ldecl.vd_type->tl_next;
    lndecl.vd_tspec = ldecl.vd_tspec;
    rndecl.vd_type = rdecl.vd_type->tl_next;
    rndecl.vd_tspec = rdecl.vd_tspec;
    return SamUQTyp(&lndecl,&rndecl);
  }
  if (!IsNumTyp(ldecl.vd_tspec->ts_type))
    return 0;
  if (!IsNumTyp(rdecl.vd_tspec->ts_type))
    return 0;
  return 1;
}

int IsCond(decl)
VDecl *decl;
{
  if (decl->vd_type!=NULL)
    return 1;
  if (IsNumTyp(decl->vd_tspec->ts_type))
    return 1;
  return 0;
}

int IsSUPtr(decl)
VDecl *decl;
{
  VDecl tmpdecl;
  ExpsType(decl,&tmpdecl);
  if (tmpdecl.vd_type==NULL || tmpdecl.vd_type->tl_op!='*')
    return 0;
  tmpdecl.vd_type = tmpdecl.vd_type->tl_next;
  return NeedSize(&tmpdecl);
}

RdBExpr(rdfn)
int (*rdfn)();
{
  Value leftval;
  int op = curtok;
  MakRVal(&curvalue);
  if (IsSUPtr(&curvalue.val_type))
    CurrCast();
  CpyValue(&leftval,&curvalue);
  SkipTok();
  (*rdfn)();
  MakRVal(&curvalue);
  if (IsSUPtr(&curvalue.val_type))
    CurrCast();
  if (leftval.val_isce && curvalue.val_isce) {
    DoCBExpr(&leftval,op,&curvalue);
  }
  else {
    VDecl *newtype = NULL;
    if (op=='+')
      newtype = CanAdd(&leftval.val_type,&curvalue.val_type);
    else if (op=='-')
      newtype = CanSub(&leftval.val_type,&curvalue.val_type);
    else if (op==T_LT || op==T_GT || op==T_EQEQ || op==T_LTEQ || op==T_GTEQ ||
             op==T_NOTEQ) {
      if (CanCmp(&leftval,&curvalue))
        newtype = &intdecl;
    }
    else if (op==T_OROR || op==T_ANDAND) {
      if (IsCond(&leftval.val_type) && IsCond(&curvalue.val_type))
        newtype = &intdecl;
    }
    else if (op=='*' || op=='/')
      newtype = CanArith(&leftval.val_type,&curvalue.val_type);
    else if (op=='%' || op=='|' || op=='&' || op=='^' || 
             op==T_SHIFTR || op==T_SHIFTL)
      newtype = CanIOp(&leftval.val_type,&curvalue.val_type);
    else if (op==',')
      newtype = &curvalue.val_type;
    if (newtype==NULL)
      InvldOp(&leftval.val_type,op,&curvalue.val_type);
    else {
      curvalue.val_type.vd_type = newtype->vd_type;
      curvalue.val_type.vd_tspec = newtype->vd_tspec;
    }
    {
      BinExpr *be = NewBExpr(op,leftval.val_expr,curvalue.val_expr);
      curvalue.val_expr = (char *)be;
      curvalue.val_isce = 0;
      curvalue.val_islv = 0;
    }
  }
}

static int FindOp(op,oplst)
int op;
int *oplst;
{
  while (*oplst) {
    if (op==*oplst)
      return 1;
    ++oplst;
  }
}

static WrBExpr(e,ops,subfn)
char *e;
int *ops;
int (*subfn)();
{
#define op (*(int *)e)
  if (FindOp(op,ops)) {
    if (((BinExpr *)e)->exb_t1!=NULL && ((BinExpr *)e)->exb_t2!=NULL) {
      WrBExpr(((BinExpr *)e)->exb_t1,ops,subfn);
      PrintTok(op);
      e = ((BinExpr *)e)->exb_t2;
    }
  }
  (*subfn)(e);
#undef op
}

static RdMDExpr()
{
  RdPrExpr();
  while (curtok=='*' || curtok=='/' || curtok=='%')
    RdBExpr(RdPrExpr);
}

static WrMDExpr(e)
char *e;
{
  static int ops[] = {'*','/','%',0};
  WrBExpr(e,ops,WrPrExpr);
}

static int LenCmp(len1,len2)
int len1;
int len2;
{
  if (len1==len2)
    return 0;
  if (len1==T_SHORT-256)
    return -1;
  if (len2==T_SHORT-256)
    return 1;
  if (len1==TS_NONE)
    return -1;
  if (len2==TS_NONE)
    return 1;
  Error("Bug in LenCmp");
  exit(1);
}

static int SignCmp(sign1,sign2)
int sign1;
int sign2;
{
  if (sign1==sign2)
    return 0;
  if ((sign1==T_SIGNED && sign2==TS_NONE) ||
      (sign2==T_SIGNED && sign1==TS_NONE))
    return 0;
  if (sign1==T_SIGNED)
    return 1;
  return -1;
}

static VDecl *UnfyITyp(decl1,decl2)
VDecl *decl1;
VDecl *decl2;
{
  int lencmp = 
    LenCmp(
      decl1->vd_tspec->ts_xdat.ts_siln.ts_len,
      decl2->vd_tspec->ts_xdat.ts_siln.ts_len
    );
  if (lencmp<0)
    return decl2;
  else if (lencmp>0)
    return decl1;
  else {
    int sgncmp =
      SignCmp(
        decl1->vd_tspec->ts_xdat.ts_siln.ts_sign,
        decl2->vd_tspec->ts_xdat.ts_siln.ts_sign
      );
    if (sgncmp<0)
      return decl2;
    else
      return decl1;
  }
}

VDecl *UnfyNTyp(decl1,decl2)
VDecl *decl1;
VDecl *decl2;
{
  if (decl1->vd_tspec->ts_type==decl2->vd_tspec->ts_type) {
    if (decl1->vd_tspec->ts_type==T_INT-256)
      return UnfyITyp(decl1,decl2);
    else
      return decl1;
  }
  else {
    if (decl1->vd_tspec->ts_type==T_CHAR)
      return decl2;
    if (decl2->vd_tspec->ts_type==T_CHAR)
      return decl1;
    if (decl1->vd_tspec->ts_type==T_INT)
      return decl2;
    else
      return decl1;
  }
}

static RdPMExpr()
{
  RdMDExpr();
  while (curtok=='+' || curtok=='-')
    RdBExpr(RdMDExpr);
}

static WrPMExpr(e)
char *e;
{
  static int ops[] = {'+','-',0};
  WrBExpr(e,ops,WrMDExpr);
}

static RdShExpr()
{
  RdPMExpr();
  while (curtok==T_SHIFTR || curtok==T_SHIFTL)
    RdBExpr(RdPMExpr);
}

static WrShExpr(e)
char *e;
{
  static int ops[] = {T_SHIFTL,T_SHIFTR,0};
  WrBExpr(e,ops,WrPMExpr);
}

static RdCpExpr()
{
  RdShExpr();
  while (IsCOMP(curtok))
    RdBExpr(RdShExpr);
}

static WrCpExpr(e)
char *e;
{
  int op = *(int *)e;
  if (IsCOMP(op)) {
    BinExpr *be = (BinExpr *)e;
    WrCpExpr(be->exb_t1);
    PrintTok(op);
    e = be->exb_t2;
  }
  WrShExpr(e);
}

static RdEqExpr()
{
  RdCpExpr();
  while (curtok==T_EQEQ || curtok==T_NOTEQ)
    RdBExpr(RdCpExpr);
}

static WrEqExpr(e)
char *e;
{
  static int ops[]={T_EQEQ,T_NOTEQ,0};
  WrBExpr(e,ops,WrCpExpr);
}

static RdAExpr()
{
  RdEqExpr();
  while (curtok=='&')
    RdBExpr(RdEqExpr);
}

static WrAExpr(e)
char *e;
{
  static int ops[]={'&',0};
  WrBExpr(e,ops,WrEqExpr);
}

static RdCtExpr()
{
  RdAExpr();
  while (curtok=='^')
    RdBExpr(RdAExpr);
}

static WrCtExpr(e)
char *e;
{
  static int ops[]={'^',0};
  WrBExpr(e,ops,WrAExpr);
}

static RdOExpr()
{
  RdCtExpr();
  while (curtok=='|')
    RdBExpr(RdCtExpr);
}

static WrOExpr(e)
char *e;
{
  static int ops[]={'|',0};
  WrBExpr(e,ops,WrCtExpr);
}

static RdAAExpr()
{
  RdOExpr();
  while (curtok==T_ANDAND)
    RdBExpr(RdOExpr);
}

static WrAAExpr(e)
char *e;
{
  static int ops[]={T_ANDAND,0};
  WrBExpr(e,ops,WrOExpr);
}

RdOOExpr()
{
  RdAAExpr();
  while (curtok==T_OROR)
    RdBExpr(RdAAExpr);
}

WrOOExpr(e)
char *e;
{
  static int ops[]={T_OROR,0};
  WrBExpr(e,ops,WrAAExpr);
}
