/*
 * Copyright (C) 2006-2018 Tobias Brunner
 * Copyright (C) 2016 Andreas Steffen
 * Copyright (C) 2005-2008 Martin Willi
 * Copyright (C) 2006 Daniel Roethlisberger
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

#define _GNU_SOURCE
#include "child_sa.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#include <daemon.h>
#include <collections/array.h>

ENUM(child_sa_state_names, CHILD_CREATED, CHILD_DESTROYING,
	"CREATED",
	"ROUTED",
	"INSTALLING",
	"INSTALLED",
	"UPDATING",
	"REKEYING",
	"REKEYED",
	"RETRYING",
	"DELETING",
	"DELETED",
	"DESTROYING",
);

ENUM_FLAGS(child_sa_outbound_state_names, CHILD_OUTBOUND_REGISTERED, CHILD_OUTBOUND_POLICIES,
	"REGISTERED",
	"SA",
	"POLICIES",
);

typedef struct private_child_sa_t private_child_sa_t;

/**
 * Private data of a child_sa_t object.
 */
struct private_child_sa_t {
	/**
	 * Public interface of child_sa_t.
	 */
	child_sa_t public;

	/**
	 * address of us
	 */
	host_t *my_addr;

	/**
	 * address of remote
	 */
	host_t *other_addr;

	/**
	 * our actually used SPI, 0 if unused
	 */
	uint32_t my_spi;

	/**
	 * others used SPI, 0 if unused
	 */
	uint32_t other_spi;

	/**
	 * our Compression Parameter Index (CPI) used, 0 if unused
	 */
	uint16_t my_cpi;

	/**
	 * others Compression Parameter Index (CPI) used, 0 if unused
	 */
	uint16_t other_cpi;

	/**
	 * Array for local traffic selectors
	 */
	array_t *my_ts;

	/**
	 * Array for remote traffic selectors
	 */
	array_t *other_ts;

	/**
	 * Outbound encryption key cached during a rekeying
	 */
	chunk_t encr_r;

	/**
	 * Outbound integrity key cached during a rekeying
	 */
	chunk_t integ_r;

	/**
	 * Whether the outbound SA has only been registered yet during a rekeying
	 */
	child_sa_outbound_state_t outbound_state;

	/**
	 * Whether the peer supports TFCv3
	 */
	bool tfcv3;

	/**
	 * The outbound SPI of the CHILD_SA that replaced this one during a rekeying
	 */
	uint32_t rekey_spi;

	/**
	 * Protocol used to protect this SA, ESP|AH
	 */
	protocol_id_t protocol;

	/**
	 * reqid used for this child_sa
	 */
	uint32_t reqid;

	/**
	 * Did we allocate/confirm and must release the reqid?
	 */
	bool reqid_allocated;

	/**
	 * Is the reqid statically configured
	 */
	bool static_reqid;

	/**
	 * Unique CHILD_SA identifier
	 */
	uint32_t unique_id;

	/**
	 * Whether FWD policieis in the outbound direction should be installed
	 */
	bool policies_fwd_out;

	/**
	 * inbound mark used for this child_sa
	 */
	mark_t mark_in;

	/**
	 * outbound mark used for this child_sa
	 */
	mark_t mark_out;

	/**
	 * absolute time when rekeying is scheduled
	 */
	time_t rekey_time;

	/**
	 * absolute time when the SA expires
	 */
	time_t expire_time;

	/**
	 * absolute time when SA has been installed
	 */
	time_t install_time;

	/**
	 * state of the CHILD_SA
	 */
	child_sa_state_t state;

	/**
	 * TRUE if this CHILD_SA is used to install trap policies
	 */
	bool trap;

	/**
	 * Specifies if UDP encapsulation is enabled (NAT traversal)
	 */
	bool encap;

	/**
	 * Specifies the IPComp transform used (IPCOMP_NONE if disabled)
	 */
	ipcomp_transform_t ipcomp;

	/**
	 * mode this SA uses, tunnel/transport
	 */
	ipsec_mode_t mode;

	/**
	 * Action to enforce if peer closes the CHILD_SA
	 */
	action_t close_action;

	/**
	 * Action to enforce if peer is considered dead
	 */
	action_t dpd_action;

	/**
	 * selected proposal
	 */
	proposal_t *proposal;

	/**
	 * config used to create this child
	 */
	child_cfg_t *config;

	/**
	 * time of last use in seconds (inbound)
	 */
	time_t my_usetime;

	/**
	 * time of last use in seconds (outbound)
	 */
	time_t other_usetime;

	/**
	 * last number of inbound bytes
	 */
	uint64_t my_usebytes;

	/**
	 * last number of outbound bytes
	 */
	uint64_t other_usebytes;

	/**
	 * last number of inbound packets
	 */
	uint64_t my_usepackets;

	/**
	 * last number of outbound bytes
	 */
	uint64_t other_usepackets;
};

/**
 * Convert an IKEv2 specific protocol identifier to the IP protocol identifier
 */
static inline uint8_t proto_ike2ip(protocol_id_t protocol)
{
	switch (protocol)
	{
		case PROTO_ESP:
			return IPPROTO_ESP;
		case PROTO_AH:
			return IPPROTO_AH;
		default:
			return protocol;
	}
}

/**
 * Returns the mark to use on the inbound SA
 */
static inline mark_t mark_in_sa(private_child_sa_t *this)
{
	if (this->config->has_option(this->config, OPT_MARK_IN_SA))
	{
		return this->mark_in;
	}
	return (mark_t){};
}

METHOD(child_sa_t, get_name, char*,
	   private_child_sa_t *this)
{
	return this->config->get_name(this->config);
}

METHOD(child_sa_t, get_reqid, uint32_t,
	   private_child_sa_t *this)
{
	return this->reqid;
}

METHOD(child_sa_t, get_unique_id, uint32_t,
	private_child_sa_t *this)
{
	return this->unique_id;
}

METHOD(child_sa_t, get_config, child_cfg_t*,
	   private_child_sa_t *this)
{
	return this->config;
}

METHOD(child_sa_t, set_state, void,
	   private_child_sa_t *this, child_sa_state_t state)
{
	if (this->state != state)
	{
		DBG2(DBG_CHD, "CHILD_SA %s{%d} state change: %N => %N",
			 get_name(this), this->unique_id,
			 child_sa_state_names, this->state,
			 child_sa_state_names, state);
		charon->bus->child_state_change(charon->bus, &this->public, state);
		this->state = state;
	}
}

METHOD(child_sa_t, get_state, child_sa_state_t,
	   private_child_sa_t *this)
{
	return this->state;
}

METHOD(child_sa_t, get_outbound_state, child_sa_outbound_state_t,
	   private_child_sa_t *this)
{
	return this->outbound_state;
}

METHOD(child_sa_t, get_spi, uint32_t,
	   private_child_sa_t *this, bool inbound)
{
	return inbound ? this->my_spi : this->other_spi;
}

METHOD(child_sa_t, get_cpi, uint16_t,
	   private_child_sa_t *this, bool inbound)
{
	return inbound ? this->my_cpi : this->other_cpi;
}

