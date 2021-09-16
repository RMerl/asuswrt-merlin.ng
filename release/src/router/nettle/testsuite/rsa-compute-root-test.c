#include "testutils.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <sys/time.h>

#include "rsa.h"

#define KEY_COUNT 20
#define COUNT 100

static void
random_fn (void *ctx, size_t n, uint8_t *dst)
{
  gmp_randstate_t *rands = (gmp_randstate_t *)ctx;
  mpz_t r;

  mpz_init (r);
  mpz_urandomb (r, *rands, n*8);
  nettle_mpz_get_str_256 (n, dst, r);
  mpz_clear (r);
}

static void
test_one (gmp_randstate_t *rands, struct rsa_public_key *pub,
          struct rsa_private_key *key, mpz_t plaintext)
{
  mpz_t ciphertext;
  mpz_t decrypted;

  mpz_init (ciphertext);
  mpz_init (decrypted);

  mpz_powm (ciphertext, plaintext, pub->e, pub->n);
  rsa_compute_root_tr (pub, key, rands, random_fn, decrypted, ciphertext);
  if (mpz_cmp (plaintext, decrypted)) {
    fprintf (stderr, "rsa_compute_root_tr failed\n");

    fprintf(stderr, "Public key: size=%u\n n:", (unsigned) pub->size);
    mpz_out_str (stderr, 10, pub->n);
    fprintf(stderr, "\n e:");
    mpz_out_str (stderr, 10, pub->e);
    fprintf(stderr, "\nPrivate key: size=%u\n p:", (unsigned) key->size);
    mpz_out_str (stderr, 10, key->p);
    fprintf(stderr, "\n q:");
    mpz_out_str (stderr, 10, key->q);
    fprintf(stderr, "\n a:");
    mpz_out_str (stderr, 10, key->a);
    fprintf(stderr, "\n b:");
    mpz_out_str (stderr, 10, key->b);
    fprintf(stderr, "\n c:");
    mpz_out_str (stderr, 10, key->c);
    fprintf(stderr, "\n d:");
    mpz_out_str (stderr, 10, key->d);
    fprintf(stderr, "\n");

    fprintf (stderr, "plaintext(%u) = ", (unsigned) mpz_sizeinbase (plaintext, 2));
    mpz_out_str (stderr, 10, plaintext);
    fprintf (stderr, "\n");
    fprintf (stderr, "ciphertext(%u) = ", (unsigned) mpz_sizeinbase (ciphertext, 2));
    mpz_out_str (stderr, 10, ciphertext);
    fprintf (stderr, "\n");
    fprintf (stderr, "decrypted(%u) = ", (unsigned) mpz_sizeinbase (decrypted, 2));
    mpz_out_str (stderr, 10, decrypted);
    fprintf (stderr, "\n");
    abort();
  }

  mpz_clear (ciphertext);
  mpz_clear (decrypted);
}

#if !NETTLE_USE_MINI_GMP
/* We want to generate keypairs that are not "standard" but have more size
 * variance between q and p.
 * Function is otherwise the same as standard rsa_generate_keypair()
 */
static void
generate_keypair (gmp_randstate_t rands,
                  struct rsa_public_key *pub, struct rsa_private_key *key)
{
  unsigned long int psize;
  unsigned long int qsize;
  mpz_t p1;
  mpz_t q1;
  mpz_t phi;
  mpz_t tmp;
  int res;

  mpz_init (p1);
  mpz_init (q1);
  mpz_init (phi);
  mpz_init (tmp);

  psize = 100 + gmp_urandomm_ui (rands, 400);
  qsize = 100 + gmp_urandomm_ui (rands, 400);

  mpz_set_ui (pub->e, 65537);

