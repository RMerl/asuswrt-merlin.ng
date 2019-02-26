/*
 * Copyright (C) 2012 Reto Buerki
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
 * HSR Hochschule fuer Technik Rapperswil
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
#include "tkm_diffie_hellman.h"

#include <daemon.h>
#include <collections/hashtable.h>

typedef struct private_tkm_diffie_hellman_t private_tkm_diffie_hellman_t;

static hashtable_t *group_map = NULL;

/**
 * Private data of a tkm_diffie_hellman_t object.
 */
struct private_tkm_diffie_hellman_t {

	/**
	 * Public tkm_diffie_hellman_t interface.
	 */
	tkm_diffie_hellman_t public;

	/**
	 * Diffie Hellman group number.
	 */
	diffie_hellman_group_t group;

	/**
	 * Diffie Hellman public value.
	 */
	dh_pubvalue_type pubvalue;

	/**
	 * Context id.
	 */
	dh_id_type context_id;

};

METHOD(diffie_hellman_t, get_my_public_value, bool,
	private_tkm_diffie_hellman_t *this, chunk_t *value)
{
	sequence_to_chunk(this->pubvalue.data, this->pubvalue.size, value);
	return TRUE;
}

METHOD(diffie_hellman_t, get_shared_secret, bool,
	private_tkm_diffie_hellman_t *this, chunk_t *secret)
{
	*secret = chunk_empty;
	return TRUE;
}


METHOD(diffie_hellman_t, set_other_public_value, bool,
	private_tkm_diffie_hellman_t *this, chunk_t value)
{
	dh_pubvalue_type othervalue;
	othervalue.size = value.len;
	memcpy(&othervalue.data, value.ptr, value.len);

	return ike_dh_generate_key(this->context_id, othervalue) == TKM_OK;
}

METHOD(diffie_hellman_t, get_dh_group, diffie_hellman_group_t,
	private_tkm_diffie_hellman_t *this)
{
	return this->group;
}

METHOD(diffie_hellman_t, destroy, void,
	private_tkm_diffie_hellman_t *this)
{
	if (ike_dh_reset(this->context_id) != TKM_OK)
	{
		DBG1(DBG_LIB, "failed to reset DH context %d", this->context_id);
	}

	tkm->idmgr->release_id(tkm->idmgr, TKM_CTX_DH, this->context_id);
	free(this);
}

METHOD(tkm_diffie_hellman_t, get_id, dh_id_type,
	private_tkm_diffie_hellman_t *this)
{
	return this->context_id;
}

static u_int hash(void *key)
{
	diffie_hellman_group_t k = *(diffie_hellman_group_t*)key;
	return chunk_hash(chunk_from_thing(k));
}

static bool equals(void *key, void *other_key)
{
	return *(diffie_hellman_group_t*)key == *(diffie_hellman_group_t*)other_key;
}

/*
 * Described in header.
 */
int register_dh_mapping()
{
	int count, i;
	char *iana_id_str, *tkm_id_str;
	diffie_hellman_group_t *iana_id;
	uint64_t *tkm_id;
	hashtable_t *map;
	enumerator_t *enumerator;

	map = hashtable_create((hashtable_hash_t)hash,
						   (hashtable_equals_t)equals, 16);

	enumerator = lib->settings->create_key_value_enumerator(lib->settings,
															"%s.dh_mapping",
															lib->ns);

	while (enumerator->enumerate(enumerator, &iana_id_str, &tkm_id_str))
	{
		iana_id = malloc_thing(diffie_hellman_group_t);
		*iana_id = settings_value_as_int(iana_id_str, 0);
		tkm_id = malloc_thing(uint64_t);
		*tkm_id = settings_value_as_int(tkm_id_str, 0);

		map->put(map, iana_id, tkm_id);
	}
	enumerator->destroy(enumerator);

	count = map->get_count(map);
	plugin_feature_t f[count + 1];
	f[0] = PLUGIN_REGISTER(DH, tkm_diffie_hellman_create);

	i = 1;
	enumerator = map->create_enumerator(map);
	while (enumerator->enumerate(enumerator, &iana_id, &tkm_id))
	{
		f[i] = PLUGIN_PROVIDE(DH, *iana_id);
		i++;
	}
	enumerator->destroy(enumerator);

	lib->plugins->add_static_features(lib->plugins, "tkm-dh", f, countof(f),
									  TRUE, NULL, NULL);

	if (count > 0)
	{
		group_map = map;
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
void destroy_dh_mapping()
{
	enumerator_t *enumerator;
	char *key, *value;

	if (group_map)
	{
		enumerator = group_map->create_enumerator(group_map);
		while (enumerator->enumerate(enumerator, &key, &value))
		{
			free(key);
			free(value);
		}
		enumerator->destroy(enumerator);
		group_map->destroy(group_map);
	}
}

/*
 * Described in header.
 */
tkm_diffie_hellman_t *tkm_diffie_hellman_create(diffie_hellman_group_t group)
{
	private_tkm_diffie_hellman_t *this;

	if (!group_map)
	{
		return NULL;
	}

	INIT(this,
		.public = {
			.dh = {
				.get_shared_secret = _get_shared_secret,
				.set_other_public_value = _set_other_public_value,
				.get_my_public_value = _get_my_public_value,
				.get_dh_group = _get_dh_group,
				.destroy = _destroy,
			},
			.get_id = _get_id,
		},
		.group = group,
		.context_id = tkm->idmgr->acquire_id(tkm->idmgr, TKM_CTX_DH),
	);

	if (!this->context_id)
	{
		free(this);
		return NULL;
	}

	uint64_t *dha_id = group_map->get(group_map, &group);
	if (!dha_id)
	{
		free(this);
		return NULL;
	}

	if (ike_dh_create(this->context_id, *dha_id, &this->pubvalue) != TKM_OK)
	{
		free(this);
		return NULL;
	}

	return &this->public;
}
