/* t-rsa-testparm.c - Check the RSA Key Generation test-parm parameter
 * Copyright (C) 2022 g10 Code GmbH
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
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gcrypt.h>

#include "stopwatch.h"

#define PGM "t-rsa-testparm"
#include "t-common.h"


static void
check_rsa_testparm (void)
{
  gpg_error_t err;
  gcry_sexp_t keyspec = NULL;
  gcry_sexp_t key = NULL;
  const char *sexp = "(genkey (rsa (nbits \"2048\") (test-parms "
    "(e \"65537\")"
    "(p #00bbccabcee15d343944a47e492d4b1f4de79633e20cbb46f7d2d6813392a807ad048"
        "cf77528edd19f77e7453f25173b9dcb70423afa2037aae147b81a33d541fc58f875ef"
        "f1e852ab55e2e09a3debfbc151b3b0d17fef6f74d81fca14fbae531418e211ef81859"
        "2af70de5cec3b92795cc3578572bf456099cd8727150e523261#)"
    "(q #00ca87ecf2883f4ed00a9ec65abdeba81d28edbfcc34ecc563d587f166b52d42bfbe2"
        "2bbc095b0b8426a2f8bbc55baaa8859b42cbc376ed3067db3ef7b135b63481322911e"
        "bbd7014db83aa051e0ca2dbf302b75cd37f2ae8df90e134226e92f6353a284b28bb30"
        "af0bbf925b345b955328379866ebac11d55bc80fe84f105d415#)"
    ")))";

  info ("Checking RSA KeyGen test-parm parameter.\n");

  err = gcry_sexp_build (&keyspec, NULL, sexp);
  if (err)
    {
      fail ("error building SEXP for test: %s", gpg_strerror (err));
      goto leave;
    }

  err = gcry_pk_genkey (&key, keyspec);
  if (err)
    {
      fail ("gcry_pk_genkey failed for test: %s", gpg_strerror (err));
      goto leave;
    }

leave:
  if (key)
    gcry_sexp_release (key);
  if (keyspec)
    gcry_sexp_release (keyspec);
}


int
main (int argc, char **argv)
{
  int last_argc = -1;

  if (argc)
    { argc--; argv++; }

  while (argc && last_argc != argc )
    {
      last_argc = argc;
      if (!strcmp (*argv, "--"))
        {
          argc--; argv++;
          break;
        }
      else if (!strcmp (*argv, "--help"))
        {
          fputs ("usage: " PGM " [options]\n"
                 "Options:\n"
                 "  --verbose       print timings etc.\n"
                 "  --debug         flyswatter\n",
                 stdout);
          exit (0);
        }
      else if (!strcmp (*argv, "--verbose"))
        {
          verbose++;
          argc--; argv++;
        }
      else if (!strcmp (*argv, "--debug"))
        {
          verbose += 2;
          debug++;
          argc--; argv++;
        }
      else if (!strncmp (*argv, "--", 2))
        die ("unknown option '%s'", *argv);

    }

  xgcry_control ((GCRYCTL_DISABLE_SECMEM, 0));
  if (!gcry_check_version (GCRYPT_VERSION))
    die ("version mismatch\n");
  if (debug)
    xgcry_control ((GCRYCTL_SET_DEBUG_FLAGS, 0xffffffff, 0));

  start_timer ();
  check_rsa_testparm ();
  stop_timer ();

  info ("All tests completed in %s.  Errors: %d\n",
        elapsed_time (1), error_count);
  return !!error_count;
}
