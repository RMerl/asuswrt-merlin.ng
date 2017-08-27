/*
 * Copyright (C) 2013 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
 * Hochschule fuer Technik Rapperswil
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

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <utils/chunk.h>
#include <threading/thread.h>

/*******************************************************************************
 * utilities
 */

static void assert_chunk_empty(chunk_t chunk)
{
	ck_assert(chunk.len == 0 && chunk.ptr == NULL);
}

/*******************************************************************************
 * equals
 */

START_TEST(test_chunk_equals)
{
	chunk_t chunk = chunk_from_str("chunk");
	chunk_t chunk_a, chunk_b;

	chunk_a = chunk_empty;
	chunk_b = chunk_empty;
	ck_assert(!chunk_equals(chunk_a, chunk_b));

	chunk_a = chunk;
	ck_assert(!chunk_equals(chunk_a, chunk_b));
	chunk_b = chunk;
	ck_assert(chunk_equals(chunk_a, chunk_b));

	chunk_b = chunk_from_str("asdf");
	ck_assert(!chunk_equals(chunk_a, chunk_b));

	chunk_b = chunk_from_str("chunk");
	ck_assert(chunk_equals(chunk_a, chunk_b));
}
END_TEST

/*******************************************************************************
 * chunk_compare test
 */

static struct {
	int result;
	chunk_t a;
	chunk_t b;
} compare_data[] = {
	{ 0, { NULL, 0 }, { NULL, 0 }},
	{ 0, chunk_from_chars(0x00), chunk_from_chars(0x00)},
	{-1, chunk_from_chars(0x00), chunk_from_chars(0x01)},
	{ 1, chunk_from_chars(0x01), chunk_from_chars(0x00)},
	{ 0, chunk_from_chars(0x00, 0x00), chunk_from_chars(0x00, 0x00)},
	{-1, chunk_from_chars(0x00, 0x00), chunk_from_chars(0x00, 0x01)},
	{ 1, chunk_from_chars(0x00, 0x01), chunk_from_chars(0x00, 0x00)},
	{-1, chunk_from_chars(0x00, 0x00), chunk_from_chars(0x01, 0x00)},
	{ 1, chunk_from_chars(0x01, 0x00), chunk_from_chars(0x00, 0x00)},
	{-1, chunk_from_chars(0xff), chunk_from_chars(0x00, 0x00)},
	{ 1, chunk_from_chars(0x00, 0x00), chunk_from_chars(0xff)},
};

START_TEST(test_compare)
{
	int result, expected;

	result = chunk_compare(compare_data[_i].a, compare_data[_i].b);
	expected = compare_data[_i].result;
	ck_assert((result == 0 && expected == 0) ||
			  (result < 0 && expected < 0) ||
			  (result > 0 && expected > 0));
}
END_TEST

/*******************************************************************************
 * clear
 */

START_TEST(test_chunk_clear)
{
	chunk_t chunk;
	u_char *ptr;
	int i;
	bool cleared = TRUE;

	chunk = chunk_empty;
	chunk_clear(&chunk);
	chunk_free(&chunk);

	chunk = chunk_alloc(64);
	ptr = chunk.ptr;
	for (i = 0; i < 64; i++)
	{
		chunk.ptr[i] = i;
	}
	chunk_clear(&chunk);
	/* check memory area of freed chunk. We can't use ck_assert() for this
	 * test directly, as it might allocate data at the freed area.  comparing
	 * two bytes at once reduces the chances of conflicts if memory got
	 * overwritten already */
	for (i = 0; i < 64; i += 2)
	{
		if (ptr[i] != 0 && ptr[i] == i &&
			ptr[i+1] != 0 && ptr[i+1] == i+1)
		{
			cleared = FALSE;
			break;
		}
	}
	assert_chunk_empty(chunk);
	ck_assert(cleared);
}
END_TEST

/*******************************************************************************
 * chunk_length
 */

START_TEST(test_chunk_length)
{
	chunk_t a, b, c;
	size_t len;

	a = chunk_empty;
	b = chunk_empty;
	c = chunk_empty;
	len = chunk_length("ccc", a, b, c);
	ck_assert_int_eq(len, 0);

	a = chunk_from_str("foo");
	b = chunk_from_str("bar");
	len = chunk_length("ccc", a, b, c);
	ck_assert_int_eq(len, 6);

	len = chunk_length("zcc", a, b, c);
	ck_assert_int_eq(len, 0);

	len = chunk_length("czc", a, b, c);
	ck_assert_int_eq(len, 3);

	a = chunk_from_str("foo");
	b = chunk_from_str("bar");
	c = chunk_from_str("baz");
	len = chunk_length("ccc", a, b, c);
	ck_assert_int_eq(len, 9);
}
END_TEST

/*******************************************************************************
 * chunk_create_cat
 */

