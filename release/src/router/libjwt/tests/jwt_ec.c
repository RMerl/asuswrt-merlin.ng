/* Public domain, no copyright. Use at your own risk. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <check.h>

#include <jwt.h>

/* Constant time to make tests consistent. */
#define TS_CONST	1475980545L

/* Macro to allocate a new JWT with checks. */
#define ALLOC_JWT(__jwt) do {		\
	int __ret = jwt_new(__jwt);	\
	ck_assert_int_eq(__ret, 0);	\
	ck_assert_ptr_ne(__jwt, NULL);	\
} while(0)

/* Older check doesn't have this. */
#ifndef ck_assert_ptr_ne
#define ck_assert_ptr_ne(X, Y) ck_assert(X != Y)
#define ck_assert_ptr_eq(X, Y) ck_assert(X == Y)
#endif

#ifndef ck_assert_int_gt
#define ck_assert_int_gt(X, Y) ck_assert(X > Y)
#endif

static unsigned char key[16384];
static size_t key_len;

/* NOTE: ES signing will generate a different signature every time, so can't
 * be simply string compared for verification like we do with RS. */

static const char jwt_es256[] = "eyJ0eXAiOiJKV1QiLCJhbGciOiJFUzI1NiJ9.eyJpYXQ"
	"iOjE0NzU5ODA1NDUsImlzcyI6ImZpbGVzLmN5cGhyZS5jb20iLCJyZWYiOiJYWFhYLVl"
	"ZWVktWlpaWi1BQUFBLUNDQ0MiLCJzdWIiOiJ1c2VyMCJ9.3AA32Mn5dMuJXxe03mxJcT"
	"fmif1eiv_doUCSVuMgny4DLKIZ3956SIGjeJpj3BSx2Lul7Zwy-PPuxyBwnL1jiWp7iw"
	"PN9G9tV75ylfWvcwkF20bQA9m1vDbUIl8PIK8Q";

static const char jwt_es384[] = "eyJ0eXAiOiJKV1QiLCJhbGciOiJFUzM4NCJ9.eyJpYXQ"
	"iOjE0NzU5ODA1NDUsImlzcyI6ImZpbGVzLmN5cGhyZS5jb20iLCJyZWYiOiJYWFhYLVl"
	"ZWVktWlpaWi1BQUFBLUNDQ0MiLCJzdWIiOiJ1c2VyMCJ9.p6McjolhuIqel0DWaI2OrD"
	"oRYcxgSMnGFirdKT5jXpe9L801HBkouKBJSae8F7LLFUKiE2VVX_514WzkuExLQs2eB1"
	"L2Qahid5VFOK3hc7HcBL-rcCXa8d2tf_MudyrM";

static const char jwt_es512[] = "eyJ0eXAiOiJKV1QiLCJhbGciOiJFUzUxMiJ9.eyJpYXQ"
	"iOjE0NzU5ODA1NDUsImlzcyI6ImZpbGVzLmN5cGhyZS5jb20iLCJyZWYiOiJYWFhYLVl"
	"ZWVktWlpaWi1BQUFBLUNDQ0MiLCJzdWIiOiJ1c2VyMCJ9._i6CCfwqgk9IEFbKjNL8Ki"
	"tPT9NEnXn2-qCSq0UgqkZ3sY-R0cnzD-WzpsEA8QWC882Y-SWwN7qVxK9e45pHUy4jye"
	"YKXJj3agq9tZ61V3TM-BjcnMkERsV37nDQcfom";

static const char jwt_es_invalid[] = "eyJ0eXAiOiJKV1QiLCJhbGciOiJFUzI1NiJ9.eyJpYXQ"
	"iOjE0NzU5ODA1IAmCornholio6ImZpbGVzLmN5cGhyZS5jb20iLCJyZWYiOiJYWFhYLVl"
	"PN9G9tV75ylfWvcwkF20bQA9m1vDbUIl8PIK8Q";

