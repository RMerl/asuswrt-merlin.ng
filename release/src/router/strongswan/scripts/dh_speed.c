/*
 * Copyright (C) 2009 Martin Willi
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

#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <library.h>
#include <utils/debug.h>
#include <crypto/key_exchange.h>

static void usage()
{
	printf("usage: dh_speed plugins rounds group1 [group2 [...]]\n");
	exit(1);
}

struct {
	char *name;
	key_exchange_method_t group;
} groups[] = {
	{"modp768",			MODP_768_BIT},
	{"modp1024",		MODP_1024_BIT},
	{"modp1024s160",	MODP_1024_160},
	{"modp1536",		MODP_1536_BIT},
	{"modp2048",		MODP_2048_BIT},
	{"modp2048s224",	MODP_2048_224},
	{"modp2048s256",	MODP_2048_256},
	{"modp3072",		MODP_3072_BIT},
	{"modp4096",		MODP_4096_BIT},
	{"modp6144",		MODP_6144_BIT},
	{"modp8192",		MODP_8192_BIT},
	{"ecp256",			ECP_256_BIT},
	{"ecp384",			ECP_384_BIT},
	{"ecp521",			ECP_521_BIT},
	{"ecp192",			ECP_192_BIT},
	{"ecp224",			ECP_224_BIT},
	{"curve25519",		CURVE_25519},
	{"curve448",		CURVE_448},
};

static void start_timing(struct timespec *start)
{
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, start);
}

static double end_timing(struct timespec *start)
{
	struct timespec end;

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end);
	return (end.tv_nsec - start->tv_nsec) / 1000000000.0 +
			(end.tv_sec - start->tv_sec) * 1.0;
}

static void run_test(key_exchange_method_t group, int rounds)
{
	key_exchange_t *l[rounds], *r;
	chunk_t chunk, chunks[rounds], lsecrets[rounds], rsecrets[rounds];
	struct timespec timing;
	int round;

	r = lib->crypto->create_ke(lib->crypto, group);
	if (!r)
	{
		printf("skipping %N, not supported\n", key_exchange_method_names,
			   group);
		return;
	}

	printf("%N:\t", key_exchange_method_names, group);

	start_timing(&timing);
	for (round = 0; round < rounds; round++)
	{
		l[round] = lib->crypto->create_ke(lib->crypto, group);
		assert(l[round]->get_public_key(l[round], &chunks[round]));
	}
	printf("A = g^a/s: %8.1f", rounds / end_timing(&timing));

	for (round = 0; round < rounds; round++)
	{
		assert(r->set_public_key(r, chunks[round]));
		assert(r->get_shared_secret(r, &rsecrets[round]));
		chunk_free(&chunks[round]);
	}

	assert(r->get_public_key(r, &chunk));
	start_timing(&timing);
	for (round = 0; round < rounds; round++)
	{
		assert(l[round]->set_public_key(l[round], chunk));
		assert(l[round]->get_shared_secret(l[round], &lsecrets[round]));
	}
	printf(" | S = B^a/s: %8.1f\n", rounds / end_timing(&timing));
	chunk_free(&chunk);

	for (round = 0; round < rounds; round++)
	{
		assert(chunk_equals(rsecrets[round], lsecrets[round]));
		free(lsecrets[round].ptr);
		free(rsecrets[round].ptr);
		l[round]->destroy(l[round]);
	}
	r->destroy(r);
}

int main(int argc, char *argv[])
{
	int rounds, i, j;

	if (argc < 4)
	{
		usage();
	}

	library_init(NULL, "dh_speed");
	lib->plugins->load(lib->plugins, argv[1]);
	atexit(library_deinit);

	rounds = atoi(argv[2]);

	for (i = 3; i < argc; i++)
	{
		bool found = FALSE;

		for (j = 0; j < countof(groups); j++)
		{
			if (streq(groups[j].name, argv[i]))
			{
				run_test(groups[j].group, rounds);
				found = TRUE;
			}
		}
		if (!found)
		{
			printf("group %s not found\n", argv[i]);
		}
	}
	return 0;
}