START_TEST(test_chunk_create_cat)
{
	chunk_t foo, bar;
	chunk_t a, b, c;
	u_char *ptra, *ptrb;

	foo = chunk_from_str("foo");
	bar = chunk_from_str("bar");

	/* to simplify things we use the chunk_cata macro */

	a = chunk_empty;
	b = chunk_empty;
	c = chunk_cata("cc", a, b);
	ck_assert_int_eq(c.len, 0);
	ck_assert(c.ptr != NULL);

	a = foo;
	b = bar;
	c = chunk_cata("cc", a, b);
	ck_assert_int_eq(c.len, 6);
	ck_assert(chunk_equals(c, chunk_from_str("foobar")));

	a = chunk_clone(foo);
	b = chunk_clone(bar);
	c = chunk_cata("mm", a, b);
	ck_assert_int_eq(c.len, 6);
	ck_assert(chunk_equals(c, chunk_from_str("foobar")));

	a = chunk_clone(foo);
	b = chunk_clone(bar);
	ptra = a.ptr;
	ptrb = b.ptr;
	c = chunk_cata("ss", a, b);
	ck_assert_int_eq(c.len, 6);
	ck_assert(chunk_equals(c, chunk_from_str("foobar")));
	/* check memory area of cleared chunk */
	ck_assert(!chunk_equals(foo, chunk_create(ptra, 3)));
	ck_assert(!chunk_equals(bar, chunk_create(ptrb, 3)));
}
END_TEST

/*******************************************************************************
 * chunk_split
 */

static bool mem_in_chunk(u_char *ptr, chunk_t chunk)
{
	return ptr >= chunk.ptr && ptr < (chunk.ptr + chunk.len);
}

START_TEST(test_chunk_split)
{
	chunk_t foo, bar, foobar;
	chunk_t a, b, c;
	u_char *ptra, *ptrb;

	foo = chunk_from_str("foo");
	bar = chunk_from_str("bar");
	foobar = chunk_from_str("foobar");

	chunk_split(foobar, "aa", 3, &a, 3, &b);
	ck_assert(chunk_equals(a, foo));
	ck_assert(chunk_equals(b, bar));
	ck_assert(!mem_in_chunk(a.ptr, foobar));
	ck_assert(!mem_in_chunk(b.ptr, foobar));
	chunk_free(&a);
	chunk_free(&b);

	chunk_split(foobar, "mm", 3, &a, 3, &b);
	ck_assert(chunk_equals(a, foo));
	ck_assert(chunk_equals(b, bar));
	ck_assert(mem_in_chunk(a.ptr, foobar));
	ck_assert(mem_in_chunk(b.ptr, foobar));

	chunk_split(foobar, "am", 3, &a, 3, &b);
	ck_assert(chunk_equals(a, foo));
	ck_assert(chunk_equals(b, bar));
	ck_assert(!mem_in_chunk(a.ptr, foobar));
	ck_assert(mem_in_chunk(b.ptr, foobar));
	chunk_free(&a);

	a = chunk_alloca(3);
	ptra = a.ptr;
	b = chunk_alloca(3);
	ptrb = b.ptr;
	chunk_split(foobar, "cc", 3, &a, 3, &b);
	ck_assert(chunk_equals(a, foo));
	ck_assert(chunk_equals(b, bar));
	ck_assert(a.ptr == ptra);
	ck_assert(b.ptr == ptrb);

	chunk_split(foobar, "mm", 1, NULL, 2, &a, 2, NULL, 1, &b);
	ck_assert(chunk_equals(a, chunk_from_str("oo")));
	ck_assert(chunk_equals(b, chunk_from_str("r")));

	chunk_split(foobar, "mm", 6, &a, 6, &b);
	ck_assert(chunk_equals(a, foobar));
	assert_chunk_empty(b);

	chunk_split(foobar, "mac", 12, &a, 12, &b, 12, &c);
	ck_assert(chunk_equals(a, foobar));
	assert_chunk_empty(b);
	assert_chunk_empty(c);
}
END_TEST

/*******************************************************************************
 * chunk_skip[_zero]
 */

START_TEST(test_chunk_skip)
{
	chunk_t foobar, a;

	foobar = chunk_from_str("foobar");
	a = foobar;
	a = chunk_skip(a, 0);
	ck_assert(chunk_equals(a, foobar));
	a = chunk_skip(a, 1);
	ck_assert(chunk_equals(a, chunk_from_str("oobar")));
	a = chunk_skip(a, 2);
	ck_assert(chunk_equals(a, chunk_from_str("bar")));
	a = chunk_skip(a, 3);
	assert_chunk_empty(a);

	a = foobar;
	a = chunk_skip(a, 6);
	assert_chunk_empty(a);

	a = foobar;
	a = chunk_skip(a, 10);
	assert_chunk_empty(a);
}
END_TEST

START_TEST(test_chunk_skip_zero)
{
	chunk_t foobar, a;

	a = chunk_empty;
	a = chunk_skip_zero(a);
	assert_chunk_empty(a);

	foobar = chunk_from_str("foobar");
	a = foobar;
	a = chunk_skip_zero(a);
	ck_assert(chunk_equals(a, foobar));

	a = chunk_from_chars(0x00, 0xaa, 0xbb, 0xcc);
	a = chunk_skip_zero(a);
	ck_assert(chunk_equals(a, chunk_from_chars(0xaa, 0xbb, 0xcc)));
	a = chunk_skip_zero(a);
	ck_assert(chunk_equals(a, chunk_from_chars(0xaa, 0xbb, 0xcc)));
}
END_TEST

