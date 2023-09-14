/*
 * Taken from https://stackoverflow.com/questions/10404448/getopt-h-compiling-linux-c-code-in-windows
 */

#include <string.h>
#include <stdio.h>

extern int     opterr,             /* if error message should be printed */
  optind,             /* index into parent argv vector */
  optopt,                 /* character checked for validity */
  optreset;               /* reset getopt */
extern char    *optarg;                /* argument associated with option */

int getopt(int nargc, char * const nargv[], const char *ostr);