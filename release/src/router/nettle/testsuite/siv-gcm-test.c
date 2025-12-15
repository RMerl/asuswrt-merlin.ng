/* siv-gcm-test.c

   Self-test and vectors for AES-GCM-SIV mode ciphers

   Copyright (C) 2022 Red Hat, Inc.

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

/* The test vectors have been collected from the following standards:
 * RFC8452
 */

#include "testutils.h"
#include "ghash-internal.h"
#include "block-internal.h"
#include "aes.h"
#include "siv-gcm.h"


static const struct nettle_aead_message
siv_gcm_aes128 = {
  "siv_gcm_aes128",
  sizeof(struct aes128_ctx),
  AES128_KEY_SIZE,
  SIV_GCM_DIGEST_SIZE,
  1, /* Supports in-place operation. */
  (nettle_set_key_func*) aes128_set_encrypt_key,
  (nettle_set_key_func*) aes128_set_encrypt_key,
  (nettle_encrypt_message_func*) siv_gcm_aes128_encrypt_message,
  (nettle_decrypt_message_func*) siv_gcm_aes128_decrypt_message,
};

static const struct nettle_aead_message
siv_gcm_aes256 = {
  "siv_gcm_aes256",
  sizeof(struct aes256_ctx),
  AES256_KEY_SIZE,
  SIV_GCM_DIGEST_SIZE,
  1, /* Supports in-place operation. */
  (nettle_set_key_func*) aes256_set_encrypt_key,
  (nettle_set_key_func*) aes256_set_encrypt_key,
  (nettle_encrypt_message_func*) siv_gcm_aes256_encrypt_message,
  (nettle_decrypt_message_func*) siv_gcm_aes256_decrypt_message,
};

static void
test_polyval_internal (const struct tstring *key,
		       const struct tstring *message,
		       const struct tstring *digest)
{
  ASSERT (key->length == GCM_BLOCK_SIZE);
  ASSERT (message->length % GCM_BLOCK_SIZE == 0);
  ASSERT (digest->length == GCM_BLOCK_SIZE);
  struct gcm_key gcm_key;
  union nettle_block16 state;

  memcpy (state.b, key->data, GCM_BLOCK_SIZE);
  _siv_ghash_set_key (&gcm_key, &state);

  block16_zero (&state);
  _siv_ghash_update (&gcm_key, &state, message->length / GCM_BLOCK_SIZE, message->data);
  block16_bswap (&state, &state);

  if (!MEMEQ(GCM_BLOCK_SIZE, state.b, digest->data))
    {
      fprintf (stderr, "POLYVAL failed\n");
      fprintf (stderr, "Key: ");
      tstring_print_hex (key);
      fprintf (stderr, "\nMessage: ");
      tstring_print_hex (message);
      fprintf (stderr, "\nOutput: ");
      print_hex (GCM_BLOCK_SIZE, state.b);
      fprintf (stderr, "\nExpected:");
      tstring_print_hex (digest);
      fprintf (stderr, "\n");
      FAIL();
    }
}

