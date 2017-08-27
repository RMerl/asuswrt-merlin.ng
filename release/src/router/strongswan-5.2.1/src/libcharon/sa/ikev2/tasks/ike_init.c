/*
 * Copyright (C) 2008-2009 Tobias Brunner
 * Copyright (C) 2005-2008 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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

#include "ike_init.h"

#include <string.h>

#include <daemon.h>
#include <sa/ikev2/keymat_v2.h>
#include <crypto/diffie_hellman.h>
#include <encoding/payloads/sa_payload.h>
#include <encoding/payloads/ke_payload.h>
#include <encoding/payloads/nonce_payload.h>

/** maximum retries to do with cookies/other dh groups */
#define MAX_RETRIES 5

typedef struct private_ike_init_t private_ike_init_t;

/**
 * Private members of a ike_init_t task.
 */
struct private_ike_init_t {

	/**
	 * Public methods and task_t interface.
	 */
	ike_init_t public;

	/**
	 * Assigned IKE_SA.
	 */
	ike_sa_t *ike_sa;

	/**
	 * Are we the initiator?
	 */
	bool initiator;

	/**
	 * IKE config to establish
	 */
	ike_cfg_t *config;

	/**
	 * diffie hellman group to use
	 */
	diffie_hellman_group_t dh_group;

	/**
	 * diffie hellman key exchange
	 */
	diffie_hellman_t *dh;

	/**
	 * Keymat derivation (from IKE_SA)
	 */
	keymat_v2_t *keymat;

	/**
	 * nonce chosen by us
	 */
	chunk_t my_nonce;

	/**
	 * nonce chosen by peer
	 */
	chunk_t other_nonce;

	/**
	 * Negotiated proposal used for IKE_SA
	 */
	proposal_t *proposal;

	/**
	 * Old IKE_SA which gets rekeyed
	 */
	ike_sa_t *old_sa;

	/**
	 * cookie received from responder
	 */
	chunk_t cookie;

	/**
	 * retries done so far after failure (cookie or bad dh group)
	 */
	u_int retry;
};

/**
 * build the payloads for the message
 */
static void build_payloads(private_ike_init_t *this, message_t *message)
{
	sa_payload_t *sa_payload;
	ke_payload_t *ke_payload;
	nonce_payload_t *nonce_payload;
	linked_list_t *proposal_list;
	ike_sa_id_t *id;
	proposal_t *proposal;
	enumerator_t *enumerator;

	id = this->ike_sa->get_id(this->ike_sa);

	this->config = this->ike_sa->get_ike_cfg(this->ike_sa);

	if (this->initiator)
	{
		proposal_list = this->config->get_proposals(this->config);
		if (this->old_sa)
		{
			/* include SPI of new IKE_SA when we are rekeying */
			enumerator = proposal_list->create_enumerator(proposal_list);
			while (enumerator->enumerate(enumerator, (void**)&proposal))
			{
				proposal->set_spi(proposal, id->get_initiator_spi(id));
			}
			enumerator->destroy(enumerator);
		}

		sa_payload = sa_payload_create_from_proposals_v2(proposal_list);
		proposal_list->destroy_offset(proposal_list, offsetof(proposal_t, destroy));
	}
	else
	{
		if (this->old_sa)
		{
			/* include SPI of new IKE_SA when we are rekeying */
			this->proposal->set_spi(this->proposal, id->get_responder_spi(id));
		}
		sa_payload = sa_payload_create_from_proposal_v2(this->proposal);
	}
	message->add_payload(message, (payload_t*)sa_payload);

	nonce_payload = nonce_payload_create(PLV2_NONCE);
	nonce_payload->set_nonce(nonce_payload, this->my_nonce);
	ke_payload = ke_payload_create_from_diffie_hellman(PLV2_KEY_EXCHANGE, this->dh);

	if (this->old_sa)
	{	/* payload order differs if we are rekeying */
		message->add_payload(message, (payload_t*)nonce_payload);
		message->add_payload(message, (payload_t*)ke_payload);
	}
	else
	{
		message->add_payload(message, (payload_t*)ke_payload);
		message->add_payload(message, (payload_t*)nonce_payload);
	}

	/* negotiate fragmentation if we are not rekeying */
	if (!this->old_sa &&
		 this->config->fragmentation(this->config) != FRAGMENTATION_NO)
	{
		if (this->initiator ||
			this->ike_sa->supports_extension(this->ike_sa,
											 EXT_IKE_FRAGMENTATION))
		{
			message->add_notify(message, FALSE, FRAGMENTATION_SUPPORTED,
								chunk_empty);
		}
	}
}

/**
 * Read payloads from message
 */
