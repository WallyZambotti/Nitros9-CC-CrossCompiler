#include <stdio.h>
#include <string.h>

#define DEBUG

#define TYPELANG 0x11 /* type 1 program - language 1 machine code */
#define DEFDATSIZ 0 /* Default data size */

#define IN 0
#define OUT 1
#define ERR 2

FILE *popen(cmd, type)
char *cmd; char *type;
{
    int in, out, paramsiz = 0, pid;
    char *modname, *params = NULL, *spaceptr;
    FILE *fpipe;
    int pn, fd;

    /* determine stdin or stdout */

    switch (toupper(*type))
    {
        case 'W': /* fork a process and connect pipe to the cmd's stdin */
            pn = IN;
        break;

        case 'R': /* fork a process and connect pipe to the cmd's stdout */
            pn = OUT;        
        break;

        default:
            return NULL;
    }

#ifdef DEBUG
    fprintf(stderr, "Path number %d\n", pn);
#endif

    /* open a pipe and dup it into stdin or stdout */

    fpipe = fopen("/pipe", type); /* use fopen instead of open because popen returns a FILE* */
#ifdef DEBUG
    fprintf(stderr, "Pipe Path number %d\n", fpipe->_fd);
#endif
    fd = dup(pn);
    /* make the pipe be the stdin or stdout of the child task */
    close(pn);
    dup(fpipe->_fd); /* get the path number from the FILE structure */

    /* Split the cmd into a cmd and a seperate params suitable for os9fork */

    modname = cmd;
    spaceptr = index(cmd, ' ');
    if (spaceptr)
    {
        *spaceptr = 0; /* terminate module name at space */
        params = spaceptr + 1;
        paramsiz = strlen(params);
    }
#ifdef DEBUG
    fprintf(stderr, "Forking %s with %s\n", modname, params);
#endif
    pid = os9fork(modname, paramsiz, params, TYPELANG, DEFDATSIZ);

    /* restore stdin or stdout */

    close(pn);
    dup(fd);
#ifdef DEBUG
    fprintf(stderr, "Returning\n");
#endif
    return fpipe;
}
