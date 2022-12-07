/*
 * Copyright (C) 2008 Martin Willi
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

#include "ha_ike.h"

#include <sa/ikev2/keymat_v2.h>
#include <sa/ikev1/keymat_v1.h>

typedef struct private_ha_ike_t private_ha_ike_t;

/**
 * Private data of an ha_ike_t object.
 */
struct private_ha_ike_t {

	/**
	 * Public ha_ike_t interface.
	 */
	ha_ike_t public;

	/**
	 * socket we use for syncing
	 */
	ha_socket_t *socket;

	/**
	 * tunnel securing sync messages
	 */
	ha_tunnel_t *tunnel;

	/**
	 * message cache
	 */
	ha_cache_t *cache;
};

/**
 * Copy conditions of IKE_SA to message as HA_CONDITIONS attribute
 */
static void copy_conditions(ha_message_t *m, ike_sa_t *ike_sa)
{
	ike_condition_t i, conditions = 0;

	for (i = 0; i < sizeof(i) * 8; ++i)
	{
		ike_condition_t cond = (1 << i);

		conditions |= (ike_sa->has_condition(ike_sa, cond) ? cond : 0);
	}

	m->add_attribute(m, HA_CONDITIONS, (uint32_t)conditions);
}

/**
 * Copy extensions of IKE_SA to message as HA_EXTENSIONS attribute
 */
static void copy_extensions(ha_message_t *m, ike_sa_t *ike_sa)
{
	ike_extension_t i, extensions = 0;

	for (i = 0; i < sizeof(i) * 8; ++i)
	{
		ike_extension_t ext = (1 << i);

		extensions |= (ike_sa->supports_extension(ike_sa, ext) ? ext : 0);
	}

	m->add_attribute(m, HA_EXTENSIONS, (uint32_t)extensions);
}

METHOD(listener_t, ike_keys, bool,
	private_ha_ike_t *this, ike_sa_t *ike_sa, key_exchange_t *dh,
	chunk_t dh_other, chunk_t nonce_i, chunk_t nonce_r, ike_sa_t *rekey,
	shared_key_t *shared, auth_method_t method)
{
	ha_message_t *m;
	chunk_t secret;
	proposal_t *proposal;
	uint16_t alg, len;

	if (this->tunnel && this->tunnel->is_sa(this->tunnel, ike_sa))
	{	/* do not sync SA between nodes */
		return TRUE;
	}
	if (!dh->get_shared_secret(dh, &secret))
	{
		return TRUE;
	}

	m = ha_message_create(HA_IKE_ADD);
	m->add_attribute(m, HA_IKE_VERSION, ike_sa->get_version(ike_sa));
	m->add_attribute(m, HA_IKE_ID, ike_sa->get_id(ike_sa));

	if (rekey && rekey->get_version(rekey) == IKEV2)
	{
		chunk_t skd;
		keymat_v2_t *keymat;

		keymat = (keymat_v2_t*)rekey->get_keymat(rekey);
		m->add_attribute(m, HA_IKE_REKEY_ID, rekey->get_id(rekey));
		m->add_attribute(m, HA_ALG_OLD_PRF, keymat->get_skd(keymat, &skd));
		m->add_attribute(m, HA_OLD_SKD, skd);
	}

	proposal = ike_sa->get_proposal(ike_sa);
	if (proposal->get_algorithm(proposal, ENCRYPTION_ALGORITHM, &alg, &len))
	{
		m->add_attribute(m, HA_ALG_ENCR, alg);
		if (len)
		{
			m->add_attribute(m, HA_ALG_ENCR_LEN, len);
		}
	}
	if (proposal->get_algorithm(proposal, INTEGRITY_ALGORITHM, &alg, NULL))
	{
		m->add_attribute(m, HA_ALG_INTEG, alg);
	}
	if (proposal->get_algorithm(proposal, PSEUDO_RANDOM_FUNCTION, &alg, NULL))
	{
		m->add_attribute(m, HA_ALG_PRF, alg);
	}
	if (proposal->get_algorithm(proposal, KEY_EXCHANGE_METHOD, &alg, NULL))
	{
		m->add_attribute(m, HA_ALG_DH, alg);
	}
	m->add_attribute(m, HA_NONCE_I, nonce_i);
	m->add_attribute(m, HA_NONCE_R, nonce_r);
	m->add_attribute(m, HA_SECRET, secret);
	chunk_clear(&secret);
	if (ike_sa->get_version(ike_sa) == IKEV1)
	{
		if (dh->get_public_key(dh, &secret))
		{
			m->add_attribute(m, HA_LOCAL_DH, secret);
			chunk_free(&secret);
		}
		m->add_attribute(m, HA_REMOTE_DH, dh_other);
		if (shared)
		{
			m->add_attribute(m, HA_PSK, shared->get_key(shared));
		}
		else
		{
			m->add_attribute(m, HA_AUTH_METHOD, method);
		}
	}
	m->add_attribute(m, HA_REMOTE_ADDR, ike_sa->get_other_host(ike_sa));

	this->socket->push(this->socket, m);
	this->cache->cache(this->cache, ike_sa, m);

	return TRUE;
}

