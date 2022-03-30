// SPDX-License-Identifier: GPL-2.0+
/*
 * Unit tests for Unicode functions
 *
 * Copyright (c) 2018 Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#include <common.h>
#include <charset.h>
#include <command.h>
#include <errno.h>
#include <test/test.h>
#include <test/suites.h>
#include <test/ut.h>

/* Linker list entry for a Unicode test */
#define UNICODE_TEST(_name) UNIT_TEST(_name, 0, unicode_test)

/* Constants c1-c4 and d1-d4 encode the same letters */

/* Six characters translating to one utf-8 byte each. */
static const u16 c1[] = {0x55, 0x2d, 0x42, 0x6f, 0x6f, 0x74, 0x00};
/* One character translating to two utf-8 bytes */
static const u16 c2[] = {0x6b, 0x61, 0x66, 0x62, 0xe1, 0x74, 0x75, 0x72, 0x00};
/* Three characters translating to three utf-8 bytes each */
static const u16 c3[] = {0x6f5c, 0x6c34, 0x8266, 0x00};
/* Three letters translating to four utf-8 bytes each */
static const u16 c4[] = {0xd801, 0xdc8d, 0xd801, 0xdc96, 0xd801, 0xdc87,
			 0x0000};

/* Illegal utf-16 strings */
static const u16 i1[] = {0x69, 0x31, 0xdc87, 0x6c, 0x00};
static const u16 i2[] = {0x69, 0x32, 0xd801, 0xd801, 0x6c, 0x00};
static const u16 i3[] = {0x69, 0x33, 0xd801, 0x00};

/* Six characters translating to one utf-16 word each. */
static const char d1[] = {0x55, 0x2d, 0x42, 0x6f, 0x6f, 0x74, 0x00};
/* Eight characters translating to one utf-16 word each */
static const char d2[] = {0x6b, 0x61, 0x66, 0x62, 0xc3, 0xa1, 0x74, 0x75,
			  0x72, 0x00};
/* Three characters translating to one utf-16 word each */
static const char d3[] = {0xe6, 0xbd, 0x9c, 0xe6, 0xb0, 0xb4, 0xe8, 0x89,
			  0xa6, 0x00};
/* Three letters translating to two utf-16 word each */
static const char d4[] = {0xf0, 0x90, 0x92, 0x8d, 0xf0, 0x90, 0x92, 0x96,
			  0xf0, 0x90, 0x92, 0x87, 0x00};

/* Illegal utf-8 strings */
static const char j1[] = {0x6a, 0x31, 0xa1, 0x6c, 0x00};
static const char j2[] = {0x6a, 0x32, 0xc3, 0xc3, 0x6c, 0x00};
static const char j3[] = {0x6a, 0x33, 0xf0, 0x90, 0xf0, 0x00};

static int unicode_test_u16_strdup(struct unit_test_state *uts)
{
	u16 *copy = u16_strdup(c4);

	ut_assert(copy != c4);
	ut_assert(!memcmp(copy, c4, sizeof(c4)));
	free(copy);
	return 0;
}
UNICODE_TEST(unicode_test_u16_strdup);

static int unicode_test_u16_strcpy(struct unit_test_state *uts)
{
	u16 *r;
	u16 copy[10];

	r = u16_strcpy(copy, c1);
	ut_assert(r == copy);
	ut_assert(!memcmp(copy, c1, sizeof(c1)));
	return 0;
}
UNICODE_TEST(unicode_test_u16_strcpy);

/* U-Boot uses UTF-16 strings in the EFI context only. */
#if CONFIG_IS_ENABLED(EFI_LOADER) && !defined(API_BUILD)
static int unicode_test_string16(struct unit_test_state *uts)
{
	char buf[20];

	/* Test length and precision */
	memset(buf, 0xff, sizeof(buf));
	sprintf(buf, "%8.6ls", c2);
	ut_asserteq(' ', buf[1]);
	ut_assert(!strncmp(&buf[2], d2, 7));
	ut_assert(!buf[9]);

	memset(buf, 0xff, sizeof(buf));
	sprintf(buf, "%8.6ls", c4);
	ut_asserteq(' ', buf[4]);
	ut_assert(!strncmp(&buf[5], d4, 12));
	ut_assert(!buf[17]);

	memset(buf, 0xff, sizeof(buf));
	sprintf(buf, "%-8.2ls", c4);
	ut_asserteq(' ', buf[8]);
	ut_assert(!strncmp(buf, d4, 8));
	ut_assert(!buf[14]);

	/* Test handling of illegal utf-16 sequences */
	memset(buf, 0xff, sizeof(buf));
	sprintf(buf, "%ls", i1);
	ut_asserteq_str("i1?l", buf);

	memset(buf, 0xff, sizeof(buf));
	sprintf(buf, "%ls", i2);
	ut_asserteq_str("i2?l", buf);

	memset(buf, 0xff, sizeof(buf));
	sprintf(buf, "%ls", i3);
	ut_asserteq_str("i3?", buf);

	return 0;
}
UNICODE_TEST(unicode_test_string16);
#endif

