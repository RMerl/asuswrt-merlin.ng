/* t-x448.c - Check the X488 computation
 * Copyright (C) 2019 g10 Code GmbH
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
 * SPDX-License-Identifier: LGPL-2.1+
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

#define PGM "t-x448"
#include "t-common.h"
#define N_TESTS 9

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
 * Test X448 functionality through higher layer crypto routines.
 *
 * Input: K (as hex string), U (as hex string), R (as hex string)
 *
 * where R is expected result of X448 (K, U).
 *
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

  if (!(buffer = hex2buffer (k_str, &buflen)) || buflen != 56)
    {
      fail ("error building s-exp for test %d, %s: %s",
            testno, "k", "invalid hex string");
      goto leave;
    }

  mpi_k = gcry_mpi_set_opaque (NULL, buffer, buflen*8);
  if ((err = gcry_sexp_build (&s_data, NULL, "%m", mpi_k)))
    {
      fail ("error building s-exp for test %d, %s: %s",
            testno, "data", gpg_strerror (err));
      goto leave;
    }

  if (!(buffer = hex2buffer (u_str, &buflen)) || buflen != 56)
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
                              "  (curve \"X448\")"
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
    fail ("gcry_pk_encrypt goto leavefailed for test %d: %s", testno,
          gpg_strerror (err));

  s_tmp = gcry_sexp_find_token (s_result, "s", 0);
  if (!s_tmp || !(res = gcry_sexp_nth_buffer (s_tmp, 1, &res_len)))
    fail ("gcry_pk_encrypt failed for test %d: %s", testno, "missing value");
  else
    {
      char *r, *r0;
      int i;

      r0 = r = xmalloc (2*(res_len)+1);
      if (!r0)
        {
          fail ("memory allocation for test %d", testno);
          goto leave;
        }

      for (i=0; i < res_len; i++, r += 2)
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
 * Test X448 functionality through the API for X448.
 *
 * Input: K (as hex string), U (as hex string), R (as hex string)
 *
 * where R is expected result of X448 (K, U).
 *
 */
static void
test_cv_x448 (int testno, const char *k_str, const char *u_str,
                const char *result_str)
{
  gpg_error_t err;
  void *scalar;
  void *point = NULL;
  size_t buflen;
  unsigned char result[56];
  char result_hex[113];
  int i;

  if (verbose > 1)
    info ("Running test %d\n", testno);

  if (!(scalar = hex2buffer (k_str, &buflen)) || buflen != 56)
    {
      fail ("error building s-exp for test %d, %s: %s",
            testno, "k", "invalid hex string");
      goto leave;
    }

  if (!(point = hex2buffer (u_str, &buflen)) || buflen != 56)
    {
      fail ("error building s-exp for test %d, %s: %s",
            testno, "u", "invalid hex string");
      goto leave;
    }

  err = gcry_ecc_mul_point (GCRY_ECC_CURVE448, result, scalar, point);
  if (in_fips_mode)
    {
      if (err != GPG_ERR_NOT_SUPPORTED)
        fail ("gcry_ecc_mul_point is not expected to work in FIPS mode for test %d: %s",
              testno, gpg_strerror (err));
      if (verbose > 1)
        info ("not executed in FIPS mode\n");
      goto leave;
    }
  if (err)
    fail ("gcry_ecc_mul_point failed for test %d: %s", testno,
          gpg_strerror (err));

  for (i=0; i < 56; i++)
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
  test_cv_x448 (testno, k_str, u_str, result_str);
}

/*
 * Test iterative X448 computation through lower layer MPI routines.
 *
 * Input: K (as hex string), ITER, R (as hex string)
 *
 * where R is expected result of iterating X448 by ITER times.
 *
 */
