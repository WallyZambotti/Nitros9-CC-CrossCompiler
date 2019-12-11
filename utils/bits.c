#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void
procbit (int val)
{
    int bts[8];
    int ofst;

    ofst = 7;

    for (ofst = 7; ofst >= 0; ofst--)
    {
        bts[ofst] = (val & 1);
        val >>= 1;
    }

    for (ofst = 0; ofst <= 7; ofst++)
    {
        printf ("%d", bts[ofst]);
    }

    printf (" ");
}

int
main (int argc, char **argv)
{
    char **argp;

    if (argc < 2)
    {
        fprintf (stderr, "Need one or more decimal numbers to convert\n");
        exit (1);
    }

    argp = &argv[1];

    while (--argc)
    {
        if (!strcmp (*argp, ","))
        {
            printf ("\n");
            ++argp;
            continue;
        }

        int val = (int)strtol (*(argp++), 0, 0);
        procbit ((val >> 8) & 0xff);
        procbit (val & 0xff);
        
        if (argc > 1)
        {
            printf (" ");
        }
    }

    printf ("\n");

    return 0;
}
