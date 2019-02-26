/*
 * Copyright (C) 2014-2017 Andreas Steffen
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

#include <asn1/asn1_parser.h>
#include <utils/chunk.h>

/*******************************************************************************
 * utilities
 */

typedef struct {
	bool success;
	int count;
	chunk_t blob;
} asn1_test_t;

static void run_parser_test(const asn1Object_t *objects, int id,
							asn1_test_t *test)
{
	asn1_parser_t *parser;
	chunk_t object;
	int objectID, count = 0;
	bool success;

	parser = asn1_parser_create(objects, test->blob);
	while (parser->iterate(parser, &objectID, &object))
	{
		if (objectID == id)
		{
			count++;
		}
	}
	success = parser->success(parser);
	parser->destroy(parser);

	ck_assert(success == test->success && count == test->count);
}

/*******************************************************************************
 * length
 */

static const asn1Object_t octetStringObjects[] = {
	{ 0, "octetString",	ASN1_OCTET_STRING,	ASN1_BODY }, /* 0 */
	{ 0, "exit",		ASN1_EOC,			ASN1_EXIT }
};

asn1_test_t length_tests[] = {
	{ FALSE, 0, { NULL, 0 } },
	{ FALSE, 0, chunk_from_chars(0x04) },
	{ TRUE,  1, chunk_from_chars(0x04, 0x00) },
	{ TRUE,  1, chunk_from_chars(0x04, 0x01, 0xaa) },
	{ FALSE, 0, chunk_from_chars(0x04, 0x7f) },
	{ FALSE, 0, chunk_from_chars(0x04, 0x80) },
	{ FALSE, 0, chunk_from_chars(0x04, 0x81) },
	{ TRUE,  1, chunk_from_chars(0x04, 0x81, 0x00) },
	{ FALSE, 0, chunk_from_chars(0x04, 0x81, 0x01) },
	{ TRUE,  1, chunk_from_chars(0x04, 0x81, 0x01, 0xaa) },
	{ FALSE, 0, chunk_from_chars(0x04, 0x82, 0x00, 0x01) },
	{ TRUE,  1, chunk_from_chars(0x04, 0x82, 0x00, 0x01, 0xaa) },
	{ FALSE, 0, chunk_from_chars(0x04, 0x83, 0x00, 0x00, 0x01) },
	{ TRUE,  1, chunk_from_chars(0x04, 0x83, 0x00, 0x00, 0x01, 0xaa) },
	{ FALSE, 0, chunk_from_chars(0x04, 0x84, 0x00, 0x00, 0x00, 0x01) },
	{ TRUE,  1, chunk_from_chars(0x04, 0x84, 0x00, 0x00, 0x00, 0x01, 0xaa) },
};

START_TEST(test_asn1_parser_length)
{
	run_parser_test(octetStringObjects, 0, &length_tests[_i]);
}
END_TEST

/*******************************************************************************
 * loop
 */

static const asn1Object_t loopObjects[] = {
	{ 0, "loopObjects",		ASN1_SEQUENCE,		ASN1_LOOP }, /* 0 */
	{ 1,   "octetString",	ASN1_OCTET_STRING,	ASN1_BODY }, /* 1 */
	{ 0, "end loop",		ASN1_EOC,			ASN1_END  }, /* 2 */
	{ 0, "exit",			ASN1_EOC,			ASN1_EXIT }
};

asn1_test_t loop_tests[] = {
	{ TRUE,  0, chunk_from_chars(0x30, 0x00) },
	{ FALSE, 0, chunk_from_chars(0x30, 0x02, 0x04, 0x01) },
	{ TRUE,  1, chunk_from_chars(0x30, 0x03, 0x04, 0x01, 0xaa) },
	{ TRUE,  2, chunk_from_chars(0x30, 0x05, 0x04, 0x01, 0xaa, 0x04, 0x00) },
	{ FALSE, 1, chunk_from_chars(0x30, 0x05, 0x04, 0x01, 0xaa, 0x05, 0x00) },
	{ TRUE,  3, chunk_from_chars(0x30, 0x09, 0x04, 0x01, 0xaa, 0x04, 0x00,
											 0x04, 0x02, 0xbb, 0xcc) },
};

START_TEST(test_asn1_parser_loop)
{
	run_parser_test(loopObjects, 1, &loop_tests[_i]);
}
END_TEST

/*******************************************************************************
 * default
 */

typedef struct {
	int i1, i2, i3;
	chunk_t blob;
} default_opt_test_t;

