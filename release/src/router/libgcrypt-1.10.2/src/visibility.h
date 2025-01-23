/* visibility.h - Set visibility attribute
 * Copyright (C) 2007  Free Software Foundation, Inc.
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

#ifndef GCRY_VISIBILITY_H
#define GCRY_VISIBILITY_H

/* Redefine all public symbols with an underscore unless we already
   use the underscore prefixed version internally.  */


/* Include the main header here so that public symbols are mapped to
   the internal underscored ones.  */
#ifdef _GCRY_INCLUDED_BY_VISIBILITY_C
  /* We need to redeclare the deprecated functions without the
     deprecated attribute.  */
# define GCRYPT_NO_DEPRECATED
# include "gcrypt-int.h"
  /* None in this version.  */
#else
# include "gcrypt-int.h"
#endif

/* Prototypes of functions exported but not ready for use.  */
gcry_err_code_t gcry_md_get (gcry_md_hd_t hd, int algo,
                             unsigned char *buffer, int buflen);


/* Our use of the ELF visibility feature works by passing
   -fvisibiliy=hidden on the command line and by explicitly marking
   all exported functions as visible.

   NOTE: When adding new functions, please make sure to add them to
         libgcrypt.vers and libgcrypt.def as well.  */

#ifdef _GCRY_INCLUDED_BY_VISIBILITY_C

/* A macro to flag a function as visible.  */
#ifdef GCRY_USE_VISIBILITY
# define MARK_VISIBLEX(name) \
    extern __typeof__ (name) name __attribute__ ((visibility("default")));
#else
# define MARK_VISIBLEX(name) /* */
#endif


/* Now mark all symbols.  */

MARK_VISIBLEX (gcry_check_version)
MARK_VISIBLEX (gcry_control)

MARK_VISIBLEX (gcry_set_allocation_handler)
MARK_VISIBLEX (gcry_set_fatalerror_handler)
MARK_VISIBLEX (gcry_set_gettext_handler)
MARK_VISIBLEX (gcry_set_log_handler)
MARK_VISIBLEX (gcry_set_outofcore_handler)
MARK_VISIBLEX (gcry_set_progress_handler)

MARK_VISIBLEX (gcry_err_code_from_errno)
MARK_VISIBLEX (gcry_err_code_to_errno)
MARK_VISIBLEX (gcry_err_make_from_errno)
MARK_VISIBLEX (gcry_error_from_errno)
MARK_VISIBLEX (gcry_strerror)
MARK_VISIBLEX (gcry_strsource)

MARK_VISIBLEX (gcry_malloc)
MARK_VISIBLEX (gcry_malloc_secure)
MARK_VISIBLEX (gcry_calloc)
MARK_VISIBLEX (gcry_calloc_secure)
MARK_VISIBLEX (gcry_realloc)
MARK_VISIBLEX (gcry_strdup)
MARK_VISIBLEX (gcry_is_secure)
MARK_VISIBLEX (gcry_xcalloc)
MARK_VISIBLEX (gcry_xcalloc_secure)
MARK_VISIBLEX (gcry_xmalloc)
MARK_VISIBLEX (gcry_xmalloc_secure)
MARK_VISIBLEX (gcry_xrealloc)
MARK_VISIBLEX (gcry_xstrdup)
MARK_VISIBLEX (gcry_free)

MARK_VISIBLEX (gcry_md_algo_info)
MARK_VISIBLEX (gcry_md_algo_name)
MARK_VISIBLEX (gcry_md_close)
MARK_VISIBLEX (gcry_md_copy)
MARK_VISIBLEX (gcry_md_ctl)
MARK_VISIBLEX (gcry_md_enable)
MARK_VISIBLEX (gcry_md_get)
MARK_VISIBLEX (gcry_md_get_algo)
MARK_VISIBLEX (gcry_md_get_algo_dlen)
MARK_VISIBLEX (gcry_md_hash_buffer)
MARK_VISIBLEX (gcry_md_hash_buffers)
MARK_VISIBLEX (gcry_md_info)
MARK_VISIBLEX (gcry_md_is_enabled)
MARK_VISIBLEX (gcry_md_is_secure)
MARK_VISIBLEX (gcry_md_map_name)
MARK_VISIBLEX (gcry_md_open)
MARK_VISIBLEX (gcry_md_read)
MARK_VISIBLEX (gcry_md_extract)
MARK_VISIBLEX (gcry_md_reset)
MARK_VISIBLEX (gcry_md_setkey)
MARK_VISIBLEX (gcry_md_write)
MARK_VISIBLEX (gcry_md_debug)

