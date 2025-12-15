#include "testutils.h"

static void
test_ecdsa (const struct ecc_curve *ecc,
	    /* Public key */
	    const char *xs, const char *ys,
	    /* Hash */
	    struct tstring *h,
	    /* Valid signature */
	    const char *r, const char *s)
{
  struct ecc_point pub;
  struct dsa_signature signature;
  mpz_t x, y;

  ecc_point_init (&pub, ecc);
  dsa_signature_init (&signature);

  mpz_init_set_str (x, xs, 16);
  mpz_init_set_str (y, ys, 16);

  if (!ecc_point_set (&pub, x, y))
    die ("ecc_point_set failed.\n");

  mpz_set_str (signature.r, r, 16);
  mpz_set_str (signature.s, s, 16);

  if (!ecdsa_verify (&pub, h->length, h->data, &signature))
    {
      fprintf (stderr, "ecdsa_verify failed with valid signature.\n");
    fail:
      fprintf (stderr, "bit_size = %u\nx = ", ecc->p.bit_size);
      mpz_out_str (stderr, 16, x);
      fprintf (stderr, "\ny = ");
      mpz_out_str (stderr, 16, y);
      fprintf (stderr, "\ndigest ");
      print_hex (h->length, h->data);
      fprintf (stderr, "r = ");
      mpz_out_str (stderr, 16, signature.r);
      fprintf (stderr, "\ns = ");
      mpz_out_str (stderr, 16, signature.s);
      fprintf (stderr, "\n");
      abort();
    }

  mpz_combit (signature.r, ecc->p.bit_size / 3);
  if (ecdsa_verify (&pub, h->length, h->data, &signature))
    {
      fprintf (stderr, "ecdsa_verify unexpectedly succeeded with invalid signature (r modified).\n");
      goto fail;
    }
  mpz_combit (signature.r, ecc->p.bit_size / 3);
  
  mpz_combit (signature.s, 4*ecc->p.bit_size / 5);
  if (ecdsa_verify (&pub, h->length, h->data, &signature))
    {
      fprintf (stderr, "ecdsa_verify unexpectedly succeeded with invalid signature (s modified).\n");
      goto fail;
    }
  mpz_combit (signature.s, 4*ecc->p.bit_size / 5);

  h->data[2*h->length / 3] ^= 0x40;
  if (ecdsa_verify (&pub, h->length, h->data, &signature))
    {
      fprintf (stderr, "ecdsa_verify unexpectedly succeeded with invalid signature (h modified).\n");
      goto fail;
    }
  h->data[2*h->length / 3] ^= 0x40;
  if (!ecdsa_verify (&pub, h->length, h->data, &signature))
    {
      fprintf (stderr, "ecdsa_verify failed, internal testsuite error.\n");
      goto fail;
    }

  ecc_point_clear (&pub);
  dsa_signature_clear (&signature);
  mpz_clear (x);
  mpz_clear (y);  
}