/*******************************************************************************
 * BASE16 encoding test
 */

START_TEST(test_base16)
{
	/* test vectors from RFC 4648:
	 *
	 * BASE16("") = ""
	 * BASE16("f") = "66"
	 * BASE16("fo") = "666F"
	 * BASE16("foo") = "666F6F"
	 * BASE16("foob") = "666F6F62"
	 * BASE16("fooba") = "666F6F6261"
	 * BASE16("foobar") = "666F6F626172"
	 */
	typedef struct {
		bool upper;
		char *in;
		char *out;
	} testdata_t;

	testdata_t test[] = {
		{TRUE,  "", ""},
		{TRUE,  "f", "66"},
		{TRUE,  "fo", "666F"},
		{TRUE,  "foo", "666F6F"},
		{TRUE,  "foob", "666F6F62"},
		{TRUE,  "fooba", "666F6F6261"},
		{TRUE,  "foobar", "666F6F626172"},
		{FALSE, "", ""},
		{FALSE, "f", "66"},
		{FALSE, "fo", "666f"},
		{FALSE, "foo", "666f6f"},
		{FALSE, "foob", "666f6f62"},
		{FALSE, "fooba", "666f6f6261"},
		{FALSE, "foobar", "666f6f626172"},
	};
	testdata_t test_colon[] = {
		{TRUE,  "", ""},
		{TRUE,  "f", "66"},
		{TRUE,  "fo", "66:6F"},
		{TRUE,  "foo", "66:6F:6F"},
		{FALSE, "foob", "66:6f:6f:62"},
		{FALSE, "fooba", "66:6f:6f:62:61"},
		{FALSE, "foobar", "66:6f:6f:62:61:72"},
		{FALSE, "foobar", "66:6f6f:6261:72"},
	};
	int i;

	for (i = 0; i < countof(test); i++)
	{
		chunk_t out;

		out = chunk_to_hex(chunk_create(test[i].in, strlen(test[i].in)), NULL,
						   test[i].upper);
		ck_assert_str_eq(out.ptr, test[i].out);
		free(out.ptr);
	}

	for (i = 0; i < countof(test); i++)
	{
		chunk_t out;

		out = chunk_from_hex(chunk_create(test[i].out, strlen(test[i].out)), NULL);
		fail_unless(strneq(out.ptr, test[i].in, out.len),
					"base16 conversion error - should '%s', is %#B",
					test[i].in, &out);
		free(out.ptr);
	}

	for (i = 0; i < countof(test_colon); i++)
	{
		chunk_t out;

		out = chunk_from_hex(chunk_create(test_colon[i].out, strlen(test_colon[i].out)), NULL);
		fail_unless(strneq(out.ptr, test_colon[i].in, out.len),
					"base16 conversion error - should '%s', is %#B",
					test_colon[i].in, &out);
		free(out.ptr);
	}
}
END_TEST

/*******************************************************************************
 * BASE64 encoding test
 */

START_TEST(test_base64)
{
	/* test vectors from RFC 4648:
	 *
	 * BASE64("") = ""
	 * BASE64("f") = "Zg=="
	 * BASE64("fo") = "Zm8="
	 * BASE64("foo") = "Zm9v"
	 * BASE64("foob") = "Zm9vYg=="
	 * BASE64("fooba") = "Zm9vYmE="
	 * BASE64("foobar") = "Zm9vYmFy"
	 */
	typedef struct {
		char *in;
		char *out;
	} testdata_t;

	testdata_t test[] = {
		{"", ""},
		{"f", "Zg=="},
		{"fo", "Zm8="},
		{"foo", "Zm9v"},
		{"foob", "Zm9vYg=="},
		{"fooba", "Zm9vYmE="},
		{"foobar", "Zm9vYmFy"},
	};
	int i;

	for (i = 0; i < countof(test); i++)
	{
		chunk_t out;

		out = chunk_to_base64(chunk_create(test[i].in, strlen(test[i].in)), NULL);
		ck_assert_str_eq(out.ptr, test[i].out);
		free(out.ptr);
	}

	for (i = 0; i < countof(test); i++)
	{
		chunk_t out;

		out = chunk_from_base64(chunk_create(test[i].out, strlen(test[i].out)), NULL);
		fail_unless(strneq(out.ptr, test[i].in, out.len),
					"base64 conversion error - should '%s', is %#B",
					test[i].in, &out);
		free(out.ptr);
	}
}
END_TEST

/*******************************************************************************
 * BASE32 encoding test
 */

START_TEST(test_base32)
{
	/* test vectors from RFC 4648:
	 *
	 * BASE32("") = ""
	 * BASE32("f") = "MY======"
	 * BASE32("fo") = "MZXQ===="
	 * BASE32("foo") = "MZXW6==="
	 * BASE32("foob") = "MZXW6YQ="
	 * BASE32("fooba") = "MZXW6YTB"
	 * BASE32("foobar") = "MZXW6YTBOI======"
	 */
	typedef struct {
		char *in;
		char *out;
	} testdata_t;

	testdata_t test[] = {
		{"", ""},
		{"f", "MY======"},
		{"fo", "MZXQ===="},
		{"foo", "MZXW6==="},
		{"foob", "MZXW6YQ="},
		{"fooba", "MZXW6YTB"},
		{"foobar", "MZXW6YTBOI======"},
	};
	int i;

	for (i = 0; i < countof(test); i++)
	{
		chunk_t out;

		out = chunk_to_base32(chunk_create(test[i].in, strlen(test[i].in)), NULL);
		ck_assert_str_eq(out.ptr, test[i].out);
		free(out.ptr);
	}
}
END_TEST

