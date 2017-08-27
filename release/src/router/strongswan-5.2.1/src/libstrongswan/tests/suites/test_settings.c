/*
 * Copyright (C) 2014 Tobias Brunner
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

#include <settings/settings.h>
#include <utils/chunk.h>
#include <utils/utils.h>
#include <collections/linked_list.h>

#ifdef WIN32
static char *path = "C:\\Windows\\Temp\\strongswan-settings-test";
#else
static char *path = "/tmp/strongswan-settings-test";
#endif
static settings_t *settings;

static void create_settings(chunk_t contents)
{
	ck_assert(chunk_write(contents, path, 0022, TRUE));
	settings = settings_create(path);
}

START_SETUP(setup_base_config)
{
	create_settings(chunk_from_str(
		"main {\n"
		"	key1 = val1\n"
		"	# this gets overridden below\n"
		"	key2 = val2\n"
		"	none = \n"
		"	empty = \"\"\n"
		"	sub1 {\n"
		"		key = value\n"
		"		key2 = value2\n"
		"		subsub {\n"
		"			foo = bar\n"
		"		}\n"
		"		# subsub is a section and a value\n"
		"		subsub = section value\n"
		"	}\n"
		"	sub% {\n"
		"		id = %any\n"
		"	}\n"
		"	key2 = with space\n"
		"	key3 = \"string with\\nnewline\"\n"
		"}\n"
		"out = side\n"
		"other {\n"
		"	key1 = other val\n"
		"	empty {\n"
		"	}\n"
		"}"));
}
END_SETUP

START_TEARDOWN(teardown_config)
{
	settings->destroy(settings);
	unlink(path);
}
END_TEARDOWN

#define verify_string(expected, key, ...) \
	ck_assert_str_eq(expected, settings->get_str(settings, key, NULL, ##__VA_ARGS__))
#define verify_null(key, ...) \
	ck_assert(!settings->get_str(settings, key, NULL, ##__VA_ARGS__))

START_TEST(test_get_str)
{
	verify_string("val1", "main.key1");
	verify_string("val1", "main..key1");
	verify_string("val1", ".main.key1");
	verify_string("", "main.empty");
	verify_string("with space", "main.key2");
	verify_string("string with\nnewline", "main.key3");
	verify_string("value", "main.sub1.key");
	verify_string("value2", "main.sub1.key2");
	verify_string("bar", "main.sub1.subsub.foo");
	verify_string("section value", "main.sub1.subsub");
	verify_string("%any", "main.sub%%.id");
	verify_string("side", "out");
	verify_string("other val", "other.key1");

	verify_null("main.none");
	verify_null("main.key4");
	verify_null("other.sub");
}
END_TEST

enum {
	KEY1,
	SUB1
} settings_test_enum;

enum_name_t *test_settings_test_names;

ENUM_BEGIN(test_settings_test_names, KEY1, SUB1,
	"key1", "sub1");
ENUM_END(test_settings_test_names, SUB1);

START_TEST(test_get_str_printf)
{
	verify_string("val1", "%s.key1", "main");
	verify_string("val1", "%s.%s", "main", "key1");
	verify_string("val1", "%s.%N", "main", test_settings_test_names, KEY1);
	verify_string("val1", "%s.%s%d", "main", "key", 1);
	verify_string("bar", "%s.sub1.%s.foo", "main", "subsub");
	verify_string("bar", "%s.%N.%s.foo", "main", test_settings_test_names, SUB1, "subsub");
	verify_string("bar", "%s.sub%d.%s.foo", "main", 1, "subsub");
	verify_string("%any", "%s.sub%%.id", "main");

	/* FIXME: this is a bit inconsistent, while this works */
	verify_string("value2", "main.%s%u.key2", "sub", 1);
	/* this won't because no argument is consumed for %u so key1 will be tried
	 * granted, we never actually used any other specifiers, but we should
	 * probably document it at least */
	verify_null("main.%s%u.key%d", "sub", 1, 2);

	verify_null("%s.%s%d", "main", "key", 4);
}
END_TEST