static void read_key(const char *key_file)
{
	FILE *fp = fopen(key_file, "r");
	char *key_path;
	int ret = 0;

	ret = asprintf(&key_path, KEYDIR "/%s", key_file);
	ck_assert_int_gt(ret, 0);

	fp = fopen(key_path, "r");
	ck_assert_ptr_ne(fp, NULL);

	jwt_free_str(key_path);

	key_len = fread(key, 1, sizeof(key), fp);
	ck_assert_int_ne(key_len, 0);

	ck_assert_int_eq(ferror(fp), 0);

	fclose(fp);

	key[key_len] = '\0';
}

static void __verify_jwt(const char *jwt_str, const jwt_alg_t alg)
{
	jwt_t *jwt = NULL;
	int ret = 0;

	read_key("ec_key_secp384r1-pub.pem");

	ret = jwt_decode(&jwt, jwt_str, key, key_len);
	ck_assert_int_eq(ret, 0);
	ck_assert_ptr_ne(jwt, NULL);

	ck_assert(jwt_get_alg(jwt) == alg);

	jwt_free(jwt);
}

static void __test_alg_key(const jwt_alg_t alg)
{
	jwt_t *jwt = NULL;
	int ret = 0;
	char *out;

	ALLOC_JWT(&jwt);

	read_key("ec_key_secp384r1.pem");

	ret = jwt_add_grant(jwt, "iss", "files.maclara-llc.com");
	ck_assert_int_eq(ret, 0);

	ret = jwt_add_grant(jwt, "sub", "user0");
	ck_assert_int_eq(ret, 0);

	ret = jwt_add_grant(jwt, "ref", "XXXX-YYYY-ZZZZ-AAAA-CCCC");
	ck_assert_int_eq(ret, 0);

	ret = jwt_add_grant_int(jwt, "iat", TS_CONST);
	ck_assert_int_eq(ret, 0);

	ret = jwt_set_alg(jwt, alg, key, key_len);
	ck_assert_int_eq(ret, 0);

	out = jwt_encode_str(jwt);
	ck_assert_ptr_ne(out, NULL);

	__verify_jwt(out, alg);

	jwt_free_str(out);
	jwt_free(jwt);
}

START_TEST(test_jwt_encode_es256)
{
	__test_alg_key(JWT_ALG_ES256);
}
END_TEST

START_TEST(test_jwt_verify_es256)
{
	__verify_jwt(jwt_es256, JWT_ALG_ES256);
}
END_TEST

START_TEST(test_jwt_encode_es384)
{
	__test_alg_key(JWT_ALG_ES384);
}
END_TEST

START_TEST(test_jwt_verify_es384)
{
	__verify_jwt(jwt_es384, JWT_ALG_ES384);
}
END_TEST

START_TEST(test_jwt_encode_es512)
{
	__test_alg_key(JWT_ALG_ES512);
}
END_TEST

START_TEST(test_jwt_verify_es512)
{
	__verify_jwt(jwt_es512, JWT_ALG_ES512);
}
END_TEST

START_TEST(test_jwt_encode_ec_with_rsa)
{
	jwt_t *jwt = NULL;
	int ret = 0;
	char *out;

	ALLOC_JWT(&jwt);

	read_key("rsa_key_4096.pem");

	ret = jwt_add_grant(jwt, "iss", "files.maclara-llc.com");
	ck_assert_int_eq(ret, 0);

	ret = jwt_add_grant(jwt, "sub", "user0");
	ck_assert_int_eq(ret, 0);

	ret = jwt_add_grant(jwt, "ref", "XXXX-YYYY-ZZZZ-AAAA-CCCC");
	ck_assert_int_eq(ret, 0);

	ret = jwt_add_grant_int(jwt, "iat", TS_CONST);
	ck_assert_int_eq(ret, 0);

	ret = jwt_set_alg(jwt, JWT_ALG_ES384, key, key_len);
	ck_assert_int_eq(ret, 0);

	out = jwt_encode_str(jwt);
	ck_assert_ptr_eq(out, NULL);
	ck_assert_int_eq(errno, EINVAL);

	jwt_free(jwt);
}
END_TEST

