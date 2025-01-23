/* mpitests.c  -  basic mpi tests
 *	Copyright (C) 2001, 2002, 2003, 2006 Free Software Foundation, Inc.
 * Copyright (C) 2013 g10 Code GmbH
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

#ifdef _GCRYPT_IN_LIBGCRYPT
# include "../src/gcrypt-int.h"
#else
# include <gcrypt.h>
#endif

#define PGM "mpitests"
#include "t-common.h"


/* Set up some test patterns */

/* 48 bytes with value 1: this results in 8 limbs for 64bit limbs, 16limb for 32 bit limbs */
unsigned char ones[] = {
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01
};

/* 48 bytes with value 2: this results in 8 limbs for 64bit limbs, 16limb for 32 bit limbs */
unsigned char twos[] = {
  0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
  0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
  0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02
};

/* 48 bytes with value 3: this results in 8 limbs for 64bit limbs, 16limb for 32 bit limbs */
unsigned char threes[] = {
  0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
  0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
  0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03
};

/* 48 bytes with value 0x80: this results in 8 limbs for 64bit limbs, 16limb for 32 bit limbs */
unsigned char eighties[] = {
  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
  0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80
};

/* 48 bytes with value 0xff: this results in 8 limbs for 64bit limbs, 16limb for 32 bit limbs */
unsigned char manyff[] = {
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};


static int
test_const_and_immutable (void)
{
  gcry_mpi_t one, second_one;

  one = gcry_mpi_set_ui (NULL, 1);
  if (gcry_mpi_get_flag (one, GCRYMPI_FLAG_IMMUTABLE)
      || gcry_mpi_get_flag (one, GCRYMPI_FLAG_CONST))
    die ("immutable or const flag initially set\n");

  second_one = gcry_mpi_copy (one);
  if (gcry_mpi_get_flag (second_one, GCRYMPI_FLAG_IMMUTABLE))
    die ("immutable flag set after copy\n");
  if (gcry_mpi_get_flag (second_one, GCRYMPI_FLAG_CONST))
    die ("const flag set after copy\n");
  gcry_mpi_release (second_one);

  gcry_mpi_set_flag (one, GCRYMPI_FLAG_IMMUTABLE);
  if (!gcry_mpi_get_flag (one, GCRYMPI_FLAG_IMMUTABLE))
    die ("failed to set immutable flag\n");
  if (gcry_mpi_get_flag (one, GCRYMPI_FLAG_CONST))
    die ("const flag unexpectly set\n");

  second_one = gcry_mpi_copy (one);
  if (gcry_mpi_get_flag (second_one, GCRYMPI_FLAG_IMMUTABLE))
    die ("immutable flag not cleared after copy\n");
  if (gcry_mpi_get_flag (second_one, GCRYMPI_FLAG_CONST))
    die ("const flag unexpectly set after copy\n");
  gcry_mpi_release (second_one);

  gcry_mpi_clear_flag (one, GCRYMPI_FLAG_IMMUTABLE);
  if (gcry_mpi_get_flag (one, GCRYMPI_FLAG_IMMUTABLE))
    die ("failed to clear immutable flag\n");
  if (gcry_mpi_get_flag (one, GCRYMPI_FLAG_CONST))
    die ("const flag unexpectly set\n");

  gcry_mpi_set_flag (one, GCRYMPI_FLAG_CONST);
  if (!gcry_mpi_get_flag (one, GCRYMPI_FLAG_CONST))
    die ("failed to set const flag\n");
  if (!gcry_mpi_get_flag (one, GCRYMPI_FLAG_IMMUTABLE))
    die ("failed to set immutable flag with const flag\n");

  second_one = gcry_mpi_copy (one);
  if (gcry_mpi_get_flag (second_one, GCRYMPI_FLAG_IMMUTABLE))
    die ("immutable flag not cleared after copy\n");
  if (gcry_mpi_get_flag (second_one, GCRYMPI_FLAG_CONST))
    die ("const flag not cleared after copy\n");
  gcry_mpi_release (second_one);

  gcry_mpi_clear_flag (one, GCRYMPI_FLAG_IMMUTABLE);
  if (!gcry_mpi_get_flag (one, GCRYMPI_FLAG_IMMUTABLE))
    die ("clearing immutable flag not ignored for a constant MPI\n");
  if (!gcry_mpi_get_flag (one, GCRYMPI_FLAG_CONST))
    die ("const flag unexpectly cleared\n");


  second_one = gcry_mpi_set (NULL, GCRYMPI_CONST_ONE);
  if (gcry_mpi_get_flag (second_one, GCRYMPI_FLAG_IMMUTABLE))
    die ("immutable flag not cleared by mpi_set (NULL,x)\n");
  if (gcry_mpi_get_flag (second_one, GCRYMPI_FLAG_CONST))
    die ("const flag not cleared by mpi_set (NULL,x)\n");
  gcry_mpi_release (second_one);

  second_one = gcry_mpi_set_ui (NULL, 42);
  gcry_mpi_set (second_one, GCRYMPI_CONST_ONE);
  if (gcry_mpi_get_flag (second_one, GCRYMPI_FLAG_IMMUTABLE))
    die ("immutable flag not cleared after mpi_set (a,x)\n");
  if (gcry_mpi_get_flag (second_one, GCRYMPI_FLAG_CONST))
    die ("const flag not cleared mpi_set (a,x)\n");
  gcry_mpi_release (second_one);


  /* Due to the the constant flag the release below should be a NOP
     and will leak memory.  */
  gcry_mpi_release (one);
  return 1;
}


