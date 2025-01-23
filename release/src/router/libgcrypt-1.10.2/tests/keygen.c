/* keygen.c  -  key generation regression tests
 * Copyright (C) 2003, 2005, 2012 Free Software Foundation, Inc.
 * Copyright (C) 2013, 2015 g10 Code GmbH
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
#include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "../src/gcrypt-int.h"


#define PGM "keygen"
#include "t-common.h"

static int in_fips_mode;


/* static void */
/* show_note (const char *format, ...) */
/* { */
/*   va_list arg_ptr; */

/*   if (!verbose && getenv ("srcdir")) */
/*     fputs ("      ", stderr);  /\* To align above "PASS: ".  *\/ */
/*   else */
/*     fprintf (stderr, "%s: ", PGM); */
/*   va_start (arg_ptr, format); */
/*   vfprintf (stderr, format, arg_ptr); */
/*   if (*format && format[strlen(format)-1] != '\n') */
/*     putc ('\n', stderr); */
/*   va_end (arg_ptr); */
/* } */


static void
show_sexp (const char *prefix, gcry_sexp_t a)
{
  char *buf;
  size_t size;

  fprintf (stderr, "%s: ", PGM);
  if (prefix)
    fputs (prefix, stderr);
  size = gcry_sexp_sprint (a, GCRYSEXP_FMT_ADVANCED, NULL, 0);
  buf = xmalloc (size);

  gcry_sexp_sprint (a, GCRYSEXP_FMT_ADVANCED, buf, size);
  fprintf (stderr, "%.*s", (int)size, buf);
  gcry_free (buf);
}


static void
show_mpi (const char *prefix, gcry_mpi_t a)
{
  char *buf;
  void *bufaddr = &buf;
  gcry_error_t rc;

  fprintf (stderr, "%s: ", PGM);
  if (prefix)
    fputs (prefix, stderr);
  rc = gcry_mpi_aprint (GCRYMPI_FMT_HEX, bufaddr, NULL, a);
  if (rc)
    fprintf (stderr, "[error printing number: %s]\n",  gpg_strerror (rc));
  else
    {
      fprintf (stderr, "%s\n", buf);
      gcry_free (buf);
    }
}


static void
check_generated_rsa_key (gcry_sexp_t key, unsigned long expected_e)
{
  gcry_sexp_t skey, pkey, list;

  pkey = gcry_sexp_find_token (key, "public-key", 0);
  if (!pkey)
    fail ("public part missing in return value\n");
  else
    {
      gcry_mpi_t e = NULL;

      list = gcry_sexp_find_token (pkey, "e", 0);
      if (!list || !(e=gcry_sexp_nth_mpi (list, 1, 0)) )
        fail ("public exponent not found\n");
      else if (!expected_e)
        {
          if (verbose)
            show_mpi ("public exponent: ", e);
        }
      else if ( gcry_mpi_cmp_ui (e, expected_e))
        {
          show_mpi ("public exponent: ", e);
          fail ("public exponent is not %lu\n", expected_e);
        }
      gcry_sexp_release (list);
      gcry_mpi_release (e);
      gcry_sexp_release (pkey);
    }

  skey = gcry_sexp_find_token (key, "private-key", 0);
  if (!skey)
    fail ("private part missing in return value\n");
  else
    {
      int rc = gcry_pk_testkey (skey);
      if (rc)
        fail ("gcry_pk_testkey failed: %s\n", gpg_strerror (rc));
      gcry_sexp_release (skey);
    }
}


