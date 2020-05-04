#ifndef CoCo
#include <ctype.h>
#include <stdlib.h>
#else
#include <ctype.h>
#endif

double atod(s)
char *s;
{
  int e = 0;
  int sign = 1;
  double v = 0.0;
  if (*s=='-') {
    sign = -1;
    ++s;
  }
  else if (*s=='+')
    ++s;
  while (isdigit(*s)) {
    v = v*10 + (*s-'0');
    ++s;
  }
  if (*s=='.') {
    double n = 1.0;
    ++s;
    while (isdigit(*s)) {
      v = v*10 + (*s-'0');
      --e;
      ++s;
    }
  }
  if (*s=='e' || *s=='E')
    e += atoi(s+1);
  if (v!=0) {
    double n = 1.0;
    if (e>0) {
      while (e>0) {
        n *= 10.0;
        --e;
      }
      v *= n;
    }
    else if (e<0) {
      while (e<0) {
        n *= 10.0;
        ++e;
      }
      v /= n;
    }
  }
  if (sign==-1)
    return -v;
  else
    return v;
}
