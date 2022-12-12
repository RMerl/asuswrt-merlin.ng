/* ecc-sqrt.c

   Copyright (C) 2014 Niels Möller

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

#include "testutils.h"

#define COUNT 5000

#if NETTLE_USE_MINI_GMP
/* Implements Legendre symbol only, requiring that p is an odd prime */
static int
mpz_ui_kronecker (mp_limb_t ul, const mpz_t p)
{
  mpz_t t, u;
  int r;

  mpz_init_set_ui (u, ul);
  mpz_init_set (t, p);
  mpz_sub_ui (t, t, 1);
  mpz_tdiv_q_2exp (t, t, 1);
  mpz_powm (t, u, t, p);

  r = mpz_cmp_ui (t, 1);
  if (r < 0)
    r = 0;
  else if (r == 0)
    r = 1;
  else
    {
      mpz_sub (t, p, t);
      ASSERT (mpz_cmp_ui (t, 1) == 0);
      r = -1;
    }
  mpz_clear (t);
  mpz_clear (u);

  return r;
}
#endif /* NETTLE_USE_MINI_GMP */

static void
test_sqrt (gmp_randstate_t rands, const struct ecc_modulo *m, int use_redc)
{
  mpz_t u;
  mpz_t p;
  mpz_t r;
  mpz_t t;

  unsigned z, i;
  mp_limb_t *up;
  mp_limb_t *rp;
  mp_limb_t *scratch;

  mpz_init (u);
  mpz_init (t);

  mpz_roinit_n (p, m->m, m->size);

  up = xalloc_limbs (m->size);
  rp = xalloc_limbs (m->size);
  scratch = xalloc_limbs (m->sqrt_itch);

  /* Check behaviour for zero input */
  mpn_zero (up, m->size);
  memset (rp, 17, m->size * sizeof(*rp));
  if (!m->sqrt (m, rp, up, scratch))
    {
      fprintf (stderr, "m->sqrt returned failure for zero input, bit_size = %d\n",
	       m->bit_size);
      abort();
    }
  if (!ecc_mod_zero_p (m, rp))
    {
      fprintf (stderr, "m->sqrt failed for zero input (bit size %u):\n",
	       m->bit_size);
      fprintf (stderr, "r = ");
      mpn_out_str (stderr, 16, rp, m->size);
      fprintf (stderr, " (bad)\n");
      abort ();
    }

  /* Find a non-square */
  for (z = 2; mpz_ui_kronecker (z, p) != -1; z++)
    ;

  if (verbose)
    fprintf(stderr, "test_sqrt on %d-bit modulo. Non square: %d\n", m->bit_size, z);

  for (i = 0; i < COUNT; i++)
    {
      if (i & 1)
	mpz_rrandomb (u, rands, m->bit_size);
      else
	mpz_urandomb (u, rands, m->bit_size);

      if (use_redc)
	{
	  /* We get non-redc sqrt if we reduce u before calling m->sqrt */
	  mpz_limbs_copy (scratch, u, m->size);
	  mpn_zero (scratch + m->size, m->size);
	  m->reduce (m, up, scratch);
	}
      else
	{
	  mpz_limbs_copy (up, u, m->size);
	}
      if (!m->sqrt (m, rp, up, scratch))
	{
	  mpz_mul_ui (u, u, z);
	  mpz_mod (u, u, p);
	  ecc_mod_mul_1 (m, up, up, z);

	  if (!m->sqrt (m, rp, up, scratch))
	    {
	      fprintf (stderr, "m->sqrt returned failure, bit_size = %d\n"
		       "u = 0x",
		       m->bit_size);
	      mpz_out_str (stderr, 16, u);
	      fprintf (stderr, "\n");
	      abort ();
	    }
	}
      /* Check that r^2 = u */
      mpz_roinit_n (r, rp, m->size);
      mpz_mul (t, r, r);
      if (!mpz_congruent_p (t, u, p))
	{
	  fprintf (stderr, "m->sqrt gave incorrect result, bit_size = %d\n"
		   "u = 0x",
		   m->bit_size);
	  mpz_out_str (stderr, 16, u);
	  fprintf (stderr, "\nr = 0x");
	  mpz_out_str (stderr, 16, r);
	  fprintf (stderr, "\n");
	  abort ();
	}
    }
  mpz_clear (u);
  mpz_clear (t);
  free (up);
  free (rp);
  free (scratch);
}

