/* pubkey.c - Public key encryption/decryption tests
 *	Copyright (C) 2001, 2002, 2003, 2005 Free Software Foundation, Inc.
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
#include <stdlib.h>
#include <string.h>


#define PGM "pubkey"
#include "t-common.h"

static int in_fips_mode;

/* Sample RSA keys, taken from basic.c.  */

static const char sample_private_key_1[] =
"(private-key\n"
" (openpgp-rsa\n"
"  (n #009F56231A3D82E3E7D613D59D53E9AB921BEF9F08A782AED0B6E46ADBC853EC"
"      7C71C422435A3CD8FA0DB9EFD55CD3295BADC4E8E2E2B94E15AE82866AB8ADE8"
"      7E469FAE76DC3577DE87F1F419C4EB41123DFAF8D16922D5EDBAD6E9076D5A1C"
"      958106F0AE5E2E9193C6B49124C64C2A241C4075D4AF16299EB87A6585BAE917"
"      DEF27FCDD165764D069BC18D16527B29DAAB549F7BBED4A7C6A842D203ED6613"
"      6E2411744E432CD26D940132F25874483DCAEECDFD95744819CBCF1EA810681C"
"      42907EBCB1C7EAFBE75C87EC32C5413EA10476545D3FC7B2ADB1B66B7F200918"
"      664B0E5261C2895AA28B0DE321E921B3F877172CCCAB81F43EF98002916156F6"
"      CB#)\n"
"  (e #010001#)\n"
"  (d #07EF82500C403899934FE993AC5A36F14FF2DF38CF1EF315F205EE4C83EDAA19"
"      8890FC23DE9AA933CAFB37B6A8A8DBA675411958337287310D3FF2F1DDC0CB93"
"      7E70F57F75F833C021852B631D2B9A520E4431A03C5C3FCB5742DCD841D9FB12"
"      771AA1620DCEC3F1583426066ED9DC3F7028C5B59202C88FDF20396E2FA0EC4F"
"      5A22D9008F3043673931BC14A5046D6327398327900867E39CC61B2D1AFE2F48"
"      EC8E1E3861C68D257D7425F4E6F99ABD77D61F10CA100EFC14389071831B33DD"
"      69CC8EABEF860D1DC2AAA84ABEAE5DFC91BC124DAF0F4C8EF5BBEA436751DE84"
"      3A8063E827A024466F44C28614F93B0732A100D4A0D86D532FE1E22C7725E401"
"      #)\n"
"  (p #00C29D438F115825779631CD665A5739367F3E128ADC29766483A46CA80897E0"
"      79B32881860B8F9A6A04C2614A904F6F2578DAE13EA67CD60AE3D0AA00A1FF9B"
"      441485E44B2DC3D0B60260FBFE073B5AC72FAF67964DE15C8212C389D20DB9CF"
"      54AF6AEF5C4196EAA56495DD30CF709F499D5AB30CA35E086C2A1589D6283F17"
"      83#)\n"
"  (q #00D1984135231CB243FE959C0CBEF551EDD986AD7BEDF71EDF447BE3DA27AF46"
"      79C974A6FA69E4D52FE796650623DE70622862713932AA2FD9F2EC856EAEAA77"
"      88B4EA6084DC81C902F014829B18EA8B2666EC41586818E0589E18876065F97E"
"      8D22CE2DA53A05951EC132DCEF41E70A9C35F4ACC268FFAC2ADF54FA1DA110B9"
"      19#)\n"
"  (u #67CF0FD7635205DD80FA814EE9E9C267C17376BF3209FB5D1BC42890D2822A04"
"      479DAF4D5B6ED69D0F8D1AF94164D07F8CD52ECEFE880641FA0F41DDAB1785E4"
"      A37A32F997A516480B4CD4F6482B9466A1765093ED95023CA32D5EDC1E34CEE9"
"      AF595BC51FE43C4BF810FA225AF697FB473B83815966188A4312C048B885E3F7"
"      #)\n"
" )\n"
")\n";

/* The same key as above but without p, q and u to test the non CRT case. */
static const char sample_private_key_1_1[] =
"(private-key\n"
" (openpgp-rsa\n"
"  (n #009F56231A3D82E3E7D613D59D53E9AB921BEF9F08A782AED0B6E46ADBC853EC"
"      7C71C422435A3CD8FA0DB9EFD55CD3295BADC4E8E2E2B94E15AE82866AB8ADE8"
"      7E469FAE76DC3577DE87F1F419C4EB41123DFAF8D16922D5EDBAD6E9076D5A1C"
"      958106F0AE5E2E9193C6B49124C64C2A241C4075D4AF16299EB87A6585BAE917"
"      DEF27FCDD165764D069BC18D16527B29DAAB549F7BBED4A7C6A842D203ED6613"
"      6E2411744E432CD26D940132F25874483DCAEECDFD95744819CBCF1EA810681C"
"      42907EBCB1C7EAFBE75C87EC32C5413EA10476545D3FC7B2ADB1B66B7F200918"
"      664B0E5261C2895AA28B0DE321E921B3F877172CCCAB81F43EF98002916156F6"
"      CB#)\n"
"  (e #010001#)\n"
"  (d #07EF82500C403899934FE993AC5A36F14FF2DF38CF1EF315F205EE4C83EDAA19"
"      8890FC23DE9AA933CAFB37B6A8A8DBA675411958337287310D3FF2F1DDC0CB93"
"      7E70F57F75F833C021852B631D2B9A520E4431A03C5C3FCB5742DCD841D9FB12"
"      771AA1620DCEC3F1583426066ED9DC3F7028C5B59202C88FDF20396E2FA0EC4F"
"      5A22D9008F3043673931BC14A5046D6327398327900867E39CC61B2D1AFE2F48"
"      EC8E1E3861C68D257D7425F4E6F99ABD77D61F10CA100EFC14389071831B33DD"
"      69CC8EABEF860D1DC2AAA84ABEAE5DFC91BC124DAF0F4C8EF5BBEA436751DE84"
"      3A8063E827A024466F44C28614F93B0732A100D4A0D86D532FE1E22C7725E401"
"      #)\n"
" )\n"
")\n";

/* The same key as above but just without q to test the non CRT case.  This
   should fail. */
