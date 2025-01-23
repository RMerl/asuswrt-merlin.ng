/* gcrypt-int.h - Internal version of gcrypt.h
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

#ifndef GCRY_GCRYPT_INT_H
#define GCRY_GCRYPT_INT_H

#ifdef _GCRYPT_H
#error  gcrypt.h already included
#endif

#include "gcrypt.h"
#include "types.h"

/* These error codes are used but not defined in the required
 * libgpg-error N.MM.  Define them here.  [None right now.] */



/* Context used with elliptic curve functions.  */
struct mpi_ec_ctx_s;
typedef struct mpi_ec_ctx_s *mpi_ec_t;



/* Underscore prefixed internal versions of the public functions.
   They return gpg_err_code_t and not gpg_error_t.  Some macros also
   need an underscore prefixed internal version.

   Note that the memory allocation functions and macros (xmalloc etc.)
   are not defined here but in g10lib.h because this file here is
   included by some test programs which define theie own xmalloc
   macros.  */

gpg_err_code_t _gcry_cipher_open (gcry_cipher_hd_t *handle,
                                  int algo, int mode, unsigned int flags);
void _gcry_cipher_close (gcry_cipher_hd_t h);
gpg_err_code_t _gcry_cipher_ctl (gcry_cipher_hd_t h, int cmd, void *buffer,
                             size_t buflen);
gpg_err_code_t _gcry_cipher_info (gcry_cipher_hd_t h, int what, void *buffer,
                                  size_t *nbytes);
gpg_err_code_t _gcry_cipher_algo_info (int algo, int what, void *buffer,
                                       size_t *nbytes);
const char *_gcry_cipher_algo_name (int algorithm) _GCRY_GCC_ATTR_PURE;
int _gcry_cipher_map_name (const char *name) _GCRY_GCC_ATTR_PURE;
int _gcry_cipher_mode_from_oid (const char *string) _GCRY_GCC_ATTR_PURE;
gpg_err_code_t _gcry_cipher_encrypt (gcry_cipher_hd_t h,
                                     void *out, size_t outsize,
                                     const void *in, size_t inlen);
gpg_err_code_t _gcry_cipher_decrypt (gcry_cipher_hd_t h,
                                     void *out, size_t outsize,
                                     const void *in, size_t inlen);
gcry_err_code_t _gcry_cipher_setkey (gcry_cipher_hd_t hd,
                                     const void *key, size_t keylen);
gcry_err_code_t _gcry_cipher_setiv (gcry_cipher_hd_t hd,
                                    const void *iv, size_t ivlen);
gpg_err_code_t _gcry_cipher_authenticate (gcry_cipher_hd_t hd, const void *abuf,
                                          size_t abuflen);
gpg_err_code_t _gcry_cipher_gettag (gcry_cipher_hd_t hd, void *outtag,
                                    size_t taglen);
gpg_err_code_t _gcry_cipher_checktag (gcry_cipher_hd_t hd, const void *intag,
                                      size_t taglen);
gpg_err_code_t _gcry_cipher_setctr (gcry_cipher_hd_t hd,
                                    const void *ctr, size_t ctrlen);
gpg_err_code_t _gcry_cipher_getctr (gcry_cipher_hd_t hd,
                                    void *ctr, size_t ctrlen);
size_t _gcry_cipher_get_algo_keylen (int algo);
size_t _gcry_cipher_get_algo_blklen (int algo);

#define _gcry_cipher_reset(h)  _gcry_cipher_ctl ((h), GCRYCTL_RESET, NULL, 0)




gpg_err_code_t _gcry_pk_encrypt (gcry_sexp_t *result,
                                 gcry_sexp_t data, gcry_sexp_t pkey);
gpg_err_code_t _gcry_pk_decrypt (gcry_sexp_t *result,
                                 gcry_sexp_t data, gcry_sexp_t skey);
gpg_err_code_t _gcry_pk_sign (gcry_sexp_t *result,
                              gcry_sexp_t data, gcry_sexp_t skey);
gpg_err_code_t _gcry_pk_verify (gcry_sexp_t sigval,
                                gcry_sexp_t data, gcry_sexp_t pkey);
