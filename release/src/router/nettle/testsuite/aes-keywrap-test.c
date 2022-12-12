#include "testutils.h"
#include "nist-keywrap.h"

typedef void nettle_nist_keywrap_func(const void *ctx,
		  const uint8_t * iv, size_t cleartext_length,
		  uint8_t * cleartext, const uint8_t * ciphertext);

typedef int nettle_nist_keyunwrap_func(const void *ctx,
		  const uint8_t * iv, size_t cleartext_length,
		  uint8_t * cleartext, const uint8_t * ciphertext);


struct nettle_wrap
{
  void *ctx;
  nettle_set_key_func *set_encrypt_key;
  nettle_cipher_func *encrypt;
  nettle_set_key_func *set_decrypt_key;
  nettle_cipher_func *decrypt;
};

struct nettle_specific_wrap
{
  void *ctx;
  nettle_set_key_func *set_encrypt_key;
  nettle_nist_keywrap_func *keywrap_func;
  nettle_set_key_func *set_decrypt_key;
  nettle_nist_keyunwrap_func *keyunwrap_func;
};

static void
test_wrap (struct nettle_wrap *w,
	   const struct tstring *key,
	   const struct tstring *iv,
	   const struct tstring *cleartext, const struct tstring *ciphertext)
{
  uint8_t data[40];
  w->set_encrypt_key (w->ctx, key->data);
  nist_keywrap16 (w->ctx, w->encrypt,
		  iv->data, cleartext->length + 8, data, cleartext->data);
  if (!MEMEQ (ciphertext->length, data, ciphertext->data))
    {
      fprintf (stderr, "test_wrap: Wrap failed:");
      fprintf (stderr, "\nOutput: ");
      print_hex (ciphertext->length, data);
      fprintf (stderr, "\nExpected:");
      tstring_print_hex (ciphertext);
      fprintf (stderr, "\n");
      FAIL ();
    }
}

static void
test_unwrap (struct nettle_wrap *w,
	     const struct tstring *key,
	     const struct tstring *iv,
	     const struct tstring *ciphertext,
	     const struct tstring *cleartext)
{
  uint8_t data[32];
  w->set_decrypt_key (w->ctx, key->data);
  nist_keyunwrap16 (w->ctx, w->decrypt,
		    iv->data, cleartext->length, data, ciphertext->data);
  if (!MEMEQ (cleartext->length, data, cleartext->data))
    {
      fprintf (stderr, "test_unwrap: Wrap failed:");
      fprintf (stderr, "\nOutput: ");
      print_hex (cleartext->length, data);
      fprintf (stderr, "\nExpected:");
      tstring_print_hex (cleartext);
      fprintf (stderr, "\n");
      FAIL ();
    }
}

static void
test_unwrap_fail (struct nettle_wrap *w,
	     const struct tstring *key,
	     const struct tstring *iv,
	     const struct tstring *ciphertext)
{
  uint8_t data[32];
  w->set_decrypt_key (w->ctx, key->data);
  if (nist_keyunwrap16 (w->ctx, w->decrypt,
		    iv->data, ciphertext->length-8, data, ciphertext->data))
    {
      FAIL ();
    }
}

static void
test_specific_wrap (struct nettle_specific_wrap *w,
		    const struct tstring *key,
		    const struct tstring *iv,
		    const struct tstring *cleartext,
		    const struct tstring *ciphertext)
{
  uint8_t data[40];
  w->set_encrypt_key (w->ctx, key->data);
  w->keywrap_func (w->ctx,
		   iv->data, cleartext->length + 8, data, cleartext->data);
  if (!MEMEQ (ciphertext->length, data, ciphertext->data))
    {
      fprintf (stderr, "test_specific_wrap: Wrap failed:");
      fprintf (stderr, "\nOutput: ");
      print_hex (ciphertext->length, data);
      fprintf (stderr, "\nExpected:");
      tstring_print_hex (ciphertext);
      fprintf (stderr, "\n");
      FAIL ();
    }
}

