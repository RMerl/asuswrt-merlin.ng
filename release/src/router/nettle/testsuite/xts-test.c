#include "testutils.h"
#include "aes.h"
#include "xts.h"
#include "nettle-internal.h"

static void
test_check_data(const char *operation,
                const uint8_t *input, const uint8_t *output,
                const uint8_t *expected, size_t length)
{
  if (!MEMEQ(length, output, expected))
    {
      fprintf(stderr, "XTS %s failed:\nInput:", operation);
      print_hex(length, input);
      fprintf(stderr, "\nOutput: ");
      print_hex(length, output);
      fprintf(stderr, "\nExpected:");
      print_hex(length, expected);
      fprintf(stderr, "\n");
      FAIL();
    }
}

static void
test_cipher_xts(const struct nettle_cipher *cipher,
		const struct tstring *key,
		const struct tstring *tweak,
		const struct tstring *cleartext,
		const struct tstring *ciphertext)
{
  void *twk_ctx = xalloc(cipher->context_size);
  void *ctx = xalloc(cipher->context_size);
  uint8_t *data, *data2;
  size_t length = cleartext->length;

  ASSERT (cleartext->length == ciphertext->length);
  ASSERT (key->length == cipher->key_size * 2);
  ASSERT (tweak->length == XTS_BLOCK_SIZE);

  data = xalloc(length);
  data2 = xalloc(length);

  cipher->set_encrypt_key(ctx, key->data);
  cipher->set_encrypt_key(twk_ctx, &key->data[key->length / 2]);
  xts_encrypt_message(ctx, twk_ctx, cipher->encrypt,
                      tweak->data, length, data, cleartext->data);
  test_check_data("encrypt", cleartext->data, data, ciphertext->data, length);

  cipher->set_decrypt_key(ctx, key->data);
  cipher->set_encrypt_key(twk_ctx, &key->data[key->length / 2]);
  xts_decrypt_message(ctx, twk_ctx, cipher->decrypt, cipher->encrypt,
                      tweak->data, length, data2, data);
  test_check_data("decrypt", data, data2, cleartext->data, length);

  memcpy(data, cleartext->data, length);

  cipher->set_encrypt_key(ctx, key->data);
  cipher->set_encrypt_key(twk_ctx, &key->data[key->length / 2]);
  xts_encrypt_message(ctx, twk_ctx, cipher->encrypt,
                      tweak->data, length, data, data);
  test_check_data("inplace encrypt",
                  cleartext->data, data, ciphertext->data, length);

  cipher->set_decrypt_key(ctx, key->data);
  cipher->set_encrypt_key(twk_ctx, &key->data[key->length / 2]);
  xts_decrypt_message(ctx, twk_ctx, cipher->decrypt, cipher->encrypt,
                      tweak->data, length, data, data);
  test_check_data("inplace decrypt", data, data, cleartext->data, length);

  /* make sure AES128 specific functions also works the same */
  if (cipher == &nettle_aes128) {
    struct xts_aes128_key xts_key;

    xts_aes128_set_encrypt_key(&xts_key, key->data);
    xts_aes128_encrypt_message(&xts_key, tweak->data, length, data,
                               cleartext->data);
    test_check_data("encrypt",
                    cleartext->data, data, ciphertext->data, length);

    xts_aes128_set_decrypt_key(&xts_key, key->data);
    xts_aes128_decrypt_message(&xts_key, tweak->data, length, data,
                               ciphertext->data);
    test_check_data("decrypt",
                    ciphertext->data, data, cleartext->data, length);
  }

  /* make sure AES256 specific functions also works the same */
  if (cipher == &nettle_aes256) {
    struct xts_aes256_key xts_key;

    xts_aes256_set_encrypt_key(&xts_key, key->data);
    xts_aes256_encrypt_message(&xts_key, tweak->data, length, data,
                               cleartext->data);
    test_check_data("encrypt",
                    cleartext->data, data, ciphertext->data, length);

    xts_aes256_set_decrypt_key(&xts_key, key->data);
    xts_aes256_decrypt_message(&xts_key, tweak->data, length, data,
                               ciphertext->data);
    test_check_data("decrypt",
                    ciphertext->data, data, cleartext->data, length);
  }

  free(twk_ctx);
  free(ctx);
  free(data);
  free(data2);
}

