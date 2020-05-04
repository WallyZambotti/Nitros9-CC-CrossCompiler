/*
** driver for the C compiler
*/
/*
**  01-23-85  Added -b and -t options. Enabled /nl.
**  ??-??-??  Added -O option to stop compilation after optimization.
**            Prevent optimization of ".a" files.
**  02-15-85  Fix naming conventions if -O is used.
**  ??-??-??  ll option for lex lib
**  07-30-86  P option for special debug and z for debug
**  03-24-87  don't optimize ".o" files, but accept as ".a"
**  03-12-88  Added two pass (CoCo) compiler support.  Bill Dickhaus
**  09-23-90  Increased command-line buffer (parmbuf) to 4k, Eddie Kuns
**  10-29-91  Added -g to use /dd/lib/getopt.r instead of library version
**            Added -V to show version number.  Bob Billson
**  02-19-92  Added lg and ls options for cgfx and sys libraries,
**              respectively.  Boisy G. Pitre
**  07-10-93  Added -A option to use ansifront.  Vaughn Cato.
**  11-22-93  Added -E opt, triggers c_prep18's trigraph expands. G. Heskett
**  03-21-94  Added -N opt, use r63 as assembler instead of rma.  G. Heskett
**  10-21-19  Added -L opt, requests assembler produce listing. Walter Zambotti
**  10-21-19  Added -Y opt, requests assembler produce symbol table. Walter Zambotti
**  11-28-19  Added Linux support. Walter Zambotti
*/

#ifdef __MINGW32__
#include <windows.h>
#endif
#include <unistd.h>
#include <string.h>
#include "cc.h"
#include "module.h"

#ifdef OS9
#define O_RDWRCR 3
#else
#include <fcntl.h>
#include <sys/stat.h>
#define O_RDWRCR (O_RDWR | O_CREAT)
#endif

/* screw ansifront's 8 char hashing */
#define intercept intercep

/* some proto info for ansifront or c.pass1 */
char * chkccdev();

static int newipath = -1;
static int tmpfd;

#ifdef __MINGW32__
PROCESS_INFORMATION pi;
#endif
#ifdef UNIX
pid_t child;
#endif

cleanup()
{
#ifdef __MINGW32__
     if (pi.hProcess)
          TerminateProcess(pi.hProcess, 1);
#endif
#ifdef OS9
     if (childid)
          kill(childid, 2);
#endif
#ifdef UNIX
     if (childid)
          kill(child, 3);
#endif
     if (newopath)
     {
          close(1);
          dup(newopath);
          close(newopath);
     }
     if (newipath > -1) {
       close(0);
       dup(newipath);
       close(newipath);
     }

     if (thisfilp)
          unlink(thisfilp);

     if (lasfilp)
          unlink(lasfilp);
}


trap(code)
int   code;
{
     cleanup();
     exit(code);
}

