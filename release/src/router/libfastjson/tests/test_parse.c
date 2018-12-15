#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "../json.h"
#include "../json_tokener.h"
#include "../debug.h"

static void test_basic_parse(void);
static void test_verbose_parse(void);
static void test_incremental_parse(void);

#define CHK(x) if (!(x)) { \
	printf("%s:%d: unexpected result with '%s'\n", \
		__FILE__, __LINE__, #x); \
	exit(1); \
}

int main(int __attribute__((unused)) argc, char __attribute__((unused)) **argv)
{
	MC_SET_DEBUG(1);

	test_basic_parse();
	printf("==================================\n");
	test_verbose_parse();
	printf("==================================\n");
	test_incremental_parse();
	printf("==================================\n");
	return 0;
}

static void test_basic_parse(void)
{
	fjson_object *new_obj;

	new_obj = fjson_tokener_parse("\"\003\"");
	printf("new_obj.to_string()=%s\n", fjson_object_to_json_string(new_obj));
	fjson_object_put(new_obj);

	new_obj = fjson_tokener_parse("/* hello */\"foo\"");
	printf("new_obj.to_string()=%s\n", fjson_object_to_json_string(new_obj));
	fjson_object_put(new_obj);

	new_obj = fjson_tokener_parse("// hello\n\"foo\"");
	printf("new_obj.to_string()=%s\n", fjson_object_to_json_string(new_obj));
	fjson_object_put(new_obj);

	new_obj = fjson_tokener_parse("\"\\u0041\\u0042\\u0043\"");
	printf("new_obj.to_string()=%s\n", fjson_object_to_json_string(new_obj));
	fjson_object_put(new_obj);

	new_obj = fjson_tokener_parse("null");
	printf("new_obj.to_string()=%s\n", fjson_object_to_json_string(new_obj));
	fjson_object_put(new_obj);

	new_obj = fjson_tokener_parse("NaN");
	printf("new_obj.to_string()=%s\n", fjson_object_to_json_string(new_obj));
	fjson_object_put(new_obj);

	new_obj = fjson_tokener_parse("-NaN"); /* non-sensical, returns null */
	printf("new_obj.to_string()=%s\n", fjson_object_to_json_string(new_obj));
	fjson_object_put(new_obj);

	new_obj = fjson_tokener_parse("Inf"); /* must use full string, returns null */
	printf("new_obj.to_string()=%s\n", fjson_object_to_json_string(new_obj));
	fjson_object_put(new_obj);

	new_obj = fjson_tokener_parse("inf"); /* must use full string, returns null */
	printf("new_obj.to_string()=%s\n", fjson_object_to_json_string(new_obj));
	fjson_object_put(new_obj);

	new_obj = fjson_tokener_parse("Infinity");
	printf("new_obj.to_string()=%s\n", fjson_object_to_json_string(new_obj));
	fjson_object_put(new_obj);

	new_obj = fjson_tokener_parse("infinity");
	printf("new_obj.to_string()=%s\n", fjson_object_to_json_string(new_obj));
	fjson_object_put(new_obj);

	new_obj = fjson_tokener_parse("-Infinity");
	printf("new_obj.to_string()=%s\n", fjson_object_to_json_string(new_obj));
	fjson_object_put(new_obj);

	new_obj = fjson_tokener_parse("-infinity");
	printf("new_obj.to_string()=%s\n", fjson_object_to_json_string(new_obj));
	fjson_object_put(new_obj);

	new_obj = fjson_tokener_parse("True");
	printf("new_obj.to_string()=%s\n", fjson_object_to_json_string(new_obj));
	fjson_object_put(new_obj);

	new_obj = fjson_tokener_parse("12");
	printf("new_obj.to_string()=%s\n", fjson_object_to_json_string(new_obj));
	fjson_object_put(new_obj);

	new_obj = fjson_tokener_parse("12.3");
	printf("new_obj.to_string()=%s\n", fjson_object_to_json_string(new_obj));
	fjson_object_put(new_obj);

	new_obj = fjson_tokener_parse("12.3.4"); /* non-sensical, returns null */
	printf("new_obj.to_string()=%s\n", fjson_object_to_json_string(new_obj));
	fjson_object_put(new_obj);

	/* was returning (int)2015 before patch, should return null */
	new_obj = fjson_tokener_parse("2015-01-15");
	printf("new_obj.to_string()=%s\n", fjson_object_to_json_string(new_obj));
	fjson_object_put(new_obj);

	new_obj = fjson_tokener_parse("{\"FoO\"  :   -12.3E512}");
	printf("new_obj.to_string()=%s\n", fjson_object_to_json_string(new_obj));
	fjson_object_put(new_obj);

	new_obj = fjson_tokener_parse("{\"FoO\"  :   -12.3E51.2}"); /* non-sensical, returns null */
	printf("new_obj.to_string()=%s\n", fjson_object_to_json_string(new_obj));
	fjson_object_put(new_obj);

	new_obj = fjson_tokener_parse("[\"\\n\"]");
	printf("new_obj.to_string()=%s\n", fjson_object_to_json_string(new_obj));
	fjson_object_put(new_obj);

	new_obj = fjson_tokener_parse("[\"\\nabc\\n\"]");
	printf("new_obj.to_string()=%s\n", fjson_object_to_json_string(new_obj));
	fjson_object_put(new_obj);

	new_obj = fjson_tokener_parse("[null]");
	printf("new_obj.to_string()=%s\n", fjson_object_to_json_string(new_obj));
	fjson_object_put(new_obj);

	new_obj = fjson_tokener_parse("[]");
	printf("new_obj.to_string()=%s\n", fjson_object_to_json_string(new_obj));
	fjson_object_put(new_obj);

	new_obj = fjson_tokener_parse("[false]");
	printf("new_obj.to_string()=%s\n", fjson_object_to_json_string(new_obj));
	fjson_object_put(new_obj);

	new_obj = fjson_tokener_parse("[\"abc\",null,\"def\",12]");
	printf("new_obj.to_string()=%s\n", fjson_object_to_json_string(new_obj));
	fjson_object_put(new_obj);

	new_obj = fjson_tokener_parse("{}");
	printf("new_obj.to_string()=%s\n", fjson_object_to_json_string(new_obj));
	fjson_object_put(new_obj);

	new_obj = fjson_tokener_parse("{ \"foo\": \"bar\" }");
	printf("new_obj.to_string()=%s\n", fjson_object_to_json_string(new_obj));
	fjson_object_put(new_obj);

	new_obj = fjson_tokener_parse("{ \"foo\": \"bar\", \"baz\": null, \"bool0\": true }");
	printf("new_obj.to_string()=%s\n", fjson_object_to_json_string(new_obj));
	fjson_object_put(new_obj);

	new_obj = fjson_tokener_parse("{ \"foo\": [null, \"foo\"] }");
	printf("new_obj.to_string()=%s\n", fjson_object_to_json_string(new_obj));
	fjson_object_put(new_obj);

	new_obj = fjson_tokener_parse("{ \"abc\": 12, \"foo\": \"bar\", \"bool0\": false, \"bool1\": true, \"arr\": [ 1, 2, 3, null, 5 ] }");
	printf("new_obj.to_string()=%s\n", fjson_object_to_json_string(new_obj));
	fjson_object_put(new_obj);
}

