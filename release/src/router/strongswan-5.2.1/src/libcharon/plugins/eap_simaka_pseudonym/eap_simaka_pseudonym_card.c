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

#include "eap_simaka_pseudonym_card.h"

#include <daemon.h>
#include <collections/hashtable.h>

typedef struct private_eap_simaka_pseudonym_card_t private_eap_simaka_pseudonym_card_t;

/**
 * Private data of an eap_simaka_pseudonym_card_t object.
 */
struct private_eap_simaka_pseudonym_card_t {

	/**
	 * Public eap_simaka_pseudonym_card_t interface.
	 */
	eap_simaka_pseudonym_card_t public;

	/**
	 * Permanent -> pseudonym mappings
	 */
	hashtable_t *pseudonym;

	/**
	 * Reverse pseudonym -> permanent mappings
	 */
	hashtable_t *permanent;
};

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

METHOD(simaka_card_t, get_pseudonym, identification_t*,
	private_eap_simaka_pseudonym_card_t *this, identification_t *id)
{
	identification_t *pseudonym;

	pseudonym = this->pseudonym->get(this->pseudonym, id);
	if (pseudonym)
	{
		return pseudonym->clone(pseudonym);
	}
	return NULL;
}

METHOD(simaka_card_t, set_pseudonym, void,
	private_eap_simaka_pseudonym_card_t *this, identification_t *id,
	identification_t *pseudonym)
{
	identification_t *permanent;

	/* create new entries */
	id = id->clone(id);
	pseudonym = pseudonym->clone(pseudonym);
	permanent = this->permanent->put(this->permanent, pseudonym, id);
	pseudonym = this->pseudonym->put(this->pseudonym, id, pseudonym);

	/* delete old entries */
	DESTROY_IF(permanent);
	DESTROY_IF(pseudonym);
}

METHOD(simaka_card_t, get_quintuplet, status_t,
	private_eap_simaka_pseudonym_card_t *this, identification_t *id,
	char rand[AKA_RAND_LEN], char autn[AKA_AUTN_LEN], char ck[AKA_CK_LEN],
	char ik[AKA_IK_LEN], char res[AKA_RES_MAX], int *res_len)
{
	return NOT_SUPPORTED;
}

METHOD(eap_simaka_pseudonym_card_t, destroy, void,
	private_eap_simaka_pseudonym_card_t *this)
{
	enumerator_t *enumerator;
	identification_t *id;
	void *key;

	enumerator = this->pseudonym->create_enumerator(this->pseudonym);
	while (enumerator->enumerate(enumerator, &key, &id))
	{
		id->destroy(id);
	}
	enumerator->destroy(enumerator);

	enumerator = this->permanent->create_enumerator(this->permanent);
	while (enumerator->enumerate(enumerator, &key, &id))
	{
		id->destroy(id);
	}
	enumerator->destroy(enumerator);

	this->pseudonym->destroy(this->pseudonym);
	this->permanent->destroy(this->permanent);
	free(this);
}

/**
 * See header
 */
eap_simaka_pseudonym_card_t *eap_simaka_pseudonym_card_create()
{
	private_eap_simaka_pseudonym_card_t *this;

	INIT(this,
		.public = {
			.card = {
				.get_triplet = (void*)return_false,
				.get_quintuplet = _get_quintuplet,
				.resync = (void*)return_false,
				.get_pseudonym = _get_pseudonym,
				.set_pseudonym = _set_pseudonym,
				.get_reauth = (void*)return_null,
				.set_reauth = (void*)nop,
			},
			.destroy = _destroy,
		},
		.pseudonym = hashtable_create((void*)hash, (void*)equals, 0),
		.permanent = hashtable_create((void*)hash, (void*)equals, 0),
	);

	return &this->public;
}