static const asn1Object_t defaultObjects[] = {
	{ 0, "defaultObjects",	ASN1_SEQUENCE,		ASN1_OBJ			}, /* 0 */
	{ 1,   "explicit int1",	ASN1_CONTEXT_C_1,	ASN1_DEF			}, /* 1 */
	{ 2,     "int1",		ASN1_INTEGER,		ASN1_BODY			}, /* 2 */
	{ 1,   "int2",			ASN1_INTEGER,		ASN1_DEF|ASN1_BODY	}, /* 3 */
	{ 1,   "implicit int3", ASN1_CONTEXT_S_3,	ASN1_DEF|ASN1_BODY	}, /* 4 */		
	{ 0, "exit",			ASN1_EOC,			ASN1_EXIT			}
};

default_opt_test_t default_tests[] = {
	{ -1, -2, -3, chunk_from_chars(0x30, 0x00) },
	{  1, -2, -3, chunk_from_chars(0x30, 0x05, 0xa1, 0x03, 0x02, 0x01, 0x01) },
	{ -1,  2, -3, chunk_from_chars(0x30, 0x03, 0x02, 0x01, 0x02) },
	{ -1, -2,  3, chunk_from_chars(0x30, 0x03, 0x83, 0x01, 0x03) },
	{  1,  2, -3, chunk_from_chars(0x30, 0x08, 0xa1, 0x03, 0x02, 0x01, 0x01,
											   0x02, 0x01, 0x02) },
	{  1, -2,  3, chunk_from_chars(0x30, 0x08, 0xa1, 0x03, 0x02, 0x01, 0x01,
											   0x83, 0x01, 0x03) },
	{ -1,  2,  3, chunk_from_chars(0x30, 0x06, 0x02, 0x01, 0x02,
											   0x83, 0x01, 0x03) },
	{  1,  2,  3, chunk_from_chars(0x30, 0x0b, 0xa1, 0x03, 0x02, 0x01, 0x01,
											   0x02, 0x01, 0x02,
											   0x83, 0x01, 0x03) },
	{  0,  0,  0, chunk_from_chars(0x30, 0x0b, 0xa1, 0x03, 0x04, 0x01, 0xaa,
											   0x02, 0x01, 0x02,
											   0x83, 0x01, 0x03) },
	{  1,  0,  0, chunk_from_chars(0x30, 0x0b, 0xa1, 0x03, 0x02, 0x01, 0x01,
											   0x02, 0x05, 0x02,
											   0x83, 0x01, 0x03) },
	{  1,  2,  0, chunk_from_chars(0x30, 0x0b, 0xa1, 0x03, 0x02, 0x01, 0x01,
											   0x02, 0x01, 0x02,
											   0x83, 0x02, 0x03) },
};

START_TEST(test_asn1_parser_default)
{
	asn1_parser_t *parser;
	chunk_t object;
	int objectID, i1 = 0, i2 = 0, i3 = 0;
	bool success;

	parser = asn1_parser_create(defaultObjects, default_tests[_i].blob);
	while (parser->iterate(parser, &objectID, &object))
	{
		switch (objectID)
		{
			case 2:
				i1 = object.len ? *object.ptr : -1;
				break;
			case 3:
				i2 = object.len ? *object.ptr : -2;
				break;
			case 4:
				i3 = object.len ? *object.ptr : -3;
				break;
			default:
				break;
		}
	}
	success = parser->success(parser);
	parser->destroy(parser);

	ck_assert(success == (default_tests[_i].i1 &&
						  default_tests[_i].i2 &&
						  default_tests[_i].i3));

	ck_assert(i1 == default_tests[_i].i1 &&
			  i2 == default_tests[_i].i2 &&
			  i3 == default_tests[_i].i3);
}
END_TEST

/*******************************************************************************
 * option
 */

static const asn1Object_t optionObjects[] = {
	{ 0, "optionalObjects",	ASN1_SEQUENCE,		ASN1_OBJ			}, /* 0 */
	{ 1,   "sequence int1",	ASN1_SEQUENCE,		ASN1_OPT			}, /* 1 */
	{ 2,     "int1",		ASN1_INTEGER,		ASN1_OPT|ASN1_BODY  }, /* 2 */
	{ 2,     "end opt",		ASN1_EOC,			ASN1_END			}, /* 3 */
	{ 1,   "end opt",		ASN1_EOC,			ASN1_END			}, /* 4 */
	{ 1,   "int2",			ASN1_INTEGER,		ASN1_OPT|ASN1_BODY	}, /* 5 */
	{ 1,   "end opt",		ASN1_EOC,			ASN1_END			}, /* 6 */
	{ 1,   "implicit int3", ASN1_CONTEXT_S_3,	ASN1_OPT|ASN1_BODY	}, /* 7 */		
	{ 1,   "end opt",		ASN1_EOC,			ASN1_END			}, /* 8 */
	{ 0, "exit",			ASN1_EOC,			ASN1_EXIT			}
};