static void
test_sqrt_ratio (gmp_randstate_t rands, const struct ecc_modulo *m)
{
  mpz_t u;
  mpz_t v;
  mpz_t p;
  mpz_t r;
  mpz_t t;

  unsigned z, i;
  mp_limb_t *up;
  mp_limb_t *vp;
  mp_limb_t *rp;
  mp_limb_t *scratch;

  mpz_init (u);
  mpz_init (v);
  mpz_init (t);

  mpz_roinit_n (p, m->m, m->size);

  up = xalloc_limbs (m->size);
  vp = xalloc_limbs (m->size);
  rp = xalloc_limbs (m->size);
  scratch = xalloc_limbs (m->sqrt_ratio_itch);

  /* Check behaviour for zero input */
  mpn_zero (up, m->size);
  mpn_zero (vp, m->size);
  vp[0] = 1;
  memset (rp, 17, m->size * sizeof(*rp));
  if (!m->sqrt_ratio (m, rp, up, vp, scratch))
    {
      fprintf (stderr, "m->sqrt_ratio returned failure for zero input, bit_size = %d\n",
	       m->bit_size);
      abort();
    }
  if (!ecc_mod_zero_p (m, rp))
    {
      fprintf (stderr, "m->sqrt_ratio failed for zero input (bit size %u):\n",
	       m->bit_size);
      fprintf (stderr, "r = ");
      mpn_out_str (stderr, 16, rp, m->size);
      fprintf (stderr, " (bad)\n");
      abort ();
    }

  /* Find a non-square */
  for (z = 2; mpz_ui_kronecker (z, p) != -1; z++)
    ;

  if (verbose)
    fprintf(stderr, "test_sqrt_ratio on %d-bit modulo. Non square: %d\n", m->bit_size, z);

  for (i = 0; i < COUNT; i++)
    {
      if (i & 1)
	{
	  mpz_rrandomb (u, rands, m->bit_size);
	  mpz_rrandomb (v, rands, m->bit_size);
	}
      else
	{
	  mpz_urandomb (u, rands, m->bit_size);
	  mpz_urandomb (v, rands, m->bit_size);
	}
      mpz_limbs_copy (up, u, m->size);
      mpz_limbs_copy (vp, v, m->size);
      if (!m->sqrt_ratio (m, rp, up, vp, scratch))
	{
	  mpz_mul_ui (u, u, z);
	  mpz_mod (u, u, p);
	  mpz_limbs_copy (up, u, m->size);
	  if (!m->sqrt_ratio (m, rp, up, vp, scratch))
	    {
	      if (mpz_divisible_p (v, p))
		/* v = 0 (mod p), sqrt_ratio should fail. */
		continue;

	      fprintf (stderr, "m->sqrt_ratio returned failure, bit_size = %d\n"
		       "u = 0x",
		       m->bit_size);
	      mpz_out_str (stderr, 16, u);
	      fprintf (stderr, "\nv = 0x");
	      mpz_out_str (stderr, 16, v);
	      fprintf (stderr, "\n");
	      abort ();
	    }
	}
      /* Check that r^2 v = u */
      mpz_roinit_n (r, rp, m->size);
      mpz_mul (t, r, r);
      mpz_mul (t, t, v);
      if (!mpz_congruent_p (t, u, p))
	{
	  fprintf (stderr, "m->sqrt_ratio gave incorrect result, bit_size = %d\n"
		   "u = 0x",
		   m->bit_size);
	  mpz_out_str (stderr, 16, u);
	  fprintf (stderr, "\nv = 0x");
	  mpz_out_str (stderr, 16, v);
	  fprintf (stderr, "\nr = 0x");
	  mpz_out_str (stderr, 16, r);
	  fprintf (stderr, "\n");
	  abort ();
	}
    }
  mpz_clear (u);
  mpz_clear (v);
  mpz_clear (t);
  free (up);
  free (vp);
  free (rp);
  free (scratch);
}

void
test_main (void)
{
  gmp_randstate_t rands;
  unsigned i;

  gmp_randinit_default (rands);
  test_randomize(rands);

  for (i = 0; ecc_curves[i]; i++)
    {
      if (ecc_curves[i]->p.sqrt)
	test_sqrt (rands, &ecc_curves[i]->p, ecc_curves[i]->use_redc);
      if (ecc_curves[i]->p.sqrt_ratio)
	test_sqrt_ratio (rands, &ecc_curves[i]->p);
    }
  gmp_randclear (rands);
}