void
test_main(void)
{
  /* From NIST CAVS 11.0,
   *
   * https://csrc.nist.gov/CSRC/media/Projects/Cryptographic-Algorithm-Validation-Program/documents/aes/XTSTestVectors.zip
   *
   * Selection of testing vectors from the above CAVS set
   */

  /* AES-128 single block - exact block size multiple */
  test_cipher_xts(&nettle_aes128,
		  SHEX("a1b90cba3f06ac353b2c343876081762"
                       "090923026e91771815f29dab01932f2f"),
		  SHEX("4faef7117cda59c66e4b92013e768ad5"),
		  SHEX("ebabce95b14d3c8d6fb350390790311c"),
		  SHEX("778ae8b43cb98d5a825081d5be471c63"));

  /* AES-128 two blocks - exact block size multiple */
  test_cipher_xts(&nettle_aes128,
		  SHEX("750372c3d82f63382867be6662acfa4a"
                       "259be3fa9bc662a1154ffaaed8b448a5"),
		  SHEX("93a29254c47e4260669621307d4f5cd3"),
		  SHEX("d8e3a56559a436ce0d8b212c80a88b23"
                       "af62b0e598f208e03c1f2e9fa563a54b"),
		  SHEX("495f7855535efd133464dc9a9abf8a0f"
                       "28facbce21bd3c22178ec489b799e491"));

  /* AES-128 partial second block */
  test_cipher_xts(&nettle_aes128,
                  SHEX("394c97881abd989d29c703e48a72b397"
                       "a7acf51b59649eeea9b33274d8541df4"),
                  SHEX("4b15c684a152d485fe9937d39b168c29"),
                  SHEX("2f3b9dcfbae729583b1d1ffdd16bb6fe"
                       "2757329435662a78f0"),
                  SHEX("f3473802e38a3ffef4d4fb8e6aa266eb"
                       "de553a64528a06463e"));

  /* AES-256 two blocks - exact block size multiple */
  test_cipher_xts(&nettle_aes256,
		  SHEX("1ea661c58d943a0e4801e42f4b094714"
                       "9e7f9f8e3e68d0c7505210bd311a0e7c"
                       "d6e13ffdf2418d8d1911c004cda58da3"
                       "d619b7e2b9141e58318eea392cf41b08"),
		  SHEX("adf8d92627464ad2f0428e84a9f87564"),
		  SHEX("2eedea52cd8215e1acc647e810bbc364"
                       "2e87287f8d2e57e36c0a24fbc12a202e"),
		  SHEX("cbaad0e2f6cea3f50b37f934d46a9b13"
                       "0b9d54f07e34f36af793e86f73c6d7db"));

  /* AES-256 three blocks - exact block size multiple */
  test_cipher_xts(&nettle_aes256,
		  SHEX("266c336b3b01489f3267f52835fd92f6"
                       "74374b88b4e1ebd2d36a5f457581d9d0"
                       "42c3eef7b0b7e5137b086496b4d9e6ac"
                       "658d7196a23f23f036172fdb8faee527"),
		  SHEX("06b209a7a22f486ecbfadb0f3137ba42"),
		  SHEX("ca7d65ef8d3dfad345b61ccddca1ad81"
                       "de830b9e86c7b426d76cb7db766852d9"
                       "81c6b21409399d78f42cc0b33a7bbb06"),
		  SHEX("c73256870cc2f4dd57acc74b5456dbd7"
                       "76912a128bc1f77d72cdebbf270044b7"
                       "a43ceed29025e1e8be211fa3c3ed002d"));
}