static void
check_rsa_keys (void)
{
  gcry_sexp_t keyparm, key;
  int rc;

  if (verbose)
    info ("creating 2048 bit RSA key\n");
  rc = gcry_sexp_new (&keyparm,
                      "(genkey\n"
                      " (rsa\n"
                      "  (nbits 4:2048)\n"
                      " ))", 0, 1);
  if (rc)
    die ("error creating S-expression: %s\n", gpg_strerror (rc));
  rc = gcry_pk_genkey (&key, keyparm);
  gcry_sexp_release (keyparm);
  if (rc)
    die ("error generating RSA key: %s\n", gpg_strerror (rc));

  if (verbose)
    info ("creating 1024 bit RSA key\n");
  rc = gcry_sexp_new (&keyparm,
                      "(genkey\n"
                      " (rsa\n"
                      "  (nbits 4:1024)\n"
                      " ))", 0, 1);
  if (rc)
    die ("error creating S-expression: %s\n", gpg_strerror (rc));

  gcry_sexp_release (key);
  rc = gcry_pk_genkey (&key, keyparm);
  gcry_sexp_release (keyparm);
  if (rc && !in_fips_mode)
    fail ("error generating RSA key: %s\n", gpg_strerror (rc));
  else if (!rc && in_fips_mode)
    fail ("generating 1024 bit RSA key must not work!");

  if (!rc)
    {
      if (verbose > 1)
        show_sexp ("1024 bit RSA key:\n", key);
      check_generated_rsa_key (key, 65537);
    }
  gcry_sexp_release (key);

  if (verbose)
    info ("creating 2048 bit RSA key with e=65539\n");
  rc = gcry_sexp_new (&keyparm,
                      "(genkey\n"
                      " (rsa\n"
                      "  (nbits 4:2048)\n"
                      "  (rsa-use-e 5:65539)\n"
                      " ))", 0, 1);
  if (rc)
    die ("error creating S-expression: %s\n", gpg_strerror (rc));
  rc = gcry_pk_genkey (&key, keyparm);
  gcry_sexp_release (keyparm);
  if (rc)
    fail ("error generating RSA key: %s\n", gpg_strerror (rc));

  if (!rc)
    check_generated_rsa_key (key, 65539);
  gcry_sexp_release (key);


  if (verbose)
    info ("creating 512 bit RSA key with e=257\n");
  rc = gcry_sexp_new (&keyparm,
                      "(genkey\n"
                      " (rsa\n"
                      "  (nbits 3:512)\n"
                      "  (rsa-use-e 3:257)\n"
                      " ))", 0, 1);
  if (rc)
    die ("error creating S-expression: %s\n", gpg_strerror (rc));
  rc = gcry_pk_genkey (&key, keyparm);
  gcry_sexp_release (keyparm);
  if (rc && !in_fips_mode)
    fail ("error generating RSA key: %s\n", gpg_strerror (rc));
  else if (!rc && in_fips_mode)
    fail ("generating 512 bit RSA key must not work!");

  if (verbose && rc && in_fips_mode)
    info ("... correctly rejected key creation in FIPS mode (%s)\n",
          gpg_strerror (rc));

  if (!rc)
    check_generated_rsa_key (key, 257);
  gcry_sexp_release (key);

  if (verbose)
    info ("creating 512 bit RSA key with default e\n");
  rc = gcry_sexp_new (&keyparm,
                      "(genkey\n"
                      " (rsa\n"
                      "  (nbits 3:512)\n"
                      "  (rsa-use-e 1:0)\n"
                      " ))", 0, 1);
  if (rc)
    die ("error creating S-expression: %s\n", gpg_strerror (rc));
  rc = gcry_pk_genkey (&key, keyparm);
  gcry_sexp_release (keyparm);
  if (rc && !in_fips_mode)
    fail ("error generating RSA key: %s\n", gpg_strerror (rc));
  else if (!rc && in_fips_mode)
    fail ("generating 512 bit RSA key must not work!");

  if (verbose && rc && in_fips_mode)
    info ("... correctly rejected key creation in FIPS mode (%s)\n",
          gpg_strerror (rc));


  if (!rc)
    check_generated_rsa_key (key, 0); /* We don't expect a constant exponent. */
  gcry_sexp_release (key);
}


static void
check_elg_keys (void)
{
  gcry_sexp_t keyparm, key;
  int rc;

  if (verbose)
    info ("creating 1024 bit Elgamal key\n");
  rc = gcry_sexp_new (&keyparm,
                      "(genkey\n"
                      " (elg\n"
                      "  (nbits 4:1024)\n"
                      " ))", 0, 1);
  if (rc)
    die ("error creating S-expression: %s\n", gpg_strerror (rc));
  rc = gcry_pk_genkey (&key, keyparm);
  gcry_sexp_release (keyparm);
  if (rc && !in_fips_mode)
    die ("error generating Elgamal key: %s\n", gpg_strerror (rc));
  else if (!rc && in_fips_mode)
    die ("generating Elgamal key must not work in FIPS mode.");
  if (verbose > 1)
    show_sexp ("1024 bit Elgamal key:\n", key);
  gcry_sexp_release (key);
}


