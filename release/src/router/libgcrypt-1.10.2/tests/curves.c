/* curves.c -  ECC curves regression tests
 *	Copyright (C) 2011 Free Software Foundation, Inc.
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
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "../src/gcrypt-int.h"


#define PGM "curves"
#include "t-common.h"

/* Number of curves defined in ../cipher/ecc-curves.c */
#define N_CURVES 27

/* A real world sample public key.  */
static char const sample_key_1[] =
"(public-key\n"
" (ecdsa\n"
"  (p #00FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFF#)\n"
"  (a #00FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFC#)\n"
"  (b #5AC635D8AA3A93E7B3EBBD55769886BC651D06B0CC53B0F63BCE3C3E27D2604B#)\n"
"  (g #046B17D1F2E12C4247F8BCE6E563A440F277037D812DEB33A0F4A13945D898C296"
        "4FE342E2FE1A7F9B8EE7EB4A7C0F9E162BCE33576B315ECECBB6406837BF51F5#)\n"
"  (n #00FFFFFFFF00000000FFFFFFFFFFFFFFFFBCE6FAADA7179E84F3B9CAC2FC632551#)\n"
"  (h #000000000000000000000000000000000000000000000000000000000000000001#)\n"
"  (q #0442B927242237639A36CE9221B340DB1A9AB76DF2FE3E171277F6A4023DED146EE"
      "86525E38CCECFF3FB8D152CC6334F70D23A525175C1BCBDDE6E023B2228770E#)\n"
"  ))";
static char const sample_key_1_curve[] = "NIST P-256";
static unsigned int sample_key_1_nbits = 256;

/* A made up sample public key.  */
static char const sample_key_2[] =
"(public-key\n"
" (ecdh\n"
"  (p #00e95e4a5f737059dc60dfc7ad95b3d8139515620f#)\n"
"  (a #340e7be2a280eb74e2be61bada745d97e8f7c300#)\n"
"  (b #1e589a8595423412134faa2dbdec95c8d8675e58#)\n"
"  (g #04bed5af16ea3f6a4f62938c4631eb5af7bdbcdbc3"
        "1667cb477a1a8ec338f94741669c976316da6321#)\n"
"  (n #00e95e4a5f737059dc60df5991d45029409e60fc09#)\n"
"  (h #000000000000000000000000000000000000000000000000000000000000000001#)\n"
"  (q #041111111111111111111111111111111111111111"
        "2222222222222222222222222222222222222222#)\n"
"  ))";
static char const sample_key_2_curve[] = "brainpoolP160r1";
static unsigned int sample_key_2_nbits = 160;

static int in_fips_mode;

static void
list_curves (void)
{
  int idx;
  const char *name;
  unsigned int nbits;

  for (idx=0; (name = gcry_pk_get_curve (NULL, idx, &nbits)); idx++)
    {
      if (verbose)
        printf ("%s - %u bits\n", name, nbits);
    }
  if (idx != N_CURVES)
    fail ("expected %d curves but got %d\n", N_CURVES, idx);
  if (gcry_pk_get_curve (NULL, -1, NULL))
    fail ("curve iteration failed\n");
}


static void
check_matching (void)
{
  gpg_error_t err;
  gcry_sexp_t key;
  const char *name;
  unsigned int nbits;

  err = gcry_sexp_new (&key, sample_key_1, 0, 1);
  if (err)
    die ("parsing s-expression string failed: %s\n", gpg_strerror (err));
  name = gcry_pk_get_curve (key, 0, &nbits);
  if (!name)
    fail ("curve name not found for sample_key_1\n");
  else if (strcmp (name, sample_key_1_curve))
    fail ("expected curve name %s but got %s for sample_key_1\n",
          sample_key_1_curve, name);
  else if (nbits != sample_key_1_nbits)
    fail ("expected curve size %u but got %u for sample_key_1\n",
          sample_key_1_nbits, nbits);

  gcry_sexp_release (key);

  if (!in_fips_mode)
    {
      err = gcry_sexp_new (&key, sample_key_2, 0, 1);
      if (err)
        die ("parsing s-expression string failed: %s\n", gpg_strerror (err));
      name = gcry_pk_get_curve (key, 0, &nbits);
      if (!name)
        fail ("curve name not found for sample_key_2\n");
      else if (strcmp (name, sample_key_2_curve))
        fail ("expected curve name %s but got %s for sample_key_2\n",
              sample_key_2_curve, name);
      else if (nbits != sample_key_2_nbits)
        fail ("expected curve size %u but got %u for sample_key_2\n",
              sample_key_2_nbits, nbits);

      gcry_sexp_release (key);
    }
}

