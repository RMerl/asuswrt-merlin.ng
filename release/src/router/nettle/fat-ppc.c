/* fat-ppc.c

   Copyright (C) 2020 Mamone Tarsha

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

#if defined(_AIX)
# include <sys/systemcfg.h>
#elif defined(__linux__) && defined(__GLIBC__) && defined(__GLIBC_PREREQ)
# if __GLIBC_PREREQ(2, 16)
#  define USE_GETAUXVAL 1
#  include <asm/cputable.h>
#  include <sys/auxv.h>
# endif
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
# include <machine/cpu.h>
# ifdef PPC_FEATURE2_HAS_VEC_CRYPTO
# define PPC_FEATURE2_VEC_CRYPTO PPC_FEATURE2_HAS_VEC_CRYPTO
# endif
# ifdef HAVE_ELF_AUX_INFO
#  include <sys/auxv.h>
# elif defined(__FreeBSD__)
#  include <sys/sysctl.h>
# endif
#endif

#include "nettle-types.h"

#include "aes-internal.h"
#include "chacha-internal.h"
#include "ghash-internal.h"
#include "poly1305.h"
#include "fat-setup.h"

/* Defines from arch/powerpc/include/uapi/asm/cputable.h in Linux kernel */
#ifndef PPC_FEATURE_HAS_ALTIVEC
#define PPC_FEATURE_HAS_ALTIVEC 0x10000000
#endif
#ifndef PPC_FEATURE_HAS_VSX
#define PPC_FEATURE_HAS_VSX 0x00000080
#endif
#ifndef PPC_FEATURE2_VEC_CRYPTO
#define PPC_FEATURE2_VEC_CRYPTO 0x02000000
#endif
#ifndef PPC_FEATURE2_ARCH_3_00
#define PPC_FEATURE2_ARCH_3_00 0x00800000
#endif

struct ppc_features
{
  int have_crypto_ext;
  int have_altivec;
  int have_power9;
};

#define MATCH(s, slen, literal, llen) \
  ((slen) == (llen) && memcmp ((s), (literal), llen) == 0)

static void
get_ppc_features (struct ppc_features *features)
{
  const char *s;
  features->have_crypto_ext = 0;
  features->have_altivec = 0;
  features->have_power9 = 0;

  s = secure_getenv (ENV_OVERRIDE);
  if (s)
    for (;;)
      {
	const char *sep = strchr (s, ',');
	size_t length = sep ? (size_t) (sep - s) : strlen(s);

	if (MATCH (s, length, "crypto_ext", 10))
	  features->have_crypto_ext = 1;
	else if (MATCH(s, length, "altivec", 7))
	  features->have_altivec = 1;
  else if (MATCH(s, length, "power9", 6))
	  features->have_power9 = 1;
	if (!sep)
	  break;
	s = sep + 1;
      }
  else
    {
#if defined(_AIX)
      features->have_crypto_ext
	= _system_configuration.implementation >= 0x10000u;
      features->have_altivec = _system_configuration.vmx_version > 1;
#else
      unsigned long hwcap = 0;
      unsigned long hwcap2 = 0;
# if USE_GETAUXVAL
      hwcap = getauxval(AT_HWCAP);
      hwcap2 = getauxval(AT_HWCAP2);
# elif defined(HAVE_ELF_AUX_INFO)
      elf_aux_info(AT_HWCAP, &hwcap, sizeof(hwcap));
      elf_aux_info(AT_HWCAP2, &hwcap2, sizeof(hwcap2));
# elif defined(__FreeBSD__)
      size_t len;
      len = sizeof(hwcap);
      sysctlbyname("hw.cpu_features", &hwcap, &len, NULL, 0);
      len = sizeof(hwcap2);
      sysctlbyname("hw.cpu_features2", &hwcap2, &len, NULL, 0);
# endif
      features->have_crypto_ext
	= ((hwcap2 & PPC_FEATURE2_VEC_CRYPTO) == PPC_FEATURE2_VEC_CRYPTO);

      features->have_power9
	= ((hwcap2 & PPC_FEATURE2_ARCH_3_00) == PPC_FEATURE2_ARCH_3_00);

      /* We also need VSX instructions, mainly for load and store. */
      features->have_altivec
	= ((hwcap & (PPC_FEATURE_HAS_ALTIVEC | PPC_FEATURE_HAS_VSX))
	   == (PPC_FEATURE_HAS_ALTIVEC | PPC_FEATURE_HAS_VSX));
#endif
    }
}