static void
check_dsa_keys (void)
{
  gcry_sexp_t keyparm, key;
  int rc;
  int i;

  /* Check that DSA generation works and that it can grok the qbits
     argument. */
  if (verbose)
    info ("creating 5 1024 bit DSA keys\n");
  for (i=0; i < 5; i++)
    {
      rc = gcry_sexp_new (&keyparm,
                          "(genkey\n"
                          " (dsa\n"
                          "  (nbits 4:1024)\n"
                          " ))", 0, 1);
      if (rc)
        die ("error creating S-expression: %s\n", gpg_strerror (rc));
      rc = gcry_pk_genkey (&key, keyparm);
      gcry_sexp_release (keyparm);
      if (rc && !in_fips_mode)
        die ("error generating DSA key: %s\n", gpg_strerror (rc));
      else if (!rc && in_fips_mode)
        die ("generating 1024 bit DSA key must not work in FIPS mode!");
      if (!i && verbose > 1)
        show_sexp ("1024 bit DSA key:\n", key);
      gcry_sexp_release (key);
    }

  if (verbose)
    info ("creating 1536 bit DSA key\n");
  rc = gcry_sexp_new (&keyparm,
                      "(genkey\n"
                      " (dsa\n"
                      "  (nbits 4:1536)\n"
                      "  (qbits 3:224)\n"
                      " ))", 0, 1);
  if (rc)
    die ("error creating S-expression: %s\n", gpg_strerror (rc));
  rc = gcry_pk_genkey (&key, keyparm);
  gcry_sexp_release (keyparm);
  if (rc && !in_fips_mode)
    die ("error generating DSA key: %s\n", gpg_strerror (rc));
  else if (!rc && in_fips_mode)
    die ("generating 1536 bit DSA key must not work in FIPS mode!");
  if (verbose > 1)
    show_sexp ("1536 bit DSA key:\n", key);
  gcry_sexp_release (key);

  if (verbose)
    info ("creating 3072 bit DSA key\n");
  rc = gcry_sexp_new (&keyparm,
                      "(genkey\n"
                      " (dsa\n"
                      "  (nbits 4:3072)\n"
                      "  (qbits 3:256)\n"
                      " ))", 0, 1);
  if (rc)
    die ("error creating S-expression: %s\n", gpg_strerror (rc));
  rc = gcry_pk_genkey (&key, keyparm);
  gcry_sexp_release (keyparm);
  if (rc && !in_fips_mode)
    die ("error generating DSA key: %s\n", gpg_strerror (rc));
  else if (!rc && in_fips_mode)
    die ("generating DSA key must not work in FIPS mode!");
  if (verbose > 1)
    show_sexp ("3072 bit DSA key:\n", key);
  gcry_sexp_release (key);

  if (verbose)
    info ("creating 2048/256 bit DSA key\n");
  rc = gcry_sexp_new (&keyparm,
                      "(genkey\n"
                      " (dsa\n"
                      "  (nbits 4:2048)\n"
                      "  (qbits 3:256)\n"
                      " ))", 0, 1);
  if (rc)
    die ("error creating S-expression: %s\n", gpg_strerror (rc));
  rc = gcry_pk_genkey (&key, keyparm);
  gcry_sexp_release (keyparm);
  if (rc && !in_fips_mode)
    die ("error generating DSA key: %s\n", gpg_strerror (rc));
  else if (!rc && in_fips_mode)
    die ("generating DSA key must not work in FIPS mode!");
  if (verbose > 1)
    show_sexp ("2048 bit DSA key:\n", key);
  gcry_sexp_release (key);

  if (verbose)
    info ("creating 2048/224 bit DSA key\n");
  rc = gcry_sexp_new (&keyparm,
                      "(genkey\n"
                      " (dsa\n"
                      "  (nbits 4:2048)\n"
                      "  (qbits 3:224)\n"
                      " ))", 0, 1);
  if (rc)
    die ("error creating S-expression: %s\n", gpg_strerror (rc));
  rc = gcry_pk_genkey (&key, keyparm);
  gcry_sexp_release (keyparm);
  if (rc && !in_fips_mode)
    die ("error generating DSA key: %s\n", gpg_strerror (rc));
  else if (!rc && in_fips_mode)
    die ("generating DSA key must not work in FIPS mode!");
  if (verbose > 1)
    show_sexp ("2048 bit DSA key:\n", key);
  gcry_sexp_release (key);
}


