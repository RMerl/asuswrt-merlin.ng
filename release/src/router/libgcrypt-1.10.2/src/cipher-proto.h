/* cipher-proto.h - Internal declarations
 *	Copyright (C) 2008, 2011 Free Software Foundation, Inc.
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

/* This file has been factored out from cipher.h so that it can be
   used standalone in visibility.c . */

#ifndef G10_CIPHER_PROTO_H
#define G10_CIPHER_PROTO_H


enum pk_encoding;


/* Definition of a function used to report selftest failures.
   DOMAIN is a string describing the function block:
          "cipher", "digest", "pubkey or "random",
   ALGO   is the algorithm under test,
   WHAT   is a string describing what has been tested,
   DESC   is a string describing the error. */
typedef void (*selftest_report_func_t)(const char *domain,
                                       int algo,
                                       const char *what,
                                       const char *errdesc);

/* Definition of the selftest functions.  */
typedef gpg_err_code_t (*selftest_func_t)
     (int algo, int extended, selftest_report_func_t report);


/*
 *
 * Public key related definitions.
 *
 */

/* Type for the pk_generate function.  */
typedef gcry_err_code_t (*gcry_pk_generate_t) (gcry_sexp_t genparms,
                                               gcry_sexp_t *r_skey);

/* Type for the pk_check_secret_key function.  */
typedef gcry_err_code_t (*gcry_pk_check_secret_key_t) (gcry_sexp_t keyparms);

/* Type for the pk_encrypt function.  */
typedef gcry_err_code_t (*gcry_pk_encrypt_t) (gcry_sexp_t *r_ciph,
                                              gcry_sexp_t s_data,
                                              gcry_sexp_t keyparms);

/* Type for the pk_decrypt function.  */
typedef gcry_err_code_t (*gcry_pk_decrypt_t) (gcry_sexp_t *r_plain,
                                              gcry_sexp_t s_data,
                                              gcry_sexp_t keyparms);

/* Type for the pk_sign function.  */
typedef gcry_err_code_t (*gcry_pk_sign_t) (gcry_sexp_t *r_sig,
                                           gcry_sexp_t s_data,
                                           gcry_sexp_t keyparms);

/* Type for the pk_verify function.  */
typedef gcry_err_code_t (*gcry_pk_verify_t) (gcry_sexp_t s_sig,
                                             gcry_sexp_t s_data,
                                             gcry_sexp_t keyparms);

/* Type for the pk_get_nbits function.  */
typedef unsigned (*gcry_pk_get_nbits_t) (gcry_sexp_t keyparms);


/* The type used to compute the keygrip.  */
typedef gpg_err_code_t (*pk_comp_keygrip_t) (gcry_md_hd_t md,
                                             gcry_sexp_t keyparm);

/* The type used to query an ECC curve name.  */
typedef const char *(*pk_get_curve_t)(gcry_sexp_t keyparms, int iterator,
                                      unsigned int *r_nbits);

/* The type used to query ECC curve parameters by name.  */
typedef gcry_sexp_t (*pk_get_curve_param_t)(const char *name);


/* Module specification structure for public key algorithms.  */
typedef struct gcry_pk_spec
{
  int algo;
  struct {
    unsigned int disabled:1;
    unsigned int fips:1;
  } flags;
  int use;
  const char *name;
  const char **aliases;
  const char *elements_pkey;
  const char *elements_skey;
  const char *elements_enc;
  const char *elements_sig;
  const char *elements_grip;
  gcry_pk_generate_t generate;
  gcry_pk_check_secret_key_t check_secret_key;
  gcry_pk_encrypt_t encrypt;
  gcry_pk_decrypt_t decrypt;
  gcry_pk_sign_t sign;
  gcry_pk_verify_t verify;
  gcry_pk_get_nbits_t get_nbits;
  selftest_func_t selftest;
  pk_comp_keygrip_t comp_keygrip;
  pk_get_curve_t get_curve;
  pk_get_curve_param_t get_curve_param;
} gcry_pk_spec_t;



/*
 *
 * Symmetric cipher related definitions.
 *
 */

struct cipher_bulk_ops;

/* Type for the cipher_setkey function.  */
typedef gcry_err_code_t (*gcry_cipher_setkey_t) (void *c,
						 const unsigned char *key,
						 unsigned keylen,
						 struct cipher_bulk_ops *bulk_ops);