MARK_VISIBLEX (gcry_cipher_algo_info)
MARK_VISIBLEX (gcry_cipher_algo_name)
MARK_VISIBLEX (gcry_cipher_close)
MARK_VISIBLEX (gcry_cipher_setkey)
MARK_VISIBLEX (gcry_cipher_setiv)
MARK_VISIBLEX (gcry_cipher_setctr)
MARK_VISIBLEX (gcry_cipher_authenticate)
MARK_VISIBLEX (gcry_cipher_checktag)
MARK_VISIBLEX (gcry_cipher_gettag)
MARK_VISIBLEX (gcry_cipher_ctl)
MARK_VISIBLEX (gcry_cipher_decrypt)
MARK_VISIBLEX (gcry_cipher_encrypt)
MARK_VISIBLEX (gcry_cipher_get_algo_blklen)
MARK_VISIBLEX (gcry_cipher_get_algo_keylen)
MARK_VISIBLEX (gcry_cipher_info)
MARK_VISIBLEX (gcry_cipher_map_name)
MARK_VISIBLEX (gcry_cipher_mode_from_oid)
MARK_VISIBLEX (gcry_cipher_open)

MARK_VISIBLEX (gcry_mac_algo_info)
MARK_VISIBLEX (gcry_mac_algo_name)
MARK_VISIBLEX (gcry_mac_map_name)
MARK_VISIBLEX (gcry_mac_get_algo)
MARK_VISIBLEX (gcry_mac_get_algo_maclen)
MARK_VISIBLEX (gcry_mac_get_algo_keylen)
MARK_VISIBLEX (gcry_mac_open)
MARK_VISIBLEX (gcry_mac_close)
MARK_VISIBLEX (gcry_mac_setkey)
MARK_VISIBLEX (gcry_mac_setiv)
MARK_VISIBLEX (gcry_mac_write)
MARK_VISIBLEX (gcry_mac_read)
MARK_VISIBLEX (gcry_mac_verify)
MARK_VISIBLEX (gcry_mac_ctl)

MARK_VISIBLEX (gcry_pk_algo_info)
MARK_VISIBLEX (gcry_pk_algo_name)
MARK_VISIBLEX (gcry_pk_ctl)
MARK_VISIBLEX (gcry_pk_decrypt)
MARK_VISIBLEX (gcry_pk_encrypt)
MARK_VISIBLEX (gcry_pk_genkey)
MARK_VISIBLEX (gcry_pk_get_keygrip)
MARK_VISIBLEX (gcry_pk_get_curve)
MARK_VISIBLEX (gcry_pk_get_param)
MARK_VISIBLEX (gcry_pk_get_nbits)
MARK_VISIBLEX (gcry_pk_map_name)
MARK_VISIBLEX (gcry_pk_sign)
MARK_VISIBLEX (gcry_pk_testkey)
MARK_VISIBLEX (gcry_pk_verify)
MARK_VISIBLEX (gcry_pubkey_get_sexp)
MARK_VISIBLEX (gcry_ecc_get_algo_keylen)
MARK_VISIBLEX (gcry_ecc_mul_point)
MARK_VISIBLEX (gcry_pk_hash_sign)
MARK_VISIBLEX (gcry_pk_hash_verify)
MARK_VISIBLEX (gcry_pk_random_override_new)

MARK_VISIBLEX (gcry_kdf_derive)
MARK_VISIBLEX (gcry_kdf_open)
MARK_VISIBLEX (gcry_kdf_compute)
MARK_VISIBLEX (gcry_kdf_final)
MARK_VISIBLEX (gcry_kdf_close)

MARK_VISIBLEX (gcry_prime_check)
MARK_VISIBLEX (gcry_prime_generate)
MARK_VISIBLEX (gcry_prime_group_generator)
MARK_VISIBLEX (gcry_prime_release_factors)

MARK_VISIBLEX (gcry_random_add_bytes)
MARK_VISIBLEX (gcry_random_bytes)
MARK_VISIBLEX (gcry_random_bytes_secure)
MARK_VISIBLEX (gcry_randomize)
MARK_VISIBLEX (gcry_create_nonce)