static void
check_generated_ecc_key (gcry_sexp_t key)
{
  gcry_sexp_t skey, pkey;

  pkey = gcry_sexp_find_token (key, "public-key", 0);
  if (!pkey)
    fail ("public part missing in return value\n");
  else
    {
      /* Fixme: Check more stuff.  */
      gcry_sexp_release (pkey);
    }

  skey = gcry_sexp_find_token (key, "private-key", 0);
  if (!skey)
    fail ("private part missing in return value\n");
  else
    {
      int rc = gcry_pk_testkey (skey);
      if (rc)
        fail ("gcry_pk_testkey failed: %s\n", gpg_strerror (rc));
      gcry_sexp_release (skey);
    }

  /* Finally check that gcry_pk_testkey also works on the entire
     S-expression.  */
  {
    int rc = gcry_pk_testkey (key);
    if (rc)
      fail ("gcry_pk_testkey failed on key pair: %s\n", gpg_strerror (rc));
  }
}


static void
check_ecc_keys (void)
{
  const char *curves[] = { "NIST P-521", "NIST P-384", "NIST P-256",
                           "Ed25519", NULL };
  int testno;
  gcry_sexp_t keyparm, key;
  int rc;

  for (testno=0; curves[testno]; testno++)
    {
      if (verbose)
        info ("creating ECC key using curve %s\n", curves[testno]);
      if (!strcmp (curves[testno], "Ed25519"))
        {
          /* Ed25519 isn't allowed in fips mode */
          if (in_fips_mode)
            continue;
          rc = gcry_sexp_build (&keyparm, NULL,
                                "(genkey(ecc(curve %s)(flags param eddsa)))",
                                curves[testno]);
        }
      else
        rc = gcry_sexp_build (&keyparm, NULL,
                              "(genkey(ecc(curve %s)(flags param)))",
                              curves[testno]);
      if (rc)
        die ("error creating S-expression: %s\n", gpg_strerror (rc));
      rc = gcry_pk_genkey (&key, keyparm);
      gcry_sexp_release (keyparm);
      if (rc)
        die ("error generating ECC key using curve %s: %s\n",
             curves[testno], gpg_strerror (rc));

      if (verbose > 1)
        show_sexp ("ECC key:\n", key);

      check_generated_ecc_key (key);

      gcry_sexp_release (key);
    }

  if (verbose)
    info ("creating ECC key using curve Ed25519 for ECDSA\n");
  rc = gcry_sexp_build (&keyparm, NULL, "(genkey(ecc(curve Ed25519)))");
  if (rc)
    die ("error creating S-expression: %s\n", gpg_strerror (rc));
  rc = gcry_pk_genkey (&key, keyparm);
  gcry_sexp_release (keyparm);
  if (rc && !in_fips_mode)
    die ("error generating ECC key using curve Ed25519 for ECDSA: %s\n",
         gpg_strerror (rc));
  else if (!rc && in_fips_mode)
    fail ("generating Ed25519 key must not work!");

  if (verbose && rc && in_fips_mode)
    info ("... correctly rejected key creation in FIPS mode (%s)\n",
          gpg_strerror (rc));

  if (!rc)
    {
      if (verbose > 1)
        show_sexp ("ECC key:\n", key);

      check_generated_ecc_key (key);
    }
  gcry_sexp_release (key);

  if (verbose)
    info ("creating ECC key using curve Ed25519 for ECDSA (nocomp)\n");
  rc = gcry_sexp_build (&keyparm, NULL,
                        "(genkey(ecc(curve Ed25519)(flags nocomp)))");
  if (rc)
    die ("error creating S-expression: %s\n", gpg_strerror (rc));
  rc = gcry_pk_genkey (&key, keyparm);
  gcry_sexp_release (keyparm);
  if (rc && !in_fips_mode)
    die ("error generating ECC key using curve Ed25519 for ECDSA"
         " (nocomp): %s\n",
         gpg_strerror (rc));
  else if (!rc && in_fips_mode)
    fail ("generating Ed25519 key must not work in FIPS mode!");

  if (verbose && rc && in_fips_mode)
    info ("... correctly rejected key creation in FIPS mode (%s)\n",
          gpg_strerror (rc));
  gcry_sexp_release (key);

  if (verbose)
    info ("creating ECC key using curve NIST P-384 for ECDSA\n");

  /* Must be specified as nistp384 (one word), because ecc_generate
   * uses _gcry_sexp_nth_string which takes the first word of the name
   * and thus libgcrypt can't find it later in its curves table.  */
  rc = gcry_sexp_build (&keyparm, NULL, "(genkey(ecc(curve nistp384)))");
  if (rc)
    die ("error creating S-expression: %s\n", gpg_strerror (rc));
  rc = gcry_pk_genkey (&key, keyparm);
  gcry_sexp_release (keyparm);
  if (rc)
    die ("error generating ECC key using curve NIST P-384 for ECDSA: %s\n",
         gpg_strerror (rc));

  if (verbose > 1)
    show_sexp ("ECC key:\n", key);

  check_generated_ecc_key (key);
  gcry_sexp_release (key);

  if (verbose)
    info ("creating ECC key using curve NIST P-384 for ECDSA (nocomp)\n");
  rc = gcry_sexp_build (&keyparm, NULL,
                        "(genkey(ecc(curve nistp384)(flags nocomp)))");
  if (rc)
    die ("error creating S-expression: %s\n", gpg_strerror (rc));
  rc = gcry_pk_genkey (&key, keyparm);
  gcry_sexp_release (keyparm);
  if (rc)
    die ("error generating ECC key using curve NIST P-384 for ECDSA"
         " (nocomp): %s\n",
         gpg_strerror (rc));

  if (verbose > 1)
    show_sexp ("ECC key:\n", key);

  check_generated_ecc_key (key);
  gcry_sexp_release (key);


  if (verbose)
    info ("creating ECC key using curve Ed25519 for ECDSA (transient-key)\n");
  rc = gcry_sexp_build (&keyparm, NULL,
                        "(genkey(ecc(curve Ed25519)(flags transient-key)))");
  if (rc)
    die ("error creating S-expression: %s\n", gpg_strerror (rc));
  rc = gcry_pk_genkey (&key, keyparm);
  gcry_sexp_release (keyparm);
  if (rc && !in_fips_mode)
    die ("error generating ECC key using curve Ed25519 for ECDSA"
         " (transient-key): %s\n",
         gpg_strerror (rc));
  else if (!rc && in_fips_mode)
    fail ("generating Ed25519 key must not work in FIPS mode!");

  if (verbose && rc && in_fips_mode)
    info ("... correctly rejected key creation in FIPS mode (%s)\n",
          gpg_strerror (rc));

  if (!rc)
    {
      if (verbose > 1)
        show_sexp ("ECC key:\n", key);
      check_generated_ecc_key (key);
    }
  gcry_sexp_release (key);

  if (verbose)
    info ("creating ECC key using curve Ed25519 for ECDSA "
          "(transient-key no-keytest)\n");
  rc = gcry_sexp_build (&keyparm, NULL,
                        "(genkey(ecc(curve Ed25519)"
                        "(flags transient-key no-keytest)))");
  if (rc)
    die ("error creating S-expression: %s\n", gpg_strerror (rc));
  rc = gcry_pk_genkey (&key, keyparm);
  gcry_sexp_release (keyparm);
  if (rc && !in_fips_mode)
    die ("error generating ECC key using curve Ed25519 for ECDSA"
         " (transient-key no-keytest): %s\n",
         gpg_strerror (rc));
  else if (!rc && in_fips_mode)
    fail ("generating Ed25519 key must not work in FIPS mode!");

  if (verbose && rc && in_fips_mode)
    info ("... correctly rejected key creation in FIPS mode (%s)\n",
          gpg_strerror (rc));

  if (!rc)
    {
      if (verbose > 1)
        show_sexp ("ECC key:\n", key);
      check_generated_ecc_key (key);
    }
  gcry_sexp_release (key);
}


