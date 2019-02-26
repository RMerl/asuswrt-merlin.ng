/*
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

#include <test_runner.h>

/* declare test suite constructors */
#define TEST_SUITE(x) test_suite_t* x();
#define TEST_SUITE_DEPEND(x, ...) TEST_SUITE(x)
#include "tls_tests.h"
#undef TEST_SUITE
#undef TEST_SUITE_DEPEND

static test_configuration_t tests[] = {
#define TEST_SUITE(x) \
	{ .suite = x, },
#define TEST_SUITE_DEPEND(x, type, ...) \
	{ .suite = x, .feature = PLUGIN_DEPENDS(type, __VA_ARGS__) },
#include "tls_tests.h"
	{ .suite = NULL, }
};

static bool test_runner_init(bool init)
{
	if (init)
	{
		plugin_loader_add_plugindirs(PLUGINDIR, PLUGINS);
		if (!lib->plugins->load(lib->plugins, PLUGINS))
		{
			return FALSE;
		}
	}
	else
	{
		lib->credmgr->flush_cache(lib->credmgr, CERT_ANY);
		lib->processor->set_threads(lib->processor, 0);
		lib->processor->cancel(lib->processor);
		lib->plugins->unload(lib->plugins);
	}
	return TRUE;
}

int main(int argc, char *argv[])
{
	return test_runner_run("libtls", tests, test_runner_init);
}