MARK_VISIBLEX (gcry_sexp_alist)
MARK_VISIBLEX (gcry_sexp_append)
MARK_VISIBLEX (gcry_sexp_build)
MARK_VISIBLEX (gcry_sexp_build_array)
MARK_VISIBLEX (gcry_sexp_cadr)
MARK_VISIBLEX (gcry_sexp_canon_len)
MARK_VISIBLEX (gcry_sexp_car)
MARK_VISIBLEX (gcry_sexp_cdr)
MARK_VISIBLEX (gcry_sexp_cons)
MARK_VISIBLEX (gcry_sexp_create)
MARK_VISIBLEX (gcry_sexp_dump)
MARK_VISIBLEX (gcry_sexp_find_token)
MARK_VISIBLEX (gcry_sexp_length)
MARK_VISIBLEX (gcry_sexp_new)
MARK_VISIBLEX (gcry_sexp_nth)
MARK_VISIBLEX (gcry_sexp_nth_buffer)
MARK_VISIBLEX (gcry_sexp_nth_data)
MARK_VISIBLEX (gcry_sexp_nth_mpi)
MARK_VISIBLEX (gcry_sexp_nth_string)
MARK_VISIBLEX (gcry_sexp_prepend)
MARK_VISIBLEX (gcry_sexp_release)
MARK_VISIBLEX (gcry_sexp_sprint)
MARK_VISIBLEX (gcry_sexp_sscan)
MARK_VISIBLEX (gcry_sexp_vlist)
MARK_VISIBLEX (gcry_sexp_extract_param)

MARK_VISIBLEX (gcry_mpi_abs)
MARK_VISIBLEX (gcry_mpi_add)
MARK_VISIBLEX (gcry_mpi_add_ui)
MARK_VISIBLEX (gcry_mpi_addm)
MARK_VISIBLEX (gcry_mpi_aprint)
MARK_VISIBLEX (gcry_mpi_clear_bit)
MARK_VISIBLEX (gcry_mpi_clear_flag)
MARK_VISIBLEX (gcry_mpi_clear_highbit)
MARK_VISIBLEX (gcry_mpi_cmp)
MARK_VISIBLEX (gcry_mpi_cmp_ui)
MARK_VISIBLEX (gcry_mpi_copy)
MARK_VISIBLEX (gcry_mpi_div)
MARK_VISIBLEX (gcry_mpi_dump)
MARK_VISIBLEX (gcry_mpi_ec_add)
MARK_VISIBLEX (gcry_mpi_ec_sub)
MARK_VISIBLEX (gcry_mpi_ec_curve_point)
MARK_VISIBLEX (gcry_mpi_ec_dup)
MARK_VISIBLEX (gcry_mpi_ec_decode_point)
MARK_VISIBLEX (gcry_mpi_ec_get_affine)
MARK_VISIBLEX (gcry_mpi_ec_mul)
MARK_VISIBLEX (gcry_mpi_ec_new)
MARK_VISIBLEX (gcry_mpi_ec_get_mpi)
MARK_VISIBLEX (gcry_mpi_ec_get_point)
MARK_VISIBLEX (gcry_mpi_ec_set_mpi)
MARK_VISIBLEX (gcry_mpi_ec_set_point)
MARK_VISIBLEX (gcry_mpi_gcd)
MARK_VISIBLEX (gcry_mpi_get_flag)
MARK_VISIBLEX (gcry_mpi_get_nbits)
MARK_VISIBLEX (gcry_mpi_get_opaque)
MARK_VISIBLEX (gcry_mpi_is_neg)
MARK_VISIBLEX (gcry_mpi_invm)
MARK_VISIBLEX (gcry_mpi_mod)
MARK_VISIBLEX (gcry_mpi_mul)
MARK_VISIBLEX (gcry_mpi_mul_2exp)
MARK_VISIBLEX (gcry_mpi_mul_ui)
MARK_VISIBLEX (gcry_mpi_mulm)
MARK_VISIBLEX (gcry_mpi_neg)
MARK_VISIBLEX (gcry_mpi_new)
MARK_VISIBLEX (gcry_mpi_point_get)
MARK_VISIBLEX (gcry_mpi_point_new)
MARK_VISIBLEX (gcry_mpi_point_release)
MARK_VISIBLEX (gcry_mpi_point_copy)
MARK_VISIBLEX (gcry_mpi_point_set)
MARK_VISIBLEX (gcry_mpi_point_snatch_get)
MARK_VISIBLEX (gcry_mpi_point_snatch_set)
MARK_VISIBLEX (gcry_mpi_powm)
MARK_VISIBLEX (gcry_mpi_print)
MARK_VISIBLEX (gcry_mpi_randomize)
MARK_VISIBLEX (gcry_mpi_release)
MARK_VISIBLEX (gcry_mpi_rshift)
MARK_VISIBLEX (gcry_mpi_lshift)
MARK_VISIBLEX (gcry_mpi_scan)
MARK_VISIBLEX (gcry_mpi_snatch)
MARK_VISIBLEX (gcry_mpi_set)
MARK_VISIBLEX (gcry_mpi_set_bit)
MARK_VISIBLEX (gcry_mpi_set_flag)
MARK_VISIBLEX (gcry_mpi_set_highbit)
MARK_VISIBLEX (gcry_mpi_set_opaque)
MARK_VISIBLEX (gcry_mpi_set_opaque_copy)
MARK_VISIBLEX (gcry_mpi_set_ui)
MARK_VISIBLEX (gcry_mpi_get_ui)
MARK_VISIBLEX (gcry_mpi_snew)
MARK_VISIBLEX (gcry_mpi_sub)
MARK_VISIBLEX (gcry_mpi_sub_ui)
MARK_VISIBLEX (gcry_mpi_subm)
MARK_VISIBLEX (gcry_mpi_swap)
MARK_VISIBLEX (gcry_mpi_test_bit)

