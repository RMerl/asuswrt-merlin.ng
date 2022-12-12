/* ecc-internal.h

   Copyright (C) 2013, 2014 Niels Möller

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

/* Development of Nettle's ECC support was funded by the .SE Internet Fund. */

#ifndef NETTLE_ECC_INTERNAL_H_INCLUDED
#define NETTLE_ECC_INTERNAL_H_INCLUDED

#include "nettle-types.h"
#include "bignum.h"
#include "ecc-curve.h"
#include "gmp-glue.h"

/* Name mangling */
#define ecc_pp1_redc _nettle_ecc_pp1_redc
#define ecc_pm1_redc _nettle_ecc_pm1_redc
#define ecc_mod_zero_p _nettle_ecc_mod_zero_p
#define ecc_mod_equal_p _nettle_ecc_mod_equal_p
#define ecc_mod_add _nettle_ecc_mod_add
#define ecc_mod_sub _nettle_ecc_mod_sub
#define ecc_mod_mul_1 _nettle_ecc_mod_mul_1
#define ecc_mod_addmul_1 _nettle_ecc_mod_addmul_1
#define ecc_mod_submul_1 _nettle_ecc_mod_submul_1
#define ecc_mod_mul _nettle_ecc_mod_mul
#define ecc_mod_sqr _nettle_ecc_mod_sqr
#define ecc_mod_mul_canonical _nettle_ecc_mod_mul_canonical
#define ecc_mod_sqr_canonical _nettle_ecc_mod_sqr_canonical
#define ecc_mod_pow_2k _nettle_ecc_mod_pow_2k
#define ecc_mod_pow_2k_mul _nettle_ecc_mod_pow_2k_mul
#define ecc_mod_random _nettle_ecc_mod_random
#define ecc_mod _nettle_ecc_mod
#define ecc_mod_inv _nettle_ecc_mod_inv
#define ecc_hash _nettle_ecc_hash
#define gost_hash _nettle_gost_hash
#define ecc_a_to_j _nettle_ecc_a_to_j
#define ecc_j_to_a _nettle_ecc_j_to_a
#define ecc_eh_to_a _nettle_ecc_eh_to_a
#define ecc_dup_jj _nettle_ecc_dup_jj
#define ecc_add_jja _nettle_ecc_add_jja
#define ecc_add_jjj _nettle_ecc_add_jjj
#define ecc_dup_eh _nettle_ecc_dup_eh
#define ecc_add_eh _nettle_ecc_add_eh
#define ecc_add_ehh _nettle_ecc_add_ehh
#define ecc_dup_th _nettle_ecc_dup_th
#define ecc_add_th _nettle_ecc_add_th
#define ecc_add_thh _nettle_ecc_add_thh
#define ecc_mul_g _nettle_ecc_mul_g
#define ecc_mul_a _nettle_ecc_mul_a
#define ecc_mul_g_eh _nettle_ecc_mul_g_eh
#define ecc_mul_a_eh _nettle_ecc_mul_a_eh
#define ecc_mul_m _nettle_ecc_mul_m
#define cnd_copy _nettle_cnd_copy
#define sec_add_1 _nettle_sec_add_1
#define sec_sub_1 _nettle_sec_sub_1
#define sec_tabselect _nettle_sec_tabselect
#define sec_modinv _nettle_sec_modinv
#define curve25519_eh_to_x _nettle_curve25519_eh_to_x
#define curve448_eh_to_x _nettle_curve448_eh_to_x

extern const struct ecc_curve _nettle_secp_192r1;
extern const struct ecc_curve _nettle_secp_224r1;
extern const struct ecc_curve _nettle_secp_256r1;
extern const struct ecc_curve _nettle_secp_384r1;
extern const struct ecc_curve _nettle_secp_521r1;

/* Keep this structure internal for now. It's misnamed (since it's
   really implementing the equivalent twisted Edwards curve, with
   different coordinates). And we're not quite ready to provide
   general ecc operations over an arbitrary type of curve. */
extern const struct ecc_curve _nettle_curve25519;
extern const struct ecc_curve _nettle_curve448;

/* GOST curves, visible with underscore prefix for now */
extern const struct ecc_curve _nettle_gost_gc256b;
extern const struct ecc_curve _nettle_gost_gc512a;