gpg_err_code_t _gcry_pk_testkey (gcry_sexp_t key);
gpg_err_code_t _gcry_pk_genkey (gcry_sexp_t *r_key, gcry_sexp_t s_parms);
gpg_err_code_t _gcry_pk_ctl (int cmd, void *buffer, size_t buflen);
gpg_err_code_t _gcry_pk_algo_info (int algo, int what,
                                   void *buffer, size_t *nbytes);
const char *_gcry_pk_algo_name (int algorithm) _GCRY_GCC_ATTR_PURE;
int _gcry_pk_map_name (const char* name) _GCRY_GCC_ATTR_PURE;
unsigned int _gcry_pk_get_nbits (gcry_sexp_t key) _GCRY_GCC_ATTR_PURE;
unsigned char *_gcry_pk_get_keygrip (gcry_sexp_t key, unsigned char *array);
const char *_gcry_pk_get_curve (gcry_sexp_t key, int iterator,
                                unsigned int *r_nbits);
gcry_sexp_t _gcry_pk_get_param (int algo, const char *name);
gpg_err_code_t _gcry_pubkey_get_sexp (gcry_sexp_t *r_sexp,
                                      int mode, gcry_ctx_t ctx);
unsigned int _gcry_ecc_get_algo_keylen (int algo);
gpg_error_t _gcry_ecc_mul_point (int algo, unsigned char *result,
                                 const unsigned char *scalar,
                                 const unsigned char *point);
gcry_err_code_t _gcry_pk_sign_md (gcry_sexp_t *r_sig, const char *tmpl,
                                  gcry_md_hd_t hd, gcry_sexp_t s_skey,
                                  gcry_ctx_t ctx);
gcry_err_code_t _gcry_pk_verify_md (gcry_sexp_t s_sig, const char *tmpl,
                                    gcry_md_hd_t hd, gcry_sexp_t s_pkey,
                                    gcry_ctx_t ctx);
gpg_err_code_t _gcry_pk_random_override_new (gcry_ctx_t *r_ctx,
                                             const unsigned char *p,
                                             size_t len);
gpg_err_code_t _gcry_pk_get_random_override (gcry_ctx_t ctx,
                                             const unsigned char **r_p,
                                             size_t *r_len);

gpg_err_code_t _gcry_md_open (gcry_md_hd_t *h, int algo, unsigned int flags);
void _gcry_md_close (gcry_md_hd_t hd);
gpg_err_code_t _gcry_md_enable (gcry_md_hd_t hd, int algo);
gpg_err_code_t _gcry_md_copy (gcry_md_hd_t *bhd, gcry_md_hd_t ahd);
void _gcry_md_reset (gcry_md_hd_t hd);
gpg_err_code_t _gcry_md_ctl (gcry_md_hd_t hd, int cmd,
                          void *buffer, size_t buflen);
void _gcry_md_write (gcry_md_hd_t hd, const void *buffer, size_t length);
unsigned char *_gcry_md_read (gcry_md_hd_t hd, int algo);
gpg_err_code_t _gcry_md_extract (gcry_md_hd_t hd, int algo, void *buffer,
                                 size_t length);
void _gcry_md_hash_buffer (int algo, void *digest,
                           const void *buffer, size_t length);
gpg_err_code_t _gcry_md_hash_buffers_extract (int algo, unsigned int flags,
                                              void *digest, int digestlen,
                                              const gcry_buffer_t *iov,
                                              int iovcnt);
gpg_err_code_t _gcry_md_hash_buffers (int algo, unsigned int flags,
                                      void *digest,
                                      const gcry_buffer_t *iov, int iovcnt);
int _gcry_md_get_algo (gcry_md_hd_t hd);
unsigned int _gcry_md_get_algo_dlen (int algo);
int _gcry_md_is_enabled (gcry_md_hd_t a, int algo);
int _gcry_md_is_secure (gcry_md_hd_t a);
gpg_err_code_t _gcry_md_info (gcry_md_hd_t h, int what, void *buffer,
                          size_t *nbytes);
gpg_err_code_t _gcry_md_algo_info (int algo, int what, void *buffer,
                                size_t *nbytes);
const char *_gcry_md_algo_name (int algo) _GCRY_GCC_ATTR_PURE;
int _gcry_md_map_name (const char* name) _GCRY_GCC_ATTR_PURE;
gpg_err_code_t _gcry_md_setkey (gcry_md_hd_t hd,
                                const void *key, size_t keylen);
