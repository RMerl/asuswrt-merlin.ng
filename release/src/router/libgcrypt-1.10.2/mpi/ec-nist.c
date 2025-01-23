/* ec-nist.c -  NIST optimized elliptic curve functions
 * Copyright (C) 2021 Jussi Kivilinna <jussi.kivilinna@iki.fi>
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

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>


#ifndef ASM_DISABLED


#include "mpi-internal.h"
#include "longlong.h"
#include "g10lib.h"
#include "context.h"
#include "ec-context.h"
#include "ec-inline.h"


/* These variables are used to generate masks from conditional operation
 * flag parameters.  Use of volatile prevents compiler optimizations from
 * converting AND-masking to conditional branches.  */
static volatile mpi_limb_t vzero = 0;
static volatile mpi_limb_t vone = 1;


static inline
void prefetch(const void *tab, size_t len)
{
  const volatile byte *vtab = tab;

  if (len > 0 * 64)
    (void)vtab[0 * 64];
  if (len > 1 * 64)
    (void)vtab[1 * 64];
  if (len > 2 * 64)
    (void)vtab[2 * 64];
  if (len > 3 * 64)
    (void)vtab[3 * 64];
  if (len > 4 * 64)
    (void)vtab[4 * 64];
  if (len > 5 * 64)
    (void)vtab[5 * 64];
  if (len > 6 * 64)
    (void)vtab[6 * 64];
  if (len > 7 * 64)
    (void)vtab[7 * 64];
  if (len > 8 * 64)
    (void)vtab[8 * 64];
  if (len > 9 * 64)
    (void)vtab[9 * 64];
  if (len > 10 * 64)
    (void)vtab[10 * 64];
  (void)vtab[len - 1];
}


/* Fast reduction routines for NIST curves.  */

void
_gcry_mpi_ec_nist192_mod (gcry_mpi_t w, mpi_ec_t ctx)
{
  static const mpi_limb64_t p_mult[3][4] =
  {
    { /* P * 1 */
      LIMB64_C(0xffffffffU, 0xffffffffU), LIMB64_C(0xffffffffU, 0xfffffffeU),
      LIMB64_C(0xffffffffU, 0xffffffffU), LIMB64_C(0x00000000U, 0x00000000U)
    },
    { /* P * 2 */
      LIMB64_C(0xffffffffU, 0xfffffffeU), LIMB64_C(0xffffffffU, 0xfffffffdU),
      LIMB64_C(0xffffffffU, 0xffffffffU), LIMB64_C(0x00000000U, 0x00000001U)
    },
    { /* P * 3 */
      LIMB64_C(0xffffffffU, 0xfffffffdU), LIMB64_C(0xffffffffU, 0xfffffffcU),
      LIMB64_C(0xffffffffU, 0xffffffffU), LIMB64_C(0x00000000U, 0x00000002U)
    }
  };
  const mpi_limb64_t zero = LIMB_TO64(0);
  mpi_ptr_t wp;
  mpi_size_t wsize = 192 / BITS_PER_MPI_LIMB64;
  mpi_limb64_t s[wsize + 1];
  mpi_limb64_t o[wsize + 1];
  mpi_limb_t mask1;
  mpi_limb_t mask2;
  mpi_limb_t s_is_negative;
  int carry;

  MPN_NORMALIZE (w->d, w->nlimbs);
  if (mpi_nbits_more_than (w, 2 * 192))
    log_bug ("W must be less than m^2\n");

  RESIZE_AND_CLEAR_IF_NEEDED (w, wsize * 2 * LIMBS_PER_LIMB64);
  RESIZE_AND_CLEAR_IF_NEEDED (ctx->p, wsize * LIMBS_PER_LIMB64);

  wp = w->d;

  prefetch (p_mult, sizeof(p_mult));

  /* See "FIPS 186-4, D.2.1 Curve P-192". */

  s[0] = LOAD64(wp, 3);
  ADD3_LIMB64 (s[3],  s[2],          s[1],
	       zero,  zero,          LOAD64(wp, 3),
	       zero,  LOAD64(wp, 4), LOAD64(wp, 4));

  ADD4_LIMB64 (s[3],  s[2],          s[1],          s[0],
	       s[3],  s[2],          s[1],          s[0],
	       zero,  LOAD64(wp, 5), LOAD64(wp, 5), LOAD64(wp, 5));

  ADD4_LIMB64 (s[3],  s[2],          s[1],          s[0],
	       s[3],  s[2],          s[1],          s[0],
	       zero,  LOAD64(wp, 2), LOAD64(wp, 1), LOAD64(wp, 0));

  /* mod p:
   *  's[3]' holds carry value (0..2). Subtract (carry + 1) * p. Result will be
   *  with in range -p...p. Handle result being negative with addition and
   *  conditional store. */

  carry = LO32_LIMB64(s[3]);

  SUB4_LIMB64 (s[3], s[2], s[1], s[0],
	       s[3], s[2], s[1], s[0],
	       p_mult[carry][3], p_mult[carry][2],
	       p_mult[carry][1], p_mult[carry][0]);

  ADD4_LIMB64 (o[3], o[2], o[1], o[0],
	       s[3], s[2], s[1], s[0],
	       zero,
	       p_mult[0][2], p_mult[0][1], p_mult[0][0]);

  s_is_negative = LO32_LIMB64(s[3]) >> 31;

  mask2 = vzero - s_is_negative;
  mask1 = s_is_negative - vone;

  STORE64_COND(wp, 0, mask2, o[0], mask1, s[0]);
  STORE64_COND(wp, 1, mask2, o[1], mask1, s[1]);
  STORE64_COND(wp, 2, mask2, o[2], mask1, s[2]);

  w->nlimbs = 192 / BITS_PER_MPI_LIMB;
  MPN_NORMALIZE (wp, w->nlimbs);
}

