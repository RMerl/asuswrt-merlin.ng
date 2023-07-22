/* Simpler gnu89 version of StandaloneFuzzTargetMain.c from LLVM */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

extern int LLVMFuzzerTestOneInput (const unsigned char *data, size_t size);

int
main (int argc, char **argv)
{
  FILE *f;
  size_t n_read, len;
  unsigned char *buf;

  if (argc < 2)
    return 1;

  f = fopen (argv[1], "r");
  assert (f);
  fseek (f, 0, SEEK_END);
  len = ftell (f);
  fseek (f, 0, SEEK_SET);
  buf = (unsigned char*) malloc (len);
  n_read = fread (buf, 1, len, f);
  assert (n_read == len);
  LLVMFuzzerTestOneInput (buf, len);

  free (buf);
  printf ("Done!\n");
  return 0;
}
