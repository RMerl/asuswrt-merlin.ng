/* ec-context.h - Private definitions for CONTEXT_TYPE_EC.
 * Copyright (C) 2013  g10 Code GmbH
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

#ifndef GCRY_EC_CONTEXT_H
#define GCRY_EC_CONTEXT_H

/* This context is used with all our EC functions. */
struct mpi_ec_ctx_s
{
  enum gcry_mpi_ec_models model; /* The model describing this curve.  */

  enum ecc_dialects dialect;     /* The ECC dialect used with the curve.  */

  int flags;                     /* Public key flags (not always used).  */

  unsigned int nbits;            /* Number of bits.  */

  /* Domain parameters.  Note that they may not all be set and if set
     the MPIs may be flaged as constant. */
  gcry_mpi_t p;         /* Prime specifying the field GF(p).  */
  gcry_mpi_t a;         /* First coefficient of the Weierstrass equation.  */
  gcry_mpi_t b;         /* Second coefficient of the Weierstrass equation.  */
  gcry_mpi_point_t G;   /* Base point (generator).  */
  gcry_mpi_t n;         /* Order of G.  */
  unsigned int h;       /* Cofactor.  */

  /* The actual key.  May not be set.  */
  gcry_mpi_point_t Q;   /* Public key.   */
  gcry_mpi_t d;         /* Private key.  */

  const char *name;      /* Name of the curve.  */

  /* This structure is private to mpi/ec.c! */
  struct {
    struct {
      unsigned int a_is_pminus3:1;
      unsigned int two_inv_p:1;
    } valid; /* Flags to help setting the helper vars below.  */

    int a_is_pminus3;  /* True if A = P - 3. */

    gcry_mpi_t two_inv_p;

    mpi_barrett_t p_barrett;

    /* Scratch variables.  */
    gcry_mpi_t scratch[11];

    /* Helper for fast reduction.  */
    /*   int nist_nbits; /\* If this is a NIST curve, the # of bits.  *\/ */
    /*   gcry_mpi_t s[10]; */
    /*   gcry_mpi_t c; */
  } t;

  /* Curve specific computation routines for the field.  */
  void (* addm) (gcry_mpi_t w, gcry_mpi_t u, gcry_mpi_t v, mpi_ec_t ctx);
  void (* subm) (gcry_mpi_t w, gcry_mpi_t u, gcry_mpi_t v, mpi_ec_t ec);
  void (* mulm) (gcry_mpi_t w, gcry_mpi_t u, gcry_mpi_t v, mpi_ec_t ctx);
  void (* pow2) (gcry_mpi_t w, const gcry_mpi_t b, mpi_ec_t ctx);
  void (* mul2) (gcry_mpi_t w, gcry_mpi_t u, mpi_ec_t ctx);
  void (* mod) (gcry_mpi_t w, mpi_ec_t ctx);
};


/*-- mpi/ec.c --*/
void _gcry_mpi_ec_get_reset (mpi_ec_t ec);


/*-- cipher/ecc-curves.c --*/
gcry_mpi_t       _gcry_ecc_get_mpi (const char *name, mpi_ec_t ec, int copy);
gcry_mpi_point_t _gcry_ecc_get_point (const char *name, mpi_ec_t ec);
gpg_err_code_t   _gcry_ecc_set_mpi (const char *name,
                                    gcry_mpi_t newvalue, mpi_ec_t ec);
gpg_err_code_t   _gcry_ecc_set_point (const char *name,
                                      gcry_mpi_point_t newvalue, mpi_ec_t ec);

/*-- cipher/ecc-misc.c --*/
gpg_err_code_t _gcry_ecc_sec_decodepoint (gcry_mpi_t value, mpi_ec_t ec,
                                          mpi_point_t result);
gpg_err_code_t _gcry_ecc_mont_decodepoint (gcry_mpi_t pk, mpi_ec_t ctx,
                                           mpi_point_t result);

/*-- cipher/ecc-eddsa.c --*/
gpg_err_code_t _gcry_ecc_eddsa_decodepoint (gcry_mpi_t pk, mpi_ec_t ctx,
                                            mpi_point_t result,
                                            unsigned char **r_encpk,
                                            unsigned int *r_encpklen);



#endif /*GCRY_EC_CONTEXT_H*/