void
_gcry_mpi_ec_nist224_mod (gcry_mpi_t w, mpi_ec_t ctx)
{
  static const mpi_limb64_t p_mult[5][4] =
  {
    { /* P * -1 */
      LIMB64_C(0xffffffffU, 0xffffffffU), LIMB64_C(0x00000000U, 0xffffffffU),
      LIMB64_C(0x00000000U, 0x00000000U), LIMB64_C(0xffffffffU, 0x00000000U)
    },
    { /* P * 0 */
      LIMB64_C(0x00000000U, 0x00000000U), LIMB64_C(0x00000000U, 0x00000000U),
      LIMB64_C(0x00000000U, 0x00000000U), LIMB64_C(0x00000000U, 0x00000000U)
    },
    { /* P * 1 */
      LIMB64_C(0x00000000U, 0x00000001U), LIMB64_C(0xffffffffU, 0x00000000U),
      LIMB64_C(0xffffffffU, 0xffffffffU), LIMB64_C(0x00000000U, 0xffffffffU)
    },
    { /* P * 2 */
      LIMB64_C(0x00000000U, 0x00000002U), LIMB64_C(0xfffffffeU, 0x00000000U),
      LIMB64_C(0xffffffffU, 0xffffffffU), LIMB64_C(0x00000001U, 0xffffffffU)
    },
    { /* P * 3 */
      LIMB64_C(0x00000000U, 0x00000003U), LIMB64_C(0xfffffffdU, 0x00000000U),
      LIMB64_C(0xffffffffU, 0xffffffffU), LIMB64_C(0x00000002U, 0xffffffffU)
    }
  };
  const mpi_limb64_t zero = LIMB_TO64(0);
  mpi_ptr_t wp;
  mpi_size_t wsize = (224 + BITS_PER_MPI_LIMB64 - 1) / BITS_PER_MPI_LIMB64;
  mpi_size_t psize = ctx->p->nlimbs;
  mpi_limb64_t s[wsize];
  mpi_limb64_t d[wsize];
  mpi_limb_t mask1;
  mpi_limb_t mask2;
  mpi_limb_t s_is_negative;
  int carry;

  MPN_NORMALIZE (w->d, w->nlimbs);
  if (mpi_nbits_more_than (w, 2 * 224))
    log_bug ("W must be less than m^2\n");

  RESIZE_AND_CLEAR_IF_NEEDED (w, wsize * 2 * LIMBS_PER_LIMB64);
  RESIZE_AND_CLEAR_IF_NEEDED (ctx->p, wsize * LIMBS_PER_LIMB64);
  ctx->p->nlimbs = psize;

  wp = w->d;

  prefetch (p_mult, sizeof(p_mult));

  /* See "FIPS 186-4, D.2.2 Curve P-224". */

  /* "S1 + S2" with 64-bit limbs:
   *     [0:A10]:[ A9: A8]:[ A7:0]:[0:0]
   *  +    [0:0]:[A13:A12]:[A11:0]:[0:0]
   *  => s[3]:s[2]:s[1]:s[0]
   */
  s[0] = zero;
  ADD3_LIMB64 (s[3], s[2], s[1],
	       LIMB64_HILO(0, LOAD32(wp, 10)),
	       LOAD64(wp, 8 / 2),
	       LIMB64_HILO(LOAD32(wp, 7), 0),
	       zero,
	       LOAD64(wp, 12 / 2),
	       LIMB64_HILO(LOAD32(wp, 11), 0));

  /* "T + S1 + S2" */
  ADD4_LIMB64 (s[3], s[2], s[1], s[0],
	       s[3], s[2], s[1], s[0],
	       LIMB64_HILO(0, LOAD32(wp, 6)),
	       LOAD64(wp, 4 / 2),
	       LOAD64(wp, 2 / 2),
	       LOAD64(wp, 0 / 2));

  /* "D1 + D2" with 64-bit limbs:
   *     [0:A13]:[A12:A11]:[A10: A9]:[ A8: A7]
   *  +    [0:0]:[  0:  0]:[  0:A13]:[A12:A11]
   *  => d[3]:d[2]:d[1]:d[0]
   */
  ADD4_LIMB64 (d[3], d[2], d[1], d[0],
	       LIMB64_HILO(0, LOAD32(wp, 13)),
	       LOAD64_UNALIGNED(wp, 11 / 2),
	       LOAD64_UNALIGNED(wp, 9 / 2),
	       LOAD64_UNALIGNED(wp, 7 / 2),
	       zero,
	       zero,
	       LIMB64_HILO(0, LOAD32(wp, 13)),
	       LOAD64_UNALIGNED(wp, 11 / 2));

  /* "T + S1 + S2 - D1 - D2" */
  SUB4_LIMB64 (s[3], s[2], s[1], s[0],
	       s[3], s[2], s[1], s[0],
	       d[3], d[2], d[1], d[0]);

  /* mod p:
   *  Upper 32-bits of 's[3]' holds carry value (-2..2).
   *  Subtract (carry + 1) * p. Result will be with in range -p...p.
   *  Handle result being negative with addition and conditional store. */

  carry = HI32_LIMB64(s[3]);

  SUB4_LIMB64 (s[3], s[2], s[1], s[0],
	       s[3], s[2], s[1], s[0],
	       p_mult[carry + 2][3], p_mult[carry + 2][2],
	       p_mult[carry + 2][1], p_mult[carry + 2][0]);

  ADD4_LIMB64 (d[3], d[2], d[1], d[0],
	       s[3], s[2], s[1], s[0],
	       p_mult[0 + 2][3], p_mult[0 + 2][2],
	       p_mult[0 + 2][1], p_mult[0 + 2][0]);

  s_is_negative = (HI32_LIMB64(s[3]) >> 31);

  mask2 = vzero - s_is_negative;
  mask1 = s_is_negative - vone;

  STORE64_COND(wp, 0, mask2, d[0], mask1, s[0]);
  STORE64_COND(wp, 1, mask2, d[1], mask1, s[1]);
  STORE64_COND(wp, 2, mask2, d[2], mask1, s[2]);
  STORE64_COND(wp, 3, mask2, d[3], mask1, s[3]);

  w->nlimbs = wsize * LIMBS_PER_LIMB64;
  MPN_NORMALIZE (wp, w->nlimbs);
}