static void test_verbose_parse(void)
{
	fjson_object *new_obj;
	enum fjson_tokener_error error = fjson_tokener_success;

	new_obj = fjson_tokener_parse_verbose("{ foo }", &error);
	CHK (error == fjson_tokener_error_parse_object_key_name);
	CHK (new_obj == NULL);

	new_obj = fjson_tokener_parse("{ foo }");
	CHK (new_obj == NULL);

	new_obj = fjson_tokener_parse("foo");
	CHK (new_obj == NULL);
	new_obj = fjson_tokener_parse_verbose("foo", &error);
	CHK (new_obj == NULL);

	/* b/c the string starts with 'f' parsing return a boolean error */
	CHK (error == fjson_tokener_error_parse_boolean);

	printf("fjson_tokener_parse_versbose() OK\n");
}

struct incremental_step {
	const char *string_to_parse;
	int length;
	int char_offset;
	enum fjson_tokener_error expected_error;
	int reset_tokener;
} incremental_steps[] = {

	/* Check that full json messages can be parsed, both w/ and w/o a reset */
	{ "{ \"foo\": 123 }", -1, -1, fjson_tokener_success,  0 },
	{ "{ \"foo\": 456 }", -1, -1, fjson_tokener_success,  1 },
	{ "{ \"foo\": 789 }", -1, -1, fjson_tokener_success,  1 },

	/*  Check a basic incremental parse */
	{ "{ \"foo",          -1, -1, fjson_tokener_continue, 0 },
	{ "\": {\"bar",       -1, -1, fjson_tokener_continue, 0 },
	{ "\":13}}",          -1, -1, fjson_tokener_success,  1 },

	/* Check that fjson_tokener_reset actually resets */
	{ "{ \"foo",          -1, -1, fjson_tokener_continue, 1 },
	{ ": \"bar\"}",       -1, 0, fjson_tokener_error_parse_unexpected, 1 },

	/* Check incremental parsing with trailing characters */
	{ "{ \"foo",          -1, -1, fjson_tokener_continue, 0 },
	{ "\": {\"bar",       -1, -1, fjson_tokener_continue, 0 },
	{ "\":13}}XXXX",      10, 6, fjson_tokener_success,  0 },
	{ "XXXX",              4, 0, fjson_tokener_error_parse_unexpected, 1 },

	/* Check that trailing characters can change w/o a reset */
	{ "{\"x\": 123 }\"X\"", -1, 11, fjson_tokener_success, 0 },
	{ "\"Y\"",            -1, -1, fjson_tokener_success, 1 },

	/* To stop parsing a number we need to reach a non-digit, e.g. a \0 */
	{ "1",                 1, 1, fjson_tokener_continue, 0 },
	{ "2",                 2, 1, fjson_tokener_success, 0 },

	/* Some bad formatting. Check we get the correct error status */
	{ "2015-01-15",       10, 4, fjson_tokener_error_parse_number, 1 },

	/* Strings have a well defined end point, so we can stop at the quote */
	{ "\"blue\"",         -1, -1, fjson_tokener_success, 0 },

	/* Check each of the escape sequences defined by the spec */
	{ "\"\\\"\"",         -1, -1, fjson_tokener_success, 0 },
	{ "\"\\\\\"",         -1, -1, fjson_tokener_success, 0 },
	{ "\"\\b\"",         -1, -1, fjson_tokener_success, 0 },
	{ "\"\\f\"",         -1, -1, fjson_tokener_success, 0 },
	{ "\"\\n\"",         -1, -1, fjson_tokener_success, 0 },
	{ "\"\\r\"",         -1, -1, fjson_tokener_success, 0 },
	{ "\"\\t\"",         -1, -1, fjson_tokener_success, 0 },

	{ "[1,2,3]",          -1, -1, fjson_tokener_success, 0 },

	/* This behaviour doesn't entirely follow the json spec, but until we have
	   a way to specify how strict to be we follow Postel's Law and be liberal
	   in what we accept (up to a point). */
	{ "[1,2,3,]",         -1, -1, fjson_tokener_success, 0 },
	{ "[1,2,,3,]",        -1, 5, fjson_tokener_error_parse_unexpected, 0 },

	{ "[1,2,3,]",         -1, 7, fjson_tokener_error_parse_unexpected, 3 },
	{ "{\"a\":1,}",         -1, 7, fjson_tokener_error_parse_unexpected, 3 },

	{ NULL, -1, -1, fjson_tokener_success, 0 },
};

