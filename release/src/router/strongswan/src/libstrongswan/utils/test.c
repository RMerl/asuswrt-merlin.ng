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

#include "test.h"

#include <library.h>

/**
 * A collection of testable functions
 */
static hashtable_t *functions = NULL;

#ifndef WIN32
bool test_runner_available __attribute__((weak));
#endif

/**
 * Check if we have libtest linkage and need testable functions
 */
static bool has_libtest_linkage()
{
#ifdef WIN32
	return dlsym(RTLD_DEFAULT, "test_runner_available");
#else
	return test_runner_available;
#endif
}

/*
 * Described in header.
 */
void testable_function_register(char *name, void *fn)
{
	bool old = FALSE;

	if (lib && lib->leak_detective)
	{
		old = lib->leak_detective->set_state(lib->leak_detective, FALSE);
	}

	if (has_libtest_linkage())
	{
		if (!functions)
		{
			chunk_hash_seed();
			functions = hashtable_create(hashtable_hash_str,
										 hashtable_equals_str, 8);
		}
		if (fn)
		{
			functions->put(functions, name, fn);
		}
		else
		{
			functions->remove(functions, name);
			if (functions->get_count(functions) == 0)
			{
				functions->destroy(functions);
				functions = NULL;
			}
		}
	}

	if (lib && lib->leak_detective)
	{
		lib->leak_detective->set_state(lib->leak_detective, old);
	}
}

/*
 * Described in header.
 */
void* testable_function_get(char *name)
{
	if (functions)
	{
		return functions->get(functions, name);
	}
	return NULL;
}
