/*
 * Copyright (C) 2009 Martin Willi
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

#include <stdio.h>
#include <time.h>
#include <library.h>
#include <utils/debug.h>
#include <credentials/keys/private_key.h>

void start_timing(struct timespec *start)
{
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, start);
}

double end_timing(struct timespec *start)
{
	struct timespec end;

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end);
	return (end.tv_nsec - start->tv_nsec) / 1000000000.0 +
			(end.tv_sec - start->tv_sec) * 1.0;
}

static void usage()
{
	printf("usage: pubkey_speed plugins rsa|ecdsa rounds < key\n");
	exit(1);
}

int main(int argc, char *argv[])
{
	private_key_t *private;
	public_key_t *public;
	struct timespec timing;
	int round, rounds, read;
	char buf[8096], *pos = buf;
	key_type_t type = KEY_ANY;
	signature_scheme_t scheme = SIGN_UNKNOWN;
	chunk_t keydata, *sigs, data;

	if (argc < 4)
	{
		usage();
	}

	rounds = atoi(argv[3]);

	if (streq(argv[2], "rsa"))
	{
		type = KEY_RSA;
		scheme = SIGN_RSA_EMSA_PKCS1_SHA1;
	}
	else if (streq(argv[2], "ecdsa"))
	{
		type = KEY_ECDSA;
	}
	else
	{
		usage();
	}

	library_init(NULL, "pubkey_speed");
	lib->plugins->load(lib->plugins, argv[1]);
	atexit(library_deinit);

	keydata = chunk_create(buf, 0);
	while ((read = fread(pos, 1, sizeof(buf) - (pos - buf), stdin)))
	{
		pos += read;
		keydata.len += read;
	}

	private = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, type,
								 BUILD_BLOB_PEM, keydata, BUILD_END);
	if (!private)
	{
		printf("parsing private key failed.\n");
		exit(1);
	}
	if (type == KEY_ECDSA)
	{
		switch (private->get_keysize(private))
		{
			case 256:
				scheme = SIGN_ECDSA_256;
				break;
			case 384:
				scheme = SIGN_ECDSA_384;
				break;
			case 521:
				scheme = SIGN_ECDSA_521;
				break;
			default:
				printf("%d bit ECDSA private key size not supported",
						private->get_keysize(private));
				exit(1);
		}
	}

	printf("%4d bit %N: ", private->get_keysize(private),
		key_type_names, type);

	sigs = malloc(sizeof(chunk_t) * rounds);

	data = chunk_from_chars(0x01,0x02,0x03,0x04,0x05,0x06,0x07);
	start_timing(&timing);
	for (round = 0; round < rounds; round++)
	{
		if (!private->sign(private, scheme, data, &sigs[round]))
		{
			printf("creating signature failed\n");
			exit(1);
		}
	};
	printf("sign()/s: %8.1f   ", rounds / end_timing(&timing));

	public = private->get_public_key(private);
	if (!public)
	{
		printf("extracting public key failed\n");
		exit(1);
	}
	start_timing(&timing);
	for (round = 0; round < rounds; round++)
	{
		if (!public->verify(public, scheme, data, sigs[round]))
		{
			printf("signature verification failed\n");
			exit(1);
		}
	}
	printf("verify()/s: %8.1f\n", rounds / end_timing(&timing));
	public->destroy(public);
	private->destroy(private);

	for (round = 0; round < rounds; round++)
	{
		free(sigs[round].ptr);
	}
	free(sigs);
	return 0;
}
