#include <stdio.h>
#include <string.h>

char command[256] = "";

int main(argc,argv)
int argc; char *argv[];
{
  int res;
  FILE *task;
  char *prog, *type, *params;

  type = argv[1];
  prog = argv[2];
  params = argv[3];

  strcpy(command, prog);
  strcat(command, " ");
  strcat(command, params);

  printf("opening %s with %s\n", prog, params);

  task = popen(command, type);

  fprintf(task, "def\n");   
  fprintf(task, "abc\n");
  fprintf(task, "xyz\n");

  pclose(task);

  printf("Exiting!\n");  
}