METHOD(child_sa_t, get_protocol, protocol_id_t,
	   private_child_sa_t *this)
{
	return this->protocol;
}

METHOD(child_sa_t, set_protocol, void,
	   private_child_sa_t *this, protocol_id_t protocol)
{
	this->protocol = protocol;
}

METHOD(child_sa_t, get_mode, ipsec_mode_t,
	   private_child_sa_t *this)
{
	return this->mode;
}

METHOD(child_sa_t, set_mode, void,
	   private_child_sa_t *this, ipsec_mode_t mode)
{
	this->mode = mode;
}

METHOD(child_sa_t, has_encap, bool,
	   private_child_sa_t *this)
{
	return this->encap;
}

METHOD(child_sa_t, get_ipcomp, ipcomp_transform_t,
	   private_child_sa_t *this)
{
	return this->ipcomp;
}

METHOD(child_sa_t, set_ipcomp, void,
	   private_child_sa_t *this, ipcomp_transform_t ipcomp)
{
	this->ipcomp = ipcomp;
}

METHOD(child_sa_t, set_close_action, void,
	   private_child_sa_t *this, action_t action)
{
	this->close_action = action;
}

METHOD(child_sa_t, get_close_action, action_t,
	   private_child_sa_t *this)
{
	return this->close_action;
}

METHOD(child_sa_t, set_dpd_action, void,
	   private_child_sa_t *this, action_t action)
{
	this->dpd_action = action;
}

METHOD(child_sa_t, get_dpd_action, action_t,
	   private_child_sa_t *this)
{
	return this->dpd_action;
}

METHOD(child_sa_t, get_proposal, proposal_t*,
	   private_child_sa_t *this)
{
	return this->proposal;
}

METHOD(child_sa_t, set_proposal, void,
	   private_child_sa_t *this, proposal_t *proposal)
{
	this->proposal = proposal->clone(proposal);
}

METHOD(child_sa_t, create_ts_enumerator, enumerator_t*,
	private_child_sa_t *this, bool local)
{
	if (local)
	{
		return array_create_enumerator(this->my_ts);
	}
	return array_create_enumerator(this->other_ts);
}

typedef struct policy_enumerator_t policy_enumerator_t;

/**
 * Private policy enumerator
 */
struct policy_enumerator_t {
	/** implements enumerator_t */
	enumerator_t public;
	/** enumerator over own TS */
	enumerator_t *mine;
	/** enumerator over others TS */
	enumerator_t *other;
	/** array of others TS, to recreate enumerator */
	array_t *array;
	/** currently enumerating TS for "me" side */
	traffic_selector_t *ts;
};

METHOD(enumerator_t, policy_enumerate, bool,
	   policy_enumerator_t *this, va_list args)
{
	traffic_selector_t *other_ts, **my_out, **other_out;

	VA_ARGS_VGET(args, my_out, other_out);

	while (this->ts || this->mine->enumerate(this->mine, &this->ts))
	{
		if (!this->other->enumerate(this->other, &other_ts))
		{	/* end of others list, restart with new of mine */
			this->other->destroy(this->other);
			this->other = array_create_enumerator(this->array);
			this->ts = NULL;
			continue;
		}
		if (this->ts->get_type(this->ts) != other_ts->get_type(other_ts))
		{	/* family mismatch */
			continue;
		}
		if (this->ts->get_protocol(this->ts) &&
			other_ts->get_protocol(other_ts) &&
			this->ts->get_protocol(this->ts) != other_ts->get_protocol(other_ts))
		{	/* protocol mismatch */
			continue;
		}
		if (my_out)
		{
			*my_out = this->ts;
		}
		if (other_out)
		{
			*other_out = other_ts;
		}
		return TRUE;
	}
	return FALSE;
}

METHOD(enumerator_t, policy_destroy, void,
	   policy_enumerator_t *this)
{
	this->mine->destroy(this->mine);
	this->other->destroy(this->other);
	free(this);
}

METHOD(child_sa_t, create_policy_enumerator, enumerator_t*,
	   private_child_sa_t *this)
{
	policy_enumerator_t *e;

	INIT(e,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _policy_enumerate,
			.destroy = _policy_destroy,
		},
		.mine = array_create_enumerator(this->my_ts),
		.other = array_create_enumerator(this->other_ts),
		.array = this->other_ts,
		.ts = NULL,
	);

	return &e->public;
}

/**
 * update the cached usebytes
 * returns SUCCESS if the usebytes have changed, FAILED if not or no SPIs
 * are available, and NOT_SUPPORTED if the kernel interface does not support
 * querying the usebytes.
 */
static status_t update_usebytes(private_child_sa_t *this, bool inbound)
{
	status_t status = FAILED;
	uint64_t bytes, packets;
	time_t time;

	if (inbound)
	{
		if (this->my_spi)
		{
			kernel_ipsec_sa_id_t id = {
				.src = this->other_addr,
				.dst = this->my_addr,
				.spi = this->my_spi,
				.proto = proto_ike2ip(this->protocol),
				.mark = mark_in_sa(this),
			};
			kernel_ipsec_query_sa_t query = {};

			status = charon->kernel->query_sa(charon->kernel, &id, &query,
											  &bytes, &packets, &time);
			if (status == SUCCESS)
			{
				if (bytes > this->my_usebytes)
				{
					this->my_usebytes = bytes;
					this->my_usepackets = packets;
					if (time)
					{
						this->my_usetime = time;
					}
				}
				else
				{
					status = FAILED;
				}
			}
		}
	}
	else
	{
		if (this->other_spi && (this->outbound_state & CHILD_OUTBOUND_SA))
		{
			kernel_ipsec_sa_id_t id = {
				.src = this->my_addr,
				.dst = this->other_addr,
				.spi = this->other_spi,
				.proto = proto_ike2ip(this->protocol),
				.mark = this->mark_out,
			};
			kernel_ipsec_query_sa_t query = {};

			status = charon->kernel->query_sa(charon->kernel, &id, &query,
											  &bytes, &packets, &time);
			if (status == SUCCESS)
			{
				if (bytes > this->other_usebytes)
				{
					this->other_usebytes = bytes;
					this->other_usepackets = packets;
					if (time)
					{
						this->other_usetime = time;
					}
				}
				else
				{
					status = FAILED;
				}
			}
		}
	}
	return status;
}

/**
 * updates the cached usetime
 */