static void
test_specific_unwrap (struct nettle_specific_wrap *w,
		      const struct tstring *key,
		      const struct tstring *iv,
		      const struct tstring *ciphertext,
		      const struct tstring *cleartext)
{
  uint8_t data[32];
  w->set_decrypt_key (w->ctx, key->data);
  w->keyunwrap_func (w->ctx,
		     iv->data, cleartext->length, data, ciphertext->data);
  if (!MEMEQ (cleartext->length, data, cleartext->data))
    {
      fprintf (stderr, "test_unwrap: Wrap failed:");
      fprintf (stderr, "\nOutput: ");
      print_hex (cleartext->length, data);
      fprintf (stderr, "\nExpected:");
      tstring_print_hex (cleartext);
      fprintf (stderr, "\n");
      FAIL ();
    }
}

static void
test_specific_unwrap_fail (struct nettle_specific_wrap *w,
		      const struct tstring *key,
		      const struct tstring *iv,
		      const struct tstring *ciphertext)
{
  uint8_t data[32];
  w->set_decrypt_key (w->ctx, key->data);
  if (w->keyunwrap_func (w->ctx,
		     iv->data, ciphertext->length-8, data, ciphertext->data))
    {
      FAIL ();
    }
}

void
test_main (void)
{
  struct aes128_ctx ctx_128;
  struct aes192_ctx ctx_192;
  struct aes256_ctx ctx_256;
  struct nettle_wrap wrap128, wrap192, wrap256;
  struct nettle_wrap unwrap128, unwrap192, unwrap256;
  struct nettle_specific_wrap swrap128, swrap192, swrap256;
  struct nettle_specific_wrap sunwrap128, sunwrap192, sunwrap256;

  wrap128.ctx = &ctx_128;
  wrap128.set_encrypt_key = (nettle_set_key_func *) & aes128_set_encrypt_key;
  wrap128.encrypt = (nettle_cipher_func *) & aes128_encrypt;

  unwrap128.ctx = &ctx_128;
  unwrap128.set_decrypt_key =
    (nettle_set_key_func *) & aes128_set_decrypt_key;
  unwrap128.decrypt = (nettle_cipher_func *) & aes128_decrypt;

  wrap192.ctx = &ctx_192;
  wrap192.set_encrypt_key = (nettle_set_key_func *) & aes192_set_encrypt_key;
  wrap192.encrypt = (nettle_cipher_func *) & aes192_encrypt;

  unwrap192.ctx = &ctx_192;
  unwrap192.set_decrypt_key =
    (nettle_set_key_func *) & aes192_set_decrypt_key;
  unwrap192.decrypt = (nettle_cipher_func *) & aes192_decrypt;

  wrap256.ctx = &ctx_256;
  wrap256.set_encrypt_key = (nettle_set_key_func *) & aes256_set_encrypt_key;
  wrap256.encrypt = (nettle_cipher_func *) & aes256_encrypt;

  unwrap256.ctx = &ctx_256;
  unwrap256.set_decrypt_key =
    (nettle_set_key_func *) & aes256_set_decrypt_key;
  unwrap256.decrypt = (nettle_cipher_func *) & aes256_decrypt;

  swrap128.ctx = &ctx_128;
  swrap128.set_encrypt_key = (nettle_set_key_func *) & aes128_set_encrypt_key;
  swrap128.keywrap_func = (nettle_nist_keywrap_func *) & aes128_keywrap;

  swrap192.ctx = &ctx_192;
  swrap192.set_encrypt_key = (nettle_set_key_func *) & aes192_set_encrypt_key;
  swrap192.keywrap_func = (nettle_nist_keywrap_func *) & aes192_keywrap;

  swrap256.ctx = &ctx_256;
  swrap256.set_encrypt_key = (nettle_set_key_func *) & aes256_set_encrypt_key;
  swrap256.keywrap_func = (nettle_nist_keywrap_func *) & aes256_keywrap;

  sunwrap128.ctx = &ctx_128;
  sunwrap128.set_decrypt_key =
    (nettle_set_key_func *) & aes128_set_decrypt_key;
  sunwrap128.keyunwrap_func =
    (nettle_nist_keyunwrap_func *) & aes128_keyunwrap;

  sunwrap192.ctx = &ctx_192;
  sunwrap192.set_decrypt_key =
    (nettle_set_key_func *) & aes192_set_decrypt_key;
  sunwrap192.keyunwrap_func =
    (nettle_nist_keyunwrap_func *) & aes192_keyunwrap;

  sunwrap256.ctx = &ctx_256;
  sunwrap256.set_decrypt_key =
    (nettle_set_key_func *) & aes256_set_decrypt_key;
  sunwrap256.keyunwrap_func =
    (nettle_nist_keyunwrap_func *) & aes256_keyunwrap;

  test_wrap (&wrap128,
	     SHEX ("0001020304050607 08090A0B0C0D0E0F"),
	     SHEX ("A6A6A6A6A6A6A6A6"),
	     SHEX ("0011223344556677 8899AABBCCDDEEFF"),
	     SHEX ("1FA68B0A8112B447 AEF34BD8FB5A7B82 9D3E862371D2CFE5"));

  test_unwrap (&unwrap128,
	       SHEX ("0001020304050607 08090A0B0C0D0E0F"),
	       SHEX ("A6A6A6A6A6A6A6A6"),
	       SHEX ("1FA68B0A8112B447 AEF34BD8FB5A7B82 9D3E862371D2CFE5"),
	       SHEX ("0011223344556677 8899AABBCCDDEEFF"));

  test_unwrap_fail (&unwrap128,
	       SHEX ("0001020304050607 08090A0B0C0D0E0F"),
	       SHEX ("A6A6A6A6A6A6A6A6"),
	       SHEX ("1EA68B0A8112B447 AEF34BD8FB5A7B82 9D3E862371D2CFE5"));

  test_wrap (&wrap192,
	     SHEX ("0001020304050607 08090A0B0C0D0E0F 1011121314151617"),
	     SHEX ("A6A6A6A6A6A6A6A6"),
	     SHEX ("0011223344556677 8899AABBCCDDEEFF"),
	     SHEX ("96778B25AE6CA435 F92B5B97C050AED2 468AB8A17AD84E5D"));

  test_unwrap (&unwrap192,
	       SHEX ("0001020304050607 08090A0B0C0D0E0F 1011121314151617"),
	       SHEX ("A6A6A6A6A6A6A6A6"),
	       SHEX ("96778B25AE6CA435 F92B5B97C050AED2 468AB8A17AD84E5D"),
	       SHEX ("0011223344556677 8899AABBCCDDEEFF"));

  test_unwrap_fail (&unwrap192,
	       SHEX ("0001020304050607 08090A0B0C0D0E0F 1011121314151617"),
	       SHEX ("A6A6A6A6A6A6A6A6"),
	       SHEX ("96778B25AE6CA435 F92B5B97C050AED2 468AB8A17AD84E5E"));

  test_wrap (&wrap256,
	     SHEX
	     ("0001020304050607 08090A0B0C0D0E0F 1011121314151617 18191A1B1C1D1E1F"),
	     SHEX ("A6A6A6A6A6A6A6A6"),
	     SHEX ("0011223344556677 8899AABBCCDDEEFF"),
	     SHEX ("64E8C3F9CE0F5BA2 63E9777905818A2A 93C8191E7D6E8AE7"));

  test_unwrap (&unwrap256,
	       SHEX
	       ("0001020304050607 08090A0B0C0D0E0F 1011121314151617 18191A1B1C1D1E1F"),
	       SHEX ("A6A6A6A6A6A6A6A6"),
	       SHEX ("64E8C3F9CE0F5BA2 63E9777905818A2A 93C8191E7D6E8AE7"),
	       SHEX ("0011223344556677 8899AABBCCDDEEFF"));

  test_unwrap_fail (&unwrap256,
	       SHEX
	       ("0001020304050607 08090A0B0C0D0E0F 1011121314151617 18191A1B1C1D1E1F"),
	       SHEX ("A6A6A6A6A6A6A6A6"),
	       SHEX ("64E8C3F9CE0F5BA2 63E9777905818A2A 93C8191E7D6E8AE6"));

  test_wrap (&wrap192,
	     SHEX ("0001020304050607 08090A0B0C0D0E0F 1011121314151617"),
	     SHEX ("A6A6A6A6A6A6A6A6"),
	     SHEX ("0011223344556677 8899AABBCCDDEEFF 0001020304050607"),
	     SHEX
	     ("031D33264E15D332 68F24EC260743EDC E1C6C7DDEE725A93 6BA814915C6762D2"));

  test_unwrap (&unwrap192,
	       SHEX ("0001020304050607 08090A0B0C0D0E0F 1011121314151617"),
	       SHEX ("A6A6A6A6A6A6A6A6"),
	       SHEX
	       ("031D33264E15D332 68F24EC260743EDC E1C6C7DDEE725A93 6BA814915C6762D2"),
	       SHEX ("0011223344556677 8899AABBCCDDEEFF 0001020304050607"));

  test_unwrap_fail (&unwrap192,
	       SHEX ("0001020304050607 08090A0B0C0D0E0F 1011121314151617"),
	       SHEX ("A6A6A6A6A6A6A6A6"),
	       SHEX
	       ("031D33264E15D332 68F24EC260743EDC E1C6C7DDEE725B93 6BA814915C6762D2"));

  test_wrap (&wrap256,
	     SHEX
	     ("0001020304050607 08090A0B0C0D0E0F 1011121314151617 18191A1B1C1D1E1F"),
	     SHEX ("A6A6A6A6A6A6A6A6"),
	     SHEX ("0011223344556677 8899AABBCCDDEEFF 0001020304050607"),
	     SHEX
	     ("A8F9BC1612C68B3F F6E6F4FBE30E71E4 769C8B80A32CB895 8CD5D17D6B254DA1"));

  test_unwrap (&unwrap256,
	       SHEX
	       ("0001020304050607 08090A0B0C0D0E0F 1011121314151617 18191A1B1C1D1E1F"),
	       SHEX ("A6A6A6A6A6A6A6A6"),
	       SHEX
	       ("A8F9BC1612C68B3F F6E6F4FBE30E71E4 769C8B80A32CB895 8CD5D17D6B254DA1"),
	       SHEX ("0011223344556677 8899AABBCCDDEEFF 0001020304050607"));

  test_unwrap_fail (&unwrap256,
	       SHEX
	       ("0001020304050607 08090A0B0C0D0E0F 1011121314151617 18191A1B1C1D1E1F"),
	       SHEX ("A6A6A6A6A6A6A6A6"),
	       SHEX
	       ("A8F9BC1612C68B3F F6E6F4FBE30E71E5 769C8B80A32CB895 8CD5D17D6B254DA1"));

  test_wrap (&wrap256,
	     SHEX
	     ("0001020304050607 08090A0B0C0D0E0F 1011121314151617 18191A1B1C1D1E1F"),
	     SHEX ("A6A6A6A6A6A6A6A6"),
	     SHEX
	     ("0011223344556677 8899AABBCCDDEEFF 0001020304050607 08090A0B0C0D0E0F"),
	     SHEX
	     ("28C9F404C4B810F4 CBCCB35CFB87F826 3F5786E2D80ED326 CBC7F0E71A99F43B FB988B9B7A02DD21"));

  test_unwrap (&unwrap256,
	       SHEX
	       ("0001020304050607 08090A0B0C0D0E0F 1011121314151617 18191A1B1C1D1E1F"),
	       SHEX ("A6A6A6A6A6A6A6A6"),
	       SHEX
	       ("28C9F404C4B810F4 CBCCB35CFB87F826 3F5786E2D80ED326 CBC7F0E71A99F43B FB988B9B7A02DD21"),
	       SHEX
	       ("0011223344556677 8899AABBCCDDEEFF 0001020304050607 08090A0B0C0D0E0F"));

  test_unwrap_fail (&unwrap256,
	       SHEX
	       ("0001020304050607 08090A0B0C0D0E0F 1011121314151617 18191A1B1C1D1E1F"),
	       SHEX ("A6A6A6A6A6A6A6A6"),
	       SHEX
	       ("28C9F404C4B810F4 CBCCB35CFB87F816 3F5786E2D80ED326 CBC7F0E71A99F43B FB988B9B7A02DD21"));

  test_specific_wrap (&swrap128,
		      SHEX ("0001020304050607 08090A0B0C0D0E0F"),
		      SHEX ("A6A6A6A6A6A6A6A6"),
		      SHEX ("0011223344556677 8899AABBCCDDEEFF"),
		      SHEX
		      ("1FA68B0A8112B447 AEF34BD8FB5A7B82 9D3E862371D2CFE5"));

  test_specific_unwrap (&sunwrap128,
			SHEX ("0001020304050607 08090A0B0C0D0E0F"),
			SHEX ("A6A6A6A6A6A6A6A6"),
			SHEX
			("1FA68B0A8112B447 AEF34BD8FB5A7B82 9D3E862371D2CFE5"),
			SHEX ("0011223344556677 8899AABBCCDDEEFF"));

  test_specific_unwrap_fail (&sunwrap128,
			SHEX ("0001020304050607 08090A0B0C0D0E0F"),
			SHEX ("A6A6A6A6A6A6A6A6"),
			SHEX
			("1FA68B0A8112B446 AEF34BD8FB5A7B82 9D3E862371D2CFE5"));

  test_specific_wrap (&swrap192,
		      SHEX
		      ("0001020304050607 08090A0B0C0D0E0F 1011121314151617"),
		      SHEX ("A6A6A6A6A6A6A6A6"),
		      SHEX ("0011223344556677 8899AABBCCDDEEFF"),
		      SHEX
		      ("96778B25AE6CA435 F92B5B97C050AED2 468AB8A17AD84E5D"));

  test_specific_unwrap (&sunwrap192,
			SHEX
			("0001020304050607 08090A0B0C0D0E0F 1011121314151617"),
			SHEX ("A6A6A6A6A6A6A6A6"),
			SHEX
			("96778B25AE6CA435 F92B5B97C050AED2 468AB8A17AD84E5D"),
			SHEX ("0011223344556677 8899AABBCCDDEEFF"));

  test_specific_unwrap_fail (&sunwrap192,
			SHEX
			("0001020304050607 08090A0B0C0D0E0F 1011121314151617"),
			SHEX ("A6A6A6A6A6A6A6A6"),
			SHEX
			("96778B25AE6CA435 F92B5B97C050AED2 478AB8A17AD84E5D"));

  test_specific_wrap (&swrap256,
		      SHEX
		      ("0001020304050607 08090A0B0C0D0E0F 1011121314151617 18191A1B1C1D1E1F"),
		      SHEX ("A6A6A6A6A6A6A6A6"),
		      SHEX ("0011223344556677 8899AABBCCDDEEFF"),
		      SHEX
		      ("64E8C3F9CE0F5BA2 63E9777905818A2A 93C8191E7D6E8AE7"));

  test_specific_unwrap (&sunwrap256,
			SHEX
			("0001020304050607 08090A0B0C0D0E0F 1011121314151617 18191A1B1C1D1E1F"),
			SHEX ("A6A6A6A6A6A6A6A6"),
			SHEX
			("64E8C3F9CE0F5BA2 63E9777905818A2A 93C8191E7D6E8AE7"),
			SHEX ("0011223344556677 8899AABBCCDDEEFF"));

  test_specific_unwrap_fail (&sunwrap256,
			SHEX
			("0001020304050607 08090A0B0C0D0E0F 1011121314151617 18191A1B1C1D1E1F"),
			SHEX ("A6A6A6A6A6A6A6A6"),
			SHEX
			("64E8C3F9CE0F5BA2 63E9777905818A2A 93C8101E7D6E8AE7"));

  test_specific_wrap (&swrap192,
		      SHEX
		      ("0001020304050607 08090A0B0C0D0E0F 1011121314151617"),
		      SHEX ("A6A6A6A6A6A6A6A6"),
		      SHEX
		      ("0011223344556677 8899AABBCCDDEEFF 0001020304050607"),
		      SHEX
		      ("031D33264E15D332 68F24EC260743EDC E1C6C7DDEE725A93 6BA814915C6762D2"));

  test_specific_unwrap (&sunwrap192,
			SHEX
			("0001020304050607 08090A0B0C0D0E0F 1011121314151617"),
			SHEX ("A6A6A6A6A6A6A6A6"),
			SHEX
			("031D33264E15D332 68F24EC260743EDC E1C6C7DDEE725A93 6BA814915C6762D2"),
			SHEX
			("0011223344556677 8899AABBCCDDEEFF 0001020304050607"));

  test_specific_unwrap_fail (&sunwrap192,
			SHEX
			("0001020304050607 08090A0B0C0D0E0F 1011121314151617"),
			SHEX ("A6A6A6A6A6A6A6A6"),
			SHEX
			("031D33264E15D332 68F24EC260743EDC E1C6C7DDEF725A93 6BA814915C6762D2"));

  test_specific_wrap (&swrap256,
		      SHEX
		      ("0001020304050607 08090A0B0C0D0E0F 1011121314151617 18191A1B1C1D1E1F"),
		      SHEX ("A6A6A6A6A6A6A6A6"),
		      SHEX
		      ("0011223344556677 8899AABBCCDDEEFF 0001020304050607"),
		      SHEX
		      ("A8F9BC1612C68B3F F6E6F4FBE30E71E4 769C8B80A32CB895 8CD5D17D6B254DA1"));

  test_specific_unwrap (&sunwrap256,
			SHEX
			("0001020304050607 08090A0B0C0D0E0F 1011121314151617 18191A1B1C1D1E1F"),
			SHEX ("A6A6A6A6A6A6A6A6"),
			SHEX
			("A8F9BC1612C68B3F F6E6F4FBE30E71E4 769C8B80A32CB895 8CD5D17D6B254DA1"),
			SHEX
			("0011223344556677 8899AABBCCDDEEFF 0001020304050607"));

  test_specific_unwrap_fail (&sunwrap256,
			SHEX
			("0001020304050607 08090A0B0C0D0E0F 1011121314151617 18191A1B1C1D1E1F"),
			SHEX ("A6A6A6A6A6A6A6A6"),
			SHEX
			("A8F9BC1612C68C3F F6E6F4FBE30E71E4 769C8B80A32CB895 8CD5D17D6B254DA1"));

  test_specific_wrap (&swrap256,
		      SHEX
		      ("0001020304050607 08090A0B0C0D0E0F 1011121314151617 18191A1B1C1D1E1F"),
		      SHEX ("A6A6A6A6A6A6A6A6"),
		      SHEX
		      ("0011223344556677 8899AABBCCDDEEFF 0001020304050607 08090A0B0C0D0E0F"),
		      SHEX
		      ("28C9F404C4B810F4 CBCCB35CFB87F826 3F5786E2D80ED326 CBC7F0E71A99F43B FB988B9B7A02DD21"));

  test_specific_unwrap (&sunwrap256,
			SHEX
			("0001020304050607 08090A0B0C0D0E0F 1011121314151617 18191A1B1C1D1E1F"),
			SHEX ("A6A6A6A6A6A6A6A6"),
			SHEX
			("28C9F404C4B810F4 CBCCB35CFB87F826 3F5786E2D80ED326 CBC7F0E71A99F43B FB988B9B7A02DD21"),
			SHEX
			("0011223344556677 8899AABBCCDDEEFF 0001020304050607 08090A0B0C0D0E0F"));

  test_specific_unwrap_fail (&sunwrap256,
			SHEX
			("0001020304050607 08090A0B0C0D0E0F 1011121314151617 18191A1B1C1D1E1F"),
			SHEX ("A6A6A6A6A6A6A6A6"),
			SHEX
			("28C9F404C4B810F4 CBCCB35CFB87F826 3F5786E2D80ED426 CBC7F0E71A99F43B FB988B9B7A02DD21"));
}
