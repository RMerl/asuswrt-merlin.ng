#include "testutils.h"
#include "gostdsa.h"
#include "streebog.h"

static void
test_vko (const struct ecc_curve *ecc,
	  const char *priv,
	  const char *x,
	  const char *y,
	  const struct tstring *ukm,
	  const struct nettle_hash *hash,
	  void * hash_ctx,
	  const struct tstring *res)
{
    struct ecc_scalar ecc_key;
    struct ecc_point ecc_pub;
    mpz_t temp1, temp2;
    uint8_t out[128];
    size_t out_len = ((ecc_bit_size(ecc) + 7) / 8) * 2;

    ASSERT(out_len <= sizeof(out));

    ecc_point_init (&ecc_pub, ecc);
    mpz_init_set_str (temp1, x, 16);
    mpz_init_set_str (temp2, y, 16);
    ASSERT (ecc_point_set (&ecc_pub, temp1, temp2) != 0);

    ecc_scalar_init (&ecc_key, ecc);
    mpz_set_str (temp1, priv, 16);
    ASSERT (ecc_scalar_set (&ecc_key, temp1) != 0);

    mpz_clear (temp1);
    mpz_clear (temp2);

    gostdsa_vko (&ecc_key, &ecc_pub,
		 ukm->length, ukm->data,
		 out);

    ecc_scalar_clear (&ecc_key);
    ecc_point_clear (&ecc_pub);

    if (hash)
      {
	hash->init (hash_ctx);
	hash->update (hash_ctx, out_len, out);
	hash->digest (hash_ctx, hash->digest_size, out);

	ASSERT (hash->digest_size == res->length);
	ASSERT (MEMEQ (res->length, out, res->data));
      }
    else
      {
	ASSERT (out_len == res->length);
	ASSERT (MEMEQ (res->length, out, res->data));
      }
}

