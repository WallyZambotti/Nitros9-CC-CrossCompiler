/* cp1.c */
int doinclude(char *ln);
int tstdupdef(void);
int doundef(char *ln);
int splittok(char *ln, int b);
int findchar(char *s, int c);
int findstr(int pos, char *string, char *pattern);
int tokopr(char *ln, int b);
int toknum(char *ln, char *lnptr);
int initstdefs(void);
int cncatstr(char *ln);
/* cp2.c */
int gettoken(char *ln, int b);
int toksrch(char *ln, char *tk, int pv, int kv, int bv, int b, int *tcnt);
int tstargs(int i);
int addqmac(char *mac);
char *getoknum(char *tok, char *buf, int num);
int putdtbl(int b, int c);
int putiddtbl(int b, int d);
int dodefine(int a, int b);
int prep(void);
int dopragma(char *ln);
int doline(char *ln);
/* cp3.c */
void ifstini(void);
void tattle(char *prc, char *ln);
void doif(char *ln);
int doelse(void);
int doendif(void);
int doelif(char *ln);
int gettorf(register char *ln);
int dodef(char *ln, int b);
int ifcalc(char *ln);
int dodfined(char *ln);
int zeroident(char *ln);
int lnprint(int x, void *y);
int doerr(int code, int errptr);
/* cp4.c */
int expand(char *ln, int *loop, int *lpcntr);
int expln(char *ln, int *l, int *lc);
int strcmp2(char *s1, char *s2);
/* cp.c */
int main(int argc, char *argv[]);
int getln(int a);
int trigraph(int a);
int gettri(int b);
int xtndln(register int a);
int wspace(int a);
int convert(int x, int y);
int cmnt(int a);
int comm(int a, int *b);
int qa(char *ln, int x, int b);
int qa2(char *ln, int x, int b);
int space(int a, int b);
int tstline(int a);
int usage(void);
int getident(char *ln, register int b);
int skpbl(char *ln, int b);
int rskpbl(char *ln, register int a);
char *escseq(char *l, char *ln);
int _errmsg(int ernum, char *fmt, ...);
/* findstr.c */
/* solve.c */
int solve(char *ln);
void pushstk(int p1, int *p2, int *p3);
int popstk(int *p1, int *ptr);
void pushop(char *p1, char *p2, int *p3);
char *popop(char *p1, int *p2);
void pshunop(char *li, char *p2, int *p3);
char popunop(char *p1, int *p2);
int getnum(int *parm1, char **ptr);
int getopr(char *p1, char **p2);
int getop1(char *cp, char **lptr);
int tstnxtop(char *p1, char **cp);
int opval(char *pt);
int calc1(int p1, char p2);
int calc2(unsigned int p1, unsigned int p2, char *p3);
