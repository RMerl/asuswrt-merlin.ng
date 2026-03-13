/*
 * Copyright (C) 2012 Reto Buerki
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
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

#include <tkm/client.h>
#include <tkm/constants.h>

#include "tkm.h"
#include "tkm_utils.h"
#include "tkm_key_exchange.h"

#include <daemon.h>
#include <collections/hashtable.h>

typedef struct private_tkm_key_exchange_t private_tkm_key_exchange_t;

static hashtable_t *method_map = NULL;

/**
 * Private data of a tkm_key_exchange_t object.
 */
struct private_tkm_key_exchange_t {

	/**
	 * Public tkm_key_exchange_t interface.
	 */
	tkm_key_exchange_t public;

	/**
	 * Key exchange method identifier.
	 */
	key_exchange_method_t method;

	/**
	 * Key exchange algorithm ID corresponding to method.
	 */
	uint64_t kea_id;

	/**
	 * Context id.
	 */
	ke_id_type context_id;

};

METHOD(key_exchange_t, get_public_key, bool,
	private_tkm_key_exchange_t *this, chunk_t *value)
{
	blob_id_type pubvalue_id;
	blob_length_type pubvalue_length;
	bool ret = FALSE;

	pubvalue_id = tkm->idmgr->acquire_id(tkm->idmgr, TKM_CTX_BLOB);
	if (pubvalue_id)
	{
		ret = ike_ke_get(this->context_id, this->kea_id, pubvalue_id,
						 &pubvalue_length) == TKM_OK &&
			  blob_to_chunk(pubvalue_id, pubvalue_length, value);

		tkm->idmgr->release_id(tkm->idmgr, TKM_CTX_BLOB, pubvalue_id);
	}
	return ret;
}

METHOD(key_exchange_t, get_shared_secret, bool,
	private_tkm_key_exchange_t *this, chunk_t *secret)
{
	*secret = chunk_empty;
	return TRUE;
}

METHOD(key_exchange_t, set_public_key, bool,
	private_tkm_key_exchange_t *this, chunk_t value)
{
	blob_id_type pubvalue_id;
	bool ret = FALSE;

	if (!key_exchange_verify_pubkey(this->method, value))
	{
		return FALSE;
	}

	pubvalue_id = tkm->idmgr->acquire_id(tkm->idmgr, TKM_CTX_BLOB);
	if (pubvalue_id)
	{
		ret = chunk_to_blob(pubvalue_id, &value) &&
		      ike_ke_set(this->context_id, this->kea_id, pubvalue_id) == TKM_OK;

		tkm->idmgr->release_id(tkm->idmgr, TKM_CTX_BLOB, pubvalue_id);
	}
	return ret;
}

METHOD(key_exchange_t, get_method, key_exchange_method_t,
	private_tkm_key_exchange_t *this)
{
	return this->method;
}

METHOD(key_exchange_t, destroy, void,
	private_tkm_key_exchange_t *this)
{
	if (ike_ke_reset(this->context_id) != TKM_OK)
	{
		DBG1(DBG_LIB, "failed to reset KE context %d", this->context_id);
	}

	tkm->idmgr->release_id(tkm->idmgr, TKM_CTX_KE, this->context_id);
	free(this);
}

METHOD(tkm_key_exchange_t, get_id, ke_id_type,
	private_tkm_key_exchange_t *this)
{
	return this->context_id;
}

static u_int hash(void *key)
{
	key_exchange_method_t k = *(key_exchange_method_t*)key;
	return chunk_hash(chunk_from_thing(k));
}

static bool equals(void *key, void *other_key)
{
	return *(key_exchange_method_t*)key == *(key_exchange_method_t*)other_key;
}

/*
 * Described in header.
 */
int register_ke_mapping()
{
	int count, i;
	char *iana_id_str, *tkm_id_str;
	key_exchange_method_t *iana_id;
	uint64_t *tkm_id;
	hashtable_t *map;
	enumerator_t *enumerator;

	map = hashtable_create((hashtable_hash_t)hash,
						   (hashtable_equals_t)equals, 16);

	enumerator = lib->settings->create_key_value_enumerator(lib->settings,
															"%s.ke_mapping",
															lib->ns);

	while (enumerator->enumerate(enumerator, &iana_id_str, &tkm_id_str))
	{
		iana_id = malloc_thing(key_exchange_method_t);
		*iana_id = settings_value_as_int(iana_id_str, 0);
		tkm_id = malloc_thing(uint64_t);
		*tkm_id = settings_value_as_int(tkm_id_str, 0);

		map->put(map, iana_id, tkm_id);
	}
	enumerator->destroy(enumerator);

	count = map->get_count(map);
	plugin_feature_t f[count + 1];
	f[0] = PLUGIN_REGISTER(KE, tkm_key_exchange_create);

	i = 1;
	enumerator = map->create_enumerator(map);
	while (enumerator->enumerate(enumerator, &iana_id, &tkm_id))
	{
		f[i] = PLUGIN_PROVIDE(KE, *iana_id);
		i++;
	}
	enumerator->destroy(enumerator);

	lib->plugins->add_static_features(lib->plugins, "tkm-ke", f, countof(f),
									  TRUE, NULL, NULL);

	if (count > 0)
	{
		method_map = map;
	}
	else
	{
		map->destroy(map);
	}

	return count;
}

/*
 * Described in header.
 */
void destroy_ke_mapping()
{
	enumerator_t *enumerator;
	char *key, *value;

	if (method_map)
	{
		enumerator = method_map->create_enumerator(method_map);
		while (enumerator->enumerate(enumerator, &key, &value))
		{
			free(key);
			free(value);
		}
		enumerator->destroy(enumerator);
		method_map->destroy(method_map);
		method_map = NULL;
	}
}

/*
 * Described in header.
 */
tkm_key_exchange_t *tkm_key_exchange_create(key_exchange_method_t method)
{
	private_tkm_key_exchange_t *this;

	if (!method_map)
	{
		return NULL;
	}

	INIT(this,
		.public = {
			.ke = {
				.get_shared_secret = _get_shared_secret,
				.set_public_key = _set_public_key,
				.get_public_key = _get_public_key,
				.get_method = _get_method,
				.destroy = _destroy,
			},
			.get_id = _get_id,
		},
		.method = method,
		.context_id = tkm->idmgr->acquire_id(tkm->idmgr, TKM_CTX_KE),
	);

	if (!this->context_id)
	{
		free(this);
		return NULL;
	}

	uint64_t *kea_id_ptr = method_map->get(method_map, &method);
	if (!kea_id_ptr)
	{
		free(this);
		return NULL;
	}

	this->kea_id = *kea_id_ptr;

	return &this->public;
}