/*******************************************************************************
 * chunk_increment test
 */

static struct {
	bool overflow;
	chunk_t in;
	chunk_t out;
} increment_data[] = {
	{TRUE,  { NULL, 0 }, { NULL, 0 }},
	{FALSE, chunk_from_chars(0x00), chunk_from_chars(0x01)},
	{FALSE, chunk_from_chars(0xfe), chunk_from_chars(0xff)},
	{TRUE,  chunk_from_chars(0xff), chunk_from_chars(0x00)},
	{FALSE, chunk_from_chars(0x00, 0x00), chunk_from_chars(0x00, 0x01)},
	{FALSE, chunk_from_chars(0x00, 0xff), chunk_from_chars(0x01, 0x00)},
	{FALSE, chunk_from_chars(0xfe, 0xff), chunk_from_chars(0xff, 0x00)},
	{TRUE,  chunk_from_chars(0xff, 0xff), chunk_from_chars(0x00, 0x00)},
};

START_TEST(test_increment)
{
	chunk_t chunk;
	bool overflow;

	chunk = chunk_clonea(increment_data[_i].in);
	overflow = chunk_increment(chunk);
	ck_assert(overflow == increment_data[_i].overflow);
	ck_assert(!increment_data[_i].out.ptr ||
			  chunk_equals(chunk, increment_data[_i].out));
}
END_TEST

/*******************************************************************************
 * chunk_printable tests
 */

static struct {
	bool printable;
	chunk_t in;
	char *out;
} printable_data[] = {
	{TRUE,  chunk_from_chars(0x31), "1"},
	{FALSE, chunk_from_chars(0x00), "?"},
	{FALSE, chunk_from_chars(0x31, 0x00), "1?"},
	{FALSE, chunk_from_chars(0x00, 0x31), "?1"},
	{TRUE,  chunk_from_chars(0x3f, 0x31), "?1"},
	{FALSE, chunk_from_chars(0x00, 0x31, 0x00), "?1?"},
	{FALSE, chunk_from_chars(0x00, 0x31, 0x00, 0x32), "?1?2"},
};

START_TEST(test_printable)
{
	bool printable;

	printable = chunk_printable(printable_data[_i].in, NULL, ' ');
	ck_assert(printable == printable_data[_i].printable);
}
END_TEST

START_TEST(test_printable_sanitize)
{
	chunk_t sane, expected;
	bool printable;

	printable = chunk_printable(printable_data[_i].in, &sane, '?');
	ck_assert(printable == printable_data[_i].printable);
	expected = chunk_from_str(printable_data[_i].out);
	ck_assert(chunk_equals(sane, expected));
	chunk_free(&sane);
}
END_TEST

START_TEST(test_printable_empty)
{
	chunk_t sane;
	bool printable;

	printable = chunk_printable(chunk_empty, NULL, ' ');
	ck_assert(printable);

	sane.ptr = (void*)1;
	sane.len = 1;
	printable = chunk_printable(chunk_empty, &sane, ' ');
	ck_assert(printable);
	assert_chunk_empty(sane);
}
END_TEST

/*******************************************************************************
 * test for chunk_mac(), i.e. SipHash-2-4
 */

/**
 * SipHash-2-4 output with
 * k = 00 01 02 ...
 * and
 * in = (empty string)
 * in = 00 (1 byte)
 * in = 00 01 (2 bytes)
 * in = 00 01 02 (3 bytes)
 * ...
 * in = 00 01 02 ... 3e (63 bytes)
 */
