/*
 * Copyright (C) 2014 Martin Willi
 *
 * Copyright (C) secunet Security Networks AG
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

#include <test_runner.h>

#include <daemon.h>

/* declare test suite constructors */
#define TEST_SUITE(x) test_suite_t* x();
#include "vici_tests.h"
#undef TEST_SUITE

static test_configuration_t tests[] = {
#define TEST_SUITE(x) \
	{ .suite = x, },
#include "vici_tests.h"
	{ .suite = NULL, }
};

static bool test_runner_init(bool init)
{
	if (!init)
	{
		lib->processor->set_threads(lib->processor, 0);
		lib->processor->cancel(lib->processor);
	}
	return TRUE;
}

int main(int argc, char *argv[])
{
	return test_runner_run("vici", tests, test_runner_init);
}
