/*
 * Copyright (C) 2008 Tobias Brunner
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

#include "child_create.h"

#include <daemon.h>
#include <hydra.h>
#include <sa/ikev2/keymat_v2.h>
#include <crypto/diffie_hellman.h>
#include <credentials/certificates/x509.h>
#include <encoding/payloads/sa_payload.h>
#include <encoding/payloads/ke_payload.h>
#include <encoding/payloads/ts_payload.h>
#include <encoding/payloads/nonce_payload.h>
#include <encoding/payloads/notify_payload.h>
#include <encoding/payloads/delete_payload.h>
#include <processing/jobs/delete_ike_sa_job.h>
#include <processing/jobs/inactivity_job.h>


typedef struct private_child_create_t private_child_create_t;

/**
 * Private members of a child_create_t task.
 */
struct private_child_create_t {

	/**
	 * Public methods and task_t interface.
	 */
	child_create_t public;

	/**
	 * Assigned IKE_SA.
	 */
	ike_sa_t *ike_sa;

	/**
	 * Are we the initiator?
	 */
	bool initiator;

	/**
	 * nonce chosen by us
	 */
	chunk_t my_nonce;

	/**
	 * nonce chosen by peer
	 */
	chunk_t other_nonce;

	/**
	 * config to create the CHILD_SA from
	 */
	child_cfg_t *config;

	/**
	 * list of proposal candidates
	 */
	linked_list_t *proposals;

	/**
	 * selected proposal to use for CHILD_SA
	 */
	proposal_t *proposal;

	/**
	 * traffic selectors for initiators side
	 */
	linked_list_t *tsi;

	/**
	 * traffic selectors for responders side
	 */
	linked_list_t *tsr;

	/**
	 * source of triggering packet
	 */
	traffic_selector_t *packet_tsi;

	/**
	 * destination of triggering packet
	 */
	traffic_selector_t *packet_tsr;

	/**
	 * optional diffie hellman exchange
	 */
	diffie_hellman_t *dh;

	/**
	 * group used for DH exchange
	 */
	diffie_hellman_group_t dh_group;

	/**
	 * IKE_SAs keymat
	 */
	keymat_v2_t *keymat;

	/**
	 * mode the new CHILD_SA uses (transport/tunnel/beet)
	 */
	ipsec_mode_t mode;

	/**
	 * peer accepts TFC padding for this SA
	 */
	bool tfcv3;

	/**
	 * IPComp transform to use
	 */
	ipcomp_transform_t ipcomp;

	/**
	 * IPComp transform proposed or accepted by the other peer
	 */
	ipcomp_transform_t ipcomp_received;

	/**
	 * Own allocated SPI
	 */
	u_int32_t my_spi;

	/**
	 * SPI received in proposal
	 */
	u_int32_t other_spi;

	/**
	 * Own allocated Compression Parameter Index (CPI)
	 */
	u_int16_t my_cpi;

	/**
	 * Other Compression Parameter Index (CPI), received via IPCOMP_SUPPORTED
	 */
	u_int16_t other_cpi;

	/**
	 * reqid to use if we are rekeying
	 */
	u_int32_t reqid;

	/**
	 * CHILD_SA which gets established
	 */
	child_sa_t *child_sa;

	/**
	 * successfully established the CHILD?
	 */
	bool established;

	/**
	 * whether the CHILD_SA rekeys an existing one
	 */
	bool rekey;

	/**
	 * whether we are retrying with another DH group
	 */
	bool retry;
};

/**
 * get the nonce from a message
 */
static status_t get_nonce(message_t *message, chunk_t *nonce)
{
	nonce_payload_t *payload;

	payload = (nonce_payload_t*)message->get_payload(message, PLV2_NONCE);
	if (payload == NULL)
	{
		return FAILED;
	}
	*nonce = payload->get_nonce(payload);
	return NEED_MORE;
}

/**
 * generate a new nonce to include in a CREATE_CHILD_SA message
 */
static status_t generate_nonce(private_child_create_t *this)
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

	return SUCCESS;
}

/**
 * Check a list of traffic selectors if any selector belongs to host
 */
static bool ts_list_is_host(linked_list_t *list, host_t *host)
{
	traffic_selector_t *ts;
	bool is_host = TRUE;
	enumerator_t *enumerator = list->create_enumerator(list);

	while (is_host && enumerator->enumerate(enumerator, (void**)&ts))
	{
		is_host = is_host && ts->is_host(ts, host);
	}
	enumerator->destroy(enumerator);
	return is_host;
}

/**
 * Allocate SPIs and update proposals
 */
static bool allocate_spi(private_child_create_t *this)
{
	enumerator_t *enumerator;
	proposal_t *proposal;
	protocol_id_t proto = PROTO_ESP;

	if (this->initiator)
	{
		/* we just get a SPI for the first protocol. TODO: If we ever support
		 * proposal lists with mixed protocols, we'd need multiple SPIs */
		if (this->proposals->get_first(this->proposals,
									   (void**)&proposal) == SUCCESS)
		{
			proto = proposal->get_protocol(proposal);
		}
	}
	else
	{
		proto = this->proposal->get_protocol(this->proposal);
	}
	this->my_spi = this->child_sa->alloc_spi(this->child_sa, proto);
	if (this->my_spi)
	{
		if (this->initiator)
		{
			enumerator = this->proposals->create_enumerator(this->proposals);
			while (enumerator->enumerate(enumerator, &proposal))
			{
				proposal->set_spi(proposal, this->my_spi);
			}
			enumerator->destroy(enumerator);
		}
		else
		{
			this->proposal->set_spi(this->proposal, this->my_spi);
		}
		return TRUE;
	}
	return FALSE;
}

/**
 * Schedule inactivity timeout for CHILD_SA with reqid, if enabled
 */
static void schedule_inactivity_timeout(private_child_create_t *this)
{
	u_int32_t timeout;
	bool close_ike;

	timeout = this->config->get_inactivity(this->config);
	if (timeout)
	{
		close_ike = lib->settings->get_bool(lib->settings,
									"%s.inactivity_close_ike", FALSE, lib->ns);
		lib->scheduler->schedule_job(lib->scheduler, (job_t*)
				inactivity_job_create(this->child_sa->get_reqid(this->child_sa),
									  timeout, close_ike), timeout);
	}
}