/*page*/
main(argc, argv)
int   argc;
char  **argv;
{
     char  **emp;
     int suffix, j, m, deltmpflg;
     char  *p;

     j = 0;
     while ((--argc > 0) && (++j < 100))
     {
          if (*(p = *++argv) == '-')
          {
               while (*++p)
               {
                    switch (*p)
                    {
                    case 'a' :             /* stop at asm (no .r) (D) */
                         aflag = TRUE;
                         break;

                    case 'A' :             /* Use ansifront */
                         Aflag = TRUE;
                         break;

                    case 'b' :              /* use different "cstart" */
                         bflag = TRUE;

                         if (*(p + 1) != '=')
                              break;

                         strcpy(mainline, (p + 2));
                         goto saver;

                    case 'c' :                /* include comments (C) */
                         cflag = TRUE;
                         break;

                    case 'd' :        /* make identifier (define) (C) */
                         if (*(p + 1) == '\0')
                              goto saver;

                         *--p = '-';
                         *(p + 1) = 'D';
                         macarray[maccnt++] = p;
                         goto saver;

                    case 'e' :                  /* set edition number */
                         emp = &edition;
                         goto emcommon;

                    case 'E' :
                         tgfex = 1;
                         break;

                    case 'f' :                /* set outfile path (L) */
                         if (*++p != '=')
                              goto saver;

                         strcpy(objname, (p + 1));

                         if (objname[0] == '\0')
                              goto saver;

                         ++fflag;
                         suffix = findsuff(objname);

                         if (suffix == 'c' || suffix == 'r')
                              error("Suffix '.%c' not allowed for output", suffix);

                         goto saver;
                         /*page*/
                    case 'g':                 /* use /dd/lib/getopt.r */
                         gtflg++;
                         break;

                    case 'l' :               /* specify a library (L) */
                         if (*(p + 1) == 'l')
                         {
                              llflg++;
                              goto saver;
                         }
                         else if (*(p + 1) == 's')
                         {
                              lsflg++;
                              goto saver;
                         }
                         else if (*(p+1) == 'g')
                         {
                              lgflg++;
                              goto saver;
                         }
                         else
                              if (*(p + 1) != '=')
                                   goto saver;

                         if (libcnt == 4)
                              error("Too many libraries");

                         *--p = '-';
                         libarray[libcnt++] = p;
                         goto saver;

                    case 'm' :                     /* set memory size */
                         emp = &xtramem;
                         *p &= 0x5f;
emcommon:
                         if ((*(p + 1)))
                         {
                              *--p = '-';
                              *emp = p;
                         }
                         goto saver;

                    case 'M' :         /* ask linker for link map (L) */
                         mflag = TRUE;
                         break;

                    case 'N' :		/* use r63 for assembler */
                         Nflag = TRUE;
                         break;

                    case 'n' :              /* give module a name (L) */
                         *--p = '-';
                         modname = p;
                         goto saver;

                    case 'o' :                    /* no optimizer (O) */
                         oflag = FALSE;
                         break;

                    case 'O' :             /* stop after optimization */
                         o2flg = TRUE;
                         break;

                    case 'P' :
                         p2flg = TRUE;         /* fall to set pflag too */

                    case 'p' :                    /* add profiler (C) */
                         pflag = TRUE;
                         break;

                    case 'q' :                      /* use quiet mode */
                         qflag = TRUE;
                         freopen("c.errors", "w", stderr);
                         break;

                    case 'r' :                /* stop at .r (no link) */
                         rflag = TRUE;

                         if (*++p != '=')
                             goto saver;

                         strcpy(rlib, (p + 1));

                         if (rlib[0] == '\0')
                             goto saver;

                         strcat(rlib, "/");
                         goto saver;
                         /*page*/
                    case 's' :               /* no stack checking (C) */
                         sflag = TRUE;
                         break;

                    case 'S' :     /* ask linker for symbol table (L) */
                         s2flg = TRUE;
                         break;

                    case 'L' :     /* ask assembler for listing  */
                         alflg = TRUE;
                         break;

                    case 'Y' :     /* ask assembler for symbol table  */
                         ylflg = TRUE;
                         break;

                    case 't' :            /* use transendental library */
                         tflag = TRUE;
                         break;

                    case 'T' :              /* use alternate (or NO) tmpdir*/
                         if (*(p + 1) != '=')
                         {
                              tmpdir = FALSE;
                              break;
                         }
                         else
                              {
                              strcpy(tmpname, (p + 2));
                              strcat(tmpname,"/");
                         }
                         goto saver;

                    case 'V':
                         logo();
                         exit (0);

                    case 'w' :    /* waste the compile for error check */
                         nullflag = TRUE;
                         break;

                    case 'x' :    /* use the work dir for main library */
                         xflag = TRUE;
                         break;

                    case 'z' :
                         zflag = TRUE;
                         break;

                    case '?':     /* print list of options */
                         usage();
                         break;

                    default  :
                         error("unknown flag : -%c\n", *p);
                    }  /* end of switch */
               }  /* end of inner while */
saver:
               continue;
          }  /* end of if */
          else
          {
               switch (suffix = findsuff(*argv))
               {
               case 'r' :
                    rsufflg = 1;
               case 'a' :
               case 'c' :
               case 'o' :
                    suffarray[filcnt] = suffix;
                    namarray[filcnt] = *argv;
                    ++filcnt;
                    break;

               default  :
                    error("%s : no recognised suffix", *argv);
               }  /* end of switch */
          }  /* end of else */
     }  /* end of outer while */
     /*page*/
     /*   command line now parsed, start real work   */
     logo();                                            /* identify us */

     if (filcnt == 0)
     {
          fprintf(stderr, "no files!\n");
          exit (0);
     }

     if ((aflag + rflag) > 1)
          error("incompatible flags");

     if (fflag)
          if (filcnt >1)
               if (aflag || rflag)
                    error("%s : output name not applicable", objname);

     if (fflag == 0)
          strcpy(objname, ((filcnt == 1) ? namarray[0] : "output"));


     if((tmpdir) && (*tmpname == '\0'))
          for(loopcnt=0;loopcnt<sizeof(tmproot)/sizeof(char *);loopcnt++)
          {
               if(access(tmproot[loopcnt],0x83) == 0)
               {
                    strcpy(tmpname, tmproot[loopcnt]);
                    strcat(tmpname,"/");
                    break;   
               }
          }

     strcat(tmpname, tmptail);

     mktemp(tmpname);
     strcat(tmpname, ".m"); /* add a suffix for chgsuff */

#ifdef OS9
     intercept(trap); /* I'm gonna bet this is the barfed name */
#endif
     dummy();

     for (j = 0; j < filcnt; ++j)         /* for each file on cmd line */
     {
          fprintf(stderr, "'%s'\n", namarray[j]);
          if (suffarray[j] == 'c')
          {
               deltmpflg = 1;          /* is C prog so prep and compile it */
               strcpy(destfile, tmpname);
               chgsuff(destfile, 'm'); 
               frkprmp = parmbuf;

               if (cflag)
                    splcat("-l");                        /* wants comments */

               if (edition)
                    splcat(edition);           /* explicit edition number */

               for (m = 0; m < maccnt; )
                    splcat(macarray[m++]);           /* tack on "defines" */

               splcat(namarray[j]);            /* and now the file name */
               newopath = dup(1);
               close(1);

#ifdef OS9
                 if ((tmpfd = creat(destfile, O_RDWRCR))!=1)
#else
                 if ((tmpfd = creat(destfile, S_IRUSR | S_IWUSR))!=1)
#endif                   
                    error("can't create temporary file for '%s'", namarray[j]);

               trmcat();
               thisfilp = 0;
               lasfilp = destfile;
               if(tgfex)
                    runit("c_prep -T ", 1);
               else
                    runit("c_prep", 1);
               close(1);
               dup(newopath);
               close(newopath);
               newopath = 0;
               /*page*/

               if (Aflag) {
/* Run ansifront */
                 strcpy(srcfile,destfile);
                 strcpy(destfile,tmpname);
                 chgsuff(destfile,'n');
                 frkprmp = parmbuf;
                 thisfilp = srcfile;
                 trmcat(); /* Add \n\0 to frkprmp */
                 lasfilp = destfile;
                 newipath = dup(0);
                 close(0);
#ifdef OS9
                 if ((tmpfd = open(srcfile, O_RDWRCR))!=0)
#else
                 if ((tmpfd = open(srcfile, O_RDWRCR, S_IRUSR | S_IWUSR))!=0)
#endif                   
                    error("can't open temporary file for '%s'", namarray[j]);
                 newopath = dup(1);
                 close(1);
#ifdef OS9
                 if ((tmpfd = creat(destfile, O_RDWRCR))!=1)
#else
                 if ((tmpfd = creat(destfile, S_IRUSR | S_IWUSR))!=1)
#endif                   
                   error("can't create temporary file for '%s'",namarray[j]);
                 runit("ansifront",0);
                 close(1);
                 dup(newopath);
                 close(newopath);
                 newopath = 0;
                 close(0);
                 dup(newipath);
                 close(newipath);
                 newipath = -1;
                 unlink(srcfile);
               }

/* now compile it */
 
#ifdef TWOPASS
               strcpy(srcfile, destfile);
               frkprmp = parmbuf;
               thisfilp = srcfile;
               splcat(srcfile); /* append a space and srcfile to frkprmp */

               if (sflag)
                   splcat("-s"); /* no stack checking */

               strcpy(destfile, tmpname);
               chgsuff(destfile, 'a');
               strcpy(ofn, "-o=");
               strcat(ofn, destfile);
               splcat(ofn);

               if (pflag)
                   splcat("-p"); /* profiler code */

               trmcat();
               lasfilp = destfile;
               runit("c.pass1", 0);
               unlink(srcfile);
#endif
               strcpy(srcfile, destfile);
               frkprmp = parmbuf;
               thisfilp = srcfile;
               splcat(srcfile);

               if (sflag)
                    splcat("-s");                    /* no stack checking */

               if (nullflag)
               {
#ifndef TWOPASS
                    splcat("-n");                    /* waste the output */
#endif
                    strcpy(destfile, "/nil");
                    deltmpflg = 0;
               }
               else
               {
                    if (aflag)
                    {
                         if (fflag)
                              strcpy(destfile, objname); /* explicit obj name */
                         else
                              {
                              strcpy(destfile, namarray[j]);
                              chgsuff(destfile, 'a');
                              }
                    }
                    else
                         chgsuff(destfile, 'i');
               }

               strcpy(ofn, "-o=");
               strcat(ofn, destfile);
               splcat(ofn);

               if (pflag)
                    splcat("-p");                  /* wants profiler code */

               trmcat();
               lasfilp = destfile;
#ifdef TWOPASS
               runit("c.pass2", 0);
#else
               runit("c.comp", 0);
#endif
               unlink(srcfile);
          }

          else
               deltmpflg = 0;
          /*page*/
          /* now assemble and perhaps optimize it */
          if (aflag || nullflag || (suffarray[j] == 'r'))
               lasfilp = 0;                   /* is .r so no work to do */
          else
          {
               /* don't optimize ".a" or ".o" files */
               if ((suffarray[j] == 'a') || (suffarray[j] == 'o'))
               {
                    strcpy(srcfile, namarray[j]);
                    thisfilp = 0;
               }
               else
               {
                    strcpy(srcfile, destfile);
                    thisfilp = srcfile;

                    if (oflag)
                    {
                         frkprmp = parmbuf;            /* yes,  optimize it */
                         splcat(srcfile);

                         if ((filcnt == 1) && (o2flg == 0))
                         {
                              strcpy(destfile, tmpname);
                              chgsuff(destfile, 'm');
                         }
                         else
                         {
                              if (fflag && o2flg)
                                  strcpy(destfile, objname);
                              else
                              {
                                  chgsuff(namarray[j], 'o');
                                  strcpy(destfile, namarray[j]);
                              }
                         }
                         splcat(destfile);
                         trmcat();
                         lasfilp = destfile;
                         runit("c.opt", 0);

                         if (deltmpflg)
                              unlink(srcfile);

                         strcpy(srcfile, destfile);
                         thisfilp = srcfile;
                    }
               }
               if (o2flg == 0)
               {
                    frkprmp = parmbuf;
                    splcat(srcfile);

                    if ((filcnt == 1) && (rflag == 0))
                    {
                         strcpy(destfile, tmpname);
                         chgsuff(destfile, 'r');
                    }
                    else
                    {
                         if (fflag && rflag)
                         {
                             strcpy(destfile, rlib);
                             strcat(destfile, objname);
                         }
                         else
                         {
                             chgsuff(namarray[j], 'r');
                             strcpy(destfile, rlib);
                             strcat(destfile, namarray[j]);
                         }
                    }
                    strcpy(ofn, "-o=");
                    strcat(ofn, destfile);
                    splcat(ofn);
                    trmcat();
                    lasfilp = destfile;
                    if (alflg)
                        splcat("-l");
                    if (ylflg)
                        splcat("-s");
		            if (Nflag)
			            runit(NASSEMBLER, 0);
                    else
                        runit(ASSEMBLER, 0);

                    if (deltmpflg)
                         unlink(srcfile);
               }
          }
     } /* end of for each file */
     if (nullflag || aflag || rflag || o2flg)
          exit (0);
     /*page*/
     /* now link all together */
     frkprmp = parmbuf;

     if (( p = chkccdev()) == NULL)
          error("Cannot find default system drive");

     if (bflag)
          strcpy(ofn, mainline);      /* use cstart.r or whatever */
     else
          strcat(strcat(strcpy(ofn, p), "/lib/"), mainline); /* global */

     splcat(ofn);

     if ((filcnt == 1) && (suffarray[0] != 'r'))
          splcat(thisfilp = destfile);
     else
          for (thisfilp = 0, j = 0; j < filcnt; ++j)
              splcat(namarray[j]);

     strcpy(ofn, "-o=");

     if (fflag == 0)
          chgsuff(objname, 0);

     strcat(ofn, objname);
     splcat(ofn);

     if (gtflg)
     {
          strcpy (ofn, "/dd/lib/getopt.r");
          splcat (ofn);
     }

     for (j = 0; j < libcnt; j++)
          splcat(libarray[j]);

     if (lgflg)
     {
          strcat (strcat (strcpy (ofn, "-l="), p), "/lib/cgfx.l");
          splcat (ofn);
     }

     if (llflg)
     {
          strcat(strcat(strcpy(ofn, "-l="), p), "/lib/lexlib.l");
          splcat(ofn);
     }

     if (lsflg)
     {
          strcat (strcat (strcpy (ofn, "-l="), p), "/lib/sys.l");
          splcat (ofn);
     }

     if (p2flg)
     {
          strcat(strcat(strcpy(ofn, "-l="), p), "/lib/dbg.l");
          splcat(ofn);
     }


     strcpy(ofn, "-l=");

     if (xflag == 0)
          strcat(strcat(ofn, p), "/lib/");

     strcat(ofn, (tflag) ? "clibt.l" : "clib.l");
     splcat(ofn);

     if (modname)
          splcat(modname);

     if (xtramem)
          splcat(xtramem);

     if (edition)
          splcat(edition);

     if (mflag)
          splcat("-m");

     if (s2flg)
          splcat("-s");

     trmcat();
     lasfilp = 0;
     runit(LINKER, 0);
     cleanup();
}

