/*
 * tests/check-all.c		overall unit test program
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2013 Thomas Graf <tgraf@suug.ch>
 */

#include <check.h>

extern Suite *make_nl_addr_suite(void);
extern Suite *make_nl_attr_suite(void);

static Suite *main_suite(void)
{
	Suite *suite = suite_create("main");

	return suite;
}

int main(int argc, char *argv[])
{
	SRunner *runner;
	int nfailed;
	
	runner = srunner_create(main_suite());

	/* Add testsuites below */

	srunner_add_suite(runner, make_nl_addr_suite());
	srunner_add_suite(runner, make_nl_attr_suite());

	/* Do not add testsuites below this line */

	srunner_run_all(runner, CK_ENV);

	nfailed = srunner_ntests_failed(runner);
	srunner_free(runner);

	return nfailed != 0;
}
