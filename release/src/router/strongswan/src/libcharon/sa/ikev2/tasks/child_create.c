/*
 * Copyright (C) 2008-2019 Tobias Brunner
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

#include "child_create.h"

#include <daemon.h>
#include <sa/ikev2/keymat_v2.h>
#include <crypto/key_exchange.h>
#include <credentials/certificates/x509.h>
#include <encoding/payloads/sa_payload.h>
#include <encoding/payloads/ke_payload.h>
#include <encoding/payloads/ts_payload.h>
#include <encoding/payloads/nonce_payload.h>
#include <encoding/payloads/notify_payload.h>
#include <encoding/payloads/delete_payload.h>
#include <processing/jobs/delete_ike_sa_job.h>
#include <processing/jobs/inactivity_job.h>
#include <processing/jobs/initiate_tasks_job.h>

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
	 * nonce generator
	 */
	nonce_gen_t *nonceg;

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
	 * traffic selectors for initiator side
	 */
	linked_list_t *tsi;

	/**
	 * traffic selectors for responder side
	 */
	linked_list_t *tsr;

	/**
	 * labels for initiator side
	 */
	linked_list_t *labels_i;

	/**
	 * labels for responder side
	 */
	linked_list_t *labels_r;

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
	key_exchange_t *dh;

	/**
	 * Applying DH public value failed?
	 */
	bool dh_failed;

	/**
	 * group used for DH exchange
	 */
	key_exchange_method_t dh_group;

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
	 * IPsec protocol
	 */
	protocol_id_t proto;

	/**
	 * Own allocated SPI
	 */
	uint32_t my_spi;

	/**
	 * SPI received in proposal
	 */
	uint32_t other_spi;

	/**
	 * Own allocated Compression Parameter Index (CPI)
	 */
	uint16_t my_cpi;

	/**
	 * Other Compression Parameter Index (CPI), received via IPCOMP_SUPPORTED
	 */
	uint16_t other_cpi;

	/**
	 * Data collected to create the CHILD_SA
	 */
	child_sa_create_t child;

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
 * Schedule a retry if creating the CHILD_SA temporary failed
 */
