#ifndef CoCo
#include <string.h>
#endif
#include "ansifront.h"

#define NewExLst() ((ExpLst *)StkAlloc(sizeof(ExpLst)))

ChkDef(vdecl)
VDecl *vdecl;
{
  if (vdecl->vd_tspec->ts_type==-1) {
    BError();
    AError("'");
    AError(vdecl->vd_tspec->ts_xdat.ts_iden);
    AError("' undefined");
    EError();
  }
}

static ExpLst *RdExpLst(ptypes)
TLLst *ptypes;
{
  ExpLst *el;
  ExpLst **elp = &el;
  int paramnum = 1;
  if (curtok!=')') {
   for (;;) {
    RdAsExpr();
    MakRVal(&curvalue);
    ChkDef(&curvalue.val_type);
    if (ptypes==NULL)
      Error("Too many parameters");
    else if (ptypes!=(TLLst *)1) {
      VDecl pdecl;
      pdecl.vd_type = ptypes->tll_type;
      pdecl.vd_tspec = &ptypes->tll_tspec;
      if (CanInit(&pdecl,&curvalue.val_type,curvalue.val_expr)) {
        if (!IsSamTyp(&pdecl,&curvalue.val_type)) {
          VDecl *castdecl = NewVDecl();
          castdecl->vd_type = pdecl.vd_type;
          castdecl->vd_tspec = pdecl.vd_tspec;
          castdecl->vd_var = NULL;
          curvalue.val_expr = 
            (char *)NewBExpr('(',castdecl,curvalue.val_expr);
        }
      }
      else {
        BError();
        WrStr("Parameter ");
        WrInt(paramnum);
        WrStr(" should be ");
        WrTDecl(&pdecl);
        WrStr(" but is ");
        WrTDecl(&curvalue.val_type);
        EError();
      }
      ptypes = ptypes->tll_next;
    }
    *elp = NewExLst();
    if (IsSorU(curvalue.val_type.vd_type,curvalue.val_type.vd_tspec))
      (*elp)->el_exp = (char *)NewBExpr('&',NULL,curvalue.val_expr);
    else
      (*elp)->el_exp = curvalue.val_expr;
    elp = &(*elp)->el_next;
    if (curtok!=',')
      break;
    Skip(',');
    ++paramnum;
   }
  }
  Skip(')');
  if (ptypes!=NULL && ptypes!=(TLLst *)1)
    Error("Not enough parameters");
  *elp = NULL;
  return el;
}

static WrExpLst(el)
ExpLst *el;
{
  for (;;) {
    WrAsExpr(el->el_exp);
    if (el->el_next==NULL)
      break;
    PrintTok(',');
    el = el->el_next;
  }
}

NeedTemp(type,tspec)
Type *type;
TSpec *tspec;
{
  VDecl vdecl;
  int size;
  vdecl.vd_type = type;
  vdecl.vd_tspec = tspec;
  size = SizeOf(&vdecl);
  if (size>tempsize)
    tempsize = size;
}

/* Return the given tspec if it has no storage class, otherwise, return
   a copy of the tspec with the storage class set to none */

static TSpec *NoSCTS(tspec)
TSpec *tspec;
{
  if (tspec->ts_sclas==TS_NONE && !tspec->ts_direc)
    return tspec;
  {
    TSpec *newtspec = NewTSpec();
    memcpy(newtspec,tspec,sizeof(TSpec));
    newtspec->ts_sclas = TS_NONE;
    newtspec->ts_direc = 0;
    return newtspec;
  }
}

static CastCTyp(casttype)
int casttype;
{
  VDecl *castdecl = NewVDecl();
  castdecl->vd_tspec = NoSCTS(curvalue.val_type.vd_tspec);
  castdecl->vd_type = curvalue.val_type.vd_type;
  castdecl->vd_var = NULL;
  curvalue.val_expr = (char *)NewBExpr(casttype,castdecl,curvalue.val_expr);
}

