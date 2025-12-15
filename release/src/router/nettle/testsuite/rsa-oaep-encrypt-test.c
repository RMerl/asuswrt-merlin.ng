#include "testutils.h"
#include "knuth-lfib.h"
#include "nettle-meta.h"
#include "rsa-internal.h"
#include "sha1.h"

#define MARK_MPZ_LIMBS_UNDEFINED(x) \
  mark_bytes_undefined (mpz_size (x) * sizeof (mp_limb_t), mpz_limbs_read (x))

#define MARK_MPZ_LIMBS_DEFINED(x) \
  mark_bytes_defined (mpz_size (x) * sizeof (mp_limb_t), mpz_limbs_read (x))

typedef int (*test_rsa_oaep_encrypt_func) (const struct rsa_public_key *key,
					   void *random_ctx,
					   nettle_random_func *random,
					   size_t label_length,
					   const uint8_t *label,
					   size_t length,
					   const uint8_t *message,
					   uint8_t *ciphertext);

typedef int (*test_rsa_oaep_decrypt_func) (const struct rsa_public_key *pub,
					   const struct rsa_private_key *key,
					   void *random_ctx,
					   nettle_random_func *random,
					   size_t label_length,
					   const uint8_t *label,
					   size_t *length,
					   uint8_t *message,
					   const uint8_t *ciphertext);

static int
rsa_decrypt_for_test(test_rsa_oaep_decrypt_func decrypt_func,
		     const struct rsa_public_key *pub,
                     const struct rsa_private_key *key,
                     void *random_ctx, nettle_random_func *random,
		     size_t label_length, const uint8_t *label,
                     size_t *length, uint8_t *message,
                     const uint8_t *ciphertext)
{
  int ret;

  /* Makes valgrind trigger on any branches depending on the input
     data. Except that (i) we have to allow rsa_sec_compute_root_tr to
     check that p and q are odd, (ii) mpn_sec_div_r may leak
     information about the most significant bits of p and q, due to
     normalization check and table lookup in invert_limb, and (iii)
     mpn_sec_powm may leak information about the least significant
     bits of p and q, due to table lookup in binvert_limb. */
  mark_bytes_undefined (*length, message);
  MARK_MPZ_LIMBS_UNDEFINED(key->a);
  MARK_MPZ_LIMBS_UNDEFINED(key->b);
  MARK_MPZ_LIMBS_UNDEFINED(key->c);
  mark_bytes_undefined ((mpz_size (key->p) - 3) * sizeof(mp_limb_t),
			mpz_limbs_read (key->p) + 1);
  mark_bytes_undefined((mpz_size (key->q) - 3) * sizeof(mp_limb_t),
		       mpz_limbs_read (key->q) + 1);

  ret = decrypt_func (pub, key, random_ctx, random, label_length, label,
		      length, message, ciphertext);

  mark_bytes_defined (sizeof(*length), length);
  mark_bytes_defined (*length, message);
  mark_bytes_defined (sizeof(ret), &ret);
  MARK_MPZ_LIMBS_DEFINED(key->a);
  MARK_MPZ_LIMBS_DEFINED(key->b);
  MARK_MPZ_LIMBS_DEFINED(key->c);
  MARK_MPZ_LIMBS_DEFINED(key->p);
  MARK_MPZ_LIMBS_DEFINED(key->q);

  return ret;
}

static void
test_rsa_oaep_encrypt_decrypt (struct rsa_public_key *pub,
			       struct rsa_private_key *key,
			       test_rsa_oaep_encrypt_func encrypt_func,
			       test_rsa_oaep_decrypt_func decrypt_func,
			       size_t label_length, const uint8_t *label,
			       size_t length, const uint8_t *message)
{
  uint8_t *ciphertext;
  uint8_t *decrypted;
  size_t decrypted_length;
  uint8_t after;
  struct knuth_lfib_ctx lfib;

  knuth_lfib_init(&lfib, 1111);

  ciphertext = xalloc (key->size + 1);
  knuth_lfib_random (&lfib, key->size + 1, ciphertext);
  after = ciphertext[key->size];

  ASSERT (encrypt_func (pub,
			&lfib, (nettle_random_func *) knuth_lfib_random,
			label_length, label,
			length, message,
			ciphertext));
  ASSERT (ciphertext[key->size] == after);

  if (verbose)
    {
      fprintf (stderr, "encrypted: ");
      print_hex (key->size, ciphertext);
      fprintf (stderr, "\n");
    }

  decrypted = xalloc (length + 1);

  knuth_lfib_random (&lfib, length + 1, decrypted);
  after = decrypted[length];

  /* Test short buffer */
  decrypted_length = length - 1;
  ASSERT (!rsa_decrypt_for_test (decrypt_func,
				 pub, key,
				 &lfib, (nettle_random_func *) knuth_lfib_random,
				 label_length, label,
				 &decrypted_length, decrypted,
				 ciphertext));

  decrypted_length = length;
  ASSERT (rsa_decrypt_for_test (decrypt_func,
				pub, key,
				&lfib, (nettle_random_func *) knuth_lfib_random,
				label_length, label,
				&decrypted_length, decrypted,
				ciphertext));
  ASSERT (decrypted_length == length);
  ASSERT (MEMEQ (length, message, decrypted));
  ASSERT (decrypted[length] == after);

  free (decrypted);
  free (ciphertext);
}

