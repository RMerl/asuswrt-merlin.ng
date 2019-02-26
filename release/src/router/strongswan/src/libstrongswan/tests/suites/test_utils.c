/*
 * Copyright (C) 2013-2015 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include "test_suite.h"

#include <library.h>
#include <utils/utils.h>
#include <ipsec/ipsec_types.h>
#include <credentials/keys/public_key.h>

#include <time.h>

/*******************************************************************************
 * object storage on lib
 */

START_TEST(test_objects)
{
	char *k1 = "key1", *k2 = "key2";
	char *v1 = "val1", *val;

	ck_assert(lib->get(lib, k1) == NULL);

	ck_assert(lib->set(lib, k1, v1));
	ck_assert(!lib->set(lib, k1, v1));

	val = lib->get(lib, k1);
	ck_assert(val != NULL);
	ck_assert(streq(val, v1));

	ck_assert(lib->set(lib, k1, NULL));
	ck_assert(!lib->set(lib, k2, NULL));

	ck_assert(lib->get(lib, k1) == NULL);
}
END_TEST

/*******************************************************************************
 * test return_... functions
 */

START_TEST(test_return_functions)
{
	ck_assert(return_null() == NULL);
	ck_assert(return_null("asdf", 5, NULL, 1, "qwer") == NULL);

	ck_assert(return_true() == TRUE);
	ck_assert(return_true("asdf", 5, NULL, 1, "qwer") == TRUE);

	ck_assert(return_false() == FALSE);
	ck_assert(return_false("asdf", 5, NULL, 1, "qwer") == FALSE);

	ck_assert(return_failed() == FAILED);
	ck_assert(return_failed("asdf", 5, NULL, 1, "qwer") == FAILED);

	ck_assert(return_success() == SUCCESS);
	ck_assert(return_success("asdf", 5, NULL, 1, "qwer") == SUCCESS);

	/* just make sure this works */
	nop();
	nop("asdf", 5, NULL, 1, "qwer");
}
END_TEST

/*******************************************************************************
 * timeval_add_ms
 */

START_TEST(test_timeval_add_ms)
{
	timeval_t tv;

	tv.tv_sec = 0;
	tv.tv_usec = 0;
	timeval_add_ms(&tv, 0);
	ck_assert_int_eq(tv.tv_sec, 0);
	ck_assert_int_eq(tv.tv_usec, 0);

	timeval_add_ms(&tv, 1);
	ck_assert_int_eq(tv.tv_sec, 0);
	ck_assert_int_eq(tv.tv_usec, 1000);

	timeval_add_ms(&tv, 0);
	ck_assert_int_eq(tv.tv_sec, 0);
	ck_assert_int_eq(tv.tv_usec, 1000);

	timeval_add_ms(&tv, 999);
	ck_assert_int_eq(tv.tv_sec, 1);
	ck_assert_int_eq(tv.tv_usec, 0);

	timeval_add_ms(&tv, 0);
	ck_assert_int_eq(tv.tv_sec, 1);
	ck_assert_int_eq(tv.tv_usec, 0);

	timeval_add_ms(&tv, 1000);
	ck_assert_int_eq(tv.tv_sec, 2);
	ck_assert_int_eq(tv.tv_usec, 0);

	timeval_add_ms(&tv, 1500);
	ck_assert_int_eq(tv.tv_sec, 3);
	ck_assert_int_eq(tv.tv_usec, 500000);
}
END_TEST

/*******************************************************************************
 * timespan_from_string
 */

static struct {
	char *s;
	char *u;
	bool v;
	time_t t;
} ts_data[] = {
	{NULL,	NULL,	FALSE,	0},
	{"",	NULL,	FALSE,	0},
	{"a",	NULL,	FALSE,	0},
	{"0",	NULL,	TRUE,	0},
	{"5",	NULL,	TRUE,	5},
	{"5s",	NULL,	TRUE,	5},
	{"5m",	NULL,	TRUE,	300},
	{"5ms",	NULL,	TRUE,	300},
	{"5h",	NULL,	TRUE,	18000},
	{"5d",	NULL,	TRUE,	432000},
	{"5x",	NULL,	FALSE,	0},
	{"5",	"",		TRUE,	5},
	{"5",	"m",	TRUE,	300},
	{"5",	"ms",	TRUE,	300},
	{"5",	"x",	FALSE,	0},
	{"5x",	"m",	FALSE,	0},
	{"18446744073709551616",	NULL,	FALSE,	0},
};

START_TEST(test_timespan_from_string)
{
	time_t val = 42;

	ck_assert(timespan_from_string(ts_data[_i].s, ts_data[_i].u,
								   NULL) == ts_data[_i].v);
	ck_assert(timespan_from_string(ts_data[_i].s, ts_data[_i].u,
								   &val) == ts_data[_i].v);
	if (ts_data[_i].v)
	{
		ck_assert_int_eq(val, ts_data[_i].t);
	}
	else
	{
		ck_assert_int_eq(val, 42);
	}
}
END_TEST

/*******************************************************************************
 * htoun/untoh
 */