static const char sample_private_key_1_2[] =
"(private-key\n"
" (openpgp-rsa\n"
"  (n #009F56231A3D82E3E7D613D59D53E9AB921BEF9F08A782AED0B6E46ADBC853EC"
"     7C71C422435A3CD8FA0DB9EFD55CD3295BADC4E8E2E2B94E15AE82866AB8ADE8"
"     7E469FAE76DC3577DE87F1F419C4EB41123DFAF8D16922D5EDBAD6E9076D5A1C"
"     958106F0AE5E2E9193C6B49124C64C2A241C4075D4AF16299EB87A6585BAE917"
"     DEF27FCDD165764D069BC18D16527B29DAAB549F7BBED4A7C6A842D203ED6613"
"     6E2411744E432CD26D940132F25874483DCAEECDFD95744819CBCF1EA810681C"
"     42907EBCB1C7EAFBE75C87EC32C5413EA10476545D3FC7B2ADB1B66B7F200918"
"     664B0E5261C2895AA28B0DE321E921B3F877172CCCAB81F43EF98002916156F6"
"     CB#)\n"
"  (e #010001#)\n"
"  (d #07EF82500C403899934FE993AC5A36F14FF2DF38CF1EF315F205EE4C83EDAA19"
"      8890FC23DE9AA933CAFB37B6A8A8DBA675411958337287310D3FF2F1DDC0CB93"
"      7E70F57F75F833C021852B631D2B9A520E4431A03C5C3FCB5742DCD841D9FB12"
"      771AA1620DCEC3F1583426066ED9DC3F7028C5B59202C88FDF20396E2FA0EC4F"
"      5A22D9008F3043673931BC14A5046D6327398327900867E39CC61B2D1AFE2F48"
"      EC8E1E3861C68D257D7425F4E6F99ABD77D61F10CA100EFC14389071831B33DD"
"      69CC8EABEF860D1DC2AAA84ABEAE5DFC91BC124DAF0F4C8EF5BBEA436751DE84"
"      3A8063E827A024466F44C28614F93B0732A100D4A0D86D532FE1E22C7725E401"
"      #)\n"
"  (p #00C29D438F115825779631CD665A5739367F3E128ADC29766483A46CA80897E0"
"      79B32881860B8F9A6A04C2614A904F6F2578DAE13EA67CD60AE3D0AA00A1FF9B"
"      441485E44B2DC3D0B60260FBFE073B5AC72FAF67964DE15C8212C389D20DB9CF"
"      54AF6AEF5C4196EAA56495DD30CF709F499D5AB30CA35E086C2A1589D6283F17"
"      83#)\n"
"  (u #67CF0FD7635205DD80FA814EE9E9C267C17376BF3209FB5D1BC42890D2822A04"
"      479DAF4D5B6ED69D0F8D1AF94164D07F8CD52ECEFE880641FA0F41DDAB1785E4"
"      A37A32F997A516480B4CD4F6482B9466A1765093ED95023CA32D5EDC1E34CEE9"
"      AF595BC51FE43C4BF810FA225AF697FB473B83815966188A4312C048B885E3F7"
"      #)\n"
" )\n"
")\n";

static const char sample_public_key_1[] =
"(public-key\n"
" (rsa\n"
"  (n #009F56231A3D82E3E7D613D59D53E9AB921BEF9F08A782AED0B6E46ADBC853EC"
"      7C71C422435A3CD8FA0DB9EFD55CD3295BADC4E8E2E2B94E15AE82866AB8ADE8"
"      7E469FAE76DC3577DE87F1F419C4EB41123DFAF8D16922D5EDBAD6E9076D5A1C"
"      958106F0AE5E2E9193C6B49124C64C2A241C4075D4AF16299EB87A6585BAE917"
"      DEF27FCDD165764D069BC18D16527B29DAAB549F7BBED4A7C6A842D203ED6613"
"      6E2411744E432CD26D940132F25874483DCAEECDFD95744819CBCF1EA810681C"
"      42907EBCB1C7EAFBE75C87EC32C5413EA10476545D3FC7B2ADB1B66B7F200918"
"      664B0E5261C2895AA28B0DE321E921B3F877172CCCAB81F43EF98002916156F6"
"      CB#)\n"
"  (e #010001#)\n"
" )\n"
")\n";


static void
show_sexp (const char *prefix, gcry_sexp_t a)
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

/* from ../cipher/pubkey-util.c */
static gpg_err_code_t
_gcry_pk_util_get_nbits (gcry_sexp_t list, unsigned int *r_nbits)
{
  char buf[50];
  const char *s;
  size_t n;

  *r_nbits = 0;

  list = gcry_sexp_find_token (list, "nbits", 0);
  if (!list)
    return 0; /* No NBITS found.  */

  s = gcry_sexp_nth_data (list, 1, &n);
  if (!s || n >= DIM (buf) - 1 )
    {
      /* NBITS given without a cdr.  */
      gcry_sexp_release (list);
      return GPG_ERR_INV_OBJ;
    }
  memcpy (buf, s, n);
  buf[n] = 0;
  *r_nbits = (unsigned int)strtoul (buf, NULL, 0);
  gcry_sexp_release (list);
  return 0;
}

/* Convert STRING consisting of hex characters into its binary
   representation and return it as an allocated buffer. The valid
   length of the buffer is returned at R_LENGTH.  The string is
   delimited by end of string.  The function returns NULL on
   error.  */
static void *
data_from_hex (const char *string, size_t *r_length)
{
  const char *s;
  unsigned char *buffer;
  size_t length;

  buffer = gcry_xmalloc (strlen(string)/2+1);
  length = 0;
  for (s=string; *s; s +=2 )
    {
      if (!hexdigitp (s) || !hexdigitp (s+1))
        die ("error parsing hex string `%s'\n", string);
      ((unsigned char*)buffer)[length++] = xtoi_2 (s);
    }
  *r_length = length;
  return buffer;
}


static void
extract_cmp_data (gcry_sexp_t sexp, const char *name, const char *expected)
{
  gcry_sexp_t l1;
  const void *a;
  size_t alen;
  void *b;
  size_t blen;

  l1 = gcry_sexp_find_token (sexp, name, 0);
  a = gcry_sexp_nth_data (l1, 1, &alen);
  b = data_from_hex (expected, &blen);
  if (!a)
    fail ("parameter \"%s\" missing in key\n", name);
  else if ( alen != blen || memcmp (a, b, alen) )
    {
      fail ("parameter \"%s\" does not match expected value\n", name);
      if (verbose)
        {
          info ("expected: %s\n", expected);
          show_sexp ("sexp: ", sexp);
        }
    }
  gcry_free (b);
  gcry_sexp_release (l1);
}


static void
check_keys_crypt (gcry_sexp_t pkey, gcry_sexp_t skey,
		  gcry_sexp_t plain0, gpg_err_code_t decrypt_fail_code)
{
  gcry_sexp_t plain1, cipher, l;
  gcry_mpi_t x0, x1;
  int rc;
  int have_flags;

  /* Extract data from plaintext.  */
  l = gcry_sexp_find_token (plain0, "value", 0);
  x0 = gcry_sexp_nth_mpi (l, 1, GCRYMPI_FMT_USG);
  gcry_sexp_release (l);

  /* Encrypt data.  */
  rc = gcry_pk_encrypt (&cipher, plain0, pkey);
  if (rc)
    die ("encryption failed: %s\n", gcry_strerror (rc));

  l = gcry_sexp_find_token (cipher, "flags", 0);
  have_flags = !!l;
  gcry_sexp_release (l);

  /* Decrypt data.  */
  rc = gcry_pk_decrypt (&plain1, cipher, skey);
  gcry_sexp_release (cipher);
  if (rc)
    {
      if (decrypt_fail_code && gpg_err_code (rc) == decrypt_fail_code)
	{
	  gcry_mpi_release (x0);
	  return; /* This is the expected failure code.  */
	}
      die ("decryption failed: %s\n", gcry_strerror (rc));
    }

  /* Extract decrypted data.  Note that for compatibility reasons, the
     output of gcry_pk_decrypt depends on whether a flags lists (even
     if empty) occurs in its input data.  Because we passed the output
     of encrypt directly to decrypt, such a flag value won't be there
     as of today.  We check it anyway. */
  l = gcry_sexp_find_token (plain1, "value", 0);
  if (l)
    {
      if (!have_flags)
        die ("compatibility mode of pk_decrypt broken\n");
      gcry_sexp_release (plain1);
      x1 = gcry_sexp_nth_mpi (l, 1, GCRYMPI_FMT_USG);
      gcry_sexp_release (l);
    }
  else
    {
      if (have_flags)
        die ("compatibility mode of pk_decrypt broken\n");
      x1 = gcry_sexp_nth_mpi (plain1, 0, GCRYMPI_FMT_USG);
      gcry_sexp_release (plain1);
    }

  /* Compare.  */
  if (gcry_mpi_cmp (x0, x1))
    die ("data corrupted\n");
  gcry_mpi_release (x0);
  gcry_mpi_release (x1);
}