#ifdef OS9
/*page*/
runit(cmd, code)
char  *cmd;
int   code;
{
     /*   fprintf(stderr, "   %-6s:\n", cmd); */
     fprintf(stderr, "   %-7s:  %s", cmd, parmbuf);
     if (zflag)
          return;

     if ((childid = os9fork(cmd, frkprmsiz, parmbuf, 1, 1, 0)) < 0)
          error("cannot execute %s", cmd);

     wait(&childstat);
     childid = 0;

     if (childstat > code)
          trap(childstat);
}
#endif
#ifdef UNIX
runit(cmd, code)
char *cmd;
int code;
{
     int childstat, i = 2;
     char *argv[32];

     /*   fprintf(stderr, "   %-6s:\n", cmd); */
     fprintf(stderr, "   %-7s:  %s\n", cmd, parmbuf);

     if (zflag)
          return;

     argv[0] = cmd;
     argv[1] = strtok(parmbuf, " ");
     while((argv[i] = strtok(NULL, " ")) != NULL) { i++; } ;

     child = fork();

     if(child == -1)
     {
          error("cannot execute %s", cmd);
          return;
     }

     if(child == 0)
     { //I am the child
          execvp(argv[0], argv);
          /* If this line is reached an error has occurred */
          error("cannot execute %s", cmd);
     }

     waitpid(child, &childstat, 0);
}
#endif

