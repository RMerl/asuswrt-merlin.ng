#include "testutils.h"

void
test_main (void)
{
  gmp_randstate_t rands;
  mpz_t r;
  unsigned i;

  gmp_randinit_default (rands);
  mpz_init (r);
  
  for (i = 0; ecc_curves[i]; i++)
    {
      const struct ecc_curve *ecc = ecc_curves[i];
      mp_size_t size = ecc_size (ecc);
      mp_limb_t *g = xalloc_limbs (ecc_size_a (ecc));
      mp_limb_t *p = xalloc_limbs (ecc_size_j (ecc));
      mp_limb_t *q = xalloc_limbs (ecc_size_j (ecc));
      mp_limb_t *n = xalloc_limbs (size);
      mp_limb_t *scratch = xalloc_limbs (ecc->mul_itch);
      unsigned j;
      
      test_ecc_get_ga (i, g);
      mpn_zero (n, size);

      for (n[0] = 1; n[0] <= 4; n[0]++)
	{
	  ecc->mul (ecc, p, n, g, scratch);
	  test_ecc_mul_h (i, n[0], p);
	}

      /* (order - 1) * g = - g */
      mpn_sub_1 (n, ecc->q.m, size, 1);
      ecc->mul (ecc, p, n, g, scratch);
      ecc->h_to_a (ecc, 0, p, p, scratch);
      if (ecc->p.bit_size == 255 || ecc->p.bit_size == 448)
	/* For edwards curves, - (x,y ) == (-x, y). FIXME: Swap x and
	   y, to get identical negation? */
	mpn_sub_n (p, ecc->p.m, p, size);
      else
	mpn_sub_n (p + size, ecc->p.m, p + size, size);

      test_ecc_ga (i, p);

      mpn_zero (n, size);

      for (j = 0; j < 100; j++)
	{
	  if (j & 1)
	    mpz_rrandomb (r, rands, size * GMP_NUMB_BITS);
	  else
	    mpz_urandomb (r, rands, size * GMP_NUMB_BITS);

	  /* Reduce so that (almost surely) n < q */
	  mpz_limbs_copy (n, r, size);
	  n[size - 1] %= ecc->q.m[size - 1];

	  ecc->mul (ecc, p, n, g, scratch);
	  ecc->h_to_a (ecc, 0, p, p, scratch);

	  ecc->mul_g (ecc, q, n, scratch);
	  ecc->h_to_a (ecc, 0, q, q, scratch);

	  if (mpn_cmp (p, q, 2*size))
	    {
	      fprintf (stderr,
		       "Different results from ecc->mul and ecc->mul_g.\n"
		       " bits = %u\n",
		       ecc->p.bit_size);
	      fprintf (stderr, " n = ");
	      mpn_out_str (stderr, 16, n, size);
	      
	      fprintf (stderr, "\np = ");
	      mpn_out_str (stderr, 16, p, size);
	      fprintf (stderr, ",\n    ");
	      mpn_out_str (stderr, 16, p + size, size);

	      fprintf (stderr, "\nq = ");
	      mpn_out_str (stderr, 16, q, size);
	      fprintf (stderr, ",\n    ");
	      mpn_out_str (stderr, 16, q + size, size);
	      fprintf (stderr, "\n");
	      abort ();
	    }
	}
      free (g);
      free (n);
      free (p);
      free (q);
      free (scratch);
    }
  mpz_clear (r); 
  gmp_randclear (rands);
}
