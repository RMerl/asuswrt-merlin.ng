#include "testutils.h"

#include "rsa.h"
#include "knuth-lfib.h"

void
test_main(void)
{
  struct rsa_public_key pub;
  struct rsa_private_key key;
  struct knuth_lfib_ctx lfib;

  /* FIXME: How is this spelled? */
  const unsigned char msg[] = "Squemish ossifrage";
  size_t msg_length = LLENGTH(msg);

  uint8_t *decrypted;
  size_t decrypted_length;
  uint8_t after;

  mpz_t gibberish;
  mpz_t bad_input;

  rsa_private_key_init(&key);
  rsa_public_key_init(&pub);
  mpz_init(gibberish);
  mpz_init(bad_input);

  knuth_lfib_init(&lfib, 17);
  
  test_rsa_set_key_1(&pub, &key);

  if (verbose)
    fprintf(stderr, "msg: `%s', length = %d\n", msg, (int) msg_length);

  ASSERT(msg_length <= key.size);
  
  ASSERT(rsa_encrypt(&pub,
		     &lfib, (nettle_random_func *) knuth_lfib_random,
		     msg_length, msg,
		     gibberish));

  if (verbose)
    {
      fprintf(stderr, "encrypted: ");
      mpz_out_str(stderr, 10, gibberish);
    }
  
  decrypted = xalloc(key.size + 1);

  knuth_lfib_random (&lfib, msg_length + 1, decrypted);
  after = decrypted[msg_length];
  
  decrypted_length = msg_length - 1;
  ASSERT(!rsa_decrypt(&key, &decrypted_length, decrypted, gibberish));

  decrypted_length = msg_length;
  ASSERT(rsa_decrypt(&key, &decrypted_length, decrypted, gibberish));
  ASSERT(decrypted_length == msg_length);
  ASSERT(MEMEQ(msg_length, msg, decrypted));
  ASSERT(decrypted[msg_length] == after);

  knuth_lfib_random (&lfib, key.size + 1, decrypted);
  after = decrypted[key.size];

  decrypted_length = key.size;
  ASSERT(rsa_decrypt(&key, &decrypted_length, decrypted, gibberish));
  ASSERT(decrypted_length == msg_length);
  ASSERT(MEMEQ(msg_length, msg, decrypted));
  ASSERT(decrypted[key.size] == after);
  
  knuth_lfib_random (&lfib, msg_length + 1, decrypted);
  after = decrypted[msg_length];

  decrypted_length = msg_length;
  ASSERT(rsa_decrypt_tr(&pub, &key,
			&lfib, (nettle_random_func *) knuth_lfib_random,
			&decrypted_length, decrypted, gibberish));
  ASSERT(decrypted_length == msg_length);
  ASSERT(MEMEQ(msg_length, msg, decrypted));
  ASSERT(decrypted[msg_length] == after);

  /* test side channel resistant variant */
  knuth_lfib_random (&lfib, msg_length + 1, decrypted);
  after = decrypted[msg_length];
  decrypted_length = msg_length;

  ASSERT(rsa_sec_decrypt(&pub, &key,
                         &lfib, (nettle_random_func *) knuth_lfib_random,
                         decrypted_length, decrypted, gibberish));
  ASSERT(MEMEQ(msg_length, msg, decrypted));
  ASSERT(decrypted[msg_length] == after);

  /* test invalid length to rsa_sec_decrypt */
  knuth_lfib_random (&lfib, msg_length + 1, decrypted);
  decrypted_length = msg_length - 1;
  after = decrypted[decrypted_length] = 'X';
  decrypted[0] = 'A';

  ASSERT(!rsa_sec_decrypt(&pub, &key,
                          &lfib, (nettle_random_func *) knuth_lfib_random,
                          decrypted_length, decrypted, gibberish));
  ASSERT(decrypted[decrypted_length] == after);
  ASSERT(decrypted[0] == 'A');

  /* Test zero input. */
  mpz_set_ui (bad_input, 0);
  decrypted_length = msg_length;
  ASSERT(!rsa_decrypt(&key, &decrypted_length, decrypted, bad_input));
  ASSERT(!rsa_decrypt_tr(&pub, &key,
			 &lfib, (nettle_random_func *) knuth_lfib_random,
			 &decrypted_length, decrypted, bad_input));
  ASSERT(!rsa_sec_decrypt(&pub, &key,
			  &lfib, (nettle_random_func *) knuth_lfib_random,
			  decrypted_length, decrypted, bad_input));
  ASSERT(decrypted_length == msg_length);

  /* Test input that is slightly larger than n */
  mpz_add(bad_input, gibberish, pub.n);
  decrypted_length = msg_length;
  ASSERT(!rsa_decrypt(&key, &decrypted_length, decrypted, bad_input));
  ASSERT(!rsa_decrypt_tr(&pub, &key,
			 &lfib, (nettle_random_func *) knuth_lfib_random,
			 &decrypted_length, decrypted, bad_input));
  ASSERT(!rsa_sec_decrypt(&pub, &key,
			  &lfib, (nettle_random_func *) knuth_lfib_random,
			  decrypted_length, decrypted, bad_input));
  ASSERT(decrypted_length == msg_length);

  /* Test input that is considerably larger than n */
  mpz_mul_2exp (bad_input, pub.n, 100);
  mpz_add (bad_input, bad_input, gibberish);
  decrypted_length = msg_length;
  ASSERT(!rsa_decrypt(&key, &decrypted_length, decrypted, bad_input));
  ASSERT(!rsa_decrypt_tr(&pub, &key,
			 &lfib, (nettle_random_func *) knuth_lfib_random,
			 &decrypted_length, decrypted, bad_input));
  ASSERT(!rsa_sec_decrypt(&pub, &key,
			  &lfib, (nettle_random_func *) knuth_lfib_random,
			  decrypted_length, decrypted, bad_input));
  ASSERT(decrypted_length == msg_length);

  /* Test invalid key. */
  mpz_add_ui (key.q, key.q, 2);
  decrypted_length = key.size;
  ASSERT(!rsa_decrypt_tr(&pub, &key,
			 &lfib, (nettle_random_func *) knuth_lfib_random,
			 &decrypted_length, decrypted, gibberish));

  rsa_private_key_clear(&key);
  rsa_public_key_clear(&pub);
  mpz_clear(gibberish);
  mpz_clear(bad_input);
  free(decrypted);
}