void
_gcry_mpi_ec_nist256_mod (gcry_mpi_t w, mpi_ec_t ctx)
{
  static const mpi_limb64_t p_mult[12][5] =
  {
    { /* P * -3 */
      LIMB64_C(0x00000000U, 0x00000003U), LIMB64_C(0xfffffffdU, 0x00000000U),
      LIMB64_C(0xffffffffU, 0xffffffffU), LIMB64_C(0x00000002U, 0xfffffffcU),
      LIMB64_C(0xffffffffU, 0xfffffffdU)
    },
    { /* P * -2 */
      LIMB64_C(0x00000000U, 0x00000002U), LIMB64_C(0xfffffffeU, 0x00000000U),
      LIMB64_C(0xffffffffU, 0xffffffffU), LIMB64_C(0x00000001U, 0xfffffffdU),
      LIMB64_C(0xffffffffU, 0xfffffffeU)
    },
    { /* P * -1 */
      LIMB64_C(0x00000000U, 0x00000001U), LIMB64_C(0xffffffffU, 0x00000000U),
      LIMB64_C(0xffffffffU, 0xffffffffU), LIMB64_C(0x00000000U, 0xfffffffeU),
      LIMB64_C(0xffffffffU, 0xffffffffU)
    },
    { /* P * 0 */
      LIMB64_C(0x00000000U, 0x00000000U), LIMB64_C(0x00000000U, 0x00000000U),
      LIMB64_C(0x00000000U, 0x00000000U), LIMB64_C(0x00000000U, 0x00000000U),
      LIMB64_C(0x00000000U, 0x00000000U)
    },
    { /* P * 1 */
      LIMB64_C(0xffffffffU, 0xffffffffU), LIMB64_C(0x00000000U, 0xffffffffU),
      LIMB64_C(0x00000000U, 0x00000000U), LIMB64_C(0xffffffffU, 0x00000001U),
      LIMB64_C(0x00000000U, 0x00000000U)
    },
    { /* P * 2 */
      LIMB64_C(0xffffffffU, 0xfffffffeU), LIMB64_C(0x00000001U, 0xffffffffU),
      LIMB64_C(0x00000000U, 0x00000000U), LIMB64_C(0xfffffffeU, 0x00000002U),
      LIMB64_C(0x00000000U, 0x00000001U)
    },
    { /* P * 3 */
      LIMB64_C(0xffffffffU, 0xfffffffdU), LIMB64_C(0x00000002U, 0xffffffffU),
      LIMB64_C(0x00000000U, 0x00000000U), LIMB64_C(0xfffffffdU, 0x00000003U),
      LIMB64_C(0x00000000U, 0x00000002U)
    },
    { /* P * 4 */
      LIMB64_C(0xffffffffU, 0xfffffffcU), LIMB64_C(0x00000003U, 0xffffffffU),
      LIMB64_C(0x00000000U, 0x00000000U), LIMB64_C(0xfffffffcU, 0x00000004U),
      LIMB64_C(0x00000000U, 0x00000003U)
    },
    { /* P * 5 */
      LIMB64_C(0xffffffffU, 0xfffffffbU), LIMB64_C(0x00000004U, 0xffffffffU),
      LIMB64_C(0x00000000U, 0x00000000U), LIMB64_C(0xfffffffbU, 0x00000005U),
      LIMB64_C(0x00000000U, 0x00000004U)
    },
    { /* P * 6 */
      LIMB64_C(0xffffffffU, 0xfffffffaU), LIMB64_C(0x00000005U, 0xffffffffU),
      LIMB64_C(0x00000000U, 0x00000000U), LIMB64_C(0xfffffffaU, 0x00000006U),
      LIMB64_C(0x00000000U, 0x00000005U)
    },
    { /* P * 7 */
      LIMB64_C(0xffffffffU, 0xfffffff9U), LIMB64_C(0x00000006U, 0xffffffffU),
      LIMB64_C(0x00000000U, 0x00000000U), LIMB64_C(0xfffffff9U, 0x00000007U),
      LIMB64_C(0x00000000U, 0x00000006U)
    }
  };
  const mpi_limb64_t zero = LIMB_TO64(0);
  mpi_ptr_t wp;
  mpi_size_t wsize = (256 + BITS_PER_MPI_LIMB64 - 1) / BITS_PER_MPI_LIMB64;
  mpi_size_t psize = ctx->p->nlimbs;
  mpi_limb64_t s[wsize + 1];
  mpi_limb64_t t[wsize + 1];
  mpi_limb64_t d[wsize + 1];
  mpi_limb64_t e[wsize + 1];
  mpi_limb_t mask1;
  mpi_limb_t mask2;
  mpi_limb_t mask3;
  mpi_limb_t s_is_negative;
  mpi_limb_t d_is_negative;
  int carry;

  MPN_NORMALIZE (w->d, w->nlimbs);
  if (mpi_nbits_more_than (w, 2 * 256))
    log_bug ("W must be less than m^2\n");

  RESIZE_AND_CLEAR_IF_NEEDED (w, wsize * 2 * LIMBS_PER_LIMB64);
  RESIZE_AND_CLEAR_IF_NEEDED (ctx->p, wsize * LIMBS_PER_LIMB64);
  ctx->p->nlimbs = psize;

  wp = w->d;

  prefetch (p_mult, sizeof(p_mult));

  /* See "FIPS 186-4, D.2.3 Curve P-256". */

  /* "S1 + S2" with 64-bit limbs:
   *     [A15:A14]:[A13:A12]:[A11:0]:[0:0]
   *  +    [0:A15]:[A14:A13]:[A12:0]:[0:0]
   *  => s[4]:s[3]:s[2]:s[1]:s[0]
   */
  s[0] = zero;
  ADD4_LIMB64 (s[4], s[3], s[2], s[1],
	       zero,
	       LOAD64(wp, 14 / 2),
	       LOAD64(wp, 12 / 2),
	       LIMB64_HILO(LOAD32(wp, 11), 0),
	       zero,
	       LIMB64_HILO(0, LOAD32(wp, 15)),
	       LOAD64_UNALIGNED(wp, 13 / 2),
	       LIMB64_HILO(LOAD32(wp, 12), 0));

  /* "S3 + S4" with 64-bit limbs:
   *     [A15:A14]:[  0:  0]:[  0:A10]:[ A9:A8]
   *  +   [A8:A13]:[A15:A14]:[A13:A11]:[A10:A9]
   *  => t[4]:t[3]:t[2]:t[1]:t[0]
   */
  ADD5_LIMB64 (t[4], t[3], t[2], t[1], t[0],
	       zero,
	       LOAD64(wp, 14 / 2),
	       zero,
	       LIMB64_HILO(0, LOAD32(wp, 10)),
	       LOAD64(wp, 8 / 2),
	       zero,
	       LIMB64_HILO(LOAD32(wp, 8), LOAD32(wp, 13)),
	       LOAD64(wp, 14 / 2),
	       LIMB64_HILO(LOAD32(wp, 13), LOAD32(wp, 11)),
	       LOAD64_UNALIGNED(wp, 9 / 2));

  /* "2*S1 + 2*S2" */
  ADD5_LIMB64 (s[4], s[3], s[2], s[1], s[0],
               s[4], s[3], s[2], s[1], s[0],
               s[4], s[3], s[2], s[1], s[0]);

  /* "T + S3 + S4" */
  ADD5_LIMB64 (t[4], t[3], t[2], t[1], t[0],
	       t[4], t[3], t[2], t[1], t[0],
	       zero,
	       LOAD64(wp, 6 / 2),
	       LOAD64(wp, 4 / 2),
	       LOAD64(wp, 2 / 2),
	       LOAD64(wp, 0 / 2));

  /* "2*S1 + 2*S2 - D3" with 64-bit limbs:
   *    s[4]:    s[3]:    s[2]:    s[1]:     s[0]
   *  -       [A12:0]:[A10:A9]:[A8:A15]:[A14:A13]
   *  => s[4]:s[3]:s[2]:s[1]:s[0]
   */
  SUB5_LIMB64 (s[4], s[3], s[2], s[1], s[0],
               s[4], s[3], s[2], s[1], s[0],
	       zero,
	       LIMB64_HILO(LOAD32(wp, 12), 0),
	       LOAD64_UNALIGNED(wp, 9 / 2),
	       LIMB64_HILO(LOAD32(wp, 8), LOAD32(wp, 15)),
	       LOAD64_UNALIGNED(wp, 13 / 2));

  /* "T + 2*S1 + 2*S2 + S3 + S4 - D3" */
  ADD5_LIMB64 (s[4], s[3], s[2], s[1], s[0],
               s[4], s[3], s[2], s[1], s[0],
               t[4], t[3], t[2], t[1], t[0]);

  /* "D1 + D2" with 64-bit limbs:
   *     [0:A13]:[A12:A11] + [A15:A14]:[A13:A12] => d[2]:d[1]:d[0]
   *     [A10:A8] + [A11:A9] => d[4]:d[3]
   */
  ADD3_LIMB64 (d[2], d[1], d[0],
	       zero,
	       LIMB64_HILO(0, LOAD32(wp, 13)),
	       LOAD64_UNALIGNED(wp, 11 / 2),
	       zero,
	       LOAD64(wp, 14 / 2),
	       LOAD64(wp, 12 / 2));
  ADD2_LIMB64 (d[4], d[3],
	       zero, LIMB64_HILO(LOAD32(wp, 10), LOAD32(wp, 8)),
	       zero, LIMB64_HILO(LOAD32(wp, 11), LOAD32(wp, 9)));

  /* "D1 + D2 + D4" with 64-bit limbs:
   *    d[4]:    d[3]:     d[2]:  d[1]:     d[0]
   *  -       [A13:0]:[A11:A10]:[A9:0]:[A15:A14]
   *  => d[4]:d[3]:d[2]:d[1]:d[0]
   */
  ADD5_LIMB64 (d[4], d[3], d[2], d[1], d[0],
               d[4], d[3], d[2], d[1], d[0],
	       zero,
	       LIMB64_HILO(LOAD32(wp, 13), 0),
	       LOAD64(wp, 10 / 2),
	       LIMB64_HILO(LOAD32(wp, 9), 0),
	       LOAD64(wp, 14 / 2));

  /* "T + 2*S1 + 2*S2 + S3 + S4 - D1 - D2 - D3 - D4" */
  SUB5_LIMB64 (s[4], s[3], s[2], s[1], s[0],
               s[4], s[3], s[2], s[1], s[0],
               d[4], d[3], d[2], d[1], d[0]);

  /* mod p:
   *  's[4]' holds carry value (-4..6). Subtract (carry + 1) * p. Result
   *  will be with in range -2*p...p. Handle result being negative with
   *  addition and conditional store. */

  carry = LO32_LIMB64(s[4]);

  SUB5_LIMB64 (s[4], s[3], s[2], s[1], s[0],
	       s[4], s[3], s[2], s[1], s[0],
	       p_mult[carry + 4][4], p_mult[carry + 4][3],
	       p_mult[carry + 4][2], p_mult[carry + 4][1],
	       p_mult[carry + 4][0]);

  /* Add 1*P */
  ADD5_LIMB64 (d[4], d[3], d[2], d[1], d[0],
	       s[4], s[3], s[2], s[1], s[0],
	       zero,
	       p_mult[0 + 4][3], p_mult[0 + 4][2],
	       p_mult[0 + 4][1], p_mult[0 + 4][0]);

  /* Add 2*P */
  ADD5_LIMB64 (e[4], e[3], e[2], e[1], e[0],
	       s[4], s[3], s[2], s[1], s[0],
	       zero,
	       p_mult[1 + 4][3], p_mult[1 + 4][2],
	       p_mult[1 + 4][1], p_mult[1 + 4][0]);

  s_is_negative = LO32_LIMB64(s[4]) >> 31;
  d_is_negative = LO32_LIMB64(d[4]) >> 31;
  mask3 = vzero - d_is_negative;
  mask2 = (vzero - s_is_negative) & ~mask3;
  mask1 = (s_is_negative - vone) & ~mask3;

  s[0] = LIMB_OR64(MASK_AND64(mask2, d[0]), MASK_AND64(mask1, s[0]));
  s[1] = LIMB_OR64(MASK_AND64(mask2, d[1]), MASK_AND64(mask1, s[1]));
  s[2] = LIMB_OR64(MASK_AND64(mask2, d[2]), MASK_AND64(mask1, s[2]));
  s[3] = LIMB_OR64(MASK_AND64(mask2, d[3]), MASK_AND64(mask1, s[3]));
  s[0] = LIMB_OR64(MASK_AND64(mask3, e[0]), s[0]);
  s[1] = LIMB_OR64(MASK_AND64(mask3, e[1]), s[1]);
  s[2] = LIMB_OR64(MASK_AND64(mask3, e[2]), s[2]);
  s[3] = LIMB_OR64(MASK_AND64(mask3, e[3]), s[3]);

  STORE64(wp, 0, s[0]);
  STORE64(wp, 1, s[1]);
  STORE64(wp, 2, s[2]);
  STORE64(wp, 3, s[3]);

  w->nlimbs = wsize * LIMBS_PER_LIMB64;
  MPN_NORMALIZE (wp, w->nlimbs);
}

