/*
 * Copyright (C) 2008-2018 Tobias Brunner
 * Copyright (C) 2005-2008 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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

#include "ike_init.h"

#include <string.h>

#include <daemon.h>
#include <bio/bio_reader.h>
#include <bio/bio_writer.h>
#include <sa/ikev2/keymat_v2.h>
#include <crypto/diffie_hellman.h>
#include <crypto/hashers/hash_algorithm_set.h>
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
	 * diffie hellman group to use
	 */
	diffie_hellman_group_t dh_group;

	/**
	 * diffie hellman key exchange
	 */
	diffie_hellman_t *dh;

	/**
	 * Applying DH public value failed?
	 */
	bool dh_failed;

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
	 * nonce generator
	 */
	nonce_gen_t *nonceg;

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

	/**
	 * Whether to use Signature Authentication as per RFC 7427
	 */
	bool signature_authentication;

	/**
	 * Whether to follow IKEv2 redirects as per RFC 5685
	 */
	bool follow_redirects;
};

/**
 * Allocate our own nonce value
 */
static bool generate_nonce(private_ike_init_t *this)
{
	if (!this->nonceg)
	{
		DBG1(DBG_IKE, "no nonce generator found to create nonce");
		return FALSE;
	}
	if (!this->nonceg->allocate_nonce(this->nonceg, NONCE_SIZE,
									  &this->my_nonce))
	{
		DBG1(DBG_IKE, "nonce allocation failed");
		return FALSE;
	}
	return TRUE;
}

/**
 * Notify the peer about the hash algorithms we support or expect,
 * as per RFC 7427
 */
static void send_supported_hash_algorithms(private_ike_init_t *this,
										   message_t *message)
{
	hash_algorithm_set_t *algos;
	enumerator_t *enumerator, *rounds;
	bio_writer_t *writer;
	hash_algorithm_t hash;
	peer_cfg_t *peer;
	auth_cfg_t *auth;
	auth_rule_t rule;
	signature_params_t *config;
	int written;
	size_t len = BUF_LEN;
	char buf[len];
	char *pos = buf;
	char *plugin_name;

	algos = hash_algorithm_set_create();
	peer = this->ike_sa->get_peer_cfg(this->ike_sa);
	if (peer)
	{
		rounds = peer->create_auth_cfg_enumerator(peer, FALSE);
		while (rounds->enumerate(rounds, &auth))
		{
			enumerator = auth->create_enumerator(auth);
			while (enumerator->enumerate(enumerator, &rule, &config))
			{
				if (rule == AUTH_RULE_IKE_SIGNATURE_SCHEME)
				{
					hash = hasher_from_signature_scheme(config->scheme,
														config->params);
					if (hasher_algorithm_for_ikev2(hash))
					{
						algos->add(algos, hash);
					}
				}
			}
			enumerator->destroy(enumerator);
		}
		rounds->destroy(rounds);
	}

	if (!algos->count(algos))
	{
		enumerator = lib->crypto->create_hasher_enumerator(lib->crypto);
		while (enumerator->enumerate(enumerator, &hash, &plugin_name))
		{
			if (hasher_algorithm_for_ikev2(hash))
			{
				algos->add(algos, hash);
			}
		}
		enumerator->destroy(enumerator);
	}

	if (algos->count(algos))
	{
		writer = bio_writer_create(0);
		enumerator = algos->create_enumerator(algos);
		while (enumerator->enumerate(enumerator, &hash))
		{
			writer->write_uint16(writer, hash);

			/* generate debug output */
			written = snprintf(pos, len, " %N", hash_algorithm_short_names,
							   hash);
			if (written > 0 && written < len)
			{
				pos += written;
				len -= written;
			}
		}
		enumerator->destroy(enumerator);
		message->add_notify(message, FALSE, SIGNATURE_HASH_ALGORITHMS,
							writer->get_buf(writer));
		writer->destroy(writer);

		*pos = '\0';
		DBG2(DBG_CFG, "sending supported signature hash algorithms:%s", buf);
	}
	algos->destroy(algos);
}