void _gcry_md_debug (gcry_md_hd_t hd, const char *suffix);

#define _gcry_md_test_algo(a) \
            _gcry_md_algo_info ((a), GCRYCTL_TEST_ALGO, NULL, NULL)

#define _gcry_md_final(a) \
            _gcry_md_ctl ((a), GCRYCTL_FINALIZE, NULL, 0)

#define _gcry_md_putc(h,c)  \
            do {                                          \
                gcry_md_hd_t h__ = (h);                   \
                if( (h__)->bufpos == (h__)->bufsize )     \
                    _gcry_md_write( (h__), NULL, 0 );     \
                (h__)->buf[(h__)->bufpos++] = (c) & 0xff; \
            } while(0)



gpg_err_code_t _gcry_mac_open (gcry_mac_hd_t *handle, int algo,
                            unsigned int flags, gcry_ctx_t ctx);
void _gcry_mac_close (gcry_mac_hd_t h);
gpg_err_code_t _gcry_mac_ctl (gcry_mac_hd_t h, int cmd, void *buffer,
                           size_t buflen);
gpg_err_code_t _gcry_mac_algo_info (int algo, int what, void *buffer,
                                 size_t *nbytes);
gpg_err_code_t _gcry_mac_setkey (gcry_mac_hd_t hd, const void *key,
                              size_t keylen);
gpg_err_code_t _gcry_mac_setiv (gcry_mac_hd_t hd, const void *iv,
                             size_t ivlen);
gpg_err_code_t _gcry_mac_write (gcry_mac_hd_t hd, const void *buffer,
                             size_t length);
gpg_err_code_t _gcry_mac_read (gcry_mac_hd_t hd, void *buffer, size_t *buflen);
gpg_err_code_t _gcry_mac_verify (gcry_mac_hd_t hd, const void *buffer,
                                 size_t buflen);
int _gcry_mac_get_algo (gcry_mac_hd_t hd);
unsigned int _gcry_mac_get_algo_maclen (int algo);
unsigned int _gcry_mac_get_algo_keylen (int algo);
const char *_gcry_mac_algo_name (int algorithm) _GCRY_GCC_ATTR_PURE;
int _gcry_mac_map_name (const char *name) _GCRY_GCC_ATTR_PURE;

#define _gcry_mac_reset(h)  _gcry_mac_ctl ((h), GCRYCTL_RESET, NULL, 0)


gpg_err_code_t _gcry_kdf_derive (const void *passphrase, size_t passphraselen,
                                 int algo, int subalgo,
                                 const void *salt, size_t saltlen,
                                 unsigned long iterations,
                                 size_t keysize, void *keybuffer);

gpg_err_code_t _gcry_kdf_open (gcry_kdf_hd_t *hd, int algo, int subalgo,
                               const unsigned long *param,
                               unsigned int paramlen,
                               const void *passphrase, size_t passphraselen,
                               const void *salt, size_t saltlen,
                               const void *key, size_t keylen,
                               const void *ad, size_t adlen);
gcry_err_code_t _gcry_kdf_compute (gcry_kdf_hd_t h,
                                   const struct gcry_kdf_thread_ops *ops);
gpg_err_code_t _gcry_kdf_final (gcry_kdf_hd_t h, size_t resultlen, void *result);
void _gcry_kdf_close (gcry_kdf_hd_t h);


gpg_err_code_t _gcry_prime_generate (gcry_mpi_t *prime,
                                     unsigned int prime_bits,
                                     unsigned int factor_bits,
                                     gcry_mpi_t **factors,
                                     gcry_prime_check_func_t cb_func,
                                     void *cb_arg,
                                     gcry_random_level_t random_level,
                                     unsigned int flags);
gpg_err_code_t _gcry_prime_group_generator (gcry_mpi_t *r_g,
                                            gcry_mpi_t prime,
                                            gcry_mpi_t *factors,
                                            gcry_mpi_t start_g);
void _gcry_prime_release_factors (gcry_mpi_t *factors);
gpg_err_code_t _gcry_prime_check (gcry_mpi_t x, unsigned int flags);


void _gcry_randomize (void *buffer, size_t length,
                      enum gcry_random_level level);