static void
check_nonce (void)
{
  char a[32], b[32];
  int i,j;
  int oops=0;

  if (verbose)
    info ("checking gcry_create_nonce\n");

  gcry_create_nonce (a, sizeof a);
  for (i=0; i < 10; i++)
    {
      gcry_create_nonce (b, sizeof b);
      if (!memcmp (a, b, sizeof a))
        die ("identical nonce found\n");
    }
  for (i=0; i < 10; i++)
    {
      gcry_create_nonce (a, sizeof a);
      if (!memcmp (a, b, sizeof a))
        die ("identical nonce found\n");
    }

 again:
  for (i=1,j=0; i < sizeof a; i++)
    if (a[0] == a[i])
      j++;
  if (j+1 == sizeof (a))
    {
      if (oops)
        die ("impossible nonce found\n");
      oops++;
      gcry_create_nonce (a, sizeof a);
      goto again;
    }
}


static void
progress_cb (void *cb_data, const char *what, int printchar,
		  int current, int total)
{
  (void)cb_data;
  (void)what;
  (void)current;
  (void)total;

  if (printchar == '\n')
    fputs ( "<LF>", stdout);
  else
    putchar (printchar);
  fflush (stdout);
}


static void
usage (int mode)
{
  fputs ("usage: " PGM " [options] [{rsa|elg|dsa|ecc|nonce}]\n"
         "Options:\n"
         "  --verbose       be verbose\n"
         "  --debug         flyswatter\n"
         "  --fips          run in FIPS mode\n"
         "  --no-quick      To not use the quick RNG hack\n"
         "  --progress      print progress indicators\n",
         mode? stderr : stdout);
  if (mode)
    exit (1);
}

