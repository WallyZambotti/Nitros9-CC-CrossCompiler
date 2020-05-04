#include "ansifront.h"
#ifndef CoCo
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#else
#include <lowio.h>
#endif

typedef struct fdeflst {
  VDecl *fdl_decl;
  struct fdeflst *fdl_prev;
} FDefLst;

static FDefLst *fdeflst;

static Type **RdLIDecl(/*vdecl*/);

extern char fldname;
extern char *KeepTI();
static char savtmp; /* >0 if reading parameter list */
int ntblfile = -1; /* File number for name table */

static ChkRedef(VDecl*);
static RdInit(Type*,TSpec*,char);

static WrPLst(plstp,fndecl)
PLst *plstp;
VDecl *fndecl;
{
  ILst  *ilp  = plstp->pl_ident;
  TLLst *tlp  = plstp->pl_type;
  if (outstrm==1) {
    if (IsSorU(fndecl->vd_type->tl_next,fndecl->vd_tspec)) {
      WrStr("_tvret");
      if (ilp!=NULL)
        WrChar(',');
    }
  }
  else {
    if (tlp==NULL) {
      WrStr("void");
      return;
    }
    if (tlp==(TLLst *)1)
      return;
    for (;;) {
      {
        VDecl pdecl;
        pdecl.vd_type = tlp->tll_type;
        pdecl.vd_tspec = &tlp->tll_tspec;
        WrTDecl(&pdecl);
      }
      if ((tlp = tlp->tll_next)==NULL)
        break;
      if (tlp==(TLLst *)1) {
        WrStr("...");
        break;
      }
      WrChar(',');
    }
    return;
  }
  while (ilp!=NULL) {
    if (outstrm==1) {
      if (tlp!=(TLLst *)1 && IsSorU(tlp->tll_type,&tlp->tll_tspec))
        WrEnIden("_tvp",tlp);
      else
        WrEncode(ilp->il_ident);
    }
    if (tlp!=(TLLst *)1 && tlp!=NULL)
      tlp=tlp->tll_next;
    ilp = ilp->il_next;
    if (ilp!=NULL)
      WrChar(',');
  }
}

WrTDecl(vdecl)
VDecl *vdecl;
{
  char *oldvar = vdecl->vd_var;
  vdecl->vd_var=NULL;
  WrVDecl(vdecl);
  vdecl->vd_var = oldvar;
}

static MSPDErr()
{
  Error("Mixed style parameter declarations");
}

RdPLst(plst)
PLst *plst;
{
  VDecl vdecl;
  char proto;
  ++savtmp;
  if (IsATYPE(curtok,curident))
    proto = 1;
  else if (curtok==T_IDENT) {
    proto = 0;
    plst->pl_type = (TLLst *)1;
  }
  else {
    plst->pl_type = (TLLst *)1;
    plst->pl_ident = NULL;
    --savtmp;
    return;
  }
  {
    TLLst **tllp = &plst->pl_type;
    ILst **ilp = &plst->pl_ident;
    TLLst *endtype = NULL;
    if (curtok==T_VOID && PeekTok()==')')
      SkipTok();
    else {
      for (;;) {
        TLLst *tllst = NewTLLst();
        ILst *ilst = (ILst *)TSAlloc(sizeof(ILst));
        if (IsATYPE(curtok,curident)) {
          if (!proto)
            MSPDErr();
          RdTSpec(&tllst->tll_tspec);
          tllst->tll_tspec.ts_sclas = TS_PARAM;
          RdIDecl(&vdecl);
          if (proto) {
            if (vdecl.vd_type!=NULL && vdecl.vd_type->tl_op=='[') {
              vdecl.vd_type->tl_op='*';
              vdecl.vd_type->tl_dat.tld_cons = 0;
            }
            tllst->tll_type = vdecl.vd_type;
            *tllp = tllst;
            tllp = &tllst->tll_next;
          }
          ilst->il_ident = vdecl.vd_var;
        }
        else if (curtok==T_IDENT) {
          if (proto)
            MSPDErr();
          ilst->il_ident = KeepTI();
          SkipTok();
        }
        else {
          Error("expecting type or identifier");
          break;
        }
        *ilp = ilst;
        ilp = &ilst->il_next;
        if (curtok==T_ELIPSE) {
          if (!proto)
            MSPDErr();
          SkipTok();
          endtype = (TLLst *)1;
          break;
        }
        if (curtok!=',')
          break;
        SkipTok();
        if (curtok==T_ELIPSE) {
          if (!proto)
            MSPDErr();
          SkipTok();
          endtype = (TLLst *)1;
          break;
        }
      }
    }
    if (proto)
      *tllp = endtype;
    *ilp = NULL;
  }
  --savtmp;
  if (savtmp<0) {
    Error("savtmp<0");
    exit(1);
  }
}