static int unicode_test_utf8_get(struct unit_test_state *uts)
{
	const char *s;
	s32 code;
	int i;

	/* Check characters less than 0x800 */
	s = d2;
	for (i = 0; i < 8; ++i) {
		code = utf8_get((const char **)&s);
		/* c2 is the utf-8 encoding of d2 */
		ut_asserteq(c2[i], code);
		if (!code)
			break;
	}
	ut_asserteq_ptr(s, d2 + 9)

	/* Check characters less than 0x10000 */
	s = d3;
	for (i = 0; i < 4; ++i) {
		code = utf8_get((const char **)&s);
		/* c3 is the utf-8 encoding of d3 */
		ut_asserteq(c3[i], code);
		if (!code)
			break;
	}
	ut_asserteq_ptr(s, d3 + 9)

	/* Check character greater 0xffff */
	s = d4;
	code = utf8_get((const char **)&s);
	ut_asserteq(0x0001048d, code);
	ut_asserteq_ptr(s, d4 + 4);

	return 0;
}
UNICODE_TEST(unicode_test_utf8_get);

static int unicode_test_utf8_put(struct unit_test_state *uts)
{
	char buffer[8] = { 0, };
	char *pos;

	/* Commercial at, translates to one character */
	pos = buffer;
	ut_assert(!utf8_put('@', &pos))
	ut_asserteq(1, pos - buffer);
	ut_asserteq('@', buffer[0]);
	ut_assert(!buffer[1]);

	/* Latin letter G with acute, translates to two charactes */
	pos = buffer;
	ut_assert(!utf8_put(0x1f4, &pos));
	ut_asserteq(2, pos - buffer);
	ut_asserteq_str("\xc7\xb4", buffer);

	/* Tagalog letter i, translates to three characters */
	pos = buffer;
	ut_assert(!utf8_put(0x1701, &pos));
	ut_asserteq(3, pos - buffer);
	ut_asserteq_str("\xe1\x9c\x81", buffer);

	/* Hamster face, translates to four characters */
	pos = buffer;
	ut_assert(!utf8_put(0x1f439, &pos));
	ut_asserteq(4, pos - buffer);
	ut_asserteq_str("\xf0\x9f\x90\xb9", buffer);

	/* Illegal code */
	pos = buffer;
	ut_asserteq(-1, utf8_put(0xd888, &pos));

	return 0;
}
UNICODE_TEST(unicode_test_utf8_put);

static int unicode_test_utf8_utf16_strlen(struct unit_test_state *uts)
{
	ut_asserteq(6, utf8_utf16_strlen(d1));
	ut_asserteq(8, utf8_utf16_strlen(d2));
	ut_asserteq(3, utf8_utf16_strlen(d3));
	ut_asserteq(6, utf8_utf16_strlen(d4));

	/* illegal utf-8 sequences */
	ut_asserteq(4, utf8_utf16_strlen(j1));
	ut_asserteq(4, utf8_utf16_strlen(j2));
	ut_asserteq(3, utf8_utf16_strlen(j3));

	return 0;
}
UNICODE_TEST(unicode_test_utf8_utf16_strlen);

static int unicode_test_utf8_utf16_strnlen(struct unit_test_state *uts)
{
	ut_asserteq(3, utf8_utf16_strnlen(d1, 3));
	ut_asserteq(6, utf8_utf16_strnlen(d1, 13));
	ut_asserteq(6, utf8_utf16_strnlen(d2, 6));
	ut_asserteq(2, utf8_utf16_strnlen(d3, 2));
	ut_asserteq(4, utf8_utf16_strnlen(d4, 2));
	ut_asserteq(6, utf8_utf16_strnlen(d4, 3));

	/* illegal utf-8 sequences */
	ut_asserteq(4, utf8_utf16_strnlen(j1, 16));
	ut_asserteq(4, utf8_utf16_strnlen(j2, 16));
	ut_asserteq(3, utf8_utf16_strnlen(j3, 16));

	return 0;
}
UNICODE_TEST(unicode_test_utf8_utf16_strnlen);

