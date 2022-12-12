/* gmp-glue.c

   Copyright (C) 2013 Niels Möller
   Copyright (C) 2013 Red Hat

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

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <assert.h>
#include <stdlib.h>

#include "gmp-glue.h"

#if NETTLE_USE_MINI_GMP
mp_limb_t
mpn_cnd_add_n (mp_limb_t cnd, mp_limb_t *rp,
	       const mp_limb_t *ap, const mp_limb_t *bp, mp_size_t n)
{
  mp_limb_t cy, mask;
  mp_size_t  i;

  mask = -(mp_limb_t) (cnd != 0);

  for (i = 0, cy = 0; i < n; i++)
    {
      mp_limb_t rl = ap[i] + cy;
      mp_limb_t bl = bp[i] & mask;
      cy = (rl < cy);
      rl += bl;
      cy += (rl < bl);
      rp[i] = rl;
    }
  return cy;
}

mp_limb_t
mpn_cnd_sub_n (mp_limb_t cnd, mp_limb_t *rp,
	       const mp_limb_t *ap, const mp_limb_t *bp, mp_size_t n)
{
  mp_limb_t cy, mask;
  mp_size_t  i;

  mask = -(mp_limb_t) (cnd != 0);

  for (i = 0, cy = 0; i < n; i++)
    {
      mp_limb_t al = ap[i];
      mp_limb_t bl = bp[i] & mask;
      mp_limb_t sl;
      sl = al - cy;
      cy = (al < cy) + (sl < bl);
      sl -= bl;
      rp[i] = sl;
    }
  return cy;
}

void
mpn_cnd_swap (mp_limb_t cnd, volatile mp_limb_t *ap, volatile mp_limb_t *bp, mp_size_t n)
{
  volatile mp_limb_t mask = - (mp_limb_t) (cnd != 0);
  mp_size_t i;
  for (i = 0; i < n; i++)
    {
      mp_limb_t a, b, t;
      a = ap[i];
      b = bp[i];
      t = (a ^ b) & mask;
      ap[i] = a ^ t;
      bp[i] = b ^ t;
    }
}

#endif /* NETTLE_USE_MINI_GMP */

int
sec_zero_p (const mp_limb_t *ap, mp_size_t n)
{
  volatile mp_limb_t w;
  mp_size_t i;

  for (i = 0, w = 0; i < n; i++)
    w |= ap[i];

  return w == 0;
}

/* Additional convenience functions. */

void
mpz_limbs_copy (mp_limb_t *xp, mpz_srcptr x, mp_size_t n)
{
  mp_size_t xn = mpz_size (x);

  assert (xn <= n);
  mpn_copyi (xp, mpz_limbs_read (x), xn);
  if (xn < n)
    mpn_zero (xp + xn, n - xn);
}

void
mpz_set_n (mpz_t r, const mp_limb_t *xp, mp_size_t xn)
{
  mpn_copyi (mpz_limbs_write (r, xn), xp, xn);
  mpz_limbs_finish (r, xn);
}

void
mpn_set_base256 (mp_limb_t *rp, mp_size_t rn,
		 const uint8_t *xp, size_t xn)
{
  size_t xi;
  mp_limb_t out;
  unsigned bits;
  for (xi = xn, out = bits = 0; xi > 0 && rn > 0; )
    {
      mp_limb_t in = xp[--xi];
      out |= (in << bits) & GMP_NUMB_MASK;
      bits += 8;
      if (bits >= GMP_NUMB_BITS)
	{
	  *rp++ = out;
	  rn--;

	  bits -= GMP_NUMB_BITS;
	  out = in >> (8 - bits);
	}
    }
  if (rn > 0)
    {
      *rp++ = out;
      if (--rn > 0)
	mpn_zero (rp, rn);
    }
}

void
mpn_set_base256_le (mp_limb_t *rp, mp_size_t rn,
		    const uint8_t *xp, size_t xn)
{
  size_t xi;
  mp_limb_t out;
  unsigned bits;
  for (xi = 0, out = bits = 0; xi < xn && rn > 0; )
    {
      mp_limb_t in = xp[xi++];
      out |= (in << bits) & GMP_NUMB_MASK;
      bits += 8;
      if (bits >= GMP_NUMB_BITS)
	{
	  *rp++ = out;
	  rn--;

	  bits -= GMP_NUMB_BITS;
	  out = in >> (8 - bits);
	}
    }
  if (rn > 0)
    {
      *rp++ = out;
      if (--rn > 0)
	mpn_zero (rp, rn);
    }
}

void
mpn_get_base256 (uint8_t *rp, size_t rn,
		 const mp_limb_t *xp, mp_size_t xn)
{
  unsigned bits;
  mp_limb_t in;
  for (bits = in = 0; xn > 0 && rn > 0; )
    {
      if (bits >= 8)
	{
	  rp[--rn] = in;
	  in >>= 8;
	  bits -= 8;
	}
      else
	{
	  uint8_t old = in;
	  in = *xp++;
	  xn--;
	  rp[--rn] = old | (in << bits);
	  in >>= (8 - bits);
	  bits += GMP_NUMB_BITS - 8;
	}
    }
  while (rn > 0)
    {
      rp[--rn] = in;
      in >>= 8;
    }
}

void
mpn_get_base256_le (uint8_t *rp, size_t rn,
		    const mp_limb_t *xp, mp_size_t xn)
{
  unsigned bits;
  mp_limb_t in;
  for (bits = in = 0; xn > 0 && rn > 0; )
    {
      if (bits >= 8)
	{
	  *rp++ = in;
	  rn--;
	  in >>= 8;
	  bits -= 8;
	}
      else
	{
	  uint8_t old = in;
	  in = *xp++;
	  xn--;
	  *rp++ = old | (in << bits);
	  rn--;
	  in >>= (8 - bits);
	  bits += GMP_NUMB_BITS - 8;
	}
    }
  while (rn > 0)
    {
      *rp++ = in;
      rn--;
      in >>= 8;
    }
}

mp_limb_t *
gmp_alloc_limbs (mp_size_t n)
{

  void *(*alloc_func)(size_t);

  assert (n > 0);

  mp_get_memory_functions (&alloc_func, NULL, NULL);
  return (mp_limb_t *) alloc_func ( (size_t) n * sizeof(mp_limb_t));
}

void
gmp_free_limbs (mp_limb_t *p, mp_size_t n)
{
  void (*free_func)(void *, size_t);
  assert (n > 0);
  assert (p != 0);
  mp_get_memory_functions (NULL, NULL, &free_func);

  free_func (p, (size_t) n * sizeof(mp_limb_t));
}

void *
gmp_alloc(size_t n)
{
  void *(*alloc_func)(size_t);
  assert (n > 0);

  mp_get_memory_functions(&alloc_func, NULL, NULL);

  return alloc_func (n);
}

void
gmp_free(void *p, size_t n)
{
  void (*free_func)(void *, size_t);
  assert (n > 0);
  assert (p != 0);
  mp_get_memory_functions (NULL, NULL, &free_func);

  free_func (p, (size_t) n);
}