void
test_main (void)
{
  /* Corresponds to nonce k = 2 and private key z =
     0x99b5b787484def12894ca507058b3bf543d72d82fa7721d2e805e5e6. z and
     hash are chosen so that intermediate scalars in the verify
     equations are u1 = 0x6b245680e700, u2 =
     259da6542d4ba7d21ad916c3bd57f811. These values require canonical
     reduction of the scalars. Bug caused by missing canonical
     reduction reported by Guido Vranken. */
  test_ecdsa (&_nettle_secp_224r1,
	      "9e7e6cc6b1bdfa8ee039b66ad85e5490"
	      "7be706a900a3cba1c8fdd014", /* x */
	      "74855db3f7c1b4097ae095745fc915e3"
	      "8a79d2a1de28f282eafb22ba", /* y */

	      SHEX("cdb887ac805a3b42e22d224c85482053"
		   "16c755d4a736bb2032c92553"),
	      "706a46dc76dcb76798e60e6d89474788"
	      "d16dc18032d268fd1a704fa6", /* r */
	      "3a41e1423b1853e8aa89747b1f987364"
	      "44705d6d6d8371ea1f578f2e"); /* s */

  /* Test case provided by Guido Vranken, from oss-fuzz */
  test_ecdsa (&_nettle_secp_192r1,
	      "14683086 f1734c6d e68743a6 48181b54 a74d4c5b 383eb6a8", /* x */
	      "  1e2584 2ab8b2b0 4017f655 1b5e4058 a2aa0612 2dae9344", /* y */
	      SHEX("00"), /* h == 0 corner case*/
	      "952800792ed19341fdeeec047f2514f3b0f150d6066151fb", /* r */
	      "ec5971222014878b50d7a19d8954bc871e7e65b00b860ffb"); /* s */

  /* Test case provided by Guido Vranken, from oss-fuzz. Triggers
     point duplication in the verify operation by using private key =
     1 (public key = generator) and hash = r. */
  test_ecdsa (&_nettle_secp_256r1,
	      "6B17D1F2E12C4247F8BCE6E563A440F2"
	      "77037D812DEB33A0F4A13945D898C296", /* x */
	      "4FE342E2FE1A7F9B8EE7EB4A7C0F9E16"
	      "2BCE33576B315ECECBB6406837BF51F5", /* y */
	      SHEX("6ff03b949241ce1dadd43519e6960e0a"
		   "85b41a69a05c328103aa2bce1594ca16"), /* hash */
	      "6ff03b949241ce1dadd43519e6960e0a"
	      "85b41a69a05c328103aa2bce1594ca16", /* r */
	      "53f097727a0e0dc284a0daa0da0ab77d"
	      "5792ae67ed075d1f8d5bda0f853fa093"); /* s */

  /* From RFC 4754 */
  test_ecdsa (&_nettle_secp_256r1,
	      "2442A5CC 0ECD015F A3CA31DC 8E2BBC70"
	      "BF42D60C BCA20085 E0822CB0 4235E970",  /* x */

	      "6FC98BD7 E50211A4 A27102FA 3549DF79"
	      "EBCB4BF2 46B80945 CDDFE7D5 09BBFD7D",  /* y */

	      SHEX("BA7816BF 8F01CFEA 414140DE 5DAE2223"
		   "B00361A3 96177A9C B410FF61 F20015AD"),  /* h */
	      
	      "CB28E099 9B9C7715 FD0A80D8 E47A7707"
	      "9716CBBF 917DD72E 97566EA1 C066957C",  /* r */
	      "86FA3BB4 E26CAD5B F90B7F81 899256CE"
	      "7594BB1E A0C89212 748BFF3B 3D5B0315"); /* s */

  test_ecdsa (&_nettle_secp_384r1,
	      "96281BF8 DD5E0525 CA049C04 8D345D30"
	      "82968D10 FEDF5C5A CA0C64E6 465A97EA"
	      "5CE10C9D FEC21797 41571072 1F437922",  /* x */

	      "447688BA 94708EB6 E2E4D59F 6AB6D7ED"
	      "FF9301D2 49FE49C3 3096655F 5D502FAD"
	      "3D383B91 C5E7EDAA 2B714CC9 9D5743CA",  /* y */

	      SHEX("CB00753F 45A35E8B B5A03D69 9AC65007"
		   "272C32AB 0EDED163 1A8B605A 43FF5BED"
		   "8086072B A1E7CC23 58BAECA1 34C825A7"),  /* h */

	      "FB017B91 4E291494 32D8BAC2 9A514640"
	      "B46F53DD AB2C6994 8084E293 0F1C8F7E"
	      "08E07C9C 63F2D21A 07DCB56A 6AF56EB3",  /* r */
	      "B263A130 5E057F98 4D38726A 1B468741"
	      "09F417BC A112674C 528262A4 0A629AF1"
	      "CBB9F516 CE0FA7D2 FF630863 A00E8B9F"); /* s*/

  test_ecdsa (&_nettle_secp_521r1,
	      "0151518F 1AF0F563 517EDD54 85190DF9"
	      "5A4BF57B 5CBA4CF2 A9A3F647 4725A35F"
	      "7AFE0A6D DEB8BEDB CD6A197E 592D4018"
	      "8901CECD 650699C9 B5E456AE A5ADD190"
	      "52A8", /* x */

	      "006F3B14 2EA1BFFF 7E2837AD 44C9E4FF"
	      "6D2D34C7 3184BBAD 90026DD5 E6E85317"
	      "D9DF45CA D7803C6C 20035B2F 3FF63AFF"
	      "4E1BA64D 1C077577 DA3F4286 C58F0AEA"
	      "E643", /* y */

	      SHEX("DDAF35A1 93617ABA CC417349 AE204131" 
		   "12E6FA4E 89A97EA2 0A9EEEE6 4B55D39A"
		   "2192992A 274FC1A8 36BA3C23 A3FEEBBD" 
		   "454D4423 643CE80E 2A9AC94F A54CA49F"), /* h */

	      "0154FD38 36AF92D0 DCA57DD5 341D3053" 
	      "988534FD E8318FC6 AAAAB68E 2E6F4339"
	      "B19F2F28 1A7E0B22 C269D93C F8794A92" 
	      "78880ED7 DBB8D936 2CAEACEE 54432055"
	      "2251", /* r */
	      "017705A7 030290D1 CEB605A9 A1BB03FF"
	      "9CDD521E 87A696EC 926C8C10 C8362DF4"
	      "97536710 1F67D1CF 9BCCBF2F 3D239534" 
	      "FA509E70 AAC851AE 01AAC68D 62F86647"
	      "2660"); /* s */
}