static bool update_usetime(private_child_sa_t *this, bool inbound)
{
	enumerator_t *enumerator;
	traffic_selector_t *my_ts, *other_ts;
	time_t last_use = 0;

	enumerator = create_policy_enumerator(this);
	while (enumerator->enumerate(enumerator, &my_ts, &other_ts))
	{
		time_t in, out, fwd;

		if (inbound)
		{
			kernel_ipsec_policy_id_t id = {
				.dir = POLICY_IN,
				.src_ts = other_ts,
				.dst_ts = my_ts,
				.mark = this->mark_in,
			};
			kernel_ipsec_query_policy_t query = {};

			if (charon->kernel->query_policy(charon->kernel, &id, &query,
											 &in) == SUCCESS)
			{
				last_use = max(last_use, in);
			}
			if (this->mode != MODE_TRANSPORT)
			{
				id.dir = POLICY_FWD;
				if (charon->kernel->query_policy(charon->kernel, &id, &query,
												 &fwd) == SUCCESS)
				{
					last_use = max(last_use, fwd);
				}
			}
		}
		else
		{
			kernel_ipsec_policy_id_t id = {
				.dir = POLICY_OUT,
				.src_ts = my_ts,
				.dst_ts = other_ts,
				.mark = this->mark_out,
				.interface = this->config->get_interface(this->config),
			};
			kernel_ipsec_query_policy_t query = {};

			if (charon->kernel->query_policy(charon->kernel, &id, &query,
											 &out) == SUCCESS)
			{
				last_use = max(last_use, out);
			}
		}
	}
	enumerator->destroy(enumerator);

	if (last_use == 0)
	{
		return FALSE;
	}
	if (inbound)
	{
		this->my_usetime = last_use;
	}
	else
	{
		this->other_usetime = last_use;
	}
	return TRUE;
}

METHOD(child_sa_t, get_usestats, void,
	private_child_sa_t *this, bool inbound,
	time_t *time, uint64_t *bytes, uint64_t *packets)
{
	if ((!bytes && !packets) || update_usebytes(this, inbound) != FAILED)
	{
		/* there was traffic since last update or the kernel interface
		 * does not support querying the number of usebytes.
		 */
		if (time)
		{
			if (!update_usetime(this, inbound) && !bytes && !packets)
			{
				/* if policy query did not yield a usetime, query SAs instead */
				update_usebytes(this, inbound);
			}
		}
	}
	if (time)
	{
		*time = inbound ? this->my_usetime : this->other_usetime;
	}
	if (bytes)
	{
		*bytes = inbound ? this->my_usebytes : this->other_usebytes;
	}
	if (packets)
	{
		*packets = inbound ? this->my_usepackets : this->other_usepackets;
	}
}

METHOD(child_sa_t, get_mark, mark_t,
	private_child_sa_t *this, bool inbound)
{
	if (inbound)
	{
		return this->mark_in;
	}
	return this->mark_out;
}

METHOD(child_sa_t, get_lifetime, time_t,
	   private_child_sa_t *this, bool hard)
{
	return hard ? this->expire_time : this->rekey_time;
}

METHOD(child_sa_t, get_installtime, time_t,
	private_child_sa_t *this)
{
	return this->install_time;
}

METHOD(child_sa_t, alloc_spi, uint32_t,
	   private_child_sa_t *this, protocol_id_t protocol)
{
	if (charon->kernel->get_spi(charon->kernel, this->other_addr, this->my_addr,
							proto_ike2ip(protocol), &this->my_spi) == SUCCESS)
	{
		/* if we allocate a SPI, but then are unable to establish the SA, we
		 * need to know the protocol family to delete the partial SA */
		this->protocol = protocol;
		return this->my_spi;
	}
	return 0;
}

METHOD(child_sa_t, alloc_cpi, uint16_t,
	   private_child_sa_t *this)
{
	if (charon->kernel->get_cpi(charon->kernel, this->other_addr, this->my_addr,
								&this->my_cpi) == SUCCESS)
	{
		return this->my_cpi;
	}
	return 0;
}

/**
 * Install the given SA in the kernel
 */
static status_t install_internal(private_child_sa_t *this, chunk_t encr,
	chunk_t integ, uint32_t spi, uint16_t cpi, bool initiator, bool inbound,
	bool tfcv3)
{
	uint16_t enc_alg = ENCR_UNDEFINED, int_alg = AUTH_UNDEFINED, size;
	uint16_t esn = NO_EXT_SEQ_NUMBERS;
	linked_list_t *my_ts, *other_ts, *src_ts, *dst_ts;
	time_t now;
	kernel_ipsec_sa_id_t id;
	kernel_ipsec_add_sa_t sa;
	lifetime_cfg_t *lifetime;
	uint32_t tfc = 0;
	host_t *src, *dst;
	status_t status;
	bool update = FALSE;

	/* BEET requires the bound address from the traffic selectors */
	my_ts = linked_list_create_from_enumerator(
									array_create_enumerator(this->my_ts));
	other_ts = linked_list_create_from_enumerator(
									array_create_enumerator(this->other_ts));

	/* now we have to decide which spi to use. Use self allocated, if "in",
	 * or the one in the proposal, if not "in" (others). Additionally,
	 * source and dest host switch depending on the role */
	if (inbound)
	{
		dst = this->my_addr;
		src = this->other_addr;
		if (this->my_spi == spi)
		{	/* alloc_spi has been called, do an SA update */
			update = TRUE;
		}
		this->my_spi = spi;
		this->my_cpi = cpi;
		dst_ts = my_ts;
		src_ts = other_ts;
	}
	else
	{
		src = this->my_addr;
		dst = this->other_addr;
		this->other_spi = spi;
		this->other_cpi = cpi;
		src_ts = my_ts;
		dst_ts = other_ts;

		if (tfcv3)
		{
			tfc = this->config->get_tfc(this->config);
		}
		this->outbound_state |= CHILD_OUTBOUND_SA;
	}

	DBG2(DBG_CHD, "adding %s %N SA", inbound ? "inbound" : "outbound",
		 protocol_id_names, this->protocol);

	/* send SA down to the kernel */
	DBG2(DBG_CHD, "  SPI 0x%.8x, src %H dst %H", ntohl(spi), src, dst);

	this->proposal->get_algorithm(this->proposal, ENCRYPTION_ALGORITHM,
								  &enc_alg, &size);
	this->proposal->get_algorithm(this->proposal, INTEGRITY_ALGORITHM,
								  &int_alg, &size);
	this->proposal->get_algorithm(this->proposal, EXTENDED_SEQUENCE_NUMBERS,
								  &esn, NULL);

	if (int_alg == AUTH_HMAC_SHA2_256_128 &&
		this->config->has_option(this->config, OPT_SHA256_96))
	{
		DBG2(DBG_CHD, "  using %N with 96-bit truncation",
			 integrity_algorithm_names, int_alg);
		int_alg = AUTH_HMAC_SHA2_256_96;
	}

	if (!this->reqid_allocated && !this->static_reqid)
	{
		status = charon->kernel->alloc_reqid(charon->kernel, my_ts, other_ts,
								this->mark_in, this->mark_out, &this->reqid);
		if (status != SUCCESS)
		{
			my_ts->destroy(my_ts);
			other_ts->destroy(other_ts);
			return status;
		}
		this->reqid_allocated = TRUE;
	}

	lifetime = this->config->get_lifetime(this->config, TRUE);

	now = time_monotonic(NULL);
	if (lifetime->time.rekey)
	{
		if (this->rekey_time)
		{
			this->rekey_time = min(this->rekey_time, now + lifetime->time.rekey);
		}
		else
		{
			this->rekey_time = now + lifetime->time.rekey;
		}
	}
	if (lifetime->time.life)
	{
		this->expire_time = now + lifetime->time.life;
	}

	if (!lifetime->time.jitter && !inbound)
	{	/* avoid triggering multiple rekey events */
		lifetime->time.rekey = 0;
	}

	id = (kernel_ipsec_sa_id_t){
		.src = src,
		.dst = dst,
		.spi = spi,
		.proto = proto_ike2ip(this->protocol),
		.mark = inbound ? mark_in_sa(this) : this->mark_out,
	};
	sa = (kernel_ipsec_add_sa_t){
		.reqid = this->reqid,
		.mode = this->mode,
		.src_ts = src_ts,
		.dst_ts = dst_ts,
		.interface = inbound ? NULL : this->config->get_interface(this->config),
		.lifetime = lifetime,
		.enc_alg = enc_alg,
		.enc_key = encr,
		.int_alg = int_alg,
		.int_key = integ,
		.replay_window = this->config->get_replay_window(this->config),
		.tfc = tfc,
		.ipcomp = this->ipcomp,
		.cpi = cpi,
		.encap = this->encap,
		.hw_offload = this->config->get_hw_offload(this->config),
		.mark = this->config->get_set_mark(this->config, inbound),
		.esn = esn,
		.copy_df = !this->config->has_option(this->config, OPT_NO_COPY_DF),
		.copy_ecn = !this->config->has_option(this->config, OPT_NO_COPY_ECN),
		.copy_dscp = this->config->get_copy_dscp(this->config),
		.initiator = initiator,
		.inbound = inbound,
		.update = update,
	};

	if (sa.mark.value == MARK_SAME)
	{
		sa.mark.value = inbound ? this->mark_in.value : this->mark_out.value;
	}

	status = charon->kernel->add_sa(charon->kernel, &id, &sa);

	my_ts->destroy(my_ts);
	other_ts->destroy(other_ts);
	free(lifetime);

	return status;
}

