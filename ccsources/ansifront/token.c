#ifndef CoCo
#include "/usr/include/ctype.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#define writeln write
#else
#include <ctype.h>
#endif
#include "ansifront.h"

#define TAB 9
#define MAXIDENT 127

char *toknames[] = {
  "",
#define TB_ATYPE 257
  "struct",
#define T_STRUCT 257
  "union",
#define T_UNION  258
  "enum",
#define T_ENUM   259
#define TB_TYPE  260
#define TB_BTYPE 260
  "int",
#define T_INT    260
  "char",
#define T_CHAR   261
  "float",
#define T_FLOAT  262
  "double",
#define T_DOUBLE 263
  "void",
#define T_VOID   264
#define TE_BTYPE 265
  "unsigned",
#define T_UNSIGN 265
  "signed",
#define T_SIGNED 266
  "short",
#define T_SHORT  267
  "long",
#define T_LONG   268
#define TB_SCLAS 269
  "static",
#define T_STATIC 269
  "typedef",
#define T_TYPEDE 270
  "extern",
#define T_EXTERN 271
  "auto",
#define T_AUTO   272
  "register",
#define T_REGIST 273
#define TE_SCLAS 274
  "const",
#define T_CONST  274
  "direct",
#define T_DIRECT 275
#define TE_TYPE  276
  "",
#define T_TDNAME 276
#define TE_ATYPE 277
  "if",
#define T_IF     277
  "else",
#define T_ELSE   278
  "do",
#define T_DO     279
  "for",
#define T_FOR    280
  "while",
#define T_WHILE  281
  "switch",
#define T_SWITCH 282
  "break",
#define T_BREAK  283
  "continue",
#define T_CONTIN 284
  "return",
#define T_RETURN 285
  "case",
#define T_CASE   286
  "default",
#define T_DEFAUL 287
  "goto",
#define T_GOTO   288
#define TB_ASNOP 289
  "=",
#define T_EQUAL  289
  "+=",
#define T_PLSEQ  290
  "-=",
#define T_MINEQ  291
  "*=",
#define T_STAREQ 292
  "/=",
#define T_DIVEQ  293
  "%=",
#define T_MODEQ  294
  ">>=",
#define T_SHREQ  295
  "<<=",
#define T_SHLEQ  296
  "|=",
#define T_OREQ   297
  "&=",
#define T_ANDEQ  298
  "^=",
#define T_XOREQ  299
#define TE_ASNOP 300
  "||",
#define T_OROR   300
  "&&",
#define T_ANDAND 301
  "==",
#define T_EQEQ   302
  "!=",
#define T_NOTEQ  303
#define TB_COMP  304
  "<",
#define T_LT     304
  "<=",
#define T_LTEQ   305
  ">=",
#define T_GTEQ   306
  ">",
#define T_GT     307
#define TE_COMP  308
  "<<",
#define T_SHIFTL 308
  ">>",
#define T_SHIFTR 309
  "++",
#define T_PLPL   310
  "--",
#define T_MIMI   311
  "sizeof",
#define T_SIZEOF 312
  "->",
#define T_DASHGT 313
  "...",
#define T_ELIPSE 314
  NULL
};

static char  inbuf[512];
static char *inbufend = inbuf;
static char *inchrp = inbuf;
static char  eofmark = 0;
static char  identbuf[(MAXIDENT+1)*2];
       int    curtok;
       int    nexttok;
       char  *curident = identbuf;
       char  *nexident = identbuf+(MAXIDENT+1);
#define SkipChr() {if (++inchrp >= inbufend) FillIBuf();}
#define SkipNChr(n) {if ((inchrp+=(n)) >= inbufend) FillIBN();}
#define InBufEnd() inbufend
#define PeekChr(x) /*
*/ ((inchrp+(x)<InBufEnd()) ? *(inchrp+(x)) : (FillIBS(),*(inchrp+x)))

int DumpLine()
{
  writeln(2, "\n>>>", 4);
  writeln(2,inchrp,inbufend-inchrp+1);
  writeln(2, "\n", 1);
  return 0;
}

static FillIBuf()
{
  /*/int bread=read(0,inbuf,sizeof(inbuf));*/
  int bread=read(0,inbuf,(ssize_t)512);
  inbufend = inbuf+bread;
  inchrp = inbuf;
  if (inbufend<=inbuf) {
    eofmark=1;
    *inchrp = 0;
  }
}

