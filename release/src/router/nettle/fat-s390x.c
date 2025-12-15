/* fat-s390x.c

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

#if defined(__GLIBC__) && defined(__GLIBC_PREREQ)
# if __GLIBC_PREREQ(2, 16)
#  define USE_GETAUXVAL 1
#  include <sys/auxv.h>
# endif
#endif

#include "nettle-types.h"

#include "memxor.h"
#include "aes-internal.h"
#include "chacha-internal.h"
#include "ghash-internal.h"
#include "fat-setup.h"

/* Max number of doublewords returned by STFLE */
#define FACILITY_DOUBLEWORDS_MAX 3
#define FACILITY_INDEX(bit) ((bit) / 64)
/* STFLE and cipher query store doublewords as bit-reversed.
   reverse facility bit or function code in doubleword */
#define FACILITY_BIT(bit) (1ULL << (63 - (bit) % 64))

/* Define from arch/s390/include/asm/elf.h in Linux kernel */
#ifndef HWCAP_S390_STFLE
#define HWCAP_S390_STFLE 4
#endif

/* Facility bits */
#define FAC_VF 129      /* vector facility */
#define FAC_MSA 17      /* message-security assist */
#define FAC_MSA_X4 77   /* message-security-assist extension 4 */

/* Function codes */
#define AES_128_CODE 18
#define AES_192_CODE 19
#define AES_256_CODE 20
#define SHA_1_CODE 1
#define SHA_256_CODE 2
#define SHA_512_CODE 3
#define GHASH_CODE 65

struct s390x_features
{
  int have_vector_facility;
  int have_km_aes128;
  int have_km_aes192;
  int have_km_aes256;
  int have_kimd_sha_1;
  int have_kimd_sha_256;
  int have_kimd_sha_512;
  int have_kimd_ghash;
};

void _nettle_stfle(uint64_t *facility, uint64_t facility_size);
void _nettle_km_status(uint64_t *status);
void _nettle_kimd_status(uint64_t *status);

#define MATCH(s, slen, literal, llen) \
  ((slen) == (llen) && memcmp ((s), (literal), llen) == 0)

static void
get_s390x_features (struct s390x_features *features)
{
  features->have_vector_facility = 0;
  features->have_km_aes128 = 0;
  features->have_km_aes192 = 0;
  features->have_km_aes256 = 0;
  features->have_kimd_sha_1 = 0;
  features->have_kimd_sha_256 = 0;
  features->have_kimd_sha_512 = 0;
  features->have_kimd_ghash = 0;

  const char *s = secure_getenv (ENV_OVERRIDE);
  if (s)
    for (;;)
    {
      const char *sep = strchr (s, ',');
      size_t length = sep ? (size_t) (sep - s) : strlen(s);

      if (MATCH (s, length, "vf", 2))
        features->have_vector_facility = 1;
      else if (MATCH (s, length, "msa", 3))
        features->have_kimd_sha_1 = 1;
      else if (MATCH (s, length, "msa_x1", 6))
      {
        features->have_km_aes128 = 1;
        features->have_kimd_sha_256 = 1;
      }
      else if (MATCH (s, length, "msa_x2", 6))
      {
        features->have_km_aes192 = 1;
        features->have_km_aes256 = 1;
        features->have_kimd_sha_512 = 1;
      }
      else if (MATCH (s, length, "msa_x4", 6))
        features->have_kimd_ghash = 1;
      if (!sep)
        break;
      s = sep + 1;
    }
  else
  {
#if USE_GETAUXVAL
    unsigned long hwcap = getauxval(AT_HWCAP);
    if (hwcap & HWCAP_S390_STFLE)
    {
      uint64_t facilities[FACILITY_DOUBLEWORDS_MAX] = {0};
      _nettle_stfle(facilities, FACILITY_DOUBLEWORDS_MAX);

      if (facilities[FACILITY_INDEX(FAC_VF)] & FACILITY_BIT(FAC_VF))
        features->have_vector_facility = 1;

      if (facilities[FACILITY_INDEX(FAC_MSA)] & FACILITY_BIT(FAC_MSA))
      {
        uint64_t query_status[2] = {0};
        _nettle_km_status(query_status);
        if (query_status[FACILITY_INDEX(AES_128_CODE)] & FACILITY_BIT(AES_128_CODE))
          features->have_km_aes128 = 1;
        if (query_status[FACILITY_INDEX(AES_192_CODE)] & FACILITY_BIT(AES_192_CODE))
          features->have_km_aes192 = 1;
        if (query_status[FACILITY_INDEX(AES_256_CODE)] & FACILITY_BIT(AES_256_CODE))
          features->have_km_aes256 = 1;
        
        memset(query_status, 0, sizeof(query_status));
        _nettle_kimd_status(query_status);
        if (query_status[FACILITY_INDEX(SHA_1_CODE)] & FACILITY_BIT(SHA_1_CODE))
          features->have_kimd_sha_1 = 1;
        if (query_status[FACILITY_INDEX(SHA_256_CODE)] & FACILITY_BIT(SHA_256_CODE))
          features->have_kimd_sha_256 = 1;
        if (query_status[FACILITY_INDEX(SHA_512_CODE)] & FACILITY_BIT(SHA_512_CODE))
          features->have_kimd_sha_512 = 1;
      }

      if (facilities[FACILITY_INDEX(FAC_MSA_X4)] & FACILITY_BIT(FAC_MSA_X4))
      {
        uint64_t query_status[2] = {0};
        _nettle_kimd_status(query_status);
        if (query_status[FACILITY_INDEX(GHASH_CODE)] & FACILITY_BIT(GHASH_CODE))
          features->have_kimd_ghash = 1;
      }
    }
#endif
  }
}