gpg_err_code_t _gcry_random_add_bytes (const void *buffer, size_t length,
                                    int quality);
void *_gcry_random_bytes (size_t nbytes, enum gcry_random_level level)
                         _GCRY_GCC_ATTR_MALLOC;
void *_gcry_random_bytes_secure (size_t nbytes, enum gcry_random_level level)
                                _GCRY_GCC_ATTR_MALLOC;
void _gcry_mpi_randomize (gcry_mpi_t w,
                         unsigned int nbits, enum gcry_random_level level);
void _gcry_create_nonce (void *buffer, size_t length);


void _gcry_ctx_release (gcry_ctx_t ctx);


const char *_gcry_check_version (const char *req_version);

void _gcry_set_allocation_handler (gcry_handler_alloc_t func_alloc,
                                  gcry_handler_alloc_t func_alloc_secure,
                                  gcry_handler_secure_check_t func_secure_check,
                                  gcry_handler_realloc_t func_realloc,
                                  gcry_handler_free_t func_free);
void _gcry_set_outofcore_handler (gcry_handler_no_mem_t h, void *opaque);
void _gcry_set_fatalerror_handler (gcry_handler_error_t fnc, void *opaque);
void _gcry_set_log_handler (gcry_handler_log_t f, void *opaque);
void _gcry_set_gettext_handler (const char *(*f)(const char*));
void _gcry_set_progress_handler (gcry_handler_progress_t cb, void *cb_data);


/* Return a pointer to a string containing a description of the error
   code in the error value ERR.  */
static inline const char *
_gcry_strerror (gcry_error_t err)
{
  return gpg_strerror (err);
}

/* Return a pointer to a string containing a description of the error
   source in the error value ERR.  */
static inline const char *
_gcry_strsource (gcry_error_t err)
{
  return gpg_strsource (err);
}

/* Retrieve the error code for the system error ERR.  This returns
   GPG_ERR_UNKNOWN_ERRNO if the system error is not mapped (report
   this).  */
static inline gcry_err_code_t
_gcry_err_code_from_errno (int err)
{
  return gpg_err_code_from_errno (err);
}

/* Retrieve the system error for the error code CODE.  This returns 0
   if CODE is not a system error code.  */
static inline int
_gcry_err_code_to_errno (gcry_err_code_t code)
{
  return gpg_err_code_to_errno (code);
}

/* Return an error value with the error source SOURCE and the system
   error ERR.  */
static inline gcry_error_t
_gcry_err_make_from_errno (gpg_err_source_t source, int err)
{
  return gpg_err_make_from_errno (source, err);
}


/* Return an error value with the system error ERR.  */
static inline gcry_error_t
_gcry_error_from_errno (int err)
{
  return gpg_error (gpg_err_code_from_errno (err));
}



gpg_err_code_t _gcry_sexp_new (gcry_sexp_t *retsexp,
                               const void *buffer, size_t length,
                               int autodetect);
gpg_err_code_t _gcry_sexp_create (gcry_sexp_t *retsexp,
                                  void *buffer, size_t length,
                                  int autodetect, void (*freefnc) (void *));
gpg_err_code_t _gcry_sexp_sscan (gcry_sexp_t *retsexp, size_t *erroff,
                              const char *buffer, size_t length);
gpg_err_code_t _gcry_sexp_build (gcry_sexp_t *retsexp, size_t *erroff,
                                 const char *format, ...);
gpg_err_code_t _gcry_sexp_build_array (gcry_sexp_t *retsexp, size_t *erroff,
                                       const char *format, void **arg_list);
void _gcry_sexp_release (gcry_sexp_t sexp);
size_t _gcry_sexp_canon_len (const unsigned char *buffer, size_t length,
                            size_t *erroff, gcry_err_code_t *errcode);
size_t _gcry_sexp_sprint (gcry_sexp_t sexp, int mode, void *buffer,
                          size_t maxlength);
void _gcry_sexp_dump (const gcry_sexp_t a);
gcry_sexp_t _gcry_sexp_cons (const gcry_sexp_t a, const gcry_sexp_t b);
gcry_sexp_t _gcry_sexp_alist (const gcry_sexp_t *array);
gcry_sexp_t _gcry_sexp_vlist (const gcry_sexp_t a, ...);
gcry_sexp_t _gcry_sexp_append (const gcry_sexp_t a, const gcry_sexp_t n);
gcry_sexp_t _gcry_sexp_prepend (const gcry_sexp_t a, const gcry_sexp_t n);
gcry_sexp_t _gcry_sexp_find_token (gcry_sexp_t list,
                                   const char *tok, size_t toklen);