static NPASErr()
{
  Error("Non positive array size");
}

static Type **RdRIDecl(vdecl)
VDecl *vdecl;
{
  Type **sp;
  if (curtok=='A') {
    vdecl->vd_var = savtmp ? KeepTI() :  KeepIden();
    SkipTok();
    sp=&vdecl->vd_type;
  }
  else if (curtok=='(') {
    SkipTok();
    sp = RdLIDecl(vdecl);
    if (curtok!=')')
      Error("Mismatched ()'s");
    else
      SkipTok();
  }
  else {
    vdecl->vd_var = NULL;
    sp=&vdecl->vd_type;
  }
  for (;;) {
    if (curtok=='[') {
      SkipTok();
      *sp=NewTQ();
      (*sp)->tl_op = '[';
      (*sp)->tl_dat.tld_num = 0;
      if (curtok!=']') {
        RdExpr();
        if (!curvalue.val_isce)
          Error("non-constant expression used for array dimension");
        else if (!TokIsInt(*(int *)curvalue.val_expr))
          Error("non-integral array size");
        else {
          unsigned asize = 0;
          switch (*(int *)curvalue.val_expr) {
            case T_INTNUM:
              {
                int val = EIntVal(curvalue.val_expr);
                if (val<1)
                  NPASErr();
                else
                  asize = val;
              }
              break;
            case T_LNGNUM:
              {
                long val = ELngVal(curvalue.val_expr);
                if (val<1)
                  NPASErr();
                if (val>65535L)
                  Error("Array size too large");
                else
                  asize = val;
              }
              break;
            default:
              Error("Bug: Unknown int type");
              exit(1);
          }
          (*sp)->tl_dat.tld_num = asize;
        }
      }
      else {
        if (sp!=&vdecl->vd_type)
          Error("non primary dimension size omitted");
      }
      Skip(']');
    }
    else if (curtok=='(') {
      SkipTok();
      *sp=NewTQ();
      (*sp)->tl_op='(';
      {
        PLst *plst;
        (*sp)->tl_dat.tld_plst = plst = NewPLst();
        RdPLst(plst);
      }
      Skip(')');
    }
    else
      break;
    sp=&(*sp)->tl_next;
  }
  return sp;
}

static Type **RdLIDecl(vdecl)
VDecl *vdecl;
{
  if (curtok=='*') {
    Type *t = NewTQ();
    SkipTok();
    if ((t->tl_dat.tld_cons = (curtok==T_CONST))!=0)
      SkipTok();
    t->tl_op='*';
    return &(*RdLIDecl(vdecl)=t)->tl_next;
  }
  else
    return RdRIDecl(vdecl);
}

RdIDecl(vdecl)
VDecl *vdecl;
{
  *RdLIDecl(vdecl) = NULL;
}

RWIDecl(vdecl)
VDecl *vdecl;
{
  RdIDecl(vdecl);
  WrIDecl(vdecl,0);
}

RdVDecl(vdecl)
VDecl *vdecl;
{
  vdecl->vd_tspec=NewTSpec();
  RdTSpec(vdecl->vd_tspec);
  RdIDecl(vdecl);
}

static VDecl *RdDeclI(tspec)
TSpec *tspec;
{
  VDecl *vdecl = NewVDecl();
  vdecl->vd_tspec = tspec;
  RdIDecl(vdecl);
  if (vdecl->vd_var==NULL)
    Error("Missing variable name in declaration");
  else {
    if (!ChkRedef(vdecl)) {
      vdecl->vd_prev = varlst;
      varlst = vdecl;
    }
  }
  return vdecl;
}

static NoASzErr()
{
  Error("no array size or initializer given in non-external declaration");
}

static int IncmpArr(vdecl)
VDecl *vdecl;
{
  return vdecl->vd_type!=NULL && vdecl->vd_type->tl_op=='[' &&
         vdecl->vd_type->tl_dat.tld_num==0;
}

static VDecl *RdLDeclI(tspec)
TSpec *tspec;
{
  VDecl *vdecl = RdDeclI(tspec);
  if (vdecl->vd_tspec->ts_sclas==TS_NONE) {
    if (vdecl->vd_type!=NULL && vdecl->vd_type->tl_op=='(') {
      vdecl->vd_tspec =
        (TSpec *)memcpy(NewTSpec(),vdecl->vd_tspec,sizeof(TSpec));
      vdecl->vd_tspec->ts_sclas = T_EXTERN-256;
    }
    else
      vdecl->vd_tspec->ts_sclas = T_AUTO-256;
  }
  if (curtok!=T_EQUAL) {
    if (IncmpArr(vdecl)) {
      if (vdecl->vd_tspec->ts_sclas!=T_EXTERN-256 &&
          vdecl->vd_tspec->ts_sclas!=T_TYPEDE-256)
        NoASzErr();
    }
  }
  StoreNam(vdecl);
  return vdecl;
}