#ifdef __MINGW32__
runit(cmd, code)
char *cmd;
int code;
{
     STARTUPINFO si;
     char cmdline[256];

     ZeroMemory(&si, sizeof(si));
     si.cb = sizeof(si);
     ZeroMemory(&pi, sizeof(pi));

     /*   fprintf(stderr, "   %-6s:\n", cmd); */
     fprintf(stderr, "   %-7s:  %s\n", cmd, parmbuf);

     if (zflag)
          return;

     strcpy(cmdline, cmd);
     strcat(cmdline, " ");
     strcat(cmdline, parmbuf);

     if(!CreateProcess(NULL,   // No module name (use command line)
          cmdline,        // Command line
          NULL,           // Process handle not inheritable
          NULL,           // Thread handle not inheritable
          FALSE,          // Set handle inheritance to FALSE
          0,              // No creation flags
          NULL,           // Use parent's environment block
          NULL,           // Use parent's starting directory 
          &si,            // Pointer to STARTUPINFO structure
          &pi))           // Pointer to PROCESS_INFORMATION structure
     {
          error("cannot execute %s", cmd);
          return;
     }

     WaitForSingleObject(pi.hProcess, INFINITE);
     CloseHandle(pi.hProcess);
     CloseHandle(pi.hThread);
}
#endif