static void
test_opaque (void)
{
  gcry_mpi_t a;
  char *p;
  unsigned int nbits;

  p = gcry_xstrdup ("This is a test buffer");
  a = gcry_mpi_set_opaque (NULL, p, 21*8+1); /* (a non byte aligned length) */

  if (!gcry_mpi_get_flag (a, GCRYMPI_FLAG_OPAQUE))
    die ("opaque flag not set\n");

  p = gcry_mpi_get_opaque (a, &nbits);
  if (!p)
    die ("gcry_mpi_get_opaque returned NULL\n");
  if (nbits != 21*8+1)
    die ("gcry_mpi_get_opaque returned a changed bit size\n");
  if (strcmp (p, "This is a test buffer"))
    die ("gcry_mpi_get_opaque returned a changed buffer\n");

  if (debug)
    gcry_log_debugmpi ("mpi", a);
  gcry_mpi_release (a);

  p = gcry_xstrdup ("This is a test buffer");
  a = gcry_mpi_set_opaque_copy (NULL, p, 21*8+1);
  gcry_free (p);

  if (!gcry_mpi_get_flag (a, GCRYMPI_FLAG_OPAQUE))
    die ("opaque flag not set\n");

  p = gcry_mpi_get_opaque (a, &nbits);
  if (!p)
    die ("gcry_mpi_get_opaque returned NULL\n");
  if (nbits != 21*8+1)
    die ("gcry_mpi_get_opaque returned a changed bit size\n");
  if (strcmp (p, "This is a test buffer"))
    die ("gcry_mpi_get_opaque returned a changed buffer\n");

  if (debug)
    gcry_log_debugmpi ("mpi", a);

  gcry_mpi_release (a);
}


