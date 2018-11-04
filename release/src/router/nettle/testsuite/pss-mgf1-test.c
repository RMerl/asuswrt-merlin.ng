#include "testutils.h"
#include "pss-mgf1.h"

void
test_main(void)
{
  struct sha1_ctx sha1ctx;
  struct sha256_ctx sha256ctx;
  const struct tstring *seed, *expected;
  uint8_t mask[120];

  /* From ftp://ftp.rsasecurity.com/pub/pkcs/pkcs-1/pkcs-1v2-1d2-vec.zip */
  seed = SHEX("df1a896f9d8bc816d97cd7a2c43bad54"
	      "6fbe8cfe");
  expected = SHEX("66e4672e836ad121ba244bed6576b867d9a447c28a6e66a5b87dee"
		  "7fbc7e65af5057f86fae8984d9ba7f969ad6fe02a4d75f7445fefd"
		  "d85b6d3a477c28d24ba1e3756f792dd1dce8ca94440ecb5279ecd3"
		  "183a311fc89739a96643136e8b0f465e87a4535cd4c59b10028d");
  sha1_init(&sha1ctx);
  sha1_update(&sha1ctx, seed->length, seed->data);
  pss_mgf1(&sha1ctx, &nettle_sha1, expected->length, mask);
  ASSERT(MEMEQ (expected->length, mask, expected->data));

  /* Test with our own data.  */
  seed = SDATA("abc");
  expected = SHEX("cf2db1ac9867debdf8ce91f99f141e5544bf26ca36b3fd4f8e4035"
		  "eec42cab0d46c386ebccef82ba0bb0b095aaa5548b03cdff695187"
		  "1c6fb505af68af688332f885d324a47d2145a3d8392c37978d7dc9"
		  "84c95728950c4cf3de6becc59e60ea506951bd40e6de3863095064"
		  "3ab2edbb47dc66cb54beb2d1");

  sha256_init(&sha256ctx);
  sha256_update(&sha256ctx, seed->length, seed->data);
  pss_mgf1(&sha256ctx, &nettle_sha256, expected->length, mask);
  ASSERT(MEMEQ (expected->length, mask, expected->data));
}