/* MEMXOR3 */
DECLARE_FAT_FUNC(nettle_memxor3, memxor3_func)
DECLARE_FAT_FUNC_VAR(memxor3, memxor3_func, c)
DECLARE_FAT_FUNC_VAR(memxor3, memxor3_func, s390x)

/* AES128 */
DECLARE_FAT_FUNC(nettle_aes128_set_encrypt_key, aes128_set_key_func)
DECLARE_FAT_FUNC_VAR(aes128_set_encrypt_key, aes128_set_key_func, c)
DECLARE_FAT_FUNC_VAR(aes128_set_encrypt_key, aes128_set_key_func, s390x)
DECLARE_FAT_FUNC(nettle_aes128_set_decrypt_key, aes128_set_key_func)
DECLARE_FAT_FUNC_VAR(aes128_set_decrypt_key, aes128_set_key_func, c)
DECLARE_FAT_FUNC_VAR(aes128_set_decrypt_key, aes128_set_key_func, s390x)
DECLARE_FAT_FUNC(nettle_aes128_invert_key, aes128_invert_key_func)
DECLARE_FAT_FUNC_VAR(aes128_invert_key, aes128_invert_key_func, c)
DECLARE_FAT_FUNC_VAR(aes128_invert_key, aes128_invert_key_func, s390x)
DECLARE_FAT_FUNC(nettle_aes128_encrypt, aes128_crypt_func)
DECLARE_FAT_FUNC_VAR(aes128_encrypt, aes128_crypt_func, c)
DECLARE_FAT_FUNC_VAR(aes128_encrypt, aes128_crypt_func, s390x)
DECLARE_FAT_FUNC(nettle_aes128_decrypt, aes128_crypt_func)
DECLARE_FAT_FUNC_VAR(aes128_decrypt, aes128_crypt_func, c)
DECLARE_FAT_FUNC_VAR(aes128_decrypt, aes128_crypt_func, s390x)