static void
test_encrypt_decrypt (void)
{
  struct rsa_public_key pub;
  struct rsa_private_key key;

  const unsigned char msg[] = "Squemish ossifrage";
  size_t msg_length = LLENGTH(msg);
  const unsigned char label[] = "This is a magic label";
  size_t label_length = LLENGTH(label);

  rsa_private_key_init(&key);
  rsa_public_key_init(&pub);

  test_rsa_set_key_2(&pub, &key);

  /* Test without label */
  test_rsa_oaep_encrypt_decrypt (&pub, &key,
				 rsa_oaep_sha256_encrypt,
				 rsa_oaep_sha256_decrypt,
				 0, NULL,
				 msg_length, msg);

  test_rsa_oaep_encrypt_decrypt (&pub, &key,
				 rsa_oaep_sha384_encrypt,
				 rsa_oaep_sha384_decrypt,
				 0, NULL,
				 msg_length, msg);

  test_rsa_oaep_encrypt_decrypt (&pub, &key,
				 rsa_oaep_sha512_encrypt,
				 rsa_oaep_sha512_decrypt,
				 0, NULL,
				 msg_length, msg);

  /* Test with label */
  test_rsa_oaep_encrypt_decrypt (&pub, &key,
				 rsa_oaep_sha256_encrypt,
				 rsa_oaep_sha256_decrypt,
				 label_length, label,
				 msg_length, msg);

  test_rsa_oaep_encrypt_decrypt (&pub, &key,
				 rsa_oaep_sha384_encrypt,
				 rsa_oaep_sha384_decrypt,
				 label_length, label,
				 msg_length, msg);

  test_rsa_oaep_encrypt_decrypt (&pub, &key,
				 rsa_oaep_sha512_encrypt,
				 rsa_oaep_sha512_decrypt,
				 label_length, label,
				 msg_length, msg);

  rsa_public_key_clear (&pub);
  rsa_private_key_clear (&key);
}

static int
rsa_oaep_sha1_encrypt (const struct rsa_public_key *key,
		       void *random_ctx, nettle_random_func *random,
		       size_t label_length, const uint8_t *label,
		       size_t length, const uint8_t *message,
		       uint8_t *ciphertext)
{
  struct sha1_ctx ctx;

  sha1_init (&ctx);

  return _rsa_oaep_encrypt (key,
			    random_ctx, random,
			    &ctx, &nettle_sha1,
			    label_length, label,
			    length, message,
			    ciphertext);
}

static int
rsa_oaep_sha1_decrypt (const struct rsa_public_key *pub,
			 const struct rsa_private_key *key,
			 void *random_ctx, nettle_random_func *random,
			 size_t label_length, const uint8_t *label,
			 size_t *length, uint8_t *message,
			 const uint8_t *ciphertext)
{
  struct sha1_ctx ctx;

  sha1_init (&ctx);

  return _rsa_oaep_decrypt (pub, key, random_ctx, random,
			    &ctx, &nettle_sha1, label_length, label,
			    length, message, ciphertext);
}

static void
random_from_seed (struct tstring *seed, size_t n, uint8_t *dst)
{
  ASSERT (n <= seed->length);
  memcpy (dst, seed->data, n);
}

