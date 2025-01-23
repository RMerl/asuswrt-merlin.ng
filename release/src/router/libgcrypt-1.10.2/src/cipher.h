/* cipher.h
 *	Copyright (C) 1998, 2002, 2003, 2009 Free Software Foundation, Inc.
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
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#ifndef G10_CIPHER_H
#define G10_CIPHER_H

#include "gcrypt-int.h"

#define DBG_CIPHER _gcry_get_debug_flag( 1 )

#include "../random/random.h"

#define PUBKEY_FLAG_NO_BLINDING    (1 << 0)
#define PUBKEY_FLAG_RFC6979        (1 << 1)
#define PUBKEY_FLAG_FIXEDLEN       (1 << 2)
#define PUBKEY_FLAG_LEGACYRESULT   (1 << 3)
#define PUBKEY_FLAG_RAW_FLAG       (1 << 4)
#define PUBKEY_FLAG_TRANSIENT_KEY  (1 << 5)
#define PUBKEY_FLAG_USE_X931       (1 << 6)
#define PUBKEY_FLAG_USE_FIPS186    (1 << 7)
#define PUBKEY_FLAG_USE_FIPS186_2  (1 << 8)
#define PUBKEY_FLAG_PARAM          (1 << 9)
#define PUBKEY_FLAG_COMP           (1 << 10)
#define PUBKEY_FLAG_NOCOMP         (1 << 11)
#define PUBKEY_FLAG_EDDSA          (1 << 12)
#define PUBKEY_FLAG_GOST           (1 << 13)
#define PUBKEY_FLAG_NO_KEYTEST     (1 << 14)
#define PUBKEY_FLAG_DJB_TWEAK      (1 << 15)
#define PUBKEY_FLAG_SM2            (1 << 16)
#define PUBKEY_FLAG_PREHASH        (1 << 17)


enum pk_operation
  {
    PUBKEY_OP_ENCRYPT,
    PUBKEY_OP_DECRYPT,
    PUBKEY_OP_SIGN,
    PUBKEY_OP_VERIFY
  };

enum pk_encoding
  {
    PUBKEY_ENC_RAW,
    PUBKEY_ENC_PKCS1,
    PUBKEY_ENC_PKCS1_RAW,
    PUBKEY_ENC_OAEP,
    PUBKEY_ENC_PSS,
    PUBKEY_ENC_UNKNOWN
  };

struct pk_encoding_ctx
{
  enum pk_operation op;
  unsigned int nbits;

  enum pk_encoding encoding;
  int flags;

  int hash_algo;

  /* for OAEP */
  unsigned char *label;
  size_t labellen;

  /* for PSS */
  size_t saltlen;

  int (* verify_cmp) (void *opaque, gcry_mpi_t tmp);
  void *verify_arg;
};

#define CIPHER_INFO_NO_WEAK_KEY    1

#include "cipher-proto.h"

/* The internal encryption modes. */
enum gcry_cipher_internal_modes
  {
    GCRY_CIPHER_MODE_INTERNAL = 0x10000,
    GCRY_CIPHER_MODE_CMAC     = 0x10000 + 1   /* Cipher-based MAC. */
  };


/*-- cipher.c --*/
gcry_err_code_t _gcry_cipher_open_internal (gcry_cipher_hd_t *handle,
					    int algo, int mode,
					    unsigned int flags);

/*-- cipher-cmac.c --*/
gcry_err_code_t _gcry_cipher_cmac_authenticate
/*           */ (gcry_cipher_hd_t c, const unsigned char *abuf, size_t abuflen);
gcry_err_code_t _gcry_cipher_cmac_get_tag
/*           */ (gcry_cipher_hd_t c,
                 unsigned char *outtag, size_t taglen);
gcry_err_code_t _gcry_cipher_cmac_check_tag
/*           */ (gcry_cipher_hd_t c,
                 const unsigned char *intag, size_t taglen);
gcry_err_code_t _gcry_cipher_cmac_set_subkeys
/*           */ (gcry_cipher_hd_t c);

/*-- sha1.c --*/
void _gcry_sha1_hash_buffer (void *outbuf,
                             const void *buffer, size_t length);

/*-- blake2.c --*/
gcry_err_code_t _gcry_blake2_init_with_key(void *ctx, unsigned int flags,
					   const unsigned char *key,
					   size_t keylen, int algo);

/*-- dsa.c --*/
void _gcry_register_pk_dsa_progress (gcry_handler_progress_t cbc, void *cb_data);

/*-- elgamal.c --*/
void _gcry_register_pk_elg_progress (gcry_handler_progress_t cb,
                                     void *cb_data);


/*-- ecc.c --*/
void _gcry_register_pk_ecc_progress (gcry_handler_progress_t cbc,
                                     void *cb_data);


/*-- primegen.c --*/
void _gcry_register_primegen_progress (gcry_handler_progress_t cb,
                                       void *cb_data);

/*-- pubkey.c --*/