/* AES192 */
DECLARE_FAT_FUNC(nettle_aes192_set_encrypt_key, aes192_set_key_func)
DECLARE_FAT_FUNC_VAR(aes192_set_encrypt_key, aes192_set_key_func, c)
DECLARE_FAT_FUNC_VAR(aes192_set_encrypt_key, aes192_set_key_func, s390x)
DECLARE_FAT_FUNC(nettle_aes192_set_decrypt_key, aes192_set_key_func)
DECLARE_FAT_FUNC_VAR(aes192_set_decrypt_key, aes192_set_key_func, c)
DECLARE_FAT_FUNC_VAR(aes192_set_decrypt_key, aes192_set_key_func, s390x)
DECLARE_FAT_FUNC(nettle_aes192_invert_key, aes192_invert_key_func)
DECLARE_FAT_FUNC_VAR(aes192_invert_key, aes192_invert_key_func, c)
DECLARE_FAT_FUNC_VAR(aes192_invert_key, aes192_invert_key_func, s390x)
DECLARE_FAT_FUNC(nettle_aes192_encrypt, aes192_crypt_func)
DECLARE_FAT_FUNC_VAR(aes192_encrypt, aes192_crypt_func, c)
DECLARE_FAT_FUNC_VAR(aes192_encrypt, aes192_crypt_func, s390x)
DECLARE_FAT_FUNC(nettle_aes192_decrypt, aes192_crypt_func)
DECLARE_FAT_FUNC_VAR(aes192_decrypt, aes192_crypt_func, c)
DECLARE_FAT_FUNC_VAR(aes192_decrypt, aes192_crypt_func, s390x)

/* AES256 */
DECLARE_FAT_FUNC(nettle_aes256_set_encrypt_key, aes256_set_key_func)
DECLARE_FAT_FUNC_VAR(aes256_set_encrypt_key, aes256_set_key_func, c)
DECLARE_FAT_FUNC_VAR(aes256_set_encrypt_key, aes256_set_key_func, s390x)
DECLARE_FAT_FUNC(nettle_aes256_set_decrypt_key, aes256_set_key_func)
DECLARE_FAT_FUNC_VAR(aes256_set_decrypt_key, aes256_set_key_func, c)
DECLARE_FAT_FUNC_VAR(aes256_set_decrypt_key, aes256_set_key_func, s390x)
DECLARE_FAT_FUNC(nettle_aes256_invert_key, aes256_invert_key_func)
DECLARE_FAT_FUNC_VAR(aes256_invert_key, aes256_invert_key_func, c)
DECLARE_FAT_FUNC_VAR(aes256_invert_key, aes256_invert_key_func, s390x)
DECLARE_FAT_FUNC(nettle_aes256_encrypt, aes256_crypt_func)
DECLARE_FAT_FUNC_VAR(aes256_encrypt, aes256_crypt_func, c)
DECLARE_FAT_FUNC_VAR(aes256_encrypt, aes256_crypt_func, s390x)
DECLARE_FAT_FUNC(nettle_aes256_decrypt, aes256_crypt_func)
DECLARE_FAT_FUNC_VAR(aes256_decrypt, aes256_crypt_func, c)
DECLARE_FAT_FUNC_VAR(aes256_decrypt, aes256_crypt_func, s390x)

/* GHASH */
DECLARE_FAT_FUNC(_nettle_ghash_set_key, ghash_set_key_func)
DECLARE_FAT_FUNC_VAR(ghash_set_key, ghash_set_key_func, c)
DECLARE_FAT_FUNC_VAR(ghash_set_key, ghash_set_key_func, s390x)

DECLARE_FAT_FUNC(_nettle_ghash_update, ghash_update_func)
DECLARE_FAT_FUNC_VAR(ghash_update, ghash_update_func, c)
DECLARE_FAT_FUNC_VAR(ghash_update, ghash_update_func, s390x)

DECLARE_FAT_FUNC(nettle_sha1_compress, sha1_compress_func)
DECLARE_FAT_FUNC_VAR(sha1_compress, sha1_compress_func, c)
DECLARE_FAT_FUNC_VAR(sha1_compress, sha1_compress_func, s390x)

DECLARE_FAT_FUNC(_nettle_sha256_compress_n, sha256_compress_n_func)
DECLARE_FAT_FUNC_VAR(sha256_compress_n, sha256_compress_n_func, c)
DECLARE_FAT_FUNC_VAR(sha256_compress_n, sha256_compress_n_func, s390x)

