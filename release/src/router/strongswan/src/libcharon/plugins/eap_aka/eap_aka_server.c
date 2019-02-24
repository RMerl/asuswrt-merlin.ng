/*
 * Copyright (C) 2006-2009 Martin Willi
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

#include "eap_aka_server.h"

#include <daemon.h>
#include <library.h>

#include <simaka_message.h>
#include <simaka_crypto.h>
#include <simaka_manager.h>

/** length of the AT_NONCE_S value */
#define NONCE_LEN 16

typedef struct private_eap_aka_server_t private_eap_aka_server_t;

/**
 * Private data of an eap_aka_server_t object.
 */
struct private_eap_aka_server_t {

	/**
	 * Public authenticator_t interface.
	 */
	eap_aka_server_t public;

	/**
	 * AKA backend manager
	 */
	simaka_manager_t *mgr;

	/**
	 * EAP-AKA crypto helper
	 */
	simaka_crypto_t *crypto;

	/**
	 * permanent ID of the peer
	 */
	identification_t *permanent;

	/**
	 * pseudonym ID of peer
	 */
	identification_t *pseudonym;

	/**
	 * reauthentication ID of peer
	 */
	identification_t *reauth;

	/**
	 * EAP message identifier
	 */
	uint8_t identifier;

	/**
	 * Expected Result XRES
	 */
	chunk_t xres;

	/**
	 * Random value RAND
	 */
	chunk_t rand;

	/**
	 * MSK
	 */
	chunk_t msk;

	/**
	 * Nonce value used in AT_NONCE_S
	 */
	chunk_t nonce;

	/**
	 * Counter value negotiated, network order
	 */
	chunk_t counter;

	/**
	 * Do we request fast reauthentication?
	 */
	bool use_reauth;

	/**
	 * Do we request pseudonym identities?
	 */
	bool use_pseudonym;

	/**
	 * Do we request permanent identities?
	 */
	bool use_permanent;

	/**
	 * EAP-AKA message we have initiated
	 */
	simaka_subtype_t pending;

	/**
	 * Did the client send a synchronize request?
	 */
	bool synchronized;
};

/**
 * Generate a payload from a message, destroy message
 */
static bool generate_payload(simaka_message_t *message, chunk_t data,
							 eap_payload_t **out)
{
	chunk_t chunk;
	bool ok;

	ok = message->generate(message, data, &chunk);
	if (ok)
	{
		*out = eap_payload_create_data_own(chunk);
	}
	message->destroy(message);
	return ok;
}

/**
 * Create EAP-AKA/Request/Identity message
 */
static status_t identity(private_eap_aka_server_t *this, eap_payload_t **out)
{
	simaka_message_t *message;

	message = simaka_message_create(TRUE, this->identifier++, EAP_AKA,
									AKA_IDENTITY, this->crypto);
	if (this->use_reauth)
	{
		message->add_attribute(message, AT_ANY_ID_REQ, chunk_empty);
	}
	else if (this->use_pseudonym)
	{
		message->add_attribute(message, AT_FULLAUTH_ID_REQ, chunk_empty);
	}
	else if (this->use_permanent)
	{
		message->add_attribute(message, AT_PERMANENT_ID_REQ, chunk_empty);
	}
	if (!generate_payload(message, chunk_empty, out))
	{
		return FAILED;
	}
	this->pending = AKA_IDENTITY;
	return NEED_MORE;
}

/**
 * Create EAP-AKA/Request/Challenge message
 */
