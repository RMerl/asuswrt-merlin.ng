/* ecc-common.h - Declarations of common ECC code
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

#ifndef GCRY_ECC_COMMON_H
#define GCRY_ECC_COMMON_H


/* Definition of a curve.  */
typedef struct
{
  enum gcry_mpi_ec_models model;/* The model descrinbing this curve.  */
  enum ecc_dialects dialect;    /* The dialect used with the curve.   */
  gcry_mpi_t p;         /* Prime specifying the field GF(p).  */
  gcry_mpi_t a;         /* First coefficient of the Weierstrass equation.  */
  gcry_mpi_t b;         /* Second coefficient of the Weierstrass equation.
                           or d as used by Twisted Edwards curves.  */
  mpi_point_struct G;   /* Base point (generator).  */
  gcry_mpi_t n;         /* Order of G.  */
  unsigned int h;       /* Cofactor.  */
  const char *name;     /* Name of the curve or NULL.  */
} elliptic_curve_t;



/* Set the value from S into D.  */
static inline void
point_set (mpi_point_t d, mpi_point_t s)
{
  mpi_set (d->x, s->x);
  mpi_set (d->y, s->y);
  mpi_set (d->z, s->z);
}

#define point_init(a)  _gcry_mpi_point_init ((a))
#define point_free(a)  _gcry_mpi_point_free_parts ((a))


/*-- ecc-curves.c --*/
gpg_err_code_t _gcry_ecc_fill_in_curve (unsigned int nbits,
                                        const char *name,
                                        elliptic_curve_t *curve,
                                        unsigned int *r_nbits);
gpg_err_code_t _gcry_ecc_update_curve_param (const char *name,
                                             enum gcry_mpi_ec_models *model,
                                             enum ecc_dialects *dialect,
                                             gcry_mpi_t *p, gcry_mpi_t *a,
                                             gcry_mpi_t *b, gcry_mpi_t *g,
                                             gcry_mpi_t *n);

const char *_gcry_ecc_get_curve (gcry_sexp_t keyparms,
                                 int iterator,
                                 unsigned int *r_nbits);
gcry_sexp_t _gcry_ecc_get_param_sexp (const char *name);

/*-- ecc-misc.c --*/
void _gcry_ecc_curve_free (elliptic_curve_t *E);
elliptic_curve_t _gcry_ecc_curve_copy (elliptic_curve_t E);
const char *_gcry_ecc_model2str (enum gcry_mpi_ec_models model);
const char *_gcry_ecc_dialect2str (enum ecc_dialects dialect);
unsigned char *_gcry_ecc_ec2os_buf (gcry_mpi_t x, gcry_mpi_t y, gcry_mpi_t p,
                                    unsigned int *r_length);
gcry_mpi_t   _gcry_ecc_ec2os (gcry_mpi_t x, gcry_mpi_t y, gcry_mpi_t p);

mpi_point_t  _gcry_ecc_compute_public (mpi_point_t Q, mpi_ec_t ec);
gpg_err_code_t _gcry_ecc_mont_encodepoint (gcry_mpi_t x, unsigned int nbits,
                                           int with_prefix,
                                           unsigned char **r_buffer,
                                           unsigned int *r_buflen);


/*-- ecc.c --*/

/*-- ecc-ecdsa.c --*/
gpg_err_code_t _gcry_ecc_ecdsa_sign (gcry_mpi_t input, gcry_mpi_t k, mpi_ec_t ec,
                                     gcry_mpi_t r, gcry_mpi_t s,
                                     int flags, int hashalgo);
gpg_err_code_t _gcry_ecc_ecdsa_verify (gcry_mpi_t input, mpi_ec_t ec,
                                       gcry_mpi_t r, gcry_mpi_t s,
                                       int flags, int hashalgo);

/*-- ecc-eddsa.c --*/
gpg_err_code_t _gcry_ecc_eddsa_recover_x (gcry_mpi_t x, gcry_mpi_t y, int sign,
                                          mpi_ec_t ec);
gpg_err_code_t _gcry_ecc_eddsa_encodepoint (mpi_point_t point, mpi_ec_t ctx,
                                            gcry_mpi_t x, gcry_mpi_t y,
                                            int with_prefix,
                                            unsigned char **r_buffer,
                                            unsigned int *r_buflen);
gpg_err_code_t _gcry_ecc_eddsa_ensure_compact (gcry_mpi_t value,
                                               unsigned int nbits);


gpg_err_code_t _gcry_ecc_eddsa_compute_h_d (unsigned char **r_digest,
                                            mpi_ec_t ec);

gpg_err_code_t _gcry_ecc_eddsa_genkey (mpi_ec_t ec, int flags);
gpg_err_code_t _gcry_ecc_eddsa_sign (gcry_mpi_t input,
                                     mpi_ec_t ec,
                                     gcry_mpi_t r_r, gcry_mpi_t s,
                                     struct pk_encoding_ctx *ctx);
gpg_err_code_t _gcry_ecc_eddsa_verify (gcry_mpi_t input,
                                       mpi_ec_t ec,
                                       gcry_mpi_t r, gcry_mpi_t s,
                                       struct pk_encoding_ctx *ctx);
void reverse_buffer (unsigned char *buffer, unsigned int length);


/*-- ecc-gost.c --*/
gpg_err_code_t _gcry_ecc_gost_sign (gcry_mpi_t input, mpi_ec_t ec,
                                    gcry_mpi_t r, gcry_mpi_t s);
gpg_err_code_t _gcry_ecc_gost_verify (gcry_mpi_t input, mpi_ec_t ec,
                                      gcry_mpi_t r, gcry_mpi_t s);


/*-- ecc-sm2.c --*/
gpg_err_code_t _gcry_ecc_sm2_encrypt (gcry_sexp_t *r_ciph,
                                      gcry_mpi_t input, mpi_ec_t ec);
gpg_err_code_t _gcry_ecc_sm2_decrypt (gcry_sexp_t *r_plain,
                                      gcry_sexp_t data_list, mpi_ec_t ec);
gpg_err_code_t _gcry_ecc_sm2_sign (gcry_mpi_t input, mpi_ec_t ec,
                                   gcry_mpi_t r, gcry_mpi_t s,
                                   int flags, int hashalgo);
gpg_err_code_t _gcry_ecc_sm2_verify (gcry_mpi_t input, mpi_ec_t ec,
                                     gcry_mpi_t r, gcry_mpi_t s);


#endif /*GCRY_ECC_COMMON_H*/