START_TEST(test_jwt_verify_invalid_token)
{
	jwt_t *jwt = NULL;
	int ret = 0;

	read_key("ec_key_secp384r1.pem");

	ret = jwt_decode(&jwt, jwt_es_invalid, key, JWT_ALG_ES256);
	ck_assert_int_ne(ret, 0);
	ck_assert_ptr_eq(jwt, NULL);
}
END_TEST

START_TEST(test_jwt_verify_invalid_alg)
{
	jwt_t *jwt = NULL;
	int ret = 0;

	read_key("ec_key_secp384r1.pem");

	ret = jwt_decode(&jwt, jwt_es256, key, JWT_ALG_ES512);
	ck_assert_int_ne(ret, 0);
	ck_assert_ptr_eq(jwt, NULL);
}
END_TEST

START_TEST(test_jwt_verify_invalid_cert)
{
	jwt_t *jwt = NULL;
	int ret = 0;

	read_key("ec_key_secp521r1-pub.pem");

	ret = jwt_decode(&jwt, jwt_es256, key, JWT_ALG_ES256);
	ck_assert_int_ne(ret, 0);
	ck_assert_ptr_eq(jwt, NULL);
}
END_TEST

START_TEST(test_jwt_verify_invalid_cert_file)
{
	jwt_t *jwt = NULL;
	int ret = 0;

	read_key("ec_key_invalid-pub.pem");

	ret = jwt_decode(&jwt, jwt_es256, key, JWT_ALG_ES256);
	ck_assert_int_ne(ret, 0);
	ck_assert_ptr_eq(jwt, NULL);
}
END_TEST

START_TEST(test_jwt_encode_invalid_key)
{
	jwt_t *jwt = NULL;
	int ret = 0;
	char *out = NULL;

	ALLOC_JWT(&jwt);

	read_key("ec_key_invalid.pem");

	ret = jwt_add_grant(jwt, "iss", "files.maclara-llc.com");
	ck_assert_int_eq(ret, 0);

	ret = jwt_add_grant(jwt, "sub", "user0");
	ck_assert_int_eq(ret, 0);

	ret = jwt_add_grant(jwt, "ref", "XXXX-YYYY-ZZZZ-AAAA-CCCC");
	ck_assert_int_eq(ret, 0);

	ret = jwt_add_grant_int(jwt, "iat", TS_CONST);
	ck_assert_int_eq(ret, 0);

	ret = jwt_set_alg(jwt, JWT_ALG_ES512, key, key_len);
	ck_assert_int_eq(ret, 0);

	out = jwt_encode_str(jwt);
	ck_assert_ptr_eq(out, NULL);

	jwt_free(jwt);
}
END_TEST

static Suite *libjwt_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("LibJWT EC Sign/Verify");

	tc_core = tcase_create("jwt_ec");

	tcase_add_test(tc_core, test_jwt_encode_es256);
	tcase_add_test(tc_core, test_jwt_verify_es256);
	tcase_add_test(tc_core, test_jwt_encode_es384);
	tcase_add_test(tc_core, test_jwt_verify_es384);
	tcase_add_test(tc_core, test_jwt_encode_es512);
	tcase_add_test(tc_core, test_jwt_verify_es512);
	tcase_add_test(tc_core, test_jwt_encode_ec_with_rsa);
	tcase_add_test(tc_core, test_jwt_verify_invalid_token);
	tcase_add_test(tc_core, test_jwt_verify_invalid_alg);
	tcase_add_test(tc_core, test_jwt_verify_invalid_cert);
	tcase_add_test(tc_core, test_jwt_verify_invalid_cert_file);
	tcase_add_test(tc_core, test_jwt_encode_invalid_key);

	tcase_set_timeout(tc_core, 30);

	suite_add_tcase(s, tc_core);

	return s;
}

int main(int argc, char *argv[])
{
	int number_failed;
	Suite *s;
	SRunner *sr;

	s = libjwt_suite();
	sr = srunner_create(s);

	srunner_run_all(sr, CK_VERBOSE);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