static void
test_rsa_oaep_encrypt_decrypt_kat (struct rsa_public_key *pub,
				   struct rsa_private_key *key,
				   test_rsa_oaep_encrypt_func encrypt_func,
				   test_rsa_oaep_decrypt_func decrypt_func,
				   void *random_ctx, nettle_random_func *random,
				   size_t label_length, const uint8_t *label,
				   size_t length, const uint8_t *message,
				   const uint8_t *expected)
{
  uint8_t *ciphertext;
  uint8_t *decrypted;
  size_t decrypted_length;
  uint8_t after;
  /* For blinding at decryption */
  struct knuth_lfib_ctx lfib;

  knuth_lfib_init(&lfib, 1111);

  ciphertext = xalloc (key->size + 1);
  knuth_lfib_random (&lfib, key->size + 1, ciphertext);
  after = ciphertext[key->size];

  ASSERT (encrypt_func (pub,
			random_ctx, random,
			label_length, label,
			length, message,
			ciphertext));
  ASSERT (MEMEQ (key->size, ciphertext, expected));
  ASSERT (ciphertext[key->size] == after);

  decrypted = xalloc (length + 1);
  knuth_lfib_random (&lfib, length + 1, decrypted);
  after = decrypted[length];

  decrypted_length = length;

  ASSERT (rsa_decrypt_for_test (decrypt_func,
				pub, key,
				&lfib, (nettle_random_func *) knuth_lfib_random,
				label_length, label,
				&decrypted_length, decrypted,
				ciphertext));
  ASSERT (decrypted_length == length);
  ASSERT (MEMEQ (length, message, decrypted));
  ASSERT (decrypted[length] == after);

  free (decrypted);
  free (ciphertext);
}

/* The below are known answer tests constructed using the draft
 * version of PKCS #1 2.1 test vectors from RSA Laboratories:
 *
 * ftp://ftp.rsasecurity.com/pub/pkcs/pkcs-1/pkcs-1v2-1d2-vec.zip
 *
 * While the original zip file is no longer accessible, a copy is
 * kept in the python-cryptography repository, under the following license:
 * https://github.com/pyca/cryptography/tree/49bf4e408cd2f93276687f451dd28982e5d501e0/vectors/cryptography_vectors/asymmetric/RSA/pkcs-1v2-1d2-vec
 */

/*
 * Copyright (c) Individual contributors.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     1. Redistributions of source code must retain the above copyright notice,
 *        this list of conditions and the following disclaimer.
 *
 *     2. Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *
 *     3. Neither the name of PyCA Cryptography nor the names of its contributors
 *        may be used to endorse or promote products derived from this software
 *        without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

static void
test_rsa_oaep_set_key(struct rsa_public_key *pub,
		      struct rsa_private_key *key)
{
  mpz_set_str(pub->n,
	      "a8b3b284af8eb50b" "387034a860f146c4" "919f318763cd6c55"
	      "98c8ae4811a1e0ab" "c4c7e0b082d693a5" "e7fced675cf46685"
	      "12772c0cbc64a742" "c6c630f533c8cc72" "f62ae833c40bf258"
	      "42e984bb78bdbf97" "c0107d55bdb662f5" "c4e0fab9845cb514"
	      "8ef7392dd3aaff93" "ae1e6b667bb3d424" "7616d4f5ba10d4cf"
	      "d226de88d39f16fb", 16);
  mpz_set_str(pub->e, "010001", 16);

  ASSERT (rsa_public_key_prepare(pub));

  /* d is not used */
#if 0
  mpz_set_str(key->d,
	      "53339cfdb79fc846" "6a655c7316aca85c" "55fd8f6dd898fdaf"
	      "119517ef4f52e8fd" "8e258df93fee180f" "a0e4ab29693cd83b"
	      "152a553d4ac4d181" "2b8b9fa5af0e7f55" "fe7304df41570926"
	      "f3311f15c4d65a73" "2c483116ee3d3d2d" "0af3549ad9bf7cbf"
	      "b78ad884f84d5beb" "04724dc7369b31de" "f37d0cf539e9cfcd"
	      "d3de653729ead5d1" , 16);
