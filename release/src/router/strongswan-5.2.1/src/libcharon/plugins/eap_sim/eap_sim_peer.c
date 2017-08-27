/*
 * Copyright (C) 2007-2009 Martin Willi
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

#include "eap_sim_peer.h"

#include <daemon.h>

#include <simaka_message.h>
#include <simaka_manager.h>

/* number of tries we do authenticate */
#define MAX_TRIES 3

/* number of triplets for one authentication */
#define TRIPLET_COUNT 3

/** length of the AT_NONCE_MT nonce value */
#define NONCE_LEN 16

typedef struct private_eap_sim_peer_t private_eap_sim_peer_t;

/**
 * Private data of an eap_sim_peer_t object.
 */
struct private_eap_sim_peer_t {

	/**
	 * Public authenticator_t interface.
	 */
	eap_sim_peer_t public;

	/**
	 * SIM backend manager
	 */
	simaka_manager_t *mgr;

	/**
	 * permanent ID of peer
	 */
	identification_t *permanent;

	/**
	 * Pseudonym identity the peer uses
	 */
	identification_t *pseudonym;

	/**
	 * Reauthentication identity the peer uses
	 */
	identification_t *reauth;

	/**
	 * EAP message identifier
	 */
	u_int8_t identifier;

	/**
	 * EAP-SIM crypto helper
	 */
	simaka_crypto_t *crypto;

	/**
	 * how many times we try to authenticate
	 */
	int tries;

	/**
	 * version list received from server
	 */
	chunk_t version_list;

	/**
	 * Nonce value used in AT_NONCE_MT/AT_NONCE_S
	 */
	chunk_t nonce;

	/**
	 * MSK, used for EAP-SIM based IKEv2 authentication
	 */
	chunk_t msk;

	/**
	 * Master key, if reauthentication is used
	 */
	char mk[HASH_SIZE_SHA1];

	/**
	 * Counter value if reauthentication is used
	 */
	u_int16_t counter;
};

/* version of SIM protocol we speak */
static chunk_t version = chunk_from_chars(0x00,0x01);

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
 * Create a SIM_CLIENT_ERROR
 */
static bool create_client_error(private_eap_sim_peer_t *this,
								simaka_client_error_t code, eap_payload_t **out)
{
	simaka_message_t *message;
	u_int16_t encoded;

	DBG1(DBG_IKE, "sending client error '%N'", simaka_client_error_names, code);

	message = simaka_message_create(FALSE, this->identifier, EAP_SIM,
									SIM_CLIENT_ERROR, this->crypto);
	encoded = htons(code);
	message->add_attribute(message, AT_CLIENT_ERROR_CODE,
						   chunk_create((char*)&encoded, sizeof(encoded)));
	return generate_payload(message, chunk_empty, out);
}

/**
 * process an EAP-SIM/Request/Start message
 */