#define ECC_MAX_SIZE ((521 + GMP_NUMB_BITS - 1) / GMP_NUMB_BITS)

/* Window size for ecc_mul_a. Using 4 bits seems like a good choice,
   for both Intel x86_64 and ARM Cortex A9. For the larger curves, of
   384 and 521 bits, we could improve speed by a few percent if we go
   up to 5 bits, but I don't think that's worth doubling the
   storage. */
#define ECC_MUL_A_WBITS 4
/* And for ecc_mul_a_eh */
#define ECC_MUL_A_EH_WBITS 4

struct ecc_modulo;

/* Reduces from 2*ecc->size to ecc->size. */
/* Required to return a result < 2q. This property is inherited by
   mod_mul and mod_sqr. May clobber input xp. rp may point to the
   start or the middle of the xp area, but no other overlap is
   allowed. */
typedef void ecc_mod_func (const struct ecc_modulo *m, mp_limb_t *rp, mp_limb_t *xp);

typedef void ecc_mod_inv_func (const struct ecc_modulo *m,
			       mp_limb_t *vp, const mp_limb_t *ap,
			       mp_limb_t *scratch);

/* Computes the square root of ap mod p. No overlap between input and output. */
typedef int ecc_mod_sqrt_func (const struct ecc_modulo *m,
			       mp_limb_t *vp, const mp_limb_t *ap,
			       mp_limb_t *scratch);

/* Computes the square root of (u/v) (mod p). */
typedef int ecc_mod_sqrt_ratio_func (const struct ecc_modulo *m,
				     mp_limb_t *rp,
				     const mp_limb_t *up, const mp_limb_t *vp,
				     mp_limb_t *scratch);

/* Allows in-place operation with r == p, but not r == q */
typedef void ecc_add_func (const struct ecc_curve *ecc,
			   mp_limb_t *r,
			   const mp_limb_t *p, const mp_limb_t *q,
			   mp_limb_t *scratch);

typedef void ecc_dup_func (const struct ecc_curve *ecc,
			   mp_limb_t *r, const mp_limb_t *p,
			   mp_limb_t *scratch);

typedef void ecc_mul_g_func (const struct ecc_curve *ecc, mp_limb_t *r,
			     const mp_limb_t *np, mp_limb_t *scratch);

typedef void ecc_mul_func (const struct ecc_curve *ecc,
			   mp_limb_t *r,
			   const mp_limb_t *np, const mp_limb_t *p,
			   mp_limb_t *scratch);

typedef void ecc_h_to_a_func (const struct ecc_curve *ecc,
			      int flags,
			      mp_limb_t *r, const mp_limb_t *p,
			      mp_limb_t *scratch);

struct ecc_modulo
{
  unsigned short bit_size;
  unsigned short size;
  unsigned short B_size;
  unsigned short redc_size;
  unsigned short invert_itch;
  unsigned short sqrt_itch;
  unsigned short sqrt_ratio_itch;

  const mp_limb_t *m;
  /* B^size mod m. Expected to have at least 32 leading zeros
     (equality for secp_256r1). */
  const mp_limb_t *B;
  /* 2^{bit_size} - m, same value as above, but shifted. */
  const mp_limb_t *B_shifted;
  /* m +/- 1, for redc, excluding redc_size low limbs. */
  const mp_limb_t *redc_mpm1;
  /* (m+1)/2 */
  const mp_limb_t *mp1h;

  ecc_mod_func *mod;
  ecc_mod_func *reduce;
  /* For moduli where we use redc, the invert and sqrt functions work
     with inputs and outputs in redc form. */
  ecc_mod_inv_func *invert;
  ecc_mod_sqrt_func *sqrt;
  ecc_mod_sqrt_ratio_func *sqrt_ratio;
};

/* Represents an elliptic curve of the form

     y^2 = x^3 - 3x + b (mod p)
*/
struct ecc_curve
{
  /* The prime p. */
  struct ecc_modulo p;
  /* Group order. Currently, many functions rely on q.size ==
     p.size. */
  struct ecc_modulo q;

  unsigned short use_redc;
  unsigned short pippenger_k;
  unsigned short pippenger_c;

