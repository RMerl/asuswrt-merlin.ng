/* ec.c -  Elliptic Curve functions
 * Copyright (C) 2007 Free Software Foundation, Inc.
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

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "mpi-internal.h"
#include "longlong.h"
#include "g10lib.h"
#include "context.h"
#include "ec-context.h"
#include "ec-internal.h"

extern void reverse_buffer (unsigned char *buffer, unsigned int length);

#define point_init(a)  _gcry_mpi_point_init ((a))
#define point_free(a)  _gcry_mpi_point_free_parts ((a))


/* Print a point using the log functions.  If CTX is not NULL affine
   coordinates will be printed.  */
void
_gcry_mpi_point_log (const char *name, mpi_point_t point, mpi_ec_t ctx)
{
  gcry_mpi_t x, y;
  char buf[100];

  if (!point)
    {
      snprintf (buf, sizeof buf - 1, "%s.*", name);
      log_mpidump (buf, NULL);
      return;
    }
  snprintf (buf, sizeof buf - 1, "%s.X", name);

  if (ctx)
    {
      x = mpi_new (0);
      y = mpi_new (0);
    }
  if (!ctx || _gcry_mpi_ec_get_affine (x, y, point, ctx))
    {
      log_mpidump (buf, point->x);
      buf[strlen(buf)-1] = 'Y';
      log_mpidump (buf, point->y);
      buf[strlen(buf)-1] = 'Z';
      log_mpidump (buf, point->z);
    }
  else
    {
      buf[strlen(buf)-1] = 'x';
      log_mpidump (buf, x);
      buf[strlen(buf)-1] = 'y';
      log_mpidump (buf, y);

    }
  if (ctx)
    {
      _gcry_mpi_release (x);
      _gcry_mpi_release (y);
    }
}


/* Create a new point option.  NBITS gives the size in bits of one
   coordinate; it is only used to pre-allocate some resources and
   might also be passed as 0 to use a default value.  */
mpi_point_t
_gcry_mpi_point_new (unsigned int nbits)
{
  mpi_point_t p;

  (void)nbits;  /* Currently not used.  */

  p = xmalloc (sizeof *p);
  _gcry_mpi_point_init (p);
  return p;
}


/* Release the point object P.  P may be NULL. */
void
_gcry_mpi_point_release (mpi_point_t p)
{
  if (p)
    {
      _gcry_mpi_point_free_parts (p);
      xfree (p);
    }
}


/* Initialize the fields of a point object.  gcry_mpi_point_free_parts
   may be used to release the fields.  */
void
_gcry_mpi_point_init (mpi_point_t p)
{
  p->x = mpi_new (0);
  p->y = mpi_new (0);
  p->z = mpi_new (0);
}


/* Release the parts of a point object. */
void
_gcry_mpi_point_free_parts (mpi_point_t p)
{
  mpi_free (p->x); p->x = NULL;
  mpi_free (p->y); p->y = NULL;
  mpi_free (p->z); p->z = NULL;
}


/* Set the value from S into D.  */
static void
point_set (mpi_point_t d, mpi_point_t s)
{
  mpi_set (d->x, s->x);
  mpi_set (d->y, s->y);
  mpi_set (d->z, s->z);
}


/* Return a copy of POINT. */
gcry_mpi_point_t
_gcry_mpi_point_copy (gcry_mpi_point_t point)
{
  mpi_point_t newpoint;

  newpoint = _gcry_mpi_point_new (0);
  if (point)
    point_set (newpoint, point);

  return newpoint;
}


void
_gcry_mpi_ec_point_resize (mpi_point_t p, mpi_ec_t ctx)
{
  size_t nlimbs = ctx->p->nlimbs;

  mpi_resize (p->x, nlimbs);
  p->x->nlimbs = nlimbs;
  mpi_resize (p->z, nlimbs);
  p->z->nlimbs = nlimbs;

  if (ctx->model != MPI_EC_MONTGOMERY)
    {
      mpi_resize (p->y, nlimbs);
      p->y->nlimbs = nlimbs;
    }
}


static void
point_swap_cond (mpi_point_t d, mpi_point_t s, unsigned long swap,
                 mpi_ec_t ctx)
{
  mpi_swap_cond (d->x, s->x, swap);
  if (ctx->model != MPI_EC_MONTGOMERY)
    mpi_swap_cond (d->y, s->y, swap);
  mpi_swap_cond (d->z, s->z, swap);
}


/* Set the projective coordinates from POINT into X, Y, and Z.  If a
   coordinate is not required, X, Y, or Z may be passed as NULL.  */
void
_gcry_mpi_point_get (gcry_mpi_t x, gcry_mpi_t y, gcry_mpi_t z,
                     mpi_point_t point)
{
  if (x)
    mpi_set (x, point->x);
  if (y)
    mpi_set (y, point->y);
  if (z)
    mpi_set (z, point->z);
}


/* Set the projective coordinates from POINT into X, Y, and Z and
   release POINT.  If a coordinate is not required, X, Y, or Z may be
   passed as NULL.  */
void
_gcry_mpi_point_snatch_get (gcry_mpi_t x, gcry_mpi_t y, gcry_mpi_t z,
                            mpi_point_t point)
{
  mpi_snatch (x, point->x);
  mpi_snatch (y, point->y);
  mpi_snatch (z, point->z);
  xfree (point);
}


/* Set the projective coordinates from X, Y, and Z into POINT.  If a
   coordinate is given as NULL, the value 0 is stored into point.  If
   POINT is given as NULL a new point object is allocated.  Returns
   POINT or the newly allocated point object. */
mpi_point_t
_gcry_mpi_point_set (mpi_point_t point,
                     gcry_mpi_t x, gcry_mpi_t y, gcry_mpi_t z)
{
  if (!point)
    point = mpi_point_new (0);

  if (x)
    mpi_set (point->x, x);
  else
    mpi_clear (point->x);
  if (y)
    mpi_set (point->y, y);
  else
    mpi_clear (point->y);
  if (z)
    mpi_set (point->z, z);
  else
    mpi_clear (point->z);

  return point;
}


/* Set the projective coordinates from X, Y, and Z into POINT.  If a
   coordinate is given as NULL, the value 0 is stored into point.  If
   POINT is given as NULL a new point object is allocated.  The
   coordinates X, Y, and Z are released.  Returns POINT or the newly
   allocated point object. */
mpi_point_t
_gcry_mpi_point_snatch_set (mpi_point_t point,
                            gcry_mpi_t x, gcry_mpi_t y, gcry_mpi_t z)
{
  if (!point)
    point = mpi_point_new (0);

  if (x)
    mpi_snatch (point->x, x);
  else
    mpi_clear (point->x);
  if (y)
    mpi_snatch (point->y, y);
  else
    mpi_clear (point->y);
  if (z)
    mpi_snatch (point->z, z);
  else
    mpi_clear (point->z);

  return point;
}


/* W = W mod P.  */
static void
ec_mod (gcry_mpi_t w, mpi_ec_t ec)
{
  if (0 && ec->dialect == ECC_DIALECT_ED25519)
    _gcry_mpi_ec_ed25519_mod (w);
  else if (ec->t.p_barrett)
    _gcry_mpi_mod_barrett (w, w, ec->t.p_barrett);
  else
    _gcry_mpi_mod (w, w, ec->p);
}

static void
ec_addm (gcry_mpi_t w, gcry_mpi_t u, gcry_mpi_t v, mpi_ec_t ctx)
{
  mpi_add (w, u, v);
  ctx->mod (w, ctx);
}

static void
ec_subm (gcry_mpi_t w, gcry_mpi_t u, gcry_mpi_t v, mpi_ec_t ec)
{
  mpi_sub (w, u, v);
  while (w->sign)
    mpi_add (w, w, ec->p);
  /*ctx->mod (w, ec);*/
}

static void
ec_mulm (gcry_mpi_t w, gcry_mpi_t u, gcry_mpi_t v, mpi_ec_t ctx)
{
  mpi_mul (w, u, v);
  ctx->mod (w, ctx);
}

/* W = 2 * U mod P.  */
static void
ec_mul2 (gcry_mpi_t w, gcry_mpi_t u, mpi_ec_t ctx)
{
  mpi_lshift (w, u, 1);
  ctx->mod (w, ctx);
}

static void
ec_powm (gcry_mpi_t w, const gcry_mpi_t b, const gcry_mpi_t e,
         mpi_ec_t ctx)
{
  mpi_powm (w, b, e, ctx->p);
  /* _gcry_mpi_abs (w); */
}


/* Shortcut for
     ec_powm (B, B, mpi_const (MPI_C_TWO), ctx);
   for easier optimization.  */
static void
ec_pow2 (gcry_mpi_t w, const gcry_mpi_t b, mpi_ec_t ctx)
{
  /* Using mpi_mul is slightly faster (at least on amd64).  */
  /* mpi_powm (w, b, mpi_const (MPI_C_TWO), ctx->p); */
  ec_mulm (w, b, b, ctx);
}


/* Shortcut for
     ec_powm (B, B, mpi_const (MPI_C_THREE), ctx);
   for easier optimization.  */
