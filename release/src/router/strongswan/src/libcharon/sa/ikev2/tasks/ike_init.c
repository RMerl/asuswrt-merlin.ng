/*
 * Copyright (C) 2008-2020 Tobias Brunner
 * Copyright (C) 2005-2008 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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

#include "ike_init.h"

#include <string.h>

#include <daemon.h>
#include <bio/bio_reader.h>
#include <bio/bio_writer.h>
#include <sa/ikev2/keymat_v2.h>
#include <crypto/key_exchange.h>
#include <crypto/hashers/hash_algorithm_set.h>
#include <encoding/payloads/sa_payload.h>
#include <encoding/payloads/ke_payload.h>
#include <encoding/payloads/nonce_payload.h>

/** maximum retries to do with cookies/other ke methods */
#define MAX_RETRIES 5

/** maximum number of key exchanges (including the initial one) */
#define MAX_KEY_EXCHANGES (ADDITIONAL_KEY_EXCHANGE_7 - \
						   ADDITIONAL_KEY_EXCHANGE_1 + 2)

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
	 * Key exchanges to perform
	 */
	struct {
		transform_type_t type;
		key_exchange_method_t method;
		bool done;
		bool derived;
	} key_exchanges[MAX_KEY_EXCHANGES];

	/**
	 * Current key exchange
	 */
	int ke_index;

	/**
	 * Key exchange method from the parsed or sent KE payload
	 */
	key_exchange_method_t ke_method;

	/**
	 * Current key exchange object
	 */
	key_exchange_t *ke;

	/**
	 * All key exchanges performed during rekeying (key_exchange_t)
	 */
	array_t *kes;

	/**
	 * Applying KE public key failed?
	 */
	bool ke_failed;

	/**
	 * Keymat derivation (from IKE_SA)
	 */
	keymat_v2_t *keymat;

	/**
	 * Nonce chosen by us
	 */
	chunk_t my_nonce;

	/**
	 * Nonce chosen by peer
	 */
	chunk_t other_nonce;

	/**
	 * Nonce generator
	 */
	nonce_gen_t *nonceg;

	/**
	 * Negotiated proposal used for IKE_SA
	 */
	proposal_t *proposal;

	/**
	 * Old IKE_SA that gets rekeyed
	 */
	ike_sa_t *old_sa;

	/**
	 * Cookie received from responder
	 */
	chunk_t cookie;

	/**
	 * Retries done so far after failure (cookie or bad KE method)
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
 * Returns the exchange type for additional exchanges when using multiple key
 * exchanges, depending on whether this happens initially or during a rekeying
 */