START_TEST(test_htoun)
{
	chunk_t net64, expected;
	uint16_t host16 = 513;
	uint32_t net16 = 0, host32 = 67305985;
	uint64_t net32 = 0, host64 = 578437695752307201ULL;

	net64 = chunk_alloca(16);
	memset(net64.ptr, 0, net64.len);

	expected = chunk_from_chars(0x00, 0x02, 0x01, 0x00);
	htoun16((char*)&net16 + 1, host16);
	ck_assert(chunk_equals(expected, chunk_from_thing(net16)));

	expected = chunk_from_chars(0x00, 0x00, 0x04, 0x03, 0x02, 0x01, 0x00, 0x00);
	htoun32((uint16_t*)&net32 + 1, host32);
	ck_assert(chunk_equals(expected, chunk_from_thing(net32)));

	expected = chunk_from_chars(0x00, 0x00, 0x00, 0x00,
								0x08, 0x07, 0x06, 0x05,
								0x04, 0x03, 0x02, 0x01,
								0x00, 0x00, 0x00, 0x00);
	htoun64((uint32_t*)net64.ptr + 1, host64);
	ck_assert(chunk_equals(expected, net64));
}
END_TEST

START_TEST(test_untoh)
{
	chunk_t net;
	uint16_t host16;
	uint32_t host32;
	uint64_t host64;

	net = chunk_from_chars(0x00, 0x02, 0x01, 0x00);
	host16 = untoh16(net.ptr + 1);
	ck_assert(host16 == 513);

	net = chunk_from_chars(0x00, 0x00, 0x04, 0x03, 0x02, 0x01, 0x00, 0x00);
	host32 = untoh32(net.ptr + 2);
	ck_assert(host32 == 67305985);

	net = chunk_from_chars(0x00, 0x00, 0x00, 0x00, 0x08, 0x07, 0x06, 0x05,
						   0x04, 0x03, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00);
	host64 = untoh64(net.ptr + 4);
	ck_assert(host64 == 578437695752307201ULL);
}
END_TEST

/*******************************************************************************
 * pad_len/round_up/down
 */

START_TEST(test_round)
{
	ck_assert_int_eq(pad_len(0, 4), 0);
	ck_assert_int_eq(pad_len(1, 4), 3);
	ck_assert_int_eq(pad_len(2, 4), 2);
	ck_assert_int_eq(pad_len(3, 4), 1);
	ck_assert_int_eq(pad_len(4, 4), 0);
	ck_assert_int_eq(pad_len(5, 4), 3);

	ck_assert_int_eq(round_up(0, 4), 0);
	ck_assert_int_eq(round_up(1, 4), 4);
	ck_assert_int_eq(round_up(2, 4), 4);
	ck_assert_int_eq(round_up(3, 4), 4);
	ck_assert_int_eq(round_up(4, 4), 4);
	ck_assert_int_eq(round_up(5, 4), 8);

	ck_assert_int_eq(round_down(0, 4), 0);
	ck_assert_int_eq(round_down(1, 4), 0);
	ck_assert_int_eq(round_down(2, 4), 0);
	ck_assert_int_eq(round_down(3, 4), 0);
	ck_assert_int_eq(round_down(4, 4), 4);
	ck_assert_int_eq(round_down(5, 4), 4);
}
END_TEST

/*******************************************************************************
 * streq
 */

static struct {
	char *a;
	char *b;
	bool eq;
	bool case_eq;
} streq_data[] = {
	{NULL, NULL, TRUE, TRUE},
	{NULL, "", FALSE, FALSE},
	{"", NULL, FALSE, FALSE},
	{"abc", "", FALSE, FALSE},
	{"abc", "abc", TRUE, TRUE},
	{"abc", "ABC", FALSE, TRUE},
};

START_TEST(test_streq)
{
	bool eq;

	ck_assert(streq(streq_data[_i].a, streq_data[_i].a));
	ck_assert(streq(streq_data[_i].b, streq_data[_i].b));
	eq = streq(streq_data[_i].a, streq_data[_i].b);
	ck_assert(eq == streq_data[_i].eq);

	ck_assert(strcaseeq(streq_data[_i].a, streq_data[_i].a));
	ck_assert(strcaseeq(streq_data[_i].b, streq_data[_i].b));
	eq = strcaseeq(streq_data[_i].a, streq_data[_i].b);
	ck_assert(eq == streq_data[_i].case_eq);
}
END_TEST

/*******************************************************************************
 * strneq
 */

static struct {
	char *a;
	char *b;
	size_t n;
	bool eq;
	bool case_eq;
} strneq_data[] = {
	{NULL, NULL, 0, TRUE, TRUE},
	{NULL, NULL, 10, TRUE, TRUE},
	{NULL, "", 0, FALSE, FALSE},
	{"", NULL, 0, FALSE, FALSE},
	{"abc", "", 0, TRUE, TRUE},
	{"abc", "", 1, FALSE, FALSE},
	{"abc", "ab", 1, TRUE, TRUE},
	{"abc", "ab", 2, TRUE, TRUE},
	{"abc", "ab", 3, FALSE, FALSE},
	{"abc", "abc", 3, TRUE, TRUE},
	{"abc", "abc", 4, TRUE, TRUE},
	{"abc", "abC", 2, TRUE, TRUE},
	{"abc", "abC", 3, FALSE, TRUE},
};