MARK_VISIBLEX (gcry_ctx_release)

MARK_VISIBLEX (gcry_log_debug)
MARK_VISIBLEX (gcry_log_debughex)
MARK_VISIBLEX (gcry_log_debugmpi)
MARK_VISIBLEX (gcry_log_debugpnt)
MARK_VISIBLEX (gcry_log_debugsxp)

MARK_VISIBLEX (gcry_get_config)

/* Functions used to implement macros.  */
MARK_VISIBLEX (_gcry_mpi_get_const)


#undef MARK_VISIBLEX

#else /*!_GCRY_INCLUDED_BY_VISIBILITY_C*/

/* To avoid accidental use of the public functions inside Libgcrypt,
   we redefine them to catch such errors.  The usual difference
   between a public and an internal version is that the internal
   version use gpg_err_code_t and the public version gpg_error_t.  */

#define gcry_check_version          _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_control                _gcry_USE_THE_UNDERSCORED_FUNCTION

#define gcry_set_allocation_handler _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_set_fatalerror_handler _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_set_gettext_handler    _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_set_log_handler        _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_set_outofcore_handler  _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_set_progress_handler   _gcry_USE_THE_UNDERSCORED_FUNCTION

#define gcry_err_code_from_errno    _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_err_code_to_errno      _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_err_make_from_errno    _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_error_from_errno       _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_strerror               _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_strsource              _gcry_USE_THE_UNDERSCORED_FUNCTION

#define gcry_malloc                 _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_malloc_secure          _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_calloc                 _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_calloc_secure          _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_realloc                _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_strdup                 _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_xcalloc                _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_xcalloc_secure         _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_xmalloc                _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_xmalloc_secure         _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_xrealloc               _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_xstrdup                _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_free                   _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_is_secure              _gcry_USE_THE_UNDERSCORED_FUNCTION

#define gcry_cipher_open            _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_cipher_close           _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_cipher_setkey          _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_cipher_setiv           _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_cipher_setctr          _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_cipher_algo_info       _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_cipher_algo_name       _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_cipher_authenticate    _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_cipher_checktag        _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_cipher_gettag          _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_cipher_ctl             _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_cipher_decrypt         _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_cipher_encrypt         _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_cipher_get_algo_blklen _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_cipher_get_algo_keylen _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_cipher_info            _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_cipher_map_name        _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_cipher_mode_from_oid   _gcry_USE_THE_UNDERSCORED_FUNCTION

#define gcry_pk_algo_info           _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_pk_algo_name           _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_pk_ctl                 _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_pk_decrypt             _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_pk_encrypt             _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_pk_genkey              _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_pk_get_keygrip         _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_pk_get_curve           _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_pk_get_param           _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_pk_get_nbits           _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_pk_map_name            _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_pk_sign                _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_pk_testkey             _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_pk_verify              _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_pubkey_get_sexp        _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_ecc_get_algo_keylen    _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_ecc_mul_point          _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_pk_hash_sign           _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_pk_hash_verify         _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_pk_random_override_new _gcry_USE_THE_UNDERSCORED_FUNCTION