int _gcry_sexp_length (const gcry_sexp_t list);
gcry_sexp_t _gcry_sexp_nth (const gcry_sexp_t list, int number);
gcry_sexp_t _gcry_sexp_car (const gcry_sexp_t list);
gcry_sexp_t _gcry_sexp_cdr (const gcry_sexp_t list);
gcry_sexp_t _gcry_sexp_cadr (const gcry_sexp_t list);
const char *_gcry_sexp_nth_data (const gcry_sexp_t list, int number,
                                 size_t *datalen);
void *_gcry_sexp_nth_buffer (const gcry_sexp_t list, int number,
                             size_t *rlength);
char *_gcry_sexp_nth_string (gcry_sexp_t list, int number);
gcry_mpi_t _gcry_sexp_nth_mpi (gcry_sexp_t list, int number, int mpifmt);
gpg_err_code_t _gcry_sexp_extract_param (gcry_sexp_t sexp,
                                         const char *path,
                                         const char *list,
                                         ...) _GCRY_GCC_ATTR_SENTINEL(0);

#define sexp_new(a, b, c, d)         _gcry_sexp_new ((a), (b), (c), (d))
#define sexp_create(a, b, c, d, e)   _gcry_sexp_create ((a), (b), (c), (d), (e))
#define sexp_sscan(a, b, c, d)       _gcry_sexp_sscan ((a), (b), (c), (d))
#define sexp_build                   _gcry_sexp_build
#define sexp_build_array(a, b, c, d) _gcry_sexp_build_array ((a), (b), (c), (d))
#define sexp_release(a)              _gcry_sexp_release ((a))
#define sexp_canon_len(a, b, c, d)   _gcry_sexp_canon_len ((a), (b), (c), (d))
#define sexp_sprint(a, b, c, d)      _gcry_sexp_sprint ((a), (b), (c), (d))
#define sexp_dump(a)                 _gcry_sexp_dump ((a))
#define sexp_cons(a, b)              _gcry_sexp_cons ((a), (b))
#define sexp_alist(a)                _gcry_sexp_alist ((a))
#define sexp_vlist                   _gcry_sexp_vlist
#define sexp_append(a, b)            _gcry_sexp_append ((a), (b))
#define sexp_prepend(a, b)           _gcry_sexp_prepend ((a), (b))
#define sexp_find_token(a, b, c)     _gcry_sexp_find_token ((a), (b), (c))
#define sexp_length(a)               _gcry_sexp_length ((a))
#define sexp_nth(a, b)               _gcry_sexp_nth ((a), (b))
#define sexp_car(a)                  _gcry_sexp_car ((a))
#define sexp_cdr(a)                  _gcry_sexp_cdr ((a))
#define sexp_cadr(a)                 _gcry_sexp_cadr ((a))
#define sexp_nth_data(a, b, c)       _gcry_sexp_nth_data ((a), (b), (c))
#define sexp_nth_buffer(a, b, c)     _gcry_sexp_nth_buffer ((a), (b), (c))
#define sexp_nth_string(a, b)        _gcry_sexp_nth_string ((a), (b))
#define sexp_nth_mpi(a, b, c)        _gcry_sexp_nth_mpi ((a), (b), (c))
#define sexp_extract_param           _gcry_sexp_extract_param



