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

#include <library.h>
#include <daemon.h>

/*******************************************************************************
 * SSH agent signature creation and verification
 ******************************************************************************/
bool test_agent()
{
	char *path;
	chunk_t sig, data = chunk_from_chars(0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08);
	private_key_t *private;
	public_key_t *public;

	path = getenv("SSH_AUTH_SOCK");
	if (!path)
	{
		DBG1(DBG_CFG, "ssh-agent not found.");
		return FALSE;
	}

	private = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, KEY_RSA,
								 BUILD_AGENT_SOCKET, path, BUILD_END);
	if (!private)
	{
		return FALSE;
	}
	if (!private->sign(private, SIGN_RSA_EMSA_PKCS1_SHA1, data, &sig))
	{
		return FALSE;
	}
	public = private->get_public_key(private);
	if (!public)
	{
		return FALSE;;
	}
	if (!public->verify(public, SIGN_RSA_EMSA_PKCS1_SHA1, data, sig))
	{
		return FALSE;
	}
	free(sig.ptr);
	data.ptr[1] = 0x01; /* fake it */
	if (public->verify(public, SIGN_RSA_EMSA_PKCS1_SHA1, data, sig))
	{
		return FALSE;
	}

	private->destroy(private);
	public->destroy(public);

	return TRUE;
}