  unsigned short add_hh_itch;
  unsigned short add_hhh_itch;
  unsigned short dup_itch;
  unsigned short mul_itch;
  unsigned short mul_g_itch;
  unsigned short h_to_a_itch;

  ecc_add_func *add_hh;
  ecc_add_func *add_hhh;
  ecc_dup_func *dup;
  ecc_mul_func *mul;
  ecc_mul_g_func *mul_g;
  ecc_h_to_a_func *h_to_a;

  /* Curve constant */
  const mp_limb_t *b;

  /* For redc, same as B mod p, otherwise 1. */
  const mp_limb_t *unit;

  /* Tables for multiplying by the generator, size determined by k and
     c. The first 2^c entries are defined by

       T[  j_0 +   j_1 2 +     ... + j_{c-1} 2^{c-1} ]
         = j_0 g + j_1 2^k g + ... + j_{c-1} 2^{k(c-1)} g

     The following entries differ by powers of 2^{kc},

       T[i] = 2^{kc} T[i-2^c]
  */
  const mp_limb_t *pippenger_table;
};

ecc_mod_func ecc_mod;
ecc_mod_func ecc_pp1_redc;
ecc_mod_func ecc_pm1_redc;

ecc_mod_inv_func ecc_mod_inv;

/* Side channel silent. Requires that x < 2m, so checks if x == 0 or x == p */
int
ecc_mod_zero_p (const struct ecc_modulo *m, const mp_limb_t *xp);

/* Requires that a < 2m, and ref < m, needs m->size limbs of scratch
   space. Overlap, a == scratch or ref == scratch, is allowed. */
int
ecc_mod_equal_p (const struct ecc_modulo *m, const mp_limb_t *a,
		 const mp_limb_t *ref, mp_limb_t *scratch);

void
ecc_mod_add (const struct ecc_modulo *m, mp_limb_t *rp,
	     const mp_limb_t *ap, const mp_limb_t *bp);
void
ecc_mod_sub (const struct ecc_modulo *m, mp_limb_t *rp,
	     const mp_limb_t *ap, const mp_limb_t *bp);

void
ecc_mod_mul_1 (const struct ecc_modulo *m, mp_limb_t *rp,
	       const mp_limb_t *ap, const mp_limb_t b);

void
ecc_mod_addmul_1 (const struct ecc_modulo *m, mp_limb_t *rp,
		  const mp_limb_t *ap, mp_limb_t b);
void
ecc_mod_submul_1 (const struct ecc_modulo *m, mp_limb_t *rp,
		  const mp_limb_t *ap, mp_limb_t b);

/* The mul and sqr function need 2*m->size limbs at tp. rp may overlap
   ap or bp, and may equal tp or tp + m->size, but no other overlap
   with tp is allowed. */
void
ecc_mod_mul (const struct ecc_modulo *m, mp_limb_t *rp,
	     const mp_limb_t *ap, const mp_limb_t *bp, mp_limb_t *tp);

void
ecc_mod_sqr (const struct ecc_modulo *m, mp_limb_t *rp,
	     const mp_limb_t *ap, mp_limb_t *tp);

/* These mul and sqr functions produce a canonical result, 0 <= R < M.
   Requirements on input and output areas are similar to the above
   functions, except that it is *not* allowed to pass rp = tp +
   m->size.
 */
void
ecc_mod_mul_canonical (const struct ecc_modulo *m, mp_limb_t *rp,
		       const mp_limb_t *ap, const mp_limb_t *bp, mp_limb_t *tp);

void
ecc_mod_sqr_canonical (const struct ecc_modulo *m, mp_limb_t *rp,
		       const mp_limb_t *ap, mp_limb_t *tp);

/* R <-- X^{2^k} mod M. Needs 2*ecc->size limbs of scratch space, same
   overlap requirements as mul and sqr above. */
void
ecc_mod_pow_2k (const struct ecc_modulo *m,
		mp_limb_t *rp, const mp_limb_t *xp,
		unsigned k, mp_limb_t *tp);

/* R <-- X^{2^k} Y mod M. Similar requirements as ecc_mod_pow_2k, but
   rp and yp can't overlap. */
