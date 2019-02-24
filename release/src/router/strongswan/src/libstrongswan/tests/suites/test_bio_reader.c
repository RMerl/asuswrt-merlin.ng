/*
 * Copyright (C) 2013 Tobias Brunner
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

#include <bio/bio_reader.h>

/*******************************************************************************
 * different integer reads
 */

#define assert_integer_read(data, bits, val) ({ \
	bio_reader_t *reader = bio_reader_create(data); \
	typeof(val) i; \
	for (i = 0; reader->remaining(reader) >= (bits / 8); i++) \
	{ \
		ck_assert(reader->read_uint##bits(reader, &val)); \
		ck_assert_int_eq(i, val); \
	} \
	ck_assert_int_eq(i, data.len / (bits / 8)); \
	ck_assert_int_eq(reader->remaining(reader), data.len % (bits / 8)); \
	ck_assert(!reader->read_uint##bits(reader, &val)); \
	reader->destroy(reader); \
})

#define assert_integer_read_uneven(data, bits, val) ({ \
	int i; \
	for (i = 0; i <= bits / 8; i++, data.len++) \
	{ \
		assert_integer_read(data, bits, val); \
	} \
})

#define assert_basic_read(bits, val) ({ \
	chunk_t data; \
	data = chunk_empty; \
	assert_integer_read(data, bits, val); \
	data = chunk_alloca(bits / 8); \
	memset(data.ptr, 0, data.len); \
	data.len = 0; \
	assert_integer_read_uneven(data, bits, val); \
})

#define assert_extended_read(data, bits, val) ({ \
	chunk_t extended = chunk_alloca(data.len + bits / 8); \
	memset(extended.ptr, 0, extended.len); \
	extended.ptr[extended.len - 1] = data.len / (bits / 8); \
	memcpy(extended.ptr, data.ptr, data.len); \
	extended.len = data.len; \
	assert_integer_read_uneven(extended, bits, val); \
})

START_TEST(test_read_uint8)
{
	chunk_t data = chunk_from_chars(0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07);
	uint8_t val;

	assert_integer_read(data, 8, val);
	assert_basic_read(8, val);
	assert_extended_read(data, 8, val);
}
END_TEST

START_TEST(test_read_uint16)
{
	chunk_t data = chunk_from_chars(0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x00, 0x03);
	uint16_t val;

	assert_integer_read(data, 16, val);
	assert_basic_read(16, val);
	assert_extended_read(data, 16, val);
}
END_TEST

START_TEST(test_read_uint24)
{
	chunk_t data = chunk_from_chars(0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x02, 0x00, 0x00, 0x03);
	uint32_t val;

	assert_integer_read(data, 24, val);
	assert_basic_read(24, val);
	assert_extended_read(data, 24, val);
}
END_TEST

START_TEST(test_read_uint32)
{
	chunk_t data = chunk_from_chars(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
									0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03);
	uint32_t val;

	assert_integer_read(data, 32, val);
	assert_basic_read(32, val);
	assert_extended_read(data, 32, val);
}
END_TEST

START_TEST(test_read_uint64)
{
	chunk_t data = chunk_from_chars(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
									0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
									0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
									0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03);
	uint64_t val;

	assert_integer_read(data, 64, val);
	assert_basic_read(64, val);
	assert_extended_read(data, 64, val);
}
END_TEST

/*******************************************************************************
 * different integer reads from the end of a buffer
 */

#define assert_integer_read_end(data, bits, val) ({ \
	bio_reader_t *reader = bio_reader_create(data); \
	typeof(val) i; \
	for (i = 0; reader->remaining(reader) >= (bits / 8); i++) \
	{ \
		ck_assert(reader->read_uint##bits##_end(reader, &val)); \
		ck_assert_int_eq(i, val); \
	} \
	ck_assert_int_eq(i, data.len / (bits / 8)); \
	ck_assert_int_eq(reader->remaining(reader), data.len % (bits / 8)); \
	ck_assert(!reader->read_uint##bits##_end(reader, &val)); \
	reader->destroy(reader); \
})

#define assert_integer_read_end_uneven(data, bits, val) ({ \
	int i; \
	data.ptr += bits / 8; \
	for (i = 0; i <= bits / 8; i++, data.ptr--, data.len++) \
	{ \
		assert_integer_read_end(data, bits, val); \
	} \
})

#define assert_basic_read_end(bits, val) ({ \
	chunk_t data; \
	data = chunk_empty; \
	assert_integer_read_end(data, bits, val); \
	data = chunk_alloca(bits / 8); \
	memset(data.ptr, 0, data.len); \
	data.len = 0; \
	assert_integer_read_end_uneven(data, bits, val); \
})