static status_t challenge(private_eap_aka_server_t *this, eap_payload_t **out)
{
	simaka_message_t *message;
	char rand[AKA_RAND_LEN], xres[AKA_RES_MAX];
	char ck[AKA_CK_LEN], ik[AKA_IK_LEN], autn[AKA_AUTN_LEN];
	int xres_len;
	chunk_t data, mk;
	identification_t *id;

	if (!this->mgr->provider_get_quintuplet(this->mgr, this->permanent,
										rand, xres, &xres_len, ck, ik, autn))
	{
		if (this->use_pseudonym)
		{
			/* probably received a pseudonym/reauth id we couldn't map */
			DBG1(DBG_IKE, "failed to map pseudonym/reauth identity '%Y', "
				 "fallback to permanent identity request", this->permanent);
			this->use_pseudonym = FALSE;
			DESTROY_IF(this->pseudonym);
			this->pseudonym = NULL;
			return identity(this, out);
		}
		return FAILED;
	}
	id = this->permanent;
	if (this->pseudonym)
	{
		id = this->pseudonym;
	}
	data = chunk_cata("cc", chunk_create(ik, AKA_IK_LEN),
					  chunk_create(ck, AKA_CK_LEN));
	chunk_clear(&this->msk);
	if (!this->crypto->derive_keys_full(this->crypto, id, data, &mk, &this->msk))
	{
		return FAILED;
	}
	this->rand = chunk_clone(chunk_create(rand, AKA_RAND_LEN));
	this->xres = chunk_clone(chunk_create(xres, xres_len));

	message = simaka_message_create(TRUE, this->identifier++, EAP_AKA,
									AKA_CHALLENGE, this->crypto);
	message->add_attribute(message, AT_RAND, this->rand);
	message->add_attribute(message, AT_AUTN, chunk_create(autn, AKA_AUTN_LEN));
	id = this->mgr->provider_gen_reauth(this->mgr, this->permanent, mk.ptr);
	free(mk.ptr);
	if (id)
	{
		message->add_attribute(message, AT_NEXT_REAUTH_ID,
							   id->get_encoding(id));
		id->destroy(id);
	}
	id = this->mgr->provider_gen_pseudonym(this->mgr, this->permanent);
	if (id)
	{
		message->add_attribute(message, AT_NEXT_PSEUDONYM,
							   id->get_encoding(id));
		id->destroy(id);
	}
	if (!generate_payload(message, chunk_empty, out))
	{
		return FAILED;
	}
	this->pending = AKA_CHALLENGE;
	return NEED_MORE;
}

/**
 * Initiate  EAP-AKA/Request/Re-authentication message
 */
static status_t reauthenticate(private_eap_aka_server_t *this,
							   char mk[HASH_SIZE_SHA1], uint16_t counter,
							   eap_payload_t **out)
{
	simaka_message_t *message;
	identification_t *next;
	chunk_t mkc;
	rng_t *rng;

	DBG1(DBG_IKE, "initiating EAP-AKA reauthentication");

	rng = this->crypto->get_rng(this->crypto);
	if (!rng->allocate_bytes(rng, NONCE_LEN, &this->nonce))
	{
		return FAILED;
	}

	mkc = chunk_create(mk, HASH_SIZE_SHA1);
	counter = htons(counter);
	this->counter = chunk_clone(chunk_create((char*)&counter, sizeof(counter)));

	if (!this->crypto->derive_keys_reauth(this->crypto, mkc) ||
		!this->crypto->derive_keys_reauth_msk(this->crypto,
					this->reauth, this->counter, this->nonce, mkc, &this->msk))
	{
		return FAILED;
	}

	message = simaka_message_create(TRUE, this->identifier++, EAP_AKA,
									AKA_REAUTHENTICATION, this->crypto);
	message->add_attribute(message, AT_COUNTER, this->counter);
	message->add_attribute(message, AT_NONCE_S, this->nonce);
	next = this->mgr->provider_gen_reauth(this->mgr, this->permanent, mk);
	if (next)
	{
		message->add_attribute(message, AT_NEXT_REAUTH_ID,
							   next->get_encoding(next));
		next->destroy(next);
	}
	if (!generate_payload(message, chunk_empty, out))
	{
		return FAILED;
	}
	this->pending = SIM_REAUTHENTICATION;
	return NEED_MORE;
}

METHOD(eap_method_t, initiate, status_t,
	private_eap_aka_server_t *this, eap_payload_t **out)
{
	if (this->use_permanent || this->use_pseudonym || this->use_reauth)
	{
		return identity(this, out);
	}
	return challenge(this, out);
}