START_TEST(test_strneq)
{
	bool eq;

	ck_assert(strneq(strneq_data[_i].a, strneq_data[_i].a, strneq_data[_i].n));
	ck_assert(strneq(strneq_data[_i].b, strneq_data[_i].b, strneq_data[_i].n));
	eq = strneq(strneq_data[_i].a, strneq_data[_i].b, strneq_data[_i].n);
	ck_assert(eq == strneq_data[_i].eq);

	ck_assert(strncaseeq(strneq_data[_i].a, strneq_data[_i].a,  strneq_data[_i].n));
	ck_assert(strncaseeq(strneq_data[_i].b, strneq_data[_i].b,  strneq_data[_i].n));
	eq = strncaseeq(strneq_data[_i].a, strneq_data[_i].b,  strneq_data[_i].n);
	ck_assert(eq == strneq_data[_i].case_eq);
}
END_TEST

/*******************************************************************************
 * strpfx
 */

static struct {
	char *str;
	char *pfx;
	bool prefix;
	bool case_prefix;
} strpfx_data[] = {
	{"", "", TRUE, TRUE},
	{"abc", "", TRUE, TRUE},
	{"abc", "a", TRUE, TRUE},
	{"abc", "ab", TRUE, TRUE},
	{"abc", "abc", TRUE, TRUE},
	{"abc", "abcd", FALSE, FALSE},
	{"abc", "AB", FALSE, TRUE},
	{"ABC", "ab", FALSE, TRUE},
	{" abc", "abc", FALSE, FALSE},
};

START_TEST(test_strpfx)
{
	bool prefix;

	prefix = strpfx(strpfx_data[_i].str, strpfx_data[_i].pfx);
	ck_assert(prefix == strpfx_data[_i].prefix);
	prefix = strcasepfx(strpfx_data[_i].str, strpfx_data[_i].pfx);
	ck_assert(prefix == strpfx_data[_i].case_prefix);
}
END_TEST

/*******************************************************************************
 * mallac_align/free_align
 */

START_TEST(test_malloc_align)
{
	void *ptr[128][256];
	int size, align;

	for (size = 0; size < countof(ptr); size++)
	{
		for (align = 0; align < countof(ptr[0]); align++)
		{
			ptr[size][align] = malloc_align(size, align);
			if (align)
			{
				ck_assert((uintptr_t)ptr[size][align] % align == 0);
			}
			if (size)
			{
				ck_assert(ptr[size][align]);
				memset(ptr[size][align], 0xEF, size);
			}
		}
	}
	for (size = 0; size < countof(ptr); size++)
	{
		for (align = 0; align < countof(ptr[0]); align++)
		{
			free_align(ptr[size][align]);
		}
	}
}
END_TEST

/*******************************************************************************
 * memxor
 */

static void do_memxor(chunk_t a, chunk_t b, chunk_t exp)
{
	chunk_t dst;

	dst = chunk_clonea(a);
	dst.len = b.len;
	memxor(dst.ptr, b.ptr, b.len);
	ck_assert(chunk_equals(dst, exp));
}

START_TEST(test_memxor)
{
	chunk_t a, b, dst;
	int i;

	a = chunk_alloca(64);
	memset(a.ptr, 0, a.len);
	b = chunk_alloca(64);
	for (i = 0; i < 64; i++)
	{
		b.ptr[i] = i;
		b.len = i;
		do_memxor(a, b, b);
	}
	b.len = 64;
	do_memxor(a, b, b);

	dst = chunk_clonea(a);
	memxor(dst.ptr, b.ptr, b.len);
	ck_assert(chunk_equals(dst, b));

	memxor(dst.ptr, b.ptr, 0);
	memxor(dst.ptr, b.ptr, 1);
	memxor(dst.ptr + 1, b.ptr + 1, 1);
	memxor(dst.ptr + 2, b.ptr + 2, b.len - 2);
	ck_assert(chunk_equals(dst, a));
}
END_TEST

START_TEST(test_memxor_aligned)
{
	uint64_t a = 0, b = 0;
	chunk_t ca, cb;
	int i;

	ca = chunk_from_thing(a);
	cb = chunk_from_thing(b);

	for (i = 0; i < 8; i++)
	{
		cb.ptr[i] = i + 1;
	}

	/* 64-bit aligned */
	memxor(ca.ptr, cb.ptr, 8);
	ck_assert(a == b);
	/* 32-bit aligned source */
	a = 0;
	memxor(ca.ptr, cb.ptr + 4, 4);
	ck_assert(chunk_equals(ca, chunk_from_chars(0x05, 0x06, 0x07, 0x08,
												0x00, 0x00, 0x00, 0x00)));
	/* 16-bit aligned source */
	a = 0;
	memxor(ca.ptr, cb.ptr + 2, 6);
	ck_assert(chunk_equals(ca, chunk_from_chars(0x03, 0x04, 0x05, 0x06,
												0x07, 0x08, 0x00, 0x00)));
	/* 8-bit aligned source */
	a = 0;
	memxor(ca.ptr, cb.ptr + 1, 7);
	ck_assert(chunk_equals(ca, chunk_from_chars(0x02, 0x03, 0x04, 0x05,
												0x06, 0x07, 0x08, 0x00)));
}
END_TEST