static void
check_keys (gcry_sexp_t pkey, gcry_sexp_t skey, unsigned int nbits_data,
            gpg_err_code_t decrypt_fail_code)
{
  gcry_sexp_t plain;
  gcry_mpi_t x;
  int rc;

  /* Create plain text.  */
  x = gcry_mpi_new (nbits_data);
  gcry_mpi_randomize (x, nbits_data, GCRY_WEAK_RANDOM);

  rc = gcry_sexp_build (&plain, NULL, "(data (flags raw) (value %m))", x);
  if (rc)
    die ("converting data for encryption failed: %s\n",
	 gcry_strerror (rc));

  check_keys_crypt (pkey, skey, plain, decrypt_fail_code);
  gcry_sexp_release (plain);
  gcry_mpi_release (x);

  /* Create plain text.  */
  x = gcry_mpi_new (nbits_data);
  gcry_mpi_randomize (x, nbits_data, GCRY_WEAK_RANDOM);

  rc = gcry_sexp_build (&plain, NULL,
                        "(data (flags raw no-blinding) (value %m))", x);
  gcry_mpi_release (x);
  if (rc)
    die ("converting data for encryption failed: %s\n",
	 gcry_strerror (rc));

  check_keys_crypt (pkey, skey, plain, decrypt_fail_code);
  gcry_sexp_release (plain);
}

static void
get_keys_sample (gcry_sexp_t *pkey, gcry_sexp_t *skey, int secret_variant)
{
  gcry_sexp_t pub_key, sec_key;
  int rc;
  static const char *secret;


  switch (secret_variant)
    {
    case 0: secret = sample_private_key_1; break;
    case 1: secret = sample_private_key_1_1; break;
    case 2: secret = sample_private_key_1_2; break;
    default: die ("BUG\n");
    }

  rc = gcry_sexp_sscan (&pub_key, NULL, sample_public_key_1,
			strlen (sample_public_key_1));
  if (!rc)
    rc = gcry_sexp_sscan (&sec_key, NULL, secret, strlen (secret));
  if (rc)
    die ("converting sample keys failed: %s\n", gcry_strerror (rc));

  *pkey = pub_key;
  *skey = sec_key;
}

static void
get_keys_new (gcry_sexp_t *pkey, gcry_sexp_t *skey)
{
  gcry_sexp_t key_spec, key, pub_key, sec_key;
  int rc;

  rc = gcry_sexp_new (&key_spec,
		      "(genkey (rsa (nbits 4:2048)))", 0, 1);
  if (rc)
    die ("error creating S-expression: %s\n", gcry_strerror (rc));
  rc = gcry_pk_genkey (&key, key_spec);
  gcry_sexp_release (key_spec);
  if (rc)
    die ("error generating RSA key: %s\n", gcry_strerror (rc));

  if (verbose > 1)
    show_sexp ("generated RSA key:\n", key);

  pub_key = gcry_sexp_find_token (key, "public-key", 0);
  if (! pub_key)
    die ("public part missing in key\n");

  sec_key = gcry_sexp_find_token (key, "private-key", 0);
  if (! sec_key)
    die ("private part missing in key\n");

  gcry_sexp_release (key);
  *pkey = pub_key;
  *skey = sec_key;
}


static void
get_keys_x931_new (gcry_sexp_t *pkey, gcry_sexp_t *skey)
{
  gcry_sexp_t key_spec, key, pub_key, sec_key;
  int rc;

  rc = gcry_sexp_new (&key_spec,
		      "(genkey (rsa (nbits 4:2048)(use-x931)))", 0, 1);
  if (rc)
    die ("error creating S-expression: %s\n", gcry_strerror (rc));
  rc = gcry_pk_genkey (&key, key_spec);
  gcry_sexp_release (key_spec);
  if (rc)
    {
      if (in_fips_mode)
        {
          if (verbose)
            fprintf (stderr, "The X9.31 RSA keygen is not available in FIPS modee.\n");
          return;
        }
      die ("error generating RSA key: %s\n", gcry_strerror (rc));
    }
  else if (in_fips_mode)
    die ("generating X9.31 RSA key unexpected worked in FIPS mode\n");

  if (verbose > 1)
    show_sexp ("generated RSA (X9.31) key:\n", key);

  pub_key = gcry_sexp_find_token (key, "public-key", 0);
  if (!pub_key)
    die ("public part missing in key\n");

  sec_key = gcry_sexp_find_token (key, "private-key", 0);
  if (!sec_key)
    die ("private part missing in key\n");

  gcry_sexp_release (key);
  *pkey = pub_key;
  *skey = sec_key;
}


static void
get_elg_key_new (gcry_sexp_t *pkey, gcry_sexp_t *skey, int fixed_x)
{
  gcry_sexp_t key_spec, key, pub_key, sec_key;
  int rc;

  rc = gcry_sexp_new
    (&key_spec,
     (fixed_x
      ? "(genkey (elg (nbits 4:1024)(xvalue my.not-so-secret.key)))"
      : "(genkey (elg (nbits 3:512)))"),
     0, 1);

  if (rc)
    die ("error creating S-expression: %s\n", gcry_strerror (rc));
  rc = gcry_pk_genkey (&key, key_spec);
  gcry_sexp_release (key_spec);
  if (rc)
    {
      if (in_fips_mode)
        {
          if (verbose)
            fprintf (stderr, "The Elgamal keys are not available in FIPS modee.\n");
          return;
        }
      die ("error generating Elgamal key: %s\n", gcry_strerror (rc));
    }

  if (verbose > 1)
    show_sexp ("generated ELG key:\n", key);

  pub_key = gcry_sexp_find_token (key, "public-key", 0);
  if (!pub_key)
    die ("public part missing in key\n");

  sec_key = gcry_sexp_find_token (key, "private-key", 0);
  if (!sec_key)
    die ("private part missing in key\n");

  gcry_sexp_release (key);
  *pkey = pub_key;
  *skey = sec_key;
}


static void
get_dsa_key_new (gcry_sexp_t *pkey, gcry_sexp_t *skey, int transient_key)
{
  gcry_sexp_t key_spec, key, pub_key, sec_key;
  int rc;

  rc = gcry_sexp_new (&key_spec,
                      transient_key
                      ? "(genkey (dsa (nbits 4:2048)(transient-key)))"
                      : "(genkey (dsa (nbits 4:2048)))",
                      0, 1);
  if (rc)
    die ("error creating S-expression: %s\n", gcry_strerror (rc));
  rc = gcry_pk_genkey (&key, key_spec);
  gcry_sexp_release (key_spec);
  if (rc)
    {
      if (in_fips_mode)
        {
          if (verbose)
            fprintf (stderr, "The DSA keys are not available in FIPS modee.\n");
          return;
        }
      die ("error generating DSA key: %s\n", gcry_strerror (rc));
    }

  if (verbose > 1)
    show_sexp ("generated DSA key:\n", key);

  pub_key = gcry_sexp_find_token (key, "public-key", 0);
  if (!pub_key)
    die ("public part missing in key\n");

  sec_key = gcry_sexp_find_token (key, "private-key", 0);
  if (!sec_key)
    die ("private part missing in key\n");

  gcry_sexp_release (key);
  *pkey = pub_key;
  *skey = sec_key;
}