static const u_char sip_vectors[64][8] =
{
	{ 0x31, 0x0e, 0x0e, 0xdd, 0x47, 0xdb, 0x6f, 0x72, },
	{ 0xfd, 0x67, 0xdc, 0x93, 0xc5, 0x39, 0xf8, 0x74, },
	{ 0x5a, 0x4f, 0xa9, 0xd9, 0x09, 0x80, 0x6c, 0x0d, },
	{ 0x2d, 0x7e, 0xfb, 0xd7, 0x96, 0x66, 0x67, 0x85, },
	{ 0xb7, 0x87, 0x71, 0x27, 0xe0, 0x94, 0x27, 0xcf, },
	{ 0x8d, 0xa6, 0x99, 0xcd, 0x64, 0x55, 0x76, 0x18, },
	{ 0xce, 0xe3, 0xfe, 0x58, 0x6e, 0x46, 0xc9, 0xcb, },
	{ 0x37, 0xd1, 0x01, 0x8b, 0xf5, 0x00, 0x02, 0xab, },
	{ 0x62, 0x24, 0x93, 0x9a, 0x79, 0xf5, 0xf5, 0x93, },
	{ 0xb0, 0xe4, 0xa9, 0x0b, 0xdf, 0x82, 0x00, 0x9e, },
	{ 0xf3, 0xb9, 0xdd, 0x94, 0xc5, 0xbb, 0x5d, 0x7a, },
	{ 0xa7, 0xad, 0x6b, 0x22, 0x46, 0x2f, 0xb3, 0xf4, },
	{ 0xfb, 0xe5, 0x0e, 0x86, 0xbc, 0x8f, 0x1e, 0x75, },
	{ 0x90, 0x3d, 0x84, 0xc0, 0x27, 0x56, 0xea, 0x14, },
	{ 0xee, 0xf2, 0x7a, 0x8e, 0x90, 0xca, 0x23, 0xf7, },
	{ 0xe5, 0x45, 0xbe, 0x49, 0x61, 0xca, 0x29, 0xa1, },
	{ 0xdb, 0x9b, 0xc2, 0x57, 0x7f, 0xcc, 0x2a, 0x3f, },
	{ 0x94, 0x47, 0xbe, 0x2c, 0xf5, 0xe9, 0x9a, 0x69, },
	{ 0x9c, 0xd3, 0x8d, 0x96, 0xf0, 0xb3, 0xc1, 0x4b, },
	{ 0xbd, 0x61, 0x79, 0xa7, 0x1d, 0xc9, 0x6d, 0xbb, },
	{ 0x98, 0xee, 0xa2, 0x1a, 0xf2, 0x5c, 0xd6, 0xbe, },
	{ 0xc7, 0x67, 0x3b, 0x2e, 0xb0, 0xcb, 0xf2, 0xd0, },
	{ 0x88, 0x3e, 0xa3, 0xe3, 0x95, 0x67, 0x53, 0x93, },
	{ 0xc8, 0xce, 0x5c, 0xcd, 0x8c, 0x03, 0x0c, 0xa8, },
	{ 0x94, 0xaf, 0x49, 0xf6, 0xc6, 0x50, 0xad, 0xb8, },
	{ 0xea, 0xb8, 0x85, 0x8a, 0xde, 0x92, 0xe1, 0xbc, },
	{ 0xf3, 0x15, 0xbb, 0x5b, 0xb8, 0x35, 0xd8, 0x17, },
	{ 0xad, 0xcf, 0x6b, 0x07, 0x63, 0x61, 0x2e, 0x2f, },
	{ 0xa5, 0xc9, 0x1d, 0xa7, 0xac, 0xaa, 0x4d, 0xde, },
	{ 0x71, 0x65, 0x95, 0x87, 0x66, 0x50, 0xa2, 0xa6, },
	{ 0x28, 0xef, 0x49, 0x5c, 0x53, 0xa3, 0x87, 0xad, },
	{ 0x42, 0xc3, 0x41, 0xd8, 0xfa, 0x92, 0xd8, 0x32, },
	{ 0xce, 0x7c, 0xf2, 0x72, 0x2f, 0x51, 0x27, 0x71, },
	{ 0xe3, 0x78, 0x59, 0xf9, 0x46, 0x23, 0xf3, 0xa7, },
	{ 0x38, 0x12, 0x05, 0xbb, 0x1a, 0xb0, 0xe0, 0x12, },
	{ 0xae, 0x97, 0xa1, 0x0f, 0xd4, 0x34, 0xe0, 0x15, },
	{ 0xb4, 0xa3, 0x15, 0x08, 0xbe, 0xff, 0x4d, 0x31, },
	{ 0x81, 0x39, 0x62, 0x29, 0xf0, 0x90, 0x79, 0x02, },
	{ 0x4d, 0x0c, 0xf4, 0x9e, 0xe5, 0xd4, 0xdc, 0xca, },
	{ 0x5c, 0x73, 0x33, 0x6a, 0x76, 0xd8, 0xbf, 0x9a, },
	{ 0xd0, 0xa7, 0x04, 0x53, 0x6b, 0xa9, 0x3e, 0x0e, },
	{ 0x92, 0x59, 0x58, 0xfc, 0xd6, 0x42, 0x0c, 0xad, },
	{ 0xa9, 0x15, 0xc2, 0x9b, 0xc8, 0x06, 0x73, 0x18, },
	{ 0x95, 0x2b, 0x79, 0xf3, 0xbc, 0x0a, 0xa6, 0xd4, },
	{ 0xf2, 0x1d, 0xf2, 0xe4, 0x1d, 0x45, 0x35, 0xf9, },
	{ 0x87, 0x57, 0x75, 0x19, 0x04, 0x8f, 0x53, 0xa9, },
	{ 0x10, 0xa5, 0x6c, 0xf5, 0xdf, 0xcd, 0x9a, 0xdb, },
	{ 0xeb, 0x75, 0x09, 0x5c, 0xcd, 0x98, 0x6c, 0xd0, },
	{ 0x51, 0xa9, 0xcb, 0x9e, 0xcb, 0xa3, 0x12, 0xe6, },
	{ 0x96, 0xaf, 0xad, 0xfc, 0x2c, 0xe6, 0x66, 0xc7, },
	{ 0x72, 0xfe, 0x52, 0x97, 0x5a, 0x43, 0x64, 0xee, },
	{ 0x5a, 0x16, 0x45, 0xb2, 0x76, 0xd5, 0x92, 0xa1, },
	{ 0xb2, 0x74, 0xcb, 0x8e, 0xbf, 0x87, 0x87, 0x0a, },
	{ 0x6f, 0x9b, 0xb4, 0x20, 0x3d, 0xe7, 0xb3, 0x81, },
	{ 0xea, 0xec, 0xb2, 0xa3, 0x0b, 0x22, 0xa8, 0x7f, },
	{ 0x99, 0x24, 0xa4, 0x3c, 0xc1, 0x31, 0x57, 0x24, },
	{ 0xbd, 0x83, 0x8d, 0x3a, 0xaf, 0xbf, 0x8d, 0xb7, },
	{ 0x0b, 0x1a, 0x2a, 0x32, 0x65, 0xd5, 0x1a, 0xea, },
	{ 0x13, 0x50, 0x79, 0xa3, 0x23, 0x1c, 0xe6, 0x60, },
	{ 0x93, 0x2b, 0x28, 0x46, 0xe4, 0xd7, 0x06, 0x66, },
	{ 0xe1, 0x91, 0x5f, 0x5c, 0xb1, 0xec, 0xa4, 0x6c, },
	{ 0xf3, 0x25, 0x96, 0x5c, 0xa1, 0x6d, 0x62, 0x9f, },
	{ 0x57, 0x5f, 0xf2, 0x8e, 0x60, 0x38, 0x1b, 0xe5, },
	{ 0x72, 0x45, 0x06, 0xeb, 0x4c, 0x32, 0x8a, 0x95, }
};

