/*
 * Copyright (C) 2023 Tobias Brunner
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
	printf("usage: dh_speed plugins rounds ke1 [ke2 [...]]\n");
	exit(1);
}

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

static void run_test(key_exchange_method_t method, int rounds)
{
	key_exchange_t *l[rounds], *r[rounds];
	chunk_t lpublic[rounds], rpublic[rounds], lsecret[rounds], rsecret[rounds];
	struct timespec timing;
	int round;

	r[0] = lib->crypto->create_ke(lib->crypto, method);
	if (!r[0])
	{
		fprintf(stderr, "skipping %N, not supported\n", key_exchange_method_names,
				method);
		return;
	}
	assert(r[0]->get_public_key(r[0], &rpublic[0]));
	for (round = 1; round < rounds; round++)
	{
		r[round] = lib->crypto->create_ke(lib->crypto, method);
		assert(r[round]->get_public_key(r[round], &rpublic[round]));
	}

	printf("%N:\t", key_exchange_method_names, method);

	start_timing(&timing);
	for (round = 0; round < rounds; round++)
	{
		l[round] = lib->crypto->create_ke(lib->crypto, method);
		assert(l[round]->get_public_key(l[round], &lpublic[round]));
	}
	printf("A = g^a/s: %8.1f", rounds / end_timing(&timing));

	for (round = 0; round < rounds; round++)
	{
		assert(r[round]->set_public_key(r[round], lpublic[round]));
		assert(r[round]->get_shared_secret(r[round], &rsecret[round]));
		chunk_free(&lpublic[round]);
	}

	start_timing(&timing);
	for (round = 0; round < rounds; round++)
	{
		assert(l[round]->set_public_key(l[round], rpublic[round]));
		assert(l[round]->get_shared_secret(l[round], &lsecret[round]));
	}
	printf(" | S = B^a/s: %8.1f\n", rounds / end_timing(&timing));

	for (round = 0; round < rounds; round++)
	{
		assert(chunk_equals(rsecret[round], lsecret[round]));
		chunk_free(&lsecret[round]);
		chunk_free(&rsecret[round]);
		chunk_free(&rpublic[round]);
		l[round]->destroy(l[round]);
		r[round]->destroy(r[round]);
	}
}

int main(int argc, char *argv[])
{
	const proposal_token_t *token;
	int rounds, i;

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
		token = lib->proposal->get_token(lib->proposal, argv[i]);
		if (!token)
		{
			fprintf(stderr, "KE method '%s' not found\n", argv[i]);
			return 1;
		}
		else if (token->type != KEY_EXCHANGE_METHOD)
		{
			fprintf(stderr, "'%s' is not a KE method\n", argv[i]);
			return 1;
		}

		run_test(token->algorithm, rounds);
	}
	return 0;
}
