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

#include <bio/bio_writer.h>

/*******************************************************************************
 * different integer writes
 */

static inline void verify_int_buffer(chunk_t data, int bits, int val)
{
	size_t i;
	int len = bits / 8;

	ck_assert_int_eq(data.len, (val + 1) * len);
	for (i = 0; i < data.len; i++)
	{
		if ((i + 1) % len)
		{
			ck_assert_int_eq(data.ptr[i], 0);
		}
		else
		{
			ck_assert_int_eq(data.ptr[i], i / len);
		}
	}
}

#define assert_integer_write(init, bits) ({ \
	int i; \
	bio_writer_t *writer = bio_writer_create(init); \
	for (i = 0; i < 16; i++) \
	{ \
		writer->write_uint##bits(writer, i); \
		verify_int_buffer(writer->get_buf(writer), bits, i); \
	} \
	writer->destroy(writer); \
})

START_TEST(test_write_uint8)
{
	/* use default buffer (and increase) size */
	assert_integer_write(0, 8);
	/* force a resize by the given size */
	assert_integer_write(1, 8);
}
END_TEST

START_TEST(test_write_uint16)
{
	assert_integer_write(0, 16);
	assert_integer_write(1, 16);
}
END_TEST

START_TEST(test_write_uint24)
{
	assert_integer_write(0, 24);
	assert_integer_write(1, 24);
}
END_TEST

START_TEST(test_write_uint32)
{
	assert_integer_write(0, 32);
	assert_integer_write(1, 32);
}
END_TEST

START_TEST(test_write_uint64)
{
	assert_integer_write(0, 64);
	assert_integer_write(1, 64);
}
END_TEST

/*******************************************************************************
 * write data / skip
 */

static inline void assert_writer_after_write(bio_writer_t *writer, int count)
{
	chunk_t buf;
	size_t i;

	buf = writer->get_buf(writer);
	ck_assert_int_eq(buf.len, count * 3);
	for (i = 0; i < buf.len; i++)
	{
		ck_assert(buf.ptr[i] == i % 3);
	}
}

START_TEST(test_write_data)
{
	chunk_t buf, data = chunk_from_chars(0x00, 0x01, 0x02);
	bio_writer_t *writer;

	/* no allocation, but default buffer size */
	writer = bio_writer_create(0);
	buf = writer->get_buf(writer);
	ck_assert_int_eq(buf.len, 0);
	ck_assert(buf.ptr == NULL);

	writer->write_data(writer, chunk_empty);
	buf = writer->get_buf(writer);
	ck_assert_int_eq(buf.len, 0);
	ck_assert(buf.ptr == NULL);
	writer->destroy(writer);

	/* custom buffer size, initial buffer allocated */
	writer = bio_writer_create(1);
	buf = writer->get_buf(writer);
	ck_assert_int_eq(buf.len, 0);
	ck_assert(buf.ptr != NULL);

	writer->write_data(writer, chunk_empty);
	buf = writer->get_buf(writer);
	ck_assert_int_eq(buf.len, 0);
	ck_assert(buf.ptr != NULL);
	writer->destroy(writer);

	writer = bio_writer_create(0);

	writer->write_data(writer, data);
	assert_writer_after_write(writer, 1);

	writer->write_data(writer, data);
	assert_writer_after_write(writer, 2);

	writer->write_data(writer, data);
	assert_writer_after_write(writer, 3);

	writer->destroy(writer);
}
END_TEST

START_TEST(test_skip)
{
	chunk_t skipped, buf, data = chunk_from_chars(0x00, 0x01, 0x02);
	bio_writer_t *writer;

	writer = bio_writer_create(4);
	skipped = writer->skip(writer, 3);
	ck_assert_int_eq(skipped.len, 3);
	buf = writer->get_buf(writer);
	ck_assert(skipped.ptr == buf.ptr);
	memset(skipped.ptr, 0, skipped.len);

	writer->write_data(writer, data);
	buf = writer->get_buf(writer);
	ck_assert(chunk_equals(buf, chunk_from_chars(0x00, 0x00, 0x00, 0x00, 0x01, 0x02)));
	writer->destroy(writer);

	writer = bio_writer_create(1);
	skipped = writer->skip(writer, 3);
	memcpy(skipped.ptr, data.ptr, data.len);

	writer->write_data(writer, data);
	assert_writer_after_write(writer, 2);
	writer->destroy(writer);
}
END_TEST

/*******************************************************************************
 * write length followed by data
 */

#define assert_write_data_len(init, bits) ({ \
	bio_writer_t *writer; \
	chunk_t buf, data; \
	int i, len = bits / 8; \
	writer = bio_writer_create(init); \
	writer->write_data##bits(writer, chunk_empty); \
	buf = writer->get_buf(writer); \
	ck_assert_int_eq(buf.len, len); \
	ck_assert_int_eq(buf.ptr[len - 1], 0); \
	writer->destroy(writer); \
	data = chunk_alloca(32); \
	memset(data.ptr, 0, data.len); \
	for (i = 0; i < 32; i++) \
	{ \
		data.ptr[i] = i; \
		data.len = i; \
		writer = bio_writer_create(init); \
		writer->write_data##bits(writer, data); \
		buf = writer->get_buf(writer); \
		ck_assert_int_eq(buf.len, len + i); \
		ck_assert_int_eq(buf.ptr[len - 1], i); \
		ck_assert(chunk_equals(chunk_create(buf.ptr + len, buf.len - len), data)); \
		writer->destroy(writer); \
	} \
})