/**
 * Process EAP-AKA/Response/Identity message
 */
static status_t process_identity(private_eap_aka_server_t *this,
								 simaka_message_t *in, eap_payload_t **out)
{
	identification_t *permanent, *id;
	enumerator_t *enumerator;
	simaka_attribute_t type;
	chunk_t data, identity = chunk_empty;

	if (this->pending != AKA_IDENTITY)
	{
		DBG1(DBG_IKE, "received %N, but not expected",
			 simaka_subtype_names, AKA_IDENTITY);
		return FAILED;
	}

	enumerator = in->create_attribute_enumerator(in);
	while (enumerator->enumerate(enumerator, &type, &data))
	{
		switch (type)
		{
			case AT_IDENTITY:
				identity = data;
				break;
			default:
				if (!simaka_attribute_skippable(type))
				{
					enumerator->destroy(enumerator);
					return FAILED;
				}
				break;
		}
	}
	enumerator->destroy(enumerator);

	if (!identity.len)
	{
		DBG1(DBG_IKE, "received incomplete Identity response");
		return FAILED;
	}

	id = identification_create_from_data(identity);
	if (this->use_reauth)
	{
		char mk[HASH_SIZE_SHA1];
		uint16_t counter;

		permanent = this->mgr->provider_is_reauth(this->mgr, id, mk, &counter);
		if (permanent)
		{
			this->permanent->destroy(this->permanent);
			this->permanent = permanent;
			this->reauth = id;
			return reauthenticate(this, mk, counter, out);
		}
		/* unable to map, maybe a pseudonym? */
		DBG1(DBG_IKE, "'%Y' is not a reauth identity", id);
		this->use_reauth = FALSE;
	}
	if (this->use_pseudonym)
	{
		permanent = this->mgr->provider_is_pseudonym(this->mgr, id);
		if (permanent)
		{
			this->permanent->destroy(this->permanent);
			this->permanent = permanent;
			this->pseudonym = id->clone(id);
			/* we already have a new permanent identity now */
			this->use_permanent = FALSE;
		}
		else
		{
			DBG1(DBG_IKE, "'%Y' is not a pseudonym", id);
		}
	}
	if (!this->pseudonym && this->use_permanent)
	{
		/* got a permanent identity or a pseudonym reauth id wou couldn't map,
		 * try to get quintuplets */
		DBG1(DBG_IKE, "received identity '%Y'", id);
		this->permanent->destroy(this->permanent);
		this->permanent = id->clone(id);
	}
	id->destroy(id);

	return challenge(this, out);
}

/**
 * Process EAP-AKA/Response/Challenge message
 */
static status_t process_challenge(private_eap_aka_server_t *this,
								  simaka_message_t *in)
{
	enumerator_t *enumerator;
	simaka_attribute_t type;
	chunk_t data, res = chunk_empty;

	if (this->pending != AKA_CHALLENGE)
	{
		DBG1(DBG_IKE, "received %N, but not expected",
			 simaka_subtype_names, AKA_CHALLENGE);
		return FAILED;
	}
	/* verify MAC of EAP message, AT_MAC */
	if (!in->verify(in, chunk_empty))
	{
		return FAILED;
	}
	enumerator = in->create_attribute_enumerator(in);
	while (enumerator->enumerate(enumerator, &type, &data))
	{
		switch (type)
		{
			case AT_RES:
				res = data;
				break;
			default:
				if (!simaka_attribute_skippable(type))
				{
					enumerator->destroy(enumerator);
					return FAILED;
				}
				break;
		}
	}
	enumerator->destroy(enumerator);

	/* compare received RES against stored XRES */
	if (!chunk_equals_const(res, this->xres))
	{
		DBG1(DBG_IKE, "received RES does not match XRES");
		return FAILED;
	}
	return SUCCESS;
}

/**
 * process an EAP-AKA/Response/Reauthentication message
 */
