/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2012 Vincent Bernat <bernat@luffy.cx>
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
#include <stdlib.h>

#include "../src/lib/fixedpoint.h"

#ifdef ENABLE_LLDPMED

START_TEST(test_string_parsing_suffix) {
	char *end;
	fp_strtofp("4541T", &end, 14, 8);
	ck_assert_int_eq(*end, 'T');
	fp_strtofp("4541.U", &end, 14, 8);
	ck_assert_int_eq(*end, 'U');
	fp_strtofp("4541.676V", &end, 14, 8);
	ck_assert_int_eq(*end, 'V');
}
END_TEST

START_TEST(test_string_parsing_positive_int) {
	struct fp_number fp = fp_strtofp("4541T", NULL, 14, 8);
	ck_assert_int_eq(fp.integer.bits, 14);
	ck_assert_int_eq(fp.integer.value, 4541);
	ck_assert_int_eq(fp.fraction.bits, 8);
	ck_assert_int_eq(fp.fraction.value, 0);
	ck_assert_int_eq(fp.fraction.precision, 0);
}
END_TEST

START_TEST(test_string_parsing_negative_int) {
	struct fp_number fp = fp_strtofp("-4214N", NULL, 14, 8);
	ck_assert_int_eq(fp.integer.bits, 14);
	ck_assert_int_eq(fp.integer.value, -4214);
	ck_assert_int_eq(fp.fraction.bits, 8);
	ck_assert_int_eq(fp.fraction.value, 0);
	ck_assert_int_eq(fp.fraction.precision, 0);
}
END_TEST

START_TEST(test_string_parsing_positive_int_overflow) {
	struct fp_number fp1 = fp_strtofp("4098", NULL, 13, 8);
	struct fp_number fp2 = fp_strtofp("4096", NULL, 13, 8);
	struct fp_number fp3 = fp_strtofp("4095", NULL, 13, 8);
	struct fp_number fp4 = fp_strtofp("4094", NULL, 13, 8);
	ck_assert_int_eq(fp1.integer.value, 4095);
	ck_assert_int_eq(fp2.integer.value, 4095);
	ck_assert_int_eq(fp3.integer.value, 4095);
	ck_assert_int_eq(fp4.integer.value, 4094);
}
END_TEST

START_TEST(test_string_parsing_negative_int_overflow) {
	struct fp_number fp1 = fp_strtofp("-4097", NULL, 13, 8);
	struct fp_number fp2 = fp_strtofp("-4096", NULL, 13, 8);
	struct fp_number fp3 = fp_strtofp("-4095", NULL, 13, 8);
	struct fp_number fp4 = fp_strtofp("-4094", NULL, 13, 8);
	ck_assert_int_eq(fp1.integer.value, -4096);
	ck_assert_int_eq(fp2.integer.value, -4096);
	ck_assert_int_eq(fp3.integer.value, -4095);
	ck_assert_int_eq(fp4.integer.value, -4094);
}
END_TEST

START_TEST(test_string_parsing_positive_float) {
	struct fp_number fp1 = fp_strtofp("1542.6250E", NULL, 13, 20);
	ck_assert_int_eq(fp1.integer.value, 1542);
	ck_assert_int_eq(fp1.fraction.precision, 14);
	ck_assert_int_eq((fp1.fraction.value * 10000) >> fp1.fraction.bits, 6250);

	struct fp_number fp2 = fp_strtofp("1542.06250E", NULL, 13, 4);
	ck_assert_int_eq(fp2.integer.value, 1542);
	ck_assert_int_eq(fp2.fraction.precision, 4);
	ck_assert_int_eq((fp2.fraction.value * 10000) >> fp2.fraction.bits, 625);
}
END_TEST

START_TEST(test_string_parsing_negative_float) {
	struct fp_number fp = fp_strtofp("-11542.6250N", NULL, 15, 4);
	ck_assert_int_eq(fp.integer.value, -11542);
	ck_assert_int_eq(fp.fraction.precision, 4);
	ck_assert_int_eq((fp.fraction.value * 10000) >> fp.fraction.bits, 6250);
}
END_TEST