int
main (int argc, char **argv)
{
  int last_argc = -1;
  int opt_fips = 0;
  int with_progress = 0;
  int no_quick = 0;

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
          usage (0);
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
      else if (!strcmp (*argv, "--fips"))
        {
          argc--; argv++;
          opt_fips = 1;
        }
      else if (!strcmp (*argv, "--progress"))
        {
          argc--; argv++;
          with_progress = 1;
        }
      else if (!strcmp (*argv, "--no-quick"))
        {
          argc--; argv++;
          no_quick = 1;
        }
      else if (!strncmp (*argv, "--", 2))
        die ("unknown option '%s'", *argv);
      else
        break;
    }

  xgcry_control ((GCRYCTL_SET_VERBOSITY, (int)verbose));
  if (opt_fips)
    xgcry_control ((GCRYCTL_FORCE_FIPS_MODE, 0));

  if (!gcry_check_version (GCRYPT_VERSION))
    die ("version mismatch\n");

  if (!opt_fips)
    xgcry_control ((GCRYCTL_DISABLE_SECMEM, 0));

  xgcry_control ((GCRYCTL_INITIALIZATION_FINISHED, 0));
  if (debug)
    xgcry_control ((GCRYCTL_SET_DEBUG_FLAGS, 1u , 0));
  /* No valuable keys are create, so we can speed up our RNG. */
  if (!no_quick)
    xgcry_control ((GCRYCTL_ENABLE_QUICK_RANDOM, 0));
  if (with_progress)
    gcry_set_progress_handler (progress_cb, NULL);

  if ( gcry_fips_mode_active () )
    in_fips_mode = 1;

  if (opt_fips && !in_fips_mode)
    die ("failed to switch into FIPS mode\n");

  if (!argc)
    {
      check_rsa_keys ();
      check_elg_keys ();
      check_dsa_keys ();
      check_ecc_keys ();
      check_nonce ();
    }
  else
    {
      for (; argc; argc--, argv++)
        if (!strcmp (*argv, "rsa"))
          check_rsa_keys ();
        else if (!strcmp (*argv, "elg"))
          check_elg_keys ();
        else if (!strcmp (*argv, "dsa"))
          check_dsa_keys ();
        else if (!strcmp (*argv, "ecc"))
          check_ecc_keys ();
        else if (!strcmp (*argv, "nonce"))
          check_nonce ();
        else
          usage (1);
    }

  return error_count? 1:0;
}