static status_t process_reauthentication(private_eap_aka_server_t *this,
									simaka_message_t *in, eap_payload_t **out)
{
	enumerator_t *enumerator;
	simaka_attribute_t type;
	chunk_t data, counter = chunk_empty;
	bool too_small = FALSE;

	if (this->pending != AKA_REAUTHENTICATION)
	{
		DBG1(DBG_IKE, "received %N, but not expected",
			 simaka_subtype_names, AKA_REAUTHENTICATION);
		return FAILED;
	}
	/* verify AT_MAC attribute, signature is over "EAP packet | NONCE_S"  */
	if (!in->verify(in, this->nonce))
	{
		return FAILED;
	}

	enumerator = in->create_attribute_enumerator(in);
	while (enumerator->enumerate(enumerator, &type, &data))
	{
		switch (type)
		{
			case AT_COUNTER:
				counter = data;
				break;
			case AT_COUNTER_TOO_SMALL:
				too_small = TRUE;
				break;
			default:
				if (!simaka_attribute_skippable(type))
				{
					enumerator->destroy(enumerator);
					return FAILED;
				}
				break;
		}
	}
	enumerator->destroy(enumerator);

	if (too_small)
	{
		DBG1(DBG_IKE, "received %N, initiating full authentication",
			 simaka_attribute_names, AT_COUNTER_TOO_SMALL);
		this->use_reauth = FALSE;
		this->crypto->clear_keys(this->crypto);
		return challenge(this, out);
	}
	if (!chunk_equals_const(counter, this->counter))
	{
		DBG1(DBG_IKE, "received counter does not match");
		return FAILED;
	}
	return SUCCESS;
}

/**
 * Process EAP-AKA/Response/SynchronizationFailure message
 */
static status_t process_synchronize(private_eap_aka_server_t *this,
									simaka_message_t *in, eap_payload_t **out)
{
	enumerator_t *enumerator;
	simaka_attribute_t type;
	chunk_t data, auts = chunk_empty;

	if (this->synchronized)
	{
		DBG1(DBG_IKE, "received %N, but peer did already resynchronize",
			 simaka_subtype_names, AKA_SYNCHRONIZATION_FAILURE);
		return FAILED;
	}

	DBG1(DBG_IKE, "received synchronization request, retrying...");

	enumerator = in->create_attribute_enumerator(in);
	while (enumerator->enumerate(enumerator, &type, &data))
	{
		switch (type)
		{
			case AT_AUTS:
				auts = data;
				break;
			default:
				if (!simaka_attribute_skippable(type))
				{
					enumerator->destroy(enumerator);
					return FAILED;
				}
				break;
		}
	}
	enumerator->destroy(enumerator);

	if (!auts.len)
	{
		DBG1(DBG_IKE, "synchronization request didn't contain usable AUTS");
		return FAILED;
	}

	if (!this->mgr->provider_resync(this->mgr, this->permanent,
									this->rand.ptr, auts.ptr))
	{
		DBG1(DBG_IKE, "no AKA provider found supporting "
			 "resynchronization for '%Y'", this->permanent);
		return FAILED;
	}
	this->synchronized = TRUE;
	return challenge(this, out);
}

/**
 * Process EAP-AKA/Response/ClientErrorCode message
 */
static status_t process_client_error(private_eap_aka_server_t *this,
									 simaka_message_t *in)
{
	enumerator_t *enumerator;
	simaka_attribute_t type;
	chunk_t data;

	enumerator = in->create_attribute_enumerator(in);
	while (enumerator->enumerate(enumerator, &type, &data))
	{
		if (type == AT_CLIENT_ERROR_CODE)
		{
			uint16_t code;

			memcpy(&code, data.ptr, sizeof(code));
			DBG1(DBG_IKE, "received EAP-AKA client error '%N'",
				 simaka_client_error_names, ntohs(code));
		}
		else if (!simaka_attribute_skippable(type))
		{
			break;
		}
	}
	enumerator->destroy(enumerator);
	return FAILED;
}