static void
get_dsa_key_fips186_new (gcry_sexp_t *pkey, gcry_sexp_t *skey)
{
  gcry_sexp_t key_spec, key, pub_key, sec_key;
  int rc;

  rc = gcry_sexp_new
    (&key_spec, "(genkey (dsa (nbits 4:2048)(use-fips186)))",  0, 1);
  if (rc)
    die ("error creating S-expression: %s\n", gcry_strerror (rc));
  rc = gcry_pk_genkey (&key, key_spec);
  gcry_sexp_release (key_spec);
  if (rc)
    {
      if (in_fips_mode)
        {
          if (verbose)
            fprintf (stderr, "The DSA keys are not available in FIPS modee.\n");
          return;
        }
      die ("error generating DSA key: %s\n", gcry_strerror (rc));
    }

  if (verbose > 1)
    show_sexp ("generated DSA key (fips 186):\n", key);

  pub_key = gcry_sexp_find_token (key, "public-key", 0);
  if (!pub_key)
    die ("public part missing in key\n");

  sec_key = gcry_sexp_find_token (key, "private-key", 0);
  if (!sec_key)
    die ("private part missing in key\n");

  gcry_sexp_release (key);
  *pkey = pub_key;
  *skey = sec_key;
}


static void
get_dsa_key_with_domain_new (gcry_sexp_t *pkey, gcry_sexp_t *skey)
{
  gcry_sexp_t key_spec, key, pub_key, sec_key;
  int rc;

  rc = gcry_sexp_new
    (&key_spec,
     "(genkey (dsa (transient-key)(domain"
     "(p #a8adb6c0b4cf9588012e5deff1a871d383e0e2a85b5e8e03d814fe13a059705e"
     "663230a377bf7323a8fa117100200bfd5adf857393b0bbd67906c081e585410e"
     "38480ead51684dac3a38f7b64c9eb109f19739a4517cd7d5d6291e8af20a3fbf"
     "17336c7bf80ee718ee087e322ee41047dabefbcc34d10b66b644ddb3160a28c0"
     "639563d71993a26543eadb7718f317bf5d9577a6156561b082a10029cd44012b"
     "18de6844509fe058ba87980792285f2750969fe89c2cd6498db3545638d5379d"
     "125dccf64e06c1af33a6190841d223da1513333a7c9d78462abaab31b9f96d5f"
     "34445ceb6309f2f6d2c8dde06441e87980d303ef9a1ff007e8be2f0be06cc15f#)"
     "(q #e71f8567447f42e75f5ef85ca20fe557ab0343d37ed09edc3f6e68604d6b9dfb#)"
     "(g #5ba24de9607b8998e66ce6c4f812a314c6935842f7ab54cd82b19fa104abfb5d"
     "84579a623b2574b37d22ccae9b3e415e48f5c0f9bcbdff8071d63b9bb956e547"
     "af3a8df99e5d3061979652ff96b765cb3ee493643544c75dbe5bb39834531952"
     "a0fb4b0378b3fcbb4c8b5800a5330392a2a04e700bb6ed7e0b85795ea38b1b96"
     "2741b3f33b9dde2f4ec1354f09e2eb78e95f037a5804b6171659f88715ce1a9b"
     "0cc90c27f35ef2f10ff0c7c7a2bb0154d9b8ebe76a3d764aa879af372f4240de"
     "8347937e5a90cec9f41ff2f26b8da9a94a225d1a913717d73f10397d2183f1ba"
     "3b7b45a68f1ff1893caf69a827802f7b6a48d51da6fbefb64fd9a6c5b75c4561#)"
     ")))", 0, 1);
  if (rc)
    die ("error creating S-expression: %s\n", gcry_strerror (rc));
  rc = gcry_pk_genkey (&key, key_spec);
  gcry_sexp_release (key_spec);
  if (rc)
    {
      if (in_fips_mode)
        {
          if (verbose)
            fprintf (stderr, "The DSA keys are not available in FIPS modee.\n");
          return;
        }
      die ("error generating DSA key: %s\n", gcry_strerror (rc));
    }

  if (verbose > 1)
    show_sexp ("generated DSA key:\n", key);

  pub_key = gcry_sexp_find_token (key, "public-key", 0);
  if (!pub_key)
    die ("public part missing in key\n");

  sec_key = gcry_sexp_find_token (key, "private-key", 0);
  if (!sec_key)
    die ("private part missing in key\n");

  gcry_sexp_release (key);
  *pkey = pub_key;
  *skey = sec_key;
}

#if 0
static void
get_dsa_key_fips186_with_domain_new (gcry_sexp_t *pkey, gcry_sexp_t *skey)
{
  gcry_sexp_t key_spec, key, pub_key, sec_key;
  int rc;

  rc = gcry_sexp_new
    (&key_spec,
     "(genkey (dsa (transient-key)(use-fips186)(domain"
     "(p #d3aed1876054db831d0c1348fbb1ada72507e5fbf9a62cbd47a63aeb7859d6921"
     "4adeb9146a6ec3f43520f0fd8e3125dd8bbc5d87405d1ac5f82073cd762a3f8d7"
     "74322657c9da88a7d2f0e1a9ceb84a39cb40876179e6a76e400498de4bb9379b0"
     "5f5feb7b91eb8fea97ee17a955a0a8a37587a272c4719d6feb6b54ba4ab69#)"
     "(q #9c916d121de9a03f71fb21bc2e1c0d116f065a4f#)"
     "(g #8157c5f68ca40b3ded11c353327ab9b8af3e186dd2e8dade98761a0996dda99ab"
     "0250d3409063ad99efae48b10c6ab2bba3ea9a67b12b911a372a2bba260176fad"
     "b4b93247d9712aad13aa70216c55da9858f7a298deb670a403eb1e7c91b847f1e"
     "ccfbd14bd806fd42cf45dbb69cd6d6b43add2a78f7d16928eaa04458dea44#)"
     ")))", 0, 1);
  if (rc)
    die ("error creating S-expression: %s\n", gcry_strerror (rc));
  rc = gcry_pk_genkey (&key, key_spec);
  gcry_sexp_release (key_spec);
  if (rc)
    die ("error generating DSA key: %s\n", gcry_strerror (rc));

  if (verbose > 1)
    show_sexp ("generated DSA key:\n", key);

  pub_key = gcry_sexp_find_token (key, "public-key", 0);
  if (!pub_key)
    die ("public part missing in key\n");

  sec_key = gcry_sexp_find_token (key, "private-key", 0);
  if (!sec_key)
    die ("private part missing in key\n");

  gcry_sexp_release (key);
  *pkey = pub_key;
  *skey = sec_key;
}
#endif /*0*/