static void
test_maxsize (void)
{
  gpg_error_t err;
  gcry_mpi_t a;
  unsigned int val;
  char buffer[2+2048]; /* For PGP: 2 length bytes and 16384 bits.  */

  memset (buffer, 0x55, sizeof buffer);

  /* We use a short buffer but a give a too large length to simulate a
   * programming error.  In case this test fails (i.e. die() is
   * called) the scan function may have access data outside of BUFFER
   * which may result in a segv but we ignore that to avoid actually
   * allocating such a long buffer.  */
  err = gcry_mpi_scan (&a, GCRYMPI_FMT_USG, buffer, 16*1024*1024 +1, NULL);
  if (gpg_err_code (err) != GPG_ERR_INV_OBJ)
    die ("gcry_mpi_scan does not detect its generic input limit\n");

  /* Now test the PGP limit.  The scan code check the two length bytes
   * from the buffer and thus it is sufficient to fake them.  */
  buffer[0] = (16385 >> 8);
  buffer[1] = (16385 & 0xff);
  err = gcry_mpi_scan (&a, GCRYMPI_FMT_PGP, buffer, sizeof buffer, NULL);
  if (gpg_err_code (err) != GPG_ERR_INV_OBJ)
    die ("gcry_mpi_scan does not detect the PGP input limit\n");

  buffer[0] = (16384 >> 8);
  buffer[1] = (16384 & 0xff);

  err = gcry_mpi_scan (&a, GCRYMPI_FMT_PGP, buffer, sizeof buffer, NULL);
  if (err)
    die ("gcry_mpi_scan did not parse a large PGP: %s\n", gpg_strerror (err));

  /* Let's also test get_ui.  */
  gcry_mpi_set_ui (a, 0);
  val = 4711;
  err = gcry_mpi_get_ui (&val, a);
  if (err || val != 0)
    die ("gcry_mpi_get_ui failed at %d: %s\n", __LINE__, gpg_strerror (err));

  gcry_mpi_sub_ui (a, a, 1);
  val = 4711;
  err = gcry_mpi_get_ui (&val, a);
  if (gpg_err_code (err) != GPG_ERR_ERANGE || val != 4711)
    die ("gcry_mpi_get_ui failed at %d: %s\n", __LINE__, gpg_strerror (err));

  gcry_mpi_set_ui (a, 0xffffffff);
  val = 4711;
  err = gcry_mpi_get_ui (&val, a);
  if (err || val != 0xffffffff)
    die ("gcry_mpi_get_ui failed at %d: %s\n", __LINE__, gpg_strerror (err));

  if (sizeof (val) == 4)
    {
      gcry_mpi_add_ui (a, a, 1);
      err = gcry_mpi_get_ui (&val, a);
      if (gpg_err_code (err) != GPG_ERR_ERANGE)
        die ("gcry_mpi_get_ui failed at %d: %s\n", __LINE__,gpg_strerror (err));
    }

  gcry_mpi_release (a);

}


