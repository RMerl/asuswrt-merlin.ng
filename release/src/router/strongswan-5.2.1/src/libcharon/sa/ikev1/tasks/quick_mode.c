/*
 * Copyright (C) 2012 Tobias Brunner
 * Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2011 Martin Willi
 * Copyright (C) 2011 revosec AG
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

/*
 * Copyright (C) 2012 Volker Rümelin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "quick_mode.h"

#include <string.h>

#include <daemon.h>
#include <sa/ikev1/keymat_v1.h>
#include <encoding/payloads/sa_payload.h>
#include <encoding/payloads/nonce_payload.h>
#include <encoding/payloads/ke_payload.h>
#include <encoding/payloads/id_payload.h>
#include <encoding/payloads/payload.h>
#include <sa/ikev1/tasks/informational.h>
#include <sa/ikev1/tasks/quick_delete.h>
#include <processing/jobs/inactivity_job.h>

typedef struct private_quick_mode_t private_quick_mode_t;

/**
 * Private members of a quick_mode_t task.
 */
struct private_quick_mode_t {

	/**
	 * Public methods and task_t interface.
	 */
	quick_mode_t public;

	/**
	 * Assigned IKE_SA.
	 */
	ike_sa_t *ike_sa;

	/**
	 * TRUE if we are initiating quick mode
	 */
	bool initiator;

	/**
	 * Traffic selector of initiator
	 */
	traffic_selector_t *tsi;

	/**
	 * Traffic selector of responder
	 */
	traffic_selector_t *tsr;

	/**
	 * Initiators nonce
	 */
	chunk_t nonce_i;

	/**
	 * Responder nonce
	 */
	chunk_t nonce_r;

	/**
	 * Initiators ESP SPI
	 */
	u_int32_t spi_i;

	/**
	 * Responder ESP SPI
	 */
	u_int32_t spi_r;

	/**
	 * Initiators IPComp CPI
	 */
	u_int16_t cpi_i;

	/**
	 * Responders IPComp CPI
	 */
	u_int16_t cpi_r;

	/**
	 * selected CHILD_SA proposal
	 */
	proposal_t *proposal;

	/**
	 * Config of CHILD_SA to establish
	 */
	child_cfg_t *config;

	/**
	 * CHILD_SA we are about to establish
	 */
	child_sa_t *child_sa;

	/**
	 * IKEv1 keymat
	 */
	keymat_v1_t *keymat;

	/**
	 * DH exchange, when PFS is in use
	 */
	diffie_hellman_t *dh;

	/**
	 * Negotiated lifetime of new SA
	 */
	u_int32_t lifetime;

	/**
	 * Negotaited lifebytes of new SA
	 */
	u_int64_t lifebytes;

	/**
	 * Reqid to use, 0 for auto-allocate
	 */
	u_int32_t reqid;

	/**
	 * SPI of SA we rekey
	 */
	u_int32_t rekey;

	/**
	 * Negotiated mode, tunnel or transport
	 */
	ipsec_mode_t mode;

	/*
	 * SA protocol (ESP|AH) negotiated
	 */
	protocol_id_t proto;

	/**
	 * Use UDP encapsulation
	 */
	bool udp;

	/** states of quick mode */
	enum {
		QM_INIT,
		QM_NEGOTIATED,
	} state;
};

/**
 * Schedule inactivity timeout for CHILD_SA with reqid, if enabled
 */
static void schedule_inactivity_timeout(private_quick_mode_t *this)
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
 * Install negotiated CHILD_SA
 */
