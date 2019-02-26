/*
 * Copyright (C) 2014 Tobias Brunner
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

#include <unistd.h>

#include <test_suite.h>

#include "../../parser/conf_parser.h"

static char *path = "/tmp/strongswan-starter-parser-test";
static conf_parser_t *parser;

static void create_parser(chunk_t contents)
{
	ck_assert(chunk_write(contents, path, 0022, TRUE));
	parser = conf_parser_create(path);
}

START_TEARDOWN(teardown_parser)
{
	parser->destroy(parser);
	unlink(path);
}
END_TEARDOWN

START_TEST(test_get_sections_config_setup)
{
	enumerator_t *enumerator;

	create_parser(chunk_from_str(""));
	ck_assert(parser->parse(parser));
	enumerator = parser->get_sections(parser, CONF_PARSER_CONFIG_SETUP);
	ck_assert(enumerator);
	ck_assert(!enumerator->enumerate(enumerator, NULL));
	enumerator->destroy(enumerator);
	parser->destroy(parser);

	create_parser(chunk_from_str("config setup\n\tfoo=bar"));
	ck_assert(parser->parse(parser));
	enumerator = parser->get_sections(parser, CONF_PARSER_CONFIG_SETUP);
	ck_assert(enumerator);
	ck_assert(!enumerator->enumerate(enumerator, NULL));
	enumerator->destroy(enumerator);
}
END_TEST

START_TEST(test_get_sections_conn)
{
	enumerator_t *enumerator;
	char *name;

	create_parser(chunk_from_str(""));
	ck_assert(parser->parse(parser));
	enumerator = parser->get_sections(parser, CONF_PARSER_CONN);
	ck_assert(enumerator);
	ck_assert(!enumerator->enumerate(enumerator, NULL));
	enumerator->destroy(enumerator);
	parser->destroy(parser);

	create_parser(chunk_from_str(
		"conn foo\n"
		"conn bar\n"
		"conn foo\n"));
	ck_assert(parser->parse(parser));
	enumerator = parser->get_sections(parser, CONF_PARSER_CONN);
	ck_assert(enumerator);
	ck_assert(enumerator->enumerate(enumerator, &name));
	ck_assert_str_eq("foo", name);
	ck_assert(enumerator->enumerate(enumerator, &name));
	ck_assert_str_eq("bar", name);
	ck_assert(!enumerator->enumerate(enumerator, &name));
	enumerator->destroy(enumerator);
}
END_TEST

START_TEST(test_get_section_config_setup)
{
	dictionary_t *dict;

	create_parser(chunk_from_str(""));
	ck_assert(parser->parse(parser));
	dict = parser->get_section(parser, CONF_PARSER_CONFIG_SETUP, "foo");
	ck_assert(dict);
	dict->destroy(dict);
	parser->destroy(parser);

	create_parser(chunk_from_str("config setup\n\tfoo=bar"));
	ck_assert(parser->parse(parser));
	dict = parser->get_section(parser, CONF_PARSER_CONFIG_SETUP, NULL);
	ck_assert(dict);
	dict->destroy(dict);
	parser->destroy(parser);

	create_parser(chunk_from_str("config setup\n\tfoo=bar"));
	ck_assert(parser->parse(parser));
	dict = parser->get_section(parser, CONF_PARSER_CONFIG_SETUP, "foo");
	ck_assert(dict);
	dict->destroy(dict);
}
END_TEST

START_TEST(test_get_section_conn)
{
	dictionary_t *dict;

	create_parser(chunk_from_str(""));
	ck_assert(parser->parse(parser));
	dict = parser->get_section(parser, CONF_PARSER_CONN, "foo");
	ck_assert(!dict);
	parser->destroy(parser);

	create_parser(chunk_from_str("conn foo\n"));
	ck_assert(parser->parse(parser));
	dict = parser->get_section(parser, CONF_PARSER_CONN, "foo");
	ck_assert(!parser->get_section(parser, CONF_PARSER_CONN, "bar"));
	ck_assert(dict);
	dict->destroy(dict);
	parser->destroy(parser);

	create_parser(chunk_from_str("conn foo\n\tfoo=bar"));
	ck_assert(parser->parse(parser));
	dict = parser->get_section(parser, CONF_PARSER_CONN, "foo");
	ck_assert(dict);
	dict->destroy(dict);
}
END_TEST

START_TEST(test_enumerate_values)
{
	enumerator_t *enumerator;
	dictionary_t *dict;
	char *key, *value;
	int i;

	create_parser(chunk_from_str(
		"conn foo\n"
		"	foo=bar\n"
		"	bar=baz"));
	ck_assert(parser->parse(parser));
	dict = parser->get_section(parser, CONF_PARSER_CONN, "foo");
	ck_assert(dict);
	ck_assert_str_eq("bar", dict->get(dict, "foo"));
	ck_assert_str_eq("baz", dict->get(dict, "bar"));
	enumerator = dict->create_enumerator(dict);
	for (i = 0; enumerator->enumerate(enumerator, &key, &value); i++)
	{
		if ((streq(key, "foo") && !streq(value, "bar")) ||
			(streq(key, "bar") && !streq(value, "baz")))
		{
			fail("unexpected setting %s=%s", key, value);
		}
	}
	enumerator->destroy(enumerator);
	ck_assert_int_eq(i, 2);
	dict->destroy(dict);
}
END_TEST

#define extensibility_config(section) \
	section "\n" \
	"	foo=bar\n" \
	"	dup=one\n" \
	"	dup=two\n" \
	"\n" \
	"	nope=val\n" \
	"\n" \
	section "\n" \
	"	foo=baz\n" \
	section "\n" \
	"	answer=42\n" \
	"	nope=\n"

static struct {
	char *conf;
	conf_parser_section_t type;
	char *name;
} extensibility_data[] = {
	{ extensibility_config("config setup"), CONF_PARSER_CONFIG_SETUP, NULL },
	{ extensibility_config("ca ca-foo"), CONF_PARSER_CA, "ca-foo" },
	{ extensibility_config("conn conn-foo"), CONF_PARSER_CONN, "conn-foo" },
};

START_TEST(test_extensibility)
{
	dictionary_t *dict;

	create_parser(chunk_from_str(extensibility_data[_i].conf));
	ck_assert(parser->parse(parser));

	dict = parser->get_section(parser, extensibility_data[_i].type,
							   extensibility_data[_i].name);
	ck_assert(dict);
	ck_assert_str_eq("baz", dict->get(dict, "foo"));
	ck_assert_str_eq("two", dict->get(dict, "dup"));
	ck_assert_str_eq("42", dict->get(dict, "answer"));
	ck_assert(!dict->get(dict, "nope"));
	ck_assert(!dict->get(dict, "anything"));
	dict->destroy(dict);
}
END_TEST

static struct {
	char *conf;
	bool check_section;
	char *value;
} comments_data[] = {
	{ "# conn foo", FALSE, NULL },
	{ "# conn foo\n", FALSE, NULL },
	{ "conn foo # asdf", TRUE, NULL },
	{ "conn foo # asdf", TRUE, NULL },
	{ "conn foo# asdf\n", TRUE, NULL },
	{ "conn foo # asdf\n\tkey=val", TRUE, "val" },
	{ "conn foo # asdf\n#\tkey=val", TRUE, NULL },
	{ "conn foo # asdf\n\t#key=val", TRUE, NULL },
	{ "conn foo # asdf\n\tkey=@#keyid", TRUE, "@#keyid" },
	{ "conn foo # asdf\n\tkey=\"@#keyid\"", TRUE, "@#keyid" },
	{ "conn foo # asdf\n\tkey=asdf@#keyid", TRUE, "asdf@" },
	{ "conn foo # asdf\n\tkey=#val", TRUE, NULL },
	{ "conn foo # asdf\n\tkey=val#asdf", TRUE, "val" },
	{ "conn foo # asdf\n\tkey=\"val#asdf\"", TRUE, "val#asdf" },
	{ "conn foo # asdf\n\tkey=val # asdf\n", TRUE, "val" },
	{ "conn foo # asdf\n# asdf\n\tkey=val\n", TRUE, "val" },
	{ "conn foo # asdf\n\t# asdf\n\tkey=val\n", TRUE, "val" },
};

START_TEST(test_comments)
{
	dictionary_t *dict;

	create_parser(chunk_from_str(comments_data[_i].conf));
	ck_assert(parser->parse(parser));
	if (comments_data[_i].check_section)
	{
		dict = parser->get_section(parser, CONF_PARSER_CONN, "foo");
		ck_assert(dict);
		if (comments_data[_i].value)
		{
			ck_assert_str_eq(comments_data[_i].value, dict->get(dict, "key"));
		}
		else
		{
			ck_assert(!dict->get(dict, "key"));
		}
		dict->destroy(dict);
	}
	else
	{
		ck_assert(!parser->get_section(parser, CONF_PARSER_CONN, "foo"));
	}
}
END_TEST

static struct {
	char *conf;
	bool check_section;
	char *value;
} whitespace_data[] = {
	{ "conn foo   ", FALSE, NULL },
	{ "conn    foo", FALSE, NULL },
	{ "conn    foo\n", FALSE, NULL },
	{ "conn    foo  \n", FALSE, NULL },
	{ "conn foo\n   ", FALSE, NULL },
	{ "conn foo\n   \n", FALSE, NULL },
	{ "conn foo\nconn bar", TRUE, NULL },
	{ "conn foo\n   \nconn bar", TRUE, NULL },
	{ "conn foo\n key=val", FALSE, "val" },
	{ "conn foo\n\tkey=val", FALSE, "val" },
	{ "conn foo\n\t  \tkey=val", FALSE, "val" },
	{ "conn foo\n\tkey  =  val  ", FALSE, "val" },
};

START_TEST(test_whitespace)
{
	dictionary_t *dict;

	create_parser(chunk_from_str(whitespace_data[_i].conf));
	ck_assert(parser->parse(parser));
	dict = parser->get_section(parser, CONF_PARSER_CONN, "foo");
	ck_assert(dict);
	if (whitespace_data[_i].value)
	{
		ck_assert_str_eq(whitespace_data[_i].value, dict->get(dict, "key"));
	}
	else
	{
		ck_assert(!dict->get(dict, "key"));
	}
	dict->destroy(dict);
	if (whitespace_data[_i].check_section)
	{
		dict = parser->get_section(parser, CONF_PARSER_CONN, "bar");
		ck_assert(dict);
		dict->destroy(dict);
	}
	else
	{
		ck_assert(!parser->get_section(parser, CONF_PARSER_CONN, "bar"));
	}
}
END_TEST

static struct {
	bool valid;
	char *conf;
	char *section;
	char *value;
} strings_data[] = {
	{ FALSE, "\"conn foo\"", NULL, NULL },
	{ TRUE, "conn \"foo\"", "foo", NULL },
	{ FALSE, "conn foo bar", NULL, NULL },
	{ TRUE, "conn \"foo bar\"", "foo bar", NULL },
	{ TRUE, "conn \"#foo\"", "#foo", NULL },
	{ FALSE, "conn foo\n\t\"key=val\"", "foo", NULL },
	{ TRUE, "conn foo\n\t\"key\"=val", "foo", "val" },
	{ TRUE, "conn foo\n\tkey=val ue", "foo", "val ue" },
	{ TRUE, "conn foo\n\tkey=val     ue", "foo", "val ue" },
	{ TRUE, "conn foo\n\tkey=\"val   ue\"", "foo", "val   ue" },
	{ TRUE, "conn foo\n\tkey=\"val\\nue\"", "foo", "val\nue" },
	{ TRUE, "conn foo\n\tkey=\"val\nue\"", "foo", "val\nue" },
	{ TRUE, "conn foo\n\tkey=\"val\\\nue\"", "foo", "value" },
	{ FALSE, "conn foo\n\tkey=\"unterminated", "foo", NULL },
};

START_TEST(test_strings)
{
	dictionary_t *dict;

	create_parser(chunk_from_str(strings_data[_i].conf));
	ck_assert(parser->parse(parser) == strings_data[_i].valid);
	if (strings_data[_i].section)
	{
		dict = parser->get_section(parser, CONF_PARSER_CONN,
								   strings_data[_i].section);
		ck_assert(dict);
		if (strings_data[_i].value)
		{
			ck_assert_str_eq(strings_data[_i].value, dict->get(dict, "key"));
		}
		else
		{
			ck_assert(!dict->get(dict, "key"));
		}
		dict->destroy(dict);
	}
}
END_TEST

START_TEST(test_refcounting)
{
	dictionary_t *dict;

	create_parser(chunk_from_str(
		"conn foo\n"
		"	key=val"));

	ck_assert(parser->parse(parser));
	dict = parser->get_section(parser, CONF_PARSER_CONN, "foo");
	ck_assert(dict);
	ck_assert_str_eq("val", dict->get(dict, "key"));
	parser->destroy(parser);
	ck_assert_str_eq("val", dict->get(dict, "key"));
	dict->destroy(dict);
}
END_TEST

START_TEST(test_default)
{
	enumerator_t *enumerator;
	dictionary_t *dict;
	char *name;

	create_parser(chunk_from_str(
		"conn %default\n"
		"	key=valdef\n"
		"	unset=set\n"
		"conn A\n"
		"	key=vala\n"
		"	unset=\n"
		"conn B\n"
		"	keyb=valb\n"
		""));

	ck_assert(parser->parse(parser));
	dict = parser->get_section(parser, CONF_PARSER_CONN, "%default");
	ck_assert(!dict);
	enumerator = parser->get_sections(parser, CONF_PARSER_CONN);
	ck_assert(enumerator);
	ck_assert(enumerator->enumerate(enumerator, &name));
	ck_assert_str_eq("A", name);
	ck_assert(enumerator->enumerate(enumerator, &name));
	ck_assert_str_eq("B", name);
	ck_assert(!enumerator->enumerate(enumerator, &name));
	enumerator->destroy(enumerator);

	dict = parser->get_section(parser, CONF_PARSER_CONN, "A");
	ck_assert(dict);
	ck_assert_str_eq("vala", dict->get(dict, "key"));
	ck_assert(!dict->get(dict, "unset"));
	dict->destroy(dict);
	dict = parser->get_section(parser, CONF_PARSER_CONN, "B");
	ck_assert(dict);
	ck_assert_str_eq("valdef", dict->get(dict, "key"));
	ck_assert_str_eq("valb", dict->get(dict, "keyb"));
	ck_assert_str_eq("set", dict->get(dict, "unset"));
	dict->destroy(dict);
}
END_TEST

START_TEST(test_also)
{
	dictionary_t *dict;

	create_parser(chunk_from_str(
		"conn A\n"
		"	key=vala\n"
		"	keya=val1\n"
		"	unset=set\n"
		"conn B\n"
		"	also=A\n"
		"	key=valb\n"
		"	keyb=val2\n"
		"	unset=\n"
		"conn C\n"
		"	keyc=val3\n"
		"	unset=set again\n"
		"	also=B\n"
		"conn D\n"
		"	keyd=val4\n"
		"	also=A\n"
		"	also=B\n"
		"conn E\n"
		"	keye=val5\n"
		"	also=B\n"
		"	also=A\n"
		""));

	ck_assert(parser->parse(parser));
	dict = parser->get_section(parser, CONF_PARSER_CONN, "B");
	ck_assert(dict);
	ck_assert_str_eq("valb", dict->get(dict, "key"));
	ck_assert_str_eq("val1", dict->get(dict, "keya"));
	ck_assert_str_eq("val2", dict->get(dict, "keyb"));
	ck_assert(!dict->get(dict, "unset"));
	dict->destroy(dict);
	dict = parser->get_section(parser, CONF_PARSER_CONN, "C");
	ck_assert(dict);
	ck_assert_str_eq("valb", dict->get(dict, "key"));
	ck_assert_str_eq("val1", dict->get(dict, "keya"));
	ck_assert_str_eq("val2", dict->get(dict, "keyb"));
	ck_assert_str_eq("val3", dict->get(dict, "keyc"));
	ck_assert_str_eq("set again", dict->get(dict, "unset"));
	dict->destroy(dict);
	/* since B includes A too the inclusion in D and E has no effect */
	dict = parser->get_section(parser, CONF_PARSER_CONN, "D");
	ck_assert(dict);
	ck_assert_str_eq("valb", dict->get(dict, "key"));
	ck_assert_str_eq("val1", dict->get(dict, "keya"));
	ck_assert_str_eq("val2", dict->get(dict, "keyb"));
	ck_assert(!dict->get(dict, "keyc"));
	ck_assert_str_eq("val4", dict->get(dict, "keyd"));
	ck_assert(!dict->get(dict, "unset"));
	dict->destroy(dict);
	dict = parser->get_section(parser, CONF_PARSER_CONN, "E");
	ck_assert(dict);
	ck_assert_str_eq("valb", dict->get(dict, "key"));
	ck_assert_str_eq("val1", dict->get(dict, "keya"));
	ck_assert_str_eq("val2", dict->get(dict, "keyb"));
	ck_assert(!dict->get(dict, "keyc"));
	ck_assert(!dict->get(dict, "keyd"));
	ck_assert_str_eq("val5", dict->get(dict, "keye"));
	ck_assert(!dict->get(dict, "unset"));
	dict->destroy(dict);
}
END_TEST