static void
ec_pow3 (gcry_mpi_t w, const gcry_mpi_t b, mpi_ec_t ctx)
{
  mpi_powm (w, b, mpi_const (MPI_C_THREE), ctx->p);
}


static void
ec_invm (gcry_mpi_t x, gcry_mpi_t a, mpi_ec_t ctx)
{
  if (!mpi_invm (x, a, ctx->p))
    {
      log_error ("ec_invm: inverse does not exist:\n");
      log_mpidump ("  a", a);
      log_mpidump ("  p", ctx->p);
    }
}

/* Routines for 2^255 - 19.  */

#define LIMB_SIZE_25519 ((256+BITS_PER_MPI_LIMB-1)/BITS_PER_MPI_LIMB)

static void
ec_addm_25519 (gcry_mpi_t w, gcry_mpi_t u, gcry_mpi_t v, mpi_ec_t ctx)
{
  mpi_ptr_t wp, up, vp;
  mpi_size_t wsize = LIMB_SIZE_25519;
  mpi_limb_t n[LIMB_SIZE_25519];
  mpi_limb_t borrow;

  if (w->nlimbs != wsize || u->nlimbs != wsize || v->nlimbs != wsize)
    log_bug ("addm_25519: different sizes\n");

  up = u->d;
  vp = v->d;
  wp = w->d;

  _gcry_mpih_add_n (wp, up, vp, wsize);
  borrow = _gcry_mpih_sub_n (n, wp, ctx->p->d, wsize);
  mpih_set_cond (wp, n, wsize, (borrow == 0UL));
  wp[LIMB_SIZE_25519-1] &= ~((mpi_limb_t)1 << (255 % BITS_PER_MPI_LIMB));
}

static void
ec_subm_25519 (gcry_mpi_t w, gcry_mpi_t u, gcry_mpi_t v, mpi_ec_t ctx)
{
  mpi_ptr_t wp, up, vp;
  mpi_size_t wsize = LIMB_SIZE_25519;
  mpi_limb_t n[LIMB_SIZE_25519];
  mpi_limb_t borrow;

  if (w->nlimbs != wsize || u->nlimbs != wsize || v->nlimbs != wsize)
    log_bug ("subm_25519: different sizes\n");

  up = u->d;
  vp = v->d;
  wp = w->d;

  borrow = _gcry_mpih_sub_n (wp, up, vp, wsize);
  _gcry_mpih_add_n (n, wp, ctx->p->d, wsize);
  mpih_set_cond (wp, n, wsize, (borrow != 0UL));
  wp[LIMB_SIZE_25519-1] &= ~((mpi_limb_t)1 << (255 % BITS_PER_MPI_LIMB));
}

static void
ec_mulm_25519 (gcry_mpi_t w, gcry_mpi_t u, gcry_mpi_t v, mpi_ec_t ctx)
{
  mpi_ptr_t wp, up, vp;
  mpi_size_t wsize = LIMB_SIZE_25519;
  mpi_limb_t n[LIMB_SIZE_25519*2];
  mpi_limb_t cy;
  int msb;

  (void)ctx;
  if (w->nlimbs != wsize || u->nlimbs != wsize || v->nlimbs != wsize)
    log_bug ("mulm_25519: different sizes\n");

  up = u->d;
  vp = v->d;
  wp = w->d;

  _gcry_mpih_mul_n (n, up, vp, wsize);
  memcpy (wp, n, wsize * BYTES_PER_MPI_LIMB);
  wp[LIMB_SIZE_25519-1] &= ~((mpi_limb_t)1 << (255 % BITS_PER_MPI_LIMB));

  _gcry_mpih_rshift (n, n+LIMB_SIZE_25519-1, LIMB_SIZE_25519+1,
		     (255 % BITS_PER_MPI_LIMB));

  cy = _gcry_mpih_addmul_1 (wp, n, wsize, 19);

  memset (n, 0, wsize * BYTES_PER_MPI_LIMB);
  msb = (wp[LIMB_SIZE_25519-1] >> (255 % BITS_PER_MPI_LIMB));
  n[0] = (cy * 2 + msb) * 19;
  wp[LIMB_SIZE_25519-1] &= ~((mpi_limb_t)1 << (255 % BITS_PER_MPI_LIMB));
  _gcry_mpih_add_n (wp, wp, n, wsize);

  cy = _gcry_mpih_sub_n (n, wp, ctx->p->d, wsize);
  mpih_set_cond (wp, n, wsize, (cy == 0UL));
}

static void
ec_mul2_25519 (gcry_mpi_t w, gcry_mpi_t u, mpi_ec_t ctx)
{
  ec_addm_25519 (w, u, u, ctx);
}

static void
ec_pow2_25519 (gcry_mpi_t w, const gcry_mpi_t b, mpi_ec_t ctx)
{
  ec_mulm_25519 (w, b, b, ctx);
}

/* Routines for 2^448 - 2^224 - 1.  */

#define LIMB_SIZE_448 ((448+BITS_PER_MPI_LIMB-1)/BITS_PER_MPI_LIMB)
#define LIMB_SIZE_HALF_448 ((LIMB_SIZE_448+1)/2)

static void
ec_addm_448 (gcry_mpi_t w, gcry_mpi_t u, gcry_mpi_t v, mpi_ec_t ctx)
{
  mpi_ptr_t wp, up, vp;
  mpi_size_t wsize = LIMB_SIZE_448;
  mpi_limb_t n[LIMB_SIZE_448];
  mpi_limb_t cy;

  if (w->nlimbs != wsize || u->nlimbs != wsize || v->nlimbs != wsize)
    log_bug ("addm_448: different sizes\n");

  up = u->d;
  vp = v->d;
  wp = w->d;

  cy = _gcry_mpih_add_n (wp, up, vp, wsize);
  _gcry_mpih_sub_n (n, wp, ctx->p->d, wsize);
  mpih_set_cond (wp, n, wsize, (cy != 0UL));
}

static void
ec_subm_448 (gcry_mpi_t w, gcry_mpi_t u, gcry_mpi_t v, mpi_ec_t ctx)
{
  mpi_ptr_t wp, up, vp;
  mpi_size_t wsize = LIMB_SIZE_448;
  mpi_limb_t n[LIMB_SIZE_448];
  mpi_limb_t borrow;

  if (w->nlimbs != wsize || u->nlimbs != wsize || v->nlimbs != wsize)
    log_bug ("subm_448: different sizes\n");

  up = u->d;
  vp = v->d;
  wp = w->d;

  borrow = _gcry_mpih_sub_n (wp, up, vp, wsize);
  _gcry_mpih_add_n (n, wp, ctx->p->d, wsize);
  mpih_set_cond (wp, n, wsize, (borrow != 0UL));
}

static void
ec_mulm_448 (gcry_mpi_t w, gcry_mpi_t u, gcry_mpi_t v, mpi_ec_t ctx)
{
  mpi_ptr_t wp, up, vp;
  mpi_size_t wsize = LIMB_SIZE_448;
  mpi_limb_t n[LIMB_SIZE_448*2];
  mpi_limb_t a2[LIMB_SIZE_HALF_448];
  mpi_limb_t a3[LIMB_SIZE_HALF_448];
  mpi_limb_t b0[LIMB_SIZE_HALF_448];
  mpi_limb_t b1[LIMB_SIZE_HALF_448];
  mpi_limb_t cy;

  if (w->nlimbs != wsize || u->nlimbs != wsize || v->nlimbs != wsize)
    log_bug ("mulm_448: different sizes\n");

  up = u->d;
  vp = v->d;
  wp = w->d;

  _gcry_mpih_mul_n (n, up, vp, wsize);

  memcpy (b0, n, LIMB_SIZE_HALF_448 * BYTES_PER_MPI_LIMB);
  memcpy (a2, n + wsize, LIMB_SIZE_HALF_448 * BYTES_PER_MPI_LIMB);

#if (LIMB_SIZE_HALF_448 > LIMB_SIZE_448/2)
  b0[LIMB_SIZE_HALF_448-1] &= ((mpi_limb_t)1UL<<32)-1;
  a2[LIMB_SIZE_HALF_448-1] &= ((mpi_limb_t)1UL<<32)-1;
  _gcry_mpih_rshift (b1, n + wsize/2, LIMB_SIZE_HALF_448, 32);
  _gcry_mpih_rshift (a3, n + wsize + wsize/2, LIMB_SIZE_HALF_448, 32);
#else
  memcpy (b1, n + wsize/2, LIMB_SIZE_HALF_448 * BYTES_PER_MPI_LIMB);
  memcpy (a3, n + wsize + wsize/2, LIMB_SIZE_HALF_448 * BYTES_PER_MPI_LIMB);
#endif

  cy = _gcry_mpih_add_n (b0, b0, a2, LIMB_SIZE_HALF_448);
  cy += _gcry_mpih_add_n (wp, b0, a3, LIMB_SIZE_HALF_448);
#if (LIMB_SIZE_HALF_448 > LIMB_SIZE_448/2)
  cy = wp[LIMB_SIZE_HALF_448-1] >> 32;
  wp[LIMB_SIZE_HALF_448-1] &= (((mpi_limb_t)1UL <<32)-1);
#endif
  memset (b0, 0, LIMB_SIZE_HALF_448 * BYTES_PER_MPI_LIMB);
  b0[0] = cy;

  cy = _gcry_mpih_add_n (b1, b1, b0, LIMB_SIZE_HALF_448);
  cy += _gcry_mpih_lshift (a3, a3, LIMB_SIZE_HALF_448, 1);
  cy += _gcry_mpih_add_n (b1, b1, a2, LIMB_SIZE_HALF_448);
  cy += _gcry_mpih_add_n (b1, b1, a3, LIMB_SIZE_HALF_448);
#if (LIMB_SIZE_HALF_448 > LIMB_SIZE_448/2)
  cy = _gcry_mpih_rshift (b1, b1, LIMB_SIZE_HALF_448, 32);
  wp[LIMB_SIZE_HALF_448-1] |= cy;
#endif
  memcpy (wp + LIMB_SIZE_HALF_448, b1, (wsize / 2) * BYTES_PER_MPI_LIMB);

#if (LIMB_SIZE_HALF_448 > LIMB_SIZE_448/2)
  cy = b1[LIMB_SIZE_HALF_448-1];
#endif

  memset (n, 0, wsize * BYTES_PER_MPI_LIMB);

#if (LIMB_SIZE_HALF_448 > LIMB_SIZE_448/2)
  n[LIMB_SIZE_HALF_448-1] = cy << 32;
#else
  n[LIMB_SIZE_HALF_448] = cy;
#endif
  n[0] = cy;
  _gcry_mpih_add_n (wp, wp, n, wsize);

  cy = _gcry_mpih_sub_n (n, wp, ctx->p->d, wsize);
  mpih_set_cond (wp, n, wsize, (cy == 0UL));
}