START_TEST(test_set_str)
{
	char *val1, *val2;

	val1 = settings->get_str(settings, "main.key1", NULL);
	ck_assert_str_eq("val1", val1);
	settings->set_str(settings, "main.key1", "val");
	verify_string("val", "main.key1");
	/* the pointer we got before is still valid */
	ck_assert_str_eq("val1", val1);

	val2 = settings->get_str(settings, "main.key1", NULL);
	ck_assert_str_eq("val", val2);
	settings->set_str(settings, "main.key1", "longer value");
	verify_string("longer value", "main.key1");
	/* the pointers we got before are still valid */
	ck_assert_str_eq("val1", val1);
	ck_assert_str_eq("val", val2);

	val1 = settings->get_str(settings, "main.key1", NULL);
	settings->set_str(settings, "main.key1", "longer value");
	val2 = settings->get_str(settings, "main.key1", NULL);
	/* setting the same string should should get us the same pointer */
	ck_assert(val1 == val2);

	settings->set_str(settings, "main", "main val");
	verify_string("main val", "main");
	settings->set_str(settings, "main.sub1.new", "added");
	verify_string("added", "main.sub1.new");
	settings->set_str(settings, "main.sub2.newsub.foo", "bar");
	verify_string("bar", "main.sub2.newsub.foo");
	settings->set_str(settings, "new.newsub.foo", "bar");
	verify_string("bar", "new.newsub.foo");
	settings->set_str(settings, "main.key1", NULL);
	verify_null("main.key1");
}
END_TEST

START_TEST(test_set_str_printf)
{
	settings->set_str(settings, "%s.key1", "val", "main");
	verify_string("val", "main.key1");
	settings->set_str(settings, "main.%N.new", "added", test_settings_test_names, SUB1);
	verify_string("added", "main.sub1.new");
	settings->set_str(settings, "main.%s%d.newsub.%s", "bar", "sub", 2, "foo");
	verify_string("bar", "main.sub2.newsub.foo");
}
END_TEST

START_TEST(test_set_default_str)
{
	settings->set_default_str(settings, "main.key1", "default");
	verify_string("val1", "main.key1");
	settings->set_default_str(settings, "main.sub1.new", "added");
	verify_string("added", "main.sub1.new");
	settings->set_str(settings, "main.sub1.new", "changed");
	verify_string("changed", "main.sub1.new");
}
END_TEST

START_SETUP(setup_bool_config)
{
	create_settings(chunk_from_str(
		"main {\n"
		"	key1 = yes\n"
		"	key2 = true\n"
		"	key3 = Enabled\n"
		"	key4 = 1\n"
		"	key5 = no\n"
		"	key6 = FALSE\n"
		"	key7 = disabled\n"
		"	key8 = 0\n"
		"	key9 = 5\n"
		"	empty = \"\"\n"
		"	none = \n"
		"	foo = bar\n"
		"}"));
}
END_SETUP