#ifdef OS9
char * chkccdev()
{
     char *s, c;
     register char  *p;
     struct mod_data *q; /* was mod_exec, m_data not member if it! */
     struct mod_config *r;

     if((q=(struct mod_data *)modlink("ccdevice",4,0))!=(struct mod_data *)-1)
     {
          strcpy(devnam1, (char *) q + q->m_data);
          munlink(q);
          return (devnam1);
     }
     else
     {
          if ( (r=(struct mod_config *)modlink("Init",0x0c,0))!=(struct mod_config *)-1)
          {
               s = (char *)(r + r->m_sysdrive);
               p = devnam1;

               while ((c = *s++) > 0)
                    *p++ = c;

               *p++ = (c & 0x7f);
               *p = 0;
               munlink(r);
               return (devnam1);
          }
     }
     return (NULL);
}
#else
char * chkccdev()
{
     strcpy(devnam1, "/dd");
     return (devnam1);
}
#endif

error(format, arg)
char  *format,   *arg;
{
     logo();                             /* print logo if not done yet */
     fprintf(stderr, format, arg);
     putc('\n', stderr);
     trap(0);
}


chgsuff(s, c)
char  *s, c;
{
     register char  *p = s;

     while(*p++)
          ;
     if (*(p - 3) != '.')
          return;

     if (c == '\0')
          *(p - 3) = 0;
     else
          *(p - 2) = c;
}