static void schedule_delayed_retry(private_child_create_t *this)
{
	child_create_t *task;
	uint32_t retry;

	retry = RETRY_INTERVAL - (random() % RETRY_JITTER);

	task = child_create_create(this->ike_sa,
							   this->config->get_ref(this->config), FALSE,
							   this->packet_tsi, this->packet_tsr);
	task->use_reqid(task, this->child.reqid);
	task->use_marks(task, this->child.mark_in, this->child.mark_out);
	task->use_if_ids(task, this->child.if_id_in, this->child.if_id_out);
	task->use_label(task, this->child.label);

	DBG1(DBG_IKE, "creating CHILD_SA failed, trying again in %d seconds",
		 retry);
	this->ike_sa->queue_task_delayed(this->ike_sa, (task_t*)task, retry);
}

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
static bool generate_nonce(private_child_create_t *this)
{
	this->nonceg = this->keymat->keymat.create_nonce_gen(&this->keymat->keymat);
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
 * Allocate local SPI
 */
static bool allocate_spi(private_child_create_t *this)
{
	proposal_t *proposal;

	if (this->initiator)
	{
		this->proto = PROTO_ESP;
		/* we just get a SPI for the first protocol. TODO: If we ever support
		 * proposal lists with mixed protocols, we'd need multiple SPIs */
		if (this->proposals->get_first(this->proposals,
									   (void**)&proposal) == SUCCESS)
		{
			this->proto = proposal->get_protocol(proposal);
		}
	}
	else
	{
		this->proto = this->proposal->get_protocol(this->proposal);
	}
	this->my_spi = this->child_sa->alloc_spi(this->child_sa, this->proto);
	return this->my_spi != 0;
}

/**
 * Update the proposals with the allocated SPIs as initiator and check the DH
 * group and promote it if necessary
 */
static bool update_and_check_proposals(private_child_create_t *this)
{
	enumerator_t *enumerator;
	proposal_t *proposal;
	linked_list_t *other_dh_groups;
	bool found = FALSE;

	other_dh_groups = linked_list_create();
	enumerator = this->proposals->create_enumerator(this->proposals);
	while (enumerator->enumerate(enumerator, &proposal))
	{
		proposal->set_spi(proposal, this->my_spi);

		/* move the selected DH group to the front, if any */
		if (this->dh_group != KE_NONE)
		{	/* proposals that don't contain the selected group are
			 * moved to the back */
			if (!proposal->promote_transform(proposal, KEY_EXCHANGE_METHOD,
											 this->dh_group))
			{
				this->proposals->remove_at(this->proposals, enumerator);
				other_dh_groups->insert_last(other_dh_groups, proposal);
			}
			else
			{
				found = TRUE;
			}
		}
	}
	enumerator->destroy(enumerator);
	enumerator = other_dh_groups->create_enumerator(other_dh_groups);
	while (enumerator->enumerate(enumerator, (void**)&proposal))
	{	/* no need to remove from the list as we destroy it anyway*/
		this->proposals->insert_last(this->proposals, proposal);
	}
	enumerator->destroy(enumerator);
	other_dh_groups->destroy(other_dh_groups);

	return this->dh_group == KE_NONE || found;
}

/**
 * Schedule inactivity timeout for CHILD_SA with reqid, if enabled
 */
static void schedule_inactivity_timeout(private_child_create_t *this)
{
	uint32_t timeout, id;
	bool close_ike;

	timeout = this->config->get_inactivity(this->config);
	if (timeout)
	{
		close_ike = lib->settings->get_bool(lib->settings,
									"%s.inactivity_close_ike", FALSE, lib->ns);
		id = this->child_sa->get_unique_id(this->child_sa);
		lib->scheduler->schedule_job(lib->scheduler, (job_t*)
						inactivity_job_create(id, timeout, close_ike), timeout);
	}
}

/**
 * Substitute any host address with NATed address in traffic selector
 */
static linked_list_t* get_transport_nat_ts(private_child_create_t *this,
										   bool local, linked_list_t *in)
{
	enumerator_t *enumerator;
	linked_list_t *out;
	traffic_selector_t *ts;
	host_t *ike, *first = NULL;
	uint8_t mask;

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
	hosts = ike_sa_get_dynamic_hosts(this->ike_sa, local);

	if (this->mode == MODE_TRANSPORT &&
		this->ike_sa->has_condition(this->ike_sa, cond))
	{
		nat = get_transport_nat_ts(this, local, in);
		ts = this->config->get_traffic_selectors(this->config, local, nat,
												 hosts, TRUE);
		nat->destroy_offset(nat, offsetof(traffic_selector_t, destroy));
	}
	else
	{
		ts = this->config->get_traffic_selectors(this->config, local, in,
												 hosts, TRUE);
	}

	hosts->destroy(hosts);

	return ts;
}

/**
 * Check if requested mode is acceptable
 */
static bool check_mode(private_child_create_t *this, host_t *i, host_t *r)
{
	switch (this->mode)
	{
		case MODE_TRANSPORT:
			if (!this->config->has_option(this->config, OPT_PROXY_MODE) &&
				   (!ts_list_is_host(this->tsi, i) ||
					!ts_list_is_host(this->tsr, r))
			   )
			{
				DBG1(DBG_IKE, "not using transport mode, not host-to-host");
				return FALSE;
			}
			if (this->config->get_mode(this->config) != MODE_TRANSPORT)
			{
				return FALSE;
			}
			break;
		case MODE_BEET:
			if (!ts_list_is_host(this->tsi, NULL) ||
				!ts_list_is_host(this->tsr, NULL))
			{
				DBG1(DBG_IKE, "not using BEET mode, not host-to-host");
				return FALSE;
			}
			if (this->config->get_mode(this->config) != MODE_BEET)
			{
				return FALSE;
			}
			break;
		default:
			break;
	}
	return TRUE;
}

/**
 * Install a CHILD_SA for usage, return value:
 * - FAILED: no acceptable proposal
 * - INVALID_ARG: diffie hellman group unacceptable
 * - NOT_FOUND: TS unacceptable
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
	proposal_selection_flag_t flags = 0;

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

	if (no_dh)
	{
		flags |= PROPOSAL_SKIP_KE;
	}
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
	this->proposal = this->config->select_proposal(this->config,
												   this->proposals, flags);
	if (this->proposal == NULL)
	{
		DBG1(DBG_IKE, "no acceptable proposal found");
		charon->bus->alert(charon->bus, ALERT_PROPOSAL_MISMATCH_CHILD,
						   this->proposals);
		return FAILED;
	}
	this->other_spi = this->proposal->get_spi(this->proposal);

	if (!this->initiator)
	{
		if (!allocate_spi(this))
		{
			/* responder has no SPI allocated yet */
			DBG1(DBG_IKE, "allocating SPI failed");
			return FAILED;
		}
		this->proposal->set_spi(this->proposal, this->my_spi);
	}
	this->child_sa->set_proposal(this->child_sa, this->proposal);

	if (!this->proposal->has_transform(this->proposal, KEY_EXCHANGE_METHOD,
									   this->dh_group))
	{
		uint16_t group;

		if (this->proposal->get_algorithm(this->proposal, KEY_EXCHANGE_METHOD,
										  &group, NULL))
		{
			DBG1(DBG_IKE, "DH group %N unacceptable, requesting %N",
				 key_exchange_method_names, this->dh_group,
				 key_exchange_method_names, group);
			this->dh_group = group;
			return INVALID_ARG;
		}
		/* the selected proposal does not use a DH group */
		DBG1(DBG_IKE, "ignoring KE exchange, agreed on a non-PFS proposal");
		DESTROY_IF(this->dh);
		this->dh = NULL;
		this->dh_group = KE_NONE;
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

		if (!check_mode(this, me, other))
		{
			DBG1(DBG_IKE, "%N mode requested by responder is unacceptable",
				 ipsec_mode_names, this->mode);
			return FAILED;
		}
	}
	else
	{
		this->tsr = my_ts;
		this->tsi = other_ts;

		if (!check_mode(this, other, me))
		{
			this->mode = MODE_TUNNEL;
		}
	}

	if (!this->initiator)
	{
		/* use a copy of the traffic selectors, as the POST hook should not
		 * change payloads */
		my_ts = this->tsr->clone_offset(this->tsr,
										offsetof(traffic_selector_t, clone));
		other_ts = this->tsi->clone_offset(this->tsi,
										offsetof(traffic_selector_t, clone));
		charon->bus->narrow(charon->bus, this->child_sa,
							NARROW_RESPONDER_POST, my_ts, other_ts);

		if (my_ts->get_count(my_ts) == 0 ||	other_ts->get_count(other_ts) == 0)
		{
			my_ts->destroy_offset(my_ts,
								  offsetof(traffic_selector_t, destroy));
			other_ts->destroy_offset(other_ts,
								  offsetof(traffic_selector_t, destroy));
			return NOT_FOUND;
		}
	}

	this->child_sa->set_ipcomp(this->child_sa, this->ipcomp);
	this->child_sa->set_mode(this->child_sa, this->mode);
	this->child_sa->set_protocol(this->child_sa,
								 this->proposal->get_protocol(this->proposal));
	this->child_sa->set_state(this->child_sa, CHILD_INSTALLING);

	/* addresses might have changed since we originally sent the request, update
	 * them before we configure any policies and install the SAs */
	this->child_sa->update(this->child_sa, me, other, NULL,
						   this->ike_sa->has_condition(this->ike_sa, COND_NAT_ANY));

	this->child_sa->set_policies(this->child_sa, my_ts, other_ts);
	if (!this->initiator)
	{
		my_ts->destroy_offset(my_ts,
							  offsetof(traffic_selector_t, destroy));
		other_ts->destroy_offset(other_ts,
							  offsetof(traffic_selector_t, destroy));
	}

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
							TRUE, this->tfcv3);
		}
		else
		{
			status_i = this->child_sa->install(this->child_sa, encr_i, integ_i,
							this->my_spi, this->my_cpi, this->initiator,
							TRUE, this->tfcv3);
		}
		if (this->rekey)
		{	/* during rekeyings we install the outbound SA and/or policies
			 * separately: as responder when we receive the delete for the old
			 * SA, as initiator pretty much immediately in the ike-rekey task,
			 * unless there was a rekey collision that we lost */
			if (this->initiator)
			{
				status_o = this->child_sa->register_outbound(this->child_sa,
							encr_i, integ_i, this->other_spi, this->other_cpi,
							this->initiator, this->tfcv3);
			}
			else
			{
				status_o = this->child_sa->register_outbound(this->child_sa,
							encr_r, integ_r, this->other_spi, this->other_cpi,
							this->initiator, this->tfcv3);
			}
		}
		else if (this->initiator)
		{
			status_o = this->child_sa->install(this->child_sa, encr_i, integ_i,
							this->other_spi, this->other_cpi, this->initiator,
							FALSE, this->tfcv3);
		}
		else
		{
			status_o = this->child_sa->install(this->child_sa, encr_r, integ_r,
							this->other_spi, this->other_cpi, this->initiator,
							FALSE, this->tfcv3);
		}
	}

	if (status_i != SUCCESS || status_o != SUCCESS)
	{
		DBG1(DBG_IKE, "unable to install %s%s%sIPsec SA (SAD) in kernel",
			(status_i != SUCCESS) ? "inbound " : "",
			(status_i != SUCCESS && status_o != SUCCESS) ? "and ": "",
			(status_o != SUCCESS) ? "outbound " : "");
		charon->bus->alert(charon->bus, ALERT_INSTALL_CHILD_SA_FAILED,
						   this->child_sa);
		status = FAILED;
	}
	else
	{
		status = this->child_sa->install_policies(this->child_sa);

		if (status != SUCCESS)
		{
			DBG1(DBG_IKE, "unable to install IPsec policies (SPD) in kernel");
			charon->bus->alert(charon->bus, ALERT_INSTALL_CHILD_POLICY_FAILED,
							   this->child_sa);
			status = NOT_FOUND;
		}
		else
		{
			charon->bus->child_derived_keys(charon->bus, this->child_sa,
											this->initiator, encr_i, encr_r,
											integ_i, integ_r);
		}
	}
	chunk_clear(&integ_i);
	chunk_clear(&integ_r);
	chunk_clear(&encr_i);
	chunk_clear(&encr_r);

	if (status != SUCCESS)
	{
		return status;
	}

	charon->bus->child_keys(charon->bus, this->child_sa, this->initiator,
							this->dh, nonce_i, nonce_r);