static void
test_cmp (void)
{
  gpg_error_t rc;
  gcry_mpi_t zero, zero2;
  gcry_mpi_t one;
  gcry_mpi_t two;
  gcry_mpi_t all_ones;
  gcry_mpi_t opa1, opa2;
  gcry_mpi_t opa1s, opa2s;
  gcry_mpi_t opa0, opa02;

  zero = gcry_mpi_new (0);
  zero2= gcry_mpi_set_ui (NULL, 0);
  one  = gcry_mpi_set_ui (NULL, 1);
  two  = gcry_mpi_set_ui (NULL, 2);
  rc = gcry_mpi_scan (&all_ones, GCRYMPI_FMT_USG, ones, sizeof(ones), NULL);
  if (rc)
    die ("scanning number failed at line %d", __LINE__);
  opa0  = gcry_mpi_set_opaque (NULL, gcry_xstrdup ("a"), 0);
  opa02 = gcry_mpi_set_opaque (NULL, gcry_xstrdup ("b"), 0);
  opa1  = gcry_mpi_set_opaque (NULL, gcry_xstrdup ("aaaaaaaaaaaaaaaa"), 16*8);
  opa1s = gcry_mpi_set_opaque (NULL, gcry_xstrdup ("a"), 1*8);
  opa2  = gcry_mpi_set_opaque (NULL, gcry_xstrdup ("bbbbbbbbbbbbbbbb"), 16*8);
  opa2s = gcry_mpi_set_opaque (NULL, gcry_xstrdup ("b"), 1*8);


  /* Single limb test with cmp_ui */
  if (gcry_mpi_cmp_ui (zero, 0))
    fail ("mpi_cmp_ui failed at line %d", __LINE__);
  if (!(gcry_mpi_cmp_ui (zero, 1) < 0))
    fail ("mpi_cmp_ui failed at line %d", __LINE__);
  if (!(gcry_mpi_cmp_ui (zero, (-1)) < 0))
    fail ("mpi_cmp_ui failed at line %d", __LINE__);

  if (gcry_mpi_cmp_ui (two, 2))
    fail ("mpi_cmp_ui failed at line %d", __LINE__);
  if (!(gcry_mpi_cmp_ui (two, 3) < 0))
    fail ("mpi_cmp_ui failed at line %d", __LINE__);
  if (!(gcry_mpi_cmp_ui (two, 1) > 0))
    fail ("mpi_cmp_ui failed at line %d", __LINE__);

  /* Multi limb tests with cmp_ui.  */
  if (!(gcry_mpi_cmp_ui (all_ones, 0) > 0))
    fail ("mpi_cmp_ui failed at line %d", __LINE__);
  if (!(gcry_mpi_cmp_ui (all_ones, (-1)) > 0))
    fail ("mpi_cmp_ui failed at line %d", __LINE__);

  /* Single limb test with cmp */
  if (gcry_mpi_cmp (zero, zero2))
    fail ("mpi_cmp failed at line %d", __LINE__);
  if (!(gcry_mpi_cmp (zero, one) < 0))
    fail ("mpi_cmp failed at line %d", __LINE__);
  if (!(gcry_mpi_cmp (one, zero) > 0))
    fail ("mpi_cmp failed at line %d", __LINE__);

  gcry_mpi_neg (one, one);
  if (!(gcry_mpi_cmp (zero, one) > 0))
    fail ("mpi_cmp failed at line %d", __LINE__);
  if (!(gcry_mpi_cmp (one, zero) < 0))
    fail ("mpi_cmp failed at line %d", __LINE__);
  if (!(gcry_mpi_cmp (one, two) < 0))
    fail ("mpi_cmp failed at line %d", __LINE__);
  gcry_mpi_neg (one, one);

  if (!(gcry_mpi_cmp (one, two) < 0))
    fail ("mpi_cmp failed at line %d", __LINE__);
  if (!(gcry_mpi_cmp (two, one) > 0))
    fail ("mpi_cmp failed at line %d", __LINE__);
  if (!(gcry_mpi_cmp (one, all_ones) < 0))
    fail ("mpi_cmp failed at line %d", __LINE__);

  /* Tests with opaque values.  */
  if (!(gcry_mpi_cmp (opa1, one) < 0))
    fail ("mpi_cmp failed at line %d", __LINE__);
  if (!(gcry_mpi_cmp (one, opa1) > 0))
    fail ("mpi_cmp failed at line %d", __LINE__);
  if (!(gcry_mpi_cmp (opa0, opa02) == 0))
    fail ("mpi_cmp failed at line %d", __LINE__);
  if (!(gcry_mpi_cmp (opa1s, opa1) < 0))
    fail ("mpi_cmp failed at line %d", __LINE__);
  if (!(gcry_mpi_cmp (opa2, opa1s) > 0))
    fail ("mpi_cmp failed at line %d", __LINE__);
  if (!(gcry_mpi_cmp (opa1, opa2) < 0))
    fail ("mpi_cmp failed at line %d", __LINE__);
  if (!(gcry_mpi_cmp (opa2, opa1) > 0))
    fail ("mpi_cmp failed at line %d", __LINE__);
  if (!(gcry_mpi_cmp (opa1, opa1) == 0))
    fail ("mpi_cmp failed at line %d", __LINE__);


  gcry_mpi_release(opa2s);
  gcry_mpi_release(opa2);
  gcry_mpi_release(opa1s);
  gcry_mpi_release(opa1);
  gcry_mpi_release(opa02);
  gcry_mpi_release(opa0);
  gcry_mpi_release(all_ones);
  gcry_mpi_release(two);
  gcry_mpi_release(one);
  gcry_mpi_release(zero2);
  gcry_mpi_release(zero);
}