#define TEST_ERROR_EXPECTED (1 << 0)
#define TEST_NOFIPS         (1 << 1)

static void
check_get_params (void)
{
  static struct {
    int algo;
    const char *name;
    int flags;
  } tv[] =
      {
       { GCRY_PK_ECC, "Ed25519", TEST_NOFIPS },
       { GCRY_PK_ECC, "1.3.6.1.4.1.11591.15.1", TEST_NOFIPS },
       { GCRY_PK_ECC, "1.3.101.112", TEST_NOFIPS },

       { GCRY_PK_ECC, "Curve25519", TEST_NOFIPS },
       { GCRY_PK_ECC, "1.3.6.1.4.1.3029.1.5.1", TEST_NOFIPS },
       { GCRY_PK_ECC, "1.3.101.110", TEST_NOFIPS },
       { GCRY_PK_ECC, "X25519", TEST_NOFIPS },

       { GCRY_PK_ECC, "Ed448", TEST_NOFIPS },
       { GCRY_PK_ECC, "X448", TEST_NOFIPS  },
       { GCRY_PK_ECC, "1.3.101.113", TEST_NOFIPS },
       { GCRY_PK_ECC, "1.3.101.111", TEST_NOFIPS },

       { GCRY_PK_ECC, "NIST P-192", TEST_NOFIPS },
       { GCRY_PK_ECC, "1.2.840.10045.3.1.1", TEST_NOFIPS },
       { GCRY_PK_ECC, "prime192v1", TEST_NOFIPS },
       { GCRY_PK_ECC, "secp192r1", TEST_NOFIPS },
       { GCRY_PK_ECC, "nistp192", TEST_NOFIPS },

       { GCRY_PK_ECC, "NIST P-224" },
       { GCRY_PK_ECC, "secp224r1"  },
       { GCRY_PK_ECC, "1.3.132.0.33" },
       { GCRY_PK_ECC, "nistp224"   },

       { GCRY_PK_ECC, "NIST P-256" },
       { GCRY_PK_ECC, "1.2.840.10045.3.1.7" },
       { GCRY_PK_ECC, "prime256v1" },
       { GCRY_PK_ECC, "secp256r1" },
       { GCRY_PK_ECC, "nistp256"  },

       { GCRY_PK_ECC, "NIST P-384" },
       { GCRY_PK_ECC, "secp384r1" },
       { GCRY_PK_ECC, "1.3.132.0.34" },
       { GCRY_PK_ECC, "nistp384"   },

       { GCRY_PK_ECC, "NIST P-521" },
       { GCRY_PK_ECC, "secp521r1" },
       { GCRY_PK_ECC, "1.3.132.0.35" },
       { GCRY_PK_ECC, "nistp521"   },

       { GCRY_PK_ECC, "brainpoolP160r1",       TEST_NOFIPS },
       { GCRY_PK_ECC, "1.3.36.3.3.2.8.1.1.1",  TEST_NOFIPS },
       { GCRY_PK_ECC, "brainpoolP192r1",       TEST_NOFIPS },
       { GCRY_PK_ECC, "1.3.36.3.3.2.8.1.1.3",  TEST_NOFIPS },
       { GCRY_PK_ECC, "brainpoolP224r1",       TEST_NOFIPS },
       { GCRY_PK_ECC, "1.3.36.3.3.2.8.1.1.5",  TEST_NOFIPS },
       { GCRY_PK_ECC, "brainpoolP256r1",       TEST_NOFIPS },
       { GCRY_PK_ECC, "1.3.36.3.3.2.8.1.1.7",  TEST_NOFIPS },
       { GCRY_PK_ECC, "brainpoolP320r1",       TEST_NOFIPS },
       { GCRY_PK_ECC, "1.3.36.3.3.2.8.1.1.9",  TEST_NOFIPS },
       { GCRY_PK_ECC, "brainpoolP384r1",       TEST_NOFIPS },
       { GCRY_PK_ECC, "1.3.36.3.3.2.8.1.1.11", TEST_NOFIPS },
       { GCRY_PK_ECC, "brainpoolP512r1",       TEST_NOFIPS },
       { GCRY_PK_ECC, "1.3.36.3.3.2.8.1.1.13", TEST_NOFIPS },

       { GCRY_PK_ECC, "GOST2001-test", TEST_NOFIPS },
       { GCRY_PK_ECC, "1.2.643.2.2.35.0", TEST_NOFIPS },
       { GCRY_PK_ECC, "GOST2001-CryptoPro-A", TEST_NOFIPS },
       { GCRY_PK_ECC, "1.2.643.2.2.35.1", TEST_NOFIPS },
       { GCRY_PK_ECC, "GOST2001-CryptoPro-B", TEST_NOFIPS },
       { GCRY_PK_ECC, "1.2.643.2.2.35.2", TEST_NOFIPS },
       { GCRY_PK_ECC, "GOST2001-CryptoPro-C", TEST_NOFIPS },
       { GCRY_PK_ECC, "1.2.643.2.2.35.3", TEST_NOFIPS },
       { GCRY_PK_ECC, "GOST2001-CryptoPro-A", TEST_NOFIPS },
       { GCRY_PK_ECC, "GOST2001-CryptoPro-XchA", TEST_NOFIPS },
       { GCRY_PK_ECC, "GOST2001-CryptoPro-C", TEST_NOFIPS },
       { GCRY_PK_ECC, "GOST2001-CryptoPro-XchB", TEST_NOFIPS },
       { GCRY_PK_ECC, "GOST2001-CryptoPro-A", TEST_NOFIPS },
       { GCRY_PK_ECC, "1.2.643.2.2.36.0", TEST_NOFIPS },
       { GCRY_PK_ECC, "GOST2001-CryptoPro-C", TEST_NOFIPS },
       { GCRY_PK_ECC, "1.2.643.2.2.36.1", TEST_NOFIPS },

       /* Noet that GOST2012-256-tc26-A" is only in the curve alias
        * list but has no parameter entry.  */
       { GCRY_PK_ECC, "GOST2001-CryptoPro-A", TEST_NOFIPS },
       { GCRY_PK_ECC, "1.2.643.7.1.2.1.1.2", TEST_NOFIPS },
       { GCRY_PK_ECC, "GOST2001-CryptoPro-A", TEST_NOFIPS },
       { GCRY_PK_ECC, "GOST2012-256-tc26-B", TEST_NOFIPS },
       { GCRY_PK_ECC, "GOST2001-CryptoPro-B", TEST_NOFIPS },
       { GCRY_PK_ECC, "1.2.643.7.1.2.1.1.3", TEST_NOFIPS },
       { GCRY_PK_ECC, "GOST2001-CryptoPro-B", TEST_NOFIPS },
       { GCRY_PK_ECC, "GOST2012-256-tc26-C", TEST_NOFIPS },
       { GCRY_PK_ECC, "GOST2001-CryptoPro-C", TEST_NOFIPS },
       { GCRY_PK_ECC, "1.2.643.7.1.2.1.1.4", TEST_NOFIPS },
       { GCRY_PK_ECC, "GOST2001-CryptoPro-C", TEST_NOFIPS },
       { GCRY_PK_ECC, "GOST2012-256-tc26-D", TEST_NOFIPS },

       { GCRY_PK_ECC, "GOST2012-512-test", TEST_NOFIPS },
       { GCRY_PK_ECC, "GOST2012-test", TEST_NOFIPS },
       { GCRY_PK_ECC, "GOST2012-512-test", TEST_NOFIPS },
       { GCRY_PK_ECC, "1.2.643.7.1.2.1.2.0", TEST_NOFIPS },
       { GCRY_PK_ECC, "GOST2012-512-tc26-A", TEST_NOFIPS },
       { GCRY_PK_ECC, "GOST2012-tc26-A", TEST_NOFIPS },
       { GCRY_PK_ECC, "GOST2012-512-tc26-B", TEST_NOFIPS },
       { GCRY_PK_ECC, "GOST2012-tc26-B", TEST_NOFIPS },
       { GCRY_PK_ECC, "GOST2012-512-tc26-A", TEST_NOFIPS },
       { GCRY_PK_ECC, "1.2.643.7.1.2.1.2.1", TEST_NOFIPS },
       { GCRY_PK_ECC, "GOST2012-512-tc26-B", TEST_NOFIPS },
       { GCRY_PK_ECC, "1.2.643.7.1.2.1.2.2", TEST_NOFIPS },
       { GCRY_PK_ECC, "GOST2012-512-tc26-C", TEST_NOFIPS },
       { GCRY_PK_ECC, "1.2.643.7.1.2.1.2.3", TEST_NOFIPS },

       { GCRY_PK_ECC, "secp256k1", TEST_NOFIPS },
       { GCRY_PK_ECC, "1.3.132.0.10", TEST_NOFIPS },

       { GCRY_PK_ECC, "sm2p256v1", TEST_NOFIPS },
       { GCRY_PK_ECC, "1.2.156.10197.1.301", TEST_NOFIPS },

       /* Check also the ECC algo mapping.  */
       { GCRY_PK_ECDSA, "Ed25519", TEST_NOFIPS },
       { GCRY_PK_EDDSA, "Ed25519", TEST_NOFIPS },
       { GCRY_PK_ECDH,  "Ed25519", TEST_NOFIPS },
       { GCRY_PK_ECDSA, "Curve25519", TEST_NOFIPS },
       { GCRY_PK_EDDSA, "Curve25519", TEST_NOFIPS },
       { GCRY_PK_ECDH,  "Curve25519", TEST_NOFIPS },
       { GCRY_PK_ECC,   "NoSuchCurve", TEST_ERROR_EXPECTED },
       { GCRY_PK_RSA,   "rsa", TEST_ERROR_EXPECTED },
       { GCRY_PK_ELG,   "elg", TEST_ERROR_EXPECTED },
       { GCRY_PK_DSA,   "dsa", TEST_ERROR_EXPECTED }
      };
  int idx;
  gcry_sexp_t param;
  const char *name;

  param = gcry_pk_get_param (GCRY_PK_ECDSA, sample_key_1_curve);
  if (!param)
    fail ("error gerring parameters for `%s'\n", sample_key_1_curve);

  name = gcry_pk_get_curve (param, 0, NULL);
  if (!name)
    fail ("get_param: curve name not found for sample_key_1\n");
  else if (strcmp (name, sample_key_1_curve))
    fail ("get_param: expected curve name %s but got %s for sample_key_1\n",
          sample_key_1_curve, name);

  gcry_sexp_release (param);

  if (!in_fips_mode)
    {
      param = gcry_pk_get_param (GCRY_PK_ECDSA, sample_key_2_curve);
      if (!param)
        fail ("error gerring parameters for `%s'\n", sample_key_2_curve);

      name = gcry_pk_get_curve (param, 0, NULL);
      if (!name)
        fail ("get_param: curve name not found for sample_key_2\n");
      else if (strcmp (name, sample_key_2_curve))
        fail ("get_param: expected curve name %s but got %s for sample_key_2\n",
              sample_key_2_curve, name);

      gcry_sexp_release (param);
    }

  /* Some simple tests */
  for (idx=0; idx < DIM (tv); idx++)
    {
      param = gcry_pk_get_param (tv[idx].algo, tv[idx].name);
      if (in_fips_mode && tv[idx].flags & TEST_NOFIPS)
        {
          if (param)
            fail ("get_param: test %d (%s) should have failed in fips mode\n",
                  idx, tv[idx].name);
        }
      else {
        if (!param)
          {
            if (!(tv[idx].flags & TEST_ERROR_EXPECTED))
              fail ("get_param: test %d (%s) failed\n", idx, tv[idx].name);
          }
        else
          {
            if (tv[idx].flags & TEST_ERROR_EXPECTED)
              fail ("get_param: test %d (%s) failed (error expected)\n",
                    idx, tv[idx].name);
          }
        }
      gcry_sexp_release (param);
    }
}


int
main (int argc, char **argv)
{
  if (argc > 1 && !strcmp (argv[1], "--verbose"))
    verbose = 1;
  else if (argc > 1 && !strcmp (argv[1], "--debug"))
    verbose = debug = 1;

  if (!gcry_check_version (GCRYPT_VERSION))
    die ("version mismatch\n");

  xgcry_control ((GCRYCTL_DISABLE_SECMEM, 0));
  xgcry_control ((GCRYCTL_INITIALIZATION_FINISHED, 0));
  if (debug)
    xgcry_control ((GCRYCTL_SET_DEBUG_FLAGS, 1u, 0));

  if (gcry_fips_mode_active ())
    in_fips_mode = 1;

  list_curves ();
  check_matching ();
  check_get_params ();

  return error_count ? 1 : 0;
}