static FillIBN()
{
  int n = inchrp-inbufend;
  FillIBuf();
  inchrp += n;
}

static FillIBS()
{
  int extra=InBufEnd()-inchrp;
  memcpy(inbuf,inchrp,extra);
  inbufend = inbuf+extra + read(0,inbuf+extra,(sizeof(inbuf)) - extra);
  if (inbufend<=inbuf)
    eofmark=1;
  inchrp = inbuf;  
}

static SkpCmmnt()
{
  SkipNChr(2);
  while ((*inchrp!='*' || PeekChr(1)!='/') && !eofmark) {
    if (*inchrp=='\n')
      ++curline;
    SkipChr()
  }
  SkipNChr(2);
}

static SkipLine()
{
  while (!eofmark && *inchrp!='\n') {
    if (*inchrp=='/' && PeekChr(1)=='*')
      SkpCmmnt();
    else {
      WrChar(*inchrp);
      SkipChr()
    }
  }
  WrLn();
  SkipChr()
}

static int MakLine(n)
int n;
{
  curline = atoi(inchrp)+1;
}

static int MakFile(n)
int n;
{
  strcpy(curfile,inchrp);
}

static int MakProg(n)
int n;
{
  strcpy(progfile,inchrp);
}

static int ShowErr(n)
int n;
{
  BError();
  AError("Preprocessing error - ");
  AError(inchrp);
  EError();
}

static int DoLine(funct)
int (*funct)();
{
  int n = 1;
  while (PeekChr(n)!='\n')
    ++n;
  *(inchrp+n) = '\0';
  (*funct)(n);
  WrStr(inchrp);
  WrLn();
  SkipNChr(n+1);
}

static char *RdEscChr(cip)
char *cip;
{
  *cip++ = *inchrp;
  SkipChr()
  if (*inchrp=='\n') {
    Error("carriage return in string constant");
    --cip;
    SkipChr()
  }
  else {
    char c = *cip++ = *inchrp;
    SkipChr()
    if (isdigit(c) && isdigit(*inchrp)) {
      *cip++ = *inchrp;
      SkipChr()
      if (isdigit(*inchrp)) {
        *cip++ = *inchrp;
        SkipChr()
      }
    }
  }
  return cip;
}

static int RdInt()
{
  int val;
  int n = 0;
  while (isspace(*inchrp))
    SkipChr()
  while (isdigit(PeekChr(n)))
    ++n;
  val = atoi(inchrp);
  SkipNChr(n);
  return val;
}

static WrDCStr(str)
char *str;
{
  while (*str!='\0') {
    if (*str=='.')
      WrChar('_');
    else
      WrChar(*str);
    ++str;
  }
}

static RdPrep()
{
  int cmdlen = 0;
  WrLn();
  while (*inchrp==' ' || *inchrp==TAB)
    SkipChr()
  while (!isspace(PeekChr(cmdlen)))
    ++cmdlen;
  if (cmdlen==1) {
    char cmdchr = *inchrp;
    SkipChr()
    WrChar('#');
    WrChar(cmdchr);
    SkipLine();
    switch (cmdchr) {
      case 'P':
        DoLine(MakProg);
        SkipLine();
        break;
      case '5':
        DoLine(MakLine);
        break;
      case '7':
        DoLine(MakFile);
        SkipLine();
        break;
      case '2':
      case '6':
        SkipLine();
        break;
      case '1':
      case '0':
        SkipLine();
        DoLine(MakLine);
        SkipLine();
        --curline;
        DoLine(ShowErr);
    }
  }
  else {
    *(inchrp+cmdlen) = '\0';
    if (strcmp(inchrp,"line")==0) {
      SkipNChr(cmdlen+1);
      WrStr("#5\n");
      WrInt(curline = RdInt());
      WrLn();
      while (*inchrp==' ')
        SkipChr()
      if (*inchrp!='\n') {
        char *fnbp = curfile;
        SkipChr()
        while (*inchrp!='"') {
          *fnbp++ = *inchrp;
          SkipChr()
        }
        SkipChr()
        *fnbp = '\0';
        WrStr("#7\n");
        WrStr(curfile);
        WrLn();
        WrDCStr(curfile);
        WrLn();
      }
    }
    else if (strcmp(inchrp,"pragma")==0) {
      int praglen = 0;
      SkipNChr(cmdlen+1);
      while (*inchrp==' ' || *inchrp==TAB)
        SkipChr()
      while (!isspace(PeekChr(praglen)))
        ++praglen;
      *(inchrp+praglen) = '\0';
      if (strcmp(inchrp,"edn")==0) {
        SkipNChr(praglen+1);
        WrStr("#P\n");
        WrDCStr(curfile);
        WrLn();
        while (!isspace(*inchrp)) {
          WrChar(*inchrp);
          SkipChr()
        }
        WrLn();
      }
      else if (strcmp(inchrp,"asm")==0) {
        SkipNChr(praglen+1);
        WrStr("#2\n");
        while (*inchrp!='\n') {
          WrChar(*inchrp);
          SkipChr()
        }
        WrLn();
      }
    }
  }
}
        
            
          