DECLARE_FAT_FUNC(_nettle_sha512_compress, sha512_compress_func)
DECLARE_FAT_FUNC_VAR(sha512_compress, sha512_compress_func, c)
DECLARE_FAT_FUNC_VAR(sha512_compress, sha512_compress_func, s390x)

DECLARE_FAT_FUNC(nettle_sha3_permute, sha3_permute_func)
DECLARE_FAT_FUNC_VAR(sha3_permute, sha3_permute_func, c)
DECLARE_FAT_FUNC_VAR(sha3_permute, sha3_permute_func, s390x)

DECLARE_FAT_FUNC(_nettle_chacha_core, chacha_core_func)
DECLARE_FAT_FUNC_VAR(chacha_core, chacha_core_func, c);
DECLARE_FAT_FUNC_VAR(chacha_core, chacha_core_func, s390x);

DECLARE_FAT_FUNC(nettle_chacha_crypt, chacha_crypt_func)
DECLARE_FAT_FUNC_VAR(chacha_crypt, chacha_crypt_func, 1core)
DECLARE_FAT_FUNC_VAR(chacha_crypt, chacha_crypt_func, 4core)

DECLARE_FAT_FUNC(nettle_chacha_crypt32, chacha_crypt_func)
DECLARE_FAT_FUNC_VAR(chacha_crypt32, chacha_crypt_func, 1core)
DECLARE_FAT_FUNC_VAR(chacha_crypt32, chacha_crypt_func, 4core)