  for (;;)
    {
      for (;;)
        {
          mpz_rrandomb (key->p, rands, psize);
          mpz_nextprime (key->p, key->p);
          mpz_sub_ui (p1, key->p, 1);
          mpz_gcd (tmp, pub->e, p1);
          if (mpz_cmp_ui (tmp, 1) == 0)
            break;
        }

      for (;;)
        {
          mpz_rrandomb (key->q, rands, qsize);
          mpz_nextprime (key->q, key->q);
          mpz_sub_ui (q1, key->q, 1);
          mpz_gcd (tmp, pub->e, q1);
          if (mpz_cmp_ui (tmp, 1) == 0)
            break;
        }

      if (mpz_invert (key->c, key->q, key->p))
        break;
    }

  mpz_mul(phi, p1, q1);
  res = mpz_invert(key->d, pub->e, phi);
  assert (res);

  mpz_fdiv_r (key->a, key->d, p1);
  mpz_fdiv_r (key->b, key->d, q1);

  mpz_mul (pub->n, key->p, key->q);

  pub->size = key->size = mpz_size(pub->n) * sizeof(mp_limb_t);

  mpz_clear (tmp);
  mpz_clear (phi);
  mpz_clear (q1);
  mpz_clear (p1);
}
#endif

#if !NETTLE_USE_MINI_GMP
static void
get_random_seed(mpz_t seed)
{
  struct timeval tv;
  FILE *f;
  f = fopen ("/dev/urandom", "rb");
  if (f)
    {
      uint8_t buf[8];
      size_t res;

      setbuf (f, NULL);
      res = fread (&buf, sizeof(buf), 1, f);
      fclose(f);
      if (res == 1)
	{
	  nettle_mpz_set_str_256_u (seed, sizeof(buf), buf);
	  return;
	}
      fprintf (stderr, "Read of /dev/urandom failed: %s\n",
	       strerror (errno));
    }
  gettimeofday(&tv, NULL);
  mpz_set_ui (seed, tv.tv_sec);
  mpz_mul_ui (seed, seed, 1000000UL);
  mpz_add_ui (seed, seed, tv.tv_usec);
}
#endif /* !NETTLE_USE_MINI_GMP */

void
test_main (void)
{
  const char *nettle_test_seed;
  gmp_randstate_t rands;
  struct rsa_public_key pub;
  struct rsa_private_key key;
  mpz_t plaintext;
  unsigned i, j;

  rsa_private_key_init(&key);
  rsa_public_key_init(&pub);
  mpz_init (plaintext);

  gmp_randinit_default (rands);

#if !NETTLE_USE_MINI_GMP
  nettle_test_seed = getenv ("NETTLE_TEST_SEED");
  if (nettle_test_seed && *nettle_test_seed)
    {
      mpz_t seed;
      mpz_init (seed);
      if (mpz_set_str (seed, nettle_test_seed, 0) < 0
	  || mpz_sgn (seed) < 0)
	die ("Invalid NETTLE_TEST_SEED: %s\n",
	     nettle_test_seed);
      if (mpz_sgn (seed) == 0)
	get_random_seed (seed);
      fprintf (stderr, "Using NETTLE_TEST_SEED=");
      mpz_out_str (stderr, 10, seed);
      fprintf (stderr, "\n");

      gmp_randseed (rands, seed);
      mpz_clear (seed);
    }
#endif

  for (j = 0; j < KEY_COUNT; j++)
    {
#if !NETTLE_USE_MINI_GMP
      generate_keypair(rands, &pub, &key);
#else
      rsa_generate_keypair(&pub, &key, &rands, random_fn, NULL, NULL, 512, 16);
#endif /* !NETTLE_USE_MINI_GMP */

      for (i = 0; i < COUNT; i++)
	{
	  mpz_urandomb(plaintext, rands, mpz_sizeinbase(pub.n, 2) - 1);
	  test_one(&rands, &pub, &key, plaintext);
	}
      for (i = 0; i < COUNT; i++)
	{
	  mpz_rrandomb(plaintext, rands, mpz_sizeinbase(pub.n, 2) - 1);
	  test_one(&rands, &pub, &key, plaintext);
	}
    }
  mpz_clear (plaintext);
  rsa_public_key_clear (&pub);
  rsa_private_key_clear (&key);

  gmp_randclear (rands);
}
