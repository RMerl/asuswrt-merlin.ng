/* Public domain, no copyright. Use at your own risk. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <check.h>

#include <jwt.h>

static void *test_malloc(size_t size)
{
	return malloc(size);
}

static void test_free(void *ptr)
{
	free(ptr);
}

static void *test_realloc(void *ptr, size_t size)
{
	return realloc(ptr, size);
}

static int test_set_alloc(void)
{
	return jwt_set_alloc(test_malloc, test_realloc, test_free);
}

START_TEST(test_alloc_funcs)
{
	jwt_malloc_t m = NULL;
	jwt_realloc_t r = NULL;
	jwt_free_t f = NULL;
	int ret;

	jwt_get_alloc(&m, &r, &f);
	ck_assert_ptr_null(m);
	ck_assert_ptr_null(r);
	ck_assert_ptr_null(f);

	ret = test_set_alloc();
	ck_assert_int_eq(ret, 0);

	jwt_get_alloc(&m, &r, &f);
	ck_assert(m == test_malloc);
	ck_assert(r == test_realloc);
	ck_assert(f == test_free);
}
END_TEST

START_TEST(test_jwt_dump_fp)
{
	FILE *out;
	jwt_t *jwt = NULL;
	int ret = 0;

	ret = test_set_alloc();
	ck_assert_int_eq(ret, 0);

	ret = jwt_new(&jwt);
	ck_assert_int_eq(ret, 0);
	ck_assert_ptr_nonnull(jwt);

	ret = jwt_add_grant(jwt, "iss", "files.maclara-llc.com");
	ck_assert_int_eq(ret, 0);

	ret = jwt_add_grant(jwt, "sub", "user0");
	ck_assert_int_eq(ret, 0);

	ret = jwt_add_grant(jwt, "ref", "XXXX-YYYY-ZZZZ-AAAA-CCCC");
	ck_assert_int_eq(ret, 0);

	ret = jwt_add_grant_int(jwt, "iat", (long)time(NULL));
	ck_assert_int_eq(ret, 0);

#ifdef _WIN32
	out = fopen("nul", "w");
#else
	out = fopen("/dev/null", "w");
#endif
	ck_assert_ptr_ne(out, NULL);

	ret = jwt_dump_fp(jwt, out, 1);
	ck_assert_int_eq(ret, 0);

	ret = jwt_dump_fp(jwt, out, 0);
	ck_assert_int_eq(ret, 0);

	fclose(out);

	jwt_free(jwt);
}
END_TEST

START_TEST(test_jwt_dump_str)
{
	jwt_t *jwt = NULL;
	int ret = 0;
	char *out;
	const char *val = NULL;

	ret = test_set_alloc();
	ck_assert_int_eq(ret, 0);

	ret = jwt_new(&jwt);
	ck_assert_int_eq(ret, 0);
	ck_assert_ptr_nonnull(jwt);

	ret = jwt_add_grant(jwt, "iss", "files.maclara-llc.com");
	ck_assert_int_eq(ret, 0);

	ret = jwt_add_grant(jwt, "sub", "user0");
	ck_assert_int_eq(ret, 0);

	ret = jwt_add_grant(jwt, "ref", "XXXX-YYYY-ZZZZ-AAAA-CCCC");
	ck_assert_int_eq(ret, 0);

	ret = jwt_add_grant_int(jwt, "iat", (long)time(NULL));
	ck_assert_int_eq(ret, 0);

	/* Test 'typ' header: should not be present, cause 'alg' is JWT_ALG_NONE. */
	val = jwt_get_header(jwt, "typ");
	ck_assert_ptr_null(val);

	out = jwt_dump_str(jwt, 1);
	ck_assert_ptr_nonnull(out);

	/* Test 'typ' header: should not be present, cause 'alg' is JWT_ALG_NONE. */
	val = jwt_get_header(jwt, "typ");
	ck_assert_ptr_null(val);

	jwt_free_str(out);

	out = jwt_dump_str(jwt, 0);
	ck_assert_ptr_nonnull(out);

	/* Test 'typ' header: should not be present, cause 'alg' is JWT_ALG_NONE. */
	val = jwt_get_header(jwt, "typ");
	ck_assert_ptr_null(val);

	jwt_free_str(out);

	jwt_free(jwt);
}
END_TEST