#endif

  mpz_set_str(key->p,
	      "d32737e7267ffe13" "41b2d5c0d150a81b" "586fb3132bed2f8d"
	      "5262864a9cb9f30a" "f38be448598d413a" "172efb802c21acf1"
	      "c11c520c2f26a471" "dcad212eac7ca39d", 16);

  mpz_set_str(key->q,
	      "cc8853d1d54da630" "fac004f471f281c7" "b8982d8224a490ed"
	      "beb33d3e3d5cc93c" "4765703d1dd79164" "2f1f116a0dd852be"
	      "2419b2af72bfe9a0" "30e860b0288b5d77", 16);

  mpz_set_str(key->a,
	      "0e12bf1718e9cef5" "599ba1c3882fe804" "6a90874eefce8f2c"
	      "cc20e4f2741fb0a3" "3a3848aec9c9305f" "becbd2d76819967d"
	      "4671acc6431e4037" "968db37878e695c1", 16);

  mpz_set_str(key->b,
	      "95297b0f95a2fa67" "d00707d609dfd4fc" "05c89dafc2ef6d6e"
	      "a55bec771ea33373" "4d9251e79082ecda" "866efef13c459e1a"
	      "631386b7e354c899" "f5f112ca85d71583", 16);

  mpz_set_str(key->c,
	      "4f456c502493bdc0" "ed2ab756a3a6ed4d" "67352a697d4216e9"
	      "3212b127a63d5411" "ce6fa98d5dbefd73" "263e372814274381"
	      "8166ed7dd63687dd" "2a8ca1d2f4fbd8e1", 16);

  ASSERT (rsa_private_key_prepare(key));
  ASSERT (pub->size == key->size);
}