static InitErr(lvdecl,rvdecl)
VDecl *lvdecl,*rvdecl;
{
  BError();
  AError("Cannot initialize a ");
  ATError(lvdecl);
  AError(" with a ");
  ATError(rvdecl);
  EError();
}

int StrLen(str)
char *str;
{
  int size = 0;
  while (*str!=0) { /* WZ - NULL changed to 0 */
    if (*str=='\\')
      CharUVal(&str);
    else
      ++str;
    ++size;
  }
  return size;
}

static RdInitE(type,tspec)
Type *type;
TSpec *tspec;
{
  VDecl vdecl;
  vdecl.vd_type = type;
  vdecl.vd_tspec = tspec;
  RdAsExpr();
  MakRVal(&curvalue);
  if (!CanInit(&vdecl,&curvalue.val_type,curvalue.val_expr))
    InitErr(&vdecl,&curvalue.val_type);
}

static RdInit(type,tspec,first)
Type *type;
TSpec *tspec;
char first; /* 0 if nested */
{
  VDecl oldtype;
  VDecl newtype;
  State state;
  StoreStt(&state);
  oldtype.vd_type = type;
  oldtype.vd_tspec = tspec;
  ExpsType(&oldtype,&newtype);
  type = newtype.vd_type;
  tspec = newtype.vd_tspec;
  if (type!=NULL && type->tl_op=='[' && 
      (!first || curtok=='{' || 
       (type->tl_next==NULL && tspec->ts_type==T_CHAR-256))) {
    int size = type->tl_dat.tld_num;
    int count = 0;
    char delim;
    if (delim=(curtok=='{'))
      SkipTok();
    PrintTok('{');
    if (type->tl_next==NULL && tspec->ts_type==T_CHAR-256 && 
        curtok==T_STRING) {
      char *str = curident;
      int lenstr = StrLen(str);
      if (size>0) {
        if (lenstr>size)
          Error("string literal too long");
      }
      else
        count = lenstr+1;
      {
        char *p = str;
        while (*p!=NULL) {
          WrInt(CharUVal(&p));
          if (*p!=NULL)
            WrChar(',');
        }
        if (size==0)
          WrStr(",0");
      }
      SkipTok();
    }
    else {
      {
        for (;;) {
          RdInit(type->tl_next,tspec,0);
          ++count;
          if (curtok=='}')
            break;
          if (!delim && count==size)
            break;
          Expect(',');
          if (curtok=='}')
            break;
          if (count==size)
            Error("Too many initializers");
        }
      }
    }
    if (size==0)
      type->tl_dat.tld_num = count;
    if (delim)
      Expect('}');
    else
      PrintTok('}');
  }
  else if (IsSorU(type,tspec) && (!first || curtok=='{')) {
    char delim;
    if ((delim=(curtok=='{')))
      SkipTok();
    PrintTok('{');
    {
      VDecl *fields;
      StrctRef *sref = tspec->ts_xdat.ts_sref;
      if (sref->sr_dorr==SR_DEF)
        fields = sref->sr_drdat.sr_def;
      else
        fields = sref->sr_drdat.sr_ref->sd_flst;
      while (fields!=NULL) {
        RdInit(fields->vd_type,fields->vd_tspec,0);
        if (curtok=='}')
          break;
        if (!delim && fields->vd_prev==NULL)
          break;
        Expect(',');
        if (curtok=='}')
          break;
        fields = fields->vd_prev;
      }
      if (fields==NULL && delim && curtok!='}')
        Error("Too many initializers");
    }
    if (delim)
      Expect('}');
    else
      PrintTok('}');
  }
  else {
    RdInitE(type,tspec);
    WrAsExpr(curvalue.val_expr);
  }
  RetrvStt(&state);
}

int IsSURef(tspec)
TSpec *tspec;
{
  char type = tspec->ts_type;
  return ((type==T_STRUCT-256 || type==T_UNION-256) &&
          tspec->ts_xdat.ts_sref->sr_dorr==SR_REF);
}

int NeedSize(vdecl)
VDecl *vdecl;
{
  VDecl tmpvdecl;
  tmpvdecl.vd_tspec = vdecl->vd_tspec;
  tmpvdecl.vd_type = vdecl->vd_type;
  for (;;) {
    ExpsType(&tmpvdecl,&tmpvdecl);
    if (tmpvdecl.vd_type==NULL) {
      char type = tmpvdecl.vd_tspec->ts_type;
      return (type==T_STRUCT-256 || type==T_UNION-256);
    }
    if (tmpvdecl.vd_type->tl_op=='*')
      return 0;
    tmpvdecl.vd_type = tmpvdecl.vd_type->tl_next;
  }
}