static exchange_type_t exchange_type_multi_ke(private_ike_init_t *this)
{
	return this->old_sa ? IKE_FOLLOWUP_KE : IKE_INTERMEDIATE;
}

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
	linked_list_t *proposal_list, *other_ke_methods;
	ike_sa_id_t *id;
	proposal_t *proposal;
	enumerator_t *enumerator;
	ike_cfg_t *ike_cfg;
	bool additional_ke = FALSE;

	id = this->ike_sa->get_id(this->ike_sa);

	ike_cfg = this->ike_sa->get_ike_cfg(this->ike_sa);

	if (this->initiator)
	{
		proposal_list = ike_cfg->get_proposals(ike_cfg);
		other_ke_methods = linked_list_create();
		enumerator = proposal_list->create_enumerator(proposal_list);
		while (enumerator->enumerate(enumerator, (void**)&proposal))
		{
			/* include SPI of new IKE_SA when we are rekeying */
			if (this->old_sa)
			{
				proposal->set_spi(proposal, id->get_initiator_spi(id));
			}
			/* move the selected KE method to the front of the proposal */
			if (!proposal->promote_transform(proposal, KEY_EXCHANGE_METHOD,
											 this->ke_method))
			{	/* the proposal does not include the group, move to the back */
				proposal_list->remove_at(proposal_list, enumerator);
				other_ke_methods->insert_last(other_ke_methods, proposal);
			}
			additional_ke = additional_ke ||
							proposal_has_additional_ke(proposal);
		}
		enumerator->destroy(enumerator);
		/* add proposals that don't contain the selected group */
		enumerator = other_ke_methods->create_enumerator(other_ke_methods);
		while (enumerator->enumerate(enumerator, (void**)&proposal))
		{	/* no need to remove from the list as we destroy it anyway*/
			proposal_list->insert_last(proposal_list, proposal);
		}
		enumerator->destroy(enumerator);
		other_ke_methods->destroy(other_ke_methods);

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
		additional_ke = proposal_has_additional_ke(this->proposal);
	}
	message->add_payload(message, (payload_t*)sa_payload);

	ke_payload = ke_payload_create_from_key_exchange(PLV2_KEY_EXCHANGE,
													 this->ke);
	if (!ke_payload)
	{
		DBG1(DBG_IKE, "creating KE payload failed");
		return FALSE;
	}
	message->add_payload(message, (payload_t*)ke_payload);

	nonce_payload = nonce_payload_create(PLV2_NONCE);
	nonce_payload->set_nonce(nonce_payload, this->my_nonce);
	message->add_payload(message, (payload_t*)nonce_payload);

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
	/* notify the peer if we accept childless IKE_SAs */
	if (!this->old_sa && !this->initiator &&
		 ike_cfg->childless(ike_cfg) != CHILDLESS_NEVER)
	{
		message->add_notify(message, FALSE, CHILDLESS_IKEV2_SUPPORTED,
							chunk_empty);
	}
	if (!this->old_sa && additional_ke)
	{
		if (this->initiator ||
			this->ike_sa->supports_extension(this->ike_sa,
											 EXT_IKE_INTERMEDIATE))
		{
			message->add_notify(message, FALSE, INTERMEDIATE_EXCHANGE_SUPPORTED,
								chunk_empty);
		}
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
	proposal_selection_flag_t flags = 0;

	ike_cfg = this->ike_sa->get_ike_cfg(this->ike_sa);

	proposal_list = sa_payload->get_proposals(sa_payload);
	if (!this->ike_sa->supports_extension(this->ike_sa, EXT_STRONGSWAN) &&
		!lib->settings->get_bool(lib->settings, "%s.accept_private_algs",
								 FALSE, lib->ns))
	{
		flags |= PROPOSAL_SKIP_PRIVATE;
	}
	if (!lib->settings->get_bool(lib->settings,
							"%s.prefer_configured_proposals", TRUE, lib->ns))
	{
		flags |= PROPOSAL_PREFER_SUPPLIED;
	}
	this->proposal = ike_cfg->select_proposal(ike_cfg, proposal_list, flags);
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
													  flags);
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
 * Collect all key exchanges from the proposal
 */
static void determine_key_exchanges(private_ike_init_t *this)
{
	transform_type_t t = KEY_EXCHANGE_METHOD;
	uint16_t alg;
	int i = 1;

	this->proposal->get_algorithm(this->proposal, t, &alg, NULL);
	this->key_exchanges[0].type = t;
	this->key_exchanges[0].method = alg;

	for (t = ADDITIONAL_KEY_EXCHANGE_1; t <= ADDITIONAL_KEY_EXCHANGE_7; t++)
	{
		if (this->proposal->get_algorithm(this->proposal, t, &alg, NULL))
		{
			this->key_exchanges[i].type = t;
			this->key_exchanges[i].method = alg;
			i++;
		}
	}
}

/**
 * Check if additional key exchanges are required
 */
