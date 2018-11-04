#include "testutils.h"

#include "pss.h"

void
test_main(void)
{
  struct tstring *salt;
  struct tstring *digest;
  mpz_t m;
  mpz_t expected;

  /* From ftp://ftp.rsasecurity.com/pub/pkcs/pkcs-1/pkcs-1v2-1d2-vec.zip */
  mpz_init(m);
  mpz_init(expected);

  salt = SHEX("e3b5d5d002c1bce50c2b65ef88a188d83bce7e61");
  digest = SHEX("37b66ae0445843353d47ecb0b4fd14c110e62d6a");
  ASSERT(pss_encode_mgf1(m, 1024, &nettle_sha1,
			 salt->length, salt->data, digest->data));

  mpz_set_str(expected,
	      "66e4672e836ad121ba244bed6576b867d9a447c28a6e66a5b87dee"
	      "7fbc7e65af5057f86fae8984d9ba7f969ad6fe02a4d75f7445fefd"
	      "d85b6d3a477c28d24ba1e3756f792dd1dce8ca94440ecb5279ecd3"
	      "183a311fc896da1cb39311af37ea4a75e24bdbfd5c1da0de7cecdf"
	      "1a896f9d8bc816d97cd7a2c43bad546fbe8cfebc", 16);

  ASSERT(mpz_cmp(m, expected) == 0);

  mpz_add_ui(m, m, 2);
  ASSERT(!pss_verify_mgf1(m, 1024, &nettle_sha1, salt->length, digest->data));

  mpz_sub_ui(m, m, 2);
  ASSERT(pss_verify_mgf1(m, 1024, &nettle_sha1, salt->length, digest->data));

  mpz_clear(m);
  mpz_clear(expected);

  /* Test with our own data.  */
  mpz_init(m);
  mpz_init(expected);

  salt = SHEX("11223344556677889900");
  /* From sha256-test.c */
  digest = SHEX("ba7816bf8f01cfea 414140de5dae2223"
		"b00361a396177a9c b410ff61f20015ad");

  mpz_set_str(expected,
	      "76b9a52705c8382c5367732f993184eff340b6305c9f73e7e308c8"
	      "004fcc15cbbaab01e976bae4b774628595379a2d448a36b3ea6fa8"
	      "353b97eeea7bdac93b4b7807ac98cd4b3bebfb31f3718e1dd3625f"
	      "227fbb8696606498e7070e21c3cbbd7386ea20eb81ac7927e0c6d1"
	      "d7788826a63af767f301bcc05dd65b00da862cbc", 16);

  /* Try bad salt */
  salt->data[6] = 0x00;
  ASSERT(pss_encode_mgf1(m, 1024, &nettle_sha256,
			 salt->length, salt->data, digest->data));
  ASSERT(mpz_cmp(m, expected) != 0);

  /* Try the good salt */
  salt->data[6] = 0x77;
  ASSERT(pss_encode_mgf1(m, 1024, &nettle_sha256,
			 salt->length, salt->data, digest->data));
  ASSERT(mpz_cmp(m, expected) == 0);

  /* Try bad message */
  mpz_add_ui(m, m, 2);
  ASSERT(!pss_verify_mgf1(m, 1024, &nettle_sha256, salt->length, digest->data));

  /* Try the good message */
  mpz_sub_ui(m, m, 2);
  ASSERT(pss_verify_mgf1(m, 1024, &nettle_sha256, salt->length, digest->data));

  /* Try bad digest */
  digest->data[17] = 0x00;
  ASSERT(!pss_verify_mgf1(m, 1024, &nettle_sha256, salt->length, digest->data));

  /* Try the good digest */
  digest->data[17] = 0x03;
  ASSERT(pss_verify_mgf1(m, 1024, &nettle_sha256, salt->length, digest->data));

  mpz_clear(m);
  mpz_clear(expected);
}
