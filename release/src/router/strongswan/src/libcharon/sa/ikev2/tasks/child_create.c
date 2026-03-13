/*
 * Copyright (C) 2008-2025 Tobias Brunner
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

#include <unistd.h>

#include "child_create.h"

#include <daemon.h>
#include <sa/ikev2/keymat_v2.h>
#include <crypto/key_exchange.h>
#include <credentials/certificates/x509.h>
#include <collections/hashtable.h>
#include <encoding/payloads/sa_payload.h>
#include <encoding/payloads/ke_payload.h>
#include <encoding/payloads/ts_payload.h>
#include <encoding/payloads/nonce_payload.h>
#include <encoding/payloads/notify_payload.h>
#include <encoding/payloads/delete_payload.h>
#include <processing/jobs/delete_ike_sa_job.h>
#include <processing/jobs/inactivity_job.h>
#include <processing/jobs/initiate_tasks_job.h>

/** Maximum number of key exchanges (including the initial one, if any) */
#define MAX_KEY_EXCHANGES (MAX_ADDITIONAL_KEY_EXCHANGES + 1)

typedef struct private_child_create_t private_child_create_t;

/** Assumed minimum CPU ID, used when searching for a CPU without SA */
#define CPU_ID_MIN 0

/**
 * Flags for IP-TFS
 */
typedef enum {
	/** Indicates that fragmentation across packets is not supported */
	IPTFS_DONT_FRAGMENT = (1 << 0),
	/** Requests periodic congestion control information from the peer */
	IPTFS_CONGESTION_CONTROL = (1 << 1),
} iptfs_flags_t;

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
	 * Local traffic selectors as configured or previously negotiated
	 */
	traffic_selector_list_t *my_ts;

	/**
	 * Remote traffic selectors as configured or previously negotiated
	 */
	traffic_selector_list_t *other_ts;

	/**
	 * Resource info sent by the peer
	 */
	chunk_t resource_info;

	/**
	 * Whether we received resource info (might be empty)
	 */
	bool resource_info_seen;

	/**
	 * Key exchanges to perform
	 */
	struct {
		transform_type_t type;
		key_exchange_method_t method;
		bool done;
	} key_exchanges[MAX_KEY_EXCHANGES];

	/**
	 * Current key exchange
	 */
	int ke_index;

	/**
	 * Kex exchange method from the parsed or sent KE payload
	 */
	key_exchange_method_t ke_method;

	/**
	 * Current key exchange object (if any)
	 */
	key_exchange_t *ke;

	/**
	 * All key exchanges performed (key_exchange_t)
	 */
	array_t *kes;

	/**
	 * Applying KE public key failed?
	 */
	bool ke_failed;

	/**
	 * Link value for current key exchange
	 */
	chunk_t link;

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
	 * IP-TFS flags received from the peer
	 */
	uint8_t iptfs_flags;

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
	 * whether we are retrying with another KE method
	 */
	bool retry;

	/**
	 * Whether the task was aborted
	 */
	bool aborted;
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
							   this->packet_tsi, this->packet_tsr,
							   this->child.seq);
	task->use_reqid(task, this->child.reqid);
	task->use_marks(task, this->child.mark_in, this->child.mark_out);
	task->use_if_ids(task, this->child.if_id_in, this->child.if_id_out);
	task->use_label(task, this->child.label);
	task->use_per_cpu(task, this->child.per_cpu, this->child.cpu);

	/* clone these directly as we don't have a public method */
	if (this->my_ts && this->other_ts)
	{
		private_child_create_t *priv = (private_child_create_t*)task;

		priv->my_ts = this->my_ts->clone(this->my_ts);
		priv->other_ts = this->other_ts->clone(this->other_ts);
	}

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
	if (!this->my_spi)
	{
		DBG1(DBG_IKE, "unable to allocate SPI from kernel");
	}
	return this->my_spi != 0;
}

/**
 * Update the proposals with the allocated SPIs as initiator and check the KE
 * method and promote it if necessary
 */
static bool update_and_check_proposals(private_child_create_t *this)
{
	enumerator_t *enumerator;
	proposal_t *proposal;
	linked_list_t *other_ke_methods;
	bool found = FALSE;

	other_ke_methods = linked_list_create();
	enumerator = this->proposals->create_enumerator(this->proposals);
	while (enumerator->enumerate(enumerator, &proposal))
	{
		proposal->set_spi(proposal, this->my_spi);

		/* move the selected KE method to the front, if any */
		if (this->ke_method != KE_NONE)
		{	/* proposals that don't contain the selected group are
			 * moved to the back */
			if (!proposal->promote_transform(proposal, KEY_EXCHANGE_METHOD,
											 this->ke_method))
			{
				this->proposals->remove_at(this->proposals, enumerator);
				other_ke_methods->insert_last(other_ke_methods, proposal);
			}
			else
			{
				found = TRUE;
			}
		}
	}
	enumerator->destroy(enumerator);
	enumerator = other_ke_methods->create_enumerator(other_ke_methods);
	while (enumerator->enumerate(enumerator, (void**)&proposal))
	{	/* no need to remove from the list as we destroy it anyway*/
		this->proposals->insert_last(this->proposals, proposal);
	}
	enumerator->destroy(enumerator);
	other_ke_methods->destroy(other_ke_methods);

	return this->ke_method == KE_NONE || found;
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
 * Ensure we have traffic selector lists when not recreating an SA.
 */
static void ensure_ts_lists(private_child_create_t *this)
{
	linked_list_t *ts;

	if (!this->my_ts)
	{
		ts = this->config->get_traffic_selectors(this->config, TRUE, NULL);
		this->my_ts = traffic_selector_list_create_from_list(ts);
	}
	if (!this->other_ts)
	{
		ts = this->config->get_traffic_selectors(this->config, FALSE, NULL);
		this->other_ts = traffic_selector_list_create_from_list(ts);
	}
}

/**
 * Narrow received traffic selectors with configuration
 */
static linked_list_t* narrow_ts(private_child_create_t *this, bool local,
								linked_list_t *in)
{
	traffic_selector_list_t *ts;
	linked_list_t *hosts, *nat, *result;
	ike_condition_t cond;

	ensure_ts_lists(this);
	ts = local ? this->my_ts : this->other_ts;
	cond = local ? COND_NAT_HERE : COND_NAT_THERE;
	hosts = ike_sa_get_dynamic_hosts(this->ike_sa, local);

	if (this->mode == MODE_TRANSPORT &&
		this->ike_sa->has_condition(this->ike_sa, cond))
	{
		nat = get_transport_nat_ts(this, local, in);
		result = child_cfg_select_ts(this->config, local, ts, nat, hosts);
		nat->destroy_offset(nat, offsetof(traffic_selector_t, destroy));
	}
	else
	{
		result = child_cfg_select_ts(this->config, local, ts, in, hosts);
	}

	hosts->destroy(hosts);
	return result;
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
		case MODE_IPTFS:
			if (this->config->get_mode(this->config) != MODE_IPTFS)
			{
				return FALSE;
			}
			if ((this->iptfs_flags & IPTFS_CONGESTION_CONTROL) ||
				(this->iptfs_flags & 0xfc))
			{
				/* congestion control is not supported, neither are any of
				 * the 6 reserved flags, so we disable IP-TFS if they are set */
				DBG1(DBG_IKE, "not using IP-TFS mode, peer sent unsupported "
					 "flags (0x%.2x) in %N notify", this->iptfs_flags,
					 notify_type_names, USE_AGGFRAG);
				return FALSE;
			}
			break;
		default:
			break;
	}
	return TRUE;
}