/* Type for the cipher_encrypt function.  */
typedef unsigned int (*gcry_cipher_encrypt_t) (void *c,
					       unsigned char *outbuf,
					       const unsigned char *inbuf);

/* Type for the cipher_decrypt function.  */
typedef unsigned int (*gcry_cipher_decrypt_t) (void *c,
					       unsigned char *outbuf,
					       const unsigned char *inbuf);

/* Type for the cipher_stencrypt function.  */
typedef void (*gcry_cipher_stencrypt_t) (void *c,
					 unsigned char *outbuf,
					 const unsigned char *inbuf,
					 size_t n);

/* Type for the cipher_stdecrypt function.  */
typedef void (*gcry_cipher_stdecrypt_t) (void *c,
					 unsigned char *outbuf,
					 const unsigned char *inbuf,
					 size_t n);

/* The type used to convey additional information to a cipher.  */
typedef gpg_err_code_t (*cipher_set_extra_info_t)
     (void *c, int what, const void *buffer, size_t buflen);

/* The type used to set an IV directly in the algorithm module.  */
typedef void (*cipher_setiv_func_t)(void *c, const byte *iv, size_t ivlen);

/* A structure to map OIDs to encryption modes.  */
typedef struct gcry_cipher_oid_spec
{
  const char *oid;
  int mode;
} gcry_cipher_oid_spec_t;


/* Module specification structure for ciphers.  */
typedef struct gcry_cipher_spec
{
  int algo;
  struct {
    unsigned int disabled:1;
    unsigned int fips:1;
  } flags;
  const char *name;
  const char **aliases;
  const gcry_cipher_oid_spec_t *oids;
  size_t blocksize;
  size_t keylen;
  size_t contextsize;
  gcry_cipher_setkey_t setkey;
  gcry_cipher_encrypt_t encrypt;
  gcry_cipher_decrypt_t decrypt;
  gcry_cipher_stencrypt_t stencrypt;
  gcry_cipher_stdecrypt_t stdecrypt;
  selftest_func_t selftest;
  cipher_set_extra_info_t set_extra_info;
  cipher_setiv_func_t setiv;
} gcry_cipher_spec_t;



/*
 *
 * Message digest related definitions.
 *
 */

/* Type for the md_init function.  */
typedef void (*gcry_md_init_t) (void *c, unsigned int flags);

/* Type for the md_write function.  */
typedef void (*gcry_md_write_t) (void *c, const void *buf, size_t nbytes);

/* Type for the md_final function.  */
typedef void (*gcry_md_final_t) (void *c);

/* Type for the md_read function.  */
typedef unsigned char *(*gcry_md_read_t) (void *c);

/* Type for the md_extract function.  */
typedef void (*gcry_md_extract_t) (void *c, void *outbuf, size_t nbytes);

/* Type for the md_hash_buffers function. */
typedef void (*gcry_md_hash_buffers_t) (void *outbuf, size_t nbytes,
					const gcry_buffer_t *iov,
					int iovcnt);

typedef struct gcry_md_oid_spec
{
  const char *oidstring;
} gcry_md_oid_spec_t;

/* Module specification structure for message digests.  */
typedef struct gcry_md_spec
{
  int algo;
  struct {
    unsigned int disabled:1;
    unsigned int fips:1;
  } flags;
  const char *name;
  const unsigned char *asnoid;
  int asnlen;
  const gcry_md_oid_spec_t *oids;
  int mdlen;
  gcry_md_init_t init;
  gcry_md_write_t write;
  gcry_md_final_t final;
  gcry_md_read_t read;
  gcry_md_extract_t extract;
  gcry_md_hash_buffers_t hash_buffers;
  size_t contextsize; /* allocate this amount of context */
  selftest_func_t selftest;
} gcry_md_spec_t;



/* The selftest functions.  */
gcry_error_t _gcry_cipher_selftest (int algo, int extended,
                                    selftest_report_func_t report);
gcry_error_t _gcry_md_selftest (int algo, int extended,
                                selftest_report_func_t report);
gcry_error_t _gcry_pk_selftest (int algo, int extended,
                                selftest_report_func_t report);
gcry_error_t _gcry_mac_selftest (int algo, int extended,
                                 selftest_report_func_t report);
gcry_error_t _gcry_kdf_selftest (int algo, int extended,
                                 selftest_report_func_t report);

gcry_error_t _gcry_random_selftest (selftest_report_func_t report);




#endif /*G10_CIPHER_PROTO_H*/