/**
 * Check if we have a an address pool configured
 */
static bool have_pool(ike_sa_t *ike_sa)
{
	enumerator_t *enumerator;
	peer_cfg_t *peer_cfg;
	char *pool;
	bool found = FALSE;

	peer_cfg = ike_sa->get_peer_cfg(ike_sa);
	if (peer_cfg)
	{
		enumerator = peer_cfg->create_pool_enumerator(peer_cfg);
		if (enumerator->enumerate(enumerator, &pool))
		{
			found = TRUE;
		}
		enumerator->destroy(enumerator);
	}
	return found;
}

/**
 * Get hosts to use for dynamic traffic selectors
 */
static linked_list_t *get_dynamic_hosts(ike_sa_t *ike_sa, bool local)
{
	enumerator_t *enumerator;
	linked_list_t *list;
	host_t *host;

	list = linked_list_create();
	enumerator = ike_sa->create_virtual_ip_enumerator(ike_sa, local);
	while (enumerator->enumerate(enumerator, &host))
	{
		list->insert_last(list, host);
	}
	enumerator->destroy(enumerator);

	if (list->get_count(list) == 0)
	{	/* no virtual IPs assigned */
		if (local)
		{
			host = ike_sa->get_my_host(ike_sa);
			list->insert_last(list, host);
		}
		else if (!have_pool(ike_sa))
		{	/* use host only if we don't have a pool configured */
			host = ike_sa->get_other_host(ike_sa);
			list->insert_last(list, host);
		}
	}
	return list;
}

/**
 * Substitude any host address with NATed address in traffic selector
 */
static linked_list_t* get_transport_nat_ts(private_child_create_t *this,
										   bool local, linked_list_t *in)
{
	enumerator_t *enumerator;
	linked_list_t *out;
	traffic_selector_t *ts;
	host_t *ike, *first = NULL;
	u_int8_t mask;

	if (local)
	{
		ike = this->ike_sa->get_my_host(this->ike_sa);
	}
	else
	{
		ike = this->ike_sa->get_other_host(this->ike_sa);
	}

	out = linked_list_create();

	enumerator = in->create_enumerator(in);
	while (enumerator->enumerate(enumerator, &ts))
	{
		/* require that all selectors match the first "host" selector */
		if (ts->is_host(ts, first))
		{
			if (!first)
			{
				ts->to_subnet(ts, &first, &mask);
			}
			ts = ts->clone(ts);
			ts->set_address(ts, ike);
			out->insert_last(out, ts);
		}
	}
	enumerator->destroy(enumerator);
	DESTROY_IF(first);

	return out;
}

/**
 * Narrow received traffic selectors with configuration
 */
static linked_list_t* narrow_ts(private_child_create_t *this, bool local,
								linked_list_t *in)
{
	linked_list_t *hosts, *nat, *ts;
	ike_condition_t cond;

	cond = local ? COND_NAT_HERE : COND_NAT_THERE;
	hosts = get_dynamic_hosts(this->ike_sa, local);

	if (this->mode == MODE_TRANSPORT &&
		this->ike_sa->has_condition(this->ike_sa, cond))
	{
		nat = get_transport_nat_ts(this, local, in);
		ts = this->config->get_traffic_selectors(this->config, local, nat, hosts);
		nat->destroy_offset(nat, offsetof(traffic_selector_t, destroy));
	}
	else
	{
		ts = this->config->get_traffic_selectors(this->config, local, in, hosts);
	}

	hosts->destroy(hosts);

	return ts;
}

/**
 * Install a CHILD_SA for usage, return value:
 * - FAILED: no acceptable proposal
 * - INVALID_ARG: diffie hellman group inacceptable
 * - NOT_FOUND: TS inacceptable
 */