static void
ec_mul2_448 (gcry_mpi_t w, gcry_mpi_t u, mpi_ec_t ctx)
{
  ec_addm_448 (w, u, u, ctx);
}

static void
ec_pow2_448 (gcry_mpi_t w, const gcry_mpi_t b, mpi_ec_t ctx)
{
  ec_mulm_448 (w, b, b, ctx);
}


/* Fast reduction for secp256k1 */
static void
ec_secp256k1_mod (gcry_mpi_t w, mpi_ec_t ctx)
{
  mpi_size_t wsize = (256 + BITS_PER_MPI_LIMB - 1) / BITS_PER_MPI_LIMB;
  mpi_limb_t n[wsize + 1];
  mpi_limb_t s[wsize + 1];
  mpi_limb_t cy, borrow;
  mpi_ptr_t wp;

  MPN_NORMALIZE (w->d, w->nlimbs);
  if (w->nlimbs > 2 * 256 / BITS_PER_MPI_LIMB)
    log_bug ("W must be less than m^2\n");

  RESIZE_AND_CLEAR_IF_NEEDED (w, wsize * 2);

  wp = w->d;

  /* mod P (2^256 - 2^32 - 977) */

  /* first pass of reduction */
  memcpy (n, wp + wsize, wsize * BYTES_PER_MPI_LIMB);
#if BITS_PER_MPI_LIMB == 64
  s[wsize] = _gcry_mpih_lshift (s, wp + wsize, wsize, 32);
#else
  s[0] = 0;
  memcpy (s + 1, wp + wsize, wsize * BYTES_PER_MPI_LIMB);
#endif
  wp[wsize] = _gcry_mpih_addmul_1 (wp, n, wsize, 977);
  cy = _gcry_mpih_add_n (wp, wp, s, wsize + 1);

  /* second pass of reduction */
#if BITS_PER_MPI_LIMB == 64
  /* cy == 0 */
  memset (n + 1, 0, (wsize - 1) * BYTES_PER_MPI_LIMB);
  umul_ppmm(n[1], n[0], wp[wsize], ((mpi_limb_t)1 << 32) + 977);
#else
  memset (n + 2, 0, (wsize - 2) * BYTES_PER_MPI_LIMB);
  umul_ppmm(n[1], n[0], wp[wsize], 977);
  add_ssaaaa(n[2], n[1], 0, n[1], 0, cy * 977);
  add_ssaaaa(n[2], n[1], n[2], n[1], cy, wp[wsize]);
#endif
  cy = _gcry_mpih_add_n (wp, wp, n, wsize);

  borrow = _gcry_mpih_sub_n (s, wp, ctx->p->d, wsize);
  mpih_set_cond (wp, s, wsize, (cy != 0UL) | (borrow == 0UL));

  w->nlimbs = wsize;
  MPN_NORMALIZE (wp, w->nlimbs);
}


struct field_table {
  const char *p;

  /* computation routines for the field.  */
  void (* addm) (gcry_mpi_t w, gcry_mpi_t u, gcry_mpi_t v, mpi_ec_t ctx);
  void (* subm) (gcry_mpi_t w, gcry_mpi_t u, gcry_mpi_t v, mpi_ec_t ctx);
  void (* mulm) (gcry_mpi_t w, gcry_mpi_t u, gcry_mpi_t v, mpi_ec_t ctx);
  void (* mul2) (gcry_mpi_t w, gcry_mpi_t u, mpi_ec_t ctx);
  void (* pow2) (gcry_mpi_t w, const gcry_mpi_t b, mpi_ec_t ctx);
  void (* mod) (gcry_mpi_t w, mpi_ec_t ctx);
};

static const struct field_table field_table[] = {
  {
    "0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFED",
    ec_addm_25519,
    ec_subm_25519,
    ec_mulm_25519,
    ec_mul2_25519,
    ec_pow2_25519,
    NULL
  },
  {
   "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFE"
   "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",
    ec_addm_448,
    ec_subm_448,
    ec_mulm_448,
    ec_mul2_448,
    ec_pow2_448,
    NULL
  },
  {
    "0xfffffffffffffffffffffffffffffffeffffffffffffffff",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    _gcry_mpi_ec_nist192_mod
  },
  {
    "0xffffffffffffffffffffffffffffffff000000000000000000000001",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    _gcry_mpi_ec_nist224_mod
  },
  {
    "0xffffffff00000001000000000000000000000000ffffffffffffffffffffffff",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    _gcry_mpi_ec_nist256_mod
  },
  {
    "0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe"
    "ffffffff0000000000000000ffffffff",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    _gcry_mpi_ec_nist384_mod
  },
  {
    "0x01ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
    "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    _gcry_mpi_ec_nist521_mod
  },
  {
    "0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    ec_secp256k1_mod
  },
  { NULL, NULL, NULL, NULL, NULL, NULL },
};

static gcry_mpi_t field_table_mpis[DIM(field_table)];


/* Force recomputation of all helper variables.  */
void
_gcry_mpi_ec_get_reset (mpi_ec_t ec)
{
  ec->t.valid.a_is_pminus3 = 0;
  ec->t.valid.two_inv_p = 0;
}


/* Accessor for helper variable.  */
static int
ec_get_a_is_pminus3 (mpi_ec_t ec)
{
  gcry_mpi_t tmp;

  if (!ec->t.valid.a_is_pminus3)
    {
      ec->t.valid.a_is_pminus3 = 1;
      tmp = mpi_alloc_like (ec->p);
      mpi_sub_ui (tmp, ec->p, 3);
      ec->t.a_is_pminus3 = !mpi_cmp (ec->a, tmp);
      mpi_free (tmp);
    }

  return ec->t.a_is_pminus3;
}


/* Accessor for helper variable.  */
static gcry_mpi_t
ec_get_two_inv_p (mpi_ec_t ec)
{
  if (!ec->t.valid.two_inv_p)
    {
      ec->t.valid.two_inv_p = 1;
      if (!ec->t.two_inv_p)
        ec->t.two_inv_p = mpi_alloc (0);
      ec_invm (ec->t.two_inv_p, mpi_const (MPI_C_TWO), ec);
    }
  return ec->t.two_inv_p;
}


static const char *const curve25519_bad_points[] = {
  "0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffed",
  "0x0000000000000000000000000000000000000000000000000000000000000000",
  "0x0000000000000000000000000000000000000000000000000000000000000001",
  "0x00b8495f16056286fdb1329ceb8d09da6ac49ff1fae35616aeb8413b7c7aebe0",
  "0x57119fd0dd4e22d8868e1c58c45c44045bef839c55b1d0b1248c50a3bc959c5f",
  "0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffec",
  "0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffee",
  NULL
};


static const char *const curve448_bad_points[] = {
  "0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffe"
  "ffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
  "0x00000000000000000000000000000000000000000000000000000000"
  "00000000000000000000000000000000000000000000000000000000",
  "0x00000000000000000000000000000000000000000000000000000000"
  "00000000000000000000000000000000000000000000000000000001",
  "0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffe"
  "fffffffffffffffffffffffffffffffffffffffffffffffffffffffe",
  "0xffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
  "00000000000000000000000000000000000000000000000000000000",
  NULL
};

static const char *const *bad_points_table[] = {
  curve25519_bad_points,
  curve448_bad_points,
};

