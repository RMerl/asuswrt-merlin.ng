/* mac-internal.h  -  Internal defs for mac.c
 * Copyright (C) 2013 Jussi Kivilinna <jussi.kivilinna@iki.fi>
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser general Public License as
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

#include <config.h>

#include "g10lib.h"
#include "cipher-proto.h"
#include "gost.h"


/* The data object used to hold a handle to an encryption object.  */
struct gcry_mac_handle;

/* The data object used to hold poly1305-mac context.  */
struct poly1305mac_context_s;


/*
 *
 * Message authentication code related definitions.
 *
 */


/* Magic values for the context structure.  */
#define CTX_MAC_MAGIC_NORMAL 0x59d9b8af
#define CTX_MAC_MAGIC_SECURE 0x12c27cd0


/* MAC module functions. */
typedef gcry_err_code_t (*gcry_mac_open_func_t)(gcry_mac_hd_t h);
typedef void (*gcry_mac_close_func_t)(gcry_mac_hd_t h);
typedef gcry_err_code_t (*gcry_mac_setkey_func_t)(gcry_mac_hd_t h,
						  const unsigned char *key,
						  size_t keylen);
typedef gcry_err_code_t (*gcry_mac_setiv_func_t)(gcry_mac_hd_t h,
						 const unsigned char *iv,
						 size_t ivlen);
typedef gcry_err_code_t (*gcry_mac_reset_func_t)(gcry_mac_hd_t h);
typedef gcry_err_code_t (*gcry_mac_write_func_t)(gcry_mac_hd_t h,
						 const unsigned char *inbuf,
						 size_t inlen);
typedef gcry_err_code_t (*gcry_mac_read_func_t)(gcry_mac_hd_t h,
						unsigned char *outbuf,
						size_t *outlen);
typedef gcry_err_code_t (*gcry_mac_verify_func_t)(gcry_mac_hd_t h,
						  const unsigned char *inbuf,
						  size_t inlen);
typedef unsigned int (*gcry_mac_get_maclen_func_t)(int algo);
typedef unsigned int (*gcry_mac_get_keylen_func_t)(int algo);

/* The type used to convey additional information to a MAC.  */
typedef gpg_err_code_t (*gcry_mac_set_extra_info_t)
     (gcry_mac_hd_t h, int what, const void *buffer, size_t buflen);

typedef struct gcry_mac_spec_ops
{
  gcry_mac_open_func_t open;
  gcry_mac_close_func_t close;
  gcry_mac_setkey_func_t setkey;
  gcry_mac_setiv_func_t setiv;
  gcry_mac_reset_func_t reset;
  gcry_mac_write_func_t write;
  gcry_mac_read_func_t read;
  gcry_mac_verify_func_t verify;
  gcry_mac_get_maclen_func_t get_maclen;
  gcry_mac_get_keylen_func_t get_keylen;
  gcry_mac_set_extra_info_t set_extra_info;
  selftest_func_t selftest;
} gcry_mac_spec_ops_t;


/* Module specification structure for message authentication codes.  */
typedef struct gcry_mac_spec
{
  int algo;
  struct {
    unsigned int disabled:1;
    unsigned int fips:1;
  } flags;
  const char *name;
  const gcry_mac_spec_ops_t *ops;
} gcry_mac_spec_t;

/* The handle structure.  */
struct gcry_mac_handle
{
  int magic;
  int algo;
  const gcry_mac_spec_t *spec;
  gcry_ctx_t gcry_ctx;
  union {
    struct {
      gcry_md_hd_t md_ctx;
      int md_algo;
    } hmac;
    struct {
      gcry_cipher_hd_t ctx;
      int cipher_algo;
      unsigned int blklen;
    } cmac;
    struct {
      gcry_cipher_hd_t ctx;
      int cipher_algo;
    } gmac;
    struct {
      struct poly1305mac_context_s *ctx;
    } poly1305mac;
    struct {
      GOST28147_context ctx;
      u32 n1, n2;
      unsigned int unused;
      unsigned int count;
      unsigned char lastiv[8]; /* IMIT blocksize */
    } imit;
  } u;
};


/*
 * The HMAC algorithm specifications (mac-hmac.c).
 */
