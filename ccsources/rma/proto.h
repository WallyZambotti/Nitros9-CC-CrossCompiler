/* chcodes.c */
/* part2.c */
CODFORM        *FindCmd(char *src_str, CODFORM * cod_strt, CODFORM * cod_stop);

/* part3.c */
int             l_brnch(void);
int             do_aim(void);
int             do_band(void);
int             cc_stuff(void);
int             q_immed(void);
int             do_md(void);
int             int_stuf(void);
int             chr_ef(void);
int             chr_stuf(void);
int             poprnd(int _lngreg);
int             do_divd(void);
int             bit_stuf(void);
int             no_opcod(void);
int             lea_s(void);
int             do_tfm(void);
int             tfr_exg(void);
int             do_addr(void);
int             stk_stuf(void);
int             lb_(void);
int             do_brnch(void);
int             equset(void);
int             asm_dirct(void);
int             do_ifs(void);
int             ilExtRef(void);
int             do_endc(void);
int             _psect(void);
int             codsetup(void);
int             do_csec(void);
int             setrmb3(void);
int             l0ef0(void);
int             setmac(void);
int             noMac(void);

/* part4.c */
int             immediat(void);
int             l0fe0(void);
int             l0ff5(void);
int             getreg(int PshPul);
int             regofst(void);
int             skparrow(int s_char);
int             ckpcr(void);
int             l11be(void);
int             l11e8(void);
int             l1215(void);
int             l12a2(void);
int             ilAdrMod(void);
int             not_pcr(void);
int             ilIdxReg(void);
int             l1326(void);
int             l1389(void);
int             l143e(void);
int             l149b(struct ref_ent * op);
int             l1541(struct symblstr * parm);
int             l156b(void);
int             l1584(void);
int             pc_rel(void);
void            storInt(char *dst, int vlu);

/* part5.c */
int             do_rzb(void);
int             l16f3(void);
int             l1723(void);
unsigned int    l174f(void);
int             l175f(int parm);
int             do_equ(void);
int             do_set(void);
int             l1780(int parm);
int             do_comon(void);
int             set_fcc(void);
int             set_fcs(void);
int             l180e(int parm);
int             cnstDef(void);
int             set_fcb(void);
int             set_fdb(void);
int             l18cd(int (*adr) (void), int cnt);
int             l18ed(int parm);
int             l1911(void);
int             do_os9(void);
int             iscomma(void);
int             l198d(void);
int             l199e(void);
int             do_nam(void);
static          copy_to(char *dest_str);
int             do_ttl(void);
int             do_pag(void);
int             do_spc(void);
int             _isnum(void);
int             set_opts(void);
static          bad_opt(void);
int             l1ae2(void);
int             do_use(void);
int             _isnul(void);
int             not_nul(void);
int             is_neg(void);
int             le_zero(void);
int             ge_zero(void);
int             is_pos(void);
int             is_pass1(void);
int             set_vsec(void);
int             is_dp(void);
int             setsect(void);
int             tel_fail(void);
int             do_rept(void);
int             end_rept(void);

/* part6.c */
int             MoveLabel(char *destpos);
int             l1e4d(void);
int             l1f63(void);
void            SetArow(struct symblstr * parm);
int             l2069(int parm);
void            l20a4(unsigned short address);
static          phaserr(void);
int             RefCreat(struct symblstr * myref, int rtyp, int adrs);
char            SkipSpac(void);
struct symblstr *WlkTreLft(struct symblstr * thistree);
struct symblstr *TreSetUp(char *newnam, struct symblstr ** oldtree, int cmp_cas);
int             FndTreEmty(char *new_nam, struct symblstr ** tptr);
int             getmem(int memreq);

/* part7.c */
int             do_long(void);
int             l2369(void);
int             l2387(void);
int             getNmbr(void);
int             e_ilxtRf(void);
int             do_math(int parm);
int             mathtyp(int tst_char);
int             l257e(void);
int             _getnum(void);

/* part8.c */
int             GetOpts(unsigned _argc, char **_argv);
FILE           *f_opn(char *fna, char *fmo);
FILE           *u_opn(char *fna, char *fmo);
int             prnthdr(void);
int             new_pag(void);
int             getUseLn(void);
static          DecSFil(void);
int             DoSymTbl(void);
static          PrtSmbl(struct symblstr * smbl);
int             do_psect(void);
int             e_xtrnl(void);
void            l2cf1(void);
int             w_ocod(unsigned char *_codaddr, int _codlen);
static void     PrnTree(int (*function) (struct symblstr * xx));
int             wrt_refs(void);
int             wrtGlbls(struct symblstr * parm);
int             wrtxtrns(struct symblstr * parm1);
int             CountRfs(struct symblstr * smbls);
static          DoRefLin(struct ref_str * parm);
int             wrtCmmns(struct symblstr * parm);

/* part9.c */
int             newMac(void);
int             mcWrtLin(void);
int             putfield(char *lin, int cchr);
int             add_nul(void);
MACDSCR        *McNamCmp(char *parm);
int             addMac(MACDSCR * parm);
int             decMac(void);
int             readmacs(void);
int             l3402(int parm);
int             l34a8(void);
int             l35df(int *parm);
int             gArgParm(int m_count, int parm2);
int             closmac(void);
int             mfilerr(void);

/* rma.c */
int             main(int argc, char *argv[]);
int             ReadFil(void);
static          AsmblLin(void);
int             ListLine(void);
int             bdrglist(void);
int             bd_rgnam(void);
int             nstmacdf(void);
int             e_report(char *report);
int             errexit(char *report);
char           *itoa(int n, char *s);
void            reverse(char *s);