static int
test_add (void)
{
  gcry_mpi_t one;
  gcry_mpi_t two;
  gcry_mpi_t ff;
  gcry_mpi_t result;
  gcry_mpi_t minusfive;
  char *pc;

  gcry_mpi_scan(&one, GCRYMPI_FMT_USG, ones, sizeof(ones), NULL);
  gcry_mpi_scan(&two, GCRYMPI_FMT_USG, twos, sizeof(twos), NULL);
  gcry_mpi_scan(&ff, GCRYMPI_FMT_USG, manyff, sizeof(manyff), NULL);
  result = gcry_mpi_new(0);

  gcry_mpi_add(result, one, two);
  gcry_mpi_aprint(GCRYMPI_FMT_HEX, (unsigned char **)&pc, NULL, result);
  if (debug)
    gcry_log_debug ("Result of one plus two:\n%s\n", pc);
  if (strcmp (pc, "030303030303030303030303030303030303030303030303"
                  "030303030303030303030303030303030303030303030303") != 0)
    fail ("mpi_add failed at line %d", __LINE__);
  gcry_free(pc);

  gcry_mpi_add(result, ff, one);
  gcry_mpi_aprint(GCRYMPI_FMT_HEX, (unsigned char **)&pc, NULL, result);
  if (debug)
    gcry_log_debug ("Result of ff plus one:\n%s\n", pc);
  if (strcmp (pc, "010101010101010101010101010101010101010101010101"
                  "01010101010101010101010101010101010101010101010100") != 0)
    fail ("mpi_add failed at line %d", __LINE__);
  gcry_free(pc);

  gcry_mpi_scan(&minusfive, GCRYMPI_FMT_HEX, "-5", 0, NULL);
  gcry_mpi_add_ui (result, minusfive, 2);

  gcry_mpi_aprint(GCRYMPI_FMT_HEX, (unsigned char **)&pc, NULL, result);
  if (debug)
    gcry_log_debug ("Result of minus five plus two:\n%s\n", pc);
  if (strcmp (pc, "-03") != 0)
    fail ("mpi_add_ui failed at line %d", __LINE__);
  gcry_free(pc);

  gcry_mpi_add_ui (result, result, 3);

  gcry_mpi_aprint(GCRYMPI_FMT_HEX, (unsigned char **)&pc, NULL, result);
  if (debug)
    gcry_log_debug ("Result of minus three plus three:\n%s\n", pc);
  if (strcmp (pc, "00") != 0)
    fail ("mpi_add_ui failed at line %d", __LINE__);
  gcry_free(pc);

  gcry_mpi_release(one);
  gcry_mpi_release(two);
  gcry_mpi_release(ff);
  gcry_mpi_release(result);
  gcry_mpi_release(minusfive);
  return 1;
}


