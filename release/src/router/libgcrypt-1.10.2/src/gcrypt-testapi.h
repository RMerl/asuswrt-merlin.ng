/* gcrypt-testapi.h - Definitiona for the Regression test API
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

/*
 * WARNING: This is a private API to be used by regression tests.  In
 * particular this API does not constitute a well defined ABI.  The
 * header may only be used with its matching Libgcrypt version.
 */

#ifndef GCRY_GCRYPT_TESTAPI_H
#define GCRY_GCRYPT_TESTAPI_H

/* For use with gcry_control:  */
#define PRIV_CTL_INIT_EXTRNG_TEST   58
#define PRIV_CTL_RUN_EXTRNG_TEST    59
#define PRIV_CTL_DEINIT_EXTRNG_TEST 60
#define PRIV_CTL_EXTERNAL_LOCK_TEST 61
#define PRIV_CTL_DUMP_SECMEM_STATS  62

#define EXTERNAL_LOCK_TEST_INIT       30111
#define EXTERNAL_LOCK_TEST_LOCK       30112
#define EXTERNAL_LOCK_TEST_UNLOCK     30113
#define EXTERNAL_LOCK_TEST_DESTROY    30114

/* For use with gcry_cipher_ctl:  */
#define PRIV_CIPHERCTL_DISABLE_WEAK_KEY   61
#define PRIV_CIPHERCTL_GET_INPUT_VECTOR   62


/* Private interfaces for testing of random-drbg.c. */
struct gcry_drbg_test_vector
{
  const char *flagstr;
  unsigned char *entropy;
  size_t entropylen;
  unsigned char *entpra;
  unsigned char *entprb;
  size_t entprlen;
  unsigned char *addtla;
  unsigned char *addtlb;
  size_t addtllen;
  unsigned char *pers;
  size_t perslen;
  unsigned char *expected;
  size_t expectedlen;
  unsigned char *entropyreseed;
  size_t entropyreseed_len;
  unsigned char *addtl_reseed;
  size_t addtl_reseed_len;
};


#endif /*GCRY_GCRYPT_TESTAPI_H*/
