#include "testutils.h"
#include "gostdsa.h"

static void
test_gostdsa (const struct ecc_curve *ecc,
	    /* Private key */
	    const char *sz,
	    /* Random nonce */
	    const char *sk,
	    /* Hash */
	    const struct tstring *h,
	    /* Expected signature */
	    const char *r, const char *s)
{
  struct dsa_signature ref;
  mpz_t z;
  mpz_t k;
  mp_limb_t *rp = xalloc_limbs (ecc->p.size);
  mp_limb_t *sp = xalloc_limbs (ecc->p.size);
  mp_limb_t *scratch = xalloc_limbs (ecc_gostdsa_sign_itch (ecc));

  dsa_signature_init (&ref);

  mpz_init_set_str (z, sz, 16);
  mpz_init_set_str (k, sk, 16);

  ecc_gostdsa_sign (ecc, mpz_limbs_read_n (z, ecc->p.size),
		  mpz_limbs_read_n (k, ecc->p.size),
		  h->length, h->data, rp, sp, scratch);

  mpz_set_str (ref.r, r, 16);
  mpz_set_str (ref.s, s, 16);

  if (mpz_limbs_cmp (ref.r, rp, ecc->p.size) != 0
      || mpz_limbs_cmp (ref.s, sp, ecc->p.size) != 0)
    {
      fprintf (stderr, "_gostdsa_sign failed, bit_size = %u\n", ecc->p.bit_size);
      fprintf (stderr, "r     = ");
      write_mpn (stderr, 16, rp, ecc->p.size);
      fprintf (stderr, "\ns     = ");
      write_mpn (stderr, 16, sp, ecc->p.size);
      fprintf (stderr, "\nref.r = ");
      mpz_out_str (stderr, 16, ref.r);
      fprintf (stderr, "\nref.s = ");
      mpz_out_str (stderr, 16, ref.s);
      fprintf (stderr, "\n");
      abort();
    }

  free (rp);
  free (sp);
  free (scratch);

  dsa_signature_clear (&ref);
  mpz_clear (k);
  mpz_clear (z);
}

void
test_main (void)
{
  test_gostdsa (nettle_get_gost_gc256b(),
	      "BFCF1D623E5CDD3032A7C6EABB4A923C46E43D640FFEAAF2C3ED39A8FA399924", /* z */

	      "5782C53F110C596F9155D35EBD25A06A89C50391850A8FEFE33B0E270318857C", /* k */

	      SHEX("1C067E20EA6CB183F22EFB0F3C6FD2A4E6A02821CB7A1B17FACD5E1F7AA76F70"), /* h */

	      "E9323A5E88DD87FB7C724383BFFE7CECD4B9FFA2AC33BEEF73A5A1F743404F6B", /* r */

	      "5E5B9B805B01147A8492C4A162643AC615DC777B9174108F3DC276A41F987AF3"); /* s */

  test_gostdsa (nettle_get_gost_gc512a(),
	      "3FC01CDCD4EC5F972EB482774C41E66DB7F380528DFE9E67992BA05AEE462435"
	      "757530E641077CE587B976C8EEB48C48FD33FD175F0C7DE6A44E014E6BCB074B", /* z */

	      "72ABB44536656BF1618CE10BF7EADD40582304A51EE4E2A25A0A32CB0E773ABB"
	      "23B7D8FDD8FA5EEE91B4AE452F2272C86E1E2221215D405F51B5D5015616E1F6", /* k */

	      SHEX("EDC257BED45FDDE4F1457B7F5B19017A8F204184366689D938532CDBAA5CB29A"
		   "1D369DA57F8B983BE272219BD2C9A4FC57ECF7A77F34EE2E8AA553976A4766C0"), /* h */

	      "891AA75C2A6F3B4DE27E3903F61CBB0F3F85A4E3C62F39A6E4E84A7477679C6E"
	      "45008DC2774CA2FF64C12C0606FF918CAE3A50115440E9BF2971B627A882A1E8", /* r */

	      "31065479996DDBDEE180AFE22CA3CDC44B45CE4C6C83909D1D3B702922A32441"
	      "A9E11DCFBEA3D847C06B1A8A38EB1671D6C82FA21B79C99BE2EA809B10DAA5DF"); /* s */
}