static void CONSTRUCTOR
fat_init (void)
{
  struct s390x_features features;
  int verbose;

  get_s390x_features (&features);
  verbose = getenv (ENV_VERBOSE) != NULL;

  /* MEMXOR3 */
  if (features.have_vector_facility)
  {
    if (verbose)
      fprintf (stderr, "libnettle: enabling vector facility code.\n");
    nettle_memxor3_vec = _nettle_memxor3_s390x;
    nettle_sha3_permute_vec = _nettle_sha3_permute_s390x;
    _nettle_chacha_core_vec = _nettle_chacha_core_s390x;
    nettle_chacha_crypt_vec = _nettle_chacha_crypt_4core;
    nettle_chacha_crypt32_vec = _nettle_chacha_crypt32_4core;
  }
  else
  {
     nettle_memxor3_vec = _nettle_memxor3_c;
     nettle_sha3_permute_vec = _nettle_sha3_permute_c;
     _nettle_chacha_core_vec = _nettle_chacha_core_c;
     nettle_chacha_crypt_vec = _nettle_chacha_crypt_1core;
     nettle_chacha_crypt32_vec = _nettle_chacha_crypt32_1core;
  }

  /* AES128 */
  if (features.have_km_aes128)
  {
    if (verbose)
      fprintf (stderr, "libnettle: enabling hardware accelerated AES128 EBC mode.\n");
    nettle_aes128_set_encrypt_key_vec = _nettle_aes128_set_encrypt_key_s390x;
    nettle_aes128_set_decrypt_key_vec = _nettle_aes128_set_decrypt_key_s390x;
    nettle_aes128_invert_key_vec = _nettle_aes128_invert_key_s390x;
    nettle_aes128_encrypt_vec = _nettle_aes128_encrypt_s390x;
    nettle_aes128_decrypt_vec = _nettle_aes128_decrypt_s390x;
  }
  else
  {
    nettle_aes128_set_encrypt_key_vec = _nettle_aes128_set_encrypt_key_c;
    nettle_aes128_set_decrypt_key_vec = _nettle_aes128_set_decrypt_key_c;
    nettle_aes128_invert_key_vec = _nettle_aes128_invert_key_c;
    nettle_aes128_encrypt_vec = _nettle_aes128_encrypt_c;
    nettle_aes128_decrypt_vec = _nettle_aes128_decrypt_c;
  }

  /* AES192 */
  if (features.have_km_aes192)
  {
    if (verbose)
      fprintf (stderr, "libnettle: enabling hardware accelerated AES192 EBC mode.\n");
    nettle_aes192_set_encrypt_key_vec = _nettle_aes192_set_encrypt_key_s390x;
    nettle_aes192_set_decrypt_key_vec = _nettle_aes192_set_decrypt_key_s390x;
    nettle_aes192_invert_key_vec = _nettle_aes192_invert_key_s390x;
    nettle_aes192_encrypt_vec = _nettle_aes192_encrypt_s390x;
    nettle_aes192_decrypt_vec = _nettle_aes192_decrypt_s390x;
  }
  else
  {
    nettle_aes192_set_encrypt_key_vec = _nettle_aes192_set_encrypt_key_c;
    nettle_aes192_set_decrypt_key_vec = _nettle_aes192_set_decrypt_key_c;
    nettle_aes192_invert_key_vec = _nettle_aes192_invert_key_c;
    nettle_aes192_encrypt_vec = _nettle_aes192_encrypt_c;
    nettle_aes192_decrypt_vec = _nettle_aes192_decrypt_c;
  }

  /* AES256 */
  if (features.have_km_aes256)
  {
    if (verbose)
      fprintf (stderr, "libnettle: enabling hardware accelerated AES256 EBC mode.\n");
    nettle_aes256_set_encrypt_key_vec = _nettle_aes256_set_encrypt_key_s390x;
    nettle_aes256_set_decrypt_key_vec = _nettle_aes256_set_decrypt_key_s390x;
    nettle_aes256_invert_key_vec = _nettle_aes256_invert_key_s390x;
    nettle_aes256_encrypt_vec = _nettle_aes256_encrypt_s390x;
    nettle_aes256_decrypt_vec = _nettle_aes256_decrypt_s390x;
  }
  else
  {
    nettle_aes256_set_encrypt_key_vec = _nettle_aes256_set_encrypt_key_c;
    nettle_aes256_set_decrypt_key_vec = _nettle_aes256_set_decrypt_key_c;
    nettle_aes256_invert_key_vec = _nettle_aes256_invert_key_c;
    nettle_aes256_encrypt_vec = _nettle_aes256_encrypt_c;
    nettle_aes256_decrypt_vec = _nettle_aes256_decrypt_c;
  }

  /* GHASH */
  if (features.have_kimd_ghash)
  {
    if (verbose)
      fprintf (stderr, "libnettle: enabling hardware accelerated GHASH.\n");
    _nettle_ghash_set_key_vec = _nettle_ghash_set_key_s390x;
    _nettle_ghash_update_vec = _nettle_ghash_update_s390x;
  }
  else
  {
    _nettle_ghash_set_key_vec = _nettle_ghash_set_key_c;
    _nettle_ghash_update_vec = _nettle_ghash_update_c;
  }

  /* SHA1 */
  if (features.have_kimd_sha_1)
  {
    if (verbose)
      fprintf (stderr, "libnettle: enabling hardware accelerated SHA1 compress code.\n");
    nettle_sha1_compress_vec = _nettle_sha1_compress_s390x;
  }
  else
  {
    nettle_sha1_compress_vec = _nettle_sha1_compress_c;
  }

  /* SHA256 */
  if (features.have_kimd_sha_256)
  {
    if (verbose)
      fprintf (stderr, "libnettle: enabling hardware accelerated SHA256 compress code.\n");
    _nettle_sha256_compress_n_vec = _nettle_sha256_compress_n_s390x;
  }
  else
  {
    _nettle_sha256_compress_n_vec = _nettle_sha256_compress_n_c;
  }

  /* SHA512 */
  if (features.have_kimd_sha_512)
  {
    if (verbose)
      fprintf (stderr, "libnettle: enabling hardware accelerated SHA512 compress code.\n");
    _nettle_sha512_compress_vec = _nettle_sha512_compress_s390x;
  }
  else
  {
    _nettle_sha512_compress_vec = _nettle_sha512_compress_c;
  }
}

/* MEMXOR3 */
DEFINE_FAT_FUNC(nettle_memxor3, void *,
		(void *dst_in, const void *a_in, const void *b_in, size_t n),
		(dst_in, a_in, b_in, n))