/**
 * Store algorithms supported by other peer
 */
static void handle_supported_hash_algorithms(private_ike_init_t *this,
											 notify_payload_t *notify)
{
	bio_reader_t *reader;
	uint16_t algo;
	int written;
	size_t len = BUF_LEN;
	char buf[len];
	char *pos = buf;
	bool added = FALSE;

	reader = bio_reader_create(notify->get_notification_data(notify));
	while (reader->remaining(reader) >= 2 && reader->read_uint16(reader, &algo))
	{
		if (hasher_algorithm_for_ikev2(algo))
		{
			this->keymat->add_hash_algorithm(this->keymat, algo);
			added = TRUE;

			/* generate debug output */
			written = snprintf(pos, len, " %N", hash_algorithm_short_names,
							   algo);
			if (written > 0 && written < len)
			{
				pos += written;
				len -= written;
			}
		}
	}
	reader->destroy(reader);

	*pos = '\0';
	DBG2(DBG_CFG, "received supported signature hash algorithms:%s", buf);

	if (added)
	{
		this->ike_sa->enable_extension(this->ike_sa, EXT_SIGNATURE_AUTH);
	}
}

/**
 * Check whether to send a USE_PPK notify
 */
static bool send_use_ppk(private_ike_init_t *this)
{
	peer_cfg_t *peer;
	enumerator_t *keys;
	shared_key_t *key;
	bool use_ppk = FALSE;

	if (this->initiator)
	{
		peer = this->ike_sa->get_peer_cfg(this->ike_sa);
		if (peer->get_ppk_id(peer))
		{
			use_ppk = TRUE;
		}
	}
	else if (this->ike_sa->supports_extension(this->ike_sa, EXT_PPK))
	{
		/* check if we have at least one PPK available */
		keys = lib->credmgr->create_shared_enumerator(lib->credmgr, SHARED_PPK,
													  NULL, NULL);
		if (keys->enumerate(keys, &key, NULL, NULL))
		{
			use_ppk = TRUE;
		}
		keys->destroy(keys);
	}
	return use_ppk;
}

/**
 * build the payloads for the message
 */