static Type **FrstNeed(vdecl)
VDecl *vdecl;
{
  Type **tp = &vdecl->vd_type;
  for (;;) {
    VDecl tmpvdecl;
    tmpvdecl.vd_type = *tp;
    tmpvdecl.vd_tspec = vdecl->vd_tspec;
    if (NeedSize(&tmpvdecl))
      break;
    if (*tp==NULL)
      break;
    tp = &(*tp)->tl_next;
  }
  return tp;
}

static WrNTSpec(tspec)
TSpec *tspec;
{
  char type = tspec->ts_type;
  if (type==T_STRUCT-256 || type==T_UNION-256) {
    WrTQual(tspec);
    PrintTok(type+256);
    WrStTag(tspec->ts_xdat.ts_sref);
    WrChar(' ');
  }
  else
    WrTSpec(tspec);
}

static TSpec *BaseType(vdecl)
VDecl *vdecl;
{
  if (vdecl->vd_tspec->ts_type!=T_TDNAME-256)
    return vdecl->vd_tspec;
  {
    VDecl tmpvdecl;
    tmpvdecl.vd_tspec = vdecl->vd_tspec;
    for (;;) {
      tmpvdecl.vd_type = NULL;
      ExpsType(&tmpvdecl,&tmpvdecl);
      if (tmpvdecl.vd_tspec->ts_type!=T_TDNAME-256)
        return tmpvdecl.vd_tspec;
    }
  }
}

WrLXDecl(vdecl)
VDecl *vdecl;
{
  TSpec *base = BaseType(vdecl);
  WrTQual(vdecl->vd_tspec);
  PrintTok(base->ts_type+256);
  WrStTag(base->ts_xdat.ts_sref);
  WrChar(' ');
  WrXLIDec(vdecl);
  WrXRIDec(vdecl,0,NULL);
}

WrIDecl2(vdecl,isfdef)
VDecl *vdecl;
int isfdef;
{
  if (vdecl->vd_var==NULL && vdecl->vd_type!=NULL &&
      vdecl->vd_type->tl_op=='[') {
/* Change a cast to an array to a cast to a pointer */
    Type ptrtype;
    VDecl ptrdecl;
    ptrtype.tl_op='*';
    ptrtype.tl_dat.tld_cons = 0;
    ptrtype.tl_next = vdecl->vd_type->tl_next;
    ptrdecl.vd_type = &ptrtype;
    ptrdecl.vd_tspec = vdecl->vd_tspec;
    ptrdecl.vd_var = NULL;
    WrIDecl(&ptrdecl,0);
  }
  else
    WrIDecl(vdecl,isfdef);
}

WrXDecl(vdecl,isfdef)
VDecl *vdecl;
int isfdef;
{
  VDecl tsvdecl;
  tsvdecl.vd_type = NULL;
  tsvdecl.vd_tspec = vdecl->vd_tspec;
  if (NeedSize(&tsvdecl)) {
    if (NeedSize(vdecl)) {
      TSpec *base = BaseType(vdecl);
      StrctRef *sref = base->ts_xdat.ts_sref;
      VDecl *fields;
      if (sref->sr_dorr==SR_DEF)
        fields = sref->sr_drdat.sr_def;
      else
        fields = sref->sr_drdat.sr_ref->sd_flst;
      if (fields==(VDecl *)1) {
        BError();
        ATSError(vdecl->vd_tspec);
        WrStr(" is incomplete");
        EError();
      }
      if (vdecl->vd_type!=NULL && vdecl->vd_type->tl_op=='(') {
        PrintTok(T_CHAR);
        WrIDecl(vdecl,1);
      }
      else {
        WrTQual(vdecl->vd_tspec);
        PrintTok(base->ts_type+256);
        WrStTag(base->ts_xdat.ts_sref);
        WrChar(' ');
        if (vdecl->vd_var==NULL && vdecl->vd_type!=NULL && 
            vdecl->vd_type->tl_op=='[') {
/* Change a cast to an array to a cast to a pointer */
          Type ptrtype;
          VDecl ptrdecl;
          ptrtype.tl_op='*';
          ptrtype.tl_dat.tld_cons = 0;
          ptrtype.tl_next = vdecl->vd_type->tl_next;
          ptrdecl.vd_type = &ptrtype;
          ptrdecl.vd_tspec = vdecl->vd_tspec;
          WrXLIDec(&ptrdecl);
          WrXRIDec(&ptrdecl,0,NULL);
        }
        else {
          WrXLIDec(vdecl);
          WrVarNam(vdecl->vd_var,vdecl->vd_tspec->ts_sclas);
          WrXRIDec(vdecl,0,NULL);
        }
      }
    }
    else {
      Type **tp = FrstNeed(vdecl);
      Type *oldnext = *tp;
      *tp = NULL;
      WrTQual(vdecl->vd_tspec);
      PrintTok(T_CHAR);
      WrIDecl2(vdecl,isfdef);
      *tp = oldnext;
    }
  }
  else {
    WrNTSpec(vdecl->vd_tspec);
    WrIDecl2(vdecl,isfdef);
  }
}

