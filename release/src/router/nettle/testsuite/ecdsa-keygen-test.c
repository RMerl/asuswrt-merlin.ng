#include "testutils.h"
#include "knuth-lfib.h"

void
test_main (void)
{
  unsigned i;
  struct knuth_lfib_ctx rctx;
  struct dsa_signature signature;

  struct tstring *digest;

  knuth_lfib_init (&rctx, 4711);
  dsa_signature_init (&signature);

  digest = SHEX (/* sha256("abc") */
		 "BA7816BF 8F01CFEA 414140DE 5DAE2223"
		 "B00361A3 96177A9C B410FF61 F20015AD");

  for (i = 0; ecc_curves[i]; i++)
    {
      const struct ecc_curve *ecc = ecc_curves[i];
      struct ecc_point pub;
      struct ecc_scalar key;

      if (ecc->p.bit_size == 255 || ecc->p.bit_size == 448)
	/* Exclude curve25519 and curve448, not supported with ECDSA. */
	continue;

      if (verbose)
	fprintf (stderr, "Curve %d\n", ecc->p.bit_size);

      ecc_point_init (&pub, ecc);
      ecc_scalar_init (&key, ecc);

      ecdsa_generate_keypair (&pub, &key,
			      &rctx,
			      (nettle_random_func *) knuth_lfib_random);

      if (verbose)
	{
	  fprintf (stderr, "Public key:\nx = ");
	  write_mpn (stderr, 16, pub.p, ecc->p.size);
	  fprintf (stderr, "\ny = ");
	  write_mpn (stderr, 16, pub.p + ecc->p.size, ecc->p.size);
	  fprintf (stderr, "\nPrivate key: ");
	  write_mpn (stderr, 16, key.p, ecc->p.size);
	  fprintf (stderr, "\n");
	}
      if (!test_ecc_point_valid_p (&pub))
	die ("ecdsa_generate_keypair produced an invalid point.\n");

      ecdsa_sign (&key,
		  &rctx, (nettle_random_func *) knuth_lfib_random,
		  digest->length, digest->data,
		  &signature);

      if (!ecdsa_verify (&pub, digest->length, digest->data,
			  &signature))
	die ("ecdsa_verify failed.\n");

      digest->data[3] ^= 17;
      if (ecdsa_verify (&pub, digest->length, digest->data,
			 &signature))
	die ("ecdsa_verify returned success with invalid digest.\n");
      digest->data[3] ^= 17;

      mpz_combit (signature.r, 117);
      if (ecdsa_verify (&pub, digest->length, digest->data,
			 &signature))
	die ("ecdsa_verify returned success with invalid signature.r.\n");

      mpz_combit (signature.r, 117);
      mpz_combit (signature.s, 93);
      if (ecdsa_verify (&pub, digest->length, digest->data,
			 &signature))
	die ("ecdsa_verify returned success with invalid signature.s.\n");

      ecc_point_clear (&pub);
      ecc_scalar_clear (&key);
    }
  dsa_signature_clear (&signature);
}