static bool build_payloads(private_ike_init_t *this, message_t *message)
{
	sa_payload_t *sa_payload;
	ke_payload_t *ke_payload;
	nonce_payload_t *nonce_payload;
	linked_list_t *proposal_list, *other_dh_groups;
	ike_sa_id_t *id;
	proposal_t *proposal;
	enumerator_t *enumerator;
	ike_cfg_t *ike_cfg;

	id = this->ike_sa->get_id(this->ike_sa);

	ike_cfg = this->ike_sa->get_ike_cfg(this->ike_sa);

	if (this->initiator)
	{
		proposal_list = ike_cfg->get_proposals(ike_cfg);
		other_dh_groups = linked_list_create();
		enumerator = proposal_list->create_enumerator(proposal_list);
		while (enumerator->enumerate(enumerator, (void**)&proposal))
		{
			/* include SPI of new IKE_SA when we are rekeying */
			if (this->old_sa)
			{
				proposal->set_spi(proposal, id->get_initiator_spi(id));
			}
			/* move the selected DH group to the front of the proposal */
			if (!proposal->promote_dh_group(proposal, this->dh_group))
			{	/* the proposal does not include the group, move to the back */
				proposal_list->remove_at(proposal_list, enumerator);
				other_dh_groups->insert_last(other_dh_groups, proposal);
			}
		}
		enumerator->destroy(enumerator);
		/* add proposals that don't contain the selected group */
		enumerator = other_dh_groups->create_enumerator(other_dh_groups);
		while (enumerator->enumerate(enumerator, (void**)&proposal))
		{	/* no need to remove from the list as we destroy it anyway*/
			proposal_list->insert_last(proposal_list, proposal);
		}
		enumerator->destroy(enumerator);
		other_dh_groups->destroy(other_dh_groups);

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

	ke_payload = ke_payload_create_from_diffie_hellman(PLV2_KEY_EXCHANGE,
													   this->dh);
	if (!ke_payload)
	{
		DBG1(DBG_IKE, "creating KE payload failed");
		return FALSE;
	}
	nonce_payload = nonce_payload_create(PLV2_NONCE);
	nonce_payload->set_nonce(nonce_payload, this->my_nonce);

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
		 ike_cfg->fragmentation(ike_cfg) != FRAGMENTATION_NO)
	{
		if (this->initiator ||
			this->ike_sa->supports_extension(this->ike_sa,
											 EXT_IKE_FRAGMENTATION))
		{
			message->add_notify(message, FALSE, FRAGMENTATION_SUPPORTED,
								chunk_empty);
		}
	}
	/* submit supported hash algorithms for signature authentication */
	if (!this->old_sa && this->signature_authentication)
	{
		if (this->initiator ||
			this->ike_sa->supports_extension(this->ike_sa,
											 EXT_SIGNATURE_AUTH))
		{
			send_supported_hash_algorithms(this, message);
		}
	}
	/* notify other peer if we support redirection */
	if (!this->old_sa && this->initiator && this->follow_redirects)
	{
		identification_t *gateway;
		host_t *from;
		chunk_t data;

		from = this->ike_sa->get_redirected_from(this->ike_sa);
		if (from)
		{
			gateway = identification_create_from_sockaddr(
													from->get_sockaddr(from));
			data = redirect_data_create(gateway, chunk_empty);
			message->add_notify(message, FALSE, REDIRECTED_FROM, data);
			chunk_free(&data);
			gateway->destroy(gateway);
		}
		else
		{
			message->add_notify(message, FALSE, REDIRECT_SUPPORTED,
								chunk_empty);
		}
	}
	/* notify the peer if we want to use/support PPK */
	if (!this->old_sa && send_use_ppk(this))
	{
		message->add_notify(message, FALSE, USE_PPK, chunk_empty);
	}
	return TRUE;
}

/**
 * Process the SA payload and select a proposal
 */
static void process_sa_payload(private_ike_init_t *this, message_t *message,
							   sa_payload_t *sa_payload)
{
	ike_cfg_t *ike_cfg, *cfg, *alt_cfg = NULL;
	enumerator_t *enumerator;
	linked_list_t *proposal_list;
	host_t *me, *other;
	bool private, prefer_configured;

	ike_cfg = this->ike_sa->get_ike_cfg(this->ike_sa);

	proposal_list = sa_payload->get_proposals(sa_payload);
	private = this->ike_sa->supports_extension(this->ike_sa, EXT_STRONGSWAN);
	prefer_configured = lib->settings->get_bool(lib->settings,
							"%s.prefer_configured_proposals", TRUE, lib->ns);

	this->proposal = ike_cfg->select_proposal(ike_cfg, proposal_list, private,
											  prefer_configured);
	if (!this->proposal)
	{
		if (!this->initiator && !this->old_sa)
		{
			me = message->get_destination(message);
			other = message->get_source(message);
			enumerator = charon->backends->create_ike_cfg_enumerator(
											charon->backends, me, other, IKEV2);
			while (enumerator->enumerate(enumerator, &cfg))
			{
				if (ike_cfg == cfg)
				{	/* already tried and failed */
					continue;
				}
				DBG1(DBG_IKE, "no matching proposal found, trying alternative "
					 "config");
				this->proposal = cfg->select_proposal(cfg, proposal_list,
													private, prefer_configured);
				if (this->proposal)
				{
					alt_cfg = cfg->get_ref(cfg);
					break;
				}
			}
			enumerator->destroy(enumerator);
		}
		if (alt_cfg)
		{
			this->ike_sa->set_ike_cfg(this->ike_sa, alt_cfg);
			alt_cfg->destroy(alt_cfg);
		}
		else
		{
			charon->bus->alert(charon->bus, ALERT_PROPOSAL_MISMATCH_IKE,
							   proposal_list);
		}
	}
	proposal_list->destroy_offset(proposal_list,
								  offsetof(proposal_t, destroy));
}

