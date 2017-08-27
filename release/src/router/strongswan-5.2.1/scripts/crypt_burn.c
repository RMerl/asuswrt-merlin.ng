/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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
#include <library.h>

int main(int argc, char *argv[])
{
	const proposal_token_t *token;
	aead_t *aead;
	crypter_t *crypter;
	char buffer[1024], assoc[8], iv[32];
	size_t bs;
	int i = 0, limit = 0;


	library_init(NULL, "crypt_burn");
	lib->plugins->load(lib->plugins, PLUGINS);
	atexit(library_deinit);

	printf("loaded: %s\n", PLUGINS);

	memset(buffer, 0x12, sizeof(buffer));
	memset(assoc, 0x34, sizeof(assoc));
	memset(iv, 0x56, sizeof(iv));

	if (argc < 2)
	{
		fprintf(stderr, "usage: %s <algorithm>!\n", argv[0]);
		return 1;
	}
	if (argc > 2)
	{
		limit = atoi(argv[2]);
	}

	token = lib->proposal->get_token(lib->proposal, argv[1]);
	if (!token)
	{
		fprintf(stderr, "algorithm '%s' unknown!\n", argv[1]);
		return 1;
	}
	if (token->type != ENCRYPTION_ALGORITHM)
	{
		fprintf(stderr, "'%s' is not an encryption/aead algorithm!\n", argv[1]);
		return 1;
	}

	if (encryption_algorithm_is_aead(token->algorithm))
	{
		aead = lib->crypto->create_aead(lib->crypto,
									token->algorithm, token->keysize / 8, 0);
		if (!aead)
		{
			fprintf(stderr, "aead '%s' not supported!\n", argv[1]);
			return 1;
		}
		while (TRUE)
		{
			if (!aead->encrypt(aead,
				chunk_create(buffer, sizeof(buffer) - aead->get_icv_size(aead)),
				chunk_from_thing(assoc),
				chunk_create(iv, aead->get_iv_size(aead)), NULL))
			{
				fprintf(stderr, "aead encryption failed!\n");
				return 1;
			}
			if (!aead->decrypt(aead, chunk_create(buffer, sizeof(buffer)),
				chunk_from_thing(assoc),
				chunk_create(iv, aead->get_iv_size(aead)), NULL))
			{
				fprintf(stderr, "aead integrity check failed!\n");
				return 1;
			}
			if (limit && ++i == limit)
			{
				break;
			}
		}
		aead->destroy(aead);
	}
	else
	{
		crypter = lib->crypto->create_crypter(lib->crypto,
										token->algorithm, token->keysize / 8);
		if (!crypter)
		{
			fprintf(stderr, "crypter '%s' not supported!\n", argv[1]);
			return 1;
		}
		bs = crypter->get_block_size(crypter);

		while (TRUE)
		{
			if (!crypter->encrypt(crypter,
					chunk_create(buffer, sizeof(buffer) / bs * bs),
					chunk_create(iv, crypter->get_iv_size(crypter)), NULL))
			{
				continue;
			}
			if (!crypter->decrypt(crypter,
					chunk_create(buffer, sizeof(buffer) / bs * bs),
					chunk_create(iv, crypter->get_iv_size(crypter)), NULL))
			{
				continue;
			}
			if (limit && ++i == limit)
			{
				break;
			}
		}
		crypter->destroy(crypter);
	}
	return 0;
}