#if DEBUG_LEVEL >= 0
	child_sa_outbound_state_t out_state;

	out_state = this->child_sa->get_outbound_state(this->child_sa);
	my_ts = linked_list_create_from_enumerator(
				this->child_sa->create_ts_enumerator(this->child_sa, TRUE));
	other_ts = linked_list_create_from_enumerator(
				this->child_sa->create_ts_enumerator(this->child_sa, FALSE));

	DBG0(DBG_IKE, "%sCHILD_SA %s{%d} established "
		 "with SPIs %.8x_i %.8x_o and TS %#R === %#R",
		 (out_state == CHILD_OUTBOUND_INSTALLED) ? "" : "inbound ",
		 this->child_sa->get_name(this->child_sa),
		 this->child_sa->get_unique_id(this->child_sa),
		 ntohl(this->child_sa->get_spi(this->child_sa, TRUE)),
		 ntohl(this->child_sa->get_spi(this->child_sa, FALSE)),
		 my_ts, other_ts);

	my_ts->destroy(my_ts);
	other_ts->destroy(other_ts);
#endif

	this->child_sa->set_state(this->child_sa, CHILD_INSTALLED);
	this->ike_sa->add_child_sa(this->ike_sa, this->child_sa);
	this->established = TRUE;

	schedule_inactivity_timeout(this);
	return SUCCESS;
}

