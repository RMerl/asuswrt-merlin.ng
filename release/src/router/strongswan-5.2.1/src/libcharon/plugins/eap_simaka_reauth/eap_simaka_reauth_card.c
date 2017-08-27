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

#include "eap_simaka_reauth_card.h"

#include <daemon.h>
#include <collections/hashtable.h>

typedef struct private_eap_simaka_reauth_card_t private_eap_simaka_reauth_card_t;

/**
 * Private data of an eap_simaka_reauth_card_t object.
 */
struct private_eap_simaka_reauth_card_t {

	/**
	 * Public eap_simaka_reauth_card_t interface.
	 */
	eap_simaka_reauth_card_t public;

	/**
	 * Permanent -> reauth_data_t mappings
	 */
	hashtable_t *reauth;
};

/**
 * Data associated to a reauthentication identity
 */
typedef struct {
	/** currently used reauthentication identity */
	identification_t *id;
	/** associated permanent identity */
	identification_t *permanent;
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

METHOD(simaka_card_t, get_reauth, identification_t*,
	private_eap_simaka_reauth_card_t *this, identification_t *id,
	char mk[HASH_SIZE_SHA1], u_int16_t *counter)
{
	reauth_data_t *data;
	identification_t *reauth;

	/* look up reauthentication data */
	data = this->reauth->remove(this->reauth, id);
	if (!data)
	{
		return NULL;
	}
	*counter = ++data->counter;
	memcpy(mk, data->mk, HASH_SIZE_SHA1);
	reauth = data->id;
	data->permanent->destroy(data->permanent);
	free(data);
	return reauth;
}

METHOD(simaka_card_t, set_reauth, void,
	private_eap_simaka_reauth_card_t *this, identification_t *id,
	identification_t* next, char mk[HASH_SIZE_SHA1], u_int16_t counter)
{
	reauth_data_t *data;

	data = this->reauth->get(this->reauth, id);
	if (data)
	{
		data->id->destroy(data->id);
	}
	else
	{
		data = malloc_thing(reauth_data_t);
		data->permanent = id->clone(id);
		this->reauth->put(this->reauth, data->permanent, data);
	}
	data->counter = counter;
	data->id = next->clone(next);
	memcpy(data->mk, mk, HASH_SIZE_SHA1);
}

METHOD(simaka_card_t, get_quintuplet, status_t,
	private_eap_simaka_reauth_card_t *this, identification_t *id,
	char rand[AKA_RAND_LEN], char autn[AKA_AUTN_LEN], char ck[AKA_CK_LEN],
	char ik[AKA_IK_LEN], char res[AKA_RES_MAX], int *res_len)
{
	return NOT_SUPPORTED;
}

METHOD(eap_simaka_reauth_card_t, destroy, void,
	private_eap_simaka_reauth_card_t *this)
{
	enumerator_t *enumerator;
	reauth_data_t *data;
	void *key;

	enumerator = this->reauth->create_enumerator(this->reauth);
	while (enumerator->enumerate(enumerator, &key, &data))
	{
		data->id->destroy(data->id);
		data->permanent->destroy(data->permanent);
		free(data);
	}
	enumerator->destroy(enumerator);

	this->reauth->destroy(this->reauth);
	free(this);
}

/**
 * See header
 */
eap_simaka_reauth_card_t *eap_simaka_reauth_card_create()
{
	private_eap_simaka_reauth_card_t *this;

	INIT(this,
		.public = {
			.card = {
				.get_triplet = (void*)return_null,
				.get_quintuplet = _get_quintuplet,
				.resync = (void*)return_false,
				.get_pseudonym = (void*)return_null,
				.set_pseudonym = (void*)nop,
				.get_reauth = _get_reauth,
				.set_reauth = _set_reauth,
			},
			.destroy = _destroy,
		},
		.reauth = hashtable_create((void*)hash, (void*)equals, 0),
	);

	return &this->public;
}