static status_t select_and_install(private_child_create_t *this,
								   bool no_dh, bool ike_auth)
{
	status_t status, status_i, status_o;
	chunk_t nonce_i, nonce_r;
	chunk_t encr_i = chunk_empty, encr_r = chunk_empty;
	chunk_t integ_i = chunk_empty, integ_r = chunk_empty;
	linked_list_t *my_ts, *other_ts;
	host_t *me, *other;
	bool private;

	if (this->proposals == NULL)
	{
		DBG1(DBG_IKE, "SA payload missing in message");
		return FAILED;
	}
	if (this->tsi == NULL || this->tsr == NULL)
	{
		DBG1(DBG_IKE, "TS payloads missing in message");
		return NOT_FOUND;
	}

	me = this->ike_sa->get_my_host(this->ike_sa);
	other = this->ike_sa->get_other_host(this->ike_sa);

	private = this->ike_sa->supports_extension(this->ike_sa, EXT_STRONGSWAN);
	this->proposal = this->config->select_proposal(this->config,
											this->proposals, no_dh, private);
	if (this->proposal == NULL)
	{
		DBG1(DBG_IKE, "no acceptable proposal found");
		charon->bus->alert(charon->bus, ALERT_PROPOSAL_MISMATCH_CHILD,
						   this->proposals);
		return FAILED;
	}
	this->other_spi = this->proposal->get_spi(this->proposal);

	if (!this->initiator && !allocate_spi(this))
	{	/* responder has no SPI allocated yet */
		DBG1(DBG_IKE, "allocating SPI failed");
		return FAILED;
	}
	this->child_sa->set_proposal(this->child_sa, this->proposal);

	if (!this->proposal->has_dh_group(this->proposal, this->dh_group))
	{
		u_int16_t group;

		if (this->proposal->get_algorithm(this->proposal, DIFFIE_HELLMAN_GROUP,
										  &group, NULL))
		{
			DBG1(DBG_IKE, "DH group %N inacceptable, requesting %N",
				 diffie_hellman_group_names, this->dh_group,
				 diffie_hellman_group_names, group);
			this->dh_group = group;
			return INVALID_ARG;
		}
		/* the selected proposal does not use a DH group */
		DBG1(DBG_IKE, "ignoring KE exchange, agreed on a non-PFS proposal");
		DESTROY_IF(this->dh);
		this->dh = NULL;
		this->dh_group = MODP_NONE;
	}

	if (this->initiator)
	{
		nonce_i = this->my_nonce;
		nonce_r = this->other_nonce;
		my_ts = narrow_ts(this, TRUE, this->tsi);
		other_ts = narrow_ts(this, FALSE, this->tsr);
	}
	else
	{
		nonce_r = this->my_nonce;
		nonce_i = this->other_nonce;
		my_ts = narrow_ts(this, TRUE, this->tsr);
		other_ts = narrow_ts(this, FALSE, this->tsi);
	}

	if (this->initiator)
	{
		if (ike_auth)
		{
			charon->bus->narrow(charon->bus, this->child_sa,
								NARROW_INITIATOR_POST_NOAUTH, my_ts, other_ts);
		}
		else
		{
			charon->bus->narrow(charon->bus, this->child_sa,
								NARROW_INITIATOR_POST_AUTH, my_ts, other_ts);
		}
	}
	else
	{
		charon->bus->narrow(charon->bus, this->child_sa,
							NARROW_RESPONDER, my_ts, other_ts);
	}

	if (my_ts->get_count(my_ts) == 0 || other_ts->get_count(other_ts) == 0)
	{
		charon->bus->alert(charon->bus, ALERT_TS_MISMATCH, this->tsi, this->tsr);
		my_ts->destroy_offset(my_ts, offsetof(traffic_selector_t, destroy));
		other_ts->destroy_offset(other_ts, offsetof(traffic_selector_t, destroy));
		DBG1(DBG_IKE, "no acceptable traffic selectors found");
		return NOT_FOUND;
	}

	this->tsr->destroy_offset(this->tsr, offsetof(traffic_selector_t, destroy));
	this->tsi->destroy_offset(this->tsi, offsetof(traffic_selector_t, destroy));
	if (this->initiator)
	{
		this->tsi = my_ts;
		this->tsr = other_ts;
	}
	else
	{
		this->tsr = my_ts;
		this->tsi = other_ts;
	}

	if (!this->initiator)
	{
		/* check if requested mode is acceptable, downgrade if required */
		switch (this->mode)
		{
			case MODE_TRANSPORT:
				if (!this->config->use_proxy_mode(this->config) &&
					   (!ts_list_is_host(this->tsi, other) ||
						!ts_list_is_host(this->tsr, me))
				   )
				{
					this->mode = MODE_TUNNEL;
					DBG1(DBG_IKE, "not using transport mode, not host-to-host");
				}
				if (this->config->get_mode(this->config) != MODE_TRANSPORT)
				{
					this->mode = MODE_TUNNEL;
				}
				break;
			case MODE_BEET:
				if (!ts_list_is_host(this->tsi, NULL) ||
					!ts_list_is_host(this->tsr, NULL))
				{
					this->mode = MODE_TUNNEL;
					DBG1(DBG_IKE, "not using BEET mode, not host-to-host");
				}
				if (this->config->get_mode(this->config) != MODE_BEET)
				{
					this->mode = MODE_TUNNEL;
				}
				break;
			default:
				break;
		}
	}

	this->child_sa->set_state(this->child_sa, CHILD_INSTALLING);
	this->child_sa->set_ipcomp(this->child_sa, this->ipcomp);
	this->child_sa->set_mode(this->child_sa, this->mode);
	this->child_sa->set_protocol(this->child_sa,
								 this->proposal->get_protocol(this->proposal));

	if (this->my_cpi == 0 || this->other_cpi == 0 || this->ipcomp == IPCOMP_NONE)
	{
		this->my_cpi = this->other_cpi = 0;
		this->ipcomp = IPCOMP_NONE;
	}
	status_i = status_o = FAILED;
	if (this->keymat->derive_child_keys(this->keymat, this->proposal,
			this->dh, nonce_i, nonce_r, &encr_i, &integ_i, &encr_r, &integ_r))
	{
		if (this->initiator)
		{
			status_i = this->child_sa->install(this->child_sa, encr_r, integ_r,
							this->my_spi, this->my_cpi, this->initiator,
							TRUE, this->tfcv3, my_ts, other_ts);
			status_o = this->child_sa->install(this->child_sa, encr_i, integ_i,
							this->other_spi, this->other_cpi, this->initiator,
							FALSE, this->tfcv3, my_ts, other_ts);
		}
		else
		{
			status_i = this->child_sa->install(this->child_sa, encr_i, integ_i,
							this->my_spi, this->my_cpi, this->initiator,
							TRUE, this->tfcv3, my_ts, other_ts);
			status_o = this->child_sa->install(this->child_sa, encr_r, integ_r,
							this->other_spi, this->other_cpi, this->initiator,
							FALSE, this->tfcv3, my_ts, other_ts);
		}
	}
	chunk_clear(&integ_i);
	chunk_clear(&integ_r);
	chunk_clear(&encr_i);
	chunk_clear(&encr_r);

	if (status_i != SUCCESS || status_o != SUCCESS)
	{
		DBG1(DBG_IKE, "unable to install %s%s%sIPsec SA (SAD) in kernel",
			(status_i != SUCCESS) ? "inbound " : "",
			(status_i != SUCCESS && status_o != SUCCESS) ? "and ": "",
			(status_o != SUCCESS) ? "outbound " : "");
		charon->bus->alert(charon->bus, ALERT_INSTALL_CHILD_SA_FAILED,
						   this->child_sa);
		return FAILED;
	}

	if (this->initiator)
	{
		status = this->child_sa->add_policies(this->child_sa, my_ts, other_ts);
	}
	else
	{
		/* use a copy of the traffic selectors, as the POST hook should not
		 * change payloads */
		my_ts = this->tsr->clone_offset(this->tsr,
										offsetof(traffic_selector_t, clone));
		other_ts = this->tsi->clone_offset(this->tsi,
										offsetof(traffic_selector_t, clone));
		charon->bus->narrow(charon->bus, this->child_sa,
							NARROW_RESPONDER_POST, my_ts, other_ts);
		if (my_ts->get_count(my_ts) == 0 || other_ts->get_count(other_ts) == 0)
		{
			status = FAILED;
		}
		else
		{
			status = this->child_sa->add_policies(this->child_sa,
												   my_ts, other_ts);
		}
		my_ts->destroy_offset(my_ts, offsetof(traffic_selector_t, destroy));
		other_ts->destroy_offset(other_ts, offsetof(traffic_selector_t, destroy));
	}
	if (status != SUCCESS)
	{
		DBG1(DBG_IKE, "unable to install IPsec policies (SPD) in kernel");
		charon->bus->alert(charon->bus, ALERT_INSTALL_CHILD_POLICY_FAILED,
						   this->child_sa);
		return NOT_FOUND;
	}

	charon->bus->child_keys(charon->bus, this->child_sa, this->initiator,
							this->dh, nonce_i, nonce_r);

	/* add to IKE_SA, and remove from task */
	this->child_sa->set_state(this->child_sa, CHILD_INSTALLED);
	this->ike_sa->add_child_sa(this->ike_sa, this->child_sa);
	this->established = TRUE;

	if (!this->rekey)
	{	/* a rekeyed SA uses the same reqid, no need for a new job */
		schedule_inactivity_timeout(this);
	}

	my_ts = linked_list_create_from_enumerator(
				this->child_sa->create_ts_enumerator(this->child_sa, TRUE));
	other_ts = linked_list_create_from_enumerator(
				this->child_sa->create_ts_enumerator(this->child_sa, FALSE));

	DBG0(DBG_IKE, "CHILD_SA %s{%d} established "
		 "with SPIs %.8x_i %.8x_o and TS %#R=== %#R",
		 this->child_sa->get_name(this->child_sa),
		 this->child_sa->get_reqid(this->child_sa),
		 ntohl(this->child_sa->get_spi(this->child_sa, TRUE)),
		 ntohl(this->child_sa->get_spi(this->child_sa, FALSE)), my_ts, other_ts);

	my_ts->destroy(my_ts);
	other_ts->destroy(other_ts);

	return SUCCESS;
}

