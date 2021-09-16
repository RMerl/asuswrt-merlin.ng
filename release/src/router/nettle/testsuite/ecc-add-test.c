#include "testutils.h"
#include <assert.h>

void
test_main (void)
{
  unsigned i;

  for (i = 0; ecc_curves[i]; i++)
    {
      const struct ecc_curve *ecc = ecc_curves[i];
      mp_limb_t *g = xalloc_limbs (ecc_size_j (ecc));
      mp_limb_t *g2 = xalloc_limbs (ecc_size_j (ecc));
      mp_limb_t *g3 = xalloc_limbs (ecc_size_j (ecc));
      mp_limb_t *p = xalloc_limbs (ecc_size_j (ecc));
      mp_limb_t *scratch = xalloc_limbs (ecc->add_hhh_itch);

      ASSERT (ecc->dup_itch <= ecc->add_hhh_itch);

      test_ecc_get_g (i, g);

      if (ecc->p.bit_size == 255 || ecc->p.bit_size == 448)
	{
	  mp_limb_t *z = xalloc_limbs (ecc_size_j (ecc));

	  ASSERT (ecc->add_hh_itch <= ecc->add_hhh_itch);

	  /* Zero point has x = 0, y = 1, z = 1 */
	  mpn_zero (z, 3*ecc->p.size);
	  z[ecc->p.size] = z[2*ecc->p.size] = 1;

	  ecc->add_hhh (ecc, p, z, z, scratch);
	  test_ecc_mul_h (i, 0, p);

	  ecc->add_hh (ecc, p, z, z, scratch);
	  test_ecc_mul_h (i, 0, p);

	  ecc->add_hhh (ecc, p, p, g, scratch);
	  test_ecc_mul_h (i, 1, p);

	  ecc->add_hh (ecc, p, z, g, scratch);
	  test_ecc_mul_h (i, 1, p);

	  ecc->add_hhh (ecc, g2, g, p, scratch);
	  test_ecc_mul_h (i, 2, g2);

	  ecc->add_hh (ecc, g2, g, g, scratch);
	  test_ecc_mul_h (i, 2, g2);

	  free (z);
	}

      ecc->dup (ecc, g2, g, scratch);
      test_ecc_mul_h (i, 2, g2);

      ecc->add_hhh (ecc, g3, g, g2, scratch);
      test_ecc_mul_h (i, 3, g3);

      ecc->add_hhh (ecc, g3, g2, g, scratch);
      test_ecc_mul_h (i, 3, g3);

      ecc->add_hhh (ecc, p, g, g3, scratch);
      test_ecc_mul_h (i, 4, p);

      ecc->add_hhh (ecc, p, g3, g, scratch);
      test_ecc_mul_h (i, 4, p);

      ecc->dup (ecc, p, g2, scratch);
      test_ecc_mul_h (i, 4, p);

      free (g);
      free (g2);
      free (g3);
      free (p);
      free (scratch);
    }
}