gcry_mpi_t _gcry_mpi_new (unsigned int nbits);
gcry_mpi_t _gcry_mpi_snew (unsigned int nbits);
void _gcry_mpi_release (gcry_mpi_t a);
gcry_mpi_t _gcry_mpi_copy (const gcry_mpi_t a);
void _gcry_mpi_snatch (gcry_mpi_t w, gcry_mpi_t u);
gcry_mpi_t _gcry_mpi_set (gcry_mpi_t w, const gcry_mpi_t u);
gcry_mpi_t _gcry_mpi_set_ui (gcry_mpi_t w, unsigned long u);
gcry_err_code_t _gcry_mpi_get_ui (unsigned int *w, gcry_mpi_t u);
void _gcry_mpi_swap (gcry_mpi_t a, gcry_mpi_t b);
int _gcry_mpi_is_neg (gcry_mpi_t a);
void _gcry_mpi_neg (gcry_mpi_t w, gcry_mpi_t u);
void _gcry_mpi_abs (gcry_mpi_t w);
int _gcry_mpi_cmp (const gcry_mpi_t u, const gcry_mpi_t v);
int _gcry_mpi_cmpabs (const gcry_mpi_t u, const gcry_mpi_t v);
int _gcry_mpi_cmp_ui (const gcry_mpi_t u, unsigned long v);
gpg_err_code_t _gcry_mpi_scan (gcry_mpi_t *ret_mpi, enum gcry_mpi_format format,
                              const void *buffer, size_t buflen,
                              size_t *nscanned);
gpg_err_code_t _gcry_mpi_print (enum gcry_mpi_format format,
                               unsigned char *buffer, size_t buflen,
                               size_t *nwritten,
                               const gcry_mpi_t a);
gpg_err_code_t _gcry_mpi_aprint (enum gcry_mpi_format format,
                                unsigned char **buffer, size_t *nwritten,
                                const gcry_mpi_t a);
void _gcry_mpi_dump (const gcry_mpi_t a);
void _gcry_mpi_add (gcry_mpi_t w, gcry_mpi_t u, gcry_mpi_t v);
void _gcry_mpi_add_ui (gcry_mpi_t w, gcry_mpi_t u, unsigned long v);
void _gcry_mpi_addm (gcry_mpi_t w, gcry_mpi_t u, gcry_mpi_t v, gcry_mpi_t m);
void _gcry_mpi_sub (gcry_mpi_t w, gcry_mpi_t u, gcry_mpi_t v);
void _gcry_mpi_sub_ui (gcry_mpi_t w, gcry_mpi_t u, unsigned long v );
void _gcry_mpi_subm (gcry_mpi_t w, gcry_mpi_t u, gcry_mpi_t v, gcry_mpi_t m);
void _gcry_mpi_mul (gcry_mpi_t w, gcry_mpi_t u, gcry_mpi_t v);
void _gcry_mpi_mul_ui (gcry_mpi_t w, gcry_mpi_t u, unsigned long v );
void _gcry_mpi_mulm (gcry_mpi_t w, gcry_mpi_t u, gcry_mpi_t v, gcry_mpi_t m);
void _gcry_mpi_mul_2exp (gcry_mpi_t w, gcry_mpi_t u, unsigned long cnt);
void _gcry_mpi_div (gcry_mpi_t q, gcry_mpi_t r,
                   gcry_mpi_t dividend, gcry_mpi_t divisor, int round);
void _gcry_mpi_mod (gcry_mpi_t r, gcry_mpi_t dividend, gcry_mpi_t divisor);
void _gcry_mpi_powm (gcry_mpi_t w,
                    const gcry_mpi_t b, const gcry_mpi_t e,
                    const gcry_mpi_t m);
int _gcry_mpi_gcd (gcry_mpi_t g, gcry_mpi_t a, gcry_mpi_t b);
int _gcry_mpi_invm (gcry_mpi_t x, gcry_mpi_t a, gcry_mpi_t m);
gcry_mpi_point_t _gcry_mpi_point_new (unsigned int nbits);
void _gcry_mpi_point_release (gcry_mpi_point_t point);
gcry_mpi_point_t _gcry_mpi_point_copy (gcry_mpi_point_t point);
void _gcry_mpi_point_get (gcry_mpi_t x, gcry_mpi_t y, gcry_mpi_t z,
                         gcry_mpi_point_t point);
void _gcry_mpi_point_snatch_get (gcry_mpi_t x, gcry_mpi_t y, gcry_mpi_t z,
                                gcry_mpi_point_t point);
gcry_mpi_point_t _gcry_mpi_point_set (gcry_mpi_point_t point,
                                     gcry_mpi_t x, gcry_mpi_t y, gcry_mpi_t z);
gcry_mpi_point_t _gcry_mpi_point_snatch_set (gcry_mpi_point_t point,
                                            gcry_mpi_t x, gcry_mpi_t y,
                                            gcry_mpi_t z);