/**
 * Our SipHash-2-4 implementation returns the result in host order, which
 * doesn't matter for practical purposes and even avoids a byte swap.  But
 * because the test vectors are in little-endian we have to account for this
 * with this custom comparison function.
 */
static inline bool sipeq(const void *a, const void *b, size_t n)
{
	u_char *ap = (u_char*)a, *bp = (u_char*)b;
	int i;

	for (i = 0; i < n; i++)
	{
#ifdef WORDS_BIGENDIAN
		if (ap[i] != bp[n - i - 1])
#else
		if (ap[i] != bp[i])
#endif
		{
			return FALSE;
		}
	}
	return TRUE;
}

START_TEST(test_chunk_mac)
{
	chunk_t in;
	u_char key[16];
	u_int64_t out;
	int i, count;

	count = countof(sip_vectors);
	in = chunk_alloca(count);

	for (i = 0; i < 16; ++i)
	{
		key[i] = i;
	}

	for (i = 0; i < count; ++i)
	{
		in.ptr[i] = i;
		in.len = i;
		out = chunk_mac(in, key);
		fail_unless(sipeq(&out, sip_vectors[i], 8),
					"test vector failed for %d bytes", i);
	}
}
END_TEST

/*******************************************************************************
 * test for chunk_hash[_inc]()
 */

START_TEST(test_chunk_hash)
{
	chunk_t chunk;
	u_int32_t hash_a, hash_b, hash_c;

	chunk = chunk_from_str("asdf");

	/* output is randomized, so there are no test-vectors we could use */
	hash_a = chunk_hash(chunk);
	hash_b = chunk_hash(chunk);
	ck_assert(hash_a == hash_b);
	hash_b = chunk_hash_inc(chunk, hash_a);
	ck_assert(hash_a != hash_b);
	hash_c = chunk_hash_inc(chunk, hash_a);
	ck_assert(hash_b == hash_c);
}
END_TEST

/*******************************************************************************
 * test for chunk_hash_static[_inc]()
 */

START_TEST(test_chunk_hash_static)
{
	chunk_t in;
	u_int32_t out, hash_a, hash_b, hash_inc = 0x7b891a95;
	int i, count;

	count = countof(sip_vectors);
	in = chunk_alloca(count);

	for (i = 0; i < count; ++i)
	{
		in.ptr[i] = i;
		in.len = i;
		/* compared to chunk_mac() we only get half the value back */
		out = chunk_hash_static(in);
		fail_unless(sipeq(&out, sip_vectors[i], 4),
					"test vector failed for %d bytes", i);
	}
	hash_a = chunk_hash_static_inc(in, out);
	ck_assert_int_eq(hash_a, hash_inc);
	hash_b = chunk_hash_static_inc(in, out);
	ck_assert_int_eq(hash_a, hash_b);
}
END_TEST

/*******************************************************************************
 * test for chunk_internet_checksum[_inc]()
 */