#define JSON_GRANTS_PRETTY "\n{\n" \
	"    \"%s\": %ld,\n" \
	"    \"%s\": \"%s\",\n" \
	"    \"%s\": \"%s\",\n" \
	"    \"%s\": \"%s\"\n" \
	"}\n"

#define JSON_GRANTS_COMPACT "{\"%s\":%ld,\"%s\":\"%s\"," \
	"\"%s\":\"%s\",\"%s\":\"%s\"}"


START_TEST(test_jwt_dump_grants_str)
{
	jwt_t *jwt = NULL;
	int ret = 0;
	char *out;
	long timestamp = (long)time(NULL);
	char buf[1024];

	ret = test_set_alloc();
	ck_assert_int_eq(ret, 0);

	ret = jwt_new(&jwt);
	ck_assert_int_eq(ret, 0);
	ck_assert_ptr_nonnull(jwt);

	ret = jwt_add_grant(jwt, "iss", "files.maclara-llc.com");
	ck_assert_int_eq(ret, 0);

	ret = jwt_add_grant(jwt, "sub", "user0");
	ck_assert_int_eq(ret, 0);

	ret = jwt_add_grant(jwt, "ref", "XXXX-YYYY-ZZZZ-AAAA-CCCC");
	ck_assert_int_eq(ret, 0);

	ret = jwt_add_grant_int(jwt, "iat", timestamp);
	ck_assert_int_eq(ret, 0);

	out = jwt_dump_grants_str(jwt, 1);
	ck_assert_ptr_nonnull(out);

	/* Sorted Keys are expected */
	snprintf(buf, sizeof(buf), JSON_GRANTS_PRETTY,
			"iat", timestamp,
			"iss", "files.maclara-llc.com",
			"ref", "XXXX-YYYY-ZZZZ-AAAA-CCCC",
			"sub", "user0");
	ck_assert_str_eq(out, buf);

	jwt_free_str(out);

	out = jwt_dump_grants_str(jwt, 0);
	ck_assert_ptr_nonnull(out);

	/* Sorted Keys are expected */
	snprintf(buf, sizeof(buf), JSON_GRANTS_COMPACT,
			"iat", timestamp,
			"iss", "files.maclara-llc.com",
			"ref", "XXXX-YYYY-ZZZZ-AAAA-CCCC",
			"sub", "user0");
	ck_assert_str_eq(out, buf);

	jwt_free_str(out);

	jwt_free(jwt);
}
END_TEST

START_TEST(test_jwt_dump_str_alg_default_typ_header)
{
	jwt_t *jwt = NULL;
	const char key[] = "My Passphrase";
	int ret = 0;
	char *out;
	const char *val = NULL;

	ret = test_set_alloc();
	ck_assert_int_eq(ret, 0);

	ret = jwt_new(&jwt);
	ck_assert_int_eq(ret, 0);
	ck_assert_ptr_nonnull(jwt);

	ret = jwt_add_grant(jwt, "iss", "files.maclara-llc.com");
	ck_assert_int_eq(ret, 0);

	ret = jwt_add_grant(jwt, "sub", "user0");
	ck_assert_int_eq(ret, 0);

	ret = jwt_add_grant(jwt, "ref", "XXXX-YYYY-ZZZZ-AAAA-CCCC");
	ck_assert_int_eq(ret, 0);

	ret = jwt_add_grant_int(jwt, "iat", (long)time(NULL));
	ck_assert_int_eq(ret, 0);

	ret = jwt_set_alg(jwt, JWT_ALG_HS256, (unsigned char *)key,
			  strlen(key));
	ck_assert_int_eq(ret, 0);

	/* Test 'typ' header: should not be present, cause jwt's header has not been touched yet
	 * by jwt_write_head, this is only called as a result of calling jwt_dump* methods. */
	val = jwt_get_header(jwt, "typ");
	ck_assert_ptr_null(val);

	out = jwt_dump_str(jwt, 1);
	ck_assert_ptr_nonnull(out);

	/* Test 'typ' header: should be added with default value of 'JWT', cause 'alg' is set explicitly
	 * and jwt's header has been processed by jwt_write_head. */
	val = jwt_get_header(jwt, "typ");
	ck_assert_ptr_nonnull(val);
	ck_assert_str_eq(val, "JWT");

	jwt_free_str(out);

	out = jwt_dump_str(jwt, 0);
	ck_assert_ptr_nonnull(out);

	/* Test 'typ' header: should be added with default value of 'JWT', cause 'alg' is set explicitly
	 * and jwt's header has been processed by jwt_write_head. */
	val = jwt_get_header(jwt, "typ");
	ck_assert_ptr_nonnull(val);
	ck_assert_str_eq(val, "JWT");

	jwt_free_str(out);

	jwt_free(jwt);
}
END_TEST