/**
 * build the payloads for the message
 */
static void build_payloads(private_child_create_t *this, message_t *message)
{
	sa_payload_t *sa_payload;
	nonce_payload_t *nonce_payload;
	ke_payload_t *ke_payload;
	ts_payload_t *ts_payload;
	kernel_feature_t features;

	/* add SA payload */
	if (this->initiator)
	{
		sa_payload = sa_payload_create_from_proposals_v2(this->proposals);
	}
	else
	{
		sa_payload = sa_payload_create_from_proposal_v2(this->proposal);
	}
	message->add_payload(message, (payload_t*)sa_payload);

	/* add nonce payload if not in IKE_AUTH */
	if (message->get_exchange_type(message) == CREATE_CHILD_SA)
	{
		nonce_payload = nonce_payload_create(PLV2_NONCE);
		nonce_payload->set_nonce(nonce_payload, this->my_nonce);
		message->add_payload(message, (payload_t*)nonce_payload);
	}

	/* diffie hellman exchange, if PFS enabled */
	if (this->dh)
	{
		ke_payload = ke_payload_create_from_diffie_hellman(PLV2_KEY_EXCHANGE,
														   this->dh);
		message->add_payload(message, (payload_t*)ke_payload);
	}

	/* add TSi/TSr payloads */
	ts_payload = ts_payload_create_from_traffic_selectors(TRUE, this->tsi);
	message->add_payload(message, (payload_t*)ts_payload);
	ts_payload = ts_payload_create_from_traffic_selectors(FALSE, this->tsr);
	message->add_payload(message, (payload_t*)ts_payload);

	/* add a notify if we are not in tunnel mode */
	switch (this->mode)
	{
		case MODE_TRANSPORT:
			message->add_notify(message, FALSE, USE_TRANSPORT_MODE, chunk_empty);
			break;
		case MODE_BEET:
			message->add_notify(message, FALSE, USE_BEET_MODE, chunk_empty);
			break;
		default:
			break;
	}

	features = hydra->kernel_interface->get_features(hydra->kernel_interface);
	if (!(features & KERNEL_ESP_V3_TFC))
	{
		message->add_notify(message, FALSE, ESP_TFC_PADDING_NOT_SUPPORTED,
							chunk_empty);
	}
}

/**
 * Adds an IPCOMP_SUPPORTED notify to the message, allocating a CPI
 */
static void add_ipcomp_notify(private_child_create_t *this,
								  message_t *message, u_int8_t ipcomp)
{
	this->my_cpi = this->child_sa->alloc_cpi(this->child_sa);
	if (this->my_cpi)
	{
		this->ipcomp = ipcomp;
		message->add_notify(message, FALSE, IPCOMP_SUPPORTED,
							chunk_cata("cc", chunk_from_thing(this->my_cpi),
									   chunk_from_thing(ipcomp)));
	}
	else
	{
		DBG1(DBG_IKE, "unable to allocate a CPI from kernel, IPComp disabled");
	}
}

/**
 * handle a received notify payload
 */
static void handle_notify(private_child_create_t *this, notify_payload_t *notify)
{
	switch (notify->get_notify_type(notify))
	{
		case USE_TRANSPORT_MODE:
			this->mode = MODE_TRANSPORT;
			break;
		case USE_BEET_MODE:
			if (this->ike_sa->supports_extension(this->ike_sa, EXT_STRONGSWAN))
			{	/* handle private use notify only if we know its meaning */
				this->mode = MODE_BEET;
			}
			else
			{
				DBG1(DBG_IKE, "received a notify strongSwan uses for BEET "
					 "mode, but peer implementation unknown, skipped");
			}
			break;
		case IPCOMP_SUPPORTED:
		{
			ipcomp_transform_t ipcomp;
			u_int16_t cpi;
			chunk_t data;

			data = notify->get_notification_data(notify);
			cpi = *(u_int16_t*)data.ptr;
			ipcomp = (ipcomp_transform_t)(*(data.ptr + 2));
			switch (ipcomp)
			{
				case IPCOMP_DEFLATE:
					this->other_cpi = cpi;
					this->ipcomp_received = ipcomp;
					break;
				case IPCOMP_LZS:
				case IPCOMP_LZJH:
				default:
					DBG1(DBG_IKE, "received IPCOMP_SUPPORTED notify with a "
						 "transform ID we don't support %N",
						 ipcomp_transform_names, ipcomp);
					break;
			}
			break;
		}
		case ESP_TFC_PADDING_NOT_SUPPORTED:
			DBG1(DBG_IKE, "received %N, not using ESPv3 TFC padding",
				 notify_type_names, notify->get_notify_type(notify));
			this->tfcv3 = FALSE;
			break;
		default:
			break;
	}
}