static gcry_mpi_t
scanval (const char *string)
{
  gpg_err_code_t rc;
  gcry_mpi_t val;

  rc = _gcry_mpi_scan (&val, GCRYMPI_FMT_HEX, string, 0, NULL);
  if (rc)
    log_fatal ("scanning ECC parameter failed: %s\n", gpg_strerror (rc));
  return val;
}


/* This function initialized a context for elliptic curve based on the
   field GF(p).  P is the prime specifying this field, A is the first
   coefficient.  CTX is expected to be zeroized.  */
static void
ec_p_init (mpi_ec_t ctx, enum gcry_mpi_ec_models model,
           enum ecc_dialects dialect,
           int flags,
           gcry_mpi_t p, gcry_mpi_t a, gcry_mpi_t b)
{
  int i;
  static int use_barrett;

  if (!use_barrett)
    {
      if (getenv ("GCRYPT_BARRETT"))
        use_barrett = 1;
      else
        use_barrett = -1;
    }

  /* Fixme: Do we want to check some constraints? e.g.  a < p  */

  ctx->model = model;
  ctx->dialect = dialect;
  ctx->flags = flags;
  ctx->nbits = mpi_get_nbits (p);
  ctx->p = mpi_copy (p);
  ctx->a = mpi_copy (a);
  ctx->b = mpi_copy (b);

  ctx->t.p_barrett = use_barrett > 0? _gcry_mpi_barrett_init (ctx->p, 0):NULL;

  _gcry_mpi_ec_get_reset (ctx);

  if (model == MPI_EC_MONTGOMERY)
    {
      for (i=0; i< DIM(bad_points_table); i++)
        {
          gcry_mpi_t p_candidate = scanval (bad_points_table[i][0]);
          int match_p = !mpi_cmp (ctx->p, p_candidate);
          int j;

          mpi_free (p_candidate);
          if (!match_p)
            continue;

          for (j=0; i< DIM(ctx->t.scratch) && bad_points_table[i][j]; j++)
            ctx->t.scratch[j] = scanval (bad_points_table[i][j]);
        }
    }
  else
    {
      /* Allocate scratch variables.  */
      for (i=0; i< DIM(ctx->t.scratch); i++)
        ctx->t.scratch[i] = mpi_alloc_like (ctx->p);
    }

  ctx->addm = ec_addm;
  ctx->subm = ec_subm;
  ctx->mulm = ec_mulm;
  ctx->mul2 = ec_mul2;
  ctx->pow2 = ec_pow2;
  ctx->mod = ec_mod;

  for (i=0; field_table[i].p; i++)
    {
      gcry_mpi_t f_p;
      gpg_err_code_t rc;

      if (field_table_mpis[i] == NULL)
	{
	  rc = _gcry_mpi_scan (&f_p, GCRYMPI_FMT_HEX, field_table[i].p, 0,
			       NULL);
	  if (rc)
	    log_fatal ("scanning ECC parameter failed: %s\n",
		       gpg_strerror (rc));
	  field_table_mpis[i] = f_p; /* cache */
	}
      else
	{
	  f_p = field_table_mpis[i];
	}

      if (!mpi_cmp (p, f_p))
        {
          ctx->addm = field_table[i].addm ? field_table[i].addm : ctx->addm;
          ctx->subm = field_table[i].subm ? field_table[i].subm : ctx->subm;
          ctx->mulm = field_table[i].mulm ? field_table[i].mulm : ctx->mulm;
          ctx->mul2 = field_table[i].mul2 ? field_table[i].mul2 : ctx->mul2;
          ctx->pow2 = field_table[i].pow2 ? field_table[i].pow2 : ctx->pow2;
          ctx->mod = field_table[i].mod ? field_table[i].mod : ctx->mod;

	  if (ctx->a)
	    {
	      mpi_resize (ctx->a, ctx->p->nlimbs);
	      ctx->a->nlimbs = ctx->p->nlimbs;
	    }

	  if (ctx->b)
	    {
	      mpi_resize (ctx->b, ctx->p->nlimbs);
	      ctx->b->nlimbs = ctx->p->nlimbs;
	    }

          for (i=0; i< DIM(ctx->t.scratch) && ctx->t.scratch[i]; i++)
            ctx->t.scratch[i]->nlimbs = ctx->p->nlimbs;

          break;
        }
    }

  /* Prepare for fast reduction.  */
  /* FIXME: need a test for NIST values.  However it does not gain us
     any real advantage, for 384 bits it is actually slower than using
     mpi_mulm.  */
/*   ctx->nist_nbits = mpi_get_nbits (ctx->p); */
/*   if (ctx->nist_nbits == 192) */
/*     { */
/*       for (i=0; i < 4; i++) */
/*         ctx->s[i] = mpi_new (192); */
/*       ctx->c    = mpi_new (192*2); */
/*     } */
/*   else if (ctx->nist_nbits == 384) */
/*     { */
/*       for (i=0; i < 10; i++) */
/*         ctx->s[i] = mpi_new (384); */
/*       ctx->c    = mpi_new (384*2); */
/*     } */
}


static void
ec_deinit (void *opaque)
{
  mpi_ec_t ctx = opaque;
  int i;

  _gcry_mpi_barrett_free (ctx->t.p_barrett);

  /* Domain parameter.  */
  mpi_free (ctx->p);
  mpi_free (ctx->a);
  mpi_free (ctx->b);
  _gcry_mpi_point_release (ctx->G);
  mpi_free (ctx->n);

  /* The key.  */
  _gcry_mpi_point_release (ctx->Q);
  mpi_free (ctx->d);

  /* Private data of ec.c.  */
  mpi_free (ctx->t.two_inv_p);

  for (i=0; i< DIM(ctx->t.scratch); i++)
    mpi_free (ctx->t.scratch[i]);

/*   if (ctx->nist_nbits == 192) */
/*     { */
/*       for (i=0; i < 4; i++) */
/*         mpi_free (ctx->s[i]); */
/*       mpi_free (ctx->c); */
/*     } */
/*   else if (ctx->nist_nbits == 384) */
/*     { */
/*       for (i=0; i < 10; i++) */
/*         mpi_free (ctx->s[i]); */
/*       mpi_free (ctx->c); */
/*     } */
}


/* This function returns a new context for elliptic curve based on the
   field GF(p).  P is the prime specifying this field, A is the first
   coefficient, B is the second coefficient, and MODEL is the model
   for the curve.  This function is only used within Libgcrypt and not
   part of the public API.

   This context needs to be released using _gcry_mpi_ec_free.  */
mpi_ec_t
_gcry_mpi_ec_p_internal_new (enum gcry_mpi_ec_models model,
                             enum ecc_dialects dialect,
                             int flags,
                             gcry_mpi_t p, gcry_mpi_t a, gcry_mpi_t b)
{
  mpi_ec_t ctx;

  ctx = xcalloc (1, sizeof *ctx);
  ec_p_init (ctx, model, dialect, flags, p, a, b);

  return ctx;
}


/* This is a variant of _gcry_mpi_ec_p_internal_new which returns an
   public context and does some error checking on the supplied
   arguments.  On success the new context is stored at R_CTX and 0 is
   returned; on error NULL is stored at R_CTX and an error code is
   returned.

   The context needs to be released using gcry_ctx_release.  */
gpg_err_code_t
_gcry_mpi_ec_p_new (gcry_ctx_t *r_ctx,
                    enum gcry_mpi_ec_models model,
                    enum ecc_dialects dialect,
                    int flags,
                    gcry_mpi_t p, gcry_mpi_t a, gcry_mpi_t b)
{
  gcry_ctx_t ctx;
  mpi_ec_t ec;

  *r_ctx = NULL;
  if (!p || !a)
    return GPG_ERR_EINVAL;

  ctx = _gcry_ctx_alloc (CONTEXT_TYPE_EC, sizeof *ec, ec_deinit);
  if (!ctx)
    return gpg_err_code_from_syserror ();
  ec = _gcry_ctx_get_pointer (ctx, CONTEXT_TYPE_EC);
  ec_p_init (ec, model, dialect, flags, p, a, b);

  *r_ctx = ctx;
  return 0;
}


void
_gcry_mpi_ec_free (mpi_ec_t ctx)
{
  if (ctx)
    {
      ec_deinit (ctx);
      xfree (ctx);
    }
}


gcry_mpi_t
_gcry_mpi_ec_get_mpi (const char *name, gcry_ctx_t ctx, int copy)
{
  mpi_ec_t ec = _gcry_ctx_get_pointer (ctx, CONTEXT_TYPE_EC);

  return _gcry_ecc_get_mpi (name, ec, copy);
}


gcry_mpi_point_t
_gcry_mpi_ec_get_point (const char *name, gcry_ctx_t ctx, int copy)
{
  mpi_ec_t ec = _gcry_ctx_get_pointer (ctx, CONTEXT_TYPE_EC);

  (void)copy;  /* Not used.  */

  return _gcry_ecc_get_point (name, ec);
}


gpg_err_code_t
_gcry_mpi_ec_set_mpi (const char *name, gcry_mpi_t newvalue,
                      gcry_ctx_t ctx)
{
  mpi_ec_t ec = _gcry_ctx_get_pointer (ctx, CONTEXT_TYPE_EC);

  return _gcry_ecc_set_mpi (name, newvalue, ec);
}


