/* chacha-test.c

   Test program for the ChaCha stream cipher implementation.

   Copyright (C) 2013 Joachim Strömbergson
   Copyright (C) 2012, 2014 Niels Möller

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

#include "chacha.h"
#include "chacha-internal.h"

static int
memzero_p (const uint8_t *p, size_t n)
{
  size_t i;
  for (i = 0; i < n; i++)
    if (p[i])
      return 0;
  return 1;
}

/* Test with simple structure of the salsa20 input, to aid
   debugging. */
static void
test_chacha_core(void)
{
  const uint32_t input[16] =
    {
     0, 1, 2, 3,
     4, 5, 6, 7,
     8, 9, 10, 11,
     /* Second block will have carry from first counter word propagate
	to next word. */
     0xffffffff, 13, 14, 15,
    };
  const struct tstring *expected
    = SHEX("32f216b0eddeee3b bade2bf5a4c0b3b3"
	   "0aab2d67b09b2a63 6127fc965d831b2c"
	   "ddc9e25ca7841f3e 938e3566a7702a0b"
	   "7f80559e639ef6da 6d39627abc7da6b1"
	   "0090a54241e68b6b d870f3b60adcaf89"
	   "09d3c7b8a8c76aa3 941d726c649636db"
	   "d6c3f0490fd38a46 070b77a757972126"
	   "6323aa95eef9d68c 7eac86e913caa80c"
	   "17dd18ae19b0b72e 0ef6e66a58c0791e"
	   "e574e44162c99484 68085365916e0fee"
	   "e3d0a5d3d2b93b4a ff245cb557af3ead"
	   "2395f5cc7a00e25a 4f69a17969360781");

  const struct tstring *expected_32 /* For 32-bit counter */
    = SHEX("32f216b0eddeee3b bade2bf5a4c0b3b3"
	   "0aab2d67b09b2a63 6127fc965d831b2c"
	   "ddc9e25ca7841f3e 938e3566a7702a0b"
	   "7f80559e639ef6da 6d39627abc7da6b1"
	   "ea56196ace461eeb f898ade2f51c425f"
	   "ff0452d728f13505 e23a1d017b40becd"
	   "6482114a4586f48a 85c5cb9f92333de6"
	   "9c248f2a809275fa 4786d5d6854fd7d7"
	   "77dd6b03073f9dbf 294eabd6affa3104"
	   "fccb19c3182a330c af2fdf0c43ebfa52"
	   "7f845ffc0a897bea 2cf27a3dfc6f31af"
	   "7db66563de442b71 f6d51f96930587ef");

  struct chacha_ctx ctx;
  uint8_t output[192];

  ASSERT (expected->length == 192);
  ASSERT (expected_32->length == 192);

  /* Three blocks, to exercises _chacha_3core, if available. */
  memcpy (&ctx, input, sizeof(ctx));
  chacha_crypt (&ctx, 192, output, expected->data);

  if (!memzero_p (output, 192))
    {
      fprintf(stderr, "chacha_crypt failed:\n");
      fprintf(stderr, "\nOutput: ");
      print_hex(192, output);
      fprintf(stderr, "\n");
      FAIL();
    }

  memcpy (&ctx, input, sizeof(ctx));
  chacha_crypt32 (&ctx, 192, output, expected_32->data);

  if (!memzero_p (output, 192))
    {
      fprintf(stderr, "chacha_crypt32 failed:\n");
      fprintf(stderr, "\nOutput: ");
      print_hex(192, output);
      fprintf(stderr, "\n");
      FAIL();
    }
}

/* For tests with non-standard number of rounds, calling
   _nettle_chacha_core directly. */
static void
test_chacha_rounds(const struct tstring *key, const struct tstring *nonce,
		   const struct tstring *expected, unsigned rounds)
{
  struct chacha_ctx ctx;
  uint32_t out[_CHACHA_STATE_LENGTH];
  ASSERT (expected->length == CHACHA_BLOCK_SIZE);

  ASSERT (key->length == CHACHA_KEY_SIZE);
  chacha_set_key (&ctx, key->data);

  ASSERT (nonce->length == CHACHA_NONCE_SIZE);
  chacha_set_nonce(&ctx, nonce->data);

  _nettle_chacha_core (out, ctx.state, rounds);

  if (!MEMEQ(CHACHA_BLOCK_SIZE, out, expected->data))
    {
      printf("Error, expected:\n");
      tstring_print_hex (expected);
      printf("Got:\n");
      print_hex(CHACHA_BLOCK_SIZE, (uint8_t *) out);
      FAIL ();
    }

  if (verbose)
    {
      printf("Result after encryption:\n");
      print_hex(CHACHA_BLOCK_SIZE, (uint8_t *) out);
    }
}