DECLARE_FAT_FUNC(_nettle_aes_encrypt, aes_crypt_internal_func)
DECLARE_FAT_FUNC_VAR(aes_encrypt, aes_crypt_internal_func, c)
DECLARE_FAT_FUNC_VAR(aes_encrypt, aes_crypt_internal_func, ppc64)

DECLARE_FAT_FUNC(_nettle_aes_decrypt, aes_crypt_internal_func)
DECLARE_FAT_FUNC_VAR(aes_decrypt, aes_crypt_internal_func, c)
DECLARE_FAT_FUNC_VAR(aes_decrypt, aes_crypt_internal_func, ppc64)

DECLARE_FAT_FUNC(_nettle_aes_invert, aes_invert_internal_func)
DECLARE_FAT_FUNC_VAR(aes_invert, aes_invert_internal_func, c)
DECLARE_FAT_FUNC_VAR(aes_invert, aes_invert_internal_func, ppc64)

DECLARE_FAT_FUNC(_nettle_ghash_set_key, ghash_set_key_func)
DECLARE_FAT_FUNC_VAR(ghash_set_key, ghash_set_key_func, c)
DECLARE_FAT_FUNC_VAR(ghash_set_key, ghash_set_key_func, ppc64)

DECLARE_FAT_FUNC(_nettle_ghash_update, ghash_update_func)
DECLARE_FAT_FUNC_VAR(ghash_update, ghash_update_func, c)
DECLARE_FAT_FUNC_VAR(ghash_update, ghash_update_func, ppc64)

DECLARE_FAT_FUNC(_nettle_gcm_aes_encrypt, gcm_aes_crypt_func)
DECLARE_FAT_FUNC_VAR(gcm_aes_encrypt, gcm_aes_crypt_func, c)
DECLARE_FAT_FUNC_VAR(gcm_aes_encrypt, gcm_aes_crypt_func, ppc64)

DECLARE_FAT_FUNC(_nettle_gcm_aes_decrypt, gcm_aes_crypt_func)
DECLARE_FAT_FUNC_VAR(gcm_aes_decrypt, gcm_aes_crypt_func, c)
DECLARE_FAT_FUNC_VAR(gcm_aes_decrypt, gcm_aes_crypt_func, ppc64)

DECLARE_FAT_FUNC(_nettle_chacha_core, chacha_core_func)
DECLARE_FAT_FUNC_VAR(chacha_core, chacha_core_func, c);
DECLARE_FAT_FUNC_VAR(chacha_core, chacha_core_func, altivec);

DECLARE_FAT_FUNC(nettle_chacha_crypt, chacha_crypt_func)
DECLARE_FAT_FUNC_VAR(chacha_crypt, chacha_crypt_func, 1core)
DECLARE_FAT_FUNC_VAR(chacha_crypt, chacha_crypt_func, 3core)

DECLARE_FAT_FUNC(nettle_chacha_crypt32, chacha_crypt_func)
DECLARE_FAT_FUNC_VAR(chacha_crypt32, chacha_crypt_func, 1core)
DECLARE_FAT_FUNC_VAR(chacha_crypt32, chacha_crypt_func, 3core)

DECLARE_FAT_FUNC(_nettle_poly1305_set_key, poly1305_set_key_func)
DECLARE_FAT_FUNC_VAR(poly1305_set_key, poly1305_set_key_func, c)
DECLARE_FAT_FUNC_VAR(poly1305_set_key, poly1305_set_key_func, ppc64)

DECLARE_FAT_FUNC(_nettle_poly1305_block, poly1305_block_func)
DECLARE_FAT_FUNC_VAR(poly1305_block, poly1305_block_func, c)
DECLARE_FAT_FUNC_VAR(poly1305_block, poly1305_block_func, ppc64)