static void
get_dsa_key_fips186_with_seed_new (gcry_sexp_t *pkey, gcry_sexp_t *skey)
{
  gcry_sexp_t key_spec, key, pub_key, sec_key;
  int rc;

  rc = gcry_sexp_new
    (&key_spec,
     "(genkey"
     "  (dsa"
     "    (nbits 4:2048)"
     "    (qbits 3:256)"
     "    (use-fips186)"
     "    (transient-key)"
     "    (derive-parms"
     "      (seed #f770a4598ff756931fc529764513b103ce57d85f4ad8c5cf297c9b4d48241c5b#))))",
     0, 1);
  if (rc)
    die ("error creating S-expression: %s\n", gcry_strerror (rc));
  rc = gcry_pk_genkey (&key, key_spec);
  gcry_sexp_release (key_spec);
  if (rc)
    {
      if (in_fips_mode)
        {
          if (verbose)
            fprintf (stderr, "The DSA keys are not available in FIPS modee.\n");
          return;
        }
      die ("error generating DSA key: %s\n", gcry_strerror (rc));
    }

  if (verbose > 1)
    show_sexp ("generated DSA key (fips 186 with seed):\n", key);

  pub_key = gcry_sexp_find_token (key, "public-key", 0);
  if (!pub_key)
    die ("public part missing in key\n");

  sec_key = gcry_sexp_find_token (key, "private-key", 0);
  if (!sec_key)
    die ("private part missing in key\n");

  gcry_sexp_release (key);
  *pkey = pub_key;
  *skey = sec_key;
}


static void
check_run (void)
{
  gpg_error_t err;
  gcry_sexp_t pkey, skey;
  int variant;

  pkey = skey = NULL;
  for (variant=0; variant < 3; variant++)
    {
      if (verbose)
        fprintf (stderr, "Checking sample key (%d).\n", variant);
      get_keys_sample (&pkey, &skey, variant);
      /* Check gcry_pk_testkey which requires all elements.  */
      err = gcry_pk_testkey (skey);
      if ((variant == 0 && err)
          || (variant > 0 && gpg_err_code (err) != GPG_ERR_NO_OBJ))
          die ("gcry_pk_testkey failed: %s\n", gpg_strerror (err));
      /* Run the usual check but expect an error from variant 2.  */
      check_keys (pkey, skey, 800, variant == 2? GPG_ERR_NO_OBJ : 0);
      gcry_sexp_release (pkey);
      gcry_sexp_release (skey);
      pkey = skey = NULL;
    }

  if (verbose)
    fprintf (stderr, "Checking generated RSA key.\n");
  get_keys_new (&pkey, &skey);
  check_keys (pkey, skey, 800, 0);
  gcry_sexp_release (pkey);
  gcry_sexp_release (skey);
  pkey = skey = NULL;

  if (verbose)
    fprintf (stderr, "Checking generated RSA key (X9.31).\n");
  get_keys_x931_new (&pkey, &skey);
  if (!in_fips_mode)
    check_keys (pkey, skey, 800, 0);
  gcry_sexp_release (pkey);
  gcry_sexp_release (skey);
  pkey = skey = NULL;

  if (verbose)
    fprintf (stderr, "Checking generated Elgamal key.\n");
  get_elg_key_new (&pkey, &skey, 0);
  if (!in_fips_mode)
    check_keys (pkey, skey, 400, 0);
  gcry_sexp_release (pkey);
  gcry_sexp_release (skey);
  pkey = skey = NULL;

  if (verbose)
    fprintf (stderr, "Checking passphrase generated Elgamal key.\n");
  get_elg_key_new (&pkey, &skey, 1);
  if (!in_fips_mode)
    check_keys (pkey, skey, 800, 0);
  gcry_sexp_release (pkey);
  gcry_sexp_release (skey);
  pkey = skey = NULL;

  if (verbose)
    fprintf (stderr, "Generating DSA key.\n");
  get_dsa_key_new (&pkey, &skey, 0);
  if (!in_fips_mode)
    {
      /* Fixme:  Add a check function for DSA keys.  */
      ;
    }

  gcry_sexp_release (pkey);
  gcry_sexp_release (skey);
  pkey = skey = NULL;

  if (verbose)
    fprintf (stderr, "Generating transient DSA key.\n");
  get_dsa_key_new (&pkey, &skey, 1);
  if (!in_fips_mode)
    {
      /* Fixme:  Add a check function for DSA keys.  */
      ;
    }
  gcry_sexp_release (pkey);
  gcry_sexp_release (skey);
  pkey = skey = NULL;

  if (verbose)
    fprintf (stderr, "Generating DSA key (FIPS 186).\n");
  get_dsa_key_fips186_new (&pkey, &skey);
  if (!in_fips_mode)
    {
      /* Fixme:  Add a check function for DSA keys.  */
      ;
    }
  gcry_sexp_release (pkey);
  gcry_sexp_release (skey);
  pkey = skey = NULL;

  if (verbose)
    fprintf (stderr, "Generating DSA key with given domain.\n");
  get_dsa_key_with_domain_new (&pkey, &skey);
  if (!in_fips_mode)
    {
      /* Fixme:  Add a check function for DSA keys.  */
      ;
    }
  gcry_sexp_release (pkey);
  gcry_sexp_release (skey);
  pkey = skey = NULL;

  /* We need new test vectors for get_dsa_key_fips186_with_domain_new.  */
  if (verbose)
    fprintf (stderr, "Generating DSA key with given domain (FIPS 186)"
             " - skipped.\n");
  /* get_dsa_key_fips186_with_domain_new (&pkey, &skey); */
  /* /\* Fixme:  Add a check function for DSA keys.  *\/ */
  /* gcry_sexp_release (pkey); */
  /* gcry_sexp_release (skey); */

  if (verbose)
    fprintf (stderr, "Generating DSA key with given seed (FIPS 186).\n");
  get_dsa_key_fips186_with_seed_new (&pkey, &skey);
  if (!in_fips_mode)
    {
      /* Fixme:  Add a check function for DSA keys.  */
      ;
    }
  gcry_sexp_release (pkey);
  gcry_sexp_release (skey);
  pkey = skey = NULL;
}



static gcry_mpi_t
key_param_from_sexp (gcry_sexp_t sexp, const char *topname, const char *name)
{
  gcry_sexp_t l1, l2;
  gcry_mpi_t result;

  l1 = gcry_sexp_find_token (sexp, topname, 0);
  if (!l1)
    return NULL;

  l2 = gcry_sexp_find_token (l1, name, 0);
  if (!l2)
    {
      gcry_sexp_release (l1);
      return NULL;
    }

  result = gcry_sexp_nth_mpi (l2, 1, GCRYMPI_FMT_USG);
  gcry_sexp_release (l2);
  gcry_sexp_release (l1);
  return result;
}