/**
 * ut_u16_strcmp() - Compare to u16 strings.
 *
 * @a1:		first string
 * @a2:		second string
 * @count:	number of u16 to compare
 * Return:	-1 if a1 < a2, 0 if a1 == a2, 1 if a1 > a2
 */
static int unicode_test_u16_strcmp(const u16 *a1, const u16 *a2, size_t count)
{
	for (; (*a1 || *a2) && count; ++a1, ++a2, --count) {
		if (*a1 < *a2)
			return -1;
		if (*a1 > *a2)
			return 1;
	}
	return 0;
}

static int unicode_test_utf8_utf16_strcpy(struct unit_test_state *uts)
{
	u16 buf[16];
	u16 *pos;

	pos = buf;
	utf8_utf16_strcpy(&pos, d1);
	ut_asserteq(6, pos - buf);
	ut_assert(!unicode_test_u16_strcmp(buf, c1, SIZE_MAX));

	pos = buf;
	utf8_utf16_strcpy(&pos, d2);
	ut_asserteq(8, pos - buf);
	ut_assert(!unicode_test_u16_strcmp(buf, c2, SIZE_MAX));

	pos = buf;
	utf8_utf16_strcpy(&pos, d3);
	ut_asserteq(3, pos - buf);
	ut_assert(!unicode_test_u16_strcmp(buf, c3, SIZE_MAX));

	pos = buf;
	utf8_utf16_strcpy(&pos, d4);
	ut_asserteq(6, pos - buf);
	ut_assert(!unicode_test_u16_strcmp(buf, c4, SIZE_MAX));

	/* Illegal utf-8 strings */
	pos = buf;
	utf8_utf16_strcpy(&pos, j1);
	ut_asserteq(4, pos - buf);
	ut_assert(!unicode_test_u16_strcmp(buf, L"j1?l", SIZE_MAX));

	pos = buf;
	utf8_utf16_strcpy(&pos, j2);
	ut_asserteq(4, pos - buf);
	ut_assert(!unicode_test_u16_strcmp(buf, L"j2?l", SIZE_MAX));

	pos = buf;
	utf8_utf16_strcpy(&pos, j3);
	ut_asserteq(3, pos - buf);
	ut_assert(!unicode_test_u16_strcmp(buf, L"j3?", SIZE_MAX));

	return 0;
}
UNICODE_TEST(unicode_test_utf8_utf16_strcpy);

static int unicode_test_utf8_utf16_strncpy(struct unit_test_state *uts)
{
	u16 buf[16];
	u16 *pos;

	pos = buf;
	memset(buf, 0, sizeof(buf));
	utf8_utf16_strncpy(&pos, d1, 4);
	ut_asserteq(4, pos - buf);
	ut_assert(!buf[4]);
	ut_assert(!unicode_test_u16_strcmp(buf, c1, 4));

	pos = buf;
	memset(buf, 0, sizeof(buf));
	utf8_utf16_strncpy(&pos, d2, 10);
	ut_asserteq(8, pos - buf);
	ut_assert(buf[4]);
	ut_assert(!unicode_test_u16_strcmp(buf, c2, SIZE_MAX));

	pos = buf;
	memset(buf, 0, sizeof(buf));
	utf8_utf16_strncpy(&pos, d3, 2);
	ut_asserteq(2, pos - buf);
	ut_assert(!buf[2]);
	ut_assert(!unicode_test_u16_strcmp(buf, c3, 2));

	pos = buf;
	memset(buf, 0, sizeof(buf));
	utf8_utf16_strncpy(&pos, d4, 2);
	ut_asserteq(4, pos - buf);
	ut_assert(!buf[4]);
	ut_assert(!unicode_test_u16_strcmp(buf, c4, 4));

	pos = buf;
	memset(buf, 0, sizeof(buf));
	utf8_utf16_strncpy(&pos, d4, 10);
	ut_asserteq(6, pos - buf);
	ut_assert(buf[5]);
	ut_assert(!unicode_test_u16_strcmp(buf, c4, SIZE_MAX));

	return 0;
}
UNICODE_TEST(unicode_test_utf8_utf16_strncpy);