gcry_mpi_t _gcry_mpi_ec_get_mpi (const char *name, gcry_ctx_t ctx, int copy);
gcry_mpi_point_t _gcry_mpi_ec_get_point (const char *name,
                                        gcry_ctx_t ctx, int copy);
int _gcry_mpi_ec_get_affine (gcry_mpi_t x, gcry_mpi_t y, gcry_mpi_point_t point,
                             mpi_ec_t ctx);
void _gcry_mpi_ec_point_resize (gcry_mpi_point_t p, mpi_ec_t ctx);
void _gcry_mpi_ec_dup (gcry_mpi_point_t w, gcry_mpi_point_t u, gcry_ctx_t ctx);
void _gcry_mpi_ec_add (gcry_mpi_point_t w,
                       gcry_mpi_point_t u, gcry_mpi_point_t v, mpi_ec_t ctx);
void _gcry_mpi_ec_sub (gcry_mpi_point_t w,
                       gcry_mpi_point_t u, gcry_mpi_point_t v, mpi_ec_t ctx);
void _gcry_mpi_ec_mul (gcry_mpi_point_t w, gcry_mpi_t n, gcry_mpi_point_t u,
                       mpi_ec_t ctx);
int _gcry_mpi_ec_curve_point (gcry_mpi_point_t w, mpi_ec_t ctx);
unsigned int _gcry_mpi_get_nbits (gcry_mpi_t a);
int _gcry_mpi_test_bit (gcry_mpi_t a, unsigned int n);
void _gcry_mpi_set_bit (gcry_mpi_t a, unsigned int n);
void _gcry_mpi_clear_bit (gcry_mpi_t a, unsigned int n);
void _gcry_mpi_set_highbit (gcry_mpi_t a, unsigned int n);
void _gcry_mpi_clear_highbit (gcry_mpi_t a, unsigned int n);
void _gcry_mpi_rshift (gcry_mpi_t x, gcry_mpi_t a, unsigned int n);
void _gcry_mpi_lshift (gcry_mpi_t x, gcry_mpi_t a, unsigned int n);
gcry_mpi_t _gcry_mpi_set_opaque (gcry_mpi_t a, void *p, unsigned int nbits);
gcry_mpi_t _gcry_mpi_set_opaque_copy (gcry_mpi_t a,
                                     const void *p, unsigned int nbits);
void *_gcry_mpi_get_opaque (gcry_mpi_t a, unsigned int *nbits);
void _gcry_mpi_set_flag (gcry_mpi_t a, enum gcry_mpi_flag flag);
void _gcry_mpi_clear_flag (gcry_mpi_t a, enum gcry_mpi_flag flag);
int _gcry_mpi_get_flag (gcry_mpi_t a, enum gcry_mpi_flag flag);


/* Private function - do not use.  */
/* gcry_mpi_t _gcry_mpi_get_const (int no); */

/* We need our internal versions of the macros.  */
#ifndef GCRYPT_NO_MPI_MACROS
# error GCRYPT_NO_MPI_MACROS is not defined
#endif

#define mpi_new(n)             _gcry_mpi_new ((n))
#define mpi_secure_new( n )    _gcry_mpi_snew ((n))
#define mpi_snew(n)            _gcry_mpi_snew ((n))

#define mpi_release(a)        \
  do                          \
    {                         \
      _gcry_mpi_release ((a));\
      (a) = NULL;             \
    }                         \
  while (0)

#define mpi_snatch( w, u)      _gcry_mpi_snatch( (w), (u) )
#define mpi_set( w, u)         _gcry_mpi_set( (w), (u) )
#define mpi_set_ui( w, u)      _gcry_mpi_set_ui( (w), (u) )
#define mpi_get_ui(w,u)        _gcry_mpi_get_ui( (w), (u) )
#define mpi_swap(a,b)          _gcry_mpi_swap ((a),(b))
#define mpi_abs( w )           _gcry_mpi_abs( (w) )
#define mpi_neg( w, u)         _gcry_mpi_neg( (w), (u) )
#define mpi_cmp( u, v )        _gcry_mpi_cmp( (u), (v) )
#define mpi_cmpabs( u, v )     _gcry_mpi_cmpabs( (u), (v) )
#define mpi_cmp_ui( u, v )     _gcry_mpi_cmp_ui( (u), (v) )
#define mpi_is_neg( a )        _gcry_mpi_is_neg ((a))

