/*
 * Copyright (C) 2015 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
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

#include <test_suite.h>

#include "../vici_message.h"
#include "../vici_builder.h"

#include <unistd.h>

static char blob[] = {
	0xd3,0xe5,0xee,0x37,0x7b,0x96,0x2f,0x3e,0x5f,0x3e,0x91,0xea,0x38,0x44,0xba,0x6c,
	0x75,0xc8,0x42,0x32,0xaf,0x7a,0x66,0x43,0x33,0x92,0xd2,0xef,0x7d,0x91,0x7b,0x59,
	0x9f,0x9f,0xd1,0x44,0xb6,0x1e,0x8c,0xd1,0xc5,0xa0,0xd9,0xe4,0xf2,0x31,0xfd,0x7b,
	0x5b,0x56,0xa7,0xfe,0x63,0x0d,0xcb,0x31,0x74,0xd8,0xd6,0x4a,0x42,0x3a,0x88,0xf3,
	0x79,0xf9,0x41,0xa6,0xc0,0x64,0x53,0x31,0x42,0xe2,0xd4,0x4a,0x22,0x5f,0x3f,0x99,
	0xe0,0x1a,0xcb,0x93,0x26,0xd0,0xec,0xac,0x90,0x97,0x0a,0x5f,0x69,0x86,0xf1,0xda,
	0xfc,0xa7,0xac,0xd0,0xd8,0x81,0xcf,0x7d,0x47,0x22,0xbe,0xbf,0x00,0x9b,0x6b,0x86,
	0x92,0x89,0xbe,0x7f,0x74,0x13,0x53,0xf1,0x4c,0x2b,0xc9,0xe1,0x39,0xd6,0xfc,0x50,
	0x3f,0x00,0xfb,0x76,0x42,0xa6,0xa4,0x70,0xfc,0x93,0x17,0x4a,0x35,0xce,0x5e,0x78,
	0x41,0x88,0x24,0x50,0x78,0xf2,0x38,0x08,0xff,0x40,0xef,0x61,0xbb,0xbf,0x16,0xff,
	0x0b,0xf6,0x33,0x21,0xcb,0x48,0xbd,0x7d,0xd1,0x73,0xfa,0x6d,0xd6,0xab,0xde,0x69,
	0x63,0x17,0xdb,0x52,0xe2,0x75,0x4b,0xb7,0x1e,0xf0,0x8a,0x55,0x4f,0x70,0x8d,0x18,
	0xe5,0x38,0x6a,0x9f,0xb8,0x06,0xb5,0x91,0x90,0x2b,0xc5,0x67,0xa9,0x12,0xe5,0xf3,
	0x48,0x2f,0x80,0x03,0xa1,0xa0,0xfc,0x43,0xe9,0x0f,0x83,0x2b,0xbc,0x7c,0xa8,0x3b,
	0x6c,0xc1,0xc8,0x72,0x5f,0x87,0x63,0x77,0x93,0x9b,0xe2,0xd7,0x4e,0xe6,0x65,0xa1,
	0x69,0x00,0xda,0xf8,0xb4,0x61,0xee,0xb7,0x20,0xe7,0x2a,0x35,0x23,0xf0,0x37,0x4b,
	0x67,0xcf,0x8d,0x85,0x72,0x22,0x6d,0x7a,0xb2,0x96,0xff,0x49,0xf4,0x94,0x3e,0x7e,
	0x87,0x26,0x5d,0x34,0x05,0x26,0x60,0x9b,0x89,0xfe,0xf9,0x91,0xd3,0x03,0xe7,0x8a,
	0x03,0xf6,0x4e,0xbf,0x68,0x13,0xc6,0xf2,0x7b,0x9c,0xe6,0x36,0x1b,0xe2,0x22,0x44,
	0xb1,0x19,0x34,0x5f,0xe8,0x44,0x48,0x3a,0x19,0xe4,0xbd,0xb0,0x4e,0xb5,0x2c,0x40,
	0x55,0x39,0xe6,0x4c,0xd5,0x68,0x34,0x72,0x6b,0x6d,0x88,0xce,0x7e,0x77,0x95,0x17,
	0x2e,0x68,0x3f,0x0e,0x9d,0x70,0x9a,0x22,0xfa,0x19,0xcc,0x15,0x9d,0xba,0xaa,0xec,
	0xb1,0x67,0x19,0x51,0xce,0x60,0x9a,0x38,0xf8,0xa7,0x4e,0xe3,0x25,0x47,0x1e,0x1d,
	0x30,0x76,0x91,0x8f,0x4d,0x13,0x59,0x06,0x2f,0x01,0x10,0x95,0xdb,0x08,0x7c,0x46,
	0xed,0x47,0xa1,0x19,0x4c,0x46,0xd1,0x3a,0x3f,0x88,0x7a,0x63,0xae,0x29,0x13,0x42,
	0xe9,0x17,0xe8,0xa9,0x95,0xfc,0xd1,0xea,0xfa,0x59,0x90,0xfe,0xb7,0xbb,0x7f,0x61,
	0x1b,0xcb,0x3d,0x12,0x99,0x96,0x3e,0x23,0x23,0xec,0x3a,0x4d,0x86,0x86,0x74,0xef,
	0x38,0xa6,0xdc,0x3a,0x83,0x85,0xf8,0xb8,0xad,0x5b,0x33,0x94,0x4d,0x0e,0x68,0xbc,
	0xf2,0xc7,0x6f,0x84,0x18,0x1e,0x5a,0x66,0x1f,0x6c,0x98,0x33,0xda,0xde,0x9e,0xda,
	0x82,0xd0,0x56,0x44,0x47,0x08,0x0c,0x07,0x81,0x9d,0x8b,0x64,0x16,0x73,0x9d,0x80,
	0x54,0x9c,0x4c,0x42,0xde,0x27,0x4e,0x97,0xb2,0xcf,0x48,0xaf,0x7e,0x85,0xc1,0xcd,
	0x6a,0x4d,0x04,0x40,0x89,0xa3,0x9d,0x4e,0x89,0x56,0x60,0x31,0x1f,0x3f,0x49,0x16,
};

typedef struct {
	vici_type_t type;
	char *name;
	chunk_t data;
} endecode_test_t;

static endecode_test_t endecode_test_simple[] = {
	{ VICI_SECTION_START,			"section1", {}							},
	{  VICI_KEY_VALUE,				"key1",		{ "value1", 6 }				},
	{  VICI_KEY_VALUE,				"key2",		{ "value2", 6 }				},
	{ VICI_SECTION_END,				NULL,		{}							},
	{ VICI_END,						NULL,		{}							},
};

static endecode_test_t endecode_test_nested[] = {
	{ VICI_SECTION_START,			"section1", {}							},
	{  VICI_SECTION_START,			"section2", {}							},
	{   VICI_SECTION_START,			"section3", {}							},
	{    VICI_KEY_VALUE,			"key1",		{ "value1", 6 }				},
	{    VICI_SECTION_START,		"section4", {}							},
	{     VICI_KEY_VALUE,			"key2",		{ "value2", 6 }				},
	{    VICI_SECTION_END,			NULL,		{}							},
	{   VICI_SECTION_END,			NULL,		{}							},
	{  VICI_SECTION_END,			NULL,		{}							},
	{  VICI_KEY_VALUE,				"key3",		{ "value3", 6 }				},
	{ VICI_SECTION_END,				NULL,		{}							},
	{ VICI_END,						NULL,		{}							},
};

static endecode_test_t endecode_test_list[] = {
	{ VICI_SECTION_START,			"section1", {}							},
	{  VICI_LIST_START,				"list1",	{}							},
	{   VICI_LIST_ITEM,				NULL,		{ "item1", 5 }				},
	{   VICI_LIST_ITEM,				NULL,		{ "item2", 5 }				},
	{  VICI_LIST_END,				NULL,		{}							},
	{  VICI_KEY_VALUE,				"key1",		{ "value1", 6 }				},
	{ VICI_SECTION_END,				NULL,		{}							},
	{ VICI_END,						NULL,		{}							},
};

static endecode_test_t endecode_test_blobs[] = {
	{ VICI_KEY_VALUE,				"key1",		{ blob, countof(blob) }		},
	{ VICI_SECTION_START,			"section1", {}							},
	{  VICI_LIST_START,				"list1",	{}							},
	{   VICI_LIST_ITEM,				NULL,		{ blob, countof(blob) }		},
	{   VICI_LIST_ITEM,				NULL,		{ blob, countof(blob) }		},
	{  VICI_LIST_END,				NULL,		{}							},
	{  VICI_KEY_VALUE,				"key2",		{ blob, countof(blob) }		},
	{ VICI_SECTION_END,				NULL,		{}							},
	{ VICI_END,						NULL,		{}							},
};

static endecode_test_t *endecode_tests[] = {
	endecode_test_simple,
	endecode_test_nested,
	endecode_test_list,
	endecode_test_blobs,
};

typedef struct {
	enumerator_t public;
	endecode_test_t *next;
} endecode_enum_t;

METHOD(enumerator_t, endecode_enumerate, bool,
	endecode_enum_t *this, va_list args)
{
	vici_type_t *type;
	chunk_t *data;
	char **name;

	VA_ARGS_VGET(args, type, name, data);
	if (this->next)
	{
		*type = this->next->type;
		*name = this->next->name;
		*data = this->next->data;
		if (this->next->type == VICI_END)
		{
			this->next = NULL;
		}
		else
		{
			this->next++;
		}
		return TRUE;
	}
	return FALSE;
}

static enumerator_t *endecode_create_enumerator(endecode_test_t *test)
{
	endecode_enum_t *enumerator;

	INIT(enumerator,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _endecode_enumerate,
			.destroy = (void*)free,
		},
		.next = test,
	);

	return &enumerator->public;
}

static void compare_vici(enumerator_t *parse, enumerator_t *tmpl)
{
	vici_type_t type, ttype;
	char *name, *tname;
	chunk_t data, tdata;;

	while (TRUE)
	{
		ck_assert(parse->enumerate(parse, &type, &name, &data));
		ck_assert(tmpl->enumerate(tmpl, &ttype, &tname, &tdata));
		ck_assert_int_eq(type, ttype);
		switch (type)
		{
			case VICI_END:
				return;
			case VICI_SECTION_START:
			case VICI_LIST_START:
				ck_assert(streq(name, tname));
				break;
			case VICI_LIST_ITEM:
				ck_assert(chunk_equals(data, tdata));
				break;
			case VICI_KEY_VALUE:
				ck_assert(streq(name, tname));
				ck_assert(chunk_equals(data, tdata));
				break;
			case VICI_SECTION_END:
			case VICI_LIST_END:
				break;
			default:
				ck_assert(FALSE);
				break;
		}
	}
}

START_TEST(test_endecode)
{
	enumerator_t *parse, *tmpl;
	vici_message_t *m;
	chunk_t data;

	tmpl = endecode_create_enumerator(endecode_tests[_i]);
	m = vici_message_create_from_enumerator(tmpl);
	ck_assert(m);
	data = chunk_clone(m->get_encoding(m));
	tmpl = endecode_create_enumerator(endecode_tests[_i]);
	parse = m->create_enumerator(m);
	ck_assert(parse);
	compare_vici(parse, tmpl);
	tmpl->destroy(tmpl);
	parse->destroy(parse);
	m->destroy(m);

	m = vici_message_create_from_data(data, TRUE);
	ck_assert(m);
	tmpl = endecode_create_enumerator(endecode_tests[_i]);
	parse = m->create_enumerator(m);
	ck_assert(parse);
	compare_vici(parse, tmpl);
	tmpl->destroy(tmpl);
	parse->destroy(parse);
	m->destroy(m);
}
END_TEST

START_TEST(test_vararg)
{
	enumerator_t *parse, *tmpl;
	vici_message_t *m;

	m = vici_message_create_from_args(
		VICI_SECTION_START, "section1",
		 VICI_LIST_START, "list1",
		  VICI_LIST_ITEM, chunk_from_str("item1"),
		  VICI_LIST_ITEM, chunk_from_str("item2"),
		 VICI_LIST_END,
		 VICI_KEY_VALUE, "key1", chunk_from_str("value1"),
		 VICI_SECTION_END,
		VICI_END);
	ck_assert(m);
	tmpl = endecode_create_enumerator(endecode_test_list);
	parse = m->create_enumerator(m);
	ck_assert(parse);

	compare_vici(parse, tmpl);

	m->destroy(m);
	tmpl->destroy(tmpl);
	parse->destroy(parse);
}
END_TEST

START_TEST(test_builder)
{
	enumerator_t *parse, *tmpl;
	vici_message_t *m;
	vici_builder_t *b;

	b = vici_builder_create();
	b->add(b, VICI_SECTION_START, "section1");
	b->add(b,  VICI_LIST_START, "list1");
	b->add(b,   VICI_LIST_ITEM, chunk_from_str("item1"));
	b->add(b,   VICI_LIST_ITEM, chunk_from_str("item2"));
	b->add(b,  VICI_LIST_END);
	b->add(b,  VICI_KEY_VALUE, "key1", chunk_from_str("value1"));
	b->add(b, VICI_SECTION_END);
	m = b->finalize(b);
	ck_assert(m);
	tmpl = endecode_create_enumerator(endecode_test_list);
	parse = m->create_enumerator(m);
	ck_assert(parse);

	compare_vici(parse, tmpl);

	m->destroy(m);
	tmpl->destroy(tmpl);
	parse->destroy(parse);
}
END_TEST

START_TEST(test_builder_fmt)
{
	enumerator_t *parse, *tmpl;
	vici_message_t *m;
	vici_builder_t *b;

	b = vici_builder_create();
	b->begin_section(b, "section1");
	b->begin_list(b, "list1");
	b->add_li(b, "item%u", 1);
	b->add_li(b, "%s%u", "item", 2);
	b->end_list(b);
	b->add_kv(b, "key1", "value%u", 1);
	b->end_section(b);
	m = b->finalize(b);
	ck_assert(m);
	tmpl = endecode_create_enumerator(endecode_test_list);
	parse = m->create_enumerator(m);
	ck_assert(parse);

	compare_vici(parse, tmpl);

	m->destroy(m);
	tmpl->destroy(tmpl);
	parse->destroy(parse);
}
END_TEST

static vici_message_t* build_getter_msg()
{
	return vici_message_create_from_args(
			VICI_KEY_VALUE, "key1", chunk_from_str("1"),
			VICI_SECTION_START, "section1",
			 VICI_KEY_VALUE, "key2", chunk_from_str("0x12"),
			 VICI_SECTION_START, "section2",
			  VICI_KEY_VALUE, "key3", chunk_from_str("-1"),
			 VICI_SECTION_END,
			 VICI_KEY_VALUE, "key4", chunk_from_str("asdf"),
			VICI_SECTION_END,
			VICI_KEY_VALUE, "key5", chunk_from_str(""),
			VICI_END);
}

START_TEST(test_get_str)
{
	vici_message_t *m;

	m = build_getter_msg();

	ck_assert_str_eq(m->get_str(m, "def", "key1"), "1");
	ck_assert_str_eq(m->get_str(m, "def", "section1.key2"), "0x12");
	ck_assert_str_eq(m->get_str(m, "def", "section%d.section2.key3", 1), "-1");
	ck_assert_str_eq(m->get_str(m, "def", "section1.key4"), "asdf");
	ck_assert_str_eq(m->get_str(m, "def", "key5"), "");
	ck_assert_str_eq(m->get_str(m, "no", "nonexistent"), "no");
	ck_assert_str_eq(m->get_str(m, "no", "n.o.n.e.x.i.s.t.e.n.t"), "no");

	m->destroy(m);
}
END_TEST

START_TEST(test_get_int)
{
	vici_message_t *m;

	m = build_getter_msg();

	ck_assert_int_eq(m->get_int(m, 2, "key1"), 1);
	ck_assert_int_eq(m->get_int(m, 2, "section1.key2"), 0x12);
	ck_assert_int_eq(m->get_int(m, 2, "section1.section2.key3"), -1);
	ck_assert_int_eq(m->get_int(m, 2, "section1.key4"), 2);
	ck_assert_int_eq(m->get_int(m, 2, "key5"), 2);
	ck_assert_int_eq(m->get_int(m, 2, "nonexistent"), 2);
	ck_assert_int_eq(m->get_int(m, 2, "n.o.n.e.x.i.s.t.e.n.t"), 2);

	m->destroy(m);
}
END_TEST

START_TEST(test_get_bool)
{
	vici_message_t *m;

	m = build_getter_msg();

	ck_assert(m->get_bool(m, TRUE, "key1"));
	ck_assert(m->get_bool(m, FALSE, "key1"));

	ck_assert(m->get_bool(m, TRUE, "section1.key2"));
	ck_assert(m->get_bool(m, TRUE, "section1.section2.key3"));
	ck_assert(m->get_bool(m, TRUE, "section1.key4"));
	ck_assert(m->get_bool(m, TRUE, "key5"));
	ck_assert(m->get_bool(m, TRUE, "nonexistent"));
	ck_assert(m->get_bool(m, TRUE, "n.o.n.e.x.i.s.t.e.n.t"));

	ck_assert(!m->get_bool(m, FALSE, "section1.key2"));
	ck_assert(!m->get_bool(m, FALSE, "section1.section2.key3"));
	ck_assert(!m->get_bool(m, FALSE, "section1.key4"));
	ck_assert(!m->get_bool(m, FALSE, "key5"));
	ck_assert(!m->get_bool(m, FALSE, "nonexistent"));
	ck_assert(!m->get_bool(m, FALSE, "n.o.n.e.x.i.s.t.e.n.t"));

	m->destroy(m);
}
END_TEST

START_TEST(test_get_value)
{
	vici_message_t *m;
	chunk_t d = chunk_from_chars('d','e','f');

	m = build_getter_msg();

	ck_assert_chunk_eq(m->get_value(m, d, "key1"), chunk_from_str("1"));
	ck_assert_chunk_eq(m->get_value(m, d, "section1.key2"), chunk_from_str("0x12"));
	ck_assert_chunk_eq(m->get_value(m, d, "section1.section2.key3"), chunk_from_str("-1"));
	ck_assert_chunk_eq(m->get_value(m, d, "section1.key4"), chunk_from_str("asdf"));
	ck_assert_chunk_eq(m->get_value(m, d, "key5"), chunk_empty);
	ck_assert_chunk_eq(m->get_value(m, d, "nonexistent"), d);
	ck_assert_chunk_eq(m->get_value(m, d, "n.o.n.e.x.i.s.t.e.n.t"), d);

	m->destroy(m);
}
END_TEST

Suite *message_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("vici message");

	tc = tcase_create("enumerator en/decode");
	tcase_add_loop_test(tc, test_endecode, 0, countof(endecode_tests));
	suite_add_tcase(s, tc);

	tc = tcase_create("vararg encode");
	tcase_add_test(tc, test_vararg);
	suite_add_tcase(s, tc);

	tc = tcase_create("builder encode");
	tcase_add_test(tc, test_builder);
	suite_add_tcase(s, tc);

	tc = tcase_create("builder format encode");
	tcase_add_test(tc, test_builder_fmt);
	suite_add_tcase(s, tc);

	tc = tcase_create("convenience getters");
	tcase_add_test(tc, test_get_str);
	tcase_add_test(tc, test_get_int);
	tcase_add_test(tc, test_get_bool);
	tcase_add_test(tc, test_get_value);
	suite_add_tcase(s, tc);

	return s;
}
