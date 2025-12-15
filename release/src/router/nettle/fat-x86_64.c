/* fat-x86_64.c

   Copyright (C) 2015 Niels Möller

   This file is part of GNU Nettle.

   GNU Nettle is free software: you can redistribute it and/or
   modify it under the terms of either:

     * the GNU Lesser General Public License as published by the Free
       Software Foundation; either version 3 of the License, or (at your
       option) any later version.

   or

     * the GNU General Public License as published by the Free
       Software Foundation; either version 2 of the License, or (at your
       option) any later version.

   or both in parallel, as here.

   GNU Nettle is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received copies of the GNU General Public License and
   the GNU Lesser General Public License along with this program.  If
   not, see http://www.gnu.org/licenses/.
*/

#define _GNU_SOURCE

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nettle-types.h"

#include "aes-internal.h"
#include "ghash-internal.h"
#include "memxor.h"
#include "fat-setup.h"

void _nettle_cpuid (uint32_t input, uint32_t regs[4]);

struct x86_features
{
  enum x86_vendor { X86_OTHER, X86_INTEL, X86_AMD } vendor;
  int have_aesni;
  int have_sha_ni;
  int have_pclmul;
};

#define SKIP(s, slen, literal, llen)				\
  (((slen) >= (llen) && memcmp ((s), (literal), llen) == 0)	\
   ? ((slen) -= (llen), (s) += (llen), 1) : 0)
#define MATCH(s, slen, literal, llen)				\
  ((slen) == (llen) && memcmp ((s), (literal), llen) == 0)

static void
get_x86_features (struct x86_features *features)
{
  const char *s;
  features->vendor = X86_OTHER;
  features->have_aesni = 0;
  features->have_sha_ni = 0;
  features->have_pclmul = 0;

  s = secure_getenv (ENV_OVERRIDE);
  if (s)
    for (;;)
      {
	const char *sep = strchr (s, ',');
	size_t length = sep ? (size_t) (sep - s) : strlen(s);

	if (SKIP (s, length, "vendor:", 7))
	  {
	    if (MATCH(s, length, "intel", 5))
	      features->vendor = X86_INTEL;
	    else if (MATCH(s, length, "amd", 3))
	      features->vendor = X86_AMD;
	    
	  }
	else if (MATCH (s, length, "aesni", 5))
	  features->have_aesni = 1;
	else if (MATCH (s, length, "sha_ni", 6))
	  features->have_sha_ni = 1;
	else if (MATCH (s, length, "pclmul", 6))
	  features->have_pclmul = 1;
	if (!sep)
	  break;
	s = sep + 1;
      }
  else
    {
      uint32_t cpuid_data[4];
      _nettle_cpuid (0, cpuid_data);
      if (memcmp (cpuid_data + 1, "Genu" "ntel" "ineI", 12) == 0)
	features->vendor = X86_INTEL;
      else if (memcmp (cpuid_data + 1, "Auth" "cAMD" "enti", 12) == 0)
	features->vendor = X86_AMD;

      _nettle_cpuid (1, cpuid_data);
      if (cpuid_data[2] & 0x2)
	features->have_pclmul = 1;
      if (cpuid_data[2] & 0x02000000)
	features->have_aesni = 1;

      _nettle_cpuid (7, cpuid_data);
      if (cpuid_data[1] & 0x20000000)
	features->have_sha_ni = 1;
    }
}

DECLARE_FAT_FUNC(nettle_aes128_encrypt, aes128_crypt_func)
DECLARE_FAT_FUNC(nettle_aes128_decrypt, aes128_crypt_func)
DECLARE_FAT_FUNC_VAR(aes128_encrypt, aes128_crypt_func, c)
DECLARE_FAT_FUNC_VAR(aes128_encrypt, aes128_crypt_func, aesni)
DECLARE_FAT_FUNC_VAR(aes128_decrypt, aes128_crypt_func, c)
DECLARE_FAT_FUNC_VAR(aes128_decrypt, aes128_crypt_func, aesni)
DECLARE_FAT_FUNC(nettle_aes192_encrypt, aes192_crypt_func)
DECLARE_FAT_FUNC(nettle_aes192_decrypt, aes192_crypt_func)
DECLARE_FAT_FUNC_VAR(aes192_encrypt, aes192_crypt_func, c)
DECLARE_FAT_FUNC_VAR(aes192_encrypt, aes192_crypt_func, aesni)
DECLARE_FAT_FUNC_VAR(aes192_decrypt, aes192_crypt_func, c)
DECLARE_FAT_FUNC_VAR(aes192_decrypt, aes192_crypt_func, aesni)
DECLARE_FAT_FUNC(nettle_aes256_encrypt, aes256_crypt_func)
DECLARE_FAT_FUNC(nettle_aes256_decrypt, aes256_crypt_func)
DECLARE_FAT_FUNC_VAR(aes256_encrypt, aes256_crypt_func, c)
DECLARE_FAT_FUNC_VAR(aes256_encrypt, aes256_crypt_func, aesni)
DECLARE_FAT_FUNC_VAR(aes256_decrypt, aes256_crypt_func, c)
DECLARE_FAT_FUNC_VAR(aes256_decrypt, aes256_crypt_func, aesni)

