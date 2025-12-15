#include "testutils.h"

#define MAX_SIZE (1 + 521 / GMP_NUMB_BITS)
#define COUNT 10000

static void
test_add(const char *name,
	 const struct ecc_modulo *m,
	 const mpz_t az, const mpz_t bz)
{
  mp_limb_t a[MAX_SIZE];
  mp_limb_t b[MAX_SIZE];
  mp_limb_t t[MAX_SIZE];
  mpz_t mz;
  mpz_t tz;
  mpz_t ref;

  mpz_init (ref);
  mpz_add (ref, az, bz);
  mpz_mod (ref, ref, mpz_roinit_n (mz, m->m, m->size));

  mpz_limbs_copy (a, az, m->size);
  mpz_limbs_copy (b, bz, m->size);
  ecc_mod_add (m, t, a, b);

  if (!mpz_congruent_p (ref, mpz_roinit_n (tz, t, m->size), mz))
    {
      fprintf (stderr, "ecc_mod_add %s failed: bit_size = %u\n",
	       name, m->bit_size);

      fprintf (stderr, "a   = ");
      mpn_out_str (stderr, 16, a, m->size);
      fprintf (stderr, "\nb   = ");
      mpn_out_str (stderr, 16, b, m->size);
      fprintf (stderr, "\nt   = ");
      mpn_out_str (stderr, 16, t, m->size);
      fprintf (stderr, " (bad)\nref = ");
      mpz_out_str (stderr, 16, ref);
      fprintf (stderr, "\n");
      abort ();
    }
  mpz_clear (ref);
}

static void
test_sub(const char *name,
	 const struct ecc_modulo *m,
	 /* If range is non-null, check that 0 <= r < range. */
	 const mp_limb_t *range,
	 const mpz_t az, const mpz_t bz)
{
  mp_limb_t a[MAX_SIZE];
  mp_limb_t b[MAX_SIZE];
  mp_limb_t t[MAX_SIZE];
  mpz_t mz;
  mpz_t tz;
  mpz_t ref;

  mpz_init (ref);
  mpz_sub (ref, az, bz);
  mpz_mod (ref, ref, mpz_roinit_n (mz, m->m, m->size));

  mpz_limbs_copy (a, az, m->size);
  mpz_limbs_copy (b, bz, m->size);
  ecc_mod_sub (m, t, a, b);

  if (!mpz_congruent_p (ref, mpz_roinit_n (tz, t, m->size), mz))
    {
      fprintf (stderr, "ecc_mod_sub %s failed: bit_size = %u\n",
	       name, m->bit_size);

      fprintf (stderr, "a   = ");
      mpn_out_str (stderr, 16, a, m->size);
      fprintf (stderr, "\nb   = ");
      mpn_out_str (stderr, 16, b, m->size);
      fprintf (stderr, "\nt   = ");
      mpn_out_str (stderr, 16, t, m->size);
      fprintf (stderr, " (bad)\nref = ");
      mpz_out_str (stderr, 16, ref);
      fprintf (stderr, "\n");
      abort ();
    }

  if (range && mpn_cmp (t, range, m->size) >= 0)
    {
      fprintf (stderr, "ecc_mod_sub %s out of range: bit_size = %u\n",
	       name, m->bit_size);

      fprintf (stderr, "a   = ");
      mpn_out_str (stderr, 16, a, m->size);
      fprintf (stderr, "\nb   = ");
      mpn_out_str (stderr, 16, b, m->size);
      fprintf (stderr, "\nt   = ");
      mpn_out_str (stderr, 16, t, m->size);
      fprintf (stderr, " \nrange = ");
      mpn_out_str (stderr, 16, range, m->size);
      fprintf (stderr, "\n");
      abort ();
    }
  mpz_clear (ref);
}

static void
test_modulo (gmp_randstate_t rands, const char *name,
	     const struct ecc_modulo *m, unsigned count)
{
  mpz_t a, b;
  unsigned j;

  mpz_init (a);
  mpz_init (b);

  for (j = 0; j < count; j++)
    {
      if (j & 1)
	{
	  mpz_rrandomb (a, rands, m->size * GMP_NUMB_BITS);
	  mpz_rrandomb (b, rands, m->size * GMP_NUMB_BITS);
	}
      else
	{
	  mpz_urandomb (a, rands, m->size * GMP_NUMB_BITS);
	  mpz_urandomb (b, rands, m->size * GMP_NUMB_BITS);
	}
      test_add (name, m, a, b);
      test_sub (name, m, NULL, a, b);
    }
  if (m->bit_size < m->size * GMP_NUMB_BITS)
    {
      mp_limb_t two_p[MAX_SIZE];
      mpn_lshift (two_p, m->m, m->size, 1);
      mpz_t range;
      mpz_roinit_n (range, two_p, m->size);
      mpz_urandomm (a, rands, range);
      mpz_urandomm (b, rands, range);
      test_sub (name, m, two_p, a, b);
    }
  mpz_clear (a);
  mpz_clear (b);
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
      test_modulo (rands, "p", &ecc_curves[i]->p, COUNT);
      test_modulo (rands, "q", &ecc_curves[i]->q, COUNT);
    }
  gmp_randclear (rands);
}
