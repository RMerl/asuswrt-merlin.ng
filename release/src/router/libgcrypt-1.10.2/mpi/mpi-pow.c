/* mpi-pow.c  -  MPI functions for exponentiation
 * Copyright (C) 1994, 1996, 1998, 2000, 2002
 *               2003  Free Software Foundation, Inc.
 *               2013  g10 Code GmbH
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
 *
 * Note: This code is heavily based on the GNU MP Library.
 *	 Actually it's the same code with only minor changes in the
 *	 way the data is stored; this is to support the abstraction
 *	 of an optional secure memory allocation which may be used
 *	 to avoid revealing of sensitive data due to paging etc.
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi-internal.h"
#include "longlong.h"


/*
 * When you need old implementation, please add compilation option
 * -DUSE_ALGORITHM_SIMPLE_EXPONENTIATION
 * or expose this line:
#define USE_ALGORITHM_SIMPLE_EXPONENTIATION 1
 */

#if defined(USE_ALGORITHM_SIMPLE_EXPONENTIATION)
/****************
 * RES = BASE ^ EXPO mod MOD
 */
void
_gcry_mpi_powm (gcry_mpi_t res,
                gcry_mpi_t base, gcry_mpi_t expo, gcry_mpi_t mod)
{
  /* Pointer to the limbs of the arguments, their size and signs. */
  mpi_ptr_t  rp, ep, mp, bp;
  mpi_size_t esize, msize, bsize, rsize;
  int               msign, bsign, rsign;
  /* Flags telling the secure allocation status of the arguments.  */
  int        esec,  msec,  bsec;
  /* Size of the result including space for temporary values.  */
  mpi_size_t size;
  /* Helper.  */
  int mod_shift_cnt;
  int negative_result;
  mpi_ptr_t mp_marker = NULL;
  mpi_ptr_t bp_marker = NULL;
  mpi_ptr_t ep_marker = NULL;
  mpi_ptr_t xp_marker = NULL;
  unsigned int mp_nlimbs = 0;
  unsigned int bp_nlimbs = 0;
  unsigned int ep_nlimbs = 0;
  unsigned int xp_nlimbs = 0;
  mpi_ptr_t tspace = NULL;
  mpi_size_t tsize = 0;


  esize = expo->nlimbs;
  msize = mod->nlimbs;
  size = 2 * msize;
  msign = mod->sign;

  esec = mpi_is_secure(expo);
  msec = mpi_is_secure(mod);
  bsec = mpi_is_secure(base);

  rp = res->d;
  ep = expo->d;
  MPN_NORMALIZE(ep, esize);

  if (!msize)
    _gcry_divide_by_zero();

  if (!esize)
    {
      /* Exponent is zero, result is 1 mod MOD, i.e., 1 or 0 depending
         on if MOD equals 1.  */
      res->nlimbs = (msize == 1 && mod->d[0] == 1) ? 0 : 1;
      if (res->nlimbs)
        {
          RESIZE_IF_NEEDED (res, 1);
          rp = res->d;
          rp[0] = 1;
        }
      res->sign = 0;
      goto leave;
    }

  /* Normalize MOD (i.e. make its most significant bit set) as
     required by mpn_divrem.  This will make the intermediate values
     in the calculation slightly larger, but the correct result is
     obtained after a final reduction using the original MOD value. */
  mp_nlimbs = msec? msize:0;
  mp = mp_marker = mpi_alloc_limb_space(msize, msec);
  count_leading_zeros (mod_shift_cnt, mod->d[msize-1]);
  if (mod_shift_cnt)
    _gcry_mpih_lshift (mp, mod->d, msize, mod_shift_cnt);
  else
    MPN_COPY( mp, mod->d, msize );

  bsize = base->nlimbs;
  bsign = base->sign;
  if (bsize > msize)
    {
      /* The base is larger than the module.  Reduce it.

         Allocate (BSIZE + 1) with space for remainder and quotient.
         (The quotient is (bsize - msize + 1) limbs.)  */
      bp_nlimbs = bsec ? (bsize + 1):0;
      bp = bp_marker = mpi_alloc_limb_space( bsize + 1, bsec );
      MPN_COPY ( bp, base->d, bsize );
      /* We don't care about the quotient, store it above the
       * remainder, at BP + MSIZE.  */
      _gcry_mpih_divrem( bp + msize, 0, bp, bsize, mp, msize );
      bsize = msize;
      /* Canonicalize the base, since we are going to multiply with it
	 quite a few times.  */
      MPN_NORMALIZE( bp, bsize );
    }
  else
    bp = base->d;

  if (!bsize)
    {
      res->nlimbs = 0;
      res->sign = 0;
      goto leave;
    }


  /* Make BASE, EXPO and MOD not overlap with RES.  */
  if ( rp == bp )
    {
      /* RES and BASE are identical.  Allocate temp. space for BASE.  */
      gcry_assert (!bp_marker);
      bp_nlimbs = bsec? bsize:0;
      bp = bp_marker = mpi_alloc_limb_space( bsize, bsec );
      MPN_COPY(bp, rp, bsize);
    }
  if ( rp == ep )
    {
      /* RES and EXPO are identical.  Allocate temp. space for EXPO.  */
      ep_nlimbs = esec? esize:0;
      ep = ep_marker = mpi_alloc_limb_space( esize, esec );
      MPN_COPY(ep, rp, esize);
    }
  if ( rp == mp )
    {
      /* RES and MOD are identical.  Allocate temporary space for MOD.*/
      gcry_assert (!mp_marker);
      mp_nlimbs = msec?msize:0;
      mp = mp_marker = mpi_alloc_limb_space( msize, msec );
      MPN_COPY(mp, rp, msize);
    }

  /* Copy base to the result.  */
  if (res->alloced < size)
    {
      mpi_resize (res, size);
      rp = res->d;
    }
  MPN_COPY ( rp, bp, bsize );
  rsize = bsize;
  rsign = 0;

  /* Main processing.  */
  {
    mpi_size_t i;
    mpi_ptr_t xp;
    int c;
    mpi_limb_t e;
    mpi_limb_t carry_limb;
    struct karatsuba_ctx karactx;
    struct gcry_mpi w, u;

    xp_nlimbs = msec? size:0;
    xp = xp_marker = mpi_alloc_limb_space( size, msec );

    w.sign = u.sign = 0;
    w.flags = u.flags = 0;
    w.alloced = w.nlimbs = size; /* RES->alloc may be longer.  */
    u.alloced = u.nlimbs = size;

    memset( &karactx, 0, sizeof karactx );
    negative_result = (ep[0] & 1) && bsign;

    i = esize - 1;
    e = ep[i];
    count_leading_zeros (c, e);
    e = (e << c) << 1;     /* Shift the expo bits to the left, lose msb.  */
    c = BITS_PER_MPI_LIMB - 1 - c;

    /* Main loop.

       Make the result be pointed to alternately by XP and RP.  This
       helps us avoid block copying, which would otherwise be
       necessary with the overlap restrictions of
       _gcry_mpih_divmod. With 50% probability the result after this
       loop will be in the area originally pointed by RP (==RES->d),
       and with 50% probability in the area originally pointed to by XP. */
    for (;;)
      {
        while (c)
          {
            mpi_ptr_t tp;
            mpi_size_t xsize;

            /*mpih_mul_n(xp, rp, rp, rsize);*/
            if ( rsize < KARATSUBA_THRESHOLD )
              _gcry_mpih_sqr_n_basecase( xp, rp, rsize );
            else
              {
                if ( !tspace )
                  {
                    tsize = 2 * rsize;
                    tspace = mpi_alloc_limb_space( tsize, 0 );
                  }
                else if ( tsize < (2*rsize) )
                  {
                    _gcry_mpi_free_limb_space (tspace, 0);
                    tsize = 2 * rsize;
                    tspace = mpi_alloc_limb_space (tsize, 0 );
                  }
                _gcry_mpih_sqr_n (xp, rp, rsize, tspace);
              }

            xsize = 2 * rsize;
            if ( xsize > msize )
              {
                _gcry_mpih_divrem(xp + msize, 0, xp, xsize, mp, msize);
                xsize = msize;
              }

            tp = rp; rp = xp; xp = tp;
            rsize = xsize;

            /* To mitigate the Yarom/Falkner flush+reload cache
             * side-channel attack on the RSA secret exponent, we do
             * the multiplication regardless of the value of the
             * high-bit of E.  But to avoid this performance penalty
             * we do it only if the exponent has been stored in secure
             * memory and we can thus assume it is a secret exponent.  */
            if (esec || (mpi_limb_signed_t)e < 0)
              {
                /*mpih_mul( xp, rp, rsize, bp, bsize );*/
                if( bsize < KARATSUBA_THRESHOLD )
                  _gcry_mpih_mul ( xp, rp, rsize, bp, bsize );
                else
                  _gcry_mpih_mul_karatsuba_case (xp, rp, rsize, bp, bsize,
                                                 &karactx);

                xsize = rsize + bsize;
                if ( xsize > msize )
                  {
                    _gcry_mpih_divrem(xp + msize, 0, xp, xsize, mp, msize);
                    xsize = msize;
                  }
              }

            w.d = rp;
            u.d = xp;
            mpi_set_cond (&w, &u, ((mpi_limb_signed_t)e < 0));

            e <<= 1;
            c--;
          }

        i--;
        if ( i < 0 )
          break;
        e = ep[i];
        c = BITS_PER_MPI_LIMB;
      }

    /* We shifted MOD, the modulo reduction argument, left
       MOD_SHIFT_CNT steps.  Adjust the result by reducing it with the
       original MOD.

       Also make sure the result is put in RES->d (where it already
       might be, see above).  */
    if ( mod_shift_cnt )
      {
        carry_limb = _gcry_mpih_lshift( res->d, rp, rsize, mod_shift_cnt);
        rp = res->d;
        if ( carry_limb )
          {
            rp[rsize] = carry_limb;
            rsize++;
          }
      }
    else if (res->d != rp)
      {
        MPN_COPY (res->d, rp, rsize);
        rp = res->d;
      }

    if ( rsize >= msize )
      {
        _gcry_mpih_divrem(rp + msize, 0, rp, rsize, mp, msize);
        rsize = msize;
      }

    /* Remove any leading zero words from the result.  */
    if ( mod_shift_cnt )
      _gcry_mpih_rshift( rp, rp, rsize, mod_shift_cnt);
    MPN_NORMALIZE (rp, rsize);

    _gcry_mpih_release_karatsuba_ctx (&karactx );
  }

  /* Fixup for negative results.  */
  if ( negative_result && rsize )
    {
      if ( mod_shift_cnt )
        _gcry_mpih_rshift( mp, mp, msize, mod_shift_cnt);
      _gcry_mpih_sub( rp, mp, msize, rp, rsize);
      rsize = msize;
      rsign = msign;
      MPN_NORMALIZE(rp, rsize);
    }
  gcry_assert (res->d == rp);
  res->nlimbs = rsize;
  res->sign = rsign;

 leave:
  if (mp_marker)
    _gcry_mpi_free_limb_space( mp_marker, mp_nlimbs );
  if (bp_marker)
    _gcry_mpi_free_limb_space( bp_marker, bp_nlimbs );
  if (ep_marker)
    _gcry_mpi_free_limb_space( ep_marker, ep_nlimbs );
  if (xp_marker)
    _gcry_mpi_free_limb_space( xp_marker, xp_nlimbs );
  if (tspace)
    _gcry_mpi_free_limb_space( tspace, 0 );
}
#else
/**
 * Internal function to compute
 *
 *    X = R * S mod M
 *
 * and set the size of X at the pointer XSIZE_P.
 * Use karatsuba structure at KARACTX_P.
 *
 * Condition:
 *   RSIZE >= SSIZE
 *   Enough space for X is allocated beforehand.
 *
 * For generic cases, we can/should use gcry_mpi_mulm.
 * This function is use for specific internal case.
 */