static void
_test_chacha(const struct tstring *key, const struct tstring *nonce,
	     const struct tstring *expected, const struct tstring *counter)
{
  struct chacha_ctx ctx;
  nettle_set_key_func *set_nonce;
  nettle_set_key_func *set_counter;
  nettle_crypt_func *crypt;
  uint8_t *data = xalloc (expected->length + 2);
  size_t length;
  data++;

  ASSERT (key->length == CHACHA_KEY_SIZE);
  chacha_set_key (&ctx, key->data);

  switch (nonce->length)
    {
    case CHACHA_NONCE_SIZE:
      set_nonce = (nettle_set_key_func *) chacha_set_nonce;
      set_counter = (nettle_set_key_func *) chacha_set_counter;
      crypt = (nettle_crypt_func *) chacha_crypt;
      if (counter)
	ASSERT (counter->length == CHACHA_COUNTER_SIZE);
      break;
    case CHACHA_NONCE96_SIZE:
      set_nonce = (nettle_set_key_func *) chacha_set_nonce96;
      set_counter = (nettle_set_key_func *) chacha_set_counter32;
      crypt = (nettle_crypt_func *) chacha_crypt32;
      if (counter)
	ASSERT (counter->length == CHACHA_COUNTER32_SIZE);
      break;
    default:
      die ("Bad nonce size %u.\n", (unsigned) nonce->length);
    }

  for (length = 1; length <= expected->length; length++)
    {
      size_t offset;

      data[-1] = 17;
      memset (data, 0, length);
      data[length] = 17;

      set_nonce (&ctx, nonce->data);

      if (counter)
	set_counter (&ctx, counter->data);

      crypt (&ctx, length, data, data);

      ASSERT (data[-1] == 17);
      ASSERT (data[length] == 17);
      if (!MEMEQ(length, data, expected->data))
	{
	  printf("Error, length %u, expected:\n", (unsigned) length);
	  print_hex (length, expected->data);
	  printf("Got:\n");
	  print_hex(length, data);
	  FAIL ();
	}
      /* Round up to next block boundary. */
      offset = (length + CHACHA_BLOCK_SIZE - 1) & -CHACHA_BLOCK_SIZE;
      if (offset < expected->length)
	{
	  memset(data, 0, expected->length - offset);
	  data[expected->length - offset] = 17;
	  crypt (&ctx, expected->length - offset, data, data);
	  if (!MEMEQ(expected->length - offset, data, expected->data + offset))
	    {
	      printf("Error, length %u, offset %u, remaining %u, expected:\n",
		     (unsigned) length, (unsigned) offset,
		     (unsigned) (expected->length - offset));
	      print_hex (expected->length - offset, expected->data + offset);
	      printf("Got:\n");
	      print_hex(expected->length - offset, data);
	      FAIL ();
	    }
	}
    }
  if (verbose)
    {
      printf("Result after encryption:\n");
      print_hex(expected->length, data);
    }
  free (data - 1);
}

static void
test_chacha(const struct tstring *key, const struct tstring *nonce,
	    const struct tstring *expected)
{
  _test_chacha(key, nonce, expected, NULL);
}

static void
test_chacha_with_counter(const struct tstring *key, const struct tstring *nonce,
			 const struct tstring *expected,
			 const struct tstring *counter)
{
  _test_chacha(key, nonce, expected, counter);
}

