#include "testutils.h"
#include "aes.h"
#include "cfb.h"
#include "knuth-lfib.h"

/* Test with more data and inplace decryption, to check that the
 * cfb_decrypt buffering works. */
#define CFB_BULK_DATA 10000

static void
test_cfb_bulk(void)
{
  struct knuth_lfib_ctx random;

  uint8_t clear[CFB_BULK_DATA];

  uint8_t cipher[CFB_BULK_DATA + 1];

  const uint8_t *key = H("966c7bf00bebe6dc 8abd37912384958a"
			 "743008105a08657d dcaad4128eee38b3");

  const uint8_t *start_iv = H("11adbff119749103 207619cfa0e8d13a");
  const uint8_t *end_iv = H("1fd0a9189b8480b7 b06a2b36ef5943ba");

  struct CFB_CTX(struct aes_ctx, AES_BLOCK_SIZE) aes;

  knuth_lfib_init(&random, CFB_BULK_DATA);
  knuth_lfib_random(&random, CFB_BULK_DATA, clear);

  /* Byte that should not be overwritten */
  cipher[CFB_BULK_DATA] = 17;

  aes_set_encrypt_key(&aes.ctx, 32, key);
  CFB_SET_IV(&aes, start_iv);

  CFB_ENCRYPT(&aes, aes_encrypt, CFB_BULK_DATA, cipher, clear);

  ASSERT(cipher[CFB_BULK_DATA] == 17);

  if (verbose)
    {
      printf("IV after bulk encryption: ");
      print_hex(AES_BLOCK_SIZE, aes.iv);
      printf("\n");
    }

  ASSERT(MEMEQ(AES_BLOCK_SIZE, aes.iv, end_iv));

  /* Decrypt, in place */
  aes_set_encrypt_key(&aes.ctx, 32, key);
  CFB_SET_IV(&aes, start_iv);
  CFB_DECRYPT(&aes, aes_encrypt, CFB_BULK_DATA, cipher, cipher);

  ASSERT(cipher[CFB_BULK_DATA] == 17);

  if (verbose)
    {
      printf("IV after bulk decryption: ");
      print_hex(AES_BLOCK_SIZE, aes.iv);
      printf("\n");
    }

  ASSERT (MEMEQ(AES_BLOCK_SIZE, aes.iv, end_iv));
  ASSERT (MEMEQ(CFB_BULK_DATA, clear, cipher));
}

void
test_main(void)
{
  /* From NIST spec 800-38a on AES modes.
   *
   * F.3  CFB Example Vectors
   * F.3.13 CFB128-AES128.Encrypt
   */

  /* Intermediate values, blocks input to AES:
   *
   *   000102030405060708090a0b0c0d0e0f
   *   3b3fd92eb72dad20333449f8e83cfb4a
   *   c8a64537a0b3a93fcde3cdad9f1ce58b
   *   26751f67a3cbb140b1808cf187a4f4df
   */
  test_cipher_cfb(&nettle_aes128,
		  SHEX("2b7e151628aed2a6abf7158809cf4f3c"),
		  SHEX("6bc1bee22e409f96e93d7e117393172a"
		       "ae2d8a571e03ac9c9eb76fac45af8e51"
		       "30c81c46a35ce411e5fbc1191a0a52ef"
		       "f69f2445df4f9b17ad2b417be66c3710"),
		  SHEX("3b3fd92eb72dad20333449f8e83cfb4a"
		       "c8a64537a0b3a93fcde3cdad9f1ce58b"
		       "26751f67a3cbb140b1808cf187a4f4df"
		       "c04b05357c5d1c0eeac4c66f9ff7f2e6"),
		  SHEX("000102030405060708090a0b0c0d0e0f"));

  /* F.3.15 CFB128-AES192.Encrypt */

  /* Intermediate values, blocks input to AES:
   *
   *   000102030405060708090a0b0c0d0e0f
   *   cdc80d6fddf18cab34c25909c99a4174
   *   67ce7f7f81173621961a2b70171d3d7a
   *   2e1e8a1dd59b88b1c8e60fed1efac4c9
   */

  test_cipher_cfb(&nettle_aes192,
		  SHEX("8e73b0f7da0e6452c810f32b809079e5"
		       "62f8ead2522c6b7b"),
		  SHEX("6bc1bee22e409f96e93d7e117393172a"
		       "ae2d8a571e03ac9c9eb76fac45af8e51"
		       "30c81c46a35ce411e5fbc1191a0a52ef"
		       "f69f2445df4f9b17ad2b417be66c3710"),
		  SHEX("cdc80d6fddf18cab34c25909c99a4174"
		       "67ce7f7f81173621961a2b70171d3d7a"
		       "2e1e8a1dd59b88b1c8e60fed1efac4c9"
		       "c05f9f9ca9834fa042ae8fba584b09ff"),
		  SHEX("000102030405060708090a0b0c0d0e0f"));

  /* F.3.17 CFB128-AES256.Encrypt */

  /* Intermediate values, blcoks input to AES:
   *
   *   000102030405060708090a0b0c0d0e0f
   *   dc7e84bfda79164b7ecd8486985d3860
   *   39ffed143b28b1c832113c6331e5407b
   *   df10132415e54b92a13ed0a8267ae2f9
   */

  test_cipher_cfb(&nettle_aes256,
		  SHEX("603deb1015ca71be2b73aef0857d7781"
		       "1f352c073b6108d72d9810a30914dff4"),
		  SHEX("6bc1bee22e409f96e93d7e117393172a"
		       "ae2d8a571e03ac9c9eb76fac45af8e51"
		       "30c81c46a35ce411e5fbc1191a0a52ef"
		       "f69f2445df4f9b17ad2b417be66c3710"),
		  SHEX("dc7e84bfda79164b7ecd8486985d3860"
		       "39ffed143b28b1c832113c6331e5407b"
		       "df10132415e54b92a13ed0a8267ae2f9"
		       "75a385741ab9cef82031623d55b1e471"),
		  SHEX("000102030405060708090a0b0c0d0e0f"));

  test_cfb_bulk();
}

