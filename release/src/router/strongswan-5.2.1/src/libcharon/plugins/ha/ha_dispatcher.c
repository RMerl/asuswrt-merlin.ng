/*
 * Copyright (C) 2008 Martin Willi
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

#include "ha_dispatcher.h"

#include <daemon.h>
#include <sa/ikev2/keymat_v2.h>
#include <sa/ikev1/keymat_v1.h>
#include <processing/jobs/callback_job.h>
#include <processing/jobs/adopt_children_job.h>

typedef struct private_ha_dispatcher_t private_ha_dispatcher_t;
typedef struct ha_diffie_hellman_t ha_diffie_hellman_t;

/**
 * Private data of an ha_dispatcher_t object.
 */
struct private_ha_dispatcher_t {

	/**
	 * Public ha_dispatcher_t interface.
	 */
	ha_dispatcher_t public;

	/**
	 * socket to pull messages from
	 */
	ha_socket_t *socket;

	/**
	 * segments to control
	 */
	ha_segments_t *segments;

	/**
	 * Cache for resync
	 */
	ha_cache_t *cache;

	/**
	 * Kernel helper
	 */
	ha_kernel_t *kernel;

	/**
	 * HA enabled pool
	 */
	ha_attribute_t *attr;
};

/**
 * DH implementation for HA synced DH values
 */
struct ha_diffie_hellman_t {

	/**
	 * Implements diffie_hellman_t
	 */
	diffie_hellman_t dh;

	/**
	 * Shared secret
	 */
	chunk_t secret;

	/**
	 * Own public value
	 */
	chunk_t pub;
};

METHOD(diffie_hellman_t, dh_get_shared_secret, status_t,
	ha_diffie_hellman_t *this, chunk_t *secret)
{
	*secret = chunk_clone(this->secret);
	return SUCCESS;
}

METHOD(diffie_hellman_t, dh_get_my_public_value, void,
	ha_diffie_hellman_t *this, chunk_t *value)
{
	*value = chunk_clone(this->pub);
}

METHOD(diffie_hellman_t, dh_destroy, void,
	ha_diffie_hellman_t *this)
{
	free(this);
}

/**
 * Create a HA synced DH implementation
 */
static diffie_hellman_t *ha_diffie_hellman_create(chunk_t secret, chunk_t pub)
{
	ha_diffie_hellman_t *this;

	INIT(this,
		.dh = {
			.get_shared_secret = _dh_get_shared_secret,
			.get_my_public_value = _dh_get_my_public_value,
			.destroy = _dh_destroy,
		},
		.secret = secret,
		.pub = pub,
	);

	return &this->dh;
}

/**
 * Process messages of type IKE_ADD
 */
