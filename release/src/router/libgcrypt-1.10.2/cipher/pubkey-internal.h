/* pubkey-internal.h  - Internal defs for pubkey.c
 * Copyright (C) 2013 g10 code GmbH
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

#ifndef GCRY_PUBKEY_INTERNAL_H
#define GCRY_PUBKEY_INTERNAL_H

/*-- pubkey-util.c --*/
gpg_err_code_t _gcry_pk_util_parse_flaglist (gcry_sexp_t list,
                                             int *r_flags,
                                             enum pk_encoding *r_encoding);
gpg_err_code_t _gcry_pk_util_get_nbits (gcry_sexp_t list,
                                        unsigned int *r_nbits);
gpg_err_code_t _gcry_pk_util_get_rsa_use_e (gcry_sexp_t list,
                                            unsigned long *r_e);
gpg_err_code_t _gcry_pk_util_preparse_sigval (gcry_sexp_t s_sig,
                                              const char **algo_names,
                                              gcry_sexp_t *r_parms,
                                              int *r_eccflags);
gpg_err_code_t _gcry_pk_util_preparse_encval (gcry_sexp_t sexp,
                                              const char **algo_names,
                                              gcry_sexp_t *r_parms,
                                              struct pk_encoding_ctx *ctx);
void _gcry_pk_util_init_encoding_ctx (struct pk_encoding_ctx *ctx,
                                      enum pk_operation op,
                                      unsigned int nbits);
void _gcry_pk_util_free_encoding_ctx (struct pk_encoding_ctx *ctx);
gcry_err_code_t _gcry_pk_util_data_to_mpi (gcry_sexp_t input,
                                           gcry_mpi_t *ret_mpi,
                                           struct pk_encoding_ctx *ctx);



/*-- rsa-common.c --*/
gpg_err_code_t
_gcry_rsa_pkcs1_encode_for_enc (gcry_mpi_t *r_result, unsigned int nbits,
                                const unsigned char *value, size_t valuelen,
                                const unsigned char *random_override,
                                size_t random_override_len);
gpg_err_code_t
_gcry_rsa_pkcs1_decode_for_enc (unsigned char **r_result, size_t *r_resultlen,
                                unsigned int nbits, gcry_mpi_t value);
gpg_err_code_t
_gcry_rsa_pkcs1_encode_raw_for_sig (gcry_mpi_t *r_result, unsigned int nbits,
                                const unsigned char *value, size_t valuelen);

gpg_err_code_t
_gcry_rsa_pkcs1_encode_for_sig (gcry_mpi_t *r_result, unsigned int nbits,
                                const unsigned char *value, size_t valuelen,
                                int algo);
gpg_err_code_t
_gcry_rsa_oaep_encode (gcry_mpi_t *r_result, unsigned int nbits, int algo,
                       const unsigned char *value, size_t valuelen,
                       const unsigned char *label, size_t labellen,
                       const void *random_override, size_t random_override_len);
gpg_err_code_t
_gcry_rsa_oaep_decode (unsigned char **r_result, size_t *r_resultlen,
                       unsigned int nbits, int algo,
                       gcry_mpi_t value,
                       const unsigned char *label, size_t labellen);
gpg_err_code_t
_gcry_rsa_pss_encode (gcry_mpi_t *r_result, unsigned int nbits, int algo,
                      int hashed_already, int saltlen,
                      const unsigned char *value, size_t valuelen,
                      const void *random_override);
gpg_err_code_t
_gcry_rsa_pss_verify (gcry_mpi_t value, int hashed_already, gcry_mpi_t encoded,
                      unsigned int nbits, int algo, size_t saltlen);



/*-- dsa-common.c --*/
void _gcry_dsa_modify_k (gcry_mpi_t k, gcry_mpi_t q, int qbits);
gcry_mpi_t _gcry_dsa_gen_k (gcry_mpi_t q, int security_level);
gpg_err_code_t _gcry_dsa_gen_rfc6979_k (gcry_mpi_t *r_k,
                                        gcry_mpi_t dsa_q, gcry_mpi_t dsa_x,
                                        const unsigned char *h1,
                                        unsigned int h1len,
                                        int halgo,
                                        unsigned int extraloops);
gpg_err_code_t _gcry_dsa_compute_hash (gcry_mpi_t *r_hash, gcry_mpi_t input,
                                       int hashalgo);
gpg_err_code_t _gcry_dsa_normalize_hash (gcry_mpi_t input,
                                         gcry_mpi_t *out,
                                         unsigned int qbits);

/*-- ecc.c --*/
gpg_err_code_t _gcry_pk_ecc_get_sexp (gcry_sexp_t *r_sexp, int mode,
                                      mpi_ec_t ec);


#endif /*GCRY_PUBKEY_INTERNAL_H*/