gpg_err_code_t
_gcry_mpi_ec_set_point (const char *name, gcry_mpi_point_t newvalue,
                        gcry_ctx_t ctx)
{
  mpi_ec_t ec = _gcry_ctx_get_pointer (ctx, CONTEXT_TYPE_EC);

  return _gcry_ecc_set_point (name, newvalue, ec);
}


/* Given an encoded point in the MPI VALUE and a context EC, decode
 * the point according to the context and store it in RESULT.  On
 * error an error code is return but RESULT might have been changed.
 * If no context is given the function tries to decode VALUE by
 * assuming a 0x04 prefixed uncompressed encoding.  */
gpg_err_code_t
_gcry_mpi_ec_decode_point (mpi_point_t result, gcry_mpi_t value, mpi_ec_t ec)
{
  gpg_err_code_t rc;

  if (ec
      && (ec->dialect == ECC_DIALECT_ED25519
          || (ec->model == MPI_EC_EDWARDS
              && ec->dialect == ECC_DIALECT_SAFECURVE)))
    rc = _gcry_ecc_eddsa_decodepoint (value, ec, result, NULL, NULL);
  else if (ec && ec->model == MPI_EC_MONTGOMERY)
    rc = _gcry_ecc_mont_decodepoint (value, ec, result);
  else
    rc = _gcry_ecc_sec_decodepoint (value, ec, result);

  return rc;
}


/* Compute the affine coordinates from the projective coordinates in
   POINT.  Set them into X and Y.  If one coordinate is not required,
   X or Y may be passed as NULL.  CTX is the usual context. Returns: 0
   on success or !0 if POINT is at infinity.  */
int
_gcry_mpi_ec_get_affine (gcry_mpi_t x, gcry_mpi_t y, mpi_point_t point,
                         mpi_ec_t ctx)
{
  if (!mpi_cmp_ui (point->z, 0))
    return -1;

  switch (ctx->model)
    {
    case MPI_EC_WEIERSTRASS: /* Using Jacobian coordinates.  */
      {
        gcry_mpi_t z1, z2, z3;

	if (!mpi_cmp_ui (point->z, 1))
	  {
	    if (x)
	      mpi_set (x, point->x);
	    if (y)
	      mpi_set (y, point->y);
	    return 0;
	  }

        z1 = mpi_new (0);
        z2 = mpi_new (0);
        ec_invm (z1, point->z, ctx);  /* z1 = z^(-1) mod p  */
        ec_mulm (z2, z1, z1, ctx);    /* z2 = z^(-2) mod p  */

        if (x)
          ec_mulm (x, point->x, z2, ctx);

        if (y)
          {
            z3 = mpi_new (0);
            ec_mulm (z3, z2, z1, ctx);      /* z3 = z^(-3) mod p  */
            ec_mulm (y, point->y, z3, ctx);
            mpi_free (z3);
          }

        mpi_free (z2);
        mpi_free (z1);
      }
      return 0;

    case MPI_EC_MONTGOMERY:
      {
        if (x)
          mpi_set (x, point->x);

        if (y)
          {
            log_fatal ("%s: Getting Y-coordinate on %s is not supported\n",
                       "_gcry_mpi_ec_get_affine", "Montgomery");
            return -1;
          }
      }
      return 0;

    case MPI_EC_EDWARDS:
      {
        gcry_mpi_t z;

	if (!mpi_cmp_ui (point->z, 1))
	  {
	    if (x)
	      mpi_set (x, point->x);
	    if (y)
	      mpi_set (y, point->y);
	    return 0;
	  }

        z = mpi_new (0);
        ec_invm (z, point->z, ctx);

        mpi_resize (z, ctx->p->nlimbs);
        z->nlimbs = ctx->p->nlimbs;

        if (x)
          {
            mpi_resize (x, ctx->p->nlimbs);
            x->nlimbs = ctx->p->nlimbs;
            ctx->mulm (x, point->x, z, ctx);
          }
        if (y)
          {
            mpi_resize (y, ctx->p->nlimbs);
            y->nlimbs = ctx->p->nlimbs;
            ctx->mulm (y, point->y, z, ctx);
          }

        _gcry_mpi_release (z);
      }
      return 0;

    default:
      return -1;
    }
}



/*  RESULT = 2 * POINT  (Weierstrass version). */
static void
dup_point_weierstrass (mpi_point_t result, mpi_point_t point, mpi_ec_t ctx)
{
#define x3 (result->x)
#define y3 (result->y)
#define z3 (result->z)
#define t1 (ctx->t.scratch[0])
#define t2 (ctx->t.scratch[1])
#define t3 (ctx->t.scratch[2])
#define l1 (ctx->t.scratch[3])
#define l2 (ctx->t.scratch[4])
#define l3 (ctx->t.scratch[5])

  if (!mpi_cmp_ui (point->y, 0) || !mpi_cmp_ui (point->z, 0))
    {
      /* P_y == 0 || P_z == 0 => [1:1:0] */
      mpi_set_ui (x3, 1);
      mpi_set_ui (y3, 1);
      mpi_set_ui (z3, 0);
    }
  else
    {
      if (ec_get_a_is_pminus3 (ctx))  /* Use the faster case.  */
        {
          /* L1 = 3(X - Z^2)(X + Z^2) */
          /*                          T1: used for Z^2. */
          /*                          T2: used for the right term.  */
          ec_pow2 (t1, point->z, ctx);
          ec_subm (l1, point->x, t1, ctx);
          ec_mulm (l1, l1, mpi_const (MPI_C_THREE), ctx);
          ec_addm (t2, point->x, t1, ctx);
          ec_mulm (l1, l1, t2, ctx);
        }
      else /* Standard case. */
        {
          /* L1 = 3X^2 + aZ^4 */
          /*                          T1: used for aZ^4. */
          ec_pow2 (l1, point->x, ctx);
          ec_mulm (l1, l1, mpi_const (MPI_C_THREE), ctx);
          ec_powm (t1, point->z, mpi_const (MPI_C_FOUR), ctx);
          ec_mulm (t1, t1, ctx->a, ctx);
          ec_addm (l1, l1, t1, ctx);
        }
      /* Z3 = 2YZ */
      ec_mulm (z3, point->y, point->z, ctx);
      ec_mul2 (z3, z3, ctx);

      /* L2 = 4XY^2 */
      /*                              T2: used for Y2; required later. */
      ec_pow2 (t2, point->y, ctx);
      ec_mulm (l2, t2, point->x, ctx);
      ec_mulm (l2, l2, mpi_const (MPI_C_FOUR), ctx);

      /* X3 = L1^2 - 2L2 */
      /*                              T1: used for L2^2. */
      ec_pow2 (x3, l1, ctx);
      ec_mul2 (t1, l2, ctx);
      ec_subm (x3, x3, t1, ctx);

      /* L3 = 8Y^4 */
      /*                              T2: taken from above. */
      ec_pow2 (t2, t2, ctx);
      ec_mulm (l3, t2, mpi_const (MPI_C_EIGHT), ctx);

      /* Y3 = L1(L2 - X3) - L3 */
      ec_subm (y3, l2, x3, ctx);
      ec_mulm (y3, y3, l1, ctx);
      ec_subm (y3, y3, l3, ctx);
    }

#undef x3
#undef y3
#undef z3
#undef t1
#undef t2
#undef t3
#undef l1
#undef l2
#undef l3
}


/*  RESULT = 2 * POINT  (Montgomery version). */
static void
dup_point_montgomery (mpi_point_t result, mpi_point_t point, mpi_ec_t ctx)
{
  (void)result;
  (void)point;
  (void)ctx;
  log_fatal ("%s: %s not yet supported\n",
             "_gcry_mpi_ec_dup_point", "Montgomery");
}


/*  RESULT = 2 * POINT  (Twisted Edwards version). */
static void
dup_point_edwards (mpi_point_t result, mpi_point_t point, mpi_ec_t ctx)
{
#define X1 (point->x)
#define Y1 (point->y)
#define Z1 (point->z)
#define X3 (result->x)
#define Y3 (result->y)
#define Z3 (result->z)
#define B (ctx->t.scratch[0])
#define C (ctx->t.scratch[1])
#define D (ctx->t.scratch[2])
#define E (ctx->t.scratch[3])
#define F (ctx->t.scratch[4])
#define H (ctx->t.scratch[5])
#define J (ctx->t.scratch[6])

  /* Compute: (X_3 : Y_3 : Z_3) = 2( X_1 : Y_1 : Z_1 ) */

  /* B = (X_1 + Y_1)^2  */
  ctx->addm (B, X1, Y1, ctx);
  ctx->pow2 (B, B, ctx);

  /* C = X_1^2 */
  /* D = Y_1^2 */
  ctx->pow2 (C, X1, ctx);
  ctx->pow2 (D, Y1, ctx);

  /* E = aC */
  if (ctx->dialect == ECC_DIALECT_ED25519)
    ctx->subm (E, ctx->p, C, ctx);
  else
    ctx->mulm (E, ctx->a, C, ctx);

  /* F = E + D */
  ctx->addm (F, E, D, ctx);

  /* H = Z_1^2 */
  ctx->pow2 (H, Z1, ctx);

  /* J = F - 2H */
  ctx->mul2 (J, H, ctx);
  ctx->subm (J, F, J, ctx);

  /* X_3 = (B - C - D) · J */
  ctx->subm (X3, B, C, ctx);
  ctx->subm (X3, X3, D, ctx);
  ctx->mulm (X3, X3, J, ctx);

  /* Y_3 = F · (E - D) */
  ctx->subm (Y3, E, D, ctx);
  ctx->mulm (Y3, Y3, F, ctx);

  /* Z_3 = F · J */
  ctx->mulm (Z3, F, J, ctx);

#undef X1
#undef Y1
#undef Z1
#undef X3
#undef Y3
#undef Z3
#undef B
#undef C
#undef D
#undef E
#undef F
#undef H
#undef J
}


