// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2002, Uwe Bonnes
 * Copyright (c) 2001-2004, Roger Dingledine.
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson.
 * Copyright (c) 2007-2016, The Tor Project, Inc.
 * Copyright (c) 2020, EPAM Systems Inc.
 *
 * Unit tests for sscanf() function
 */

#include <common.h>
#include <command.h>
#include <log.h>
#include <test/lib.h>
#include <test/test.h>
#include <test/ut.h>

#define EOF -1

/**
 * lib_sscanf() - unit test for sscanf()
 * @uts: unit test state
 *
 * Test sscanf() with varied parameters in different combinations passed
 * as arguments.
 *
 * Return: 0 - success
 *	   1 - failure
 */
static int lib_sscanf(struct unit_test_state *uts)
{
	char buffer[100], buffer1[100];
	int result, ret;
	static const char pname[] = " Hello World!\n";
	int hour = 21, min = 59, sec = 20;
	int number, number_so_far;
	unsigned int u1, u2, u3;
	char s1[20], s2[10], s3[10], ch;
	int r, int1, int2;
	long lng1;

	/* check EOF */
	strcpy(buffer, "");
	ret = sscanf(buffer, "%d", &result);
	ut_asserteq(ret, EOF);

	/* check %x */
	strcpy(buffer, "0x519");
	ut_asserteq(sscanf(buffer, "%x", &result), 1);
	ut_asserteq(result, 0x519);

	strcpy(buffer, "0x51a");
	ut_asserteq(sscanf(buffer, "%x", &result), 1);
	ut_asserteq(result, 0x51a);

	strcpy(buffer, "0x51g");
	ut_asserteq(sscanf(buffer, "%x", &result), 1);
	ut_asserteq(result, 0x51);

	/* check strings */
	ret = sprintf(buffer, " %s", pname);
	ret = sscanf(buffer, "%*c%[^\n]", buffer1);
	ut_asserteq(ret, 1);
	ut_asserteq(strncmp(pname, buffer1, strlen(buffer1)), 0);

	/* check digits */
	ret = sprintf(buffer, "%d:%d:%d", hour, min, sec);
	ret = sscanf(buffer, "%d%n", &number, &number_so_far);
	ut_asserteq(ret, 1);
	ut_asserteq(number, hour);
	ut_asserteq(number_so_far, 2);

	ret = sscanf(buffer + 2, "%*c%n", &number_so_far);
	ut_asserteq(ret, 0);
	ut_asserteq(number_so_far, 1);

	/* Check %i */
	strcpy(buffer, "123");
	ret = sscanf(buffer, "%i", &result);
	ut_asserteq(ret, 1);
	ut_asserteq(result, 123);
	ret = sscanf(buffer, "%d", &result);
	ut_asserteq(ret, 1);
	ut_asserteq(result, 123);

	ut_asserteq(0, sscanf("hello world", "hello world"));
	ut_asserteq(0, sscanf("hello world", "good bye"));
	/* Excess data */
	ut_asserteq(0, sscanf("hello 3", "%u", &u1));  /* have to match the start */
	ut_asserteq(1, sscanf("3 hello", "%u", &u1));  /* but trailing is alright */

	/* Numbers (ie. %u) */
	ut_asserteq(0, sscanf("hello world 3", "hello worlb %u", &u1)); /* d vs b */
	ut_asserteq(1, sscanf("12345", "%u", &u1));
	ut_asserteq(12345u, u1);
	ut_asserteq(1, sscanf("0", "%u", &u1));
	ut_asserteq(0u, u1);
	ut_asserteq(1, sscanf("0000", "%u", &u2));
	ut_asserteq(0u, u2);
	ut_asserteq(0, sscanf("A", "%u", &u1)); /* bogus number */

	/* Numbers with size (eg. %2u) */
	ut_asserteq(2, sscanf("123456", "%2u%u", &u1, &u2));
	ut_asserteq(12u, u1);
	ut_asserteq(3456u, u2);
	ut_asserteq(1, sscanf("123456", "%8u", &u1));
	ut_asserteq(123456u, u1);
	ut_asserteq(1, sscanf("123457  ", "%8u", &u1));
	ut_asserteq(123457u, u1);
	ut_asserteq(3, sscanf("!12:3:456", "!%2u:%2u:%3u", &u1, &u2, &u3));
	ut_asserteq(12u, u1);
	ut_asserteq(3u, u2);
	ut_asserteq(456u, u3);
	ut_asserteq(3, sscanf("67:8:099", "%2u:%2u:%3u", &u1, &u2, &u3)); /* 0s */
	ut_asserteq(67u, u1);
	ut_asserteq(8u, u2);
	ut_asserteq(99u, u3);
	/* Arbitrary amounts of 0-padding are okay */
	ut_asserteq(3, sscanf("12:03:000000000000000099", "%2u:%2u:%u", &u1, &u2, &u3));
	ut_asserteq(12u, u1);
	ut_asserteq(3u, u2);
	ut_asserteq(99u, u3);

	/* Hex (ie. %x) */
	ut_asserteq(3, sscanf("1234 02aBcdEf ff", "%x %x %x", &u1, &u2, &u3));
	ut_asserteq(0x1234, u1);
	ut_asserteq(0x2ABCDEF, u2);
	ut_asserteq(0xFF, u3);
	/* Width works on %x */
	ut_asserteq(3, sscanf("f00dcafe444", "%4x%4x%u", &u1, &u2, &u3));
	ut_asserteq(0xf00d, u1);
	ut_asserteq(0xcafe, u2);
	ut_asserteq(444, u3);

	/* Literal '%' (ie. '%%') */
	ut_asserteq(1, sscanf("99% fresh", "%3u%% fresh", &u1));
	ut_asserteq(99, u1);
	ut_asserteq(0, sscanf("99 fresh", "%% %3u %s", &u1, s1));
	ut_asserteq(1, sscanf("99 fresh", "%3u%% %s", &u1, s1));
	ut_asserteq(2, sscanf("99 fresh", "%3u %5s %%", &u1, s1));
	ut_asserteq(99, u1);
	ut_asserteq_str(s1, "fresh");
	ut_asserteq(1, sscanf("% boo", "%% %3s", s1));
	ut_asserteq_str("boo", s1);

	/* Strings (ie. %s) */
	ut_asserteq(2, sscanf("hello", "%3s%7s", s1, s2));
	ut_asserteq_str(s1, "hel");
	ut_asserteq_str(s2, "lo");
	ut_asserteq(2, sscanf("WD40", "%2s%u", s3, &u1)); /* %s%u */
	ut_asserteq_str(s3, "WD");
	ut_asserteq(40, u1);
	ut_asserteq(2, sscanf("WD40", "%3s%u", s3, &u1)); /* %s%u */
	ut_asserteq_str(s3, "WD4");
	ut_asserteq(0, u1);
	ut_asserteq(2, sscanf("76trombones", "%6u%9s", &u1, s1)); /* %u%s */
	ut_asserteq(76, u1);
	ut_asserteq_str(s1, "trombones");

	ut_asserteq(3, sscanf("1.2.3", "%u.%u.%u%c", &u1, &u2, &u3, &ch));
	ut_asserteq(4, sscanf("1.2.3 foobar", "%u.%u.%u%c", &u1, &u2, &u3, &ch));
	ut_asserteq(' ', ch);

	r = sscanf("12345 -67890 -1", "%d %ld %d", &int1, &lng1, &int2);
	ut_asserteq(r, 3);
	ut_asserteq(int1, 12345);
	ut_asserteq(lng1, -67890);
	ut_asserteq(int2, -1);

	return 0;
}

LIB_TEST(lib_sscanf, 0);