/**
 * Do traffic selector narrowing and check mode:
 * - FAILED: mode mismatch
 * - NOT_FOUND: TS unacceptable
 */
static status_t narrow_and_check_ts(private_child_create_t *this, bool ike_auth)
{
	linked_list_t *my_ts, *other_ts;
	host_t *me, *other;

	me = this->ike_sa->get_my_host(this->ike_sa);
	other = this->ike_sa->get_other_host(this->ike_sa);

	this->child_sa->set_proposal(this->child_sa, this->proposal);

	if (this->initiator)
	{
		my_ts = narrow_ts(this, TRUE, this->tsi);
		other_ts = narrow_ts(this, FALSE, this->tsr);
	}
	else
	{
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
	return SUCCESS;
}

/**
 * Install a CHILD_SA:
 * - FAILED: failure to install SAs
 * - NOT_FOUND: TS unacceptable (only responder), or failure to install policies
 */
static status_t install_child_sa(private_child_create_t *this)
{
	status_t status, status_i, status_o;
	chunk_t nonce_i, nonce_r;
	chunk_t encr_i = chunk_empty, encr_r = chunk_empty;
	chunk_t integ_i = chunk_empty, integ_r = chunk_empty;
	linked_list_t *my_ts, *other_ts;

	if (this->initiator)
	{
		nonce_i = this->my_nonce;
		nonce_r = this->other_nonce;

		my_ts = this->tsi;
		other_ts = this->tsr;
	}
	else
	{
		nonce_i = this->other_nonce;
		nonce_r = this->my_nonce;

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
	if (this->mode == MODE_IPTFS && this->iptfs_flags & IPTFS_DONT_FRAGMENT)
	{
		this->child_sa->set_iptfs_dont_fragment(this->child_sa);
	}
	this->child_sa->set_protocol(this->child_sa,
								 this->proposal->get_protocol(this->proposal));

	/* addresses might have changed since we originally sent the request, update
	 * them before we configure any policies and install the SAs */
	this->child_sa->update(this->child_sa,
						   this->ike_sa->get_my_host(this->ike_sa),
						   this->ike_sa->get_other_host(this->ike_sa), NULL,
						   this->ike_sa->has_condition(this->ike_sa, COND_NAT_ANY));

	this->child_sa->set_policies(this->child_sa, my_ts, other_ts);
	if (!this->initiator)
	{
		my_ts->destroy_offset(my_ts,
							  offsetof(traffic_selector_t, destroy));
		other_ts->destroy_offset(other_ts,
							  offsetof(traffic_selector_t, destroy));
	}
	this->child_sa->set_state(this->child_sa, CHILD_INSTALLING);

	if (this->my_cpi == 0 || this->other_cpi == 0 || this->ipcomp == IPCOMP_NONE)
	{
		this->my_cpi = this->other_cpi = 0;
		this->ipcomp = IPCOMP_NONE;
	}
	status_i = status_o = FAILED;
	if (this->keymat->derive_child_keys(this->keymat, this->proposal,
			this->kes, nonce_i, nonce_r, &encr_i, &integ_i, &encr_r, &integ_r))
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
		{
			/* during rekeyings we install the outbound SA and/or policies
			 * separately: as responder, when we receive the delete for the old
			 * SA, as initiator, pretty much immediately in the ike-rekey task,
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
			charon->bus->child_keys(charon->bus, this->child_sa,
								this->initiator, this->kes, nonce_i, nonce_r);
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
 * Select a proposal
 */
static bool select_proposal(private_child_create_t *this, bool no_ke)
{
	proposal_selection_flag_t flags = 0;

	if (!this->proposals)
	{
		DBG1(DBG_IKE, "SA payload missing in message");
		return FALSE;
	}

	if (no_ke)
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
	if (!this->proposal)
	{
		DBG1(DBG_IKE, "no acceptable proposal found");
		charon->bus->alert(charon->bus, ALERT_PROPOSAL_MISMATCH_CHILD,
						   this->proposals);
		return FALSE;
	}
	return TRUE;
}

/**
 * Returns the number of CPUs that are configured on the system or only those
 * that are currently online (if we can distinguish this).
 */
static uint32_t get_system_cpus(bool online)
{
	long cpus;

#ifdef WIN32
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	cpus = sysinfo.dwNumberOfProcessors;
#else /* WIN32 */
	cpus = sysconf(online ? _SC_NPROCESSORS_ONLN : _SC_NPROCESSORS_CONF);

	if (cpus < 0)
	{
		DBG1(DBG_IKE, "failed to determine number of CPUs for this system");
		/* assume we have at least one CPU */
		cpus = 1;
	}
#endif /* WIN32 */
	return cpus;
}

/**
 * Finds an unused CPU ID to be assigned to a responder's CHILD_SA, or return
 * CPU_ID_MAX if we don't have a fallback yet.
 */
static uint32_t get_cpu(private_child_create_t *this)
{
	enumerator_t *enumerator;
	hashtable_t *used;
	child_sa_t *child_sa;
	bool found_fallback = FALSE;
	uint32_t cpu;

	used = hashtable_create(hashtable_hash_ptr, hashtable_equals_ptr, 32);

	enumerator = this->ike_sa->create_child_sa_enumerator(this->ike_sa);
	while (enumerator->enumerate(enumerator, (void**)&child_sa))
	{
		if (child_sa->get_state(child_sa) == CHILD_INSTALLED &&
			this->config->equals(this->config, child_sa->get_config(child_sa)))
		{
			cpu = child_sa->get_cpu(child_sa);
			if (cpu != CPU_ID_MAX)
			{
				used->put(used, (void*)(uintptr_t)cpu, child_sa);
			}
			else
			{
				found_fallback = TRUE;
			}
		}
	}
	enumerator->destroy(enumerator);

	if (!found_fallback)
	{
		cpu = CPU_ID_MAX;
	}
	else
	{
		cpu = get_system_cpus(TRUE) + CPU_ID_MIN - 1;
		for (; cpu > CPU_ID_MIN; cpu--)
		{
			if (!used->get(used, (void*)(uintptr_t)cpu))
			{
				break;
			}
		}
	}
	used->destroy(used);
	return cpu;
}

/**
 * Add a KE payload if a key exchange is used.  As responder we might already
 * have stored the object in the list of completed exchanges.
 */
static bool add_ke_payload(private_child_create_t *this,
						   message_t *message)
{
	key_exchange_t *ke;
	ke_payload_t *pld;

	if (this->ke)
	{
		ke = this->ke;
	}
	else if (!array_get(this->kes, ARRAY_TAIL, &ke))
	{
		return TRUE;
	}

	pld = ke_payload_create_from_key_exchange(PLV2_KEY_EXCHANGE, ke);
	if (!pld)
	{
		DBG1(DBG_IKE, "creating KE payload failed");
		return FALSE;
	}
	message->add_payload(message, (payload_t*)pld);
	return TRUE;
}

/**
 * Build payloads in additional exchanges when using multiple key exchanges
 */
static bool build_payloads_multi_ke(private_child_create_t *this,
									message_t *message)
{
	if (!add_ke_payload(this, message))
	{
		return FALSE;
	}
	if (this->link.ptr)
	{
		message->add_notify(message, FALSE, ADDITIONAL_KEY_EXCHANGE, this->link);
	}
	return TRUE;
}

/**
 * build the payloads for the message
 */
static bool build_payloads(private_child_create_t *this, message_t *message)
{
	sa_payload_t *sa_payload;
	nonce_payload_t *nonce_payload;
	ts_payload_t *ts_payload;
	kernel_feature_t features;

	if (message->get_exchange_type(message) == IKE_FOLLOWUP_KE)
	{
		return build_payloads_multi_ke(this, message);
	}

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

	if (this->link.ptr)
	{
		message->add_notify(message, FALSE, ADDITIONAL_KEY_EXCHANGE, this->link);
	}

	if (!add_ke_payload(this, message))
	{
		return FALSE;
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
		case MODE_IPTFS:
		{
			uint8_t iptfs_flags = 0;

			if (!lib->settings->get_bool(lib->settings,
									"%s.iptfs.accept_fragments", TRUE, lib->ns))
			{
				iptfs_flags |= IPTFS_DONT_FRAGMENT;
			}
			message->add_notify(message, FALSE, USE_AGGFRAG,
								chunk_from_thing(iptfs_flags));
			break;
		}
		default:
			break;
	}

	features = charon->kernel->get_features(charon->kernel);
	if (!(features & KERNEL_ESP_V3_TFC))
	{
		message->add_notify(message, FALSE, ESP_TFC_PADDING_NOT_SUPPORTED,
							chunk_empty);
	}

	if (!this->rekey && this->child.per_cpu)
	{
		chunk_t cpu_id = chunk_empty;
		uint32_t cpu;

		/* always send the notify, but possibly without CPU ID */
		if (this->child.cpu != CPU_ID_MAX)
		{
			cpu = htonl(this->child.cpu);
			cpu_id = chunk_from_thing(cpu);
		}
		message->add_notify(message, FALSE, SA_RESOURCE_INFO,
							cpu_id);
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
	chunk_t data;

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
		case USE_AGGFRAG:
			this->mode = MODE_IPTFS;
			data = notify->get_notification_data(notify);
			this->iptfs_flags = *data.ptr;
			break;
		case IPCOMP_SUPPORTED:
		{
			ipcomp_transform_t ipcomp;
			uint16_t cpi;

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
		case SA_RESOURCE_INFO:
			chunk_free(&this->resource_info);
			this->resource_info = chunk_clone(notify->get_notification_data(notify));
			this->resource_info_seen = TRUE;
			break;
		default:
			break;
	}
}

/**
 * Collect all key exchanges from the proposal
 */
static void determine_key_exchanges(private_child_create_t *this)
{
	transform_type_t t = KEY_EXCHANGE_METHOD;
	uint16_t alg;
	int i = 1;

	if (!this->proposal->get_algorithm(this->proposal, t, &alg, NULL))
	{	/* no PFS */
		return;
	}

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
static bool additional_key_exchange_required(private_child_create_t *this)
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
static void clear_key_exchanges(private_child_create_t *this)
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
static void process_ke_payload(private_child_create_t *this, ke_payload_t *ke)
{
	key_exchange_method_t method = this->key_exchanges[this->ke_index].method;
	key_exchange_method_t received = ke->get_key_exchange_method(ke);

	/* the proposal is selected after processing the KE payload, so this is
	 * only relevant for additional key exchanges */
	if (method && method != received)
	{
		DBG1(DBG_IKE, "key exchange method in received payload %N doesn't "
			 "match negotiated %N", key_exchange_method_names, received,
			 key_exchange_method_names, method);
		this->ke_failed = TRUE;
		return;
	}

	this->ke_method = received;

	if (!this->initiator)
	{
		DESTROY_IF(this->ke);
		this->ke = this->keymat->keymat.create_ke(&this->keymat->keymat,
												  received);
		if (!this->ke)
		{
			DBG1(DBG_IKE, "key exchange method %N not supported",
				 key_exchange_method_names, received);
		}
	}
	else if (this->ke)
	{
		if (this->ke->get_method(this->ke) != received)
		{
			DBG1(DBG_IKE, "key exchange method %N in received payload doesn't "
				 "match %N", key_exchange_method_names, received,
				 key_exchange_method_names, this->ke->get_method(this->ke));
			this->ke_failed = TRUE;
		}
	}

	if (this->ke && !this->ke_failed)
	{
		if (!this->ke->set_public_key(this->ke, ke->get_key_exchange_data(ke)))
		{
			DBG1(DBG_IKE, "applying key exchange public key failed");
			this->ke_failed = TRUE;
		}
	}
}

/**
 * Check if the proposed KE method in CREATE_CHILD_SA (received via KE payload)
 * is valid according to the selected proposal.
 */
static bool check_ke_method(private_child_create_t *this, uint16_t *req)
{
	uint16_t alg;

	if (!this->proposal->has_transform(this->proposal, KEY_EXCHANGE_METHOD,
									   this->ke_method))
	{
		if (this->proposal->get_algorithm(this->proposal, KEY_EXCHANGE_METHOD,
										  &alg, NULL))
		{
			if (req)
			{
				*req = alg;
			}
			return FALSE;
		}
		/* the selected proposal does not use a key exchange method */
		DBG1(DBG_IKE, "ignoring KE payload, agreed on a non-PFS proposal");
		DESTROY_IF(this->ke);
		this->ke = NULL;
		this->ke_method = KE_NONE;
		/* ignore errors that occurred while handling the KE payload */
		this->ke_failed = FALSE;
	}
	return TRUE;
}

/**
 * Check if the proposed key exchange method is valid as responder or whether
 * we should request another KE payload.
 */
static bool check_ke_method_r(private_child_create_t *this, message_t *message)
{
	uint16_t alg;

	if (!check_ke_method(this, &alg))
	{
		DBG1(DBG_IKE, "key exchange method %N unacceptable, requesting %N",
			 key_exchange_method_names, this->ke_method,
			 key_exchange_method_names, alg);
		alg = htons(alg);
		message->add_notify(message, FALSE, INVALID_KE_PAYLOAD,
							chunk_from_thing(alg));
		return FALSE;
	}
	else if (this->ke_method != KE_NONE && !this->ke)
	{
		message->add_notify(message, TRUE, NO_PROPOSAL_CHOSEN, chunk_empty);
		return FALSE;
	}
	return TRUE;
}

/**
 * Read payloads from message
 */
static void process_payloads(private_child_create_t *this, message_t *message)
{
	enumerator_t *enumerator;
	payload_t *payload;
	sa_payload_t *sa_payload;
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
				process_ke_payload(this, (ke_payload_t*)payload);
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
 * Compare the reqids and possibly traffic selectors of two CHILD_SAs for
 * equality.
 *
 * The second CHILD_SA is assumed to be the one newly created.
 */
static bool reqid_and_ts_equals(private_child_create_t *this, child_sa_t *a,
								child_sa_t *b)
{
	/* reqids are allocated based on the traffic selectors. if all the other
	 * selectors are the same, they can only differ if narrowing occurred.
	 *
	 * if the new SA has no reqid assigned, it was initiated manually or due to
	 * a start action and we assume the peer will do the same narrowing it
	 * possibly did before, so we treat the SAs as equal.
	 *
	 * if the new SA has a reqid, it was either triggered by an acquire or
	 * during a reestablishment.  if they are equal, we are done */
	if (!b->get_reqid(b) || a->get_reqid(a) == b->get_reqid(b))
	{
		return TRUE;
	}
	/* if the reqids differ, the one of the established SA was changed due to
	 * narrowing.  in this case, we check if we have either triggering TS or
	 * previous TS.  if so, we check whether the available TS match the TS of
	 * the existing SA.  if they do, there is no point to negotiate another SA.
	 * if not, the peer will potentially narrow the TS to a different set for
	 * the new SA. */
	if (this->packet_tsi && this->packet_tsr)
	{
		return child_sa_ts_match(a, this->packet_tsi, this->packet_tsr);
	}
	if (this->my_ts && this->other_ts)
	{
		return child_sa_ts_lists_match(a, this->my_ts, this->other_ts);
	}
	/* if we don't have any TS to compare, we assume the peer will do the same
	 * narrowing and treat the SAs equal.*/
	return TRUE;
}

/**
 * Compare two CHILD_SA objects for equality.
 *
 * The second CHILD_SA is assumed to be the one newly created.
 */
static bool child_sa_equals(private_child_create_t *this, child_sa_t *a,
							child_sa_t *b)
{
	child_cfg_t *cfg = a->get_config(a);
	return cfg->equals(cfg, b->get_config(b)) &&
		a->get_mark(a, TRUE).value == b->get_mark(b, TRUE).value &&
		a->get_mark(a, FALSE).value == b->get_mark(b, FALSE).value &&
		a->get_if_id(a, TRUE) == b->get_if_id(b, TRUE) &&
		a->get_if_id(a, FALSE) == b->get_if_id(b, FALSE) &&
		a->get_cpu(a) == b->get_cpu(b) &&
		sec_labels_equal(a->get_label(a), b->get_label(b)) &&
		reqid_and_ts_equals(this, a, b);
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
			child_sa_equals(this, child_sa, this->child_sa))
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

METHOD(task_t, build_i_multi_ke, status_t,
	private_child_create_t *this, message_t *message)
{
	key_exchange_method_t method;

	message->set_exchange_type(message, IKE_FOLLOWUP_KE);
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
	if (!this->link.ptr)
	{
		DBG1(DBG_IKE, "%N notify missing", notify_type_names,
			 ADDITIONAL_KEY_EXCHANGE);
		return FAILED;
	}

	if (!build_payloads_multi_ke(this, message))
	{
		return FAILED;
	}
	return NEED_MORE;
}

/**
 * Prepare proposed traffic selectors as initiator.
 */
static void prepare_proposed_ts(private_child_create_t *this)
{
	enumerator_t *enumerator;
	peer_cfg_t *peer_cfg;
	linked_list_t *list;
	host_t *vip;

	ensure_ts_lists(this);

	list = linked_list_create();
	if (!this->rekey)
	{
		/* check if we want a virtual IP */
		peer_cfg = this->ike_sa->get_peer_cfg(this->ike_sa);
		enumerator = peer_cfg->create_virtual_ip_enumerator(peer_cfg);
		while (enumerator->enumerate(enumerator, &vip))
		{
			/* propose a 0.0.0.0/0 or ::/0 subnet when we use a virtual IP */
			vip = host_create_any(vip->get_family(vip));
			list->insert_last(list, vip);
		}
		enumerator->destroy(enumerator);
	}
	if (list->get_count(list))
	{
		this->tsi = child_cfg_select_ts(this->config, TRUE, this->my_ts, NULL,
										list);
		list->destroy_offset(list, offsetof(host_t, destroy));
	}
	else
	{
		list->destroy(list);
		list = ike_sa_get_dynamic_hosts(this->ike_sa, TRUE);
		this->tsi = child_cfg_select_ts(this->config, TRUE, this->my_ts, NULL,
										list);
		list->destroy(list);
	}
	list = ike_sa_get_dynamic_hosts(this->ike_sa, FALSE);
	this->tsr = child_cfg_select_ts(this->config, FALSE, this->other_ts, NULL,
									list);
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
}

METHOD(task_t, build_i, status_t,
	private_child_create_t *this, message_t *message)
{
	bool no_ke = TRUE;

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
			no_ke = FALSE;
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

	prepare_proposed_ts(this);

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
	if (!this->rekey)
	{
		this->child.per_cpu = this->config->has_option(this->config,
													   OPT_PER_CPU_SAS);
	}

	this->proposals = this->config->get_proposals(this->config, no_ke);
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
		return FAILED;
	}

	if (no_ke)
	{	/* we might have one set if we are recreating this SA */
		this->ke_method = KE_NONE;
	}
	else if (!this->retry)
	{	/* during a rekeying the method might already be set */
		if (this->ke_method == KE_NONE)
		{
			this->ke_method = this->config->get_algorithm(this->config,
														  KEY_EXCHANGE_METHOD);
		}
	}

	if (!update_and_check_proposals(this))
	{
		DBG1(DBG_IKE, "requested key exchange method %N not contained in any "
			 "of our proposals", key_exchange_method_names, this->ke_method);
		return FAILED;
	}

	if (this->ke_method != KE_NONE)
	{
		this->ke = this->keymat->keymat.create_ke(&this->keymat->keymat,
												  this->ke_method);
		if (!this->ke)
		{
			DBG1(DBG_IKE, "selected key exchange method %N not supported",
				 key_exchange_method_names, this->ke_method);
			return FAILED;
		}
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

/**
 * Process payloads in a IKE_FOLLOWUP_KE message or a CREATE_CHILD_SA response
 */
static void process_link(private_child_create_t *this, message_t *message)
{
	notify_payload_t *notify;
	chunk_t link;

	notify = message->get_notify(message, ADDITIONAL_KEY_EXCHANGE);
	if (notify)
	{
		link = notify->get_notification_data(notify);
		if (this->initiator)
		{
			chunk_free(&this->link);
			this->link = chunk_clone(link);
		}
		else if (!chunk_equals_const(this->link, link))
		{
			DBG1(DBG_IKE, "data in %N notify doesn't match", notify_type_names,
				 ADDITIONAL_KEY_EXCHANGE);
			chunk_free(&this->link);
		}
	}
	else
	{
		chunk_free(&this->link);
	}
}

/**
 * Process payloads in additional exchanges when using multiple key exchanges
 */
static void process_payloads_multi_ke(private_child_create_t *this,
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
		this->ke_failed = TRUE;
	}
	process_link(this, message);
}

METHOD(task_t, process_r_multi_ke, status_t,
	private_child_create_t *this, message_t *message)
{
	if (message->get_exchange_type(message) == IKE_FOLLOWUP_KE)
	{
		process_payloads_multi_ke(this, message);
	}
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

	if (this->aborted)
	{
		return;
	}

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
	child_cfg_t *child_cfg = NULL;

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

/**
 * Handle per-resource information.
 */
static void handle_per_resource(private_child_create_t *this)
{
	if (!this->resource_info_seen)
	{
		DBG1(DBG_IKE, "peer didn't send %N notify, creating regular SA",
			 notify_type_names, SA_RESOURCE_INFO);
		this->child.per_cpu = FALSE;
		this->child.cpu = CPU_ID_MAX;
		return;
	}

	/* as responder, assign a CPU ID (or CPU_ID_MAX if we don't have a
	 * fallback SA yet) */
	if (!this->initiator)
	{
		this->child.per_cpu = TRUE;
		this->child.cpu = get_cpu(this);
	}
	if (this->child.cpu != CPU_ID_MAX && this->resource_info.len)
	{
		DBG1(DBG_IKE, "creating per-resource SA as one of several (our ID %u, "
			 "peer's ID %+B)", this->child.cpu, &this->resource_info);
	}
	else if (this->child.cpu != CPU_ID_MAX)
	{
		DBG1(DBG_IKE, "creating per-resource SA as one of several (our ID %u)",
			 this->child.cpu);
	}
	else if (this->resource_info.len)
	{
		DBG1(DBG_IKE, "creating SA as one of several per-resource SAs (peer's "
			 "ID %+B)", &this->resource_info);
	}
	else
	{
		DBG1(DBG_IKE, "creating SA as one of several per-resource SAs");
	}
}

/**
 * Called when a key exchange is done, returns TRUE once all are done.
 */
static bool key_exchange_done(private_child_create_t *this)
{
	bool additional_ke;

	if (!this->ke)
	{
		return TRUE;
	}

	this->key_exchanges[this->ke_index++].done = TRUE;
	additional_ke = additional_key_exchange_required(this);

	array_insert_create(&this->kes, ARRAY_TAIL, this->ke);
	this->ke = NULL;

	return additional_ke ? FALSE : TRUE;
}

/**
 * Complete the current key exchange and install the CHILD_SA if all are done
 * as responder.
 */
static bool key_exchange_done_and_install_r(private_child_create_t *this,
											message_t *message, bool ike_auth)
{
	bool all_done = FALSE;

	if (key_exchange_done(this))
	{
		chunk_clear(&this->link);
		all_done = TRUE;
	}
	else if (!this->link.ptr)
	{
		this->link = chunk_clone(chunk_from_chars(0x42));
	}

	if (!build_payloads(this, message))
	{
		message->add_notify(message, FALSE, NO_PROPOSAL_CHOSEN, chunk_empty);
		handle_child_sa_failure(this, message);
		return TRUE;
	}

	if (all_done)
	{
		switch (install_child_sa(this))
		{
			case SUCCESS:
				break;
			case NOT_FOUND:
				message->add_notify(message, TRUE, TS_UNACCEPTABLE,
									chunk_empty);
				handle_child_sa_failure(this, message);
				return TRUE;
			case FAILED:
			default:
				message->add_notify(message, TRUE, NO_PROPOSAL_CHOSEN,
									chunk_empty);
				handle_child_sa_failure(this, message);
				return TRUE;
		}
		if (!this->rekey)
		{	/* invoke the child_up() hook if we are not rekeying */
			charon->bus->child_updown(charon->bus, this->child_sa, TRUE);
		}
	}
	return all_done;
}

METHOD(task_t, build_r_multi_ke, status_t,
	private_child_create_t *this, message_t *message)
{
	if (!this->ke)
	{
		message->add_notify(message, FALSE, INVALID_SYNTAX, chunk_empty);
		handle_child_sa_failure(this, message);
		return SUCCESS;
	}
	if (this->ke_failed)
	{
		message->add_notify(message, FALSE, NO_PROPOSAL_CHOSEN, chunk_empty);
		handle_child_sa_failure(this, message);
		return SUCCESS;
	}
	if (!this->link.ptr)
	{
		DBG1(DBG_IKE, "%N notify missing", notify_type_names,
			 ADDITIONAL_KEY_EXCHANGE);
		message->add_notify(message, FALSE, STATE_NOT_FOUND, chunk_empty);
		handle_child_sa_failure(this, message);
		return SUCCESS;
	}
	if (!key_exchange_done_and_install_r(this, message, FALSE))
	{
		return NEED_MORE;
	}
	return SUCCESS;
}

METHOD(task_t, build_r, status_t,
	private_child_create_t *this, message_t *message)
{
	payload_t *payload;
	enumerator_t *enumerator;
	bool no_ke = TRUE, ike_auth = FALSE;

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
			no_ke = FALSE;
			break;
		case IKE_AUTH:
			if (!this->ike_sa->has_condition(this->ike_sa, COND_AUTHENTICATED))
			{	/* wait until all authentication rounds completed */
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

	if (!this->config)
	{
		this->config = select_child_cfg(this);
	}
	if (!this->config || !this->tsi || !this->tsr)
	{
		if (!this->tsi || !this->tsr)
		{
			DBG1(DBG_IKE, "TS payloads missing in message");
		}
		else
		{
			DBG1(DBG_IKE, "traffic selectors %#R === %#R unacceptable",
				 this->tsr, this->tsi);
			charon->bus->alert(charon->bus, ALERT_TS_MISMATCH, this->tsi,
							   this->tsr);
		}
		message->add_notify(message, FALSE, TS_UNACCEPTABLE, chunk_empty);
		handle_child_sa_failure(this, message);
		return SUCCESS;
	}

	/* check if ike_config_t task included non-critical error notifies */
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

	if (!select_proposal(this, no_ke))
	{
		message->add_notify(message, FALSE, NO_PROPOSAL_CHOSEN, chunk_empty);
		handle_child_sa_failure(this, message);
		return SUCCESS;
	}

	if (!check_ke_method_r(this, message))
	{	/* the peer will retry, we don't handle this as failure */
		return SUCCESS;
	}

	/* this flag might get reset if the check above notices a proposal without
	 * KE was selected */
	if (this->ke_failed)
	{
		message->add_notify(message, FALSE, NO_PROPOSAL_CHOSEN, chunk_empty);
		handle_child_sa_failure(this, message);
		return SUCCESS;
	}

	determine_key_exchanges(this);

	if (!select_label(this))
	{
		message->add_notify(message, FALSE, TS_UNACCEPTABLE, chunk_empty);
		handle_child_sa_failure(this, message);
		return SUCCESS;
	}

	this->child.if_id_in_def = this->ike_sa->get_if_id(this->ike_sa, TRUE);
	this->child.if_id_out_def = this->ike_sa->get_if_id(this->ike_sa, FALSE);
	this->child.encap = this->ike_sa->has_condition(this->ike_sa, COND_NAT_ANY);

	if (!this->rekey && this->config->has_option(this->config, OPT_PER_CPU_SAS))
	{
		handle_per_resource(this);
		/* if it's a per-resource SA, we could check if we already have
		 * "too many" and reject this one with TS_MAX_QUEUE, but for now we just
		 * keep it */
	}

	this->child_sa = child_sa_create(this->ike_sa->get_my_host(this->ike_sa),
									 this->ike_sa->get_other_host(this->ike_sa),
									 this->config, &this->child);

	this->other_spi = this->proposal->get_spi(this->proposal);
	if (!allocate_spi(this))
	{
		message->add_notify(message, FALSE, NO_PROPOSAL_CHOSEN, chunk_empty);
		handle_child_sa_failure(this, message);
		return SUCCESS;
	}
	this->proposal->set_spi(this->proposal, this->my_spi);

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

	switch (narrow_and_check_ts(this, ike_auth))
	{
		case SUCCESS:
			break;
		case NOT_FOUND:
			message->add_notify(message, FALSE, TS_UNACCEPTABLE, chunk_empty);
			handle_child_sa_failure(this, message);
			return SUCCESS;
		case FAILED:
		default:
			message->add_notify(message, FALSE, NO_PROPOSAL_CHOSEN, chunk_empty);
			handle_child_sa_failure(this, message);
			return SUCCESS;
	}

	if (!key_exchange_done_and_install_r(this, message, ike_auth))
	{
		this->public.task.build = _build_r_multi_ke;
		this->public.task.process = _process_r_multi_ke;
		return NEED_MORE;
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
		case TS_UNACCEPTABLE:
			charon->bus->alert(charon->bus, ALERT_TS_MISMATCH,
							   this->tsi, this->tsr);
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
	return SUCCESS;
}

/**
 * Change task to delete the failed CHILD_SA as initiator
 */
static status_t delete_failed_sa(private_child_create_t *this)
{
	if (this->my_spi && this->proto)
	{
		this->public.task.build = _build_i_delete;
		/* destroying it here allows the rekey task to differentiate between
		 * this and the multi-KE case */
		this->child_sa->destroy(this->child_sa);
		this->child_sa = NULL;
		return NEED_MORE;
	}
	return SUCCESS;
}

/**
 * Complete the current key exchange and install the CHILD_SA if all are done
 * as initiator.
 */
static status_t key_exchange_done_and_install_i(private_child_create_t *this,
												message_t *message, bool ike_auth)
{
	if (key_exchange_done(this))
	{
		if (install_child_sa(this) == SUCCESS)
		{
			if (!this->rekey)
			{	/* invoke the child_up() hook if we are not rekeying */
				charon->bus->child_updown(charon->bus, this->child_sa,
										  TRUE);
			}
			return SUCCESS;
		}
		handle_child_sa_failure(this, message);
		return delete_failed_sa(this);
	}
	return NEED_MORE;
}

METHOD(task_t, process_i_multi_ke, status_t,
	private_child_create_t *this, message_t *message)
{
	if (message->get_notify(message, TEMPORARY_FAILURE))
	{
		DBG1(DBG_IKE, "received %N notify", notify_type_names,
			 TEMPORARY_FAILURE);
		if (!this->rekey && !this->aborted)
		{	/* the rekey task will retry itself if necessary */
			schedule_delayed_retry(this);
		}
		return SUCCESS;
	}

	process_payloads_multi_ke(this, message);

	if (this->ke_failed || this->aborted)
	{
		handle_child_sa_failure(this, message);
		return delete_failed_sa(this);
	}

	return key_exchange_done_and_install_i(this, message, FALSE);
}

METHOD(task_t, process_i, status_t,
	private_child_create_t *this, message_t *message)
{
	enumerator_t *enumerator;
	payload_t *payload;
	bool no_ke = TRUE, ike_auth = FALSE;

	switch (message->get_exchange_type(message))
	{
		case IKE_SA_INIT:
			return get_nonce(message, &this->other_nonce);
		case CREATE_CHILD_SA:
			get_nonce(message, &this->other_nonce);
			no_ke = FALSE;
			break;
		case IKE_AUTH:
			if (!this->ike_sa->has_condition(this->ike_sa, COND_AUTHENTICATED))
			{	/* wait until all authentication rounds completed */
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
					DBG1(DBG_IKE, "received %N notify", notify_type_names, type);
					enumerator->destroy(enumerator);
					if (!this->rekey && !this->aborted)
					{	/* the rekey task will retry itself if necessary */
						schedule_delayed_retry(this);
					}
					return SUCCESS;
				}
				case INVALID_KE_PAYLOAD:
				{
					chunk_t data;
					uint16_t alg = KE_NONE;

					if (this->aborted)
					{	/* nothing to do if the task was aborted */
						DBG1(DBG_IKE, "received %N notify in aborted %N task",
							 notify_type_names, type, task_type_names,
							 TASK_CHILD_CREATE);
						enumerator->destroy(enumerator);
						return SUCCESS;
					}
					data = notify->get_notification_data(notify);
					if (data.len == sizeof(alg))
					{
						alg = untoh16(data.ptr);
					}
					if (this->retry)
					{
						DBG1(DBG_IKE, "already retried with key exchange method "
							 "%N, ignore requested %N", key_exchange_method_names,
							 this->ke_method, key_exchange_method_names, alg);
						handle_child_sa_failure(this, message);
						/* an error in CHILD_SA creation is not critical */
						return SUCCESS;
					}
					DBG1(DBG_IKE, "peer didn't accept key exchange method %N, "
						 "it requested %N", key_exchange_method_names,
						 this->ke_method, key_exchange_method_names, alg);
					this->retry = TRUE;
					this->ke_method = alg;
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

	if (!select_proposal(this, no_ke))
	{
		handle_child_sa_failure(this, message);
		return delete_failed_sa(this);
	}

	this->other_spi = this->proposal->get_spi(this->proposal);
	this->proposal->set_spi(this->proposal, this->my_spi);

	if (this->aborted)
	{
		DBG1(DBG_IKE, "deleting CHILD_SA %s{%d} with SPIs %.8x_i %.8x_o of "
			 "aborted %N task",
			 this->child_sa->get_name(this->child_sa),
			 this->child_sa->get_unique_id(this->child_sa),
			 ntohl(this->my_spi), ntohl(this->other_spi),
			 task_type_names, TASK_CHILD_CREATE);
		return delete_failed_sa(this);
	}

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

	if (!check_ke_method(this, NULL))
	{
		handle_child_sa_failure(this, message);
		return delete_failed_sa(this);
	}

	if (this->ke_failed)
	{
		handle_child_sa_failure(this, message);
		return delete_failed_sa(this);
	}

	determine_key_exchanges(this);

	if (!select_label(this))
	{
		handle_child_sa_failure(this, message);
		return delete_failed_sa(this);
	}

	if (narrow_and_check_ts(this, ike_auth) != SUCCESS)
	{
		handle_child_sa_failure(this, message);
		return delete_failed_sa(this);
	}

	if (!this->rekey && this->config->has_option(this->config, OPT_PER_CPU_SAS))
	{
		handle_per_resource(this);
		/* the peer might not have returned the notify, update the flag */
		this->child_sa->set_per_cpu(this->child_sa, this->child.per_cpu);
	}

	if (key_exchange_done_and_install_i(this, message, ike_auth) == NEED_MORE)
	{
		/* if the installation failed, we delete the failed SA, i.e. build() was
		 * changed, otherwise, we switch to multi-KE mode */
		if (this->public.task.build == _build_i)
		{
			/* if we don't have the notify, we handle it in build() */
			process_link(this, message);
			this->public.task.build = _build_i_multi_ke;
			this->public.task.process = _process_i_multi_ke;
		}
		return NEED_MORE;
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

METHOD(child_create_t, use_per_cpu, void,
	private_child_create_t *this, bool per_cpu, uint32_t cpu)
{
	this->child.per_cpu = per_cpu ? per_cpu : cpu != CPU_ID_MAX;
	this->child.cpu = cpu;
}

/**
 * Prepare traffic selectors for reuse when recreating a CHILD_SA.
 */
static void reuse_ts(private_child_create_t *this, bool local, child_sa_t *old,
					 traffic_selector_list_t **target)
{
	enumerator_t *old_ts, *hosts_enum;
	linked_list_t *hosts, *list;
	traffic_selector_t *ts, *new_ts;
	host_t *host;

	old_ts = old->create_ts_enumerator(old, local);
	if (this->rekey)
	{
		/* when rekeying, we just reuse the previous TS. this is also the only
		 * way a responder reuses TS */
		*target = traffic_selector_list_create_from_enumerator(old_ts);
		return;
	}

	/* when recreating/reauthenticating, we check whether the dynamic IPs of
	 * the IKE_SA (as copied from the old SA) match the TS and replace
	 * them with dynamic TS (reusing protocol/ports in case of narrowing) so
	 * they get updated to possibly new IPs when the TS are prepared later */
	list = linked_list_create();
	hosts = ike_sa_get_dynamic_hosts(this->ike_sa, local);
	hosts_enum = hosts->create_enumerator(hosts);
	while (old_ts->enumerate(old_ts, &ts))
	{
		new_ts = NULL;
		while (hosts_enum->enumerate(hosts_enum, &host))
		{
			if (ts->is_host(ts, host))
			{
				new_ts = traffic_selector_create_dynamic(ts->get_protocol(ts),
														 ts->get_from_port(ts),
														 ts->get_to_port(ts));
				break;
			}
		}
		hosts->reset_enumerator(hosts, hosts_enum);

		if (!new_ts)
		{
			new_ts = ts->clone(ts);
		}
		list->insert_last(list, new_ts);
	}
	hosts_enum->destroy(hosts_enum);
	hosts->destroy(hosts);
	old_ts->destroy(old_ts);

	*target = traffic_selector_list_create_from_list(list);
}

METHOD(child_create_t, recreate_sa, void,
	private_child_create_t *this, child_sa_t *old)
{
	if (this->initiator)
	{
		proposal_t *proposal;
		uint16_t ke_method;

		proposal = old->get_proposal(old);
		if (proposal->get_algorithm(proposal, KEY_EXCHANGE_METHOD,
									&ke_method, NULL))
		{
			/* reuse the KE method negotiated previously */
			this->ke_method = ke_method;
		}
	}

	/* use previously negotiated traffic selectors */
	reuse_ts(this, TRUE, old, &this->my_ts);
	reuse_ts(this, FALSE, old, &this->other_ts);
}

METHOD(child_create_t, get_child, child_sa_t*,
	private_child_create_t *this)
{
	return this->child_sa;
}

METHOD(child_create_t, get_other_spi, uint32_t,
	private_child_create_t *this)
{
	return this->other_spi;
}

METHOD(child_create_t, set_config, void,
	private_child_create_t *this, child_cfg_t *cfg)
{
	DESTROY_IF(this->config);
	this->config = cfg;
}

METHOD(child_create_t, get_config, child_cfg_t*,
	private_child_create_t *this)
{
	return this->initiator ? this->config : NULL;
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

METHOD(child_create_t, abort_, void,
	private_child_create_t *this)
{
	this->aborted = TRUE;
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
	chunk_free(&this->link);
	chunk_free(&this->resource_info);
	DESTROY_OFFSET_IF(this->tsr, offsetof(traffic_selector_t, destroy));
	DESTROY_OFFSET_IF(this->tsi, offsetof(traffic_selector_t, destroy));
	DESTROY_OFFSET_IF(this->labels_i, offsetof(sec_label_t, destroy));
	DESTROY_OFFSET_IF(this->labels_r, offsetof(sec_label_t, destroy));
	DESTROY_IF(this->child_sa);
	DESTROY_IF(this->proposal);
	DESTROY_IF(this->nonceg);
	DESTROY_IF(this->ke);
	this->ke_failed = FALSE;
	clear_key_exchanges(this);
	DESTROY_OFFSET_IF(this->proposals, offsetof(proposal_t, destroy));
	if (!this->rekey && !this->retry)
	{
		this->ke_method = KE_NONE;
	}
	this->ike_sa = ike_sa;
	this->keymat = (keymat_v2_t*)ike_sa->get_keymat(ike_sa);
	this->proposal = NULL;
	this->proposals = NULL;
	this->tsi = NULL;
	this->tsr = NULL;
	this->labels_i = NULL;
	this->labels_r = NULL;
	this->ke = NULL;
	this->nonceg = NULL;
	this->child_sa = NULL;
	this->mode = MODE_TUNNEL;
	this->ipcomp = IPCOMP_NONE;
	this->ipcomp_received = IPCOMP_NONE;
	this->other_cpi = 0;
	this->established = FALSE;
	this->resource_info_seen = FALSE;
	this->public.task.build = _build_i;
	this->public.task.process = _process_i;
}

METHOD(task_t, destroy, void,
	private_child_create_t *this)
{
	chunk_free(&this->my_nonce);
	chunk_free(&this->other_nonce);
	chunk_free(&this->link);
	chunk_free(&this->resource_info);
	DESTROY_OFFSET_IF(this->tsr, offsetof(traffic_selector_t, destroy));
	DESTROY_OFFSET_IF(this->tsi, offsetof(traffic_selector_t, destroy));
	DESTROY_OFFSET_IF(this->labels_i, offsetof(sec_label_t, destroy));
	DESTROY_OFFSET_IF(this->labels_r, offsetof(sec_label_t, destroy));
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
	DESTROY_IF(this->my_ts);
	DESTROY_IF(this->other_ts);
	DESTROY_IF(this->proposal);
	DESTROY_IF(this->ke);
	clear_key_exchanges(this);
	DESTROY_OFFSET_IF(this->proposals, offsetof(proposal_t, destroy));
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
									traffic_selector_t *tsi,
									traffic_selector_t *tsr, uint32_t seq)
{
	private_child_create_t *this;

	INIT(this,
		.public = {
			.get_child = _get_child,
			.get_other_spi = _get_other_spi,
			.set_config = _set_config,
			.get_config = _get_config,
			.get_lower_nonce = _get_lower_nonce,
			.use_reqid = _use_reqid,
			.use_marks = _use_marks,
			.use_if_ids = _use_if_ids,
			.use_label = _use_label,
			.use_per_cpu = _use_per_cpu,
			.recreate_sa = _recreate_sa,
			.abort = _abort_,
			.task = {
				.get_type = _get_type,
				.migrate = _migrate,
				.destroy = _destroy,
			},
		},
		.child = {
			.cpu = CPU_ID_MAX,
			.seq = seq,
		},
		.ike_sa = ike_sa,
		.config = config,
		.packet_tsi = tsi ? tsi->clone(tsi) : NULL,
		.packet_tsr = tsr ? tsr->clone(tsr) : NULL,
		.ke_method = KE_NONE,
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