static bool install(private_quick_mode_t *this)
{
	status_t status, status_i, status_o;
	chunk_t encr_i, encr_r, integ_i, integ_r;
	linked_list_t *tsi, *tsr, *my_ts, *other_ts;
	child_sa_t *old = NULL;

	this->child_sa->set_proposal(this->child_sa, this->proposal);
	this->child_sa->set_state(this->child_sa, CHILD_INSTALLING);
	this->child_sa->set_mode(this->child_sa, this->mode);

	if (this->cpi_i && this->cpi_r)
	{	/* DEFLATE is the only transform we currently support */
		this->child_sa->set_ipcomp(this->child_sa, IPCOMP_DEFLATE);
	}
	else
	{
		this->cpi_i = this->cpi_r = 0;
	}

	this->child_sa->set_protocol(this->child_sa,
								 this->proposal->get_protocol(this->proposal));

	status_i = status_o = FAILED;
	encr_i = encr_r = integ_i = integ_r = chunk_empty;
	tsi = linked_list_create_with_items(this->tsi->clone(this->tsi), NULL);
	tsr = linked_list_create_with_items(this->tsr->clone(this->tsr), NULL);
	if (this->initiator)
	{
		charon->bus->narrow(charon->bus, this->child_sa,
							NARROW_INITIATOR_POST_AUTH, tsi, tsr);
	}
	else
	{
		charon->bus->narrow(charon->bus, this->child_sa,
							NARROW_RESPONDER_POST, tsr, tsi);
	}
	if (tsi->get_count(tsi) == 0 || tsr->get_count(tsr) == 0)
	{
		tsi->destroy_offset(tsi, offsetof(traffic_selector_t, destroy));
		tsr->destroy_offset(tsr, offsetof(traffic_selector_t, destroy));
		DBG1(DBG_IKE, "no acceptable traffic selectors found");
		return FALSE;
	}

	if (this->keymat->derive_child_keys(this->keymat, this->proposal, this->dh,
						this->spi_i, this->spi_r, this->nonce_i, this->nonce_r,
						&encr_i, &integ_i, &encr_r, &integ_r))
	{
		if (this->initiator)
		{
			status_i = this->child_sa->install(this->child_sa,
									encr_r, integ_r, this->spi_i, this->cpi_i,
									this->initiator, TRUE, FALSE, tsi, tsr);
			status_o = this->child_sa->install(this->child_sa,
									encr_i, integ_i, this->spi_r, this->cpi_r,
									this->initiator, FALSE, FALSE, tsi, tsr);
		}
		else
		{
			status_i = this->child_sa->install(this->child_sa,
									encr_i, integ_i, this->spi_r, this->cpi_r,
									this->initiator, TRUE, FALSE, tsr, tsi);
			status_o = this->child_sa->install(this->child_sa,
									encr_r, integ_r, this->spi_i, this->cpi_i,
									this->initiator, FALSE, FALSE, tsr, tsi);
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
		tsi->destroy_offset(tsi, offsetof(traffic_selector_t, destroy));
		tsr->destroy_offset(tsr, offsetof(traffic_selector_t, destroy));
		return FALSE;
	}

	if (this->initiator)
	{
		status = this->child_sa->add_policies(this->child_sa, tsi, tsr);
	}
	else
	{
		status = this->child_sa->add_policies(this->child_sa, tsr, tsi);
	}
	tsi->destroy_offset(tsi, offsetof(traffic_selector_t, destroy));
	tsr->destroy_offset(tsr, offsetof(traffic_selector_t, destroy));
	if (status != SUCCESS)
	{
		DBG1(DBG_IKE, "unable to install IPsec policies (SPD) in kernel");
		return FALSE;
	}

	charon->bus->child_keys(charon->bus, this->child_sa, this->initiator,
							this->dh, this->nonce_i, this->nonce_r);

	/* add to IKE_SA, and remove from task */
	this->child_sa->set_state(this->child_sa, CHILD_INSTALLED);
	this->ike_sa->add_child_sa(this->ike_sa, this->child_sa);

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

	if (this->rekey)
	{
		old = this->ike_sa->get_child_sa(this->ike_sa,
								this->proposal->get_protocol(this->proposal),
								this->rekey, TRUE);
	}
	if (old)
	{
		charon->bus->child_rekey(charon->bus, old, this->child_sa);
	}
	else
	{
		charon->bus->child_updown(charon->bus, this->child_sa, TRUE);
	}
	if (!this->rekey)
	{
		schedule_inactivity_timeout(this);
	}
	this->child_sa = NULL;
	return TRUE;
}

/**
 * Generate and add NONCE
 */
static bool add_nonce(private_quick_mode_t *this, chunk_t *nonce,
					  message_t *message)
{
	nonce_payload_t *nonce_payload;
	nonce_gen_t *nonceg;

	nonceg = this->keymat->keymat.create_nonce_gen(&this->keymat->keymat);
	if (!nonceg)
	{
		DBG1(DBG_IKE, "no nonce generator found to create nonce");
		return FALSE;
	}
	if (!nonceg->allocate_nonce(nonceg, NONCE_SIZE, nonce))
	{
		DBG1(DBG_IKE, "nonce allocation failed");
		nonceg->destroy(nonceg);
		return FALSE;
	}
	nonceg->destroy(nonceg);

	nonce_payload = nonce_payload_create(PLV1_NONCE);
	nonce_payload->set_nonce(nonce_payload, *nonce);
	message->add_payload(message, &nonce_payload->payload_interface);

	return TRUE;
}

/**
 * Extract nonce from NONCE payload
 */
static bool get_nonce(private_quick_mode_t *this, chunk_t *nonce,
					  message_t *message)
{
	nonce_payload_t *nonce_payload;

	nonce_payload = (nonce_payload_t*)message->get_payload(message, PLV1_NONCE);
	if (!nonce_payload)
	{
		DBG1(DBG_IKE, "NONCE payload missing in message");
		return FALSE;
	}
	*nonce = nonce_payload->get_nonce(nonce_payload);

	return TRUE;
}

/**
 * Add KE payload to message
 */
static void add_ke(private_quick_mode_t *this, message_t *message)
{
	ke_payload_t *ke_payload;

	ke_payload = ke_payload_create_from_diffie_hellman(PLV1_KEY_EXCHANGE, this->dh);
	message->add_payload(message, &ke_payload->payload_interface);
}

/**
 * Get DH value from a KE payload
 */
static bool get_ke(private_quick_mode_t *this, message_t *message)
{
	ke_payload_t *ke_payload;

	ke_payload = (ke_payload_t*)message->get_payload(message, PLV1_KEY_EXCHANGE);
	if (!ke_payload)
	{
		DBG1(DBG_IKE, "KE payload missing");
		return FALSE;
	}
	this->dh->set_other_public_value(this->dh,
								ke_payload->get_key_exchange_data(ke_payload));
	return TRUE;
}

/**
 * Select a traffic selector from configuration
 */
static traffic_selector_t* select_ts(private_quick_mode_t *this, bool local,
									 linked_list_t *supplied)
{
	traffic_selector_t *ts;
	linked_list_t *list, *hosts;

	hosts = get_dynamic_hosts(this->ike_sa, local);
	list = this->config->get_traffic_selectors(this->config,
											   local, supplied, hosts);
	hosts->destroy(hosts);
	if (list->get_first(list, (void**)&ts) == SUCCESS)
	{
		ts = ts->clone(ts);
	}
	else
	{
		DBG1(DBG_IKE, "%s traffic selector missing in configuration",
			 local ? "local" : "remote");
		ts = NULL;
	}
	list->destroy_offset(list, offsetof(traffic_selector_t, destroy));
	return ts;
}

/**
 * Add selected traffic selectors to message
 */
static void add_ts(private_quick_mode_t *this, message_t *message)
{
	id_payload_t *id_payload;

	id_payload = id_payload_create_from_ts(this->tsi);
	message->add_payload(message, &id_payload->payload_interface);
	id_payload = id_payload_create_from_ts(this->tsr);
	message->add_payload(message, &id_payload->payload_interface);
}

/**
 * Get traffic selectors from received message
 */
static bool get_ts(private_quick_mode_t *this, message_t *message)
{
	traffic_selector_t *tsi = NULL, *tsr = NULL;
	enumerator_t *enumerator;
	id_payload_t *id_payload;
	payload_t *payload;
	host_t *hsi, *hsr;
	bool first = TRUE;

	enumerator = message->create_payload_enumerator(message);
	while (enumerator->enumerate(enumerator, &payload))
	{
		if (payload->get_type(payload) == PLV1_ID)
		{
			id_payload = (id_payload_t*)payload;

			if (first)
			{
				tsi = id_payload->get_ts(id_payload);
				first = FALSE;
			}
			else
			{
				tsr = id_payload->get_ts(id_payload);
				break;
			}
		}
	}
	enumerator->destroy(enumerator);

	/* create host2host selectors if ID payloads missing */
	if (this->initiator)
	{
		hsi = this->ike_sa->get_my_host(this->ike_sa);
		hsr = this->ike_sa->get_other_host(this->ike_sa);
	}
	else
	{
		hsr = this->ike_sa->get_my_host(this->ike_sa);
		hsi = this->ike_sa->get_other_host(this->ike_sa);
	}
	if (!tsi)
	{
		tsi = traffic_selector_create_from_subnet(hsi->clone(hsi),
					hsi->get_family(hsi) == AF_INET ? 32 : 128, 0, 0, 65535);
	}
	if (!tsr)
	{
		tsr = traffic_selector_create_from_subnet(hsr->clone(hsr),
					hsr->get_family(hsr) == AF_INET ? 32 : 128, 0, 0, 65535);
	}
	if (this->mode == MODE_TRANSPORT && this->udp &&
	   (!tsi->is_host(tsi, hsi) || !tsr->is_host(tsr, hsr)))
	{	/* change TS in case of a NAT in transport mode */
		DBG2(DBG_IKE, "changing received traffic selectors %R=== %R due to NAT",
			 tsi, tsr);
		tsi->set_address(tsi, hsi);
		tsr->set_address(tsr, hsr);
	}

	if (this->initiator)
	{
		traffic_selector_t *tsisub, *tsrsub;

		/* check if peer selection is valid */
		tsisub = this->tsi->get_subset(this->tsi, tsi);
		tsrsub = this->tsr->get_subset(this->tsr, tsr);
		if (!tsisub || !tsrsub)
		{
			DBG1(DBG_IKE, "peer selected invalid traffic selectors: "
				 "%R for %R, %R for %R", tsi, this->tsi, tsr, this->tsr);
			DESTROY_IF(tsisub);
			DESTROY_IF(tsrsub);
			tsi->destroy(tsi);
			tsr->destroy(tsr);
			return FALSE;
		}
		tsi->destroy(tsi);
		tsr->destroy(tsr);
		this->tsi->destroy(this->tsi);
		this->tsr->destroy(this->tsr);
		this->tsi = tsisub;
		this->tsr = tsrsub;
	}
	else
	{
		this->tsi = tsi;
		this->tsr = tsr;
	}
	return TRUE;
}

/**
 * Get encap
 */
static encap_t get_encap(ike_sa_t* ike_sa, bool udp)
{
	if (!udp)
	{
		return ENCAP_NONE;
	}
	if (ike_sa->supports_extension(ike_sa, EXT_NATT_DRAFT_02_03))
	{
		return ENCAP_UDP_DRAFT_00_03;
	}
	return ENCAP_UDP;
}

/**
 * Get NAT-OA payload type (RFC 3947 or RFC 3947 drafts).
 */
static payload_type_t get_nat_oa_payload_type(ike_sa_t *ike_sa)
{
	if (ike_sa->supports_extension(ike_sa, EXT_NATT_DRAFT_02_03))
	{
		return PLV1_NAT_OA_DRAFT_00_03;
	}
	return PLV1_NAT_OA;
}

/**
 * Add NAT-OA payloads
 */
static void add_nat_oa_payloads(private_quick_mode_t *this, message_t *message)
{
	identification_t *id;
	id_payload_t *nat_oa;
	host_t *src, *dst;
	payload_type_t nat_oa_payload_type;

	src = message->get_source(message);
	dst = message->get_destination(message);

	src = this->initiator ? src : dst;
	dst = this->initiator ? dst : src;

	nat_oa_payload_type = get_nat_oa_payload_type(this->ike_sa);

	/* first NAT-OA is the initiator's address */
	id = identification_create_from_sockaddr(src->get_sockaddr(src));
	nat_oa = id_payload_create_from_identification(nat_oa_payload_type, id);
	message->add_payload(message, (payload_t*)nat_oa);
	id->destroy(id);

	/* second NAT-OA is that of the responder */
	id = identification_create_from_sockaddr(dst->get_sockaddr(dst));
	nat_oa = id_payload_create_from_identification(nat_oa_payload_type, id);
	message->add_payload(message, (payload_t*)nat_oa);
	id->destroy(id);
}

/**
 * Look up lifetimes
 */
static void get_lifetimes(private_quick_mode_t *this)
{
	lifetime_cfg_t *lft;

	lft = this->config->get_lifetime(this->config);
	if (lft->time.life)
	{
		this->lifetime = lft->time.life;
	}
	else if (lft->bytes.life)
	{
		this->lifebytes = lft->bytes.life;
	}
	free(lft);
}

/**
 * Check and apply lifetimes
 */
static void apply_lifetimes(private_quick_mode_t *this, sa_payload_t *sa_payload)
{
	u_int32_t lifetime;
	u_int64_t lifebytes;

	lifetime = sa_payload->get_lifetime(sa_payload);
	lifebytes = sa_payload->get_lifebytes(sa_payload);
	if (this->lifetime != lifetime)
	{
		DBG1(DBG_IKE, "received %us lifetime, configured %us",
			 lifetime, this->lifetime);
		this->lifetime = lifetime;
	}
	if (this->lifebytes != lifebytes)
	{
		DBG1(DBG_IKE, "received %llu lifebytes, configured %llu",
			 lifebytes, this->lifebytes);
		this->lifebytes = lifebytes;
	}
}

/**
 * Set the task ready to build notify error message
 */
static status_t send_notify(private_quick_mode_t *this, notify_type_t type)
{
	notify_payload_t *notify;

	notify = notify_payload_create_from_protocol_and_type(PLV1_NOTIFY,
														  this->proto, type);
	notify->set_spi(notify, this->spi_i);

	this->ike_sa->queue_task(this->ike_sa,
						(task_t*)informational_create(this->ike_sa, notify));
	/* cancel all active/passive tasks in favour of informational */
	this->ike_sa->flush_queue(this->ike_sa,
					this->initiator ? TASK_QUEUE_ACTIVE : TASK_QUEUE_PASSIVE);
	return ALREADY_DONE;
}

/**
 * Prepare a list of proposals from child_config containing only the specified
 * DH group, unless it is set to MODP_NONE.
 */
static linked_list_t *get_proposals(private_quick_mode_t *this,
									diffie_hellman_group_t group)
{
	linked_list_t *list;
	proposal_t *proposal;
	enumerator_t *enumerator;

	list = this->config->get_proposals(this->config, FALSE);
	enumerator = list->create_enumerator(list);
	while (enumerator->enumerate(enumerator, &proposal))
	{
		if (group != MODP_NONE)
		{
			if (!proposal->has_dh_group(proposal, group))
			{
				list->remove_at(list, enumerator);
				proposal->destroy(proposal);
				continue;
			}
			proposal->strip_dh(proposal, group);
		}
		proposal->set_spi(proposal, this->spi_i);
	}
	enumerator->destroy(enumerator);

	return list;
}

METHOD(task_t, build_i, status_t,
	private_quick_mode_t *this, message_t *message)
{
	switch (this->state)
	{
		case QM_INIT:
		{
			sa_payload_t *sa_payload;
			linked_list_t *list, *tsi, *tsr;
			proposal_t *proposal;
			diffie_hellman_group_t group;
			encap_t encap;

			this->udp = this->ike_sa->has_condition(this->ike_sa, COND_NAT_ANY);
			this->mode = this->config->get_mode(this->config);
			this->child_sa = child_sa_create(
									this->ike_sa->get_my_host(this->ike_sa),
									this->ike_sa->get_other_host(this->ike_sa),
									this->config, this->reqid, this->udp);

			if (this->udp && this->mode == MODE_TRANSPORT)
			{
				/* TODO-IKEv1: disable NAT-T for TRANSPORT mode by default? */
				add_nat_oa_payloads(this, message);
			}

			if (this->config->use_ipcomp(this->config))
			{
				this->cpi_i = this->child_sa->alloc_cpi(this->child_sa);
				if (!this->cpi_i)
				{
					DBG1(DBG_IKE, "unable to allocate a CPI from kernel, "
						 "IPComp disabled");
				}
			}

			list = this->config->get_proposals(this->config, MODP_NONE);
			if (list->get_first(list, (void**)&proposal) == SUCCESS)
			{
				this->proto = proposal->get_protocol(proposal);
			}
			list->destroy_offset(list, offsetof(proposal_t, destroy));
			this->spi_i = this->child_sa->alloc_spi(this->child_sa, this->proto);
			if (!this->spi_i)
			{
				DBG1(DBG_IKE, "allocating SPI from kernel failed");
				return FAILED;
			}

			group = this->config->get_dh_group(this->config);
			if (group != MODP_NONE)
			{
				proposal_t *proposal;
				u_int16_t preferred_group;

				proposal = this->ike_sa->get_proposal(this->ike_sa);
				proposal->get_algorithm(proposal, DIFFIE_HELLMAN_GROUP,
										&preferred_group, NULL);
				/* try the negotiated DH group from IKE_SA */
				list = get_proposals(this, preferred_group);
				if (list->get_count(list))
				{
					group = preferred_group;
				}
				else
				{
					/* fall back to the first configured DH group */
					list->destroy(list);
					list = get_proposals(this, group);
				}

				this->dh = this->keymat->keymat.create_dh(&this->keymat->keymat,
														  group);
				if (!this->dh)
				{
					DBG1(DBG_IKE, "configured DH group %N not supported",
						 diffie_hellman_group_names, group);
					list->destroy_offset(list, offsetof(proposal_t, destroy));
					return FAILED;
				}
			}
			else
			{
				list = get_proposals(this, MODP_NONE);
			}

			get_lifetimes(this);
			encap = get_encap(this->ike_sa, this->udp);
			sa_payload = sa_payload_create_from_proposals_v1(list,
								this->lifetime, this->lifebytes, AUTH_NONE,
								this->mode, encap, this->cpi_i);
			list->destroy_offset(list, offsetof(proposal_t, destroy));
			message->add_payload(message, &sa_payload->payload_interface);

			if (!add_nonce(this, &this->nonce_i, message))
			{
				return FAILED;
			}
			if (group != MODP_NONE)
			{
				add_ke(this, message);
			}
			if (!this->tsi)
			{
				this->tsi = select_ts(this, TRUE, NULL);
			}
			if (!this->tsr)
			{
				this->tsr = select_ts(this, FALSE, NULL);
			}
			tsi = linked_list_create_with_items(this->tsi, NULL);
			tsr = linked_list_create_with_items(this->tsr, NULL);
			this->tsi = this->tsr = NULL;
			charon->bus->narrow(charon->bus, this->child_sa,
								NARROW_INITIATOR_PRE_AUTH, tsi, tsr);
			tsi->remove_first(tsi, (void**)&this->tsi);
			tsr->remove_first(tsr, (void**)&this->tsr);
			tsi->destroy_offset(tsi, offsetof(traffic_selector_t, destroy));
			tsr->destroy_offset(tsr, offsetof(traffic_selector_t, destroy));
			if (!this->tsi || !this->tsr)
			{
				return FAILED;
			}
			add_ts(this, message);
			return NEED_MORE;
		}
		case QM_NEGOTIATED:
		{
			return SUCCESS;
		}
		default:
			return FAILED;
	}
}

/**
 * Check for notify errors, return TRUE if error found
 */
static bool has_notify_errors(private_quick_mode_t *this, message_t *message)
{
	enumerator_t *enumerator;
	payload_t *payload;
	bool err = FALSE;

	enumerator = message->create_payload_enumerator(message);
	while (enumerator->enumerate(enumerator, &payload))
	{
		if (payload->get_type(payload) == PLV1_NOTIFY)
		{
			notify_payload_t *notify;
			notify_type_t type;

			notify = (notify_payload_t*)payload;
			type = notify->get_notify_type(notify);
			if (type < 16384)
			{

				DBG1(DBG_IKE, "received %N error notify",
					 notify_type_names, type);
				err = TRUE;
			}
			else
			{
				DBG1(DBG_IKE, "received %N notify", notify_type_names, type);
			}
		}
	}
	enumerator->destroy(enumerator);

	return err;
}

/**
 * Check if this is a rekey for an existing CHILD_SA, reuse reqid if so
 */
static void check_for_rekeyed_child(private_quick_mode_t *this)
{
	enumerator_t *enumerator, *policies;
	traffic_selector_t *local, *remote;
	child_sa_t *child_sa;
	proposal_t *proposal;
	char *name;

	name = this->config->get_name(this->config);
	enumerator = this->ike_sa->create_child_sa_enumerator(this->ike_sa);
	while (this->reqid == 0 && enumerator->enumerate(enumerator, &child_sa))
	{
		if (streq(child_sa->get_name(child_sa), name))
		{
			proposal = child_sa->get_proposal(child_sa);
			switch (child_sa->get_state(child_sa))
			{
				case CHILD_INSTALLED:
				case CHILD_REKEYING:
					policies = child_sa->create_policy_enumerator(child_sa);
					if (policies->enumerate(policies, &local, &remote) &&
						local->equals(local, this->tsr) &&
						remote->equals(remote, this->tsi) &&
						this->proposal->equals(this->proposal, proposal))
					{
						this->reqid = child_sa->get_reqid(child_sa);
						this->rekey = child_sa->get_spi(child_sa, TRUE);
						child_sa->set_state(child_sa, CHILD_REKEYING);
						DBG1(DBG_IKE, "detected rekeying of CHILD_SA %s{%u}",
							 child_sa->get_name(child_sa), this->reqid);
					}
					policies->destroy(policies);
				break;
			default:
				break;
			}
		}
	}
	enumerator->destroy(enumerator);
}

METHOD(task_t, process_r, status_t,
	private_quick_mode_t *this, message_t *message)
{
	switch (this->state)
	{
		case QM_INIT:
		{
			sa_payload_t *sa_payload;
			linked_list_t *tsi, *tsr, *hostsi, *hostsr, *list = NULL;
			peer_cfg_t *peer_cfg;
			u_int16_t group;
			bool private;

			sa_payload = (sa_payload_t*)message->get_payload(message,
													PLV1_SECURITY_ASSOCIATION);
			if (!sa_payload)
			{
				DBG1(DBG_IKE, "sa payload missing");
				return send_notify(this, INVALID_PAYLOAD_TYPE);
			}

			this->mode = sa_payload->get_encap_mode(sa_payload, &this->udp);

			if (!get_ts(this, message))
			{
				return FAILED;
			}
			peer_cfg = this->ike_sa->get_peer_cfg(this->ike_sa);
			tsi = linked_list_create_with_items(this->tsi, NULL);
			tsr = linked_list_create_with_items(this->tsr, NULL);
			this->tsi = this->tsr = NULL;
			hostsi = get_dynamic_hosts(this->ike_sa, FALSE);
			hostsr = get_dynamic_hosts(this->ike_sa, TRUE);
			this->config = peer_cfg->select_child_cfg(peer_cfg, tsr, tsi,
													  hostsr, hostsi);
			hostsi->destroy(hostsi);
			hostsr->destroy(hostsr);
			if (this->config)
			{
				this->tsi = select_ts(this, FALSE, tsi);
				this->tsr = select_ts(this, TRUE, tsr);
			}
			tsi->destroy_offset(tsi, offsetof(traffic_selector_t, destroy));
			tsr->destroy_offset(tsr, offsetof(traffic_selector_t, destroy));
			if (!this->config || !this->tsi || !this->tsr ||
				this->mode != this->config->get_mode(this->config))
			{
				DBG1(DBG_IKE, "no matching CHILD_SA config found");
				return send_notify(this, INVALID_ID_INFORMATION);
			}

			if (this->config->use_ipcomp(this->config))
			{
				list = sa_payload->get_ipcomp_proposals(sa_payload,
														&this->cpi_i);
				if (!list->get_count(list))
				{
					DBG1(DBG_IKE, "expected IPComp proposal but peer did "
						 "not send one, IPComp disabled");
					this->cpi_i = 0;
				}
			}
			if (!list || !list->get_count(list))
			{
				DESTROY_IF(list);
				list = sa_payload->get_proposals(sa_payload);
			}
			private = this->ike_sa->supports_extension(this->ike_sa,
													   EXT_STRONGSWAN);
			this->proposal = this->config->select_proposal(this->config,
														   list, FALSE, private);
			list->destroy_offset(list, offsetof(proposal_t, destroy));

			get_lifetimes(this);
			apply_lifetimes(this, sa_payload);

			if (!this->proposal)
			{
				DBG1(DBG_IKE, "no matching proposal found, sending %N",
					 notify_type_names, NO_PROPOSAL_CHOSEN);
				return send_notify(this, NO_PROPOSAL_CHOSEN);
			}
			this->spi_i = this->proposal->get_spi(this->proposal);

			if (!get_nonce(this, &this->nonce_i, message))
			{
				return send_notify(this, INVALID_PAYLOAD_TYPE);
			}

			if (this->proposal->get_algorithm(this->proposal,
										DIFFIE_HELLMAN_GROUP, &group, NULL))
			{
				this->dh = this->keymat->keymat.create_dh(&this->keymat->keymat,
														  group);
				if (!this->dh)
				{
					DBG1(DBG_IKE, "negotiated DH group %N not supported",
						 diffie_hellman_group_names, group);
					return send_notify(this, INVALID_KEY_INFORMATION);
				}
				if (!get_ke(this, message))
				{
					return send_notify(this, INVALID_PAYLOAD_TYPE);
				}
			}

			check_for_rekeyed_child(this);

			this->child_sa = child_sa_create(
									this->ike_sa->get_my_host(this->ike_sa),
									this->ike_sa->get_other_host(this->ike_sa),
									this->config, this->reqid, this->udp);

			tsi = linked_list_create_with_items(this->tsi, NULL);
			tsr = linked_list_create_with_items(this->tsr, NULL);
			this->tsi = this->tsr = NULL;
			charon->bus->narrow(charon->bus, this->child_sa,
								NARROW_RESPONDER, tsr, tsi);
			if (tsi->remove_first(tsi, (void**)&this->tsi) != SUCCESS ||
				tsr->remove_first(tsr, (void**)&this->tsr) != SUCCESS)
			{
				tsi->destroy_offset(tsi, offsetof(traffic_selector_t, destroy));
				tsr->destroy_offset(tsr, offsetof(traffic_selector_t, destroy));
				return send_notify(this, INVALID_ID_INFORMATION);
			}
			tsi->destroy_offset(tsi, offsetof(traffic_selector_t, destroy));
			tsr->destroy_offset(tsr, offsetof(traffic_selector_t, destroy));

			return NEED_MORE;
		}
		case QM_NEGOTIATED:
		{
			if (has_notify_errors(this, message))
			{
				return SUCCESS;
			}
			if (message->get_exchange_type(message) == INFORMATIONAL_V1)
			{
				if (message->get_payload(message, PLV1_DELETE))
				{
					/* If the DELETE for a Quick Mode follows immediately
					 * after rekeying, we might receive it before the
					 * third completing Quick Mode message. Ignore it, as
					 * it gets handled by a separately queued delete task. */
					return NEED_MORE;
				}
				return SUCCESS;
			}
			if (!install(this))
			{
				ike_sa_t *ike_sa = this->ike_sa;
				task_t *task;

				task = (task_t*)quick_delete_create(this->ike_sa,
								this->proposal->get_protocol(this->proposal),
								this->spi_i, TRUE, TRUE);
				/* flush_queue() destroys the current task */
				ike_sa->flush_queue(ike_sa, TASK_QUEUE_PASSIVE);
				ike_sa->queue_task(ike_sa, task);
				return ALREADY_DONE;
			}
			return SUCCESS;
		}
		default:
			return FAILED;
	}
}

METHOD(task_t, build_r, status_t,
	private_quick_mode_t *this, message_t *message)
{
	switch (this->state)
	{
		case QM_INIT:
		{
			sa_payload_t *sa_payload;
			encap_t encap;

			this->proto = this->proposal->get_protocol(this->proposal);
			this->spi_r = this->child_sa->alloc_spi(this->child_sa, this->proto);
			if (!this->spi_r)
			{
				DBG1(DBG_IKE, "allocating SPI from kernel failed");
				return send_notify(this, NO_PROPOSAL_CHOSEN);
			}
			this->proposal->set_spi(this->proposal, this->spi_r);

			if (this->cpi_i)
			{
				this->cpi_r = this->child_sa->alloc_cpi(this->child_sa);
				if (!this->cpi_r)
				{
					DBG1(DBG_IKE, "unable to allocate a CPI from "
						 "kernel, IPComp disabled");
					return send_notify(this, NO_PROPOSAL_CHOSEN);
				}
			}

			if (this->udp && this->mode == MODE_TRANSPORT)
			{
				/* TODO-IKEv1: disable NAT-T for TRANSPORT mode by default? */
				add_nat_oa_payloads(this, message);
			}

			encap = get_encap(this->ike_sa, this->udp);
			sa_payload = sa_payload_create_from_proposal_v1(this->proposal,
								this->lifetime, this->lifebytes, AUTH_NONE,
								this->mode, encap, this->cpi_r);
			message->add_payload(message, &sa_payload->payload_interface);

			if (!add_nonce(this, &this->nonce_r, message))
			{
				return FAILED;
			}
			if (this->dh)
			{
				add_ke(this, message);
			}

			add_ts(this, message);

			this->state = QM_NEGOTIATED;
			return NEED_MORE;
		}
		case QM_NEGOTIATED:
			if (message->get_exchange_type(message) == INFORMATIONAL_V1)
			{
				/* skip INFORMATIONAL response if we received a INFORMATIONAL
				 * delete, see process_r() */
				return ALREADY_DONE;
			}
			/* fall */
		default:
			return FAILED;
	}
}

METHOD(task_t, process_i, status_t,
	private_quick_mode_t *this, message_t *message)
{
	switch (this->state)
	{
		case QM_INIT:
		{
			sa_payload_t *sa_payload;
			linked_list_t *list = NULL;
			bool private;

			sa_payload = (sa_payload_t*)message->get_payload(message,
													PLV1_SECURITY_ASSOCIATION);
			if (!sa_payload)
			{
				DBG1(DBG_IKE, "sa payload missing");
				return send_notify(this, NO_PROPOSAL_CHOSEN);
			}
			if (this->cpi_i)
			{
				list = sa_payload->get_ipcomp_proposals(sa_payload,
														&this->cpi_r);
				if (!list->get_count(list))
				{
					DBG1(DBG_IKE, "peer did not acccept our IPComp proposal, "
						 "IPComp disabled");
					this->cpi_i = 0;
				}
			}
			if (!list || !list->get_count(list))
			{
				DESTROY_IF(list);
				list = sa_payload->get_proposals(sa_payload);
			}
			private = this->ike_sa->supports_extension(this->ike_sa,
													   EXT_STRONGSWAN);
			this->proposal = this->config->select_proposal(this->config,
														   list, FALSE, private);
			list->destroy_offset(list, offsetof(proposal_t, destroy));
			if (!this->proposal)
			{
				DBG1(DBG_IKE, "no matching proposal found");
				return send_notify(this, NO_PROPOSAL_CHOSEN);
			}
			this->spi_r = this->proposal->get_spi(this->proposal);

			apply_lifetimes(this, sa_payload);

			if (!get_nonce(this, &this->nonce_r, message))
			{
				return send_notify(this, INVALID_PAYLOAD_TYPE);
			}
			if (this->dh && !get_ke(this, message))
			{
				return send_notify(this, INVALID_KEY_INFORMATION);
			}
			if (!get_ts(this, message))
			{
				return send_notify(this, INVALID_PAYLOAD_TYPE);
			}
			if (!install(this))
			{
				return send_notify(this, NO_PROPOSAL_CHOSEN);
			}
			this->state = QM_NEGOTIATED;
			return NEED_MORE;
		}
		default:
			return FAILED;
	}
}

METHOD(task_t, get_type, task_type_t,
	private_quick_mode_t *this)
{
	return TASK_QUICK_MODE;
}

METHOD(quick_mode_t, use_reqid, void,
	private_quick_mode_t *this, u_int32_t reqid)
{
	this->reqid = reqid;
}

METHOD(quick_mode_t, rekey, void,
	private_quick_mode_t *this, u_int32_t spi)
{
	this->rekey = spi;
}

METHOD(task_t, migrate, void,
	private_quick_mode_t *this, ike_sa_t *ike_sa)
{
	chunk_free(&this->nonce_i);
	chunk_free(&this->nonce_r);
	DESTROY_IF(this->tsi);
	DESTROY_IF(this->tsr);
	DESTROY_IF(this->proposal);
	DESTROY_IF(this->child_sa);
	DESTROY_IF(this->dh);

	this->ike_sa = ike_sa;
	this->keymat = (keymat_v1_t*)ike_sa->get_keymat(ike_sa);
	this->state = QM_INIT;
	this->tsi = NULL;
	this->tsr = NULL;
	this->proposal = NULL;
	this->child_sa = NULL;
	this->dh = NULL;
	this->spi_i = 0;
	this->spi_r = 0;

	if (!this->initiator)
	{
		DESTROY_IF(this->config);
		this->config = NULL;
	}
}

METHOD(task_t, destroy, void,
	private_quick_mode_t *this)
{
	chunk_free(&this->nonce_i);
	chunk_free(&this->nonce_r);
	DESTROY_IF(this->tsi);
	DESTROY_IF(this->tsr);
	DESTROY_IF(this->proposal);
	DESTROY_IF(this->child_sa);
	DESTROY_IF(this->config);
	DESTROY_IF(this->dh);
	free(this);
}

/*
 * Described in header.
 */
quick_mode_t *quick_mode_create(ike_sa_t *ike_sa, child_cfg_t *config,
							traffic_selector_t *tsi, traffic_selector_t *tsr)
{
	private_quick_mode_t *this;

	INIT(this,
		.public = {
			.task = {
				.get_type = _get_type,
				.migrate = _migrate,
				.destroy = _destroy,
			},
			.use_reqid = _use_reqid,
			.rekey = _rekey,
		},
		.ike_sa = ike_sa,
		.initiator = config != NULL,
		.config = config,
		.keymat = (keymat_v1_t*)ike_sa->get_keymat(ike_sa),
		.state = QM_INIT,
		.tsi = tsi ? tsi->clone(tsi) : NULL,
		.tsr = tsr ? tsr->clone(tsr) : NULL,
		.proto = PROTO_ESP,
	);

	if (config)
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
