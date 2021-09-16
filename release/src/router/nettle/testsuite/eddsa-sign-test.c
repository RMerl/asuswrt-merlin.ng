/* eddsa-sign-test.c

   Copyright (C) 2014 Niels Möller

   This file is part of GNU Nettle.

   GNU Nettle is free software: you can redistribute it and/or
   modify it under the terms of either:

     * the GNU Lesser General Public License as published by the Free
       Software Foundation; either version 3 of the License, or (at your
       option) any later version.

   or

     * the GNU General Public License as published by the Free
       Software Foundation; either version 2 of the License, or (at your
       option) any later version.

   or both in parallel, as here.

   GNU Nettle is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received copies of the GNU General Public License and
   the GNU Lesser General Public License along with this program.  If
   not, see http://www.gnu.org/licenses/.
*/

#include "testutils.h"

#include "eddsa.h"
#include "eddsa-internal.h"
#include "sha3.h"

static void
test_eddsa_sign (const struct ecc_curve *ecc,
		 const struct ecc_eddsa *eddsa,
		 void *ctx,
		 const struct tstring *public,
		 const struct tstring *private,
		 const struct tstring *msg,
		 const struct tstring *ref)
{
  mp_limb_t *scratch = xalloc_limbs (_eddsa_sign_itch (ecc));
  size_t nbytes = 1 + ecc->p.bit_size / 8;
  uint8_t *signature = xalloc (2*nbytes);
  uint8_t *public_out = xalloc (nbytes);
  uint8_t *digest = xalloc (2*nbytes);
  const uint8_t *k1 = digest + nbytes;
  mp_limb_t *k2 = xalloc_limbs (ecc->p.size);

  ASSERT (public->length == nbytes);
  ASSERT (private->length == nbytes);
  ASSERT (ref->length == 2*nbytes);

  _eddsa_expand_key (ecc, eddsa, ctx, private->data,
		     digest, k2);
  _eddsa_public_key (ecc, k2, public_out, scratch);

  if (!MEMEQ (nbytes, public_out, public->data))
    {
      fprintf (stderr, "Bad public key from _eddsa_expand_key + _eddsa_public_key.\n");
      fprintf (stderr, "got:");
      print_hex (nbytes, public_out);
      fprintf (stderr, "\nref:");
      tstring_print_hex (public);
      fprintf (stderr, "\n");
      abort ();
    }
  _eddsa_sign (ecc, eddsa, ctx,
	       public->data, k1, k2,
	       msg->length, msg->data, signature, scratch);

  if (!MEMEQ (2*nbytes, signature, ref->data))
    {
      fprintf (stderr, "Bad _eddsa_sign output.\n");
      fprintf (stderr, "Public key:");
      tstring_print_hex (public);
      fprintf (stderr, "\nPrivate key:");
      tstring_print_hex (private);
      fprintf (stderr, "\nk2:");
      mpn_out_str (stderr, 16, k2, ecc->p.size);
      fprintf (stderr, "\nMessage (length %u):", (unsigned) msg->length);
      tstring_print_hex (msg);      
      fprintf (stderr, "\ngot:");
      print_hex (2*nbytes, signature);
      fprintf (stderr, "\nref:");
      tstring_print_hex (ref);
      fprintf (stderr, "\n");
      abort ();
    }
  
  free (scratch);
  free (signature);
  free (digest);
  free (k2);
  free (public_out);
}

static void
test_ed25519_sign (const struct tstring *public,
		   const struct tstring *private,
		   const struct tstring *msg,
		   const struct tstring *ref)
{
  struct sha512_ctx ctx;

  sha512_init (&ctx);
  test_eddsa_sign (&_nettle_curve25519, &_nettle_ed25519_sha512, &ctx,
		   public, private, msg, ref);
}

static void
test_ed448_sign (const struct tstring *public,
		 const struct tstring *private,
		 const struct tstring *msg,
		 const struct tstring *ref)
{
  struct sha3_256_ctx ctx;

  sha3_256_init (&ctx);
  test_eddsa_sign (&_nettle_curve448, &_nettle_ed448_shake256, &ctx,
		   public, private, msg, ref);
}

