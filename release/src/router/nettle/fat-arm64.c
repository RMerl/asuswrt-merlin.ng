/* fat-arm64.c

   Copyright (C) 2021 Mamone Tarsha

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

#if defined(__linux__) && defined(__GLIBC__) && defined(__GLIBC_PREREQ)
# if __GLIBC_PREREQ(2, 16)
#  define USE_GETAUXVAL 1
#  include <asm/hwcap.h>
#  include <sys/auxv.h>
# endif
#elif defined(__OpenBSD__)
# include <sys/sysctl.h>
# include <machine/cpu.h>
# include <machine/armreg.h>
#endif

#include "nettle-types.h"

#include "aes.h"
#include "ghash-internal.h"
#include "fat-setup.h"

/* Defines from arch/arm64/include/uapi/asm/hwcap.h in Linux kernel */
#ifndef HWCAP_ASIMD
#define HWCAP_ASIMD (1 << 1)
#endif
#ifndef HWCAP_AES
#define HWCAP_AES (1 << 3)
#endif
#ifndef HWCAP_PMULL
#define HWCAP_PMULL (1 << 4)
#endif
#ifndef HWCAP_SHA1
#define HWCAP_SHA1 (1 << 5)
#endif
#ifndef HWCAP_SHA2
#define HWCAP_SHA2 (1 << 6)
#endif

struct arm64_features
{
  int have_aes;
  int have_pmull;
  int have_sha1;
  int have_sha2;
};

#define MATCH(s, slen, literal, llen) \
  ((slen) == (llen) && memcmp ((s), (literal), llen) == 0)

static void
get_arm64_features (struct arm64_features *features)
{
  const char *s;
  features->have_aes = 0;
  features->have_pmull = 0;
  features->have_sha1 = 0;
  features->have_sha2 = 0;

  s = secure_getenv (ENV_OVERRIDE);
  if (s)
    for (;;)
      {
	const char *sep = strchr (s, ',');
	size_t length = sep ? (size_t) (sep - s) : strlen(s);

	if (MATCH (s, length, "aes", 3))
	  features->have_aes = 1;
  else if (MATCH (s, length, "pmull", 5))
	  features->have_pmull = 1;
  else if (MATCH (s, length, "sha1", 4))
	  features->have_sha1 = 1;
  else if (MATCH (s, length, "sha2", 4))
	  features->have_sha2 = 1;
	if (!sep)
	  break;
	s = sep + 1;
      }
  else
    {
#if USE_GETAUXVAL
      unsigned long hwcap = getauxval(AT_HWCAP);
      features->have_aes
	= ((hwcap & (HWCAP_ASIMD | HWCAP_AES)) == (HWCAP_ASIMD | HWCAP_AES));
      features->have_pmull
	= ((hwcap & (HWCAP_ASIMD | HWCAP_PMULL)) == (HWCAP_ASIMD | HWCAP_PMULL));
      features->have_sha1
	= ((hwcap & (HWCAP_ASIMD | HWCAP_SHA1)) == (HWCAP_ASIMD | HWCAP_SHA1));
      features->have_sha2
	= ((hwcap & (HWCAP_ASIMD | HWCAP_SHA2)) == (HWCAP_ASIMD | HWCAP_SHA2));
#elif defined(__OpenBSD__)
      const int isar0_mib[] = { CTL_MACHDEP, CPU_ID_AA64ISAR0 };
      uint64_t isar0;
      size_t len = sizeof(isar0);

      if (sysctl(isar0_mib, 2, &isar0, &len, NULL, 0) < 0)
        return;
      features->have_aes
        = (ID_AA64ISAR0_AES(isar0) >= ID_AA64ISAR0_AES_BASE);
      features->have_pmull
        = (ID_AA64ISAR0_AES(isar0) >= ID_AA64ISAR0_AES_PMULL);
      features->have_sha1
        = (ID_AA64ISAR0_SHA1(isar0) >= ID_AA64ISAR0_SHA1_BASE);
      features->have_sha2
        = (ID_AA64ISAR0_SHA2(isar0) >= ID_AA64ISAR0_SHA2_BASE);
#endif
    }
}

DECLARE_FAT_FUNC(nettle_aes128_encrypt, aes128_crypt_func)
DECLARE_FAT_FUNC_VAR(aes128_encrypt, aes128_crypt_func, c)
DECLARE_FAT_FUNC_VAR(aes128_encrypt, aes128_crypt_func, arm64)
DECLARE_FAT_FUNC(nettle_aes128_decrypt, aes128_crypt_func)
DECLARE_FAT_FUNC_VAR(aes128_decrypt, aes128_crypt_func, c)
DECLARE_FAT_FUNC_VAR(aes128_decrypt, aes128_crypt_func, arm64)

DECLARE_FAT_FUNC(nettle_aes192_encrypt, aes192_crypt_func)
DECLARE_FAT_FUNC_VAR(aes192_encrypt, aes192_crypt_func, c)
DECLARE_FAT_FUNC_VAR(aes192_encrypt, aes192_crypt_func, arm64)
DECLARE_FAT_FUNC(nettle_aes192_decrypt, aes192_crypt_func)
DECLARE_FAT_FUNC_VAR(aes192_decrypt, aes192_crypt_func, c)
DECLARE_FAT_FUNC_VAR(aes192_decrypt, aes192_crypt_func, arm64)

