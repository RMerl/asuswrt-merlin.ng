/* eddsa-verify-test.c

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
test_eddsa (const struct ecc_curve *ecc,
	    const struct ecc_eddsa *eddsa,
	    void *ctx,
	    const uint8_t *pub,
	    const struct tstring *msg,
	    const uint8_t *signature)
{
  mp_limb_t *A = xalloc_limbs (ecc_size_a (ecc));
  mp_limb_t *scratch = xalloc_limbs (_eddsa_verify_itch (ecc));
  size_t nbytes = 1 + ecc->p.bit_size / 8;
  uint8_t *cmsg = xalloc (msg->length);
  uint8_t *csignature = xalloc (2*nbytes);

  if (!_eddsa_decompress (ecc, A, pub, scratch))
    die ("Invalid eddsa public key.\n");

  memcpy (csignature, signature, 2*nbytes);
  if (!_eddsa_verify (ecc, eddsa, pub, A, ctx,
		      msg->length, msg->data, csignature, scratch))
    {
      fprintf (stderr, "eddsa_verify failed with valid signature.\n");
    fail:
      fprintf (stderr, "bit_size = %u\npub = ", ecc->p.bit_size);
      print_hex (nbytes, pub);
      fprintf (stderr, "\nmsg = ");
      tstring_print_hex (msg);
      fprintf (stderr, "\nsign = ");
      print_hex (2*nbytes, csignature);
      fprintf (stderr, "\n");
      abort();
    }

  memcpy (csignature, signature, 2*nbytes);
  csignature[nbytes/3] ^= 0x40;
  if (_eddsa_verify (ecc, eddsa, pub, A, ctx,
		     msg->length, msg->data, csignature, scratch))
    {
      fprintf (stderr,
	       "ecdsa_verify unexpectedly succeeded with invalid signature r.\n");
      goto fail;
    }

  memcpy (csignature, signature, 2*nbytes);
  csignature[5*nbytes/3] ^= 0x8;

  if (_eddsa_verify (ecc, eddsa, pub, A, ctx,
		     msg->length, msg->data, csignature, scratch))
    {
      fprintf (stderr,
	       "ecdsa_verify unexpectedly succeeded with invalid signature s.\n");
      goto fail;
    }

  if (msg->length == 0)
    {
      if (_eddsa_verify  (ecc, eddsa, pub, A, ctx,
			  LDATA("foo"), signature, scratch))
	{
	  fprintf (stderr,
		   "ecdsa_verify unexpectedly succeeded with different message.\n");
	  goto fail;
	}
    }
  else
    {
      if (_eddsa_verify  (ecc, eddsa, pub, A, ctx,
			  msg->length - 1, msg->data,
			  signature, scratch))
	{
	  fprintf (stderr,
		   "ecdsa_verify unexpectedly succeeded with truncated message.\n");
	  goto fail;
	}
      memcpy (cmsg, msg->data, msg->length);
      cmsg[2*msg->length / 3] ^= 0x20;
      if (_eddsa_verify  (ecc, eddsa, pub, A, ctx,
			  msg->length, cmsg, signature, scratch))
	{
	  fprintf (stderr,
		   "ecdsa_verify unexpectedly succeeded with modified message.\n");
	  goto fail;
	}
    }
  free (A);
  free (scratch);
  free (cmsg);
  free (csignature);
}

static void
test_ed25519 (const uint8_t *pub,
	      const struct tstring *msg,
	      const uint8_t *signature)
{
  struct sha512_ctx ctx;

  sha512_init (&ctx);
  test_eddsa (&_nettle_curve25519, &_nettle_ed25519_sha512, &ctx,
	      pub, msg, signature);
}

static void
test_ed448 (const uint8_t *pub,
	    const struct tstring *msg,
	    const uint8_t *signature)
{
  struct sha3_256_ctx ctx;

  sha3_256_init (&ctx);
  test_eddsa (&_nettle_curve448, &_nettle_ed448_shake256, &ctx,
	      pub, msg, signature);
}

void
test_main (void)
{
  test_ed25519 (H("d75a980182b10ab7 d54bfed3c964073a"
		  "0ee172f3daa62325 af021a68f707511a"),
		SHEX(""),
		H("e5564300c360ac72 9086e2cc806e828a"
		  "84877f1eb8e5d974 d873e06522490155"
		  "5fb8821590a33bac c61e39701cf9b46b"
		  "d25bf5f0595bbe24 655141438e7a100b"));
  test_ed25519 (H("3d4017c3e843895a 92b70aa74d1b7ebc"
		  "9c982ccf2ec4968c c0cd55f12af4660c"),
		SHEX("72"),
		H("92a009a9f0d4cab8 720e820b5f642540"
		  "a2b27b5416503f8f b3762223ebdb69da"
		  "085ac1e43e15996e 458f3613d0f11d8c"
		  "387b2eaeb4302aee b00d291612bb0c00"));
  test_ed25519 (H("1ed506485b09a645 0be7c9337d9fe87e"
		  "f99c96f8bd11cd63 1ca160d0fd73067e"),
		SHEX("fbed2a7df418ec0e 8036312ec239fcee"
		     "6ef97dc8c2df1f2e 14adee287808b788"
		     "a6072143b851d975 c8e8a0299df846b1"
		     "9113e38cee83da71 ea8e9bd6f57bdcd3"
		     "557523f4feb616ca a595aea01eb0b3d4"
		     "90b99b525ea4fbb9 258bc7fbb0deea8f"
		     "568cb2"),
		H("cbef65b6f3fd5809 69fc3340cfae4f7c"
		  "99df1340cce54626 183144ef46887163"
		  "4b0a5c0033534108 e1c67c0dc99d3014"
		  "f01084e98c95e101 4b309b1dbb2e6704"));
  /* Based on a few of the test vectors from RFC 8032 */
  test_ed448 (H("5fd7449b59b461fd 2ce787ec616ad46a"
		"1da1342485a70e1f 8a0ea75d80e96778"
		"edf124769b46c706 1bd6783df1e50f6c"
		"d1fa1abeafe82561 80"),
	      SHEX(""),
	      H("533a37f6bbe45725 1f023c0d88f976ae"
		"2dfb504a843e34d2 074fd823d41a591f"
		"2b233f034f628281 f2fd7a22ddd47d78"
		"28c59bd0a21bfd39 80ff0d2028d4b18a"
		"9df63e006c5d1c2d 345b925d8dc00b41"
		"04852db99ac5c7cd da8530a113a0f4db"
		"b61149f05a736326 8c71d95808ff2e65"
		"2600"));
  test_ed448 (H("43ba28f430cdff45 6ae531545f7ecd0a"
		"c834a55d9358c037 2bfa0c6c6798c086"
		"6aea01eb00742802 b8438ea4cb82169c"
		"235160627b4c3a94 80"),
	      SHEX("03"),
	      H("26b8f91727bd6289 7af15e41eb43c377"
		"efb9c610d48f2335 cb0bd0087810f435"
		"2541b143c4b981b7 e18f62de8ccdf633"
		"fc1bf037ab7cd779 805e0dbcc0aae1cb"
		"cee1afb2e027df36 bc04dcecbf154336"
		"c19f0af7e0a64729 05e799f1953d2a0f"
		"f3348ab21aa4adaf d1d234441cf807c0"
		"3a00"));
  test_ed448 (H("df9705f58edbab80 2c7f8363cfe5560a"
		"b1c6132c20a9f1dd 163483a26f8ac53a"
		"39d6808bf4a1dfbd 261b099bb03b3fb5"
		"0906cb28bd8a081f 00"),
	      SHEX("bd0f6a3747cd561b dddf4640a332461a"
		   "4a30a12a434cd0bf 40d766d9c6d458e5"
		   "512204a30c17d1f5 0b5079631f64eb31"
		   "12182da300583546 1113718d1a5ef944"),
	      H("554bc2480860b49e ab8532d2a533b7d5"
		"78ef473eeb58c98b b2d0e1ce488a98b1"
		"8dfde9b9b90775e6 7f47d4a1c3482058"
		"efc9f40d2ca033a0 801b63d45b3b722e"
		"f552bad3b4ccb667 da350192b61c508c"
		"f7b6b5adadc2c8d9 a446ef003fb05cba"
		"5f30e88e36ec2703 b349ca229c267083"
		"3900"));
}