START_TEST(test_string_parsing_no_fract_part) {
	struct fp_number fp = fp_strtofp("11542.", NULL, 15, 4);
	ck_assert_int_eq(fp.integer.value, 11542);
	ck_assert_int_eq(fp.fraction.value, 0);
	ck_assert_int_eq(fp.fraction.precision, 1);
}
END_TEST

START_TEST(test_string_parsing_no_int_part) {
	struct fp_number fp = fp_strtofp(".6250E", NULL, 13, 4);
	ck_assert_int_eq(fp.integer.value, 0);
	ck_assert_int_eq(fp.fraction.precision, 4);
	ck_assert_int_eq((fp.fraction.value * 10000) >> fp.fraction.bits, 6250);
}
END_TEST


START_TEST(test_string_representation_positive_int) {
	struct fp_number fp1 = fp_strtofp("214", NULL, 9, 9);
	struct fp_number fp2 = fp_strtofp("11178.0000", NULL, 15, 9);
	ck_assert_str_eq(fp_fptostr(fp1, NULL), "214");
	ck_assert_str_eq(fp_fptostr(fp2, NULL), "11178");
	ck_assert_str_eq(fp_fptostr(fp2, "ES"), "11178E");
}
END_TEST

START_TEST(test_string_representation_negative_int) {
	struct fp_number fp1 = fp_strtofp("-214", NULL, 9, 9);
	struct fp_number fp2 = fp_strtofp("-11178.0000", NULL, 15, 9);
	ck_assert_str_eq(fp_fptostr(fp1, NULL), "-214");
	ck_assert_str_eq(fp_fptostr(fp2, NULL), "-11178");
	ck_assert_str_eq(fp_fptostr(fp2, "ES"), "11178S");
}
END_TEST

START_TEST(test_string_representation_positive_float) {
	struct fp_number fp = fp_strtofp("214.6250", NULL, 9, 20);
	ck_assert_str_eq(fp_fptostr(fp, NULL), "214.6250");
	ck_assert_str_eq(fp_fptostr(fp, "ES"), "214.6250E");
}
END_TEST

START_TEST(test_string_representation_positive_float_with_leading_zero) {
	struct fp_number fp = fp_strtofp("214.06250", NULL, 9, 24);
	ck_assert_str_eq(fp_fptostr(fp, NULL), "214.06250");
	ck_assert_str_eq(fp_fptostr(fp, "ES"), "214.06250E");
}
END_TEST

START_TEST(test_string_representation_negative_float) {
	struct fp_number fp1 = fp_strtofp("-214.625", NULL, 22, 10);
	struct fp_number fp2 = fp_strtofp("-415.5", NULL, 22, 4);
	ck_assert_str_eq(fp_fptostr(fp1, NULL), "-214.625");
	ck_assert_str_eq(fp_fptostr(fp2, NULL), "-415.5");
	ck_assert_str_eq(fp_fptostr(fp2, "ES"), "415.5S");
}
END_TEST

START_TEST(test_buffer_representation_positive_float) {
	unsigned char buffer[5] = {};
	unsigned char expected[] = { 0x21 << 2, 47 << 1, 0x68, 0x00, 0x00 };
	/* 47.2031250 = 47 + 2**-3 + 2**-4 + 2**-6, precision = 9+24 */
	struct fp_number fp = fp_strtofp("47.2031250", NULL, 9, 25);
	fp_fptobuf(fp, buffer, 0);
	fail_unless(memcmp(buffer, expected, sizeof(expected)) == 0);
}
END_TEST