static status_t process_start(private_eap_sim_peer_t *this,
							  simaka_message_t *in, eap_payload_t **out)
{
	simaka_message_t *message;
	enumerator_t *enumerator;
	simaka_attribute_t type;
	chunk_t data, id = chunk_empty;
	rng_t *rng;
	bool supported = FALSE;
	simaka_attribute_t id_req = 0;

	/* reset previously uses reauthentication/pseudonym data */
	this->crypto->clear_keys(this->crypto);
	DESTROY_IF(this->pseudonym);
	this->pseudonym = NULL;
	DESTROY_IF(this->reauth);
	this->reauth = NULL;

	enumerator = in->create_attribute_enumerator(in);
	while (enumerator->enumerate(enumerator, &type, &data))
	{
		switch (type)
		{
			case AT_VERSION_LIST:
			{
				free(this->version_list.ptr);
				this->version_list = chunk_clone(data);
				while (data.len >= version.len)
				{
					if (memeq(data.ptr, version.ptr, version.len))
					{
						supported = TRUE;
						break;
					}
				}
				break;
			}
			case AT_ANY_ID_REQ:
			case AT_FULLAUTH_ID_REQ:
			case AT_PERMANENT_ID_REQ:
				id_req = type;
				break;
			default:
				if (!simaka_attribute_skippable(type))
				{
					enumerator->destroy(enumerator);
					if (!create_client_error(this, SIM_UNABLE_TO_PROCESS, out))
					{
						return FAILED;
					}
					return NEED_MORE;
				}
				break;
		}
	}
	enumerator->destroy(enumerator);

	if (!supported)
	{
		DBG1(DBG_IKE, "server does not support EAP-SIM version number 1");
		if (!create_client_error(this, SIM_UNSUPPORTED_VERSION, out))
		{
			return FAILED;
		}
		return NEED_MORE;
	}

	switch (id_req)
	{
		case AT_ANY_ID_REQ:
			this->reauth = this->mgr->card_get_reauth(this->mgr,
									this->permanent, this->mk, &this->counter);
			if (this->reauth)
			{
				id = this->reauth->get_encoding(this->reauth);
				break;
			}
			/* FALL */
		case AT_FULLAUTH_ID_REQ:
			this->pseudonym = this->mgr->card_get_pseudonym(this->mgr,
															this->permanent);
			if (this->pseudonym)
			{
				id = this->pseudonym->get_encoding(this->pseudonym);
				break;
			}
			/* FALL */
		case AT_PERMANENT_ID_REQ:
			id = this->permanent->get_encoding(this->permanent);
			break;
		default:
			break;
	}

	/* generate AT_NONCE_MT value */
	rng = this->crypto->get_rng(this->crypto);
	free(this->nonce.ptr);
	if (!rng->allocate_bytes(rng, NONCE_LEN, &this->nonce))
	{
		return FAILED;
	}

	message = simaka_message_create(FALSE, this->identifier, EAP_SIM,
									SIM_START, this->crypto);
	if (!this->reauth)
	{
		message->add_attribute(message, AT_SELECTED_VERSION, version);
		message->add_attribute(message, AT_NONCE_MT, this->nonce);
	}
	if (id.len)
	{
		message->add_attribute(message, AT_IDENTITY, id);
	}
	if (!generate_payload(message, chunk_empty, out))
	{
		return FAILED;
	}
	return NEED_MORE;
}

/**
 * process an EAP-SIM/Request/Challenge message
 */