/**
 * Read payloads from message
 */
static void process_payloads(private_child_create_t *this, message_t *message)
{
	enumerator_t *enumerator;
	payload_t *payload;
	sa_payload_t *sa_payload;
	ke_payload_t *ke_payload;
	ts_payload_t *ts_payload;

	/* defaults to TUNNEL mode */
	this->mode = MODE_TUNNEL;

	enumerator = message->create_payload_enumerator(message);
	while (enumerator->enumerate(enumerator, &payload))
	{
		switch (payload->get_type(payload))
		{
			case PLV2_SECURITY_ASSOCIATION:
				sa_payload = (sa_payload_t*)payload;
				this->proposals = sa_payload->get_proposals(sa_payload);
				break;
			case PLV2_KEY_EXCHANGE:
				ke_payload = (ke_payload_t*)payload;
				if (!this->initiator)
				{
					this->dh_group = ke_payload->get_dh_group_number(ke_payload);
					this->dh = this->keymat->keymat.create_dh(
										&this->keymat->keymat, this->dh_group);
				}
				if (this->dh)
				{
					this->dh->set_other_public_value(this->dh,
								ke_payload->get_key_exchange_data(ke_payload));
				}
				break;
			case PLV2_TS_INITIATOR:
				ts_payload = (ts_payload_t*)payload;
				this->tsi = ts_payload->get_traffic_selectors(ts_payload);
				break;
			case PLV2_TS_RESPONDER:
				ts_payload = (ts_payload_t*)payload;
				this->tsr = ts_payload->get_traffic_selectors(ts_payload);
				break;
			case PLV2_NOTIFY:
				handle_notify(this, (notify_payload_t*)payload);
				break;
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);
}

METHOD(task_t, build_i, status_t,
	private_child_create_t *this, message_t *message)
{
	enumerator_t *enumerator;
	host_t *vip;
	peer_cfg_t *peer_cfg;
	linked_list_t *list;

	switch (message->get_exchange_type(message))
	{
		case IKE_SA_INIT:
			return get_nonce(message, &this->my_nonce);
		case CREATE_CHILD_SA:
			if (generate_nonce(this) != SUCCESS)
			{
				message->add_notify(message, FALSE, NO_PROPOSAL_CHOSEN, chunk_empty);
				return SUCCESS;
			}
			if (!this->retry)
			{
				this->dh_group = this->config->get_dh_group(this->config);
			}
			break;
		case IKE_AUTH:
			if (message->get_message_id(message) != 1)
			{
				/* send only in the first request, not in subsequent rounds */
				return NEED_MORE;
			}
			break;
		default:
			break;
	}

	if (this->reqid)
	{
		DBG0(DBG_IKE, "establishing CHILD_SA %s{%d}",
			 this->config->get_name(this->config), this->reqid);
	}
	else
	{
		DBG0(DBG_IKE, "establishing CHILD_SA %s",
			 this->config->get_name(this->config));
	}

	/* check if we want a virtual IP, but don't have one */
	list = linked_list_create();
	peer_cfg = this->ike_sa->get_peer_cfg(this->ike_sa);
	if (!this->rekey)
	{
		enumerator = peer_cfg->create_virtual_ip_enumerator(peer_cfg);
		while (enumerator->enumerate(enumerator, &vip))
		{
			/* propose a 0.0.0.0/0 or ::/0 subnet when we use virtual ip */
			vip = host_create_any(vip->get_family(vip));
			list->insert_last(list, vip);
		}
		enumerator->destroy(enumerator);
	}
	if (list->get_count(list))
	{
		this->tsi = this->config->get_traffic_selectors(this->config,
														TRUE, NULL, list);
		list->destroy_offset(list, offsetof(host_t, destroy));
	}
	else
	{	/* no virtual IPs configured */
		list->destroy(list);
		list = get_dynamic_hosts(this->ike_sa, TRUE);
		this->tsi = this->config->get_traffic_selectors(this->config,
														TRUE, NULL, list);
		list->destroy(list);
	}
	list = get_dynamic_hosts(this->ike_sa, FALSE);
	this->tsr = this->config->get_traffic_selectors(this->config,
													FALSE, NULL, list);
	list->destroy(list);

	if (this->packet_tsi)
	{
		this->tsi->insert_first(this->tsi,
								this->packet_tsi->clone(this->packet_tsi));
	}
	if (this->packet_tsr)
	{
		this->tsr->insert_first(this->tsr,
								this->packet_tsr->clone(this->packet_tsr));
	}
	this->proposals = this->config->get_proposals(this->config,
												  this->dh_group == MODP_NONE);
	this->mode = this->config->get_mode(this->config);

	this->child_sa = child_sa_create(this->ike_sa->get_my_host(this->ike_sa),
			this->ike_sa->get_other_host(this->ike_sa), this->config, this->reqid,
			this->ike_sa->has_condition(this->ike_sa, COND_NAT_ANY));

	if (!allocate_spi(this))
	{
		DBG1(DBG_IKE, "unable to allocate SPIs from kernel");
		return FAILED;
	}

	if (this->dh_group != MODP_NONE)
	{
		this->dh = this->keymat->keymat.create_dh(&this->keymat->keymat,
												  this->dh_group);
	}

	if (this->config->use_ipcomp(this->config))
	{
		/* IPCOMP_DEFLATE is the only transform we support at the moment */
		add_ipcomp_notify(this, message, IPCOMP_DEFLATE);
	}

	if (message->get_exchange_type(message) == IKE_AUTH)
	{
		charon->bus->narrow(charon->bus, this->child_sa,
							NARROW_INITIATOR_PRE_NOAUTH, this->tsi, this->tsr);
	}
	else
	{
		charon->bus->narrow(charon->bus, this->child_sa,
							NARROW_INITIATOR_PRE_AUTH, this->tsi, this->tsr);
	}

	build_payloads(this, message);

	this->tsi->destroy_offset(this->tsi, offsetof(traffic_selector_t, destroy));
	this->tsr->destroy_offset(this->tsr, offsetof(traffic_selector_t, destroy));
	this->proposals->destroy_offset(this->proposals, offsetof(proposal_t, destroy));
	this->tsi = NULL;
	this->tsr = NULL;
	this->proposals = NULL;

	return NEED_MORE;
}

METHOD(task_t, process_r, status_t,
	private_child_create_t *this, message_t *message)
{
	switch (message->get_exchange_type(message))
	{
		case IKE_SA_INIT:
			return get_nonce(message, &this->other_nonce);
		case CREATE_CHILD_SA:
			get_nonce(message, &this->other_nonce);
			break;
		case IKE_AUTH:
			if (message->get_message_id(message) != 1)
			{
				/* only handle first AUTH payload, not additional rounds */
				return NEED_MORE;
			}
		default:
			break;
	}

	process_payloads(this, message);

	return NEED_MORE;
}

/**
 * handle CHILD_SA setup failure
 */
static void handle_child_sa_failure(private_child_create_t *this,
									message_t *message)
{
	if (message->get_exchange_type(message) == IKE_AUTH &&
		lib->settings->get_bool(lib->settings,
								"%s.close_ike_on_child_failure", FALSE, lib->ns))
	{
		/* we delay the delete for 100ms, as the IKE_AUTH response must arrive
		 * first */
		DBG1(DBG_IKE, "closing IKE_SA due CHILD_SA setup failure");
		lib->scheduler->schedule_job_ms(lib->scheduler, (job_t*)
			delete_ike_sa_job_create(this->ike_sa->get_id(this->ike_sa), TRUE),
			100);
	}
	else
	{
		DBG1(DBG_IKE, "failed to establish CHILD_SA, keeping IKE_SA");
		charon->bus->alert(charon->bus, ALERT_KEEP_ON_CHILD_SA_FAILURE);
	}
}

/**
 * Substitute transport mode NAT selectors, if applicable
 */
static linked_list_t* get_ts_if_nat_transport(private_child_create_t *this,
											  bool local, linked_list_t *in)
{
	linked_list_t *out = NULL;
	ike_condition_t cond;

	if (this->mode == MODE_TRANSPORT)
	{
		cond = local ? COND_NAT_HERE : COND_NAT_THERE;
		if (this->ike_sa->has_condition(this->ike_sa, cond))
		{
			out = get_transport_nat_ts(this, local, in);
			if (out->get_count(out) == 0)
			{
				out->destroy(out);
				out = NULL;
			}
		}
	}
	return out;
}

/**
 * Select a matching CHILD config as responder
 */
static child_cfg_t* select_child_cfg(private_child_create_t *this)
{
	peer_cfg_t *peer_cfg;
	child_cfg_t *child_cfg = NULL;;

	peer_cfg = this->ike_sa->get_peer_cfg(this->ike_sa);
	if (peer_cfg && this->tsi && this->tsr)
	{
		linked_list_t *listr, *listi, *tsr, *tsi;

		tsr = get_ts_if_nat_transport(this, TRUE, this->tsr);
		tsi = get_ts_if_nat_transport(this, FALSE, this->tsi);

		listr = get_dynamic_hosts(this->ike_sa, TRUE);
		listi = get_dynamic_hosts(this->ike_sa, FALSE);
		child_cfg = peer_cfg->select_child_cfg(peer_cfg,
											tsr ?: this->tsr, tsi ?: this->tsi,
											listr, listi);
		if ((tsi || tsr) && child_cfg &&
			child_cfg->get_mode(child_cfg) != MODE_TRANSPORT)
		{
			/* found a CHILD config, but it doesn't use transport mode */
			child_cfg->destroy(child_cfg);
			child_cfg = NULL;
		}
		if (!child_cfg && (tsi || tsr))
		{
			/* no match for the substituted NAT selectors, try it without */
			child_cfg = peer_cfg->select_child_cfg(peer_cfg,
											this->tsr, this->tsi, listr, listi);
		}
		listr->destroy(listr);
		listi->destroy(listi);
		DESTROY_OFFSET_IF(tsi, offsetof(traffic_selector_t, destroy));
		DESTROY_OFFSET_IF(tsr, offsetof(traffic_selector_t, destroy));
	}

	return child_cfg;
}

METHOD(task_t, build_r, status_t,
	private_child_create_t *this, message_t *message)
{
	payload_t *payload;
	enumerator_t *enumerator;
	bool no_dh = TRUE, ike_auth = FALSE;

	switch (message->get_exchange_type(message))
	{
		case IKE_SA_INIT:
			return get_nonce(message, &this->my_nonce);
		case CREATE_CHILD_SA:
			if (generate_nonce(this) != SUCCESS)
			{
				message->add_notify(message, FALSE, NO_PROPOSAL_CHOSEN,
									chunk_empty);
				return SUCCESS;
			}
			no_dh = FALSE;
			break;
		case IKE_AUTH:
			if (this->ike_sa->get_state(this->ike_sa) != IKE_ESTABLISHED)
			{	/* wait until all authentication round completed */
				return NEED_MORE;
			}
			ike_auth = TRUE;
		default:
			break;
	}

	if (this->ike_sa->get_state(this->ike_sa) == IKE_REKEYING)
	{
		DBG1(DBG_IKE, "unable to create CHILD_SA while rekeying IKE_SA");
		message->add_notify(message, TRUE, NO_ADDITIONAL_SAS, chunk_empty);
		return SUCCESS;
	}
	if (this->ike_sa->get_state(this->ike_sa) == IKE_DELETING)
	{
		DBG1(DBG_IKE, "unable to create CHILD_SA while deleting IKE_SA");
		message->add_notify(message, TRUE, NO_ADDITIONAL_SAS, chunk_empty);
		return SUCCESS;
	}

	if (this->config == NULL)
	{
		this->config = select_child_cfg(this);
	}
	if (this->config == NULL)
	{
		DBG1(DBG_IKE, "traffic selectors %#R=== %#R inacceptable",
			 this->tsr, this->tsi);
		charon->bus->alert(charon->bus, ALERT_TS_MISMATCH, this->tsi, this->tsr);
		message->add_notify(message, FALSE, TS_UNACCEPTABLE, chunk_empty);
		handle_child_sa_failure(this, message);
		return SUCCESS;
	}

	/* check if ike_config_t included non-critical error notifies */
	enumerator = message->create_payload_enumerator(message);
	while (enumerator->enumerate(enumerator, &payload))
	{
		if (payload->get_type(payload) == PLV2_NOTIFY)
		{
			notify_payload_t *notify = (notify_payload_t*)payload;

			switch (notify->get_notify_type(notify))
			{
				case INTERNAL_ADDRESS_FAILURE:
				case FAILED_CP_REQUIRED:
				{
					DBG1(DBG_IKE,"configuration payload negotiation "
						 "failed, no CHILD_SA built");
					enumerator->destroy(enumerator);
					handle_child_sa_failure(this, message);
					return SUCCESS;
				}
				default:
					break;
			}
		}
	}
	enumerator->destroy(enumerator);

	this->child_sa = child_sa_create(this->ike_sa->get_my_host(this->ike_sa),
			this->ike_sa->get_other_host(this->ike_sa), this->config, this->reqid,
			this->ike_sa->has_condition(this->ike_sa, COND_NAT_ANY));

	if (this->ipcomp_received != IPCOMP_NONE)
	{
		if (this->config->use_ipcomp(this->config))
		{
			add_ipcomp_notify(this, message, this->ipcomp_received);
		}
		else
		{
			DBG1(DBG_IKE, "received %N notify but IPComp is disabled, ignoring",
				 notify_type_names, IPCOMP_SUPPORTED);
		}
	}

	switch (select_and_install(this, no_dh, ike_auth))
	{
		case SUCCESS:
			break;
		case NOT_FOUND:
			message->add_notify(message, FALSE, TS_UNACCEPTABLE, chunk_empty);
			handle_child_sa_failure(this, message);
			return SUCCESS;
		case INVALID_ARG:
		{
			u_int16_t group = htons(this->dh_group);
			message->add_notify(message, FALSE, INVALID_KE_PAYLOAD,
								chunk_from_thing(group));
			handle_child_sa_failure(this, message);
			return SUCCESS;
		}
		case FAILED:
		default:
			message->add_notify(message, FALSE, NO_PROPOSAL_CHOSEN, chunk_empty);
			handle_child_sa_failure(this, message);
			return SUCCESS;
	}

	build_payloads(this, message);

	if (!this->rekey)
	{	/* invoke the child_up() hook if we are not rekeying */
		charon->bus->child_updown(charon->bus, this->child_sa, TRUE);
	}
	return SUCCESS;
}

/**
 * Raise alerts for received notify errors
 */
static void raise_alerts(private_child_create_t *this, notify_type_t type)
{
	linked_list_t *list;

	switch (type)
	{
		case NO_PROPOSAL_CHOSEN:
			list = this->config->get_proposals(this->config, FALSE);
			charon->bus->alert(charon->bus, ALERT_PROPOSAL_MISMATCH_CHILD, list);
			list->destroy_offset(list, offsetof(proposal_t, destroy));
			break;
		default:
			break;
	}
}

METHOD(task_t, build_i_delete, status_t,
	private_child_create_t *this, message_t *message)
{
	message->set_exchange_type(message, INFORMATIONAL);
	if (this->child_sa && this->proposal)
	{
		protocol_id_t proto;
		delete_payload_t *del;
		u_int32_t spi;

		proto = this->proposal->get_protocol(this->proposal);
		spi = this->child_sa->get_spi(this->child_sa, TRUE);
		del = delete_payload_create(PLV2_DELETE, proto);
		del->add_spi(del, spi);
		message->add_payload(message, (payload_t*)del);

		DBG1(DBG_IKE, "sending DELETE for %N CHILD_SA with SPI %.8x",
			 protocol_id_names, proto, ntohl(spi));
	}
	return NEED_MORE;
}

/**
 * Change task to delete the failed CHILD_SA as initiator
 */
static status_t delete_failed_sa(private_child_create_t *this)
{
	this->public.task.build = _build_i_delete;
	this->public.task.process = (void*)return_success;
	return NEED_MORE;
}

METHOD(task_t, process_i, status_t,
	private_child_create_t *this, message_t *message)
{
	enumerator_t *enumerator;
	payload_t *payload;
	bool no_dh = TRUE, ike_auth = FALSE;

	switch (message->get_exchange_type(message))
	{
		case IKE_SA_INIT:
			return get_nonce(message, &this->other_nonce);
		case CREATE_CHILD_SA:
			get_nonce(message, &this->other_nonce);
			no_dh = FALSE;
			break;
		case IKE_AUTH:
			if (this->ike_sa->get_state(this->ike_sa) != IKE_ESTABLISHED)
			{	/* wait until all authentication round completed */
				return NEED_MORE;
			}
			ike_auth = TRUE;
		default:
			break;
	}

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
				/* handle notify errors related to CHILD_SA only */
				case NO_PROPOSAL_CHOSEN:
				case SINGLE_PAIR_REQUIRED:
				case NO_ADDITIONAL_SAS:
				case INTERNAL_ADDRESS_FAILURE:
				case FAILED_CP_REQUIRED:
				case TS_UNACCEPTABLE:
				case INVALID_SELECTORS:
				{
					DBG1(DBG_IKE, "received %N notify, no CHILD_SA built",
						 notify_type_names, type);
					enumerator->destroy(enumerator);
					raise_alerts(this, type);
					handle_child_sa_failure(this, message);
					/* an error in CHILD_SA creation is not critical */
					return SUCCESS;
				}
				case INVALID_KE_PAYLOAD:
				{
					chunk_t data;
					u_int16_t group = MODP_NONE;

					data = notify->get_notification_data(notify);
					if (data.len == sizeof(group))
					{
						memcpy(&group, data.ptr, data.len);
						group = ntohs(group);
					}
					DBG1(DBG_IKE, "peer didn't accept DH group %N, "
						 "it requested %N", diffie_hellman_group_names,
						 this->dh_group, diffie_hellman_group_names, group);
					this->retry = TRUE;
					this->dh_group = group;
					this->public.task.migrate(&this->public.task, this->ike_sa);
					enumerator->destroy(enumerator);
					return NEED_MORE;
				}
				default:
				{
					if (message->get_exchange_type(message) == CREATE_CHILD_SA)
					{	/* handle notifies if not handled in IKE_AUTH */
						if (type <= 16383)
						{
							DBG1(DBG_IKE, "received %N notify error",
								 notify_type_names, type);
							enumerator->destroy(enumerator);
							return SUCCESS;
						}
						DBG2(DBG_IKE, "received %N notify",
							 notify_type_names, type);
					}
					break;
				}
			}
		}
	}
	enumerator->destroy(enumerator);

	process_payloads(this, message);

	if (this->ipcomp == IPCOMP_NONE && this->ipcomp_received != IPCOMP_NONE)
	{
		DBG1(DBG_IKE, "received an IPCOMP_SUPPORTED notify without requesting"
			 " one, no CHILD_SA built");
		handle_child_sa_failure(this, message);
		return delete_failed_sa(this);
	}
	else if (this->ipcomp != IPCOMP_NONE && this->ipcomp_received == IPCOMP_NONE)
	{
		DBG1(DBG_IKE, "peer didn't accept our proposed IPComp transforms, "
			 "IPComp is disabled");
		this->ipcomp = IPCOMP_NONE;
	}
	else if (this->ipcomp != IPCOMP_NONE && this->ipcomp != this->ipcomp_received)
	{
		DBG1(DBG_IKE, "received an IPCOMP_SUPPORTED notify we didn't propose, "
			 "no CHILD_SA built");
		handle_child_sa_failure(this, message);
		return delete_failed_sa(this);
	}

	if (select_and_install(this, no_dh, ike_auth) == SUCCESS)
	{
		if (!this->rekey)
		{	/* invoke the child_up() hook if we are not rekeying */
			charon->bus->child_updown(charon->bus, this->child_sa, TRUE);
		}
	}
	else
	{
		handle_child_sa_failure(this, message);
		return delete_failed_sa(this);
	}
	return SUCCESS;
}