void
ecc_mod_pow_2k_mul (const struct ecc_modulo *m,
		    mp_limb_t *rp, const mp_limb_t *xp,
		    unsigned k, const mp_limb_t *yp,
		    mp_limb_t *tp);

/* R <-- X^{2^k + 1}. Here, rp and xp must not overlap. */
#define ecc_mod_pow_2kp1(m, rp, xp, k, tp) \
  ecc_mod_pow_2k_mul (m, rp, xp, k, xp, tp)

/* mod q operations. */
void
ecc_mod_random (const struct ecc_modulo *m, mp_limb_t *xp,
		void *ctx, nettle_random_func *random, mp_limb_t *scratch);

void
ecc_hash (const struct ecc_modulo *m,
	  mp_limb_t *hp,
	  size_t length, const uint8_t *digest);

void
gost_hash (const struct ecc_modulo *m,
	  mp_limb_t *hp,
	  size_t length, const uint8_t *digest);

/* Converts a point P in affine coordinates into a point R in jacobian
   coordinates. */
void
ecc_a_to_j (const struct ecc_curve *ecc,
	    mp_limb_t *r, const mp_limb_t *p);

/* Converts a point P in jacobian coordinates into a point R in affine
   coordinates. If op == 1, produce x coordinate only. If op == 2,
   produce the x coordinate only, and also reduce it modulo q. */
void
ecc_j_to_a (const struct ecc_curve *ecc,
	    int op,
	    mp_limb_t *r, const mp_limb_t *p,
	    mp_limb_t *scratch);

/* Converts a point P in homogeneous coordinates on an Edwards curve
   to affine coordinates. Meaning of op is the same as for
   ecc_j_to_a. */
void
ecc_eh_to_a (const struct ecc_curve *ecc,
	     int op,
	     mp_limb_t *r, const mp_limb_t *p,
	     mp_limb_t *scratch);

/* Group operations */

/* Point doubling, with jacobian input and output. Corner cases:
   Correctly sets R = 0 (r_Z = 0) if p = 0 or 2p = 0. */
void
ecc_dup_jj (const struct ecc_curve *ecc,
	    mp_limb_t *r, const mp_limb_t *p,
	    mp_limb_t *scratch);

/* Point addition, with jacobian output, one jacobian input and one
   affine input. Corner cases: Fails for the cases

     P = Q != 0                       Duplication of non-zero point
     P = 0, Q != 0 or P != 0, Q = 0   One input zero

     Correctly gives R = 0 if P = Q = 0 or P = -Q. */
void
ecc_add_jja (const struct ecc_curve *ecc,
	     mp_limb_t *r, const mp_limb_t *p, const mp_limb_t *q,
	     mp_limb_t *scratch);

/* Point addition with Jacobian input and output. */
void
ecc_add_jjj (const struct ecc_curve *ecc,
	     mp_limb_t *r, const mp_limb_t *p, const mp_limb_t *q,
	     mp_limb_t *scratch);

/* Point doubling on a twisted Edwards curve, with homogeneous
   cooordinates. */
void
ecc_dup_eh (const struct ecc_curve *ecc,
	    mp_limb_t *r, const mp_limb_t *p,
	    mp_limb_t *scratch);

void
ecc_add_eh (const struct ecc_curve *ecc,
	    mp_limb_t *r, const mp_limb_t *p, const mp_limb_t *q,
	    mp_limb_t *scratch);

void
ecc_add_ehh (const struct ecc_curve *ecc,
	     mp_limb_t *r, const mp_limb_t *p, const mp_limb_t *q,
	     mp_limb_t *scratch);

void
ecc_dup_th (const struct ecc_curve *ecc,
	    mp_limb_t *r, const mp_limb_t *p,
	    mp_limb_t *scratch);

void
ecc_add_th (const struct ecc_curve *ecc,
	    mp_limb_t *r, const mp_limb_t *p, const mp_limb_t *q,
	    mp_limb_t *scratch);

void
ecc_add_thh (const struct ecc_curve *ecc,
	     mp_limb_t *r, const mp_limb_t *p, const mp_limb_t *q,
	     mp_limb_t *scratch);