int RdToken()
{
  for (;;) {
    while (*inchrp==' ' || *inchrp=='\n' || *inchrp==TAB) {
      if (*inchrp=='\n')
        ++curline;
      SkipChr()
    }
    if (*inchrp=='/' && PeekChr(1)=='*')
      SkpCmmnt();
    else if (*inchrp=='#') {
      SkipChr()
      RdPrep();
    }
    else
      break;
  }
  if (eofmark)
    return -1;
  {
    char c=*inchrp;
    if (isalpha(c) || c=='_') {
      char *cip = curident;
      *cip++ = c;
      SkipChr()
      for (;;) {
        c=*inchrp;
        if (!isalpha(c) && !isdigit(c) && c!='_')
          break;
        if (cip<curident+(MAXIDENT-1))
          *cip++ = c;
        SkipChr()
      }
      *cip='\0';
      {
        char **toknamp = toknames;
        while (*toknamp!=NULL) {
          if (strcmp(curident,*toknamp)==0)
            return 256+(toknamp-toknames);
          ++toknamp;
        }
      }
      return T_IDENT;
    }
    else if (isdigit(c) || (c=='.' && isdigit(PeekChr(1)))) {
      char *cip = curident;
      int isflt = 0;
      if (c=='0' && PeekChr(1)=='x') {
        *cip++ = *inchrp;
        SkipChr()
        *cip++ = *inchrp;
        SkipChr()
        while (isxdigit(*inchrp)) {
          *cip++ = *inchrp;
          SkipChr()
        }
      }
      else {
        int c1;
        while (isdigit(*inchrp)) {
          *cip++ = *inchrp;
          SkipChr()
        }
        if (*inchrp=='.') {
          *cip++ = *inchrp;
          SkipChr()
          while (isdigit(*inchrp)) {
            *cip++ = *inchrp;
            SkipChr()
          }
          isflt=1;
        }
        if ((*inchrp=='e' || *inchrp=='E') &&
            (isdigit(c1=PeekChr(1)) || 
             ((c1=='+' || c1=='-') && isdigit(PeekChr(2)))
            )
           ) {
          *cip++ = *inchrp;
          SkipChr()
          if (*inchrp=='+' || *inchrp=='-') {
            *cip++ = *inchrp;
            SkipChr()
          }
          while (isdigit(*inchrp)) {
            *cip++ = *inchrp;
            SkipChr()
          }
          isflt=1;
        }
      }
      *cip = '\0';
      if (isflt) {
        if (*inchrp=='F' || *inchrp=='f')
          SkipChr()
        else if (*inchrp=='L' || *inchrp=='l')
          SkipChr()
        return T_NUMBER;
      }
      else {
        int uflag = 0;
        int lflag = 0;
        if (*inchrp=='U' || *inchrp=='u') {
          uflag = 1;
          SkipChr()
        }
        if (*inchrp=='L' || *inchrp=='l') {
          lflag = 1;
          SkipChr()
        }
        if (!uflag && (*inchrp=='U' || *inchrp=='u')) {
          uflag = 1;
          SkipChr()
        }
        return (lflag) ? T_LNGNUM : T_INTNUM;
      }
    }
    else if (c=='\'') {
      char *cip = curident;
      SkipChr()
      for (;;) {
        if (*inchrp=='\'') {
          if (cip-curident<1) {
            Error("null character constant");
            *cip++ = '\\';
            *cip++ = '0';
          }
          SkipChr()
          break;
        }
        else if (*inchrp=='\\')
          cip = RdEscChr(cip);
        else if (*inchrp=='\n') {
          Error("carriage return in character literal");
          *cip++ = '\\';
          *cip++ = 'n';
          SkipChr()
        }
        else if (eofmark) {
          Error("character literal unfinished");
          break;
        }
        else {
          *cip++ = *inchrp;
          SkipChr()
        }
      }
      *cip='\0';
      return T_CHRLIT;
    }
    else if (c=='"') {
      char *cip = curident;
      SkipChr()
      for (;;) {
        if (*inchrp=='"') {
          SkipChr()
          break;
        }
        else if (*inchrp=='\\')
          cip = RdEscChr(cip);
        else if (eofmark) {
          Error("String literal not finished");
          break;
        }
        else {
          *cip++ = *inchrp;
          SkipChr()
        }
      }
      *cip = '\0';
      return 'S';
    }
    else {
      switch (c) {
        case '+':
          SkipChr()
          switch (*inchrp) {
            case '+':
              SkipChr()
              return T_PLPL;
            case '=':
              SkipChr()
              return T_PLSEQ;
          }
          return '+';
        case '-':
          SkipChr()
          switch (*inchrp) {
            case '-':
              SkipChr()
              return T_MIMI;
            case '=':
              SkipChr()
              return T_MINEQ;
            case '>':
              SkipChr()
              return T_DASHGT;
          }
          return '-';
        case '>':
          SkipChr()
          switch (*inchrp) {
            case '=':
              SkipChr()
              return T_GTEQ;
            case '>':
              SkipChr()
              if (*inchrp=='=') {
                SkipChr()
                return T_SHREQ;
              }
              return T_SHIFTR;
          }
          return T_GT;
        case '<':
          SkipChr()
          switch (*inchrp) {
            case '=':
              SkipChr()
              return T_LTEQ;
            case '<':
              SkipChr()
              if (*inchrp=='=') {
                SkipChr()
                return T_SHLEQ;
              }
              return T_SHIFTL;
          }
          return T_LT;
        case '&':
          SkipChr()
          switch (*inchrp) {
            case '&':
              SkipChr()
              return T_ANDAND;
            case '=':
              SkipChr()
              return T_ANDEQ;
          }
          return '&';
        case '|':
          SkipChr()
          switch (*inchrp) {
            case '|':
              SkipChr()
              return T_OROR;
            case '=':
              SkipChr()
              return T_OREQ;
          }
          return '|';
        case '=':
          SkipChr()
          if (*inchrp=='=') {
            SkipChr()
            return T_EQEQ;
          }
          return T_EQUAL;
        case '!':
          SkipChr()
          if (*inchrp=='=') {
            SkipChr()
            return T_NOTEQ;
          }
          return '!';
        case '*':
          SkipChr()
          if (*inchrp=='=') {
            SkipChr()
            return T_STAREQ;
          }
          return '*';
        case '/':
          SkipChr()
          if (*inchrp=='=') {
            SkipChr()
            return T_DIVEQ;
          }
          return '/';
        case '%':
          SkipChr()
          if (*inchrp=='=') {
            SkipChr()
            return T_MODEQ;
          }
          return '%';
        case '^':
          SkipChr()
          if (*inchrp=='=') {
            SkipChr()
            return T_XOREQ;
          }
          return '^';
        case '.':
          SkipChr()
          if (*inchrp=='.' && PeekChr(1)=='.') {
            SkipNChr(2);
            return T_ELIPSE;
          }
          return '.';
        default:
          SkipChr()
          return c;
      }
    }
  }
}

SkipTok()
{
  if (nexttok==0)
    curtok=RdToken();
  else {
    curtok = nexttok;
    nexttok = 0;
    if (curident==identbuf)
      curident = identbuf+(MAXIDENT+1);
    else
      curident = identbuf;
  }
}

int PeekTok()
{
  if (nexttok==0) {
    char *itemp = curident;
    if (curident==identbuf)
      curident = identbuf+(MAXIDENT+1);
    else
      curident = identbuf;
    nexttok = RdToken();
    nexident = curident;
    curident = itemp;
  }
  return nexttok;
}

InitRdTk()
{
  SkipChr()
  nexttok = 0;
  SkipTok();
}