static void
test_it (int testno, const char *k_str, int iter, const char *result_str)
{
  gcry_ctx_t ctx = NULL;
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

  gcry_mpi_ec_new (&ctx, NULL, "X448");
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

  if (!(buffer = hex2buffer (k_str, &buflen)) || buflen != 56)
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
       * Another variant of decodeScalar448 thing.
       */
      mpi_kk = gcry_mpi_set (mpi_kk, mpi_k);
      gcry_mpi_set_bit (mpi_kk, 447);
      gcry_mpi_clear_bit (mpi_kk, 0);
      gcry_mpi_clear_bit (mpi_kk, 1);

      gcry_mpi_ec_mul (Q, mpi_kk, P, ctx);

      P = gcry_mpi_point_set (P, mpi_k, NULL, GCRYMPI_CONST_ONE);
      gcry_mpi_ec_get_affine (mpi_k, NULL, Q, ctx);

      if (debug)
        print_mpi ("k", mpi_k);
    }

  {
    unsigned char res[56];
    char *r, *r0;

    gcry_mpi_print (GCRYMPI_FMT_USG, res, 56, NULL, mpi_k);
    reverse_buffer (res, 56);

    r0 = r = xmalloc (113);
    if (!r0)
      {
        fail ("memory allocation for test %d", testno);
        goto leave;
      }

    for (i=0; i < 56; i++, r += 2)
      snprintf (r, 3, "%02x", res[i]);

    if (strcmp (result_str, r0))
      {
        fail ("X448 failed for test %d: %s",
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
 * X-coordinate of generator of the X448.
 */
#define G_X ("0500000000000000000000000000000000000000000000000000000000000000" \
             "000000000000000000000000000000000000000000000000")

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
check_x448 (void)
{
  int ntests;

  info ("Checking X448.\n");

  ntests = 0;

  /*
   * Values are cited from RFC-7748: 5.2.  Test Vectors.
   * Following two tests are for the first type test.
   */
  test_cv (1,
           "3d262fddf9ec8e88495266fea19a34d28882acef045104d0d1aae121"
           "700a779c984c24f8cdd78fbff44943eba368f54b29259a4f1c600ad3",
           "06fce640fa3487bfda5f6cf2d5263f8aad88334cbd07437f020f08f9"
           "814dc031ddbdc38c19c6da2583fa5429db94ada18aa7a7fb4ef8a086",
           "ce3e4ff95a60dc6697da1db1d85e6afbdf79b50a2412d7546d5f239f"
           "e14fbaadeb445fc66a01b0779d98223961111e21766282f73dd96b6f");
  ntests++;
  test_cv (2,
           "203d494428b8399352665ddca42f9de8fef600908e0d461cb021f8c5"
           "38345dd77c3e4806e25f46d3315c44e0a5b4371282dd2c8d5be3095f",
           "0fbcc2f993cd56d3305b0b7d9e55d4c1a8fb5dbb52f8e9a1e9b6201b"
           "165d015894e56c4d3570bee52fe205e28a78b91cdfbde71ce8d157db",
           "884a02576239ff7a2f2f63b2db6a9ff37047ac13568e1e30fe63c4a7"
           "ad1b3ee3a5700df34321d62077e63633c575c1c954514e99da7c179d");
  ntests++;

  /*
   * Additional test.  Value is from second type test.
   */
  test_cv (3,
           G_X,
           G_X,
           "3f482c8a9f19b01e6c46ee9711d9dc14fd4bf67af30765c2ae2b846a"
           "4d23a8cd0db897086239492caf350b51f833868b9bc2b3bca9cf4113");
  ntests++;

  /*
   * Following two tests are for the second type test,
   * with one iteration and 1,000 iterations.  (1,000,000 iterations
   * takes too long.)
   */
  test_it (4,
           G_X,
           1,
           "3f482c8a9f19b01e6c46ee9711d9dc14fd4bf67af30765c2ae2b846a"
           "4d23a8cd0db897086239492caf350b51f833868b9bc2b3bca9cf4113");
  ntests++;

  test_it (5,
           G_X,
           1000,
           "aa3b4749d55b9daf1e5b00288826c467274ce3ebbdd5c17b975e09d4"
           "af6c67cf10d087202db88286e2b79fceea3ec353ef54faa26e219f38");
  ntests++;

  /*
   * Last test is from: 6.  Diffie-Hellman, 6.2.  Curve448
   */
  test_dh (6,
           /* Alice's private key, a */
           "9a8f4925d1519f5775cf46b04b5800d4ee9ee8bae8bc5565d498c28d"
           "d9c9baf574a9419744897391006382a6f127ab1d9ac2d8c0a598726b",
           /* Alice's public key, X448(a, 5) */
           "9b08f7cc31b7e3e67d22d5aea121074a273bd2b83de09c63faa73d2c"
           "22c5d9bbc836647241d953d40c5b12da88120d53177f80e532c41fa0",
           /* Bob's private key, b */
           "1c306a7ac2a0e2e0990b294470cba339e6453772b075811d8fad0d1d"
           "6927c120bb5ee8972b0d3e21374c9c921b09d1b0366f10b65173992d",
           /* Bob's public key, X448(b, 5) */
           "3eb7a829b0cd20f5bcfc0b599b6feccf6da4627107bdb0d4f345b430"
           "27d8b972fc3e34fb4232a13ca706dcb57aec3dae07bdc1c67bf33609",
           /* Their shared secret, K */
           "07fff4181ac6cc95ec1c16a94a0f74d12da232ce40a77552281d282b"
           "b60c0b56fd2464c335543936521c24403085d59a449a5037514a879d");
  ntests++;

  /* three tests which results 0. */
  test_cv (7,
           "3d262fddf9ec8e88495266fea19a34d28882acef045104d0d1aae121"
           "700a779c984c24f8cdd78fbff44943eba368f54b29259a4f1c600ad3",
           "00000000000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000000000",
           "00000000000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000000000");
  ntests++;

  test_cv (8,
           "3d262fddf9ec8e88495266fea19a34d28882acef045104d0d1aae121"
           "700a779c984c24f8cdd78fbff44943eba368f54b29259a4f1c600ad3",
           "01000000000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000000000",
           "00000000000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000000000");
  ntests++;

  test_cv (9,
           "3d262fddf9ec8e88495266fea19a34d28882acef045104d0d1aae121"
           "700a779c984c24f8cdd78fbff44943eba368f54b29259a4f1c600ad3",
           "feffffffffffffffffffffffffffffffffffffffffffffffffffffff"
           "feffffffffffffffffffffffffffffffffffffffffffffffffffffff",
           "00000000000000000000000000000000000000000000000000000000"
           "00000000000000000000000000000000000000000000000000000000");
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
  check_x448 ();
  stop_timer ();

  info ("All tests completed in %s.  Errors: %d\n",
        elapsed_time (1), error_count);
  return !!error_count;
}
