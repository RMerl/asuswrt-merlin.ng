/* t-cv25519.c - Check the cv25519 crypto
 * Copyright (C) 2016 g10 Code GmbH
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
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "stopwatch.h"

#define PGM "t-cv25519"
#include "t-common.h"
#define N_TESTS 18

static int in_fips_mode;

static void
print_mpi (const char *text, gcry_mpi_t a)
{
  gcry_error_t err;
  char *buf;
  void *bufaddr = &buf;

  err = gcry_mpi_aprint (GCRYMPI_FMT_HEX, bufaddr, NULL, a);
  if (err)
    fprintf (stderr, "%s: [error printing number: %s]\n",
             text, gpg_strerror (err));
  else
    {
      fprintf (stderr, "%s: %s\n", text, buf);
      gcry_free (buf);
    }
}


static void
show_note (const char *format, ...)
{
  va_list arg_ptr;

  if (!verbose && getenv ("srcdir"))
    fputs ("      ", stderr);  /* To align above "PASS: ".  */
  else
    fprintf (stderr, "%s: ", PGM);
  va_start (arg_ptr, format);
  vfprintf (stderr, format, arg_ptr);
  if (*format && format[strlen(format)-1] != '\n')
    putc ('\n', stderr);
  va_end (arg_ptr);
}


/* Convert STRING consisting of hex characters into its binary
   representation and return it as an allocated buffer. The valid
   length of the buffer is returned at R_LENGTH.  The string is
   delimited by end of string.  The function returns NULL on
   error.  */
static void *
hex2buffer (const char *string, size_t *r_length)
{
  const char *s;
  unsigned char *buffer;
  size_t length;

  buffer = xmalloc (strlen(string)/2+1);
  length = 0;
  for (s=string; *s; s +=2 )
    {
      if (!hexdigitp (s) || !hexdigitp (s+1))
        return NULL;           /* Invalid hex digits. */
      ((unsigned char*)buffer)[length++] = xtoi_2 (s);
    }
  *r_length = length;
  return buffer;
}

static void
reverse_buffer (unsigned char *buffer, unsigned int length)
{
  unsigned int tmp, i;

  for (i=0; i < length/2; i++)
    {
      tmp = buffer[i];
      buffer[i] = buffer[length-1-i];
      buffer[length-1-i] = tmp;
    }
}


/*
 * Test X25519 functionality through higher layer crypto routines.
 *
 * Input: K (as hex string), U (as hex string), R (as hex string)
 *
 * where R is expected result of X25519 (K, U).
 *
 * It calls gcry_pk_encrypt with Curve25519 private key and let
 * it compute X25519.
 */