static int WrLDeclI(vdecl)
VDecl *vdecl;
{
  if (vdecl->vd_tspec->ts_sclas==T_TYPEDE-256) {
    if (NeedSize(vdecl))
      return 0;
  }
  WrXDecl(vdecl,0);
  return 1;
}

int HasDef(tspec)
TSpec *tspec;
{
  char type = tspec->ts_type;
  if (type==T_STRUCT-256 || type==T_UNION-256) {
    char dorr = tspec->ts_xdat.ts_sref->sr_dorr;
    if (dorr!=SR_REF)
      return 1;
  }
  return 0;
}

static int RdLDecl()
{
  int scpdepth = 0;
  if (savtmp!=0) {
    Error("RdLDecl: savtmp!=0");
    exit(1);
  }
  TSReset();
  {
    int isfunct;
    TSpec *tspec = NewTSpec();
    RdTSpec(tspec);
    if (tspec->ts_direc && 
        !(tspec->ts_sclas==TS_NONE || tspec->ts_sclas==T_AUTO-256))
      Error("'direct' specified for auto variable");
    if (HasDef(tspec)) {
      WrStRef(tspec->ts_xdat.ts_sref,tspec->ts_type);
      WrStr(";\n");
    }
    if (curtok==';') {
      SkipTok();
      return 0;
    }
    {
      for (;;) {
        VDecl *vdecl = RdLDeclI(tspec);
        isfunct = (vdecl->vd_type!=NULL && vdecl->vd_type->tl_op=='(');
        {
          if (curtok==T_EQUAL) {
            SkipTok();
            if (isfunct)
              Error("Attempt to initialize function");
            if (vdecl->vd_tspec->ts_type==T_EXTERN-256)
              Error("External variable initialized in function scope\n");
            {
              VDecl xdecl;
              int inittype;
              ExpsType(vdecl,&xdecl);
              if (vdecl->vd_tspec->ts_sclas==TS_NONE ||
                  vdecl->vd_tspec->ts_sclas==T_AUTO-256 ||
                  vdecl->vd_tspec->ts_sclas==T_REGIST-256) {
                if (vdecl->vd_type==NULL) {
                  if (vdecl->vd_tspec->ts_type==T_UNION-256) {
                    Error("Unimplemented - Initialization of union\n");
                    RdInit(vdecl->vd_type,vdecl->vd_tspec,1);
                  }
                  else if (vdecl->vd_tspec->ts_type==T_STRUCT-256) {
                    if (curtok=='{')
                      inittype = 1;
                    else
                      inittype = 2;
                  }
                  else
                    inittype = 3;
                } 
                else if (vdecl->vd_type->tl_op=='[')
                  inittype = 1;
                else 
                  inittype = 3;
              }
              else
                inittype = 0;
              if (inittype==1) {
                {
                  VDecl tmpdecl;
                  char varname[9];
                  strcpy(varname,"_tvs");
                  SWr04x(varname+4,vdecl->vd_var);
                  tmpdecl.vd_var = varname;
                  tmpdecl.vd_type = vdecl->vd_type;
                  tmpdecl.vd_tspec = vdecl->vd_tspec;
                  WrStr("static ");
                  WrLDeclI(&tmpdecl);
                  WrChar('=');
                  RdInit(vdecl->vd_type,vdecl->vd_tspec,1);
                  WrStr(";\n");
                  WrLDeclI(vdecl);
                  WrStr(";\n");
                  WrStr("memcpy(");
                  {
                    int useaddr = 
                      !vdecl->vd_type || vdecl->vd_type->tl_op!='[';
                    if (useaddr)
                      WrChar('&');
                    WrStr(vdecl->vd_var);
                    WrChar(',');
                    if (useaddr)
                      WrChar('&');
                    WrStr(varname);
                  }
                  WrChar(',');
                  WrInt(SizeOf(vdecl));
                  WrStr(");\n");
                  WrStr("{\n");
                  ++scpdepth;
                }
              }
              else if (inittype==2) {
                WrLDeclI(vdecl);
                RdExpr();
                WrStr(";\n");
                WrTemp();
                WrStr("memcpy(&");
                WrStr(vdecl->vd_var);
                WrStr(",&(");
                WrExpr(curvalue.val_expr);
                WrStr("),");
                WrInt(SizeOf(vdecl));
                WrStr(");\n");
                if (tempsize>0)
                  WrStr("}\n");
                WrStr("{\n");
                ++scpdepth;
              }
              else if (inittype==3) {
                WrLDeclI(vdecl);
                RdInitE(vdecl->vd_type,vdecl->vd_tspec);
                if (tempsize>0) {
                  WrStr(";\n");
                  WrTemp();
                  WrStr(vdecl->vd_var);
                  WrChar('=');
                  WrAsExpr(curvalue.val_expr);
                  WrStr(";\n");
                  WrStr("}\n");
                  WrStr("{\n");
                  ++scpdepth;
                }
                else {
                  WrChar('=');
                  WrAsExpr(curvalue.val_expr);
                  WrStr(";\n");
                }
              }
              else {
                WrLDeclI(vdecl);
                WrChar('=');
                RdInit(vdecl->vd_type,vdecl->vd_tspec,1);
                WrStr(";\n");
              }
            }
          }
          else {
            if (WrLDeclI(vdecl))
              WrStr(";\n");
          }
          if (curtok==',')
            SkipTok();
          else if (curtok==';') {
            SkipTok();
            break;
          }
          else {
            Error("Expecting ',' or ';'");
            break;
          }
        }
      }
    }
  }
  return scpdepth;
}