START_TEST(test_buffer_representation_negative_float) {
	unsigned char buffer[5] = {};
	unsigned char expected[] = { (0x21 << 2) | 3, 0xa1, 0x98, 0x00, 0x00 };
	/* 47.2031250  = 000101111.0011010000000000000000000 */
	/* -47.2031250 = 111010000.1100101111111111111111111 + 1 */
	/* -47.2031250 = 111010000.1100110000000000000000000 */
	struct fp_number fp = fp_strtofp("-47.2031250", NULL, 9, 25);
	fp_fptobuf(fp, buffer, 0);
	fail_unless(memcmp(buffer, expected, sizeof(expected)) == 0);
}
END_TEST

START_TEST(test_buffer_representation_with_shift) {
	unsigned char buffer[] = { 0x77, 0xc6, 0x0, 0x0, 0x0, 0x0, 0xc7 };
	unsigned char expected[] = { 0x77, 0xc8, 0x45, 0xe6, 0x80, 0x00, 0x07 };
	struct fp_number fp = fp_strtofp("47.2031250", NULL, 9, 25);
	fp_fptobuf(fp, buffer, 12);
	fail_unless(memcmp(buffer, expected, sizeof(buffer)) == 0);
}
END_TEST

START_TEST(test_buffer_representation_altitude) {
	unsigned char buffer[5] = {};
	unsigned char expected[] = { (22 + 4) << 2, 0, 0, 14 << 4 | 1 << 3, 0 };
	struct fp_number fp = fp_strtofp("14.5", NULL, 22, 8);
	fp_fptobuf(fp, buffer, 0);
	fail_unless(memcmp(buffer, expected, sizeof(buffer)) == 0);
}
END_TEST

START_TEST(test_buffer_parsing_positive_float) {
	unsigned char buffer[] = { 0x21 << 2, 47 << 1, 0x68, 0x00, 0x00 };
	struct fp_number fp = fp_buftofp(buffer, 9, 25, 0);
	ck_assert_int_eq(fp.integer.value, 47);
	ck_assert_int_eq(fp.integer.bits, 9);
	ck_assert_int_eq((fp.fraction.value * 10000000) >> fp.fraction.bits, 2031250);
	ck_assert_int_eq(fp.fraction.bits, 25);
	ck_assert_int_eq(fp.fraction.precision, 24);
}
END_TEST

START_TEST(test_buffer_parsing_negative_float) {
	unsigned char buffer[] = { (0x21 << 2) | 3, 0xa1, 0x98, 0x00, 0x00 };
	struct fp_number fp = fp_buftofp(buffer, 9, 25, 0);
	ck_assert_int_eq(fp.integer.value, -47);
	ck_assert_int_eq(fp.integer.bits, 9);
	ck_assert_int_eq((fp.fraction.value * 10000000) >> fp.fraction.bits, 2031250);
	ck_assert_int_eq(fp.fraction.bits, 25);
	ck_assert_int_eq(fp.fraction.precision, 24);
}
END_TEST

/* This is some corner case */
START_TEST(test_buffer_parsing_positive_float_2) {
	unsigned char buffer[] = { 0x40, 0x9c, 0x80, 0x00, 0x00 };
	struct fp_number fp = fp_buftofp(buffer, 9, 25, 0);
	ck_assert_int_eq(fp.integer.value, 78);
}
END_TEST

START_TEST(test_buffer_parsing_positive_float_with_shift) {
	unsigned char buffer[] = { 0x77, 0xc8, 0x45, 0xe6, 0x80, 0x00, 0x07 };
	struct fp_number fp = fp_buftofp(buffer, 9, 25, 12);
	ck_assert_int_eq(fp.integer.value, 47);
	ck_assert_int_eq(fp.integer.bits, 9);
	ck_assert_int_eq((fp.fraction.value * 10000000) >> fp.fraction.bits, 2031250);
	ck_assert_int_eq(fp.fraction.bits, 25);
	ck_assert_int_eq(fp.fraction.precision, 24);
}
END_TEST