/*******************************************************************************
 * memeq/const
 */

static struct {
	char *a;
	char *b;
	size_t n;
	bool res;
} memeq_data[] = {
	{NULL, NULL, 0, TRUE},
	{"a", "b", 0, TRUE},
	{"", "", 1, TRUE},
	{"abcdefgh", "abcdefgh", 8, TRUE},
	{"a", "b", 1, FALSE},
	{"A", "a", 1, FALSE},
	{"\0a", "\0b", 2, FALSE},
	{"abc", "abd", 3, FALSE},
	{"abc", "dbd", 3, FALSE},
	{"abcdefgh", "abcdffgh", 8, FALSE},
	{"abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz",
	 "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz", 52, TRUE},
	{"abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz",
	 "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyy", 52, FALSE},
	{"bbcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz",
	 "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz", 52, FALSE},
};

START_TEST(test_memeq)
{
	ck_assert(memeq(memeq_data[_i].a, memeq_data[_i].b,
					memeq_data[_i].n) == memeq_data[_i].res);
}
END_TEST

START_TEST(test_memeq_const)
{
	ck_assert(memeq_const(memeq_data[_i].a, memeq_data[_i].b,
						  memeq_data[_i].n) == memeq_data[_i].res);
}
END_TEST

/*******************************************************************************
 * memstr
 */

static struct {
	char *haystack;
	char *needle;
	size_t n;
	int offset;
} memstr_data[] = {
	{NULL, NULL, 0, -1},
	{NULL, NULL, 3, -1},
	{NULL, "abc", 0, -1},
	{NULL, "abc", 3, -1},
	{"", "", 0, -1},
	{"abc", NULL, 3, -1},
	{"abc", "", 3, -1},
	{"abc", "abc", 3, 0},
	{" abc", "abc", 4, 1},
	{" abc", "abc", 3, -1},
	{"abcabc", "abc", 6, 0},
	{" abc ", "abc", 5, 1},
};

START_TEST(test_memstr)
{
	char *ret;

	ret = memstr(memstr_data[_i].haystack, memstr_data[_i].needle, memstr_data[_i].n);
	if (memstr_data[_i].offset >= 0)
	{
		ck_assert(ret == memstr_data[_i].haystack + memstr_data[_i].offset);
	}
	else
	{
		ck_assert(ret == NULL);
	}
}
END_TEST

/*******************************************************************************
 * utils_memrchr
 */

static struct {
	char *s;
	int c;
	size_t n;
	int offset;
} memrchr_data[] = {
	{NULL, 'f', 0, -1},
	{NULL, 'f', 3, -1},
	{"", 'f', 0, -1},
	{"", '\0', 1, 0},
	{"foo", '\0', 3, -1},
	{"foo", '\0', 4, 3},
	{"foo", 'f', 3, 0},
	{"foo", 'o', 3, 2},
	{"foo", 'o', 2, 1},
	{"foo", 'o', 1, -1},
	{"foo", 'o', 0, -1},
	{"foo", 'x', 3, -1},
};

START_TEST(test_utils_memrchr)
{
	void *ret;

	ret = utils_memrchr(memrchr_data[_i].s, memrchr_data[_i].c, memrchr_data[_i].n);
	if (memrchr_data[_i].offset >= 0)
	{
		ck_assert(ret == memrchr_data[_i].s + memrchr_data[_i].offset);
	}
	else
	{
		ck_assert(ret == NULL);
	}
}
END_TEST

/*******************************************************************************
 * translate
 */

static struct {
	char *in;
	char *from;
	char *to;
	char *out;
} translate_data[] = {
	{NULL, "", "", NULL},
	{"abc", "", "", "abc"},
	{"abc", "", "x", "abc"},
	{"abc", "x", "", "abc"},
	{"abc", "abc", "xyz", "xyz"},
	{"aabbcc", "abc", "xyz", "xxyyzz"},
	{"abbaccb", "abc", "xyz", "xyyxzzy"},
	{"abxyzc", "abc", "xyz", "xyxyzz"},
	{"abcdef", "abc", "xyz", "xyzdef"},
	{"aaa", "abc", "xyz", "xxx"},
	{"abc", "aaa", "xyz", "xbc"},
	{"abc", "abc", "xxx", "xxx"},
};

START_TEST(test_translate)
{
	char *str, *ret;

	str = strdupnull(translate_data[_i].in);
	ret = translate(str, translate_data[_i].from, translate_data[_i].to);
	ck_assert(ret == str);
	if (ret != translate_data[_i].out)
	{
		ck_assert_str_eq(str, translate_data[_i].out);
	}
	free(str);
}
END_TEST

/*******************************************************************************
 * strreplace
 */