/*  RESULT = 2 * POINT  */
void
_gcry_mpi_ec_dup_point (mpi_point_t result, mpi_point_t point, mpi_ec_t ctx)
{
  switch (ctx->model)
    {
    case MPI_EC_WEIERSTRASS:
      dup_point_weierstrass (result, point, ctx);
      break;
    case MPI_EC_MONTGOMERY:
      dup_point_montgomery (result, point, ctx);
      break;
    case MPI_EC_EDWARDS:
      dup_point_edwards (result, point, ctx);
      break;
    }
}


/* RESULT = P1 + P2  (Weierstrass version).*/
static void
add_points_weierstrass (mpi_point_t result,
                        mpi_point_t p1, mpi_point_t p2,
                        mpi_ec_t ctx)
{
#define x1 (p1->x    )
#define y1 (p1->y    )
#define z1 (p1->z    )
#define x2 (p2->x    )
#define y2 (p2->y    )
#define z2 (p2->z    )
#define x3 (result->x)
#define y3 (result->y)
#define z3 (result->z)
#define l1 (ctx->t.scratch[0])
#define l2 (ctx->t.scratch[1])
#define l3 (ctx->t.scratch[2])
#define l4 (ctx->t.scratch[3])
#define l5 (ctx->t.scratch[4])
#define l6 (ctx->t.scratch[5])
#define l7 (ctx->t.scratch[6])
#define l8 (ctx->t.scratch[7])
#define l9 (ctx->t.scratch[8])
#define t1 (ctx->t.scratch[9])
#define t2 (ctx->t.scratch[10])

  if ( (!mpi_cmp (x1, x2)) && (!mpi_cmp (y1, y2)) && (!mpi_cmp (z1, z2)) )
    {
      /* Same point; need to call the duplicate function.  */
      _gcry_mpi_ec_dup_point (result, p1, ctx);
    }
  else if (!mpi_cmp_ui (z1, 0))
    {
      /* P1 is at infinity.  */
      mpi_set (x3, p2->x);
      mpi_set (y3, p2->y);
      mpi_set (z3, p2->z);
    }
  else if (!mpi_cmp_ui (z2, 0))
    {
      /* P2 is at infinity.  */
      mpi_set (x3, p1->x);
      mpi_set (y3, p1->y);
      mpi_set (z3, p1->z);
    }
  else
    {
      int z1_is_one = !mpi_cmp_ui (z1, 1);
      int z2_is_one = !mpi_cmp_ui (z2, 1);

      /* l1 = x1 z2^2  */
      /* l2 = x2 z1^2  */
      if (z2_is_one)
        mpi_set (l1, x1);
      else
        {
          ec_pow2 (l1, z2, ctx);
          ec_mulm (l1, l1, x1, ctx);
        }
      if (z1_is_one)
        mpi_set (l2, x2);
      else
        {
          ec_pow2 (l2, z1, ctx);
          ec_mulm (l2, l2, x2, ctx);
        }
      /* l3 = l1 - l2 */
      ec_subm (l3, l1, l2, ctx);
      /* l4 = y1 z2^3  */
      ec_powm (l4, z2, mpi_const (MPI_C_THREE), ctx);
      ec_mulm (l4, l4, y1, ctx);
      /* l5 = y2 z1^3  */
      ec_powm (l5, z1, mpi_const (MPI_C_THREE), ctx);
      ec_mulm (l5, l5, y2, ctx);
      /* l6 = l4 - l5  */
      ec_subm (l6, l4, l5, ctx);

      if (!mpi_cmp_ui (l3, 0))
        {
          if (!mpi_cmp_ui (l6, 0))
            {
              /* P1 and P2 are the same - use duplicate function.  */
              _gcry_mpi_ec_dup_point (result, p1, ctx);
            }
          else
            {
              /* P1 is the inverse of P2.  */
              mpi_set_ui (x3, 1);
              mpi_set_ui (y3, 1);
              mpi_set_ui (z3, 0);
            }
        }
      else
        {
          /* l7 = l1 + l2  */
          ec_addm (l7, l1, l2, ctx);
          /* l8 = l4 + l5  */
          ec_addm (l8, l4, l5, ctx);
          /* z3 = z1 z2 l3  */
          ec_mulm (z3, z1, z2, ctx);
          ec_mulm (z3, z3, l3, ctx);
          /* x3 = l6^2 - l7 l3^2  */
          ec_pow2 (t1, l6, ctx);
          ec_pow2 (t2, l3, ctx);
          ec_mulm (t2, t2, l7, ctx);
          ec_subm (x3, t1, t2, ctx);
          /* l9 = l7 l3^2 - 2 x3  */
          ec_mul2 (t1, x3, ctx);
          ec_subm (l9, t2, t1, ctx);
          /* y3 = (l9 l6 - l8 l3^3)/2  */
          ec_mulm (l9, l9, l6, ctx);
          ec_powm (t1, l3, mpi_const (MPI_C_THREE), ctx); /* fixme: Use saved value*/
          ec_mulm (t1, t1, l8, ctx);
          ec_subm (y3, l9, t1, ctx);
          ec_mulm (y3, y3, ec_get_two_inv_p (ctx), ctx);
        }
    }

#undef x1
#undef y1
#undef z1
#undef x2
#undef y2
#undef z2
#undef x3
#undef y3
#undef z3
#undef l1
#undef l2
#undef l3
#undef l4
#undef l5
#undef l6
#undef l7
#undef l8
#undef l9
#undef t1
#undef t2
}


/* RESULT = P1 + P2  (Montgomery version).*/
static void
add_points_montgomery (mpi_point_t result,
                       mpi_point_t p1, mpi_point_t p2,
                       mpi_ec_t ctx)
{
  (void)result;
  (void)p1;
  (void)p2;
  (void)ctx;
  log_fatal ("%s: %s not yet supported\n",
             "_gcry_mpi_ec_add_points", "Montgomery");
}


/* RESULT = P1 + P2  (Twisted Edwards version).*/
static void
add_points_edwards (mpi_point_t result,
                    mpi_point_t p1, mpi_point_t p2,
                    mpi_ec_t ctx)
{
#define X1 (p1->x)
#define Y1 (p1->y)
#define Z1 (p1->z)
#define X2 (p2->x)
#define Y2 (p2->y)
#define Z2 (p2->z)
#define X3 (result->x)
#define Y3 (result->y)
#define Z3 (result->z)
#define A (ctx->t.scratch[0])
#define B (ctx->t.scratch[1])
#define C (ctx->t.scratch[2])
#define D (ctx->t.scratch[3])
#define E (ctx->t.scratch[4])
#define F (ctx->t.scratch[5])
#define G (ctx->t.scratch[6])
#define tmp (ctx->t.scratch[7])

  mpi_point_resize (result, ctx);

  /* Compute: (X_3 : Y_3 : Z_3) = (X_1 : Y_1 : Z_1) + (X_2 : Y_2 : Z_3)  */

  /* A = Z1 · Z2 */
  ctx->mulm (A, Z1, Z2, ctx);

  /* B = A^2 */
  ctx->pow2 (B, A, ctx);

  /* C = X1 · X2 */
  ctx->mulm (C, X1, X2, ctx);

  /* D = Y1 · Y2 */
  ctx->mulm (D, Y1, Y2, ctx);

  /* E = d · C · D */
  ctx->mulm (E, ctx->b, C, ctx);
  ctx->mulm (E, E, D, ctx);

  /* F = B - E */
  ctx->subm (F, B, E, ctx);

  /* G = B + E */
  ctx->addm (G, B, E, ctx);

  /* X_3 = A · F · ((X_1 + Y_1) · (X_2 + Y_2) - C - D) */
  ctx->addm (tmp, X1, Y1, ctx);
  ctx->addm (X3, X2, Y2, ctx);
  ctx->mulm (X3, X3, tmp, ctx);
  ctx->subm (X3, X3, C, ctx);
  ctx->subm (X3, X3, D, ctx);
  ctx->mulm (X3, X3, F, ctx);
  ctx->mulm (X3, X3, A, ctx);

  /* Y_3 = A · G · (D - aC) */
  if (ctx->dialect == ECC_DIALECT_ED25519)
    {
      ctx->addm (Y3, D, C, ctx);
    }
  else
    {
      ctx->mulm (Y3, ctx->a, C, ctx);
      ctx->subm (Y3, D, Y3, ctx);
    }
  ctx->mulm (Y3, Y3, G, ctx);
  ctx->mulm (Y3, Y3, A, ctx);

  /* Z_3 = F · G */
  ctx->mulm (Z3, F, G, ctx);


#undef X1
#undef Y1
#undef Z1
#undef X2
#undef Y2
#undef Z2
#undef X3
#undef Y3
#undef Z3
#undef A
#undef B
#undef C
#undef D
#undef E
#undef F
#undef G
#undef tmp
}