DECLARE_FAT_FUNC(_nettle_poly1305_digest, poly1305_digest_func)
DECLARE_FAT_FUNC_VAR(poly1305_digest, poly1305_digest_func, c)
DECLARE_FAT_FUNC_VAR(poly1305_digest, poly1305_digest_func, ppc64)

DECLARE_FAT_FUNC(_nettle_poly1305_blocks, poly1305_blocks_func)
DECLARE_FAT_FUNC_VAR(poly1305_blocks, poly1305_blocks_func, c)
DECLARE_FAT_FUNC_VAR(poly1305_blocks, poly1305_blocks_func, ppc64)

DECLARE_FAT_FUNC(_nettle_sha256_compress_n, sha256_compress_n_func)
DECLARE_FAT_FUNC_VAR(sha256_compress_n, sha256_compress_n_func, c)
DECLARE_FAT_FUNC_VAR(sha256_compress_n, sha256_compress_n_func, ppc64)

/* Nop implementation for _gcm_aes_encrypt and _gcm_aes_decrypt. */
static size_t
gcm_aes_crypt_c (struct gcm_key *key UNUSED, unsigned rounds UNUSED,
		 size_t size UNUSED, uint8_t *dst UNUSED, const uint8_t *src UNUSED)
{
  return 0;
}

static void CONSTRUCTOR
fat_init (void)
{
  struct ppc_features features;
  int verbose;

  get_ppc_features (&features);

  verbose = getenv (ENV_VERBOSE) != NULL;
  if (verbose)
    fprintf (stderr, "libnettle: cpu features: %s\n",
	     features.have_crypto_ext ? "crypto extensions" : "");

  if (features.have_crypto_ext)
    {
      if (verbose)
	fprintf (stderr, "libnettle: enabling arch 2.07 code.\n");
      _nettle_aes_encrypt_vec = _nettle_aes_encrypt_ppc64;
      _nettle_aes_decrypt_vec = _nettle_aes_decrypt_ppc64;
      _nettle_aes_invert_vec = _nettle_aes_invert_ppc64;

      /* Make sure _nettle_ghash_set_key_vec function is compatible
         with _nettle_ghash_update_vec function e.g. _nettle_ghash_key_c()
         fills gcm_key table with values that are incompatible with
         _nettle_ghash_update_arm64() */
      _nettle_ghash_set_key_vec = _nettle_ghash_set_key_ppc64;
      _nettle_ghash_update_vec = _nettle_ghash_update_ppc64;
      _nettle_gcm_aes_encrypt_vec = _nettle_gcm_aes_encrypt_ppc64;
      _nettle_gcm_aes_decrypt_vec = _nettle_gcm_aes_decrypt_ppc64;
      _nettle_sha256_compress_n_vec = _nettle_sha256_compress_n_ppc64;
    }
  else
    {
      _nettle_aes_encrypt_vec = _nettle_aes_encrypt_c;
      _nettle_aes_decrypt_vec = _nettle_aes_decrypt_c;
      _nettle_aes_invert_vec = _nettle_aes_invert_c;
      _nettle_ghash_set_key_vec = _nettle_ghash_set_key_c;
      _nettle_ghash_update_vec = _nettle_ghash_update_c;
      _nettle_gcm_aes_encrypt_vec = gcm_aes_crypt_c;
      _nettle_gcm_aes_decrypt_vec = gcm_aes_crypt_c;
      _nettle_sha256_compress_n_vec = _nettle_sha256_compress_n_c;
    }
  if (features.have_altivec)
    {
      if (verbose)
	fprintf (stderr, "libnettle: enabling altivec code.\n");
      _nettle_chacha_core_vec = _nettle_chacha_core_altivec;
      nettle_chacha_crypt_vec = _nettle_chacha_crypt_4core;
      nettle_chacha_crypt32_vec = _nettle_chacha_crypt32_4core;
    }
  else
    {
      _nettle_chacha_core_vec = _nettle_chacha_core_c;
      nettle_chacha_crypt_vec = _nettle_chacha_crypt_1core;
      nettle_chacha_crypt32_vec = _nettle_chacha_crypt32_1core;
    }

  if (features.have_power9)
    {
      if (verbose)
	fprintf (stderr, "libnettle: enabling arch 3.00 code.\n");
      _nettle_poly1305_set_key_vec = _nettle_poly1305_set_key_ppc64;
    _nettle_poly1305_block_vec = _nettle_poly1305_block_ppc64;
    _nettle_poly1305_digest_vec = _nettle_poly1305_digest_ppc64;
    _nettle_poly1305_blocks_vec = _nettle_poly1305_blocks_ppc64;
    }
  else
    {
      _nettle_poly1305_set_key_vec = _nettle_poly1305_set_key_c;
    _nettle_poly1305_block_vec = _nettle_poly1305_block_c;
    _nettle_poly1305_digest_vec = _nettle_poly1305_digest_c;
    _nettle_poly1305_blocks_vec = _nettle_poly1305_blocks_c;
    }
}

