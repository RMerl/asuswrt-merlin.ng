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

static const char jwt_ps256_2048[] = "eyJhbGciOiJQUzI1NiIsInR5cCI6IkpXVCJ9.ey"
	"JpYXQiOjE0NzU5ODA1NDUsImlzcyI6ImZpbGVzLm1hY2xhcmEtbGxjLmNvbSIsInJlZi"
	"I6IlhYWFgtWVlZWS1aWlpaLUFBQUEtQ0NDQyIsInN1YiI6InVzZXIwIn0.B9gxqtbZae"
	"9PyGkjQaBMyBITOieALP39yCDSqmynmvnE2L8JJzNxOKjm5dy_ORhYjagghE18ti90v2"
	"whAwRFFvA7MlQC2rQm-4pXrHqAyhT7Dl1_lSeL98WGToZgJ646WLjr-SwbMNjp3RWwZz"
	"F-IwnB1D1f-RoA9yUoaNEFHUYVuL4okVj4ImnUE07pW-l2eal3bxUg6lzqGWSctbT46t"
	"y8qFlsOyrifev3y_z6-eKPHUruYEbWb1zw3-snBtcPfGMWAQ91PVoNkPLTO6G56I8FAF"
	"IufXyyp6k9VuKQ_WRzRQhwO8zBOto4RsTUjYbDJEY2FSFYVZUdPctwojNlCw";

static const char jwt_ps384_2048[] = "eyJhbGciOiJQUzM4NCIsInR5cCI6IkpXVCJ9.ey"
	"JpYXQiOjE0NzU5ODA1NDUsImlzcyI6ImZpbGVzLm1hY2xhcmEtbGxjLmNvbSIsInJlZi"
	"I6IlhYWFgtWVlZWS1aWlpaLUFBQUEtQ0NDQyIsInN1YiI6InVzZXIwIn0.GY8aZobXTy"
	"6DzooRUt6vwgBbWwWvTchFtDCVMto_NM68aqT_OI8_X1MAHwE7ppS1S-yxg1aEeGzZEG"
	"VEAdeIzswd7ilCpQrUQ2Qcym6SuK3NAKLtr6NyUZwdaEPTeEx3GWQbmvY66hVs7g2o4c"
	"luSfp3I4McgLCm-HS5Dl_xHoyV_1ympz_n3n7YDoe5l0EoHaX3-XPMtUvL4kxeMV5pLh"
	"72Yj2qNM5Dbbe9F_WSxoeQsyktg8MmPb22LWAAW7uafazr7TinJvPtBhPqT7hc2sUFbA"
	"Jui_TSM60Kjfqg15QQELifywNvgW0ZO6xKEI5GKgaIi2S9F2iqQehBBkjMrg";

static const char jwt_ps512_2048[] = "eyJhbGciOiJQUzUxMiIsInR5cCI6IkpXVCJ9.ey"
	"JpYXQiOjE0NzU5ODA1NDUsImlzcyI6ImZpbGVzLm1hY2xhcmEtbGxjLmNvbSIsInJlZi"
	"I6IlhYWFgtWVlZWS1aWlpaLUFBQUEtQ0NDQyIsInN1YiI6InVzZXIwIn0.OxnjxVNAEC"
	"xEnNVg6S6sx-JIxOq3sJimEefq4OONsYomWz1TAM8_42bmAnvda0bhC8LTmIogQwnYj3"
	"qIYrjef3s7nrs5USS3_ffqeMuog_Xp7cH1YhVwvkXEWzfeT-SLZiEdxGBrPvEASxwzv0"
	"CitQrfDGvFe20UXkhAvOKIc_1K5Fzv9IQiaKaPR2Jg8Ub0qQ6qZq1whnwDbjutWCFlW3"
	"62UOQbhA2WtE72Q60OFXMr2J0PYrScGgTRRrL6V2G7cNRend14FzDFG586dGUCwp9iKF"
	"nCrshFefpaFsOJYHG70Ka6CNIDG4LDiLatjjz1UCtAgbnHfy9qyJEpcJYPWg";