static void process_ike_add(private_ha_dispatcher_t *this, ha_message_t *message)
{
	ha_message_attribute_t attribute;
	ha_message_value_t value;
	enumerator_t *enumerator;
	ike_sa_t *ike_sa = NULL, *old_sa = NULL;
	ike_version_t version = IKEV2;
	u_int16_t encr = 0, len = 0, integ = 0, prf = 0, old_prf = PRF_UNDEFINED;
	chunk_t nonce_i = chunk_empty, nonce_r = chunk_empty;
	chunk_t secret = chunk_empty, old_skd = chunk_empty;
	chunk_t dh_local = chunk_empty, dh_remote = chunk_empty, psk = chunk_empty;
	bool ok = FALSE;

	enumerator = message->create_attribute_enumerator(message);
	while (enumerator->enumerate(enumerator, &attribute, &value))
	{
		switch (attribute)
		{
			case HA_IKE_ID:
				ike_sa = ike_sa_create(value.ike_sa_id,
						value.ike_sa_id->is_initiator(value.ike_sa_id), version);
				break;
			case HA_IKE_REKEY_ID:
				old_sa = charon->ike_sa_manager->checkout(charon->ike_sa_manager,
														  value.ike_sa_id);
				break;
			case HA_IKE_VERSION:
				version = value.u8;
				break;
			case HA_NONCE_I:
				nonce_i = value.chunk;
				break;
			case HA_NONCE_R:
				nonce_r = value.chunk;
				break;
			case HA_SECRET:
				secret = value.chunk;
				break;
			case HA_LOCAL_DH:
				dh_local = value.chunk;
				break;
			case HA_REMOTE_DH:
				dh_remote = value.chunk;
				break;
			case HA_PSK:
				psk = value.chunk;
				break;
			case HA_OLD_SKD:
				old_skd = value.chunk;
				break;
			case HA_ALG_ENCR:
				encr = value.u16;
				break;
			case HA_ALG_ENCR_LEN:
				len = value.u16;
				break;
			case HA_ALG_INTEG:
				integ = value.u16;
				break;
			case HA_ALG_PRF:
				prf = value.u16;
				break;
			case HA_ALG_OLD_PRF:
				old_prf = value.u16;
				break;
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);

	if (ike_sa)
	{
		proposal_t *proposal;
		diffie_hellman_t *dh;

		proposal = proposal_create(PROTO_IKE, 0);
		if (integ)
		{
			proposal->add_algorithm(proposal, INTEGRITY_ALGORITHM, integ, 0);
		}
		if (encr)
		{
			proposal->add_algorithm(proposal, ENCRYPTION_ALGORITHM, encr, len);
		}
		if (prf)
		{
			proposal->add_algorithm(proposal, PSEUDO_RANDOM_FUNCTION, prf, 0);
		}
		charon->bus->set_sa(charon->bus, ike_sa);
		dh = ha_diffie_hellman_create(secret, dh_local);
		if (ike_sa->get_version(ike_sa) == IKEV2)
		{
			keymat_v2_t *keymat_v2 = (keymat_v2_t*)ike_sa->get_keymat(ike_sa);

			ok = keymat_v2->derive_ike_keys(keymat_v2, proposal, dh, nonce_i,
							nonce_r, ike_sa->get_id(ike_sa), old_prf, old_skd);
		}
		if (ike_sa->get_version(ike_sa) == IKEV1)
		{
			keymat_v1_t *keymat_v1 = (keymat_v1_t*)ike_sa->get_keymat(ike_sa);
			shared_key_t *shared = NULL;
			auth_method_t method = AUTH_RSA;

			if (psk.len)
			{
				method = AUTH_PSK;
				shared = shared_key_create(SHARED_IKE, chunk_clone(psk));
			}
			if (keymat_v1->create_hasher(keymat_v1, proposal))
			{
				ok = keymat_v1->derive_ike_keys(keymat_v1, proposal,
								dh, dh_remote, nonce_i, nonce_r,
								ike_sa->get_id(ike_sa), method, shared);
			}
			DESTROY_IF(shared);
		}
		dh->destroy(dh);
		if (ok)
		{
			if (old_sa)
			{
				ike_sa->inherit_pre(ike_sa, old_sa);
				ike_sa->inherit_post(ike_sa, old_sa);
				charon->ike_sa_manager->checkin_and_destroy(
												charon->ike_sa_manager, old_sa);
				old_sa = NULL;
			}
			ike_sa->set_state(ike_sa, IKE_CONNECTING);
			ike_sa->set_proposal(ike_sa, proposal);
			this->cache->cache(this->cache, ike_sa, message);
			message = NULL;
			charon->ike_sa_manager->checkin(charon->ike_sa_manager, ike_sa);
		}
		else
		{
			DBG1(DBG_IKE, "HA keymat derivation failed");
			ike_sa->destroy(ike_sa);
		}
		charon->bus->set_sa(charon->bus, NULL);
		proposal->destroy(proposal);
	}
	if (old_sa)
	{
		charon->ike_sa_manager->checkin(charon->ike_sa_manager, old_sa);
	}
	DESTROY_IF(message);
}

/**
 * Apply a condition flag to the IKE_SA if it is in set
 */
static void set_condition(ike_sa_t *ike_sa, ike_condition_t set,
						  ike_condition_t flag)
{
	ike_sa->set_condition(ike_sa, flag, flag & set);
}

/**
 * Apply a extension flag to the IKE_SA if it is in set
 */
static void set_extension(ike_sa_t *ike_sa, ike_extension_t set,
						  ike_extension_t flag)
{
	if (flag & set)
	{
		ike_sa->enable_extension(ike_sa, flag);
	}
}

/**
 * Process messages of type IKE_UPDATE
 */
static void process_ike_update(private_ha_dispatcher_t *this,
							   ha_message_t *message)
{
	ha_message_attribute_t attribute;
	ha_message_value_t value;
	enumerator_t *enumerator;
	ike_sa_t *ike_sa = NULL;
	peer_cfg_t *peer_cfg = NULL;
	auth_cfg_t *auth;
	bool received_vip = FALSE, first_local_vip = TRUE, first_peer_addr = TRUE;

	enumerator = message->create_attribute_enumerator(message);
	while (enumerator->enumerate(enumerator, &attribute, &value))
	{
		if (attribute != HA_IKE_ID && ike_sa == NULL)
		{
			/* must be first attribute */
			break;
		}
		switch (attribute)
		{
			case HA_IKE_ID:
				ike_sa = charon->ike_sa_manager->checkout(charon->ike_sa_manager,
														  value.ike_sa_id);
				break;
			case HA_LOCAL_ID:
				ike_sa->set_my_id(ike_sa, value.id->clone(value.id));
				break;
			case HA_REMOTE_ID:
				ike_sa->set_other_id(ike_sa, value.id->clone(value.id));
				break;
			case HA_REMOTE_EAP_ID:
				auth = auth_cfg_create();
				auth->add(auth, AUTH_RULE_EAP_IDENTITY, value.id->clone(value.id));
				ike_sa->add_auth_cfg(ike_sa, FALSE, auth);
				break;
			case HA_LOCAL_ADDR:
				ike_sa->set_my_host(ike_sa, value.host->clone(value.host));
				break;
			case HA_REMOTE_ADDR:
				ike_sa->set_other_host(ike_sa, value.host->clone(value.host));
				break;
			case HA_LOCAL_VIP:
				if (first_local_vip)
				{
					ike_sa->clear_virtual_ips(ike_sa, TRUE);
					first_local_vip = FALSE;
				}
				ike_sa->add_virtual_ip(ike_sa, TRUE, value.host);
				break;
			case HA_REMOTE_VIP:
				if (!received_vip)
				{
					ike_sa->clear_virtual_ips(ike_sa, FALSE);
				}
				ike_sa->add_virtual_ip(ike_sa, FALSE, value.host);
				received_vip = TRUE;
				break;
			case HA_PEER_ADDR:
				if (first_peer_addr)
				{
					ike_sa->clear_peer_addresses(ike_sa);
					first_peer_addr = FALSE;
				}
				ike_sa->add_peer_address(ike_sa, value.host->clone(value.host));
				break;
			case HA_CONFIG_NAME:
				peer_cfg = charon->backends->get_peer_cfg_by_name(
												charon->backends, value.str);
				if (peer_cfg)
				{
					ike_sa->set_peer_cfg(ike_sa, peer_cfg);
					peer_cfg->destroy(peer_cfg);
				}
				else
				{
					DBG1(DBG_IKE, "HA is missing nodes peer configuration");
				}
				break;
			case HA_EXTENSIONS:
				set_extension(ike_sa, value.u32, EXT_NATT);
				set_extension(ike_sa, value.u32, EXT_MOBIKE);
				set_extension(ike_sa, value.u32, EXT_HASH_AND_URL);
				set_extension(ike_sa, value.u32, EXT_MULTIPLE_AUTH);
				set_extension(ike_sa, value.u32, EXT_STRONGSWAN);
				set_extension(ike_sa, value.u32, EXT_EAP_ONLY_AUTHENTICATION);
				set_extension(ike_sa, value.u32, EXT_MS_WINDOWS);
				set_extension(ike_sa, value.u32, EXT_XAUTH);
				set_extension(ike_sa, value.u32, EXT_DPD);
				break;
			case HA_CONDITIONS:
				set_condition(ike_sa, value.u32, COND_NAT_ANY);
				set_condition(ike_sa, value.u32, COND_NAT_HERE);
				set_condition(ike_sa, value.u32, COND_NAT_THERE);
				set_condition(ike_sa, value.u32, COND_NAT_FAKE);
				set_condition(ike_sa, value.u32, COND_EAP_AUTHENTICATED);
				set_condition(ike_sa, value.u32, COND_CERTREQ_SEEN);
				set_condition(ike_sa, value.u32, COND_ORIGINAL_INITIATOR);
				set_condition(ike_sa, value.u32, COND_STALE);
				set_condition(ike_sa, value.u32, COND_INIT_CONTACT_SEEN);
				set_condition(ike_sa, value.u32, COND_XAUTH_AUTHENTICATED);
				break;
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);

	if (ike_sa)
	{
		if (ike_sa->get_state(ike_sa) == IKE_CONNECTING &&
			ike_sa->get_peer_cfg(ike_sa))
		{
			DBG1(DBG_CFG, "installed HA passive IKE_SA '%s' %H[%Y]...%H[%Y]",
				 ike_sa->get_name(ike_sa),
				 ike_sa->get_my_host(ike_sa), ike_sa->get_my_id(ike_sa),
				 ike_sa->get_other_host(ike_sa), ike_sa->get_other_id(ike_sa));
			ike_sa->set_state(ike_sa, IKE_PASSIVE);
		}
		if (received_vip)
		{
			enumerator_t *pools, *vips;
			host_t *vip;
			char *pool;

			peer_cfg = ike_sa->get_peer_cfg(ike_sa);
			if (peer_cfg)
			{
				pools = peer_cfg->create_pool_enumerator(peer_cfg);
				while (pools->enumerate(pools, &pool))
				{
					vips = ike_sa->create_virtual_ip_enumerator(ike_sa, FALSE);
					while (vips->enumerate(vips, &vip))
					{
						this->attr->reserve(this->attr, pool, vip);
					}
					vips->destroy(vips);
				}
				pools->destroy(pools);
			}
		}
#ifdef USE_IKEV1
		if (ike_sa->get_version(ike_sa) == IKEV1)
		{
			lib->processor->queue_job(lib->processor, (job_t*)
							adopt_children_job_create(ike_sa->get_id(ike_sa)));
		}
#endif /* USE_IKEV1 */
		this->cache->cache(this->cache, ike_sa, message);
		charon->ike_sa_manager->checkin(charon->ike_sa_manager, ike_sa);
	}
	else
	{
		DBG1(DBG_CFG, "passive HA IKE_SA to update not found");
		message->destroy(message);
	}
}

/**
 * Process messages of type IKE_MID_INITIATOR/RESPONDER
 */
static void process_ike_mid(private_ha_dispatcher_t *this,
							   ha_message_t *message, bool initiator)
{
	ha_message_attribute_t attribute;
	ha_message_value_t value;
	enumerator_t *enumerator;
	ike_sa_t *ike_sa = NULL;
	u_int32_t mid = 0;

	enumerator = message->create_attribute_enumerator(message);
	while (enumerator->enumerate(enumerator, &attribute, &value))
	{
		switch (attribute)
		{
			case HA_IKE_ID:
				ike_sa = charon->ike_sa_manager->checkout(charon->ike_sa_manager,
														  value.ike_sa_id);
				break;
			case HA_MID:
				mid = value.u32;
				break;
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);

	if (ike_sa)
	{
		if (mid)
		{
			ike_sa->set_message_id(ike_sa, initiator, mid);
		}
		this->cache->cache(this->cache, ike_sa, message);
		charon->ike_sa_manager->checkin(charon->ike_sa_manager, ike_sa);
	}
	else
	{
		message->destroy(message);
	}
}

/**
 * Process messages of type IKE_IV
 */
static void process_ike_iv(private_ha_dispatcher_t *this, ha_message_t *message)
{
	ha_message_attribute_t attribute;
	ha_message_value_t value;
	enumerator_t *enumerator;
	ike_sa_t *ike_sa = NULL;
	chunk_t iv = chunk_empty;

	enumerator = message->create_attribute_enumerator(message);
	while (enumerator->enumerate(enumerator, &attribute, &value))
	{
		switch (attribute)
		{
			case HA_IKE_ID:
				ike_sa = charon->ike_sa_manager->checkout(charon->ike_sa_manager,
														  value.ike_sa_id);
				break;
			case HA_IV:
				iv = value.chunk;
				break;
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);

	if (ike_sa)
	{
		if (ike_sa->get_version(ike_sa) == IKEV1)
		{
			if (iv.len)
			{
				keymat_v1_t *keymat;

				keymat = (keymat_v1_t*)ike_sa->get_keymat(ike_sa);
				if (keymat->update_iv(keymat, 0, iv))
				{
					keymat->confirm_iv(keymat, 0);
				}
			}
		}
		this->cache->cache(this->cache, ike_sa, message);
		charon->ike_sa_manager->checkin(charon->ike_sa_manager, ike_sa);
	}
	else
	{
		message->destroy(message);
	}
}

/**
 * Process messages of type IKE_DELETE
 */
static void process_ike_delete(private_ha_dispatcher_t *this,
							   ha_message_t *message)
{
	ha_message_attribute_t attribute;
	ha_message_value_t value;
	enumerator_t *enumerator;
	ike_sa_t *ike_sa = NULL;

	enumerator = message->create_attribute_enumerator(message);
	while (enumerator->enumerate(enumerator, &attribute, &value))
	{
		switch (attribute)
		{
			case HA_IKE_ID:
				ike_sa = charon->ike_sa_manager->checkout(
									charon->ike_sa_manager, value.ike_sa_id);
				break;
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);
	if (ike_sa)
	{
		this->cache->cache(this->cache, ike_sa, message);
		charon->ike_sa_manager->checkin_and_destroy(
						charon->ike_sa_manager, ike_sa);
	}
	else
	{
		message->destroy(message);
	}
}

/**
 * Lookup a child cfg from the peer cfg by name
 */
static child_cfg_t* find_child_cfg(ike_sa_t *ike_sa, char *name)
{
	peer_cfg_t *peer_cfg;
	child_cfg_t *current, *found = NULL;
	enumerator_t *enumerator;

	peer_cfg = ike_sa->get_peer_cfg(ike_sa);
	if (peer_cfg)
	{
		enumerator = peer_cfg->create_child_cfg_enumerator(peer_cfg);
		while (enumerator->enumerate(enumerator, &current))
		{
			if (streq(current->get_name(current), name))
			{
				found = current;
				break;
			}
		}
		enumerator->destroy(enumerator);
	}
	return found;
}

/**
 * Process messages of type CHILD_ADD
 */
static void process_child_add(private_ha_dispatcher_t *this,
							  ha_message_t *message)
{
	ha_message_attribute_t attribute;
	ha_message_value_t value;
	enumerator_t *enumerator;
	ike_sa_t *ike_sa = NULL;
	char *config_name = "";
	child_cfg_t *config = NULL;
	child_sa_t *child_sa;
	proposal_t *proposal;
	bool initiator = FALSE, failed = FALSE, ok = FALSE;
	u_int32_t inbound_spi = 0, outbound_spi = 0;
	u_int16_t inbound_cpi = 0, outbound_cpi = 0;
	u_int8_t mode = MODE_TUNNEL, ipcomp = 0;
	u_int16_t encr = ENCR_UNDEFINED, integ = AUTH_UNDEFINED, len = 0;
	u_int16_t esn = NO_EXT_SEQ_NUMBERS;
	u_int seg_i, seg_o;
	chunk_t nonce_i = chunk_empty, nonce_r = chunk_empty, secret = chunk_empty;
	chunk_t encr_i, integ_i, encr_r, integ_r;
	linked_list_t *local_ts, *remote_ts;
	diffie_hellman_t *dh = NULL;

	enumerator = message->create_attribute_enumerator(message);
	while (enumerator->enumerate(enumerator, &attribute, &value))
	{
		switch (attribute)
		{
			case HA_IKE_ID:
				ike_sa = charon->ike_sa_manager->checkout(charon->ike_sa_manager,
														  value.ike_sa_id);
				break;
			case HA_CONFIG_NAME:
				config_name = value.str;
				break;
			case HA_INITIATOR:
				initiator = value.u8;
				break;
			case HA_INBOUND_SPI:
				inbound_spi = value.u32;
				break;
			case HA_OUTBOUND_SPI:
				outbound_spi = value.u32;
				break;
			case HA_INBOUND_CPI:
				inbound_cpi = value.u32;
				break;
			case HA_OUTBOUND_CPI:
				outbound_cpi = value.u32;
				break;
			case HA_IPSEC_MODE:
				mode = value.u8;
				break;
			case HA_IPCOMP:
				ipcomp = value.u8;
				break;
			case HA_ALG_ENCR:
				encr = value.u16;
				break;
			case HA_ALG_ENCR_LEN:
				len = value.u16;
				break;
			case HA_ALG_INTEG:
				integ = value.u16;
				break;
			case HA_ESN:
				esn = value.u16;
				break;
			case HA_NONCE_I:
				nonce_i = value.chunk;
				break;
			case HA_NONCE_R:
				nonce_r = value.chunk;
				break;
			case HA_SECRET:
				secret = value.chunk;
				break;
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);

	if (!ike_sa)
	{
		DBG1(DBG_CHD, "IKE_SA for HA CHILD_SA not found");
		message->destroy(message);
		return;
	}
	config = find_child_cfg(ike_sa, config_name);
	if (!config)
	{
		DBG1(DBG_CHD, "HA is missing nodes child configuration");
		charon->ike_sa_manager->checkin(charon->ike_sa_manager, ike_sa);
		message->destroy(message);
		return;
	}

	child_sa = child_sa_create(ike_sa->get_my_host(ike_sa),
							   ike_sa->get_other_host(ike_sa), config, 0,
							   ike_sa->has_condition(ike_sa, COND_NAT_ANY));
	child_sa->set_mode(child_sa, mode);
	child_sa->set_protocol(child_sa, PROTO_ESP);
	child_sa->set_ipcomp(child_sa, ipcomp);

	proposal = proposal_create(PROTO_ESP, 0);
	if (integ)
	{
		proposal->add_algorithm(proposal, INTEGRITY_ALGORITHM, integ, 0);
	}
	if (encr)
	{
		proposal->add_algorithm(proposal, ENCRYPTION_ALGORITHM, encr, len);
	}
	proposal->add_algorithm(proposal, EXTENDED_SEQUENCE_NUMBERS, esn, 0);
	if (secret.len)
	{
		dh = ha_diffie_hellman_create(secret, chunk_empty);
	}
	if (ike_sa->get_version(ike_sa) == IKEV2)
	{
		keymat_v2_t *keymat_v2 = (keymat_v2_t*)ike_sa->get_keymat(ike_sa);

		ok = keymat_v2->derive_child_keys(keymat_v2, proposal, dh,
						nonce_i, nonce_r, &encr_i, &integ_i, &encr_r, &integ_r);
	}
	if (ike_sa->get_version(ike_sa) == IKEV1)
	{
		keymat_v1_t *keymat_v1 = (keymat_v1_t*)ike_sa->get_keymat(ike_sa);
		u_int32_t spi_i, spi_r;

		spi_i = initiator ? inbound_spi : outbound_spi;
		spi_r = initiator ? outbound_spi : inbound_spi;

		ok = keymat_v1->derive_child_keys(keymat_v1, proposal, dh, spi_i, spi_r,
						nonce_i, nonce_r, &encr_i, &integ_i, &encr_r, &integ_r);
	}
	DESTROY_IF(dh);
	if (!ok)
	{
		DBG1(DBG_CHD, "HA CHILD_SA key derivation failed");
		child_sa->destroy(child_sa);
		proposal->destroy(proposal);
		charon->ike_sa_manager->checkin(charon->ike_sa_manager, ike_sa);
		return;
	}
	child_sa->set_proposal(child_sa, proposal);
	child_sa->set_state(child_sa, CHILD_INSTALLING);
	proposal->destroy(proposal);

	/* TODO: Change CHILD_SA API to avoid cloning twice */
	local_ts = linked_list_create();
	remote_ts = linked_list_create();
	enumerator = message->create_attribute_enumerator(message);
	while (enumerator->enumerate(enumerator, &attribute, &value))
	{
		switch (attribute)
		{
			case HA_LOCAL_TS:
				local_ts->insert_last(local_ts, value.ts->clone(value.ts));
				break;
			case HA_REMOTE_TS:
				remote_ts->insert_last(remote_ts, value.ts->clone(value.ts));
				break;
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);

	if (initiator)
	{
		if (child_sa->install(child_sa, encr_r, integ_r, inbound_spi,
							  inbound_cpi, initiator, TRUE, TRUE,
							  local_ts, remote_ts) != SUCCESS ||
			child_sa->install(child_sa, encr_i, integ_i, outbound_spi,
							  outbound_cpi, initiator, FALSE, TRUE,
							  local_ts, remote_ts) != SUCCESS)
		{
			failed = TRUE;
		}
	}
	else
	{
		if (child_sa->install(child_sa, encr_i, integ_i, inbound_spi,
							  inbound_cpi, initiator, TRUE, TRUE,
							  local_ts, remote_ts) != SUCCESS ||
			child_sa->install(child_sa, encr_r, integ_r, outbound_spi,
							  outbound_cpi, initiator, FALSE, TRUE,
							  local_ts, remote_ts) != SUCCESS)
		{
			failed = TRUE;
		}
	}
	chunk_clear(&encr_i);
	chunk_clear(&integ_i);
	chunk_clear(&encr_r);
	chunk_clear(&integ_r);

	if (failed)
	{
		DBG1(DBG_CHD, "HA CHILD_SA installation failed");
		child_sa->destroy(child_sa);
		local_ts->destroy_offset(local_ts, offsetof(traffic_selector_t, destroy));
		remote_ts->destroy_offset(remote_ts, offsetof(traffic_selector_t, destroy));
		charon->ike_sa_manager->checkin(charon->ike_sa_manager, ike_sa);
		message->destroy(message);
		return;
	}

	seg_i = this->kernel->get_segment_spi(this->kernel,
								ike_sa->get_my_host(ike_sa), inbound_spi);
	seg_o = this->kernel->get_segment_spi(this->kernel,
								ike_sa->get_other_host(ike_sa), outbound_spi);

	DBG1(DBG_CFG, "installed HA CHILD_SA %s{%d} %#R=== %#R "
		"(segment in: %d%s, out: %d%s)", child_sa->get_name(child_sa),
		child_sa->get_reqid(child_sa), local_ts, remote_ts,
		seg_i, this->segments->is_active(this->segments, seg_i) ? "*" : "",
		seg_o, this->segments->is_active(this->segments, seg_o) ? "*" : "");
	child_sa->add_policies(child_sa, local_ts, remote_ts);
	local_ts->destroy_offset(local_ts, offsetof(traffic_selector_t, destroy));
	remote_ts->destroy_offset(remote_ts, offsetof(traffic_selector_t, destroy));

	child_sa->set_state(child_sa, CHILD_INSTALLED);
	ike_sa->add_child_sa(ike_sa, child_sa);
	message->destroy(message);
	charon->ike_sa_manager->checkin(charon->ike_sa_manager, ike_sa);
}

/**
 * Process messages of type CHILD_DELETE
 */
static void process_child_delete(private_ha_dispatcher_t *this,
								 ha_message_t *message)
{
	ha_message_attribute_t attribute;
	ha_message_value_t value;
	enumerator_t *enumerator;
	ike_sa_t *ike_sa = NULL;
	child_sa_t *child_sa;
	u_int32_t spi = 0;

	enumerator = message->create_attribute_enumerator(message);
	while (enumerator->enumerate(enumerator, &attribute, &value))
	{
		switch (attribute)
		{
			case HA_IKE_ID:
				ike_sa = charon->ike_sa_manager->checkout(charon->ike_sa_manager,
														  value.ike_sa_id);
				break;
			case HA_INBOUND_SPI:
				spi = value.u32;
				break;
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);

	if (ike_sa)
	{
		child_sa = ike_sa->get_child_sa(ike_sa, PROTO_ESP, spi, TRUE);
		if (child_sa)
		{
			ike_sa->destroy_child_sa(ike_sa, PROTO_ESP, spi);
		}
		charon->ike_sa_manager->checkin(charon->ike_sa_manager, ike_sa);
	}
	message->destroy(message);
}

/**
 * Process messages of type SEGMENT_TAKE/DROP
 */
static void process_segment(private_ha_dispatcher_t *this,
							ha_message_t *message, bool take)
{
	ha_message_attribute_t attribute;
	ha_message_value_t value;
	enumerator_t *enumerator;

	enumerator = message->create_attribute_enumerator(message);
	while (enumerator->enumerate(enumerator, &attribute, &value))
	{
		switch (attribute)
		{
			case HA_SEGMENT:
				if (take)
				{
					DBG1(DBG_CFG, "remote node takes segment %d", value.u16);
					this->segments->deactivate(this->segments, value.u16, FALSE);
				}
				else
				{
					DBG1(DBG_CFG, "remote node drops segment %d", value.u16);
					this->segments->activate(this->segments, value.u16, FALSE);
				}
				break;
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);
	message->destroy(message);
}

/**
 * Process messages of type STATUS
 */
static void process_status(private_ha_dispatcher_t *this,
						   ha_message_t *message)
{
	ha_message_attribute_t attribute;
	ha_message_value_t value;
	enumerator_t *enumerator;
	segment_mask_t mask = 0;

	enumerator = message->create_attribute_enumerator(message);
	while (enumerator->enumerate(enumerator, &attribute, &value))
	{
		switch (attribute)
		{
			case HA_SEGMENT:
				mask |= SEGMENTS_BIT(value.u16);
				break;
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);

	this->segments->handle_status(this->segments, mask);
	message->destroy(message);
}

/**
 * Process messages of type RESYNC
 */
static void process_resync(private_ha_dispatcher_t *this,
						   ha_message_t *message)
{
	ha_message_attribute_t attribute;
	ha_message_value_t value;
	enumerator_t *enumerator;

	enumerator = message->create_attribute_enumerator(message);
	while (enumerator->enumerate(enumerator, &attribute, &value))
	{
		switch (attribute)
		{
			case HA_SEGMENT:
				this->cache->resync(this->cache, value.u16);
				break;
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);
	message->destroy(message);
}

/**
 * Dispatcher job function
 */
static job_requeue_t dispatch(private_ha_dispatcher_t *this)
{
	ha_message_t *message;
	ha_message_type_t type;

	message = this->socket->pull(this->socket);
	type = message->get_type(message);
	if (type != HA_STATUS)
	{
		DBG2(DBG_CFG, "received HA %N message", ha_message_type_names,
			 message->get_type(message));
	}
	switch (type)
	{
		case HA_IKE_ADD:
			process_ike_add(this, message);
			break;
		case HA_IKE_UPDATE:
			process_ike_update(this, message);
			break;
		case HA_IKE_MID_INITIATOR:
			process_ike_mid(this, message, TRUE);
			break;
		case HA_IKE_MID_RESPONDER:
			process_ike_mid(this, message, FALSE);
			break;
		case HA_IKE_IV:
			process_ike_iv(this, message);
			break;
		case HA_IKE_DELETE:
			process_ike_delete(this, message);
			break;
		case HA_CHILD_ADD:
			process_child_add(this, message);
			break;
		case HA_CHILD_DELETE:
			process_child_delete(this, message);
			break;
		case HA_SEGMENT_DROP:
			process_segment(this, message, FALSE);
			break;
		case HA_SEGMENT_TAKE:
			process_segment(this, message, TRUE);
			break;
		case HA_STATUS:
			process_status(this, message);
			break;
		case HA_RESYNC:
			process_resync(this, message);
			break;
		default:
			DBG1(DBG_CFG, "received unknown HA message type %d", type);
			message->destroy(message);
			break;
	}
	return JOB_REQUEUE_DIRECT;
}

METHOD(ha_dispatcher_t, destroy, void,
	private_ha_dispatcher_t *this)
{
	free(this);
}

/**
 * See header
 */
ha_dispatcher_t *ha_dispatcher_create(ha_socket_t *socket,
									ha_segments_t *segments, ha_cache_t *cache,
									ha_kernel_t *kernel, ha_attribute_t *attr)
{
	private_ha_dispatcher_t *this;


	INIT(this,
		.public = {
			.destroy = _destroy,
		},
		.socket = socket,
		.segments = segments,
		.cache = cache,
		.kernel = kernel,
		.attr = attr,
	);
	lib->processor->queue_job(lib->processor,
		(job_t*)callback_job_create_with_prio((callback_job_cb_t)dispatch, this,
				NULL, (callback_job_cancel_t)return_false, JOB_PRIO_CRITICAL));

	return &this->public;
}
