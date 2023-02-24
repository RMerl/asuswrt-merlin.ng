/*
 * Copyright (C) 2008-2018 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
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

/* the ENGINE API has been deprecated with OpenSSL 3.0 (the provider API should
 * be used instead) */
#define OPENSSL_SUPPRESS_DEPRECATED

#include <openssl/opensslv.h>
#include <openssl/opensslconf.h>

#include "openssl_engine.h"

#if !defined(OPENSSL_NO_ENGINE) && \
	(OPENSSL_VERSION_NUMBER < 0x30000000L || !defined(OPENSSL_NO_DEPRECATED))

#include <openssl/engine.h>

#include "openssl_ec_private_key.h"
#include "openssl_ed_private_key.h"
#include "openssl_rsa_private_key.h"

/**
 * Login to engine with a PIN specified for a keyid
 */
static bool login(ENGINE *engine, chunk_t keyid)
{
	enumerator_t *enumerator;
	shared_key_t *shared;
	identification_t *id;
	chunk_t key;
	char pin[64];
	bool found = FALSE, success = FALSE;

	id = identification_create_from_encoding(ID_KEY_ID, keyid);
	enumerator = lib->credmgr->create_shared_enumerator(lib->credmgr,
														SHARED_PIN, id, NULL);
	while (enumerator->enumerate(enumerator, &shared, NULL, NULL))
	{
		found = TRUE;
		key = shared->get_key(shared);
		if (snprintf(pin, sizeof(pin),
					 "%.*s", (int)key.len, key.ptr) >= sizeof(pin))
		{
			continue;
		}
		if (ENGINE_ctrl_cmd_string(engine, "PIN", pin, 0))
		{
			success = TRUE;
			break;
		}
		else
		{
			DBG1(DBG_CFG, "setting PIN on engine failed");
		}
	}
	enumerator->destroy(enumerator);
	id->destroy(id);
	if (!found)
	{
		DBG1(DBG_CFG, "no PIN found for %#B", &keyid);
	}
	return success;
}

/*
 * Described in header
 */
private_key_t *openssl_private_key_connect(key_type_t type, va_list args)
{
	char *engine_id = NULL;
	char keyname[BUF_LEN];
	chunk_t keyid = chunk_empty;
	EVP_PKEY *key;
	ENGINE *engine;
	int slot = -1;

	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_PKCS11_KEYID:
				keyid = va_arg(args, chunk_t);
				continue;
			case BUILD_PKCS11_SLOT:
				slot = va_arg(args, int);
				continue;
			case BUILD_PKCS11_MODULE:
				engine_id = va_arg(args, char*);
				continue;
			case BUILD_END:
				break;
			default:
				return NULL;
		}
		break;
	}
	if (!keyid.len)
	{
		return NULL;
	}

	memset(keyname, 0, sizeof(keyname));
	if (slot != -1)
	{
		snprintf(keyname, sizeof(keyname), "%d:", slot);
	}
	if (sizeof(keyname) - strlen(keyname) <= keyid.len * 2 + 1)
	{
		return NULL;
	}
	chunk_to_hex(keyid, keyname + strlen(keyname), FALSE);

	if (!engine_id)
	{
		engine_id = lib->settings->get_str(lib->settings,
							"%s.plugins.openssl.engine_id", "pkcs11", lib->ns);
	}
	engine = ENGINE_by_id(engine_id);
	if (!engine)
	{
		DBG2(DBG_LIB, "engine '%s' is not available", engine_id);
		return NULL;
	}
	if (!ENGINE_init(engine))
	{
		DBG1(DBG_LIB, "failed to initialize engine '%s'", engine_id);
		ENGINE_free(engine);
		return NULL;
	}
	ENGINE_free(engine);
	if (!login(engine, keyid))
	{
		DBG1(DBG_LIB, "login to engine '%s' failed", engine_id);
		ENGINE_finish(engine);
		return NULL;
	}
	key = ENGINE_load_private_key(engine, keyname, NULL, NULL);
	ENGINE_finish(engine);
	if (!key)
	{
		DBG1(DBG_LIB, "failed to load private key with ID '%s' from "
			 "engine '%s'", keyname, engine_id);
		return NULL;
	}

	switch (EVP_PKEY_base_id(key))
	{
#ifndef OPENSSL_NO_RSA
		case EVP_PKEY_RSA:
			return openssl_rsa_private_key_create(key, TRUE);
#endif
#ifndef OPENSSL_NO_ECDSA
		case EVP_PKEY_EC:
			return openssl_ec_private_key_create(key, TRUE);
#endif
#if OPENSSL_VERSION_NUMBER >= 0x1010100fL && !defined(OPENSSL_NO_EC)
		case EVP_PKEY_ED25519:
		case EVP_PKEY_ED448:
			return openssl_ed_private_key_create(key, TRUE);
#endif /* OPENSSL_VERSION_NUMBER */
		default:
			EVP_PKEY_free(key);
			break;
	}
	return NULL;
}

/*
 * Described in header
 */
void openssl_engine_deinit()
{
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	ENGINE_cleanup();
#endif
}

/*
 * Described in header
 */
void openssl_engine_init()
{
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	/* activate support for hardware accelerators */
	ENGINE_load_builtin_engines();
	ENGINE_register_all_complete();
#endif
}

#else /* OPENSSL_NO_ENGINE */

private_key_t *openssl_private_key_connect(key_type_t type, va_list args)
{
	return NULL;
}

void openssl_engine_deinit() {}

void openssl_engine_init() {}

#endif /* OPENSSL_NO_ENGINE */