/**
 * build the payloads for the message
 */
static bool build_payloads(private_child_create_t *this, message_t *message)
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
		ke_payload = ke_payload_create_from_key_exchange(PLV2_KEY_EXCHANGE,
														 this->dh);
		if (!ke_payload)
		{
			DBG1(DBG_IKE, "creating KE payload failed");
			return FALSE;
		}
		message->add_payload(message, (payload_t*)ke_payload);
	}

	/* add TSi/TSr payloads */
	ts_payload = ts_payload_create_from_traffic_selectors(TRUE, this->tsi,
														  this->child.label);
	message->add_payload(message, (payload_t*)ts_payload);
	ts_payload = ts_payload_create_from_traffic_selectors(FALSE, this->tsr,
														  this->child.label);
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

	features = charon->kernel->get_features(charon->kernel);
	if (!(features & KERNEL_ESP_V3_TFC))
	{
		message->add_notify(message, FALSE, ESP_TFC_PADDING_NOT_SUPPORTED,
							chunk_empty);
	}
	return TRUE;
}

/**
 * Adds an IPCOMP_SUPPORTED notify to the message, allocating a CPI
 */
static void add_ipcomp_notify(private_child_create_t *this,
								  message_t *message, uint8_t ipcomp)
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
			uint16_t cpi;
			chunk_t data;

			data = notify->get_notification_data(notify);
			cpi = *(uint16_t*)data.ptr;
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
					this->dh_group = ke_payload->get_key_exchange_method(
																	ke_payload);
					this->dh = this->keymat->keymat.create_ke(
										&this->keymat->keymat, this->dh_group);
				}
				else if (this->dh)
				{
					this->dh_failed = this->dh->get_method(this->dh) !=
								ke_payload->get_key_exchange_method(ke_payload);
				}
				if (this->dh && !this->dh_failed)
				{
					this->dh_failed = !this->dh->set_public_key(this->dh,
								ke_payload->get_key_exchange_data(ke_payload));
				}
				break;
			case PLV2_TS_INITIATOR:
				ts_payload = (ts_payload_t*)payload;
				this->tsi = ts_payload->get_traffic_selectors(ts_payload);
				this->labels_i = ts_payload->get_sec_labels(ts_payload);
				break;
			case PLV2_TS_RESPONDER:
				ts_payload = (ts_payload_t*)payload;
				this->tsr = ts_payload->get_traffic_selectors(ts_payload);
				this->labels_r = ts_payload->get_sec_labels(ts_payload);
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

/**
 * Check if we have only the generic label available when using SELinux and not
 * a specific one from an acquire.
 */
static bool generic_label_only(private_child_create_t *this)
{
	return this->config->get_label(this->config) && !this->child.label &&
		   this->config->get_label_mode(this->config) == SEC_LABEL_MODE_SELINUX;
}

/**
 * Check if we should defer the creation of this CHILD_SA until after the
 * IKE_SA has been established childless.
 */
static status_t defer_child_sa(private_child_create_t *this)
{
	ike_cfg_t *ike_cfg;
	childless_t policy;

	ike_cfg = this->ike_sa->get_ike_cfg(this->ike_sa);
	policy = ike_cfg->childless(ike_cfg);

	if (this->ike_sa->supports_extension(this->ike_sa, EXT_IKE_CHILDLESS))
	{
		/* with SELinux, we prefer not to create a CHILD_SA when we only have
		 * the generic label available.  if the peer does not support it,
		 * creating the SA will most likely fail */
		if (policy == CHILDLESS_PREFER ||
			policy == CHILDLESS_FORCE ||
			generic_label_only(this))
		{
			return NEED_MORE;
		}
	}
	else if (policy == CHILDLESS_FORCE)
	{
		DBG1(DBG_IKE, "peer does not support childless IKE_SA initiation");
		return DESTROY_ME;
	}
	return NOT_SUPPORTED;
}

/**
 * Compare two CHILD_SA objects for equality
 */