START_TEST(test_write_data8)
{
	assert_write_data_len(0, 8);
	assert_write_data_len(1, 8);
}
END_TEST

START_TEST(test_write_data16)
{
	assert_write_data_len(0, 16);
	assert_write_data_len(1, 16);
}
END_TEST

START_TEST(test_write_data24)
{
	assert_write_data_len(0, 24);
	assert_write_data_len(1, 24);
}
END_TEST

START_TEST(test_write_data32)
{
	assert_write_data_len(0, 32);
	assert_write_data_len(1, 32);
}
END_TEST


/*******************************************************************************
 * add length header before current data
 */

#define assert_wrap_data(init, bits) ({ \
	bio_writer_t *writer; \
	chunk_t buf, data; \
	int i, len = bits / 8; \
	writer = bio_writer_create(init); \
	writer->wrap##bits(writer); \
	buf = writer->get_buf(writer); \
	ck_assert_int_eq(buf.len, len); \
	ck_assert_int_eq(buf.ptr[len - 1], 0); \
	writer->destroy(writer); \
	data = chunk_alloca(32); \
	memset(data.ptr, 0, data.len); \
	for (i = 0; i < 32; i++) \
	{ \
		data.ptr[i] = i; \
		data.len = i; \
		writer = bio_writer_create(init); \
		writer->write_data(writer, data); \
		writer->wrap##bits(writer); \
		buf = writer->get_buf(writer); \
		ck_assert_int_eq(buf.len, len + i); \
		ck_assert_int_eq(buf.ptr[len - 1], i); \
		ck_assert(chunk_equals(chunk_create(buf.ptr + len, buf.len - len), data)); \
		writer->wrap##bits(writer); \
		buf = writer->get_buf(writer); \
		ck_assert_int_eq(buf.len, 2 * len + i); \
		ck_assert_int_eq(buf.ptr[len - 1], len + i); \
		ck_assert(chunk_equals(chunk_create(buf.ptr + 2 * len, buf.len - 2 * len), data)); \
		writer->destroy(writer); \
	} \
})

START_TEST(test_wrap8)
{
	assert_wrap_data(0, 8);
	assert_wrap_data(1, 8);
}
END_TEST

START_TEST(test_wrap16)
{
	assert_wrap_data(0, 16);
	assert_wrap_data(1, 16);
}
END_TEST

START_TEST(test_wrap24)
{
	assert_wrap_data(0, 24);
	assert_wrap_data(1, 24);
}
END_TEST

START_TEST(test_wrap32)
{
	assert_wrap_data(0, 32);
	assert_wrap_data(1, 32);
}
END_TEST

/*******************************************************************************
 * test data extraction
 */

START_TEST(test_get_buf)
{
	bio_writer_t *writer;
	chunk_t data1, data2;

	writer = bio_writer_create(0);
	writer->write_uint8(writer, 1);
	data1 = writer->get_buf(writer);
	ck_assert_int_eq(data1.len, 1);
	ck_assert(data1.ptr[0] == 1);

	data2 = writer->get_buf(writer);
	ck_assert(chunk_equals(data1, data2));
	ck_assert(data1.ptr == data2.ptr);
	writer->destroy(writer);
}
END_TEST

START_TEST(test_extract_buf)
{
	bio_writer_t *writer;
	chunk_t data1, data2;

	writer = bio_writer_create(0);
	writer->write_uint8(writer, 1);
	data1 = writer->extract_buf(writer);
	ck_assert_int_eq(data1.len, 1);
	ck_assert(data1.ptr[0] == 1);

	data2 = writer->get_buf(writer);
	ck_assert_int_eq(data2.len, 0);
	ck_assert(data2.ptr == NULL);
	data2 = writer->extract_buf(writer);
	ck_assert_int_eq(data2.len, 0);
	ck_assert(data2.ptr == NULL);

	writer->write_uint8(writer, 1);
	data2 = writer->get_buf(writer);
	ck_assert(chunk_equals(data1, data2));
	ck_assert(data1.ptr != data2.ptr);

	writer->destroy(writer);
	chunk_free(&data1);
}
END_TEST

Suite *bio_writer_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("bio_writer");

	tc = tcase_create("integer writes");
	tcase_add_test(tc, test_write_uint8);
	tcase_add_test(tc, test_write_uint16);
	tcase_add_test(tc, test_write_uint24);
	tcase_add_test(tc, test_write_uint32);
	tcase_add_test(tc, test_write_uint64);
	suite_add_tcase(s, tc);

	tc = tcase_create("data writes/skip");
	tcase_add_test(tc, test_write_data);
	tcase_add_test(tc, test_skip);
	suite_add_tcase(s, tc);

	tc = tcase_create("data length writes");
	tcase_add_test(tc, test_write_data8);
	tcase_add_test(tc, test_write_data16);
	tcase_add_test(tc, test_write_data24);
	tcase_add_test(tc, test_write_data32);
	suite_add_tcase(s, tc);

	tc = tcase_create("wrap writes");
	tcase_add_test(tc, test_wrap8);
	tcase_add_test(tc, test_wrap16);
	tcase_add_test(tc, test_wrap24);
	tcase_add_test(tc, test_wrap32);
	suite_add_tcase(s, tc);

	tc = tcase_create("get/extract");
	tcase_add_test(tc, test_get_buf);
	tcase_add_test(tc, test_extract_buf);
	suite_add_tcase(s, tc);

	return s;
}