/**
 * Read payloads from message
 */
static void process_payloads(private_ike_init_t *this, message_t *message)
{
	enumerator_t *enumerator;
	payload_t *payload;
	ke_payload_t *ke_payload = NULL;

	enumerator = message->create_payload_enumerator(message);
	while (enumerator->enumerate(enumerator, &payload))
	{
		switch (payload->get_type(payload))
		{
			case PLV2_SECURITY_ASSOCIATION:
			{
				process_sa_payload(this, message, (sa_payload_t*)payload);
				break;
			}
			case PLV2_KEY_EXCHANGE:
			{
				ke_payload = (ke_payload_t*)payload;

				this->dh_group = ke_payload->get_dh_group_number(ke_payload);
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

				switch (notify->get_notify_type(notify))
				{
					case FRAGMENTATION_SUPPORTED:
						this->ike_sa->enable_extension(this->ike_sa,
													   EXT_IKE_FRAGMENTATION);
						break;
					case SIGNATURE_HASH_ALGORITHMS:
						if (this->signature_authentication)
						{
							handle_supported_hash_algorithms(this, notify);
						}
						break;
					case USE_PPK:
						if (!this->old_sa)
						{
							this->ike_sa->enable_extension(this->ike_sa,
														   EXT_PPK);
						}
						break;
					case REDIRECTED_FROM:
					{
						identification_t *gateway;
						chunk_t data;

						data = notify->get_notification_data(notify);
						gateway = redirect_data_parse(data, NULL);
						if (!gateway)
						{
							DBG1(DBG_IKE, "received invalid REDIRECTED_FROM "
								 "notify, ignored");
							break;
						}
						DBG1(DBG_IKE, "client got redirected from %Y", gateway);
						gateway->destroy(gateway);
						/* fall-through */
					}
					case REDIRECT_SUPPORTED:
						if (!this->old_sa)
						{
							this->ike_sa->enable_extension(this->ike_sa,
														   EXT_IKE_REDIRECTION);
						}
						break;
					default:
						/* other notifies are handled elsewhere */
						break;
				}

			}
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);

	if (this->proposal)
	{
		this->ike_sa->set_proposal(this->ike_sa, this->proposal);
	}

	if (ke_payload && this->proposal &&
		this->proposal->has_dh_group(this->proposal, this->dh_group))
	{
		if (!this->initiator)
		{
			this->dh = this->keymat->keymat.create_dh(
								&this->keymat->keymat, this->dh_group);
		}
		else if (this->dh)
		{
			this->dh_failed = this->dh->get_dh_group(this->dh) != this->dh_group;
		}
		if (this->dh && !this->dh_failed)
		{
			this->dh_failed = !this->dh->set_other_public_value(this->dh,
								ke_payload->get_key_exchange_data(ke_payload));
		}
	}
}

METHOD(task_t, build_i, status_t,
	private_ike_init_t *this, message_t *message)
{
	ike_cfg_t *ike_cfg;

	ike_cfg = this->ike_sa->get_ike_cfg(this->ike_sa);

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

	/* if we are retrying after an INVALID_KE_PAYLOAD we already have one */
	if (!this->dh)
	{
		if (this->old_sa && lib->settings->get_bool(lib->settings,
								"%s.prefer_previous_dh_group", TRUE, lib->ns))
		{	/* reuse the DH group we used for the old IKE_SA when rekeying */
			proposal_t *proposal;
			uint16_t dh_group;

			proposal = this->old_sa->get_proposal(this->old_sa);
			if (proposal->get_algorithm(proposal, DIFFIE_HELLMAN_GROUP,
										&dh_group, NULL))
			{
				this->dh_group = dh_group;
			}
			else
			{	/* this shouldn't happen, but let's be safe */
				this->dh_group = ike_cfg->get_dh_group(ike_cfg);
			}
		}
		else
		{
			this->dh_group = ike_cfg->get_dh_group(ike_cfg);
		}
		this->dh = this->keymat->keymat.create_dh(&this->keymat->keymat,
												  this->dh_group);
		if (!this->dh)
		{
			DBG1(DBG_IKE, "configured DH group %N not supported",
				diffie_hellman_group_names, this->dh_group);
			return FAILED;
		}
	}
	else if (this->dh->get_dh_group(this->dh) != this->dh_group)
	{	/* reset DH instance if group changed (INVALID_KE_PAYLOAD) */
		this->dh->destroy(this->dh);
		this->dh = this->keymat->keymat.create_dh(&this->keymat->keymat,
												  this->dh_group);
		if (!this->dh)
		{
			DBG1(DBG_IKE, "requested DH group %N not supported",
				 diffie_hellman_group_names, this->dh_group);
			return FAILED;
		}
	}

	/* generate nonce only when we are trying the first time */
	if (this->my_nonce.ptr == NULL)
	{
		if (!generate_nonce(this))
		{
			return FAILED;
		}
	}

	if (this->cookie.ptr)
	{
		message->add_notify(message, FALSE, COOKIE, this->cookie);
	}

	if (!build_payloads(this, message))
	{
		return FAILED;
	}

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
	DBG0(DBG_IKE, "%H is initiating an IKE_SA", message->get_source(message));
	this->ike_sa->set_state(this->ike_sa, IKE_CONNECTING);

	if (!generate_nonce(this))
	{
		return FAILED;
	}

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
						  nonce_i, nonce_r, this->old_sa, NULL, AUTH_NONE);
	return TRUE;
}