static bool child_sa_equals(child_sa_t *a, child_sa_t *b)
{
	child_cfg_t *cfg = a->get_config(a);
	return cfg->equals(cfg, b->get_config(b)) &&
		/* reqids are allocated based on the final TS, so we can only compare
		 * them if they are static (i.e. both have them) */
		(!a->get_reqid(a) || !b->get_reqid(b) ||
		  a->get_reqid(a) == b->get_reqid(b)) &&
		a->get_mark(a, TRUE).value == b->get_mark(b, TRUE).value &&
		a->get_mark(a, FALSE).value == b->get_mark(b, FALSE).value &&
		a->get_if_id(a, TRUE) == b->get_if_id(b, TRUE) &&
		a->get_if_id(a, FALSE) == b->get_if_id(b, FALSE) &&
		sec_labels_equal(a->get_label(a), b->get_label(b));
}

/**
 * Check if there is a duplicate CHILD_SA already established and we can abort
 * initiating this one.
 */
static bool check_for_duplicate(private_child_create_t *this)
{
	enumerator_t *enumerator;
	child_sa_t *child_sa, *found = NULL;

	enumerator = this->ike_sa->create_child_sa_enumerator(this->ike_sa);
	while (enumerator->enumerate(enumerator, (void**)&child_sa))
	{
		if (child_sa->get_state(child_sa) == CHILD_INSTALLED &&
			child_sa_equals(child_sa, this->child_sa))
		{
			found = child_sa;
			break;
		}
	}
	enumerator->destroy(enumerator);

	if (found)
	{
		linked_list_t *my_ts, *other_ts;

		my_ts = linked_list_create_from_enumerator(
					found->create_ts_enumerator(found, TRUE));
		other_ts = linked_list_create_from_enumerator(
					found->create_ts_enumerator(found, FALSE));

		DBG1(DBG_IKE, "not establishing CHILD_SA %s{%d} due to existing "
			 "duplicate {%d} with SPIs %.8x_i %.8x_o and TS %#R === %#R",
			 this->child_sa->get_name(this->child_sa),
			 this->child_sa->get_unique_id(this->child_sa),
			 found->get_unique_id(found),
			 ntohl(found->get_spi(found, TRUE)),
			 ntohl(found->get_spi(found, FALSE)), my_ts, other_ts);

		my_ts->destroy(my_ts);
		other_ts->destroy(other_ts);
	}
	return found;
}

/**
 * Check if this is an attempt to create an SA with generic label and should
 * be aborted.
 */