#define verify_bool(expected, def, key, ...) \
	ck_assert(expected == settings->get_bool(settings, key, def, ##__VA_ARGS__))

START_TEST(test_get_bool)
{
	verify_bool(TRUE, FALSE, "main.key1");
	verify_bool(TRUE, FALSE, "main.key2");
	verify_bool(TRUE, FALSE, "main.key3");
	verify_bool(TRUE, FALSE, "main.key4");
	verify_bool(FALSE, TRUE, "main.key5");
	verify_bool(FALSE, TRUE, "main.key6");
	verify_bool(FALSE, TRUE, "main.key7");
	verify_bool(FALSE, TRUE, "main.key8");

	verify_bool(FALSE, FALSE, "main.empty");
	verify_bool(TRUE, TRUE, "main.empty");
	verify_bool(FALSE, FALSE, "main.none");
	verify_bool(TRUE, TRUE, "main.none");
	verify_bool(FALSE, FALSE, "main.foo");
	verify_bool(TRUE, TRUE, "main.foo");

	verify_bool(FALSE, FALSE, "main.key9");
	verify_bool(TRUE, TRUE, "main.key9");
	verify_bool(FALSE, FALSE, "main");
	verify_bool(TRUE, TRUE, "main");

}
END_TEST

START_TEST(test_set_bool)
{
	settings->set_str(settings, "main.key1", "no");
	verify_bool(FALSE, TRUE, "main.key1");
	settings->set_bool(settings, "main.key2", FALSE);
	verify_bool(FALSE, TRUE, "main.key2");
	settings->set_str(settings, "main.key3", NULL);
	verify_bool(FALSE, FALSE, "main.key3");
	verify_bool(TRUE, TRUE, "main.key3");
	settings->set_bool(settings, "main.key5", TRUE);
	verify_bool(TRUE, FALSE, "main.key5");
	settings->set_bool(settings, "main.new", TRUE);
	verify_bool(TRUE, FALSE, "main.new");
}
END_TEST

START_SETUP(setup_int_config)
{
	create_settings(chunk_from_str(
		"main {\n"
		"	key1 = 5\n"
		"	key2 = 5.5\n"
		"	key3 = -42\n"
		"	empty = \"\"\n"
		"	none = \n"
		"	foo1 = bar\n"
		"	foo2 = bar13\n"
		"	foo3 = 13bar\n"
		"}"));
}
END_SETUP

#define verify_int(expected, def, key, ...) \
	ck_assert_int_eq(expected, settings->get_int(settings, key, def, ##__VA_ARGS__))

START_TEST(test_get_int)
{
	verify_int(5, 0, "main.key1");
	verify_int(0, 0, "main.key2");
	verify_int(-42, 0, "main.key3");

	verify_int(11, 11, "main.empty");
	verify_int(11, 11, "main.none");
	verify_int(11, 11, "main.foo1");
	verify_int(11, 11, "main.foo2");
	verify_int(11, 11, "main.foo3");

	verify_int(13, 13, "main.key4");
	verify_int(-13, -13, "main");
}
END_TEST

START_TEST(test_set_int)
{
	settings->set_str(settings, "main.key1", "13");
	verify_int(13, 0, "main.key1");
	settings->set_int(settings, "main.key2", 6);
	verify_int(6, 0, "main.key2");
	settings->set_int(settings, "main.key3", -6);
	verify_int(-6, 0, "main.key3");
	settings->set_str(settings, "main.key3", NULL);
	verify_int(15, 15, "main.key3");
	settings->set_int(settings, "main.new", 314);
	verify_int(314, 0, "main.new");
}
END_TEST

START_SETUP(setup_double_config)
{
	create_settings(chunk_from_str(
		"main {\n"
		"	key1 = 5\n"
		"	key2 = 5.5\n"
		"	key3 = -42\n"
		"	key4 = -42.5\n"
		"	empty = \"\"\n"
		"	none = \n"
		"	foo1 = bar\n"
		"	foo2 = bar13.5\n"
		"	foo3 = 13.5bar\n"
		"}"));
}
END_SETUP

#define verify_double(expected, def, key, ...) \
	ck_assert(expected == settings->get_double(settings, key, def, ##__VA_ARGS__))

START_TEST(test_get_double)
{
	verify_double(5, 0, "main.key1");
	verify_double(5.5, 0, "main.key2");
	verify_double(-42, 0, "main.key3");
	verify_double(-42.5, 0, "main.key4");

	verify_double(11.5, 11.5, "main.empty");
	verify_double(11.5, 11.5, "main.none");
	verify_double(11.5, 11.5, "main.foo1");
	verify_double(11.5, 11.5, "main.foo2");
	verify_double(11.5, 11.5, "main.foo3");

	verify_double(11.5, 11.5, "main.key5");
	verify_double(-11.5, -11.5, "main");
}
END_TEST

START_TEST(test_set_double)
{
	settings->set_str(settings, "main.key1", "5.5");
	verify_double(5.5, 0, "main.key1");
	settings->set_double(settings, "main.key2", 13);
	verify_double(13, 0, "main.key2");
	settings->set_double(settings, "main.key3", -13.5);
	verify_double(-13.5, 0, "main.key3");
	settings->set_double(settings, "main.key4", 11.5);
	verify_double(11.5, 0, "main.key4");
	settings->set_str(settings, "main.key4", NULL);
	verify_double(42.5, 42.5, "main.key4");
	settings->set_double(settings, "main.new", 3.14);
	verify_double(3.14, 0, "main.new");
}
END_TEST

START_SETUP(setup_time_config)
{
	create_settings(chunk_from_str(
		"main {\n"
		"	key0 = 5\n"
		"	key1 = 5s\n"
		"	key2 = 5m\n"
		"	key3 = 5 h\n"
		"	key4 = 5\td\n"
		"	empty = \"\"\n"
		"	none = \n"
		"	foo1 = bar\n"
		"	foo2 = bar13\n"
		"	foo3 = 13bar\n"
		"}"));
}
END_SETUP

#define verify_time(expected, def, key, ...) \
	ck_assert_int_eq(expected, settings->get_time(settings, key, def, ##__VA_ARGS__))

START_TEST(test_get_time)
{
	verify_time(5, 0, "main.key0");
	verify_time(5, 0, "main.key1");
	verify_time(300, 0, "main.key2");
	verify_time(18000, 0, "main.key3");
	verify_time(432000, 0, "main.key4");

	verify_time(11, 11, "main.empty");
	verify_time(11, 11, "main.none");
	verify_time(11, 11, "main.foo1");
	verify_time(11, 11, "main.foo2");
	verify_time(11, 11, "main.foo3");

	verify_time(11, 11, "main.key5");
	verify_time(11, 11, "main");
}
END_TEST

START_TEST(test_set_time)
{
	settings->set_str(settings, "main.key1", "15m");
	verify_time(900, 0, "main.key1");
	settings->set_time(settings, "main.key2", 15);
	verify_time(15, 0, "main.key2");
	settings->set_str(settings, "main.key3", NULL);
	verify_time(300, 300, "main.key3");
	settings->set_time(settings, "main.new", 314);
	verify_time(314, 0, "main.new");
}
END_TEST

static void verify_sections(linked_list_t *verifier, char *parent)
{
	enumerator_t *enumerator, *ver;
	char *section, *current;

	enumerator = settings->create_section_enumerator(settings, parent);
	ver = verifier->create_enumerator(verifier);
	while (enumerator->enumerate(enumerator, &section) &&
		   ver->enumerate(ver, &current))
	{
		ck_assert_str_eq(section, current);
		verifier->remove_at(verifier, ver);
	}
	enumerator->destroy(enumerator);
	ver->destroy(ver);
	ck_assert_int_eq(0, verifier->get_count(verifier));
	verifier->destroy(verifier);
}

START_TEST(test_section_enumerator)
{
	linked_list_t *verifier;

	verifier = linked_list_create_with_items("sub1", "sub%", NULL);
	verify_sections(verifier, "main");

	settings->set_str(settings, "main.sub0.new", "added");
	verifier = linked_list_create_with_items("sub1", "sub%", "sub0", NULL);
	verify_sections(verifier, "main");

	verifier = linked_list_create_with_items("subsub", NULL);
	verify_sections(verifier, "main.sub1");

	verifier = linked_list_create_with_items(NULL);
	verify_sections(verifier, "main.sub%%");

	verifier = linked_list_create_with_items(NULL);
	verify_sections(verifier, "main.key1");

	verifier = linked_list_create_with_items(NULL);
	verify_sections(verifier, "main.unknown");
}
END_TEST

static void verify_key_values(linked_list_t *keys, linked_list_t *values,
							  char *parent)
{
	enumerator_t *enumerator, *enum_keys, *enum_values;
	char *key, *value, *current_key, *current_value;

	enumerator = settings->create_key_value_enumerator(settings, parent);
	enum_keys = keys->create_enumerator(keys);
	enum_values = values->create_enumerator(values);
	while (enumerator->enumerate(enumerator, &key, &value) &&
		   enum_keys->enumerate(enum_keys, &current_key) &&
		   enum_values->enumerate(enum_values, &current_value))
	{
		ck_assert_str_eq(current_key, key);
		ck_assert_str_eq(current_value, value);
		keys->remove_at(keys, enum_keys);
		values->remove_at(values, enum_values);
	}
	enumerator->destroy(enumerator);
	enum_keys->destroy(enum_keys);
	enum_values->destroy(enum_values);
	ck_assert_int_eq(0, keys->get_count(keys));
	keys->destroy(keys);
	values->destroy(values);
}

START_TEST(test_key_value_enumerator)
{
	linked_list_t *keys, *values;

	keys = linked_list_create_with_items("key1", "key2", "empty", "key3", NULL);
	values = linked_list_create_with_items("val1", "with space", "", "string with\nnewline", NULL);
	verify_key_values(keys, values, "main");

	keys = linked_list_create_with_items("key", "key2", "subsub", NULL);
	values = linked_list_create_with_items("value", "value2", "section value", NULL);
	verify_key_values(keys, values, "main.sub1");

	settings->set_str(settings, "main.sub2.new", "added");
	keys = linked_list_create_with_items("new", NULL);
	values = linked_list_create_with_items("added", NULL);
	verify_key_values(keys, values, "main.sub2");

	keys = linked_list_create_with_items(NULL);
	values = linked_list_create_with_items(NULL);
	verify_key_values(keys, values, "other.empty");

	settings->set_str(settings, "other.empty.new", "added");
	keys = linked_list_create_with_items("new", NULL);
	values = linked_list_create_with_items("added", NULL);
	verify_key_values(keys, values, "other.empty");

	keys = linked_list_create_with_items(NULL);
	values = linked_list_create_with_items(NULL);
	verify_key_values(keys, values, "main.unknown");
}
END_TEST

#ifdef WIN32
# define include1 "C:\\Windows\\Temp\\strongswan-settings-test-include1"
# define include2 "C:\\Windows\\Temp\\strongswan-settings-test-include2"
#else
# define include1 "/tmp/strongswan-settings-test-include1"
# define include2 "/tmp/strongswan-settings-test-include2"
#endif

START_SETUP(setup_include_config)
{
	chunk_t inc1 = chunk_from_str(
		"main {\n"
		"	key1 = n1\n"
		"	key2 = n2\n"
		"	key3 = val3\n"
		"	none = \n"
		"	sub1 {\n"
		"		key3 = value\n"
		"	}\n"
		"	sub2 {\n"
		"		sub3 = val3\n"
		"	}\n"
		"	include " include2 "\n"
		"}");
	chunk_t inc2 = chunk_from_str(
		"key2 = v2\n"
		"sub1 {\n"
		"	key = val\n"
		"}");
	ck_assert(chunk_write(inc1, include1, 0022, TRUE));
	ck_assert(chunk_write(inc2, include2, 0022, TRUE));
}
END_SETUP

START_TEARDOWN(teardown_include_config)
{
	settings->destroy(settings);
	unlink(include2);
	unlink(include1);
	unlink(path);
}
END_TEARDOWN

static void verify_include()
{
	verify_string("n1", "main.key1");
	verify_string("v2", "main.key2");
	verify_string("val3", "main.key3");
	verify_string("val", "main.sub1.key");
	verify_string("v2", "main.sub1.key2");
	verify_string("val", "main.sub1.sub1.key");
	verify_string("value", "main.sub1.key3");
	verify_string("value", "main.sub1.include");
	verify_string("val3", "main.sub2.sub3");

	verify_null("main.none");
}

START_TEST(test_include)
{
	chunk_t contents = chunk_from_str(
		"main {\n"
		"	key1 = val1\n"
		"	key2 = val2\n"
		"	none = x\n"
		"	sub1 {\n"
		"		include this/does/not/exist.conf\n"
		"		include = value\n"
		"		key2 = value2\n"
		"		include " include2 "\n"
		"	}\n"
		"}\n"
		"include " include1);

	create_settings(contents);
	verify_include();
}
END_TEST

START_TEST(test_load_files)
{
	chunk_t contents = chunk_from_str(
		"main {\n"
		"	key1 = val1\n"
		"	key2 = val2\n"
		"	key3 = val3\n"
		"	none = x\n"
		"	sub1 {\n"
		"		include = value\n"
		"		key2 = v2\n"
		"		sub1 {\n"
		"			key = val\n"
		"		}\n"
		"	}\n"
		"}");
	char *val1, *val2, *val3;

	create_settings(contents);

	val1 = settings->get_str(settings, "main.key1", NULL);
	val2 = settings->get_str(settings, "main.sub1.key2", NULL);
	/* loading the same file twice should not change anything, with... */
	ck_assert(settings->load_files(settings, path, TRUE));
	ck_assert(val1 == settings->get_str(settings, "main.key1", NULL));
	ck_assert(val2 == settings->get_str(settings, "main.sub1.key2", NULL));
	/* ...or without merging */
	ck_assert(settings->load_files(settings, path, FALSE));
	ck_assert(val1 == settings->get_str(settings, "main.key1", NULL));
	ck_assert(val2 == settings->get_str(settings, "main.sub1.key2", NULL));

	val1 = settings->get_str(settings, "main.key2", NULL);
	val2 = settings->get_str(settings, "main.key3", NULL);
	val3 = settings->get_str(settings, "main.none", NULL);
	/* only pointers for modified settings should change, but still be valid */
	ck_assert(settings->load_files(settings, include1, FALSE));
	ck_assert(val1 != settings->get_str(settings, "main.key2", NULL));
	ck_assert_str_eq(val1, "val2");
	ck_assert(val2 == settings->get_str(settings, "main.key3", NULL));
	ck_assert(val3 != settings->get_str(settings, "main.none", NULL));
	ck_assert_str_eq(val3, "x");

	settings->destroy(settings);
	create_settings(contents);

	ck_assert(settings->load_files(settings, include1, TRUE));
	verify_include();

	ck_assert(settings->load_files(settings, include2, FALSE));
	verify_null("main.key1");
	verify_string("v2", "key2");
	verify_string("val", "sub1.key");
	verify_null("main.sub1.key3");
}
END_TEST

START_TEST(test_load_files_section)
{
	chunk_t contents = chunk_from_str(
		"main {\n"
		"	key1 = val1\n"
		"	key2 = val2\n"
		"	none = x\n"
		"	sub1 {\n"
		"		include = value\n"
		"		key2 = value2\n"
		"	}\n"
		"}");

	create_settings(contents);

	ck_assert(settings->load_files_section(settings, include1, TRUE, ""));
	ck_assert(settings->load_files_section(settings, include2, TRUE, "main.sub1"));
	verify_include();

	/* non existing files are a failure here */
	ck_assert(!settings->load_files_section(settings, include1".conf", TRUE, ""));
	verify_include();

#ifndef WIN32
	/* unreadable files are too (only fails when not running as root) */
	if (getuid() != 0)
	{
		ck_assert(chunk_write(contents, include1".no", 0444, TRUE));
		ck_assert(!settings->load_files_section(settings, include1".no", TRUE, ""));
		unlink(include1".no");
		verify_include();
	}
#endif

	ck_assert(settings->load_files_section(settings, include2, FALSE, "main"));
	verify_null("main.key1");
	verify_string("v2", "main.key2");
	verify_string("val", "main.sub1.key");
	verify_null("main.sub1.key3");
	verify_null("main.sub2.sub3");

	ck_assert(settings->load_files_section(settings, include2, TRUE, "main.sub2"));
	verify_string("v2", "main.sub2.key2");
	verify_string("val", "main.sub2.sub1.key");
}
END_TEST

START_TEST(test_order_kv)
{
	chunk_t base = chunk_from_str(
		"main {\n"
		"	key1 = val1\n"
		"	key2 = val2\n"
		"	key3 = val3\n"
		"}");
	chunk_t include = chunk_from_str(
		"main {\n"
		"	key0 = val0\n"
		"	key3 = val3\n"
		"	key1 = val1\n"
		"}");
	linked_list_t *keys, *values;

	create_settings(base);
	ck_assert(chunk_write(include, include1, 0022, TRUE));

	keys = linked_list_create_with_items("key1", "key2", "key3", NULL);
	values = linked_list_create_with_items("val1", "val2", "val3", NULL);
	verify_key_values(keys, values, "main");

	/* the original order is maintained if the settings are merged */
	ck_assert(settings->load_files(settings, include1, TRUE));
	keys = linked_list_create_with_items("key1", "key2", "key3", "key0", NULL);
	values = linked_list_create_with_items("val1", "val2", "val3", "val0", NULL);
	verify_key_values(keys, values, "main");

	/* but the new order is adopted if the settings are replaced */
	ck_assert(settings->load_files(settings, include1, FALSE));
	keys = linked_list_create_with_items("key0", "key3", "key1", NULL);
	values = linked_list_create_with_items("val0", "val3", "val1", NULL);
	verify_key_values(keys, values, "main");

	unlink(include1);
}
END_TEST

START_TEST(test_order_section)
{
	chunk_t base = chunk_from_str(
		"main {\n"
		"	sub1 {\n"
		"	}\n"
		"	sub2 {\n"
		"	}\n"
		"	sub3 {\n"
		"	}\n"
		"}");
	chunk_t include = chunk_from_str(
		"main {\n"
		"	sub0 {\n"
		"	}\n"
		"	sub3 {\n"
		"	}\n"
		"	sub1 {\n"
		"	}\n"
		"}");
	linked_list_t *sections;

	create_settings(base);
	ck_assert(chunk_write(include, include1, 0022, TRUE));

	sections = linked_list_create_with_items("sub1", "sub2", "sub3", NULL);
	verify_sections(sections, "main");

	/* the original order is maintained if the settings are merged */
	ck_assert(settings->load_files(settings, include1, TRUE));
	sections = linked_list_create_with_items("sub1", "sub2", "sub3", "sub0", NULL);
	verify_sections(sections, "main");

	/* but the new order is adopted if the settings are replaced */
	ck_assert(settings->load_files(settings, include1, FALSE));
	sections = linked_list_create_with_items("sub0", "sub3", "sub1", NULL);
	verify_sections(sections, "main");

	unlink(include1);
}
END_TEST

START_SETUP(setup_fallback_config)
{
	create_settings(chunk_from_str(
		"main {\n"
		"	key1 = val1\n"
		"	sub1 {\n"
		"		key1 = val1\n"
		"	}\n"
		"}\n"
		"sub {\n"
		"	key1 = subval1\n"
		"	key2 = subval2\n"
		"	subsub {\n"
		"		subkey1 = subsubval1\n"
		"	}\n"
		"}\n"
		"base {\n"
		"	key1 = baseval1\n"
		"	key2 = baseval2\n"
		"	sub1 {\n"
		"		key1 = subbase1\n"
		"		key2 = subbase2\n"
		"		key3 = subbase3\n"
		"		subsub {\n"
		"			subkey1 = subsubbaseval1\n"
		"			subkey2 = subsubbaseval2\n"
		"		}\n"
		"	}\n"
		"	sub2 {\n"
		"		key4 = subbase4\n"
		"	}\n"
		"}"));
}
END_SETUP

START_TEST(test_add_fallback)
{
	linked_list_t *keys, *values;

	settings->add_fallback(settings, "main.sub1", "sub");
	verify_string("val1", "main.sub1.key1");
	verify_string("subval2", "main.sub1.key2");
	verify_string("subsubval1", "main.sub1.subsub.subkey1");

	/* fallbacks are preserved even if the complete config is replaced */
	settings->load_files(settings, path, FALSE);
	verify_string("val1", "main.sub1.key1");
	verify_string("subval2", "main.sub1.key2");
	verify_string("subsubval1", "main.sub1.subsub.subkey1");

	keys = linked_list_create_with_items("sub1", NULL);
	verify_sections(keys, "main");
	keys = linked_list_create_with_items("subsub", NULL);
	verify_sections(keys, "main.sub1");

	keys = linked_list_create_with_items("key1", NULL);
	values = linked_list_create_with_items("val1", NULL);
	verify_key_values(keys, values, "main");

	keys = linked_list_create_with_items("key1", "key2", NULL);
	values = linked_list_create_with_items("val1", "subval2", NULL);
	verify_key_values(keys, values, "main.sub1");

	keys = linked_list_create_with_items("subkey1", NULL);
	values = linked_list_create_with_items("subsubval1", NULL);
	verify_key_values(keys, values, "main.sub1.subsub");

	settings->add_fallback(settings, "main", "base");
	verify_string("val1", "main.key1");
	verify_string("baseval2", "main.key2");
	verify_string("val1", "main.sub1.key1");
	verify_string("subval2", "main.sub1.key2");
	verify_string("subsubval1", "main.sub1.subsub.subkey1");
	verify_string("subsubbaseval2", "main.sub1.subsub.subkey2");
	verify_string("subbase3", "main.sub1.key3");
	verify_string("subbase4", "main.sub2.key4");


	keys = linked_list_create_with_items("sub1", "sub2", NULL);
	verify_sections(keys, "main");
	keys = linked_list_create_with_items("subsub", NULL);
	verify_sections(keys, "main.sub1");

	keys = linked_list_create_with_items("key1", "key2", NULL);
	values = linked_list_create_with_items("val1", "baseval2", NULL);
	verify_key_values(keys, values, "main");

	keys = linked_list_create_with_items("key1", "key2", "key3", NULL);
	values = linked_list_create_with_items("val1", "subval2", "subbase3", NULL);
	verify_key_values(keys, values, "main.sub1");

	keys = linked_list_create_with_items("subkey1", "subkey2", NULL);
	values = linked_list_create_with_items("subsubval1", "subsubbaseval2", NULL);
	verify_key_values(keys, values, "main.sub1.subsub");

	settings->set_str(settings, "main.sub1.key2", "val2");
	verify_string("val2", "main.sub1.key2");
	settings->set_str(settings, "main.sub1.subsub.subkey2", "val2");
	verify_string("val2", "main.sub1.subsub.subkey2");
	verify_string("subsubval1", "main.sub1.subsub.subkey1");
}
END_TEST

START_TEST(test_add_fallback_printf)
{
	settings->add_fallback(settings, "%s.sub1", "sub", "main");
	verify_string("val1", "main.sub1.key1");
	verify_string("subval2", "main.sub1.key2");
	verify_string("subsubval1", "main.sub1.subsub.subkey1");

	settings->add_fallback(settings, "%s.%s2", "%s.%s1", "main", "sub");
	verify_string("val1", "main.sub2.key1");
	verify_string("subval2", "main.sub2.key2");
	verify_string("subsubval1", "main.sub2.subsub.subkey1");
}
END_TEST

START_SETUP(setup_string_config)
{
	create_settings(chunk_from_str(
		"string = \"  with    accurate\twhitespace\"\n"
		"special = \"all { special } characters # can be used.\"\n"
		"unterminated = \"is fine\n"
		"but = produces a warning\n"
		"newlines = \"can either be encoded\\nor\\\n"
		"escaped\"\n"
		"quotes = \"\\\"and\\\" slashes \\\\ can \\\\ be\" # escaped too\n"
		"multiple = \"strings\" are \"combined\"\n"
		));
}
END_SETUP

START_TEST(test_strings)
{
	verify_string("  with    accurate\twhitespace", "string");
	verify_string("all { special } characters # can be used.", "special");
	verify_string("is fine", "unterminated");
	verify_string("produces a warning", "but");
	verify_string("can either be encoded\nor\nescaped", "newlines");
	verify_string("\"and\" slashes \\ can \\ be", "quotes");
	verify_string("strings are combined", "multiple");
}
END_TEST

START_TEST(test_valid)
{
	chunk_t contents;

	contents = chunk_from_str(
		"single = value");
	ck_assert(chunk_write(contents, path, 0022, TRUE));
	ck_assert(settings->load_files(settings, path, FALSE));
	verify_string("value", "single");

	contents = chunk_from_str(
		"singleline { single = value }");
	ck_assert(chunk_write(contents, path, 0022, TRUE));
	ck_assert(settings->load_files(settings, path, FALSE));
	verify_string("value", "singleline.single");

	contents = chunk_from_str(
		"singleline { sub { sub1 = val1 } single = value }");
	ck_assert(chunk_write(contents, path, 0022, TRUE));
	ck_assert(settings->load_files(settings, path, FALSE));
	verify_string("val1", "singleline.sub.sub1");

	contents = chunk_from_str(
		"newline\n { single = value }");
	ck_assert(chunk_write(contents, path, 0022, TRUE));
	ck_assert(settings->load_files(settings, path, FALSE));
	verify_string("value", "newline.single");

	contents = chunk_from_str(
		"section {\n"
		"	include # without pattern produces a warning, but is fine\n"
		"}\n");
	ck_assert(chunk_write(contents, path, 0022, TRUE));
	ck_assert(settings->load_files(settings, path, FALSE));
}
END_TEST

START_TEST(test_invalid)
{
	chunk_t contents;

	contents = chunk_from_str(
		"{\n"
		"	no = section name\n"
		"}\n");
	ck_assert(chunk_write(contents, path, 0022, TRUE));
	ck_assert(!settings->load_files(settings, path, FALSE));

	contents = chunk_from_str(
		"no {\n"
		"	= key name\n"
		"}\n");
	ck_assert(chunk_write(contents, path, 0022, TRUE));
	ck_assert(!settings->load_files(settings, path, FALSE));

	contents = chunk_from_str(
		"unterminated {\n"
		"	not = valid\n");
	ck_assert(chunk_write(contents, path, 0022, TRUE));
	ck_assert(!settings->load_files(settings, path, FALSE));

	contents = chunk_from_str(
		"spaces in name {}");
	ck_assert(chunk_write(contents, path, 0022, TRUE));
	ck_assert(!settings->load_files(settings, path, FALSE));

	contents = chunk_from_str(
		"only = a single setting = per line");
	ck_assert(chunk_write(contents, path, 0022, TRUE));
	ck_assert(!settings->load_files(settings, path, FALSE));
}
END_TEST

Suite *settings_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("settings");

	tc = tcase_create("get/set_str (basic behavior)");
	tcase_add_checked_fixture(tc, setup_base_config, teardown_config);
	tcase_add_test(tc, test_get_str);
	tcase_add_test(tc, test_get_str_printf);
	tcase_add_test(tc, test_set_str);
	tcase_add_test(tc, test_set_str_printf);
	tcase_add_test(tc, test_set_default_str);
	suite_add_tcase(s, tc);

	tc = tcase_create("get/set_bool");
	tcase_add_checked_fixture(tc, setup_bool_config, teardown_config);
	tcase_add_test(tc, test_get_bool);
	tcase_add_test(tc, test_set_bool);
	suite_add_tcase(s, tc);

	tc = tcase_create("get/set_int");
	tcase_add_checked_fixture(tc, setup_int_config, teardown_config);
	tcase_add_test(tc, test_get_int);
	tcase_add_test(tc, test_set_int);
	suite_add_tcase(s, tc);

	tc = tcase_create("get/set_double");
	tcase_add_checked_fixture(tc, setup_double_config, teardown_config);
	tcase_add_test(tc, test_get_double);
	tcase_add_test(tc, test_set_double);
	suite_add_tcase(s, tc);

	tc = tcase_create("get/set_time");
	tcase_add_checked_fixture(tc, setup_time_config, teardown_config);
	tcase_add_test(tc, test_get_time);
	tcase_add_test(tc, test_set_time);
	suite_add_tcase(s, tc);

	tc = tcase_create("section enumerator");
	tcase_add_checked_fixture(tc, setup_base_config, teardown_config);
	tcase_add_test(tc, test_section_enumerator);
	suite_add_tcase(s, tc);

	tc = tcase_create("key/value enumerator");
	tcase_add_checked_fixture(tc, setup_base_config, teardown_config);
	tcase_add_test(tc, test_key_value_enumerator);
	suite_add_tcase(s, tc);

	tc = tcase_create("include/load_files[_section]");
	tcase_add_checked_fixture(tc, setup_include_config, teardown_include_config);
	tcase_add_test(tc, test_include);
	tcase_add_test(tc, test_load_files);
	tcase_add_test(tc, test_load_files_section);
	tcase_add_test(tc, test_order_kv);
	tcase_add_test(tc, test_order_section);
	suite_add_tcase(s, tc);

	tc = tcase_create("fallback");
	tcase_add_checked_fixture(tc, setup_fallback_config, teardown_config);
	tcase_add_test(tc, test_add_fallback);
	tcase_add_test(tc, test_add_fallback_printf);
	suite_add_tcase(s, tc);

	tc = tcase_create("strings");
	tcase_add_checked_fixture(tc, setup_string_config, teardown_config);
	tcase_add_test(tc, test_strings);
	suite_add_tcase(s, tc);

	tc = tcase_create("valid/invalid data");
	tcase_add_checked_fixture(tc, setup_base_config, teardown_config);
	tcase_add_test(tc, test_valid);
	tcase_add_test(tc, test_invalid);
	suite_add_tcase(s, tc);

	return s;
}