static const char jwt_ps256_2048_invalid[] = "eyJhbGciOiJQUzI1NiIsInR5cCI6Ikp"
	"XVCJ9.eyJpYXQiOjE0NzU5ODA1NDUsImlzcyI6ImZpbGVzLm1hY2xhcmEtbGxjLmNvbS"
	"IsInJlZiI6IlhYWFgtWVlZWS1aWlpaLUFBQUEtQ0NDQyIsInN1YiI6InVzZXIwIn0.WX"
	"41yYTKxf6lDg7toDAAnwuLKCUSdEWUsJ-5neEbOPE4l09EEIDW2cjK4NZkAgySgCZHCa"
	"NUSn8XaOouoLoEMVua5f0g6U-_-c380KRfmiqFGe39vjHCqiw8j-WkdxHisi7eXw3fvL"
	"kp0VoyeWA6Fnp2x-shfHU5Br67Wagp7OgCk-SvVL08xyfvgZr6fzEqc486zdNhQE71Pv"
	"in5dRQ75Lg3rr1W8Xmx2zRrFKZALsEwGMhRL7e-x46mt6KF1UlwTYAW6FYoKTrrW62sH"
	"OgpgvsIwhE93RfCmJ_xvZNkKrqnB6RxfpHEbZYTS8iAI3va2S8IBEL_pH-2etsr1fqAg";

#define RSA_PSS_KEY_PRE "rsa-pss_key_2048"
#define PS_KEY_PRIV RSA_PSS_KEY_PRE ".pem"
#define PS_KEY_PUB RSA_PSS_KEY_PRE "-pub.pem"

static void read_key(const char *key_file)
{
	FILE *fp;
	char *key_path;
	int ret;

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

static void __verify_alg_key(const char *key_file, const char *jwt_str,
			     const jwt_alg_t alg)
{
	jwt_valid_t *jwt_valid;
	jwt_t *jwt;
	int ret;

	read_key(key_file);

	ret = jwt_decode(&jwt, jwt_str, key, key_len);
	ck_assert_int_eq(ret, 0);
	ck_assert_ptr_ne(jwt, NULL);

	jwt_valid_new(&jwt_valid, alg);
	ck_assert_ptr_ne(jwt_valid, NULL);

	ret = jwt_validate(jwt, jwt_valid);
	ck_assert_int_eq(JWT_VALIDATION_SUCCESS, ret);

	jwt_valid_free(jwt_valid);
	jwt_free(jwt);
}

static void __test_rsa_pss_encode(const char *priv_key_file,
				  const char *pub_key_file,
				  const jwt_alg_t alg)
{
	jwt_t *jwt;
	int ret;
	char *out;

	ALLOC_JWT(&jwt);

	read_key(priv_key_file);

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

	__verify_alg_key(pub_key_file, out, alg);

	jwt_free_str(out);
	jwt_free(jwt);
}

START_TEST(test_jwt_encode_ps256)
{
	__test_rsa_pss_encode(PS_KEY_PRIV, PS_KEY_PUB, JWT_ALG_PS256);
}
END_TEST

START_TEST(test_jwt_encode_ps384)
{
	__test_rsa_pss_encode(PS_KEY_PRIV, PS_KEY_PUB, JWT_ALG_PS384);
}
END_TEST

START_TEST(test_jwt_encode_ps512)
{
	__test_rsa_pss_encode(PS_KEY_PRIV, PS_KEY_PUB, JWT_ALG_PS512);
}
END_TEST

START_TEST(test_jwt_verify_ps256)
{
	__verify_alg_key(PS_KEY_PUB, jwt_ps256_2048, JWT_ALG_PS256);
}
END_TEST

START_TEST(test_jwt_verify_ps384)
{
	__verify_alg_key(PS_KEY_PUB, jwt_ps384_2048, JWT_ALG_PS384);
}
END_TEST

START_TEST(test_jwt_verify_ps512)
{
	__verify_alg_key(PS_KEY_PUB, jwt_ps512_2048, JWT_ALG_PS512);
}
END_TEST

START_TEST(test_jwt_verify_invalid_rsa_pss)
{
	jwt_t *jwt = NULL;
	int ret = 0;

	read_key(PS_KEY_PUB);

	ret = jwt_decode(&jwt, jwt_ps256_2048_invalid, key, key_len);
	ck_assert_int_ne(ret, 0);
	ck_assert_ptr_eq(jwt, NULL);
}
END_TEST

static Suite *libjwt_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("LibJWT RSA-PSS Sign/Verify");

	tc_core = tcase_create("jwt_rsa_pss");

	tcase_add_test(tc_core, test_jwt_encode_ps256);
	tcase_add_test(tc_core, test_jwt_encode_ps384);
	tcase_add_test(tc_core, test_jwt_encode_ps512);
	tcase_add_test(tc_core, test_jwt_verify_ps256);
	tcase_add_test(tc_core, test_jwt_verify_ps384);
	tcase_add_test(tc_core, test_jwt_verify_ps512);
	tcase_add_test(tc_core,test_jwt_verify_invalid_rsa_pss);

	tcase_set_timeout(tc_core, 120);

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
