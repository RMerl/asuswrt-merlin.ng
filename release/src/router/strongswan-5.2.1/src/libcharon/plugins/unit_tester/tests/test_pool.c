/*
 * Copyright (C) 2008 Martin Willi
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

#include <time.h>

#include <library.h>
#include <threading/thread.h>
#include <hydra.h>

#define ALLOCS 1000
#define THREADS 20

static void* testing(void *thread)
{
	int i;
	host_t *addr[ALLOCS];
	identification_t *id[ALLOCS];
	linked_list_t *pools;

	/* prepare identities */
	for (i = 0; i < ALLOCS; i++)
	{
		char buf[256];

		snprintf(buf, sizeof(buf), "%d-%d@strongswan.org", (uintptr_t)thread, i);
		id[i] = identification_create_from_string(buf);
	}

	pools = linked_list_create();
	pools->insert_last(pools, "test");

	/* allocate addresses */
	for (i = 0; i < ALLOCS; i++)
	{
		addr[i] = hydra->attributes->acquire_address(hydra->attributes,
													 pools, id[i], NULL);
		if (!addr[i])
		{
			pools->destroy(pools);
			return (void*)FALSE;
		}
	}

	/* release addresses */
	for (i = 0; i < ALLOCS; i++)
	{
		hydra->attributes->release_address(hydra->attributes,
										   pools, addr[i], id[i]);
	}

	pools->destroy(pools);

	/* cleanup */
	for (i = 0; i < ALLOCS; i++)
	{
		addr[i]->destroy(addr[i]);
		id[i]->destroy(id[i]);
	}
	return (void*)TRUE;
}


/*******************************************************************************
 * SQL pool performance test
 ******************************************************************************/
bool test_pool()
{
	thread_t *threads[THREADS];
	uintptr_t i;

	for (i = 0; i < THREADS; i++)
	{
		if (!(threads[i] = thread_create((thread_main_t)testing, (void*)i)))
		{
			return FALSE;
		}
	}
	for (i = 0; i < THREADS; i++)
	{
		bool *res = threads[i]->join(threads[i]);
		if (!res)
		{
			return FALSE;
		}
	}
	return TRUE;
}