#if USE_SHA1
extern const gcry_mac_spec_t _gcry_mac_type_spec_hmac_sha1;
#endif
#if USE_SHA256
extern const gcry_mac_spec_t _gcry_mac_type_spec_hmac_sha256;
extern const gcry_mac_spec_t _gcry_mac_type_spec_hmac_sha224;
#endif
#if USE_SHA512
extern const gcry_mac_spec_t _gcry_mac_type_spec_hmac_sha512;
extern const gcry_mac_spec_t _gcry_mac_type_spec_hmac_sha384;
extern const gcry_mac_spec_t _gcry_mac_type_spec_hmac_sha512_224;
extern const gcry_mac_spec_t _gcry_mac_type_spec_hmac_sha512_256;
#endif
#if USE_SHA3
extern const gcry_mac_spec_t _gcry_mac_type_spec_hmac_sha3_224;
extern const gcry_mac_spec_t _gcry_mac_type_spec_hmac_sha3_256;
extern const gcry_mac_spec_t _gcry_mac_type_spec_hmac_sha3_384;
extern const gcry_mac_spec_t _gcry_mac_type_spec_hmac_sha3_512;
#endif
#if USE_GOST_R_3411_94
extern const gcry_mac_spec_t _gcry_mac_type_spec_hmac_gost3411_94;
extern const gcry_mac_spec_t _gcry_mac_type_spec_hmac_gost3411_cp;
#endif
#if USE_GOST_R_3411_12
extern const gcry_mac_spec_t _gcry_mac_type_spec_hmac_stribog256;
extern const gcry_mac_spec_t _gcry_mac_type_spec_hmac_stribog512;
#endif
#if USE_WHIRLPOOL
extern const gcry_mac_spec_t _gcry_mac_type_spec_hmac_whirlpool;
#endif
#if USE_RMD160
extern const gcry_mac_spec_t _gcry_mac_type_spec_hmac_rmd160;
#endif
#if USE_TIGER
extern const gcry_mac_spec_t _gcry_mac_type_spec_hmac_tiger1;
#endif
#if USE_MD5
extern const gcry_mac_spec_t _gcry_mac_type_spec_hmac_md5;
#endif
#if USE_MD4
extern const gcry_mac_spec_t _gcry_mac_type_spec_hmac_md4;
#endif
#if USE_BLAKE2
extern const gcry_mac_spec_t _gcry_mac_type_spec_hmac_blake2b_512;
extern const gcry_mac_spec_t _gcry_mac_type_spec_hmac_blake2b_384;
extern const gcry_mac_spec_t _gcry_mac_type_spec_hmac_blake2b_256;
extern const gcry_mac_spec_t _gcry_mac_type_spec_hmac_blake2b_160;
extern const gcry_mac_spec_t _gcry_mac_type_spec_hmac_blake2s_256;
extern const gcry_mac_spec_t _gcry_mac_type_spec_hmac_blake2s_224;
extern const gcry_mac_spec_t _gcry_mac_type_spec_hmac_blake2s_160;
extern const gcry_mac_spec_t _gcry_mac_type_spec_hmac_blake2s_128;
#endif
#if USE_SM3
extern const gcry_mac_spec_t _gcry_mac_type_spec_hmac_sm3;
#endif

/*
 * The CMAC algorithm specifications (mac-cmac.c).
 */
#if USE_BLOWFISH
extern const gcry_mac_spec_t _gcry_mac_type_spec_cmac_blowfish;
#endif
#if USE_DES
extern const gcry_mac_spec_t _gcry_mac_type_spec_cmac_tripledes;
#endif
#if USE_CAST5
extern const gcry_mac_spec_t _gcry_mac_type_spec_cmac_cast5;
#endif
#if USE_AES
extern const gcry_mac_spec_t _gcry_mac_type_spec_cmac_aes;
#endif
#if USE_TWOFISH
extern const gcry_mac_spec_t _gcry_mac_type_spec_cmac_twofish;
#endif
#if USE_SERPENT
extern const gcry_mac_spec_t _gcry_mac_type_spec_cmac_serpent;
#endif
#if USE_RFC2268
extern const gcry_mac_spec_t _gcry_mac_type_spec_cmac_rfc2268;
#endif
#if USE_SEED
extern const gcry_mac_spec_t _gcry_mac_type_spec_cmac_seed;
#endif
#if USE_CAMELLIA
extern const gcry_mac_spec_t _gcry_mac_type_spec_cmac_camellia;
#endif
#if USE_IDEA
extern const gcry_mac_spec_t _gcry_mac_type_spec_cmac_idea;
#endif
#if USE_GOST28147
extern const gcry_mac_spec_t _gcry_mac_type_spec_cmac_gost28147;
#endif
#if USE_GOST28147
extern const gcry_mac_spec_t _gcry_mac_type_spec_gost28147_imit;
#endif
#if USE_SM4
extern const gcry_mac_spec_t _gcry_mac_type_spec_cmac_sm4;
#endif

/*
 * The GMAC algorithm specifications (mac-gmac.c).
 */
#if USE_AES
extern const gcry_mac_spec_t _gcry_mac_type_spec_gmac_aes;
#endif
#if USE_TWOFISH
extern const gcry_mac_spec_t _gcry_mac_type_spec_gmac_twofish;
#endif
#if USE_SERPENT
extern const gcry_mac_spec_t _gcry_mac_type_spec_gmac_serpent;
#endif
#if USE_SEED
extern const gcry_mac_spec_t _gcry_mac_type_spec_gmac_seed;
#endif
#if USE_CAMELLIA
extern const gcry_mac_spec_t _gcry_mac_type_spec_gmac_camellia;
#endif

/*
 * The Poly1305 MAC algorithm specifications (mac-poly1305.c).
 */
extern const gcry_mac_spec_t _gcry_mac_type_spec_poly1305mac;
#if USE_AES
extern const gcry_mac_spec_t _gcry_mac_type_spec_poly1305mac_aes;
#endif
#if USE_CAMELLIA
extern const gcry_mac_spec_t _gcry_mac_type_spec_poly1305mac_camellia;
#endif
#if USE_TWOFISH
extern const gcry_mac_spec_t _gcry_mac_type_spec_poly1305mac_twofish;
#endif
#if USE_SERPENT
extern const gcry_mac_spec_t _gcry_mac_type_spec_poly1305mac_serpent;
#endif
#if USE_SEED
extern const gcry_mac_spec_t _gcry_mac_type_spec_poly1305mac_seed;
#endif