#define assert_extended_read_end(data, bits, val) ({ \
	chunk_t extended = chunk_alloca(data.len + bits / 8); \
	memset(extended.ptr, 0, extended.len); \
	extended.ptr[bits / 8 - 1] = data.len / (bits / 8); \
	memcpy(extended.ptr + bits / 8, data.ptr, data.len); \
	extended.len = data.len; \
	assert_integer_read_end_uneven(extended, bits, val); \
})

START_TEST(test_read_uint8_end)
{
	chunk_t data = chunk_from_chars(0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00);
	uint8_t val;

	assert_integer_read_end(data, 8, val);
	assert_basic_read_end(8, val);
	assert_extended_read_end(data, 8, val);
}
END_TEST

START_TEST(test_read_uint16_end)
{
	chunk_t data = chunk_from_chars(0x00, 0x03, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00);
	uint16_t val;

	assert_integer_read_end(data, 16, val);
	assert_basic_read_end(16, val);
	assert_extended_read_end(data, 16, val);
}
END_TEST

START_TEST(test_read_uint24_end)
{
	chunk_t data = chunk_from_chars(0x00, 0x00, 0x03, 0x00, 0x00, 0x02, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00);
	uint32_t val;

	assert_integer_read_end(data, 24, val);
	assert_basic_read_end(24, val);
	assert_extended_read_end(data, 24, val);
}
END_TEST

START_TEST(test_read_uint32_end)
{
	chunk_t data = chunk_from_chars(0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x02,
									0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00);
	uint32_t val;

	assert_integer_read_end(data, 32, val);
	assert_basic_read_end(32, val);
	assert_extended_read_end(data, 32, val);
}
END_TEST