static int
test_sub (void)
{
  gcry_mpi_t zero;
  gcry_mpi_t one;
  gcry_mpi_t two;
  gcry_mpi_t five;
  gcry_mpi_t result;
  gcry_mpi_t minusfive;
  char *pc;

  gcry_mpi_scan(&one, GCRYMPI_FMT_USG, ones, sizeof(ones), NULL);
  gcry_mpi_scan(&two, GCRYMPI_FMT_USG, twos, sizeof(twos), NULL);
  result = gcry_mpi_new(0);
  gcry_mpi_sub(result, two, one);

  gcry_mpi_aprint(GCRYMPI_FMT_HEX, (unsigned char **)&pc, NULL, result);
  if (debug)
    gcry_log_debug ("Result of two minus one:\n%s\n", pc);
  if (strcmp (pc, "010101010101010101010101010101010101010101010101"
                  "010101010101010101010101010101010101010101010101") != 0)
    fail ("mpi_sub failed at line %d", __LINE__);
  gcry_free(pc);

  zero = gcry_mpi_new(0);
  five = gcry_mpi_new(0);
  minusfive = gcry_mpi_new(0);
  gcry_mpi_set_ui (zero, 0);
  gcry_mpi_set_ui (one, 1);
  gcry_mpi_set_ui (two, 2);
  gcry_mpi_set_ui (five, 5);
  gcry_mpi_sub (minusfive, zero, five);

  gcry_mpi_aprint(GCRYMPI_FMT_HEX, (unsigned char **)&pc, NULL, minusfive);
  if (debug)
    gcry_log_debug ("Result of zero minus five:\n%s\n", pc);
  if (strcmp (pc, "-05") != 0)
    fail ("mpi_sub failed at line %d", __LINE__);
  gcry_free(pc);

  gcry_mpi_sub_ui (result, five, 2);

  gcry_mpi_aprint(GCRYMPI_FMT_HEX, (unsigned char **)&pc, NULL, result);
  if (debug)
    gcry_log_debug ("Result of five minus two:\n%s\n", pc);
  if (strcmp (pc, "03") != 0)
    fail ("mpi_sub_ui failed at line %d", __LINE__);
  gcry_free(pc);

  gcry_mpi_sub_ui (result, one, 10);

  gcry_mpi_aprint(GCRYMPI_FMT_HEX, (unsigned char **)&pc, NULL, result);
  if (debug)
    gcry_log_debug ("Result of one minus ten:\n%s\n", pc);
  if (strcmp (pc, "-09") != 0)
    fail ("mpi_sub_ui failed at line %d", __LINE__);
  gcry_free(pc);

  gcry_mpi_sub_ui (result, minusfive, 2);

  gcry_mpi_aprint(GCRYMPI_FMT_HEX, (unsigned char **)&pc, NULL, result);
  if (debug)
    gcry_log_debug ("Result of minus five minus two:\n%s\n", pc);
  if (strcmp (pc, "-07") != 0)
    fail ("mpi_sub_ui failed at line %d", __LINE__);
  gcry_free(pc);

  gcry_mpi_release(one);
  gcry_mpi_release(two);
  gcry_mpi_release(result);
  gcry_mpi_release(zero);
  gcry_mpi_release(five);
  gcry_mpi_release(minusfive);
  return 1;
}