default_opt_test_t option_tests[] = {
	{ 0, 0, 0, chunk_from_chars(0x30, 0x00) },
	{ 1, 0, 0, chunk_from_chars(0x30, 0x05, 0x30, 0x03, 0x02, 0x01, 0x01) },
	{ 0, 2, 0, chunk_from_chars(0x30, 0x03, 0x02, 0x01, 0x02) },
	{ 0, 0, 3, chunk_from_chars(0x30, 0x03, 0x83, 0x01, 0x03) },
	{ 1, 2, 0, chunk_from_chars(0x30, 0x08, 0x30, 0x03, 0x02, 0x01, 0x01,
											0x02, 0x01, 0x02) },
	{ 1, 0, 3, chunk_from_chars(0x30, 0x08, 0x30, 0x03, 0x02, 0x01, 0x01,
											0x83, 0x01, 0x03) },
	{ 0, 2, 3, chunk_from_chars(0x30, 0x06, 0x02, 0x01, 0x02,
											0x83, 0x01, 0x03) },
	{ 1, 2, 3, chunk_from_chars(0x30, 0x0b, 0x30, 0x03, 0x02, 0x01, 0x01,
											0x02, 0x01, 0x02,
											0x83, 0x01, 0x03) },
	{ 0, 2, 3, chunk_from_chars(0x30, 0x08, 0x30, 0x00,
											0x02, 0x01, 0x02,
											0x83, 0x01, 0x03) },
};

START_TEST(test_asn1_parser_option)
{
	asn1_parser_t *parser;
	chunk_t object;
	int objectID, i1 = 0, i2 = 0, i3 = 0;
	bool success;

	parser = asn1_parser_create(optionObjects, option_tests[_i].blob);
	while (parser->iterate(parser, &objectID, &object))
	{
		switch (objectID)
		{
			case 2:
				i1 = *object.ptr;
				break;
			case 5:
				i2 = *object.ptr;
				break;
			case 7:
				i3 = *object.ptr;
				break;
			default:

				break;
		}
	}
	success = parser->success(parser);
	parser->destroy(parser);

	ck_assert(success);

	ck_assert(i1 == option_tests[_i].i1 &&
			  i2 == option_tests[_i].i2 &&
			  i3 == option_tests[_i].i3);
}
END_TEST

/*******************************************************************************
 * choice
 */

typedef struct {
	int i1, i2, i3, i4;
	chunk_t blob;
} choice_test_t;

static const asn1Object_t choiceObjects[] = {
	{ 0, "choiceObject",      ASN1_EOC,          ASN1_CHOICE          }, /*  0 */
	{ 1,   "choiceA",         ASN1_CONTEXT_C_0,  ASN1_OPT|ASN1_CHOICE }, /*  1 */
	{ 2,     "choice1",       ASN1_OCTET_STRING, ASN1_OPT|ASN1_BODY   }, /*  2 */
	{ 2,     "end choice1",   ASN1_EOC,          ASN1_END|ASN1_CH     }, /*  3 */
	{ 2,     "choice2",       ASN1_INTEGER,      ASN1_OPT|ASN1_BODY   }, /*  4 */
	{ 2,     "end choice2",   ASN1_EOC,          ASN1_END|ASN1_CH     }, /*  5 */
	{ 1,   "end choiceA",     ASN1_EOC,          ASN1_END|ASN1_CHOICE|
	                                             ASN1_CH              }, /*  6 */
	{ 1,   "choiceB",         ASN1_SEQUENCE,     ASN1_OPT|ASN1_LOOP   }, /*  7 */
	{ 2,     "choiceObject",  ASN1_EOC,          ASN1_CHOICE          }, /*  8 */
	{ 3,       "choice3",     ASN1_INTEGER,      ASN1_OPT|ASN1_BODY   }, /*  9 */
	{ 3,       "end choice3", ASN1_EOC,          ASN1_END|ASN1_CH     }, /* 10 */
	{ 3,       "choice4",     ASN1_OCTET_STRING, ASN1_OPT|ASN1_BODY   }, /* 11 */
	{ 3,       "end choice4", ASN1_EOC,          ASN1_END|ASN1_CH     }, /* 12 */
	{ 2,     "end choices",   ASN1_EOC,          ASN1_END|ASN1_CHOICE }, /* 13 */
	{ 1,   "end loop/choice", ASN1_EOC,          ASN1_END|ASN1_CH     }, /* 14 */
	{ 0, "end choices",       ASN1_EOC,          ASN1_END|ASN1_CHOICE }, /* 15 */
	{ 0, "exit",              ASN1_EOC,          ASN1_EXIT            }
};