START_TEST(test_read_uint64_end)
{
	chunk_t data = chunk_from_chars(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03,
									0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
									0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
									0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
	uint64_t val;

	assert_integer_read_end(data, 64, val);
	assert_basic_read_end(64, val);
	assert_extended_read_end(data, 64, val);
}
END_TEST

/*******************************************************************************
 * read data
 */

static inline void assert_reader_after_read(bio_reader_t *reader, chunk_t data)
{
	chunk_t peek;

	ck_assert_int_eq(reader->remaining(reader), data.len);
	peek = reader->peek(reader);
	ck_assert_int_eq(reader->remaining(reader), data.len);
	ck_assert(peek.ptr == data.ptr);
	data.ptr != NULL ? ck_assert(chunk_equals(peek, data))
					 : ck_assert(peek.ptr == NULL);
}

START_TEST(test_read_data)
{
	chunk_t read, data = chunk_from_chars(0x00, 0x00, 0x00, 0x00);
	bio_reader_t *reader;

	reader = bio_reader_create(chunk_empty);
	ck_assert_int_eq(reader->remaining(reader), 0);
	ck_assert(reader->read_data(reader, 0, &read));
	ck_assert(!reader->read_data(reader, 1, &read));
	reader->destroy(reader);

	reader = bio_reader_create(data);
	ck_assert(reader->read_data(reader, 0, &read));
	ck_assert_int_eq(read.len, 0);
	ck_assert(read.ptr == data.ptr);
	assert_reader_after_read(reader, data);

	ck_assert(reader->read_data(reader, 1, &read));
	ck_assert_int_eq(read.len, 1);
	ck_assert(read.ptr == data.ptr);
	assert_reader_after_read(reader, chunk_skip(data, 1));

	ck_assert(reader->read_data(reader, 2, &read));
	ck_assert_int_eq(read.len, 2);
	ck_assert(read.ptr == data.ptr + 1);
	assert_reader_after_read(reader, chunk_skip(data, 3));

	ck_assert(!reader->read_data(reader, 2, &read));
	ck_assert(reader->read_data(reader, 1, &read));
	ck_assert_int_eq(read.len, 1);
	ck_assert(read.ptr == data.ptr + 3);
	assert_reader_after_read(reader, chunk_skip(data, 4));

	ck_assert_int_eq(reader->remaining(reader), 0);
	ck_assert(reader->read_data(reader, 0, &read));
	ck_assert(!reader->read_data(reader, 1, &read));
	reader->destroy(reader);
}
END_TEST

START_TEST(test_read_data_end)
{
	chunk_t read, data = chunk_from_chars(0x00, 0x00, 0x00, 0x00);
	bio_reader_t *reader;

	reader = bio_reader_create(chunk_empty);
	ck_assert_int_eq(reader->remaining(reader), 0);
	ck_assert(reader->read_data_end(reader, 0, &read));
	ck_assert(!reader->read_data_end(reader, 1, &read));
	reader->destroy(reader);

	reader = bio_reader_create(data);
	ck_assert(reader->read_data_end(reader, 0, &read));
	ck_assert_int_eq(read.len, 0);
	ck_assert(read.ptr == data.ptr + data.len);
	assert_reader_after_read(reader, data);

	ck_assert(reader->read_data_end(reader, 1, &read));
	ck_assert_int_eq(read.len, 1);
	data.len--;
	ck_assert(read.ptr == data.ptr + data.len);
	assert_reader_after_read(reader, data);

	ck_assert(reader->read_data_end(reader, 2, &read));
	ck_assert_int_eq(read.len, 2);
	data.len -= 2;
	ck_assert(read.ptr == data.ptr + data.len);
	assert_reader_after_read(reader, data);

	ck_assert(!reader->read_data(reader, 2, &read));
	ck_assert(reader->read_data(reader, 1, &read));
	ck_assert_int_eq(read.len, 1);
	ck_assert(read.ptr == data.ptr);
	assert_reader_after_read(reader, chunk_empty);

	ck_assert_int_eq(reader->remaining(reader), 0);
	ck_assert(reader->read_data(reader, 0, &read));
	ck_assert(!reader->read_data(reader, 1, &read));
	reader->destroy(reader);
}
END_TEST

/*******************************************************************************
 * read length followed by data
 */

#define assert_read_data_len(bits) ({ \
	bio_reader_t *reader; \
	chunk_t read, data; \
	int i, len = bits / 8; \
	data = chunk_empty; \
	reader = bio_reader_create(data); \
	ck_assert(!reader->read_data##bits(reader, &read)); \
	reader->destroy(reader); \
	data = chunk_alloca(len + 8); \
	memset(data.ptr, 0, data.len); \
	for (i = 0; i <= 8; i++) \
	{ \
		data.ptr[len - 1] = i; \
		data.len = len + i; \
		reader = bio_reader_create(data); \
		ck_assert(reader->read_data##bits(reader, &read)); \
		ck_assert_int_eq(reader->remaining(reader), 0); \
		ck_assert_int_eq(read.len, i); \
		ck_assert((!read.ptr && !read.len) || (read.ptr == data.ptr + len)); \
		reader->destroy(reader); \
	} \
	data.ptr[len - 1] = i; \
	reader = bio_reader_create(data); \
	ck_assert(!reader->read_data##bits(reader, &read)); \
	reader->destroy(reader); \
})

START_TEST(test_read_data8)
{
	assert_read_data_len(8);
}
END_TEST

START_TEST(test_read_data16)
{
	assert_read_data_len(16);
}
END_TEST

START_TEST(test_read_data24)
{
	assert_read_data_len(24);
}
END_TEST

START_TEST(test_read_data32)
{
	assert_read_data_len(32);
}
END_TEST

/*******************************************************************************
 * test constructors
 */

START_TEST(test_create)
{
	chunk_t data = chunk_from_str("foobar");
	bio_reader_t *reader;

	data = chunk_clone(data);
	reader = bio_reader_create(data);
	reader->destroy(reader);
	chunk_free(&data);
}
END_TEST

START_TEST(test_create_own)
{
	chunk_t data = chunk_from_str("foobar");
	bio_reader_t *reader;

	data = chunk_clone(data);
	reader = bio_reader_create_own(data);
	reader->destroy(reader);
}
END_TEST

Suite *bio_reader_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("bio_reader");

	tc = tcase_create("integer reads");
	tcase_add_test(tc, test_read_uint8);
	tcase_add_test(tc, test_read_uint16);
	tcase_add_test(tc, test_read_uint24);
	tcase_add_test(tc, test_read_uint32);
	tcase_add_test(tc, test_read_uint64);
	suite_add_tcase(s, tc);

	tc = tcase_create("integer reads from end");
	tcase_add_test(tc, test_read_uint8_end);
	tcase_add_test(tc, test_read_uint16_end);
	tcase_add_test(tc, test_read_uint24_end);
	tcase_add_test(tc, test_read_uint32_end);
	tcase_add_test(tc, test_read_uint64_end);
	suite_add_tcase(s, tc);

	tc = tcase_create("data reads and peek");
	tcase_add_test(tc, test_read_data);
	tcase_add_test(tc, test_read_data_end);
	suite_add_tcase(s, tc);

	tc = tcase_create("data length reads");
	tcase_add_test(tc, test_read_data8);
	tcase_add_test(tc, test_read_data16);
	tcase_add_test(tc, test_read_data24);
	tcase_add_test(tc, test_read_data32);
	suite_add_tcase(s, tc);

	tc = tcase_create("constructors");
	tcase_add_test(tc, test_create);
	tcase_add_test(tc, test_create_own);
	suite_add_tcase(s, tc);

	return s;
}