static int
test_mul (void)
{
  gcry_mpi_t two;
  gcry_mpi_t three;
  gcry_mpi_t result;
  gcry_mpi_t minusfive;
  char *pc;

  gcry_mpi_scan(&two, GCRYMPI_FMT_USG, twos, sizeof(twos), NULL);
  gcry_mpi_scan(&three, GCRYMPI_FMT_USG, threes, sizeof(threes), NULL);
  result = gcry_mpi_new(0);
  gcry_mpi_mul(result, two, three);

  gcry_mpi_aprint(GCRYMPI_FMT_HEX, (unsigned char **)&pc, NULL, result);
  if (debug)
    gcry_log_debug ("Result of two mul three:\n%s\n", pc);
  if (strcmp (pc, "060C12181E242A30363C42484E545A60666C72787E848A90"
                  "969CA2A8AEB4BAC0C6CCD2D8DEE4EAF0F6FD03090F151B21"
                  "1B150F0902FCF6F0EAE4DED8D2CCC6C0BAB4AEA8A29C9690"
                  "8A847E78726C66605A544E48423C36302A241E18120C06") != 0)
    fail ("mpi_mul failed at line %d", __LINE__);
  gcry_free(pc);

  gcry_mpi_scan(&minusfive, GCRYMPI_FMT_HEX, "-5", 0, NULL);
  gcry_mpi_mul_ui (result, minusfive, 3);

  gcry_mpi_aprint(GCRYMPI_FMT_HEX, (unsigned char **)&pc, NULL, result);
  if (debug)
    gcry_log_debug ("Result of minus five mul three:\n%s\n", pc);
  if (strcmp (pc, "-0F") != 0)
    fail ("mpi_mul_ui failed at line %d", __LINE__);
  gcry_free(pc);

  gcry_mpi_mul_ui (result, result, 0);

  gcry_mpi_aprint(GCRYMPI_FMT_HEX, (unsigned char **)&pc, NULL, result);
  if (debug)
    gcry_log_debug ("Result of minus fifteen mul zero:\n%s\n", pc);
  if (strcmp (pc, "00") != 0)
    fail ("mpi_mul_ui failed at line %d", __LINE__);
  gcry_free(pc);

  gcry_mpi_release(two);
  gcry_mpi_release(three);
  gcry_mpi_release(result);
  gcry_mpi_release(minusfive);
  return 1;
}


/* What we test here is that we don't overwrite our args and that
   using the same mpi for several args works.  */
