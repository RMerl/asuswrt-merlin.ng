#include "testutils.h"
#include "non-nettle.h"

struct ocb_aes128_message_key
{
  struct ocb_aes128_encrypt_key encrypt_key;
  struct aes128_ctx decrypt_key;
};

static void
ocb_aes128_set_encrypt_key_wrapper (struct ocb_aes128_message_key *key,
				    const uint8_t *aes_key)
{
  ocb_aes128_set_encrypt_key (&key->encrypt_key, aes_key);
}
static void
ocb_aes128_set_decrypt_key_wrapper (struct ocb_aes128_message_key *key,
				    const uint8_t *aes_key)
{
  ocb_aes128_set_decrypt_key (&key->encrypt_key, &key->decrypt_key, aes_key);
}
static void
ocb_aes128_encrypt_message_wrapper (const struct ocb_aes128_message_key *key,
				    size_t nlength, const uint8_t *nonce,
				    size_t alength, const uint8_t *adata,
				    size_t clength, uint8_t *dst, const uint8_t *src)
{
  ocb_aes128_encrypt_message (&key->encrypt_key, nlength, nonce, alength, adata,
			      OCB_DIGEST_SIZE, clength, dst, src);
}
static int
ocb_aes128_decrypt_message_wrapper (const struct ocb_aes128_message_key *key,
				    size_t nlength, const uint8_t *nonce,
				    size_t alength, const uint8_t *adata,
				    size_t mlength, uint8_t *dst, const uint8_t *src)
{
  return ocb_aes128_decrypt_message (&key->encrypt_key, &key->decrypt_key,
				     nlength, nonce, alength, adata,
				     OCB_DIGEST_SIZE, mlength, dst, src);
}

static const struct nettle_aead_message
ocb_aes128_message = {
  "ocb_aes128",
  sizeof(struct ocb_aes128_message_key),
  AES128_KEY_SIZE,
  OCB_DIGEST_SIZE,
  1, /* Supports in-place operation. */
  (nettle_set_key_func*) ocb_aes128_set_encrypt_key_wrapper,
  (nettle_set_key_func*) ocb_aes128_set_decrypt_key_wrapper,
  (nettle_encrypt_message_func*) ocb_aes128_encrypt_message_wrapper,
  (nettle_decrypt_message_func*) ocb_aes128_decrypt_message_wrapper,
};

/* For 96-bit tag */
static void
set_nonce_tag96 (struct ocb_aes128_ctx *ctx, size_t length, const uint8_t *nonce)
{
  ASSERT (length == OCB_NONCE_SIZE);
  ocb_aes128_set_nonce (&ctx->ocb, &ctx->key,
			12, OCB_NONCE_SIZE, nonce);
}

