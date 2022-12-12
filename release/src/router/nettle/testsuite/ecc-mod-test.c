#include "testutils.h"

static void
ref_mod (mp_limb_t *rp, const mp_limb_t *ap, const mp_limb_t *mp, mp_size_t mn)
{
  mpz_t r, a, m;
  mpz_init (r);
  mpz_mod (r, mpz_roinit_n (a, ap, 2*mn), mpz_roinit_n (m, mp, mn));
  mpz_limbs_copy (rp, r, mn);

  mpz_clear (r);
}

#define MAX_ECC_SIZE (1 + 521 / GMP_NUMB_BITS)
#define MAX_SIZE (2*MAX_ECC_SIZE)
#define COUNT 50000

/* Destructively normalize tp, then compare */
static int
mod_equal(const struct ecc_modulo *m, const mp_limb_t *ref, mp_limb_t *tp)
{
  if (mpn_cmp (tp, m->m, m->size) >= 0)
    mpn_sub_n (tp, tp, m->m, m->size);
  return mpn_cmp (ref, tp, m->size) == 0;
}

static void
test_one(const char *name,
	 const struct ecc_modulo *m,
	 const mpz_t r)
{
  mp_limb_t a[MAX_SIZE];
  mp_limb_t t[MAX_SIZE];
  mp_limb_t ref[MAX_SIZE];

  mpz_limbs_copy (a, r, 2*m->size);

  ref_mod (ref, a, m->m, m->size);

  mpn_copyi (t, a, 2*m->size);
  m->mod (m, t, t);
  if (!mod_equal (m, ref, t))
    {
      fprintf (stderr, "m->mod %s failed: bit_size = %u, rp == xp\n",
	       name, m->bit_size);

      fprintf (stderr, "a   = ");
      mpn_out_str (stderr, 16, a, 2*m->size);
      fprintf (stderr, "\nt   = ");
      mpn_out_str (stderr, 16, t, m->size);
      fprintf (stderr, " (bad)\nref = ");
      mpn_out_str (stderr, 16, ref, m->size);
      fprintf (stderr, "\n");
      abort ();
    }

  mpn_copyi (t, a, 2*m->size);
  m->mod (m, t + m->size, t);
  if (!mod_equal (m, ref, t + m->size))
    {
      fprintf (stderr, "m->mod %s failed: bit_size = %u, rp == xp + size\n",
	       name, m->bit_size);

      fprintf (stderr, "a   = ");
      mpn_out_str (stderr, 16, a, 2*m->size);
      fprintf (stderr, "\nt   = ");
      mpn_out_str (stderr, 16, t + m->size, m->size);
      fprintf (stderr, " (bad)\nref = ");
      mpn_out_str (stderr, 16, ref, m->size);
      fprintf (stderr, "\n");
      abort ();
    }

  if (m->B_size < m->size)
    {
      mpn_copyi (t, a, 2*m->size);
      ecc_mod (m, t, t);
      if (!mod_equal (m, ref, t))
	{
	  fprintf (stderr, "ecc_mod %s failed: bit_size = %u, rp == xp\n",
		   name, m->bit_size);
	  fprintf (stderr, "a   = ");
	  mpn_out_str (stderr, 16, a, 2*m->size);
	  fprintf (stderr, "\nt   = ");
	  mpn_out_str (stderr, 16, t, m->size);
	  fprintf (stderr, " (bad)\nref = ");
	  mpn_out_str (stderr, 16, ref, m->size);
	  fprintf (stderr, "\n");
	  abort ();
	}

      mpn_copyi (t, a, 2*m->size);
      ecc_mod (m, t + m->size, t);
      if (!mod_equal (m, ref, t + m->size))
	{
	  fprintf (stderr, "ecc_mod %s failed: bit_size = %u, rp == xp + size\n",
		   name, m->bit_size);
	  fprintf (stderr, "a   = ");
	  mpn_out_str (stderr, 16, a, 2*m->size);
	  fprintf (stderr, "\nt   = ");
	  mpn_out_str (stderr, 16, t + m->size, m->size);
	  fprintf (stderr, " (bad)\nref = ");
	  mpn_out_str (stderr, 16, ref, m->size);
	  fprintf (stderr, "\n");
	  abort ();
	}
    }
}