METHOD(child_sa_t, install, status_t,
	private_child_sa_t *this, chunk_t encr, chunk_t integ, uint32_t spi,
	uint16_t cpi, bool initiator, bool inbound, bool tfcv3)
{
	return install_internal(this, encr, integ, spi, cpi, initiator, inbound,
							tfcv3);
}

/**
 * Check kernel interface if policy updates are required
 */
static bool require_policy_update()
{
	kernel_feature_t f;

	f = charon->kernel->get_features(charon->kernel);
	return !(f & KERNEL_NO_POLICY_UPDATES);
}

/**
 * Prepare SA config to install/delete policies
 */
static void prepare_sa_cfg(private_child_sa_t *this, ipsec_sa_cfg_t *my_sa,
						   ipsec_sa_cfg_t *other_sa)
{
	enumerator_t *enumerator;

	*my_sa = (ipsec_sa_cfg_t){
		.mode = this->mode,
		.reqid = this->reqid,
		.ipcomp = {
			.transform = this->ipcomp,
		},
	};
	*other_sa = *my_sa;

	my_sa->ipcomp.cpi = this->my_cpi;
	other_sa->ipcomp.cpi = this->other_cpi;

	if (this->protocol == PROTO_ESP)
	{
		my_sa->esp.use = TRUE;
		my_sa->esp.spi = this->my_spi;
		other_sa->esp.use = TRUE;
		other_sa->esp.spi = this->other_spi;
	}
	else
	{
		my_sa->ah.use = TRUE;
		my_sa->ah.spi = this->my_spi;
		other_sa->ah.use = TRUE;
		other_sa->ah.spi = this->other_spi;
	}

	enumerator = create_policy_enumerator(this);
	while (enumerator->enumerate(enumerator, NULL, NULL))
	{
		my_sa->policy_count++;
		other_sa->policy_count++;
	}
	enumerator->destroy(enumerator);
}

/**
 * Install inbound policies: in, fwd
 */
static status_t install_policies_inbound(private_child_sa_t *this,
	host_t *my_addr, host_t *other_addr, traffic_selector_t *my_ts,
	traffic_selector_t *other_ts, ipsec_sa_cfg_t *my_sa,
	ipsec_sa_cfg_t *other_sa, policy_type_t type,
	policy_priority_t priority,	uint32_t manual_prio)
{
	kernel_ipsec_policy_id_t in_id = {
		.dir = POLICY_IN,
		.src_ts = other_ts,
		.dst_ts = my_ts,
		.mark = this->mark_in,
	};
	kernel_ipsec_manage_policy_t in_policy = {
		.type = type,
		.prio = priority,
		.manual_prio = manual_prio,
		.src = other_addr,
		.dst = my_addr,
		.sa = my_sa,
	};
	status_t status = SUCCESS;

	status |= charon->kernel->add_policy(charon->kernel, &in_id, &in_policy);
	if (this->mode != MODE_TRANSPORT)
	{
		in_id.dir = POLICY_FWD;
		status |= charon->kernel->add_policy(charon->kernel, &in_id, &in_policy);
	}
	return status;
}

/**
 * Install outbound policies: out, [fwd]
 */
static status_t install_policies_outbound(private_child_sa_t *this,
	host_t *my_addr, host_t *other_addr, traffic_selector_t *my_ts,
	traffic_selector_t *other_ts, ipsec_sa_cfg_t *my_sa,
	ipsec_sa_cfg_t *other_sa, policy_type_t type,
	policy_priority_t priority,	uint32_t manual_prio)
{
	kernel_ipsec_policy_id_t out_id = {
		.dir = POLICY_OUT,
		.src_ts = my_ts,
		.dst_ts = other_ts,
		.mark = this->mark_out,
		.interface = this->config->get_interface(this->config),
	};
	kernel_ipsec_manage_policy_t out_policy = {
		.type = type,
		.prio = priority,
		.manual_prio = manual_prio,
		.src = my_addr,
		.dst = other_addr,
		.sa = other_sa,
	};
	status_t status = SUCCESS;

	status |= charon->kernel->add_policy(charon->kernel, &out_id, &out_policy);

	if (this->mode != MODE_TRANSPORT && this->policies_fwd_out)
	{
		/* install an "outbound" FWD policy in case there is a drop policy
		 * matching outbound forwarded traffic, to allow another tunnel to use
		 * the reversed subnets and do the same we don't set a reqid (this also
		 * allows the kernel backend to distinguish between the two types of
		 * FWD policies). To avoid problems with symmetrically overlapping
		 * policies of two SAs we install them with reduced priority.  As they
		 * basically act as bypass policies for drop policies we use a higher
		 * priority than is used for them. */
		out_id.dir = POLICY_FWD;
		other_sa->reqid = 0;
		if (priority == POLICY_PRIORITY_DEFAULT)
		{
			out_policy.prio = POLICY_PRIORITY_ROUTED;
		}
		status |= charon->kernel->add_policy(charon->kernel, &out_id,
											 &out_policy);
		/* reset the reqid for any other further policies */
		other_sa->reqid = this->reqid;
	}
	return status;
}

/**
 * Install all policies
 */