static void
check_x931_derived_key (int what)
{
  static struct {
    const char *param;
    const char *expected_d;
  } testtable[] = {
    { /* First example from X9.31 (D.1.1).  */
      "(genkey\n"
      "  (rsa\n"
      "    (nbits 4:1024)\n"
      "    (rsa-use-e 1:3)\n"
      "    (derive-parms\n"
      "      (Xp1 #1A1916DDB29B4EB7EB6732E128#)\n"
      "      (Xp2 #192E8AAC41C576C822D93EA433#)\n"
      "      (Xp  #D8CD81F035EC57EFE822955149D3BFF70C53520D\n"
      "            769D6D76646C7A792E16EBD89FE6FC5B605A6493\n"
      "            39DFC925A86A4C6D150B71B9EEA02D68885F5009\n"
      "            B98BD984#)\n"
      "      (Xq1 #1A5CF72EE770DE50CB09ACCEA9#)\n"
      "      (Xq2 #134E4CAA16D2350A21D775C404#)\n"
      "      (Xq  #CC1092495D867E64065DEE3E7955F2EBC7D47A2D\n"
      "            7C9953388F97DDDC3E1CA19C35CA659EDC2FC325\n"
      "            6D29C2627479C086A699A49C4C9CEE7EF7BD1B34\n"
      "            321DE34A#))))\n",
      "1CCDA20BCFFB8D517EE9666866621B11822C7950D55F4BB5BEE37989A7D173"
      "12E326718BE0D79546EAAE87A56623B919B1715FFBD7F16028FC4007741961"
      "C88C5D7B4DAAAC8D36A98C9EFBB26C8A4A0E6BC15B358E528A1AC9D0F042BE"
      "B93BCA16B541B33F80C933A3B769285C462ED5677BFE89DF07BED5C127FD13"
      "241D3C4B"
    },

    { /* Second example from X9.31 (D.2.1).  */
      "(genkey\n"
      "  (rsa\n"
      "    (nbits 4:1536)\n"
      "    (rsa-use-e 1:3)\n"
      "    (derive-parms\n"
      "      (Xp1 #18272558B61316348297EACA74#)\n"
      "      (Xp2 #1E970E8C6C97CEF91F05B0FA80#)\n"
      "      (Xp  #F7E943C7EF2169E930DCF23FE389EF7507EE8265\n"
      "            0D42F4A0D3A3CEFABE367999BB30EE680B2FE064\n"
      "            60F707F46005F8AA7CBFCDDC4814BBE7F0F8BC09\n"
      "            318C8E51A48D134296E40D0BBDD282DCCBDDEE1D\n"
      "            EC86F0B1C96EAFF5CDA70F9AEB6EE31E#)\n"
      "      (Xq1 #11FDDA6E8128DC1629F75192BA#)\n"
      "      (Xq2 #18AB178ECA907D72472F65E480#)\n"
      "      (Xq  #C47560011412D6E13E3E7D007B5C05DBF5FF0D0F\n"
      "            CFF1FA2070D16C7ABA93EDFB35D8700567E5913D\n"
      "            B734E3FBD15862EBC59FA0425DFA131E549136E8\n"
      "            E52397A8ABE4705EC4877D4F82C4AAC651B33DA6\n"
      "            EA14B9D5F2A263DC65626E4D6CEAC767#))))\n",
      "1FB56069985F18C4519694FB71055721A01F14422DC901C35B03A64D4A5BD1"
      "259D573305F5B056AC931B82EDB084E39A0FD1D1A86CC5B147A264F7EF4EB2"
      "0ED1E7FAAE5CAE4C30D5328B7F74C3CAA72C88B70DED8EDE207B8629DA2383"
      "B78C3CE1CA3F9F218D78C938B35763AF2A8714664CC57F5CECE2413841F5E9"
      "EDEC43B728E25A41BF3E1EF8D9EEE163286C9F8BF0F219D3B322C3E4B0389C"
      "2E8BB28DC04C47DA2BF38823731266D2CF6CC3FC181738157624EF051874D0"
      "BBCCB9F65C83"
      /* Note that this example in X9.31 gives this value for D:

        "7ED581A6617C6311465A53EDC4155C86807C5108B724070D6C0E9935296F44"
        "96755CCC17D6C15AB24C6E0BB6C2138E683F4746A1B316C51E8993DFBD3AC8"
        "3B479FEAB972B930C354CA2DFDD30F2A9CB222DC37B63B7881EE18A7688E0E"
        "DE30F38728FE7C8635E324E2CD5D8EBCAA1C51993315FD73B38904E107D7A7"
        "B7B10EDCA3896906FCF87BE367BB858CA1B27E2FC3C8674ECC8B0F92C0E270"
        "BA2ECA3701311F68AFCE208DCC499B4B3DB30FF0605CE055D893BC1461D342"
        "EF32E7D9720B"

        This is a bug in X9.31, obviously introduced by using

           d = e^{-1} mod (p-1)(q-1)

         instead of using the universal exponent as required by 4.1.3:

           d = e^{-1} mod lcm(p-1,q-1)

         The examples in X9.31 seem to be pretty buggy, see
         cipher/primegen.c for another bug.  Not only that I had to
         spend 100 USD for the 66 pages of the document, it also took
         me several hours to figure out that the bugs are in the
         document and not in my code.
       */
    },

    { /* First example from NIST RSAVS (B.1.1).  */
      "(genkey\n"
      "  (rsa\n"
      "    (nbits 4:1024)\n"
      "    (rsa-use-e 1:3)\n"
      "    (derive-parms\n"
      "      (Xp1 #1ed3d6368e101dab9124c92ac8#)\n"
      "      (Xp2 #16e5457b8844967ce83cab8c11#)\n"
      "      (Xp  #b79f2c2493b4b76f329903d7555b7f5f06aaa5ea\n"
      "            ab262da1dcda8194720672a4e02229a0c71f60ae\n"
      "            c4f0d2ed8d49ef583ca7d5eeea907c10801c302a\n"
      "            cab44595#)\n"
      "      (Xq1 #1a5d9e3fa34fb479bedea412f6#)\n"
      "      (Xq2 #1f9cca85f185341516d92e82fd#)\n"
      "      (Xq  #c8387fd38fa33ddcea6a9de1b2d55410663502db\n"
      "            c225655a9310cceac9f4cf1bce653ec916d45788\n"
      "            f8113c46bc0fa42bf5e8d0c41120c1612e2ea8bb\n"
      "            2f389eda#))))\n",
      "17ef7ad4fd96011b62d76dfb2261b4b3270ca8e07bc501be954f8719ef586b"
      "f237e8f693dd16c23e7adecc40279dc6877c62ab541df5849883a5254fccfd"
      "4072a657b7f4663953930346febd6bbd82f9a499038402cbf97fd5f068083a"
      "c81ad0335c4aab0da19cfebe060a1bac7482738efafea078e21df785e56ea0"
      "dc7e8feb"
    },

    { /* Second example from NIST RSAVS (B.1.1).  */
      "(genkey\n"
      "  (rsa\n"
      "    (nbits 4:1536)\n"
      "    (rsa-use-e 1:3)\n"
      "    (derive-parms\n"
      "      (Xp1 #1e64c1af460dff8842c22b64d0#)\n"
      "      (Xp2 #1e948edcedba84039c81f2ac0c#)\n"
      "      (Xp  #c8c67df894c882045ede26a9008ab09ea0672077\n"
      "            d7bc71d412511cd93981ddde8f91b967da404056\n"
      "            c39f105f7f239abdaff92923859920f6299e82b9\n"
      "            5bd5b8c959948f4a034d81613d6235a3953b49ce\n"
      "            26974eb7bb1f14843841281b363b9cdb#)\n"
      "      (Xq1 #1f3df0f017ddd05611a97b6adb#)\n"
      "      (Xq2 #143edd7b22d828913abf24ca4d#)\n"
      "      (Xq  #f15147d0e7c04a1e3f37adde802cdc610999bf7a\n"
      "            b0088434aaeda0c0ab3910b14d2ce56cb66bffd9\n"
      "            7552195fae8b061077e03920814d8b9cfb5a3958\n"
      "            b3a82c2a7fc97e55db543948d3396289245336ec\n"
      "            9e3cb308cc655aebd766340da8921383#))))\n",
      "1f8b19f3f5f2ac9fc599f110cad403dcd9bdf5f7f00fb2790e78e820398184"
      "1f3fb3dd230fb223d898f45719d9b2d3525587ff2b8bcc7425e40550a5b536"
      "1c8e9c1d26e83fbd9c33c64029c0e878b829d55def12912b73d94fd758c461"
      "0f473e230c41b5e4c86e27c5a5029d82c811c88525d0269b95bd2ff272994a"
      "dbd80f2c2ecf69065feb8abd8b445b9c6d306b1585d7d3d7576d49842bc7e2"
      "8b4a2f88f4a47e71c3edd35fdf83f547ea5c2b532975c551ed5268f748b2c4"
      "2ccf8a84835b"
    }
  };
  gpg_error_t err;
  gcry_sexp_t key_spec = NULL, key = NULL, pub_key = NULL, sec_key = NULL;
  gcry_mpi_t d_expected = NULL, d_have = NULL;

  if (what < 0 && what >= sizeof testtable)
    die ("invalid WHAT value\n");

  err = gcry_sexp_new (&key_spec, testtable[what].param, 0, 1);
  if (err)
    die ("error creating S-expression [%d]: %s\n", what, gpg_strerror (err));

  {
    unsigned nbits;
    err = _gcry_pk_util_get_nbits(key_spec, &nbits);
    if (err)
      die ("nbits not found\n");
    if (in_fips_mode && nbits < 2048)
      {
        info("RSA key test with %d bits skipped in fips mode\n", nbits);
        gcry_sexp_release (key_spec);
        goto leave;
      }
  }

  err = gcry_pk_genkey (&key, key_spec);
  gcry_sexp_release (key_spec);
  if (err)
    {
      fail ("error generating RSA key [%d]: %s\n", what, gpg_strerror (err));
      goto leave;
    }

  pub_key = gcry_sexp_find_token (key, "public-key", 0);
  if (!pub_key)
    die ("public part missing in key [%d]\n", what);

  sec_key = gcry_sexp_find_token (key, "private-key", 0);
  if (!sec_key)
    die ("private part missing in key [%d]\n", what);

  err = gcry_mpi_scan
    (&d_expected, GCRYMPI_FMT_HEX, testtable[what].expected_d, 0, NULL);
  if (err)
    die ("error converting string [%d]\n", what);

  if (verbose > 1)
    show_sexp ("generated key:\n", key);

  d_have = key_param_from_sexp (sec_key, "rsa", "d");
  if (!d_have)
    die ("parameter d not found in RSA secret key [%d]\n", what);
  if (gcry_mpi_cmp (d_expected, d_have))
    {
      show_sexp (NULL, sec_key);
      die ("parameter d does match expected value [%d]\n", what);
    }
leave:
  gcry_mpi_release (d_expected);
  gcry_mpi_release (d_have);

  gcry_sexp_release (key);
  gcry_sexp_release (pub_key);
  gcry_sexp_release (sec_key);
}