/* Computes N * the group generator. N is an array of ecc_size()
   limbs. It must be in the range 0 < N < group order, then R != 0,
   and the algorithm can work without any intermediate values getting
   to zero. */
void
ecc_mul_g (const struct ecc_curve *ecc, mp_limb_t *r,
	   const mp_limb_t *np, mp_limb_t *scratch);

/* Computes N * P. The scalar N is the same as for ecc_mul_g. P is a
   non-zero point on the curve, in affine coordinates. Output R is a
   non-zero point, in Jacobian coordinates. */
void
ecc_mul_a (const struct ecc_curve *ecc,
	   mp_limb_t *r,
	   const mp_limb_t *np, const mp_limb_t *p,
	   mp_limb_t *scratch);

void
ecc_mul_g_eh (const struct ecc_curve *ecc, mp_limb_t *r,
	      const mp_limb_t *np, mp_limb_t *scratch);

void
ecc_mul_a_eh (const struct ecc_curve *ecc,
	      mp_limb_t *r,
	      const mp_limb_t *np, const mp_limb_t *p,
	      mp_limb_t *scratch);

void
ecc_mul_m (const struct ecc_modulo *m,
	   mp_limb_t a24,
	   unsigned bit_low, unsigned bit_high,
	   mp_limb_t *qx, const uint8_t *n, const mp_limb_t *px,
	   mp_limb_t *scratch);

void
cnd_copy (int cnd, mp_limb_t *rp, const mp_limb_t *ap, mp_size_t n);

mp_limb_t
sec_add_1 (mp_limb_t *rp, mp_limb_t *ap, mp_size_t n, mp_limb_t b);

mp_limb_t
sec_sub_1 (mp_limb_t *rp, mp_limb_t *ap, mp_size_t n, mp_limb_t b);

void
sec_tabselect (mp_limb_t *rp, mp_size_t rn,
	       const mp_limb_t *table, unsigned tn,
	       unsigned k);

void
curve25519_eh_to_x (mp_limb_t *xp, const mp_limb_t *p,
		    mp_limb_t *scratch);

void
curve448_eh_to_x (mp_limb_t *xp, const mp_limb_t *p,
		  mp_limb_t *scratch);

/* Current scratch needs: */
#define ECC_MOD_INV_ITCH(size) (3*(size))
#define ECC_J_TO_A_ITCH(size, inv) ((size)+(inv))
#define ECC_EH_TO_A_ITCH(size, inv) ((size)+(inv))
#define ECC_DUP_JJ_ITCH(size) (4*(size))
#define ECC_DUP_EH_ITCH(size) (3*(size))
#define ECC_DUP_TH_ITCH(size) (3*(size))
#define ECC_ADD_JJA_ITCH(size) (5*(size))
#define ECC_ADD_JJJ_ITCH(size) (5*(size))
#define ECC_ADD_EH_ITCH(size) (4*(size))
#define ECC_ADD_EHH_ITCH(size) (4*(size))
#define ECC_ADD_TH_ITCH(size) (4*(size))
#define ECC_ADD_THH_ITCH(size) (4*(size))
#define ECC_MUL_G_ITCH(size) (8*(size))
#define ECC_MUL_G_EH_ITCH(size) (7*(size))
#if ECC_MUL_A_WBITS == 0
#define ECC_MUL_A_ITCH(size) (11*(size))
#else
#define ECC_MUL_A_ITCH(size) \
  (((3 << ECC_MUL_A_WBITS) + 8) * (size))
#endif
#if ECC_MUL_A_EH_WBITS == 0
#define ECC_MUL_A_EH_ITCH(size) (10*(size))
#else
#define ECC_MUL_A_EH_ITCH(size) \
  (((3 << ECC_MUL_A_EH_WBITS) + 7) * (size))
#endif
#define ECC_MUL_M_ITCH(size) (8*(size))
#define ECC_ECDSA_SIGN_ITCH(size) (11*(size))
#define ECC_GOSTDSA_SIGN_ITCH(size) (11*(size))
#define ECC_MOD_RANDOM_ITCH(size) (size)
#define ECC_HASH_ITCH(size) (1+(size))

#endif /* NETTLE_ECC_INTERNAL_H_INCLUDED */