int RdDeclLs()
{
  int scpdepth = 0;
  for (;;) {
    scpdepth += RdLDecl();
    if (!IsATYPE(curtok,curident))
      break;
  }
  return scpdepth;
}

WrVDecl(vdecl)
VDecl *vdecl;
{
  WrTSpec(vdecl->vd_tspec);
  WrIDecl(vdecl,0);
}

WrOSPLst(plst,fndecl)
PLst *plst;
VDecl *fndecl;
{
  TLLst *tlp = plst->pl_type;
  ILst *ilp = plst->pl_ident;
  if (fndecl->vd_type->tl_op=='(' && 
      IsSorU(fndecl->vd_type->tl_next,fndecl->vd_tspec)) {
    PrintTok(T_CHAR);
    WrStr("*_tvret;\n");
  }
  while (tlp!=NULL && tlp!=(TLLst *)1) {
    if (IsSorU(tlp->tll_type,&tlp->tll_tspec)) {
      PrintTok(T_CHAR);
      WrEnIden("_tvp",tlp);
    }
    else {
      VDecl vdecl;
      vdecl.vd_type = tlp->tll_type;
      vdecl.vd_var = ilp->il_ident;
      vdecl.vd_tspec = &tlp->tll_tspec;
      WrXDecl(&vdecl,0);
    }
#ifdef IGNORE
    WrTSpec(&tlp->tll_tspec);
    if (IsSorU(tlp->tll_type,&tlp->tll_tspec)) {
      WrChar('*');
      WrEnIden("_tvp",tlp);
    }
    else {
      WrLIDecl(tlp->tll_type);
      WrEncode(ilp->il_ident);
      WrRIDecl(tlp->tll_type,0,NULL);
    }
#endif /* IGNORE */
    WrStr(";\n");
    tlp = tlp->tll_next;
    ilp = ilp->il_next;
  }
}

VDecl *SrchDecl(name,scope)
char *name;
VDecl *scope;
{
  char name0 = name[0];
  VDecl *list = varlst;
  while (list!=scope) {
    if (list->vd_var[0]==name0 && strcmp(list->vd_var,name)==0)
      return list;
    list = list->vd_prev;
  }
  return NULL;
}

static RedecErr(varname)
char *varname;
{
  BError();
  AError("Redeclaration of '");
  AError(varname);
  AError("'");
  EError();
}

static DifTypEr(pdecl,cdecl)
VDecl *pdecl;
VDecl *cdecl;
{
  BError();
  AError("Previous declaration of '");
  AError(cdecl->vd_var);
  AError("' was type ");
  WrTDecl(pdecl);
  AError(" but is now ");
  WrTDecl(cdecl);
  EError();
}

static int IsFDefd(fname)
char *fname;
{
  FDefLst *fdlp = fdeflst;
  while (fdlp!=NULL) {
    if (strcmp(fdeflst->fdl_decl->vd_var,fname)==0)
      return 1;
    fdlp = fdlp->fdl_prev;
  }
}