void
test_main(void)
{
  /* From RFC 7253 */
  test_aead(&nettle_ocb_aes128, NULL,
	    SHEX("000102030405060708090A0B0C0D0E0F"), /* key */
	    SHEX(""), /* auth data */
	    SHEX(""), /* plaintext */
	    SHEX(""), /* ciphertext */
	    SHEX("BBAA99887766554433221100"), /* nonce */
	    SHEX("785407BFFFC8AD9EDCC5520AC9111EE6")); /* tag */

  test_aead(&nettle_ocb_aes128, NULL,
	    SHEX("000102030405060708090A0B0C0D0E0F"), /* key */
	    SHEX("0001020304050607"), /* auth data */
	    SHEX("0001020304050607"), /* plaintext */
	    SHEX("6820B3657B6F615A"), /* ciphertext */
	    SHEX("BBAA99887766554433221101"), /* nonce */
	    SHEX("5725BDA0D3B4EB3A257C9AF1F8F03009")); /* tag */

  test_aead(&nettle_ocb_aes128, NULL,
	    SHEX("000102030405060708090A0B0C0D0E0F"), /* key */
	    SHEX("0001020304050607"), /* auth data */
	    SHEX(""), /* plaintext */
	    SHEX(""), /* ciphertext */
	    SHEX("BBAA99887766554433221102"), /* nonce */
	    SHEX("81017F8203F081277152FADE694A0A00")); /* tag */

  test_aead(&nettle_ocb_aes128, NULL,
	    SHEX("000102030405060708090A0B0C0D0E0F"), /* key */
	    SHEX(""), /* auth data */
	    SHEX("0001020304050607"), /* plaintext */
	    SHEX("45DD69F8F5AAE724"), /* ciphertext */
	    SHEX("BBAA99887766554433221103"), /* nonce */
	    SHEX("14054CD1F35D82760B2CD00D2F99BFA9")); /* tag */

  test_aead(&nettle_ocb_aes128, NULL,
	    SHEX("000102030405060708090A0B0C0D0E0F"), /* key */
	    SHEX("000102030405060708090A0B0C0D0E0F"), /* auth data */
	    SHEX("000102030405060708090A0B0C0D0E0F"), /* plaintext */
	    SHEX("571D535B60B277188BE5147170A9A22C"), /* ciphertext */
	    SHEX("BBAA99887766554433221104"), /* nonce */
	    SHEX("3AD7A4FF3835B8C5701C1CCEC8FC3358")); /* tag */

  test_aead(&nettle_ocb_aes128, NULL,
	    SHEX("000102030405060708090A0B0C0D0E0F"), /* key */
	    SHEX("000102030405060708090A0B0C0D0E0F"), /* auth data */
	    SHEX(""), /* plaintext */
	    SHEX(""), /* ciphertext */
	    SHEX("BBAA99887766554433221105"), /* nonce */
	    SHEX("8CF761B6902EF764462AD86498CA6B97")); /* tag */

  test_aead(&nettle_ocb_aes128, NULL,
	    SHEX("000102030405060708090A0B0C0D0E0F"), /* key */
	    SHEX(""), /* auth data */
	    SHEX("000102030405060708090A0B0C0D0E0F"), /* plaintext */
	    SHEX("5CE88EC2E0692706A915C00AEB8B2396"), /* ciphertext */
	    SHEX("BBAA99887766554433221106"), /* nonce */
	    SHEX("F40E1C743F52436BDF06D8FA1ECA343D")); /* tag */

  test_aead(&nettle_ocb_aes128, NULL,
	    SHEX("000102030405060708090A0B0C0D0E0F"), /* key */
	    SHEX("000102030405060708090A0B0C0D0E0F1011121314151617"), /* auth data */
	    SHEX("000102030405060708090A0B0C0D0E0F1011121314151617"), /* plaintext */
	    SHEX("1CA2207308C87C010756104D8840CE1952F09673A448A122"), /* ciphertext */
	    SHEX("BBAA99887766554433221107"), /* nonce */
	    SHEX("C92C62241051F57356D7F3C90BB0E07F")); /* tag */

  test_aead(&nettle_ocb_aes128, NULL,
	    SHEX("000102030405060708090A0B0C0D0E0F"), /* key */
	    SHEX("000102030405060708090A0B0C0D0E0F1011121314151617"), /* auth data */
	    SHEX(""), /* plaintext */
	    SHEX(""), /* ciphertext */
	    SHEX("BBAA99887766554433221108"), /* nonce */
	    SHEX("6DC225A071FC1B9F7C69F93B0F1E10DE")); /* tag */

  test_aead(&nettle_ocb_aes128, NULL,
	    SHEX("000102030405060708090A0B0C0D0E0F"), /* key */
	    SHEX(""), /* auth data */
	    SHEX("000102030405060708090A0B0C0D0E0F1011121314151617"), /* plaintext */
	    SHEX("221BD0DE7FA6FE993ECCD769460A0AF2D6CDED0C395B1C3C"), /* ciphertext */
	    SHEX("BBAA99887766554433221109"), /* nonce */
	    SHEX("E725F32494B9F914D85C0B1EB38357FF")); /* tag */

  test_aead(&nettle_ocb_aes128, NULL,
	    SHEX("000102030405060708090A0B0C0D0E0F"), /* key */
	    SHEX("000102030405060708090A0B0C0D0E0F1011121314151617"
		 "18191A1B1C1D1E1F"), /* auth data */
	    SHEX("000102030405060708090A0B0C0D0E0F1011121314151617"
		 "18191A1B1C1D1E1F"), /* plaintext */
	    SHEX("BD6F6C496201C69296C11EFD138A467ABD3C707924B964DE"
		 "AFFC40319AF5A485"), /* ciphertext */
	    SHEX("BBAA9988776655443322110A"), /* nonce */
	    SHEX("40FBBA186C5553C68AD9F592A79A4240")); /* tag */

  test_aead(&nettle_ocb_aes128, NULL,
	    SHEX("000102030405060708090A0B0C0D0E0F"), /* key */
	    SHEX("000102030405060708090A0B0C0D0E0F1011121314151617"
		 "18191A1B1C1D1E1F"), /* auth data */
	    SHEX(""), /* plaintext */
	    SHEX(""), /* ciphertext */
	    SHEX("BBAA9988776655443322110B"), /* nonce */
	    SHEX("FE80690BEE8A485D11F32965BC9D2A32")); /* tag */

  test_aead(&nettle_ocb_aes128, NULL,
	    SHEX("000102030405060708090A0B0C0D0E0F"), /* key */
	    SHEX(""), /* auth data */
	    SHEX("000102030405060708090A0B0C0D0E0F1011121314151617"
		 "18191A1B1C1D1E1F"), /* plaintext */
	    SHEX("2942BFC773BDA23CABC6ACFD9BFD5835BD300F0973792EF4"
		 "6040C53F1432BCDF"), /* ciphertext */
	    SHEX("BBAA9988776655443322110C"), /* nonce */
	    SHEX("B5E1DDE3BC18A5F840B52E653444D5DF")); /* tag */

  test_aead(&nettle_ocb_aes128, NULL,
	    SHEX("000102030405060708090A0B0C0D0E0F"), /* key */
	    SHEX("000102030405060708090A0B0C0D0E0F1011121314151617"
		 "18191A1B1C1D1E1F2021222324252627"), /* auth data */
	    SHEX("000102030405060708090A0B0C0D0E0F1011121314151617"
		 "18191A1B1C1D1E1F2021222324252627"), /* plaintext */
	    SHEX("D5CA91748410C1751FF8A2F618255B68A0A12E093FF45460"
		 "6E59F9C1D0DDC54B65E8628E568BAD7A"), /* ciphertext */
	    SHEX("BBAA9988776655443322110D"), /* nonce */
	    SHEX("ED07BA06A4A69483A7035490C5769E60")); /* tag */

  test_aead(&nettle_ocb_aes128, NULL,
	    SHEX("000102030405060708090A0B0C0D0E0F"), /* key */
	    SHEX("000102030405060708090A0B0C0D0E0F1011121314151617"
		 "18191A1B1C1D1E1F2021222324252627"), /* auth data */
	    SHEX(""), /* plaintext */
	    SHEX(""), /* ciphertext */
	    SHEX("BBAA9988776655443322110E"), /* nonce */
	    SHEX("C5CD9D1850C141E358649994EE701B68")); /* tag */

  test_aead(&nettle_ocb_aes128, NULL,
	    SHEX("000102030405060708090A0B0C0D0E0F"), /* key */
	    SHEX(""), /* auth data */
	    SHEX("000102030405060708090A0B0C0D0E0F1011121314151617"
		 "18191A1B1C1D1E1F2021222324252627"), /* plaintext */
	    SHEX("4412923493C57D5DE0D700F753CCE0D1D2D95060122E9F15"
		 "A5DDBFC5787E50B5CC55EE507BCB084E"), /* ciphertext */
	    SHEX("BBAA9988776655443322110F"), /* nonce */
	    SHEX("479AD363AC366B95 A98CA5F3000B1479")); /* tag */

  /* Test with 96-bit tag. */
  test_aead(&nettle_ocb_aes128, (nettle_hash_update_func *) set_nonce_tag96,
	    SHEX("0F0E0D0C0B0A09080706050403020100"), /* key */
	    SHEX("000102030405060708090A0B0C0D0E0F1011121314151617"
		 "18191A1B1C1D1E1F2021222324252627"), /* auth data */
	    SHEX("000102030405060708090A0B0C0D0E0F1011121314151617"
		 "18191A1B1C1D1E1F2021222324252627"), /* plaintext */
	    SHEX("1792A4E31E0755FB03E31B22116E6C2DDF9EFD6E33D536F1"
		 "A0124B0A55BAE884ED93481529C76B6A"), /* ciphertext */
	    SHEX("BBAA9988776655443322110D"), /* nonce */
	    SHEX("D0C515F4D1CDD4FDAC4F02AA")); /* tag */

  /* 16 blocks, not verified with other implementations or any
     authoritative test vector.not an authoritative test vector. */
  test_aead(&nettle_ocb_aes128, NULL,
	    SHEX("000102030405060708090A0B0C0D0E0F"), /* key */
	    SHEX(""), /* auth data */
	    SHEX("000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f"
		 "202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f"
		 "404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f"
		 "606162636465666768696a6b6c6d6e6f707172737475767778797a7b7c7d7e7f"
		 "808182838485868788898a8b8c8d8e8f909192939495969798999a9b9c9d9e9f"
		 "a0a1a2a3a4a5a6a7a8a9aaabacadaeafb0b1b2b3b4b5b6b7b8b9babbbcbdbebf"
		 "c0c1c2c3c4c5c6c7c8c9cacbcccdcecfd0d1d2d3d4d5d6d7d8d9dadbdcdddedf"
		 "e0e1e2e3e4e5e6e7e8e9eaebecedeeeff0f1f2f3f4f5f6f7f8f9fafbfcfdfeff"),
	    SHEX("4412923493c57d5d e0d700f753cce0d1"
		 "d2d95060122e9f15 a5ddbfc5787e50b5"
		 "11dfb888da244711 f051dbce82b0b9a7"
		 "cb14869b164e55eb 578e41fa435ff220"
		 "25ed114f6ec18cd6 7b743ab299e596f6"
		 "6100fba539db164d 765eaff0bf489ace"
		 "90ff6af96d1c395b 8dd586b154a0ecea"
		 "504395c5592cf2f0 03a3878585a0bfd3"
		 "b4039d15bc47a6d6 4a51f7302a976bb0"
		 "175167bcb5d8f071 a3faff70544ab2ba"
		 "52947d35d6e545e9 bda57b3972ecad10"
		 "f0e85aec389f4276 2e58978918d4c285"
		 "c2088ca8ac48095c 976065aa47766756"
		 "7a507bab08315b2e 36327e8103a6a70d"
		 "7f9f5318684697b2 bf95d65fa5458e6e"
		 "f40a974cb940e8fd 63baf0ce96773279"),
	    SHEX("BBAA9988776655443322110F"), /* nonce */
	    SHEX("3aa4f4e4b4ff142c 9357291589fa25d8")); /* tag */

  /* 16 complete blocks + left-over bytes, not verified with other
     implementations or any authoritative test vector. */
  test_aead(&nettle_ocb_aes128, NULL,
	    SHEX("000102030405060708090A0B0C0D0E0F"), /* key */
	    SHEX(""), /* auth data */
	    SHEX("000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f"
		 "202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f"
		 "404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f"
		 "606162636465666768696a6b6c6d6e6f707172737475767778797a7b7c7d7e7f"
		 "808182838485868788898a8b8c8d8e8f909192939495969798999a9b9c9d9e9f"
		 "a0a1a2a3a4a5a6a7a8a9aaabacadaeafb0b1b2b3b4b5b6b7b8b9babbbcbdbebf"
		 "c0c1c2c3c4c5c6c7c8c9cacbcccdcecfd0d1d2d3d4d5d6d7d8d9dadbdcdddedf"
		 "e0e1e2e3e4e5e6e7e8e9eaebecedeeeff0f1f2f3f4f5f6f7f8f9fafbfcfdfeff"
		 "deadbeaf"),
	    SHEX("4412923493c57d5d e0d700f753cce0d1"
		 "d2d95060122e9f15 a5ddbfc5787e50b5"
		 "11dfb888da244711 f051dbce82b0b9a7"
		 "cb14869b164e55eb 578e41fa435ff220"
		 "25ed114f6ec18cd6 7b743ab299e596f6"
		 "6100fba539db164d 765eaff0bf489ace"
		 "90ff6af96d1c395b 8dd586b154a0ecea"
		 "504395c5592cf2f0 03a3878585a0bfd3"
		 "b4039d15bc47a6d6 4a51f7302a976bb0"
		 "175167bcb5d8f071 a3faff70544ab2ba"
		 "52947d35d6e545e9 bda57b3972ecad10"
		 "f0e85aec389f4276 2e58978918d4c285"
		 "c2088ca8ac48095c 976065aa47766756"
		 "7a507bab08315b2e 36327e8103a6a70d"
		 "7f9f5318684697b2 bf95d65fa5458e6e"
		 "f40a974cb940e8fd 63baf0ce96773279"
		 "1dd97611"),
	    SHEX("BBAA9988776655443322110F"), /* nonce */
	    SHEX("8a24edb596b59425 43ec197d5369979b")); /* tag */

  /* Test the all-in-one message functions. */
  test_aead_message(&ocb_aes128_message,
	    SHEX("000102030405060708090A0B0C0D0E0F"), /* key */
	    SHEX("BBAA99887766554433221100"), /* nonce */
	    SHEX(""), /* auth data */
	    SHEX(""), /* plaintext */
	    SHEX("785407BFFFC8AD9EDCC5520AC9111EE6"));

  test_aead_message(&ocb_aes128_message,
	    SHEX("000102030405060708090A0B0C0D0E0F"), /* key */
	    SHEX("BBAA99887766554433221101"), /* nonce */
	    SHEX("0001020304050607"), /* auth data */
	    SHEX("0001020304050607"), /* plaintext */
	    SHEX("6820B3657B6F615A5725BDA0D3B4EB3A257C9AF1F8F03009")); /* ciphertext */

  /* Test-vector from libgcrypt:tests/basic.c: */
  test_aead(&nettle_ocb_aes128, (nettle_hash_update_func *) set_nonce_tag96,
	   SHEX("0F0E0D0C0B0A09080706050403020100"), /* key */
	   SHEX("000102030405060708090A0B0C0D0E0F1011121314151617"
		"18191A1B1C1D1E1F2021222324252627"), /* auth data */
	   /* test vector for checksumming */
	   SHEX("01000000000000000000000000000000"
		"02000000000000000000000000000000"
		"04000000000000000000000000000000"
		"08000000000000000000000000000000"
		"10000000000000000000000000000000"
		"20000000000000000000000000000000"
		"40000000000000000000000000000000"
		"80000000000000000000000000000000"
		"00010000000000000000000000000000"
		"00020000000000000000000000000000"
		"00040000000000000000000000000000"
		"00080000000000000000000000000000"
		"00100000000000000000000000000000"
		"00200000000000000000000000000000"
		"00400000000000000000000000000000"
		"00800000000000000000000000000000"
		"00000100000000000000000000000000"
		"00000200000000000000000000000000"
		"00000400000000000000000000000000"
		"00000800000000000000000000000000"
		"00001000000000000000000000000000"
		"00002000000000000000000000000000"
		"00004000000000000000000000000000"
		"00008000000000000000000000000000"
		"00000001000000000000000000000000"
		"00000002000000000000000000000000"
		"00000004000000000000000000000000"
		"00000008000000000000000000000000"
		"00000010000000000000000000000000"
		"00000020000000000000000000000000"
		"00000040000000000000000000000000"
		"00000080000000000000000000000000"
		"00000000010000000000000000000000"
		"00000000020000000000000000000000"
		"00000000040000000000000000000000"
		"00000000080000000000000000000000"
		"00000000100000000000000000000000"
		"00000000200000000000000000000000"
		"00000000400000000000000000000000"
		"00000000800000000000000000000000"
		"00000000000100000000000000000000"
		"00000000000200000000000000000000"
		"00000000000400000000000000000000"
		"00000000000800000000000000000000"
		"00000000001000000000000000000000"
		"00000000002000000000000000000000"
		"00000000004000000000000000000000"
		"00000000008000000000000000000000"), /* plaintext */
	   SHEX("01105c6e36f6ac480f022c51e31ed702"
		"90fda4b7b783194d4b4be8e4e1e2dff4"
		"6a0804d1c5f9f808ea7933e31c063233"
		"2bf65a22b20bb13cde3b80b3682ba965"
		"b1207c58916f7856fa9968b410e50dee"
		"98b35c071163d1b352b9bbccd09fde29"
		"b850f40e71a8ae7d2e2d577f5ee39c46"
		"7fa28130b50a123c29958e4665dda9a5"
		"e0793997f8f19633a96392141d6e0e88"
		"77850ed4364065d1d2f8746e2f1d5fd1"
		"996cdde03215306503a30e41f58ef3c4"
		"400365cfea4fa6381157c12a46598edf"
		"18604854462ec66e3d3cf26d4723cb6a"
		"9d801095048086a606fdb9192760889b"
		"a8ce2e70e1b55a469137a9e2e6734565"
		"283cb1e2c74f37e0854d03e33f8ba499"
		"ef5d9af4edfce077c6280338f0a64286"
		"2e6bc27ebd5a4c91b3778e22631251c8"
		"c5bb75a10945597a9d6c274fc82d3338"
		"b403a0a549d1375f26e71ef22bce0941"
		"93ea87e2ed72fce0546148c351eec3be"
		"867bb1b96070c377fff3c98e21562beb"
		"475cfe28abcaaedf49981f6599b15140"
		"ea6130d24407079f18ba9d4a8960b082"
		"b39c57320e2e064f02fde88c23112146"
		"1cac3655868aef584714826ee4f361fb"
		"e6d692e1589cbb9dd3c74fa628df2a1f"
		"3b0029b1d62b7e9978013ed3c793c1dd"
		"1f184c8f7022a853cac40b74ac749aa3"
		"f33f0d14732dfda0f2c3c20591bf1f5a"
		"710ec0d0bca342baa5146068a78ff58c"
		"66316312b7a98af35a0f4e92799b4047"
		"f047ae61f25c28d232ce5c168cc745d6"
		"6da13cb0f9e38a696635dba7a21571cf"
		"cd64ec8cc33db7879f59a90d9edd00f6"
		"a899e39ab36b9269a3ac04ebad9326bf"
		"53cd9b400168a61714cd628a4056d236"
		"bd8622c76daa54cb65f5db2fe03bafbe"
		"0b23549ae31136f607293e8093a21934"
		"74fd5e9c2451b4c8e0499e6ad34fafc8"
		"ab77722a282f7f84b14ddebf7e696300"
		"c1ef92d4a0263c6cca104530f996e272"
		"f58992ff68d642b071a5848dc4acf2ae"
		"28fb1f27ae0f297d5136a7a0a4a03e89"
		"b588755b8217a1c62773790e69261269"
		"19f45daf7b3ccf18e3fc590a9a0e172f"
		"033ac4d13c3decc4c62d7de718ace802"
		"140452dc850989f6762e3578bbb04be3"), /* ciphertext */
	   SHEX("BBAA9988776655443322110D"), /* nonce */
	   SHEX("1a237c599c4649f4e586b2de")); /* tag */
}