void
_gcry_mpi_ec_nist384_mod (gcry_mpi_t w, mpi_ec_t ctx)
{
  static const mpi_limb64_t p_mult[11][7] =
  {
    { /* P * -2 */
      LIMB64_C(0xfffffffeU, 0x00000002U), LIMB64_C(0x00000001U, 0xffffffffU),
      LIMB64_C(0x00000000U, 0x00000002U), LIMB64_C(0x00000000U, 0x00000000U),
      LIMB64_C(0x00000000U, 0x00000000U), LIMB64_C(0x00000000U, 0x00000000U),
      LIMB64_C(0xffffffffU, 0xfffffffeU)
    },
    { /* P * -1 */
      LIMB64_C(0xffffffffU, 0x00000001U), LIMB64_C(0x00000000U, 0xffffffffU),
      LIMB64_C(0x00000000U, 0x00000001U), LIMB64_C(0x00000000U, 0x00000000U),
      LIMB64_C(0x00000000U, 0x00000000U), LIMB64_C(0x00000000U, 0x00000000U),
      LIMB64_C(0xffffffffU, 0xffffffffU)
    },
    { /* P * 0 */
      LIMB64_C(0x00000000U, 0x00000000U), LIMB64_C(0x00000000U, 0x00000000U),
      LIMB64_C(0x00000000U, 0x00000000U), LIMB64_C(0x00000000U, 0x00000000U),
      LIMB64_C(0x00000000U, 0x00000000U), LIMB64_C(0x00000000U, 0x00000000U),
      LIMB64_C(0x00000000U, 0x00000000U)
    },
    { /* P * 1 */
      LIMB64_C(0x00000000U, 0xffffffffU), LIMB64_C(0xffffffffU, 0x00000000U),
      LIMB64_C(0xffffffffU, 0xfffffffeU), LIMB64_C(0xffffffffU, 0xffffffffU),
      LIMB64_C(0xffffffffU, 0xffffffffU), LIMB64_C(0xffffffffU, 0xffffffffU),
      LIMB64_C(0x00000000U, 0x00000000U)
    },
    { /* P * 2 */
      LIMB64_C(0x00000001U, 0xfffffffeU), LIMB64_C(0xfffffffeU, 0x00000000U),
      LIMB64_C(0xffffffffU, 0xfffffffdU), LIMB64_C(0xffffffffU, 0xffffffffU),
      LIMB64_C(0xffffffffU, 0xffffffffU), LIMB64_C(0xffffffffU, 0xffffffffU),
      LIMB64_C(0x00000000U, 0x00000001U)
    },
    { /* P * 3 */
      LIMB64_C(0x00000002U, 0xfffffffdU), LIMB64_C(0xfffffffdU, 0x00000000U),
      LIMB64_C(0xffffffffU, 0xfffffffcU), LIMB64_C(0xffffffffU, 0xffffffffU),
      LIMB64_C(0xffffffffU, 0xffffffffU), LIMB64_C(0xffffffffU, 0xffffffffU),
      LIMB64_C(0x00000000U, 0x00000002U)
    },
    { /* P * 4 */
      LIMB64_C(0x00000003U, 0xfffffffcU), LIMB64_C(0xfffffffcU, 0x00000000U),
      LIMB64_C(0xffffffffU, 0xfffffffbU), LIMB64_C(0xffffffffU, 0xffffffffU),
      LIMB64_C(0xffffffffU, 0xffffffffU), LIMB64_C(0xffffffffU, 0xffffffffU),
      LIMB64_C(0x00000000U, 0x00000003U)
    },
    { /* P * 5 */
      LIMB64_C(0x00000004U, 0xfffffffbU), LIMB64_C(0xfffffffbU, 0x00000000U),
      LIMB64_C(0xffffffffU, 0xfffffffaU), LIMB64_C(0xffffffffU, 0xffffffffU),
      LIMB64_C(0xffffffffU, 0xffffffffU), LIMB64_C(0xffffffffU, 0xffffffffU),
      LIMB64_C(0x00000000U, 0x00000004U)
    },
    { /* P * 6 */
      LIMB64_C(0x00000005U, 0xfffffffaU), LIMB64_C(0xfffffffaU, 0x00000000U),
      LIMB64_C(0xffffffffU, 0xfffffff9U), LIMB64_C(0xffffffffU, 0xffffffffU),
      LIMB64_C(0xffffffffU, 0xffffffffU), LIMB64_C(0xffffffffU, 0xffffffffU),
      LIMB64_C(0x00000000U, 0x00000005U)
    },
    { /* P * 7 */
      LIMB64_C(0x00000006U, 0xfffffff9U), LIMB64_C(0xfffffff9U, 0x00000000U),
      LIMB64_C(0xffffffffU, 0xfffffff8U), LIMB64_C(0xffffffffU, 0xffffffffU),
      LIMB64_C(0xffffffffU, 0xffffffffU), LIMB64_C(0xffffffffU, 0xffffffffU),
      LIMB64_C(0x00000000U, 0x00000006U)
    },
    { /* P * 8 */
      LIMB64_C(0x00000007U, 0xfffffff8U), LIMB64_C(0xfffffff8U, 0x00000000U),
      LIMB64_C(0xffffffffU, 0xfffffff7U), LIMB64_C(0xffffffffU, 0xffffffffU),
      LIMB64_C(0xffffffffU, 0xffffffffU), LIMB64_C(0xffffffffU, 0xffffffffU),
      LIMB64_C(0x00000000U, 0x00000007U)
    },
  };
  const mpi_limb64_t zero = LIMB_TO64(0);
  mpi_ptr_t wp;
  mpi_size_t wsize = (384 + BITS_PER_MPI_LIMB64 - 1) / BITS_PER_MPI_LIMB64;
  mpi_size_t psize = ctx->p->nlimbs;
#if (BITS_PER_MPI_LIMB64 == BITS_PER_MPI_LIMB) && defined(WORDS_BIGENDIAN)
  mpi_limb_t wp_shr32[wsize * LIMBS_PER_LIMB64];
#endif
  mpi_limb64_t s[wsize + 1];
  mpi_limb64_t t[wsize + 1];
  mpi_limb64_t d[wsize + 1];
  mpi_limb64_t x[wsize + 1];
  mpi_limb_t mask1;
  mpi_limb_t mask2;
  mpi_limb_t s_is_negative;
  int carry;

  MPN_NORMALIZE (w->d, w->nlimbs);
  if (mpi_nbits_more_than (w, 2 * 384))
    log_bug ("W must be less than m^2\n");

  RESIZE_AND_CLEAR_IF_NEEDED (w, wsize * 2 * LIMBS_PER_LIMB64);
  RESIZE_AND_CLEAR_IF_NEEDED (ctx->p, wsize * LIMBS_PER_LIMB64);
  ctx->p->nlimbs = psize;

  wp = w->d;

  prefetch (p_mult, sizeof(p_mult));

  /* See "FIPS 186-4, D.2.4 Curve P-384". */

#if BITS_PER_MPI_LIMB64 == BITS_PER_MPI_LIMB
# ifdef WORDS_BIGENDIAN
#  define LOAD64_SHR32(idx) LOAD64(wp_shr32, ((idx) / 2 - wsize))
  _gcry_mpih_rshift (wp_shr32, wp + 384 / BITS_PER_MPI_LIMB,
		     wsize * LIMBS_PER_LIMB64, 32);
# else
# define LOAD64_SHR32(idx) LOAD64_UNALIGNED(wp, idx / 2)
#endif
#else
# define LOAD64_SHR32(idx) LIMB64_HILO(LOAD32(wp, (idx) + 1), LOAD32(wp, idx))
#endif

  /* "S1 + S1" with 64-bit limbs:
   *     [0:A23]:[A22:A21]
   *  +  [0:A23]:[A22:A21]
   *  => s[3]:s[2]
   */
  ADD2_LIMB64 (s[3], s[2],
	       LIMB64_HILO(0, LOAD32(wp, 23)),
	       LOAD64_SHR32(21),
	       LIMB64_HILO(0, LOAD32(wp, 23)),
	       LOAD64_SHR32(21));

  /* "S5 + S6" with 64-bit limbs:
   *     [A23:A22]:[A21:A20]:[  0:0]:[0:  0]
   *  +  [  0:  0]:[A23:A22]:[A21:0]:[0:A20]
   *  => x[4]:x[3]:x[2]:x[1]:x[0]
   */
  x[0] = LIMB64_HILO(0, LOAD32(wp, 20));
  x[1] = LIMB64_HILO(LOAD32(wp, 21), 0);
  ADD3_LIMB64 (x[4], x[3], x[2],
	       zero, LOAD64(wp, 22 / 2), LOAD64(wp, 20 / 2),
	       zero, zero, LOAD64(wp, 22 / 2));

  /* "D2 + D3" with 64-bit limbs:
   *     [0:A23]:[A22:A21]:[A20:0]
   *  +  [0:A23]:[A23:0]:[0:0]
   *  => d[2]:d[1]:d[0]
   */
  d[0] = LIMB64_HILO(LOAD32(wp, 20), 0);
  ADD2_LIMB64 (d[2], d[1],
	       LIMB64_HILO(0, LOAD32(wp, 23)),
	       LOAD64_SHR32(21),
	       LIMB64_HILO(0, LOAD32(wp, 23)),
	       LIMB64_HILO(LOAD32(wp, 23), 0));

  /* "2*S1 + S5 + S6" with 64-bit limbs:
   *     s[4]:s[3]:s[2]:s[1]:s[0]
   *  +  x[4]:x[3]:x[2]:x[1]:x[0]
   *  => s[4]:s[3]:s[2]:s[1]:s[0]
   */
  s[0] = x[0];
  s[1] = x[1];
  ADD3_LIMB64(s[4], s[3], s[2],
	      zero, s[3], s[2],
	      x[4], x[3], x[2]);

  /* "T + S2" with 64-bit limbs:
   *     [A11:A10]:[ A9: A8]:[ A7: A6]:[ A5: A4]:[ A3: A2]:[ A1: A0]
   *  +  [A23:A22]:[A21:A20]:[A19:A18]:[A17:A16]:[A15:A14]:[A13:A12]
   *  => t[6]:t[5]:t[4]:t[3]:t[2]:t[1]:t[0]
   */
  ADD7_LIMB64 (t[6], t[5], t[4], t[3], t[2], t[1], t[0],
	       zero,
	       LOAD64(wp, 10 / 2), LOAD64(wp, 8 / 2), LOAD64(wp, 6 / 2),
	       LOAD64(wp, 4 / 2), LOAD64(wp, 2 / 2), LOAD64(wp, 0 / 2),
	       zero,
	       LOAD64(wp, 22 / 2), LOAD64(wp, 20 / 2), LOAD64(wp, 18 / 2),
	       LOAD64(wp, 16 / 2), LOAD64(wp, 14 / 2), LOAD64(wp, 12 / 2));

  /* "2*S1 + S4 + S5 + S6" with 64-bit limbs:
   *     s[6]:     s[5]:     s[4]:     s[3]:     s[2]:   s[1]:   s[0]
   *  +       [A19:A18]:[A17:A16]:[A15:A14]:[A13:A12]:[A20:0]:[A23:0]
   *  => s[6]:s[5]:s[4]:s[3]:s[2]:s[1]:s[0]
   */
  ADD7_LIMB64 (s[6], s[5], s[4], s[3], s[2], s[1], s[0],
	       zero, zero, s[4], s[3], s[2], s[1], s[0],
	       zero,
	       LOAD64(wp, 18 / 2), LOAD64(wp, 16 / 2),
	       LOAD64(wp, 14 / 2), LOAD64(wp, 12 / 2),
	       LIMB64_HILO(LOAD32(wp, 20), 0),
	       LIMB64_HILO(LOAD32(wp, 23), 0));

  /* "D1 + D2 + D3" with 64-bit limbs:
   *     d[6]:     d[5]:     d[4]:     d[3]:     d[2]:     d[1]:     d[0]
   *  +       [A22:A21]:[A20:A19]:[A18:A17]:[A16:A15]:[A14:A13]:[A12:A23]
   *  => d[6]:d[5]:d[4]:d[3]:d[2]:d[1]:d[0]
   */
  ADD7_LIMB64 (d[6], d[5], d[4], d[3], d[2], d[1], d[0],
	       zero, zero, zero, zero, d[2], d[1], d[0],
	       zero,
	       LOAD64_SHR32(21),
	       LOAD64_SHR32(19),
	       LOAD64_SHR32(17),
	       LOAD64_SHR32(15),
	       LOAD64_SHR32(13),
	       LIMB64_HILO(LOAD32(wp, 12), LOAD32(wp, 23)));

  /* "2*S1 + S3 + S4 + S5 + S6" with 64-bit limbs:
   *     s[6]:     s[5]:     s[4]:     s[3]:     s[2]:     s[1]:     s[0]
   *  +       [A20:A19]:[A18:A17]:[A16:A15]:[A14:A13]:[A12:A23]:[A22:A21]
   *  => s[6]:s[5]:s[4]:s[3]:s[2]:s[1]:s[0]
   */
  ADD7_LIMB64 (s[6], s[5], s[4], s[3], s[2], s[1], s[0],
	       s[6], s[5], s[4], s[3], s[2], s[1], s[0],
	       zero,
	       LOAD64_SHR32(19),
	       LOAD64_SHR32(17),
	       LOAD64_SHR32(15),
	       LOAD64_SHR32(13),
	       LIMB64_HILO(LOAD32(wp, 12), LOAD32(wp, 23)),
	       LOAD64_SHR32(21));

  /* "T + 2*S1 + S2 + S3 + S4 + S5 + S6" */
  ADD7_LIMB64 (s[6], s[5], s[4], s[3], s[2], s[1], s[0],
               s[6], s[5], s[4], s[3], s[2], s[1], s[0],
               t[6], t[5], t[4], t[3], t[2], t[1], t[0]);

  /* "T + 2*S1 + S2 + S3 + S4 + S5 + S6 - D1 - D2 - D3" */
  SUB7_LIMB64 (s[6], s[5], s[4], s[3], s[2], s[1], s[0],
               s[6], s[5], s[4], s[3], s[2], s[1], s[0],
               d[6], d[5], d[4], d[3], d[2], d[1], d[0]);

#undef LOAD64_SHR32

  /* mod p:
   *  's[6]' holds carry value (-3..7). Subtract (carry + 1) * p. Result
   *  will be with in range -p...p. Handle result being negative with
   *  addition and conditional store. */

  carry = LO32_LIMB64(s[6]);

  SUB7_LIMB64 (s[6], s[5], s[4], s[3], s[2], s[1], s[0],
	       s[6], s[5], s[4], s[3], s[2], s[1], s[0],
	       p_mult[carry + 3][6], p_mult[carry + 3][5],
	       p_mult[carry + 3][4], p_mult[carry + 3][3],
	       p_mult[carry + 3][2], p_mult[carry + 3][1],
	       p_mult[carry + 3][0]);

  ADD7_LIMB64 (d[6], d[5], d[4], d[3], d[2], d[1], d[0],
	       s[6], s[5], s[4], s[3], s[2], s[1], s[0],
	       zero,
	       p_mult[0 + 3][5], p_mult[0 + 3][4],
	       p_mult[0 + 3][3], p_mult[0 + 3][2],
	       p_mult[0 + 3][1], p_mult[0 + 3][0]);

  s_is_negative = LO32_LIMB64(s[6]) >> 31;
  mask2 = vzero - s_is_negative;
  mask1 = s_is_negative - vone;

  STORE64_COND(wp, 0, mask2, d[0], mask1, s[0]);
  STORE64_COND(wp, 1, mask2, d[1], mask1, s[1]);
  STORE64_COND(wp, 2, mask2, d[2], mask1, s[2]);
  STORE64_COND(wp, 3, mask2, d[3], mask1, s[3]);
  STORE64_COND(wp, 4, mask2, d[4], mask1, s[4]);
  STORE64_COND(wp, 5, mask2, d[5], mask1, s[5]);

  w->nlimbs = wsize * LIMBS_PER_LIMB64;
  MPN_NORMALIZE (wp, w->nlimbs);

#if (BITS_PER_MPI_LIMB64 == BITS_PER_MPI_LIMB) && defined(WORDS_BIGENDIAN)
  wipememory(wp_shr32, sizeof(wp_shr32));
#endif
}