static void
test_cv_hl (int testno, const char *k_str, const char *u_str,
            const char *result_str)
{
  gpg_error_t err;
  void *buffer = NULL;
  size_t buflen;
  gcry_sexp_t s_pk = NULL;
  gcry_mpi_t mpi_k = NULL;
  gcry_sexp_t s_data = NULL;
  gcry_sexp_t s_result = NULL;
  gcry_sexp_t s_tmp = NULL;
  unsigned char *res = NULL;
  size_t res_len;

  if (verbose > 1)
    info ("Running test %d\n", testno);

  if (!(buffer = hex2buffer (k_str, &buflen)) || buflen != 32)
    {
      fail ("error building s-exp for test %d, %s: %s",
            testno, "k", "invalid hex string");
      goto leave;
    }

  reverse_buffer (buffer, buflen);
  if ((err = gcry_mpi_scan (&mpi_k, GCRYMPI_FMT_USG, buffer, buflen, NULL)))
    {
      fail ("error converting MPI for test %d: %s", testno, gpg_strerror (err));
      goto leave;
    }

  if ((err = gcry_sexp_build (&s_data, NULL, "%m", mpi_k)))
    {
      fail ("error building s-exp for test %d, %s: %s",
            testno, "data", gpg_strerror (err));
      goto leave;
    }

  xfree (buffer);
  if (!(buffer = hex2buffer (u_str, &buflen)) || buflen != 32)
    {
      fail ("error building s-exp for test %d, %s: %s",
            testno, "u", "invalid hex string");
      goto leave;
    }

  /*
   * The procedure of decodeUCoordinate will be done internally
   * by _gcry_ecc_mont_decodepoint.  So, we just put the little-endian
   * binary to build S-exp.
   *
   * We could add the prefix 0x40, but libgcrypt also supports
   * format with no prefix.  So, it is OK not to put the prefix.
   */
  if ((err = gcry_sexp_build (&s_pk, NULL,
                              "(public-key"
                              " (ecc"
                              "  (curve \"Curve25519\")"
                              "  (flags djb-tweak)"
                              "  (q%b)))", (int)buflen, buffer)))
    {
      fail ("error building s-exp for test %d, %s: %s",
            testno, "pk", gpg_strerror (err));
      goto leave;
    }

  xfree (buffer);
  buffer = NULL;

  err = gcry_pk_encrypt (&s_result, s_data, s_pk);
  if (in_fips_mode)
    {
      if (!err)
        fail ("gcry_pk_encrypt is not expected to work in FIPS mode for test %d",
              testno);
      if (verbose > 1)
        info ("not executed in FIPS mode\n");
      goto leave;
    }
  if (err)
    fail ("gcry_pk_encrypt failed for test %d: %s", testno,
          gpg_strerror (err));

  s_tmp = gcry_sexp_find_token (s_result, "s", 0);
  if (!s_tmp || !(res = gcry_sexp_nth_buffer (s_tmp, 1, &res_len)))
    fail ("gcry_pk_encrypt failed for test %d: %s", testno, "missing value");
  else
    {
      char *r, *r0;
      int i;

      /* To skip the prefix 0x40, for-loop start with i=1 */
      r0 = r = xmalloc (2*(res_len)+1);
      if (!r0)
        {
          fail ("memory allocation for test %d", testno);
          goto leave;
        }

      for (i=1; i < res_len; i++, r += 2)
        snprintf (r, 3, "%02x", res[i]);
      if (strcmp (result_str, r0))
        {
          fail ("gcry_pk_encrypt failed for test %d: %s",
                testno, "wrong value returned");
          info ("  expected: '%s'", result_str);
          info ("       got: '%s'", r0);
        }
      xfree (r0);
    }

 leave:
  xfree (res);
  gcry_mpi_release (mpi_k);
  gcry_sexp_release (s_tmp);
  gcry_sexp_release (s_result);
  gcry_sexp_release (s_data);
  gcry_sexp_release (s_pk);
  xfree (buffer);
}

/*
 * Test X25519 functionality through the API for X25519.
 *
 * Input: K (as hex string), U (as hex string), R (as hex string)
 *
 * where R is expected result of X25519 (K, U).
 *
 */
static void
test_cv_x25519 (int testno, const char *k_str, const char *u_str,
                const char *result_str)
{
  gpg_error_t err;
  void *scalar = NULL;
  void *point = NULL;
  size_t buflen;
  unsigned char result[32];
  char result_hex[65];
  int i;
  int algo = GCRY_ECC_CURVE25519;
  unsigned int keylen;

  if (verbose > 1)
    info ("Running test %d\n", testno);

  if (!(keylen = gcry_ecc_get_algo_keylen (algo)))
    {
      fail ("error getting keylen for test %d", testno);
      goto leave;
    }

  if (keylen != 32)
    {
      fail ("error invalid keylen for test %d", testno);
      goto leave;
    }

  if (!(scalar = hex2buffer (k_str, &buflen)) || buflen != keylen)
    {
      fail ("error of input for test %d, %s: %s",
            testno, "k", "invalid hex string");
      goto leave;
    }

  if (!(point = hex2buffer (u_str, &buflen)) || buflen != keylen)
    {
      fail ("error of input for test %d, %s: %s",
            testno, "u", "invalid hex string");
      goto leave;
    }

  err = gcry_ecc_mul_point (algo, result, scalar, point);
  if (in_fips_mode)
    {
      if (!err)
        fail ("gcry_ecc_mul_point is not expected to work in FIPS mode for test %d",
              testno);
      if (verbose > 1)
        info ("not executed in FIPS mode\n");
      goto leave;
    }
  if (err)
    fail ("gcry_ecc_mul_point failed for test %d: %s", testno,
          gpg_strerror (err));

  for (i=0; i < keylen; i++)
    snprintf (&result_hex[i*2], 3, "%02x", result[i]);

  if (strcmp (result_str, result_hex))
    {
      fail ("gcry_ecc_mul_point failed for test %d: %s",
            testno, "wrong value returned");
      info ("  expected: '%s'", result_str);
      info ("       got: '%s'", result_hex);
    }

 leave:
  xfree (scalar);
  xfree (point);
}