static void
check_ecc_sample_key (void)
{
  static const char ecc_private_key[] =
    "(private-key\n"
    " (ecdsa\n"
    "  (curve \"NIST P-256\")\n"
    "  (q #04D4F6A6738D9B8D3A7075C1E4EE95015FC0C9B7E4272D2BEB6644D3609FC781"
    "B71F9A8072F58CB66AE2F89BB12451873ABF7D91F9E1FBF96BF2F70E73AAC9A283#)\n"
    "  (d #5A1EF0035118F19F3110FB81813D3547BCE1E5BCE77D1F744715E1D5BBE70378#)"
    "))";
  static const char ecc_private_key_wo_q[] =
    "(private-key\n"
    " (ecdsa\n"
    "  (curve \"NIST P-256\")\n"
    "  (d #5A1EF0035118F19F3110FB81813D3547BCE1E5BCE77D1F744715E1D5BBE70378#)"
    "))";
  static const char ecc_public_key[] =
    "(public-key\n"
    " (ecdsa\n"
    "  (curve \"NIST P-256\")\n"
    "  (q #04D4F6A6738D9B8D3A7075C1E4EE95015FC0C9B7E4272D2BEB6644D3609FC781"
    "B71F9A8072F58CB66AE2F89BB12451873ABF7D91F9E1FBF96BF2F70E73AAC9A283#)"
    "))";
  static const char hash_string[] =
    "(data (flags raw)\n"
    " (value #00112233445566778899AABBCCDDEEFF"
    /* */    "000102030405060708090A0B0C0D0E0F#))";
  static const char hash2_string[] =
    "(data (flags raw)\n"
    " (hash sha1 #00112233445566778899AABBCCDDEEFF"
    /* */    "000102030405060708090A0B0C0D0E0F"
    /* */    "000102030405060708090A0B0C0D0E0F"
    /* */    "00112233445566778899AABBCCDDEEFF#))";
  /* hash2, but longer than curve length, so it will be truncated */
  static const char hash3_string[] =
    "(data (flags raw)\n"
    " (hash sha1 #00112233445566778899AABBCCDDEEFF"
    /* */    "000102030405060708090A0B0C0D0E0F"
    /* */    "000102030405060708090A0B0C0D0E0F"
    /* */    "00112233445566778899AABBCCDDEEFF"
    /* */    "000102030405060708090A0B0C0D0E0F#))";

  gpg_error_t err;
  gcry_sexp_t key, hash, hash2, hash3, sig, sig2;

  if (verbose)
    fprintf (stderr, "Checking sample ECC key.\n");

  if ((err = gcry_sexp_new (&hash, hash_string, 0, 1)))
    die ("line %d: %s", __LINE__, gpg_strerror (err));

  if ((err = gcry_sexp_new (&hash2, hash2_string, 0, 1)))
    die ("line %d: %s", __LINE__, gpg_strerror (err));

  if ((err = gcry_sexp_new (&hash3, hash3_string, 0, 1)))
    die ("line %d: %s", __LINE__, gpg_strerror (err));

  if ((err = gcry_sexp_new (&key, ecc_private_key, 0, 1)))
    die ("line %d: %s", __LINE__, gpg_strerror (err));

  if ((err = gcry_pk_sign (&sig, hash, key)))
    die ("gcry_pk_sign failed: %s", gpg_strerror (err));

  gcry_sexp_release (key);
  if ((err = gcry_sexp_new (&key, ecc_public_key, 0, 1)))
    die ("line %d: %s", __LINE__, gpg_strerror (err));

  if ((err = gcry_pk_verify (sig, hash, key)))
    die ("gcry_pk_verify failed: %s", gpg_strerror (err));

  /* Verify hash truncation */
  gcry_sexp_release (key);
  if ((err = gcry_sexp_new (&key, ecc_private_key, 0, 1)))
    die ("line %d: %s", __LINE__, gpg_strerror (err));

  if ((err = gcry_pk_sign (&sig2, hash2, key)))
    die ("gcry_pk_sign failed: %s", gpg_strerror (err));

  gcry_sexp_release (sig);
  if ((err = gcry_pk_sign (&sig, hash3, key)))
    die ("gcry_pk_sign failed: %s", gpg_strerror (err));

  gcry_sexp_release (key);
  if ((err = gcry_sexp_new (&key, ecc_public_key, 0, 1)))
    die ("line %d: %s", __LINE__, gpg_strerror (err));

  if ((err = gcry_pk_verify (sig, hash2, key)))
    die ("gcry_pk_verify failed: %s", gpg_strerror (err));

  if ((err = gcry_pk_verify (sig2, hash3, key)))
    die ("gcry_pk_verify failed: %s", gpg_strerror (err));

  /* Now try signing without the Q parameter.  */

  gcry_sexp_release (key);
  if ((err = gcry_sexp_new (&key, ecc_private_key_wo_q, 0, 1)))
    die ("line %d: %s", __LINE__, gpg_strerror (err));

  gcry_sexp_release (sig);
  if ((err = gcry_pk_sign (&sig, hash, key)))
    die ("gcry_pk_sign without Q failed: %s", gpg_strerror (err));

  gcry_sexp_release (key);
  if ((err = gcry_sexp_new (&key, ecc_public_key, 0, 1)))
    die ("line %d: %s", __LINE__, gpg_strerror (err));

  if ((err = gcry_pk_verify (sig, hash, key)))
    die ("gcry_pk_verify signed without Q failed: %s", gpg_strerror (err));

  gcry_sexp_release (sig);
  gcry_sexp_release (sig2);
  gcry_sexp_release (key);
  gcry_sexp_release (hash);
  gcry_sexp_release (hash2);
  gcry_sexp_release (hash3);
}