#define gcry_md_algo_info           _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_md_algo_name           _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_md_close               _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_md_copy                _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_md_ctl                 _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_md_enable              _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_md_get                 _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_md_get_algo            _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_md_get_algo_dlen       _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_md_hash_buffer         _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_md_hash_buffers        _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_md_info                _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_md_is_enabled          _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_md_is_secure           _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_md_map_name            _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_md_open                _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_md_read                _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_md_extract             _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_md_reset               _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_md_setkey              _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_md_write               _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_md_debug               _gcry_USE_THE_UNDERSCORED_FUNCTION

#define gcry_mac_algo_info          _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mac_algo_name          _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mac_map_name           _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mac_get_algo           _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mac_get_algo_maclen    _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mac_get_algo_keylen    _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mac_open               _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mac_close              _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mac_setkey             _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mac_setiv              _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mac_write              _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mac_read               _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mac_verify             _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mac_ctl                _gcry_USE_THE_UNDERSCORED_FUNCTION

#define gcry_kdf_derive             _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_kdf_open               _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_kdf_compute            _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_kdf_final              _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_kdf_close              _gcry_USE_THE_UNDERSCORED_FUNCTION

#define gcry_prime_check            _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_prime_generate         _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_prime_group_generator  _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_prime_release_factors  _gcry_USE_THE_UNDERSCORED_FUNCTION

#define gcry_random_add_bytes       _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_random_bytes           _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_random_bytes_secure    _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_randomize              _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_create_nonce           _gcry_USE_THE_UNDERSCORED_FUNCTION

#define gcry_ctx_release            _gcry_USE_THE_UNDERSCORED_FUNCTION

#define gcry_sexp_alist             _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_sexp_append            _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_sexp_build             _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_sexp_build_array       _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_sexp_cadr              _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_sexp_canon_len         _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_sexp_car               _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_sexp_cdr               _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_sexp_cons              _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_sexp_create            _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_sexp_dump              _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_sexp_find_token        _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_sexp_length            _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_sexp_new               _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_sexp_nth               _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_sexp_nth_buffer        _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_sexp_nth_data          _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_sexp_nth_mpi           _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_sexp_nth_string        _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_sexp_prepend           _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_sexp_release           _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_sexp_sprint            _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_sexp_sscan             _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_sexp_vlist             _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_sexp_extract_param     _gcry_USE_THE_UNDERSCORED_FUNCTION

#define gcry_mpi_add                _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_add_ui             _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_addm               _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_aprint             _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_clear_bit          _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_clear_flag         _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_clear_highbit      _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_cmp                _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_cmp_ui             _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_copy               _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_div                _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_dump               _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_gcd                _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_get_flag           _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_get_nbits          _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_get_opaque         _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_invm               _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_mod                _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_mul                _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_mul_2exp           _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_mul_ui             _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_mulm               _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_new                _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_point_get          _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_point_new          _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_point_release      _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_point_copy         _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_point_set          _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_point_snatch_get   _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_point_snatch_set   _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_powm               _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_print              _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_randomize          _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_release            _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_rshift             _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_lshift             _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_scan               _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_set                _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_set_bit            _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_set_flag           _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_set_highbit        _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_set_opaque         _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_set_ui             _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_get_ui             _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_snatch             _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_snew               _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_sub                _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_sub_ui             _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_subm               _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_swap               _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_test_bit           _gcry_USE_THE_UNDERSCORED_FUNCTION

#define gcry_mpi_abs                _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_ec_add             _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_ec_sub             _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_ec_curve_point     _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_ec_dup             _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_ec_decode_point    _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_ec_get_affine      _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_ec_get_mpi         _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_ec_get_point       _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_ec_mul             _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_ec_new             _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_ec_set_mpi         _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_ec_set_point       _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_is_neg             _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_neg                _gcry_USE_THE_UNDERSCORED_FUNCTION
#define gcry_mpi_set_opaque_copy    _gcry_USE_THE_UNDERSCORED_FUNCTION


#endif /*!_GCRY_INCLUDED_BY_VISIBILITY_C*/

#endif /*GCRY_VISIBILITY_H*/
