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

#include "eap_simaka_reauth_provider.h"

#include <daemon.h>
#include <collections/hashtable.h>

typedef struct private_eap_simaka_reauth_provider_t private_eap_simaka_reauth_provider_t;

/**
 * Private data of an eap_simaka_reauth_provider_t object.
 */
struct private_eap_simaka_reauth_provider_t {

	/**
	 * Public eap_simaka_reauth_provider_t interface.
	 */
	eap_simaka_reauth_provider_t public;

	/**
	 * Permanent -> reauth_data_t mappings
	 */
	hashtable_t *reauth;

	/**
	 * Reverse reauth -> permanent mappings
	 */
	hashtable_t *permanent;

	/**
	 * RNG for pseudonyms/reauth identities
	 */
	rng_t *rng;
};

/**
 * Data associated to a reauthentication identity
 */
typedef struct {
	/** currently used reauthentication identity */
	identification_t *id;
	/** counter value */
	u_int16_t counter;
	/** master key */
	char mk[HASH_SIZE_SHA1];
} reauth_data_t;

/**
 * hashtable hash function
 */
static u_int hash(identification_t *key)
{
	return chunk_hash(key->get_encoding(key));
}

/**
 * hashtable equals function
 */
static bool equals(identification_t *key1, identification_t *key2)
{
	return key1->equals(key1, key2);
}

/**
 * Generate a random identity
 */
static identification_t *gen_identity(private_eap_simaka_reauth_provider_t *this)
{
	char buf[8], hex[sizeof(buf) * 2 + 1];

	if (!this->rng->get_bytes(this->rng, sizeof(buf), buf))
	{
		return NULL;
	}
	chunk_to_hex(chunk_create(buf, sizeof(buf)), hex, FALSE);

	return identification_create_from_string(hex);
}

METHOD(simaka_provider_t, is_reauth, identification_t*,
	private_eap_simaka_reauth_provider_t *this, identification_t *id,
	char mk[HASH_SIZE_SHA1], u_int16_t *counter)
{
	identification_t *permanent;
	reauth_data_t *data;

	/* look up permanent identity */
	permanent = this->permanent->get(this->permanent, id);
	if (!permanent)
	{
		return NULL;
	}
	/* look up reauthentication data */
	data = this->reauth->get(this->reauth, permanent);
	if (!data)
	{
		return NULL;
	}
	*counter = ++data->counter;
	memcpy(mk, data->mk, HASH_SIZE_SHA1);
	return permanent->clone(permanent);
}

METHOD(simaka_provider_t, gen_reauth, identification_t*,
	private_eap_simaka_reauth_provider_t *this, identification_t *id,
	char mk[HASH_SIZE_SHA1])
{
	reauth_data_t *data;
	identification_t *permanent, *new_id;

	new_id = gen_identity(this);
	if (!new_id)
	{
		DBG1(DBG_CFG, "failed to generate identity");
		return NULL;
	}

	data = this->reauth->get(this->reauth, id);
	if (data)
	{	/* update existing entry */
		permanent = this->permanent->remove(this->permanent, data->id);
		if (permanent)
		{
			data->id->destroy(data->id);
			data->id = new_id;
			this->permanent->put(this->permanent, data->id, permanent);
		}
		else
		{
			new_id->destroy(new_id);
		}
	}
	else
	{	/* generate new entry */
		INIT(data,
			.id = new_id,
		);
		id = id->clone(id);
		this->reauth->put(this->reauth, id, data);
		this->permanent->put(this->permanent, data->id, id);
	}
	memcpy(data->mk, mk, HASH_SIZE_SHA1);

	return data->id->clone(data->id);
}

METHOD(eap_simaka_reauth_provider_t, destroy, void,
	private_eap_simaka_reauth_provider_t *this)
{
	enumerator_t *enumerator;
	identification_t *id;
	reauth_data_t *data;
	void *key;

	enumerator = this->permanent->create_enumerator(this->permanent);
	while (enumerator->enumerate(enumerator, &key, &id))
	{
		id->destroy(id);
	}
	enumerator->destroy(enumerator);

	enumerator = this->reauth->create_enumerator(this->reauth);
	while (enumerator->enumerate(enumerator, &key, &data))
	{
		data->id->destroy(data->id);
		free(data);
	}
	enumerator->destroy(enumerator);

	this->permanent->destroy(this->permanent);
	this->reauth->destroy(this->reauth);
	this->rng->destroy(this->rng);
	free(this);
}

/**
 * See header
 */
eap_simaka_reauth_provider_t *eap_simaka_reauth_provider_create()
{
	private_eap_simaka_reauth_provider_t *this;

	INIT(this,
		.public = {
			.provider = {
				.get_triplet = (void*)return_false,
				.get_quintuplet = (void*)return_false,
				.resync = (void*)return_false,
				.is_pseudonym = (void*)return_null,
				.gen_pseudonym = (void*)return_null,
				.is_reauth = _is_reauth,
				.gen_reauth = _gen_reauth,
			},
			.destroy = _destroy,
		},
		.rng = lib->crypto->create_rng(lib->crypto, RNG_WEAK),
	);
	if (!this->rng)
	{
		free(this);
		return NULL;
	}
	this->permanent = hashtable_create((void*)hash, (void*)equals, 0);
	this->reauth = hashtable_create((void*)hash, (void*)equals, 0);

	return &this->public;
}