static status_t install_policies_internal(private_child_sa_t *this,
	host_t *my_addr, host_t *other_addr, traffic_selector_t *my_ts,
	traffic_selector_t *other_ts, ipsec_sa_cfg_t *my_sa,
	ipsec_sa_cfg_t *other_sa, policy_type_t type,
	policy_priority_t priority,	uint32_t manual_prio, bool outbound)
{
	status_t status = SUCCESS;

	status |= install_policies_inbound(this, my_addr, other_addr, my_ts,
						other_ts, my_sa, other_sa, type, priority, manual_prio);
	if (outbound)
	{
		status |= install_policies_outbound(this, my_addr, other_addr, my_ts,
						other_ts, my_sa, other_sa, type, priority, manual_prio);
	}
	return status;
}

/**
 * Delete inbound policies: in, fwd
 */
static void del_policies_inbound(private_child_sa_t *this,
	host_t *my_addr, host_t *other_addr, traffic_selector_t *my_ts,
	traffic_selector_t *other_ts, ipsec_sa_cfg_t *my_sa,
	ipsec_sa_cfg_t *other_sa, policy_type_t type,
	policy_priority_t priority, uint32_t manual_prio)
{
	kernel_ipsec_policy_id_t in_id = {
		.dir = POLICY_IN,
		.src_ts = other_ts,
		.dst_ts = my_ts,
		.mark = this->mark_in,
	};
	kernel_ipsec_manage_policy_t in_policy = {
		.type = type,
		.prio = priority,
		.manual_prio = manual_prio,
		.src = other_addr,
		.dst = my_addr,
		.sa = my_sa,
	};

	charon->kernel->del_policy(charon->kernel, &in_id, &in_policy);

	if (this->mode != MODE_TRANSPORT)
	{
		in_id.dir = POLICY_FWD;
		charon->kernel->del_policy(charon->kernel, &in_id, &in_policy);
	}
}

/**
 * Delete outbound policies: out, [fwd]
 */
static void del_policies_outbound(private_child_sa_t *this,
	host_t *my_addr, host_t *other_addr, traffic_selector_t *my_ts,
	traffic_selector_t *other_ts, ipsec_sa_cfg_t *my_sa,
	ipsec_sa_cfg_t *other_sa, policy_type_t type,
	policy_priority_t priority, uint32_t manual_prio)
{
	kernel_ipsec_policy_id_t out_id = {
		.dir = POLICY_OUT,
		.src_ts = my_ts,
		.dst_ts = other_ts,
		.mark = this->mark_out,
		.interface = this->config->get_interface(this->config),
	};
	kernel_ipsec_manage_policy_t out_policy = {
		.type = type,
		.prio = priority,
		.manual_prio = manual_prio,
		.src = my_addr,
		.dst = other_addr,
		.sa = other_sa,
	};

	charon->kernel->del_policy(charon->kernel, &out_id, &out_policy);

	if (this->mode != MODE_TRANSPORT && this->policies_fwd_out)
	{
		out_id.dir = POLICY_FWD;
		other_sa->reqid = 0;
		if (priority == POLICY_PRIORITY_DEFAULT)
		{
			out_policy.prio = POLICY_PRIORITY_ROUTED;
		}
		charon->kernel->del_policy(charon->kernel, &out_id, &out_policy);
		other_sa->reqid = this->reqid;
	}
}

/**
 * Delete in- and outbound policies
 */
static void del_policies_internal(private_child_sa_t *this,
	host_t *my_addr, host_t *other_addr, traffic_selector_t *my_ts,
	traffic_selector_t *other_ts, ipsec_sa_cfg_t *my_sa,
	ipsec_sa_cfg_t *other_sa, policy_type_t type,
	policy_priority_t priority, uint32_t manual_prio, bool outbound)
{
	if (outbound)
	{
		del_policies_outbound(this, my_addr, other_addr, my_ts, other_ts, my_sa,
						other_sa, type, priority, manual_prio);
	}
	del_policies_inbound(this, my_addr, other_addr, my_ts, other_ts, my_sa,
						other_sa, type, priority, manual_prio);
}

METHOD(child_sa_t, set_policies, void,
	   private_child_sa_t *this, linked_list_t *my_ts_list,
	   linked_list_t *other_ts_list)
{
	enumerator_t *enumerator;
	traffic_selector_t *my_ts, *other_ts;

	if (array_count(this->my_ts))
	{
		array_destroy_offset(this->my_ts,
							 offsetof(traffic_selector_t, destroy));
		this->my_ts = array_create(0, 0);
	}
	enumerator = my_ts_list->create_enumerator(my_ts_list);
	while (enumerator->enumerate(enumerator, &my_ts))
	{
		array_insert(this->my_ts, ARRAY_TAIL, my_ts->clone(my_ts));
	}
	enumerator->destroy(enumerator);
	array_sort(this->my_ts, (void*)traffic_selector_cmp, NULL);

	if (array_count(this->other_ts))
	{
		array_destroy_offset(this->other_ts,
							 offsetof(traffic_selector_t, destroy));
		this->other_ts = array_create(0, 0);
	}
	enumerator = other_ts_list->create_enumerator(other_ts_list);
	while (enumerator->enumerate(enumerator, &other_ts))
	{
		array_insert(this->other_ts, ARRAY_TAIL, other_ts->clone(other_ts));
	}
	enumerator->destroy(enumerator);
	array_sort(this->other_ts, (void*)traffic_selector_cmp, NULL);
}

METHOD(child_sa_t, install_policies, status_t,
	   private_child_sa_t *this)
{
	enumerator_t *enumerator;
	linked_list_t *my_ts_list, *other_ts_list;
	traffic_selector_t *my_ts, *other_ts;
	status_t status = SUCCESS;
	bool install_outbound = FALSE;

	if (!this->reqid_allocated && !this->static_reqid)
	{
		my_ts_list = linked_list_create_from_enumerator(
									array_create_enumerator(this->my_ts));
		other_ts_list = linked_list_create_from_enumerator(
									array_create_enumerator(this->other_ts));
		status = charon->kernel->alloc_reqid(
							charon->kernel, my_ts_list, other_ts_list,
							this->mark_in, this->mark_out, &this->reqid);
		my_ts_list->destroy(my_ts_list);
		other_ts_list->destroy(other_ts_list);
		if (status != SUCCESS)
		{
			return status;
		}
		this->reqid_allocated = TRUE;
	}

	if (!(this->outbound_state & CHILD_OUTBOUND_REGISTERED))
	{
		install_outbound = TRUE;
		this->outbound_state |= CHILD_OUTBOUND_POLICIES;
	}

	if (!this->config->has_option(this->config, OPT_NO_POLICIES))
	{
		policy_priority_t priority;
		ipsec_sa_cfg_t my_sa, other_sa;
		uint32_t manual_prio;

		prepare_sa_cfg(this, &my_sa, &other_sa);
		manual_prio = this->config->get_manual_prio(this->config);

		/* if we're not in state CHILD_INSTALLING (i.e. if there is no SAD
		 * entry) we install a trap policy */
		this->trap = this->state == CHILD_CREATED;
		priority = this->trap ? POLICY_PRIORITY_ROUTED
							  : POLICY_PRIORITY_DEFAULT;

		/* enumerate pairs of traffic selectors */
		enumerator = create_policy_enumerator(this);
		while (enumerator->enumerate(enumerator, &my_ts, &other_ts))
		{
			status |= install_policies_internal(this, this->my_addr,
									this->other_addr, my_ts, other_ts,
									&my_sa, &other_sa, POLICY_IPSEC, priority,
									manual_prio, install_outbound);
			if (status != SUCCESS)
			{
				break;
			}
		}
		enumerator->destroy(enumerator);
	}

	if (status == SUCCESS && this->trap)
	{
		set_state(this, CHILD_ROUTED);
	}
	return status;
}