/**
 * Process EAP-AKA/Response/AuthenticationReject message
 */
static status_t process_authentication_reject(private_eap_aka_server_t *this,
											  simaka_message_t *in)
{
	DBG1(DBG_IKE, "received %N, authentication failed",
		 simaka_subtype_names, in->get_subtype(in));
	return FAILED;
}

METHOD(eap_method_t, process, status_t,
	private_eap_aka_server_t *this,	eap_payload_t *in, eap_payload_t **out)
{
	simaka_message_t *message;
	status_t status;

	message = simaka_message_create_from_payload(in->get_data(in), this->crypto);
	if (!message)
	{
		return FAILED;
	}
	if (!message->parse(message))
	{
		message->destroy(message);
		return FAILED;
	}
	switch (message->get_subtype(message))
	{
		case AKA_IDENTITY:
			status = process_identity(this, message, out);
			break;
		case AKA_CHALLENGE:
			status = process_challenge(this, message);
			break;
		case AKA_REAUTHENTICATION:
			status = process_reauthentication(this, message, out);
			break;
		case AKA_SYNCHRONIZATION_FAILURE:
			status = process_synchronize(this, message, out);
			break;
		case AKA_CLIENT_ERROR:
			status = process_client_error(this, message);
			break;
		case AKA_AUTHENTICATION_REJECT:
			status = process_authentication_reject(this, message);
			break;
		default:
			DBG1(DBG_IKE, "unable to process EAP-AKA subtype %N",
				 simaka_subtype_names, message->get_subtype(message));
			status = FAILED;
			break;
	}
	message->destroy(message);
	return status;
}

METHOD(eap_method_t, get_type, eap_type_t,
	private_eap_aka_server_t *this, uint32_t *vendor)
{
	*vendor = 0;
	return EAP_AKA;
}

METHOD(eap_method_t, get_msk, status_t,
	private_eap_aka_server_t *this, chunk_t *msk)
{
	if (this->msk.ptr)
	{
		*msk = this->msk;
		return SUCCESS;
	}
	return FAILED;
}

METHOD(eap_method_t, get_identifier, uint8_t,
	private_eap_aka_server_t *this)
{
	return this->identifier;
}

METHOD(eap_method_t, set_identifier, void,
	private_eap_aka_server_t *this, uint8_t identifier)
{
	this->identifier = identifier;
}

METHOD(eap_method_t, is_mutual, bool,
	private_eap_aka_server_t *this)
{
	return TRUE;
}

METHOD(eap_method_t, destroy, void,
	private_eap_aka_server_t *this)
{
	this->crypto->destroy(this->crypto);
	this->permanent->destroy(this->permanent);
	DESTROY_IF(this->pseudonym);
	DESTROY_IF(this->reauth);
	free(this->xres.ptr);
	free(this->rand.ptr);
	free(this->nonce.ptr);
	free(this->msk.ptr);
	free(this->counter.ptr);
	free(this);
}

/*
 * Described in header.
 */
eap_aka_server_t *eap_aka_server_create(identification_t *server,
										identification_t *peer)
{
	private_eap_aka_server_t *this;

	INIT(this,
		.public = {
			.interface = {
				.initiate = _initiate,
				.process = _process,
				.get_type = _get_type,
				.is_mutual = _is_mutual,
				.get_msk = _get_msk,
				.get_identifier = _get_identifier,
				.set_identifier = _set_identifier,
				.destroy = _destroy,
			},
		},
		.crypto = simaka_crypto_create(EAP_AKA),
		.mgr = lib->get(lib, "aka-manager"),
	);

	if (!this->crypto)
	{
		free(this);
		return NULL;
	}

	this->permanent = peer->clone(peer);
	this->use_reauth = this->use_pseudonym = this->use_permanent =
		lib->settings->get_bool(lib->settings,
						"%s.plugins.eap-aka.request_identity", TRUE, lib->ns);

	/* generate a non-zero identifier */
	do {
		this->identifier = random();
	} while (!this->identifier);

	return &this->public;
}