void
test_main (void)
{
    struct streebog256_ctx ctx_256;
    struct streebog256_ctx ctx_512;

    /* RFC 7836, App B, provides test vectors, values there are little endian.
     *
     * However those test vectors depend on the availability of Streebog hash
     * functions, which is not available (yet). So these test vectors capture
     * the VKO value just before hash function. One can verify them by
     * calculating the Streeebog function and comparing the result with RFC
     * 7836, App B. */
    test_vko(nettle_get_gost_gc512a(),
	     "67b63ca4ac8d2bb32618d89296c7476dbeb9f9048496f202b1902cf2ce41dbc2f847712d960483458d4b380867f426c7ca0ff5782702dbc44ee8fc72d9ec90c9",
	     "51a6d54ee932d176e87591121cce5f395cb2f2f147114d95f463c8a7ed74a9fc5ecd2325a35fb6387831ea66bc3d2aa42ede35872cc75372073a71b983e12f19",
	     "793bde5bf72840ad22b02a363ae4772d4a52fc08ba1a20f7458a222a13bf98b53be002d1973f1e398ce46c17da6d00d9b6d0076f8284dcc42e599b4c413b8804",
	     SHEX("1d 80 60 3c 85 44 c7 27"),
	     NULL,
	     NULL,
	     SHEX("5fb5261b61e872f9 3efc03200f47378e f039aa89b993a274 a25dec5e5d49ed59"
		  "84b7dfdf5970c3f7 3059a26d08f7bbc5 0830799bda18b533 499c4f00c21cff3e"
		  "3b8e53a1ea920eb1 d7f3d08aa9e47595 4a53ac018c210b48 15451b7accc4a797"
		  "a2b8faf3d89ee717 d07a857794b9b053 f8e0fd5456ccfcc2 2fd081c873416a3f"));

    test_vko(nettle_get_gost_gc512a(),
	     "dbd09213a592da5bbfd8ed068cccccbbfbeda4feac96b9b4908591440b0714803b9eb763ef932266d4c0181a9b73eacf9013efc65ec07c888515f1b6f759c848",
	     "a7c0adb12743c10c3c1beb97c8f631242f7937a1deb6bce5e664e49261baccd3f5dc56ec53b2abb90ca1eb703078ba546655a8b99f79188d2021ffaba4edb0aa",
	     "5adb1c63a4e4465e0bbefd897fb9016475934cfa0f8c95f992ea402d47921f46382d00481b720314b19d8c878e75d81b9763358dd304b2ed3a364e07a3134691",
	     SHEX("1d 80 60 3c 85 44 c7 27"),
	     NULL,
	     NULL,
	     SHEX("5fb5261b61e872f9 3efc03200f47378e f039aa89b993a274 a25dec5e5d49ed59"
		  "84b7dfdf5970c3f7 3059a26d08f7bbc5 0830799bda18b533 499c4f00c21cff3e"
		  "3b8e53a1ea920eb1 d7f3d08aa9e47595 4a53ac018c210b48 15451b7accc4a797"
		  "a2b8faf3d89ee717 d07a857794b9b053 f8e0fd5456ccfcc2 2fd081c873416a3f"));

    /* RFC 7836, App B, 7), values there are little endian, calculation for size A */
    test_vko(nettle_get_gost_gc512a(),
	     "67b63ca4ac8d2bb32618d89296c7476dbeb9f9048496f202b1902cf2ce41dbc2f847712d960483458d4b380867f426c7ca0ff5782702dbc44ee8fc72d9ec90c9",
	     "51a6d54ee932d176e87591121cce5f395cb2f2f147114d95f463c8a7ed74a9fc5ecd2325a35fb6387831ea66bc3d2aa42ede35872cc75372073a71b983e12f19",
	     "793bde5bf72840ad22b02a363ae4772d4a52fc08ba1a20f7458a222a13bf98b53be002d1973f1e398ce46c17da6d00d9b6d0076f8284dcc42e599b4c413b8804",
	     SHEX("1d 80 60 3c 85 44 c7 27"),
	     &nettle_streebog256,
	     &ctx_256,
	     SHEX("c9 a9 a7 73 20 e2 cc 55 9e d7 2d ce 6f 47 e2 19 2c ce a9 5f a6 48 67 05 82 c0 54 c0 ef 36 c2 21"));

    /* RFC 7836, App B, 7), values there are little endian, calculation for size B */
    test_vko(nettle_get_gost_gc512a(),
	     "dbd09213a592da5bbfd8ed068cccccbbfbeda4feac96b9b4908591440b0714803b9eb763ef932266d4c0181a9b73eacf9013efc65ec07c888515f1b6f759c848",
	     "a7c0adb12743c10c3c1beb97c8f631242f7937a1deb6bce5e664e49261baccd3f5dc56ec53b2abb90ca1eb703078ba546655a8b99f79188d2021ffaba4edb0aa",
	     "5adb1c63a4e4465e0bbefd897fb9016475934cfa0f8c95f992ea402d47921f46382d00481b720314b19d8c878e75d81b9763358dd304b2ed3a364e07a3134691",
	     SHEX("1d 80 60 3c 85 44 c7 27"),
	     &nettle_streebog256,
	     &ctx_256,
	     SHEX("c9 a9 a7 73 20 e2 cc 55 9e d7 2d ce 6f 47 e2 19 2c ce a9 5f a6 48 67 05 82 c0 54 c0 ef 36 c2 21"));

    /* RFC 7836, App B, 8), values there are little endian, calculation for size A */
    test_vko(nettle_get_gost_gc512a(),
	     "67b63ca4ac8d2bb32618d89296c7476dbeb9f9048496f202b1902cf2ce41dbc2f847712d960483458d4b380867f426c7ca0ff5782702dbc44ee8fc72d9ec90c9",
	     "51a6d54ee932d176e87591121cce5f395cb2f2f147114d95f463c8a7ed74a9fc5ecd2325a35fb6387831ea66bc3d2aa42ede35872cc75372073a71b983e12f19",
	     "793bde5bf72840ad22b02a363ae4772d4a52fc08ba1a20f7458a222a13bf98b53be002d1973f1e398ce46c17da6d00d9b6d0076f8284dcc42e599b4c413b8804",
	     SHEX("1d 80 60 3c 85 44 c7 27"),
	     &nettle_streebog512,
	     &ctx_512,
	     SHEX("79 f0 02 a9 69 40 ce 7b de 32 59 a5 2e 01 52 97 ad aa d8 45 97 a0 d2 05 b5 0e 3e 17 19 f9 7b fa"
		  "7e e1 d2 66 1f a9 97 9a 5a a2 35 b5 58 a7 e6 d9 f8 8f 98 2d d6 3f c3 5a 8e c0 dd 5e 24 2d 3b df"));

    /* RFC 7836, App B, 8), values there are little endian, calculation for size B */
    test_vko(nettle_get_gost_gc512a(),
	     "dbd09213a592da5bbfd8ed068cccccbbfbeda4feac96b9b4908591440b0714803b9eb763ef932266d4c0181a9b73eacf9013efc65ec07c888515f1b6f759c848",
	     "a7c0adb12743c10c3c1beb97c8f631242f7937a1deb6bce5e664e49261baccd3f5dc56ec53b2abb90ca1eb703078ba546655a8b99f79188d2021ffaba4edb0aa",
	     "5adb1c63a4e4465e0bbefd897fb9016475934cfa0f8c95f992ea402d47921f46382d00481b720314b19d8c878e75d81b9763358dd304b2ed3a364e07a3134691",
	     SHEX("1d 80 60 3c 85 44 c7 27"),
	     &nettle_streebog512,
	     &ctx_512,
	     SHEX("79 f0 02 a9 69 40 ce 7b de 32 59 a5 2e 01 52 97 ad aa d8 45 97 a0 d2 05 b5 0e 3e 17 19 f9 7b fa"
		  "7e e1 d2 66 1f a9 97 9a 5a a2 35 b5 58 a7 e6 d9 f8 8f 98 2d d6 3f c3 5a 8e c0 dd 5e 24 2d 3b df"));
}
