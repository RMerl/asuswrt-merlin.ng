/* This file tests deprecated functions */
#define _NETTLE_ATTRIBUTE_DEPRECATED

#include "testutils.h"
#include "nettle-internal.h"
#include "gcm.h"
#include "ghash-internal.h"

static void
test_gcm_hash (const struct tstring *msg, const struct tstring *ref)
{
  struct gcm_aes128_ctx ctx;
  const uint8_t z16[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
  uint8_t digest[16];

  ASSERT (ref->length == sizeof(digest));
  gcm_aes128_set_key (&ctx, z16);
  gcm_aes128_set_iv (&ctx, 16, z16);
  gcm_aes128_update (&ctx, msg->length, msg->data);
  gcm_aes128_digest (&ctx, sizeof(digest), digest);
  if (!MEMEQ (ref->length, ref->data, digest))
    {
      fprintf (stderr, "gcm_hash failed, msg: %s\nOutput: ", msg->data);
      print_hex (16, digest);
      fprintf(stderr, "Expected:");
      tstring_print_hex(ref);
      fprintf(stderr, "\n");
      FAIL();
    }
}

static void
test_ghash_internal (const struct tstring *key,
		     const struct tstring *iv,
		     const struct tstring *message,
		     const struct tstring *digest)
{
  ASSERT (key->length == GCM_BLOCK_SIZE);
  ASSERT (iv->length == GCM_BLOCK_SIZE);
  ASSERT (digest->length == GCM_BLOCK_SIZE);
  ASSERT (message->length % GCM_BLOCK_SIZE == 0);
  struct gcm_key gcm_key;
  union nettle_block16 state;

  /* Mark inputs as "undefined" to valgrind, to get warnings about any
     branches or memory accesses depending on secret data. */
  memcpy (state.b, key->data, GCM_BLOCK_SIZE);
  mark_bytes_undefined (sizeof(state), &state);
  _ghash_set_key (&gcm_key, &state);

  memcpy (state.b, iv->data, GCM_BLOCK_SIZE);
  mark_bytes_undefined (sizeof(state), &state);
  mark_bytes_undefined (message->length, message->data);
  _ghash_update (&gcm_key, &state, message->length / GCM_BLOCK_SIZE, message->data);
  mark_bytes_defined (sizeof(state), &state);
  mark_bytes_defined (message->length, message->data);

  if (!MEMEQ(GCM_BLOCK_SIZE, state.b, digest->data))
    {
      fprintf (stderr, "gcm_hash (internal) failed\n");
      fprintf(stderr, "Key: ");
      tstring_print_hex(key);
      fprintf(stderr, "\nIV: ");
      tstring_print_hex(iv);
      fprintf(stderr, "\nMessage: ");
      tstring_print_hex(message);
      fprintf(stderr, "\nOutput: ");
      print_hex(GCM_BLOCK_SIZE, state.b);
      fprintf(stderr, "\nExpected:");
      tstring_print_hex(digest);
      fprintf(stderr, "\n");
      FAIL();
    }
}

static nettle_set_key_func gcm_unified_aes128_set_key;
static nettle_set_key_func gcm_unified_aes128_set_iv;
static void
gcm_unified_aes128_set_key (void *ctx, const uint8_t *key)
{
  gcm_aes_set_key (ctx, AES128_KEY_SIZE, key);
}
static void
gcm_unified_aes128_set_iv (void *ctx, const uint8_t *iv)
{
  gcm_aes_set_iv (ctx, GCM_IV_SIZE, iv);
}
static const struct nettle_aead
nettle_gcm_unified_aes128 = {
  "gcm-aes128",
  sizeof (struct gcm_aes_ctx),
  GCM_BLOCK_SIZE, AES128_KEY_SIZE,
  GCM_IV_SIZE, GCM_DIGEST_SIZE,
  (nettle_set_key_func *) gcm_unified_aes128_set_key,
  (nettle_set_key_func *) gcm_unified_aes128_set_key,
  (nettle_set_key_func *) gcm_unified_aes128_set_iv,
  (nettle_hash_update_func *) gcm_aes_update,
  (nettle_crypt_func *) gcm_aes_encrypt,
  (nettle_crypt_func *) gcm_aes_decrypt,
  (nettle_hash_digest_func *) gcm_aes_digest
};

/* Hack that uses a 16-byte nonce, a 12-byte standard GCM nonce and an
   explicit initial value for the counter. */
static void
gcm_aes128_set_iv_hack (struct gcm_aes128_ctx *ctx, size_t size, const uint8_t *iv) {
  assert (size == 16);
  gcm_aes128_set_iv (ctx, 12, iv);
  memcpy (ctx->gcm.ctr.b + 12, iv + 12, 4);
}

void
test_main(void)
{
  /* 
   * GCM-AES Test Vectors from
   * http://www.cryptobarn.com/papers/gcm-spec.pdf
   */

  /* Test case 1 */
  test_aead(&nettle_gcm_aes128, NULL,
	    SHEX("00000000000000000000000000000000"),	/* key */
	    SHEX(""),					/* auth data */ 
	    SHEX(""),					/* plaintext */
	    SHEX(""),					/* ciphertext*/
	    SHEX("000000000000000000000000"),		/* IV */
	    SHEX("58e2fccefa7e3061367f1d57a4e7455a"));	/* tag */

  /* Test case 2 */
  test_aead(&nettle_gcm_aes128, NULL,
	    SHEX("00000000000000000000000000000000"),
	    SHEX(""),
	    SHEX("00000000000000000000000000000000"),
	    SHEX("0388dace60b6a392f328c2b971b2fe78"),
	    SHEX("000000000000000000000000"),
	    SHEX("ab6e47d42cec13bdf53a67b21257bddf"));

  /* Test case 3 */
  test_aead(&nettle_gcm_aes128, NULL,
	    SHEX("feffe9928665731c6d6a8f9467308308"),
	    SHEX(""),
	    SHEX("d9313225f88406e5a55909c5aff5269a"
		 "86a7a9531534f7da2e4c303d8a318a72"
		 "1c3c0c95956809532fcf0e2449a6b525"
		 "b16aedf5aa0de657ba637b391aafd255"),
	    SHEX("42831ec2217774244b7221b784d0d49c"
		 "e3aa212f2c02a4e035c17e2329aca12e"
		 "21d514b25466931c7d8f6a5aac84aa05"
		 "1ba30b396a0aac973d58e091473f5985"),
	    SHEX("cafebabefacedbaddecaf888"),
	    SHEX("4d5c2af327cd64a62cf35abd2ba6fab4"));

  /* Test case 4 */
  test_aead(&nettle_gcm_aes128, NULL,
	    SHEX("feffe9928665731c6d6a8f9467308308"),
	    SHEX("feedfacedeadbeeffeedfacedeadbeef"
		 "abaddad2"),
	    SHEX("d9313225f88406e5a55909c5aff5269a"
		 "86a7a9531534f7da2e4c303d8a318a72"
		 "1c3c0c95956809532fcf0e2449a6b525"
		 "b16aedf5aa0de657ba637b39"),
	    SHEX("42831ec2217774244b7221b784d0d49c"
		 "e3aa212f2c02a4e035c17e2329aca12e"
		 "21d514b25466931c7d8f6a5aac84aa05"
		 "1ba30b396a0aac973d58e091"),
	    SHEX("cafebabefacedbaddecaf888"),
	    SHEX("5bc94fbc3221a5db94fae95ae7121a47"));

  /* Regression test, same inputs but explicitly setting the counter
     value. */
  test_aead(&nettle_gcm_aes128,
	    (nettle_hash_update_func *) gcm_aes128_set_iv_hack,
	    SHEX("feffe9928665731c6d6a8f9467308308"),
	    SHEX("feedfacedeadbeeffeedfacedeadbeef"
		 "abaddad2"),
	    SHEX("d9313225f88406e5a55909c5aff5269a"
		 "86a7a9531534f7da2e4c303d8a318a72"
		 "1c3c0c95956809532fcf0e2449a6b525"
		 "b16aedf5aa0de657ba637b39"),
	    SHEX("42831ec2217774244b7221b784d0d49c"
		 "e3aa212f2c02a4e035c17e2329aca12e"
		 "21d514b25466931c7d8f6a5aac84aa05"
		 "1ba30b396a0aac973d58e091"),
	    SHEX("cafebabefacedbaddecaf88800000002"), /* ctr == 2, same as the spec */
	    SHEX("5bc94fbc3221a5db94fae95ae7121a47"));

  test_aead(&nettle_gcm_aes128,
	    (nettle_hash_update_func *) gcm_aes128_set_iv_hack,
	    SHEX("feffe9928665731c6d6a8f9467308308"),
	    SHEX("feedfacedeadbeeffeedfacedeadbeef"
		 "abaddad2"),
	    SHEX("d9313225f88406e5a55909c5aff5269a"
		 "86a7a9531534f7da2e4c303d8a318a72"
		 "1c3c0c95956809532fcf0e2449a6b525"
		 "b16aedf5aa0de657ba637b39"),
	    SHEX("77ffd1ba63b141ba fb2efb329c9c25ee"
		 "99e5e06e603dd5c6 8efe1cb2cefc0677"
		 "2e7b14dea92760f7 6273dc0cce1d013d"
		 "2ad8c11273fe9496 5448534b"),
	    SHEX("cafebabefacedbaddecaf888ffffffff"), /* ctr == 2^31-1 */
	    SHEX("83cf46eb0407be56 72f756a4caebcda7"));

  /* Test case 5 */
  test_aead(&nettle_gcm_aes128,
	    (nettle_hash_update_func *) gcm_aes128_set_iv,
	    SHEX("feffe9928665731c6d6a8f9467308308"),
	    SHEX("feedfacedeadbeeffeedfacedeadbeef"
		 "abaddad2"),
	    SHEX("d9313225f88406e5a55909c5aff5269a"
		 "86a7a9531534f7da2e4c303d8a318a72"
		 "1c3c0c95956809532fcf0e2449a6b525"
		 "b16aedf5aa0de657ba637b39"),
	    SHEX("61353b4c2806934a777ff51fa22a4755"
		 "699b2a714fcdc6f83766e5f97b6c7423"
		 "73806900e49f24b22b097544d4896b42"
		 "4989b5e1ebac0f07c23f4598"),
	    SHEX("cafebabefacedbad"),
	    SHEX("3612d2e79e3b0785561be14aaca2fccb"));

  /* Test case 6 */
  test_aead(&nettle_gcm_aes128,
	    (nettle_hash_update_func *) gcm_aes128_set_iv,
	    SHEX("feffe9928665731c6d6a8f9467308308"),
	    SHEX("feedfacedeadbeeffeedfacedeadbeef"
		 "abaddad2"),
	    SHEX("d9313225f88406e5a55909c5aff5269a"
		 "86a7a9531534f7da2e4c303d8a318a72"
		 "1c3c0c95956809532fcf0e2449a6b525"
		 "b16aedf5aa0de657ba637b39"),
	    SHEX("8ce24998625615b603a033aca13fb894"
		 "be9112a5c3a211a8ba262a3cca7e2ca7"
		 "01e4a9a4fba43c90ccdcb281d48c7c6f"
		 "d62875d2aca417034c34aee5"),
	    SHEX("9313225df88406e555909c5aff5269aa"
		 "6a7a9538534f7da1e4c303d2a318a728"
		 "c3c0c95156809539fcf0e2429a6b5254"
		 "16aedbf5a0de6a57a637b39b"),
	    SHEX("619cc5aefffe0bfa462af43c1699d050"));

  /* Same test, but with old gcm_aes interface */
  test_aead(&nettle_gcm_unified_aes128,
	    (nettle_hash_update_func *) gcm_aes_set_iv,
	    SHEX("feffe9928665731c6d6a8f9467308308"),
	    SHEX("feedfacedeadbeeffeedfacedeadbeef"
		 "abaddad2"),
	    SHEX("d9313225f88406e5a55909c5aff5269a"
		 "86a7a9531534f7da2e4c303d8a318a72"
		 "1c3c0c95956809532fcf0e2449a6b525"
		 "b16aedf5aa0de657ba637b39"),
	    SHEX("8ce24998625615b603a033aca13fb894"
		 "be9112a5c3a211a8ba262a3cca7e2ca7"
		 "01e4a9a4fba43c90ccdcb281d48c7c6f"
		 "d62875d2aca417034c34aee5"),
	    SHEX("9313225df88406e555909c5aff5269aa"
		 "6a7a9538534f7da1e4c303d2a318a728"
		 "c3c0c95156809539fcf0e2429a6b5254"
		 "16aedbf5a0de6a57a637b39b"),
	    SHEX("619cc5aefffe0bfa462af43c1699d050"));

  /* Test 128 bytes */
  test_aead(&nettle_gcm_aes128, NULL,
	    SHEX("feffe9928665731c6d6a8f9467308308"),
	    SHEX(""),
	    SHEX("d9313225f88406e5a55909c5aff5269a"
		 "86a7a9531534f7da2e4c303d8a318a72"
		 "1c3c0c95956809532fcf0e2449a6b525"
		 "b16aedf5aa0de657ba637b391aafd255"
		 "5ae376bc5e9f6a1b08e34db7a6ee0736"
		 "9ba662ea12f6f197e6bc3ed69d2480f3"
		 "ea5691347f2ba69113eb37910ebc18c8"
		 "0f697234582016fa956ca8f63ae6b473"),
	    SHEX("42831ec2217774244b7221b784d0d49c"
		 "e3aa212f2c02a4e035c17e2329aca12e"
		 "21d514b25466931c7d8f6a5aac84aa05"
		 "1ba30b396a0aac973d58e091473f5985"
		 "874b1178906ddbeab04ab2fe6cce8c57"
		 "8d7e961bd13fd6a8c56b66ca5e576492"
		 "1a48cd8bda04e66343e73055118b69b9"
		 "ced486813846958a11e602c03cfc232b"),
	    SHEX("cafebabefacedbaddecaf888"),
	    SHEX("796836f1246c9d735c5e1be0a715ccc3"));

  /* Test 719 bytes */
  test_aead(&nettle_gcm_aes256, NULL,
	    SHEX("6235f895fca5ebf60e921204d3a13f2e"
	         "8b32cfe744ed1359043877b0b9adb438"),
	    SHEX(""),
	    SHEX("42c1cc08486f413f2f11668b2a16f0e0"
		"5883f0c37014c05b3fec1d253c51d203"
		"cf59741fb285b407c66a63398a5bdecb"
		"af0844bd6f9115e1f57a6e18bddd6150"
		"59a997abbb0e745c00a4435404549b3b"
		"77ecfd5ca6e87b08aee6103f3265d1fc"
		"a41d2c31fb337ab33523f42041d4ad82"
		"8ba4ad961c2053be0ea6f4dc78493e72"
		"b1a9b583cb0854b7ad493aae98cea666"
		"1030908c5583d77c8be653ded26e1821"
		"0152d19f9dbb9c7357cc8909759b7870"
		"ed26974db4e40ca5fa700470c6961c7d"
		"544177a8e3b07e9682d9eca2876855f9"
		"8f9e7343476a08369367a82ddeac41a9"
		"5c4d73970f7068fa564d00c23b1fc8b9"
		"781f5107e39a134eed2b2ea3f744b2e7"
		"ab1937d9ba765ed2f25315174c6b169f"
		"026649ca7c9105f245361ef577ad1f46"
		"a813fb63b608996382a2edb3acdf4319"
		"45ea7873d9b73911a3137cf83ff7ad81"
		"482fa95c5fa0f079a4477d802026fd63"
		"0ac77e6d7547ff76662e8a6c8135af0b"
		"2e6a4960c110e1e15403a4090c377a15"
		"23275b8b4ba56497ae4a50731f661c5c"
		"03253c8d485871340eec4e551a036ae5"
		"b6192b842a20d1ea806f960e0562c778"
		"8779603846b425576e1663f8ad6ed742"
		"69e188ef6ed5b49a3c786c3be5a01d22"
		"865c743aeb2426c709fc919647874f1a"
		"d66b2c1847c0b824a85a4a9ecb03e72a"
		"09e64d9c6d8660f52f4869379ff2d2cb"
		"0e5add6e8afb6afe0b63de8742798a68"
		"51289b7aebafb82f9dd1c7459008c983"
		"e98384cb28690969ce99460054cbd838"
		"f9534abf31ce571533fa96043342e3c0"
		"b7544a657a7c02e61995d00e820763f9"
		"e12b2afc559252c9b59f232860e72051"
		"10d3ed6d9babb8e25d9a34b3be9c64cb"
		"78c69122409180bed7785c0e0adc08e9"
		"6710a483987923e792daa92216b1e778"
		"a31c6c8f357c4d372f6e0b505c34b9f9"
		"e63d910d3295aa3d481106bb2df26388"
		"3f7309e245563151fa5e4e62f790f9a9"
		"7d7b1bb1c8266e66f6909a7ff257cc23"
		"59fafaaa440401a7a478db743d8bb5"),
	    SHEX("840bdbd5b7a8fe20bbb1127f41eab3c0"
		 "a2b437191158b60b4c1d380554d11673"
		 "8e1c2090a29ab77447e6d8fc183ab4ea"
		 "d5165a2c530146b31833746c50f2e8c0"
		 "73da6022ebe3e59b20936c4b3799b823"
		 "3b4eace85be80fb7c38ffb4a37d93995"
		 "34f1db8f71d9c70b02f163fc9bfcc5ab"
		 "b9141321dfceaa8844301ece260192f8"
		 "9f004b0c4bf75fe089ca9466112197ca"
		 "3e83742ddb4d11eb97c214ff9e1ea06b"
		 "08b4312b85c6856c90ec39c0ecb3b54e"
		 "f39ce7833a770af456fece18336d0b2d"
		 "33dac8055cb4092ade6b529801ef363d"
		 "bdf98fa83eaacdd1012d4249c3b684bb"
		 "4896e090936c4864d4fa7f932ca621c8"
		 "7a237baa205612ae169d940f54a1ecca"
		 "514ef239f4f85f045a0dbff583a115e1"
		 "f53cd862a3ed4789854ce5dbac9e171d"
		 "0c09e33e395b4d740ef534ee70114cfd"
		 "db34b1b5103f73b7f5faedb01fa5cd3c"
		 "8d3583d411446e6c5be00e69a539e5bb"
		 "a9572437e61fddcf162a13f96a2d90a0"
		 "03607aed69d5008b7e4fcbb9fa91b937"
		 "c126ce9097226464c172431bf6acc154"
		 "8a109cdd8dd58eb2e485dae0205ff4b4"
		 "15b5a08d127449233adf4ad3f03b89eb"
		 "f8cc627bfb9307416126945870a63ce4"
		 "ff58c4133dcb366b32e5b26d03746f76"
		 "9377de48c4fa304ada4980770f1cbe11"
		 "c848b1e5bbf28ae1962f9fd18e8a5ce2"
		 "f7d7d854f33fc491b8fb86dc46249160"
		 "6c2fc94137514954098121f3039f2be3"
		 "1f3963aff4d75360a7c754f9eeb1b17d"
		 "75546593feb1686b5702f9bb0ef9f8bf"
		 "011227b4fee4797a405b514bdf38ecb1"
		 "6a56ff354d4233aa6f1be4dce0db8535"
		 "6210d4ecebc57e451c6f17ca3b8e2d66"
		 "4f4b3656cd1b59aad29b17b958df7b64"
		 "8aff3b9ca6b5489eaae25d0971325fb6"
		 "29bee7c7527e91826b6d33e134063621"
		 "5ebe1e2f3ec1fbea492cb5caf7b037ea"
		 "1fed1004d9480d1a1cfbe7840e835374"
		 "c765e25ce5ba734c0ee1b51145614346"
		 "aa258fbd8508fa4c15c1c0d8f5dc16bb"
		 "7b1de38757a72a1d38589e8a43dc57"),
	    SHEX("00ffffffff0000ffffff00ff"),
	    SHEX("d1817d2be9ff993a4b24525855e14914"));

  /* Same input, but different initial counter value, to trigger wraparound. */
  test_aead(&nettle_gcm_aes256,
	    (nettle_hash_update_func *) gcm_aes128_set_iv_hack,
	    SHEX("6235f895fca5ebf60e921204d3a13f2e"
	         "8b32cfe744ed1359043877b0b9adb438"),
	    SHEX(""),
	    SHEX("42c1cc08486f413f2f11668b2a16f0e0"
		"5883f0c37014c05b3fec1d253c51d203"
		"cf59741fb285b407c66a63398a5bdecb"
		"af0844bd6f9115e1f57a6e18bddd6150"
		"59a997abbb0e745c00a4435404549b3b"
		"77ecfd5ca6e87b08aee6103f3265d1fc"
		"a41d2c31fb337ab33523f42041d4ad82"
		"8ba4ad961c2053be0ea6f4dc78493e72"
		"b1a9b583cb0854b7ad493aae98cea666"
		"1030908c5583d77c8be653ded26e1821"
		"0152d19f9dbb9c7357cc8909759b7870"
		"ed26974db4e40ca5fa700470c6961c7d"
		"544177a8e3b07e9682d9eca2876855f9"
		"8f9e7343476a08369367a82ddeac41a9"
		"5c4d73970f7068fa564d00c23b1fc8b9"
		"781f5107e39a134eed2b2ea3f744b2e7"
		"ab1937d9ba765ed2f25315174c6b169f"
		"026649ca7c9105f245361ef577ad1f46"
		"a813fb63b608996382a2edb3acdf4319"
		"45ea7873d9b73911a3137cf83ff7ad81"
		"482fa95c5fa0f079a4477d802026fd63"
		"0ac77e6d7547ff76662e8a6c8135af0b"
		"2e6a4960c110e1e15403a4090c377a15"
		"23275b8b4ba56497ae4a50731f661c5c"
		"03253c8d485871340eec4e551a036ae5"
		"b6192b842a20d1ea806f960e0562c778"
		"8779603846b425576e1663f8ad6ed742"
		"69e188ef6ed5b49a3c786c3be5a01d22"
		"865c743aeb2426c709fc919647874f1a"
		"d66b2c1847c0b824a85a4a9ecb03e72a"
		"09e64d9c6d8660f52f4869379ff2d2cb"
		"0e5add6e8afb6afe0b63de8742798a68"
		"51289b7aebafb82f9dd1c7459008c983"
		"e98384cb28690969ce99460054cbd838"
		"f9534abf31ce571533fa96043342e3c0"
		"b7544a657a7c02e61995d00e820763f9"
		"e12b2afc559252c9b59f232860e72051"
		"10d3ed6d9babb8e25d9a34b3be9c64cb"
		"78c69122409180bed7785c0e0adc08e9"
		"6710a483987923e792daa92216b1e778"
		"a31c6c8f357c4d372f6e0b505c34b9f9"
		"e63d910d3295aa3d481106bb2df26388"
		"3f7309e245563151fa5e4e62f790f9a9"
		"7d7b1bb1c8266e66f6909a7ff257cc23"
		"59fafaaa440401a7a478db743d8bb5"),
	    SHEX("abdf2d43d5acea0e 037296441e544e8f"
		 "d9b2fdfc434b3966 be04e88b226b9bbd"
		 "ed6c798834bf5283 30d5a386d49b5a45"
		 "2076bc49acf3d854 c15b52b15ab0008f"
		 "b28069951f2baf3a d845ead585168f25"
		 "b126ea81592fc417 3a4664cb599992dc"
		 "5e2aebeb9a7f0ce3 46d2d100295469f2"
		 "cae1f9190c3f50cd 8f2a4f19ea285453"
		 "cbb7ab12f79807e5 400020da75e12ff6"
		 "3a436705056e46bb abd17cc1e1a33b39"
		 "4df0802b60bbe8cc 3aa5627c70279019"
		 "7dca60f33e0eb11d cda293ac1cbe7454"
		 "66f1c91f205e87a0 c84f06b0d920f973"
		 "a1378dccc7950361 b7e406e557437005"
		 "72fe973681beae6d 4a6947e3776f70f3"
		 "71f9b1b3fbe70a51 2a0b9e6a6e6c7fd9"
		 "b5a3471734e55883 5edddf7fb99001cf"
		 "65fdf667c395724e 1984a0cff12a7c82"
		 "9a740788cfc85c84 10e807d7b1c5860b"
		 "5131eb7445ab198f 21a403a9284e44f0"
		 "4a1383f19c6cf199 5ff1c72c83c3a34e"
		 "f090bb8d3bc9fea0 ce70208e4effac75"
		 "d930d8c81e6ca39a 94795f27d704724e"
		 "873d43d6c4f6b080 221d892ec3a813b8"
		 "9dfbf54b81d03b92 5805df1d3a510a58"
		 "7303010c64c44fff 7fc8e5e7807ddfa0"
		 "24e93a62d5ec07ee 1e12fa6d4676e8e9"
		 "44ebbb62c61055f0 1634038b1306de00"
		 "645de12137a32634 66c482feae4d9212"
		 "5f5e8c48824d47a2 4233de2bf15f797b"
		 "aa4ac69555d2f83c 95f8b5ea6aab9c58"
		 "71efdf2d37dc48e8 045329279fb161ce"
		 "c791d786b8b13ade 934c191376dcbbd7"
		 "fca82eb907b71fe4 1d2c57e11c502933"
		 "e770d742cdfc65d0 0d8f434b76cb5808"
		 "4965dfade4c5a682 8e263fe55bd12052"
		 "835e3ed3e8387163 b77ddc5c210181da"
		 "5ec215b884d353ad 678ca70fc0251c35"
		 "e411707a9649e1bc 4ee3e3b550ee286e"
		 "9f51c98857530d88 e17b6e6dfacbe809"
		 "7e1ed9df02427c7b 59e03f823ee85f35"
		 "65066f1d8cc286b1 e1e13259769b6ebf"
		 "60ebd2d913e6d019 755f6d6811d3e606"
		 "8f42b10f2e02a646 8b0a9b7889b99b7c"
		 "1754b9ee8e03b3c2 5dcf41b71f3c64"),
	    SHEX("00ffffffff0000ff ffff00fffffffffd"), /* ctr == 2^31-3 */
	    SHEX("d64dd27c678a2827 859bd29e7ea4ae07"));


  /* Test case 7 */
  test_aead(&nettle_gcm_aes192, NULL,
	    SHEX("00000000000000000000000000000000"
		 "0000000000000000"),
	    SHEX(""),
	    SHEX(""),
	    SHEX(""),
	    SHEX("000000000000000000000000"),
	    SHEX("cd33b28ac773f74ba00ed1f312572435"));

  /* Test case 8 */
  test_aead(&nettle_gcm_aes192, NULL,
	    SHEX("00000000000000000000000000000000"
		 "0000000000000000"),
	    SHEX(""),
	    SHEX("00000000000000000000000000000000"),
	    SHEX("98e7247c07f0fe411c267e4384b0f600"),
	    SHEX("000000000000000000000000"),
	    SHEX("2ff58d80033927ab8ef4d4587514f0fb"));

  /* Test case 9 */
  test_aead(&nettle_gcm_aes192, NULL,
	    SHEX("feffe9928665731c6d6a8f9467308308"
		 "feffe9928665731c"),
	    SHEX(""),
	    SHEX("d9313225f88406e5a55909c5aff5269a"
		 "86a7a9531534f7da2e4c303d8a318a72"
		 "1c3c0c95956809532fcf0e2449a6b525"
		 "b16aedf5aa0de657ba637b391aafd255"),
	    SHEX("3980ca0b3c00e841eb06fac4872a2757"
		 "859e1ceaa6efd984628593b40ca1e19c"
		 "7d773d00c144c525ac619d18c84a3f47"
		 "18e2448b2fe324d9ccda2710acade256"),
	    SHEX("cafebabefacedbaddecaf888"),
	    SHEX("9924a7c8587336bfb118024db8674a14"));

  /* Test case 10 */
  test_aead(&nettle_gcm_aes192, NULL,
	    SHEX("feffe9928665731c6d6a8f9467308308"
		 "feffe9928665731c"),
	    SHEX("feedfacedeadbeeffeedfacedeadbeef"
		 "abaddad2"),
	    SHEX("d9313225f88406e5a55909c5aff5269a"
		 "86a7a9531534f7da2e4c303d8a318a72"
		 "1c3c0c95956809532fcf0e2449a6b525"
		 "b16aedf5aa0de657ba637b39"),
	    SHEX("3980ca0b3c00e841eb06fac4872a2757"
		 "859e1ceaa6efd984628593b40ca1e19c"
		 "7d773d00c144c525ac619d18c84a3f47"
		 "18e2448b2fe324d9ccda2710"),
	    SHEX("cafebabefacedbaddecaf888"),
	    SHEX("2519498e80f1478f37ba55bd6d27618c"));

  /* Test case 11 */
  test_aead(&nettle_gcm_aes192,
	    (nettle_hash_update_func *) gcm_aes192_set_iv,
	    SHEX("feffe9928665731c6d6a8f9467308308"
		 "feffe9928665731c"),
	    SHEX("feedfacedeadbeeffeedfacedeadbeef"
		 "abaddad2"),
	    SHEX("d9313225f88406e5a55909c5aff5269a"
		 "86a7a9531534f7da2e4c303d8a318a72"
		 "1c3c0c95956809532fcf0e2449a6b525"
		 "b16aedf5aa0de657ba637b39"),
	    SHEX("0f10f599ae14a154ed24b36e25324db8"
		 "c566632ef2bbb34f8347280fc4507057"
		 "fddc29df9a471f75c66541d4d4dad1c9"
		 "e93a19a58e8b473fa0f062f7"),
	    SHEX("cafebabefacedbad"),
	    SHEX("65dcc57fcf623a24094fcca40d3533f8"));

  /* Test case 12 */
  test_aead(&nettle_gcm_aes192,
	    (nettle_hash_update_func *) gcm_aes192_set_iv,
	    SHEX("feffe9928665731c6d6a8f9467308308"
		 "feffe9928665731c"),
	    SHEX("feedfacedeadbeeffeedfacedeadbeef"
		 "abaddad2"),
	    SHEX("d9313225f88406e5a55909c5aff5269a"
		 "86a7a9531534f7da2e4c303d8a318a72"
		 "1c3c0c95956809532fcf0e2449a6b525"
		 "b16aedf5aa0de657ba637b39"),
	    SHEX("d27e88681ce3243c4830165a8fdcf9ff"
		 "1de9a1d8e6b447ef6ef7b79828666e45"
		 "81e79012af34ddd9e2f037589b292db3"
		 "e67c036745fa22e7e9b7373b"),
	    SHEX("9313225df88406e555909c5aff5269aa"
		 "6a7a9538534f7da1e4c303d2a318a728"
		 "c3c0c95156809539fcf0e2429a6b5254"
		 "16aedbf5a0de6a57a637b39b"),
	    SHEX("dcf566ff291c25bbb8568fc3d376a6d9"));

  /* Test case 13 */
  test_aead(&nettle_gcm_aes256, NULL,
	    SHEX("00000000000000000000000000000000"
		 "00000000000000000000000000000000"),
	    SHEX(""),
	    SHEX(""),
	    SHEX(""),
	    SHEX("000000000000000000000000"),
	    SHEX("530f8afbc74536b9a963b4f1c4cb738b"));

  /* Test case 14 */
  test_aead(&nettle_gcm_aes256, NULL,
	    SHEX("00000000000000000000000000000000"
		 "00000000000000000000000000000000"),
	    SHEX(""),
	    SHEX("00000000000000000000000000000000"),
	    SHEX("cea7403d4d606b6e074ec5d3baf39d18"),
	    SHEX("000000000000000000000000"),
	    SHEX("d0d1c8a799996bf0265b98b5d48ab919"));

  /* Test case 15 */
  test_aead(&nettle_gcm_aes256, NULL,
	    SHEX("feffe9928665731c6d6a8f9467308308"
		 "feffe9928665731c6d6a8f9467308308"),
	    SHEX(""),
	    SHEX("d9313225f88406e5a55909c5aff5269a"
		 "86a7a9531534f7da2e4c303d8a318a72"
		 "1c3c0c95956809532fcf0e2449a6b525"
		 "b16aedf5aa0de657ba637b391aafd255"),
	    SHEX("522dc1f099567d07f47f37a32a84427d"
		 "643a8cdcbfe5c0c97598a2bd2555d1aa"
		 "8cb08e48590dbb3da7b08b1056828838"
		 "c5f61e6393ba7a0abcc9f662898015ad"),
	    SHEX("cafebabefacedbaddecaf888"),
	    SHEX("b094dac5d93471bdec1a502270e3cc6c"));

  /* Test case 16 */
  test_aead(&nettle_gcm_aes256, NULL,
	    SHEX("feffe9928665731c6d6a8f9467308308"
		 "feffe9928665731c6d6a8f9467308308"),
	    SHEX("feedfacedeadbeeffeedfacedeadbeef"
		 "abaddad2"),
	    SHEX("d9313225f88406e5a55909c5aff5269a"
		 "86a7a9531534f7da2e4c303d8a318a72"
		 "1c3c0c95956809532fcf0e2449a6b525"
		 "b16aedf5aa0de657ba637b39"),
	    SHEX("522dc1f099567d07f47f37a32a84427d"
		 "643a8cdcbfe5c0c97598a2bd2555d1aa"
		 "8cb08e48590dbb3da7b08b1056828838"
		 "c5f61e6393ba7a0abcc9f662"),
	    SHEX("cafebabefacedbaddecaf888"),
	    SHEX("76fc6ece0f4e1768cddf8853bb2d551b"));

  /* Test case 17 */
  test_aead(&nettle_gcm_aes256,
	    (nettle_hash_update_func *) gcm_aes256_set_iv,
	    SHEX("feffe9928665731c6d6a8f9467308308"
		 "feffe9928665731c6d6a8f9467308308"),
	    SHEX("feedfacedeadbeeffeedfacedeadbeef"
		 "abaddad2"),
	    SHEX("d9313225f88406e5a55909c5aff5269a"
		 "86a7a9531534f7da2e4c303d8a318a72"
		 "1c3c0c95956809532fcf0e2449a6b525"
		 "b16aedf5aa0de657ba637b39"),
	    SHEX("c3762df1ca787d32ae47c13bf19844cb"
		 "af1ae14d0b976afac52ff7d79bba9de0"
		 "feb582d33934a4f0954cc2363bc73f78"
		 "62ac430e64abe499f47c9b1f"),
	    SHEX("cafebabefacedbad"),
	    SHEX("3a337dbf46a792c45e454913fe2ea8f2"));

  /* Test case 18 */
  test_aead(&nettle_gcm_aes256,
	    (nettle_hash_update_func *) gcm_aes256_set_iv,
	    SHEX("feffe9928665731c6d6a8f9467308308"
		 "feffe9928665731c6d6a8f9467308308"),
	    SHEX("feedfacedeadbeeffeedfacedeadbeef"
		 "abaddad2"),
	    SHEX("d9313225f88406e5a55909c5aff5269a"
		 "86a7a9531534f7da2e4c303d8a318a72"
		 "1c3c0c95956809532fcf0e2449a6b525"
		 "b16aedf5aa0de657ba637b39"),
	    SHEX("5a8def2f0c9e53f1f75d7853659e2a20"
		 "eeb2b22aafde6419a058ab4f6f746bf4"
		 "0fc0c3b780f244452da3ebf1c5d82cde"
		 "a2418997200ef82e44ae7e3f"),
	    SHEX("9313225df88406e555909c5aff5269aa"
		 "6a7a9538534f7da1e4c303d2a318a728"
		 "c3c0c95156809539fcf0e2429a6b5254"
		 "16aedbf5a0de6a57a637b39b"),
	    SHEX("a44a8266ee1c8eb0c8b5d4cf5ae9f19a"));



  /* 
   * GCM-Camellia Test Vectors obtained from the authors
   */

  /* Test case 1 */
  test_aead(&nettle_gcm_camellia128,
	    (nettle_hash_update_func *) gcm_camellia128_set_iv,
	    SHEX("00000000000000000000000000000000"),	/* key */
	    SHEX(""),					/* auth data */ 
	    SHEX(""),					/* plaintext */
	    SHEX(""),					/* ciphertext*/
	    SHEX("000000000000000000000000"),		/* IV */
	    SHEX("f5574acc3148dfcb9015200631024df9"));	/* tag */

  /* Test case 3 */
  test_aead(&nettle_gcm_camellia128,
	    (nettle_hash_update_func *) gcm_camellia128_set_iv,
	    SHEX("feffe9928665731c6d6a8f9467308308"),	/* key */
	    SHEX(""),					/* auth data */ 
	    SHEX("d9313225f88406e5a55909c5aff5269a86a7a9531534f7da2e4c303d8a318a72"
	         "1c3c0c95956809532fcf0e2449a6b525b16aedf5aa0de657ba637b391aafd255"),					/* plaintext */
	    SHEX("d0d94a13b632f337a0cc9955b94fa020c815f903aab12f1efaf2fe9d90f729a6"
	         "cccbfa986ef2ff2c33de418d9a2529091cf18fe652c1cfde13f8260614bab815"),					/* ciphertext*/
	    SHEX("cafebabefacedbaddecaf888"),		/* IV */
	    SHEX("86e318012dd8329dc9dae6a170f61b24"));	/* tag */

  /* Test case 4 */
  test_aead(&nettle_gcm_camellia128,
	    (nettle_hash_update_func *) gcm_camellia128_set_iv,
	    SHEX("feffe9928665731c6d6a8f9467308308"),	/* key */
	    SHEX("feedfacedeadbeeffeedfacedeadbeefabaddad2"),					/* auth data */ 
	    SHEX("d9313225f88406e5a55909c5aff5269a86a7a9531534f7da2e4c303d8a318a72"
	         "1c3c0c95956809532fcf0e2449a6b525b16aedf5aa0de657ba637b39"),					/* plaintext */
	    SHEX("d0d94a13b632f337a0cc9955b94fa020c815f903aab12f1efaf2fe9d90f729a6"
	         "cccbfa986ef2ff2c33de418d9a2529091cf18fe652c1cfde13f82606"),					/* ciphertext*/
	    SHEX("cafebabefacedbaddecaf888"),		/* IV */
	    SHEX("9f458869431576ea6a095456ec6b8101"));	/* tag */

  /* Test case 5 */
  test_aead(&nettle_gcm_camellia128,
	    (nettle_hash_update_func *) gcm_camellia128_set_iv,
	    SHEX("feffe9928665731c6d6a8f9467308308"),	/* key */
	    SHEX("feedfacedeadbeeffeedfacedeadbeefabaddad2"),					/* auth data */ 
	    SHEX("d9313225f88406e5a55909c5aff5269a86a7a9531534f7da2e4c303d8a318a72"
	         "1c3c0c95956809532fcf0e2449a6b525b16aedf5aa0de657ba637b39"),					/* plaintext */
	    SHEX("28fd7434d5cd424a5353818fc21a982460d20cf632eb1e6c4fbfca17d5abcf6a"
	         "52111086162fe9570e7774c7a912aca3dfa10067ddaad40688645bdd"),					/* ciphertext*/
	    SHEX("cafebabefacedbad"),		/* IV */
	    SHEX("e86f8f2e730c49d536f00fb5225d28b1"));	/* tag */

  /* Test case 6 */
  test_aead(&nettle_gcm_camellia128,
	    (nettle_hash_update_func *) gcm_camellia128_set_iv,
	    SHEX("feffe9928665731c6d6a8f9467308308"),	/* key */
	    SHEX("feedfacedeadbeeffeedfacedeadbeefabaddad2"),					/* auth data */ 
	    SHEX("d9313225f88406e5a55909c5aff5269a86a7a9531534f7da2e4c303d8a318a72"
	         "1c3c0c95956809532fcf0e2449a6b525b16aedf5aa0de657ba637b39"),					/* plaintext */
	    SHEX("2e582b8417c93f2ff4f6f7ee3c361e4496e710ee12433baa964987d02f42953e"
	         "402e6f4af407fe08cd2f35123696014c34db19128df4056faebcd647"),					/* ciphertext*/
	    SHEX("9313225df88406e555909c5aff5269aa6a7a9538534f7da1e4c303d2a318a728"
	         "c3c0c95156809539fcf0e2429a6b525416aedbf5a0de6a57a637b39b"),		/* IV */
	    SHEX("ceae5569b2af8641572622731aed3e53"));	/* tag */

  /* gcm-camellia256 */

  /* Test case 13 */
  test_aead(&nettle_gcm_camellia256,
	    (nettle_hash_update_func *) gcm_camellia256_set_iv,
	    SHEX("0000000000000000 0000000000000000"
		 "0000000000000000 0000000000000000"),	/* key */
	    SHEX(""),	/* auth data */
	    SHEX(""),	/* plaintext */
	    SHEX(""),	/* ciphertext */
	    SHEX("000000000000000000000000"),	/* iv */
	    SHEX("9cdb269b5d293bc5db9c55b057d9b591"));	/* tag */

  /* Test case 14 */
  test_aead(&nettle_gcm_camellia256,
	    (nettle_hash_update_func *) gcm_camellia256_set_iv,
	    SHEX("0000000000000000 0000000000000000"
		 "0000000000000000 0000000000000000"),	/* key */
	    SHEX(""),	/* auth data */
	    SHEX("0000000000000000 0000000000000000"),	/* plaintext */
	    SHEX("3d4b2cde666761ba 5dfb305178e667fb"),	/* ciphertext */
	    SHEX("000000000000000000000000"),	/* iv */
	    SHEX("284b63bb143c40ce100fb4dea6bb617b"));	/* tag */

  /* Test case 15 */
  test_aead(&nettle_gcm_camellia256,
	    (nettle_hash_update_func *) gcm_camellia256_set_iv,
	    SHEX("feffe9928665731c 6d6a8f9467308308"
		 "feffe9928665731c 6d6a8f9467308308"),	/* key */
	    SHEX(""),	/* auth data */
	    SHEX("d9313225f88406e5 a55909c5aff5269a"
		 "86a7a9531534f7da 2e4c303d8a318a72"
		 "1c3c0c9595680953 2fcf0e2449a6b525"
		 "b16aedf5aa0de657 ba637b391aafd255"),	/* plaintext */
	    SHEX("ad142c11579dd95e 41f3c1f324dabc25"
		 "5864d920f1b65759 d8f560d4948d4477"
		 "58dfdcf77aa9f625 81c7ff572a037f81"
		 "0cb1a9c4b3ca6ed6 38179b776549e092"),	/* ciphertext */
	    SHEX("cafebabefacedbaddecaf888"),	/* iv */
	    SHEX("c912686270a2b9966415fca3be75c468"));	/* tag */

  /* Test case 16 */
  test_aead(&nettle_gcm_camellia256,
	    (nettle_hash_update_func *) gcm_camellia256_set_iv,
	    SHEX("feffe9928665731c 6d6a8f9467308308"
		 "feffe9928665731c 6d6a8f9467308308"),	/* key */
	    SHEX("feedfacedeadbeef feedfacedeadbeef"
		 "abaddad2"),	/* auth data */
	    SHEX("d9313225f88406e5 a55909c5aff5269a"
		 "86a7a9531534f7da 2e4c303d8a318a72"
		 "1c3c0c9595680953 2fcf0e2449a6b525"
		 "b16aedf5aa0de657 ba637b39"),	/* plaintext */
	    SHEX("ad142c11579dd95e 41f3c1f324dabc25"
		 "5864d920f1b65759 d8f560d4948d4477"
		 "58dfdcf77aa9f625 81c7ff572a037f81"
		 "0cb1a9c4b3ca6ed6 38179b77"),	/* ciphertext */
	    SHEX("cafebabefacedbaddecaf888"),	/* iv */
	    SHEX("4e4b178d8fe26fdc95e2e7246dd94bec"));	/* tag */

  /* Test case 17 */
  test_aead(&nettle_gcm_camellia256,
	    (nettle_hash_update_func *) gcm_camellia256_set_iv,
	    SHEX("feffe9928665731c 6d6a8f9467308308"
		 "feffe9928665731c 6d6a8f9467308308"),	/* key */
	    SHEX("feedfacedeadbeef feedfacedeadbeef"
		 "abaddad2"),	/* auth data */
	    SHEX("d9313225f88406e5 a55909c5aff5269a"
		 "86a7a9531534f7da 2e4c303d8a318a72"
		 "1c3c0c9595680953 2fcf0e2449a6b525"
		 "b16aedf5aa0de657 ba637b39"),	/* plaintext */
	    SHEX("6ca95fbb7d16577a 9ef2fded94dc85b5"
		 "d40c629f6bef2c64 9888e3cbb0ededc7"
		 "810c04b12c2983bb bbc482e16e45c921"
		 "5ae12c15c55f2f48 09d06652"),	/* ciphertext */
	    SHEX("cafebabefacedbad"),	/* iv */
	    SHEX("e6472b8ebd331bfcc7c0fa63ce094461"));	/* tag */

  /* Test case 18 */
  test_aead(&nettle_gcm_camellia256,
	    (nettle_hash_update_func *) gcm_camellia256_set_iv,
	    SHEX("feffe9928665731c 6d6a8f9467308308"
		 "feffe9928665731c 6d6a8f9467308308"),	/* key */
	    SHEX("feedfacedeadbeef feedfacedeadbeef"
		 "abaddad2"),	/* auth data */
	    SHEX("d9313225f88406e5 a55909c5aff5269a"
		 "86a7a9531534f7da 2e4c303d8a318a72"
		 "1c3c0c9595680953 2fcf0e2449a6b525"
		 "b16aedf5aa0de657 ba637b39"),	/* plaintext */
	    SHEX("e0cddd7564d09c4d c522dd65949262bb"
		 "f9dcdb07421cf67f 3032becb7253c284"
		 "a16e5bf0f556a308 043f53fab9eebb52"
		 "6be7f7ad33d697ac 77c67862"),	/* ciphertext */
	    SHEX("9313225df88406e5 55909c5aff5269aa"
		 "6a7a9538534f7da1 e4c303d2a318a728"
		 "c3c0c95156809539 fcf0e2429a6b5254"
		 "16aedbf5a0de6a57 a637b39b"),	/* iv */
	    SHEX("5791883f822013f8bd136fc36fb9946b"));	/* tag */

  /*
   * GCM-SM4 Test Vectors from
   * https://datatracker.ietf.org/doc/html/rfc8998
   */
  test_aead(&nettle_gcm_sm4, NULL,
	    SHEX("0123456789ABCDEFFEDCBA9876543210"),
	    SHEX("FEEDFACEDEADBEEFFEEDFACEDEADBEEFABADDAD2"),
	    SHEX("AAAAAAAAAAAAAAAABBBBBBBBBBBBBBBB"
	         "CCCCCCCCCCCCCCCCDDDDDDDDDDDDDDDD"
	         "EEEEEEEEEEEEEEEEFFFFFFFFFFFFFFFF"
	         "EEEEEEEEEEEEEEEEAAAAAAAAAAAAAAAA"),
	    SHEX("17F399F08C67D5EE19D0DC9969C4BB7D"
	         "5FD46FD3756489069157B282BB200735"
	         "D82710CA5C22F0CCFA7CBF93D496AC15"
	         "A56834CBCF98C397B4024A2691233B8D"),
	    SHEX("00001234567800000000ABCD"),
	    SHEX("83DE3541E4C2B58177E065A9BF7B62EC"));

  /* Test gcm_hash, with varying message size, keys and iv all zero.
     Not compared to any other implementation. */
  test_gcm_hash (SDATA("a"),
		 SHEX("1521c9a442bbf63b 2293a21d4874a5fd"));
  test_gcm_hash (SDATA("ab"),
		 SHEX("afb4592d2c7c1687 37f27271ee30412a"));
  test_gcm_hash (SDATA("abc"), 
		 SHEX("9543ca3e1662ba03 9a921ec2a20769be"));
  test_gcm_hash (SDATA("abcd"),
		 SHEX("8f041cc12bcb7e1b 0257a6da22ee1185"));
  test_gcm_hash (SDATA("abcde"),
		 SHEX("0b2376e5fed58ffb 717b520c27cd5c35"));
  test_gcm_hash (SDATA("abcdef"), 
		 SHEX("9679497a1eafa161 4942963380c1a76f"));
  test_gcm_hash (SDATA("abcdefg"),
		 SHEX("83862e40339536bc 723d9817f7df8282"));
  test_gcm_hash (SDATA("abcdefgh"), 
		 SHEX("b73bcc4d6815c4dc d7424a04e61b87c5"));
  test_gcm_hash (SDATA("abcdefghi"), 
		 SHEX("8e7846a383f0b3b2 07b01160a5ef993d"));
  test_gcm_hash (SDATA("abcdefghij"),
		 SHEX("37651643b6f8ecac 4ea1b320e6ea308c"));
  test_gcm_hash (SDATA("abcdefghijk"), 
		 SHEX("c1ce10106ee23286 f00513f55e2226b0"));
  test_gcm_hash (SDATA("abcdefghijkl"),
		 SHEX("c6a3e32a90196cdf b2c7a415d637e6ca"));
  test_gcm_hash (SDATA("abcdefghijklm"), 
		 SHEX("6cca29389d4444fa 3d20e65497088fd8"));
  test_gcm_hash (SDATA("abcdefghijklmn"),
		 SHEX("19476a997ec0a824 2022db0f0e8455ce"));
  test_gcm_hash (SDATA("abcdefghijklmno"), 
		 SHEX("f66931cee7eadcbb d42753c3ac3c4c16"));
  test_gcm_hash (SDATA("abcdefghijklmnop"),
		 SHEX("a79699ce8bed61f9 b8b1b4c5abb1712e"));
  test_gcm_hash (SDATA("abcdefghijklmnopq"), 
		 SHEX("65f8245330febf15 6fd95e324304c258"));
  test_gcm_hash (SDATA("abcdefghijklmnopqr"),
		 SHEX("d07259e85d4fc998 5a662eed41c8ed1d"));

  /* Test internal ghash function. */
  test_ghash_internal(SHEX("0000000000000000 0000000000000000"),
		      SHEX("0000000000000000 0000000000000000"),
		      SHEX("0011223344556677 89abcdef01234567"),
		      SHEX("0000000000000000 0000000000000000"));

  test_ghash_internal(SHEX("8000000000000000 0000000000000000"), /* 1 polynomial */
		      SHEX("0000000000000000 0000000000000000"),
		      SHEX("0011223344556677 89abcdef01234567"),
		      SHEX("0011223344556677 89abcdef01234567"));

  test_ghash_internal(SHEX("8000000000000000 0000000000000000"),
		      SHEX("0123456789abcdef fedcba9876543210"), /* XOR:ed to the message */
		      SHEX("0011223344556677 89abcdef01234567"),
		      SHEX("01326754cdfeab98 7777777777777777"));

  test_ghash_internal(SHEX("4000000000000000 0000000000000000"), /* x polynomial */
		      SHEX("0000000000000000 0000000000000000"),
		      SHEX("0011223344556677 89abcdef01234567"),
		      SHEX("e1089119a22ab33b c4d5e6f78091a2b3"));

  test_ghash_internal(SHEX("0800000000000000 0000000000000000"), /* x^4 polynomial */
		      SHEX("0000000000000000 0000000000000000"),
		      SHEX("0011223344556677 89abcdef01234567"),
		      SHEX("54e1122334455667 789abcdef0123456"));

  test_ghash_internal(SHEX("0000000080000000 0000000000000000"), /* x^32 polynomial */
		      SHEX("0000000000000000 0000000000000000"),
		      SHEX("0011223344556677 89abcdef01234567"),
		      SHEX("01f870078e112233 4455667789abcdef"));

  test_ghash_internal(SHEX("0000000000000001 0000000000000000"), /* x^63 polynomial */
		      SHEX("0000000000000000 0000000000000000"),
		      SHEX("0011223344556677 89abcdef01234567"),
		      SHEX("1e0f1ff13ff0e00f 1c22446688aaccef"));

  test_ghash_internal(SHEX("0000000000000000 8000000000000000"), /* x^64 polynomial */
		      SHEX("0000000000000000 0000000000000000"),
		      SHEX("0011223344556677 89abcdef01234567"),
		      SHEX("ee078ff89ff87007 8e11223344556677"));

  test_ghash_internal(SHEX("0000000000000000 0000000100000000"), /* x^95 polynomial */
		      SHEX("0000000000000000 0000000000000000"),
		      SHEX("0011223344556677 89abcdef01234567"),
		      SHEX("efc44c3a800f1ff1 3ff0e00f1c224466"));

  test_ghash_internal(SHEX("0000000000000000 0000000080000000"), /* x^96 polynomial */
		      SHEX("0000000000000000 0000000000000000"),
		      SHEX("0011223344556677 89abcdef01234567"),
		      SHEX("77e2261d40078ff8 9ff870078e112233"));

  test_ghash_internal(SHEX("0000000000000000 0000000000000001"), /* x^127 polynomial */
		      SHEX("0000000000000000 0000000000000000"),
		      SHEX("0011223344556677 89abcdef01234567"),
		      SHEX("1503b3c4a3c44c3a 800f1ff13ff0e00f"));
}