/* Compute a step of Montgomery Ladder (only use X and Z in the point).
   Inputs:  P1, P2, and x-coordinate of DIF = P1 - P1.
   Outputs: PRD = 2 * P1 and  SUM = P1 + P2. */
static void
montgomery_ladder (mpi_point_t prd, mpi_point_t sum,
                   mpi_point_t p1, mpi_point_t p2, gcry_mpi_t dif_x,
                   mpi_ec_t ctx)
{
  ctx->addm (sum->x, p2->x, p2->z, ctx);
  ctx->subm (p2->z, p2->x, p2->z, ctx);
  ctx->addm (prd->x, p1->x, p1->z, ctx);
  ctx->subm (p1->z, p1->x, p1->z, ctx);
  ctx->mulm (p2->x, p1->z, sum->x, ctx);
  ctx->mulm (p2->z, prd->x, p2->z, ctx);
  ctx->pow2 (p1->x, prd->x, ctx);
  ctx->pow2 (p1->z, p1->z, ctx);
  ctx->addm (sum->x, p2->x, p2->z, ctx);
  ctx->subm (p2->z, p2->x, p2->z, ctx);
  ctx->mulm (prd->x, p1->x, p1->z, ctx);
  ctx->subm (p1->z, p1->x, p1->z, ctx);
  ctx->pow2 (sum->x, sum->x, ctx);
  ctx->pow2 (sum->z, p2->z, ctx);
  ctx->mulm (prd->z, p1->z, ctx->a, ctx); /* CTX->A: (a-2)/4 */
  ctx->mulm (sum->z, sum->z, dif_x, ctx);
  ctx->addm (prd->z, p1->x, prd->z, ctx);
  ctx->mulm (prd->z, prd->z, p1->z, ctx);
}


/* RESULT = P1 + P2 */
void
_gcry_mpi_ec_add_points (mpi_point_t result,
                         mpi_point_t p1, mpi_point_t p2,
                         mpi_ec_t ctx)
{
  switch (ctx->model)
    {
    case MPI_EC_WEIERSTRASS:
      add_points_weierstrass (result, p1, p2, ctx);
      break;
    case MPI_EC_MONTGOMERY:
      add_points_montgomery (result, p1, p2, ctx);
      break;
    case MPI_EC_EDWARDS:
      add_points_edwards (result, p1, p2, ctx);
      break;
    }
}


/* RESULT = P1 - P2  (Weierstrass version).*/
static void
sub_points_weierstrass (mpi_point_t result,
                        mpi_point_t p1, mpi_point_t p2,
                        mpi_ec_t ctx)
{
  (void)result;
  (void)p1;
  (void)p2;
  (void)ctx;
  log_fatal ("%s: %s not yet supported\n",
             "_gcry_mpi_ec_sub_points", "Weierstrass");
}


/* RESULT = P1 - P2  (Montgomery version).*/
static void
sub_points_montgomery (mpi_point_t result,
                       mpi_point_t p1, mpi_point_t p2,
                       mpi_ec_t ctx)
{
  (void)result;
  (void)p1;
  (void)p2;
  (void)ctx;
  log_fatal ("%s: %s not yet supported\n",
             "_gcry_mpi_ec_sub_points", "Montgomery");
}


/* RESULT = P1 - P2  (Twisted Edwards version).*/
static void
sub_points_edwards (mpi_point_t result,
                    mpi_point_t p1, mpi_point_t p2,
                    mpi_ec_t ctx)
{
  mpi_point_t p2i = _gcry_mpi_point_new (0);
  point_set (p2i, p2);
  ctx->subm (p2i->x, ctx->p, p2i->x, ctx);
  add_points_edwards (result, p1, p2i, ctx);
  _gcry_mpi_point_release (p2i);
}


/* RESULT = P1 - P2 */
void
_gcry_mpi_ec_sub_points (mpi_point_t result,
                         mpi_point_t p1, mpi_point_t p2,
                         mpi_ec_t ctx)
{
  switch (ctx->model)
    {
    case MPI_EC_WEIERSTRASS:
      sub_points_weierstrass (result, p1, p2, ctx);
      break;
    case MPI_EC_MONTGOMERY:
      sub_points_montgomery (result, p1, p2, ctx);
      break;
    case MPI_EC_EDWARDS:
      sub_points_edwards (result, p1, p2, ctx);
      break;
    }
}


/* Scalar point multiplication - the main function for ECC.  It takes
   an integer SCALAR and a POINT as well as the usual context CTX.
   RESULT will be set to the resulting point. */