static struct {
	char *in;
	char *out;
	char *search;
	char *replace;
	bool allocated;
} strreplace_data[] = {
	/* invalid arguments */
	{NULL, NULL, NULL, NULL, FALSE},
	{"", "", NULL, NULL, FALSE},
	{"", "", "", NULL, FALSE},
	{"", "", NULL, "", FALSE},
	{"", "", "", "", FALSE},
	{"", "", "", "asdf", FALSE},
	{"", "", "asdf", "", FALSE},
	{"asdf", "asdf", NULL, NULL, FALSE},
	{"asdf", "asdf", "", NULL, FALSE},
	{"asdf", "asdf", NULL, "", FALSE},
	{"asdf", "asdf", "", "", FALSE},
	{"asdf", "asdf", "", "asdf", FALSE},
	{"asdf", "asdf", "asdf", NULL, FALSE},
	{"qwer", "qwer", "", "asdf", FALSE},
	/* replacement shorter */
	{"asdf", "", "asdf", "", TRUE},
	{"asdfasdf", "", "asdf", "", TRUE},
	{"asasdfdf", "asdf", "asdf", "", TRUE},
	{"asdf", "df", "as", "", TRUE},
	{"asdf", "as", "df", "", TRUE},
	{"qwer", "qwer", "asdf", "", FALSE},
	/* replacement same length */
	{"a", "b", "a", "b", TRUE},
	{"aaa", "bbb", "a", "b", TRUE},
	{"aaa", "bbb", "aaa", "bbb", TRUE},
	{"asdf", "asdf", "asdf", "asdf", TRUE},
	{"qwer", "qwer", "asdf", "asdf", FALSE},
	/* replacement longer */
	{"asdf", "asdf", "", "asdf", FALSE},
	{"asdf", "asdfasdf", "asdf", "asdfasdf", TRUE},
	{"asdf", "asdfsdf", "a", "asdf", TRUE},
	{"asdf", "asdasdf", "f", "asdf", TRUE},
	{"aaa", "asdfasdfasdf", "a", "asdf", TRUE},
	{"qwer", "qwer", "asdf", "asdfasdf", FALSE},
	/* real examples */
	{"http://x.org/no/spaces", "http://x.org/no/spaces", " ", "%20", FALSE},
	{"http://x.org/end ", "http://x.org/end%20", " ", "%20", TRUE},
	{" http://x.org/start", "%20http://x.org/start", " ", "%20", TRUE},
	{" http://x.org/both ", "%20http://x.org/both%20", " ", "%20", TRUE},
	{"http://x.org/ /slash", "http://x.org/%20/slash", " ", "%20", TRUE},
	{"http://x.org/   /three", "http://x.org/%20%20%20/three", " ", "%20", TRUE},
	{"http://x.org/      ", "http://x.org/%20%20%20%20%20%20", " ", "%20", TRUE},
	{"http://x.org/%20/encoded", "http://x.org/%20/encoded", " ", "%20", FALSE},
};

START_TEST(test_strreplace)
{
	char *ret;

	ret = strreplace(strreplace_data[_i].in, strreplace_data[_i].search,
					 strreplace_data[_i].replace);
	if (ret && strreplace_data[_i].out)
	{
		ck_assert_str_eq(ret, strreplace_data[_i].out);
	}
	else
	{
		ck_assert(ret == strreplace_data[_i].out);
	}
	if (strreplace_data[_i].allocated)
	{
		ck_assert(ret != strreplace_data[_i].in);
		free(ret);
	}
	else
	{
		ck_assert(ret == strreplace_data[_i].in);
	}
}
END_TEST

/*******************************************************************************
 * path_dirname/basename/absolute
 */

static struct {
	char *path;
	char *dir;
	char *base;
	bool absolute;
} path_data[] = {
	{NULL, ".", ".", FALSE},
	{"", ".", ".", FALSE},
	{".", ".", ".", FALSE},
	{"..", ".", "..", FALSE},
#ifdef WIN32
	{"C:\\", "C:", "C:", TRUE},
	{"X:\\\\", "X:", "X:", TRUE},
	{"foo", ".", "foo", FALSE},
	{"f\\", ".", "f", FALSE},
	{"foo\\", ".", "foo", FALSE},
	{"foo\\\\", ".", "foo", FALSE},
	{"d:\\f", "d:", "f", TRUE},
	{"C:\\f\\", "C:", "f", TRUE},
	{"C:\\foo", "C:", "foo", TRUE},
	{"C:\\foo\\", "C:", "foo", TRUE},
	{"foo\\bar", "foo", "bar", FALSE},
	{"foo\\\\bar", "foo", "bar", FALSE},
	{"C:\\foo\\bar", "C:\\foo", "bar", TRUE},
	{"C:\\foo\\bar\\", "C:\\foo", "bar", TRUE},
	{"C:\\foo\\bar\\baz", "C:\\foo\\bar", "baz", TRUE},
	{"\\foo\\bar", "\\foo", "bar", FALSE},
	{"\\\\foo\\bar", "\\\\foo", "bar", TRUE},
#else /* !WIN32 */
	{"/", "/", "/", TRUE},
	{"//", "/", "/", TRUE},
	{"foo", ".", "foo", FALSE},
	{"f/", ".", "f", FALSE},
	{"foo/", ".", "foo", FALSE},
	{"foo//", ".", "foo", FALSE},
	{"/f", "/", "f", TRUE},
	{"/f/", "/", "f", TRUE},
	{"/foo", "/", "foo", TRUE},
	{"/foo/", "/", "foo", TRUE},
	{"//foo/", "/", "foo", TRUE},
	{"foo/bar", "foo", "bar", FALSE},
	{"foo//bar", "foo", "bar", FALSE},
	{"/foo/bar", "/foo", "bar", TRUE},
	{"/foo/bar/", "/foo", "bar", TRUE},
	{"/foo/bar/baz", "/foo/bar", "baz", TRUE},
#endif
};

