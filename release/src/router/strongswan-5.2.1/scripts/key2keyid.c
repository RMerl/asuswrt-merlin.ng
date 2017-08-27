/*
 * Copyright (C) 2008-2009 Martin Willi
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
#include <library.h>
#include <utils/debug.h>
#include <credentials/keys/private_key.h>
#include <credentials/keys/public_key.h>

/**
 * print the keyids of a private or public key
 */
int main(int argc, char *argv[])
{
	public_key_t *public;
	private_key_t *private;
	chunk_t chunk;
	char buf[8096];
	int read;

	library_init(NULL, "key2keyid");
	lib->plugins->load(lib->plugins, PLUGINS);
	atexit(library_deinit);

	read = fread(buf, 1, sizeof(buf), stdin);
	if (read <= 0)
	{
		fprintf(stderr, "reading key failed.\n");
		return -1;
	}

	chunk = chunk_create(buf, read);

	private = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, KEY_RSA,
								 BUILD_BLOB_PEM, chunk_clone(chunk),
								 BUILD_END);
	if (private)
	{
		printf("parsed %d bits %N private key.\n",
			   private->get_keysize(private),
			   key_type_names, private->get_type(private));
		if (private->get_fingerprint(private, KEYID_PUBKEY_INFO_SHA1, &chunk))
		{
			printf("subjectPublicKeyInfo keyid: %#B\n", &chunk);
		}
		if (private->get_fingerprint(private, KEYID_PUBKEY_SHA1, &chunk))
		{
			printf("subjectPublicKey keyid:     %#B\n", &chunk);
		}
		if (private->get_fingerprint(private, KEYID_PGPV3, &chunk))
		{
			printf("PGP version 3 keyid:        %#B\n", &chunk);
		}
		private->destroy(private);
		return 0;
	}

	public = lib->creds->create(lib->creds, CRED_PUBLIC_KEY, KEY_ANY,
								BUILD_BLOB_PEM, chunk_clone(chunk),
								BUILD_END);
	if (!public)
	{
		public = lib->creds->create(lib->creds, CRED_PUBLIC_KEY, KEY_RSA,
									BUILD_BLOB_PEM, chunk_clone(chunk),
									BUILD_END);
	}
	if (public)
	{
		printf("parsed %d bits %N public key.\n",
			   public->get_keysize(public),
			   key_type_names, public->get_type(public));
		if (public->get_fingerprint(public, KEYID_PUBKEY_INFO_SHA1, &chunk))
		{
			printf("subjectPublicKeyInfo keyid: %#B\n", &chunk);
		}
		if (public->get_fingerprint(public, KEYID_PUBKEY_SHA1, &chunk))
		{
			printf("subjectPublicKey keyid:     %#B\n", &chunk);
		}
		if (public->get_fingerprint(public, KEYID_PGPV3, &chunk))
		{
			printf("PGP version 3 keyid:        %#B\n", &chunk);
		}
		public->destroy(public);
		return 0;
	}

	fprintf(stderr, "unable to parse input key.\n");
	return -1;
}