static void test_incremental_parse(void)
{
	fjson_object *new_obj;
	enum fjson_tokener_error jerr;
	fjson_tokener *tok;
	const char *string_to_parse;
	int ii;
	int num_ok, num_error;

	num_ok = 0;
	num_error = 0;

	printf("Starting incremental tests.\n");
	printf("Note: quotes and backslashes seen in the output here are literal values passed\n");
	printf("     to the parse functions.  e.g. this is 4 characters: \"\\f\"\n");

	string_to_parse = "{ \"foo"; /* } */
	printf("fjson_tokener_parse(%s) ... ", string_to_parse);
	new_obj = fjson_tokener_parse(string_to_parse);
	if (new_obj == NULL) printf("got error as expected\n");

	/* test incremental parsing in various forms */
	tok = fjson_tokener_new();
	for (ii = 0; incremental_steps[ii].string_to_parse != NULL; ii++)
	{
		int this_step_ok = 0;
		struct incremental_step *step = &incremental_steps[ii];
		int length = step->length;
		int expected_char_offset = step->char_offset;

		if (step->reset_tokener & 2)
			fjson_tokener_set_flags(tok, FJSON_TOKENER_STRICT);
		else
			fjson_tokener_set_flags(tok, 0);

		if (length == -1)
			length = strlen(step->string_to_parse);
		if (expected_char_offset == -1)
			expected_char_offset = length;

		printf("fjson_tokener_parse_ex(tok, %-12s, %3d) ... ",
			step->string_to_parse, length);
		new_obj = fjson_tokener_parse_ex(tok, step->string_to_parse, length);

		jerr = fjson_tokener_get_error(tok);
		if (step->expected_error != fjson_tokener_success)
		{
			if (new_obj != NULL)
				printf("ERROR: invalid object returned: %s\n",
					fjson_object_to_json_string(new_obj));
			else if (jerr != step->expected_error)
				printf("ERROR: got wrong error: %s\n",
					fjson_tokener_error_desc(jerr));
			else if (tok->char_offset != expected_char_offset)
				printf("ERROR: wrong char_offset %d != expected %d\n",
					tok->char_offset,
					expected_char_offset);
			else
			{
				printf("OK: got correct error: %s\n", fjson_tokener_error_desc(jerr));
				this_step_ok = 1;
			}
		}
		else
		{
			if (new_obj == NULL)
				printf("ERROR: expected valid object, instead: %s\n",
					fjson_tokener_error_desc(jerr));
			else if (tok->char_offset != expected_char_offset)
				printf("ERROR: wrong char_offset %d != expected %d\n",
					tok->char_offset,
					expected_char_offset);
			else
			{
				printf("OK: got object of type [%s]: %s\n",
					fjson_type_to_name(fjson_object_get_type(new_obj)),
					fjson_object_to_json_string(new_obj));
				this_step_ok = 1;
			}
		}

		if (new_obj)
			fjson_object_put(new_obj);

		if (step->reset_tokener & 1)
			fjson_tokener_reset(tok);

		if (this_step_ok)
			num_ok++;
		else
			num_error++;
	}

	fjson_tokener_free(tok);

	printf("End Incremental Tests OK=%d ERROR=%d\n", num_ok, num_error);

	return;
}