static bool additional_key_exchange_required(private_ike_init_t *this)
{
	int i;

	for (i = this->ke_index; i < MAX_KEY_EXCHANGES; i++)
	{
		if (this->key_exchanges[i].type && !this->key_exchanges[i].done)
		{
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * Clear data on key exchanges
 */
static void clear_key_exchanges(private_ike_init_t *this)
{
	int i;

	for (i = 0; i < MAX_KEY_EXCHANGES; i++)
	{
		this->key_exchanges[i].type = 0;
		this->key_exchanges[i].method = 0;
		this->key_exchanges[i].done = FALSE;
	}
	this->ke_index = 0;

	array_destroy_offset(this->kes, offsetof(key_exchange_t, destroy));
	this->kes = NULL;
}

/**
 * Process a KE payload
 */
static void process_ke_payload(private_ike_init_t *this, ke_payload_t *ke)
{
	key_exchange_method_t method = this->key_exchanges[this->ke_index].method;
	key_exchange_method_t received = ke->get_key_exchange_method(ke);

	if (method != received)
	{
		DBG1(DBG_IKE, "key exchange method in received payload %N doesn't "
			 "match negotiated %N", key_exchange_method_names, received,
			 key_exchange_method_names, method);
		this->ke_failed = TRUE;
		return;
	}

	if (!this->initiator)
	{
		DESTROY_IF(this->ke);
		this->ke = this->keymat->keymat.create_ke(&this->keymat->keymat,
												  method);
		if (!this->ke)
		{
			DBG1(DBG_IKE, "negotiated key exchange method %N not supported",
				 key_exchange_method_names, method);
		}
	}
	else if (this->ke)
	{
		this->ke_failed = this->ke->get_method(this->ke) != received;
	}

	if (this->ke && !this->ke_failed)
	{
		this->ke_failed = !this->ke->set_public_key(this->ke,
												ke->get_key_exchange_data(ke));
	}
}

/**
 * Read payloads from message
 */
static void process_payloads(private_ike_init_t *this, message_t *message)
{
	enumerator_t *enumerator;
	payload_t *payload;
	ike_sa_id_t *id;
	ke_payload_t *ke_pld = NULL;

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
				ke_pld = (ke_payload_t*)payload;

				this->ke_method = ke_pld->get_key_exchange_method(ke_pld);
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
					case CHILDLESS_IKEV2_SUPPORTED:
						if (this->initiator && !this->old_sa)
						{
							this->ike_sa->enable_extension(this->ike_sa,
														   EXT_IKE_CHILDLESS);
						}
						break;
					case INTERMEDIATE_EXCHANGE_SUPPORTED:
						if (!this->old_sa)
						{
							this->ike_sa->enable_extension(this->ike_sa,
														   EXT_IKE_INTERMEDIATE);
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

		if (this->old_sa)
		{	/* retrieve SPI of new IKE_SA when rekeying */
			id = this->ike_sa->get_id(this->ike_sa);
			if (this->initiator)
			{
				id->set_responder_spi(id,
									  this->proposal->get_spi(this->proposal));
			}
			else
			{
				id->set_initiator_spi(id,
									  this->proposal->get_spi(this->proposal));
			}
		}

		determine_key_exchanges(this);
		if (ke_pld)
		{
			process_ke_payload(this, ke_pld);
		}
	}
}

/**
 * Build payloads in additional exchanges when using multiple key exchanges
 */
static bool build_payloads_multi_ke(private_ike_init_t *this,
									message_t *message)
{
	ke_payload_t *ke;

	ke = ke_payload_create_from_key_exchange(PLV2_KEY_EXCHANGE, this->ke);
	if (!ke)
	{
		DBG1(DBG_IKE, "creating KE payload failed");
		return FALSE;
	}
	message->add_payload(message, (payload_t*)ke);
	return TRUE;
}

METHOD(task_t, build_i_multi_ke, status_t,
	private_ike_init_t *this, message_t *message)
{
	key_exchange_method_t method;

	message->set_exchange_type(message, exchange_type_multi_ke(this));

	DESTROY_IF(this->ke);
	method = this->key_exchanges[this->ke_index].method;
	this->ke = this->keymat->keymat.create_ke(&this->keymat->keymat,
											  method);
	if (!this->ke)
	{
		DBG1(DBG_IKE, "negotiated key exchange method %N not supported",
			 key_exchange_method_names, method);
		return FAILED;
	}
	if (!build_payloads_multi_ke(this, message))
	{
		return FAILED;
	}
	return NEED_MORE;
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
	if (!this->ke)
	{
		if (this->old_sa &&
			lib->settings->get_bool(lib->settings,
								"%s.prefer_previous_dh_group", TRUE, lib->ns))
		{	/* reuse the KE method we used for the old IKE_SA when rekeying */
			proposal_t *proposal;
			uint16_t ke_method;

			proposal = this->old_sa->get_proposal(this->old_sa);
			if (proposal->get_algorithm(proposal, KEY_EXCHANGE_METHOD,
										&ke_method, NULL))
			{
				this->ke_method = ke_method;
			}
			else
			{	/* this shouldn't happen, but let's be safe */
				this->ke_method = ike_cfg->get_algorithm(ike_cfg,
														 KEY_EXCHANGE_METHOD);
			}
		}
		else
		{
			this->ke_method = ike_cfg->get_algorithm(ike_cfg,
													 KEY_EXCHANGE_METHOD);
		}
		this->ke = this->keymat->keymat.create_ke(&this->keymat->keymat,
												  this->ke_method);
		if (!this->ke)
		{
			DBG1(DBG_IKE, "configured key exchange method %N not supported",
				 key_exchange_method_names, this->ke_method);
			return FAILED;
		}
	}
	else if (this->ke->get_method(this->ke) != this->ke_method)
	{	/* reset KE instance if method changed (INVALID_KE_PAYLOAD) */
		this->ke->destroy(this->ke);
		this->ke = this->keymat->keymat.create_ke(&this->keymat->keymat,
												  this->ke_method);
		if (!this->ke)
		{
			DBG1(DBG_IKE, "requested key exchange method %N not supported",
				 key_exchange_method_names, this->ke_method);
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

/**
 * Process payloads in additional exchanges when using multiple key exchanges
 */
static void process_payloads_multi_ke(private_ike_init_t *this,
									  message_t *message)
{
	ke_payload_t *ke;

	ke = (ke_payload_t*)message->get_payload(message, PLV2_KEY_EXCHANGE);
	if (ke)
	{
		process_ke_payload(this, ke);
	}
	else
	{
		DBG1(DBG_IKE, "KE payload missing in message");
	}
}

METHOD(task_t, process_r_multi_ke, status_t,
	private_ike_init_t *this, message_t *message)
{
	if (message->get_exchange_type(message) == exchange_type_multi_ke(this))
	{
		process_payloads_multi_ke(this, message);
	}
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
static bool derive_keys_internal(private_ike_init_t *this, chunk_t nonce_i,
								 chunk_t nonce_r)
{
	ike_sa_t *old_sa;
	keymat_v2_t *old_keymat;
	pseudo_random_function_t prf_alg = PRF_UNDEFINED;
	chunk_t skd = chunk_empty;
	ike_sa_id_t *id;
	array_t *kes = NULL;
	bool success;

	if (this->old_sa)
	{
		if (additional_key_exchange_required(this))
		{	/* when rekeying, we only derive keys once all exchanges are done */
			return FALSE;
		}
		old_sa = this->old_sa;
		kes = this->kes;
	}
	else
	{	/* key derivation for additional key exchanges is like rekeying, so pass
		 * our own SA as old SA to get SK_d */
		old_sa = this->ike_sa;
		array_insert_create(&kes, ARRAY_HEAD, this->ke);
	}

	id = this->ike_sa->get_id(this->ike_sa);
	old_keymat = (keymat_v2_t*)old_sa->get_keymat(old_sa);
	prf_alg = old_keymat->get_skd(old_keymat, &skd);
	success = this->keymat->derive_ike_keys(this->keymat, this->proposal, kes,
											nonce_i, nonce_r, id, prf_alg, skd);
	if (success)
	{
		charon->bus->ike_keys(charon->bus, this->ike_sa, kes, chunk_empty,
							  nonce_i, nonce_r, skd.len ? old_sa : NULL, NULL,
							  AUTH_NONE);
	}
	if (kes != this->kes)
	{
		array_destroy(kes);
	}
	return success;
}

METHOD(ike_init_t, derive_keys, status_t,
	private_ike_init_t *this)
{
	bool success;

	if (!this->ke_index || this->key_exchanges[this->ke_index-1].derived)
	{
		return NEED_MORE;
	}

	if (this->initiator)
	{
		success = derive_keys_internal(this, this->my_nonce, this->other_nonce);
	}
	else
	{
		success = derive_keys_internal(this, this->other_nonce, this->my_nonce);
	}

	this->key_exchanges[this->ke_index-1].derived = TRUE;

	if (!success)
	{
		DBG1(DBG_IKE, "key derivation failed");
		return FAILED;
	}
	return additional_key_exchange_required(this) ? NEED_MORE : SUCCESS;
}

/**
 * Called when a key exchange is done
 */
static status_t key_exchange_done(private_ike_init_t *this)
{
	if (this->old_sa)
	{
		/* during rekeying, we store all the key exchanges performed */
		array_insert_create(&this->kes, ARRAY_TAIL, this->ke);
		this->ke = NULL;
	}

	this->key_exchanges[this->ke_index++].done = TRUE;

	return additional_key_exchange_required(this) ? NEED_MORE : SUCCESS;
}

METHOD(task_t, build_r_multi_ke, status_t,
	private_ike_init_t *this, message_t *message)
{
	if (!this->ke)
	{
		message->add_notify(message, FALSE, INVALID_SYNTAX, chunk_empty);
		return FAILED;
	}
	if (this->ke_failed)
	{
		message->add_notify(message, FALSE, NO_PROPOSAL_CHOSEN, chunk_empty);
		return FAILED;
	}
	if (!build_payloads_multi_ke(this, message))
	{
		return FAILED;
	}

	if (key_exchange_done(this) != NEED_MORE && this->old_sa)
	{
		/* during rekeying, we derive keys once all exchanges are done */
		if (derive_keys(this) != SUCCESS)
		{
			message->add_notify(message, FALSE, NO_PROPOSAL_CHOSEN, chunk_empty);
			return FAILED;
		}
		return SUCCESS;
	}
	/* when not rekeying, we derive keys after each IKE_INTERMEDIATE but only
	 * once we receive the next message, so IntAuth is based on the right keys */
	return NEED_MORE;
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

	if (!this->ke ||
		!this->proposal->has_transform(this->proposal, KEY_EXCHANGE_METHOD,
									   this->ke_method))
	{
		uint16_t group;

		if (this->proposal->get_algorithm(this->proposal, KEY_EXCHANGE_METHOD,
										  &group, NULL) &&
			this->ke_method != group)
		{
			DBG1(DBG_IKE, "key exchange method %N unacceptable, requesting %N",
				 key_exchange_method_names, this->ke_method,
				 key_exchange_method_names, group);
			this->ke_method = group;
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

	if (this->ke_failed)
	{
		DBG1(DBG_IKE, "applying KE public value failed");
		message->add_notify(message, TRUE, NO_PROPOSAL_CHOSEN, chunk_empty);
		return FAILED;
	}

	if (!build_payloads(this, message))
	{
		message->add_notify(message, TRUE, NO_PROPOSAL_CHOSEN, chunk_empty);
		return FAILED;
	}

	if (key_exchange_done(this) == NEED_MORE)
	{
		if (!this->old_sa &&
			!this->ike_sa->supports_extension(this->ike_sa, EXT_IKE_INTERMEDIATE))
		{
			DBG1(DBG_IKE, "peer didn't send %N while proposing multiple key "
				 "exchanges", notify_type_names, INTERMEDIATE_EXCHANGE_SUPPORTED);
			message->add_notify(message, TRUE, NO_PROPOSAL_CHOSEN, chunk_empty);
			return FAILED;
		}
		/* use other exchange type for additional key exchanges */
		this->public.task.build = _build_r_multi_ke;
		this->public.task.process = _process_r_multi_ke;
	}
	else if (this->old_sa)
	{
		/* during rekeying, we derive keys here directly */
		if (derive_keys(this) != SUCCESS)
		{
			message->add_notify(message, FALSE, NO_PROPOSAL_CHOSEN, chunk_empty);
			return FAILED;
		}
		return SUCCESS;
	}
	/* key derivation is done before the next request is processed */
	return NEED_MORE;
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

METHOD(task_t, process_i_multi_ke, status_t,
	private_ike_init_t *this, message_t *message)
{
	process_payloads_multi_ke(this, message);

	if (this->ke_failed)
	{
		return FAILED;
	}

	if (key_exchange_done(this) != NEED_MORE && this->old_sa)
	{
		/* during rekeying, we derive keys once all exchanges are done */
		return derive_keys(this);
	}
	/* when not rekeying, we derive keys after each IKE_INTERMEDIATE but only
	 * once we send the next message, so IntAuth is based on the right keys */
	return NEED_MORE;
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
					key_exchange_method_t bad_method DBG_UNUSED;

					bad_method = this->ke_method;
					data = notify->get_notification_data(notify);
					this->ke_method = ntohs(*((uint16_t*)data.ptr));
					DBG1(DBG_IKE, "peer didn't accept key exchange method %N, "
						 "it requested %N", key_exchange_method_names,
						 bad_method, key_exchange_method_names, this->ke_method);

					if (!this->old_sa)
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
					if (this->old_sa)
					{
						DBG1(DBG_IKE, "received COOKIE notify during rekeying"
						     ", ignored");
						break;
					}
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
	if (!this->proposal ||
		this->other_nonce.len == 0 || this->my_nonce.len == 0)
	{
		DBG1(DBG_IKE, "peer's proposal selection invalid");
		return FAILED;
	}

	if (!this->proposal->has_transform(this->proposal, KEY_EXCHANGE_METHOD,
									   this->ke_method))
	{
		DBG1(DBG_IKE, "peer's key exchange method selection invalid");
		return FAILED;
	}

	if (this->ke_failed)
	{
		DBG1(DBG_IKE, "applying key exchange public value failed");
		return FAILED;
	}

	if (key_exchange_done(this) == NEED_MORE)
	{
		if (!this->old_sa &&
			!this->ike_sa->supports_extension(this->ike_sa, EXT_IKE_INTERMEDIATE))
		{
			DBG1(DBG_IKE, "peer didn't send %N while accepting multiple key "
				 "exchanges", notify_type_names, INTERMEDIATE_EXCHANGE_SUPPORTED);
			return FAILED;
		}
		/* use other exchange type for additional key exchanges */
		this->public.task.build = _build_i_multi_ke;
		this->public.task.process = _process_i_multi_ke;
	}
	else if (this->old_sa)
	{
		/* during rekeying, we derive keys here directly */
		return derive_keys(this);
	}
	/* key derivation is done before we send the next message */
	return NEED_MORE;
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
	clear_key_exchanges(this);

	this->ike_sa = ike_sa;
	this->keymat = (keymat_v2_t*)ike_sa->get_keymat(ike_sa);
	this->proposal = NULL;
	this->ke_failed = FALSE;
	this->public.task.build = _build_i;
	this->public.task.process = _process_i;
}

METHOD(task_t, destroy, void,
	private_ike_init_t *this)
{
	DESTROY_IF(this->ke);
	DESTROY_IF(this->proposal);
	DESTROY_IF(this->nonceg);
	chunk_free(&this->my_nonce);
	chunk_free(&this->other_nonce);
	chunk_free(&this->cookie);
	clear_key_exchanges(this);
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
			.derive_keys = _derive_keys,
			.get_lower_nonce = _get_lower_nonce,
		},
		.ike_sa = ike_sa,
		.initiator = initiator,
		.ke_method = KE_NONE,
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