START_TEST(test_chunk_internet_checksum)
{
	chunk_t chunk;
	u_int16_t sum;

	chunk = chunk_from_chars(0x45,0x00,0x00,0x30,0x44,0x22,0x40,0x00,0x80,0x06,
							 0x00,0x00,0x8c,0x7c,0x19,0xac,0xae,0x24,0x1e,0x2b);

	sum = chunk_internet_checksum(chunk);
	ck_assert_int_eq(0x442e, ntohs(sum));

	sum = chunk_internet_checksum(chunk_create(chunk.ptr, 10));
	sum = chunk_internet_checksum_inc(chunk_create(chunk.ptr+10, 10), sum);
	ck_assert_int_eq(0x442e, ntohs(sum));

	/* need to compensate for even/odd alignment */
	sum = chunk_internet_checksum(chunk_create(chunk.ptr, 9));
	sum = ntohs(sum);
	sum = chunk_internet_checksum_inc(chunk_create(chunk.ptr+9, 11), sum);
	sum = ntohs(sum);
	ck_assert_int_eq(0x442e, ntohs(sum));

	chunk = chunk_from_chars(0x45,0x00,0x00,0x30,0x44,0x22,0x40,0x00,0x80,0x06,
							 0x00,0x00,0x8c,0x7c,0x19,0xac,0xae,0x24,0x1e);

	sum = chunk_internet_checksum(chunk);
	ck_assert_int_eq(0x4459, ntohs(sum));

	sum = chunk_internet_checksum(chunk_create(chunk.ptr, 10));
	sum = chunk_internet_checksum_inc(chunk_create(chunk.ptr+10, 9), sum);
	ck_assert_int_eq(0x4459, ntohs(sum));

	/* need to compensate for even/odd alignment */
	sum = chunk_internet_checksum(chunk_create(chunk.ptr, 9));
	sum = ntohs(sum);
	sum = chunk_internet_checksum_inc(chunk_create(chunk.ptr+9, 10), sum);
	sum = ntohs(sum);
	ck_assert_int_eq(0x4459, ntohs(sum));
}
END_TEST

/*******************************************************************************
 * test for chunk_map and friends
 */

START_TEST(test_chunk_map)
{
	chunk_t *map, contents = chunk_from_chars(0x01,0x02,0x03,0x04,0x05);
#ifdef WIN32
	char *path = "C:\\Windows\\Temp\\strongswan-chunk-map-test";
#else
	char *path = "/tmp/strongswan-chunk-map-test";
#endif

	ck_assert(chunk_write(contents, path, 022, TRUE));

	/* read */
	map = chunk_map(path, FALSE);
	ck_assert(map != NULL);
	ck_assert_msg(chunk_equals(*map, contents), "%B", map);
	/* altering mapped chunk should not hurt */
	*map = chunk_empty;
	ck_assert(chunk_unmap(map));

	/* write */
	map = chunk_map(path, TRUE);
	ck_assert(map != NULL);
	ck_assert_msg(chunk_equals(*map, contents), "%B", map);
	map->ptr[0] = 0x06;
	ck_assert(chunk_unmap(map));

	/* verify write */
	contents.ptr[0] = 0x06;
	map = chunk_map(path, FALSE);
	ck_assert(map != NULL);
	ck_assert_msg(chunk_equals(*map, contents), "%B", map);
	ck_assert(chunk_unmap(map));

	unlink(path);
}
END_TEST

/*******************************************************************************
 * test for chunk_from_fd
 */

START_TEST(test_chunk_from_fd_file)
{
	chunk_t in, contents = chunk_from_chars(0x01,0x02,0x03,0x04,0x05);
#ifdef WIN32
	char *path = "C:\\Windows\\Temp\\strongswan-chunk-fd-test";
#else
	char *path = "/tmp/strongswan-chunk-fd-test";
#endif
	int fd;

	ck_assert(chunk_write(contents, path, 022, TRUE));

	fd = open(path, O_RDONLY);
	ck_assert(fd != -1);

	ck_assert(chunk_from_fd(fd, &in));
	close(fd);
	ck_assert_msg(chunk_equals(in, contents), "%B", &in);
	unlink(path);
	free(in.ptr);
}
END_TEST

START_TEST(test_chunk_from_fd_skt)
{
	chunk_t in, contents = chunk_from_chars(0x01,0x02,0x03,0x04,0x05);
	int s[2];

	ck_assert(socketpair(AF_UNIX, SOCK_STREAM, 0, s) == 0);
	ck_assert_int_eq(send(s[1], contents.ptr, contents.len, 0), contents.len);
	close(s[1]);
	ck_assert_msg(chunk_from_fd(s[0], &in), "%s", strerror(errno));
	close(s[0]);
	ck_assert_msg(chunk_equals(in, contents), "%B", &in);
	free(in.ptr);
}
END_TEST

#define FROM_FD_COUNT 8192

void *chunk_from_fd_run(void *data)
{
	int i, fd = (uintptr_t)data;

	for (i = 0; i < FROM_FD_COUNT; i++)
	{
		ck_assert(send(fd, &i, sizeof(i), 0) == sizeof(i));
	}
	close(fd);
	return NULL;
}

START_TEST(test_chunk_from_fd_huge)
{
	thread_t *thread;
	chunk_t in;
	int s[2], i;

	ck_assert(socketpair(AF_UNIX, SOCK_STREAM, 0, s) == 0);

	thread = thread_create(chunk_from_fd_run, (void*)(uintptr_t)s[1]);
	ck_assert_msg(chunk_from_fd(s[0], &in), "%s", strerror(errno));
	ck_assert_int_eq(in.len, FROM_FD_COUNT * sizeof(i));
	for (i = 0; i < FROM_FD_COUNT; i++)
	{
		ck_assert_int_eq(((int*)in.ptr)[i], i);
	}
	thread->join(thread);
	close(s[0]);
	free(in.ptr);
}
END_TEST

