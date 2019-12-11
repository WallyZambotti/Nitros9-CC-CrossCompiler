/* opt01.c */
int main(int argc, char **argv);
/* opt02.c */
char *fetchfield(int wrdtyp, char *deststr, char *srcstr, int *isglbl);
/* opt03.c */
void inittlist(void);
LBLENT *newlblref(char *strng);
LBLENT *findlbl(char *strng);
int str_sum(char *strng);
void pull_lbl(LBLENT *thislbl);
char *getoptlbl(char *strngaddr);
/* opt04.c */
void initvars(void);
CMDENT *bldcmdent(CMDENT *oldcmd, char *opcode, char *operand, LBLENT **curlbl);
void setupbrnch(CMDENT *brnchcmd, LBLENT *oprnd);
void comnlblcmd(CMDENT *frstcmd, CMDENT *commoncmd);
void prtlblcmd(void);
void removcmd(CMDENT *delcmd);
void lbl_del(CMDENT *src_cmd, LBLENT *dstlbl);
CMDENT *getNewDest(void);
void to_NewDest(CMDENT *cmd);
/* opt05.c */
void initwwlist(void);
void doaltcode(CMDENT *frstcmd);
void optimizecode(CMDENT *commncmd);
/* opt06.c */
void *add_mem(int memreq);
void errexit(char *pmpt, char *parm2);