METHOD(listener_t, ike_updown, bool,
	private_ha_ike_t *this, ike_sa_t *ike_sa, bool up)
{
	ha_message_t *m;

	if (ike_sa->get_state(ike_sa) == IKE_PASSIVE)
	{	/* only sync active IKE_SAs */
		return TRUE;
	}
	if (this->tunnel && this->tunnel->is_sa(this->tunnel, ike_sa))
	{	/* do not sync SA between nodes */
		return TRUE;
	}

	if (up)
	{
		enumerator_t *enumerator;
		peer_cfg_t *peer_cfg;
		host_t *addr;
		ike_sa_id_t *id;
		identification_t *eap_id;

		peer_cfg = ike_sa->get_peer_cfg(ike_sa);

		id = ike_sa->get_id(ike_sa);

		m = ha_message_create(HA_IKE_UPDATE);
		m->add_attribute(m, HA_IKE_ID, id);
		m->add_attribute(m, HA_LOCAL_ID, ike_sa->get_my_id(ike_sa));
		m->add_attribute(m, HA_REMOTE_ID, ike_sa->get_other_id(ike_sa));
		eap_id = ike_sa->get_other_eap_id(ike_sa);
		if (!eap_id->equals(eap_id, ike_sa->get_other_id(ike_sa)))
		{
			m->add_attribute(m, HA_REMOTE_EAP_ID, eap_id);
		}
		m->add_attribute(m, HA_LOCAL_ADDR, ike_sa->get_my_host(ike_sa));
		m->add_attribute(m, HA_REMOTE_ADDR, ike_sa->get_other_host(ike_sa));
		copy_conditions(m, ike_sa);
		copy_extensions(m, ike_sa);
		m->add_attribute(m, HA_CONFIG_NAME, peer_cfg->get_name(peer_cfg));
		enumerator = ike_sa->create_peer_address_enumerator(ike_sa);
		while (enumerator->enumerate(enumerator, (void**)&addr))
		{
			m->add_attribute(m, HA_PEER_ADDR, addr);
		}
		enumerator->destroy(enumerator);
	}
	else
	{
		m = ha_message_create(HA_IKE_DELETE);
		m->add_attribute(m, HA_IKE_ID, ike_sa->get_id(ike_sa));
	}
	this->socket->push(this->socket, m);
	this->cache->cache(this->cache, ike_sa, m);
	return TRUE;
}

METHOD(listener_t, ike_rekey, bool,
	private_ha_ike_t *this, ike_sa_t *old, ike_sa_t *new)
{
	ike_updown(this, old, FALSE);
	ike_updown(this, new, TRUE);
	return TRUE;
}

METHOD(listener_t, alert, bool,
	private_ha_ike_t *this, ike_sa_t *ike_sa, alert_t alert, va_list args)
{
	switch (alert)
	{
		case ALERT_HALF_OPEN_TIMEOUT:
			ike_updown(this, ike_sa, FALSE);
			break;
		default:
			break;
	}
	return TRUE;
}

METHOD(listener_t, ike_state_change, bool,
	private_ha_ike_t *this, ike_sa_t *ike_sa, ike_sa_state_t new)
{
	/* delete any remaining cache entry if IKE_SA gets destroyed */
	if (new == IKE_DESTROYING)
	{
		this->cache->delete(this->cache, ike_sa);
	}
	return TRUE;
}

/**
 * Send a virtual IP sync message for remote VIPs
 */
static void sync_vips(private_ha_ike_t *this, ike_sa_t *ike_sa)
{
	ha_message_t *m = NULL;
	enumerator_t *enumerator;
	host_t *vip;

	enumerator = ike_sa->create_virtual_ip_enumerator(ike_sa, FALSE);
	while (enumerator->enumerate(enumerator, &vip))
	{
		if (!m)
		{
			m = ha_message_create(HA_IKE_UPDATE);
			m->add_attribute(m, HA_IKE_ID, ike_sa->get_id(ike_sa));
		}
		m->add_attribute(m, HA_REMOTE_VIP, vip);
	}
	enumerator->destroy(enumerator);

	if (m)
	{
		this->socket->push(this->socket, m);
		this->cache->cache(this->cache, ike_sa, m);
	}
}

