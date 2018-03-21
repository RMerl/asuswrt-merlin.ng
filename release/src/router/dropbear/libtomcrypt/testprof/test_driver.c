#include <tomcrypt_test.h>

void run_cmd(int res, int line, char *file, char *cmd)
{
   if (res != CRYPT_OK) {
      fprintf(stderr, "%s (%d)\n%s:%d:%s\n", error_to_string(res), res, file, line, cmd);
      if (res != CRYPT_NOP) {
         exit(EXIT_FAILURE);
      }
   }
}

/* $Source$ */
/* $Revision$ */
/* $Date$ */