void
_gcry_mpi_ec_mul_point (mpi_point_t result,
                        gcry_mpi_t scalar, mpi_point_t point,
                        mpi_ec_t ctx)
{
  gcry_mpi_t x1, y1, z1, k, h, yy;
  unsigned int i, loops;
  mpi_point_struct p1, p2, p1inv;

  /* First try HW accelerated scalar multiplications.  Error
     is returned if acceleration is not supported or if HW
     does not support acceleration of given input.  */
  if (mpi_ec_hw_mul_point (result, scalar, point, ctx) >= 0)
    {
      return;
    }

  if (ctx->model == MPI_EC_EDWARDS
      || (ctx->model == MPI_EC_WEIERSTRASS
          && mpi_is_secure (scalar)))
    {
      /* Simple left to right binary method.  Algorithm 3.27 from
       * {author={Hankerson, Darrel and Menezes, Alfred J. and Vanstone, Scott},
       *  title = {Guide to Elliptic Curve Cryptography},
       *  year = {2003}, isbn = {038795273X},
       *  url = {http://www.cacr.math.uwaterloo.ca/ecc/},
       *  publisher = {Springer-Verlag New York, Inc.}} */
      unsigned int nbits;
      int j;

      if (mpi_cmp (scalar, ctx->p) >= 0)
        nbits = mpi_get_nbits (scalar);
      else
        nbits = mpi_get_nbits (ctx->p);

      if (ctx->model == MPI_EC_WEIERSTRASS)
        {
          mpi_set_ui (result->x, 1);
          mpi_set_ui (result->y, 1);
          mpi_set_ui (result->z, 0);
        }
      else
        {
          mpi_set_ui (result->x, 0);
          mpi_set_ui (result->y, 1);
          mpi_set_ui (result->z, 1);
          mpi_point_resize (point, ctx);
        }

      if (mpi_is_secure (scalar))
        {
          /* If SCALAR is in secure memory we assume that it is the
             secret key we use constant time operation.  */
          mpi_point_struct tmppnt;

          point_init (&tmppnt);
          mpi_point_resize (result, ctx);
          mpi_point_resize (&tmppnt, ctx);
          for (j=nbits-1; j >= 0; j--)
            {
              _gcry_mpi_ec_dup_point (result, result, ctx);
              _gcry_mpi_ec_add_points (&tmppnt, result, point, ctx);
              point_swap_cond (result, &tmppnt, mpi_test_bit (scalar, j), ctx);
            }
          point_free (&tmppnt);
        }
      else
        {
          if (ctx->model == MPI_EC_EDWARDS)
            {
              mpi_point_resize (result, ctx);
              mpi_point_resize (point, ctx);
            }

          for (j=nbits-1; j >= 0; j--)
            {
              _gcry_mpi_ec_dup_point (result, result, ctx);
              if (mpi_test_bit (scalar, j))
                _gcry_mpi_ec_add_points (result, result, point, ctx);
            }
        }
      return;
    }
  else if (ctx->model == MPI_EC_MONTGOMERY)
    {
      unsigned int nbits;
      int j;
      mpi_point_struct p1_, p2_;
      mpi_point_t q1, q2, prd, sum;
      unsigned long sw;
      mpi_size_t rsize;
      int scalar_copied = 0;

      /* Compute scalar point multiplication with Montgomery Ladder.
         Note that we don't use Y-coordinate in the points at all.
         RESULT->Y will be filled by zero.  */

      nbits = mpi_get_nbits (scalar);
      point_init (&p1);
      point_init (&p2);
      point_init (&p1_);
      point_init (&p2_);
      mpi_set_ui (p1.x, 1);
      mpi_free (p2.x);
      p2.x  = mpi_copy (point->x);
      mpi_set_ui (p2.z, 1);

      if (mpi_is_opaque (scalar))
        {
          const unsigned int pbits = ctx->nbits;
          gcry_mpi_t a;
          unsigned int n;
          unsigned char *raw;

          scalar_copied = 1;

          raw = _gcry_mpi_get_opaque_copy (scalar, &n);
          if ((n+7)/8 != (pbits+7)/8)
            log_fatal ("scalar size (%d) != prime size (%d)\n",
                       (n+7)/8, (pbits+7)/8);

          reverse_buffer (raw, (n+7)/8);
          if ((pbits % 8))
            raw[0] &= (1 << (pbits % 8)) - 1;
          raw[0] |= (1 << ((pbits + 7) % 8));
          raw[(pbits+7)/8 - 1] &= (256 - ctx->h);
          a = mpi_is_secure (scalar) ? mpi_snew (pbits): mpi_new (pbits);
          _gcry_mpi_set_buffer (a, raw, (n+7)/8, 0);
          xfree (raw);

          scalar = a;
        }

      mpi_point_resize (&p1, ctx);
      mpi_point_resize (&p2, ctx);
      mpi_point_resize (&p1_, ctx);
      mpi_point_resize (&p2_, ctx);

      mpi_resize (point->x, ctx->p->nlimbs);
      point->x->nlimbs = ctx->p->nlimbs;

      q1 = &p1;
      q2 = &p2;
      prd = &p1_;
      sum = &p2_;

      for (j=nbits-1; j >= 0; j--)
        {
          mpi_point_t t;

          sw = mpi_test_bit (scalar, j);
          point_swap_cond (q1, q2, sw, ctx);
          montgomery_ladder (prd, sum, q1, q2, point->x, ctx);
          point_swap_cond (prd, sum, sw, ctx);
          t = q1;  q1 = prd;  prd = t;
          t = q2;  q2 = sum;  sum = t;
        }

      mpi_clear (result->y);
      sw = (nbits & 1);
      point_swap_cond (&p1, &p1_, sw, ctx);

      rsize = p1.z->nlimbs;
      MPN_NORMALIZE (p1.z->d, rsize);
      if (rsize == 0)
        {
          mpi_set_ui (result->x, 1);
          mpi_set_ui (result->z, 0);
        }
      else
        {
          z1 = mpi_new (0);
          ec_invm (z1, p1.z, ctx);
          ec_mulm (result->x, p1.x, z1, ctx);
          mpi_set_ui (result->z, 1);
          mpi_free (z1);
        }

      point_free (&p1);
      point_free (&p2);
      point_free (&p1_);
      point_free (&p2_);
      if (scalar_copied)
        _gcry_mpi_release (scalar);
      return;
    }

  x1 = mpi_alloc_like (ctx->p);
  y1 = mpi_alloc_like (ctx->p);
  h  = mpi_alloc_like (ctx->p);
  k  = mpi_copy (scalar);
  yy = mpi_copy (point->y);

  if ( mpi_has_sign (k) )
    {
      k->sign = 0;
      ec_invm (yy, yy, ctx);
    }

  if (!mpi_cmp_ui (point->z, 1))
    {
      mpi_set (x1, point->x);
      mpi_set (y1, yy);
    }
  else
    {
      gcry_mpi_t z2, z3;

      z2 = mpi_alloc_like (ctx->p);
      z3 = mpi_alloc_like (ctx->p);
      ec_mulm (z2, point->z, point->z, ctx);
      ec_mulm (z3, point->z, z2, ctx);
      ec_invm (z2, z2, ctx);
      ec_mulm (x1, point->x, z2, ctx);
      ec_invm (z3, z3, ctx);
      ec_mulm (y1, yy, z3, ctx);
      mpi_free (z2);
      mpi_free (z3);
    }
  z1 = mpi_copy (mpi_const (MPI_C_ONE));

  mpi_mul (h, k, mpi_const (MPI_C_THREE)); /* h = 3k */
  loops = mpi_get_nbits (h);
  if (loops < 2)
    {
      /* If SCALAR is zero, the above mpi_mul sets H to zero and thus
         LOOPs will be zero.  To avoid an underflow of I in the main
         loop we set LOOP to 2 and the result to (0,0,0).  */
      loops = 2;
      mpi_clear (result->x);
      mpi_clear (result->y);
      mpi_clear (result->z);
    }
  else
    {
      mpi_set (result->x, point->x);
      mpi_set (result->y, yy);
      mpi_set (result->z, point->z);
    }
  mpi_free (yy); yy = NULL;

  p1.x = x1; x1 = NULL;
  p1.y = y1; y1 = NULL;
  p1.z = z1; z1 = NULL;
  point_init (&p2);
  point_init (&p1inv);

  /* Invert point: y = p - y mod p  */
  point_set (&p1inv, &p1);
  ec_subm (p1inv.y, ctx->p, p1inv.y, ctx);

  for (i=loops-2; i > 0; i--)
    {
      _gcry_mpi_ec_dup_point (result, result, ctx);
      if (mpi_test_bit (h, i) == 1 && mpi_test_bit (k, i) == 0)
        {
          point_set (&p2, result);
          _gcry_mpi_ec_add_points (result, &p2, &p1, ctx);
        }
      if (mpi_test_bit (h, i) == 0 && mpi_test_bit (k, i) == 1)
        {
          point_set (&p2, result);
          _gcry_mpi_ec_add_points (result, &p2, &p1inv, ctx);
        }
    }

  point_free (&p1);
  point_free (&p2);
  point_free (&p1inv);
  mpi_free (h);
  mpi_free (k);
}


/* Return true if POINT is on the curve described by CTX.  */
int
_gcry_mpi_ec_curve_point (gcry_mpi_point_t point, mpi_ec_t ctx)
{
  int res = 0;
  gcry_mpi_t x, y, w;

  x = mpi_new (0);
  y = mpi_new (0);
  w = mpi_new (0);

  /* Check that the point is in range.  This needs to be done here and
   * not after conversion to affine coordinates.  */
  if (mpi_cmpabs (point->x, ctx->p) >= 0)
    goto leave;
  if (mpi_cmpabs (point->y, ctx->p) >= 0)
    goto leave;
  if (mpi_cmpabs (point->z, ctx->p) >= 0)
    goto leave;

  switch (ctx->model)
    {
    case MPI_EC_WEIERSTRASS:
      {
        gcry_mpi_t xxx;

        if (_gcry_mpi_ec_get_affine (x, y, point, ctx))
          goto leave;

        xxx = mpi_new (0);

        /* y^2 == x^3 + a·x + b */
        ec_pow2 (y, y, ctx);

        ec_pow3 (xxx, x, ctx);
        ec_mulm (w, ctx->a, x, ctx);
        ec_addm (w, w, ctx->b, ctx);
        ec_addm (w, w, xxx, ctx);

        if (!mpi_cmp (y, w))
          res = 1;

        _gcry_mpi_release (xxx);
      }
      break;
    case MPI_EC_MONTGOMERY:
      {
#define xx y
        /* With Montgomery curve, only X-coordinate is valid.  */
        if (_gcry_mpi_ec_get_affine (x, NULL, point, ctx))
          goto leave;

        /* The equation is: b * y^2 == x^3 + a · x^2 + x */
        /* We check if right hand is quadratic residue or not by
           Euler's criterion.  */
        /* CTX->A has (a-2)/4 and CTX->B has b^-1 */
        ec_mulm (w, ctx->a, mpi_const (MPI_C_FOUR), ctx);
        ec_addm (w, w, mpi_const (MPI_C_TWO), ctx);
        ec_mulm (w, w, x, ctx);
        ec_pow2 (xx, x, ctx);
        ec_addm (w, w, xx, ctx);
        ec_addm (w, w, mpi_const (MPI_C_ONE), ctx);
        ec_mulm (w, w, x, ctx);
        ec_mulm (w, w, ctx->b, ctx);
#undef xx
        /* Compute Euler's criterion: w^(p-1)/2 */
#define p_minus1 y
        ec_subm (p_minus1, ctx->p, mpi_const (MPI_C_ONE), ctx);
        mpi_rshift (p_minus1, p_minus1, 1);
        ec_powm (w, w, p_minus1, ctx);

        res = !mpi_cmp_ui (w, 1);
#undef p_minus1
      }
      break;
    case MPI_EC_EDWARDS:
      {
        if (_gcry_mpi_ec_get_affine (x, y, point, ctx))
          goto leave;

        mpi_resize (w, ctx->p->nlimbs);
        w->nlimbs = ctx->p->nlimbs;

        /* a · x^2 + y^2 - 1 - b · x^2 · y^2 == 0 */
        ctx->pow2 (x, x, ctx);
        ctx->pow2 (y, y, ctx);
        if (ctx->dialect == ECC_DIALECT_ED25519)
          ctx->subm (w, ctx->p, x, ctx);
        else
          ctx->mulm (w, ctx->a, x, ctx);
        ctx->addm (w, w, y, ctx);
        ctx->mulm (x, x, y, ctx);
        ctx->mulm (x, x, ctx->b, ctx);
        ctx->subm (w, w, x, ctx);
        if (!mpi_cmp_ui (w, 1))
          res = 1;
      }
      break;
    }

 leave:
  _gcry_mpi_release (w);
  _gcry_mpi_release (x);
  _gcry_mpi_release (y);

  return res;
}


int
_gcry_mpi_ec_bad_point (gcry_mpi_point_t point, mpi_ec_t ctx)
{
  int i;
  gcry_mpi_t x_bad;

  for (i = 0; (x_bad = ctx->t.scratch[i]); i++)
    if (!mpi_cmp (point->x, x_bad))
      return 1;

  return 0;
}