static void
check_ed25519ecdsa_sample_key (void)
{
  static const char ecc_private_key[] =
    "(private-key\n"
    " (ecc\n"
    "  (curve \"Ed25519\")\n"
    "  (q #044C056555BE4084BB3D8D8895FDF7C2893DFE0256251923053010977D12658321"
    "        156D1ADDC07987713A418783658B476358D48D582DB53233D9DED3C1C2577B04#)"
    "  (d #09A0C38E0F1699073541447C19DA12E3A07A7BFDB0C186E4AC5BCE6F23D55252#)"
    "))";
  static const char ecc_private_key_wo_q[] =
    "(private-key\n"
    " (ecc\n"
    "  (curve \"Ed25519\")\n"
    "  (d #09A0C38E0F1699073541447C19DA12E3A07A7BFDB0C186E4AC5BCE6F23D55252#)"
    "))";
  static const char ecc_public_key[] =
    "(public-key\n"
    " (ecc\n"
    "  (curve \"Ed25519\")\n"
    "  (q #044C056555BE4084BB3D8D8895FDF7C2893DFE0256251923053010977D12658321"
    "        156D1ADDC07987713A418783658B476358D48D582DB53233D9DED3C1C2577B04#)"
    "))";
  static const char ecc_public_key_comp[] =
    "(public-key\n"
    " (ecc\n"
    "  (curve \"Ed25519\")\n"
    "  (q #047b57c2c1d3ded93332b52d588dd45863478b658387413a718779c0dd1a6d95#)"
    "))";
  static const char hash_string[] =
    "(data (flags rfc6979)\n"
    " (hash sha256 #00112233445566778899AABBCCDDEEFF"
    /* */          "000102030405060708090A0B0C0D0E0F#))";

  gpg_error_t err;
  gcry_sexp_t key, hash, sig;

  if (verbose)
    fprintf (stderr, "Checking sample Ed25519/ECDSA key.\n");

  /* Sign.  */
  if ((err = gcry_sexp_new (&hash, hash_string, 0, 1)))
    die ("line %d: %s", __LINE__, gpg_strerror (err));
  if ((err = gcry_sexp_new (&key, ecc_private_key, 0, 1)))
    die ("line %d: %s", __LINE__, gpg_strerror (err));
  if ((err = gcry_pk_sign (&sig, hash, key)))
    die ("gcry_pk_sign failed: %s", gpg_strerror (err));

  /* Verify.  */
  gcry_sexp_release (key);
  if ((err = gcry_sexp_new (&key, ecc_public_key, 0, 1)))
    die ("line %d: %s", __LINE__, gpg_strerror (err));
  if ((err = gcry_pk_verify (sig, hash, key)))
    die ("gcry_pk_verify failed: %s", gpg_strerror (err));

  /* Verify again using a compressed public key.  */
  gcry_sexp_release (key);
  if ((err = gcry_sexp_new (&key, ecc_public_key_comp, 0, 1)))
    die ("line %d: %s", __LINE__, gpg_strerror (err));
  if ((err = gcry_pk_verify (sig, hash, key)))
    die ("gcry_pk_verify failed (comp): %s", gpg_strerror (err));

  /* Sign without a Q parameter.  */
  gcry_sexp_release (key);
  if ((err = gcry_sexp_new (&key, ecc_private_key_wo_q, 0, 1)))
    die ("line %d: %s", __LINE__, gpg_strerror (err));
  gcry_sexp_release (sig);
  if ((err = gcry_pk_sign (&sig, hash, key)))
    die ("gcry_pk_sign w/o Q failed: %s", gpg_strerror (err));

  /* Verify.  */
  gcry_sexp_release (key);
  if ((err = gcry_sexp_new (&key, ecc_public_key, 0, 1)))
    die ("line %d: %s", __LINE__, gpg_strerror (err));
  if ((err = gcry_pk_verify (sig, hash, key)))
    die ("gcry_pk_verify signed w/o Q failed: %s", gpg_strerror (err));

  /* Verify again using a compressed public key.  */
  gcry_sexp_release (key);
  if ((err = gcry_sexp_new (&key, ecc_public_key_comp, 0, 1)))
    die ("line %d: %s", __LINE__, gpg_strerror (err));
  if ((err = gcry_pk_verify (sig, hash, key)))
    die ("gcry_pk_verify signed w/o Q failed (comp): %s", gpg_strerror (err));

  extract_cmp_data (sig, "r", ("a63123a783ef29b8276e08987daca4"
                               "655d0179e22199bf63691fd88eb64e15"));
  extract_cmp_data (sig, "s", ("0d9b45c696ab90b96b08812b485df185"
                               "623ddaf5d02fa65ca5056cb6bd0f16f1"));

  gcry_sexp_release (sig);
  gcry_sexp_release (key);
  gcry_sexp_release (hash);
}


int
main (int argc, char **argv)
{
  int i;

  if (argc > 1 && !strcmp (argv[1], "--verbose"))
    verbose = 1;
  else if (argc > 1 && !strcmp (argv[1], "--debug"))
    {
      verbose = 2;
      debug = 1;
    }

  xgcry_control ((GCRYCTL_DISABLE_SECMEM, 0));
  if (!gcry_check_version (GCRYPT_VERSION))
    die ("version mismatch\n");
  xgcry_control ((GCRYCTL_INITIALIZATION_FINISHED, 0));
  if (debug)
    xgcry_control ((GCRYCTL_SET_DEBUG_FLAGS, 1u , 0));
  /* No valuable keys are create, so we can speed up our RNG. */
  xgcry_control ((GCRYCTL_ENABLE_QUICK_RANDOM, 0));

  if (gcry_fips_mode_active ())
    in_fips_mode = 1;

  for (i=0; i < 2; i++)
    check_run ();

  for (i=0; i < 4; i++)
    check_x931_derived_key (i);

  check_ecc_sample_key ();
  if (!in_fips_mode)
    check_ed25519ecdsa_sample_key ();

  return !!error_count;
}