DECLARE_FAT_FUNC(nettle_aes256_encrypt, aes256_crypt_func)
DECLARE_FAT_FUNC_VAR(aes256_encrypt, aes256_crypt_func, c)
DECLARE_FAT_FUNC_VAR(aes256_encrypt, aes256_crypt_func, arm64)
DECLARE_FAT_FUNC(nettle_aes256_decrypt, aes256_crypt_func)
DECLARE_FAT_FUNC_VAR(aes256_decrypt, aes256_crypt_func, c)
DECLARE_FAT_FUNC_VAR(aes256_decrypt, aes256_crypt_func, arm64)

DECLARE_FAT_FUNC(_nettle_ghash_set_key, ghash_set_key_func)
DECLARE_FAT_FUNC_VAR(ghash_set_key, ghash_set_key_func, c)
DECLARE_FAT_FUNC_VAR(ghash_set_key, ghash_set_key_func, arm64)

DECLARE_FAT_FUNC(_nettle_ghash_update, ghash_update_func)
DECLARE_FAT_FUNC_VAR(ghash_update, ghash_update_func, c)
DECLARE_FAT_FUNC_VAR(ghash_update, ghash_update_func, arm64)

DECLARE_FAT_FUNC(nettle_sha1_compress, sha1_compress_func)
DECLARE_FAT_FUNC_VAR(sha1_compress, sha1_compress_func, c)
DECLARE_FAT_FUNC_VAR(sha1_compress, sha1_compress_func, arm64)

DECLARE_FAT_FUNC(_nettle_sha256_compress, sha256_compress_func)
DECLARE_FAT_FUNC_VAR(sha256_compress, sha256_compress_func, c)
DECLARE_FAT_FUNC_VAR(sha256_compress, sha256_compress_func, arm64)

static void CONSTRUCTOR
fat_init (void)
{
  struct arm64_features features;
  int verbose;

  get_arm64_features (&features);

  verbose = getenv (ENV_VERBOSE) != NULL;
  if (verbose)
    fprintf (stderr, "libnettle: cpu features:%s%s%s%s\n",
	     features.have_aes ? " aes instructions" : "",
	     features.have_pmull ? " polynomial multiply long instructions (PMULL/PMULL2)" : "",
       features.have_sha1 ? " sha1 instructions" : "",
       features.have_sha2 ? " sha2 instructions" : "");

  if (features.have_aes)
  {
    if (verbose)
      fprintf (stderr, "libnettle: enabling hardware accelerated AES encrypt/decrypt code.\n");
    nettle_aes128_encrypt_vec = _nettle_aes128_encrypt_arm64;
    nettle_aes128_decrypt_vec = _nettle_aes128_decrypt_arm64;
    nettle_aes192_encrypt_vec = _nettle_aes192_encrypt_arm64;
    nettle_aes192_decrypt_vec = _nettle_aes192_decrypt_arm64;
    nettle_aes256_encrypt_vec = _nettle_aes256_encrypt_arm64;
    nettle_aes256_decrypt_vec = _nettle_aes256_decrypt_arm64;
  }
  else
  {
    nettle_aes128_encrypt_vec = _nettle_aes128_encrypt_c;
    nettle_aes128_decrypt_vec = _nettle_aes128_decrypt_c;
    nettle_aes192_encrypt_vec = _nettle_aes192_encrypt_c;
    nettle_aes192_decrypt_vec = _nettle_aes192_decrypt_c;
    nettle_aes256_encrypt_vec = _nettle_aes256_encrypt_c;
    nettle_aes256_decrypt_vec = _nettle_aes256_decrypt_c;
  }
  
  if (features.have_pmull)
    {
      if (verbose)
	fprintf (stderr, "libnettle: enabling hardware-accelerated polynomial multiply code.\n");

      /* Make sure _nettle_ghash_set_key_vec function is compatible
         with _nettle_ghash_update_vec function e.g. _nettle_ghash_key_c()
         fills gcm_key table with values that are incompatible with
         _nettle_ghash_update_arm64() */
      _nettle_ghash_set_key_vec = _nettle_ghash_set_key_arm64;
      _nettle_ghash_update_vec = _nettle_ghash_update_arm64;
    }
  else
    {
      _nettle_ghash_set_key_vec = _nettle_ghash_set_key_c;
      _nettle_ghash_update_vec = _nettle_ghash_update_c;
    }
  if (features.have_sha1)
    {
      if (verbose)
	fprintf (stderr, "libnettle: enabling hardware-accelerated sha1 compress code.\n");
      nettle_sha1_compress_vec = _nettle_sha1_compress_arm64;
    }
  else
    {
      nettle_sha1_compress_vec = _nettle_sha1_compress_c;
    }
  if (features.have_sha2)
    {
      if (verbose)
	fprintf (stderr, "libnettle: enabling hardware-accelerated sha256 compress code.\n");
      _nettle_sha256_compress_vec = _nettle_sha256_compress_arm64;
    }
  else
    {
      _nettle_sha256_compress_vec = _nettle_sha256_compress_c;
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

DEFINE_FAT_FUNC(_nettle_ghash_set_key, void,
		(struct gcm_key *ctx, const union nettle_block16 *key),
		(ctx, key))
DEFINE_FAT_FUNC(_nettle_ghash_update, const uint8_t *,
		(const struct gcm_key *ctx, union nettle_block16 *state,
		 size_t blocks, const uint8_t *data),
		(ctx, state, blocks, data))

DEFINE_FAT_FUNC(nettle_sha1_compress, void,
		(uint32_t *state, const uint8_t *input),
		(state, input))

DEFINE_FAT_FUNC(_nettle_sha256_compress, void,
		(uint32_t *state, const uint8_t *input, const uint32_t *k),
		(state, input, k))