static int unicode_test_utf16_get(struct unit_test_state *uts)
{
	const u16 *s;
	s32 code;
	int i;

	/* Check characters less than 0x10000 */
	s = c2;
	for (i = 0; i < 9; ++i) {
		code = utf16_get((const u16 **)&s);
		ut_asserteq(c2[i], code);
		if (!code)
			break;
	}
	ut_asserteq_ptr(c2 + 8, s);

	/* Check character greater 0xffff */
	s = c4;
	code = utf16_get((const u16 **)&s);
	ut_asserteq(0x0001048d, code);
	ut_asserteq_ptr(c4 + 2, s);

	return 0;
}
UNICODE_TEST(unicode_test_utf16_get);

static int unicode_test_utf16_put(struct unit_test_state *uts)
{
	u16 buffer[4] = { 0, };
	u16 *pos;

	/* Commercial at, translates to one word */
	pos = buffer;
	ut_assert(!utf16_put('@', &pos));
	ut_asserteq(1, pos - buffer);
	ut_asserteq((u16)'@', buffer[0]);
	ut_assert(!buffer[1]);

	/* Hamster face, translates to two words */
	pos = buffer;
	ut_assert(!utf16_put(0x1f439, &pos));
	ut_asserteq(2, pos - buffer);
	ut_asserteq((u16)0xd83d, buffer[0]);
	ut_asserteq((u16)0xdc39, buffer[1]);
	ut_assert(!buffer[2]);

	/* Illegal code */
	pos = buffer;
	ut_asserteq(-1, utf16_put(0xd888, &pos));

	return 0;
}
UNICODE_TEST(unicode_test_utf16_put);

static int unicode_test_utf16_strnlen(struct unit_test_state *uts)
{
	ut_asserteq(3, utf16_strnlen(c1, 3));
	ut_asserteq(6, utf16_strnlen(c1, 13));
	ut_asserteq(6, utf16_strnlen(c2, 6));
	ut_asserteq(2, utf16_strnlen(c3, 2));
	ut_asserteq(2, utf16_strnlen(c4, 2));
	ut_asserteq(3, utf16_strnlen(c4, 3));

	/* illegal utf-16 word sequences */
	ut_asserteq(4, utf16_strnlen(i1, 16));
	ut_asserteq(4, utf16_strnlen(i2, 16));
	ut_asserteq(3, utf16_strnlen(i3, 16));

	return 0;
}
UNICODE_TEST(unicode_test_utf16_strnlen);

static int unicode_test_utf16_utf8_strlen(struct unit_test_state *uts)
{
	ut_asserteq(6, utf16_utf8_strlen(c1));
	ut_asserteq(9, utf16_utf8_strlen(c2));
	ut_asserteq(9, utf16_utf8_strlen(c3));
	ut_asserteq(12, utf16_utf8_strlen(c4));

	/* illegal utf-16 word sequences */
	ut_asserteq(4, utf16_utf8_strlen(i1));
	ut_asserteq(4, utf16_utf8_strlen(i2));
	ut_asserteq(3, utf16_utf8_strlen(i3));

	return 0;
}
UNICODE_TEST(unicode_test_utf16_utf8_strlen);

static int unicode_test_utf16_utf8_strnlen(struct unit_test_state *uts)
{
	ut_asserteq(3, utf16_utf8_strnlen(c1, 3));
	ut_asserteq(6, utf16_utf8_strnlen(c1, 13));
	ut_asserteq(7, utf16_utf8_strnlen(c2, 6));
	ut_asserteq(6, utf16_utf8_strnlen(c3, 2));
	ut_asserteq(8, utf16_utf8_strnlen(c4, 2));
	ut_asserteq(12, utf16_utf8_strnlen(c4, 3));
	return 0;
}
UNICODE_TEST(unicode_test_utf16_utf8_strnlen);

