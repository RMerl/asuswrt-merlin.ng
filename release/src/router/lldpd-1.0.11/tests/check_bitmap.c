/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2020 Vincent Bernat <bernat@luffy.cx>
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
	uint32_t vlan_bmap[VLAN_BITMAP_LEN] = {};
	ck_assert(bitmap_isempty(vlan_bmap));
	ck_assert_int_eq(bitmap_numbits(vlan_bmap), 0);
}
END_TEST

START_TEST(test_first_bit) {
	uint32_t vlan_bmap[VLAN_BITMAP_LEN] = {};
	bitmap_set(vlan_bmap, 1);
	ck_assert_int_eq(vlan_bmap[0], 2);
	ck_assert_int_eq(bitmap_numbits(vlan_bmap), 1);
}
END_TEST

START_TEST(test_some_bits) {
	uint32_t vlan_bmap[VLAN_BITMAP_LEN] = {};
	bitmap_set(vlan_bmap, 1);
	bitmap_set(vlan_bmap, 6);
	bitmap_set(vlan_bmap, 31);
	bitmap_set(vlan_bmap, 50);
	ck_assert_int_eq(vlan_bmap[0], (1UL << 1) | (1UL << 6) | (1UL << 31));
	ck_assert_int_eq(vlan_bmap[1], (1UL << (50-32)));
	ck_assert_int_eq(vlan_bmap[2], 0);
	ck_assert_int_eq(bitmap_numbits(vlan_bmap), 4);
}
END_TEST

Suite *
bitmap_suite(void)
{
	Suite *s = suite_create("Bitmap handling");

	TCase *tc_bitmap = tcase_create("Bitmap handling");
	tcase_add_test(tc_bitmap, test_empty);
	tcase_add_test(tc_bitmap, test_first_bit);
	tcase_add_test(tc_bitmap, test_some_bits);
	suite_add_tcase(s, tc_bitmap);

	return s;
}

int
main()
{
	int number_failed;
	Suite *s = bitmap_suite();
	SRunner *sr = srunner_create(s);
	srunner_run_all(sr, CK_ENV);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