#define mpi_add_ui(w,u,v)      _gcry_mpi_add_ui((w),(u),(v))
#define mpi_add(w,u,v)         _gcry_mpi_add ((w),(u),(v))
#define mpi_addm(w,u,v,m)      _gcry_mpi_addm ((w),(u),(v),(m))
#define mpi_sub_ui(w,u,v)      _gcry_mpi_sub_ui ((w),(u),(v))
#define mpi_sub(w,u,v)         _gcry_mpi_sub ((w),(u),(v))
#define mpi_subm(w,u,v,m)      _gcry_mpi_subm ((w),(u),(v),(m))
#define mpi_mul_ui(w,u,v)      _gcry_mpi_mul_ui ((w),(u),(v))
#define mpi_mul_2exp(w,u,v)    _gcry_mpi_mul_2exp ((w),(u),(v))
#define mpi_mul(w,u,v)         _gcry_mpi_mul ((w),(u),(v))
#define mpi_mulm(w,u,v,m)      _gcry_mpi_mulm ((w),(u),(v),(m))
#define mpi_powm(w,b,e,m)      _gcry_mpi_powm ( (w), (b), (e), (m) )
#define mpi_tdiv(q,r,a,m)      _gcry_mpi_div ( (q), (r), (a), (m), 0)
#define mpi_fdiv(q,r,a,m)      _gcry_mpi_div ( (q), (r), (a), (m), -1)
#define mpi_mod(r,a,m)         _gcry_mpi_mod ((r), (a), (m))
#define mpi_gcd(g,a,b)         _gcry_mpi_gcd ( (g), (a), (b) )
#define mpi_invm(g,a,b)        _gcry_mpi_invm ( (g), (a), (b) )

#define mpi_point_new(n)       _gcry_mpi_point_new((n))

#define mpi_point_release(p)                     \
  do                                             \
    {                                            \
      _gcry_mpi_point_release ((p));             \
      (p) = NULL;                                \
    }                                            \
  while (0)

#define mpi_point_copy(p)      _gcry_mpi_point_copy((p))

#define mpi_point_get(x,y,z,p)        _gcry_mpi_point_get((x),(y),(z),(p))
#define mpi_point_snatch_get(x,y,z,p) _gcry_mpi_point_snatch_get((x),(y), \
                                                                 (z),(p))
#define mpi_point_set(p,x,y,z)        _gcry_mpi_point_set((p),(x),(y),(z))
#define mpi_point_snatch_set(p,x,y,z) _gcry_mpi_point_snatch_set((p),(x), \
                                                                 (y),(z))
#define mpi_point_resize(p,ctx) _gcry_mpi_ec_point_resize (p, ctx)

#define mpi_get_nbits(a)       _gcry_mpi_get_nbits ((a))
#define mpi_test_bit(a,b)      _gcry_mpi_test_bit ((a),(b))
#define mpi_set_bit(a,b)       _gcry_mpi_set_bit ((a),(b))
#define mpi_set_highbit(a,b)   _gcry_mpi_set_highbit ((a),(b))
#define mpi_clear_bit(a,b)     _gcry_mpi_clear_bit ((a),(b))
#define mpi_clear_highbit(a,b) _gcry_mpi_clear_highbit ((a),(b))
#define mpi_rshift(a,b,c)      _gcry_mpi_rshift ((a),(b),(c))
#define mpi_lshift(a,b,c)      _gcry_mpi_lshift ((a),(b),(c))

#define mpi_set_opaque(a,b,c)  _gcry_mpi_set_opaque ((a), (b), (c))
#define mpi_get_opaque(a,b)    _gcry_mpi_get_opaque ((a), (b))
#define mpi_set_flag(a,f)      _gcry_mpi_set_flag ((a), (f))
#define mpi_set_flag(a,f)      _gcry_mpi_set_flag ((a), (f))
#define mpi_clear_flag(a,f)    _gcry_mpi_clear_flag ((a), (f))
#define mpi_get_flag(a,f)      _gcry_mpi_get_flag ((a), (f))


#endif /*GCRY_GCRYPT_INT_H*/
