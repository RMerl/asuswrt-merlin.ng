#include "testutils.h"

#include "rsa.h"
#include "knuth-lfib.h"

#if HAVE_VALGRIND_MEMCHECK_H
# include <valgrind/memcheck.h>

#define MARK_MPZ_LIMBS_UNDEFINED(parm) \
  VALGRIND_MAKE_MEM_UNDEFINED (mpz_limbs_read (parm), \
                               mpz_size (parm) * sizeof (mp_limb_t))
#define MARK_MPZ_LIMBS_DEFINED(parm) \
  VALGRIND_MAKE_MEM_DEFINED (mpz_limbs_read (parm), \
                               mpz_size (parm) * sizeof (mp_limb_t))
static int
rsa_decrypt_for_test(const struct rsa_public_key *pub,
                     const struct rsa_private_key *key,
                     void *random_ctx, nettle_random_func *random,
                     size_t length, uint8_t *message,
                     const mpz_t gibberish)
{
  int ret;
  /* Makes valgrind trigger on any branches depending on the input
     data. Except that (i) we have to allow rsa_sec_compute_root_tr to
     check that p and q are odd, (ii) mpn_sec_div_r may leak
     information about the most significant bits of p and q, due to
     normalization check and table lookup in invert_limb, and (iii)
     mpn_sec_powm may leak information about the least significant
     bits of p and q, due to table lookup in binvert_limb. */
  VALGRIND_MAKE_MEM_UNDEFINED (message, length);
  MARK_MPZ_LIMBS_UNDEFINED(gibberish);
  MARK_MPZ_LIMBS_UNDEFINED(key->a);
  MARK_MPZ_LIMBS_UNDEFINED(key->b);
  MARK_MPZ_LIMBS_UNDEFINED(key->c);
  VALGRIND_MAKE_MEM_UNDEFINED(mpz_limbs_read (key->p) + 1,
			      (mpz_size (key->p) - 3) * sizeof(mp_limb_t));
  VALGRIND_MAKE_MEM_UNDEFINED(mpz_limbs_read (key->q) + 1,
			      (mpz_size (key->q) - 3) * sizeof(mp_limb_t));

  ret = rsa_sec_decrypt (pub, key, random_ctx, random, length, message, gibberish);

  VALGRIND_MAKE_MEM_DEFINED (message, length);
  VALGRIND_MAKE_MEM_DEFINED (&ret, sizeof(ret));
  MARK_MPZ_LIMBS_DEFINED(gibberish);
  MARK_MPZ_LIMBS_DEFINED(key->a);
  MARK_MPZ_LIMBS_DEFINED(key->b);
  MARK_MPZ_LIMBS_DEFINED(key->c);
  MARK_MPZ_LIMBS_DEFINED(key->p);
  MARK_MPZ_LIMBS_DEFINED(key->q);

  return ret;
}
#else
#define rsa_decrypt_for_test rsa_sec_decrypt
#endif

#define PAYLOAD_SIZE 50
#define DECRYPTED_SIZE 256
void
test_main(void)
{
  struct rsa_public_key pub;
  struct rsa_private_key key;
  struct knuth_lfib_ctx random_ctx;

  uint8_t plaintext[PAYLOAD_SIZE];
  uint8_t decrypted[DECRYPTED_SIZE];
  uint8_t verifybad[PAYLOAD_SIZE];
  unsigned n_size = 1024;
  mpz_t gibberish;
  mpz_t garbage;
  size_t size;

  rsa_private_key_init(&key);
  rsa_public_key_init(&pub);
  mpz_init(gibberish);
  mpz_init(garbage);

  knuth_lfib_init (&random_ctx, 19);

  memset(verifybad, 'A', PAYLOAD_SIZE);

  for (size = 1; size < 51; size++)
    {
      ASSERT (rsa_generate_keypair(&pub, &key, &random_ctx,
			           (nettle_random_func *) knuth_lfib_random,
			           NULL, NULL, n_size, 17));

      /* the next key will be 19 bits larger */
      n_size += 19;

      knuth_lfib_random (&random_ctx, PAYLOAD_SIZE, plaintext);
      ASSERT(rsa_encrypt(&pub, &random_ctx,
                         (nettle_random_func *) knuth_lfib_random,
                         PAYLOAD_SIZE, plaintext, gibberish));

      /* good decryption */
      ASSERT (rsa_decrypt_for_test (&pub, &key, &random_ctx,
                                    (nettle_random_func *) knuth_lfib_random,
                                    PAYLOAD_SIZE, decrypted, gibberish) == 1);
      ASSERT (MEMEQ (PAYLOAD_SIZE, plaintext, decrypted));

      ASSERT (pub.size > 10);
      ASSERT (pub.size <= DECRYPTED_SIZE);

      /* Check that too large message length is rejected, largest
	 valid size is pub.size - 11. */
      ASSERT (!rsa_decrypt_for_test (&pub, &key, &random_ctx,
				     (nettle_random_func *) knuth_lfib_random,
				     pub.size - 10, decrypted, gibberish));

      /* This case used to result in arithmetic underflow and a crash. */
      ASSERT (!rsa_decrypt_for_test (&pub, &key, &random_ctx,
				     (nettle_random_func *) knuth_lfib_random,
				     pub.size, decrypted, gibberish));

      /* bad one */
      memcpy(decrypted, verifybad, PAYLOAD_SIZE);
      nettle_mpz_random_size(garbage, &random_ctx,
                             (nettle_random_func *) knuth_lfib_random,
                             mpz_sizeinbase(gibberish, 2));

      ASSERT (rsa_decrypt_for_test (&pub, &key, &random_ctx,
                                    (nettle_random_func *) knuth_lfib_random,
                                    PAYLOAD_SIZE, decrypted, garbage) == 0);
      ASSERT (MEMEQ (PAYLOAD_SIZE, verifybad, decrypted));
    }

  rsa_private_key_clear(&key);
  rsa_public_key_clear(&pub);
  mpz_clear(gibberish);
  mpz_clear(garbage);
}

