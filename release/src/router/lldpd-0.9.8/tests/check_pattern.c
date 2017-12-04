/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2014 Vincent Bernat <bernat@luffy.cx>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <check.h>

#include "../src/daemon/lldpd.h"

START_TEST(test_empty) {
	ck_assert_int_eq(pattern_match("eth0", "", 0), 0);
	ck_assert_int_eq(pattern_match("eth0", "", 1), 1);
}
END_TEST

START_TEST(test_simple_match) {
	ck_assert_int_eq(pattern_match("eth0", "eth0", 0), 2);
	ck_assert_int_eq(pattern_match("eth0", "eth0", 1), 2);
	ck_assert_int_eq(pattern_match("eth0", "eth1", 0), 0);
	ck_assert_int_eq(pattern_match("eth0", "eth1", 1), 1);
}
END_TEST

START_TEST(test_wildcard) {
	ck_assert_int_eq(pattern_match("eth0", "eth*", 0), 1);
	ck_assert_int_eq(pattern_match("eth0", "eth*", 1), 1);
	ck_assert_int_eq(pattern_match("vlan0", "eth*", 0), 0);
	ck_assert_int_eq(pattern_match("vlan0", "eth*", 1), 1);
}
END_TEST

START_TEST(test_match_list) {
	ck_assert_int_eq(pattern_match("eth0", "eth0,eth1,eth2", 0), 2);
	ck_assert_int_eq(pattern_match("eth1", "eth0,eth1,eth2", 0), 2);
	ck_assert_int_eq(pattern_match("eth3", "eth0,eth1,eth2", 0), 0);
	ck_assert_int_eq(pattern_match("eth3", "eth0,eth1,eth2", 1), 1);
}
END_TEST

START_TEST(test_match_list_with_wildcards) {
	ck_assert_int_eq(pattern_match("eth0", "eth0,eth*,eth2", 0), 2);
	ck_assert_int_eq(pattern_match("eth1", "eth0,eth*,eth2", 0), 1);
	ck_assert_int_eq(pattern_match("eth2", "eth0,eth*,eth2", 0), 2);
	ck_assert_int_eq(pattern_match("eth3", "eth0,eth*,eth2", 0), 1);
	ck_assert_int_eq(pattern_match("vlan3", "eth0,eth*,eth2", 0), 0);
	ck_assert_int_eq(pattern_match("vlan3", "eth0,eth*,eth2", 1), 1);
}
END_TEST

START_TEST(test_simple_blacklist) {
	ck_assert_int_eq(pattern_match("eth0", "!eth0", 0), 0);
	ck_assert_int_eq(pattern_match("eth0", "!eth0", 1), 0);
	ck_assert_int_eq(pattern_match("eth1", "!eth0", 0), 0);
	ck_assert_int_eq(pattern_match("eth1", "!eth0", 1), 1);
}
END_TEST

START_TEST(test_match_and_blacklist) {
	ck_assert_int_eq(pattern_match("eth0", "eth0,!eth0", 0), 0);
	ck_assert_int_eq(pattern_match("eth0", "eth0,!eth0", 1), 0);
	ck_assert_int_eq(pattern_match("eth1", "eth0,!eth0", 0), 0);
	ck_assert_int_eq(pattern_match("eth1", "eth0,!eth0", 1), 1);
}
END_TEST

START_TEST(test_blacklist_wildcard) {
	ck_assert_int_eq(pattern_match("eth0", "!eth*", 0), 0);
	ck_assert_int_eq(pattern_match("eth0", "!eth*", 1), 0);
	ck_assert_int_eq(pattern_match("eth1", "!eth*", 0), 0);
	ck_assert_int_eq(pattern_match("eth1", "!eth*", 1), 0);
	ck_assert_int_eq(pattern_match("eth1", "eth*,!eth1", 0), 0);
	ck_assert_int_eq(pattern_match("eth1", "eth*,!eth1", 1), 0);
	ck_assert_int_eq(pattern_match("eth0", "eth*,!eth1", 0), 1);
	ck_assert_int_eq(pattern_match("eth0", "eth*,!eth1", 1), 1);
}
END_TEST

START_TEST(test_whitelist) {
	ck_assert_int_eq(pattern_match("eth0", "!!eth0", 0), 2);
	ck_assert_int_eq(pattern_match("eth0", "!!eth0", 1), 2);
	ck_assert_int_eq(pattern_match("eth1", "!!eth0", 1), 1);
	ck_assert_int_eq(pattern_match("eth0", "!eth*,!!eth0", 0), 2);
	ck_assert_int_eq(pattern_match("eth0", "!eth*,!!eth0", 1), 2);
	ck_assert_int_eq(pattern_match("eth1", "!eth*,!!eth0", 0), 0);
	ck_assert_int_eq(pattern_match("eth1", "!eth*,!!eth0", 1), 0);
	ck_assert_int_eq(pattern_match("vlan0", "*,!eth*,!!eth0", 0), 1);
	ck_assert_int_eq(pattern_match("vlan0", "*,!eth*,!!eth0", 1), 1);
	ck_assert_int_eq(pattern_match("eth0", "*,!eth*,!!eth0", 0), 2);
	ck_assert_int_eq(pattern_match("eth0", "*,!eth*,!!eth0", 1), 2);
	ck_assert_int_eq(pattern_match("eth1", "*,!eth*,!!eth0", 0), 0);
	ck_assert_int_eq(pattern_match("eth1", "*,!!eth0,!eth*", 1), 0);
	ck_assert_int_eq(pattern_match("eth0", "*,!!eth0,!eth*", 0), 2);
	ck_assert_int_eq(pattern_match("eth0", "*,!!eth0,!eth*", 1), 2);
}
END_TEST

Suite *
pattern_suite(void)
{
	Suite *s = suite_create("Pattern matching");

	TCase *tc_pattern = tcase_create("Pattern matching");
	tcase_add_test(tc_pattern, test_empty);
	tcase_add_test(tc_pattern, test_simple_match);
	tcase_add_test(tc_pattern, test_wildcard);
	tcase_add_test(tc_pattern, test_match_list);
	tcase_add_test(tc_pattern, test_match_list_with_wildcards);
	tcase_add_test(tc_pattern, test_simple_blacklist);
	tcase_add_test(tc_pattern, test_match_and_blacklist);
	tcase_add_test(tc_pattern, test_blacklist_wildcard);
	tcase_add_test(tc_pattern, test_whitelist);
	suite_add_tcase(s, tc_pattern);

	return s;
}

int
main()
{
	int number_failed;
	Suite *s = pattern_suite();
	SRunner *sr = srunner_create(s);
	srunner_run_all(sr, CK_ENV);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