void
_gcry_mpi_ec_nist521_mod (gcry_mpi_t w, mpi_ec_t ctx)
{
  mpi_size_t wsize = (521 + BITS_PER_MPI_LIMB - 1) / BITS_PER_MPI_LIMB;
  mpi_limb_t s[wsize];
  mpi_limb_t cy;
  mpi_ptr_t wp;

  MPN_NORMALIZE (w->d, w->nlimbs);
  if (mpi_nbits_more_than (w, 2 * 521))
    log_bug ("W must be less than m^2\n");

  RESIZE_AND_CLEAR_IF_NEEDED (w, wsize * 2);

  wp = w->d;

  /* See "FIPS 186-4, D.2.5 Curve P-521". */

  _gcry_mpih_rshift (s, wp + wsize - 1, wsize, 521 % BITS_PER_MPI_LIMB);
  s[wsize - 1] &= (1 << (521 % BITS_PER_MPI_LIMB)) - 1;
  wp[wsize - 1] &= (1 << (521 % BITS_PER_MPI_LIMB)) - 1;
  _gcry_mpih_add_n (wp, wp, s, wsize);

  /* "mod p" */
  cy = _gcry_mpih_sub_n (wp, wp, ctx->p->d, wsize);
  _gcry_mpih_add_n (s, wp, ctx->p->d, wsize);
  mpih_set_cond (wp, s, wsize, (cy != 0UL));

  w->nlimbs = wsize;
  MPN_NORMALIZE (wp, w->nlimbs);
}

#endif /* !ASM_DISABLED */