IndCast()
{
  if (curvalue.val_type.vd_type!=NULL && 
      curvalue.val_type.vd_type->tl_op=='[')
    CastCTyp('(');
}

CurrCast()
{
  CastCTyp('L');
}

CPtrCast()
{
  CastCTyp('L');
}

RdPoExpr()
{
  RdPriExp();
  for (;;) {
    if (curtok==T_PLPL || curtok==T_MIMI) {
      if (IsSUPtr(&curvalue.val_type))
        CPtrCast();
      curvalue.val_expr = (char *)NewBExpr(curtok,curvalue.val_expr,NULL);
      {
        VDecl vdecl;
        ExpsType(&curvalue.val_type,&vdecl);
        if (!curvalue.val_islv)
          Error("++/-- of rvalue");
        else if (vdecl.vd_type!=NULL) {
          if (vdecl.vd_type->tl_op=='[')
            Error("++/-- of array");
        }
        else {
          if (vdecl.vd_tspec->ts_type==T_STRUCT || 
              vdecl.vd_tspec->ts_type==T_UNION)
            Error("++/-- of struct or union");
        }
      }
      curvalue.val_islv = 0;
      SkipTok();
    }
    else if (curtok=='(') {
      VDecl vdecl;
      ExpLst *plst;
      char *lexpr;
      int retstrct = 0;
      SkipTok();
      ExpsType(&curvalue.val_type,&vdecl);
      if (vdecl.vd_type!=NULL && vdecl.vd_type->tl_op=='*') {
        AddPreOp('*');
        vdecl.vd_type = vdecl.vd_type->tl_next;
        ExpsType(&vdecl,&vdecl);
      }
      lexpr = curvalue.val_expr;
      if (vdecl.vd_tspec->ts_type==-1) {
        if (warn) {
          BWarning();
          WrStr("No declaration for '");
          WrStr(vdecl.vd_tspec->ts_xdat.ts_iden);
          WrStr("'");
          EError();
        }
        plst = RdExpLst((TLLst *)1);
        curvalue.val_type.vd_type=NULL;
        curvalue.val_type.vd_tspec=&inttspec;
        curvalue.val_type.vd_var = vdecl.vd_tspec->ts_xdat.ts_iden;
        StoreNam(&curvalue);
      }
      else if (vdecl.vd_type==NULL || vdecl.vd_type->tl_op!='(') {
        Error("Attempt to call non-function");
        RdExpLst((TLLst *)1);
      }
      else {
        TLLst *ptypes = vdecl.vd_type->tl_dat.tld_plst->pl_type;
        plst = RdExpLst(ptypes);
        if (IsSorU(vdecl.vd_type->tl_next,vdecl.vd_tspec)) {
          {
            ExpLst *retparm = (ExpLst *)StkAlloc(sizeof(ExpLst));
            NeedTemp(vdecl.vd_type->tl_next,vdecl.vd_tspec);
            retparm->el_exp = (char *)NewUExpr(-1,"_tvstrct");
            retparm->el_next = plst;
            plst = retparm;
          }
          retstrct = 1;
        }
        curvalue.val_type.vd_type = vdecl.vd_type->tl_next;
        curvalue.val_type.vd_tspec = vdecl.vd_tspec;
      }
      curvalue.val_expr = (char *)NewBExpr(')',lexpr,plst);
      if (retstrct)
        curvalue.val_expr = (char *)NewBExpr('*',NULL,curvalue.val_expr);
      curvalue.val_islv = 0;
      curvalue.val_isce = 0;
    }
    else if (curtok=='[') {
      VDecl vdecl;
      char *lexpr;
      if (IsSUPtr(&curvalue.val_type))
        CurrCast();
      lexpr = curvalue.val_expr;
      SkipTok();
      ExpsType(&curvalue.val_type,&vdecl);
      ChkDef(&vdecl);
      RdExpr();
      Skip(']');
      if (vdecl.vd_type==NULL || 
          (vdecl.vd_type->tl_op!='*' && vdecl.vd_type->tl_op!='['))
        Error("Attempt to indirect non-pointer");
      else {
        curvalue.val_type.vd_type = vdecl.vd_type->tl_next;
        curvalue.val_type.vd_tspec = vdecl.vd_tspec;
        curvalue.val_expr = (char *)NewBExpr('[',lexpr,curvalue.val_expr);
        IndCast();
      }
      curvalue.val_islv = 1;
      curvalue.val_isce = 0;
    }
    else if (curtok=='.' || curtok==T_DASHGT) {
      int op = curtok;
      VDecl vdecl;
      ExpsType(&curvalue.val_type,&vdecl);
      ChkDef(&vdecl);
      if (curtok==T_DASHGT) {
        if (vdecl.vd_type==NULL || 
            (vdecl.vd_type->tl_op!='*' && vdecl.vd_type->tl_op!='['))
          Error("'->' used with non-pointer");
        else
          vdecl.vd_type = vdecl.vd_type->tl_next;
        ExpsType(&vdecl,&vdecl);
      }
      SkipTok();
      if (curtok!=T_IDENT)
        Error("Expecting field name");
      else {
        if (vdecl.vd_type!=NULL ||
            (vdecl.vd_tspec->ts_type!=T_STRUCT-256 && 
             vdecl.vd_tspec->ts_type!=T_UNION-256))
          Error("struct or union needed for '.' or '->'");
        else {
          VDecl *fields;
          StrctRef *sref = vdecl.vd_tspec->ts_xdat.ts_sref;
          if (sref->sr_dorr==SR_DEF)
            fields = sref->sr_drdat.sr_def;
          else
            fields = sref->sr_drdat.sr_ref->sd_flst;
          if (fields!=(VDecl *)1) {
            while (fields!=NULL) {
              if (strcmp(fields->vd_var,curident)==0) {
                curvalue.val_type.vd_type = fields->vd_type;
                curvalue.val_type.vd_tspec = fields->vd_tspec;
                curvalue.val_expr = 
                  (char *)NewBExpr(
                            op,(char *)curvalue.val_expr,(char *)fields
                          );
                curvalue.val_islv = 1;
                curvalue.val_isce = 0;
                break;
              }
              fields = fields->vd_prev;
            }
          }
          if (fields==NULL || fields==(VDecl *)1) {
            BError();
            if (fields==NULL) {
              WrStr(" field ");
              WrStr(curident);
              WrStr(" is not a member of ");
              ATSError(vdecl.vd_tspec);
            }
            else {
              ATSError(vdecl.vd_tspec);
              WrStr(" is incomplete");
            }
            EError();
            curvalue.val_type.vd_tspec = &undtspec;
            curvalue.val_type.vd_type = NULL;
            curvalue.val_isce = 0;
          }
          SkipTok();
        }
      }
    }
    else
      break;
  }
}

WrPoExpr(e)
char *e;
{
  int op = *(int *)e;
  if ((op==T_PLPL || op==T_MIMI) && ((BinExpr *)e)->exb_t2==NULL) {
    BinExpr *be = (BinExpr *)e;
    WrPoExpr(be->exb_t1);
    PrintTok(op);
  }
  else if (op==')') {
    BinExpr *be = (BinExpr *)e;
    WrPoExpr(be->exb_t1);
    PrintTok('(');
    if (be->exb_t2!=NULL)
      WrExpLst(be->exb_t2);
    PrintTok(')');
  }
  else if (op=='[') {
    BinExpr *be = (BinExpr *)e;
    WrPoExpr(be->exb_t1);
    PrintTok('[');
    WrExpr(be->exb_t2);
    PrintTok(']');
  }
  else if (op=='.' || op==T_DASHGT) {
    BinExpr *be = (BinExpr *)e;
    WrPoExpr(be->exb_t1);
    PrintTok(op);
    WrEnIden("_tvf",(int)((VDecl *)be->exb_t2)->vd_var);
    WrChar(' ');
  }
  else
    WrPriExp(e);
}