START_TEST(test_buffer_parsing_negative_float_with_shift) {
	unsigned char buffer[] = { 0x00, 0xff, (0x21 << 2) | 3, 0xa1, 0x98, 0x00, 0x00 };
	struct fp_number fp = fp_buftofp(buffer, 9, 25, 16);
	ck_assert_int_eq(fp.integer.value, -47);
	ck_assert_int_eq(fp.integer.bits, 9);
	ck_assert_int_eq((fp.fraction.value * 10000000) >> fp.fraction.bits, 2031250);
	ck_assert_int_eq(fp.fraction.bits, 25);
	ck_assert_int_eq(fp.fraction.precision, 24);
}
END_TEST

START_TEST(test_negate_positive) {
	struct fp_number fp = fp_strtofp("14.5", NULL, 9, 25);
	struct fp_number nfp = fp_negate(fp);
	ck_assert_int_eq(nfp.integer.value, -14);
	ck_assert_int_eq(fp.fraction.value, nfp.fraction.value);
	ck_assert_str_eq(fp_fptostr(nfp, NULL), "-14.5");
}
END_TEST

START_TEST(test_negate_negative) {
	struct fp_number fp = fp_strtofp("-14.5", NULL, 9, 25);
	struct fp_number nfp = fp_negate(fp);
	ck_assert_int_eq(nfp.integer.value, 14);
	ck_assert_int_eq(fp.fraction.value, nfp.fraction.value);
	ck_assert_str_eq(fp_fptostr(nfp, NULL), "14.5");
}
END_TEST

#endif

Suite *
fixedpoint_suite(void)
{
	Suite *s = suite_create("Fixed point representation");

#ifdef ENABLE_LLDPMED
	TCase *tc_fp = tcase_create("Fixed point representation");
	tcase_add_test(tc_fp, test_string_parsing_suffix);
	tcase_add_test(tc_fp, test_string_parsing_positive_int);
	tcase_add_test(tc_fp, test_string_parsing_negative_int);
	tcase_add_test(tc_fp, test_string_parsing_no_fract_part);
	tcase_add_test(tc_fp, test_string_parsing_no_int_part);
	tcase_add_test(tc_fp, test_string_parsing_positive_int_overflow);
	tcase_add_test(tc_fp, test_string_parsing_negative_int_overflow);
	tcase_add_test(tc_fp, test_string_parsing_positive_float);
	tcase_add_test(tc_fp, test_string_parsing_negative_float);
	tcase_add_test(tc_fp, test_string_representation_positive_int);
	tcase_add_test(tc_fp, test_string_representation_negative_int);
	tcase_add_test(tc_fp, test_string_representation_positive_float);
	tcase_add_test(tc_fp, test_string_representation_positive_float_with_leading_zero);
	tcase_add_test(tc_fp, test_string_representation_negative_float);
	tcase_add_test(tc_fp, test_buffer_representation_positive_float);
	tcase_add_test(tc_fp, test_buffer_representation_negative_float);
	tcase_add_test(tc_fp, test_buffer_representation_with_shift);
	tcase_add_test(tc_fp, test_buffer_representation_altitude);
	tcase_add_test(tc_fp, test_buffer_parsing_positive_float);
	tcase_add_test(tc_fp, test_buffer_parsing_positive_float_2);
	tcase_add_test(tc_fp, test_buffer_parsing_negative_float);
	tcase_add_test(tc_fp, test_buffer_parsing_positive_float_with_shift);
	tcase_add_test(tc_fp, test_buffer_parsing_negative_float_with_shift);
	tcase_add_test(tc_fp, test_negate_positive);
	tcase_add_test(tc_fp, test_negate_negative);
	suite_add_tcase(s, tc_fp);
#endif

	return s;
}

/* Disable leak detection sanitizer */
int __lsan_is_turned_off() {
	return 1;
}

int
main()
{
	int number_failed;
	Suite *s = fixedpoint_suite();
	SRunner *sr = srunner_create(s);
	srunner_run_all(sr, CK_ENV);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