static int unicode_test_utf16_utf8_strcpy(struct unit_test_state *uts)
{
	char buf[16];
	char *pos;

	pos = buf;
	utf16_utf8_strcpy(&pos, c1);
	ut_asserteq(6, pos - buf);
	ut_asserteq_str(d1, buf);

	pos = buf;
	utf16_utf8_strcpy(&pos, c2);
	ut_asserteq(9, pos - buf);
	ut_asserteq_str(d2, buf);

	pos = buf;
	utf16_utf8_strcpy(&pos, c3);
	ut_asserteq(9, pos - buf);
	ut_asserteq_str(d3, buf);

	pos = buf;
	utf16_utf8_strcpy(&pos, c4);
	ut_asserteq(12, pos - buf);
	ut_asserteq_str(d4, buf);

	/* Illegal utf-16 strings */
	pos = buf;
	utf16_utf8_strcpy(&pos, i1);
	ut_asserteq(4, pos - buf);
	ut_asserteq_str("i1?l", buf);

	pos = buf;
	utf16_utf8_strcpy(&pos, i2);
	ut_asserteq(4, pos - buf);
	ut_asserteq_str("i2?l", buf);

	pos = buf;
	utf16_utf8_strcpy(&pos, i3);
	ut_asserteq(3, pos - buf);
	ut_asserteq_str("i3?", buf);

	return 0;
}
UNICODE_TEST(unicode_test_utf16_utf8_strcpy);

static int unicode_test_utf16_utf8_strncpy(struct unit_test_state *uts)
{
	char buf[16];
	char *pos;

	pos = buf;
	memset(buf, 0, sizeof(buf));
	utf16_utf8_strncpy(&pos, c1, 4);
	ut_asserteq(4, pos - buf);
	ut_assert(!buf[4]);
	ut_assert(!strncmp(buf, d1, 4));

	pos = buf;
	memset(buf, 0, sizeof(buf));
	utf16_utf8_strncpy(&pos, c2, 10);
	ut_asserteq(9, pos - buf);
	ut_assert(buf[4]);
	ut_assert(!strncmp(buf, d2, SIZE_MAX));

	pos = buf;
	memset(buf, 0, sizeof(buf));
	utf16_utf8_strncpy(&pos, c3, 2);
	ut_asserteq(6, pos - buf);
	ut_assert(!buf[6]);
	ut_assert(!strncmp(buf, d3, 6));

	pos = buf;
	memset(buf, 0, sizeof(buf));
	utf16_utf8_strncpy(&pos, c4, 2);
	ut_asserteq(8, pos - buf);
	ut_assert(!buf[8]);
	ut_assert(!strncmp(buf, d4, 8));

	pos = buf;
	memset(buf, 0, sizeof(buf));
	utf16_utf8_strncpy(&pos, c4, 10);
	ut_asserteq(12, pos - buf);
	ut_assert(buf[5]);
	ut_assert(!strncmp(buf, d4, SIZE_MAX));

	return 0;
}
UNICODE_TEST(unicode_test_utf16_utf8_strncpy);

static int unicode_test_utf_to_lower(struct unit_test_state *uts)
{
	ut_asserteq('@', utf_to_lower('@'));
	ut_asserteq('a', utf_to_lower('A'));
	ut_asserteq('z', utf_to_lower('Z'));
	ut_asserteq('[', utf_to_lower('['));
	ut_asserteq('m', utf_to_lower('m'));
	/* Latin letter O with diaresis (umlaut) */
	ut_asserteq(0x00f6, utf_to_lower(0x00d6));
#ifdef CONFIG_EFI_UNICODE_CAPITALIZATION
	/* Cyrillic letter I*/
	ut_asserteq(0x0438, utf_to_lower(0x0418));
#endif
	return 0;
}
UNICODE_TEST(unicode_test_utf_to_lower);

static int unicode_test_utf_to_upper(struct unit_test_state *uts)
{
	ut_asserteq('`', utf_to_upper('`'));
	ut_asserteq('A', utf_to_upper('a'));
	ut_asserteq('Z', utf_to_upper('z'));
	ut_asserteq('{', utf_to_upper('{'));
	ut_asserteq('M', utf_to_upper('M'));
	/* Latin letter O with diaresis (umlaut) */
	ut_asserteq(0x00d6, utf_to_upper(0x00f6));
#ifdef CONFIG_EFI_UNICODE_CAPITALIZATION
	/* Cyrillic letter I */
	ut_asserteq(0x0418, utf_to_upper(0x0438));
#endif
	return 0;
}
UNICODE_TEST(unicode_test_utf_to_upper);

int do_ut_unicode(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct unit_test *tests = ll_entry_start(struct unit_test, unicode_test);
	const int n_ents = ll_entry_count(struct unit_test, unicode_test);

	return cmd_ut_category("Unicode", tests, n_ents, argc, argv);
}