static bool check_for_generic_label(private_child_create_t *this)
{
	if (generic_label_only(this))
	{
#if DEBUG_LEVEL >= 1
		sec_label_t *label = this->config->get_label(this->config);
		DBG1(DBG_IKE, "not establishing CHILD_SA %s{%d} with generic "
			 "label '%s'", this->child_sa->get_name(this->child_sa),
			 this->child_sa->get_unique_id(this->child_sa),
			 label->get_string(label));
#endif
		return TRUE;
	}
	return FALSE;
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
			if (!generate_nonce(this))
			{
				message->set_exchange_type(message, EXCHANGE_TYPE_UNDEFINED);
				return SUCCESS;
			}
			if (!this->retry && this->dh_group == KE_NONE)
			{	/* during a rekeying the group might already be set */
				this->dh_group = this->config->get_algorithm(this->config,
														KEY_EXCHANGE_METHOD);
			}
			break;
		case IKE_AUTH:
			switch (defer_child_sa(this))
			{
				case DESTROY_ME:
					/* config mismatch */
					return DESTROY_ME;
				case NEED_MORE:
					/* defer until after IKE_SA has been established */
					chunk_free(&this->my_nonce);
					return NEED_MORE;
				default:
					/* just continue to establish the CHILD_SA */
					break;
			}
			/* send only in the first request, not in subsequent rounds */
			this->public.task.build = (void*)return_need_more;
			break;
		default:
			return NEED_MORE;
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
														TRUE, NULL, list, TRUE);
		list->destroy_offset(list, offsetof(host_t, destroy));
	}
	else
	{	/* no virtual IPs configured */
		list->destroy(list);
		list = ike_sa_get_dynamic_hosts(this->ike_sa, TRUE);
		this->tsi = this->config->get_traffic_selectors(this->config,
														TRUE, NULL, list, TRUE);
		list->destroy(list);
	}
	list = ike_sa_get_dynamic_hosts(this->ike_sa, FALSE);
	this->tsr = this->config->get_traffic_selectors(this->config,
													FALSE, NULL, list, TRUE);
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

	if (!generic_label_only(this) && !this->child.label)
	{	/* in the simple label mode we propose the configured label as we
		 * won't have labels from acquires */
		this->child.label = this->config->get_label(this->config);
		if (this->child.label)
		{
			this->child.label = this->child.label->clone(this->child.label);
		}
	}
	if (this->child.label)
	{
		DBG2(DBG_CFG, "proposing security label '%s'",
			 this->child.label->get_string(this->child.label));
	}

	this->proposals = this->config->get_proposals(this->config,
												  this->dh_group == KE_NONE);
	this->mode = this->config->get_mode(this->config);

	this->child.if_id_in_def = this->ike_sa->get_if_id(this->ike_sa, TRUE);
	this->child.if_id_out_def = this->ike_sa->get_if_id(this->ike_sa, FALSE);
	this->child.encap = this->ike_sa->has_condition(this->ike_sa, COND_NAT_ANY);
	this->child_sa = child_sa_create(this->ike_sa->get_my_host(this->ike_sa),
									 this->ike_sa->get_other_host(this->ike_sa),
									 this->config, &this->child);

	/* check this after creating the object so that its destruction is detected
	 * by controller and trap manager */
	if (!this->rekey &&
		message->get_exchange_type(message) == CREATE_CHILD_SA &&
		(check_for_generic_label(this) || check_for_duplicate(this)))
	{
		message->set_exchange_type(message, EXCHANGE_TYPE_UNDEFINED);
		return SUCCESS;
	}

	if (this->child.reqid)
	{
		DBG0(DBG_IKE, "establishing CHILD_SA %s{%d} reqid %d",
			 this->child_sa->get_name(this->child_sa),
			 this->child_sa->get_unique_id(this->child_sa), this->child.reqid);
	}
	else
	{
		DBG0(DBG_IKE, "establishing CHILD_SA %s{%d}",
			 this->child_sa->get_name(this->child_sa),
			 this->child_sa->get_unique_id(this->child_sa));
	}

	if (!allocate_spi(this))
	{
		DBG1(DBG_IKE, "unable to allocate SPIs from kernel");
		return FAILED;
	}

	if (!update_and_check_proposals(this))
	{
		DBG1(DBG_IKE, "requested DH group %N not contained in any of our "
			 "proposals",
			 key_exchange_method_names, this->dh_group);
		return FAILED;
	}

	if (this->dh_group != KE_NONE)
	{
		this->dh = this->keymat->keymat.create_ke(&this->keymat->keymat,
												  this->dh_group);
	}

	if (this->config->has_option(this->config, OPT_IPCOMP))
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

	if (!build_payloads(this, message))
	{
		return FAILED;
	}

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
			/* only handle first AUTH payload, not additional rounds */
			this->public.task.process = (void*)return_need_more;
			break;
		default:
			return NEED_MORE;
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
	bool is_first;

	is_first = message->get_exchange_type(message) == IKE_AUTH;
	if (is_first &&
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
		charon->bus->alert(charon->bus, ALERT_KEEP_ON_CHILD_SA_FAILURE,
						   is_first);
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

		listr = ike_sa_get_dynamic_hosts(this->ike_sa, TRUE);
		listi = ike_sa_get_dynamic_hosts(this->ike_sa, FALSE);
		child_cfg = peer_cfg->select_child_cfg(peer_cfg,
									tsr ?: this->tsr, tsi ?: this->tsi,
									listr, listi, this->labels_r, this->labels_i);
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
									this->tsr, this->tsi,
									listr, listi, this->labels_r, this->labels_i);
		}
		listr->destroy(listr);
		listi->destroy(listi);
		DESTROY_OFFSET_IF(tsi, offsetof(traffic_selector_t, destroy));
		DESTROY_OFFSET_IF(tsr, offsetof(traffic_selector_t, destroy));
	}

	return child_cfg;
}

/**
 * Check how to handle a possibly childless IKE_SA
 */
static status_t handle_childless(private_child_create_t *this)
{
	ike_cfg_t *ike_cfg;

	ike_cfg = this->ike_sa->get_ike_cfg(this->ike_sa);

	if (!this->proposals && !this->tsi && !this->tsr)
	{
		/* looks like a childless IKE_SA, check if we allow it */
		if (ike_cfg->childless(ike_cfg) == CHILDLESS_NEVER)
		{
			/* we don't allow childless initiation */
			DBG1(DBG_IKE, "peer tried to initiate a childless IKE_SA");
			return INVALID_STATE;
		}
		return SUCCESS;
	}

	/* the peer apparently wants to create a regular IKE_SA */
	if (ike_cfg->childless(ike_cfg) == CHILDLESS_FORCE)
	{
		/* reject it if we only allow childless initiation */
		DBG1(DBG_IKE, "peer did not initiate a childless IKE_SA");
		return INVALID_STATE;
	}
	return NOT_SUPPORTED;
}

/**
 * Select a security label.
 *
 * We already know that the proposed labels match the selected config, just make
 * sure that the proposed/returned labels are the same.
 */