DEFINE_FAT_FUNC(_nettle_aes_encrypt, void,
 (unsigned rounds, const uint32_t *keys,
 const struct aes_table *T,
 size_t length, uint8_t *dst,
 const uint8_t *src),
 (rounds, keys, T, length, dst, src))

DEFINE_FAT_FUNC(_nettle_aes_decrypt, void,
 (unsigned rounds, const uint32_t *keys,
 const struct aes_table *T,
 size_t length, uint8_t *dst,
 const uint8_t *src),
 (rounds, keys, T, length, dst, src))

DEFINE_FAT_FUNC(_nettle_aes_invert, void,
 (unsigned rounds, uint32_t *dst, const uint32_t *src),
 (rounds, dst, src))

DEFINE_FAT_FUNC(_nettle_ghash_set_key, void,
		(struct gcm_key *ctx, const union nettle_block16 *key),
		(ctx, key))
DEFINE_FAT_FUNC(_nettle_ghash_update, const uint8_t *,
		(const struct gcm_key *ctx, union nettle_block16 *state,
		 size_t blocks, const uint8_t *data),
		(ctx, state, blocks, data))

DEFINE_FAT_FUNC(_nettle_gcm_aes_encrypt, size_t,
		(struct gcm_key *key, unsigned rounds,
                 size_t len, uint8_t *dst, const uint8_t *src),
		(key, rounds, len, dst, src))

DEFINE_FAT_FUNC(_nettle_gcm_aes_decrypt, size_t,
		(struct gcm_key *key, unsigned rounds,
                 size_t len, uint8_t *dst, const uint8_t *src),
		(key, rounds, len, dst, src))

DEFINE_FAT_FUNC(_nettle_chacha_core, void,
		(uint32_t *dst, const uint32_t *src, unsigned rounds),
		(dst, src, rounds))

DEFINE_FAT_FUNC(nettle_chacha_crypt, void,
		(struct chacha_ctx *ctx,
		 size_t length,
		 uint8_t *dst,
		 const uint8_t *src),
		(ctx, length, dst, src))

DEFINE_FAT_FUNC(nettle_chacha_crypt32, void,
		(struct chacha_ctx *ctx,
		 size_t length,
		 uint8_t *dst,
		 const uint8_t *src),
		(ctx, length, dst, src))

DEFINE_FAT_FUNC(_nettle_poly1305_set_key, void,
		(struct poly1305_ctx *ctx,
     const uint8_t *key),
		(ctx, key))

DEFINE_FAT_FUNC(_nettle_poly1305_block, void,
		(struct poly1305_ctx *ctx,
     const uint8_t *m,
     unsigned high),
		(ctx, m, high))

DEFINE_FAT_FUNC(_nettle_poly1305_digest, void,
		(struct poly1305_ctx *ctx,
     union nettle_block16 *s),
		(ctx, s))

DEFINE_FAT_FUNC(_nettle_poly1305_blocks, const uint8_t *,
		(struct poly1305_ctx *ctx,
     size_t blocks,
		 const uint8_t *m),
		(ctx, blocks, m))

DEFINE_FAT_FUNC(_nettle_sha256_compress_n, const uint8_t *,
		(uint32_t *state, const uint32_t *k,
		 size_t blocks, const uint8_t *input),
		(state, k, blocks, input))
