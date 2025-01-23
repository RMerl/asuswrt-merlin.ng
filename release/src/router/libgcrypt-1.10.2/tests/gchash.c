/* gchash.c - Calculate hash values
 * Copyright (C) 2013 Dmitry Eremin-Solenikov
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef _GCRYPT_IN_LIBGCRYPT
# undef _GCRYPT_IN_LIBGCRYPT
# include "gcrypt.h"
#else
# include <gcrypt.h>
#endif

#define PGM "gchash"
#include "t-common.h"


void
init_gcrypt (void)
{
  if (!gcry_check_version (GCRYPT_VERSION)) {
    fputs ("libgcrypt version mismatch\n", stderr);
    exit (2);
  }

  xgcry_control ((GCRYCTL_SUSPEND_SECMEM_WARN));

  /* Allocate a pool of 16k secure memory.  This make the secure memory
   * available and also drops privileges where needed.  */
  xgcry_control ((GCRYCTL_INIT_SECMEM, 16384, 0));

  xgcry_control ((GCRYCTL_RESUME_SECMEM_WARN));

  xgcry_control ((GCRYCTL_INITIALIZATION_FINISHED, 0));
}

int
main (int argc, char **argv)
{
  gcry_md_hd_t hd;
  gcry_error_t err;
  int algo;

  init_gcrypt();

  if (argc < 2 || (argv[1] && !strcmp(argv[1], "--help")))
    {
      fprintf (stderr, "Usage: %s <digest> <file>...\n", argv[0]);
      return 1;
    }

  algo = gcry_md_map_name (argv[1]);
  if (algo == GCRY_MD_NONE)
    {
      fprintf (stderr, "Unknown algorithm '%s'\n", argv[1]);
      return 1;
    }

  err = gcry_md_open(&hd, algo, 0);
  if (err)
    {
      fprintf (stderr, "LibGCrypt error %s/%s\n",
          gcry_strsource (err),
          gcry_strerror (err));
      exit (1);
    }

  for (argv += 2; *argv; argv++)
    {
      FILE *fp;
      unsigned char buf[1024];
      size_t size;
      int i;
      unsigned char *h;
      if (!strcmp (*argv, "-"))
        fp = stdin;
      else
        fp = fopen (*argv, "r");

      if (fp == NULL)
        {
          perror ("fopen");
          return 1;
        }

      while (!feof (fp))
        {
          size = fread (buf, 1, sizeof(buf), fp);
          gcry_md_write (hd, buf, size);
        }

      h  = gcry_md_read(hd, 0);

      for (i = 0; i < gcry_md_get_algo_dlen (algo); i++)
        printf("%02x", h[i]);
      printf("  %s\n", *argv);

      gcry_md_reset(hd);
    }

  gcry_md_close(hd);
  return 0;
}