START_TEST(test_ambiguous)
{
	dictionary_t *dict;

	create_parser(chunk_from_str(
		"conn A\n"
		"	key=vala\n"
		"conn B\n"
		"	key=valb\n"
		"conn C\n"
		"	also=A\n"
		"	also=B\n"
		"conn D\n"
		"	also=B\n"
		"	also=A\n"
		"conn E\n"
		"	also=C\n"
		"	also=D\n"
		"conn F\n"
		"	also=D\n"
		"	also=C\n"));

	ck_assert(parser->parse(parser));
	dict = parser->get_section(parser, CONF_PARSER_CONN, "E");
	ck_assert(dict);
	ck_assert_str_eq("valb", dict->get(dict, "key"));
	dict->destroy(dict);
	dict = parser->get_section(parser, CONF_PARSER_CONN, "F");
	ck_assert(dict);
	ck_assert_str_eq("vala", dict->get(dict, "key"));
	dict->destroy(dict);
}
END_TEST

Suite *parser_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("ipsec.conf parser");

	tc = tcase_create("get_section(s)");
	tcase_add_checked_fixture(tc, NULL, teardown_parser);
	tcase_add_test(tc, test_get_sections_config_setup);
	tcase_add_test(tc, test_get_sections_conn);
	tcase_add_test(tc, test_get_section_config_setup);
	tcase_add_test(tc, test_get_section_conn);
	suite_add_tcase(s, tc);

	tc = tcase_create("enumerate settings");
	tcase_add_checked_fixture(tc, NULL, teardown_parser);
	tcase_add_test(tc, test_enumerate_values);
	suite_add_tcase(s, tc);

	tc = tcase_create("extensibility");
	tcase_add_checked_fixture(tc, NULL, teardown_parser);
	tcase_add_loop_test(tc, test_extensibility, 0, countof(extensibility_data));
	suite_add_tcase(s, tc);

	tc = tcase_create("comments");
	tcase_add_checked_fixture(tc, NULL, teardown_parser);
	tcase_add_loop_test(tc, test_comments, 0, countof(comments_data));
	suite_add_tcase(s, tc);

	tc = tcase_create("whitespace");
	tcase_add_checked_fixture(tc, NULL, teardown_parser);
	tcase_add_loop_test(tc, test_whitespace, 0, countof(whitespace_data));
	suite_add_tcase(s, tc);

	tc = tcase_create("strings");
	tcase_add_checked_fixture(tc, NULL, teardown_parser);
	tcase_add_loop_test(tc, test_strings, 0, countof(strings_data));
	suite_add_tcase(s, tc);

	tc = tcase_create("refcounting");
	tcase_add_test(tc, test_refcounting);
	suite_add_tcase(s, tc);

	tc = tcase_create("%default");
	tcase_add_checked_fixture(tc, NULL, teardown_parser);
	tcase_add_test(tc, test_default);
	suite_add_tcase(s, tc);

	tc = tcase_create("also=");
	tcase_add_checked_fixture(tc, NULL, teardown_parser);
	tcase_add_test(tc, test_also);
	tcase_add_test(tc, test_ambiguous);
	suite_add_tcase(s, tc);

	return s;
}
