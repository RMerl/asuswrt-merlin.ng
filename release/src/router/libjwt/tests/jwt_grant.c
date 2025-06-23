/* Public domain, no copyright. Use at your own risk. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <check.h>

#include <jwt.h>

START_TEST(test_jwt_add_grant)
{
	jwt_t *jwt = NULL;
	int ret = 0;

	ret = jwt_new(&jwt);
	ck_assert_int_eq(ret, 0);
	ck_assert_ptr_nonnull(jwt);

	ret = jwt_add_grant(jwt, "iss", "test");
	ck_assert_int_eq(ret, 0);

	/* No duplicates */
	ret = jwt_add_grant(jwt, "iss", "other");
	ck_assert_int_eq(ret, EEXIST);

	/* No duplicates for int */
	ret = jwt_add_grant_int(jwt, "iat", (long)time(NULL));
	ck_assert_int_eq(ret, 0);

	ret = jwt_add_grant_int(jwt, "iat", (long)time(NULL));
	ck_assert_int_eq(ret, EEXIST);

	jwt_free(jwt);
}
END_TEST

START_TEST(test_jwt_get_grant)
{
	jwt_t *jwt = NULL;
	const char *val;
	const char testval[] = "testing";
	int ret = 0;

	ret = jwt_new(&jwt);
	ck_assert_int_eq(ret, 0);
	ck_assert_ptr_nonnull(jwt);

	ret = jwt_add_grant(jwt, "iss", testval);
	ck_assert_int_eq(ret, 0);

	val = jwt_get_grant(jwt, "iss");
	ck_assert_ptr_nonnull(val);
	ck_assert_str_eq(val, testval);

	jwt_free(jwt);
}
END_TEST

START_TEST(test_jwt_add_grant_int)
{
	jwt_t *jwt = NULL;
	long val;
	int ret = 0;

	ret = jwt_new(&jwt);
	ck_assert_int_eq(ret, 0);
	ck_assert_ptr_nonnull(jwt);

	ret = jwt_add_grant_int(jwt, "int", 1);
	ck_assert_int_eq(ret, 0);

	val = jwt_get_grant_int(jwt, "int");
	ck_assert(val);

	val = jwt_get_grant_int(jwt, "not found");
	ck_assert_int_eq(errno, ENOENT);

	jwt_free(jwt);
}
END_TEST

START_TEST(test_jwt_add_grant_bool)
{
	jwt_t *jwt = NULL;
	int val;
	int ret = 0;

	ret = jwt_new(&jwt);
	ck_assert_int_eq(ret, 0);
	ck_assert_ptr_nonnull(jwt);

	ret = jwt_add_grant_bool(jwt, "admin", 1);
	ck_assert_int_eq(ret, 0);

	val = jwt_get_grant_bool(jwt, "admin");
	ck_assert(val);

	ret = jwt_add_grant_bool(jwt, "test", 0);
	ck_assert_int_eq(ret, 0);

	val = jwt_get_grant_bool(jwt, "test");
	ck_assert(!val);

	val = jwt_get_grant_bool(jwt, "not found");
	ck_assert_int_eq(errno, ENOENT);

	jwt_free(jwt);
}
END_TEST

START_TEST(test_jwt_del_grants)
{
	jwt_t *jwt = NULL;
	const char *val;
	const char testval[] = "testing";
	int ret = 0;

	ret = jwt_new(&jwt);
	ck_assert_int_eq(ret, 0);
	ck_assert_ptr_nonnull(jwt);

	ret = jwt_add_grant(jwt, "iss", testval);
	ck_assert_int_eq(ret, 0);

	ret = jwt_add_grant(jwt, "other", testval);
	ck_assert_int_eq(ret, 0);

	ret = jwt_del_grants(jwt, "iss");
	ck_assert_int_eq(ret, 0);

	val = jwt_get_grant(jwt, "iss");
	ck_assert_ptr_null(val);

	/* Delete non existent. */
	ret = jwt_del_grants(jwt, "iss");
	ck_assert_int_eq(ret, 0);

	/* Delete all grants. */
	ret = jwt_del_grants(jwt, NULL);
	ck_assert_int_eq(ret, 0);

	val = jwt_get_grant(jwt, "other");
	ck_assert_ptr_null(val);

	jwt_free(jwt);
}
END_TEST

START_TEST(test_jwt_grant_invalid)
{
	jwt_t *jwt = NULL;
	const char *val;
	long valint = 0;
	int valbool = 0;
	int ret = 0;

	ret = jwt_new(&jwt);
	ck_assert_int_eq(ret, 0);
	ck_assert_ptr_nonnull(jwt);

	ret = jwt_add_grant(jwt, "iss", NULL);
	ck_assert_int_eq(ret, EINVAL);

	ret = jwt_add_grant_int(jwt, "", (long)time(NULL));
	ck_assert_int_eq(ret, EINVAL);

	val = jwt_get_grant(jwt, NULL);
	ck_assert_int_eq(errno, EINVAL);
	ck_assert_ptr_null(val);

	valint = jwt_get_grant_int(jwt, NULL);
	ck_assert_int_eq(errno, EINVAL);
	ck_assert(valint == 0);

	valbool = jwt_get_grant_bool(jwt, NULL);
	ck_assert_int_eq(errno, EINVAL);
	ck_assert(valbool == 0);

	jwt_free(jwt);
}
END_TEST

START_TEST(test_jwt_grants_json)
{
	const char *json = "{\"id\":\"FVvGYTr3FhiURCFebsBOpBqTbzHdX/DvImiA2yheXr8=\","
		"\"iss\":\"localhost\",\"other\":[\"foo\",\"bar\"],"
		"\"ref\":\"385d6518-fb73-45fc-b649-0527d8576130\","
		"\"scopes\":\"storage\",\"sub\":\"user0\"}";
	jwt_t *jwt = NULL;
	const char *val;
	char *json_val;
	int ret = 0;

	ret = jwt_new(&jwt);
	ck_assert_int_eq(ret, 0);
	ck_assert_ptr_nonnull(jwt);

	ret = jwt_add_grants_json(jwt, json);
	ck_assert_int_eq(ret, 0);

	val = jwt_get_grant(jwt, "ref");
	ck_assert_ptr_nonnull(val);
	ck_assert_str_eq(val, "385d6518-fb73-45fc-b649-0527d8576130");

	json_val = jwt_get_grants_json(NULL, "other");
	ck_assert_ptr_null(json_val);
	ck_assert_int_eq(errno, EINVAL);

	json_val = jwt_get_grants_json(jwt, "other");
	ck_assert_ptr_nonnull(json_val);
	ck_assert_str_eq(json_val, "[\"foo\",\"bar\"]");

	jwt_free_str(json_val);

	json_val = jwt_get_grants_json(jwt, NULL);
	ck_assert_ptr_nonnull(json_val);
	ck_assert_str_eq(json_val, json);

	jwt_free_str(json_val);

	jwt_free(jwt);
}
END_TEST

static Suite *libjwt_suite(void)
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("LibJWT Grant");

	tc_core = tcase_create("jwt_grant");

	tcase_add_test(tc_core, test_jwt_add_grant);
	tcase_add_test(tc_core, test_jwt_add_grant_int);
	tcase_add_test(tc_core, test_jwt_add_grant_bool);
	tcase_add_test(tc_core, test_jwt_get_grant);
	tcase_add_test(tc_core, test_jwt_del_grants);
	tcase_add_test(tc_core, test_jwt_grant_invalid);
	tcase_add_test(tc_core, test_jwt_grants_json);

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
