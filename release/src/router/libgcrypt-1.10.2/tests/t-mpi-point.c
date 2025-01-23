/* t-mpi-point.c  - Tests for mpi point functions
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
# include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#define PGM "t-mpi-point"
#include "t-common.h"

static const struct
{
  const char *desc;           /* Description of the curve.  */
  const char *p;              /* Order of the prime field.  */
  const char *a, *b;          /* The coefficients. */
  const char *n;              /* The order of the base point.  */
  const char *g_x, *g_y;      /* Base point.  */
  const char *h;              /* Cofactor.  */
} test_curve[] =
  {
    {
      "NIST P-192",
      "0xfffffffffffffffffffffffffffffffeffffffffffffffff",
      "0xfffffffffffffffffffffffffffffffefffffffffffffffc",
      "0x64210519e59c80e70fa7e9ab72243049feb8deecc146b9b1",
      "0xffffffffffffffffffffffff99def836146bc9b1b4d22831",

      "0x188da80eb03090f67cbf20eb43a18800f4ff0afd82ff1012",
      "0x07192b95ffc8da78631011ed6b24cdd573f977a11e794811",
      "0x01"
    },
    {
      "NIST P-224",
      "0xffffffffffffffffffffffffffffffff000000000000000000000001",
      "0xfffffffffffffffffffffffffffffffefffffffffffffffffffffffe",
      "0xb4050a850c04b3abf54132565044b0b7d7bfd8ba270b39432355ffb4",
      "0xffffffffffffffffffffffffffff16a2e0b8f03e13dd29455c5c2a3d" ,

      "0xb70e0cbd6bb4bf7f321390b94a03c1d356c21122343280d6115c1d21",
      "0xbd376388b5f723fb4c22dfe6cd4375a05a07476444d5819985007e34",
      "0x01"
    },
    {
      "NIST P-256",
      "0xffffffff00000001000000000000000000000000ffffffffffffffffffffffff",
      "0xffffffff00000001000000000000000000000000fffffffffffffffffffffffc",
      "0x5ac635d8aa3a93e7b3ebbd55769886bc651d06b0cc53b0f63bce3c3e27d2604b",
      "0xffffffff00000000ffffffffffffffffbce6faada7179e84f3b9cac2fc632551",

      "0x6b17d1f2e12c4247f8bce6e563a440f277037d812deb33a0f4a13945d898c296",
      "0x4fe342e2fe1a7f9b8ee7eb4a7c0f9e162bce33576b315ececbb6406837bf51f5",
      "0x01"
    },
    {
      "NIST P-384",
      "0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe"
      "ffffffff0000000000000000ffffffff",
      "0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe"
      "ffffffff0000000000000000fffffffc",
      "0xb3312fa7e23ee7e4988e056be3f82d19181d9c6efe8141120314088f5013875a"
      "c656398d8a2ed19d2a85c8edd3ec2aef",
      "0xffffffffffffffffffffffffffffffffffffffffffffffffc7634d81f4372ddf"
      "581a0db248b0a77aecec196accc52973",

      "0xaa87ca22be8b05378eb1c71ef320ad746e1d3b628ba79b9859f741e082542a38"
      "5502f25dbf55296c3a545e3872760ab7",
      "0x3617de4a96262c6f5d9e98bf9292dc29f8f41dbd289a147ce9da3113b5f0b8c0"
      "0a60b1ce1d7e819d7a431d7c90ea0e5f",
      "0x01"
    },
    {
      "NIST P-521",
      "0x01ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
      "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
      "0x01ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
      "fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffc",
      "0x051953eb9618e1c9a1f929a21a0b68540eea2da725b99b315f3b8b489918ef10"
      "9e156193951ec7e937b1652c0bd3bb1bf073573df883d2c34f1ef451fd46b503f00",
      "0x1fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
      "ffa51868783bf2f966b7fcc0148f709a5d03bb5c9b8899c47aebb6fb71e91386409",

      "0xc6858e06b70404e9cd9e3ecb662395b4429c648139053fb521f828af606b4d3d"
      "baa14b5e77efe75928fe1dc127a2ffa8de3348b3c1856a429bf97e7e31c2e5bd66",
      "0x11839296a789a3bc0045c8a5fb42c7d1bd998f54449579b446817afbd17273e6"
      "62c97ee72995ef42640c550b9013fad0761353c7086a272c24088be94769fd16650",
      "0x01"
    },
    {
      "Ed25519",
      "0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFED",
      "0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEC",
      "0x52036CEE2B6FFE738CC740797779E89800700A4D4141D8AB75EB4DCA135978A3",
      "0x1000000000000000000000000000000014DEF9DEA2F79CD65812631A5CF5D3ED",
      "0x216936D3CD6E53FEC0A4E231FDD6DC5C692CC7609525A7B2C9562D608F25D51A",
      "0x6666666666666666666666666666666666666666666666666666666666666658",
      "0x08"
    },
    { NULL, NULL, NULL, NULL, NULL, NULL }
  };

/* A sample public key for NIST P-256.  */
static const char sample_p256_q[] =
  "04"
  "42B927242237639A36CE9221B340DB1A9AB76DF2FE3E171277F6A4023DED146E"
  "E86525E38CCECFF3FB8D152CC6334F70D23A525175C1BCBDDE6E023B2228770E";
static const char sample_p256_q_x[] =
  "42B927242237639A36CE9221B340DB1A9AB76DF2FE3E171277F6A4023DED146E";
static const char sample_p256_q_y[] =
  "00E86525E38CCECFF3FB8D152CC6334F70D23A525175C1BCBDDE6E023B2228770E";


/* A sample public key for Ed25519.  */
static const char sample_ed25519_q[] =
  "04"
  "55d0e09a2b9d34292297e08d60d0f620c513d47253187c24b12786bd777645ce"
  "1a5107f7681a02af2523a6daf372e10e3a0764c9d3fe4bd5b70ab18201985ad7";
static const char sample_ed25519_q_x[] =
  "55d0e09a2b9d34292297e08d60d0f620c513d47253187c24b12786bd777645ce";
static const char sample_ed25519_q_y[] =
  "1a5107f7681a02af2523a6daf372e10e3a0764c9d3fe4bd5b70ab18201985ad7";
static const char sample_ed25519_q_eddsa[] =
  "d75a980182b10ab7d54bfed3c964073a0ee172f3daa62325af021a68f707511a";
static const char sample_ed25519_d[] =
  "9d61b19deffd5a60ba844af492ec2cc44449c5697b326919703bac031cae7f60";


static void
print_mpi_2 (const char *text, const char *text2, gcry_mpi_t a)
{
  gcry_error_t err;
  char *buf;
  void *bufaddr = &buf;

  err = gcry_mpi_aprint (GCRYMPI_FMT_HEX, bufaddr, NULL, a);
  if (err)
    fprintf (stderr, "%s%s: [error printing number: %s]\n",
             text, text2? text2:"", gpg_strerror (err));
  else
    {
      fprintf (stderr, "%s%s: %s\n", text, text2? text2:"", buf);
      gcry_free (buf);
    }
}


static void
print_mpi (const char *text, gcry_mpi_t a)
{
  print_mpi_2 (text, NULL, a);
}


static void
print_point (const char *text, gcry_mpi_point_t a)
{
  gcry_mpi_t x, y, z;

  x = gcry_mpi_new (0);
  y = gcry_mpi_new (0);
  z = gcry_mpi_new (0);
  gcry_mpi_point_get (x, y, z, a);
  print_mpi_2 (text, ".x", x);
  print_mpi_2 (text, ".y", y);
  print_mpi_2 (text, ".z", z);
  gcry_mpi_release (x);
  gcry_mpi_release (y);
  gcry_mpi_release (z);
}


static void
print_sexp (const char *prefix, gcry_sexp_t a)
{
  char *buf;
  size_t size;

  if (prefix)
    fputs (prefix, stderr);
  size = gcry_sexp_sprint (a, GCRYSEXP_FMT_ADVANCED, NULL, 0);
  buf = gcry_xmalloc (size);

  gcry_sexp_sprint (a, GCRYSEXP_FMT_ADVANCED, buf, size);
  fprintf (stderr, "%.*s", (int)size, buf);
  gcry_free (buf);
}