static void
test_modulo (gmp_randstate_t rands, const char *name,
	     const struct ecc_modulo *m, unsigned count)
{
  mpz_t r;
  unsigned j;

  mpz_init (r);

  for (j = 0; j < count; j++)
    {
      if (j & 2)
	{
	  if (j & 1)
	    mpz_rrandomb (r, rands, 2*m->size * GMP_NUMB_BITS);
	  else
	    mpz_urandomb (r, rands, 2*m->size * GMP_NUMB_BITS);
	}
      else
	{
	  /* Test inputs close to a multiple of m. */
	  mpz_t q;
	  unsigned q_size;
	  int diff;

	  mpz_urandomb(r, rands, 30);
	  q_size = 11 + mpz_get_ui(r) % (m->size * GMP_NUMB_BITS - 10);
	  mpz_urandomb(r, rands, 30);
	  diff = mpz_get_si(r) % 20 - 10;

	  if (j & 1)
	    mpz_rrandomb (r, rands, q_size);
	  else
	    mpz_urandomb (r, rands, q_size);

	  mpz_mul (r, r, mpz_roinit_n(q, m->m, m->size));
	  if (diff >= 0)
	    mpz_add_ui (r, r, diff);
	  else
	    mpz_sub_ui (r, r, -diff);

	  if (mpz_sgn(r) < 0)
	    continue;
	}

      test_one (name, m, r);
    }
  mpz_clear (r);
}

static void
test_fixed (void)
{
  mpz_t r;
  mpz_init (r);

  /* Triggered a bug reported by Hanno Böck. */
  mpz_set_str (r, "FFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFF001C2C00", 16);
  mpz_mul_2exp (r, r, 256);
  test_one ("p", &_nettle_secp_256r1.p, r);
  test_one ("q", &_nettle_secp_256r1.q, r);

  mpz_set_str (r, "ffffffff00000001fffffffeffffffffffffffffffffffffffffffc0000000000007ffffffffffffffffffffffffffff00000000000000000fffffffffffffff", 16);
  test_one ("p", &_nettle_secp_256r1.p, r);
  test_one ("q", &_nettle_secp_256r1.q, r);

  /* Triggered a bug reported by Hanno Böck. */
  mpz_set_str (r, "4c9000000000000000000000000000000000000000000000004a604db486e000000000000000000000000000000000000000121025be29575adb2c8ffffffffffffffffffffffffffffffffffffffffffffffffffffffff", 16);
  test_one ("p", &_nettle_secp_384r1.p, r);
  test_one ("q", &_nettle_secp_384r1.q, r);

  /* Triggered a carry bug in development version. */
  mpz_set_str (r, "e64a84643150260640e4677c19ffc4faef06042132b86af6e9ee33fe1850222e57a514d5f1d6d444008bb896a96a43d5629945e57548f5e12f66be132b24110cbb2df6d7d3dd3aaadc98b0bbf29573843ad72e57f59fc5d4f56cc599da18bb99", 16);

  test_one ("p", &_nettle_secp_384r1.p, r);
  test_one ("q", &_nettle_secp_384r1.q, r);

  mpz_clear (r);
}

static void
test_patterns (const char *name,
	       const struct ecc_modulo *m)
{
  mpz_t r;
  unsigned j;

  mpz_init (r);

  for (j = m->bit_size; j < 2*m->bit_size; j++)
    {
      /* Single one bit */
      mpz_set_ui (r, 1);
      mpz_mul_2exp (r, r, j);
      test_one (name, m, r);

      /* All ones. */
      mpz_mul_2exp (r, r, 1);
      mpz_sub_ui (r, r, 1);
      test_one (name, m, r);
    }
  mpz_clear (r);
}

void
test_main (void)
{
  gmp_randstate_t rands;
  unsigned count = COUNT;
  unsigned i;

  gmp_randinit_default (rands);

  test_fixed ();

  for (i = 0; ecc_curves[i]; i++)
    {
      test_patterns ("p", &ecc_curves[i]->p);
      test_patterns ("q", &ecc_curves[i]->p);
    }

  if (test_randomize(rands))
    count *= 20;

  for (i = 0; ecc_curves[i]; i++)
    {
      test_modulo (rands, "p", &ecc_curves[i]->p, count);
      test_modulo (rands, "q", &ecc_curves[i]->q, count);
    }
  gmp_randclear (rands);
}