METHOD(task_t, build_r, status_t,
	private_ike_init_t *this, message_t *message)
{
	identification_t *gateway;

	/* check if we have everything we need */
	if (this->proposal == NULL ||
		this->other_nonce.len == 0 || this->my_nonce.len == 0)
	{
		DBG1(DBG_IKE, "received proposals unacceptable");
		message->add_notify(message, TRUE, NO_PROPOSAL_CHOSEN, chunk_empty);
		return FAILED;
	}

	/* check if we'd have to redirect the client */
	if (!this->old_sa &&
		this->ike_sa->supports_extension(this->ike_sa, EXT_IKE_REDIRECTION) &&
		charon->redirect->redirect_on_init(charon->redirect, this->ike_sa,
										   &gateway))
	{
		chunk_t data;

		DBG1(DBG_IKE, "redirecting peer to %Y", gateway);
		data = redirect_data_create(gateway, this->other_nonce);
		message->add_notify(message, TRUE, REDIRECT, data);
		gateway->destroy(gateway);
		chunk_free(&data);
		return FAILED;
	}

	if (this->dh == NULL ||
		!this->proposal->has_dh_group(this->proposal, this->dh_group))
	{
		uint16_t group;

		if (this->proposal->get_algorithm(this->proposal, DIFFIE_HELLMAN_GROUP,
										  &group, NULL))
		{
			DBG1(DBG_IKE, "DH group %N unacceptable, requesting %N",
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
			message->add_notify(message, TRUE, NO_PROPOSAL_CHOSEN, chunk_empty);
		}
		return FAILED;
	}

	if (this->dh_failed)
	{
		DBG1(DBG_IKE, "applying DH public value failed");
		message->add_notify(message, TRUE, NO_PROPOSAL_CHOSEN, chunk_empty);
		return FAILED;
	}

	if (!derive_keys(this, this->other_nonce, this->my_nonce))
	{
		DBG1(DBG_IKE, "key derivation failed");
		message->add_notify(message, TRUE, NO_PROPOSAL_CHOSEN, chunk_empty);
		return FAILED;
	}
	if (!build_payloads(this, message))
	{
		message->add_notify(message, TRUE, NO_PROPOSAL_CHOSEN, chunk_empty);
		return FAILED;
	}
	return SUCCESS;
}

/**
 * Raise alerts for received notify errors
 */
static void raise_alerts(private_ike_init_t *this, notify_type_t type)
{
	ike_cfg_t *ike_cfg;
	linked_list_t *list;

	switch (type)
	{
		case NO_PROPOSAL_CHOSEN:
			ike_cfg = this->ike_sa->get_ike_cfg(this->ike_sa);
			list = ike_cfg->get_proposals(ike_cfg);
			charon->bus->alert(charon->bus, ALERT_PROPOSAL_MISMATCH_IKE, list);
			list->destroy_offset(list, offsetof(proposal_t, destroy));
			break;
		default:
			break;
	}
}

METHOD(task_t, pre_process_i, status_t,
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
				case COOKIE:
				{
					chunk_t cookie;

					cookie = notify->get_notification_data(notify);
					if (chunk_equals(cookie, this->cookie))
					{
						DBG1(DBG_IKE, "ignore response with duplicate COOKIE "
							 "notify");
						enumerator->destroy(enumerator);
						return FAILED;
					}
					break;
				}
				case REDIRECT:
				{
					identification_t *gateway;
					chunk_t data, nonce = chunk_empty;
					status_t status = SUCCESS;

					if (this->old_sa)
					{
						break;
					}
					data = notify->get_notification_data(notify);
					gateway = redirect_data_parse(data, &nonce);
					if (!gateway || !chunk_equals(nonce, this->my_nonce))
					{
						DBG1(DBG_IKE, "received invalid REDIRECT notify");
						status = FAILED;
					}
					DESTROY_IF(gateway);
					chunk_free(&nonce);
					enumerator->destroy(enumerator);
					return status;
				}
				default:
					break;
			}
		}
	}
	enumerator->destroy(enumerator);
	return SUCCESS;
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
					this->dh_group = ntohs(*((uint16_t*)data.ptr));
					DBG1(DBG_IKE, "peer didn't accept DH group %N, "
						 "it requested %N", diffie_hellman_group_names,
						 bad_group, diffie_hellman_group_names, this->dh_group);

					if (this->old_sa == NULL)
					{	/* reset the IKE_SA if we are not rekeying */
						this->ike_sa->reset(this->ike_sa, FALSE);
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
					this->ike_sa->reset(this->ike_sa, FALSE);
					enumerator->destroy(enumerator);
					DBG2(DBG_IKE, "received %N notify", notify_type_names, type);
					this->retry++;
					return NEED_MORE;
				}
				case REDIRECT:
				{
					identification_t *gateway;
					chunk_t data, nonce = chunk_empty;
					status_t status = FAILED;

					if (this->old_sa)
					{
						DBG1(DBG_IKE, "received REDIRECT notify during rekeying"
						     ", ignored");
						break;
					}
					data = notify->get_notification_data(notify);
					gateway = redirect_data_parse(data, &nonce);
					if (this->ike_sa->handle_redirect(this->ike_sa, gateway))
					{
						status = NEED_MORE;
					}
					DESTROY_IF(gateway);
					chunk_free(&nonce);
					enumerator->destroy(enumerator);
					return status;
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

	if (this->dh == NULL ||
		!this->proposal->has_dh_group(this->proposal, this->dh_group))
	{
		DBG1(DBG_IKE, "peer DH group selection invalid");
		return FAILED;
	}

	if (this->dh_failed)
	{
		DBG1(DBG_IKE, "applying DH public value failed");
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
	this->dh_failed = FALSE;
}

METHOD(task_t, destroy, void,
	private_ike_init_t *this)
{
	DESTROY_IF(this->dh);
	DESTROY_IF(this->proposal);
	DESTROY_IF(this->nonceg);
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
		.signature_authentication = lib->settings->get_bool(lib->settings,
								"%s.signature_authentication", TRUE, lib->ns),
		.follow_redirects = lib->settings->get_bool(lib->settings,
								"%s.follow_redirects", TRUE, lib->ns),
	);
	this->nonceg = this->keymat->keymat.create_nonce_gen(&this->keymat->keymat);

	if (initiator)
	{
		this->public.task.build = _build_i;
		this->public.task.process = _process_i;
		this->public.task.pre_process = _pre_process_i;
	}
	else
	{
		this->public.task.build = _build_r;
		this->public.task.process = _process_r;
	}
	return &this->public;
}