/* Declarations for the cipher specifications.  */
extern gcry_cipher_spec_t _gcry_cipher_spec_blowfish;
extern gcry_cipher_spec_t _gcry_cipher_spec_des;
extern gcry_cipher_spec_t _gcry_cipher_spec_tripledes;
extern gcry_cipher_spec_t _gcry_cipher_spec_arcfour;
extern gcry_cipher_spec_t _gcry_cipher_spec_cast5;
extern gcry_cipher_spec_t _gcry_cipher_spec_aes;
extern gcry_cipher_spec_t _gcry_cipher_spec_aes192;
extern gcry_cipher_spec_t _gcry_cipher_spec_aes256;
extern gcry_cipher_spec_t _gcry_cipher_spec_twofish;
extern gcry_cipher_spec_t _gcry_cipher_spec_twofish128;
extern gcry_cipher_spec_t _gcry_cipher_spec_serpent128;
extern gcry_cipher_spec_t _gcry_cipher_spec_serpent192;
extern gcry_cipher_spec_t _gcry_cipher_spec_serpent256;
extern gcry_cipher_spec_t _gcry_cipher_spec_rfc2268_40;
extern gcry_cipher_spec_t _gcry_cipher_spec_rfc2268_128;
extern gcry_cipher_spec_t _gcry_cipher_spec_seed;
extern gcry_cipher_spec_t _gcry_cipher_spec_camellia128;
extern gcry_cipher_spec_t _gcry_cipher_spec_camellia192;
extern gcry_cipher_spec_t _gcry_cipher_spec_camellia256;
extern gcry_cipher_spec_t _gcry_cipher_spec_idea;
extern gcry_cipher_spec_t _gcry_cipher_spec_salsa20;
extern gcry_cipher_spec_t _gcry_cipher_spec_salsa20r12;
extern gcry_cipher_spec_t _gcry_cipher_spec_gost28147;
extern gcry_cipher_spec_t _gcry_cipher_spec_gost28147_mesh;
extern gcry_cipher_spec_t _gcry_cipher_spec_chacha20;
extern gcry_cipher_spec_t _gcry_cipher_spec_sm4;

/* Declarations for the digest specifications.  */
extern const gcry_md_spec_t _gcry_digest_spec_crc32;
extern const gcry_md_spec_t _gcry_digest_spec_crc32_rfc1510;
extern const gcry_md_spec_t _gcry_digest_spec_crc24_rfc2440;
extern const gcry_md_spec_t _gcry_digest_spec_gost3411_94;
extern const gcry_md_spec_t _gcry_digest_spec_gost3411_cp;
extern const gcry_md_spec_t _gcry_digest_spec_stribog_256;
extern const gcry_md_spec_t _gcry_digest_spec_stribog_512;
extern const gcry_md_spec_t _gcry_digest_spec_md2;
extern const gcry_md_spec_t _gcry_digest_spec_md4;
extern const gcry_md_spec_t _gcry_digest_spec_md5;
extern const gcry_md_spec_t _gcry_digest_spec_rmd160;
extern const gcry_md_spec_t _gcry_digest_spec_sha1;
extern const gcry_md_spec_t _gcry_digest_spec_sha224;
extern const gcry_md_spec_t _gcry_digest_spec_sha256;
extern const gcry_md_spec_t _gcry_digest_spec_sha384;
extern const gcry_md_spec_t _gcry_digest_spec_sha512;
extern const gcry_md_spec_t _gcry_digest_spec_sha512_224;
extern const gcry_md_spec_t _gcry_digest_spec_sha512_256;
extern const gcry_md_spec_t _gcry_digest_spec_sha3_224;
extern const gcry_md_spec_t _gcry_digest_spec_sha3_256;
extern const gcry_md_spec_t _gcry_digest_spec_sha3_512;
extern const gcry_md_spec_t _gcry_digest_spec_sha3_384;
extern const gcry_md_spec_t _gcry_digest_spec_shake128;
extern const gcry_md_spec_t _gcry_digest_spec_shake256;
extern const gcry_md_spec_t _gcry_digest_spec_tiger;
extern const gcry_md_spec_t _gcry_digest_spec_tiger1;
extern const gcry_md_spec_t _gcry_digest_spec_tiger2;
extern const gcry_md_spec_t _gcry_digest_spec_whirlpool;
extern const gcry_md_spec_t _gcry_digest_spec_blake2b_512;
extern const gcry_md_spec_t _gcry_digest_spec_blake2b_384;
extern const gcry_md_spec_t _gcry_digest_spec_blake2b_256;
extern const gcry_md_spec_t _gcry_digest_spec_blake2b_160;
extern const gcry_md_spec_t _gcry_digest_spec_blake2s_256;
extern const gcry_md_spec_t _gcry_digest_spec_blake2s_224;
extern const gcry_md_spec_t _gcry_digest_spec_blake2s_160;
extern const gcry_md_spec_t _gcry_digest_spec_blake2s_128;
extern const gcry_md_spec_t _gcry_digest_spec_sm3;

/* Declarations for the pubkey cipher specifications.  */
extern gcry_pk_spec_t _gcry_pubkey_spec_rsa;
extern gcry_pk_spec_t _gcry_pubkey_spec_elg;
extern gcry_pk_spec_t _gcry_pubkey_spec_elg_e;
extern gcry_pk_spec_t _gcry_pubkey_spec_dsa;
extern gcry_pk_spec_t _gcry_pubkey_spec_ecc;


#endif /*G10_CIPHER_H*/