static void
mul_mod (mpi_ptr_t xp, mpi_size_t *xsize_p,
         mpi_ptr_t rp, mpi_size_t rsize,
         mpi_ptr_t sp, mpi_size_t ssize,
         mpi_ptr_t mp, mpi_size_t msize,
         struct karatsuba_ctx *karactx_p)
{
  if( ssize < KARATSUBA_THRESHOLD )
    _gcry_mpih_mul ( xp, rp, rsize, sp, ssize );
  else
    _gcry_mpih_mul_karatsuba_case (xp, rp, rsize, sp, ssize, karactx_p);

   if (rsize + ssize > msize)
    {
      _gcry_mpih_divrem (xp + msize, 0, xp, rsize + ssize, mp, msize);
      *xsize_p = msize;
    }
   else
     *xsize_p = rsize + ssize;
}

#define SIZE_PRECOMP ((1 << (5 - 1)))

/****************
 * RES = BASE ^ EXPO mod MOD
 *
 * To mitigate the Yarom/Falkner flush+reload cache side-channel
 * attack on the RSA secret exponent, we don't use the square
 * routine but multiplication.
 *
 * Reference:
 *   Handbook of Applied Cryptography
 *       Algorithm 14.83: Modified left-to-right k-ary exponentiation
 */
void
_gcry_mpi_powm (gcry_mpi_t res,
                gcry_mpi_t base, gcry_mpi_t expo, gcry_mpi_t mod)
{
  /* Pointer to the limbs of the arguments, their size and signs. */
  mpi_ptr_t  rp, ep, mp, bp;
  mpi_size_t esize, msize, bsize, rsize;
  int               msign, bsign, rsign;
  /* Flags telling the secure allocation status of the arguments.  */
  int        esec,  msec,  bsec;
  /* Size of the result including space for temporary values.  */
  mpi_size_t size;
  /* Helper.  */
  int mod_shift_cnt;
  int negative_result;
  mpi_ptr_t mp_marker = NULL;
  mpi_ptr_t bp_marker = NULL;
  mpi_ptr_t ep_marker = NULL;
  mpi_ptr_t xp_marker = NULL;
  unsigned int mp_nlimbs = 0;
  unsigned int bp_nlimbs = 0;
  unsigned int ep_nlimbs = 0;
  unsigned int xp_nlimbs = 0;
  mpi_ptr_t precomp[SIZE_PRECOMP]; /* Pre-computed array: BASE^1, ^3, ^5, ... */
  mpi_size_t precomp_size[SIZE_PRECOMP];
  mpi_size_t W;
  mpi_ptr_t base_u;
  mpi_size_t base_u_size;
  mpi_size_t max_u_size;

  esize = expo->nlimbs;
  msize = mod->nlimbs;
  size = 2 * msize;
  msign = mod->sign;

  ep = expo->d;
  MPN_NORMALIZE(ep, esize);

  if (esize * BITS_PER_MPI_LIMB > 512)
    W = 5;
  else if (esize * BITS_PER_MPI_LIMB > 256)
    W = 4;
  else if (esize * BITS_PER_MPI_LIMB > 128)
    W = 3;
  else if (esize * BITS_PER_MPI_LIMB > 64)
    W = 2;
  else
    W = 1;

  esec = mpi_is_secure(expo);
  msec = mpi_is_secure(mod);
  bsec = mpi_is_secure(base);

  rp = res->d;

  if (!msize)
    _gcry_divide_by_zero();

  if (!esize)
    {
      /* Exponent is zero, result is 1 mod MOD, i.e., 1 or 0 depending
         on if MOD equals 1.  */
      res->nlimbs = (msize == 1 && mod->d[0] == 1) ? 0 : 1;
      if (res->nlimbs)
        {
          RESIZE_IF_NEEDED (res, 1);
          rp = res->d;
          rp[0] = 1;
        }
      res->sign = 0;
      goto leave;
    }

  /* Normalize MOD (i.e. make its most significant bit set) as
     required by mpn_divrem.  This will make the intermediate values
     in the calculation slightly larger, but the correct result is
     obtained after a final reduction using the original MOD value. */
  mp_nlimbs = msec? msize:0;
  mp = mp_marker = mpi_alloc_limb_space(msize, msec);
  count_leading_zeros (mod_shift_cnt, mod->d[msize-1]);
  if (mod_shift_cnt)
    _gcry_mpih_lshift (mp, mod->d, msize, mod_shift_cnt);
  else
    MPN_COPY( mp, mod->d, msize );

  bsize = base->nlimbs;
  bsign = base->sign;
  if (bsize > msize)
    {
      /* The base is larger than the module.  Reduce it.

         Allocate (BSIZE + 1) with space for remainder and quotient.
         (The quotient is (bsize - msize + 1) limbs.)  */
      bp_nlimbs = bsec ? (bsize + 1):0;
      bp = bp_marker = mpi_alloc_limb_space( bsize + 1, bsec );
      MPN_COPY ( bp, base->d, bsize );
      /* We don't care about the quotient, store it above the
       * remainder, at BP + MSIZE.  */
      _gcry_mpih_divrem( bp + msize, 0, bp, bsize, mp, msize );
      bsize = msize;
      /* Canonicalize the base, since we are going to multiply with it
         quite a few times.  */
      MPN_NORMALIZE( bp, bsize );
    }
  else
    bp = base->d;

  if (!bsize)
    {
      res->nlimbs = 0;
      res->sign = 0;
      goto leave;
    }


  /* Make BASE, EXPO not overlap with RES.  We don't need to check MOD
     because that has already been copied to the MP var.  */
  if ( rp == bp )
    {
      /* RES and BASE are identical.  Allocate temp. space for BASE.  */
      gcry_assert (!bp_marker);
      bp_nlimbs = bsec? bsize:0;
      bp = bp_marker = mpi_alloc_limb_space( bsize, bsec );
      MPN_COPY(bp, rp, bsize);
    }
  if ( rp == ep )
    {
      /* RES and EXPO are identical.  Allocate temp. space for EXPO.  */
      ep_nlimbs = esec? esize:0;
      ep = ep_marker = mpi_alloc_limb_space( esize, esec );
      MPN_COPY(ep, rp, esize);
    }

  /* Copy base to the result.  */
  if (res->alloced < size)
    {
      mpi_resize (res, size);
      rp = res->d;
    }

  /* Main processing.  */
  {
    mpi_size_t i, j, k;
    mpi_ptr_t xp;
    mpi_size_t xsize;
    int c;
    mpi_limb_t e;
    mpi_limb_t carry_limb;
    struct karatsuba_ctx karactx;
    mpi_ptr_t tp;

    xp_nlimbs = msec? size:0;
    xp = xp_marker = mpi_alloc_limb_space( size, msec );

    memset( &karactx, 0, sizeof karactx );
    negative_result = (ep[0] & 1) && bsign;

    /* Precompute PRECOMP[], BASE^(2 * i + 1), BASE^1, ^3, ^5, ... */
    if (W > 1)                  /* X := BASE^2 */
      mul_mod (xp, &xsize, bp, bsize, bp, bsize, mp, msize, &karactx);
    base_u = precomp[0] = mpi_alloc_limb_space (bsize, esec);
    base_u_size = max_u_size = precomp_size[0] = bsize;
    MPN_COPY (precomp[0], bp, bsize);
    for (i = 1; i < (1 << (W - 1)); i++)
      {                         /* PRECOMP[i] = BASE^(2 * i + 1) */
        if (xsize >= base_u_size)
          mul_mod (rp, &rsize, xp, xsize, base_u, base_u_size,
                   mp, msize, &karactx);
        else
          mul_mod (rp, &rsize, base_u, base_u_size, xp, xsize,
                   mp, msize, &karactx);
        base_u = precomp[i] = mpi_alloc_limb_space (rsize, esec);
        base_u_size = precomp_size[i] = rsize;
        if (max_u_size < base_u_size)
          max_u_size = base_u_size;
        MPN_COPY (precomp[i], rp, rsize);
      }

    if (msize > max_u_size)
      max_u_size = msize;
    base_u = mpi_alloc_limb_space (max_u_size, esec);
    MPN_ZERO (base_u, max_u_size);

    i = esize - 1;

    /* Main loop.

       Make the result be pointed to alternately by XP and RP.  This
       helps us avoid block copying, which would otherwise be
       necessary with the overlap restrictions of
       _gcry_mpih_divmod. With 50% probability the result after this
       loop will be in the area originally pointed by RP (==RES->d),
       and with 50% probability in the area originally pointed to by XP. */
    rsign = 0;
    if (W == 1)
      {
        rsize = bsize;
      }
    else
      {
        rsize = msize;
        MPN_ZERO (rp, rsize);
      }
    MPN_COPY ( rp, bp, bsize );

    e = ep[i];
    count_leading_zeros (c, e);
    e = (e << c) << 1;
    c = BITS_PER_MPI_LIMB - 1 - c;

    j = 0;

    for (;;)
      if (e == 0)
        {
          j += c;
          if ( --i < 0 )
            break;

          e = ep[i];
          c = BITS_PER_MPI_LIMB;
        }
      else
        {
          int c0;
          mpi_limb_t e0;
          struct gcry_mpi w, u;
          w.sign = u.sign = 0;
          w.flags = u.flags = 0;
          w.d = base_u;

          count_leading_zeros (c0, e);
          e = (e << c0);
          c -= c0;
          j += c0;

          e0 = (e >> (BITS_PER_MPI_LIMB - W));
          if (c >= W)
            c0 = 0;
          else
            {
              if ( --i < 0 )
                {
                  e0 = (e >> (BITS_PER_MPI_LIMB - c));
                  j += c - W;
                  goto last_step;
                }
              else
                {
                  c0 = c;
                  e = ep[i];
                  c = BITS_PER_MPI_LIMB;
                  e0 |= (e >> (BITS_PER_MPI_LIMB - (W - c0)));
                }
            }

          e = e << (W - c0);
          c -= (W - c0);

        last_step:
          count_trailing_zeros (c0, e0);
          e0 = (e0 >> c0) >> 1;

          for (j += W - c0; j >= 0; j--)
            {

              /*
               *  base_u <= precomp[e0]
               *  base_u_size <= precomp_size[e0]
               */
              base_u_size = 0;
              for (k = 0; k < (1<< (W - 1)); k++)
                {
                  w.alloced = w.nlimbs = precomp_size[k];
                  u.alloced = u.nlimbs = precomp_size[k];
                  u.d = precomp[k];

                  mpi_set_cond (&w, &u, k == e0);
                  base_u_size |= ( precomp_size[k] & (0UL - (k == e0)) );
                }

              w.alloced = w.nlimbs = rsize;
              u.alloced = u.nlimbs = rsize;
              u.d = rp;
              mpi_set_cond (&w, &u, j != 0);
              base_u_size ^= ((base_u_size ^ rsize)  & (0UL - (j != 0)));

              mul_mod (xp, &xsize, rp, rsize, base_u, base_u_size,
                       mp, msize, &karactx);
              tp = rp; rp = xp; xp = tp;
              rsize = xsize;
            }

          j = c0;
          if ( i < 0 )
            break;
        }

    while (j--)
      {
        mul_mod (xp, &xsize, rp, rsize, rp, rsize, mp, msize, &karactx);
        tp = rp; rp = xp; xp = tp;
        rsize = xsize;
      }

    /* We shifted MOD, the modulo reduction argument, left
       MOD_SHIFT_CNT steps.  Adjust the result by reducing it with the
       original MOD.

       Also make sure the result is put in RES->d (where it already
       might be, see above).  */
    if ( mod_shift_cnt )
      {
        carry_limb = _gcry_mpih_lshift( res->d, rp, rsize, mod_shift_cnt);
        rp = res->d;
        if ( carry_limb )
          {
            rp[rsize] = carry_limb;
            rsize++;
          }
      }
    else if (res->d != rp)
      {
        MPN_COPY (res->d, rp, rsize);
        rp = res->d;
      }

    if ( rsize >= msize )
      {
        _gcry_mpih_divrem(rp + msize, 0, rp, rsize, mp, msize);
        rsize = msize;
      }

    /* Remove any leading zero words from the result.  */
    if ( mod_shift_cnt )
      _gcry_mpih_rshift( rp, rp, rsize, mod_shift_cnt);
    MPN_NORMALIZE (rp, rsize);

    _gcry_mpih_release_karatsuba_ctx (&karactx );
    for (i = 0; i < (1 << (W - 1)); i++)
      _gcry_mpi_free_limb_space( precomp[i], esec ? precomp_size[i] : 0 );
    _gcry_mpi_free_limb_space (base_u, esec ? max_u_size : 0);
  }

  /* Fixup for negative results.  */
  if ( negative_result && rsize )
    {
      if ( mod_shift_cnt )
        _gcry_mpih_rshift( mp, mp, msize, mod_shift_cnt);
      _gcry_mpih_sub( rp, mp, msize, rp, rsize);
      rsize = msize;
      rsign = msign;
      MPN_NORMALIZE(rp, rsize);
    }
  gcry_assert (res->d == rp);
  res->nlimbs = rsize;
  res->sign = rsign;

 leave:
  if (mp_marker)
    _gcry_mpi_free_limb_space( mp_marker, mp_nlimbs );
  if (bp_marker)
    _gcry_mpi_free_limb_space( bp_marker, bp_nlimbs );
  if (ep_marker)
    _gcry_mpi_free_limb_space( ep_marker, ep_nlimbs );
  if (xp_marker)
    _gcry_mpi_free_limb_space( xp_marker, xp_nlimbs );
}
#endif