static void
test_encrypt (void)
{
  struct rsa_public_key pub;
  struct rsa_private_key key;

  struct tstring *seed;
  struct tstring *msg;
  struct tstring *ciphertext;

  rsa_private_key_init(&key);
  rsa_public_key_init(&pub);

  /* Test vector from RSA labs */
  test_rsa_oaep_set_key(&pub, &key);

  /* Example 1.1 */
  msg = tstring_hex ("6628194e12073db03ba94cda9ef9532397d50dba79b987004afefe34");
  seed = tstring_hex ("18b776ea21069d69776a33e96bad48e1dda0a5ef");
  ciphertext = tstring_hex ("354fe67b4a126d5d" "35fe36c777791a3f"
			    "7ba13def484e2d39" "08aff722fad468fb"
			    "21696de95d0be911" "c2d3174f8afcc201"
			    "035f7b6d8e69402d" "e5451618c21a535f"
			    "a9d7bfc5b8dd9fc2" "43f8cf927db31322"
			    "d6e881eaa91a9961" "70e657a05a266426"
			    "d98c88003f8477c1" "227094a0d9fa1e8c"
			    "4024309ce1ecccb5" "210035d47ac72e8a");
  ASSERT (ciphertext->length == key.size);

  test_rsa_oaep_encrypt_decrypt_kat (&pub, &key,
				     rsa_oaep_sha1_encrypt,
				     rsa_oaep_sha1_decrypt,
				     seed, (nettle_random_func *) random_from_seed,
				     0, NULL,
				     msg->length, msg->data,
				     ciphertext->data);

  /* Example 1.2 */
  msg = tstring_hex ("750c4047f547e8e41411856523298ac9bae245efaf1397fbe56f9dd5");
  seed = tstring_hex ("0cc742ce4a9b7f32f951bcb251efd925fe4fe35f");
  ciphertext = tstring_hex ("640db1acc58e0568" "fe5407e5f9b701df"
			    "f8c3c91e716c536f" "c7fcec6cb5b71c11"
			    "65988d4a279e1577" "d730fc7a29932e3f"
			    "00c81515236d8d8e" "31017a7a09df4352"
			    "d904cdeb79aa583a" "dcc31ea698a4c052"
			    "83daba9089be5491" "f67c1a4ee48dc74b"
			    "bbe6643aef846679" "b4cb395a352d5ed1"
			    "15912df696ffe070" "2932946d71492b44");
  ASSERT (ciphertext->length == key.size);

  test_rsa_oaep_encrypt_decrypt_kat (&pub, &key,
				     rsa_oaep_sha1_encrypt,
				     rsa_oaep_sha1_decrypt,
				     seed, (nettle_random_func *) random_from_seed,
				     0, NULL,
				     msg->length, msg->data,
				     ciphertext->data);

  /* Example 1.3 */
  msg = tstring_hex ("d94ae0832e6445ce42331cb06d531a82b1db4baad30f746dc916df24d4e3c2451fff59a6423eb0e1d02d4fe646cf699dfd818c6e97b051");
  seed = tstring_hex ("2514df4695755a67b288eaf4905c36eec66fd2fd");
  ciphertext = tstring_hex ("423736ed035f6026" "af276c35c0b3741b"
			    "365e5f76ca091b4e" "8c29e2f0befee603"
			    "595aa8322d602d2e" "625e95eb81b2f1c9"
			    "724e822eca76db86" "18cf09c5343503a4"
			    "360835b5903bc637" "e3879fb05e0ef326"
			    "85d5aec5067cd7cc" "96fe4b2670b6eac3"
			    "066b1fcf5686b685" "89aafb7d629b02d8"
			    "f8625ca3833624d4" "800fb081b1cf94eb");
  ASSERT (ciphertext->length == key.size);

  test_rsa_oaep_encrypt_decrypt_kat (&pub, &key,
				     rsa_oaep_sha1_encrypt,
				     rsa_oaep_sha1_decrypt,
				     seed, (nettle_random_func *) random_from_seed,
				     0, NULL,
				     msg->length, msg->data,
				     ciphertext->data);

  /* Example 1.4 */
  msg = tstring_hex ("52e650d98e7f2a048b4f86852153b97e01dd316f346a19f67a85");
  seed = tstring_hex ("c4435a3e1a18a68b6820436290a37cefb85db3fb");
  ciphertext = tstring_hex ("45ead4ca551e662c" "9800f1aca8283b05"
			    "25e6abae30be4b4a" "ba762fa40fd3d38e"
			    "22abefc69794f6eb" "bbc05ddbb1121624"
			    "7d2f412fd0fba87c" "6e3acd888813646f"
			    "d0e48e785204f9c3" "f73d6d8239562722"
			    "dddd8771fec48b83" "a31ee6f592c4cfd4"
			    "bc88174f3b13a112" "aae3b9f7b80e0fc6"
			    "f7255ba880dc7d80" "21e22ad6a85f0755");
  ASSERT (ciphertext->length == key.size);

  test_rsa_oaep_encrypt_decrypt_kat (&pub, &key,
				     rsa_oaep_sha1_encrypt,
				     rsa_oaep_sha1_decrypt,
				     seed, (nettle_random_func *) random_from_seed,
				     0, NULL,
				     msg->length, msg->data,
				     ciphertext->data);

  /* Example 1.5 */
  msg = tstring_hex ("8da89fd9e5f974a29feffb462b49180f6cf9e802");
  seed = tstring_hex ("b318c42df3be0f83fea823f5a7b47ed5e425a3b5");
  ciphertext = tstring_hex ("36f6e34d94a8d34d" "aacba33a2139d00a"
			    "d85a9345a86051e7" "3071620056b920e2"
			    "19005855a213a0f2" "3897cdcd731b4525"
			    "7c777fe908202bef" "dd0b58386b1244ea"
			    "0cf539a05d5d1032" "9da44e13030fd760"
			    "dcd644cfef2094d1" "910d3f433e1c7c6d"
			    "d18bc1f2df7f643d" "662fb9dd37ead905"
			    "9190f4fa66ca39e8" "69c4eb449cbdc439");
  ASSERT (ciphertext->length == key.size);

  test_rsa_oaep_encrypt_decrypt_kat (&pub, &key,
				     rsa_oaep_sha1_encrypt,
				     rsa_oaep_sha1_decrypt,
				     seed, (nettle_random_func *) random_from_seed,
				     0, NULL,
				     msg->length, msg->data,
				     ciphertext->data);

  /* Example 1.6 */
  msg = tstring_hex ("26521050844271");
  seed = tstring_hex ("e4ec0982c2336f3a677f6a356174eb0ce887abc2");
  ciphertext = tstring_hex ("42cee2617b1ecea4" "db3f4829386fbd61"
			    "dafbf038e180d837" "c96366df24c097b4"
			    "ab0fac6bdf590d82" "1c9f10642e681ad0"
			    "5b8d78b378c0f46c" "e2fad63f74e0ad3d"
			    "f06b075d7eb5f563" "6f8d403b9059ca76"
			    "1b5c62bb52aa4500" "2ea70baace08ded2"
			    "43b9d8cbd62a68ad" "e265832b56564e43"
			    "a6fa42ed199a0997" "69742df1539e8255");
  ASSERT (ciphertext->length == key.size);

  test_rsa_oaep_encrypt_decrypt_kat (&pub, &key,
				     rsa_oaep_sha1_encrypt,
				     rsa_oaep_sha1_decrypt,
				     seed, (nettle_random_func *) random_from_seed,
				     0, NULL,
				     msg->length, msg->data,
				     ciphertext->data);

  rsa_public_key_clear (&pub);
  rsa_private_key_clear (&key);
}

void
test_main (void)
{
#if NETTLE_USE_MINI_GMP || WITH_EXTRA_ASSERTS
  if (test_side_channel)
    SKIP();
#endif
  test_encrypt_decrypt ();
  test_encrypt ();
}