START_TEST(test_jwt_dump_str_alg_custom_typ_header)
{
	jwt_t *jwt = NULL;
	const char key[] = "My Passphrase";
	int ret = 0;
	char *out;
	const char *val = NULL;

	ret = test_set_alloc();
	ck_assert_int_eq(ret, 0);

	ret = jwt_new(&jwt);
	ck_assert_int_eq(ret, 0);
	ck_assert_ptr_nonnull(jwt);

	ret = jwt_add_grant(jwt, "iss", "files.maclara-llc.com");
	ck_assert_int_eq(ret, 0);

	ret = jwt_add_grant(jwt, "sub", "user0");
	ck_assert_int_eq(ret, 0);

	ret = jwt_add_grant(jwt, "ref", "XXXX-YYYY-ZZZZ-AAAA-CCCC");
	ck_assert_int_eq(ret, 0);

	ret = jwt_add_grant_int(jwt, "iat", (long)time(NULL));
	ck_assert_int_eq(ret, 0);

	ret = jwt_add_header(jwt, "typ", "favourite");
	ck_assert_int_eq(ret, 0);

	/* Test that 'typ' header has been added. */
	val = jwt_get_header(jwt, "typ");
	ck_assert_ptr_nonnull(val);
	ck_assert_str_eq(val, "favourite");

	ret = jwt_set_alg(jwt, JWT_ALG_HS256, (unsigned char *)key,
			  strlen(key));
	ck_assert_int_eq(ret, 0);

	/* Test 'typ' header: should be left untouched. */
	val = jwt_get_header(jwt, "typ");
	ck_assert_ptr_nonnull(val);
	ck_assert_str_eq(val, "favourite");

	out = jwt_dump_str(jwt, 1);
	ck_assert_ptr_nonnull(out);

	/* Test 'typ' header: should be left untouched. */
	val = jwt_get_header(jwt, "typ");
	ck_assert_ptr_nonnull(val);
	ck_assert_str_eq(val, "favourite");

	jwt_free_str(out);

	out = jwt_dump_str(jwt, 0);
	ck_assert_ptr_nonnull(out);

	/* Test 'typ' header: should be left untouched. */
	val = jwt_get_header(jwt, "typ");
	ck_assert_ptr_nonnull(val);
	ck_assert_str_eq(val, "favourite");

	jwt_free_str(out);

	jwt_free(jwt);
}
END_TEST

static Suite *libjwt_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("LibJWT Dump");

	tc_core = tcase_create("jwt_dump");

	tcase_add_test(tc_core, test_alloc_funcs);
	tcase_add_test(tc_core, test_jwt_dump_fp);
	tcase_add_test(tc_core, test_jwt_dump_str);
	tcase_add_test(tc_core, test_jwt_dump_grants_str);
	tcase_add_test(tc_core, test_jwt_dump_str_alg_default_typ_header);
	tcase_add_test(tc_core, test_jwt_dump_str_alg_custom_typ_header);

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