static bool select_label(private_child_create_t *this)
{
	sec_label_t *li, *lr;

	if (!this->config->select_label(this->config, this->labels_i, FALSE, &li, NULL) ||
		!this->config->select_label(this->config, this->labels_r, FALSE, &lr, NULL))
	{	/* sanity check */
		return FALSE;
	}

	if (li)
	{
		if (!li->equals(li, lr))
		{
			DBG1(DBG_CHD, "security labels in TSi and TSr don't match");
			return FALSE;
		}
		else if (!this->child.label)
		{
			this->child.label = li->clone(li);
		}
		else if (!this->child.label->equals(this->child.label, li))
		{
			DBG1(DBG_CHD, "returned security label '%s' doesn't match proposed "
				 "'%s'", li->get_string(li),
				 this->child.label->get_string(this->child.label));
			return FALSE;
		}
	}
	if (this->child.label)
	{
		DBG1(DBG_CFG, "selected security label: %s",
			 this->child.label->get_string(this->child.label));
	}
	return TRUE;
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
			if (!generate_nonce(this))
			{
				message->add_notify(message, FALSE, NO_PROPOSAL_CHOSEN,
									chunk_empty);
				return SUCCESS;
			}
			if (this->dh_failed)
			{
				DBG1(DBG_IKE, "applying DH public value failed");
				message->add_notify(message, FALSE, NO_PROPOSAL_CHOSEN,
									chunk_empty);
				return SUCCESS;
			}
			no_dh = FALSE;
			break;
		case IKE_AUTH:
			if (!this->ike_sa->has_condition(this->ike_sa, COND_AUTHENTICATED))
			{	/* wait until all authentication round completed */
				return NEED_MORE;
			}
			if (this->ike_sa->has_condition(this->ike_sa, COND_REDIRECTED))
			{	/* no CHILD_SA is created for redirected SAs */
				return SUCCESS;
			}
			switch (handle_childless(this))
			{
				case SUCCESS:
					/* no CHILD_SA built */
					return SUCCESS;
				case INVALID_STATE:
					message->add_notify(message, FALSE, INVALID_SYNTAX,
										chunk_empty);
					return FAILED;
				default:
					/* continue with regular initiation */
					break;
			}
			ike_auth = TRUE;
			break;
		default:
			return NEED_MORE;
	}

	if (this->ike_sa->get_state(this->ike_sa) == IKE_REKEYING)
	{
		DBG1(DBG_IKE, "unable to create CHILD_SA while rekeying IKE_SA");
		message->add_notify(message, TRUE, TEMPORARY_FAILURE, chunk_empty);
		return SUCCESS;
	}
	if (this->ike_sa->get_state(this->ike_sa) == IKE_DELETING)
	{
		DBG1(DBG_IKE, "unable to create CHILD_SA while deleting IKE_SA");
		message->add_notify(message, TRUE, TEMPORARY_FAILURE, chunk_empty);
		return SUCCESS;
	}

	if (this->config == NULL)
	{
		this->config = select_child_cfg(this);
	}
	if (this->config == NULL)
	{
		DBG1(DBG_IKE, "traffic selectors %#R === %#R unacceptable",
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

	if (!select_label(this))
	{
		message->add_notify(message, FALSE, TS_UNACCEPTABLE, chunk_empty);
		handle_child_sa_failure(this, message);
		return SUCCESS;
	}

	this->child.if_id_in_def = this->ike_sa->get_if_id(this->ike_sa, TRUE);
	this->child.if_id_out_def = this->ike_sa->get_if_id(this->ike_sa, FALSE);
	this->child.encap = this->ike_sa->has_condition(this->ike_sa, COND_NAT_ANY);
	this->child_sa = child_sa_create(this->ike_sa->get_my_host(this->ike_sa),
									 this->ike_sa->get_other_host(this->ike_sa),
									 this->config, &this->child);

	if (this->ipcomp_received != IPCOMP_NONE)
	{
		if (this->config->has_option(this->config, OPT_IPCOMP))
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
			uint16_t group = htons(this->dh_group);
			message->add_notify(message, FALSE, INVALID_KE_PAYLOAD,
								chunk_from_thing(group));
			return SUCCESS;
		}
		case FAILED:
		default:
			message->add_notify(message, FALSE, NO_PROPOSAL_CHOSEN, chunk_empty);
			handle_child_sa_failure(this, message);
			return SUCCESS;
	}

	if (!build_payloads(this, message))
	{
		message->add_notify(message, FALSE, NO_PROPOSAL_CHOSEN, chunk_empty);
		handle_child_sa_failure(this, message);
		return SUCCESS;
	}

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
	if (this->my_spi && this->proto)
	{
		delete_payload_t *del;

		del = delete_payload_create(PLV2_DELETE, this->proto);
		del->add_spi(del, this->my_spi);
		message->add_payload(message, (payload_t*)del);

		DBG1(DBG_IKE, "sending DELETE for %N CHILD_SA with SPI %.8x",
			 protocol_id_names, this->proto, ntohl(this->my_spi));
	}
	return NEED_MORE;
}

/**
 * Change task to delete the failed CHILD_SA as initiator
 */