DECLARE_FAT_FUNC(nettle_cbc_aes128_encrypt, cbc_aes128_encrypt_func);
DECLARE_FAT_FUNC_VAR(cbc_aes128_encrypt, cbc_aes128_encrypt_func, c);
DECLARE_FAT_FUNC_VAR(cbc_aes128_encrypt, cbc_aes128_encrypt_func, aesni);
DECLARE_FAT_FUNC(nettle_cbc_aes192_encrypt, cbc_aes192_encrypt_func);
DECLARE_FAT_FUNC_VAR(cbc_aes192_encrypt, cbc_aes192_encrypt_func, c);
DECLARE_FAT_FUNC_VAR(cbc_aes192_encrypt, cbc_aes192_encrypt_func, aesni);
DECLARE_FAT_FUNC(nettle_cbc_aes256_encrypt, cbc_aes256_encrypt_func);
DECLARE_FAT_FUNC_VAR(cbc_aes256_encrypt, cbc_aes256_encrypt_func, c);
DECLARE_FAT_FUNC_VAR(cbc_aes256_encrypt, cbc_aes256_encrypt_func, aesni);

DECLARE_FAT_FUNC(nettle_memxor, memxor_func)
DECLARE_FAT_FUNC_VAR(memxor, memxor_func, x86_64)
DECLARE_FAT_FUNC_VAR(memxor, memxor_func, sse2)

DECLARE_FAT_FUNC(nettle_sha1_compress, sha1_compress_func)
DECLARE_FAT_FUNC_VAR(sha1_compress, sha1_compress_func, x86_64)
DECLARE_FAT_FUNC_VAR(sha1_compress, sha1_compress_func, sha_ni)

DECLARE_FAT_FUNC(_nettle_sha256_compress_n, sha256_compress_n_func)
DECLARE_FAT_FUNC_VAR(sha256_compress_n, sha256_compress_n_func, x86_64)
DECLARE_FAT_FUNC_VAR(sha256_compress_n, sha256_compress_n_func, sha_ni)

DECLARE_FAT_FUNC(_nettle_ghash_set_key, ghash_set_key_func)
DECLARE_FAT_FUNC_VAR(ghash_set_key, ghash_set_key_func, c)
DECLARE_FAT_FUNC_VAR(ghash_set_key, ghash_set_key_func, pclmul)

DECLARE_FAT_FUNC(_nettle_ghash_update, ghash_update_func)
DECLARE_FAT_FUNC_VAR(ghash_update, ghash_update_func, table)
DECLARE_FAT_FUNC_VAR(ghash_update, ghash_update_func, pclmul)


/* This function should usually be called only once, at startup. But
   it is idempotent, and on x86, pointer updates are atomic, so
   there's no danger if it is called simultaneously from multiple
   threads. */
static void CONSTRUCTOR
fat_init (void)
{
  struct x86_features features;
  int verbose;

  /* FIXME: Replace all getenv calls by getenv_secure? */
  verbose = getenv (ENV_VERBOSE) != NULL;
  if (verbose)
    fprintf (stderr, "libnettle: fat library initialization.\n");

  get_x86_features (&features);
  if (verbose)
    {
      const char * const vendor_names[3] =
	{ "other", "intel", "amd" };
      fprintf (stderr, "libnettle: cpu features: vendor:%s%s%s%s\n",
	       vendor_names[features.vendor],
	       features.have_aesni ? ",aesni" : "",
	       features.have_sha_ni ? ",sha_ni" : "",
	       features.have_pclmul ? ",pclmul" : "");
    }
  if (features.have_aesni)
    {
      if (verbose)
	fprintf (stderr, "libnettle: using aes instructions.\n");
      nettle_aes128_encrypt_vec = _nettle_aes128_encrypt_aesni;
      nettle_aes128_decrypt_vec = _nettle_aes128_decrypt_aesni;
      nettle_aes192_encrypt_vec = _nettle_aes192_encrypt_aesni;
      nettle_aes192_decrypt_vec = _nettle_aes192_decrypt_aesni;
      nettle_aes256_encrypt_vec = _nettle_aes256_encrypt_aesni;
      nettle_aes256_decrypt_vec = _nettle_aes256_decrypt_aesni;
      nettle_cbc_aes128_encrypt_vec = _nettle_cbc_aes128_encrypt_aesni;
      nettle_cbc_aes192_encrypt_vec = _nettle_cbc_aes192_encrypt_aesni;
      nettle_cbc_aes256_encrypt_vec = _nettle_cbc_aes256_encrypt_aesni;
    }
  else
    {
      if (verbose)
	fprintf (stderr, "libnettle: not using aes instructions.\n");
      nettle_aes128_encrypt_vec = _nettle_aes128_encrypt_c;
      nettle_aes128_decrypt_vec = _nettle_aes128_decrypt_c;
      nettle_aes192_encrypt_vec = _nettle_aes192_encrypt_c;
      nettle_aes192_decrypt_vec = _nettle_aes192_decrypt_c;
      nettle_aes256_encrypt_vec = _nettle_aes256_encrypt_c;
      nettle_aes256_decrypt_vec = _nettle_aes256_decrypt_c;
      nettle_cbc_aes128_encrypt_vec = _nettle_cbc_aes128_encrypt_c;
      nettle_cbc_aes192_encrypt_vec = _nettle_cbc_aes192_encrypt_c;
      nettle_cbc_aes256_encrypt_vec = _nettle_cbc_aes256_encrypt_c;
    }

  if (features.have_sha_ni)
    {
      if (verbose)
	fprintf (stderr, "libnettle: using sha_ni instructions.\n");
      nettle_sha1_compress_vec = _nettle_sha1_compress_sha_ni;
      _nettle_sha256_compress_n_vec = _nettle_sha256_compress_n_sha_ni;
    }
  else
    {
      if (verbose)
	fprintf (stderr, "libnettle: not using sha_ni instructions.\n");
      nettle_sha1_compress_vec = _nettle_sha1_compress_x86_64;
      _nettle_sha256_compress_n_vec = _nettle_sha256_compress_n_x86_64;
    }

  if (features.have_pclmul)
    {
      if (verbose)
	fprintf (stderr, "libnettle: using pclmulqdq instructions.\n");
      _nettle_ghash_set_key_vec = _nettle_ghash_set_key_pclmul;
      _nettle_ghash_update_vec = _nettle_ghash_update_pclmul;
    }
  else
    {
      if (verbose)
	fprintf (stderr, "libnettle: not using pclmulqdq instructions.\n");
      _nettle_ghash_set_key_vec = _nettle_ghash_set_key_c;
      _nettle_ghash_update_vec = _nettle_ghash_update_table;
    }

  if (features.vendor == X86_INTEL)
    {
      if (verbose)
	fprintf (stderr, "libnettle: intel SSE2 will be used for memxor.\n");
      nettle_memxor_vec = _nettle_memxor_sse2;
    }
  else
    {
      if (verbose)
	fprintf (stderr, "libnettle: intel SSE2 will not be used for memxor.\n");
      nettle_memxor_vec = _nettle_memxor_x86_64;
    }
}