static void
test_cv (int testno, const char *k_str, const char *u_str,
         const char *result_str)
{
  test_cv_hl (testno, k_str, u_str, result_str);
  test_cv_x25519 (testno, k_str, u_str, result_str);
}

/*
 * Test iterative X25519 computation through lower layer MPI routines.
 *
 * Input: K (as hex string), ITER, R (as hex string)
 *
 * where R is expected result of iterating X25519 by ITER times.
 *
 */
static void
test_it (int testno, const char *k_str, int iter, const char *result_str)
{
  gcry_ctx_t ctx;
  gpg_error_t err;
  void *buffer = NULL;
  size_t buflen;
  gcry_mpi_t mpi_k = NULL;
  gcry_mpi_t mpi_x = NULL;
  gcry_mpi_point_t P = NULL;
  gcry_mpi_point_t Q = NULL;
  int i;
  gcry_mpi_t mpi_kk = NULL;

  if (verbose > 1)
    info ("Running test %d: iteration=%d\n", testno, iter);

  gcry_mpi_ec_new (&ctx, NULL, "Curve25519");
  if (in_fips_mode)
    {
      if (ctx)
        fail ("gcry_mpi_ec_new should fail in FIPS mode for test %d",
              testno);
      if (verbose > 1)
        info ("not executed in FIPS mode\n");
      goto leave;
    }
  Q = gcry_mpi_point_new (0);

  if (!(buffer = hex2buffer (k_str, &buflen)) || buflen != 32)
    {
      fail ("error scanning MPI for test %d, %s: %s",
            testno, "k", "invalid hex string");
      goto leave;
    }
  reverse_buffer (buffer, buflen);
  if ((err = gcry_mpi_scan (&mpi_x, GCRYMPI_FMT_USG, buffer, buflen, NULL)))
    {
      fail ("error scanning MPI for test %d, %s: %s",
            testno, "x", gpg_strerror (err));
      goto leave;
    }

  xfree (buffer);
  buffer = NULL;

  P = gcry_mpi_point_set (NULL, mpi_x, NULL, GCRYMPI_CONST_ONE);

  mpi_k = gcry_mpi_copy (mpi_x);
  if (debug)
    print_mpi ("k", mpi_k);

  for (i = 0; i < iter; i++)
    {
      /*
       * Another variant of decodeScalar25519 thing.
       */
      mpi_kk = gcry_mpi_set (mpi_kk, mpi_k);
      gcry_mpi_set_bit (mpi_kk, 254);
      gcry_mpi_clear_bit (mpi_kk, 255);
      gcry_mpi_clear_bit (mpi_kk, 0);
      gcry_mpi_clear_bit (mpi_kk, 1);
      gcry_mpi_clear_bit (mpi_kk, 2);

      gcry_mpi_ec_mul (Q, mpi_kk, P, ctx);

      P = gcry_mpi_point_set (P, mpi_k, NULL, GCRYMPI_CONST_ONE);
      gcry_mpi_ec_get_affine (mpi_k, NULL, Q, ctx);

      if (debug)
        print_mpi ("k", mpi_k);
    }

  {
    unsigned char res[32];
    char *r, *r0;

    gcry_mpi_print (GCRYMPI_FMT_USG, res, 32, NULL, mpi_k);
    reverse_buffer (res, 32);

    r0 = r = xmalloc (65);
    if (!r0)
      {
        fail ("memory allocation for test %d", testno);
        goto leave;
      }

    for (i=0; i < 32; i++, r += 2)
      snprintf (r, 3, "%02x", res[i]);

    if (strcmp (result_str, r0))
      {
        fail ("curv25519 failed for test %d: %s",
              testno, "wrong value returned");
        info ("  expected: '%s'", result_str);
        info ("       got: '%s'", r0);
      }
    xfree (r0);
  }

 leave:
  gcry_mpi_release (mpi_kk);
  gcry_mpi_release (mpi_k);
  gcry_mpi_point_release (P);
  gcry_mpi_release (mpi_x);
  xfree (buffer);
  gcry_mpi_point_release (Q);
  gcry_ctx_release (ctx);
}