static ChkRedef(decl)
VDecl *decl;
{
  VDecl *match = SrchDecl(decl->vd_var,varscope);
  if (match!=NULL) {
    if (varscope!=NULL) {
      RedecErr(decl->vd_var);
      return 1;
    }
    if (decl->vd_type!=NULL && decl->vd_type->tl_op=='(') {
      if (curtok!=',' && curtok!=';') {
        if (IsFDefd(decl->vd_var))
          Error("Function already defined");
      }
      if (!IsSamTyp(decl,match) || 
           (decl->vd_tspec->ts_sclas==T_STATIC-256)!=
           (match->vd_tspec->ts_sclas==T_STATIC-256))
        DifTypEr(match,decl);
      return 1;
    }
    else {
      if (decl->vd_tspec->ts_sclas!=T_EXTERN-256 &&
          match->vd_tspec->ts_sclas!=T_EXTERN-256) {
        RedecErr(decl->vd_var);
        return 1;
      }
    }
    if ((decl->vd_type==NULL) != (match->vd_type==NULL)) {
      DifTypEr(match,decl);
      return 1;
    }
    if (decl->vd_type!=NULL && decl->vd_type->tl_op!=match->vd_type->tl_op) {
      DifTypEr(match,decl);
      return 1;
    }
    if (decl->vd_type->tl_op=='[') {
      VDecl vdecl1,vdecl2;
      vdecl1.vd_type = decl->vd_type->tl_next;
      vdecl1.vd_tspec = decl->vd_tspec;
      vdecl2.vd_type = match->vd_type->tl_next;
      vdecl2.vd_tspec = match->vd_tspec;
      if (!IsSamTyp(&vdecl1,&vdecl2)) {
        DifTypEr(match,decl);
        return 1;
      }
    }
    else {
      if (!IsSamTyp(decl,match)) {
        DifTypEr(match,decl);
        return 1;
      }
    }
    if (decl->vd_tspec->ts_sclas==T_EXTERN-256)
      return 1;
  }
  return 0;
}

static VDecl *RdGDeclI(tspec)
TSpec *tspec;
{
  VDecl *vdecl = RdDeclI(tspec);
  int sclass = vdecl->vd_tspec->ts_sclas;
  if (sclass==T_AUTO-256)
    Error("variables in file scope cannot be auto");
  if (curtok!=T_EQUAL) {
    if (IncmpArr(vdecl)) {
      if (sclass==T_STATIC-256)
        NoASzErr();
      if (sclass!=T_TYPEDE-256 && sclass!=T_EXTERN-256) {
        vdecl->vd_tspec = 
          (TSpec *)memcpy(NewTSpec(),vdecl->vd_tspec,sizeof(TSpec));
        vdecl->vd_tspec->ts_sclas=T_EXTERN-256;
      }
    }
  }
  StoreNam(vdecl);
  return vdecl;
}

RdGDecl()
{
  if (savtmp!=0) {
    Error("RdGDecl: savtmp!=0");
    exit(1);
  }
  TSReset();
  {
    int isfunct;
    char needsimi = 0;
    TSpec *tspec = NewTSpec();
    VDecl *vdecl;
    RdTSpec(tspec);
    if (HasDef(tspec)) {
      WrStRef(tspec->ts_xdat.ts_sref,tspec->ts_type);
      WrStr(";\n");
    }
    if (curtok==';') {
      SkipTok();
      return;
    }
    {
      vdecl = RdGDeclI(tspec);
      isfunct = (vdecl->vd_type!=NULL && vdecl->vd_type->tl_op=='(');
      if (isfunct && curtok!=';' && curtok!=',') {
        PLst *params = vdecl->vd_type->tl_dat.tld_plst;
        WrXDecl(vdecl,1);
        WrLn();
        if (IsProto(params))
          WrOSPLst(params,vdecl);
      }
      else
        needsimi = WrLDeclI(vdecl);
    }
    {
      if (isfunct && curtok!=',' && curtok!=';') {
        FDefLst *newfdef = (FDefLst *)StkAlloc(sizeof(FDefLst));
        newfdef->fdl_decl = vdecl;
        newfdef->fdl_prev = fdeflst;
        fdeflst = newfdef;
        RdFDef(vdecl);
      }
      else {
        for (;;) {
          if (curtok==T_EQUAL) {
            if (isfunct)
              Error("Attempt to initialize function");
            PrintTok(curtok);
            SkipTok();
            RdInit(vdecl->vd_type,vdecl->vd_tspec,1);
          }
          if (needsimi)
            WrStr(";\n");
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
          vdecl = RdGDeclI(tspec);
          needsimi = WrLDeclI(vdecl);
        }
      }
    }
  }
}

WrLIDecl(t)
Type *t;
{
  if (t==NULL)
    return;
  WrLIDecl(t->tl_next);
  if (t->tl_op=='*') {
    if (t->tl_next!=NULL && t->tl_next->tl_op!='*')
      WrStr("(*");
    else
      WrChar('*');
  }
}

