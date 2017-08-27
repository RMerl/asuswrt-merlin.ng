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
#include <collections/enumerator.h>

#include <unistd.h>

/*******************************************************************************
 * fetch public key from mediation database
 ******************************************************************************/

bool test_med_db()
{
	chunk_t found, keyid = chunk_from_chars(
		0xed,0x90,0xe6,0x4f,0xec,0xa2,0x1f,0x4b,
		0x68,0x97,0x99,0x24,0x22,0xe0,0xde,0x21,
		0xb9,0xd6,0x26,0x29
	);
	identification_t *id;
	enumerator_t *enumerator;
	public_key_t *public;
	auth_cfg_t *auth;
	bool good = FALSE;

	id = identification_create_from_encoding(ID_KEY_ID, keyid);
	enumerator = lib->credmgr->create_public_enumerator(lib->credmgr,
														KEY_ANY, id, NULL);
	while (enumerator->enumerate(enumerator, &public, &auth))
	{
		good = public->get_fingerprint(public, KEYID_PUBKEY_SHA1, &found);
		if (good)
		{
			good = chunk_equals(id->get_encoding(id), found);
		}
	}
	enumerator->destroy(enumerator);
	id->destroy(id);
	return good;
}