START_TEST(test_path_dirname)
{
	char *dir;

	dir = path_dirname(path_data[_i].path);
	ck_assert_str_eq(path_data[_i].dir, dir);
	free(dir);
}
END_TEST

START_TEST(test_path_basename)
{
	char *base;

	base = path_basename(path_data[_i].path);
	ck_assert_str_eq(path_data[_i].base, base);
	free(base);
}
END_TEST

START_TEST(test_path_absolute)
{
	ck_assert(path_data[_i].absolute == path_absolute(path_data[_i].path));
}
END_TEST

/*******************************************************************************
 * time_printf_hook
 */

static struct {
	time_t in;
	bool utc;
	char *out;
} time_data[] = {
	{UNDEFINED_TIME, FALSE, "--- -- --:--:-- ----"},
	{UNDEFINED_TIME, TRUE , "--- -- --:--:-- UTC ----"},
	{1, FALSE, "Jan 01 01:00:01 1970"},
	{1, TRUE , "Jan 01 00:00:01 UTC 1970"},
	{1341150196, FALSE, "Jul 01 15:43:16 2012"},
	{1341150196, TRUE , "Jul 01 13:43:16 UTC 2012"},
};

START_TEST(test_time_printf_hook)
{
	char buf[32];
	int len;

	len = snprintf(buf, sizeof(buf), "%T", &time_data[_i].in, time_data[_i].utc);
	ck_assert(len >= 0 && len < sizeof(buf));
	ck_assert_str_eq(buf, time_data[_i].out);
}
END_TEST

/*******************************************************************************
 * time_delta_printf_hook
 */

static struct {
	time_t a;
	time_t b;
	char *out;
} time_delta_data[] = {
	{0, 0, "0 seconds"},
	{0, 1, "1 second"},
	{0, -1, "1 second"},
	{1, 0, "1 second"},
	{0, 2, "2 seconds"},
	{2, 0, "2 seconds"},
	{0, 60, "60 seconds"},
	{0, 120, "120 seconds"},
	{0, 121, "2 minutes"},
	{0, 3600, "60 minutes"},
	{0, 7200, "120 minutes"},
	{0, 7201, "2 hours"},
	{0, 86400, "24 hours"},
	{0, 172800, "48 hours"},
	{0, 172801, "2 days"},
	{172801, 86400, "24 hours"},
};

START_TEST(test_time_delta_printf_hook)
{
	char buf[16];
	int len;

	len = snprintf(buf, sizeof(buf), "%V", &time_delta_data[_i].a, &time_delta_data[_i].b);
	ck_assert(len >= 0 && len < sizeof(buf));
	ck_assert_str_eq(buf, time_delta_data[_i].out);
}
END_TEST

/*******************************************************************************
 * mark_from_string
 */