static status_t delete_failed_sa(private_child_create_t *this)
{
	if (this->my_spi && this->proto)
	{
		this->public.task.build = _build_i_delete;
		this->public.task.process = (void*)return_success;
		return NEED_MORE;
	}
	return SUCCESS;
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
			if (!this->ike_sa->has_condition(this->ike_sa, COND_AUTHENTICATED))
			{	/* wait until all authentication round completed */
				return NEED_MORE;
			}
			if (defer_child_sa(this) == NEED_MORE)
			{	/* defer until after IKE_SA has been established */
				chunk_free(&this->other_nonce);
				return NEED_MORE;
			}
			ike_auth = TRUE;
			break;
		default:
			return NEED_MORE;
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
				case TEMPORARY_FAILURE:
				{
					DBG1(DBG_IKE, "received %N notify, will retry later",
						 notify_type_names, type);
					enumerator->destroy(enumerator);
					if (!this->rekey)
					{	/* the rekey task will retry itself if necessary */
						schedule_delayed_retry(this);
					}
					return SUCCESS;
				}
				case INVALID_KE_PAYLOAD:
				{
					chunk_t data;
					uint16_t group = KE_NONE;

					data = notify->get_notification_data(notify);
					if (data.len == sizeof(group))
					{
						memcpy(&group, data.ptr, data.len);
						group = ntohs(group);
					}
					if (this->retry)
					{
						DBG1(DBG_IKE, "already retried with DH group %N, "
							 "ignore requested %N", key_exchange_method_names,
							 this->dh_group, key_exchange_method_names, group);
						handle_child_sa_failure(this, message);
						/* an error in CHILD_SA creation is not critical */
						return SUCCESS;
					}
					DBG1(DBG_IKE, "peer didn't accept DH group %N, "
						 "it requested %N", key_exchange_method_names,
						 this->dh_group, key_exchange_method_names, group);
					this->retry = TRUE;
					this->dh_group = group;
					this->child_sa->set_state(this->child_sa, CHILD_RETRYING);
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

	if (this->dh_failed)
	{
		DBG1(DBG_IKE, "applying DH public value failed");
		handle_child_sa_failure(this, message);
		return delete_failed_sa(this);
	}

	if (!select_label(this))
	{
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
	private_child_create_t *this, uint32_t reqid)
{
	uint32_t existing_reqid = this->child.reqid;

	if (!reqid || charon->kernel->ref_reqid(charon->kernel, reqid) == SUCCESS)
	{
		this->child.reqid = reqid;
		if (existing_reqid)
		{
			charon->kernel->release_reqid(charon->kernel, existing_reqid);
		}
	}
}

METHOD(child_create_t, use_marks, void,
	private_child_create_t *this, uint32_t in, uint32_t out)
{
	this->child.mark_in = in;
	this->child.mark_out = out;
}

METHOD(child_create_t, use_if_ids, void,
	private_child_create_t *this, uint32_t in, uint32_t out)
{
	this->child.if_id_in = in;
	this->child.if_id_out = out;
}

METHOD(child_create_t, use_label, void,
	private_child_create_t *this, sec_label_t *label)
{
	DESTROY_IF(this->child.label);
	this->child.label = label ? label->clone(label) : NULL;
}

METHOD(child_create_t, use_dh_group, void,
	private_child_create_t *this, key_exchange_method_t dh_group)
{
	this->dh_group = dh_group;
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
	if (this->labels_i)
	{
		this->labels_i->destroy_offset(this->labels_i, offsetof(sec_label_t, destroy));
	}
	if (this->labels_r)
	{
		this->labels_r->destroy_offset(this->labels_r, offsetof(sec_label_t, destroy));
	}
	DESTROY_IF(this->child_sa);
	DESTROY_IF(this->proposal);
	DESTROY_IF(this->nonceg);
	DESTROY_IF(this->dh);
	this->dh_failed = FALSE;
	if (this->proposals)
	{
		this->proposals->destroy_offset(this->proposals, offsetof(proposal_t, destroy));
	}
	if (!this->rekey && !this->retry)
	{
		this->dh_group = KE_NONE;
	}
	this->ike_sa = ike_sa;
	this->keymat = (keymat_v2_t*)ike_sa->get_keymat(ike_sa);
	this->proposal = NULL;
	this->proposals = NULL;
	this->tsi = NULL;
	this->tsr = NULL;
	this->dh = NULL;
	this->nonceg = NULL;
	this->child_sa = NULL;
	this->mode = MODE_TUNNEL;
	this->ipcomp = IPCOMP_NONE;
	this->ipcomp_received = IPCOMP_NONE;
	this->other_cpi = 0;
	this->established = FALSE;
	this->public.task.build = _build_i;
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
	if (this->labels_i)
	{
		this->labels_i->destroy_offset(this->labels_i, offsetof(sec_label_t, destroy));
	}
	if (this->labels_r)
	{
		this->labels_r->destroy_offset(this->labels_r, offsetof(sec_label_t, destroy));
	}
	if (!this->established)
	{
		DESTROY_IF(this->child_sa);
	}
	if (this->child.reqid)
	{
		charon->kernel->release_reqid(charon->kernel, this->child.reqid);
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
	DESTROY_IF(this->nonceg);
	DESTROY_IF(this->child.label);
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
			.use_marks = _use_marks,
			.use_if_ids = _use_if_ids,
			.use_label = _use_label,
			.use_dh_group = _use_dh_group,
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
		.dh_group = KE_NONE,
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