/*
F.3.13 CFB128-AES128.Encrypt
Key 2b7e151628aed2a6abf7158809cf4f3c
IV 000102030405060708090a0b0c0d0e0f
Segment #1
Input Block 000102030405060708090a0b0c0d0e0f
Output Block 50fe67cc996d32b6da0937e99bafec60
Plaintext 6bc1bee22e409f96e93d7e117393172a
Ciphertext 3b3fd92eb72dad20333449f8e83cfb4a
Segment #2
Input Block 3b3fd92eb72dad20333449f8e83cfb4a
Output Block 668bcf60beb005a35354a201dab36bda
Plaintext ae2d8a571e03ac9c9eb76fac45af8e51
Ciphertext c8a64537a0b3a93fcde3cdad9f1ce58b
Segment #3
Input Block c8a64537a0b3a93fcde3cdad9f1ce58b
Output Block 16bd032100975551547b4de89daea630
Plaintext 30c81c46a35ce411e5fbc1191a0a52ef
Ciphertext 26751f67a3cbb140b1808cf187a4f4df
Segment #4
Input Block 26751f67a3cbb140b1808cf187a4f4df
Output Block 36d42170a312871947ef8714799bc5f6
Plaintext f69f2445df4f9b17ad2b417be66c3710
Ciphertext c04b05357c5d1c0eeac4c66f9ff7f2e6
F.3.14 CFB128-AES128.Decrypt
Key 2b7e151628aed2a6abf7158809cf4f3c
IV 000102030405060708090a0b0c0d0e0f
Segment #1
Input Block 000102030405060708090a0b0c0d0e0f
Output Block 50fe67cc996d32b6da0937e99bafec60
Ciphertext 3b3fd92eb72dad20333449f8e83cfb4a
Plaintext 6bc1bee22e409f96e93d7e117393172a
Segment #2
Input Block 3b3fd92eb72dad20333449f8e83cfb4a
Output Block 668bcf60beb005a35354a201dab36bda
Ciphertext c8a64537a0b3a93fcde3cdad9f1ce58b
Plaintext ae2d8a571e03ac9c9eb76fac45af8e51
Segment #3
Input Block c8a64537a0b3a93fcde3cdad9f1ce58b
Output Block 16bd032100975551547b4de89daea630
Ciphertext 26751f67a3cbb140b1808cf187a4f4df
Plaintext 30c81c46a35ce411e5fbc1191a0a52ef
Segment #4
Input Block 26751f67a3cbb140b1808cf187a4f4df
Output Block 36d42170a312871947ef8714799bc5f6
Ciphertext c04b05357c5d1c0eeac4c66f9ff7f2e6
Plaintext f69f2445df4f9b17ad2b417be66c3710
F.3.15 CFB128-AES192.Encrypt
Key 	 8e73b0f7da0e6452c810f32b809079e562f8ead2522c6b7b
000102030405060708090a0b0c0d0e0f
Segment #1
50
IV
Input Block 000102030405060708090a0b0c0d0e0f
Output Block a609b38df3b1133dddff2718ba09565e
Plaintext 6bc1bee22e409f96e93d7e117393172a
Ciphertext cdc80d6fddf18cab34c25909c99a4174
Segment #2
Input Block cdc80d6fddf18cab34c25909c99a4174
Output Block c9e3f5289f149abd08ad44dc52b2b32b
Plaintext ae2d8a571e03ac9c9eb76fac45af8e51
Ciphertext 67ce7f7f81173621961a2b70171d3d7a
Segment #3
Input Block 67ce7f7f81173621961a2b70171d3d7a
Output Block 1ed6965b76c76ca02d1dcef404f09626
Plaintext 30c81c46a35ce411e5fbc1191a0a52ef
Ciphertext 2e1e8a1dd59b88b1c8e60fed1efac4c9
Segment #4
Input Block 2e1e8a1dd59b88b1c8e60fed1efac4c9
Output Block 36c0bbd976ccd4b7ef85cec1be273eef
Plaintext f69f2445df4f9b17ad2b417be66c3710
Ciphertext c05f9f9ca9834fa042ae8fba584b09ff
F.3.16 CFB128-AES192.Decrypt
Key 8e73b0f7da0e6452c810f32b809079e562f8ead2522c6b7b
IV 000102030405060708090a0b0c0d0e0f
Segment #1
Input Block 000102030405060708090a0b0c0d0e0f
Output Block a609b38df3b1133dddff2718ba09565e
Ciphertext cdc80d6fddf18cab34c25909c99a4174
Plaintext 6bc1bee22e409f96e93d7e117393172a
Segment #2
Input Block cdc80d6fddf18cab34c25909c99a4174
Output Block c9e3f5289f149abd08ad44dc52b2b32b
Ciphertext 67ce7f7f81173621961a2b70171d3d7a
Plaintext ae2d8a571e03ac9c9eb76fac45af8e51
Segment #3
Input Block 67ce7f7f81173621961a2b70171d3d7a
Output Block 1ed6965b76c76ca02d1dcef404f09626
Ciphertext 2e1e8a1dd59b88b1c8e60fed1efac4c9
Plaintext 30c81c46a35ce411e5fbc1191a0a52ef
Segment #4
Input Block 2e1e8a1dd59b88b1c8e60fed1efac4c9
Output Block 36c0bbd976ccd4b7ef85cec1be273eef
Ciphertext c05f9f9ca9834fa042ae8fba584b09ff
Plaintext f69f2445df4f9b17ad2b417be66c3710
F.3.17 CFB128-AES256.Encrypt
Key 603deb1015ca71be2b73aef0857d7781
1f352c073b6108d72d9810a30914dff4
IV 000102030405060708090a0b0c0d0e0f
Segment #1
Input Block 000102030405060708090a0b0c0d0e0f
Output Block b7bf3a5df43989dd97f0fa97ebce2f4a
Plaintext 6bc1bee22e409f96e93d7e117393172a
Ciphertext dc7e84bfda79164b7ecd8486985d3860
Segment #2
Input Block dc7e84bfda79164b7ecd8486985d3860
Output Block 97d26743252b1d54aca653cf744ace2a
Plaintext ae2d8a571e03ac9c9eb76fac45af8e51
Ciphertext 39ffed143b28b1c832113c6331e5407b
Segment #3
Input Block 39ffed143b28b1c832113c6331e5407b
Output Block efd80f62b6b9af8344c511b13c70b016
Plaintext 30c81c46a35ce411e5fbc1191a0a52ef
Ciphertext df10132415e54b92a13ed0a8267ae2f9
Segment #4
Input Block df10132415e54b92a13ed0a8267ae2f9
Output Block 833ca131c5f655ef8d1a2346b3ddd361
Plaintext f69f2445df4f9b17ad2b417be66c3710
Ciphertext 75a385741ab9cef82031623d55b1e471
F.3.18 CFB128-AES256.Decrypt
Key 603deb1015ca71be2b73aef0857d7781
1f352c073b6108d72d9810a30914dff4
IV 000102030405060708090a0b0c0d0e0f
Segment #1
Input Block 000102030405060708090a0b0c0d0e0f
Output Block b7bf3a5df43989dd97f0fa97ebce2f4a
Ciphertext dc7e84bfda79164b7ecd8486985d3860
Plaintext 6bc1bee22e409f96e93d7e117393172a
Segment #2
Input Block dc7e84bfda79164b7ecd8486985d3860
Output Block 97d26743252b1d54aca653cf744ace2a
Ciphertext 39ffed143b28b1c832113c6331e5407b
Plaintext ae2d8a571e03ac9c9eb76fac45af8e51
Segment #3
Input Block 39ffed143b28b1c832113c6331e5407b
Output Block efd80f62b6b9af8344c511b13c70b016
Ciphertext df10132415e54b92a13ed0a8267ae2f9
Plaintext 30c81c46a35ce411e5fbc1191a0a52ef
Segment #4
Input Block df10132415e54b92a13ed0a8267ae2f9
Output Block 833ca131c5f655ef8d1a2346b3ddd361
Ciphertext 75a385741ab9cef82031623d55b1e471
Plaintext f69f2445df4f9b17ad2b417be66c3710
*/