static struct {
	char *s;
	bool ok;
	mark_op_t ops;
	mark_t m;
} mark_data[] = {
	{NULL,			FALSE,	MARK_OP_NONE, { 0 }},
	{"",			TRUE,	MARK_OP_NONE, { 0, 0xffffffff }},
	{"/",			TRUE,	MARK_OP_NONE, { 0, 0 }},
	{"42",			TRUE,	MARK_OP_NONE, { 42, 0xffffffff }},
	{"0x42",		TRUE,	MARK_OP_NONE, { 0x42, 0xffffffff }},
	{"x",			FALSE,	MARK_OP_NONE, { 0 }},
	{"42/",			TRUE,	MARK_OP_NONE, { 0, 0 }},
	{"42/0",		TRUE,	MARK_OP_NONE, { 0, 0 }},
	{"42/x",		FALSE,	MARK_OP_NONE, { 0 }},
	{"42/42",		TRUE,	MARK_OP_NONE, { 42, 42 }},
	{"42/0xff",		TRUE,	MARK_OP_NONE, { 42, 0xff }},
	{"0x42/0xff",	TRUE,	MARK_OP_NONE, { 0x42, 0xff }},
	{"/0xff",		TRUE,	MARK_OP_NONE, { 0, 0xff }},
	{"/x",			FALSE,	MARK_OP_NONE, { 0 }},
	{"x/x",			FALSE,	MARK_OP_NONE, { 0 }},
	{"0xfffffff0/0x0000ffff",	TRUE,	MARK_OP_UNIQUE,
		{ 0x0000fff0, 0x0000ffff }},
	{"%unique",					TRUE,	MARK_OP_UNIQUE,
		{ MARK_UNIQUE, 0xffffffff }},
	{"%unique/",				TRUE,	MARK_OP_UNIQUE,
		{ MARK_UNIQUE, 0 }},
	{"%unique",					FALSE,	MARK_OP_NONE,
		{ 0, 0 }},
	{"%unique/0x0000ffff",		TRUE,	MARK_OP_UNIQUE,
		{ MARK_UNIQUE, 0x0000ffff }},
	{"%unique/0xffffffff",		TRUE,	MARK_OP_UNIQUE,
		{ MARK_UNIQUE, 0xffffffff }},
	{"%unique0xffffffffff",		FALSE,	MARK_OP_UNIQUE,
		{ 0, 0 }},
	{"0xffffffff/0x0000ffff",	TRUE,	MARK_OP_UNIQUE,
		{ MARK_UNIQUE, 0x0000ffff }},
	{"0xffffffff/0xffffffff",	TRUE,	MARK_OP_UNIQUE,
		{ MARK_UNIQUE, 0xffffffff }},
	{"%unique-dir",				TRUE,	MARK_OP_UNIQUE,
		{ MARK_UNIQUE_DIR, 0xffffffff }},
	{"%unique-dir/",			TRUE,	MARK_OP_UNIQUE,
		{ MARK_UNIQUE_DIR, 0 }},
	{"%unique-dir",				FALSE,	MARK_OP_NONE,
		{ 0, 0 }},
	{"%unique-dir/0x0000ffff",	TRUE,	MARK_OP_UNIQUE,
		{ MARK_UNIQUE_DIR, 0x0000ffff }},
	{"%unique-dir/0xffffffff",	TRUE,	MARK_OP_UNIQUE,
		{ MARK_UNIQUE_DIR, 0xffffffff }},
	{"%unique-dir0xffffffff",	FALSE,	MARK_OP_UNIQUE,
		{ 0, 0 }},
	{"0xfffffffe/0x0000ffff",	TRUE,	MARK_OP_UNIQUE,
		{ MARK_UNIQUE_DIR, 0x0000ffff }},
	{"0xfffffffe/0xffffffff",	TRUE,	MARK_OP_UNIQUE,
		{ MARK_UNIQUE_DIR, 0xffffffff }},
	{"%unique-/0xffffffff",		FALSE,	MARK_OP_UNIQUE,
		{ 0, 0 }},
	{"%unique-foo/0xffffffff",	FALSE,	MARK_OP_UNIQUE,
		{ 0, 0 }},
	{"%same",					TRUE,	MARK_OP_SAME,
		{ MARK_SAME, 0xffffffff }},
	{"%same/0x0000ffff",		TRUE,	MARK_OP_SAME,
		{ MARK_SAME, 0x0000ffff }},
	{"%%same",					FALSE,	MARK_OP_NONE,
		{ 0, 0 }},
};

START_TEST(test_mark_from_string)
{
	mark_t mark;

	if (mark_from_string(mark_data[_i].s, mark_data[_i].ops, &mark))
	{
		ck_assert_int_eq(mark.value, mark_data[_i].m.value);
		ck_assert_int_eq(mark.mask, mark_data[_i].m.mask);
	}
	else
	{
		ck_assert(!mark_data[_i].ok);
	}
}
END_TEST

/*******************************************************************************
 * signature_schemes_for_key
 */

static struct {
	key_type_t type;
	int size;
	signature_scheme_t expected[7];
} scheme_data[] = {
	{KEY_RSA,   1024, { SIGN_RSA_EMSA_PSS, SIGN_RSA_EMSA_PSS,
						SIGN_RSA_EMSA_PSS, SIGN_RSA_EMSA_PKCS1_SHA2_256,
						SIGN_RSA_EMSA_PKCS1_SHA2_384, SIGN_RSA_EMSA_PKCS1_SHA2_512,
						SIGN_UNKNOWN }},
	{KEY_RSA,   2048, { SIGN_RSA_EMSA_PSS, SIGN_RSA_EMSA_PSS,
						SIGN_RSA_EMSA_PSS, SIGN_RSA_EMSA_PKCS1_SHA2_256,
						SIGN_RSA_EMSA_PKCS1_SHA2_384, SIGN_RSA_EMSA_PKCS1_SHA2_512,
						SIGN_UNKNOWN }},
	{KEY_RSA,   4096, { SIGN_RSA_EMSA_PSS, SIGN_RSA_EMSA_PSS,
						SIGN_RSA_EMSA_PKCS1_SHA2_384, SIGN_RSA_EMSA_PKCS1_SHA2_512,
						SIGN_UNKNOWN }},
	{KEY_RSA,   8192, { SIGN_RSA_EMSA_PSS, SIGN_RSA_EMSA_PKCS1_SHA2_512, SIGN_UNKNOWN }},
	{KEY_ECDSA,  256, { SIGN_ECDSA_WITH_SHA256_DER, SIGN_ECDSA_WITH_SHA384_DER,
						SIGN_ECDSA_WITH_SHA512_DER, SIGN_UNKNOWN }},
	{KEY_ECDSA,  384, { SIGN_ECDSA_WITH_SHA384_DER, SIGN_ECDSA_WITH_SHA512_DER,
						SIGN_UNKNOWN }},
	{KEY_ECDSA,  512, { SIGN_ECDSA_WITH_SHA512_DER, SIGN_UNKNOWN }},
	{KEY_BLISS,  128, { SIGN_BLISS_WITH_SHA2_256, SIGN_BLISS_WITH_SHA2_384,
						SIGN_BLISS_WITH_SHA2_512, SIGN_UNKNOWN }},
	{KEY_BLISS,  192, { SIGN_BLISS_WITH_SHA2_384, SIGN_BLISS_WITH_SHA2_512,
						SIGN_UNKNOWN }},
	{KEY_BLISS,  256, { SIGN_BLISS_WITH_SHA2_512, SIGN_UNKNOWN }},
};