choice_test_t choice_tests[] = {
	{ 0, 0, 0, 0, { NULL, 0 } },
	{ 0, 0, 0, 0, chunk_from_chars(0xA0, 0x00) },
	{ 1, 0, 0, 0, chunk_from_chars(0xA0, 0x03, 0x04, 0x01, 0x01) },
	{ 1, 0, 0, 0, chunk_from_chars(0xA0, 0x06, 0x04, 0x01, 0x01,
	                                           0x02, 0x01, 0x02) },
	{ 0, 2, 0, 0, chunk_from_chars(0xA0, 0x03, 0x02, 0x01, 0x02) },
	{ 0, 2, 0, 0, chunk_from_chars(0xA0, 0x03, 0x02, 0x01, 0x02,
	                               0x30, 0x03, 0x02, 0x01, 0x03) },
	{ 0, 0, 0, 0, chunk_from_chars(0xA0, 0x04, 0x03, 0x02, 0x00, 0x04) },
	{ 0, 0, 3, 0, chunk_from_chars(0x30, 0x03, 0x02, 0x01, 0x03) },
	{ 0, 0, 0, 4, chunk_from_chars(0x30, 0x03, 0x04, 0x01, 0x04) },
	{ 0, 0, 3, 4, chunk_from_chars(0x30, 0x06, 0x04, 0x01, 0x04,
	                                           0x02, 0x01, 0x03) },
	{ 0, 0, 3, 4, chunk_from_chars(0x30, 0x06, 0x02, 0x01, 0x03,
	                                           0x04, 0x01, 0x04) },
	{ 0, 0, 6, 0, chunk_from_chars(0x30, 0x06, 0x02, 0x01, 0x03,
	                                           0x02, 0x01, 0x03) },
	{ 0, 0, 0, 8, chunk_from_chars(0x30, 0x06, 0x04, 0x01, 0x04,
	                                           0x04, 0x01, 0x04) },
	{ 0, 0, 0, 0, chunk_from_chars(0x30, 0x04, 0x03, 0x02, 0x00, 0x04) },
	{ 0, 0, 0, 0, chunk_from_chars(0x03, 0x02, 0x00, 0x04) }
};

START_TEST(test_asn1_parser_choice)
{
	asn1_parser_t *parser;
	chunk_t object;
	int objectID, i1 = 0, i2 = 0, i3 = 0, i4 = 0;
	bool success;

	parser = asn1_parser_create(choiceObjects, choice_tests[_i].blob);
	while (parser->iterate(parser, &objectID, &object))
	{
		switch (objectID)
		{
			case 2:
				i1 += *object.ptr;
				break;
			case 4:
				i2 += *object.ptr;
				break;
			case 9:
				i3 += *object.ptr;
				break;
			case 11:
				i4 += *object.ptr;
				break;
			default:

				break;
		}
	}
	success = parser->success(parser);
	parser->destroy(parser);

	ck_assert(success == (choice_tests[_i].i1 ||
						  choice_tests[_i].i2 ||
						  choice_tests[_i].i3 ||
						  choice_tests[_i].i4 ));

	ck_assert(i1 == choice_tests[_i].i1 &&
			  i2 == choice_tests[_i].i2 &&
			  i3 == choice_tests[_i].i3 &&
			  i4 == choice_tests[_i].i4 );
}
END_TEST


Suite *asn1_parser_suite_create()
{
	Suite *s;
	TCase *tc;

	s = suite_create("asn1_parser");

	tc = tcase_create("length");
	tcase_add_loop_test(tc, test_asn1_parser_length, 0, countof(length_tests));
	suite_add_tcase(s, tc);

	tc = tcase_create("loop");
	tcase_add_loop_test(tc, test_asn1_parser_loop, 0, countof(loop_tests));
	suite_add_tcase(s, tc);

	tc = tcase_create("default");
	tcase_add_loop_test(tc, test_asn1_parser_default, 0, countof(default_tests));
	suite_add_tcase(s, tc);

	tc = tcase_create("option");
	tcase_add_loop_test(tc, test_asn1_parser_option, 0, countof(option_tests));
	suite_add_tcase(s, tc);

	tc = tcase_create("choice");
	tcase_add_loop_test(tc, test_asn1_parser_choice, 0, countof(choice_tests));
	suite_add_tcase(s, tc);

	return s;
}