static gcry_mpi_t
hex2mpi (const char *string)
{
  gpg_error_t err;
  gcry_mpi_t val;

  err = gcry_mpi_scan (&val, GCRYMPI_FMT_HEX, string, 0, NULL);
  if (err)
    die ("hex2mpi '%s' failed: %s\n", string, gpg_strerror (err));
  return val;
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


static gcry_mpi_t
hex2mpiopa (const char *string)
{
  char *buffer;
  size_t buflen;
  gcry_mpi_t val;

  buffer = hex2buffer (string, &buflen);
  if (!buffer)
    die ("hex2mpiopa '%s' failed: parser error\n", string);
  val = gcry_mpi_set_opaque (NULL, buffer, buflen*8);
  if (!buffer)
    die ("hex2mpiopa '%s' failed: set_opaque error\n", string);
  return val;
}


/* Compare A to B, where B is given as a hex string.  */
static int
cmp_mpihex (gcry_mpi_t a, const char *b)
{
  gcry_mpi_t bval;
  int res;

  if (gcry_mpi_get_flag (a, GCRYMPI_FLAG_OPAQUE))
    bval = hex2mpiopa (b);
  else
    bval = hex2mpi (b);
  res = gcry_mpi_cmp (a, bval);
  gcry_mpi_release (bval);
  return res;
}


/* Wrapper to emulate the libgcrypt internal EC context allocation
   function.  */
static gpg_error_t
ec_p_new (gcry_ctx_t *r_ctx, gcry_mpi_t p, gcry_mpi_t a)
{
  gpg_error_t err;
  gcry_sexp_t sexp;

  if (p && a)
    err = gcry_sexp_build (&sexp, NULL, "(ecdsa (p %m)(a %m))", p, a);
  else if (p)
    err = gcry_sexp_build (&sexp, NULL, "(ecdsa (p %m))", p);
  else if (a)
    err = gcry_sexp_build (&sexp, NULL, "(ecdsa (a %m))", a);
  else
    err = gcry_sexp_build (&sexp, NULL, "(ecdsa)");
  if (err)
    return err;
  err = gcry_mpi_ec_new (r_ctx, sexp, NULL);
  gcry_sexp_release (sexp);
  return err;
}



static void
set_get_point (void)
{
  gcry_mpi_point_t point, point2;
  gcry_mpi_t x, y, z;

  wherestr = "set_get_point";
  info ("checking point setting functions\n");

  point = gcry_mpi_point_new (0);
  x = gcry_mpi_set_ui (NULL, 17);
  y = gcry_mpi_set_ui (NULL, 42);
  z = gcry_mpi_set_ui (NULL, 11371);
  gcry_mpi_point_get (x, y, z, point);
  if (gcry_mpi_cmp_ui (x, 0)
      || gcry_mpi_cmp_ui (y, 0) || gcry_mpi_cmp_ui (z, 0))
    fail ("new point not initialized to (0,0,0)\n");
  gcry_mpi_point_snatch_get (x, y, z, point);
  point = NULL;
  if (gcry_mpi_cmp_ui (x, 0)
      || gcry_mpi_cmp_ui (y, 0) || gcry_mpi_cmp_ui (z, 0))
    fail ("snatch_get failed\n");
  gcry_mpi_release (x);
  gcry_mpi_release (y);
  gcry_mpi_release (z);

  point = gcry_mpi_point_new (0);
  x = gcry_mpi_set_ui (NULL, 17);
  y = gcry_mpi_set_ui (NULL, 42);
  z = gcry_mpi_set_ui (NULL, 11371);
  gcry_mpi_point_set (point, x, y, z);
  gcry_mpi_set_ui (x, 23);
  gcry_mpi_set_ui (y, 24);
  gcry_mpi_set_ui (z, 25);
  gcry_mpi_point_get (x, y, z, point);
  if (gcry_mpi_cmp_ui (x, 17)
      || gcry_mpi_cmp_ui (y, 42) || gcry_mpi_cmp_ui (z, 11371))
    fail ("point_set/point_get failed\n");
  gcry_mpi_point_snatch_set (point, x, y, z);
  x = gcry_mpi_new (0);
  y = gcry_mpi_new (0);
  z = gcry_mpi_new (0);
  gcry_mpi_point_get (x, y, z, point);
  if (gcry_mpi_cmp_ui (x, 17)
      || gcry_mpi_cmp_ui (y, 42) || gcry_mpi_cmp_ui (z, 11371))
    fail ("point_snatch_set/point_get failed\n");

  point2 = gcry_mpi_point_copy (point);

  gcry_mpi_point_get (x, y, z, point2);
  if (gcry_mpi_cmp_ui (x, 17)
      || gcry_mpi_cmp_ui (y, 42) || gcry_mpi_cmp_ui (z, 11371))
    fail ("point_copy failed (1)\n");

  gcry_mpi_point_release (point);

  gcry_mpi_point_get (x, y, z, point2);
  if (gcry_mpi_cmp_ui (x, 17)
      || gcry_mpi_cmp_ui (y, 42) || gcry_mpi_cmp_ui (z, 11371))
    fail ("point_copy failed (2)\n");

  gcry_mpi_point_release (point2);

  gcry_mpi_release (x);
  gcry_mpi_release (y);
  gcry_mpi_release (z);
}


static void
context_alloc (void)
{
  gpg_error_t err;
  gcry_ctx_t ctx;
  gcry_mpi_t p, a;

  wherestr = "context_alloc";
  info ("checking context functions\n");

  p = gcry_mpi_set_ui (NULL, 1);
  a = gcry_mpi_set_ui (NULL, 1);
  err = ec_p_new (&ctx, p, a);
  if (err)
    die ("ec_p_new returned an error: %s\n", gpg_strerror (err));
  gcry_mpi_release (p);
  gcry_mpi_release (a);
  gcry_ctx_release (ctx);

  p = NULL;
  a = gcry_mpi_set_ui (NULL, 0);

  err = ec_p_new (&ctx, p, a);
  if (!err || gpg_err_code (err) != GPG_ERR_EINVAL)
    fail ("ec_p_new: bad parameter detection failed (1)\n");

  gcry_mpi_release (a);
  a = NULL;
  err = ec_p_new (&ctx, p, a);
  if (!err || gpg_err_code (err) != GPG_ERR_EINVAL)
    fail ("ec_p_new: bad parameter detection failed (2)\n");

}


static int
get_and_cmp_mpi (const char *name, const char *mpistring, const char *desc,
                 gcry_ctx_t ctx)
{
  gcry_mpi_t mpi;

  mpi = gcry_mpi_ec_get_mpi (name, ctx, 1);
  if (!mpi)
    {
      fail ("error getting parameter '%s' of curve '%s'\n", name, desc);
      return 1;
    }
  if (debug)
    print_mpi (name, mpi);
  if (cmp_mpihex (mpi, mpistring))
    {
      fail ("parameter '%s' of curve '%s' does not match\n", name, desc);
      gcry_mpi_release (mpi);
      return 1;
    }
  gcry_mpi_release (mpi);
  return 0;
}


static int
get_and_cmp_point (const char *name,
                   const char *mpi_x_string, const char *mpi_y_string,
                   const char *desc, gcry_ctx_t ctx)
{
  gcry_mpi_point_t point;
  gcry_mpi_t x, y, z;
  int result = 0;

  point = gcry_mpi_ec_get_point (name, ctx, 1);
  if (!point)
    {
      fail ("error getting point parameter '%s' of curve '%s'\n", name, desc);
      return 1;
    }
  if (debug)
    print_point (name, point);

  x = gcry_mpi_new (0);
  y = gcry_mpi_new (0);
  z = gcry_mpi_new (0);
  gcry_mpi_point_snatch_get (x, y, z, point);
  if (cmp_mpihex (x, mpi_x_string))
    {
      fail ("x coordinate of '%s' of curve '%s' does not match\n", name, desc);
      result = 1;
    }
  if (cmp_mpihex (y, mpi_y_string))
    {
      fail ("y coordinate of '%s' of curve '%s' does not match\n", name, desc);
      result = 1;
    }
  if (cmp_mpihex (z, "01"))
    {
      fail ("z coordinate of '%s' of curve '%s' is not 1\n", name, desc);
      result = 1;
    }
  gcry_mpi_release (x);
  gcry_mpi_release (y);
  gcry_mpi_release (z);
  return result;
}


static void
context_param (void)
{
  gpg_error_t err;
  int idx;
  gcry_ctx_t ctx = NULL;
  gcry_mpi_t q, d;
  gcry_sexp_t keyparam;

  wherestr = "context_param";

  info ("checking standard curves\n");
  for (idx=0; test_curve[idx].desc; idx++)
    {
      /* P-192 and Ed25519 are not supported in fips mode */
      if (gcry_fips_mode_active())
        {
          if (!strcmp(test_curve[idx].desc, "NIST P-192")
              || !strcmp(test_curve[idx].desc, "Ed25519"))
            {
	      info ("skipping %s in fips mode\n", test_curve[idx].desc );
              continue;
            }
        }

      gcry_ctx_release (ctx);
      err = gcry_mpi_ec_new (&ctx, NULL, test_curve[idx].desc);
      if (err)
        {
          fail ("can't create context for curve '%s': %s\n",
                test_curve[idx].desc, gpg_strerror (err));
          continue;
        }
      if (get_and_cmp_mpi ("p", test_curve[idx].p, test_curve[idx].desc, ctx))
        continue;
      if (get_and_cmp_mpi ("a", test_curve[idx].a, test_curve[idx].desc, ctx))
        continue;
      if (get_and_cmp_mpi ("b", test_curve[idx].b, test_curve[idx].desc, ctx))
        continue;
      if (get_and_cmp_mpi ("g.x",test_curve[idx].g_x, test_curve[idx].desc,ctx))
        continue;
      if (get_and_cmp_mpi ("g.y",test_curve[idx].g_y, test_curve[idx].desc,ctx))
        continue;
      if (get_and_cmp_mpi ("n", test_curve[idx].n, test_curve[idx].desc, ctx))
        continue;
      if (get_and_cmp_point ("g", test_curve[idx].g_x, test_curve[idx].g_y,
                             test_curve[idx].desc, ctx))
        continue;
      if (get_and_cmp_mpi ("h", test_curve[idx].h, test_curve[idx].desc, ctx))
        continue;

    }

  info ("checking sample public key (nistp256)\n");
  q = hex2mpi (sample_p256_q);
  err = gcry_sexp_build (&keyparam, NULL,
                        "(public-key(ecc(curve %s)(q %m)))",
                        "NIST P-256", q);
  if (err)
    die ("gcry_sexp_build failed: %s\n", gpg_strerror (err));
  gcry_mpi_release (q);

  /* We can't call gcry_pk_testkey because it is only implemented for
     private keys.  */
  /* err = gcry_pk_testkey (keyparam); */
  /* if (err) */
  /*   fail ("gcry_pk_testkey failed for sample public key: %s\n", */
  /*         gpg_strerror (err)); */

  gcry_ctx_release (ctx);
  err = gcry_mpi_ec_new (&ctx, keyparam, NULL);
  if (err)
    fail ("gcry_mpi_ec_new failed for sample public key (nistp256): %s\n",
          gpg_strerror (err));
  else
    {
      gcry_sexp_t sexp;

      get_and_cmp_mpi ("q", sample_p256_q, "nistp256", ctx);
      get_and_cmp_point ("q", sample_p256_q_x, sample_p256_q_y, "nistp256",
                         ctx);

      /* Delete Q.  */
      err = gcry_mpi_ec_set_mpi ("q", NULL, ctx);
      if (err)
        fail ("clearing Q for nistp256 failed: %s\n", gpg_strerror (err));
      if (gcry_mpi_ec_get_mpi ("q", ctx, 0))
        fail ("clearing Q for nistp256 did not work\n");

      /* Set Q again.  */
      q = hex2mpi (sample_p256_q);
      err = gcry_mpi_ec_set_mpi ("q", q, ctx);
      if (err)
        fail ("setting Q for nistp256 failed: %s\n", gpg_strerror (err));
      get_and_cmp_mpi ("q", sample_p256_q, "nistp256(2)", ctx);
      gcry_mpi_release (q);

      /* Get as s-expression.  */
      err = gcry_pubkey_get_sexp (&sexp, 0, ctx);
      if (err)
        fail ("gcry_pubkey_get_sexp(0) failed: %s\n", gpg_strerror (err));
      else if (debug)
        print_sexp ("Result of gcry_pubkey_get_sexp (0):\n", sexp);
      gcry_sexp_release (sexp);

      err = gcry_pubkey_get_sexp (&sexp, GCRY_PK_GET_PUBKEY, ctx);
      if (err)
        fail ("gcry_pubkey_get_sexp(GET_PUBKEY) failed: %s\n",
              gpg_strerror (err));
      else if (debug)
        print_sexp ("Result of gcry_pubkey_get_sexp (GET_PUBKEY):\n", sexp);
      gcry_sexp_release (sexp);

      err = gcry_pubkey_get_sexp (&sexp, GCRY_PK_GET_SECKEY, ctx);
      if (gpg_err_code (err) != GPG_ERR_NO_SECKEY)
        fail ("gcry_pubkey_get_sexp(GET_SECKEY) returned wrong error: %s\n",
              gpg_strerror (err));
      gcry_sexp_release (sexp);
    }

  /* Skipping Ed25519 if in FIPS mode (it isn't supported) */
  if (gcry_fips_mode_active())
    goto cleanup;

  info ("checking sample public key (Ed25519)\n");
  q = hex2mpi (sample_ed25519_q);
  gcry_sexp_release (keyparam);
  err = gcry_sexp_build (&keyparam, NULL,
                        "(public-key(ecc(curve %s)(flags eddsa)(q %m)))",
                        "Ed25519", q);
  if (err)
    die ("gcry_sexp_build failed: %s\n", gpg_strerror (err));
  gcry_mpi_release (q);

  /* We can't call gcry_pk_testkey because it is only implemented for
     private keys.  */
  /* err = gcry_pk_testkey (keyparam); */
  /* if (err) */
  /*   fail ("gcry_pk_testkey failed for sample public key: %s\n", */
  /*         gpg_strerror (err)); */

  gcry_ctx_release (ctx);
  err = gcry_mpi_ec_new (&ctx, keyparam, NULL);
  if (err)
    fail ("gcry_mpi_ec_new failed for sample public key: %s\n",
          gpg_strerror (err));
  else
    {
      gcry_sexp_t sexp;

      get_and_cmp_mpi ("q", sample_ed25519_q, "Ed25519", ctx);
      get_and_cmp_point ("q", sample_ed25519_q_x, sample_ed25519_q_y,
                         "Ed25519", ctx);
      get_and_cmp_mpi ("q@eddsa", sample_ed25519_q_eddsa, "Ed25519", ctx);

      /* Set d to see whether Q is correctly re-computed.  */
      d = hex2mpi (sample_ed25519_d);
      err = gcry_mpi_ec_set_mpi ("d", d, ctx);
      if (err)
        fail ("setting d for Ed25519 failed: %s\n", gpg_strerror (err));
      gcry_mpi_release (d);
      get_and_cmp_mpi ("q", sample_ed25519_q, "Ed25519(recompute Q)", ctx);

      /* Delete Q by setting d and then clearing d.  The clearing is
         required so that we can check whether Q has been cleared and
         because further tests only expect a public key.  */
      d = hex2mpi (sample_ed25519_d);
      err = gcry_mpi_ec_set_mpi ("d", d, ctx);
      if (err)
        fail ("setting d for Ed25519 failed: %s\n", gpg_strerror (err));
      gcry_mpi_release (d);
      err = gcry_mpi_ec_set_mpi ("d", NULL, ctx);
      if (err)
        fail ("setting d for Ed25519 failed(2): %s\n", gpg_strerror (err));
      if (gcry_mpi_ec_get_mpi ("q", ctx, 0))
        fail ("setting d for Ed25519 did not reset Q\n");

      /* Set Q again.  We need to use an opaque MPI here because
         sample_ed25519_q is in uncompressed format which can only be
         auto-detected if passed opaque.  */
      q = hex2mpiopa (sample_ed25519_q);
      err = gcry_mpi_ec_set_mpi ("q", q, ctx);
      if (err)
        fail ("setting Q for Ed25519 failed: %s\n", gpg_strerror (err));
      gcry_mpi_release (q);
      get_and_cmp_mpi ("q", sample_ed25519_q, "Ed25519(2)", ctx);

      /* Get as s-expression.  */
      err = gcry_pubkey_get_sexp (&sexp, 0, ctx);
      if (err)
        fail ("gcry_pubkey_get_sexp(0) failed: %s\n", gpg_strerror (err));
      else if (debug)
        print_sexp ("Result of gcry_pubkey_get_sexp (0):\n", sexp);
      gcry_sexp_release (sexp);

      err = gcry_pubkey_get_sexp (&sexp, GCRY_PK_GET_PUBKEY, ctx);
      if (err)
        fail ("gcry_pubkey_get_sexp(GET_PUBKEY) failed: %s\n",
              gpg_strerror (err));
      else if (debug)
        print_sexp ("Result of gcry_pubkey_get_sexp (GET_PUBKEY):\n", sexp);
      gcry_sexp_release (sexp);

      err = gcry_pubkey_get_sexp (&sexp, GCRY_PK_GET_SECKEY, ctx);
      if (gpg_err_code (err) != GPG_ERR_NO_SECKEY)
        fail ("gcry_pubkey_get_sexp(GET_SECKEY) returned wrong error: %s\n",
              gpg_strerror (err));
      gcry_sexp_release (sexp);

    }

 cleanup:
  gcry_ctx_release (ctx);
  gcry_sexp_release (keyparam);
}




/* Create a new point from (X,Y,Z) given as hex strings.  */
gcry_mpi_point_t
make_point (const char *x, const char *y, const char *z)
{
  gcry_mpi_point_t point;

  point = gcry_mpi_point_new (0);
  gcry_mpi_point_snatch_set (point, hex2mpi (x), hex2mpi (y), hex2mpi (z));

  return point;
}


/* This tests checks that the low-level EC API yields the same result
   as using the high level API.  The values have been taken from a
   test run using the high level API.  */
static void
basic_ec_math (void)
{
  gpg_error_t err;
  gcry_ctx_t ctx;
  gcry_mpi_t P, A;
  gcry_mpi_point_t G, Q;
  gcry_mpi_t d;
  gcry_mpi_t x, y, z;

  wherestr = "basic_ec_math";
  info ("checking basic math functions for EC\n");

  P = hex2mpi ("0xfffffffffffffffffffffffffffffffeffffffffffffffff");
  A = hex2mpi ("0xfffffffffffffffffffffffffffffffefffffffffffffffc");
  G = make_point ("188DA80EB03090F67CBF20EB43A18800F4FF0AFD82FF1012",
                  "7192B95FFC8DA78631011ED6B24CDD573F977A11E794811",
                  "1");
  d = hex2mpi ("D4EF27E32F8AD8E2A1C6DDEBB1D235A69E3CEF9BCE90273D");
  Q = gcry_mpi_point_new (0);

  err = ec_p_new (&ctx, P, A);
  if (err)
    die ("ec_p_new failed: %s\n", gpg_strerror (err));

  x = gcry_mpi_new (0);
  y = gcry_mpi_new (0);
  z = gcry_mpi_new (0);

  {
    /* A quick check that multiply by zero works.  */
    gcry_mpi_t tmp;

    tmp = gcry_mpi_new (0);
    gcry_mpi_ec_mul (Q, tmp, G, ctx);
    gcry_mpi_release (tmp);
    gcry_mpi_point_get (x, y, z, Q);
    if (gcry_mpi_cmp_ui (z, 0))
      fail ("multiply a point by zero failed\n");
  }

  gcry_mpi_ec_mul (Q, d, G, ctx);

  if (gcry_mpi_ec_get_affine (x, y, Q, ctx))
    fail ("failed to get affine coordinates\n");
  if (cmp_mpihex (x, "008532093BA023F4D55C0424FA3AF9367E05F309DC34CDC3FE")
      || cmp_mpihex (y, "00C13CA9E617C6C8487BFF6A726E3C4F277913D97117939966"))
    fail ("computed affine coordinates of public key do not match\n");
  if (debug)
    {
      print_mpi ("q.x", x);
      print_mpi ("q.y", y);
    }

  gcry_mpi_release (z);
  gcry_mpi_release (y);
  gcry_mpi_release (x);
  gcry_mpi_point_release (Q);
  gcry_mpi_release (d);
  gcry_mpi_point_release (G);
  gcry_mpi_release (A);
  gcry_mpi_release (P);
  gcry_ctx_release (ctx);
}


/* This is the same as basic_ec_math but uses more advanced
   features.  */
static void
basic_ec_math_simplified (void)
{
  gpg_error_t err;
  gcry_ctx_t ctx;
  gcry_mpi_point_t G, Q;
  gcry_mpi_t d;
  gcry_mpi_t x, y, z;
  gcry_sexp_t sexp;

  wherestr = "basic_ec_math_simplified";
  info ("checking basic math functions for EC (variant)\n");

  d = hex2mpi ("D4EF27E32F8AD8E2A1C6DDEBB1D235A69E3CEF9BCE90273D");
  Q = gcry_mpi_point_new (0);

  err = gcry_mpi_ec_new (&ctx, NULL, "NIST P-192");
  if (err)
    die ("gcry_mpi_ec_new failed: %s\n", gpg_strerror (err));
  G = gcry_mpi_ec_get_point ("g", ctx, 1);
  if (!G)
    die ("gcry_mpi_ec_get_point(G) failed\n");
  gcry_mpi_ec_mul (Q, d, G, ctx);

  x = gcry_mpi_new (0);
  y = gcry_mpi_new (0);
  z = gcry_mpi_new (0);

  if (gcry_mpi_ec_get_affine (x, y, Q, ctx))
    fail ("failed to get affine coordinates\n");
  if (cmp_mpihex (x, "008532093BA023F4D55C0424FA3AF9367E05F309DC34CDC3FE")
      || cmp_mpihex (y, "00C13CA9E617C6C8487BFF6A726E3C4F277913D97117939966"))
    fail ("computed affine coordinates of public key do not match\n");
  if (debug)
    {
      print_mpi ("q.x", x);
      print_mpi ("q.y", y);
    }

  gcry_mpi_release (z);
  gcry_mpi_release (y);
  gcry_mpi_release (x);

  /* Let us also check whether we can update the context.  */
  err = gcry_mpi_ec_set_point ("g", G, ctx);
  if (err)
    die ("gcry_mpi_ec_set_point(G) failed\n");
  err = gcry_mpi_ec_set_mpi ("d", d, ctx);
  if (err)
    die ("gcry_mpi_ec_set_mpi(d) failed\n");

  /* FIXME: Below we need to check that the returned S-expression is
     as requested.  For now we use manual inspection using --debug.  */

  /* Does get_sexp return the private key?  */
  err = gcry_pubkey_get_sexp (&sexp, 0, ctx);
  if (err)
    fail ("gcry_pubkey_get_sexp(0) failed: %s\n", gpg_strerror (err));
  else if (debug)
    print_sexp ("Result of gcry_pubkey_get_sexp (0):\n", sexp);
  gcry_sexp_release (sexp);

  /* Does get_sexp return the public key if requested?  */
  err = gcry_pubkey_get_sexp (&sexp, GCRY_PK_GET_PUBKEY, ctx);
  if (err)
    fail ("gcry_pubkey_get_sexp(GET_PUBKEY) failed: %s\n", gpg_strerror (err));
  else if (debug)
    print_sexp ("Result of gcry_pubkey_get_sexp (GET_PUBKEY):\n", sexp);
  gcry_sexp_release (sexp);

  /* Does get_sexp return the public key after d has been deleted?  */
  err = gcry_mpi_ec_set_mpi ("d", NULL, ctx);
  if (err)
    die ("gcry_mpi_ec_set_mpi(d=NULL) failed\n");
  err = gcry_pubkey_get_sexp (&sexp, 0, ctx);
  if (err)
    fail ("gcry_pubkey_get_sexp(0 w/o d) failed: %s\n", gpg_strerror (err));
  else if (debug)
    print_sexp ("Result of gcry_pubkey_get_sexp (0 w/o d):\n", sexp);
  gcry_sexp_release (sexp);

  /* Does get_sexp return an error after d has been deleted?  */
  err = gcry_pubkey_get_sexp (&sexp, GCRY_PK_GET_SECKEY, ctx);
  if (gpg_err_code (err) != GPG_ERR_NO_SECKEY)
    fail ("gcry_pubkey_get_sexp(GET_SECKEY) returned wrong error: %s\n",
          gpg_strerror (err));
  gcry_sexp_release (sexp);

  /* Does get_sexp return an error after d and Q have been deleted?  */
  err = gcry_mpi_ec_set_point ("q", NULL, ctx);
  if (err)
    die ("gcry_mpi_ec_set_point(q=NULL) failed\n");
  err = gcry_pubkey_get_sexp (&sexp, 0, ctx);
  if (gpg_err_code (err) != GPG_ERR_BAD_CRYPT_CTX)
    fail ("gcry_pubkey_get_sexp(0 w/o Q,d) returned wrong error: %s\n",
          gpg_strerror (err));
  gcry_sexp_release (sexp);


  gcry_mpi_point_release (Q);
  gcry_mpi_release (d);
  gcry_mpi_point_release (G);
  gcry_ctx_release (ctx);
}


/* Check the math used with Twisted Edwards curves.  */
static void
twistededwards_math (void)
{
  gpg_error_t err;
  gcry_ctx_t ctx;
  gcry_mpi_point_t G, Q;
  gcry_mpi_t k;
  gcry_mpi_t w, a, x, y, z, p, n, b, I;

  wherestr = "twistededwards_math";
  info ("checking basic Twisted Edwards math\n");

  err = gcry_mpi_ec_new (&ctx, NULL, "Ed25519");
  if (err)
    die ("gcry_mpi_ec_new failed: %s\n", gpg_strerror (err));

  k = hex2mpi
    ("2D3501E723239632802454EE5DDC406EFB0BDF18486A5BDE9C0390A9C2984004"
     "F47252B628C953625B8DEB5DBCB8DA97AA43A1892D11FA83596F42E0D89CB1B6");
  G = gcry_mpi_ec_get_point ("g", ctx, 1);
  if (!G)
    die ("gcry_mpi_ec_get_point(G) failed\n");
  Q = gcry_mpi_point_new (0);


  w = gcry_mpi_new (0);
  a = gcry_mpi_new (0);
  x = gcry_mpi_new (0);
  y = gcry_mpi_new (0);
  z = gcry_mpi_new (0);
  I = gcry_mpi_new (0);
  p = gcry_mpi_ec_get_mpi ("p", ctx, 1);
  n = gcry_mpi_ec_get_mpi ("n", ctx, 1);
  b = gcry_mpi_ec_get_mpi ("b", ctx, 1);

  /* Check: 2^{p-1} mod p == 1 */
  gcry_mpi_sub_ui (a, p, 1);
  gcry_mpi_powm (w, GCRYMPI_CONST_TWO, a, p);
  if (gcry_mpi_cmp_ui (w, 1))
    fail ("failed assertion: 2^{p-1} mod p == 1\n");

  /* Check: p % 4 == 1 */
  gcry_mpi_mod (w, p, GCRYMPI_CONST_FOUR);
  if (gcry_mpi_cmp_ui (w, 1))
    fail ("failed assertion: p %% 4 == 1\n");

  /* Check: 2^{n-1} mod n == 1 */
  gcry_mpi_sub_ui (a, n, 1);
  gcry_mpi_powm (w, GCRYMPI_CONST_TWO, a, n);
  if (gcry_mpi_cmp_ui (w, 1))
    fail ("failed assertion: 2^{n-1} mod n == 1\n");

  /* Check: b^{(p-1)/2} mod p == p-1 */
  gcry_mpi_sub_ui (a, p, 1);
  gcry_mpi_div (x, NULL, a, GCRYMPI_CONST_TWO, -1);
  gcry_mpi_powm (w, b, x, p);
  gcry_mpi_abs (w);
  if (gcry_mpi_cmp (w, a))
    fail ("failed assertion: b^{(p-1)/2} mod p == p-1\n");

  /* I := 2^{(p-1)/4} mod p */
  gcry_mpi_sub_ui (a, p, 1);
  gcry_mpi_div (x, NULL, a, GCRYMPI_CONST_FOUR, -1);
  gcry_mpi_powm (I, GCRYMPI_CONST_TWO, x, p);

  /* Check: I^2 mod p == p-1 */
  gcry_mpi_powm (w, I, GCRYMPI_CONST_TWO, p);
  if (gcry_mpi_cmp (w, a))
    fail ("failed assertion: I^2 mod p == p-1\n");

  /* Check: G is on the curve */
  if (!gcry_mpi_ec_curve_point (G, ctx))
    fail ("failed assertion: G is on the curve\n");

  /* Check: nG == (0,1) */
  gcry_mpi_ec_mul (Q, n, G, ctx);
  if (gcry_mpi_ec_get_affine (x, y, Q, ctx))
    fail ("failed to get affine coordinates\n");
  if (gcry_mpi_cmp_ui (x, 0) || gcry_mpi_cmp_ui (y, 1))
    fail ("failed assertion: nG == (0,1)\n");

  /* Now two arbitrary point operations taken from the ed25519.py
     sample data.  */
  gcry_mpi_release (a);
  a = hex2mpi
    ("4f71d012df3c371af3ea4dc38385ca5bb7272f90cb1b008b3ed601c76de1d496"
     "e30cbf625f0a756a678d8f256d5325595cccc83466f36db18f0178eb9925edd3");
  gcry_mpi_ec_mul (Q, a, G, ctx);
  if (gcry_mpi_ec_get_affine (x, y, Q, ctx))
    fail ("failed to get affine coordinates\n");
  if (cmp_mpihex (x, ("157f7361c577aad36f67ed33e38dc7be"
                      "00014fecc2165ca5cee9eee19fe4d2c1"))
      || cmp_mpihex (y, ("5a69dbeb232276b38f3f5016547bb2a2"
                         "4025645f0b820e72b8cad4f0a909a092")))
    {
      fail ("sample point multiply failed:\n");
      print_mpi ("r", a);
      print_mpi ("Rx", x);
      print_mpi ("Ry", y);
    }

  gcry_mpi_release (a);
  a = hex2mpi
    ("2d3501e723239632802454ee5ddc406efb0bdf18486a5bde9c0390a9c2984004"
     "f47252b628c953625b8deb5dbcb8da97aa43a1892d11fa83596f42e0d89cb1b6");
  gcry_mpi_ec_mul (Q, a, G, ctx);
  if (gcry_mpi_ec_get_affine (x, y, Q, ctx))
    fail ("failed to get affine coordinates\n");
  if (cmp_mpihex (x, ("6218e309d40065fcc338b3127f468371"
                      "82324bd01ce6f3cf81ab44e62959c82a"))
      || cmp_mpihex (y, ("5501492265e073d874d9e5b81e7f8784"
                         "8a826e80cce2869072ac60c3004356e5")))
    {
      fail ("sample point multiply failed:\n");
      print_mpi ("r", a);
      print_mpi ("Rx", x);
      print_mpi ("Ry", y);
    }


  gcry_mpi_release (I);
  gcry_mpi_release (b);
  gcry_mpi_release (n);
  gcry_mpi_release (p);
  gcry_mpi_release (w);
  gcry_mpi_release (a);
  gcry_mpi_release (x);
  gcry_mpi_release (y);
  gcry_mpi_release (z);
  gcry_mpi_point_release (Q);
  gcry_mpi_point_release (G);
  gcry_mpi_release (k);
  gcry_ctx_release (ctx);
}


/* Check the point on curve function.  */
static void
point_on_curve (void)
{
  static struct {
    const char *curve;
    int oncurve;      /* Point below is on the curve.  */
    const char *qx;
    const char *qy;
  } t[] = {
    {
      "NIST P-256", 0,
      "015B4F6775D68D4D2E2192C6B8027FC5A3D49957E453CB251155AA3FF5D3EC9974",
      "4BC4C87B57A25E1056831208AB5B8F091142F891E9FF19F1E090B030DF1087B3"
    }, {
      "NIST P-256", 0,
      "D22C316E7EBE7B293BD66808E000806F0754398A5D72A4F9BBC21C26EAC0A651",
      "3C8DB80CC3CDE5E530D040536E6A58AAB41C33FA70B30896943513FF3690132D"
    }, {
      "NIST P-256", 0,
      "0130F7E7BC52854CA493A0DE87DC4AB3B4343758F2B634F15B10D70DBC0A5A5291",
      "86F9CA73C25CE86D54CB21C181AECBB52A5971334FF5040F76CAE9845ED46023"
    }, {
      "NIST P-256", 1,
      "14957B602C7849F28858C7407696F014BC091D6D68C449560B7A38147D6E6A9B",
      "A8E09EFEECFE00C797A0848F38B61992D30C61FAB13021E88C8BD3545B3A6C63"
    }, {
      "NIST P-256", 0,
      "923DE4957241DD97780841C76294DB0D4F5DC04C3045081174764D2D32AD2D53",
      "01B4B1A2027C02F0F520A3B01E4CE3C668BF481346A74499C5D1044A53E210B600"
    }, {
      "NIST P-256", 1,
      "9021DFAB8B4DAEAADA634AAA26D6E5FFDF8C0476FF5CA31606C870A1B933FB36",
      "9AFC65EEB24E46C7B75712EF29A981CB09FAC56E2B81D3ED024748CCAB1CB77E"
    }, {
      "NIST P-256", 0,
      "011529F0B26DE5E0EB2DA4BFB6C149C802CB52EE479DD666553286928A4005E990",
      "0EBC63DB2104884456DC0AA81A3F4E99D93B7AE2CD4B1489655EA9BE6289CF9E"
    }, {
      "NIST P-256", 1,
      "216EC5DE8CA989199D31F0DFCD381DCC9270A0785365EC3E34CA347C070A87BE",
      "87A88897BA763509ECC1DBE28D9D37F6F4E70E3B99B1CD3C0B934D4190968A6D"
    }, {
      "NIST P-256", 1,
      "7ABAA44ACBC6016FDB52A6F45F6178E65CBFC35F9920D99149CA9999612CE945",
      "88F7684BDCDA31EAFB6CAD859F8AB29B5D921D7DB2B34DF7E40CE36235F45B63"
    }, {
      "NIST P-256", 0,
      "E765B4272D211DD0064189B55421FB76BB3A7756364A6CB1627FAED848157A84",
      "C13171CFFB243E06B203F0996BBDD16F52292AD11F2DA81106E9C2FD87F4FA0F"
    }, {
      "NIST P-256", 0,
      "EE4999DFC3A1871EE7A592BE26A09BEC9D9B561613EE9EFB6ED42F17985C9CDC",
      "8399E967338A7A618336AF70DA67D9CAC1C19267809652F5C5183C8B129E0902"
    }, {
      "NIST P-256", 0,
      "F755D0CF2642A2C7FBACCC8E9E442B8B047A99C6E052B2FA5AB0544B36B4D51C",
      "AA080F17657B6565D9A4D94BD260B54D92FEE8DC4A78C4FC9C19209933AF39B0"
    } , {
      "NIST P-384", 0,
      "CBFC7DBEBF15BEAD682549757F9BBA0E3F67669DF13FCE0EBE8024B725B38B00"
      "83EC46A8F2FF3203C5C7F8C7E722A5EF",
      "0548FE281BEAB18FD1AB86F59B0CA524479A4A81373C83B78AFFD801FAC75922"
      "96470753DCF46173C9AA4A8A4C2FBE51"
    }, {
      "NIST P-384", 0,
      "1DC8E054A883DB81EAEDE6C487B26816C927B8196780525A6CA8F675D2557752"
      "02CE06CCBE705EA8A38AA2894D4BEEE6",
      "010191050E867AFAA96A199FE9C591CF8B853D81486786DA889124881FB39D2F"
      "8E0875F4C4BB1E3D0F8535C7A52306FB82"
    }, {
      "NIST P-384", 1,
      "2539FC368CE1D5E464B6C0FBB12D557B712327DB086975255AD7D17F7E7E4F23"
      "D719ED4116E2CC907AEB92CF22331A60",
      "8843FDBA742CB64323E49CEBE8DD74908CFC9C3AA0015662DFBB7219E92CF32E"
      "9FC63F61EF19DE9B3CEA98D163ABF254"
    }, {
      "NIST P-384", 0,
      "0B786DACF400D43575394349EDD9F9CD145FC7EF737A3C5F69B253BE7639DB24"
      "EC2F0CA62FF1F90B6515DE356EC2A404",
      "225D6B2939CC7F7133F43353946A682C68DAC6BB75EE9CF6BD9A1609FA915692"
      "72F4D3A87E88529754E109BB9B61B03B"
    }, {
      "NIST P-384", 0,
      "76C660C9F58CF2051F9F8B06049694AB6FE418009DE6F0A0833BC690CEC06CC2"
      "9A440AD51C94CF5BC28817C8C6E2D302",
      "012974E5D9E55304ED294AB6C7A3C65B663E67ABC5E6F6C0F6498B519F2F6CA1"
      "8306976291F3ADC0B5ABA42DED376EA9A5"
    }, {
      "NIST P-384", 0,
      "23D758B1EDB8E12E9E707C53C131A19D9464B20EE05C99766F5ABDF9F906AD03"
      "B958BF28B022E54E320672C4BAD4EEC0",
      "01E9E72870C88F4C82A5AB3CC8A3398E8F006BF3EC05FFBB1EFF8AEE88020FEA"
      "9E558E9F58ED1D324C9DCBCB4E8F2A5970"
    }, {
      "NIST P-384", 0,
      "D062B96D5A10F715ACF361F99262ABF0F7693A8BB60ECB1DF459CF95750E4293"
      "18BCB9FC60499D009F949298F3F9F47B",
      "9089C6328E4B39A73D7EE6FAE1A77E48CE354B83BBCE432082C32C8FD6784B86"
      "CFE9C552E2E720F5DA5806503D3784CD"
    }, {
      "NIST P-384", 0,
      "2A951D4D6EB35C43D94866280D37365B82441BC84D62CBFF3365CAB1FD0A3E20"
      "823CA8F84D2BBF4EA687885437DE7839",
      "01CC7D762AFE613F7B5568BC516568A421159C40599E8D52DE10E8F9488931E1"
      "69F3656C322DE45C4A70DC6DB9A661E599"
    }, {
      "NIST P-384", 1,
      "A4BAEE6CDAF3AEB69032B3FBA811707C54F5753670DA5173D891547E8CBAEEF3"
      "89B92C9A55573A596123415FBFA26991",
      "3241EA716583C11C71BB30AF6C5E3A6637956F17ADBBE641BAB52E8539F9FC7B"
      "F3B04F46DBFFE08151E0F0950CC70081"
    }, {
      "NIST P-384", 0,
      "5C0E18B0DE3261BCBCFC7B702C2D75CF481336BFBADF420BADC616235C1966AB"
      "4C0F876575DDEC1BDB3F3F04061C9AE4",
      "E90C78550D1C922F1D8161D8C9C0576E29BD09CA665376FA887D13FA8DF48352"
      "D7BBEEFB803F6CC8FC7895E47F348D33"
    }, {
      "NIST P-384", 1,
      "2015864CD50F0A1A50E6401F44191665C19E4AD4B4903EA9EB464E95D1070E36"
      "F1D8325E45734D5A0FDD103F4DF6F83E",
      "5FB3E9A5C59DD5C5262A8176CB7032A00AE33AED08485884A3E5D68D9EEB990B"
      "F26E8D87EC175577E782AD51A6A12C02"
    }, {
      "NIST P-384", 1,
      "56EBF5310EEF5A5D8D001F570A18625383ECD4882B3FC738A69874E7C9D8F89C"
      "187BECA23369DFD6C15CC0DA0629958F",
      "C1230B349FB662CB762563DB8F9FCB32D5CCA16120681C474D67D279CCA6F6DB"
      "73DE6AA96140B5C457B7486E06D318CE"
    }, {
      "NIST P-521", 0,
      "01E4D82EE5CD6DA37080252295EFA273BBBA6952012D0120EAF131E73F1E5024"
      "36E3324624471040030E1C345D65490ECEE9B64E03B15B6C7EB69A39C618BAFEED70",
      "03EE3A3C88A6933B7B16016BE4CC4E3BF5EA0625CB3DB2604CDCBBD02CABBC90"
      "8904D9DB42998F6C5101D4D4318ACFC9643C9CD641F636D1810ED86F1840EA74F3C0"
    }, {
      "NIST P-521", 0,
      "01F3DFCB5433387B6B2E3F74177F4F3D7300F05E1AD49DE112630E27B1C8A437"
      "1E742CB020E0039B5477FC897D17332034F9660B3066764EFF5FB440EB8856E782E3",
      "02D337616C9D202DC5E290C486F5855CBD6A8470AE62CA96245834CF49257D8D"
      "96D4041B15007650DEE668C00DDBF749054256C571F60980AC74D0DBCA7FB96C2F48"
    }, {
      "NIST P-521", 1,
      "822A846606DC9E96452CAC373567A8B57D9ACA15B177F75DD7EF10C635F52CE4"
      "EF6ABEEDB90D3F48F50A0C9015A95C955A25C45DE8413DE3BF899B6B1E62CF7CB8",
      "0102771B5F3EC8C36838CEC04DCBC28AD1E38C37DAB0EA89B5EE92D21F7A35CE"
      "ABC8B155EDC70154D6DFA2E77EC1D8C4A3406A6BD0ECF8F1EE2AC33A02464CB70C97"
    }, {
      "NIST P-521", 0,
      "F733D48467912D1FFE46CF442F27FDD218D190E7B8A829D822DA3B6BAF9B987E"
      "5B4BCCE34499248F59EEAF74F63ED15FF73F243C6FC3FD5E5842F6A3BA34C2022D",
      "0281AAAD1B7EEBABEB6EC67932CB7E95717AFA3B4CF7A2DB151CD537C419C3A5"
      "156ED9160758190B47696CDC15E81BBAD12975283907A571604DB23F702AEA4B38FF"
    }, {
      "NIST P-521", 0,
      "03B1B274175AAEB5907152E5114CCAEADA28A7ADD4A2B1831C3D8302E8596489"
      "E2C98B9B8D0CAE98C03BB11E28CE66D4736449758AF58BAFE40EF5A5FA22C9A43117",
      "94C5951F81D544E959EDFC5DC1D5F42FE427871D4FB91A43A0B4A6BEA6B35B9E"
      "BC5FB444C70BE4FD47B4ED16704F8C86EF019FC47C7FF2271F8B0DDEA9E2D3BCDD"
    }, {
      "NIST P-521", 1,
      "F2248C318055DE37CD706D4FCAF7E7D96737A4A7B6B8067A66DCD58B6B8DFC55"
      "90ECE67F6AA67F9C51B57E7B023075F2F42909BF47361CB6881C10F55FB7215B56",
      "0162F735CE6A2ADA54CAF96A12D6888C02DE0A74638CF34CE39DABBACA4D651B"
      "7E6ED1A65B551B36BAE7BE474BB6E6905ED0E33C7BA2021885027C7C6E40C5613004"
    }, {
      "NIST P-521", 0,
      "9F08E97FEADCF0A391CA1EA4D97B5FE62D3B164593E12027EB967BD6E1FA841A"
      "9831158DF164BCAD0BF3ADA96127745E25F349BDDD52EEA1654892B35960C9C023",
      "AE2A25F5440F258AFACA6925C4C9F7AEAD3CB67153C4FACB31AC33F58B43A78C"
      "B14F682FF726CEE2A6B6F6B481AEEB29A9B3150F02D1CFB764672BA8294C477291"
    }, {
      "NIST P-521", 0,
      "01047B52014748C904980716953206A93F0D01B34CA94A997407FA93FE304F86"
      "17BB6E402B2BB8B434C2671ECE953ABE7BADB75713CD9DF950943A33A9A19ACCDABE",
      "7433533F098037DEA616337986887D01C5CC8DEC3DC1FDB9CDF7287EF27CC125"
      "54FCF3A5E212DF9DAD9F8A3A7173B23FC6E15930704F3AEE1B074BDDB0ED6823E4"
    }, {
      "NIST P-521", 0,
      "01C2A9EBF51592FE6589F618EAADA1697D9B2EC7CE5D48C9E80FC597642B23F1"
      "F0EBE953449762BD3F094F57791D9850AFE98BBDA9872BE399B7BDD617860076BB03",
      "0B822E27692F63DB8E12C59BB3CCA172B9BBF613CAE5F9D1474186E45E8B26FF"
      "962084E1C6BE74821EDBB60941A3B75516F603719563433383812BFEA89EC14B89"
    }, {
      "NIST P-521", 0,
      "99390F342C3F0D46E80C5B65C61E8AA8ACA0B6D4E1352404586364A05D8398E9"
      "2BC71A644E8663F0A9B87D0B3ACAEE32F2AB9B321317AD23059D045EBAB91C5D93",
      "82FCF93AE4467EB57766F2B150E736636727E7282500CD482DA70D153D195F2B"
      "DF9B96D689A0DC1BB9137B41557A33F202F1B71840544CBEFF03072E77E4BB6F0B"
    }, {
      "NIST P-521", 1,
      "018E48E80594FF5496D8CC7DF8A19D6AA18805A4EF4490038AED6A1E9AA18056"
      "D0244A97DCF6D132C6804E3F4F369922119544B4C057D783C848FB798B48730A382C",
      "01AF510B4F5E1C40BC9C110216D35E7C6D7A2BEE52914FC98258676288449901"
      "F27A07EE91DF2D5D79259712906C3E18A990CBF35BCAC41A952820CE2BA8D0220080"
    }, {
      "NIST P-521", 1,
      "ADCEF3539B4BC831DC0AFD173137A4426152058AFBAE06A17FCB89F4DB6E48B5"
      "335CB88F8E4DB475A1E390E5656072F06605BFB84CBF9795B7992ECA04A8E10CA1",
      "01BCB985AFD6404B9EDA49B6190AAA346BF7D5909CA440C0F7E505C62FAC8635"
      "31D3EB7B2AC4DD4F4404E4B12E9D6D3C596179587F3724B1EFFF684CFDB4B21826B9"
    }
  };
  gpg_error_t err;
  int tidx;
  const char *lastcurve = NULL;
  gcry_ctx_t ctx = NULL;
  gcry_mpi_t qx = NULL;
  gcry_mpi_t qy = NULL;
  gcry_mpi_point_t Q;
  int oncurve;

  wherestr = "point_on_curve";
  for (tidx=0; tidx < DIM (t); tidx++)
    {
      if (!t[tidx].curve)
        {
          if (!lastcurve || !ctx)
            die ("invalid test vectors at idx %d\n", tidx);
        }
      else if (!ctx || !lastcurve || strcmp (t[tidx].curve, lastcurve))
        {
          lastcurve = t[tidx].curve;
          gcry_ctx_release (ctx);
          err = gcry_mpi_ec_new (&ctx, NULL, lastcurve);
          if (err)
            die ("error creating context for curve %s at idx %d: %s\n",
                 lastcurve, tidx, gpg_strerror (err));

          info ("checking points on curve %s\n", lastcurve);
        }

      gcry_mpi_release (qx);
      gcry_mpi_release (qy);
      qx = hex2mpi (t[tidx].qx);
      qy = hex2mpi (t[tidx].qy);

      Q = gcry_mpi_point_set (NULL, qx, qy, GCRYMPI_CONST_ONE);
      if (!Q)
        die ("gcry_mpi_point_set(Q) failed at idx %d\n", tidx);

      oncurve = gcry_mpi_ec_curve_point (Q, ctx);

      if (t[tidx].oncurve && !oncurve)
        {
          fail ("point expected on curve but not identified as such (i=%d):\n",
                tidx);
          print_point ("  Q", Q);
        }
      else if (!t[tidx].oncurve && oncurve)
        {
          fail ("point not expected on curve but identified as such (i=%d):\n",
                tidx);
          print_point ("  Q", Q);
        }
      gcry_mpi_point_release (Q);
    }

  gcry_mpi_release (qx);
  gcry_mpi_release (qy);
  gcry_ctx_release (ctx);
}


static gcry_mpi_t
mpi_base10_scan (const char *str)
{
  gcry_mpi_t k;

  k = gcry_mpi_new (0);

  while (*str)
    {
      gcry_mpi_mul_ui (k, k, 10);
      gcry_mpi_add_ui (k, k, *str - '0');
      str++;
    }

  return k;
}


static void
check_ec_mul (void)
{
  static struct {
    const char *curve;
    const char *k_base10;
    const char *qx;
    const char *qy;
  } tv[] = {
    /* NIST EC test vectors from http://point-at-infinity.org/ecc/nisttv */
    { /* tv 0 */
      "NIST P-192",
      "1",
      "188DA80EB03090F67CBF20EB43A18800F4FF0AFD82FF1012",
      "07192B95FFC8DA78631011ED6B24CDD573F977A11E794811"
    },
    { /* tv 1 */
      "NIST P-192",
      "2",
      "DAFEBF5828783F2AD35534631588A3F629A70FB16982A888",
      "DD6BDA0D993DA0FA46B27BBC141B868F59331AFA5C7E93AB"
    },
    { /* tv 2 */
      "NIST P-192",
      "3",
      "76E32A2557599E6EDCD283201FB2B9AADFD0D359CBB263DA",
      "782C37E372BA4520AA62E0FED121D49EF3B543660CFD05FD"
    },
    { /* tv 3 */
      "NIST P-192",
      "4",
      "35433907297CC378B0015703374729D7A4FE46647084E4BA",
      "A2649984F2135C301EA3ACB0776CD4F125389B311DB3BE32"
    },
    { /* tv 4 */
      "NIST P-192",
      "5",
      "10BB8E9840049B183E078D9C300E1605590118EBDD7FF590",
      "31361008476F917BADC9F836E62762BE312B72543CCEAEA1"
    },
    { /* tv 5 */
      "NIST P-192",
      "6",
      "A37ABC6C431F9AC398BF5BD1AA6678320ACE8ECB93D23F2A",
      "851B3CAEC99908DBFED7040A1BBDA90E081F7C5710BC68F0"
    },
    { /* tv 6 */
      "NIST P-192",
      "7",
      "8DA75A1F75DDCD7660F923243060EDCE5DE37F007011FCFD",
      "57CB5FCF6860B35418240DB8FDB3C01DD4B702F96409FFB5"
    },
    { /* tv 7 */
      "NIST P-192",
      "8",
      "2FA1F92D1ECCE92014771993CC14899D4B5977883397EDDE",
      "A338AFDEF78B7214273B8B5978EF733FF2DD8A8E9738F6C0"
    },
    { /* tv 8 */
      "NIST P-192",
      "9",
      "818A4D308B1CABB74E9E8F2BA8D27C9E1D9D375AB980388F",
      "01D1AA5E208D87CD7C292F7CBB457CDF30EA542176C8E739"
    },
    { /* tv 9 */
      "NIST P-192",
      "10",
      "AA7C4F9EF99E3E96D1AEDE2BD9238842859BB150D1FE9D85",
      "3212A36547EDC62901EE3658B2F4859460EB5EB2491397B0"
    },
    { /* tv 10 */
      "NIST P-192",
      "11",
      "1C995995EB76324F1844F7164D22B652280940370628A2AA",
      "EF1765CE37E9EB73029F556400FA77BDB34CB8611AAA9C04"
    },
    { /* tv 11 */
      "NIST P-192",
      "12",
      "1061343F3D456D0ECA013877F8C9E7B28FCCDCDA67EEB8AB",
      "5A064CAA2EA6B03798FEF8E3E7A48648681EAC020B27293F"
    },
    { /* tv 12 */
      "NIST P-192",
      "13",
      "112AF141D33EFB9F2F68821E051E4EA004144A363C4A090A",
      "6E0CBE3BFC5293F72A2C1726E081E09E7F10A094432B1C1E"
    },
    { /* tv 13 */
      "NIST P-192",
      "14",
      "13B9310646EBC93B591746B3F7C64E05DEE08843DE1081C1",
      "1EDCEA63B44142DD15F3B427EC41A1EC4FBACA95E186E6B4"
    },
    { /* tv 14 */
      "NIST P-192",
      "15",
      "8C9595E63B56B633BA3546B2B5414DE736DE4A9E7578B1E7",
      "266B762A934F00C17CF387993AA566B6AD7537CDD98FC7B1"
    },
    { /* tv 15 */
      "NIST P-192",
      "16",
      "B7310B4548FBFDBD29005092A5355BFCD99473733048AFDF",
      "FF9EAE9EDCD27C1E42D8585C4546D9491845C56629CF2290"
    },
    { /* tv 16 */
      "NIST P-192",
      "17",
      "44275CD2E1F46DC3F9F57636C2B4213B8BB445930510FF8A",
      "EFAD8348FDE30C87DE438612A818E98D9B76A67AD25DDFD0"
    },
    { /* tv 17 */
      "NIST P-192",
      "18",
      "C1B4DB0227210613A6CA15C428024E40B6513365D72591A3",
      "1E26B286BCA1D08F4FE8F801267DF9FD7782EC3EC3F47F53"
    },
    { /* tv 18 */
      "NIST P-192",
      "19",
      "C0626BCF247DE5D307FD839238D72688774FC97A1CF8AD1B",
      "9CDC99D753973DC197E12778E829C804EC1A6B4E71FAA20A"
    },
    { /* tv 19 */
      "NIST P-192",
      "20",
      "BB6F082321D34DBD786A1566915C6DD5EDF879AB0F5ADD67",
      "91E4DD8A77C4531C8B76DEF2E5339B5EB95D5D9479DF4C8D"
    },
    { /* tv 20 */
      "NIST P-192",
      "112233445566778899",
      "81E6E0F14C9302C8A8DCA8A038B73165E9687D0490CD9F85",
      "F58067119EED8579388C4281DC645A27DB7764750E812477"
    },
    { /* tv 21 */
      "NIST P-192",
      "112233445566778899112233445566778899",
      "B357B10AC985C891B29FB37DA56661CCCF50CEC21128D4F6",
      "BA20DC2FA1CC228D3C2D8B538C2177C2921884C6B7F0D96F"
    },
    { /* tv 22 */
      "NIST P-192",
      "1618292094200346491064154703205151664562462359653015613567",
      "74FEC215F253C6BD845831E059B318C87F727B136A700B91",
      "4B702B15B126A703E7A7CEC3E0EC81F8DFCA73A59F5D88B9"
    },
    { /* tv 23 */
      "NIST P-192",
      "1484605055214526729816930749766694384906446681761906688",
      "0C40230F9C4B8C0FD91F2C604FCBA9B87C2DFA153F010B4F",
      "5FC4F5771F467971B2C82752413833A68CE00F4A9A692B02"
    },
    { /* tv 24 */
      "NIST P-192",
      "1569275434166462877105627261392580354519833538813866540831",
      "28783BBF6208E1FF0F965FD8DC0C26FF1D8E02B433EDF2F7",
      "A5852BBC44FD8164C1ABA9A3EC7A88E461D5D77ABD743E87"
    },
    { /* tv 25 */
      "NIST P-192",
      "3138550867681922400546388175470823984762234518836963313664",
      "45DAF0A306121BDB3B82E734CB44FDF65C9930F0E4FD2068",
      "F039FACE58EB7DE34E3374ADB28DF81F019C4548BAA75B64"
    },
    { /* tv 26 */
      "NIST P-192",
      "3138550119404545973088374812479323842475901485681169401600",
      "1D5EC85004EA2ABA905CEF98A818A8C3516D7CB69A6FD575",
      "4008F35F5820F66C902195644162E5AA231DD69C9E1ECC97"
    },
    { /* tv 27 */
      "NIST P-192",
      "24519928471166604179655321383971467003990211439919824896",
      "F063727C2EA4D358AB02F6B0BEEB14DBEAF2E8A1DB3208EE",
      "427418C015553361769B6A0C42923C4CA103740B6DCD9703"
    },
    { /* tv 28 */
      "NIST P-192",
      "46756768218837031708063422466358611246556475572231",
      "DC81D33CA6604B1EFE49386CD492979EF807B8BAEB8566E3",
      "D454247FF478514556333B3901C9F1CCC18DBC9AB938CFA0"
    },
    { /* tv 29 */
      "NIST P-192",
      "3138502977207688322901699644928655553044791844086883549215",
      "D932741DF6AA0E1EED24279150436C752AA5ADCFD0698D72",
      "9759B6D2EF21D885E94CDFF219F17004D8763401DAB021B5"
    },
    { /* tv 30 */
      "NIST P-192",
      "47890485652059026491391979477371914515865621847605503",
      "571477E9D9F2A628780742257F7250C4224C483B30F3A97E",
      "1AD35EE3177D22DD5F01B5A46FFDEC547B6A41786EBB8C8F"
    },
    { /* tv 31 */
      "NIST P-192",
      "3138549376958826959341570842566593375326996431013993775615",
      "4C69939642792776C826DB8B4EBF4BD8C03FC9DFA2AEC822",
      "29BF35BE52A6036E07EBA5741CFEB4C143310216EF1B9A2E"
    },
    { /* tv 32 */
      "NIST P-192",
      "6277101735386680763835789423176059013767194773182842284061",
      "BB6F082321D34DBD786A1566915C6DD5EDF879AB0F5ADD67",
      "6E1B2275883BACE37489210D1ACC64A046A2A26B8620B372"
    },
    { /* tv 33 */
      "NIST P-192",
      "6277101735386680763835789423176059013767194773182842284062",
      "C0626BCF247DE5D307FD839238D72688774FC97A1CF8AD1B",
      "63236628AC68C23E681ED88717D637FA13E594B18E055DF5"
    },
    { /* tv 34 */
      "NIST P-192",
      "6277101735386680763835789423176059013767194773182842284063",
      "C1B4DB0227210613A6CA15C428024E40B6513365D72591A3",
      "E1D94D79435E2F70B01707FED9820601887D13C13C0B80AC"
    },
    { /* tv 35 */
      "NIST P-192",
      "6277101735386680763835789423176059013767194773182842284064",
      "44275CD2E1F46DC3F9F57636C2B4213B8BB445930510FF8A",
      "10527CB7021CF37821BC79ED57E71671648959852DA2202F"
    },
    { /* tv 36 */
      "NIST P-192",
      "6277101735386680763835789423176059013767194773182842284065",
      "B7310B4548FBFDBD29005092A5355BFCD99473733048AFDF",
      "00615161232D83E1BD27A7A3BAB926B5E7BA3A99D630DD6F"
    },
    { /* tv 37 */
      "NIST P-192",
      "6277101735386680763835789423176059013767194773182842284066",
      "8C9595E63B56B633BA3546B2B5414DE736DE4A9E7578B1E7",
      "D99489D56CB0FF3E830C7866C55A9948528AC8322670384E"
    },
    { /* tv 38 */
      "NIST P-192",
      "6277101735386680763835789423176059013767194773182842284067",
      "13B9310646EBC93B591746B3F7C64E05DEE08843DE1081C1",
      "E123159C4BBEBD22EA0C4BD813BE5E12B045356A1E79194B"
    },
    { /* tv 39 */
      "NIST P-192",
      "6277101735386680763835789423176059013767194773182842284068",
      "112AF141D33EFB9F2F68821E051E4EA004144A363C4A090A",
      "91F341C403AD6C08D5D3E8D91F7E1F6080EF5F6BBCD4E3E1"
    },
    { /* tv 40 */
      "NIST P-192",
      "6277101735386680763835789423176059013767194773182842284069",
      "1061343F3D456D0ECA013877F8C9E7B28FCCDCDA67EEB8AB",
      "A5F9B355D1594FC86701071C185B79B697E153FDF4D8D6C0"
    },
    { /* tv 41 */
      "NIST P-192",
      "6277101735386680763835789423176059013767194773182842284070",
      "1C995995EB76324F1844F7164D22B652280940370628A2AA",
      "10E89A31C816148CFD60AA9BFF0588414CB3479EE55563FB"
    },
    { /* tv 42 */
      "NIST P-192",
      "6277101735386680763835789423176059013767194773182842284071",
      "AA7C4F9EF99E3E96D1AEDE2BD9238842859BB150D1FE9D85",
      "CDED5C9AB81239D6FE11C9A74D0B7A6A9F14A14DB6EC684F"
    },
    { /* tv 43 */
      "NIST P-192",
      "6277101735386680763835789423176059013767194773182842284072",
      "818A4D308B1CABB74E9E8F2BA8D27C9E1D9D375AB980388F",
      "FE2E55A1DF72783283D6D08344BA831FCF15ABDE893718C6"
    },
    { /* tv 44 */
      "NIST P-192",
      "6277101735386680763835789423176059013767194773182842284073",
      "2FA1F92D1ECCE92014771993CC14899D4B5977883397EDDE",
      "5CC7502108748DEBD8C474A687108CBF0D22757168C7093F"
    },
    { /* tv 45 */
      "NIST P-192",
      "6277101735386680763835789423176059013767194773182842284074",
      "8DA75A1F75DDCD7660F923243060EDCE5DE37F007011FCFD",
      "A834A030979F4CABE7DBF247024C3FE12B48FD069BF6004A"
    },
    { /* tv 46 */
      "NIST P-192",
      "6277101735386680763835789423176059013767194773182842284075",
      "A37ABC6C431F9AC398BF5BD1AA6678320ACE8ECB93D23F2A",
      "7AE4C3513666F7240128FBF5E44256F0F7E083A8EF43970F"
    },
    { /* tv 47 */
      "NIST P-192",
      "6277101735386680763835789423176059013767194773182842284076",
      "10BB8E9840049B183E078D9C300E1605590118EBDD7FF590",
      "CEC9EFF7B8906E84523607C919D89D40CED48DABC331515E"
    },
    { /* tv 48 */
      "NIST P-192",
      "6277101735386680763835789423176059013767194773182842284077",
      "35433907297CC378B0015703374729D7A4FE46647084E4BA",
      "5D9B667B0DECA3CFE15C534F88932B0DDAC764CEE24C41CD"
    },
    { /* tv 49 */
      "NIST P-192",
      "6277101735386680763835789423176059013767194773182842284078",
      "76E32A2557599E6EDCD283201FB2B9AADFD0D359CBB263DA",
      "87D3C81C8D45BADF559D1F012EDE2B600C4ABC99F302FA02"
    },
    { /* tv 50 */
      "NIST P-192",
      "6277101735386680763835789423176059013767194773182842284079",
      "DAFEBF5828783F2AD35534631588A3F629A70FB16982A888",
      "229425F266C25F05B94D8443EBE4796FA6CCE505A3816C54"
    },
    { /* tv 51 */
      "NIST P-192",
      "6277101735386680763835789423176059013767194773182842284080",
      "188DA80EB03090F67CBF20EB43A18800F4FF0AFD82FF1012",
      "F8E6D46A003725879CEFEE1294DB32298C06885EE186B7EE"
    },
    { /* tv 52 */
      "NIST P-224",
      "1",
      "B70E0CBD6BB4BF7F321390B94A03C1D356C21122343280D6115C1D21",
      "BD376388B5F723FB4C22DFE6CD4375A05A07476444D5819985007E34"
    },
    { /* tv 53 */
      "NIST P-224",
      "2",
      "706A46DC76DCB76798E60E6D89474788D16DC18032D268FD1A704FA6",
      "1C2B76A7BC25E7702A704FA986892849FCA629487ACF3709D2E4E8BB"
    },
    { /* tv 54 */
      "NIST P-224",
      "3",
      "DF1B1D66A551D0D31EFF822558B9D2CC75C2180279FE0D08FD896D04",
      "A3F7F03CADD0BE444C0AA56830130DDF77D317344E1AF3591981A925"
    },
    { /* tv 55 */
      "NIST P-224",
      "4",
      "AE99FEEBB5D26945B54892092A8AEE02912930FA41CD114E40447301",
      "0482580A0EC5BC47E88BC8C378632CD196CB3FA058A7114EB03054C9"
    },
    { /* tv 56 */
      "NIST P-224",
      "5",
      "31C49AE75BCE7807CDFF22055D94EE9021FEDBB5AB51C57526F011AA",
      "27E8BFF1745635EC5BA0C9F1C2EDE15414C6507D29FFE37E790A079B"
    },
    { /* tv 57 */
      "NIST P-224",
      "6",
      "1F2483F82572251FCA975FEA40DB821DF8AD82A3C002EE6C57112408",
      "89FAF0CCB750D99B553C574FAD7ECFB0438586EB3952AF5B4B153C7E"
    },
    { /* tv 58 */
      "NIST P-224",
      "7",
      "DB2F6BE630E246A5CF7D99B85194B123D487E2D466B94B24A03C3E28",
      "0F3A30085497F2F611EE2517B163EF8C53B715D18BB4E4808D02B963"
    },
    { /* tv 59 */
      "NIST P-224",
      "8",
      "858E6F9CC6C12C31F5DF124AA77767B05C8BC021BD683D2B55571550",
      "046DCD3EA5C43898C5C5FC4FDAC7DB39C2F02EBEE4E3541D1E78047A"
    },
    { /* tv 60 */
      "NIST P-224",
      "9",
      "2FDCCCFEE720A77EF6CB3BFBB447F9383117E3DAA4A07E36ED15F78D",
      "371732E4F41BF4F7883035E6A79FCEDC0E196EB07B48171697517463"
    },
    { /* tv 61 */
      "NIST P-224",
      "10",
      "AEA9E17A306517EB89152AA7096D2C381EC813C51AA880E7BEE2C0FD",
      "39BB30EAB337E0A521B6CBA1ABE4B2B3A3E524C14A3FE3EB116B655F"
    },
    { /* tv 62 */
      "NIST P-224",
      "11",
      "EF53B6294ACA431F0F3C22DC82EB9050324F1D88D377E716448E507C",
      "20B510004092E96636CFB7E32EFDED8265C266DFB754FA6D6491A6DA"
    },
    { /* tv 63 */
      "NIST P-224",
      "12",
      "6E31EE1DC137F81B056752E4DEAB1443A481033E9B4C93A3044F4F7A",
      "207DDDF0385BFDEAB6E9ACDA8DA06B3BBEF224A93AB1E9E036109D13"
    },
    { /* tv 64 */
      "NIST P-224",
      "13",
      "34E8E17A430E43289793C383FAC9774247B40E9EBD3366981FCFAECA",
      "252819F71C7FB7FBCB159BE337D37D3336D7FEB963724FDFB0ECB767"
    },
    { /* tv 65 */
      "NIST P-224",
      "14",
      "A53640C83DC208603DED83E4ECF758F24C357D7CF48088B2CE01E9FA",
      "D5814CD724199C4A5B974A43685FBF5B8BAC69459C9469BC8F23CCAF"
    },
    { /* tv 66 */
      "NIST P-224",
      "15",
      "BAA4D8635511A7D288AEBEEDD12CE529FF102C91F97F867E21916BF9",
      "979A5F4759F80F4FB4EC2E34F5566D595680A11735E7B61046127989"
    },
    { /* tv 67 */
      "NIST P-224",
      "16",
      "0B6EC4FE1777382404EF679997BA8D1CC5CD8E85349259F590C4C66D",
      "3399D464345906B11B00E363EF429221F2EC720D2F665D7DEAD5B482"
    },
    { /* tv 68 */
      "NIST P-224",
      "17",
      "B8357C3A6CEEF288310E17B8BFEFF9200846CA8C1942497C484403BC",
      "FF149EFA6606A6BD20EF7D1B06BD92F6904639DCE5174DB6CC554A26"
    },
    { /* tv 69 */
      "NIST P-224",
      "18",
      "C9FF61B040874C0568479216824A15EAB1A838A797D189746226E4CC",
      "EA98D60E5FFC9B8FCF999FAB1DF7E7EF7084F20DDB61BB045A6CE002"
    },
    { /* tv 70 */
      "NIST P-224",
      "19",
      "A1E81C04F30CE201C7C9ACE785ED44CC33B455A022F2ACDBC6CAE83C",
      "DCF1F6C3DB09C70ACC25391D492FE25B4A180BABD6CEA356C04719CD"
    },
    { /* tv 71 */
      "NIST P-224",
      "20",
      "FCC7F2B45DF1CD5A3C0C0731CA47A8AF75CFB0347E8354EEFE782455",
      "0D5D7110274CBA7CDEE90E1A8B0D394C376A5573DB6BE0BF2747F530"
    },
    { /* tv 72 */
      "NIST P-224",
      "112233445566778899",
      "61F077C6F62ED802DAD7C2F38F5C67F2CC453601E61BD076BB46179E",
      "2272F9E9F5933E70388EE652513443B5E289DD135DCC0D0299B225E4"
    },
    { /* tv 73 */
      "NIST P-224",
      "112233445566778899112233445566778899",
      "029895F0AF496BFC62B6EF8D8A65C88C613949B03668AAB4F0429E35",
      "3EA6E53F9A841F2019EC24BDE1A75677AA9B5902E61081C01064DE93"
    },
    { /* tv 74 */
      "NIST P-224",
      "6950511619965839450988900688150712778015737983940691968051900319680",
      "AB689930BCAE4A4AA5F5CB085E823E8AE30FD365EB1DA4ABA9CF0379",
      "3345A121BBD233548AF0D210654EB40BAB788A03666419BE6FBD34E7"
    },
    { /* tv 75 */
      "NIST P-224",
      "13479972933410060327035789020509431695094902435494295338570602119423",
      "BDB6A8817C1F89DA1C2F3DD8E97FEB4494F2ED302A4CE2BC7F5F4025",
      "4C7020D57C00411889462D77A5438BB4E97D177700BF7243A07F1680"
    },
    { /* tv 76 */
      "NIST P-224",
      "13479971751745682581351455311314208093898607229429740618390390702079",
      "D58B61AA41C32DD5EBA462647DBA75C5D67C83606C0AF2BD928446A9",
      "D24BA6A837BE0460DD107AE77725696D211446C5609B4595976B16BD"
    },
    { /* tv 77 */
      "NIST P-224",
      "13479972931865328106486971546324465392952975980343228160962702868479",
      "DC9FA77978A005510980E929A1485F63716DF695D7A0C18BB518DF03",
      "EDE2B016F2DDFFC2A8C015B134928275CE09E5661B7AB14CE0D1D403"
    },
    { /* tv 78 */
      "NIST P-224",
      "11795773708834916026404142434151065506931607341523388140225443265536",
      "499D8B2829CFB879C901F7D85D357045EDAB55028824D0F05BA279BA",
      "BF929537B06E4015919639D94F57838FA33FC3D952598DCDBB44D638"
    },
    { /* tv 79 */
      "NIST P-224",
      "784254593043826236572847595991346435467177662189391577090",
      "8246C999137186632C5F9EDDF3B1B0E1764C5E8BD0E0D8A554B9CB77",
      "E80ED8660BC1CB17AC7D845BE40A7A022D3306F116AE9F81FEA65947"
    },
    { /* tv 80 */
      "NIST P-224",
      "13479767645505654746623887797783387853576174193480695826442858012671",
      "6670C20AFCCEAEA672C97F75E2E9DD5C8460E54BB38538EBB4BD30EB",
      "F280D8008D07A4CAF54271F993527D46FF3FF46FD1190A3F1FAA4F74"
    },
    { /* tv 81 */
      "NIST P-224",
      "205688069665150753842126177372015544874550518966168735589597183",
      "000ECA934247425CFD949B795CB5CE1EFF401550386E28D1A4C5A8EB",
      "D4C01040DBA19628931BC8855370317C722CBD9CA6156985F1C2E9CE"
    },
    { /* tv 82 */
      "NIST P-224",
      "13479966930919337728895168462090683249159702977113823384618282123295",
      "EF353BF5C73CD551B96D596FBC9A67F16D61DD9FE56AF19DE1FBA9CD",
      "21771B9CDCE3E8430C09B3838BE70B48C21E15BC09EE1F2D7945B91F"
    },
    { /* tv 83 */
      "NIST P-224",
      "50210731791415612487756441341851895584393717453129007497216",
      "4036052A3091EB481046AD3289C95D3AC905CA0023DE2C03ECD451CF",
      "D768165A38A2B96F812586A9D59D4136035D9C853A5BF2E1C86A4993"
    },
    { /* tv 84 */
      "NIST P-224",
      "26959946667150639794667015087019625940457807714424391721682722368041",
      "FCC7F2B45DF1CD5A3C0C0731CA47A8AF75CFB0347E8354EEFE782455",
      "F2A28EEFD8B345832116F1E574F2C6B2C895AA8C24941F40D8B80AD1"
    },
    { /* tv 85 */
      "NIST P-224",
      "26959946667150639794667015087019625940457807714424391721682722368042",
      "A1E81C04F30CE201C7C9ACE785ED44CC33B455A022F2ACDBC6CAE83C",
      "230E093C24F638F533DAC6E2B6D01DA3B5E7F45429315CA93FB8E634"
    },
    { /* tv 86 */
      "NIST P-224",
      "26959946667150639794667015087019625940457807714424391721682722368043",
      "C9FF61B040874C0568479216824A15EAB1A838A797D189746226E4CC",
      "156729F1A003647030666054E208180F8F7B0DF2249E44FBA5931FFF"
    },
    { /* tv 87 */
      "NIST P-224",
      "26959946667150639794667015087019625940457807714424391721682722368044",
      "B8357C3A6CEEF288310E17B8BFEFF9200846CA8C1942497C484403BC",
      "00EB610599F95942DF1082E4F9426D086FB9C6231AE8B24933AAB5DB"
    },
    { /* tv 88 */
      "NIST P-224",
      "26959946667150639794667015087019625940457807714424391721682722368045",
      "0B6EC4FE1777382404EF679997BA8D1CC5CD8E85349259F590C4C66D",
      "CC662B9BCBA6F94EE4FF1C9C10BD6DDD0D138DF2D099A282152A4B7F"
    },
    { /* tv 89 */
      "NIST P-224",
      "26959946667150639794667015087019625940457807714424391721682722368046",
      "BAA4D8635511A7D288AEBEEDD12CE529FF102C91F97F867E21916BF9",
      "6865A0B8A607F0B04B13D1CB0AA992A5A97F5EE8CA1849EFB9ED8678"
    },
    { /* tv 90 */
      "NIST P-224",
      "26959946667150639794667015087019625940457807714424391721682722368047",
      "A53640C83DC208603DED83E4ECF758F24C357D7CF48088B2CE01E9FA",
      "2A7EB328DBE663B5A468B5BC97A040A3745396BA636B964370DC3352"
    },
    { /* tv 91 */
      "NIST P-224",
      "26959946667150639794667015087019625940457807714424391721682722368048",
      "34E8E17A430E43289793C383FAC9774247B40E9EBD3366981FCFAECA",
      "DAD7E608E380480434EA641CC82C82CBC92801469C8DB0204F13489A"
    },
    { /* tv 92 */
      "NIST P-224",
      "26959946667150639794667015087019625940457807714424391721682722368049",
      "6E31EE1DC137F81B056752E4DEAB1443A481033E9B4C93A3044F4F7A",
      "DF82220FC7A4021549165325725F94C3410DDB56C54E161FC9EF62EE"
    },
    { /* tv 93 */
      "NIST P-224",
      "26959946667150639794667015087019625940457807714424391721682722368050",
      "EF53B6294ACA431F0F3C22DC82EB9050324F1D88D377E716448E507C",
      "DF4AEFFFBF6D1699C930481CD102127C9A3D992048AB05929B6E5927"
    },
    { /* tv 94 */
      "NIST P-224",
      "26959946667150639794667015087019625940457807714424391721682722368051",
      "AEA9E17A306517EB89152AA7096D2C381EC813C51AA880E7BEE2C0FD",
      "C644CF154CC81F5ADE49345E541B4D4B5C1ADB3EB5C01C14EE949AA2"
    },
    { /* tv 95 */
      "NIST P-224",
      "26959946667150639794667015087019625940457807714424391721682722368052",
      "2FDCCCFEE720A77EF6CB3BFBB447F9383117E3DAA4A07E36ED15F78D",
      "C8E8CD1B0BE40B0877CFCA1958603122F1E6914F84B7E8E968AE8B9E"
    },
    { /* tv 96 */
      "NIST P-224",
      "26959946667150639794667015087019625940457807714424391721682722368053",
      "858E6F9CC6C12C31F5DF124AA77767B05C8BC021BD683D2B55571550",
      "FB9232C15A3BC7673A3A03B0253824C53D0FD1411B1CABE2E187FB87"
    },
    { /* tv 97 */
      "NIST P-224",
      "26959946667150639794667015087019625940457807714424391721682722368054",
      "DB2F6BE630E246A5CF7D99B85194B123D487E2D466B94B24A03C3E28",
      "F0C5CFF7AB680D09EE11DAE84E9C1072AC48EA2E744B1B7F72FD469E"
    },
    { /* tv 98 */
      "NIST P-224",
      "26959946667150639794667015087019625940457807714424391721682722368055",
      "1F2483F82572251FCA975FEA40DB821DF8AD82A3C002EE6C57112408",
      "76050F3348AF2664AAC3A8B05281304EBC7A7914C6AD50A4B4EAC383"
    },
    { /* tv 99 */
      "NIST P-224",
      "26959946667150639794667015087019625940457807714424391721682722368056",
      "31C49AE75BCE7807CDFF22055D94EE9021FEDBB5AB51C57526F011AA",
      "D817400E8BA9CA13A45F360E3D121EAAEB39AF82D6001C8186F5F866"
    },
    { /* tv 100 */
      "NIST P-224",
      "26959946667150639794667015087019625940457807714424391721682722368057",
      "AE99FEEBB5D26945B54892092A8AEE02912930FA41CD114E40447301",
      "FB7DA7F5F13A43B81774373C879CD32D6934C05FA758EEB14FCFAB38"
    },
    { /* tv 101 */
      "NIST P-224",
      "26959946667150639794667015087019625940457807714424391721682722368058",
      "DF1B1D66A551D0D31EFF822558B9D2CC75C2180279FE0D08FD896D04",
      "5C080FC3522F41BBB3F55A97CFECF21F882CE8CBB1E50CA6E67E56DC"
    },
    { /* tv 102 */
      "NIST P-224",
      "26959946667150639794667015087019625940457807714424391721682722368059",
      "706A46DC76DCB76798E60E6D89474788D16DC18032D268FD1A704FA6",
      "E3D4895843DA188FD58FB0567976D7B50359D6B78530C8F62D1B1746"
    },
    { /* tv 103 */
      "NIST P-224",
      "26959946667150639794667015087019625940457807714424391721682722368060",
      "B70E0CBD6BB4BF7F321390B94A03C1D356C21122343280D6115C1D21",
      "42C89C774A08DC04B3DD201932BC8A5EA5F8B89BBB2A7E667AFF81CD"
    },
    { /* tv 104 */
      "NIST P-256",
      "1",
      "6B17D1F2E12C4247F8BCE6E563A440F277037D812DEB33A0F4A13945D898C296",
      "4FE342E2FE1A7F9B8EE7EB4A7C0F9E162BCE33576B315ECECBB6406837BF51F5"
    },
    { /* tv 105 */
      "NIST P-256",
      "2",
      "7CF27B188D034F7E8A52380304B51AC3C08969E277F21B35A60B48FC47669978",
      "07775510DB8ED040293D9AC69F7430DBBA7DADE63CE982299E04B79D227873D1"
    },
    { /* tv 106 */
      "NIST P-256",
      "3",
      "5ECBE4D1A6330A44C8F7EF951D4BF165E6C6B721EFADA985FB41661BC6E7FD6C",
      "8734640C4998FF7E374B06CE1A64A2ECD82AB036384FB83D9A79B127A27D5032"
    },
    { /* tv 107 */
      "NIST P-256",
      "4",
      "E2534A3532D08FBBA02DDE659EE62BD0031FE2DB785596EF509302446B030852",
      "E0F1575A4C633CC719DFEE5FDA862D764EFC96C3F30EE0055C42C23F184ED8C6"
    },
    { /* tv 108 */
      "NIST P-256",
      "5",
      "51590B7A515140D2D784C85608668FDFEF8C82FD1F5BE52421554A0DC3D033ED",
      "E0C17DA8904A727D8AE1BF36BF8A79260D012F00D4D80888D1D0BB44FDA16DA4"
    },
    { /* tv 109 */
      "NIST P-256",
      "6",
      "B01A172A76A4602C92D3242CB897DDE3024C740DEBB215B4C6B0AAE93C2291A9",
      "E85C10743237DAD56FEC0E2DFBA703791C00F7701C7E16BDFD7C48538FC77FE2"
    },
    { /* tv 110 */
      "NIST P-256",
      "7",
      "8E533B6FA0BF7B4625BB30667C01FB607EF9F8B8A80FEF5B300628703187B2A3",
      "73EB1DBDE03318366D069F83A6F5900053C73633CB041B21C55E1A86C1F400B4"
    },
    { /* tv 111 */
      "NIST P-256",
      "8",
      "62D9779DBEE9B0534042742D3AB54CADC1D238980FCE97DBB4DD9DC1DB6FB393",
      "AD5ACCBD91E9D8244FF15D771167CEE0A2ED51F6BBE76A78DA540A6A0F09957E"
    },
    { /* tv 112 */
      "NIST P-256",
      "9",
      "EA68D7B6FEDF0B71878938D51D71F8729E0ACB8C2C6DF8B3D79E8A4B90949EE0",
      "2A2744C972C9FCE787014A964A8EA0C84D714FEAA4DE823FE85A224A4DD048FA"
    },
    { /* tv 113 */
      "NIST P-256",
      "10",
      "CEF66D6B2A3A993E591214D1EA223FB545CA6C471C48306E4C36069404C5723F",
      "878662A229AAAE906E123CDD9D3B4C10590DED29FE751EEECA34BBAA44AF0773"
    },
    { /* tv 114 */
      "NIST P-256",
      "11",
      "3ED113B7883B4C590638379DB0C21CDA16742ED0255048BF433391D374BC21D1",
      "9099209ACCC4C8A224C843AFA4F4C68A090D04DA5E9889DAE2F8EEFCE82A3740"
    },
    { /* tv 115 */
      "NIST P-256",
      "12",
      "741DD5BDA817D95E4626537320E5D55179983028B2F82C99D500C5EE8624E3C4",
      "0770B46A9C385FDC567383554887B1548EEB912C35BA5CA71995FF22CD4481D3"
    },
    { /* tv 116 */
      "NIST P-256",
      "13",
      "177C837AE0AC495A61805DF2D85EE2FC792E284B65EAD58A98E15D9D46072C01",
      "63BB58CD4EBEA558A24091ADB40F4E7226EE14C3A1FB4DF39C43BBE2EFC7BFD8"
    },
    { /* tv 117 */
      "NIST P-256",
      "14",
      "54E77A001C3862B97A76647F4336DF3CF126ACBE7A069C5E5709277324D2920B",
      "F599F1BB29F4317542121F8C05A2E7C37171EA77735090081BA7C82F60D0B375"
    },
    { /* tv 118 */
      "NIST P-256",
      "15",
      "F0454DC6971ABAE7ADFB378999888265AE03AF92DE3A0EF163668C63E59B9D5F",
      "B5B93EE3592E2D1F4E6594E51F9643E62A3B21CE75B5FA3F47E59CDE0D034F36"
    },
    { /* tv 119 */
      "NIST P-256",
      "16",
      "76A94D138A6B41858B821C629836315FCD28392EFF6CA038A5EB4787E1277C6E",
      "A985FE61341F260E6CB0A1B5E11E87208599A0040FC78BAA0E9DDD724B8C5110"
    },
    { /* tv 120 */
      "NIST P-256",
      "17",
      "47776904C0F1CC3A9C0984B66F75301A5FA68678F0D64AF8BA1ABCE34738A73E",
      "AA005EE6B5B957286231856577648E8381B2804428D5733F32F787FF71F1FCDC"
    },
    { /* tv 121 */
      "NIST P-256",
      "18",
      "1057E0AB5780F470DEFC9378D1C7C87437BB4C6F9EA55C63D936266DBD781FDA",
      "F6F1645A15CBE5DC9FA9B7DFD96EE5A7DCC11B5C5EF4F1F78D83B3393C6A45A2"
    },
    { /* tv 122 */
      "NIST P-256",
      "19",
      "CB6D2861102C0C25CE39B7C17108C507782C452257884895C1FC7B74AB03ED83",
      "58D7614B24D9EF515C35E7100D6D6CE4A496716E30FA3E03E39150752BCECDAA"
    },
    { /* tv 123 */
      "NIST P-256",
      "20",
      "83A01A9378395BAB9BCD6A0AD03CC56D56E6B19250465A94A234DC4C6B28DA9A",
      "76E49B6DE2F73234AE6A5EB9D612B75C9F2202BB6923F54FF8240AAA86F640B8"
    },
    { /* tv 124 */
      "NIST P-256",
      "112233445566778899",
      "339150844EC15234807FE862A86BE77977DBFB3AE3D96F4C22795513AEAAB82F",
      "B1C14DDFDC8EC1B2583F51E85A5EB3A155840F2034730E9B5ADA38B674336A21"
    },
    { /* tv 125 */
      "NIST P-256",
      "112233445566778899112233445566778899",
      "1B7E046A076CC25E6D7FA5003F6729F665CC3241B5ADAB12B498CD32F2803264",
      "BFEA79BE2B666B073DB69A2A241ADAB0738FE9D2DD28B5604EB8C8CF097C457B"
    },
    { /* tv 126 */
      "NIST P-256",
      "29852220098221261079183923314599206100666902414330245206392788703677"
      "545185283",
      "9EACE8F4B071E677C5350B02F2BB2B384AAE89D58AA72CA97A170572E0FB222F",
      "1BBDAEC2430B09B93F7CB08678636CE12EAAFD58390699B5FD2F6E1188FC2A78"
    },
    { /* tv 127 */
      "NIST P-256",
      "57896042899961394862005778464643882389978449576758748073725983489954"
      "366354431",
      "878F22CC6DB6048D2B767268F22FFAD8E56AB8E2DC615F7BD89F1E350500DD8D",
      "714A5D7BB901C9C5853400D12341A892EF45D87FC553786756C4F0C9391D763E"
    },
    { /* tv 128 */
      "NIST P-256",
      "17668453929457101515018891057290498829976600048248489159554196603666"
      "36031",
      "659A379625AB122F2512B8DADA02C6348D53B54452DFF67AC7ACE4E8856295CA",
      "49D81AB97B648464D0B4A288BD7818FAB41A16426E943527C4FED8736C53D0F6"
    },
    { /* tv 129 */
      "NIST P-256",
      "28948025760307534517734791687894775804466072615242963443097661355606"
      "862201087",
      "CBCEAAA8A4DD44BBCE58E8DB7740A5510EC2CB7EA8DA8D8F036B3FB04CDA4DE4",
      "4BD7AA301A80D7F59FD983FEDBE59BB7B2863FE46494935E3745B360E32332FA"
    },
    { /* tv 130 */
      "NIST P-256",
      "11307821046087054894481169596029064497322922462583843642447709583464"
      "5696384",
      "F0C4A0576154FF3A33A3460D42EAED806E854DFA37125221D37935124BA462A4",
      "5B392FA964434D29EEC6C9DBC261CF116796864AA2FAADB984A2DF38D1AEF7A3"
    },
    { /* tv 131 */
      "NIST P-256",
      "12078056106883488161242983286051341125085761470677906721917479268909"
      "056",
      "5E6C8524B6369530B12C62D31EC53E0288173BD662BDF680B53A41ECBCAD00CC",
      "447FE742C2BFEF4D0DB14B5B83A2682309B5618E0064A94804E9282179FE089F"
    },
    { /* tv 132 */
      "NIST P-256",
      "57782969857385448082319957860328652998540760998293976083718804450708"
      "503920639",
      "03792E541BC209076A3D7920A915021ECD396A6EB5C3960024BE5575F3223484",
      "FC774AE092403101563B712F68170312304F20C80B40C06282063DB25F268DE4"
    },
    { /* tv 133 */
      "NIST P-256",
      "57896017119460046759583662757090100341435943767777707906455551163257"
      "755533312",
      "2379FF85AB693CDF901D6CE6F2473F39C04A2FE3DCD842CE7AAB0E002095BCF8",
      "F8B476530A634589D5129E46F322B02FBC610A703D80875EE70D7CE1877436A1"
    },
    { /* tv 134 */
      "NIST P-256",
      "45231284837428728468128217101764741272643368423846421299930586483716"
      "0993279",
      "C1E4072C529BF2F44DA769EFC934472848003B3AF2C0F5AA8F8DDBD53E12ED7C",
      "39A6EE77812BB37E8079CD01ED649D3830FCA46F718C1D3993E4A591824ABCDB"
    },
    { /* tv 135 */
      "NIST P-256",
      "90457133917406513429363440794605400077474605586691772987667636755846"
      "9746684",
      "34DFBC09404C21E250A9B40FA8772897AC63A094877DB65862B61BD1507B34F3",
      "CF6F8A876C6F99CEAEC87148F18C7E1E0DA6E165FFC8ED82ABB65955215F77D3"
    },
    { /* tv 136 */
      "NIST P-256",
      "11579208921035624876269744694940757352999695522413576034242225906106"
      "8512044349",
      "83A01A9378395BAB9BCD6A0AD03CC56D56E6B19250465A94A234DC4C6B28DA9A",
      "891B64911D08CDCC5195A14629ED48A360DDFD4596DC0AB007DBF5557909BF47"
    },
    { /* tv 137 */
      "NIST P-256",
      "11579208921035624876269744694940757352999695522413576034242225906106"
      "8512044350",
      "CB6D2861102C0C25CE39B7C17108C507782C452257884895C1FC7B74AB03ED83",
      "A7289EB3DB2610AFA3CA18EFF292931B5B698E92CF05C1FC1C6EAF8AD4313255"
    },
    { /* tv 138 */
      "NIST P-256",
      "11579208921035624876269744694940757352999695522413576034242225906106"
      "8512044351",
      "1057E0AB5780F470DEFC9378D1C7C87437BB4C6F9EA55C63D936266DBD781FDA",
      "090E9BA4EA341A246056482026911A58233EE4A4A10B0E08727C4CC6C395BA5D"
    },
    { /* tv 139 */
      "NIST P-256",
      "11579208921035624876269744694940757352999695522413576034242225906106"
      "8512044352",
      "47776904C0F1CC3A9C0984B66F75301A5FA68678F0D64AF8BA1ABCE34738A73E",
      "55FFA1184A46A8D89DCE7A9A889B717C7E4D7FBCD72A8CC0CD0878008E0E0323"
    },
    { /* tv 140 */
      "NIST P-256",
      "11579208921035624876269744694940757352999695522413576034242225906106"
      "8512044353",
      "76A94D138A6B41858B821C629836315FCD28392EFF6CA038A5EB4787E1277C6E",
      "567A019DCBE0D9F2934F5E4A1EE178DF7A665FFCF0387455F162228DB473AEEF"
    },
    { /* tv 141 */
      "NIST P-256",
      "11579208921035624876269744694940757352999695522413576034242225906106"
      "8512044354",
      "F0454DC6971ABAE7ADFB378999888265AE03AF92DE3A0EF163668C63E59B9D5F",
      "4A46C11BA6D1D2E1B19A6B1AE069BC19D5C4DE328A4A05C0B81A6321F2FCB0C9"
    },
    { /* tv 142 */
      "NIST P-256",
      "11579208921035624876269744694940757352999695522413576034242225906106"
      "8512044355",
      "54E77A001C3862B97A76647F4336DF3CF126ACBE7A069C5E5709277324D2920B",
      "0A660E43D60BCE8BBDEDE073FA5D183C8E8E15898CAF6FF7E45837D09F2F4C8A"
    },
    { /* tv 143 */
      "NIST P-256",
      "11579208921035624876269744694940757352999695522413576034242225906106"
      "8512044356",
      "177C837AE0AC495A61805DF2D85EE2FC792E284B65EAD58A98E15D9D46072C01",
      "9C44A731B1415AA85DBF6E524BF0B18DD911EB3D5E04B20C63BC441D10384027"
    },
    { /* tv 144 */
      "NIST P-256",
      "11579208921035624876269744694940757352999695522413576034242225906106"
      "8512044357",
      "741DD5BDA817D95E4626537320E5D55179983028B2F82C99D500C5EE8624E3C4",
      "F88F4B9463C7A024A98C7CAAB7784EAB71146ED4CA45A358E66A00DD32BB7E2C"
    },
    { /* tv 145 */
      "NIST P-256",
      "11579208921035624876269744694940757352999695522413576034242225906106"
      "8512044358",
      "3ED113B7883B4C590638379DB0C21CDA16742ED0255048BF433391D374BC21D1",
      "6F66DF64333B375EDB37BC505B0B3975F6F2FB26A16776251D07110317D5C8BF"
    },
    { /* tv 146 */
      "NIST P-256",
      "11579208921035624876269744694940757352999695522413576034242225906106"
      "8512044359",
      "CEF66D6B2A3A993E591214D1EA223FB545CA6C471C48306E4C36069404C5723F",
      "78799D5CD655517091EDC32262C4B3EFA6F212D7018AE11135CB4455BB50F88C"
    },
    { /* tv 147 */
      "NIST P-256",
      "11579208921035624876269744694940757352999695522413576034242225906106"
      "8512044360",
      "EA68D7B6FEDF0B71878938D51D71F8729E0ACB8C2C6DF8B3D79E8A4B90949EE0",
      "D5D8BB358D36031978FEB569B5715F37B28EB0165B217DC017A5DDB5B22FB705"
    },
    { /* tv 148 */
      "NIST P-256",
      "11579208921035624876269744694940757352999695522413576034242225906106"
      "8512044361",
      "62D9779DBEE9B0534042742D3AB54CADC1D238980FCE97DBB4DD9DC1DB6FB393",
      "52A533416E1627DCB00EA288EE98311F5D12AE0A4418958725ABF595F0F66A81"
    },
    { /* tv 149 */
      "NIST P-256",
      "11579208921035624876269744694940757352999695522413576034242225906106"
      "8512044362",
      "8E533B6FA0BF7B4625BB30667C01FB607EF9F8B8A80FEF5B300628703187B2A3",
      "8C14E2411FCCE7CA92F9607C590A6FFFAC38C9CD34FBE4DE3AA1E5793E0BFF4B"
    },
    { /* tv 150 */
      "NIST P-256",
      "11579208921035624876269744694940757352999695522413576034242225906106"
      "8512044363",
      "B01A172A76A4602C92D3242CB897DDE3024C740DEBB215B4C6B0AAE93C2291A9",
      "17A3EF8ACDC8252B9013F1D20458FC86E3FF0890E381E9420283B7AC7038801D"
    },
    { /* tv 151 */
      "NIST P-256",
      "11579208921035624876269744694940757352999695522413576034242225906106"
      "8512044364",
      "51590B7A515140D2D784C85608668FDFEF8C82FD1F5BE52421554A0DC3D033ED",
      "1F3E82566FB58D83751E40C9407586D9F2FED1002B27F7772E2F44BB025E925B"
    },
    { /* tv 152 */
      "NIST P-256",
      "11579208921035624876269744694940757352999695522413576034242225906106"
      "8512044365",
      "E2534A3532D08FBBA02DDE659EE62BD0031FE2DB785596EF509302446B030852",
      "1F0EA8A4B39CC339E62011A02579D289B103693D0CF11FFAA3BD3DC0E7B12739"
    },
    { /* tv 153 */
      "NIST P-256",
      "11579208921035624876269744694940757352999695522413576034242225906106"
      "8512044366",
      "5ECBE4D1A6330A44C8F7EF951D4BF165E6C6B721EFADA985FB41661BC6E7FD6C",
      "78CB9BF2B6670082C8B4F931E59B5D1327D54FCAC7B047C265864ED85D82AFCD"
    },
    { /* tv 154 */
      "NIST P-256",
      "11579208921035624876269744694940757352999695522413576034242225906106"
      "8512044367",
      "7CF27B188D034F7E8A52380304B51AC3C08969E277F21B35A60B48FC47669978",
      "F888AAEE24712FC0D6C26539608BCF244582521AC3167DD661FB4862DD878C2E"
    },
    { /* tv 155 */
      "NIST P-256",
      "11579208921035624876269744694940757352999695522413576034242225906106"
      "8512044368",
      "6B17D1F2E12C4247F8BCE6E563A440F277037D812DEB33A0F4A13945D898C296",
      "B01CBD1C01E58065711814B583F061E9D431CCA994CEA1313449BF97C840AE0A"
    },
    { /* tv 156 */
      "NIST P-384",
      "1",
      "AA87CA22BE8B05378EB1C71EF320AD746E1D3B628BA79B9859F741E082542A385502"
      "F25DBF55296C3A545E3872760AB7",
      "3617DE4A96262C6F5D9E98BF9292DC29F8F41DBD289A147CE9DA3113B5F0B8C00A60"
      "B1CE1D7E819D7A431D7C90EA0E5F"
    },
    { /* tv 157 */
      "NIST P-384",
      "2",
      "08D999057BA3D2D969260045C55B97F089025959A6F434D651D207D19FB96E9E4FE0"
      "E86EBE0E64F85B96A9C75295DF61",
      "8E80F1FA5B1B3CEDB7BFE8DFFD6DBA74B275D875BC6CC43E904E505F256AB4255FFD"
      "43E94D39E22D61501E700A940E80"
    },
    { /* tv 158 */
      "NIST P-384",
      "3",
      "077A41D4606FFA1464793C7E5FDC7D98CB9D3910202DCD06BEA4F240D3566DA6B408"
      "BBAE5026580D02D7E5C70500C831",
      "C995F7CA0B0C42837D0BBE9602A9FC998520B41C85115AA5F7684C0EDC111EACC24A"
      "BD6BE4B5D298B65F28600A2F1DF1"
    },
    { /* tv 159 */
      "NIST P-384",
      "4",
      "138251CD52AC9298C1C8AAD977321DEB97E709BD0B4CA0ACA55DC8AD51DCFC9D1589"
      "A1597E3A5120E1EFD631C63E1835",
      "CACAE29869A62E1631E8A28181AB56616DC45D918ABC09F3AB0E63CF792AA4DCED73"
      "87BE37BBA569549F1C02B270ED67"
    },
    { /* tv 160 */
      "NIST P-384",
      "5",
      "11DE24A2C251C777573CAC5EA025E467F208E51DBFF98FC54F6661CBE56583B03788"
      "2F4A1CA297E60ABCDBC3836D84BC",
      "8FA696C77440F92D0F5837E90A00E7C5284B447754D5DEE88C986533B6901AEB3177"
      "686D0AE8FB33184414ABE6C1713A"
    },
    { /* tv 161 */
      "NIST P-384",
      "6",
      "627BE1ACD064D2B2226FE0D26F2D15D3C33EBCBB7F0F5DA51CBD41F2625738302131"
      "7D7202FF30E50937F0854E35C5DF",
      "09766A4CB3F8B1C21BE6DDA6C14F1575B2C95352644F774C99864F613715441604C4"
      "5B8D84E165311733A408D3F0F934"
    },
    { /* tv 162 */
      "NIST P-384",
      "7",
      "283C1D7365CE4788F29F8EBF234EDFFEAD6FE997FBEA5FFA2D58CC9DFA7B1C508B05"
      "526F55B9EBB2040F05B48FB6D0E1",
      "9475C99061E41B88BA52EFDB8C1690471A61D867ED799729D9C92CD01DBD225630D8"
      "4EDE32A78F9E64664CDAC512EF8C"
    },
    { /* tv 163 */
      "NIST P-384",
      "8",
      "1692778EA596E0BE75114297A6FA383445BF227FBE58190A900C3C73256F11FB5A32"
      "58D6F403D5ECE6E9B269D822C87D",
      "DCD2365700D4106A835388BA3DB8FD0E22554ADC6D521CD4BD1C30C2EC0EEC196BAD"
      "E1E9CDD1708D6F6ABFA4022B0AD2"
    },
    { /* tv 164 */
      "NIST P-384",
      "9",
      "8F0A39A4049BCB3EF1BF29B8B025B78F2216F7291E6FD3BAC6CB1EE285FB6E21C388"
      "528BFEE2B9535C55E4461079118B",
      "62C77E1438B601D6452C4A5322C3A9799A9B3D7CA3C400C6B7678854AED9B3029E74"
      "3EFEDFD51B68262DA4F9AC664AF8"
    },
    { /* tv 165 */
      "NIST P-384",
      "10",
      "A669C5563BD67EEC678D29D6EF4FDE864F372D90B79B9E88931D5C29291238CCED8E"
      "85AB507BF91AA9CB2D13186658FB",
      "A988B72AE7C1279F22D9083DB5F0ECDDF70119550C183C31C502DF78C3B705A8296D"
      "8195248288D997784F6AB73A21DD"
    },
    { /* tv 166 */
      "NIST P-384",
      "11",
      "099056E27DA7B998DA1EEEC2904816C57FE935ED5837C37456C9FD14892D3F8C4749"
      "B66E3AFB81D626356F3B55B4DDD8",
      "2E4C0C234E30AB96688505544AC5E0396FC4EED8DFC363FD43FF93F41B52A3255466"
      "D51263AAFF357D5DBA8138C5E0BB"
    },
    { /* tv 167 */
      "NIST P-384",
      "12",
      "952A7A349BD49289AB3AC421DCF683D08C2ED5E41F6D0E21648AF2691A481406DA4A"
      "5E22DA817CB466DA2EA77D2A7022",
      "A0320FAF84B5BC0563052DEAE6F66F2E09FB8036CE18A0EBB9028B096196B50D031A"
      "A64589743E229EF6BACCE21BD16E"
    },
    { /* tv 168 */
      "NIST P-384",
      "13",
      "A567BA97B67AEA5BAFDAF5002FFCC6AB9632BFF9F01F873F6267BCD1F0F11C139EE5"
      "F441ABD99F1BAAF1CA1E3B5CBCE7",
      "DE1B38B3989F3318644E4147AF164ECC5185595046932EC086329BE057857D66776B"
      "CB8272218A7D6423A12736F429CC"
    },
    { /* tv 169 */
      "NIST P-384",
      "14",
      "E8C8F94D44FBC2396BBEAC481B89D2B0877B1DFFD23E7DC95DE541EB651CCA2C41AB"
      "A24DBC02DE6637209ACCF0F59EA0",
      "891AE44356FC8AE0932BCBF6DE52C8A933B86191E7728D79C8319413A09D0F48FC46"
      "8BA05509DE22D7EE5C9E1B67B888"
    },
    { /* tv 170 */
      "NIST P-384",
      "15",
      "B3D13FC8B32B01058CC15C11D813525522A94156FFF01C205B21F9F7DA7C4E9CA849"
      "557A10B6383B4B88701A9606860B",
      "152919E7DF9162A61B049B2536164B1BEEBAC4A11D749AF484D1114373DFBFD9838D"
      "24F8B284AF50985D588D33F7BD62"
    },
    { /* tv 171 */
      "NIST P-384",
      "16",
      "D5D89C3B5282369C5FBD88E2B231511A6B80DFF0E5152CF6A464FA9428A8583BAC8E"
      "BC773D157811A462B892401DAFCF",
      "D815229DE12906D241816D5E9A9448F1D41D4FC40E2A3BDB9CABA57E440A7ABAD121"
      "0CB8F49BF2236822B755EBAB3673"
    },
    { /* tv 172 */
      "NIST P-384",
      "17",
      "4099952208B4889600A5EBBCB13E1A32692BEFB0733B41E6DCC614E42E5805F81701"
      "2A991AF1F486CAF3A9ADD9FFCC03",
      "5ECF94777833059839474594AF603598163AD3F8008AD0CD9B797D277F2388B304DA"
      "4D2FAA9680ECFA650EF5E23B09A0"
    },
    { /* tv 173 */
      "NIST P-384",
      "18",
      "DFB1FE3A40F7AC9B64C41D39360A7423828B97CB088A4903315E402A7089FA0F8B6C"
      "2355169CC9C99DFB44692A9B93DD",
      "453ACA1243B5EC6B423A68A25587E1613A634C1C42D2EE7E6C57F449A1C91DC89168"
      "B7036EC0A7F37A366185233EC522"
    },
    { /* tv 174 */
      "NIST P-384",
      "19",
      "8D481DAB912BC8AB16858A211D750B77E07DBECCA86CD9B012390B430467AABF59C8"
      "651060801C0E9599E68713F5D41B",
      "A1592FF0121460857BE99F2A60669050B2291B68A1039AA0594B32FD7ADC0E8C11FF"
      "BA5608004E646995B07E75E52245"
    },
    { /* tv 175 */
      "NIST P-384",
      "20",
      "605508EC02C534BCEEE9484C86086D2139849E2B11C1A9CA1E2808DEC2EAF161AC8A"
      "105D70D4F85C50599BE5800A623F",
      "5158EE87962AC6B81F00A103B8543A07381B7639A3A65F1353AEF11B733106DDE92E"
      "99B78DE367B48E238C38DAD8EEDD"
    },
    { /* tv 176 */
      "NIST P-384",
      "112233445566778899",
      "A499EFE48839BC3ABCD1C5CEDBDD51904F9514DB44F4686DB918983B0C9DC3AEE05A"
      "88B72433E9515F91A329F5F4FA60",
      "3B7CA28EF31F809C2F1BA24AAED847D0F8B406A4B8968542DE139DB5828CA410E615"
      "D1182E25B91B1131E230B727D36A"
    },
    { /* tv 177 */
      "NIST P-384",
      "112233445566778899112233445566778899",
      "90A0B1CAC601676B083F21E07BC7090A3390FE1B9C7F61D842D27FA315FB38D83667"
      "A11A71438773E483F2A114836B24",
      "3197D3C6123F0D6CD65D5F0DE106FEF36656CB16DC7CD1A6817EB1D51510135A8F49"
      "2F72665CFD1053F75ED03A7D04C9"
    },
    { /* tv 178 */
      "NIST P-384",
      "10158184112867540819754776755819761756724522948540419979637868435924"
      "061464745859402573149498125806098880003248619520",
      "F2A066BD332DC59BBC3D01DA1B124C687D8BB44611186422DE94C1DA4ECF150E664D"
      "353CCDB5CB2652685F8EB4D2CD49",
      "D6ED0BF75FDD8E53D87765FA746835B673881D6D1907163A2C43990D75B454294F94"
      "2EC571AD5AAE1806CAF2BB8E9A4A"
    },
    { /* tv 179 */
      "NIST P-384",
      "98505015511059910282450526050569921398100949089127992541158476838813"
      "57749738726091734403950439157209401153690566655",
      "5C7F9845D1C4AA44747F9137B6F9C39B36B26B8A62E8AF97290434D5F3B214F5A013"
      "1550ADB19058DC4C8780C4165C4A",
      "712F7FCCC86F647E70DB8798228CB16344AF3D00B139B6F8502939C2A965AF0EB4E3"
      "9E2E16AB8F597B8D5630A50C9D85"
    },
    { /* tv 180 */
      "NIST P-384",
      "98505027234057470973172711947633104824627514551856996305716616579463"
      "08788426092983270628740691202018691293898608608",
      "DD5838F7EC3B8ACF1BECFD746F8B668C577107E93548ED93ED0D254C112E76B10F05"
      "3109EF8428BFCD50D38C4C030C57",
      "33244F479CDAC34F160D9E4CE2D19D2FF0E3305B5BF0EEF29E91E9DE6E28F678C61B"
      "773AA7E3C03740E1A49D1AA2493C"
    },
    { /* tv 181 */
      "NIST P-384",
      "11461893718178329909476114004508894060702157352553702808117365878450"
      "16396640969656447803207438173695115264",
      "CB8ED893530BFBA04B4CA655923AAAD109A62BC8411D5925316C32D33602459C3305"
      "7A1FBCB5F70AEB295D90F9165FBC",
      "426AEE3E91B08420F9B357B66D5AFCBCF3956590BF5564DBF9086042EB880493D19D"
      "A39AAA6436C6B5FC66CE5596B43F"
    },
    { /* tv 182 */
      "NIST P-384",
      "96193414382170976418653902971897088589380179864261526226395001797746"
      "24579127744608993294698873437325090751520764",
      "67F714012B6B070182122DDD435CC1C2262A1AB88939BC6A2906CB2B4137C5E82B45"
      "82160F6403CAB887ACDF5786A268",
      "90E31CF398CE2F8C5897C7380BF541075D1B4D3CB70547262B7095731252F181AC05"
      "97C66AF8311C7780DB39DEC0BD32"
    },
    { /* tv 183 */
      "NIST P-384",
      "12313079966238337423874003523801725660779274151368132827356419183955"
      "85376659282194317590461518639141730493780722175",
      "55A79DF7B53A99D31462C7E1A5ED5623970715BB1021098CB973A7520CBD6365E613"
      "E4B2467486FB37E86E01CEE09B8F",
      "B95AEB71693189911661B709A886A1867F056A0EFE401EE11C06030E46F7A87731DA"
      "4575863178012208707DD666727C"
    },
    { /* tv 184 */
      "NIST P-384",
      "58711883885468380094290672250481034308669902145190694600327412897305"
      "8942197377013128840514404789143516741631",
      "9539A968CF819A0E52E10EEA3BACA1B6480D7E4DF69BC07002C568569047110EE4FE"
      "72FCA423FDD5179D6E0E19C44844",
      "A7728F37A0AE0DF2716061900D83A4DA149144129F89A214A8260464BAB609BB322E"
      "4E67DE5E4C4C6CB8D25983EC19B0"
    },
    { /* tv 185 */
      "NIST P-384",
      "15391407753067173966379507087689476645146601937464415054145255714789"
      "0542143280855693795882295846834387672681660416",
      "933FC13276672AB360D909161CD02D830B1628935DF0D800C6ED602C59D575A86A8A"
      "97E3A2D697E3ED06BE741C0097D6",
      "F35296BD7A6B4C6C025ED6D84338CCCC7522A45C5D4FBDB1442556CAEFB598128FA1"
      "88793ADA510EB5F44E90A4E4BEF1"
    },
    { /* tv 186 */
      "NIST P-384",
      "75148784606135150476268171850082176256856776750560539466196504390587"
      "921789283134009866871754361028131485122560",
      "0CE31E1C4A937071E6EBACA026A93D783848BCC0C1585DAF639518125FCD1F1629D6"
      "3041ABFB11FFC8F03FA8B6FCF6BF",
      "A69EA55BE4BEAB2D5224050FEBFFBDFCFD614624C3B4F228909EB80012F003756D1C"
      "377E52F04FA539237F24DD080E2E"
    },
    { /* tv 187 */
      "NIST P-384",
      "19691383761310193665095292424754807745686799029814707849273381514021"
      "788371252213000473497648851202400395528761229312",
      "6842CFE3589AC268818291F31D44177A9168DCBC19F321ED66D81ECF59E31B54CCA0"
      "DDFD4C4136780171748D69A91C54",
      "E3A5ECD5AC725F13DBC631F358C6E817EDCF3A613B83832741A9DB591A0BAE767FC7"
      "14F70C2E7EA891E4312047DECCC0"
    },
    { /* tv 188 */
      "NIST P-384",
      "39402006196394479212279040100143613805079739270465446667946905279627"
      "659399113263569398956308152294913554433653942623",
      "605508EC02C534BCEEE9484C86086D2139849E2B11C1A9CA1E2808DEC2EAF161AC8A"
      "105D70D4F85C50599BE5800A623F",
      "AEA7117869D53947E0FF5EFC47ABC5F8C7E489C65C59A0ECAC510EE48CCEF92116D1"
      "6647721C984B71DC73C825271122"
    },
    { /* tv 189 */
      "NIST P-384",
      "39402006196394479212279040100143613805079739270465446667946905279627"
      "659399113263569398956308152294913554433653942624",
      "8D481DAB912BC8AB16858A211D750B77E07DBECCA86CD9B012390B430467AABF59C8"
      "651060801C0E9599E68713F5D41B",
      "5EA6D00FEDEB9F7A841660D59F996FAF4DD6E4975EFC655FA6B4CD028523F172EE00"
      "45A8F7FFB19B966A4F828A1ADDBA"
    },
    { /* tv 190 */
      "NIST P-384",
      "39402006196394479212279040100143613805079739270465446667946905279627"
      "659399113263569398956308152294913554433653942625",
      "DFB1FE3A40F7AC9B64C41D39360A7423828B97CB088A4903315E402A7089FA0F8B6C"
      "2355169CC9C99DFB44692A9B93DD",
      "BAC535EDBC4A1394BDC5975DAA781E9EC59CB3E3BD2D118193A80BB65E36E2366E97"
      "48FB913F580C85C99E7BDCC13ADD"
    },
    { /* tv 191 */
      "NIST P-384",
      "39402006196394479212279040100143613805079739270465446667946905279627"
      "659399113263569398956308152294913554433653942626",
      "4099952208B4889600A5EBBCB13E1A32692BEFB0733B41E6DCC614E42E5805F81701"
      "2A991AF1F486CAF3A9ADD9FFCC03",
      "A1306B8887CCFA67C6B8BA6B509FCA67E9C52C07FF752F32648682D880DC774BFB25"
      "B2CF55697F13059AF10B1DC4F65F"
    },
    { /* tv 192 */
      "NIST P-384",
      "39402006196394479212279040100143613805079739270465446667946905279627"
      "659399113263569398956308152294913554433653942627",
      "D5D89C3B5282369C5FBD88E2B231511A6B80DFF0E5152CF6A464FA9428A8583BAC8E"
      "BC773D157811A462B892401DAFCF",
      "27EADD621ED6F92DBE7E92A1656BB70E2BE2B03BF1D5C42463545A81BBF585442EDE"
      "F3460B640DDC97DD48AB1454C98C"
    },
    { /* tv 193 */
      "NIST P-384",
      "39402006196394479212279040100143613805079739270465446667946905279627"
      "659399113263569398956308152294913554433653942628",
      "B3D13FC8B32B01058CC15C11D813525522A94156FFF01C205B21F9F7DA7C4E9CA849"
      "557A10B6383B4B88701A9606860B",
      "EAD6E618206E9D59E4FB64DAC9E9B4E411453B5EE28B650B7B2EEEBC8C2040257C72"
      "DB064D7B50AF67A2A773CC08429D"
    },
    { /* tv 194 */
      "NIST P-384",
      "39402006196394479212279040100143613805079739270465446667946905279627"
      "659399113263569398956308152294913554433653942629",
      "E8C8F94D44FBC2396BBEAC481B89D2B0877B1DFFD23E7DC95DE541EB651CCA2C41AB"
      "A24DBC02DE6637209ACCF0F59EA0",
      "76E51BBCA903751F6CD4340921AD3756CC479E6E188D728637CE6BEC5F62F0B603B9"
      "745EAAF621DD2811A362E4984777"
    },
    { /* tv 195 */
      "NIST P-384",
      "39402006196394479212279040100143613805079739270465446667946905279627"
      "659399113263569398956308152294913554433653942630",
      "A567BA97B67AEA5BAFDAF5002FFCC6AB9632BFF9F01F873F6267BCD1F0F11C139EE5"
      "F441ABD99F1BAAF1CA1E3B5CBCE7",
      "21E4C74C6760CCE79BB1BEB850E9B133AE7AA6AFB96CD13F79CD641FA87A82988894"
      "347C8DDE75829BDC5ED9C90BD633"
    },
    { /* tv 196 */
      "NIST P-384",
      "39402006196394479212279040100143613805079739270465446667946905279627"
      "659399113263569398956308152294913554433653942631",
      "952A7A349BD49289AB3AC421DCF683D08C2ED5E41F6D0E21648AF2691A481406DA4A"
      "5E22DA817CB466DA2EA77D2A7022",
      "5FCDF0507B4A43FA9CFAD215190990D1F6047FC931E75F1446FD74F69E694AF1FCE5"
      "59B9768BC1DD610945341DE42E91"
    },
    { /* tv 197 */
      "NIST P-384",
      "39402006196394479212279040100143613805079739270465446667946905279627"
      "659399113263569398956308152294913554433653942632",
      "099056E27DA7B998DA1EEEC2904816C57FE935ED5837C37456C9FD14892D3F8C4749"
      "B66E3AFB81D626356F3B55B4DDD8",
      "D1B3F3DCB1CF5469977AFAABB53A1FC6903B1127203C9C02BC006C0BE4AD5CD9AB99"
      "2AEC9C5500CA82A2457FC73A1F44"
    },
    { /* tv 198 */
      "NIST P-384",
      "39402006196394479212279040100143613805079739270465446667946905279627"
      "659399113263569398956308152294913554433653942633",
      "A669C5563BD67EEC678D29D6EF4FDE864F372D90B79B9E88931D5C29291238CCED8E"
      "85AB507BF91AA9CB2D13186658FB",
      "567748D5183ED860DD26F7C24A0F132208FEE6AAF3E7C3CE3AFD20873C48FA56D692"
      "7E69DB7D77266887B09648C5DE22"
    },
    { /* tv 199 */
      "NIST P-384",
      "39402006196394479212279040100143613805079739270465446667946905279627"
      "659399113263569398956308152294913554433653942634",
      "8F0A39A4049BCB3EF1BF29B8B025B78F2216F7291E6FD3BAC6CB1EE285FB6E21C388"
      "528BFEE2B9535C55E4461079118B",
      "9D3881EBC749FE29BAD3B5ACDD3C56866564C2835C3BFF39489877AB51264CFC618B"
      "C100202AE497D9D25B075399B507"
    },
    { /* tv 200 */
      "NIST P-384",
      "39402006196394479212279040100143613805079739270465446667946905279627"
      "659399113263569398956308152294913554433653942635",
      "1692778EA596E0BE75114297A6FA383445BF227FBE58190A900C3C73256F11FB5A32"
      "58D6F403D5ECE6E9B269D822C87D",
      "232DC9A8FF2BEF957CAC7745C24702F1DDAAB52392ADE32B42E3CF3D13F113E59452"
      "1E15322E8F729095405CFDD4F52D"
    },
    { /* tv 201 */
      "NIST P-384",
      "39402006196394479212279040100143613805079739270465446667946905279627"
      "659399113263569398956308152294913554433653942636",
      "283C1D7365CE4788F29F8EBF234EDFFEAD6FE997FBEA5FFA2D58CC9DFA7B1C508B05"
      "526F55B9EBB2040F05B48FB6D0E1",
      "6B8A366F9E1BE47745AD102473E96FB8E59E2798128668D62636D32FE242DDA8CF27"
      "B120CD5870619B99B3263AED1073"
    },
    { /* tv 202 */
      "NIST P-384",
      "39402006196394479212279040100143613805079739270465446667946905279627"
      "659399113263569398956308152294913554433653942637",
      "627BE1ACD064D2B2226FE0D26F2D15D3C33EBCBB7F0F5DA51CBD41F2625738302131"
      "7D7202FF30E50937F0854E35C5DF",
      "F68995B34C074E3DE41922593EB0EA8A4D36ACAD9BB088B36679B09EC8EABBE8FB3B"
      "A4717B1E9ACEE8CC5BF82C0F06CB"
    },
    { /* tv 203 */
      "NIST P-384",
      "39402006196394479212279040100143613805079739270465446667946905279627"
      "659399113263569398956308152294913554433653942638",
      "11DE24A2C251C777573CAC5EA025E467F208E51DBFF98FC54F6661CBE56583B03788"
      "2F4A1CA297E60ABCDBC3836D84BC",
      "705969388BBF06D2F0A7C816F5FF183AD7B4BB88AB2A211773679ACC496FE513CE88"
      "9791F51704CCE7BBEB55193E8EC5"
    },
    { /* tv 204 */
      "NIST P-384",
      "39402006196394479212279040100143613805079739270465446667946905279627"
      "659399113263569398956308152294913554433653942639",
      "138251CD52AC9298C1C8AAD977321DEB97E709BD0B4CA0ACA55DC8AD51DCFC9D1589"
      "A1597E3A5120E1EFD631C63E1835",
      "35351D679659D1E9CE175D7E7E54A99E923BA26E7543F60C54F19C3086D55B22128C"
      "7840C8445A96AB60E3FE4D8F1298"
    },
    { /* tv 205 */
      "NIST P-384",
      "39402006196394479212279040100143613805079739270465446667946905279627"
      "659399113263569398956308152294913554433653942640",
      "077A41D4606FFA1464793C7E5FDC7D98CB9D3910202DCD06BEA4F240D3566DA6B408"
      "BBAE5026580D02D7E5C70500C831",
      "366A0835F4F3BD7C82F44169FD5603667ADF4BE37AEEA55A0897B3F123EEE1523DB5"
      "42931B4A2D6749A0D7A0F5D0E20E"
    },
    { /* tv 206 */
      "NIST P-384",
      "39402006196394479212279040100143613805079739270465446667946905279627"
      "659399113263569398956308152294913554433653942641",
      "08D999057BA3D2D969260045C55B97F089025959A6F434D651D207D19FB96E9E4FE0"
      "E86EBE0E64F85B96A9C75295DF61",
      "717F0E05A4E4C312484017200292458B4D8A278A43933BC16FB1AFA0DA954BD9A002"
      "BC15B2C61DD29EAFE190F56BF17F"
    },
    { /* tv 207 */
      "NIST P-384",
      "39402006196394479212279040100143613805079739270465446667946905279627"
      "659399113263569398956308152294913554433653942642",
      "AA87CA22BE8B05378EB1C71EF320AD746E1D3B628BA79B9859F741E082542A385502"
      "F25DBF55296C3A545E3872760AB7",
      "C9E821B569D9D390A26167406D6D23D6070BE242D765EB831625CEEC4A0F473EF59F"
      "4E30E2817E6285BCE2846F15F1A0"
    },
    { /* tv 208 */
      "NIST P-521",
      "1",
      "00C6858E06B70404E9CD9E3ECB662395B4429C648139053FB521F828AF606B4D3DBA"
      "A14B5E77EFE75928FE1DC127A2FFA8DE3348B3C1856A429BF97E7E31C2E5BD66",
      "011839296A789A3BC0045C8A5FB42C7D1BD998F54449579B446817AFBD17273E662C"
      "97EE72995EF42640C550B9013FAD0761353C7086A272C24088BE94769FD16650"
    },
    { /* tv 209 */
      "NIST P-521",
      "2",
      "00433C219024277E7E682FCB288148C282747403279B1CCC06352C6E5505D769BE97"
      "B3B204DA6EF55507AA104A3A35C5AF41CF2FA364D60FD967F43E3933BA6D783D",
      "00F4BB8CC7F86DB26700A7F3ECEEEED3F0B5C6B5107C4DA97740AB21A29906C42DBB"
      "B3E377DE9F251F6B93937FA99A3248F4EAFCBE95EDC0F4F71BE356D661F41B02"
    },
    { /* tv 210 */
      "NIST P-521",
      "3",
      "01A73D352443DE29195DD91D6A64B5959479B52A6E5B123D9AB9E5AD7A112D7A8DD1"
      "AD3F164A3A4832051DA6BD16B59FE21BAEB490862C32EA05A5919D2EDE37AD7D",
      "013E9B03B97DFA62DDD9979F86C6CAB814F2F1557FA82A9D0317D2F8AB1FA355CEEC"
      "2E2DD4CF8DC575B02D5ACED1DEC3C70CF105C9BC93A590425F588CA1EE86C0E5"
    },
    { /* tv 211 */
      "NIST P-521",
      "4",
      "0035B5DF64AE2AC204C354B483487C9070CDC61C891C5FF39AFC06C5D55541D3CEAC"
      "8659E24AFE3D0750E8B88E9F078AF066A1D5025B08E5A5E2FBC87412871902F3",
      "0082096F84261279D2B673E0178EB0B4ABB65521AEF6E6E32E1B5AE63FE2F19907F2"
      "79F283E54BA385405224F750A95B85EEBB7FAEF04699D1D9E21F47FC346E4D0D"
    },
    { /* tv 212 */
      "NIST P-521",
      "5",
      "00652BF3C52927A432C73DBC3391C04EB0BF7A596EFDB53F0D24CF03DAB8F177ACE4"
      "383C0C6D5E3014237112FEAF137E79A329D7E1E6D8931738D5AB5096EC8F3078",
      "015BE6EF1BDD6601D6EC8A2B73114A8112911CD8FE8E872E0051EDD817C9A0347087"
      "BB6897C9072CF374311540211CF5FF79D1F007257354F7F8173CC3E8DEB090CB"
    },
    { /* tv 213 */
      "NIST P-521",
      "6",
      "01EE4569D6CDB59219532EFF34F94480D195623D30977FD71CF3981506ADE4AB0152"
      "5FBCCA16153F7394E0727A239531BE8C2F66E95657F380AE23731BEDF79206B9",
      "01DE0255AD0CC64F586AE2DD270546E3B1112AABBB73DA5A808E7240A926201A8A96"
      "CAB72D0E56648C9DF96C984DE274F2203DC7B8B55CA0DADE1EACCD7858D44F17"
    },
    { /* tv 214 */
      "NIST P-521",
      "7",
      "0056D5D1D99D5B7F6346EEB65FDA0B073A0C5F22E0E8F5483228F018D2C2F7114C5D"
      "8C308D0ABFC698D8C9A6DF30DCE3BBC46F953F50FDC2619A01CEAD882816ECD4",
      "003D2D1B7D9BAAA2A110D1D8317A39D68478B5C582D02824F0DD71DBD98A26CBDE55"
      "6BD0F293CDEC9E2B9523A34591CE1A5F9E76712A5DDEFC7B5C6B8BC90525251B"
    },
    { /* tv 215 */
      "NIST P-521",
      "8",
      "000822C40FB6301F7262A8348396B010E25BD4E29D8A9B003E0A8B8A3B05F826298F"
      "5BFEA5B8579F49F08B598C1BC8D79E1AB56289B5A6F4040586F9EA54AA78CE68",
      "016331911D5542FC482048FDAB6E78853B9A44F8EDE9E2C0715B5083DE610677A8F1"
      "89E9C0AA5911B4BFF0BA0DF065C578699F3BA940094713538AD642F11F17801C"
    },
    { /* tv 216 */
      "NIST P-521",
      "9",
      "01585389E359E1E21826A2F5BF157156D488ED34541B988746992C4AB145B8C6B665"
      "7429E1396134DA35F3C556DF725A318F4F50BABD85CD28661F45627967CBE207",
      "002A2E618C9A8AEDF39F0B55557A27AE938E3088A654EE1CEBB6C825BA263DDB446E"
      "0D69E5756057AC840FF56ECF4ABFD87D736C2AE928880F343AA0EA86B9AD2A4E"
    },
    { /* tv 217 */
      "NIST P-521",
      "10",
      "0190EB8F22BDA61F281DFCFE7BB6721EC4CD901D879AC09AC7C34A9246B11ADA8910"
      "A2C7C178FCC263299DAA4DA9842093F37C2E411F1A8E819A87FF09A04F2F3320",
      "01EB5D96B8491614BA9DBAEAB3B0CA2BA760C2EEB2144251B20BA97FD78A62EF62D2"
      "BF5349D44D9864BB536F6163DC57EBEFF3689639739FAA172954BC98135EC759"
    },
    { /* tv 218 */
      "NIST P-521",
      "11",
      "008A75841259FDEDFF546F1A39573B4315CFED5DC7ED7C17849543EF2C54F2991652"
      "F3DBC5332663DA1BD19B1AEBE3191085015C024FA4C9A902ECC0E02DDA0CDB9A",
      "0096FB303FCBBA2129849D0CA877054FB2293ADD566210BD0493ED2E95D4E0B9B82B"
      "1BC8A90E8B42A4AB3892331914A95336DCAC80E3F4819B5D58874F92CE48C808"
    },
    { /* tv 219 */
      "NIST P-521",
      "12",
      "01C0D9DCEC93F8221C5DE4FAE9749C7FDE1E81874157958457B6107CF7A5967713A6"
      "44E90B7C3FB81B31477FEE9A60E938013774C75C530928B17BE69571BF842D8C",
      "014048B5946A4927C0FE3CE1D103A682CA4763FE65AB71494DA45E404ABF6A17C097"
      "D6D18843D86FCDB6CC10A6F951B9B630884BA72224F5AE6C79E7B1A3281B17F0"
    },
    { /* tv 220 */
      "NIST P-521",
      "13",
      "007E3E98F984C396AD9CD7865D2B4924861A93F736CDE1B4C2384EEDD2BEAF5B8661"
      "32C45908E03C996A3550A5E79AB88EE94BEC3B00AB38EFF81887848D32FBCDA7",
      "0108EE58EB6D781FEDA91A1926DAA3ED5A08CED50A386D5421C69C7A67AE5C1E212A"
      "C1BD5D5838BC763F26DFDD351CBFBBC36199EAAF9117E9F7291A01FB022A71C9"
    },
    { /* tv 221 */
      "NIST P-521",
      "14",
      "01875BC7DC551B1B65A9E1B8CCFAAF84DED1958B401494116A2FD4FB0BABE0B31999"
      "74FC06C8B897222D79DF3E4B7BC744AA6767F6B812EFBF5D2C9E682DD3432D74",
      "005CA4923575DACB5BD2D66290BBABB4BDFB8470122B8E51826A0847CE9B86D7ED62"
      "D07781B1B4F3584C11E89BF1D133DC0D5B690F53A87C84BE41669F852700D54A"
    },
    { /* tv 222 */
      "NIST P-521",
      "15",
      "006B6AD89ABCB92465F041558FC546D4300FB8FBCC30B40A0852D697B532DF128E11"
      "B91CCE27DBD00FFE7875BD1C8FC0331D9B8D96981E3F92BDE9AFE337BCB8DB55",
      "01B468DA271571391D6A7CE64D2333EDBF63DF0496A9BAD20CBA4B62106997485ED5"
      "7E9062C899470A802148E2232C96C99246FD90CC446ABDD956343480A1475465"
    },
    { /* tv 223 */
      "NIST P-521",
      "16",
      "01D17D10D8A89C8AD05DDA97DA26AC743B0B2A87F66192FD3F3DD632F8D20B188A52"
      "943FF18861CA00A0E5965DA7985630DF0DBF5C8007DCDC533A6C508F81A8402F",
      "007A37343C582D77001FC714B18D3D3E69721335E4C3B800D50EC7CA30C94B6B82C1"
      "C182E1398DB547AA0B3075AC9D9988529E3004D28D18633352E272F89BC73ABE"
    },
    { /* tv 224 */
      "NIST P-521",
      "17",
      "01B00DDB707F130EDA13A0B874645923906A99EE9E269FA2B3B4D66524F269250858"
      "760A69E674FE0287DF4E799B5681380FF8C3042AF0D1A41076F817A853110AE0",
      "0085683F1D7DB16576DBC111D4E4AEDDD106B799534CF69910A98D68AC2B22A1323D"
      "F9DA564EF6DD0BF0D2F6757F16ADF420E6905594C2B755F535B9CB7C70E64647"
    },
    { /* tv 225 */
      "NIST P-521",
      "18",
      "01BC33425E72A12779EACB2EDCC5B63D1281F7E86DBC7BF99A7ABD0CFE367DE4666D"
      "6EDBB8525BFFE5222F0702C3096DEC0884CE572F5A15C423FDF44D01DD99C61D",
      "010D06E999885B63535DE3E74D33D9E63D024FB07CE0D196F2552C8E4A00AC84C044"
      "234AEB201F7A9133915D1B4B45209B9DA79FE15B19F84FD135D841E2D8F9A86A"
    },
    { /* tv 226 */
      "NIST P-521",
      "19",
      "00998DCCE486419C3487C0F948C2D5A1A07245B77E0755DF547EFFF0ACDB3790E7F1"
      "FA3B3096362669679232557D7A45970DFECF431E725BBDE478FF0B2418D6A19B",
      "0137D5DA0626A021ED5CC3942497535B245D67D28AEE2B7BCF4ACC50EEE365457727"
      "73AD963FF2EB8CF9B0EC39991631C377F5A4D89EA9FBFE44A9091A695BFD0575"
    },
    { /* tv 227 */
      "NIST P-521",
      "20",
      "018BDD7F1B889598A4653DEEAE39CC6F8CC2BD767C2AB0D93FB12E968FBED342B517"
      "09506339CB1049CB11DD48B9BDB3CD5CAD792E43B74E16D8E2603BFB11B0344F",
      "00C5AADBE63F68CA5B6B6908296959BF0AF89EE7F52B410B9444546C550952D31120"
      "4DA3BDDDC6D4EAE7EDFAEC1030DA8EF837CCB22EEE9CFC94DD3287FED0990F94"
    },
    { /* tv 228 */
      "NIST P-521",
      "112233445566778899",
      "01650048FBD63E8C30B305BF36BD7643B91448EF2206E8A0CA84A140789A99B0423A"
      "0A2533EA079CA7E049843E69E5FA2C25A163819110CEC1A30ACBBB3A422A40D8",
      "010C9C64A0E0DB6052DBC5646687D06DECE5E9E0703153EFE9CB816FE025E85354D3"
      "C5F869D6DB3F4C0C01B5F97919A5E72CEEBE03042E5AA99112691CFFC2724828"
    },
    { /* tv 229 */
      "NIST P-521",
      "112233445566778899112233445566778899",
      "017E1370D39C9C63925DAEEAC571E21CAAF60BD169191BAEE8352E0F54674443B297"
      "86243564ABB705F6FC0FE5FC5D3F98086B67CA0BE7AC8A9DEC421D9F1BC6B37F",
      "01CD559605EAD19FBD99E83600A6A81A0489E6F20306EE0789AE00CE16A6EFEA2F42"
      "F7534186CF1C60DF230BD9BCF8CB95E5028AD9820B2B1C0E15597EE54C4614A6"
    },
    { /* tv 230 */
      "NIST P-521",
      "17698052779751630352537759308423671290937417867253767860073493326533"
      "23812656658291413435033257677579095366632521448854141275926144187294"
      "499863933403633025023",
      "00B45CB84651C9D4F08858B867F82D816E84E94FE4CAE3DA5F65E420B08398D0C5BF"
      "019253A6C26D20671BDEF0B8E6C1D348A4B0734687F73AC6A4CBB2E085C68B3F",
      "01C84942BBF538903062170A4BA8B3410D385719BA2037D29CA5248BFCBC8478220F"
      "EC79244DCD45D31885A1764DEE479CE20B12CEAB62F9001C7AA4282CE4BE7F56"
    },
    { /* tv 231 */
      "NIST P-521",
      "10474840033715746231626262792913259631724379050679813326769821870752"
      "87502926828892214143101559079638247121149165524401608805506660439970"
      "30661040721887239",
      "01CCEF4CDA108CEBE6568820B54A3CA3A3997E4EF0EDA6C350E7ED3DBB1861EDD801"
      "81C650CEBE5440FEBA880F9C8A7A86F8B82659794F6F5B88E501E5DD84E65D7E",
      "01026565F8B195D03C3F6139C3A63EAA1C29F7090AB2A8F75027939EC05109035F1B"
      "38E6C508E0C14CE53AB7E2DA33AA28140EDBF3964862FB157119517454E60F07"
    },
    { /* tv 232 */
      "NIST P-521",
      "67039038650783458881413816514301680394966640773509650542881331265493"
      "07058741788671148197429777343936466127575938031786147409472627479702"
      "469884214509568000",
      "00C1002DC2884EEDADB3F9B468BBEBD55980799852C506D37271FFCD006919DB3A96"
      "DF8FE91EF6ED4B9081B1809E8F2C2B28AF5FCBF524147C73CB0B913D6FAB0995",
      "01614E8A62C8293DD2AA6EF27D30974A4FD185019FA8EF4F982DA48698CECF706581"
      "F69EE9ED67A9C231EC9D0934D0F674646153273BCBB345E923B1EC1386A1A4AD"
    },
    { /* tv 233 */
      "NIST P-521",
      "16759256436823953054045171656435622518800269587808965316988567370241"
      "79880343339878336382412050263431942974939646683480906434632963478257"
      "639757341102436352",
      "010ED3E085ECDE1E66874286B5D5642B9D37853A026A0A025C7B84936E2ECEEC5F34"
      "2E14C80C79CCF814D5AD085C5303F2823251F2B9276F88C9D7A43E387EBD87AC",
      "01BE399A7666B29E79BBF3D277531A97CE05CAC0B49BECE4781E7AEE0D6E80FEE883"
      "C76E9F08453DC1ADE4E49300F3D56FEE6A1510DA1B1F12EEAA39A05AA0508119"
    },
    { /* tv 234 */
      "NIST P-521",
      "12785133382149415221402495202586701798620696169446772599038235721862"
      "33869219015616395155896385695905923238160286474392442745178676951515"
      "4396810706943",
      "013070A29B059D317AF37089E40FCB135868F52290EFF3E9F3E32CDADCA18EA234D8"
      "589C665A4B8E3D0714DE004A419DEA7091A3BBA97263C438FE9413AA598FD4A5",
      "00238A27FD9E5E7324C8B538EF2E334B71AC2611A95F42F4F2544D8C4A65D2A32A8B"
      "AFA15EFD4FC2BD8AB2B0C51F65B680879589F4D5FE8A84CEB17A2E8D3587F011"
    },
    { /* tv 235 */
      "NIST P-521",
      "21452487583224925587220685549573442688947752933626165525549242527332"
      "27278613418256777229473754067116763723353140430716009349416151854185"
      "40320233184489636351",
      "01A3D88799878EC74E66FF1AD8C7DFA9A9B4445A17F0810FF8189DD27AE3B6C580D3"
      "52476DBDAEB08D7DA0DE3866F7C7FDBEBB8418E19710F1F7AFA88C22280B1404",
      "00B39703D2053EC7B8812BDFEBFD81B4CB76F245FE535A1F1E46801C35DE03C15063"
      "A99A203981529C146132863CA0E68544D0F0A638D8A2859D82B4DD266F27C3AE"
    },
    { /* tv 236 */
      "NIST P-521",
      "51140486275567859131139077890835526884648461857823088348651153840508"
      "28762136685450683124474653127224662029512310426956586705594937826639"
      "5604768784399",
      "01D16B4365DEFE6FD356DC1F31727AF2A32C7E86C5AE87ED2950A08BC8653F203C7F"
      "7860E80F95AA27C93EA76E8CD094127B15ED42CC5F96DC0A0F9A1C1E31D0D526",
      "006E3710A0F9366E0BB8A14FFE8EBC2722EECF4A123EC9BA98DCCCA335D6FAFD289D"
      "C69FD90903C9AC982FEB46DF93F03A7C8C9549D32C1C386D17F37340E63822A8"
    },
    { /* tv 237 */
      "NIST P-521",
      "66515297160252068810352799528815206278411522472127845209144250393126"
      "06120198879080839643311347169019249080198239408356563413447402270445"
      "462102068592377843",
      "01B1220F67C985E9FC9C588C0C86BB16E6FE4CC11E168A98D701AE4670724B3D030E"
      "D9965FADF4207C7A1BE9BE0F40DEF2BBFFF0C7EABCB5B42526CE1D3CAA468F52",
      "006CDAD2860F6D2C37159A5A866D11605F2E7D87430DCFE6E6816AB6423CD9003CA6"
      "F2527B9C2A2483C541D456C963D18A0D2A46E158CB2A44C0BF42D562881FB748"
    },
    { /* tv 238 */
      "NIST P-521",
      "32245518246132322325376800779468186601568352887780873448053703978113"
      "79731631671254853846826682273677870214778462237171365140390183770226"
      "853329363961324241919",
      "00F25E545213C8C074BE38A0612EA9B66336B14A874372548D9716392DFA31CD0D13"
      "E94F86CD48B8D43B80B5299144E01245C873B39F6AC6C4FB397746AF034AD67C",
      "01733ABB21147CC27E35F41FAF40290AFD1EEB221D983FFABBD88E5DC8776450A409"
      "EACDC1BCA2B9F517289C68645BB96781808FEAE42573C2BB289F16E2AECECE17"
    },
    { /* tv 239 */
      "NIST P-521",
      "12486613128442885430380874043991285080254917488396284953815149251315"
      "41260063458153906666309229761204066997801762358775284540965316727702"
      "1864132608",
      "0172CD22CBE0634B6BFEE24BB1D350F384A945ED618ECAD48AADC6C1BC0DCC107F0F"
      "FE9FE14DC929F90153F390C25BE5D3A73A56F9ACCB0C72C768753869732D0DC4",
      "00D249CFB570DA4CC48FB5426A928B43D7922F787373B6182408FBC71706E7527E84"
      "14C79167F3C999FF58DE352D238F1FE7168C658D338F72696F2F889A97DE23C5"
    },
    { /* tv 240 */
      "NIST P-521",
      "68647976601306097149819007990813932172694353001433054093944634591855"
      "43183397655394245057746333217197532963996371363321113864768612440380"
      "340372808892707005429",
      "018BDD7F1B889598A4653DEEAE39CC6F8CC2BD767C2AB0D93FB12E968FBED342B517"
      "09506339CB1049CB11DD48B9BDB3CD5CAD792E43B74E16D8E2603BFB11B0344F",
      "013A552419C09735A49496F7D696A640F50761180AD4BEF46BBBAB93AAF6AD2CEEDF"
      "B25C4222392B1518120513EFCF257107C8334DD11163036B22CD78012F66F06B"
    },
    { /* tv 241 */
      "NIST P-521",
      "68647976601306097149819007990813932172694353001433054093944634591855"
      "43183397655394245057746333217197532963996371363321113864768612440380"
      "340372808892707005430",
      "00998DCCE486419C3487C0F948C2D5A1A07245B77E0755DF547EFFF0ACDB3790E7F1"
      "FA3B3096362669679232557D7A45970DFECF431E725BBDE478FF0B2418D6A19B",
      "00C82A25F9D95FDE12A33C6BDB68ACA4DBA2982D7511D48430B533AF111C9ABA88D8"
      "8C5269C00D1473064F13C666E9CE3C880A5B2761560401BB56F6E596A402FA8A"
    },
    { /* tv 242 */
      "NIST P-521",
      "68647976601306097149819007990813932172694353001433054093944634591855"
      "43183397655394245057746333217197532963996371363321113864768612440380"
      "340372808892707005431",
      "01BC33425E72A12779EACB2EDCC5B63D1281F7E86DBC7BF99A7ABD0CFE367DE4666D"
      "6EDBB8525BFFE5222F0702C3096DEC0884CE572F5A15C423FDF44D01DD99C61D",
      "00F2F9166677A49CACA21C18B2CC2619C2FDB04F831F2E690DAAD371B5FF537B3FBB"
      "DCB514DFE0856ECC6EA2E4B4BADF646258601EA4E607B02ECA27BE1D27065795"
    },
    { /* tv 243 */
      "NIST P-521",
      "68647976601306097149819007990813932172694353001433054093944634591855"
      "43183397655394245057746333217197532963996371363321113864768612440380"
      "340372808892707005432",
      "01B00DDB707F130EDA13A0B874645923906A99EE9E269FA2B3B4D66524F269250858"
      "760A69E674FE0287DF4E799B5681380FF8C3042AF0D1A41076F817A853110AE0",
      "017A97C0E2824E9A89243EEE2B1B51222EF94866ACB30966EF56729753D4DD5ECDC2"
      "0625A9B10922F40F2D098A80E9520BDF196FAA6B3D48AA0ACA4634838F19B9B8"
    },
    { /* tv 244 */
      "NIST P-521",
      "68647976601306097149819007990813932172694353001433054093944634591855"
      "43183397655394245057746333217197532963996371363321113864768612440380"
      "340372808892707005433",
      "01D17D10D8A89C8AD05DDA97DA26AC743B0B2A87F66192FD3F3DD632F8D20B188A52"
      "943FF18861CA00A0E5965DA7985630DF0DBF5C8007DCDC533A6C508F81A8402F",
      "0185C8CBC3A7D288FFE038EB4E72C2C1968DECCA1B3C47FF2AF13835CF36B4947D3E"
      "3E7D1EC6724AB855F4CF8A53626677AD61CFFB2D72E79CCCAD1D8D076438C541"
    },
    { /* tv 245 */
      "NIST P-521",
      "68647976601306097149819007990813932172694353001433054093944634591855"
      "43183397655394245057746333217197532963996371363321113864768612440380"
      "340372808892707005434",
      "006B6AD89ABCB92465F041558FC546D4300FB8FBCC30B40A0852D697B532DF128E11"
      "B91CCE27DBD00FFE7875BD1C8FC0331D9B8D96981E3F92BDE9AFE337BCB8DB55",
      "004B9725D8EA8EC6E2958319B2DCCC12409C20FB6956452DF345B49DEF9668B7A12A"
      "816F9D3766B8F57FDEB71DDCD369366DB9026F33BB954226A9CBCB7F5EB8AB9A"
    },
    { /* tv 246 */
      "NIST P-521",
      "68647976601306097149819007990813932172694353001433054093944634591855"
      "43183397655394245057746333217197532963996371363321113864768612440380"
      "340372808892707005435",
      "01875BC7DC551B1B65A9E1B8CCFAAF84DED1958B401494116A2FD4FB0BABE0B31999"
      "74FC06C8B897222D79DF3E4B7BC744AA6767F6B812EFBF5D2C9E682DD3432D74",
      "01A35B6DCA8A2534A42D299D6F44544B42047B8FEDD471AE7D95F7B831647928129D"
      "2F887E4E4B0CA7B3EE17640E2ECC23F2A496F0AC57837B41BE99607AD8FF2AB5"
    },
    { /* tv 247 */
      "NIST P-521",
      "68647976601306097149819007990813932172694353001433054093944634591855"
      "43183397655394245057746333217197532963996371363321113864768612440380"
      "340372808892707005436",
      "007E3E98F984C396AD9CD7865D2B4924861A93F736CDE1B4C2384EEDD2BEAF5B8661"
      "32C45908E03C996A3550A5E79AB88EE94BEC3B00AB38EFF81887848D32FBCDA7",
      "00F711A7149287E01256E5E6D9255C12A5F7312AF5C792ABDE3963859851A3E1DED5"
      "3E42A2A7C74389C0D92022CAE340443C9E6615506EE81608D6E5FE04FDD58E36"
    },
    { /* tv 248 */
      "NIST P-521",
      "68647976601306097149819007990813932172694353001433054093944634591855"
      "43183397655394245057746333217197532963996371363321113864768612440380"
      "340372808892707005437",
      "01C0D9DCEC93F8221C5DE4FAE9749C7FDE1E81874157958457B6107CF7A5967713A6"
      "44E90B7C3FB81B31477FEE9A60E938013774C75C530928B17BE69571BF842D8C",
      "00BFB74A6B95B6D83F01C31E2EFC597D35B89C019A548EB6B25BA1BFB54095E83F68"
      "292E77BC2790324933EF5906AE4649CF77B458DDDB0A519386184E5CD7E4E80F"
    },
    { /* tv 249 */
      "NIST P-521",
      "68647976601306097149819007990813932172694353001433054093944634591855"
      "43183397655394245057746333217197532963996371363321113864768612440380"
      "340372808892707005438",
      "008A75841259FDEDFF546F1A39573B4315CFED5DC7ED7C17849543EF2C54F2991652"
      "F3DBC5332663DA1BD19B1AEBE3191085015C024FA4C9A902ECC0E02DDA0CDB9A",
      "016904CFC03445DED67B62F35788FAB04DD6C522A99DEF42FB6C12D16A2B1F4647D4"
      "E43756F174BD5B54C76DCCE6EB56ACC923537F1C0B7E64A2A778B06D31B737F7"
    },
    { /* tv 250 */
      "NIST P-521",
      "68647976601306097149819007990813932172694353001433054093944634591855"
      "43183397655394245057746333217197532963996371363321113864768612440380"
      "340372808892707005439",
      "0190EB8F22BDA61F281DFCFE7BB6721EC4CD901D879AC09AC7C34A9246B11ADA8910"
      "A2C7C178FCC263299DAA4DA9842093F37C2E411F1A8E819A87FF09A04F2F3320",
      "0014A26947B6E9EB456245154C4F35D4589F3D114DEBBDAE4DF4568028759D109D2D"
      "40ACB62BB2679B44AC909E9C23A814100C9769C68C6055E8D6AB4367ECA138A6"
    },
    { /* tv 251 */
      "NIST P-521",
      "68647976601306097149819007990813932172694353001433054093944634591855"
      "43183397655394245057746333217197532963996371363321113864768612440380"
      "340372808892707005440",
      "01585389E359E1E21826A2F5BF157156D488ED34541B988746992C4AB145B8C6B665"
      "7429E1396134DA35F3C556DF725A318F4F50BABD85CD28661F45627967CBE207",
      "01D5D19E736575120C60F4AAAA85D8516C71CF7759AB11E3144937DA45D9C224BB91"
      "F2961A8A9FA8537BF00A9130B54027828C93D516D777F0CBC55F15794652D5B1"
    },
    { /* tv 252 */
      "NIST P-521",
      "68647976601306097149819007990813932172694353001433054093944634591855"
      "43183397655394245057746333217197532963996371363321113864768612440380"
      "340372808892707005441",
      "000822C40FB6301F7262A8348396B010E25BD4E29D8A9B003E0A8B8A3B05F826298F"
      "5BFEA5B8579F49F08B598C1BC8D79E1AB56289B5A6F4040586F9EA54AA78CE68",
      "009CCE6EE2AABD03B7DFB7025491877AC465BB0712161D3F8EA4AF7C219EF988570E"
      "76163F55A6EE4B400F45F20F9A3A879660C456BFF6B8ECAC7529BD0EE0E87FE3"
    },
    { /* tv 253 */
      "NIST P-521",
      "68647976601306097149819007990813932172694353001433054093944634591855"
      "43183397655394245057746333217197532963996371363321113864768612440380"
      "340372808892707005442",
      "0056D5D1D99D5B7F6346EEB65FDA0B073A0C5F22E0E8F5483228F018D2C2F7114C5D"
      "8C308D0ABFC698D8C9A6DF30DCE3BBC46F953F50FDC2619A01CEAD882816ECD4",
      "01C2D2E48264555D5EEF2E27CE85C6297B874A3A7D2FD7DB0F228E242675D93421AA"
      "942F0D6C321361D46ADC5CBA6E31E5A061898ED5A2210384A3947436FADADAE4"
    },
    { /* tv 254 */
      "NIST P-521",
      "68647976601306097149819007990813932172694353001433054093944634591855"
      "43183397655394245057746333217197532963996371363321113864768612440380"
      "340372808892707005443",
      "01EE4569D6CDB59219532EFF34F94480D195623D30977FD71CF3981506ADE4AB0152"
      "5FBCCA16153F7394E0727A239531BE8C2F66E95657F380AE23731BEDF79206B9",
      "0021FDAA52F339B0A7951D22D8FAB91C4EEED554448C25A57F718DBF56D9DFE57569"
      "3548D2F1A99B7362069367B21D8B0DDFC238474AA35F2521E1533287A72BB0E8"
    },
    { /* tv 255 */
      "NIST P-521",
      "68647976601306097149819007990813932172694353001433054093944634591855"
      "43183397655394245057746333217197532963996371363321113864768612440380"
      "340372808892707005444",
      "00652BF3C52927A432C73DBC3391C04EB0BF7A596EFDB53F0D24CF03DAB8F177ACE4"
      "383C0C6D5E3014237112FEAF137E79A329D7E1E6D8931738D5AB5096EC8F3078",
      "00A41910E42299FE291375D48CEEB57EED6EE327017178D1FFAE1227E8365FCB8F78"
      "44976836F8D30C8BCEEABFDEE30A00862E0FF8DA8CAB0807E8C33C17214F6F34"
    },
    { /* tv 256 */
      "NIST P-521",
      "68647976601306097149819007990813932172694353001433054093944634591855"
      "43183397655394245057746333217197532963996371363321113864768612440380"
      "340372808892707005445",
      "0035B5DF64AE2AC204C354B483487C9070CDC61C891C5FF39AFC06C5D55541D3CEAC"
      "8659E24AFE3D0750E8B88E9F078AF066A1D5025B08E5A5E2FBC87412871902F3",
      "017DF6907BD9ED862D498C1FE8714F4B5449AADE5109191CD1E4A519C01D0E66F80D"
      "860D7C1AB45C7ABFADDB08AF56A47A114480510FB9662E261DE0B803CB91B2F2"
    },
    { /* tv 257 */
      "NIST P-521",
      "68647976601306097149819007990813932172694353001433054093944634591855"
      "43183397655394245057746333217197532963996371363321113864768612440380"
      "340372808892707005446",
      "01A73D352443DE29195DD91D6A64B5959479B52A6E5B123D9AB9E5AD7A112D7A8DD1"
      "AD3F164A3A4832051DA6BD16B59FE21BAEB490862C32EA05A5919D2EDE37AD7D",
      "00C164FC4682059D2226686079393547EB0D0EAA8057D562FCE82D0754E05CAA3113"
      "D1D22B30723A8A4FD2A5312E213C38F30EFA36436C5A6FBDA0A7735E11793F1A"
    },
    { /* tv 258 */
      "NIST P-521",
      "68647976601306097149819007990813932172694353001433054093944634591855"
      "43183397655394245057746333217197532963996371363321113864768612440380"
      "340372808892707005447",
      "00433C219024277E7E682FCB288148C282747403279B1CCC06352C6E5505D769BE97"
      "B3B204DA6EF55507AA104A3A35C5AF41CF2FA364D60FD967F43E3933BA6D783D",
      "010B44733807924D98FF580C1311112C0F4A394AEF83B25688BF54DE5D66F93BD244"
      "4C1C882160DAE0946C6C805665CDB70B1503416A123F0B08E41CA9299E0BE4FD"
    },
    { /* tv 259 */
      "NIST P-521",
      "68647976601306097149819007990813932172694353001433054093944634591855"
      "43183397655394245057746333217197532963996371363321113864768612440380"
      "340372808892707005448",
      "00C6858E06B70404E9CD9E3ECB662395B4429C648139053FB521F828AF606B4D3DBA"
      "A14B5E77EFE75928FE1DC127A2FFA8DE3348B3C1856A429BF97E7E31C2E5BD66",
      "00E7C6D6958765C43FFBA375A04BD382E426670ABBB6A864BB97E85042E8D8C199D3"
      "68118D66A10BD9BF3AAF46FEC052F89ECAC38F795D8D3DBF77416B89602E99AF"
    },

    /* secp256k1 test-vectors from
       https://chuckbatson.wordpress.com/2014/11/26/secp256k1-test-vectors/ */
    { /* tv 260 */
      "secp256k1",
      "1",
      "79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798",
      "483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8"
    },
    { /* tv 261 */
      "secp256k1",
      "2",
      "C6047F9441ED7D6D3045406E95C07CD85C778E4B8CEF3CA7ABAC09B95C709EE5",
      "1AE168FEA63DC339A3C58419466CEAEEF7F632653266D0E1236431A950CFE52A"
    },
    { /* tv 262 */
      "secp256k1",
      "3",
      "F9308A019258C31049344F85F89D5229B531C845836F99B08601F113BCE036F9",
      "388F7B0F632DE8140FE337E62A37F3566500A99934C2231B6CB9FD7584B8E672"
    },
    { /* tv 263 */
      "secp256k1",
      "4",
      "E493DBF1C10D80F3581E4904930B1404CC6C13900EE0758474FA94ABE8C4CD13",
      "51ED993EA0D455B75642E2098EA51448D967AE33BFBDFE40CFE97BDC47739922"
    },
    { /* tv 264 */
      "secp256k1",
      "5",
      "2F8BDE4D1A07209355B4A7250A5C5128E88B84BDDC619AB7CBA8D569B240EFE4",
      "D8AC222636E5E3D6D4DBA9DDA6C9C426F788271BAB0D6840DCA87D3AA6AC62D6"
    },
    { /* tv 265 */
      "secp256k1",
      "6",
      "FFF97BD5755EEEA420453A14355235D382F6472F8568A18B2F057A1460297556",
      "AE12777AACFBB620F3BE96017F45C560DE80F0F6518FE4A03C870C36B075F297"
    },
    { /* tv 266 */
      "secp256k1",
      "7",
      "5CBDF0646E5DB4EAA398F365F2EA7A0E3D419B7E0330E39CE92BDDEDCAC4F9BC",
      "6AEBCA40BA255960A3178D6D861A54DBA813D0B813FDE7B5A5082628087264DA"
    },
    { /* tv 267 */
      "secp256k1",
      "8",
      "2F01E5E15CCA351DAFF3843FB70F3C2F0A1BDD05E5AF888A67784EF3E10A2A01",
      "5C4DA8A741539949293D082A132D13B4C2E213D6BA5B7617B5DA2CB76CBDE904"
    },
    { /* tv 268 */
      "secp256k1",
      "9",
      "ACD484E2F0C7F65309AD178A9F559ABDE09796974C57E714C35F110DFC27CCBE",
      "CC338921B0A7D9FD64380971763B61E9ADD888A4375F8E0F05CC262AC64F9C37"
    },
    { /* tv 269 */
      "secp256k1",
      "10",
      "A0434D9E47F3C86235477C7B1AE6AE5D3442D49B1943C2B752A68E2A47E247C7",
      "893ABA425419BC27A3B6C7E693A24C696F794C2ED877A1593CBEE53B037368D7"
    },
    { /* tv 270 */
      "secp256k1",
      "11",
      "774AE7F858A9411E5EF4246B70C65AAC5649980BE5C17891BBEC17895DA008CB",
      "D984A032EB6B5E190243DD56D7B7B365372DB1E2DFF9D6A8301D74C9C953C61B"
    },
    { /* tv 271 */
      "secp256k1",
      "12",
      "D01115D548E7561B15C38F004D734633687CF4419620095BC5B0F47070AFE85A",
      "A9F34FFDC815E0D7A8B64537E17BD81579238C5DD9A86D526B051B13F4062327"
    },
    { /* tv 272 */
      "secp256k1",
      "13",
      "F28773C2D975288BC7D1D205C3748651B075FBC6610E58CDDEEDDF8F19405AA8",
      "0AB0902E8D880A89758212EB65CDAF473A1A06DA521FA91F29B5CB52DB03ED81"
    },
    { /* tv 273 */
      "secp256k1",
      "14",
      "499FDF9E895E719CFD64E67F07D38E3226AA7B63678949E6E49B241A60E823E4",
      "CAC2F6C4B54E855190F044E4A7B3D464464279C27A3F95BCC65F40D403A13F5B"
    },
    { /* tv 274 */
      "secp256k1",
      "15",
      "D7924D4F7D43EA965A465AE3095FF41131E5946F3C85F79E44ADBCF8E27E080E",
      "581E2872A86C72A683842EC228CC6DEFEA40AF2BD896D3A5C504DC9FF6A26B58"
    },
    { /* tv 275 */
      "secp256k1",
      "16",
      "E60FCE93B59E9EC53011AABC21C23E97B2A31369B87A5AE9C44EE89E2A6DEC0A",
      "F7E3507399E595929DB99F34F57937101296891E44D23F0BE1F32CCE69616821"
    },
    { /* tv 276 */
      "secp256k1",
      "17",
      "DEFDEA4CDB677750A420FEE807EACF21EB9898AE79B9768766E4FAA04A2D4A34",
      "4211AB0694635168E997B0EAD2A93DAECED1F4A04A95C0F6CFB199F69E56EB77"
    },
    { /* tv 277 */
      "secp256k1",
      "18",
      "5601570CB47F238D2B0286DB4A990FA0F3BA28D1A319F5E7CF55C2A2444DA7CC",
      "C136C1DC0CBEB930E9E298043589351D81D8E0BC736AE2A1F5192E5E8B061D58"
    },
    { /* tv 278 */
      "secp256k1",
      "19",
      "2B4EA0A797A443D293EF5CFF444F4979F06ACFEBD7E86D277475656138385B6C",
      "85E89BC037945D93B343083B5A1C86131A01F60C50269763B570C854E5C09B7A"
    },
    { /* tv 279 */
      "secp256k1",
      "20",
      "4CE119C96E2FA357200B559B2F7DD5A5F02D5290AFF74B03F3E471B273211C97",
      "12BA26DCB10EC1625DA61FA10A844C676162948271D96967450288EE9233DC3A"
    },
    { /* tv 280 */
      "secp256k1",
      "112233445566778899",
      "A90CC3D3F3E146DAADFC74CA1372207CB4B725AE708CEF713A98EDD73D99EF29",
      "5A79D6B289610C68BC3B47F3D72F9788A26A06868B4D8E433E1E2AD76FB7DC76"
    },
    { /* tv 281 */
      "secp256k1",
      "112233445566778899112233445566778899",
      "E5A2636BCFD412EBF36EC45B19BFB68A1BC5F8632E678132B885F7DF99C5E9B3",
      "736C1CE161AE27B405CAFD2A7520370153C2C861AC51D6C1D5985D9606B45F39"
    },
    { /* tv 282 */
      "secp256k1",
      "28948022309329048855892746252171976963209391069768726095651290785379"
      "540373584",
      "A6B594B38FB3E77C6EDF78161FADE2041F4E09FD8497DB776E546C41567FEB3C",
      "71444009192228730CD8237A490FEBA2AFE3D27D7CC1136BC97E439D13330D55"
    },
    { /* tv 283 */
      "secp256k1",
      "57896044618658097711785492504343953926418782139537452191302581570759"
      "080747168",
      "00000000000000000000003B78CE563F89A0ED9414F5AA28AD0D96D6795F9C63",
      "3F3979BF72AE8202983DC989AEC7F2FF2ED91BDD69CE02FC0700CA100E59DDF3"
    },
    { /* tv 284 */
      "secp256k1",
      "86844066927987146567678238756515930889628173209306178286953872356138"
      "621120752",
      "E24CE4BEEE294AA6350FAA67512B99D388693AE4E7F53D19882A6EA169FC1CE1",
      "8B71E83545FC2B5872589F99D948C03108D36797C4DE363EBD3FF6A9E1A95B10"
    },
    { /* tv 285 */
      "secp256k1",
      "11579208923731619542357098500868790785283756427907490438260516314151"
      "8161494317",
      "4CE119C96E2FA357200B559B2F7DD5A5F02D5290AFF74B03F3E471B273211C97",
      "ED45D9234EF13E9DA259E05EF57BB3989E9D6B7D8E269698BAFD77106DCC1FF5"
    },
    { /* tv 286 */
      "secp256k1",
      "11579208923731619542357098500868790785283756427907490438260516314151"
      "8161494318",
      "2B4EA0A797A443D293EF5CFF444F4979F06ACFEBD7E86D277475656138385B6C",
      "7A17643FC86BA26C4CBCF7C4A5E379ECE5FE09F3AFD9689C4A8F37AA1A3F60B5"
    },
    { /* tv 287 */
      "secp256k1",
      "11579208923731619542357098500868790785283756427907490438260516314151"
      "8161494319",
      "5601570CB47F238D2B0286DB4A990FA0F3BA28D1A319F5E7CF55C2A2444DA7CC",
      "3EC93E23F34146CF161D67FBCA76CAE27E271F438C951D5E0AE6D1A074F9DED7"
    },
    { /* tv 288 */
      "secp256k1",
      "11579208923731619542357098500868790785283756427907490438260516314151"
      "8161494320",
      "DEFDEA4CDB677750A420FEE807EACF21EB9898AE79B9768766E4FAA04A2D4A34",
      "BDEE54F96B9CAE9716684F152D56C251312E0B5FB56A3F09304E660861A910B8"
    },
    { /* tv 289 */
      "secp256k1",
      "11579208923731619542357098500868790785283756427907490438260516314151"
      "8161494321",
      "E60FCE93B59E9EC53011AABC21C23E97B2A31369B87A5AE9C44EE89E2A6DEC0A",
      "081CAF8C661A6A6D624660CB0A86C8EFED6976E1BB2DC0F41E0CD330969E940E"
    },
    { /* tv 290 */
      "secp256k1",
      "11579208923731619542357098500868790785283756427907490438260516314151"
      "8161494322",
      "D7924D4F7D43EA965A465AE3095FF41131E5946F3C85F79E44ADBCF8E27E080E",
      "A7E1D78D57938D597C7BD13DD733921015BF50D427692C5A3AFB235F095D90D7"
    },
    { /* tv 291 */
      "secp256k1",
      "11579208923731619542357098500868790785283756427907490438260516314151"
      "8161494323",
      "499FDF9E895E719CFD64E67F07D38E3226AA7B63678949E6E49B241A60E823E4",
      "353D093B4AB17AAE6F0FBB1B584C2B9BB9BD863D85C06A4339A0BF2AFC5EBCD4"
    },
    { /* tv 292 */
      "secp256k1",
      "11579208923731619542357098500868790785283756427907490438260516314151"
      "8161494324",
      "F28773C2D975288BC7D1D205C3748651B075FBC6610E58CDDEEDDF8F19405AA8",
      "F54F6FD17277F5768A7DED149A3250B8C5E5F925ADE056E0D64A34AC24FC0EAE"
    },
    { /* tv 293 */
      "secp256k1",
      "11579208923731619542357098500868790785283756427907490438260516314151"
      "8161494325",
      "D01115D548E7561B15C38F004D734633687CF4419620095BC5B0F47070AFE85A",
      "560CB00237EA1F285749BAC81E8427EA86DC73A2265792AD94FAE4EB0BF9D908"
    },
    { /* tv 294 */
      "secp256k1",
      "11579208923731619542357098500868790785283756427907490438260516314151"
      "8161494326",
      "774AE7F858A9411E5EF4246B70C65AAC5649980BE5C17891BBEC17895DA008CB",
      "267B5FCD1494A1E6FDBC22A928484C9AC8D24E1D20062957CFE28B3536AC3614"
    },
    { /* tv 295 */
      "secp256k1",
      "11579208923731619542357098500868790785283756427907490438260516314151"
      "8161494327",
      "A0434D9E47F3C86235477C7B1AE6AE5D3442D49B1943C2B752A68E2A47E247C7",
      "76C545BDABE643D85C4938196C5DB3969086B3D127885EA6C3411AC3FC8C9358"
    },
    { /* tv 296 */
      "secp256k1",
      "11579208923731619542357098500868790785283756427907490438260516314151"
      "8161494328",
      "ACD484E2F0C7F65309AD178A9F559ABDE09796974C57E714C35F110DFC27CCBE",
      "33CC76DE4F5826029BC7F68E89C49E165227775BC8A071F0FA33D9D439B05FF8"
    },
    { /* tv 297 */
      "secp256k1",
      "11579208923731619542357098500868790785283756427907490438260516314151"
      "8161494329",
      "2F01E5E15CCA351DAFF3843FB70F3C2F0A1BDD05E5AF888A67784EF3E10A2A01",
      "A3B25758BEAC66B6D6C2F7D5ECD2EC4B3D1DEC2945A489E84A25D3479342132B"
    },
    { /* tv 298 */
      "secp256k1",
      "11579208923731619542357098500868790785283756427907490438260516314151"
      "8161494330",
      "5CBDF0646E5DB4EAA398F365F2EA7A0E3D419B7E0330E39CE92BDDEDCAC4F9BC",
      "951435BF45DAA69F5CE8729279E5AB2457EC2F47EC02184A5AF7D9D6F78D9755"
    },
    { /* tv 299 */
      "secp256k1",
      "11579208923731619542357098500868790785283756427907490438260516314151"
      "8161494331",
      "FFF97BD5755EEEA420453A14355235D382F6472F8568A18B2F057A1460297556",
      "51ED8885530449DF0C4169FE80BA3A9F217F0F09AE701B5FC378F3C84F8A0998"
    },
    { /* tv 300 */
      "secp256k1",
      "11579208923731619542357098500868790785283756427907490438260516314151"
      "8161494332",
      "2F8BDE4D1A07209355B4A7250A5C5128E88B84BDDC619AB7CBA8D569B240EFE4",
      "2753DDD9C91A1C292B24562259363BD90877D8E454F297BF235782C459539959"
    },
    { /* tv 301 */
      "secp256k1",
      "11579208923731619542357098500868790785283756427907490438260516314151"
      "8161494333",
      "E493DBF1C10D80F3581E4904930B1404CC6C13900EE0758474FA94ABE8C4CD13",
      "AE1266C15F2BAA48A9BD1DF6715AEBB7269851CC404201BF30168422B88C630D"
    },
    { /* tv 302 */
      "secp256k1",
      "11579208923731619542357098500868790785283756427907490438260516314151"
      "8161494334",
      "F9308A019258C31049344F85F89D5229B531C845836F99B08601F113BCE036F9",
      "C77084F09CD217EBF01CC819D5C80CA99AFF5666CB3DDCE4934602897B4715BD"
    },
    { /* tv 303 */
      "secp256k1",
      "11579208923731619542357098500868790785283756427907490438260516314151"
      "8161494335",
      "C6047F9441ED7D6D3045406E95C07CD85C778E4B8CEF3CA7ABAC09B95C709EE5",
      "E51E970159C23CC65C3A7BE6B99315110809CD9ACD992F1EDC9BCE55AF301705"
    },
    { /* tv 304 */
      "secp256k1",
      "11579208923731619542357098500868790785283756427907490438260516314151"
      "8161494336",
      "79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798",
      "B7C52588D95C3B9AA25B0403F1EEF75702E84BB7597AABE663B82F6F04EF2777"
    },

    { NULL, NULL, NULL, NULL }
  };
  gpg_error_t err;
  gcry_ctx_t ctx;
  gcry_mpi_t k = NULL, x = NULL, y = NULL;
  gcry_mpi_point_t G = NULL, Q = NULL;
  int idx;

  for (idx = 0; tv[idx].curve; idx++)
    {
      /* P-192 is not supported in fips mode */
      if (gcry_fips_mode_active())
        {
          if (!strcmp(tv[idx].curve, "NIST P-192"))
            {
	      static int once;
	      if (!once)
		info ("skipping %s in fips mode\n", tv[idx].curve);
	      once = 1;
              continue;
            }
          if (!strcmp(tv[idx].curve, "secp256k1"))
            {
	      static int once;
	      if (!once)
		info ("skipping %s in fips mode\n", tv[idx].curve);
	      once = 1;
              continue;
            }
        }

      err = gcry_mpi_ec_new (&ctx, NULL, tv[idx].curve);
      if (err)
        {
          fail ("tv[%d].'%s': can't create context: %s\n",
		idx, tv[idx].curve, gpg_strerror (err));
          return;
        }

      G = gcry_mpi_ec_get_point ("g", ctx, 1);
      if (!G)
	{
	  fail ("tv[%d].'%s': error getting point parameter 'g'\n",
		idx, tv[idx].curve);
          goto err;
	}

      if (tv[idx].k_base10)
	k = mpi_base10_scan (tv[idx].k_base10);
      else
	die ("tv[%d].'%s': missing 'k'\n",
		idx, tv[idx].curve);

      Q = gcry_mpi_point_new (0);
      x = gcry_mpi_new (0);
      y = gcry_mpi_new (0);

      gcry_mpi_ec_mul (Q, k, G, ctx);
      if (gcry_mpi_ec_get_affine (x, y, Q, ctx))
	{
	  fail ("tv[%d].'%s': failed to get affine coordinates\n",
		idx, tv[idx].curve);
	  goto err;
	}

      if (cmp_mpihex (x, tv[idx].qx) || cmp_mpihex (y, tv[idx].qy))
	{
	  fail ("tv[%d].'%s': sample point multiply failed:\n",
		idx, tv[idx].curve);
	  print_mpi ("          k", k);
	  print_mpi ("         Qx", x);
	  printf ("expected Qx: %s\n", tv[idx].qx);
	  print_mpi ("         Qy", y);
	  printf ("expected Qy: %s\n", tv[idx].qy);
	}

err:
      gcry_mpi_release (k);
      gcry_mpi_release (y);
      gcry_mpi_release (x);
      gcry_mpi_point_release (Q);
      gcry_mpi_point_release (G);
      gcry_ctx_release (ctx);
    }
}


static void
check_ec_mul_reduction (void)
{
  static struct
  {
    const char *curve;
    const char *scalar;
    const char *ux;
    const char *uy;
    const char *uz;
    const char *qx;
    const char *qy;
  } tv[] =
  {
    /* --- NIST P-192 --- */

    /* Bug report: https://dev.gnupg.org/T5510 */
    {
      "NIST P-192",
      "0FFFFFFFFFFFFFFFFFFFFFFFF99DEF836146BC9B1B4D22830",
      "000000000000000000000000000000000FFFFFFFFFFFFFF00",
      "030A893B61A4E76AF2B682FDB86E043C427C1AD2DFDABB62E",
      NULL,
      "000000000000000000000000000000000FFFFFFFFFFFFFF00",
      "0CF576C49E5B18950D497D024791FBC3AD83E52D2025449D1"
    },
    {
      "NIST P-192",
      "0FFFFFFFFFFFFFFFFFFFFFFFF99DEF836146BC9B1B4D22830",
      "000000000000000000000000000000000FFFFFFFFFFFFFF00",
      "0CF576C49E5B18950D497D024791FBC3AD83E52D2025449D1",
      NULL,
      "000000000000000000000000000000000FFFFFFFFFFFFFF00",
      "030A893B61A4E76AF2B682FDB86E043C427C1AD2DFDABB62E"
    },

    /* Test #1 for NIST P-192 fast reduction */
    {
      "NIST P-192",
      "09977FBFF49FFB1F115C20E3378819ACCFB2A2A19EF9D00C2",
      "06D9D789820A2C19237C96AD4B8D86B87FB49D4D6C728B84F",
      "0000000000000000000000000000000000000000000000001",
      NULL,
      "019298C549B3982415E831422E8FF991D25C589B214B3EC20",
      "09CF5FFD92F0218F704E1F5D4F888FEB1AE5C09674428D9FE"
    },
    {
      "NIST P-192",
      "01234567890ABCDEF01234567890ABCDEF01234567890ABCD",
      "019298C549B3982415E831422E8FF991D25C589B214B3EC20",
      "09CF5FFD92F0218F704E1F5D4F888FEB1AE5C09674428D9FE",
      NULL,
      "06D9D789820A2C19237C96AD4B8D86B87FB49D4D6C728B84F",
      "0000000000000000000000000000000000000000000000001"
    },

    /* Test #2 for NIST P-192 fast reduction */
    {
      "NIST P-192",
      "09977FBFF49FFB1F115C20E3378819ACCFB2A2A19EF9D00C2",
      "03DA9DE5C6BFA357FDAD6F1B64479BFF526480BF1647D1DDE",
      "0000000000000000000000000000000010000000000000001",
      NULL,
      "0FA56FFAE18551DEC7B10335F9E4D9844B11EEF9C3124B7CF",
      "06EC4073A143832560600767FDDC0499A1721BE8C2DEE9B68"
    },
    {
      "NIST P-192",
      "01234567890ABCDEF01234567890ABCDEF01234567890ABCD",
      "0FA56FFAE18551DEC7B10335F9E4D9844B11EEF9C3124B7CF",
      "06EC4073A143832560600767FDDC0499A1721BE8C2DEE9B68",
      NULL,
      "03DA9DE5C6BFA357FDAD6F1B64479BFF526480BF1647D1DDE",
      "0000000000000000000000000000000010000000000000001"
    },

    /* Test #3 for NIST P-192 fast reduction */
    {
      "NIST P-192",
      "09977FBFF49FFB1F115C20E3378819ACCFB2A2A19EF9D00C2",
      "06D9D789820A2C19237C96AD4B8D86B87FB49D4D6C728B84F",
      "0FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFFFFFFFFFE",
      NULL,
      "019298C549B3982415E831422E8FF991D25C589B214B3EC20",
      "0630A0026D0FDE708FB1E0A2B0777014D51A3F698BBD72601"
    },
    {
      "NIST P-192",
      "01234567890ABCDEF01234567890ABCDEF01234567890ABCD",
      "019298C549B3982415E831422E8FF991D25C589B214B3EC20",
      "0630A0026D0FDE708FB1E0A2B0777014D51A3F698BBD72601",
      NULL,
      "06D9D789820A2C19237C96AD4B8D86B87FB49D4D6C728B84F",
      "0FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFFFFFFFFFE"
    },

    /* Test #4 for NIST P-192 fast reduction */
    {
      "NIST P-192",
      "0FFFFFFFFFFFFFFFFFFFFFFFF99DEF836146BC9B1B4D22830",
      "06D9D789820A2C19237C96AD4B8D86B87FB49D4D6C728B84F",
      "0000000000000000000000000000000000000000000000001",
      NULL,
      "06D9D789820A2C19237C96AD4B8D86B87FB49D4D6C728B84F",
      "0FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFFFFFFFFFE"
    },
    {
      "NIST P-192",
      "0FFFFFFFFFFFFFFFFFFFFFFFF99DEF836146BC9B1B4D22830",
      "06D9D789820A2C19237C96AD4B8D86B87FB49D4D6C728B84F",
      "0FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFFFFFFFFFFFE",
      NULL,
      "06D9D789820A2C19237C96AD4B8D86B87FB49D4D6C728B84F",
      "0000000000000000000000000000000000000000000000001"
    },

    /* --- NIST P-224 --- */

    /* Test #1 for NIST P-224 fast reduction */
    {
      "NIST P-224",
      "05793BE916A89FC5475A3267BE6F3530901AEC6010E8C2EA2D79F9EE8",
      "03B5889352DDF7468BF8C0729212AA1B2A3FCB1A844B8BE91ABB753D5",
      "000000000000000000000000000000000000000000000000000000001",
      NULL,
      "0B5457C849D65FBA90C724C9528F500C10930B744F1ECFF60015238D7",
      "05BBF4A452ADCAFB15484C0E3AAEBF37153543DA0785EBB61A8F2A1B9"
    },
    {
      "NIST P-224",
      "01234567890ABCDEF01234567890ABCDEF01234567890ABCDEF012345",
      "0B5457C849D65FBA90C724C9528F500C10930B744F1ECFF60015238D7",
      "05BBF4A452ADCAFB15484C0E3AAEBF37153543DA0785EBB61A8F2A1B9",
      NULL,
      "03B5889352DDF7468BF8C0729212AA1B2A3FCB1A844B8BE91ABB753D5",
      "000000000000000000000000000000000000000000000000000000001"
    },

    /* Test #2 for NIST P-224 fast reduction */
    {
      "NIST P-224",
      "05793BE916A89FC5475A3267BE6F3530901AEC6010E8C2EA2D79F9EE8",
      "02FEB134F401D499197917C3470242239093606F5B8EA463A763C3AC4",
      "0C0000000000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFF",
      NULL,
      "0384BD56C5E3FA9C109C841485CCB0B8C90FB5928A87388A658E164AA",
      "04701B697F333FD522787DF61F3CDB3F964FF43F288858BC4524CA3CD"
    },
    {
      "NIST P-224",
      "01234567890ABCDEF01234567890ABCDEF01234567890ABCDEF012345",
      "0384BD56C5E3FA9C109C841485CCB0B8C90FB5928A87388A658E164AA",
      "04701B697F333FD522787DF61F3CDB3F964FF43F288858BC4524CA3CD",
      NULL,
      "02FEB134F401D499197917C3470242239093606F5B8EA463A763C3AC4",
      "0C0000000000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFF"
    },

    /* Test #3 for NIST P-224 fast reduction */
    {
      "NIST P-224",
      "05793BE916A89FC5475A3267BE6F3530901AEC6010E8C2EA2D79F9EE8",
      "03B5889352DDF7468BF8C0729212AA1B2A3FCB1A844B8BE91ABB753D5",
      "0FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000",
      NULL,
      "0B5457C849D65FBA90C724C9528F500C10930B744F1ECFF60015238D7",
      "0A440B5BAD523504EAB7B3F1C55140C8DACABC25F87A1449E570D5E48"
    },
    {
      "NIST P-224",
      "01234567890ABCDEF01234567890ABCDEF01234567890ABCDEF012345",
      "0B5457C849D65FBA90C724C9528F500C10930B744F1ECFF60015238D7",
      "0A440B5BAD523504EAB7B3F1C55140C8DACABC25F87A1449E570D5E48",
      NULL,
      "03B5889352DDF7468BF8C0729212AA1B2A3FCB1A844B8BE91ABB753D5",
      "0FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000"
    },

    /* Test #4 for NIST P-224 fast reduction */
    {
      "NIST P-224",
      "0FFFFFFFFFFFFFFFFFFFFFFFFFFFF16A2E0B8F03E13DD29455C5C2A3C",
      "03B5889352DDF7468BF8C0729212AA1B2A3FCB1A844B8BE91ABB753D5",
      "000000000000000000000000000000000000000000000000000000001",
      NULL,
      "03B5889352DDF7468BF8C0729212AA1B2A3FCB1A844B8BE91ABB753D5",
      "0FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000"
    },
    {
      "NIST P-224",
      "0FFFFFFFFFFFFFFFFFFFFFFFFFFFF16A2E0B8F03E13DD29455C5C2A3C",
      "03B5889352DDF7468BF8C0729212AA1B2A3FCB1A844B8BE91ABB753D5",
      "0FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000000",
      NULL,
      "03B5889352DDF7468BF8C0729212AA1B2A3FCB1A844B8BE91ABB753D5",
      "000000000000000000000000000000000000000000000000000000001"
    },

    /* --- NIST P-256 --- */

    /* Bug report: https://dev.gnupg.org/T5510 */
    {
      "NIST P-256",
      "00000000000FF0000000400000000000000000000005D00003277002000010000",
      "00000FFFFFFFF0000000000000000000000000000000000000000000000000000",
      "0CFE26D107A5134D6FEB38CE3577075BDC7AA70FF7523D3B203C8A973F2D3DC8E",
      NULL,
      "0FD351B304AD50F36153D8193C4BBF7D4C3BEE26E5AF52A9C70133EDFA62C273E",
      "005DA8312615436E9C81B5B0624E68667233ACE6307AFC8056EAE85049CA63226"
    },
    {
      "NIST P-256",
      "096D8332E8F1265354CB7D213EF7809BA2D9639DFA634F321420C238CD2ED1830",
      "0FD351B304AD50F36153D8193C4BBF7D4C3BEE26E5AF52A9C70133EDFA62C273E",
      "005DA8312615436E9C81B5B0624E68667233ACE6307AFC8056EAE85049CA63226",
      NULL,
      "00000FFFFFFFF0000000000000000000000000000000000000000000000000000",
      "0CFE26D107A5134D6FEB38CE3577075BDC7AA70FF7523D3B203C8A973F2D3DC8E"
    },

    /* Bug report: https://dev.gnupg.org/T5510 */
    {
      "NIST P-256",
      "02020FF2020202020202020202020202020202020202020202020202020202020",
      "0555555FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
      "073A865E2E128733884FB82CE625ADE822F7D8A59A4DCC09266966CF1BF082856",
      NULL,
      "05549408909DD3E772D7D669F8FBA2248D334B54BE3D18833223D944A328948C7",
      "06198AC3B29712256DCD9CE1A09471F04267684E1EDD45910D61D0B7847DB2D58"
    },

    /* Test #1 for NIST P-256 fast reduction */
    {
      "NIST P-256",
      "0F14DE6DA4CBA6F4C7B7E988EEF4D168088B6300BAB207054AA24E6D3019D5A23",
      "08D0177EBAB9C6E9E10DB6DD095DBAC0D6375E8A97B70F611875D877F0069D2C7",
      "00000000000000000000000000000000000000000000000000000000000000001",
      NULL,
      "0688856989B61877BC62ED4D2EE8E0FCA4588D90C2F9A282FA4AD6ACCFBAA95DF",
      "0D2B9F7FA3982EE114FBBA108E132CECB5A605520661B3C4F7CF2906714B400E2"
    },
    {
      "NIST P-256",
      "01234567890ABCDEF01234567890ABCDEF01234567890ABCDEF01234567890ABC",
      "0688856989B61877BC62ED4D2EE8E0FCA4588D90C2F9A282FA4AD6ACCFBAA95DF",
      "0D2B9F7FA3982EE114FBBA108E132CECB5A605520661B3C4F7CF2906714B400E2",
      NULL,
      "08D0177EBAB9C6E9E10DB6DD095DBAC0D6375E8A97B70F611875D877F0069D2C7",
      "00000000000000000000000000000000000000000000000000000000000000001"
    },

    /* Test #2 for NIST P-256 fast reduction */
    {
      "NIST P-256",
      "0F14DE6DA4CBA6F4C7B7E988EEF4D168088B6300BAB207054AA24E6D3019D5A23",
      "0F650B5048E7DE9587399E64274C008A4CA0992BA29DC05F7E51243D34C2949EC",
      "0FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000001",
      NULL,
      "0BEF9CCBF964B86724C97CF35D3890CAB3E9802EDAD1A3D9070695FD04C640087",
      "0E540F49BAC4AA879B7A1D7C40CE03F7097A910B77F098DA2F7C441407BAD119B"
    },
    {
      "NIST P-256",
      "01234567890ABCDEF01234567890ABCDEF01234567890ABCDEF01234567890ABC",
      "0BEF9CCBF964B86724C97CF35D3890CAB3E9802EDAD1A3D9070695FD04C640087",
      "0E540F49BAC4AA879B7A1D7C40CE03F7097A910B77F098DA2F7C441407BAD119B",
      NULL,
      "0F650B5048E7DE9587399E64274C008A4CA0992BA29DC05F7E51243D34C2949EC",
      "0FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000001"
    },

    /* Test #3 for NIST P-256 fast reduction */
    {
      "NIST P-256",
      "0F14DE6DA4CBA6F4C7B7E988EEF4D168088B6300BAB207054AA24E6D3019D5A23",
      "08D0177EBAB9C6E9E10DB6DD095DBAC0D6375E8A97B70F611875D877F0069D2C7",
      "0FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFE",
      NULL,
      "0688856989B61877BC62ED4D2EE8E0FCA4588D90C2F9A282FA4AD6ACCFBAA95DF",
      "02D460804C67D11EFB0445EF71ECD3134A59FAAE099E4C3B0830D6F98EB4BFF1D"
    },
    {
      "NIST P-256",
      "01234567890ABCDEF01234567890ABCDEF01234567890ABCDEF01234567890ABC",
      "0688856989B61877BC62ED4D2EE8E0FCA4588D90C2F9A282FA4AD6ACCFBAA95DF",
      "02D460804C67D11EFB0445EF71ECD3134A59FAAE099E4C3B0830D6F98EB4BFF1D",
      NULL,
      "08D0177EBAB9C6E9E10DB6DD095DBAC0D6375E8A97B70F611875D877F0069D2C7",
      "0FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFE"
    },

    /* Test #4 for NIST P-256 fast reduction */
    {
      "NIST P-256",
      "0FD1AEFA76594FC4D14169064B1AE89B2224AEB9F9C314E21BDCFEB5F2AFB3B81",
      "0FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000001",
      "0AFE89640E5A59FE8F69ADB58DB0059E522E5CC1D0CEDAB7AC6F8455F49AADED2",
      NULL,
      "0063257268860D4F13CFC6071CE09E3D1BD8B77B08A0714CFF42880F430A4F1CE",
      "09665C75111B793CE3BF6E2BF45F9B55E065D22F93C4063A79DF8C9D139E83173"
    },
    {
      "NIST P-256",
      "0C0DE4C0FFEE1111C0DE4C0FFEE1111C0DE4C0FFEE1111C0DE4C0FFEE1111C0DE",
      "0063257268860D4F13CFC6071CE09E3D1BD8B77B08A0714CFF42880F430A4F1CE",
      "09665C75111B793CE3BF6E2BF45F9B55E065D22F93C4063A79DF8C9D139E83173",
      NULL,
      "0FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF000000000000000000000001",
      "0AFE89640E5A59FE8F69ADB58DB0059E522E5CC1D0CEDAB7AC6F8455F49AADED2"
    },

    /* Test #5 for NIST P-256 fast reduction */
    {
      "NIST P-256",
      "0FFFFFFFF00000000FFFFFFFFFFFFFFFFBCE6FAADA7179E84F3B9CAC2FC632550",
      "08D0177EBAB9C6E9E10DB6DD095DBAC0D6375E8A97B70F611875D877F0069D2C7",
      "00000000000000000000000000000000000000000000000000000000000000001",
      NULL,
      "08D0177EBAB9C6E9E10DB6DD095DBAC0D6375E8A97B70F611875D877F0069D2C7",
      "0FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFE"
    },
    {
      "NIST P-256",
      "0FFFFFFFF00000000FFFFFFFFFFFFFFFFBCE6FAADA7179E84F3B9CAC2FC632550",
      "08D0177EBAB9C6E9E10DB6DD095DBAC0D6375E8A97B70F611875D877F0069D2C7",
      "0FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFE",
      NULL,
      "08D0177EBAB9C6E9E10DB6DD095DBAC0D6375E8A97B70F611875D877F0069D2C7",
      "00000000000000000000000000000000000000000000000000000000000000001"
    },

    /* --- NIST P-384 --- */

    /* Bug report: https://dev.gnupg.org/T5510 */
    {
      "NIST P-384",
      "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC7634D81F4372DDF"
      "581A0DB248B0A77AECEC196ACCC52972",
      "0000000000000000000000000000000000000000000000000000000000000000"
      "0000000000FFFFFFFFFFFFFFFFFFFFFC",
      "1B0D6F8FB7F2DE5B8875645B64042AE20F119F3E1CFEFC0215857EEAE5F4A8FC"
      "A737057D69A42C44D958E7CFCC77CE6B",
      NULL,
      "0000000000000000000000000000000000000000000000000000000000000000"
      "0000000000FFFFFFFFFFFFFFFFFFFFFC",
      "E4F29070480D21A4778A9BA49BFBD51DF0EE60C1E30103FDEA7A81151A0B5702"
      "58C8FA81965BD3BB26A7183133883194"
    },
    {
      "NIST P-384",
      "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC7634D81F4372DDF"
      "581A0DB248B0A77AECEC196ACCC52972",
      "0000000000000000000000000000000000000000000000000000000000000000"
      "0000000000FFFFFFFFFFFFFFFFFFFFFC",
      "E4F29070480D21A4778A9BA49BFBD51DF0EE60C1E30103FDEA7A81151A0B5702"
      "58C8FA81965BD3BB26A7183133883194",
      NULL,
      "0000000000000000000000000000000000000000000000000000000000000000"
      "0000000000FFFFFFFFFFFFFFFFFFFFFC",
      "1B0D6F8FB7F2DE5B8875645B64042AE20F119F3E1CFEFC0215857EEAE5F4A8FC"
      "A737057D69A42C44D958E7CFCC77CE6B"
    },

    /* Test #1 for NIST P-384 fast reduction */
    {
      "NIST P-384",
      "0739C89BDC9F071F1DC8391A52A0764EE0E15387DBB56E4FDAD647993F52C9121"
      "D2DCE779834057EEAE390D113E337DDB",
      "02261B2BF605C22F2F3AEF6338719B2C486388AD5240719A5257315969EF01BA2"
      "7F0A104C89704773A81FDABEE6AB5C78",
      "00000000000000000000000000000000000000000000000000000000000000000"
      "00000000000000000000000000000001",
      NULL,
      "041CF9FB4C86482B7EC4850FB8AA31771D5F2D3944168B16D47C307468929A860"
      "B21334F09C2F33AAA565E7190225C793",
      "00512D4C3F17BA90EEDBCC74C7CBA4410A4F68C8FC0EFB9B981501B454F574744"
      "8947DFC29E4BAEE30439A84D455627C6"
    },
    {
      "NIST P-384",
      "01234567890ABCDEF01234567890ABCDEF01234567890ABCDEF01234567890ABC"
      "DEF01234567890ABCDEF01234567890A",
      "041CF9FB4C86482B7EC4850FB8AA31771D5F2D3944168B16D47C307468929A860"
      "B21334F09C2F33AAA565E7190225C793",
      "00512D4C3F17BA90EEDBCC74C7CBA4410A4F68C8FC0EFB9B981501B454F574744"
      "8947DFC29E4BAEE30439A84D455627C6",
      NULL,
      "02261B2BF605C22F2F3AEF6338719B2C486388AD5240719A5257315969EF01BA2"
      "7F0A104C89704773A81FDABEE6AB5C78",
      "00000000000000000000000000000000000000000000000000000000000000000"
      "00000000000000000000000000000001"
    },

    /* Test #2 for NIST P-384 fast reduction */
    {
      "NIST P-384",
      "0739C89BDC9F071F1DC8391A52A0764EE0E15387DBB56E4FDAD647993F52C9121"
      "D2DCE779834057EEAE390D113E337DDB",
      "0C4A2DEA4D2A725EAEEFA6CA6FA301155510309F46EA1C70C909B988B49E1D612"
      "468051EB758869259D1BF892E0A555C2",
      "00000000000000000000000000000000000000000000000000000000000000001"
      "00000000FFFFFFFFFFFFFFFF00000001",
      NULL,
      "0ED97AD545DB70C379C94BDA35926921F26FA9B9B338D32D8F2A163DBBEC5CFAF"
      "600830D133A14C0B599B53E57D206DE2",
      "069B72397B725CA8FA7A59173D4F588273C8303B5F6A5AFD8ACBD6B56CC7CCF32"
      "B6FCE2DBB991DE4C6E9CCFAF21B8ECCF"
    },
    {
      "NIST P-384",
      "01234567890ABCDEF01234567890ABCDEF01234567890ABCDEF01234567890ABC"
      "DEF01234567890ABCDEF01234567890A",
      "0ED97AD545DB70C379C94BDA35926921F26FA9B9B338D32D8F2A163DBBEC5CFAF"
      "600830D133A14C0B599B53E57D206DE2",
      "069B72397B725CA8FA7A59173D4F588273C8303B5F6A5AFD8ACBD6B56CC7CCF32"
      "B6FCE2DBB991DE4C6E9CCFAF21B8ECCF",
      NULL,
      "0C4A2DEA4D2A725EAEEFA6CA6FA301155510309F46EA1C70C909B988B49E1D612"
      "468051EB758869259D1BF892E0A555C2",
      "00000000000000000000000000000000000000000000000000000000000000001"
      "00000000FFFFFFFFFFFFFFFF00000001"
    },

    /* Test #3 for NIST P-384 fast reduction */
    {
      "NIST P-384",
      "0739C89BDC9F071F1DC8391A52A0764EE0E15387DBB56E4FDAD647993F52C9121"
      "D2DCE779834057EEAE390D113E337DDB",
      "05ADE55A6BD4FA27FCA00D1C38653843E3E8421E018DFC7DDAD34076C4D41A621"
      "980D62417D12330DAE5948C2C6D7D347",
      "0FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE"
      "FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFD",
      NULL,
      "0A85779C7427F50E92166268399F4ABEDDAA7B8866B0A495ED603BE80A8900786"
      "DD287FF5C28708D039AF02DD1FC197CC",
      "0CB97C2A5822A86B82DFE300E7F6278A087506A718EBD4FF317D7529581E98470"
      "56FCE58652AA1B6D6FBB62851099A9FF"
    },
    {
      "NIST P-384",
      "01234567890ABCDEF01234567890ABCDEF01234567890ABCDEF01234567890ABC"
      "DEF01234567890ABCDEF01234567890A",
      "0A85779C7427F50E92166268399F4ABEDDAA7B8866B0A495ED603BE80A8900786"
      "DD287FF5C28708D039AF02DD1FC197CC",
      "0CB97C2A5822A86B82DFE300E7F6278A087506A718EBD4FF317D7529581E98470"
      "56FCE58652AA1B6D6FBB62851099A9FF",
      NULL,
      "05ADE55A6BD4FA27FCA00D1C38653843E3E8421E018DFC7DDAD34076C4D41A621"
      "980D62417D12330DAE5948C2C6D7D347",
      "0FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE"
      "FFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFD"
    },

    /* Test #4 for NIST P-384 fast reduction */
    {
      "NIST P-384",
      "0739C89BDC9F071F1DC8391A52A0764EE0E15387DBB56E4FDAD647993F52C9121"
      "D2DCE779834057EEAE390D113E337DDB",
      "02261B2BF605C22F2F3AEF6338719B2C486388AD5240719A5257315969EF01BA2"
      "7F0A104C89704773A81FDABEE6AB5C78",
      "0FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE"
      "FFFFFFFF0000000000000000FFFFFFFE",
      NULL,
      "041CF9FB4C86482B7EC4850FB8AA31771D5F2D3944168B16D47C307468929A860"
      "B21334F09C2F33AAA565E7190225C793",
      "0FAED2B3C0E8456F1124338B38345BBEF5B0973703F1046467EAFE4BAB0A8B8BA"
      "76B8203C61B4511CFBC657B3BAA9D839"
    },
    {
      "NIST P-384",
      "01234567890ABCDEF01234567890ABCDEF01234567890ABCDEF01234567890ABC"
      "DEF01234567890ABCDEF01234567890A",
      "041CF9FB4C86482B7EC4850FB8AA31771D5F2D3944168B16D47C307468929A860"
      "B21334F09C2F33AAA565E7190225C793",
      "0FAED2B3C0E8456F1124338B38345BBEF5B0973703F1046467EAFE4BAB0A8B8BA"
      "76B8203C61B4511CFBC657B3BAA9D839",
      NULL,
      "02261B2BF605C22F2F3AEF6338719B2C486388AD5240719A5257315969EF01BA2"
      "7F0A104C89704773A81FDABEE6AB5C78",
      "0FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE"
      "FFFFFFFF0000000000000000FFFFFFFE"
    },

    /* Test #5 for NIST P-384 fast reduction */
    {
      "NIST P-384",
      "0FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC7634D81F4372DDF"
      "581A0DB248B0A77AECEC196ACCC52972",
      "02261B2BF605C22F2F3AEF6338719B2C486388AD5240719A5257315969EF01BA2"
      "7F0A104C89704773A81FDABEE6AB5C78",
      "0000000000000000000000000000000000000000000000000000000000000000"
      "00000000000000000000000000000001",
      NULL,
      "02261B2BF605C22F2F3AEF6338719B2C486388AD5240719A5257315969EF01BA2"
      "7F0A104C89704773A81FDABEE6AB5C78",
      "0FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE"
      "FFFFFFFF0000000000000000FFFFFFFE"
    },
    {
      "NIST P-384",
      "0FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFC7634D81F4372DDF"
      "581A0DB248B0A77AECEC196ACCC52972",
      "02261B2BF605C22F2F3AEF6338719B2C486388AD5240719A5257315969EF01BA2"
      "7F0A104C89704773A81FDABEE6AB5C78",
      "0FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE"
      "FFFFFFFF0000000000000000FFFFFFFE",
      NULL,
      "02261B2BF605C22F2F3AEF6338719B2C486388AD5240719A5257315969EF01BA2"
      "7F0A104C89704773A81FDABEE6AB5C78",
      "0000000000000000000000000000000000000000000000000000000000000000"
      "00000000000000000000000000000001",
    },

    /* --- NIST P-521 --- */

    /* Test #1 for NIST P-521 fast reduction */
    {
      "NIST P-521",
      "014395758AE77FEE059DA0B5ABC00C374ACA51984B95451C08AD8309DC8CEFB9F"
      "0B7D03950131E54E63319D4592D679C1F8CFD610289F7B791D5C1C09B7BC5122195",
      "00D9CB7A32DAB342F863EDB340F3EA61DDF833E755CE66BB1A918A42714BA05BC"
      "DF4FF10994F616A9D80CD0B48B326E3A8A2A8F5634D824875B6E71FB7CDDD7B5018",
      "00000000000000000000000000000000000000000000000000000000000000000"
      "0000000000000000000000000000000000000000000000000000000000000000001",
      NULL,
      "0091EC67F53DC1C752724942D6B94A3D66D91C16C997F42524400D3CD10074C07"
      "FF6F7981A44292B25B478AE551DE980999404EC9221FD3FACE26C40FE783576EF1B",
      "00534EE41FFDDCE7E171AF630593437896A4C0344900671D1886ED21EF61AC26D"
      "A3DF4C6D7F1330B3EA25ABB6CA2127A605A650F2A6E916EAB757FE8B43116227FA1"
    },
    {
      "NIST P-521",
      "01234567890ABCDEF01234567890ABCDEF01234567890ABCDEF01234567890ABC"
      "DEF01234567890ABCDEF01234567890ABCDEF01234567890ABCDEF01234567890AB",
      "0091EC67F53DC1C752724942D6B94A3D66D91C16C997F42524400D3CD10074C07"
      "FF6F7981A44292B25B478AE551DE980999404EC9221FD3FACE26C40FE783576EF1B",
      "00534EE41FFDDCE7E171AF630593437896A4C0344900671D1886ED21EF61AC26D"
      "A3DF4C6D7F1330B3EA25ABB6CA2127A605A650F2A6E916EAB757FE8B43116227FA1",
      NULL,
      "00D9CB7A32DAB342F863EDB340F3EA61DDF833E755CE66BB1A918A42714BA05BC"
      "DF4FF10994F616A9D80CD0B48B326E3A8A2A8F5634D824875B6E71FB7CDDD7B5018",
      "00000000000000000000000000000000000000000000000000000000000000000"
      "0000000000000000000000000000000000000000000000000000000000000000001"
    },

    /* Test #2 for NIST P-521 fast reduction */
    {
      "NIST P-521",
      "014395758AE77FEE059DA0B5ABC00C374ACA51984B95451C08AD8309DC8CEFB9F"
      "0B7D03950131E54E63319D4592D679C1F8CFD610289F7B791D5C1C09B7BC5122195",
      "01A255D0D3B19B08BE171002C6D73096CA0E41498E805ABF61016EBF1A41E8018"
      "D5F876D2870BFF35AA254BE8D6A303A79C4E4A3427AA4BDF8EE59DCAC22F4C83CD6",
      "01000000000000000000000000000000000000000000000000000000000000000"
      "0000000000000000000000000000000000000000000000000000000000000000003",
      NULL,
      "00CEA3C8CE463F76789E7B0FCBEE5253E64BFA0FCBE101944CEFFC04D1786FF88"
      "C9EC5650BFAC13D4037C34064E6E0CE00AA666EAC026F7EE4CE6F7805A10AD9C743",
      "01F2D67A2DD50C916C48011BEE7BADFA488CD8C4F8BD69064C8BB454579E7B2FB"
      "EDE9B52418239251C5B81638970916D1B3CACD25487927978CE7B0CD2A25916F9FE"
    },
    {
      "NIST P-521",
      "01234567890ABCDEF01234567890ABCDEF01234567890ABCDEF01234567890ABC"
      "DEF01234567890ABCDEF01234567890ABCDEF01234567890ABCDEF01234567890AB",
      "00CEA3C8CE463F76789E7B0FCBEE5253E64BFA0FCBE101944CEFFC04D1786FF88"
      "C9EC5650BFAC13D4037C34064E6E0CE00AA666EAC026F7EE4CE6F7805A10AD9C743",
      "01F2D67A2DD50C916C48011BEE7BADFA488CD8C4F8BD69064C8BB454579E7B2FB"
      "EDE9B52418239251C5B81638970916D1B3CACD25487927978CE7B0CD2A25916F9FE",
      NULL,
      "01A255D0D3B19B08BE171002C6D73096CA0E41498E805ABF61016EBF1A41E8018"
      "D5F876D2870BFF35AA254BE8D6A303A79C4E4A3427AA4BDF8EE59DCAC22F4C83CD6",
      "01000000000000000000000000000000000000000000000000000000000000000"
      "0000000000000000000000000000000000000000000000000000000000000000003"
    },

    /* Test #3 for NIST P-521 fast reduction */
    {
      "NIST P-521",
      "014395758AE77FEE059DA0B5ABC00C374ACA51984B95451C08AD8309DC8CEFB9F"
      "0B7D03950131E54E63319D4592D679C1F8CFD610289F7B791D5C1C09B7BC5122195",
      "00D9CB7A32DAB342F863EDB340F3EA61DDF833E755CE66BB1A918A42714BA05BC"
      "DF4FF10994F616A9D80CD0B48B326E3A8A2A8F5634D824875B6E71FB7CDDD7B5018",
      "01FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
      "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE",
      NULL,
      "0091EC67F53DC1C752724942D6B94A3D66D91C16C997F42524400D3CD10074C07"
      "FF6F7981A44292B25B478AE551DE980999404EC9221FD3FACE26C40FE783576EF1B",
      "01ACB11BE00223181E8E509CFA6CBC87695B3FCBB6FF98E2E77912DE109E53D92"
      "5C20B39280ECCF4C15DA544935DED859FA59AF0D5916E91548A80174BCEE9DD805E"
    },
    {
      "NIST P-521",
      "01234567890ABCDEF01234567890ABCDEF01234567890ABCDEF01234567890ABC"
      "DEF01234567890ABCDEF01234567890ABCDEF01234567890ABCDEF01234567890AB",
      "0091EC67F53DC1C752724942D6B94A3D66D91C16C997F42524400D3CD10074C07"
      "FF6F7981A44292B25B478AE551DE980999404EC9221FD3FACE26C40FE783576EF1B",
      "01ACB11BE00223181E8E509CFA6CBC87695B3FCBB6FF98E2E77912DE109E53D92"
      "5C20B39280ECCF4C15DA544935DED859FA59AF0D5916E91548A80174BCEE9DD805E",
      NULL,
      "00D9CB7A32DAB342F863EDB340F3EA61DDF833E755CE66BB1A918A42714BA05BC"
      "DF4FF10994F616A9D80CD0B48B326E3A8A2A8F5634D824875B6E71FB7CDDD7B5018",
      "01FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
      "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE"
    },

    /* Test #4 for NIST P-521 fast reduction */
    {
      "NIST P-521",
      "01FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
      "FFA51868783BF2F966B7FCC0148F709A5D03BB5C9B8899C47AEBB6FB71E91386408",
      "00D9CB7A32DAB342F863EDB340F3EA61DDF833E755CE66BB1A918A42714BA05BC"
      "DF4FF10994F616A9D80CD0B48B326E3A8A2A8F5634D824875B6E71FB7CDDD7B5018",
      "00000000000000000000000000000000000000000000000000000000000000000"
      "0000000000000000000000000000000000000000000000000000000000000000001",
      NULL,
      "00D9CB7A32DAB342F863EDB340F3EA61DDF833E755CE66BB1A918A42714BA05BC"
      "DF4FF10994F616A9D80CD0B48B326E3A8A2A8F5634D824875B6E71FB7CDDD7B5018",
      "01FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
      "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE"
    },
    {
      "NIST P-521",
      "01FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
      "FFA51868783BF2F966B7FCC0148F709A5D03BB5C9B8899C47AEBB6FB71E91386408",
      "00D9CB7A32DAB342F863EDB340F3EA61DDF833E755CE66BB1A918A42714BA05BC"
      "DF4FF10994F616A9D80CD0B48B326E3A8A2A8F5634D824875B6E71FB7CDDD7B5018",
      "01FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
      "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE",
      NULL,
      "00D9CB7A32DAB342F863EDB340F3EA61DDF833E755CE66BB1A918A42714BA05BC"
      "DF4FF10994F616A9D80CD0B48B326E3A8A2A8F5634D824875B6E71FB7CDDD7B5018",
      "00000000000000000000000000000000000000000000000000000000000000000"
      "0000000000000000000000000000000000000000000000000000000000000000001"
    },

    /* --- secp256k1 --- */

    /* Test #1 for secp256k1 fast reduction */
    {
      "secp256k1",
      "0FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364140",
      "0CBB0DEAB125754F1FDB2038B0434ED9CB3FB53AB735391129994A535D925F673",
      "00000000000000000000000000000000000000000000000000000000000000001",
      NULL,
      "0CBB0DEAB125754F1FDB2038B0434ED9CB3FB53AB735391129994A535D925F673",
      "0FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2E"
    },
    {
      "secp256k1",
      "0FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364140",
      "0CBB0DEAB125754F1FDB2038B0434ED9CB3FB53AB735391129994A535D925F673",
      "0FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2E",
      NULL,
      "0CBB0DEAB125754F1FDB2038B0434ED9CB3FB53AB735391129994A535D925F673",
      "00000000000000000000000000000000000000000000000000000000000000001"
    },

    /* Test #2 for secp256k1 fast reduction */
    {
      "secp256k1",
      "0D1C9BD73010C07E357CBBD709520D081A5423846B5860C97ED2553F765A8475A",
      "0CBB0DEAB125754F1FDB2038B0434ED9CB3FB53AB735391129994A535D925F673",
      "0FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2E",
      NULL,
      "0AC71D7AB263AAD9929E3324B857C31FF3FF3E90571C8BE45AB7D5833B1A19533",
      "0CA4B80E756494A1897628246243770B3BE1AD0EEE79581D8C8EE80F053CBB44A"
    },
    {
      "secp256k1",
      "01234567890ABCDEF01234567890ABCDEF01234567890ABCDEF01234567890ABC",
      "0AC71D7AB263AAD9929E3324B857C31FF3FF3E90571C8BE45AB7D5833B1A19533",
      "0CA4B80E756494A1897628246243770B3BE1AD0EEE79581D8C8EE80F053CBB44A",
      NULL,
      "0CBB0DEAB125754F1FDB2038B0434ED9CB3FB53AB735391129994A535D925F673",
      "0FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2E"
    },

    /* Test #3 for secp256k1 fast reduction */
    {
      "secp256k1",
      "0D1C9BD73010C07E357CBBD709520D081A5423846B5860C97ED2553F765A8475A",
      "0A7E45D967E07558AE405F7F571ADD1ACF966F3619FD4F2A9708EEB766B98F423",
      "080000000000000000000000000000000000000000000000000000001000003D1",
      NULL,
      "0173E9E40BA3CCD35E4393C219A15C375783112C4C965C70BC043E8B17A342E79",
      "0AAA2D3810F2BB72D06FF8CA3B19027F4450CC73C0338F6379D6006CC239B1CD6"
    },
    {
      "secp256k1",
      "01234567890ABCDEF01234567890ABCDEF01234567890ABCDEF01234567890ABC",
      "0173E9E40BA3CCD35E4393C219A15C375783112C4C965C70BC043E8B17A342E79",
      "0AAA2D3810F2BB72D06FF8CA3B19027F4450CC73C0338F6379D6006CC239B1CD6",
      NULL,
      "0A7E45D967E07558AE405F7F571ADD1ACF966F3619FD4F2A9708EEB766B98F423",
      "080000000000000000000000000000000000000000000000000000001000003D1"
    },

    { NULL, NULL, NULL, NULL, NULL, NULL }
  };
  gpg_error_t err;
  gcry_ctx_t ctx;
  gcry_mpi_t scalar, ux, uy, uz, x, y;
  gcry_mpi_point_t U, Q;
  int idx;

  for (idx = 0; tv[idx].curve; idx++)
    {
      /* P-192 is not supported in fips mode */
      if (gcry_fips_mode_active())
        {
          if (!strcmp(tv[idx].curve, "NIST P-192"))
            {
	      static int once;
	      if (!once)
		info ("skipping %s in fips mode\n", tv[idx].curve);
	      once = 1;
              continue;
            }
          if (!strcmp(tv[idx].curve, "secp256k1"))
            {
	      static int once;
	      if (!once)
		info ("skipping %s in fips mode\n", tv[idx].curve);
	      once = 1;
              continue;
            }
        }

      err = gcry_mpi_ec_new (&ctx, NULL, tv[idx].curve);
      if (err)
        {
          fail ("tv[%d].'%s': can't create context: %s\n",
		idx, tv[idx].curve, gpg_strerror (err));
          return;
        }

      if (tv[idx].ux)
	ux = hex2mpi (tv[idx].ux);
      else
	die ("tv[%d].'%s': missing 'ux'\n", idx, tv[idx].curve);

      if (tv[idx].uy)
	uy = hex2mpi (tv[idx].uy);
      else
	die ("tv[%d].'%s': missing 'uy'\n", idx, tv[idx].curve);

      if (tv[idx].uz)
	uz = hex2mpi (tv[idx].uz);
      else
	uz = gcry_mpi_set_ui(NULL, 1);

      if (tv[idx].scalar)
	scalar = hex2mpi (tv[idx].scalar);
      else
	die ("tv[%d].'%s': missing 'scalar'\n", idx, tv[idx].curve);

      U = gcry_mpi_point_new (0);
      gcry_mpi_point_set (U, ux, uy, uz);
      Q = gcry_mpi_point_new (0);
      x = gcry_mpi_new (0);
      y = gcry_mpi_new (0);

      if (!gcry_mpi_ec_curve_point (U, ctx))
        {
          print_point ("  U", U);
	  die ("tv[%d].'%s': point expected on curve but not "
	       "identified as such\n", idx, tv[idx].curve);
        }

      gcry_mpi_ec_mul (Q, scalar, U, ctx);

      if (!gcry_mpi_ec_curve_point (Q, ctx))
        {
          print_point ("  Q", Q);
	  die ("tv[%d].'%s': point expected on curve but not "
	       "identified as such\n", idx, tv[idx].curve);
        }

      if (gcry_mpi_ec_get_affine (x, y, Q, ctx))
	{
	  fail ("tv[%d].'%s': failed to get affine coordinates\n",
		idx, tv[idx].curve);
	  goto out;
	}

      if ((tv[idx].qx != NULL && tv[idx].qy != NULL)
	  && (cmp_mpihex (x, tv[idx].qx) || cmp_mpihex (y, tv[idx].qy)))
	{
	  fail ("tv[%d].'%s': sample point multiply failed:\n",
		idx, tv[idx].curve);
	  print_mpi ("     scalar", scalar);
	  print_mpi ("         Qx", x);
	  printf ("expected Qx: %s\n", tv[idx].qx);
	  print_mpi ("         Qy", y);
	  printf ("expected Qy: %s\n", tv[idx].qy);
	}

out:
      gcry_mpi_release (uy);
      gcry_mpi_release (ux);
      gcry_mpi_release (uz);
      gcry_mpi_release (scalar);
      gcry_mpi_release (y);
      gcry_mpi_release (x);
      gcry_mpi_point_release (Q);
      gcry_mpi_point_release (U);
      gcry_ctx_release (ctx);
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
  xgcry_control ((GCRYCTL_ENABLE_QUICK_RANDOM, 0));
  if (debug)
    xgcry_control ((GCRYCTL_SET_DEBUG_FLAGS, 1u, 0));
  xgcry_control ((GCRYCTL_INITIALIZATION_FINISHED, 0));

  set_get_point ();
  context_alloc ();
  context_param ();
  basic_ec_math ();
  point_on_curve ();
  check_ec_mul ();
  check_ec_mul_reduction ();

  /* The tests are for P-192 and ed25519 which are not supported in
     FIPS mode.  */
  if (!gcry_fips_mode_active())
    {
      basic_ec_math_simplified ();
      twistededwards_math ();
    }

  info ("All tests completed. Errors: %d\n", error_count);
  return error_count ? 1 : 0;
}