/* AES128 */
DEFINE_FAT_FUNC(nettle_aes128_set_encrypt_key, void,
 (struct aes128_ctx *ctx, const uint8_t *key),
 (ctx, key))
DEFINE_FAT_FUNC(nettle_aes128_set_decrypt_key, void,
 (struct aes128_ctx *ctx, const uint8_t *key),
 (ctx, key))
DEFINE_FAT_FUNC(nettle_aes128_invert_key, void,
 (struct aes128_ctx *dst, const struct aes128_ctx *src),
 (dst, src))
DEFINE_FAT_FUNC(nettle_aes128_encrypt, void,
 (const struct aes128_ctx *ctx, size_t length,
  uint8_t *dst,const uint8_t *src),
 (ctx, length, dst, src))
DEFINE_FAT_FUNC(nettle_aes128_decrypt, void,
 (const struct aes128_ctx *ctx, size_t length,
  uint8_t *dst,const uint8_t *src),
 (ctx, length, dst, src))

/* AES192 */
DEFINE_FAT_FUNC(nettle_aes192_set_encrypt_key, void,
 (struct aes192_ctx *ctx, const uint8_t *key),
 (ctx, key))
DEFINE_FAT_FUNC(nettle_aes192_set_decrypt_key, void,
 (struct aes192_ctx *ctx, const uint8_t *key),
 (ctx, key))
DEFINE_FAT_FUNC(nettle_aes192_invert_key, void,
 (struct aes192_ctx *dst, const struct aes192_ctx *src),
 (dst, src))
DEFINE_FAT_FUNC(nettle_aes192_encrypt, void,
 (const struct aes192_ctx *ctx, size_t length,
  uint8_t *dst,const uint8_t *src),
 (ctx, length, dst, src))
DEFINE_FAT_FUNC(nettle_aes192_decrypt, void,
 (const struct aes192_ctx *ctx, size_t length,
  uint8_t *dst,const uint8_t *src),
 (ctx, length, dst, src))

/* AES256 */
DEFINE_FAT_FUNC(nettle_aes256_set_encrypt_key, void,
 (struct aes256_ctx *ctx, const uint8_t *key),
 (ctx, key))
DEFINE_FAT_FUNC(nettle_aes256_set_decrypt_key, void,
 (struct aes256_ctx *ctx, const uint8_t *key),
 (ctx, key))
DEFINE_FAT_FUNC(nettle_aes256_invert_key, void,
 (struct aes256_ctx *dst, const struct aes256_ctx *src),
 (dst, src))
DEFINE_FAT_FUNC(nettle_aes256_encrypt, void,
 (const struct aes256_ctx *ctx, size_t length,
  uint8_t *dst,const uint8_t *src),
 (ctx, length, dst, src))
DEFINE_FAT_FUNC(nettle_aes256_decrypt, void,
 (const struct aes256_ctx *ctx, size_t length,
  uint8_t *dst,const uint8_t *src),
 (ctx, length, dst, src))

/* GHASH */
DEFINE_FAT_FUNC(_nettle_ghash_set_key, void,
		(struct gcm_key *ctx, const union nettle_block16 *key),
		(ctx, key))
DEFINE_FAT_FUNC(_nettle_ghash_update, const uint8_t *,
		(const struct gcm_key *ctx, union nettle_block16 *state,
		 size_t blocks, const uint8_t *data),
		(ctx, state, blocks, data))

/* SHA1 */
DEFINE_FAT_FUNC(nettle_sha1_compress, void,
		(uint32_t *state, const uint8_t *input),
		(state, input))

/* SHA256 */
DEFINE_FAT_FUNC(_nettle_sha256_compress_n, const uint8_t *,
		(uint32_t *state, const uint32_t *k,
		 size_t blocks, const uint8_t *input),
		(state, k, blocks, input))

/* SHA512 */
DEFINE_FAT_FUNC(_nettle_sha512_compress, void,
		(uint64_t *state, const uint8_t *input, const uint64_t *k),
		(state, input, k))

/* SHA3 */
DEFINE_FAT_FUNC(nettle_sha3_permute, void,
		(struct sha3_state *state), (state))

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