WrXLIDec(vdecl)
VDecl *vdecl;
{
  VDecl tmpvdecl;
  if (vdecl->vd_tspec->ts_type==T_TDNAME-256) {
    tmpvdecl.vd_type = NULL;
    tmpvdecl.vd_tspec = vdecl->vd_tspec;
    ExpsType(&tmpvdecl,&tmpvdecl);
    WrXLIDec(&tmpvdecl);
  }
  WrLIDecl(vdecl->vd_type);
}

WrRIDecl(t,listparm,fndecl)
Type *t;
int listparm;
VDecl *fndecl;
{
  if (t==NULL)
    return;
  if (t->tl_op=='[') {
    WrChar('[');
    if (t->tl_dat.tld_num!=0)
      WrUInt(t->tl_dat.tld_num);
    WrChar(']');
  }
  else if (t->tl_op=='(') {
    WrChar('(');
    if (listparm || outstrm!=1)
      WrPLst(t->tl_dat.tld_plst,fndecl);
    WrChar(')');
  }
  else if (t->tl_op=='*' && t->tl_next!=NULL && t->tl_next->tl_op!='*')
    WrChar(')');
  WrRIDecl(t->tl_next,0,fndecl);
}

WrXRIDec(vdecl,listparm,fndecl)
VDecl *vdecl;
int listparm;
VDecl *fndecl;
{
  WrRIDecl(vdecl->vd_type,listparm,fndecl);
  if (vdecl->vd_tspec->ts_type==T_TDNAME-256) {
    VDecl tmpvdecl;
    tmpvdecl.vd_type = NULL;
    tmpvdecl.vd_tspec = vdecl->vd_tspec;
    ExpsType(&tmpvdecl,&tmpvdecl);
    WrXRIDec(&tmpvdecl,0,fndecl);
  }
}

WrEncode(name)
char *name;
{
  if (outstrm!=1)
    WrStr(name);
  else {
    char *p = name;
    for (;;) {
      if (*p=='\0') {
        WrStr(name);
        break;
      }
      if (p-name>=8) {
        WrEnIden("_tvl",name);
        break;
      }
      ++p;
    }
  }
}

StoreNam(vdecl)
VDecl *vdecl;
{
  char *name;
  {
    int sclas = vdecl->vd_tspec->ts_sclas;
    if (sclas==T_EXTERN-256 || sclas==TS_NONE)
      name = vdecl->vd_var;
    else
      return;
  }
  if (!name)
    return;
  if (strlen(name)<=8)
    return;
  if (ntblfile==-1) {
    char namestr[30];
    int namelen = strlen(progfile);
    memcpy(namestr,progfile,namelen);
    strcpy(namestr+namelen-1,"ntbl");
#ifndef CoCo
    ntblfile=creat(namestr,S_IRUSR|S_IWUSR);/*ntblfile=open(namestr, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);*/
#else
    ntblfile=creat(namestr,WRITE|READ);
#endif
    if (ntblfile==-1) {
      Error("Cannot open nametable");
      ntblfile = 2;
    }
  }
  outstrm = ntblfile;
  WrStr(name);
  WrChar(' ');
  WrHash(name);
  WrLn();
  outstrm = 1;
}

WrHash(name)
char *name;
{
  char *p = name;
  for (;;) {
    if (*p=='\0') {
      WrStr(name);
      return;
    }
    if (p-name>=8)
      break;
    ++p;
  }
  {
    unsigned hash1 = 0;
    unsigned hash2 = 0;
    p = name;
    while (*p!='\0') {
      hash1 = (hash1+781+*p)*2309;
      hash2 = (hash2+1432+*p)*2311;
      ++p;
    }
    WrReserv(8);
    WrStr("_");
    Wr03x(hash1 & 0xfff);
    Wr04x(hash2);
  }
}

WrVarNam(name,sclas)
char *name;
int sclas;
{
  if (outstrm==1) {
    if (fldname)
      WrEnIden("_tvf",name);
    else {
      if (sclas!=T_EXTERN-256 && sclas!=T_STATIC-256 && sclas!=TS_NONE)
        WrEncode(name);
      else
        WrHash(name);
    }
    return;
  }
  WrStr(name);
}

WrIDecl(vdecl,listparm)
VDecl *vdecl;
int listparm;
{
  Type *t = vdecl->vd_type;
  if (t!=NULL && outstrm==1 && t->tl_op=='(' && IsSorU(t->tl_next,vdecl->vd_tspec))
    WrChar('*');
  WrLIDecl(t);
  if (vdecl->vd_var!=NULL)
    WrVarNam(vdecl->vd_var,vdecl->vd_tspec->ts_sclas);
  WrRIDecl(t,listparm,vdecl);
}