METHOD(listener_t, message_hook, bool,
	private_ha_ike_t *this, ike_sa_t *ike_sa, message_t *message,
	bool incoming, bool plain)
{
	if (this->tunnel && this->tunnel->is_sa(this->tunnel, ike_sa))
	{	/* do not sync SA between nodes */
		return TRUE;
	}

	if (plain && ike_sa->get_version(ike_sa) == IKEV2)
	{
		if (message->get_exchange_type(message) != IKE_SA_INIT &&
			message->get_request(message))
		{	/* we sync on requests, but skip it on IKE_SA_INIT */
			ha_message_t *m;

			if (incoming)
			{
				m = ha_message_create(HA_IKE_MID_RESPONDER);
			}
			else
			{
				m = ha_message_create(HA_IKE_MID_INITIATOR);
			}
			m->add_attribute(m, HA_IKE_ID, ike_sa->get_id(ike_sa));
			m->add_attribute(m, HA_MID, message->get_message_id(message) + 1);
			this->socket->push(this->socket, m);
			this->cache->cache(this->cache, ike_sa, m);
		}
		if (ike_sa->get_state(ike_sa) == IKE_ESTABLISHED &&
			message->get_exchange_type(message) == IKE_AUTH &&
			!message->get_request(message))
		{	/* After IKE_SA has been established, sync peers virtual IP.
			 * We cannot sync it in the state_change hook, it is installed later.
			 * TODO: where to sync local VIP? */
			sync_vips(this, ike_sa);
		}
	}
	if (ike_sa->get_version(ike_sa) == IKEV1)
	{
		ha_message_t *m;
		keymat_v1_t *keymat;
		chunk_t iv;

		/* we need the last block (or expected next IV) of Phase 1, which gets
		 * updated after successful en-/decryption depending on direction */
		if (incoming == plain)
		{
			if (message->get_message_id(message) == 0)
			{
				keymat = (keymat_v1_t*)ike_sa->get_keymat(ike_sa);
				if (keymat->get_iv(keymat, 0, &iv))
				{
					m = ha_message_create(HA_IKE_IV);
					m->add_attribute(m, HA_IKE_ID, ike_sa->get_id(ike_sa));
					m->add_attribute(m, HA_IV, iv);
					this->socket->push(this->socket, m);
					this->cache->cache(this->cache, ike_sa, m);
				}
			}
		}
		if (!plain && !incoming &&
			message->get_exchange_type(message) == TRANSACTION)
		{
			sync_vips(this, ike_sa);
		}
	}
	if (plain && ike_sa->get_version(ike_sa) == IKEV1 &&
		message->get_exchange_type(message) == INFORMATIONAL_V1)
	{
		ha_message_t *m;
		notify_payload_t *notify;
		chunk_t data;
		uint32_t seq;

		notify = message->get_notify(message, DPD_R_U_THERE);
		if (notify)
		{
			data = notify->get_notification_data(notify);
			if (data.len == 4)
			{
				seq = untoh32(data.ptr);
				if (incoming)
				{
					m = ha_message_create(HA_IKE_MID_RESPONDER);
				}
				else
				{
					m = ha_message_create(HA_IKE_MID_INITIATOR);
				}
				m->add_attribute(m, HA_IKE_ID, ike_sa->get_id(ike_sa));
				m->add_attribute(m, HA_MID, seq + 1);
				this->socket->push(this->socket, m);
				this->cache->cache(this->cache, ike_sa, m);
			}
		}
	}
	return TRUE;
}

METHOD(ha_ike_t, destroy, void,
	private_ha_ike_t *this)
{
	free(this);
}

/**
 * See header
 */
ha_ike_t *ha_ike_create(ha_socket_t *socket, ha_tunnel_t *tunnel,
						ha_cache_t *cache)
{
	private_ha_ike_t *this;

	INIT(this,
		.public = {
			.listener = {
				.alert = _alert,
				.ike_keys = _ike_keys,
				.ike_updown = _ike_updown,
				.ike_rekey = _ike_rekey,
				.ike_state_change = _ike_state_change,
				.message = _message_hook,
			},
			.destroy = _destroy,
		},
		.socket = socket,
		.tunnel = tunnel,
		.cache = cache,
	);

	return &this->public;
}