/*
 * X-coordinate of generator of the Curve25519.
 */
#define G_X "0900000000000000000000000000000000000000000000000000000000000000"

/*
 * Test Diffie-Hellman in RFC-7748.
 *
 * Note that it's not like the ECDH of OpenPGP, where we use
 * ephemeral public key.
 */
static void
test_dh (int testno, const char *a_priv_str, const char *a_pub_str,
          const char *b_priv_str, const char *b_pub_str,
          const char *result_str)
{
  /* Test A for private key corresponds to public key. */
  test_cv (testno, a_priv_str, G_X, a_pub_str);
  /* Test B for private key corresponds to public key. */
  test_cv (testno, b_priv_str, G_X, b_pub_str);
  /* Test DH with A's private key and B's public key. */
  test_cv (testno, a_priv_str, b_pub_str, result_str);
  /* Test DH with B's private key and A's public key. */
  test_cv (testno, b_priv_str, a_pub_str, result_str);
}


static void
check_cv25519 (void)
{
  int ntests;

  info ("Checking Curve25519.\n");

  ntests = 0;

  /*
   * Values are cited from RFC-7748: 5.2.  Test Vectors.
   * Following two tests are for the first type test.
   */
  test_cv (1,
           "a546e36bf0527c9d3b16154b82465edd62144c0ac1fc5a18506a2244ba449ac4",
           "e6db6867583030db3594c1a424b15f7c726624ec26b3353b10a903a6d0ab1c4c",
           "c3da55379de9c6908e94ea4df28d084f32eccf03491c71f754b4075577a28552");
  ntests++;
  test_cv (2,
           "4b66e9d4d1b4673c5ad22691957d6af5c11b6421e0ea01d42ca4169e7918ba0d",
           "e5210f12786811d3f4b7959d0538ae2c31dbe7106fc03c3efc4cd549c715a493",
           "95cbde9476e8907d7aade45cb4b873f88b595a68799fa152e6f8f7647aac7957");
  ntests++;

  /*
   * Additional test.  Value is from second type test.
   */
  test_cv (3,
           G_X,
           G_X,
           "422c8e7a6227d7bca1350b3e2bb7279f7897b87bb6854b783c60e80311ae3079");
  ntests++;

  /*
   * Following two tests are for the second type test,
   * with one iteration and 1,000 iterations.  (1,000,000 iterations
   * takes too long.)
   */
  test_it (4,
           G_X,
           1,
           "422c8e7a6227d7bca1350b3e2bb7279f7897b87bb6854b783c60e80311ae3079");
  ntests++;

  test_it (5,
           G_X,
           1000,
           "684cf59ba83309552800ef566f2f4d3c1c3887c49360e3875f2eb94d99532c51");
  ntests++;

  /*
   * Last test is from: 6.  Diffie-Hellman, 6.1.  Curve25519
   */
  test_dh (6,
           /* Alice's private key, a */
           "77076d0a7318a57d3c16c17251b26645df4c2f87ebc0992ab177fba51db92c2a",
           /* Alice's public key, X25519(a, 9) */
           "8520f0098930a754748b7ddcb43ef75a0dbf3a0d26381af4eba4a98eaa9b4e6a",
           /* Bob's private key, b */
           "5dab087e624a8a4b79e17f8b83800ee66f3bb1292618b6fd1c2f8b27ff88e0eb",
           /* Bob's public key, X25519(b, 9) */
           "de9edb7d7b7dc1b4d35b61c2ece435373f8343c85b78674dadfc7e146f882b4f",
           /* Their shared secret, K */
           "4a5d9d5ba4ce2de1728e3bf480350f25e07e21c947d19e3376f09b3c1e161742");
  ntests++;

  /* Seven tests which results 0. */
  test_cv (7,
           "a546e36bf0527c9d3b16154b82465edd62144c0ac1fc5a18506a2244ba449ac4",
           "0000000000000000000000000000000000000000000000000000000000000000",
           "0000000000000000000000000000000000000000000000000000000000000000");
  ntests++;

  test_cv (8,
           "a546e36bf0527c9d3b16154b82465edd62144c0ac1fc5a18506a2244ba449ac4",
           "0100000000000000000000000000000000000000000000000000000000000000",
           "0000000000000000000000000000000000000000000000000000000000000000");
  ntests++;

  test_cv (9,
           "a546e36bf0527c9d3b16154b82465edd62144c0ac1fc5a18506a2244ba449ac4",
           "e0eb7a7c3b41b8ae1656e3faf19fc46ada098deb9c32b1fd866205165f49b800",
           "0000000000000000000000000000000000000000000000000000000000000000");
  ntests++;

  test_cv (10,
           "a546e36bf0527c9d3b16154b82465edd62144c0ac1fc5a18506a2244ba449ac4",
           "5f9c95bca3508c24b1d0b1559c83ef5b04445cc4581c8e86d8224eddd09f1157",
           "0000000000000000000000000000000000000000000000000000000000000000");
  ntests++;

  test_cv (11,
           "a546e36bf0527c9d3b16154b82465edd62144c0ac1fc5a18506a2244ba449ac4",
           "ecffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff7f",
           "0000000000000000000000000000000000000000000000000000000000000000");
  ntests++;

  test_cv (12,
           "a546e36bf0527c9d3b16154b82465edd62144c0ac1fc5a18506a2244ba449ac4",
           "edffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff7f",
           "0000000000000000000000000000000000000000000000000000000000000000");
  ntests++;

  test_cv (13,
           "a546e36bf0527c9d3b16154b82465edd62144c0ac1fc5a18506a2244ba449ac4",
           "eeffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff7f",
           "0000000000000000000000000000000000000000000000000000000000000000");
  ntests++;

  /* Five tests which resulted 0 if decodeUCoordinate didn't change MSB. */
  test_cv (14,
           "a546e36bf0527c9d3b16154b82465edd62144c0ac1fc5a18506a2244ba449ac4",
           "cdeb7a7c3b41b8ae1656e3faf19fc46ada098deb9c32b1fd866205165f49b880",
           "7ce548bc4919008436244d2da7a9906528fe3a6d278047654bd32d8acde9707b");
  ntests++;

  test_cv (15,
           "a546e36bf0527c9d3b16154b82465edd62144c0ac1fc5a18506a2244ba449ac4",
           "4c9c95bca3508c24b1d0b1559c83ef5b04445cc4581c8e86d8224eddd09f11d7",
           "e17902e989a034acdf7248260e2c94cdaf2fe1e72aaac7024a128058b6189939");
  ntests++;

  test_cv (16,
           "a546e36bf0527c9d3b16154b82465edd62144c0ac1fc5a18506a2244ba449ac4",
           "d9ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
           "ea6e6ddf0685c31e152d5818441ac9ac8db1a01f3d6cb5041b07443a901e7145");
  ntests++;

  test_cv (17,
           "a546e36bf0527c9d3b16154b82465edd62144c0ac1fc5a18506a2244ba449ac4",
           "daffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
           "845ddce7b3a9b3ee01a2f1fd4282ad293310f7a232cbc5459fb35d94bccc9d05");
  ntests++;

  test_cv (18,
           "a546e36bf0527c9d3b16154b82465edd62144c0ac1fc5a18506a2244ba449ac4",
           "dbffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
           "6989e2cb1cea159acf121b0af6bf77493189c9bd32c2dac71669b540f9488247");
  ntests++;

  if (ntests != N_TESTS)
    fail ("did %d tests but expected %d", ntests, N_TESTS);
  else if ((ntests % 256))
    show_note ("%d tests done\n", ntests);
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
    xgcry_control ((GCRYCTL_SET_DEBUG_FLAGS, 1u , 0));
  xgcry_control ((GCRYCTL_ENABLE_QUICK_RANDOM, 0));
  xgcry_control ((GCRYCTL_INITIALIZATION_FINISHED, 0));

  if (gcry_fips_mode_active ())
    in_fips_mode = 1;

  start_timer ();
  check_cv25519 ();
  stop_timer ();

  info ("All tests completed in %s.  Errors: %d\n",
        elapsed_time (1), error_count);
  return !!error_count;
}
