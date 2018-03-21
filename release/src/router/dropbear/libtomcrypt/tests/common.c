/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 */

#include "common.h"

/**
  @file common.c

  Steffen Jaeckel
*/

void run_cmd(int res, int line, const char *file, const char *cmd, const char *algorithm)
{
   if (res != CRYPT_OK) {
      fprintf(stderr, "%s (%d)%s%s\n%s:%d:%s\n",
              error_to_string(res), res,
              (algorithm ? " - " : ""), (algorithm ? algorithm : ""),
              file, line, cmd);
      if (res != CRYPT_NOP) {
         exit(EXIT_FAILURE);
      }
   }
}

void print_hex(const char* what, const void* v, const unsigned long l)
{
  const unsigned char* p = v;
  unsigned long x, y = 0, z;
  fprintf(stderr, "%s contents: \n", what);
  for (x = 0; x < l; ) {
      fprintf(stderr, "%02X ", p[x]);
      if (!(++x % 16) || x == l) {
         if((x % 16) != 0) {
            z = 16 - (x % 16);
            if(z >= 8)
               fprintf(stderr, " ");
            for (; z != 0; --z) {
               fprintf(stderr, "   ");
            }
         }
         fprintf(stderr, " | ");
         for(; y < x; y++) {
            if((y % 8) == 0)
               fprintf(stderr, " ");
            if(isgraph(p[y]))
               fprintf(stderr, "%c", p[y]);
            else
               fprintf(stderr, ".");
         }
         fprintf(stderr, "\n");
      }
      else if((x % 8) == 0) {
         fprintf(stderr, " ");
      }
  }
}

prng_state yarrow_prng;

/* ref:         $Format:%D$ */
/* git commit:  $Format:%H$ */
/* commit time: $Format:%ai$ */