void
test_main (void)
{
  /* Based on a few of the test vectors at
     http://ed25519.cr.yp.to/python/sign.input */
  test_ed25519_sign (SHEX("d75a980182b10ab7 d54bfed3c964073a"
			  "0ee172f3daa62325 af021a68f707511a"),
		     SHEX("9d61b19deffd5a60 ba844af492ec2cc4"
			  "4449c5697b326919 703bac031cae7f60"),
		     SHEX(""),
		     SHEX("e5564300c360ac72 9086e2cc806e828a"
			  "84877f1eb8e5d974 d873e06522490155"
			  "5fb8821590a33bac c61e39701cf9b46b"
			  "d25bf5f0595bbe24 655141438e7a100b"));
  test_ed25519_sign (SHEX("3d4017c3e843895a 92b70aa74d1b7ebc"
			  "9c982ccf2ec4968c c0cd55f12af4660c"),
		     SHEX("4ccd089b28ff96da 9db6c346ec114e0f"
			  "5b8a319f35aba624 da8cf6ed4fb8a6fb"),
		     SHEX("72"),
		     SHEX("92a009a9f0d4cab8 720e820b5f642540"
			  "a2b27b5416503f8f b3762223ebdb69da"
			  "085ac1e43e15996e 458f3613d0f11d8c"
			  "387b2eaeb4302aee b00d291612bb0c00"));
  test_ed25519_sign (SHEX("1ed506485b09a645 0be7c9337d9fe87e"
			  "f99c96f8bd11cd63 1ca160d0fd73067e"),
		     SHEX("f215d34fe2d757cf f9cf5c05430994de"
			  "587987ce45cb0459 f61ec6c825c62259"),
		     SHEX("fbed2a7df418ec0e 8036312ec239fcee"
			  "6ef97dc8c2df1f2e 14adee287808b788"
			  "a6072143b851d975 c8e8a0299df846b1"
			  "9113e38cee83da71 ea8e9bd6f57bdcd3"
			  "557523f4feb616ca a595aea01eb0b3d4"
			  "90b99b525ea4fbb9 258bc7fbb0deea8f"
			  "568cb2"),
		     SHEX("cbef65b6f3fd5809 69fc3340cfae4f7c"
			  "99df1340cce54626 183144ef46887163"
			  "4b0a5c0033534108 e1c67c0dc99d3014"
			  "f01084e98c95e101 4b309b1dbb2e6704"));
  /* Based on a few of the test vectors from RFC 8032 */
  test_ed448_sign (SHEX("5fd7449b59b461fd 2ce787ec616ad46a"
			"1da1342485a70e1f 8a0ea75d80e96778"
			"edf124769b46c706 1bd6783df1e50f6c"
			"d1fa1abeafe82561 80"),
		   SHEX("6c82a562cb808d10 d632be89c8513ebf"
			"6c929f34ddfa8c9f 63c9960ef6e348a3"
			"528c8a3fcc2f044e 39a3fc5b94492f8f"
			"032e7549a20098f9 5b"),
		   SHEX(""),
		   SHEX("533a37f6bbe45725 1f023c0d88f976ae"
			"2dfb504a843e34d2 074fd823d41a591f"
			"2b233f034f628281 f2fd7a22ddd47d78"
			"28c59bd0a21bfd39 80ff0d2028d4b18a"
			"9df63e006c5d1c2d 345b925d8dc00b41"
			"04852db99ac5c7cd da8530a113a0f4db"
			"b61149f05a736326 8c71d95808ff2e65"
			"2600"));
  test_ed448_sign (SHEX("43ba28f430cdff45 6ae531545f7ecd0a"
			"c834a55d9358c037 2bfa0c6c6798c086"
			"6aea01eb00742802 b8438ea4cb82169c"
			"235160627b4c3a94 80"),
		   SHEX("c4eab05d357007c6 32f3dbb48489924d"
			"552b08fe0c353a0d 4a1f00acda2c463a"
			"fbea67c5e8d2877c 5e3bc397a659949e"
			"f8021e954e0a1227 4e"),
		   SHEX("03"),
		   SHEX("26b8f91727bd6289 7af15e41eb43c377"
			"efb9c610d48f2335 cb0bd0087810f435"
			"2541b143c4b981b7 e18f62de8ccdf633"
			"fc1bf037ab7cd779 805e0dbcc0aae1cb"
			"cee1afb2e027df36 bc04dcecbf154336"
			"c19f0af7e0a64729 05e799f1953d2a0f"
			"f3348ab21aa4adaf d1d234441cf807c0"
			"3a00"));
  test_ed448_sign (SHEX("df9705f58edbab80 2c7f8363cfe5560a"
			"b1c6132c20a9f1dd 163483a26f8ac53a"
			"39d6808bf4a1dfbd 261b099bb03b3fb5"
			"0906cb28bd8a081f 00"),
		   SHEX("d65df341ad13e008 567688baedda8e9d"
			"cdc17dc024974ea5 b4227b6530e339bf"
			"f21f99e68ca6968f 3cca6dfe0fb9f4fa"
			"b4fa135d5542ea3f 01"),
		   SHEX("bd0f6a3747cd561b dddf4640a332461a"
			"4a30a12a434cd0bf 40d766d9c6d458e5"
			"512204a30c17d1f5 0b5079631f64eb31"
			"12182da300583546 1113718d1a5ef944"),
		   SHEX("554bc2480860b49e ab8532d2a533b7d5"
			"78ef473eeb58c98b b2d0e1ce488a98b1"
			"8dfde9b9b90775e6 7f47d4a1c3482058"
			"efc9f40d2ca033a0 801b63d45b3b722e"
			"f552bad3b4ccb667 da350192b61c508c"
			"f7b6b5adadc2c8d9 a446ef003fb05cba"
			"5f30e88e36ec2703 b349ca229c267083"
			"3900"));
}