METHOD(child_create_t, use_reqid, void,
	private_child_create_t *this, u_int32_t reqid)
{
	this->reqid = reqid;
}

METHOD(child_create_t, get_child, child_sa_t*,
	private_child_create_t *this)
{
	return this->child_sa;
}

METHOD(child_create_t, set_config, void,
	private_child_create_t *this, child_cfg_t *cfg)
{
	DESTROY_IF(this->config);
	this->config = cfg;
}

METHOD(child_create_t, get_lower_nonce, chunk_t,
	private_child_create_t *this)
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

METHOD(task_t, get_type, task_type_t,
	private_child_create_t *this)
{
	return TASK_CHILD_CREATE;
}

METHOD(task_t, migrate, void,
	private_child_create_t *this, ike_sa_t *ike_sa)
{
	chunk_free(&this->my_nonce);
	chunk_free(&this->other_nonce);
	if (this->tsr)
	{
		this->tsr->destroy_offset(this->tsr, offsetof(traffic_selector_t, destroy));
	}
	if (this->tsi)
	{
		this->tsi->destroy_offset(this->tsi, offsetof(traffic_selector_t, destroy));
	}
	DESTROY_IF(this->child_sa);
	DESTROY_IF(this->proposal);
	DESTROY_IF(this->dh);
	if (this->proposals)
	{
		this->proposals->destroy_offset(this->proposals, offsetof(proposal_t, destroy));
	}

	this->ike_sa = ike_sa;
	this->keymat = (keymat_v2_t*)ike_sa->get_keymat(ike_sa);
	this->proposal = NULL;
	this->proposals = NULL;
	this->tsi = NULL;
	this->tsr = NULL;
	this->dh = NULL;
	this->child_sa = NULL;
	this->mode = MODE_TUNNEL;
	this->ipcomp = IPCOMP_NONE;
	this->ipcomp_received = IPCOMP_NONE;
	this->other_cpi = 0;
	this->reqid = 0;
	this->established = FALSE;
}