static status_t process_challenge(private_eap_sim_peer_t *this,
								  simaka_message_t *in, eap_payload_t **out)
{
	simaka_message_t *message;
	enumerator_t *enumerator;
	simaka_attribute_t type;
	chunk_t data, rands = chunk_empty, kcs, kc, sreses, sres, mk;
	identification_t *id;

	if (this->tries-- <= 0)
	{
		/* give up without notification. This hack is required as some buggy
		 * server implementations won't respect our client-error. */
		return FAILED;
	}

	enumerator = in->create_attribute_enumerator(in);
	while (enumerator->enumerate(enumerator, &type, &data))
	{
		switch (type)
		{
			case AT_RAND:
				rands = data;
				break;
			default:
				if (!simaka_attribute_skippable(type))
				{
					enumerator->destroy(enumerator);
					if (!create_client_error(this, SIM_UNABLE_TO_PROCESS, out))
					{
						return FAILED;
					}
					return NEED_MORE;
				}
				break;
		}
	}
	enumerator->destroy(enumerator);

	/* excepting two or three RAND, each 16 bytes. We require two valid
	 * and different RANDs */
	if ((rands.len != 2 * SIM_RAND_LEN && rands.len != 3 * SIM_RAND_LEN) ||
		memeq(rands.ptr, rands.ptr + SIM_RAND_LEN, SIM_RAND_LEN))
	{
		DBG1(DBG_IKE, "no valid AT_RAND received");
		if (!create_client_error(this, SIM_INSUFFICIENT_CHALLENGES, out))
		{
			return FAILED;
		}
		return NEED_MORE;
	}
	/* get two or three KCs/SRESes from SIM using RANDs */
	kcs = kc = chunk_alloca(rands.len / 2);
	sreses = sres = chunk_alloca(rands.len / 4);
	while (rands.len >= SIM_RAND_LEN)
	{
		if (!this->mgr->card_get_triplet(this->mgr, this->permanent,
										 rands.ptr, sres.ptr, kc.ptr))
		{
			DBG1(DBG_IKE, "unable to get EAP-SIM triplet");
			if (!create_client_error(this, SIM_UNABLE_TO_PROCESS, out))
			{
				return FAILED;
			}
			return NEED_MORE;
		}
		DBG3(DBG_IKE, "got triplet for RAND %b\n  Kc %b\n  SRES %b",
			 rands.ptr, SIM_RAND_LEN, sres.ptr, SIM_SRES_LEN, kc.ptr, SIM_KC_LEN);
		kc = chunk_skip(kc, SIM_KC_LEN);
		sres = chunk_skip(sres, SIM_SRES_LEN);
		rands = chunk_skip(rands, SIM_RAND_LEN);
	}

	id = this->permanent;
	if (this->pseudonym)
	{
		id = this->pseudonym;
	}
	data = chunk_cata("cccc", kcs, this->nonce, this->version_list, version);
	chunk_clear(&this->msk);
	if (!this->crypto->derive_keys_full(this->crypto, id, data, &mk, &this->msk))
	{
		return FAILED;
	}
	memcpy(this->mk, mk.ptr, mk.len);
	chunk_clear(&mk);

	/* Verify AT_MAC attribute, signature is over "EAP packet | NONCE_MT", and
	 * parse() again after key derivation, reading encrypted attributes */
	if (!in->verify(in, this->nonce) || !in->parse(in))
	{
		if (!create_client_error(this, SIM_UNABLE_TO_PROCESS, out))
		{
			return FAILED;
		}
		return NEED_MORE;
	}

	enumerator = in->create_attribute_enumerator(in);
	while (enumerator->enumerate(enumerator, &type, &data))
	{
		switch (type)
		{
			case AT_NEXT_REAUTH_ID:
				this->counter = 0;
				id = identification_create_from_data(data);
				this->mgr->card_set_reauth(this->mgr, this->permanent, id,
										   this->mk, this->counter);
				id->destroy(id);
				break;
			case AT_NEXT_PSEUDONYM:
				id = identification_create_from_data(data);
				this->mgr->card_set_pseudonym(this->mgr, this->permanent, id);
				id->destroy(id);
				break;
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);

	/* build response with AT_MAC, built over "EAP packet | n*SRES" */
	message = simaka_message_create(FALSE, this->identifier, EAP_SIM,
									SIM_CHALLENGE, this->crypto);
	if (!generate_payload(message, sreses, out))
	{
		return FAILED;
	}
	return NEED_MORE;
}

/**
 * Check if a received counter value is acceptable
 */
static bool counter_too_small(private_eap_sim_peer_t *this, chunk_t chunk)
{
	u_int16_t counter;

	memcpy(&counter, chunk.ptr, sizeof(counter));
	counter = htons(counter);
	return counter < this->counter;
}

/**
 * process an EAP-SIM/Request/Re-Authentication message
 */
static status_t process_reauthentication(private_eap_sim_peer_t *this,
									simaka_message_t *in, eap_payload_t **out)
{
	simaka_message_t *message;
	enumerator_t *enumerator;
	simaka_attribute_t type;
	chunk_t data, counter = chunk_empty, nonce = chunk_empty, id = chunk_empty;

	if (!this->reauth)
	{
		DBG1(DBG_IKE, "received %N, but not expected",
			 simaka_subtype_names, SIM_REAUTHENTICATION);
		if (!create_client_error(this, SIM_UNABLE_TO_PROCESS, out))
		{
			return FAILED;
		}
		return NEED_MORE;
	}

	if (!this->crypto->derive_keys_reauth(this->crypto,
									chunk_create(this->mk, HASH_SIZE_SHA1)))
	{
		return FAILED;
	}

	/* verify MAC and parse again with decryption key */
	if (!in->verify(in, chunk_empty) || !in->parse(in))
	{
		if (!create_client_error(this, SIM_UNABLE_TO_PROCESS, out))
		{
			return FAILED;
		}
		return NEED_MORE;
	}

	enumerator = in->create_attribute_enumerator(in);
	while (enumerator->enumerate(enumerator, &type, &data))
	{
		switch (type)
		{
			case AT_COUNTER:
				counter = data;
				break;
			case AT_NONCE_S:
				nonce = data;
				break;
			case AT_NEXT_REAUTH_ID:
				id = data;
				break;
			default:
				if (!simaka_attribute_skippable(type))
				{
					enumerator->destroy(enumerator);
					if (!create_client_error(this, SIM_UNABLE_TO_PROCESS, out))
					{
						return FAILED;
					}
					return NEED_MORE;
				}
				break;
		}
	}
	enumerator->destroy(enumerator);

	if (!nonce.len || !counter.len)
	{
		DBG1(DBG_IKE, "EAP-SIM/Request/Re-Authentication message incomplete");
		if (!create_client_error(this, SIM_UNABLE_TO_PROCESS, out))
		{
			return FAILED;
		}
		return NEED_MORE;
	}

	message = simaka_message_create(FALSE, this->identifier, EAP_SIM,
									SIM_REAUTHENTICATION, this->crypto);
	if (counter_too_small(this, counter))
	{
		DBG1(DBG_IKE, "reauthentication counter too small");
		message->add_attribute(message, AT_COUNTER_TOO_SMALL, chunk_empty);
	}
	else
	{
		chunk_clear(&this->msk);
		if (!this->crypto->derive_keys_reauth_msk(this->crypto,
						this->reauth, counter, nonce,
						chunk_create(this->mk, HASH_SIZE_SHA1), &this->msk))
		{
			message->destroy(message);
			return FAILED;
		}
		if (id.len)
		{
			identification_t *reauth;

			reauth = identification_create_from_data(data);
			this->mgr->card_set_reauth(this->mgr, this->permanent, reauth,
									   this->mk, this->counter);
			reauth->destroy(reauth);
		}
	}
	message->add_attribute(message, AT_COUNTER, counter);
	if (!generate_payload(message, nonce, out))
	{
		return FAILED;
	}
	return NEED_MORE;
}

/**
 * process an EAP-SIM/Request/Notification message
 */
static status_t process_notification(private_eap_sim_peer_t *this,
									 simaka_message_t *in, eap_payload_t **out)
{
	simaka_message_t *message;
	enumerator_t *enumerator;
	simaka_attribute_t type;
	chunk_t data;
	bool success = TRUE;

	enumerator = in->create_attribute_enumerator(in);
	while (enumerator->enumerate(enumerator, &type, &data))
	{
		if (type == AT_NOTIFICATION)
		{
			u_int16_t code;

			memcpy(&code, data.ptr, sizeof(code));
			code = ntohs(code);

			/* test success bit */
			if (!(data.ptr[0] & 0x80))
			{
				DBG1(DBG_IKE, "received EAP-SIM notification error '%N'",
					 simaka_notification_names, code);
			}
			else
			{
				DBG1(DBG_IKE, "received EAP-SIM notification '%N'",
					 simaka_notification_names, code);
			}
		}
		else if (!simaka_attribute_skippable(type))
		{
			success = FALSE;
			break;
		}
	}
	enumerator->destroy(enumerator);

	if (success)
	{	/* empty notification reply */
		message = simaka_message_create(FALSE, this->identifier, EAP_SIM,
										SIM_NOTIFICATION, this->crypto);
		if (!generate_payload(message, chunk_empty, out))
		{
			return FAILED;
		}
	}
	else
	{
		if (!create_client_error(this, SIM_UNABLE_TO_PROCESS, out))
		{
			return FAILED;
		}
	}
	return NEED_MORE;
}

METHOD(eap_method_t, process, status_t,
	private_eap_sim_peer_t *this, eap_payload_t *in, eap_payload_t **out)
{
	simaka_message_t *message;
	status_t status;

	/* store received EAP message identifier */
	this->identifier = in->get_identifier(in);

	message = simaka_message_create_from_payload(in->get_data(in), this->crypto);
	if (!message)
	{
		if (!create_client_error(this, SIM_UNABLE_TO_PROCESS, out))
		{
			return FAILED;
		}
		return NEED_MORE;
	}
	if (!message->parse(message))
	{
		message->destroy(message);
		if (!create_client_error(this, SIM_UNABLE_TO_PROCESS, out))
		{
			return FAILED;
		}
		return NEED_MORE;
	}
	switch (message->get_subtype(message))
	{
		case SIM_START:
			status = process_start(this, message, out);
			break;
		case SIM_CHALLENGE:
			status = process_challenge(this, message, out);
			break;
		case SIM_REAUTHENTICATION:
			status = process_reauthentication(this, message, out);
			break;
		case SIM_NOTIFICATION:
			status = process_notification(this, message, out);
			break;
		default:
			DBG1(DBG_IKE, "unable to process EAP-SIM subtype %N",
				 simaka_subtype_names, message->get_subtype(message));
			if (!create_client_error(this, SIM_UNABLE_TO_PROCESS, out))
			{
				status = FAILED;
			}
			else
			{
				status = NEED_MORE;
			}
			break;
	}
	message->destroy(message);
	return status;
}

METHOD(eap_method_t, initiate, status_t,
	private_eap_sim_peer_t *this, eap_payload_t **out)
{
	/* peer never initiates */
	return FAILED;
}

METHOD(eap_method_t, get_type, eap_type_t,
	private_eap_sim_peer_t *this, u_int32_t *vendor)
{
	*vendor = 0;
	return EAP_SIM;
}

METHOD(eap_method_t, get_msk, status_t,
	private_eap_sim_peer_t *this, chunk_t *msk)
{
	if (this->msk.ptr)
	{
		*msk = this->msk;
		return SUCCESS;
	}
	return FAILED;
}

METHOD(eap_method_t, get_identifier, u_int8_t,
	private_eap_sim_peer_t *this)
{
	return this->identifier;
}

METHOD(eap_method_t, set_identifier, void,
	private_eap_sim_peer_t *this, u_int8_t identifier)
{
	this->identifier = identifier;
}

METHOD(eap_method_t, is_mutual, bool,
	private_eap_sim_peer_t *this)
{
	return TRUE;
}

METHOD(eap_method_t, destroy, void,
	private_eap_sim_peer_t *this)
{
	this->permanent->destroy(this->permanent);
	DESTROY_IF(this->pseudonym);
	DESTROY_IF(this->reauth);
	this->crypto->destroy(this->crypto);
	free(this->version_list.ptr);
	free(this->nonce.ptr);
	free(this->msk.ptr);
	free(this);
}

/*
 * Described in header.
 */
eap_sim_peer_t *eap_sim_peer_create(identification_t *server,
									identification_t *peer)
{
	private_eap_sim_peer_t *this;

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
		.crypto = simaka_crypto_create(EAP_SIM),
		.mgr = lib->get(lib, "sim-manager"),
	);

	if (!this->crypto)
	{
		free(this);
		return NULL;
	}

	this->permanent = peer->clone(peer);
	this->tries = MAX_TRIES;

	return &this->public;
}