findsuff(p)
register char *p;
{
     int   j;
     char  c;

     j = 0;

     while (c = *p++)
          if (c == '/')
               j = 0;
          else
               j++;

     if (j <= 29 && j > 2 && *(p - 3) == '.')
          return (*(p - 2) | 0x40);
     else
          return (0);
}


splcat(s)
char  *s;
{
     register char *p = s;

     *frkprmp++ = 0x20;
     while (*frkprmp++ = *p++)
          ;
     --frkprmp;
}


trmcat()
{
#ifdef OS9
     *frkprmp++ = '\n';
#endif
     *frkprmp = '\0';
     frkprmsiz = frkprmp - parmbuf;
}


dummy()
{
}


logo()
{
     if (hello == 0)
          fprintf(stderr, "\n cc version %d.%d.%d\n", VERSION, MAJREV, MINREV);
}


usage()
{
     register char **p;
     static char *help[] =  {
    "cc: C compiler executive",
    "Usage: cc <opts> <files> <opts>",
    "OPTIONS",
    "   -a        = stop at the assembly",
    "   -A        = run ansifront for ANSI-C processing",
    "   -b=<path> = use a different \"cstart\"",
    "   -c        = include comments",
    "   -d        = create an identifier",
    "   -e#       = set edition number of module",
    "   -f=<path> = set outfile path",
    "   -g        = use /dd/lib/getopt.r",
    "   -l=<path> = specify a library",
    "   -lg       = cgfx.l      (graphics library)",
    "   -ls       = sys.l       (system library)",
    "   -ll       = lexlib.l    (lexical library)",
    "   -P        = dbg.l       (special debugger library)",
    "   -m<memK>  = set memory size",
    "   -M        = ask linker for link map",
    "   -n=<name> = give module a name",
    "   -N        = use r63 assembler for native source .a code",
    "   -o        = no optimizer",
    "   -O        = stop after optimization",
    "   -p        = add profiler",
    "   -q        = quiet mode (don't echo lines)",
    "   -r        = stop at .r (no link)",
    "   -s        = no stack checking",
    "   -S        = ask linker for symbol table",
    "   -Y        = ask assembler for symbol table",
    "   -L        = ask assembler for listing",
    "   -t        = use transendental library (clibt.l)",
    "   -T=<path> = use alternate (or NO) temp. dir.",
    "   -V        = show version number", 
    "   -w        = waste the compile for error checking only",
    "   -x        = use the work dir. for the main library",
    "   -z        = debug mode",
    NULL
   };


   for (p = help; *p != NULL; ++p)
       fprintf (stderr, "%s\n", *p);

   exit (0);
}