void
test_main(void)
{
  test_chacha_core();

  /* Test vectors from draft-strombergson-chacha-test-vectors */
  test_chacha_rounds (SHEX("0000000000000000 0000000000000000"
			   "0000000000000000 0000000000000000"),
		      SHEX("0000000000000000"),
		      SHEX("3e00ef2f895f40d6 7f5bb8e81f09a5a1"
			   "2c840ec3ce9a7f3b 181be188ef711a1e"
			   "984ce172b9216f41 9f445367456d5619"
			   "314a42a3da86b001 387bfdb80e0cfe42"

			   /* "d2aefa0deaa5c151 bf0adb6c01f2a5ad"
			      "c0fd581259f9a2aa dcf20f8fd566a26b"
			      "5032ec38bbc5da98 ee0c6f568b872a65"
			      "a08abf251deb21bb 4b56e5d8821e68aa" */),
		      8);

  test_chacha_rounds (SHEX("0000000000000000 0000000000000000"
			   "0000000000000000 0000000000000000"),
		      SHEX("0000000000000000"),
		      SHEX("9bf49a6a0755f953 811fce125f2683d5"
			   "0429c3bb49e07414 7e0089a52eae155f"
			   "0564f879d27ae3c0 2ce82834acfa8c79"
			   "3a629f2ca0de6919 610be82f411326be"

			   /* "0bd58841203e74fe 86fc71338ce0173d"
			      "c628ebb719bdcbcc 151585214cc089b4"
			      "42258dcda14cf111 c602b8971b8cc843"
			      "e91e46ca905151c0 2744a6b017e69316" */),
		      12);

  test_chacha (SHEX("0000000000000000 0000000000000000"
		    "0000000000000000 0000000000000000"),
	       SHEX("0000000000000000"),
	       SHEX("76b8e0ada0f13d90 405d6ae55386bd28"
		    "bdd219b8a08ded1a a836efcc8b770dc7"
		    "da41597c5157488d 7724e03fb8d84a37"
		    "6a43b8f41518a11c c387b669b2ee6586"

		    "9f07e7be5551387a 98ba977c732d080d"
		    "cb0f29a048e36569 12c6533e32ee7aed"
		    "29b721769ce64e43 d57133b074d839d5"
		    "31ed1f28510afb45 ace10a1f4b794d6f"));

  /* TC2: Single bit in key set. All zero IV */
  test_chacha_rounds (SHEX("0100000000000000 0000000000000000"
			   "0000000000000000 0000000000000000"),
		      SHEX("0000000000000000"),
		      SHEX("cf5ee9a0494aa961 3e05d5ed725b804b"
			   "12f4a465ee635acc 3a311de8740489ea"
			   "289d04f43c7518db 56eb4433e498a123"
			   "8cd8464d3763ddbb 9222ee3bd8fae3c8"),
		      8);

  test_chacha_rounds (SHEX("0100000000000000 0000000000000000"
			   "0000000000000000 0000000000000000"),
		      SHEX("0000000000000000"),
		      SHEX("12056e595d56b0f6 eef090f0cd25a209"
			   "49248c2790525d0f 930218ff0b4ddd10"
			   "a6002239d9a454e2 9e107a7d06fefdfe"
			   "f0210feba044f9f2 9b1772c960dc29c0"),
		      12);

  test_chacha (SHEX("0100000000000000 0000000000000000"
		    "0000000000000000 0000000000000000"),
	       SHEX("0000000000000000"),
	       SHEX("c5d30a7ce1ec1193 78c84f487d775a85"
		    "42f13ece238a9455 e8229e888de85bbd"
		    "29eb63d0a17a5b99 9b52da22be4023eb"
		    "07620a54f6fa6ad8 737b71eb0464dac0"

		    "10f656e6d1fd5505 3e50c4875c9930a3"
		    "3f6d0263bd14dfd6 ab8c70521c19338b"
		    "2308b95cf8d0bb7d 202d2102780ea352"
		    "8f1cb48560f76b20 f382b942500fceac"));

  /* TC3: Single bit in IV set. All zero key */
  test_chacha_rounds (SHEX("0000000000000000 0000000000000000"
			   "0000000000000000 0000000000000000"),
		      SHEX("0100000000000000"),
		      SHEX("2b8f4bb3798306ca 5130d47c4f8d4ed1"
			   "3aa0edccc1be6942 090faeeca0d7599b"
			   "7ff0fe616bb25aa0 153ad6fdc88b9549"
			   "03c22426d478b97b 22b8f9b1db00cf06"),
		      8);

  test_chacha_rounds (SHEX("0000000000000000 0000000000000000"
			   "0000000000000000 0000000000000000"),
		      SHEX("0100000000000000"),
		      SHEX("64b8bdf87b828c4b 6dbaf7ef698de03d"
			   "f8b33f635714418f 9836ade59be12969"
			   "46c953a0f38ecffc 9ecb98e81d5d99a5"
			   "edfc8f9a0a45b9e4 1ef3b31f028f1d0f"),
		      12);

  test_chacha (SHEX("0000000000000000 0000000000000000"
		    "0000000000000000 0000000000000000"),
	       SHEX("0100000000000000"),
	       SHEX("ef3fdfd6c61578fb f5cf35bd3dd33b80"
		    "09631634d21e42ac 33960bd138e50d32"
		    "111e4caf237ee53c a8ad6426194a8854"
		    "5ddc497a0b466e7d 6bbdb0041b2f586b"

		    "5305e5e44aff19b2 35936144675efbe4"
		    "409eb7e8e5f1430f 5f5836aeb49bb532"
		    "8b017c4b9dc11f8a 03863fa803dc71d5"
		    "726b2b6b31aa3270 8afe5af1d6b69058"));

  /* TC4: All bits in key and IV are set. */
  test_chacha_rounds (SHEX("ffffffffffffffff ffffffffffffffff"
			   "ffffffffffffffff ffffffffffffffff"),
		      SHEX("ffffffffffffffff"),
		      SHEX("e163bbf8c9a739d1 8925ee8362dad2cd"
			   "c973df05225afb2a a26396f2a9849a4a"
			   "445e0547d31c1623 c537df4ba85c70a9"
			   "884a35bcbf3dfab0 77e98b0f68135f54"),
		      8);

  test_chacha_rounds (SHEX("ffffffffffffffff ffffffffffffffff"
			   "ffffffffffffffff ffffffffffffffff"),
		      SHEX("ffffffffffffffff"),
		      SHEX("04bf88dae8e47a22 8fa47b7e6379434b"
			   "a664a7d28f4dab84 e5f8b464add20c3a"
			   "caa69c5ab221a23a 57eb5f345c96f4d1"
			   "322d0a2ff7a9cd43 401cd536639a615a"),
		      12);

  test_chacha (SHEX("ffffffffffffffff ffffffffffffffff"
		    "ffffffffffffffff ffffffffffffffff"),
	       SHEX("ffffffffffffffff"),
	       SHEX("d9bf3f6bce6ed0b5 4254557767fb5744"
		    "3dd4778911b60605 5c39cc25e674b836"
		    "3feabc57fde54f79 0c52c8ae43240b79"
		    "d49042b777bfd6cb 80e931270b7f50eb"

		    "5bac2acd86a836c5 dc98c116c1217ec3"
		    "1d3a63a9451319f0 97f3b4d6dab07787"
		    "19477d24d24b403a 12241d7cca064f79"
		    "0f1d51ccaff6b166 7d4bbca1958c4306"));

  /* TC5: Every even bit set in key and IV. */
  test_chacha_rounds (SHEX("5555555555555555 5555555555555555"
			   "5555555555555555 5555555555555555"),
		      SHEX("5555555555555555"),
		      SHEX("7cb78214e4d3465b 6dc62cf7a1538c88"
			   "996952b4fb72cb61 05f1243ce3442e29"
			   "75a59ebcd2b2a598 290d7538491fe65b"
			   "dbfefd060d887981 20a70d049dc2677d"),
		      8);

  test_chacha_rounds (SHEX("5555555555555555 5555555555555555"
			   "5555555555555555 5555555555555555"),
		      SHEX("5555555555555555"),
		      SHEX("a600f07727ff93f3 da00dd74cc3e8bfb"
			   "5ca7302f6a0a2944 953de00450eecd40"
			   "b860f66049f2eaed 63b2ef39cc310d2c"
			   "488f5d9a241b615d c0ab70f921b91b95"),
		      12);

  test_chacha (SHEX("5555555555555555 5555555555555555"
		    "5555555555555555 5555555555555555"),
	       SHEX("5555555555555555"),
	       SHEX("bea9411aa453c543 4a5ae8c92862f564"
		    "396855a9ea6e22d6 d3b50ae1b3663311"
		    "a4a3606c671d605c e16c3aece8e61ea1"
		    "45c59775017bee2f a6f88afc758069f7"

		    "e0b8f676e644216f 4d2a3422d7fa36c6"
		    "c4931aca950e9da4 2788e6d0b6d1cd83"
		    "8ef652e97b145b14 871eae6c6804c700"
		    "4db5ac2fce4c68c7 26d004b10fcaba86"));

  /* TC6: Every odd bit set in key and IV. */
  test_chacha_rounds (SHEX("aaaaaaaaaaaaaaaa aaaaaaaaaaaaaaaa"
			   "aaaaaaaaaaaaaaaa aaaaaaaaaaaaaaaa"),
		      SHEX("aaaaaaaaaaaaaaaa"),
		      SHEX("40f9ab86c8f9a1a0 cdc05a75e5531b61"
			   "2d71ef7f0cf9e387 df6ed6972f0aae21"
			   "311aa581f816c90e 8a99de990b6b95aa"
			   "c92450f4e1127126 67b804c99e9c6eda"),
		      8);

  test_chacha_rounds (SHEX("aaaaaaaaaaaaaaaa aaaaaaaaaaaaaaaa"
			   "aaaaaaaaaaaaaaaa aaaaaaaaaaaaaaaa"),
		      SHEX("aaaaaaaaaaaaaaaa"),
		      SHEX("856505b01d3b47aa e03d6a97aa0f033a"
			   "9adcc94377babd86 08864fb3f625b6e3"
			   "14f086158f9f725d 811eeb953b7f7470"
			   "76e4c3f639fa841f ad6c9a709e621397"),
		      12);

  test_chacha (SHEX("aaaaaaaaaaaaaaaa aaaaaaaaaaaaaaaa"
		    "aaaaaaaaaaaaaaaa aaaaaaaaaaaaaaaa"),
	       SHEX("aaaaaaaaaaaaaaaa"),
	       SHEX("9aa2a9f656efde5a a7591c5fed4b35ae"
		    "a2895dec7cb4543b 9e9f21f5e7bcbcf3"
		    "c43c748a970888f8 248393a09d43e0b7"
		    "e164bc4d0b0fb240 a2d72115c4808906"

		    "72184489440545d0 21d97ef6b693dfe5"
		    "b2c132d47e6f041c 9063651f96b623e6"
		    "2a11999a23b6f7c4 61b2153026ad5e86"
		    "6a2e597ed07b8401 dec63a0934c6b2a9"));

  /* TC7: Sequence patterns in key and IV. */
  test_chacha_rounds (SHEX("0011223344556677 8899aabbccddeeff"
			   "ffeeddccbbaa9988 7766554433221100"),
		      SHEX("0f1e2d3c4b5a6978"),
		      SHEX("db43ad9d1e842d12 72e4530e276b3f56"
			   "8f8859b3f7cf6d9d 2c74fa53808cb515"
			   "7a8ebf46ad3dcc4b 6c7dadde131784b0"
			   "120e0e22f6d5f9ff a7407d4a21b695d9"),
		      8);

  test_chacha_rounds (SHEX("0011223344556677 8899aabbccddeeff"
			   "ffeeddccbbaa9988 7766554433221100"),
		      SHEX("0f1e2d3c4b5a6978"),
		      SHEX("7ed12a3a63912ae9 41ba6d4c0d5e862e"
			   "568b0e5589346935 505f064b8c2698db"
			   "f7d850667d8e67be 639f3b4f6a16f92e"
			   "65ea80f6c7429445 da1fc2c1b9365040"),
		      12);

  test_chacha (SHEX("0011223344556677 8899aabbccddeeff"
		    "ffeeddccbbaa9988 7766554433221100"),
	       SHEX("0f1e2d3c4b5a6978"),
	       SHEX("9fadf409c00811d0 0431d67efbd88fba"
		    "59218d5d6708b1d6 85863fabbb0e961e"
		    "ea480fd6fb532bfd 494b215101505742"
		    "3ab60a63fe4f55f7 a212e2167ccab931"

		    "fbfd29cf7bc1d279 eddf25dd316bb884"
		    "3d6edee0bd1ef121 d12fa17cbc2c574c"
		    "ccab5e275167b08b d686f8a09df87ec3"
		    "ffb35361b94ebfa1 3fec0e4889d18da5"));

  /* TC8: hashed string patterns */
  test_chacha_rounds (SHEX("c46ec1b18ce8a878 725a37e780dfb735"
			   "1f68ed2e194c79fb c6aebee1a667975d"),
		      SHEX("1ada31d5cf688221"),
		      SHEX("838751b42d8ddd8a 3d77f48825a2ba75"
			   "2cf4047cb308a597 8ef274973be374c9"
			   "6ad848065871417b 08f034e681fe46a9"
			   "3f7d5c61d1306614 d4aaf257a7cff08b"),
		      8);

  test_chacha_rounds (SHEX("c46ec1b18ce8a878 725a37e780dfb735"
			   "1f68ed2e194c79fb c6aebee1a667975d"),
		      SHEX("1ada31d5cf688221"),
		      SHEX("1482072784bc6d06 b4e73bdc118bc010"
			   "3c7976786ca918e0 6986aa251f7e9cc1"
			   "b2749a0a16ee83b4 242d2e99b08d7c20"
			   "092b80bc466c8728 3b61b1b39d0ffbab"),
		      12);

  test_chacha(SHEX("c46ec1b18ce8a878 725a37e780dfb735"
		   "1f68ed2e194c79fb c6aebee1a667975d"),
	      SHEX("1ada31d5cf688221"),
	      SHEX("f63a89b75c2271f9 368816542ba52f06"
		   "ed49241792302b00 b5e8f80ae9a473af"
		   "c25b218f519af0fd d406362e8d69de7f"
		   "54c604a6e00f353f 110f771bdca8ab92"

		   "e5fbc34e60a1d9a9 db17345b0a402736"
		   "853bf910b060bdf1 f897b6290f01d138"
		   "ae2c4c90225ba9ea 14d518f55929dea0"
		   "98ca7a6ccfe61227 053c84e49a4a3332"));

  /* From draft-irtf-cfrg-chacha20-poly1305-08, with 96-bit nonce. Manually
     sets the 32-bit counter value to 1. */
  test_chacha_with_counter(SHEX("0001020304050607 08090a0b0c0d0e0f"
				"1011121314151617 18191a1b1c1d1e1f"),
			   SHEX("000000090000004a 00000000"),
			   SHEX("10f1e7e4d13b5915 500fdd1fa32071c4"
				"c7d1f4c733c06803 0422aa9ac3d46c4e"
				"d2826446079faa09 14c2d705d98b02a2"
				"b5129cd1de164eb9 cbd083e8a2503c4e"),
			   SHEX("01000000"));

  /* This is identical to the above 96-bit nonce test, but it manually
     sets the 64-bit counter value */
  test_chacha_with_counter(SHEX("0001020304050607 08090a0b0c0d0e0f"
				"1011121314151617 18191a1b1c1d1e1f"),
			   SHEX("0000004a00000000"),
			   SHEX("10f1e7e4d13b5915 500fdd1fa32071c4"
				"c7d1f4c733c06803 0422aa9ac3d46c4e"
				"d2826446079faa09 14c2d705d98b02a2"
				"b5129cd1de164eb9 cbd083e8a2503c4e"),
			   SHEX("0100000000000009"));

  /* Tests with long message, 16 blocks. */
  test_chacha (SHEX("8c34799cd41aaece 303d33eeaac74e6b"
		    "09742e5a6858def1 c1413425719ba204"),
	       SHEX("f9a864f273cc10fe baf531d7"),
	       SHEX("3bed2adca04c96c8 d74e08aff4d5d9e0"
		    "836209a15db7ea25 87c61d61a3472c4f"
		    "b09ec92d020a25fd 183eb31870bf01c2"
		    "9a87c2280ca12718 2af7a0b626ba4906"

		    "51268430b180c7f9 5bb680bacf4a84e7"
		    "dadd67bb6e2aa32d 22fb99ae4dfac283"
		    "acb0f596f75518f9 3227448d63048f1a"
		    "20108699cc0504e5 524e11e71f05d3f9"

		    "b37725c68260448d dd61b690f614571b"
		    "a57a848f6f2b0e10 5c044986d9d2f7ca"
		    "10177a27c5ccbb94 ffefcb87d836b02a"
		    "e6c312ebd5862520 c67d05259326a1d5"

		    "645a6f4cdda94de2 8d5d8069d95263e7"
		    "037c4fbe1322fcb5 cf5ab14237039b76"
		    "f76134345777d647 799f248b7f8c9a94"
		    "713e5678bfdef4f2 7285a34be8800146"

		    "1f9cffbf24228377 d583f34ddb9c6010"
		    "8c035f7e5285953c 74b56e1508531705"
		    "f49398b409d94276 435814ee76232b57"
		    "0b461913fa88381f 238a241afa7042f0"

		    "35e796cb4a058dfc 4098b614464ad230"
		    "9c6e7719d940e26b 8d77472d357230f5"
		    "cb35cfa86bf46324 eafb305ae2d50509"
		    "0e635ebe9e4a090a 41c8e9e931d7bf5f"

		    "22a59d75130c62e3 a4829fc38d07d458"
		    "072956a6282b9a90 f9a491e064dcc689"
		    "28e47fd1c7131e97 6f755027fbf190cf"
		    "977de188faa80a7c 83921745bb14f534"

		    "3cbc9d86d01379f3 d59fa5b454ed8855"
		    "46f34b9e419dfe08 9879cae4b297a3d8"
		    "28367da777756aaa 9de2005e0ad1af6f"
		    "af6daf9c14aedc39 fcbf95863a128816"

		    "93f22a580c96ca98 da6c13166cd17b22"
		    "2c468df637bf2961 e6b7692c6ed3463b"
		    "84a1837108de1ea4 20c0a890c92768b8"
		    "ac6732645a1a609c 28345d5e69f9475c"

		    "21401fe2b7504230 ce4866d09484ffdc"
		    "0bdd08f193993b26 083808e88f109d3b"
		    "753d61a4391b1461 fe3a9bc07e2fee7b"
		    "b37c9997c8f3d081 d6d145fb060da3f2"

		    "28a31a2be1a35c6c 4c6181bac0f1119e"
		    "e87bc230512e0fb4 38040e89e8af7452"
		    "71530ae0e34bca8e 0cfbe5bbad577bea"
		    "bc1c9c673d197185 7033822cd0ea21ed"

		    "ccfdf4788f62c26b bd2718e576db61f5"
		    "4ebef978884c7fe7 b4934918cb89e3a2"
		    "df3b5c664ab9ff09 0c016ba84ebbbd73"
		    "8cf905fc8ca95dee 757a901ea27a33d2"

		    "813f0a17692c5646 6f9690cddb3e15f7"
		    "1020f9321bd45ccb 49c41deb49103a7d"
		    "b8535b705e4d3389 f4495ee49f96dc5d"
		    "c7eab39129652078 8b3876576f5dadb0"

		    "a674bdad75a9f581 d48fbc2e6de734f0"
		    "73f78ed77b041a09 760ef6eb754cc60e"
		    "8fabd6e1b3ce02bb b302f8a73be42ddf"
		    "1dae4d8b251320d2 184adaf92eb76b12"

		    "37d169621de761fb cb6942a7b9972cca"
		    "9d35d58b2779523b ad584c27bb991acd"
		    "a0be4174dcd85ffe fe16abf2235829c5"
		    "0ac49897e2b2a7ef b1c5c07e80d7fce1"

		    "c4f5596231ad3436 0c75cd60b5088281"
		    "83c74ffc805177f7 c687454582a3cbc3"
		    "79180c9a90680eda 68499e0ee435c7c6"
		    "e028d305299b43dd eb68c387ae694a53"));

  test_chacha (SHEX("8c34799cd41aaece 303d33eeaac74e6b"
		    "09742e5a6858def1 c1413425719ba204"),
	       SHEX("f9a864f273cc10fe"),
	       SHEX("bd11678b8300a275 60dadbde311b1660"
		    "edf6ec14eff4b553 418c7a99c8a0512b"
		    "f79cfbf853f0df4f daa806dd6ac12aea"
		    "bc92f8d4964f4d86 e12934589e46b6f3"

		    "cb00ac786355852a f066f1ca2485f7bb"
		    "232096e5a6042498 149600c6d438598a"
		    "8b89bf6b3adcbbd8 010e91148c01c06d"
		    "4f5b651333f32292 0e149f6fe2dcc8a4"

		    "98744655d32732f8 69a88b5cf80761a8"
		    "218888d5ba5788c8 7eef9340e2f03b0c"
		    "b1caf7cfe0d9cde6 434d615a7f1c603e"
		    "1302e311cadb7c69 95ca53981ae58aac"

		    "40718e7dc61eabe3 35c253988217fc10"
		    "1a1633b9bce6fbc9 b9fd9c9a2ab319a5"
		    "9ba134ef7505e64c c35cccbb320bec09"
		    "4dc950849b49d86d 572f795c1a24dbe3"

		    "2d51e61a7291375a 85b150f0530f53c1"
		    "3c987c0beedb2107 49c847c774523858"
		    "dbeb997609cf89ad cf7433e668a460bc"
		    "52cfa00951daac95 e5edc8baf32a867e"

		    "81c3d7de7f34ea28 f74339985f2643cc"
		    "9b5d30d76872d20e fb18d914a58f0083"
		    "d5d322f5cdd5a3dd 63148988bb79e97a"
		    "1e8a9595e1f4cccd 8910a043f1b07cd1"

		    "bcb8e7c4c8018de9 8ff65b2787304751"
		    "333b94ae56dbbb36 4e9ed750dc77d9ce"
		    "c9c5a440663b78a6 0dba2ac3b5e7fcf1"
		    "c1f14cd1a9794c49 ff082df137a4a35d"

		    "bee8409979b49275 3a4a32825b6c9903"
		    "28440e2c7d2c2cae 4bdd5769dc0cc31f"
		    "4bd5b8d090ec6cbe d743b44bf62531c5"
		    "f12e1bbb68cca686 36953a259c9d4b9b"

		    "c1ec206c8a506f49 9a13a2e60c026f53"
		    "61bd34428f9d6fa7 207c63589d1efd5f"
		    "161cc957275a7f00 992c7e1dfda6f913"
		    "9d35ae670cff55da 1dba6a2f13424b3f"

		    "61c68580885eba6a f2aecb29d138b209"
		    "1c7227902aceddfb d4761f7cbae75d89"
		    "aaca5808a4704410 448a6eb13cf4b4f6"
		    "cd7c37341ae80b23 6affef543aef78aa"

		    "15a4581380cb19b8 e684ddd3efaea4e6"
		    "bb88c07aa9325398 cb67e241a59732d0"
		    "dfe999532b53d255 fb34a937aa55ae4d"
		    "02b7850831b7b669 1e4ee269c5d38a9d"

		    "80133265072ab3f3 af627298a265d7c0"
		    "1fe95f895b08d4c8 7dd4f6f7a6ce1393"
		    "de4225fedd1bf3c1 fe76a171f99d5e3f"
		    "975e31ca21d58fa6 daf580dcb46379a6"

		    "8a6a65e72b4df392 d3f94697f352286e"
		    "0f00ce97f2656011 4ccf17bdcedc9589"
		    "a9c8041e9f3daf9c f5c222d6ddbd2cfc"
		    "b26065a9f85592d5 e6f85a46e0e9fd79"

		    "f25197451c8d18d3 ed15cac7ba27870f"
		    "8f0cbe7c17409a4e 66e95adde633d2d6"
		    "270e0d17ca774efa 1ce9908e03baf208"
		    "cfee33add11dcd9e 032db6fbb7b209a6"

		    "30ac76c88e695413 a3c75d885a2fe9c4"
		    "50236bf7a59110d5 62c77bc046afeb0d"
		    "a8210a75a79e6732 9e49a225bee17b84"
		    "bf24bdd32f77fdf4 05fd06955b0802d9"

		    "7a4f115b8b052184 1a80620b2d66e572"
		    "41d137cbcb2131c5 c8cbabe8a1b0ca25"
		    "d760f988e68843ef ddc7449daf2b18ce"
		    "5825cc4f79aaf7ac 659c77a1ad294b51"));

  /* From https://github.com/weidai11/cryptopp/blob/master/TestVectors/chacha.txt */
  test_chacha_with_counter (SHEX("0000000000000000 0000000000000000"
				 "0000000000000000 0000000000000000"),
			    SHEX("0000000000000000"),
			    SHEX("032CC123482C3171 1F94C941AF5AB1F4"
				 "155784332ED5348F E79AEC5EAD4C06C3"
				 "F13C280D8CC49925 E4A6A5922EC80E13"
				 "A4CDFA840C70A142 7A3CB699166991A5"
				 "ACE4CD09E294D191 2D4AD205D06F95D9"
				 "C2F2BFCF453E8753 F128765B62215F4D"
				 "92C74F2F626C6A64 0C0B1284D839EC81"
				 "F1696281DAFC3E68 4593937023B58B1D"
				 "3DB41D3AA0D32928 5DE6F225E6E24BD5"
				 "9C9A17006943D5C9 B680E3873BDC683A"
				 "5819469899989690 C281CD17C96159AF"
				 "0682B5B903468A61 F50228CF09622B5A"
				 "46F0F6EFEE15C8F1 B198CB49D92B9908"
				 "67905159440CC723 916DC00128269810"
				 "39CE1766AA2542B0 5DB3BD809AB14248"
				 "9D5DBFE1273E7399 637B4B3213768AAA"
				 "89B1889375E99FE2 442C4F68ADF54158"
				 "F4B8135713D00999 B92B38E3AAFE5FF4"
				 "959B1834BE3DC54F C36AA9D32EB121E0"
				 "F688B90E7C7E2649 F4AAEF407BDD2B94"
				 "09EFEC03114CB5D4 FFD1788E0FE1897B"
				 "D176C1311E368368 C657A5EE55C9CA03"
				 "CC71744F030822D5 3A0486A97B9D9824"
				 "0274FADEAF262BD8 1B58BCE3DFA98414"
				 "C24B5BC517FD9199 3A6B2E6232B05021"
				 "25C6F48A6921E2DD A8EB6B3C4ECF2AAE"
				 "889602AD90B5D253 7FF45DF525C67B98"
				 "3B51DBD23E1280AA 656EAE85B63CC42D"
				 "E8C70E7C19C1D66E 3F902BEA9D1ACFD3"
				 "326B5985AD7C8CAB D431ACBC62976CE5"
				 "23C938EA447D4AF0 F560DC52B0AB1D7D"
				 "66A42AB8272E2C40 BD66470FE6F68846"
				 "12A11D899A0B7EB5 4907BBEDD6483EFC"
				 "ED1F15621D4673FF 928C5AAB5F465257"
				 "123679EF17C39648 C537E150108E0F66"
				 "08732E9F5B240689 EEB5402FA04CCB89"
				 "B7CA9649A361C526 B41ED110402D9497"
				 "715B03441118BC49 53FCBEF395267570"
				 "BD43EC0EEF7B6167 F14FED205EB81290"
				 "7D0C134DC49FA5B1 8F5A3A3A9BD0A71B"
				 "2FFE445EE4FABEB4 054CC922BA360E45"
				 "89B681F01E2A43B5 A0C0F0C39A5ADB94"
				 "B3BC2D20FF7F287F DF17F53B7CB5E3A1"
				 "ABD46FC0819A3559 C03C6B4106603066"
				 "359A4A09B468B6DF EF8A363C7B31D9E8"
				 "8ABB85914F4A27C3 0E9915C66AAC3576"
				 "9E481C87AEE4C313 8CF40F288ED3C172"
				 "FFC17D3D78F8D32C 3C756C13CFBFB95F"
				 "3ECCE6D8B54344D7 8998F58148C4B43B"
				 "1A6201ABFF3D4FB4 B76E3BBA104CFAA5"
				 "5D8DA4319A9E0606 644B07DC204E9635"
				 "502186C1EF9E4332 2EFD69F86D4DA1F6"
				 "A98BF0B800BA04BD 9FBA5C5BE8EC49D4"
				 "8D9EECBADEE669EF 69C9522C730110BB"
				 "8339AF0E45185262 C9183307C5EEA59D"
				 "E5095CAC26E8428D 4CA9E44DCF8FC7B4"
				 "1F9624A2DBA36F44 415BAC489BF46CB6"
				 "BB1BD2B70D719772 FDABB3B166EA615A"
				 "BDF208C39BA8A708 D933CBC8A3236D4A"
				 "15629FCAA35E00C2 B361527326E7AB51"
				 "409A7DE42C909334 6E41D3A3C4529D95"
				 "57BBC01EEFF927F1 052B5E02F74542B0"
				 "4E78F1E933C67DBC 2C9187527C86DA77"
				 "F045D4B07CF646BA 9547646905F1F117"),
			    SHEX("feffffff00000000")); /* 32-bit overflow */
}