static int
test_powm (void)
{
  int b_int = 17;
  int e_int = 3;
  int m_int = 19;
  gcry_mpi_t base = gcry_mpi_set_ui (NULL, b_int);
  gcry_mpi_t exp = gcry_mpi_set_ui (NULL, e_int);
  gcry_mpi_t mod = gcry_mpi_set_ui (NULL, m_int);
  gcry_mpi_t res = gcry_mpi_new (0);

  gcry_mpi_powm (res, base, exp, mod);
  if (gcry_mpi_cmp_ui (base, b_int))
    die ("test_powm failed for base at %d\n", __LINE__);
  if (gcry_mpi_cmp_ui (exp, e_int))
    die ("test_powm_ui failed for exp at %d\n", __LINE__);
  if (gcry_mpi_cmp_ui (mod, m_int))
    die ("test_powm failed for mod at %d\n", __LINE__);

  /* Check using base for the result.  */
  gcry_mpi_set_ui (base, b_int);
  gcry_mpi_set_ui (exp, e_int);
  gcry_mpi_set_ui(mod, m_int);
  gcry_mpi_powm (base, base, exp, mod);
  if (gcry_mpi_cmp (res, base))
    die ("test_powm failed at %d\n", __LINE__);
  if (gcry_mpi_cmp_ui (exp, e_int))
    die ("test_powm_ui failed for exp at %d\n", __LINE__);
  if (gcry_mpi_cmp_ui (mod, m_int))
    die ("test_powm failed for mod at %d\n", __LINE__);

  /* Check using exp for the result.  */
  gcry_mpi_set_ui (base, b_int);
  gcry_mpi_set_ui (exp, e_int);
  gcry_mpi_set_ui(mod, m_int);
  gcry_mpi_powm (exp, base, exp, mod);
  if (gcry_mpi_cmp (res, exp))
    die ("test_powm failed at %d\n", __LINE__);
  if (gcry_mpi_cmp_ui (base, b_int))
    die ("test_powm failed for base at %d\n", __LINE__);
  if (gcry_mpi_cmp_ui (mod, m_int))
    die ("test_powm failed for mod at %d\n", __LINE__);

  /* Check using mod for the result.  */
  gcry_mpi_set_ui (base, b_int);
  gcry_mpi_set_ui (exp, e_int);
  gcry_mpi_set_ui(mod, m_int);
  gcry_mpi_powm (mod, base, exp, mod);
  if (gcry_mpi_cmp (res, mod))
    die ("test_powm failed at %d\n", __LINE__);
  if (gcry_mpi_cmp_ui (base, b_int))
    die ("test_powm failed for base at %d\n", __LINE__);
  if (gcry_mpi_cmp_ui (exp, e_int))
    die ("test_powm_ui failed for exp at %d\n", __LINE__);

  /* Now check base ^ base mod mod.  */
  gcry_mpi_set_ui (base, b_int);
  gcry_mpi_set_ui(mod, m_int);
  gcry_mpi_powm (res, base, base, mod);
  if (gcry_mpi_cmp_ui (base, b_int))
    die ("test_powm failed for base at %d\n", __LINE__);
  if (gcry_mpi_cmp_ui (mod, m_int))
    die ("test_powm failed for mod at %d\n", __LINE__);

  /* Check base ^ base mod mod with base as result.  */
  gcry_mpi_set_ui (base, b_int);
  gcry_mpi_set_ui(mod, m_int);
  gcry_mpi_powm (base, base, base, mod);
  if (gcry_mpi_cmp (res, base))
    die ("test_powm failed at %d\n", __LINE__);
  if (gcry_mpi_cmp_ui (mod, m_int))
    die ("test_powm failed for mod at %d\n", __LINE__);

  /* Check base ^ base mod mod with mod as result.  */
  gcry_mpi_set_ui (base, b_int);
  gcry_mpi_set_ui(mod, m_int);
  gcry_mpi_powm (mod, base, base, mod);
  if (gcry_mpi_cmp (res, mod))
    die ("test_powm failed at %d\n", __LINE__);
  if (gcry_mpi_cmp_ui (base, b_int))
    die ("test_powm failed for base at %d\n", __LINE__);

  /* Now check base ^ base mod base.  */
  gcry_mpi_set_ui (base, b_int);
  gcry_mpi_powm (res, base, base, base);
  if (gcry_mpi_cmp_ui (base, b_int))
    die ("test_powm failed for base at %d\n", __LINE__);

  /* Check base ^ base mod base with base as result.  */
  gcry_mpi_set_ui (base, b_int);
  gcry_mpi_powm (base, base, base, base);
  if (gcry_mpi_cmp (res, base))
    die ("test_powm failed at %d\n", __LINE__);

  /* Check for a case: base is negative and expo is even.  */
  gcry_mpi_set_ui (base, b_int);
  gcry_mpi_neg (base, base);
  gcry_mpi_set_ui (exp, e_int * 2);
  gcry_mpi_set_ui(mod, m_int);
  gcry_mpi_powm (res, base, exp, mod);
  /* Result should be positive and it's 7 = (-17)^6 mod 19.  */
  if (gcry_mpi_is_neg (res) || gcry_mpi_cmp_ui (res, 7))
    {
      if (verbose)
        {
          fprintf (stderr, "is_neg: %d\n", gcry_mpi_is_neg (res));
          fprintf (stderr, "mpi: ");
          gcry_mpi_dump (res);
          putc ('\n', stderr);
        }
      die ("test_powm failed for negative base at %d\n", __LINE__);
    }

  gcry_mpi_release (base);
  gcry_mpi_release (exp);
  gcry_mpi_release (mod);
  gcry_mpi_release (res);
  /* Fixme: We should add the rest of the cases of course.  */



  return 1;
}


int
main (int argc, char* argv[])
{
  if (argc > 1 && !strcmp (argv[1], "--verbose"))
    verbose = 1;
  else if (argc > 1 && !strcmp (argv[1], "--debug"))
    verbose = debug = 1;

  if (!gcry_check_version (GCRYPT_VERSION))
    {
      fputs ("version mismatch\n", stderr);
      exit (1);
    }
  xgcry_control ((GCRYCTL_DISABLE_SECMEM));

  test_const_and_immutable ();
  test_opaque ();
  test_maxsize ();
  test_cmp ();
  test_add ();
  test_sub ();
  test_mul ();
  test_powm ();

  return !!error_count;
}