METHOD(child_sa_t, register_outbound, status_t,
	private_child_sa_t *this, chunk_t encr, chunk_t integ, uint32_t spi,
	uint16_t cpi, bool tfcv3)
{
	status_t status;

	/* if the kernel supports installing SPIs with policies we install the
	 * SA immediately as it will only be used once we update the policies */
	if (charon->kernel->get_features(charon->kernel) & KERNEL_POLICY_SPI)
	{
		status = install_internal(this, encr, integ, spi, cpi, FALSE, FALSE,
								  tfcv3);
	}
	else
	{
		DBG2(DBG_CHD, "registering outbound %N SA", protocol_id_names,
			 this->protocol);
		DBG2(DBG_CHD, "  SPI 0x%.8x, src %H dst %H", ntohl(spi), this->my_addr,
			 this->other_addr);

		this->other_spi = spi;
		this->other_cpi = cpi;
		this->encr_r = chunk_clone(encr);
		this->integ_r = chunk_clone(integ);
		this->tfcv3 = tfcv3;
		status = SUCCESS;
	}
	this->outbound_state |= CHILD_OUTBOUND_REGISTERED;
	return status;
}

METHOD(child_sa_t, install_outbound, status_t,
	private_child_sa_t *this)
{
	enumerator_t *enumerator;
	traffic_selector_t *my_ts, *other_ts;
	status_t status = SUCCESS;

	if (!(this->outbound_state & CHILD_OUTBOUND_SA))
	{
		status = install_internal(this, this->encr_r, this->integ_r,
								  this->other_spi, this->other_cpi, FALSE,
								  FALSE, this->tfcv3);
		chunk_clear(&this->encr_r);
		chunk_clear(&this->integ_r);
	}
	this->outbound_state &= ~CHILD_OUTBOUND_REGISTERED;
	if (status != SUCCESS)
	{
		return status;
	}
	if (!this->config->has_option(this->config, OPT_NO_POLICIES) &&
		!(this->outbound_state & CHILD_OUTBOUND_POLICIES))
	{
		ipsec_sa_cfg_t my_sa, other_sa;
		uint32_t manual_prio;

		prepare_sa_cfg(this, &my_sa, &other_sa);
		manual_prio = this->config->get_manual_prio(this->config);

		enumerator = create_policy_enumerator(this);
		while (enumerator->enumerate(enumerator, &my_ts, &other_ts))
		{
			status |= install_policies_outbound(this, this->my_addr,
									this->other_addr, my_ts, other_ts,
									&my_sa, &other_sa, POLICY_IPSEC,
									POLICY_PRIORITY_DEFAULT, manual_prio);
			if (status != SUCCESS)
			{
				break;
			}
		}
		enumerator->destroy(enumerator);
	}
	this->outbound_state |= CHILD_OUTBOUND_POLICIES;
	return status;
}

METHOD(child_sa_t, remove_outbound, void,
	private_child_sa_t *this)
{
	enumerator_t *enumerator;
	traffic_selector_t *my_ts, *other_ts;

	if (!(this->outbound_state & CHILD_OUTBOUND_SA))
	{
		if (this->outbound_state & CHILD_OUTBOUND_REGISTERED)
		{
			chunk_clear(&this->encr_r);
			chunk_clear(&this->integ_r);
			this->outbound_state = CHILD_OUTBOUND_NONE;
		}
		return;
	}

	if (!this->config->has_option(this->config, OPT_NO_POLICIES) &&
		(this->outbound_state & CHILD_OUTBOUND_POLICIES))
	{
		ipsec_sa_cfg_t my_sa, other_sa;
		uint32_t manual_prio;

		prepare_sa_cfg(this, &my_sa, &other_sa);
		manual_prio = this->config->get_manual_prio(this->config);

		enumerator = create_policy_enumerator(this);
		while (enumerator->enumerate(enumerator, &my_ts, &other_ts))
		{
			del_policies_outbound(this, this->my_addr, this->other_addr,
								  my_ts, other_ts, &my_sa, &other_sa,
								  POLICY_IPSEC, POLICY_PRIORITY_DEFAULT,
								  manual_prio);
		}
		enumerator->destroy(enumerator);
	}

	kernel_ipsec_sa_id_t id = {
		.src = this->my_addr,
		.dst = this->other_addr,
		.spi = this->other_spi,
		.proto = proto_ike2ip(this->protocol),
		.mark = this->mark_out,
	};
	kernel_ipsec_del_sa_t sa = {
		.cpi = this->other_cpi,
	};
	charon->kernel->del_sa(charon->kernel, &id, &sa);
	this->outbound_state = CHILD_OUTBOUND_NONE;
}

METHOD(child_sa_t, set_rekey_spi, void,
	private_child_sa_t *this, uint32_t spi)
{
	this->rekey_spi = spi;
}

METHOD(child_sa_t, get_rekey_spi, uint32_t,
	private_child_sa_t *this)
{
	return this->rekey_spi;
}

CALLBACK(reinstall_vip, void,
	host_t *vip, va_list args)
{
	host_t *me;
	char *iface;

	VA_ARGS_VGET(args, me);
	if (charon->kernel->get_interface(charon->kernel, me, &iface))
	{
		charon->kernel->del_ip(charon->kernel, vip, -1, TRUE);
		charon->kernel->add_ip(charon->kernel, vip, -1, iface);
		free(iface);
	}
}

/**
 * Update addresses and encap state of IPsec SAs in the kernel
 */
static status_t update_sas(private_child_sa_t *this, host_t *me, host_t *other,
						   bool encap)
{
	/* update our (initiator) SA */
	if (this->my_spi)
	{
		kernel_ipsec_sa_id_t id = {
			.src = this->other_addr,
			.dst = this->my_addr,
			.spi = this->my_spi,
			.proto = proto_ike2ip(this->protocol),
			.mark = mark_in_sa(this),
		};
		kernel_ipsec_update_sa_t sa = {
			.cpi = this->ipcomp != IPCOMP_NONE ? this->my_cpi : 0,
			.new_src = other,
			.new_dst = me,
			.encap = this->encap,
			.new_encap = encap,
		};
		if (charon->kernel->update_sa(charon->kernel, &id,
									  &sa) == NOT_SUPPORTED)
		{
			return NOT_SUPPORTED;
		}
	}

	/* update his (responder) SA */
	if (this->other_spi && (this->outbound_state & CHILD_OUTBOUND_SA))
	{
		kernel_ipsec_sa_id_t id = {
			.src = this->my_addr,
			.dst = this->other_addr,
			.spi = this->other_spi,
			.proto = proto_ike2ip(this->protocol),
			.mark = this->mark_out,
		};
		kernel_ipsec_update_sa_t sa = {
			.cpi = this->ipcomp != IPCOMP_NONE ? this->other_cpi : 0,
			.new_src = me,
			.new_dst = other,
			.encap = this->encap,
			.new_encap = encap,
		};
		if (charon->kernel->update_sa(charon->kernel, &id,
									  &sa) == NOT_SUPPORTED)
		{
			return NOT_SUPPORTED;
		}
	}
	/* we currently ignore the actual return values above */
	return SUCCESS;
}