/*******************************************************************************
 * printf_hook tests
 */

static struct {
	chunk_t in;
	char *out;
	char *out_plus;
} printf_hook_data[] = {
	{chunk_from_chars(), "", ""},
	{chunk_from_chars(0x00), "00", "00"},
	{chunk_from_chars(0x00, 0x01), "00:01", "0001"},
	{chunk_from_chars(0x00, 0x01, 0x02), "00:01:02", "000102"},
};

START_TEST(test_printf_hook_hash)
{
	char buf[16];
	int len;

	len = snprintf(buf, sizeof(buf), "%#B", &printf_hook_data[_i].in);
	ck_assert(len >= 0 && len < sizeof(buf));
	ck_assert_str_eq(buf, printf_hook_data[_i].out);
}
END_TEST

START_TEST(test_printf_hook_plus)
{
	char buf[16];
	int len;

	len = snprintf(buf, sizeof(buf), "%+B", &printf_hook_data[_i].in);
	ck_assert(len >= 0 && len < sizeof(buf));
	ck_assert_str_eq(buf, printf_hook_data[_i].out_plus);
}
END_TEST

START_TEST(test_printf_hook)
{
	char buf[128], mem[128];
	int len;

	/* %B should be the same as %b, which is what we check, comparing the
	 * acutal result could be tricky as %b prints the chunk's memory address */
	len = snprintf(buf, sizeof(buf), "%B", &printf_hook_data[_i].in);
	ck_assert(len >= 0 && len < sizeof(buf));
	len = snprintf(mem, sizeof(mem), "%b", printf_hook_data[_i].in.ptr,
				  (u_int)printf_hook_data[_i].in.len);
	ck_assert(len >= 0 && len < sizeof(mem));
	ck_assert_str_eq(buf, mem);
}
END_TEST

Suite *chunk_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("chunk");

	tc = tcase_create("equals");
	tcase_add_test(tc, test_chunk_equals);
	suite_add_tcase(s, tc);

	tc = tcase_create("chunk_compare");
	tcase_add_loop_test(tc, test_compare, 0, countof(compare_data));
	suite_add_tcase(s, tc);

	tc = tcase_create("clear");
	tcase_add_test(tc, test_chunk_clear);
	suite_add_tcase(s, tc);

	tc = tcase_create("chunk_length");
	tcase_add_test(tc, test_chunk_length);
	suite_add_tcase(s, tc);

	tc = tcase_create("chunk_create_cat");
	tcase_add_test(tc, test_chunk_create_cat);
	suite_add_tcase(s, tc);

	tc = tcase_create("chunk_split");
	tcase_add_test(tc, test_chunk_split);
	suite_add_tcase(s, tc);

	tc = tcase_create("chunk_skip");
	tcase_add_test(tc, test_chunk_skip);
	tcase_add_test(tc, test_chunk_skip_zero);
	suite_add_tcase(s, tc);

	tc = tcase_create("chunk_increment");
	tcase_add_loop_test(tc, test_increment, 0, countof(increment_data));
	suite_add_tcase(s, tc);

	tc = tcase_create("chunk_printable");
	tcase_add_loop_test(tc, test_printable, 0, countof(printable_data));
	tcase_add_loop_test(tc, test_printable_sanitize, 0, countof(printable_data));
	tcase_add_test(tc, test_printable_empty);
	suite_add_tcase(s, tc);

	tc = tcase_create("baseXX");
	tcase_add_test(tc, test_base64);
	tcase_add_test(tc, test_base32);
	tcase_add_test(tc, test_base16);
	suite_add_tcase(s, tc);

	tc = tcase_create("chunk_mac");
	tcase_add_test(tc, test_chunk_mac);
	suite_add_tcase(s, tc);

	tc = tcase_create("chunk_hash");
	tcase_add_test(tc, test_chunk_hash);
	suite_add_tcase(s, tc);

	tc = tcase_create("chunk_hash_static");
	tcase_add_test(tc, test_chunk_hash_static);
	suite_add_tcase(s, tc);

	tc = tcase_create("chunk_internet_checksum");
	tcase_add_test(tc, test_chunk_internet_checksum);
	suite_add_tcase(s, tc);

	tc = tcase_create("chunk_map");
	tcase_add_test(tc, test_chunk_map);
	suite_add_tcase(s, tc);

	tc = tcase_create("chunk_from_fd");
	tcase_add_test(tc, test_chunk_from_fd_file);
	tcase_add_test(tc, test_chunk_from_fd_skt);
	tcase_add_test(tc, test_chunk_from_fd_huge);
	suite_add_tcase(s, tc);

	tc = tcase_create("printf_hook");
	tcase_add_loop_test(tc, test_printf_hook_hash, 0, countof(printf_hook_data));
	tcase_add_loop_test(tc, test_printf_hook_plus, 0, countof(printf_hook_data));
	tcase_add_loop_test(tc, test_printf_hook, 0, countof(printf_hook_data));
	suite_add_tcase(s, tc);

	return s;
}