DEFINE_FAT_FUNC(nettle_aes128_encrypt, void,
 (const struct aes128_ctx *ctx, size_t length,
  uint8_t *dst,const uint8_t *src),
 (ctx, length, dst, src))
DEFINE_FAT_FUNC(nettle_aes128_decrypt, void,
 (const struct aes128_ctx *ctx, size_t length,
  uint8_t *dst,const uint8_t *src),
 (ctx, length, dst, src))

DEFINE_FAT_FUNC(nettle_aes192_encrypt, void,
 (const struct aes192_ctx *ctx, size_t length,
  uint8_t *dst,const uint8_t *src),
 (ctx, length, dst, src))
DEFINE_FAT_FUNC(nettle_aes192_decrypt, void,
 (const struct aes192_ctx *ctx, size_t length,
  uint8_t *dst,const uint8_t *src),
 (ctx, length, dst, src))

DEFINE_FAT_FUNC(nettle_aes256_encrypt, void,
 (const struct aes256_ctx *ctx, size_t length,
  uint8_t *dst,const uint8_t *src),
 (ctx, length, dst, src))
DEFINE_FAT_FUNC(nettle_aes256_decrypt, void,
 (const struct aes256_ctx *ctx, size_t length,
  uint8_t *dst,const uint8_t *src),
 (ctx, length, dst, src))

DEFINE_FAT_FUNC(nettle_cbc_aes128_encrypt, void,
 (const struct aes128_ctx *ctx, uint8_t *iv,
  size_t length, uint8_t *dst, const uint8_t *src),
 (ctx, iv, length, dst, src))
DEFINE_FAT_FUNC(nettle_cbc_aes192_encrypt, void,
 (const struct aes192_ctx *ctx, uint8_t *iv,
  size_t length, uint8_t *dst, const uint8_t *src),
 (ctx, iv, length, dst, src))
DEFINE_FAT_FUNC(nettle_cbc_aes256_encrypt, void,
 (const struct aes256_ctx *ctx, uint8_t *iv,
  size_t length, uint8_t *dst, const uint8_t *src),
 (ctx, iv, length, dst, src))

DEFINE_FAT_FUNC(nettle_memxor, void *,
		(void *dst, const void *src, size_t n),
		(dst, src, n))

DEFINE_FAT_FUNC(nettle_sha1_compress, void,
		(uint32_t *state, const uint8_t *input),
		(state, input))

DEFINE_FAT_FUNC(_nettle_sha256_compress_n, const uint8_t *,
		(uint32_t *state, const uint32_t *k,
		 size_t blocks, const uint8_t *input),
		(state, k, blocks, input))

DEFINE_FAT_FUNC(_nettle_ghash_set_key, void,
		(struct gcm_key *ctx, const union nettle_block16 *key),
		(ctx, key))
DEFINE_FAT_FUNC(_nettle_ghash_update, const uint8_t *,
		(const struct gcm_key *ctx, union nettle_block16 *state,
		 size_t blocks, const uint8_t *data),
		(ctx, state, blocks, data))