METHOD(child_sa_t, update, status_t,
	private_child_sa_t *this, host_t *me, host_t *other, linked_list_t *vips,
	bool encap)
{
	child_sa_state_t old;
	bool transport_proxy_mode;

	/* anything changed at all? */
	if (me->equals(me, this->my_addr) &&
		other->equals(other, this->other_addr) && this->encap == encap)
	{
		return SUCCESS;
	}

	old = this->state;
	set_state(this, CHILD_UPDATING);
	transport_proxy_mode = this->mode == MODE_TRANSPORT &&
						   this->config->has_option(this->config,
													OPT_PROXY_MODE);

	if (!this->config->has_option(this->config, OPT_NO_POLICIES) &&
		require_policy_update())
	{
		ipsec_sa_cfg_t my_sa, other_sa;
		enumerator_t *enumerator;
		traffic_selector_t *my_ts, *other_ts;
		uint32_t manual_prio;
		status_t state;
		bool outbound;

		prepare_sa_cfg(this, &my_sa, &other_sa);
		manual_prio = this->config->get_manual_prio(this->config);
		outbound = (this->outbound_state & CHILD_OUTBOUND_POLICIES);

		enumerator = create_policy_enumerator(this);
		while (enumerator->enumerate(enumerator, &my_ts, &other_ts))
		{
			/* install drop policy to avoid traffic leaks, acquires etc. */
			if (outbound)
			{
				install_policies_outbound(this, this->my_addr, this->other_addr,
							my_ts, other_ts, &my_sa, &other_sa, POLICY_DROP,
							POLICY_PRIORITY_DEFAULT, manual_prio);
			}
			/* remove old policies */
			del_policies_internal(this, this->my_addr, this->other_addr,
						my_ts, other_ts, &my_sa, &other_sa, POLICY_IPSEC,
						POLICY_PRIORITY_DEFAULT, manual_prio, outbound);
		}
		enumerator->destroy(enumerator);

		/* update the IPsec SAs */
		state = update_sas(this, me, other, encap);

		enumerator = create_policy_enumerator(this);
		while (enumerator->enumerate(enumerator, &my_ts, &other_ts))
		{
			traffic_selector_t *old_my_ts = NULL, *old_other_ts = NULL;

			/* reinstall the previous policies if we can't update the SAs */
			if (state == NOT_SUPPORTED)
			{
				install_policies_internal(this, this->my_addr, this->other_addr,
						my_ts, other_ts, &my_sa, &other_sa, POLICY_IPSEC,
						POLICY_PRIORITY_DEFAULT, manual_prio, outbound);
			}
			else
			{
				/* check if we have to update a "dynamic" traffic selector */
				if (!me->ip_equals(me, this->my_addr) &&
					my_ts->is_host(my_ts, this->my_addr))
				{
					old_my_ts = my_ts->clone(my_ts);
					my_ts->set_address(my_ts, me);
				}
				if (!other->ip_equals(other, this->other_addr) &&
					other_ts->is_host(other_ts, this->other_addr))
				{
					old_other_ts = other_ts->clone(other_ts);
					other_ts->set_address(other_ts, other);
				}

				/* we reinstall the virtual IP to handle interface roaming
				 * correctly */
				vips->invoke_function(vips, reinstall_vip, me);

				/* reinstall updated policies */
				install_policies_internal(this, me, other, my_ts, other_ts,
						&my_sa, &other_sa, POLICY_IPSEC,
						POLICY_PRIORITY_DEFAULT, manual_prio, outbound);
			}
			/* remove the drop policy */
			if (outbound)
			{
				del_policies_outbound(this, this->my_addr, this->other_addr,
						old_my_ts ?: my_ts, old_other_ts ?: other_ts,
						&my_sa, &other_sa, POLICY_DROP,
						POLICY_PRIORITY_DEFAULT, 0);
			}

			DESTROY_IF(old_my_ts);
			DESTROY_IF(old_other_ts);
		}
		enumerator->destroy(enumerator);

		if (state == NOT_SUPPORTED)
		{
			set_state(this, old);
			return NOT_SUPPORTED;
		}

	}
	else if (!transport_proxy_mode)
	{
		if (update_sas(this, me, other, encap) == NOT_SUPPORTED)
		{
			set_state(this, old);
			return NOT_SUPPORTED;
		}
	}

	if (!transport_proxy_mode)
	{
		/* apply hosts */
		if (!me->equals(me, this->my_addr))
		{
			this->my_addr->destroy(this->my_addr);
			this->my_addr = me->clone(me);
		}
		if (!other->equals(other, this->other_addr))
		{
			this->other_addr->destroy(this->other_addr);
			this->other_addr = other->clone(other);
		}
	}

	this->encap = encap;
	set_state(this, old);

	return SUCCESS;
}

METHOD(child_sa_t, destroy, void,
	   private_child_sa_t *this)
{
	enumerator_t *enumerator;
	traffic_selector_t *my_ts, *other_ts;
	policy_priority_t priority;

	priority = this->trap ? POLICY_PRIORITY_ROUTED : POLICY_PRIORITY_DEFAULT;

	set_state(this, CHILD_DESTROYING);

	if (!this->config->has_option(this->config, OPT_NO_POLICIES))
	{
		ipsec_sa_cfg_t my_sa, other_sa;
		uint32_t manual_prio;
		bool del_outbound;

		prepare_sa_cfg(this, &my_sa, &other_sa);
		manual_prio = this->config->get_manual_prio(this->config);
		del_outbound = (this->outbound_state & CHILD_OUTBOUND_POLICIES) ||
						this->trap;

		/* delete all policies in the kernel */
		enumerator = create_policy_enumerator(this);
		while (enumerator->enumerate(enumerator, &my_ts, &other_ts))
		{
			del_policies_internal(this, this->my_addr,
						this->other_addr, my_ts, other_ts, &my_sa, &other_sa,
						POLICY_IPSEC, priority, manual_prio, del_outbound);
		}
		enumerator->destroy(enumerator);
	}

	/* delete SAs in the kernel, if they are set up */
	if (this->my_spi)
	{
		kernel_ipsec_sa_id_t id = {
			.src = this->other_addr,
			.dst = this->my_addr,
			.spi = this->my_spi,
			.proto = proto_ike2ip(this->protocol),
			.mark = mark_in_sa(this),
		};
		kernel_ipsec_del_sa_t sa = {
			.cpi = this->my_cpi,
		};
		charon->kernel->del_sa(charon->kernel, &id, &sa);
	}
	if (this->other_spi && (this->outbound_state & CHILD_OUTBOUND_SA))
	{
		kernel_ipsec_sa_id_t id = {
			.src = this->my_addr,
			.dst = this->other_addr,
			.spi = this->other_spi,
			.proto = proto_ike2ip(this->protocol),
			.mark = this->mark_out,
		};
		kernel_ipsec_del_sa_t sa = {
			.cpi = this->other_cpi,
		};
		charon->kernel->del_sa(charon->kernel, &id, &sa);
	}

	if (this->reqid_allocated)
	{
		if (charon->kernel->release_reqid(charon->kernel,
						this->reqid, this->mark_in, this->mark_out) != SUCCESS)
		{
			DBG1(DBG_CHD, "releasing reqid %u failed", this->reqid);
		}
	}

	array_destroy_offset(this->my_ts, offsetof(traffic_selector_t, destroy));
	array_destroy_offset(this->other_ts, offsetof(traffic_selector_t, destroy));
	this->my_addr->destroy(this->my_addr);
	this->other_addr->destroy(this->other_addr);
	DESTROY_IF(this->proposal);
	this->config->destroy(this->config);
	chunk_clear(&this->encr_r);
	chunk_clear(&this->integ_r);
	free(this);
}