void
test_main(void)
{
  /* RFC8452, Appendix A.  */
  test_polyval_internal (SHEX("25629347589242761d31f826ba4b757b"),
			 SHEX("4f4f95668c83dfb6401762bb2d01a262"
			      "d1a24ddd2721d006bbe45f20d3c9f362"),
			 SHEX("f7a3b47b846119fae5b7866cf5e5b77e"));

  /* RFC8452, Appendix C.1.  */
  test_aead_message(&siv_gcm_aes128,
		       SHEX("01000000000000000000000000000000"),
		       SHEX("030000000000000000000000"),
		       SHEX(""),
		       SHEX(""),
		       SHEX("dc20e2d83f25705bb49e439eca56de25"));

  test_aead_message(&siv_gcm_aes128,
		       SHEX("01000000000000000000000000000000"),
		       SHEX("030000000000000000000000"),
		       SHEX(""),
		       SHEX("0100000000000000"),
		       SHEX("b5d839330ac7b786578782fff6013b81"
			    "5b287c22493a364c"));

  test_aead_message(&siv_gcm_aes128,
		       SHEX("01000000000000000000000000000000"),
		       SHEX("030000000000000000000000"),
		       SHEX(""),
		       SHEX("010000000000000000000000"),
		       SHEX("7323ea61d05932260047d942a4978db3"
			    "57391a0bc4fdec8b0d106639"));

  test_aead_message(&siv_gcm_aes128,
		       SHEX("01000000000000000000000000000000"),
		       SHEX("030000000000000000000000"),
		       SHEX(""),
		       SHEX("01000000000000000000000000000000"),
		       SHEX("743f7c8077ab25f8624e2e948579cf77"
			    "303aaf90f6fe21199c6068577437a0c4"));

  test_aead_message(&siv_gcm_aes128,
		       SHEX("01000000000000000000000000000000"),
		       SHEX("030000000000000000000000"),
		       SHEX(""),
		       SHEX("01000000000000000000000000000000"
			    "02000000000000000000000000000000"),
		       SHEX("84e07e62ba83a6585417245d7ec413a9"
			    "fe427d6315c09b57ce45f2e3936a9445"
			    "1a8e45dcd4578c667cd86847bf6155ff"));

  test_aead_message(&siv_gcm_aes128,
		       SHEX("01000000000000000000000000000000"),
		       SHEX("030000000000000000000000"),
		       SHEX(""),
		       SHEX("01000000000000000000000000000000"
		            "02000000000000000000000000000000"
		            "03000000000000000000000000000000"),
		       SHEX("3fd24ce1f5a67b75bf2351f181a475c7"
		            "b800a5b4d3dcf70106b1eea82fa1d64d"
		            "f42bf7226122fa92e17a40eeaac1201b"
		            "5e6e311dbf395d35b0fe39c2714388f8"));

  test_aead_message(&siv_gcm_aes128,
		       SHEX("01000000000000000000000000000000"),
		       SHEX("030000000000000000000000"),
		       SHEX(""),
		       SHEX("01000000000000000000000000000000"
		            "02000000000000000000000000000000"
		            "03000000000000000000000000000000"
		            "04000000000000000000000000000000"),
		       SHEX("2433668f1058190f6d43e360f4f35cd8"
		            "e475127cfca7028ea8ab5c20f7ab2af0"
		            "2516a2bdcbc08d521be37ff28c152bba"
		            "36697f25b4cd169c6590d1dd39566d3f"
		            "8a263dd317aa88d56bdf3936dba75bb8"));

  test_aead_message(&siv_gcm_aes128,
		       SHEX("01000000000000000000000000000000"),
		       SHEX("030000000000000000000000"),
		       SHEX("01"),
		       SHEX("0200000000000000"),
		       SHEX("1e6daba35669f4273b0a1a2560969cdf"
		            "790d99759abd1508"));

  test_aead_message(&siv_gcm_aes128,
		       SHEX("01000000000000000000000000000000"),
		       SHEX("030000000000000000000000"),
		       SHEX("01"),
		       SHEX("020000000000000000000000"),
		       SHEX("296c7889fd99f41917f4462008299c51"
		            "02745aaa3a0c469fad9e075a"));

  test_aead_message(&siv_gcm_aes128,
		       SHEX("01000000000000000000000000000000"),
		       SHEX("030000000000000000000000"),
		       SHEX("01"),
		       SHEX("02000000000000000000000000000000"),
		       SHEX("e2b0c5da79a901c1745f700525cb335b"
		            "8f8936ec039e4e4bb97ebd8c4457441f"));

  test_aead_message(&siv_gcm_aes128,
		       SHEX("01000000000000000000000000000000"),
		       SHEX("030000000000000000000000"),
		       SHEX("01"),
		       SHEX("02000000000000000000000000000000"
		            "03000000000000000000000000000000"),
		       SHEX("620048ef3c1e73e57e02bb8562c416a3"
		            "19e73e4caac8e96a1ecb2933145a1d71"
		            "e6af6a7f87287da059a71684ed3498e1"));

  test_aead_message(&siv_gcm_aes128,
		       SHEX("01000000000000000000000000000000"),
		       SHEX("030000000000000000000000"),
		       SHEX("01"),
		       SHEX("02000000000000000000000000000000"
		            "03000000000000000000000000000000"
		            "04000000000000000000000000000000"),
		       SHEX("50c8303ea93925d64090d07bd109dfd9"
		            "515a5a33431019c17d93465999a8b005"
		            "3201d723120a8562b838cdff25bf9d1e"
		            "6a8cc3865f76897c2e4b245cf31c51f2"));

  test_aead_message(&siv_gcm_aes128,
		       SHEX("01000000000000000000000000000000"),
		       SHEX("030000000000000000000000"),
		       SHEX("01"),
		       SHEX("02000000000000000000000000000000"
		            "03000000000000000000000000000000"
		            "04000000000000000000000000000000"
		            "05000000000000000000000000000000"),
		       SHEX("2f5c64059db55ee0fb847ed513003746"
		            "aca4e61c711b5de2e7a77ffd02da42fe"
		            "ec601910d3467bb8b36ebbaebce5fba3"
		            "0d36c95f48a3e7980f0e7ac299332a80"
		            "cdc46ae475563de037001ef84ae21744"));

  test_aead_message(&siv_gcm_aes128,
		       SHEX("01000000000000000000000000000000"),
		       SHEX("030000000000000000000000"),
		       SHEX("010000000000000000000000"),
		       SHEX("02000000"),
		       SHEX("a8fe3e8707eb1f84fb28f8cb73de8e99"
		            "e2f48a14"));

  test_aead_message(&siv_gcm_aes128,
		       SHEX("01000000000000000000000000000000"),
		       SHEX("030000000000000000000000"),
		       SHEX("01000000000000000000000000000000"
		            "0200"),
		       SHEX("03000000000000000000000000000000"
		            "04000000"),
		       SHEX("6bb0fecf5ded9b77f902c7d5da236a43"
		            "91dd029724afc9805e976f451e6d87f6"
		            "fe106514"));

  test_aead_message(&siv_gcm_aes128,
		       SHEX("01000000000000000000000000000000"),
		       SHEX("030000000000000000000000"),
		       SHEX("01000000000000000000000000000000"
		            "02000000"),
		       SHEX("03000000000000000000000000000000"
		            "0400"),
		       SHEX("44d0aaf6fb2f1f34add5e8064e83e12a"
		            "2adabff9b2ef00fb47920cc72a0c0f13"
		            "b9fd"));

  test_aead_message(&siv_gcm_aes128,
		       SHEX("e66021d5eb8e4f4066d4adb9c33560e4"),
		       SHEX("f46e44bb3da0015c94f70887"),
		       SHEX(""),
		       SHEX(""),
		       SHEX("a4194b79071b01a87d65f706e3949578"));

  test_aead_message(&siv_gcm_aes128,
		       SHEX("36864200e0eaf5284d884a0e77d31646"),
		       SHEX("bae8e37fc83441b16034566b"),
		       SHEX("46bb91c3c5"),
		       SHEX("7a806c"),
		       SHEX("af60eb711bd85bc1e4d3e0a462e074ee"
		            "a428a8"));

  test_aead_message(&siv_gcm_aes128,
		       SHEX("aedb64a6c590bc84d1a5e269e4b47801"),
		       SHEX("afc0577e34699b9e671fdd4f"),
		       SHEX("fc880c94a95198874296"),
		       SHEX("bdc66f146545"),
		       SHEX("bb93a3e34d3cd6a9c45545cfc11f03ad"
		            "743dba20f966"));

  test_aead_message(&siv_gcm_aes128,
		       SHEX("d5cc1fd161320b6920ce07787f86743b"),
		       SHEX("275d1ab32f6d1f0434d8848c"),
		       SHEX("046787f3ea22c127aaf195d1894728"),
		       SHEX("1177441f195495860f"),
		       SHEX("4f37281f7ad12949d01d02fd0cd174c8"
		            "4fc5dae2f60f52fd2b"));

  test_aead_message(&siv_gcm_aes128,
		       SHEX("b3fed1473c528b8426a582995929a149"),
		       SHEX("9e9ad8780c8d63d0ab4149c0"),
		       SHEX("c9882e5386fd9f92ec489c8fde2be2cf"
		            "97e74e93"),
		       SHEX("9f572c614b4745914474e7c7"),
		       SHEX("f54673c5ddf710c745641c8bc1dc2f87"
		            "1fb7561da1286e655e24b7b0"));

  test_aead_message(&siv_gcm_aes128,
		       SHEX("2d4ed87da44102952ef94b02b805249b"),
		       SHEX("ac80e6f61455bfac8308a2d4"),
		       SHEX("2950a70d5a1db2316fd568378da107b5"
		            "2b0da55210cc1c1b0a"),
		       SHEX("0d8c8451178082355c9e940fea2f58"),
		       SHEX("c9ff545e07b88a015f05b274540aa183"
		            "b3449b9f39552de99dc214a1190b0b"));

  test_aead_message(&siv_gcm_aes128,
		       SHEX("bde3b2f204d1e9f8b06bc47f9745b3d1"),
		       SHEX("ae06556fb6aa7890bebc18fe"),
		       SHEX("1860f762ebfbd08284e421702de0de18"
		            "baa9c9596291b08466f37de21c7f"),
		       SHEX("6b3db4da3d57aa94842b9803a96e07fb"
		            "6de7"),
		       SHEX("6298b296e24e8cc35dce0bed484b7f30"
		            "d5803e377094f04709f64d7b985310a4"
		            "db84"));

  test_aead_message(&siv_gcm_aes128,
		       SHEX("f901cfe8a69615a93fdf7a98cad48179"),
		       SHEX("6245709fb18853f68d833640"),
		       SHEX("7576f7028ec6eb5ea7e298342a94d4b2"
		            "02b370ef9768ec6561c4fe6b7e7296fa"
		            "859c21"),
		       SHEX("e42a3c02c25b64869e146d7b233987bd"
		            "dfc240871d"),
		       SHEX("391cc328d484a4f46406181bcd62efd9"
		            "b3ee197d052d15506c84a9edd65e13e9"
		            "d24a2a6e70"));

  /* RFC8452, Appendix C.2.  */
  test_aead_message(&siv_gcm_aes256,
		       SHEX("01000000000000000000000000000000"
			    "00000000000000000000000000000000"),
		       SHEX("030000000000000000000000"),
		       SHEX(""),
		       SHEX(""),
		       SHEX("07f5f4169bbf55a8400cd47ea6fd400f"));

  test_aead_message(&siv_gcm_aes256,
		       SHEX("01000000000000000000000000000000"
			    "00000000000000000000000000000000"),
		       SHEX("030000000000000000000000"),
		       SHEX(""),
		       SHEX("0100000000000000"),
		       SHEX("c2ef328e5c71c83b843122130f7364b7"
			    "61e0b97427e3df28"));

  test_aead_message(&siv_gcm_aes256,
		       SHEX("01000000000000000000000000000000"
			    "00000000000000000000000000000000"),
		       SHEX("030000000000000000000000"),
		       SHEX(""),
		       SHEX("010000000000000000000000"),
		       SHEX("9aab2aeb3faa0a34aea8e2b18ca50da9"
			    "ae6559e48fd10f6e5c9ca17e"));

  test_aead_message(&siv_gcm_aes256,
		       SHEX("01000000000000000000000000000000"
		            "00000000000000000000000000000000"),
		       SHEX("030000000000000000000000"),
		       SHEX(""),
		       SHEX("01000000000000000000000000000000"),
		       SHEX("85a01b63025ba19b7fd3ddfc033b3e76"
		            "c9eac6fa700942702e90862383c6c366"));

  test_aead_message(&siv_gcm_aes256,
		       SHEX("01000000000000000000000000000000"
		            "00000000000000000000000000000000"),
		       SHEX("030000000000000000000000"),
		       SHEX(""),
		       SHEX("01000000000000000000000000000000"
		            "02000000000000000000000000000000"),
		       SHEX("4a6a9db4c8c6549201b9edb53006cba8"
		            "21ec9cf850948a7c86c68ac7539d027f"
		            "e819e63abcd020b006a976397632eb5d"));

  test_aead_message(&siv_gcm_aes256,
		       SHEX("01000000000000000000000000000000"
		            "00000000000000000000000000000000"),
		       SHEX("030000000000000000000000"),
		       SHEX(""),
		       SHEX("01000000000000000000000000000000"
		            "02000000000000000000000000000000"
		            "03000000000000000000000000000000"),
		       SHEX("c00d121893a9fa603f48ccc1ca3c57ce"
		            "7499245ea0046db16c53c7c66fe717e3"
		            "9cf6c748837b61f6ee3adcee17534ed5"
		            "790bc96880a99ba804bd12c0e6a22cc4"));

  test_aead_message(&siv_gcm_aes256,
		       SHEX("01000000000000000000000000000000"
		            "00000000000000000000000000000000"),
		       SHEX("030000000000000000000000"),
		       SHEX(""),
		       SHEX("01000000000000000000000000000000"
		            "02000000000000000000000000000000"
		            "03000000000000000000000000000000"
		            "04000000000000000000000000000000"),
		       SHEX("c2d5160a1f8683834910acdafc41fbb1"
		            "632d4a353e8b905ec9a5499ac34f96c7"
		            "e1049eb080883891a4db8caaa1f99dd0"
		            "04d80487540735234e3744512c6f90ce"
		            "112864c269fc0d9d88c61fa47e39aa08"));

  test_aead_message(&siv_gcm_aes256,
		       SHEX("01000000000000000000000000000000"
		            "00000000000000000000000000000000"),
		       SHEX("030000000000000000000000"),
		       SHEX("01"),
		       SHEX("0200000000000000"),
		       SHEX("1de22967237a813291213f267e3b452f"
		            "02d01ae33e4ec854"));

  test_aead_message(&siv_gcm_aes256,
		       SHEX("01000000000000000000000000000000"
		            "00000000000000000000000000000000"),
		       SHEX("030000000000000000000000"),
		       SHEX("01"),
		       SHEX("020000000000000000000000"),
		       SHEX("163d6f9cc1b346cd453a2e4cc1a4a19a"
		            "e800941ccdc57cc8413c277f"));

  test_aead_message(&siv_gcm_aes256,
		       SHEX("01000000000000000000000000000000"
		            "00000000000000000000000000000000"),
		       SHEX("030000000000000000000000"),
		       SHEX("01"),
		       SHEX("02000000000000000000000000000000"),
		       SHEX("c91545823cc24f17dbb0e9e807d5ec17"
		            "b292d28ff61189e8e49f3875ef91aff7"));

  test_aead_message(&siv_gcm_aes256,
		       SHEX("01000000000000000000000000000000"
		            "00000000000000000000000000000000"),
		       SHEX("030000000000000000000000"),
		       SHEX("01"),
		       SHEX("02000000000000000000000000000000"
		            "03000000000000000000000000000000"),
		       SHEX("07dad364bfc2b9da89116d7bef6daaaf"
		            "6f255510aa654f920ac81b94e8bad365"
		            "aea1bad12702e1965604374aab96dbbc"));

  test_aead_message(&siv_gcm_aes256,
		       SHEX("01000000000000000000000000000000"
		            "00000000000000000000000000000000"),
		       SHEX("030000000000000000000000"),
		       SHEX("01"),
		       SHEX("02000000000000000000000000000000"
		            "03000000000000000000000000000000"
		            "04000000000000000000000000000000"),
		       SHEX("c67a1f0f567a5198aa1fcc8e3f213143"
		            "36f7f51ca8b1af61feac35a86416fa47"
		            "fbca3b5f749cdf564527f2314f42fe25"
		            "03332742b228c647173616cfd44c54eb"));

  test_aead_message(&siv_gcm_aes256,
		       SHEX("01000000000000000000000000000000"
		            "00000000000000000000000000000000"),
		       SHEX("030000000000000000000000"),
		       SHEX("01"),
		       SHEX("02000000000000000000000000000000"
		            "03000000000000000000000000000000"
		            "04000000000000000000000000000000"
		            "05000000000000000000000000000000"),
		       SHEX("67fd45e126bfb9a79930c43aad2d3696"
		            "7d3f0e4d217c1e551f59727870beefc9"
		            "8cb933a8fce9de887b1e40799988db1f"
		            "c3f91880ed405b2dd298318858467c89"
		            "5bde0285037c5de81e5b570a049b62a0"));

  test_aead_message(&siv_gcm_aes256,
		       SHEX("01000000000000000000000000000000"
		            "00000000000000000000000000000000"),
		       SHEX("030000000000000000000000"),
		       SHEX("010000000000000000000000"),
		       SHEX("02000000"),
		       SHEX("22b3f4cd1835e517741dfddccfa07fa4"
		            "661b74cf"));

  test_aead_message(&siv_gcm_aes256,
		       SHEX("01000000000000000000000000000000"
		            "00000000000000000000000000000000"),
		       SHEX("030000000000000000000000"),
		       SHEX("01000000000000000000000000000000"
		            "0200"),
		       SHEX("03000000000000000000000000000000"
		            "04000000"),
		       SHEX("43dd0163cdb48f9fe3212bf61b201976"
		            "067f342bb879ad976d8242acc188ab59"
		            "cabfe307"));

  test_aead_message(&siv_gcm_aes256,
		       SHEX("01000000000000000000000000000000"
		            "00000000000000000000000000000000"),
		       SHEX("030000000000000000000000"),
		       SHEX("01000000000000000000000000000000"
		            "02000000"),
		       SHEX("03000000000000000000000000000000"
		            "0400"),
		       SHEX("462401724b5ce6588d5a54aae5375513"
		            "a075cfcdf5042112aa29685c912fc205"
		            "6543"));

  test_aead_message(&siv_gcm_aes256,
		       SHEX("e66021d5eb8e4f4066d4adb9c33560e4"
		            "f46e44bb3da0015c94f7088736864200"),
		       SHEX("e0eaf5284d884a0e77d31646"),
		       SHEX(""),
		       SHEX(""),
		       SHEX("169fbb2fbf389a995f6390af22228a62"));

  test_aead_message(&siv_gcm_aes256,
		       SHEX("bae8e37fc83441b16034566b7a806c46"
		            "bb91c3c5aedb64a6c590bc84d1a5e269"),
		       SHEX("e4b47801afc0577e34699b9e"),
		       SHEX("4fbdc66f14"),
		       SHEX("671fdd"),
		       SHEX("0eaccb93da9bb81333aee0c785b240d3"
		            "19719d"));

  test_aead_message(&siv_gcm_aes256,
		       SHEX("6545fc880c94a95198874296d5cc1fd1"
		            "61320b6920ce07787f86743b275d1ab3"),
		       SHEX("2f6d1f0434d8848c1177441f"),
		       SHEX("6787f3ea22c127aaf195"),
		       SHEX("195495860f04"),
		       SHEX("a254dad4f3f96b62b84dc40c84636a5e"
		            "c12020ec8c2c"));

  test_aead_message(&siv_gcm_aes256,
		       SHEX("d1894728b3fed1473c528b8426a58299"
		            "5929a1499e9ad8780c8d63d0ab4149c0"),
		       SHEX("9f572c614b4745914474e7c7"),
		       SHEX("489c8fde2be2cf97e74e932d4ed87d"),
		       SHEX("c9882e5386fd9f92ec"),
		       SHEX("0df9e308678244c44bc0fd3dc6628dfe"
		            "55ebb0b9fb2295c8c2"));

  test_aead_message(&siv_gcm_aes256,
		       SHEX("a44102952ef94b02b805249bac80e6f6"
		            "1455bfac8308a2d40d8c845117808235"),
		       SHEX("5c9e940fea2f582950a70d5a"),
		       SHEX("0da55210cc1c1b0abde3b2f204d1e9f8"
		            "b06bc47f"),
		       SHEX("1db2316fd568378da107b52b"),
		       SHEX("8dbeb9f7255bf5769dd56692404099c2"
		            "587f64979f21826706d497d5"));

  test_aead_message(&siv_gcm_aes256,
		       SHEX("9745b3d1ae06556fb6aa7890bebc18fe"
		            "6b3db4da3d57aa94842b9803a96e07fb"),
		       SHEX("6de71860f762ebfbd08284e4"),
		       SHEX("f37de21c7ff901cfe8a69615a93fdf7a"
		            "98cad481796245709f"),
		       SHEX("21702de0de18baa9c9596291b08466"),
		       SHEX("793576dfa5c0f88729a7ed3c2f1bffb3"
		            "080d28f6ebb5d3648ce97bd5ba67fd"));

  test_aead_message(&siv_gcm_aes256,
		       SHEX("b18853f68d833640e42a3c02c25b6486"
		            "9e146d7b233987bddfc240871d7576f7"),
		       SHEX("028ec6eb5ea7e298342a94d4"),
		       SHEX("9c2159058b1f0fe91433a5bdc20e214e"
		            "ab7fecef4454a10ef0657df21ac7"),
		       SHEX("b202b370ef9768ec6561c4fe6b7e7296"
		            "fa85"),
		       SHEX("857e16a64915a787637687db4a951963"
		            "5cdd454fc2a154fea91f8363a39fec7d"
		            "0a49"));

  test_aead_message(&siv_gcm_aes256,
		       SHEX("3c535de192eaed3822a2fbbe2ca9dfc8"
		            "8255e14a661b8aa82cc54236093bbc23"),
		       SHEX("688089e55540db1872504e1c"),
		       SHEX("734320ccc9d9bbbb19cb81b2af4ecbc3"
		            "e72834321f7aa0f70b7282b4f33df23f"
		            "167541"),
		       SHEX("ced532ce4159b035277d4dfbb7db6296"
		            "8b13cd4eec"),
		       SHEX("626660c26ea6612fb17ad91e8e767639"
		            "edd6c9faee9d6c7029675b89eaf4ba1d"
		            "ed1a286594"));

  /* RFC8452, Appendix C.3.  */
  test_aead_message(&siv_gcm_aes256,
		       SHEX("00000000000000000000000000000000"
		            "00000000000000000000000000000000"),
		       SHEX("000000000000000000000000"),
		       SHEX(""),
		       SHEX("00000000000000000000000000000000"
		            "4db923dc793ee6497c76dcc03a98e108"),
		       SHEX("f3f80f2cf0cb2dd9c5984fcda908456c"
		            "c537703b5ba70324a6793a7bf218d3ea"
		            "ffffffff000000000000000000000000"));

  test_aead_message(&siv_gcm_aes256,
		       SHEX("00000000000000000000000000000000"
		            "00000000000000000000000000000000"),
		       SHEX("000000000000000000000000"),
		       SHEX(""),
		       SHEX("eb3640277c7ffd1303c7a542d02d3e4c"
		            "0000000000000000"),
		       SHEX("18ce4f0b8cb4d0cac65fea8f79257b20"
		            "888e53e72299e56dffffffff00000000"
		            "0000000000000000"));
}