START_TEST(test_signature_schemes_for_key)
{
	enumerator_t  *enumerator;
	signature_params_t *params;
	int i;

	enumerator = signature_schemes_for_key(scheme_data[_i].type, scheme_data[_i].size);
	for (i = 0; scheme_data[_i].expected[i] != SIGN_UNKNOWN; i++)
	{
		ck_assert(enumerator->enumerate(enumerator, &params));
		ck_assert_int_eq(scheme_data[_i].expected[i], params->scheme);
	}
	ck_assert(!enumerator->enumerate(enumerator, &params));
	enumerator->destroy(enumerator);
}
END_TEST

Suite *utils_suite_create()
{
	Suite *s;
	TCase *tc;

	/* force a timezone to match non-UTC conversions */
#ifdef WIN32
	_putenv("TZ=GST-1GDT");
#else
	setenv("TZ", "Europe/Zurich", 1);
#endif
	tzset();

	s = suite_create("utils");

	tc = tcase_create("objects");
	tcase_add_test(tc, test_objects);
	suite_add_tcase(s, tc);

	tc = tcase_create("return functions");
	tcase_add_test(tc, test_return_functions);
	suite_add_tcase(s, tc);

	tc = tcase_create("timeval_add_ms");
	tcase_add_test(tc, test_timeval_add_ms);
	suite_add_tcase(s, tc);

	tc = tcase_create("timespan_from_string");
	tcase_add_loop_test(tc, test_timespan_from_string, 0, countof(ts_data));
	suite_add_tcase(s, tc);

	tc = tcase_create("htoun,untoh");
	tcase_add_test(tc, test_htoun);
	tcase_add_test(tc, test_untoh);
	suite_add_tcase(s, tc);

	tc = tcase_create("round");
	tcase_add_test(tc, test_round);
	suite_add_tcase(s, tc);

	tc = tcase_create("string helper");
	tcase_add_loop_test(tc, test_streq, 0, countof(streq_data));
	tcase_add_loop_test(tc, test_strneq, 0, countof(strneq_data));
	tcase_add_loop_test(tc, test_strpfx, 0, countof(strpfx_data));
	suite_add_tcase(s, tc);

	tc = tcase_create("malloc_align");
	tcase_add_test(tc, test_malloc_align);
	suite_add_tcase(s, tc);

	tc = tcase_create("memxor");
	tcase_add_test(tc, test_memxor);
	tcase_add_test(tc, test_memxor_aligned);
	suite_add_tcase(s, tc);

	tc = tcase_create("memeq");
	tcase_add_loop_test(tc, test_memeq, 0, countof(memeq_data));
	tcase_add_loop_test(tc, test_memeq_const, 0, countof(memeq_data));
	suite_add_tcase(s, tc);

	tc = tcase_create("memstr");
	tcase_add_loop_test(tc, test_memstr, 0, countof(memstr_data));
	suite_add_tcase(s, tc);

	tc = tcase_create("utils_memrchr");
	tcase_add_loop_test(tc, test_utils_memrchr, 0, countof(memrchr_data));
	suite_add_tcase(s, tc);

	tc = tcase_create("translate");
	tcase_add_loop_test(tc, test_translate, 0, countof(translate_data));
	suite_add_tcase(s, tc);

	tc = tcase_create("strreplace");
	tcase_add_loop_test(tc, test_strreplace, 0, countof(strreplace_data));
	suite_add_tcase(s, tc);

	tc = tcase_create("path_dirname");
	tcase_add_loop_test(tc, test_path_dirname, 0, countof(path_data));
	suite_add_tcase(s, tc);

	tc = tcase_create("path_basename");
	tcase_add_loop_test(tc, test_path_basename, 0, countof(path_data));
	suite_add_tcase(s, tc);

	tc = tcase_create("path_absolute");
	tcase_add_loop_test(tc, test_path_absolute, 0, countof(path_data));
	suite_add_tcase(s, tc);

	tc = tcase_create("printf_hooks");
	tcase_add_loop_test(tc, test_time_printf_hook, 0, countof(time_data));
	tcase_add_loop_test(tc, test_time_delta_printf_hook, 0, countof(time_delta_data));
	suite_add_tcase(s, tc);

	tc = tcase_create("mark_from_string");
	tcase_add_loop_test(tc, test_mark_from_string, 0, countof(mark_data));
	suite_add_tcase(s, tc);

	tc = tcase_create("signature_schemes_for_key");
	tcase_add_loop_test(tc, test_signature_schemes_for_key, 0, countof(scheme_data));
	suite_add_tcase(s, tc);

	return s;
}