/**
 * Get proxy address for one side, if any
 */
static host_t* get_proxy_addr(child_cfg_t *config, host_t *ike, bool local)
{
	host_t *host = NULL;
	uint8_t mask;
	enumerator_t *enumerator;
	linked_list_t *ts_list, *list;
	traffic_selector_t *ts;

	list = linked_list_create_with_items(ike, NULL);
	ts_list = config->get_traffic_selectors(config, local, NULL, list, FALSE);
	list->destroy(list);

	enumerator = ts_list->create_enumerator(ts_list);
	while (enumerator->enumerate(enumerator, &ts))
	{
		if (ts->is_host(ts, NULL) && ts->to_subnet(ts, &host, &mask))
		{
			DBG1(DBG_CHD, "%s address: %H is a transport mode proxy for %H",
				 local ? "my" : "other", ike, host);
			break;
		}
	}
	enumerator->destroy(enumerator);
	ts_list->destroy_offset(ts_list, offsetof(traffic_selector_t, destroy));

	if (!host)
	{
		host = ike->clone(ike);
	}
	return host;
}

/**
 * Described in header.
 */
child_sa_t * child_sa_create(host_t *me, host_t* other,
							 child_cfg_t *config, uint32_t reqid, bool encap,
							 u_int mark_in, u_int mark_out)
{
	private_child_sa_t *this;
	static refcount_t unique_id = 0, unique_mark = 0;
	refcount_t mark = 0;

	INIT(this,
		.public = {
			.get_name = _get_name,
			.get_reqid = _get_reqid,
			.get_unique_id = _get_unique_id,
			.get_config = _get_config,
			.get_state = _get_state,
			.set_state = _set_state,
			.get_outbound_state = _get_outbound_state,
			.get_spi = _get_spi,
			.get_cpi = _get_cpi,
			.get_protocol = _get_protocol,
			.set_protocol = _set_protocol,
			.get_mode = _get_mode,
			.set_mode = _set_mode,
			.get_proposal = _get_proposal,
			.set_proposal = _set_proposal,
			.get_lifetime = _get_lifetime,
			.get_installtime = _get_installtime,
			.get_usestats = _get_usestats,
			.get_mark = _get_mark,
			.has_encap = _has_encap,
			.get_ipcomp = _get_ipcomp,
			.set_ipcomp = _set_ipcomp,
			.get_close_action = _get_close_action,
			.set_close_action = _set_close_action,
			.get_dpd_action = _get_dpd_action,
			.set_dpd_action = _set_dpd_action,
			.alloc_spi = _alloc_spi,
			.alloc_cpi = _alloc_cpi,
			.install = _install,
			.register_outbound = _register_outbound,
			.install_outbound = _install_outbound,
			.remove_outbound = _remove_outbound,
			.set_rekey_spi = _set_rekey_spi,
			.get_rekey_spi = _get_rekey_spi,
			.update = _update,
			.set_policies = _set_policies,
			.install_policies = _install_policies,
			.create_ts_enumerator = _create_ts_enumerator,
			.create_policy_enumerator = _create_policy_enumerator,
			.destroy = _destroy,
		},
		.encap = encap,
		.ipcomp = IPCOMP_NONE,
		.state = CHILD_CREATED,
		.my_ts = array_create(0, 0),
		.other_ts = array_create(0, 0),
		.protocol = PROTO_NONE,
		.mode = MODE_TUNNEL,
		.close_action = config->get_close_action(config),
		.dpd_action = config->get_dpd_action(config),
		.reqid = config->get_reqid(config),
		.unique_id = ref_get(&unique_id),
		.mark_in = config->get_mark(config, TRUE),
		.mark_out = config->get_mark(config, FALSE),
		.install_time = time_monotonic(NULL),
		.policies_fwd_out = config->has_option(config, OPT_FWD_OUT_POLICIES),
	);

	this->config = config;
	config->get_ref(config);

	if (mark_in)
	{
		this->mark_in.value = mark_in;
	}
	if (mark_out)
	{
		this->mark_out.value = mark_out;
	}

	if (MARK_IS_UNIQUE(this->mark_in.value) ||
		MARK_IS_UNIQUE(this->mark_out.value))
	{
		bool unique_dir;

		unique_dir = this->mark_in.value == MARK_UNIQUE_DIR ||
					 this->mark_out.value == MARK_UNIQUE_DIR;

		if (!unique_dir)
		{
			mark = ref_get(&unique_mark);
		}
		if (MARK_IS_UNIQUE(this->mark_in.value))
		{
			if (unique_dir)
			{
				mark = ref_get(&unique_mark);
			}
			this->mark_in.value = mark;
		}
		if (MARK_IS_UNIQUE(this->mark_out.value))
		{
			if (unique_dir)
			{
				mark = ref_get(&unique_mark);
			}
			this->mark_out.value = mark;
		}
	}

	if (!this->reqid)
	{
		/* reuse old reqid if we are rekeying an existing CHILD_SA and when
		 * initiating a trap policy. While the reqid cache would find the same
		 * reqid for our selectors, this does not work in a special case: If an
		 * SA is triggered by a trap policy, but the negotiated TS get
		 * narrowed, we still must reuse the same reqid to successfully
		 * replace the temporary SA on the kernel level. Rekeying such an SA
		 * requires an explicit reqid, as the cache currently knows the original
		 * selectors only for that reqid. */
		this->reqid = reqid;
	}
	else
	{
		this->static_reqid = TRUE;
	}

	/* MIPv6 proxy transport mode sets SA endpoints to TS hosts */
	if (config->get_mode(config) == MODE_TRANSPORT &&
		config->has_option(config, OPT_PROXY_MODE))
	{
		this->mode = MODE_TRANSPORT;

		this->my_addr = get_proxy_addr(config, me, TRUE);
		this->other_addr = get_proxy_addr(config, other, FALSE);
	}
	else
	{
		this->my_addr = me->clone(me);
		this->other_addr = other->clone(other);
	}
	return &this->public;
}