static void process_payloads(private_ike_init_t *this, message_t *message)
{
	enumerator_t *enumerator;
	payload_t *payload;

	enumerator = message->create_payload_enumerator(message);
	while (enumerator->enumerate(enumerator, &payload))
	{
		switch (payload->get_type(payload))
		{
			case PLV2_SECURITY_ASSOCIATION:
			{
				sa_payload_t *sa_payload = (sa_payload_t*)payload;
				linked_list_t *proposal_list;
				bool private;

				proposal_list = sa_payload->get_proposals(sa_payload);
				private = this->ike_sa->supports_extension(this->ike_sa,
														   EXT_STRONGSWAN);
				this->proposal = this->config->select_proposal(this->config,
														proposal_list, private);
				if (!this->proposal)
				{
					charon->bus->alert(charon->bus, ALERT_PROPOSAL_MISMATCH_IKE,
									   proposal_list);
				}
				proposal_list->destroy_offset(proposal_list,
											  offsetof(proposal_t, destroy));
				break;
			}
			case PLV2_KEY_EXCHANGE:
			{
				ke_payload_t *ke_payload = (ke_payload_t*)payload;

				this->dh_group = ke_payload->get_dh_group_number(ke_payload);
				if (!this->initiator)
				{
					this->dh = this->keymat->keymat.create_dh(
										&this->keymat->keymat, this->dh_group);
				}
				if (this->dh)
				{
					this->dh->set_other_public_value(this->dh,
								ke_payload->get_key_exchange_data(ke_payload));
				}
				break;
			}
			case PLV2_NONCE:
			{
				nonce_payload_t *nonce_payload = (nonce_payload_t*)payload;

				this->other_nonce = nonce_payload->get_nonce(nonce_payload);
				break;
			}
			case PLV2_NOTIFY:
			{
				notify_payload_t *notify = (notify_payload_t*)payload;

				if (notify->get_notify_type(notify) == FRAGMENTATION_SUPPORTED)
				{
					this->ike_sa->enable_extension(this->ike_sa,
												   EXT_IKE_FRAGMENTATION);
				}
			}
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);
}

METHOD(task_t, build_i, status_t,
	private_ike_init_t *this, message_t *message)
{
	this->config = this->ike_sa->get_ike_cfg(this->ike_sa);
	DBG0(DBG_IKE, "initiating IKE_SA %s[%d] to %H",
		 this->ike_sa->get_name(this->ike_sa),
		 this->ike_sa->get_unique_id(this->ike_sa),
		 this->ike_sa->get_other_host(this->ike_sa));
	this->ike_sa->set_state(this->ike_sa, IKE_CONNECTING);

	if (this->retry >= MAX_RETRIES)
	{
		DBG1(DBG_IKE, "giving up after %d retries", MAX_RETRIES);
		return FAILED;
	}

	/* if the DH group is set via use_dh_group(), we already have a DH object */
	if (!this->dh)
	{
		this->dh_group = this->config->get_dh_group(this->config);
		this->dh = this->keymat->keymat.create_dh(&this->keymat->keymat,
												  this->dh_group);
		if (!this->dh)
		{
			DBG1(DBG_IKE, "configured DH group %N not supported",
				diffie_hellman_group_names, this->dh_group);
			return FAILED;
		}
	}

	/* generate nonce only when we are trying the first time */
	if (this->my_nonce.ptr == NULL)
	{
		nonce_gen_t *nonceg;

		nonceg = this->keymat->keymat.create_nonce_gen(&this->keymat->keymat);
		if (!nonceg)
		{
			DBG1(DBG_IKE, "no nonce generator found to create nonce");
			return FAILED;
		}
		if (!nonceg->allocate_nonce(nonceg, NONCE_SIZE, &this->my_nonce))
		{
			DBG1(DBG_IKE, "nonce allocation failed");
			nonceg->destroy(nonceg);
			return FAILED;
		}
		nonceg->destroy(nonceg);
	}

	if (this->cookie.ptr)
	{
		message->add_notify(message, FALSE, COOKIE, this->cookie);
	}

	build_payloads(this, message);

#ifdef ME
	{
		chunk_t connect_id = this->ike_sa->get_connect_id(this->ike_sa);
		if (connect_id.ptr)
		{
			message->add_notify(message, FALSE, ME_CONNECTID, connect_id);
		}
	}
#endif /* ME */

	return NEED_MORE;
}

METHOD(task_t, process_r,  status_t,
	private_ike_init_t *this, message_t *message)
{
	nonce_gen_t *nonceg;

	this->config = this->ike_sa->get_ike_cfg(this->ike_sa);
	DBG0(DBG_IKE, "%H is initiating an IKE_SA", message->get_source(message));
	this->ike_sa->set_state(this->ike_sa, IKE_CONNECTING);

	nonceg = this->keymat->keymat.create_nonce_gen(&this->keymat->keymat);
	if (!nonceg)
	{
		DBG1(DBG_IKE, "no nonce generator found to create nonce");
		return FAILED;
	}
	if (!nonceg->allocate_nonce(nonceg, NONCE_SIZE, &this->my_nonce))
	{
		DBG1(DBG_IKE, "nonce allocation failed");
		nonceg->destroy(nonceg);
		return FAILED;
	}
	nonceg->destroy(nonceg);

#ifdef ME
	{
		notify_payload_t *notify = message->get_notify(message, ME_CONNECTID);
		if (notify)
		{
			chunk_t connect_id = notify->get_notification_data(notify);
			DBG2(DBG_IKE, "received ME_CONNECTID %#B", &connect_id);
			charon->connect_manager->stop_checks(charon->connect_manager,
												 connect_id);
		}
	}
#endif /* ME */

	process_payloads(this, message);

	return NEED_MORE;
}

/**
 * Derive the keymat for the IKE_SA
 */
static bool derive_keys(private_ike_init_t *this,
						chunk_t nonce_i, chunk_t nonce_r)
{
	keymat_v2_t *old_keymat;
	pseudo_random_function_t prf_alg = PRF_UNDEFINED;
	chunk_t skd = chunk_empty;
	ike_sa_id_t *id;

	id = this->ike_sa->get_id(this->ike_sa);
	if (this->old_sa)
	{
		/* rekeying: Include old SKd, use old PRF, apply SPI */
		old_keymat = (keymat_v2_t*)this->old_sa->get_keymat(this->old_sa);
		prf_alg = old_keymat->get_skd(old_keymat, &skd);
		if (this->initiator)
		{
			id->set_responder_spi(id, this->proposal->get_spi(this->proposal));
		}
		else
		{
			id->set_initiator_spi(id, this->proposal->get_spi(this->proposal));
		}
	}
	if (!this->keymat->derive_ike_keys(this->keymat, this->proposal, this->dh,
									   nonce_i, nonce_r, id, prf_alg, skd))
	{
		return FALSE;
	}
	charon->bus->ike_keys(charon->bus, this->ike_sa, this->dh, chunk_empty,
						  nonce_i, nonce_r, this->old_sa, NULL);
	return TRUE;
}

METHOD(task_t, build_r, status_t,
	private_ike_init_t *this, message_t *message)
{
	/* check if we have everything we need */
	if (this->proposal == NULL ||
		this->other_nonce.len == 0 || this->my_nonce.len == 0)
	{
		DBG1(DBG_IKE, "received proposals inacceptable");
		message->add_notify(message, TRUE, NO_PROPOSAL_CHOSEN, chunk_empty);
		return FAILED;
	}
	this->ike_sa->set_proposal(this->ike_sa, this->proposal);

	if (this->dh == NULL ||
		!this->proposal->has_dh_group(this->proposal, this->dh_group))
	{
		u_int16_t group;

		if (this->proposal->get_algorithm(this->proposal, DIFFIE_HELLMAN_GROUP,
										  &group, NULL))
		{
			DBG1(DBG_IKE, "DH group %N inacceptable, requesting %N",
				 diffie_hellman_group_names, this->dh_group,
				 diffie_hellman_group_names, group);
			this->dh_group = group;
			group = htons(group);
			message->add_notify(message, FALSE, INVALID_KE_PAYLOAD,
								chunk_from_thing(group));
		}
		else
		{
			DBG1(DBG_IKE, "no acceptable proposal found");
		}
		return FAILED;
	}

	if (!derive_keys(this, this->other_nonce, this->my_nonce))
	{
		DBG1(DBG_IKE, "key derivation failed");
		message->add_notify(message, TRUE, NO_PROPOSAL_CHOSEN, chunk_empty);
		return FAILED;
	}
	build_payloads(this, message);
	return SUCCESS;
}

/**
 * Raise alerts for received notify errors
 */
static void raise_alerts(private_ike_init_t *this, notify_type_t type)
{
	linked_list_t *list;

	switch (type)
	{
		case NO_PROPOSAL_CHOSEN:
			list = this->config->get_proposals(this->config);
			charon->bus->alert(charon->bus, ALERT_PROPOSAL_MISMATCH_IKE, list);
			list->destroy_offset(list, offsetof(proposal_t, destroy));
			break;
		default:
			break;
	}
}

METHOD(task_t, process_i, status_t,
	private_ike_init_t *this, message_t *message)
{
	enumerator_t *enumerator;
	payload_t *payload;

	/* check for erroneous notifies */
	enumerator = message->create_payload_enumerator(message);
	while (enumerator->enumerate(enumerator, &payload))
	{
		if (payload->get_type(payload) == PLV2_NOTIFY)
		{
			notify_payload_t *notify = (notify_payload_t*)payload;
			notify_type_t type = notify->get_notify_type(notify);

			switch (type)
			{
				case INVALID_KE_PAYLOAD:
				{
					chunk_t data;
					diffie_hellman_group_t bad_group;

					bad_group = this->dh_group;
					data = notify->get_notification_data(notify);
					this->dh_group = ntohs(*((u_int16_t*)data.ptr));
					DBG1(DBG_IKE, "peer didn't accept DH group %N, "
						 "it requested %N", diffie_hellman_group_names,
						 bad_group, diffie_hellman_group_names, this->dh_group);

					if (this->old_sa == NULL)
					{	/* reset the IKE_SA if we are not rekeying */
						this->ike_sa->reset(this->ike_sa);
					}

					enumerator->destroy(enumerator);
					this->retry++;
					return NEED_MORE;
				}
				case NAT_DETECTION_SOURCE_IP:
				case NAT_DETECTION_DESTINATION_IP:
					/* skip, handled in ike_natd_t */
					break;
				case MULTIPLE_AUTH_SUPPORTED:
					/* handled in ike_auth_t */
					break;
				case COOKIE:
				{
					chunk_free(&this->cookie);
					this->cookie = chunk_clone(notify->get_notification_data(notify));
					this->ike_sa->reset(this->ike_sa);
					enumerator->destroy(enumerator);
					DBG2(DBG_IKE, "received %N notify", notify_type_names, type);
					this->retry++;
					return NEED_MORE;
				}
				default:
				{
					if (type <= 16383)
					{
						DBG1(DBG_IKE, "received %N notify error",
							 notify_type_names, type);
						enumerator->destroy(enumerator);
						raise_alerts(this, type);
						return FAILED;
					}
					DBG2(DBG_IKE, "received %N notify",
						notify_type_names, type);
					break;
				}
			}
		}
	}
	enumerator->destroy(enumerator);

	process_payloads(this, message);

	/* check if we have everything */
	if (this->proposal == NULL ||
		this->other_nonce.len == 0 || this->my_nonce.len == 0)
	{
		DBG1(DBG_IKE, "peers proposal selection invalid");
		return FAILED;
	}
	this->ike_sa->set_proposal(this->ike_sa, this->proposal);

	if (this->dh == NULL ||
		!this->proposal->has_dh_group(this->proposal, this->dh_group))
	{
		DBG1(DBG_IKE, "peer DH group selection invalid");
		return FAILED;
	}

	if (!derive_keys(this, this->my_nonce, this->other_nonce))
	{
		DBG1(DBG_IKE, "key derivation failed");
		return FAILED;
	}
	return SUCCESS;
}

METHOD(task_t, get_type, task_type_t,
	private_ike_init_t *this)
{
	return TASK_IKE_INIT;
}

METHOD(task_t, migrate, void,
	private_ike_init_t *this, ike_sa_t *ike_sa)
{
	DESTROY_IF(this->proposal);
	chunk_free(&this->other_nonce);

	this->ike_sa = ike_sa;
	this->keymat = (keymat_v2_t*)ike_sa->get_keymat(ike_sa);
	this->proposal = NULL;
	if (this->dh && this->dh->get_dh_group(this->dh) != this->dh_group)
	{	/* reset DH value only if group changed (INVALID_KE_PAYLOAD) */
		this->dh->destroy(this->dh);
		this->dh = this->keymat->keymat.create_dh(&this->keymat->keymat,
												  this->dh_group);
	}
}

METHOD(task_t, destroy, void,
	private_ike_init_t *this)
{
	DESTROY_IF(this->dh);
	DESTROY_IF(this->proposal);
	chunk_free(&this->my_nonce);
	chunk_free(&this->other_nonce);
	chunk_free(&this->cookie);
	free(this);
}

METHOD(ike_init_t, get_lower_nonce, chunk_t,
	private_ike_init_t *this)
{
	if (memcmp(this->my_nonce.ptr, this->other_nonce.ptr,
			   min(this->my_nonce.len, this->other_nonce.len)) < 0)
	{
		return this->my_nonce;
	}
	else
	{
		return this->other_nonce;
	}
}

/*
 * Described in header.
 */
ike_init_t *ike_init_create(ike_sa_t *ike_sa, bool initiator, ike_sa_t *old_sa)
{
	private_ike_init_t *this;

	INIT(this,
		.public = {
			.task = {
				.get_type = _get_type,
				.migrate = _migrate,
				.destroy = _destroy,
			},
			.get_lower_nonce = _get_lower_nonce,
		},
		.ike_sa = ike_sa,
		.initiator = initiator,
		.dh_group = MODP_NONE,
		.keymat = (keymat_v2_t*)ike_sa->get_keymat(ike_sa),
		.old_sa = old_sa,
	);

	if (initiator)
	{
		this->public.task.build = _build_i;
		this->public.task.process = _process_i;
	}
	else
	{
		this->public.task.build = _build_r;
		this->public.task.process = _process_r;
	}

	return &this->public;
}