METHOD(task_t, destroy, void,
	private_child_create_t *this)
{
	chunk_free(&this->my_nonce);
	chunk_free(&this->other_nonce);
	if (this->tsr)
	{
		this->tsr->destroy_offset(this->tsr, offsetof(traffic_selector_t, destroy));
	}
	if (this->tsi)
	{
		this->tsi->destroy_offset(this->tsi, offsetof(traffic_selector_t, destroy));
	}
	if (!this->established)
	{
		DESTROY_IF(this->child_sa);
	}
	DESTROY_IF(this->packet_tsi);
	DESTROY_IF(this->packet_tsr);
	DESTROY_IF(this->proposal);
	DESTROY_IF(this->dh);
	if (this->proposals)
	{
		this->proposals->destroy_offset(this->proposals, offsetof(proposal_t, destroy));
	}

	DESTROY_IF(this->config);
	free(this);
}

/*
 * Described in header.
 */
child_create_t *child_create_create(ike_sa_t *ike_sa,
							child_cfg_t *config, bool rekey,
							traffic_selector_t *tsi, traffic_selector_t *tsr)
{
	private_child_create_t *this;

	INIT(this,
		.public = {
			.get_child = _get_child,
			.set_config = _set_config,
			.get_lower_nonce = _get_lower_nonce,
			.use_reqid = _use_reqid,
			.task = {
				.get_type = _get_type,
				.migrate = _migrate,
				.destroy = _destroy,
			},
		},
		.ike_sa = ike_sa,
		.config = config,
		.packet_tsi = tsi ? tsi->clone(tsi) : NULL,
		.packet_tsr = tsr ? tsr->clone(tsr) : NULL,
		.dh_group = MODP_NONE,
		.keymat = (keymat_v2_t*)ike_sa->get_keymat(ike_sa),
		.mode = MODE_TUNNEL,
		.tfcv3 = TRUE,
		.ipcomp = IPCOMP_NONE,
		.ipcomp_received = IPCOMP_NONE,
		.rekey = rekey,
		.retry = FALSE,
	);

	if (config)
	{
		this->public.task.build = _build_i;
		this->public.task.process = _process_i;
		this->initiator = TRUE;
	}
	else
	{
		this->public.task.build = _build_r;
		this->public.task.process = _process_r;
		this->initiator = FALSE;
	}

	return &this->public;
}
