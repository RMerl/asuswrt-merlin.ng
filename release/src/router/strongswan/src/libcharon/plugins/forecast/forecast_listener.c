/*
 * Copyright (C) 2015 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2010-2014 Martin Willi
 * Copyright (C) 2010-2014 revosec AG
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

#include "forecast_listener.h"

#include <errno.h>
#include <libiptc/libiptc.h>
#include <linux/netfilter/xt_MARK.h>
#include <linux/netfilter/xt_esp.h>

#include <daemon.h>
#include <collections/array.h>
#include <collections/hashtable.h>
#include <threading/rwlock.h>

/**
 * Add a struct at the current position in the buffer
 */
#define ADD_STRUCT(pos, st, ...) ({\
	typeof(pos) _cur = pos; pos += XT_ALIGN(sizeof(st));\
	*(st*)_cur = (st){ __VA_ARGS__ };\
	(st*)_cur;\
})

typedef struct private_forecast_listener_t private_forecast_listener_t;

/**
 * Private data of an forecast_listener_t object.
 */
struct private_forecast_listener_t {

	/**
	 * Public forecast_listener_t interface.
	 */
	forecast_listener_t public;

	/**
	 * List of entries
	 */
	linked_list_t *entries;

	/**
	 * RWlock for IP list
	 */
	rwlock_t *lock;

	/**
	 * Configs we do reinjection
	 */
	char *reinject_configs;

	/**
	 * Broadcast address on LAN interface, network order
	 */
	uint32_t broadcast;
};

/**
 * Hashtable entry
 */
typedef struct {
	/** local traffic selectors */
	array_t *lts;
	/** remote traffic selectors */
	array_t *rts;
	/** firewall mark used by CHILD_SA */
	u_int mark;
	/** local IKE_SA endpoint */
	host_t *lhost;
	/** remote IKE_SA endpoint */
	host_t *rhost;
	/** inbound SPI */
	uint32_t spi;
	/** use UDP encapsulation */
	bool encap;
	/** whether we should allow reencapsulation of IPsec received forecasts */
	bool reinject;
	/** broadcast address used for that entry */
	uint32_t broadcast;
} entry_t;

/**
 * Destroy an entry
 */
static void entry_destroy(entry_t *entry)
{
	if (entry)
	{
		entry->lhost->destroy(entry->lhost);
		entry->rhost->destroy(entry->rhost);
		array_destroy_offset(entry->lts, offsetof(traffic_selector_t, destroy));
		array_destroy_offset(entry->rts, offsetof(traffic_selector_t, destroy));
		free(entry);
	}
}

/**
 * Convert an (IPv4) traffic selector to an address and mask
 */
static bool ts2in(traffic_selector_t *ts,
				  struct in_addr *addr, struct in_addr *mask)
{
	uint8_t bits;
	host_t *net;

	if (ts->get_type(ts) == TS_IPV4_ADDR_RANGE &&
		ts->to_subnet(ts, &net, &bits))
	{
		memcpy(&addr->s_addr, net->get_address(net).ptr, 4);
		net->destroy(net);
		mask->s_addr = htonl(0xffffffffU << (32 - bits));
		return TRUE;
	}
	return FALSE;
}

/**
 * Convert an (IPv4) host to an address with mask
 */
static bool host2in(host_t *host, struct in_addr *addr, struct in_addr *mask)
{
	if (host->get_family(host) == AF_INET)
	{
		memcpy(&addr->s_addr, host->get_address(host).ptr, 4);
		mask->s_addr = ~0;
		return TRUE;
	}
	return FALSE;
}

/**
 * Add or remove a rule to/from the specified chain
 */
static bool manage_rule(struct iptc_handle *ipth, const char *chain,
						bool add, struct ipt_entry *e)
{
	if (add)
	{
		if (!iptc_insert_entry(chain, e, 0, ipth))
		{
			DBG1(DBG_CFG, "appending %s rule failed: %s",
				 chain, iptc_strerror(errno));
			return FALSE;
		}
	}
	else
	{
		u_char matchmask[e->next_offset];

		memset(matchmask, 255, sizeof(matchmask));
		if (!iptc_delete_entry(chain, e, matchmask, ipth))
		{
			DBG1(DBG_CFG, "deleting %s rule failed: %s",
				 chain, iptc_strerror(errno));
			return FALSE;
		}
	}
	return TRUE;
}

/**
 * Add rule marking UDP-encapsulated ESP packets to match the correct policy
 */
static bool manage_pre_esp_in_udp(struct iptc_handle *ipth,
								  entry_t *entry, bool add)
{
	uint16_t match_size	= XT_ALIGN(sizeof(struct ipt_entry_match)) +
							  XT_ALIGN(sizeof(struct xt_udp));
	uint16_t target_offset = XT_ALIGN(sizeof(struct ipt_entry)) + match_size;
	uint16_t target_size	= XT_ALIGN(sizeof(struct ipt_entry_target)) +
							  XT_ALIGN(sizeof(struct xt_mark_tginfo2));
	uint16_t entry_size	= target_offset + target_size;
	u_char ipt[entry_size], *pos = ipt;
	struct ipt_entry *e;

	memset(ipt, 0, sizeof(ipt));
	e = ADD_STRUCT(pos, struct ipt_entry,
		.target_offset = target_offset,
		.next_offset = entry_size,
		.ip = {
			.proto = IPPROTO_UDP,
		},
	);
	if (!host2in(entry->lhost, &e->ip.dst, &e->ip.dmsk) ||
		!host2in(entry->rhost, &e->ip.src, &e->ip.smsk))
	{
		return FALSE;
	}
	ADD_STRUCT(pos, struct ipt_entry_match,
		.u = {
			.user = {
				.match_size = match_size,
				.name = "udp",
			},
		},
	);
	ADD_STRUCT(pos, struct xt_udp,
		.spts = {
			entry->rhost->get_port(entry->rhost),
			entry->rhost->get_port(entry->rhost)
		},
		.dpts = {
			entry->lhost->get_port(entry->lhost),
			entry->lhost->get_port(entry->lhost)
		},
	);
	ADD_STRUCT(pos, struct ipt_entry_target,
		.u = {
			.user = {
				.target_size = target_size,
				.name = "MARK",
				.revision = 2,
			},
		},
	);
	ADD_STRUCT(pos, struct xt_mark_tginfo2,
		.mark = entry->mark,
		.mask = ~0,
	);
	return manage_rule(ipth, "PREROUTING", add, e);
}

/**
 * Add rule marking non-encapsulated ESP packets to match the correct policy
 */
static bool manage_pre_esp(struct iptc_handle *ipth, entry_t *entry, bool add)
{
	uint16_t match_size	= XT_ALIGN(sizeof(struct ipt_entry_match)) +
							  XT_ALIGN(sizeof(struct xt_esp));
	uint16_t target_offset = XT_ALIGN(sizeof(struct ipt_entry)) + match_size;
	uint16_t target_size	= XT_ALIGN(sizeof(struct ipt_entry_target)) +
							  XT_ALIGN(sizeof(struct xt_mark_tginfo2));
	uint16_t entry_size	= target_offset + target_size;
	u_char ipt[entry_size], *pos = ipt;
	struct ipt_entry *e;

	memset(ipt, 0, sizeof(ipt));
	e = ADD_STRUCT(pos, struct ipt_entry,
		.target_offset = target_offset,
		.next_offset = entry_size,
		.ip = {
			.proto = IPPROTO_ESP,
		},
	);
	if (!host2in(entry->lhost, &e->ip.dst, &e->ip.dmsk) ||
		!host2in(entry->rhost, &e->ip.src, &e->ip.smsk))
	{
		return FALSE;
	}
	ADD_STRUCT(pos, struct ipt_entry_match,
		.u = {
			.user = {
				.match_size = match_size,
				.name = "esp",
			},
		},
	);
	ADD_STRUCT(pos, struct xt_esp,
		.spis = { htonl(entry->spi), htonl(entry->spi) },
	);
	ADD_STRUCT(pos, struct ipt_entry_target,
		.u = {
			.user = {
				.target_size = target_size,
				.name = "MARK",
				.revision = 2,
			},
		},
	);
	ADD_STRUCT(pos, struct xt_mark_tginfo2,
		.mark = entry->mark,
		.mask = ~0,
	);
	return manage_rule(ipth, "PREROUTING", add, e);
}

/**
 * Add rule marking ESP packets to match the correct policy
 */
static bool manage_pre(struct iptc_handle *ipth, entry_t *entry, bool add)
{
	if (entry->encap)
	{
		return manage_pre_esp_in_udp(ipth, entry, add);
	}
	return manage_pre_esp(ipth, entry, add);
}

/**
 * Add rule handling outbound traffic to use correct mark
 */
static bool manage_out(struct iptc_handle *ipth, entry_t *entry, bool add)
{
	uint16_t target_offset = XT_ALIGN(sizeof(struct ipt_entry));
	uint16_t target_size	= XT_ALIGN(sizeof(struct ipt_entry_target)) +
							  XT_ALIGN(sizeof(struct xt_mark_tginfo2));
	uint16_t entry_size	= target_offset + target_size;
	u_char ipt[entry_size], *pos = ipt;
	struct ipt_entry *e;

	memset(ipt, 0, sizeof(ipt));
	e = ADD_STRUCT(pos, struct ipt_entry,
		.target_offset = target_offset,
		.next_offset = entry_size,
	);
	ADD_STRUCT(pos, struct ipt_entry_target,
		.u = {
			.user = {
				.target_size = target_size,
				.name = "MARK",
				.revision = 2,
			},
		},
	);
	ADD_STRUCT(pos, struct xt_mark_tginfo2,
		.mark = entry->mark,
		.mask = ~0,
	);

	enumerator_t *enumerator;
	traffic_selector_t *ts;

	enumerator = array_create_enumerator(entry->rts);
	while (enumerator->enumerate(enumerator, &ts))
	{
		if (!ts2in(ts, &e->ip.dst, &e->ip.dmsk))
		{
			continue;
		}
		if (e->ip.dst.s_addr == 0xffffffff ||
			e->ip.dst.s_addr == entry->broadcast ||
			memeq(&e->ip.dst.s_addr, "\xe0", 1))
		{
			/* skip broadcast/multicast selectors, they are shared and the mark
			 * is set by the socket we use for reinjection */
			continue;
		}
		if (!manage_rule(ipth, "PREROUTING", add, e) ||
			!manage_rule(ipth, "OUTPUT", add, e))
		{
			enumerator->destroy(enumerator);
			return FALSE;
		}
	}
	enumerator->destroy(enumerator);

	return TRUE;
}

/**
 * Check if config is whitelisted to reinject traffic
 */
static bool is_reinject_config(private_forecast_listener_t *this, char *name)
{
	enumerator_t *enumerator;
	bool reinject = FALSE;
	char *token;

	enumerator = enumerator_create_token(this->reinject_configs, ",", " ");
	while (enumerator->enumerate(enumerator, &token))
	{
		if (streq(token, name))
		{
			reinject = TRUE;
			break;
		}
	}
	enumerator->destroy(enumerator);

	return reinject;
}

/**
 * Add rules and entry for given CHILD_SA
 */
static bool add_entry(private_forecast_listener_t *this,
					  struct iptc_handle *ipth, host_t *lhost, host_t *rhost,
					  child_sa_t *child_sa, bool encap)
{
	enumerator_t *enumerator;
	traffic_selector_t *ts;
	entry_t *entry;

	INIT(entry,
		.lts = array_create(0, 0),
		.rts = array_create(0, 0),
		.lhost = lhost->clone(lhost),
		.rhost = rhost->clone(rhost),
		.spi = child_sa->get_spi(child_sa, TRUE),
		.encap = encap,
		.mark = child_sa->get_mark(child_sa, TRUE).value,
		.reinject = is_reinject_config(this, child_sa->get_name(child_sa)),
		.broadcast = this->broadcast,
	);

	enumerator = child_sa->create_ts_enumerator(child_sa, TRUE);
	while (enumerator->enumerate(enumerator, &ts))
	{
		array_insert(entry->lts, ARRAY_TAIL, ts->clone(ts));
	}
	enumerator->destroy(enumerator);

	enumerator = child_sa->create_ts_enumerator(child_sa, FALSE);
	while (enumerator->enumerate(enumerator, &ts))
	{
		array_insert(entry->rts, ARRAY_TAIL, ts->clone(ts));
	}
	enumerator->destroy(enumerator);

	if (manage_pre(ipth, entry, TRUE) &&
		manage_out(ipth, entry, TRUE))
	{
		this->lock->write_lock(this->lock);
		this->entries->insert_last(this->entries, entry);
		this->lock->unlock(this->lock);
		return TRUE;
	}
	entry_destroy(entry);
	return FALSE;
}

/**
 * Remove an entry and rules for a given mark
 */
static bool remove_entry(private_forecast_listener_t *this,
						 struct iptc_handle *ipth, child_sa_t *child_sa)
{
	enumerator_t *enumerator;
	entry_t *entry;
	bool done = FALSE;

	this->lock->write_lock(this->lock);
	enumerator = this->entries->create_enumerator(this->entries);
	while (enumerator->enumerate(enumerator, &entry))
	{
		if (entry->mark == child_sa->get_mark(child_sa, TRUE).value)
		{
			this->entries->remove_at(this->entries, enumerator);
			if (manage_pre(ipth, entry, FALSE) &&
				manage_out(ipth, entry, FALSE))
			{
				done = TRUE;
			}
			entry_destroy(entry);
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);

	return done;
}

/**
 * Initialize iptables handle, log error
 */
static struct iptc_handle* init_handle()
{
	struct iptc_handle *ipth;

	ipth = iptc_init("mangle");
	if (ipth)
	{
		return ipth;
	}
	DBG1(DBG_CFG, "initializing iptables failed: %s", iptc_strerror(errno));
	return NULL;
}

/**
 * Commit iptables rules, log error
 */
static bool commit_handle(struct iptc_handle *ipth)
{
	if (iptc_commit(ipth))
	{
		return TRUE;
	}
	DBG1(DBG_CFG, "forecast iptables commit failed: %s", iptc_strerror(errno));
	return FALSE;
}

/**
 * Check if we should handle the given CHILD_SA
 */
static bool handle_sa(child_sa_t *child_sa)
{
	return child_sa->get_mark(child_sa, TRUE).value &&
		   child_sa->get_mark(child_sa, FALSE).value;
}

METHOD(listener_t, child_updown, bool,
	private_forecast_listener_t *this, ike_sa_t *ike_sa, child_sa_t *child_sa,
	bool up)
{
	struct iptc_handle *ipth;
	host_t *lhost, *rhost;
	bool encap;

	lhost = ike_sa->get_my_host(ike_sa);
	rhost = ike_sa->get_other_host(ike_sa);
	encap = child_sa->has_encap(child_sa);

	if (handle_sa(child_sa))
	{
		ipth = init_handle();
		if (ipth)
		{
			if (up)
			{
				if (add_entry(this, ipth, lhost, rhost, child_sa, encap))
				{
					commit_handle(ipth);
				}
			}
			else
			{
				if (remove_entry(this, ipth, child_sa))
				{
					commit_handle(ipth);
				}
			}
			iptc_free(ipth);
		}
	}
	return TRUE;
}

METHOD(listener_t, child_rekey, bool,
	private_forecast_listener_t *this, ike_sa_t *ike_sa,
	child_sa_t *old, child_sa_t *new)
{
	struct iptc_handle *ipth;;
	host_t *lhost, *rhost;

	lhost = ike_sa->get_my_host(ike_sa);
	rhost = ike_sa->get_other_host(ike_sa);

	if (handle_sa(old))
	{
		ipth = init_handle();
		if (ipth)
		{
			if (remove_entry(this, ipth, old) &&
				add_entry(this, ipth, lhost, rhost, new, new->has_encap(new)))
			{
				commit_handle(ipth);
			}
			iptc_free(ipth);
		}
	}
	return TRUE;
}

METHOD(listener_t, ike_update, bool,
	private_forecast_listener_t *this, ike_sa_t *ike_sa,
	bool local, host_t *new)
{
	struct iptc_handle *ipth;
	enumerator_t *enumerator;
	child_sa_t *child_sa;
	host_t *lhost, *rhost;
	bool encap;

	if (local)
	{
		lhost = new;
		rhost = ike_sa->get_other_host(ike_sa);
	}
	else
	{
		lhost = ike_sa->get_my_host(ike_sa);
		rhost = new;
	}
	/* during ike_update(), has_encap() on the CHILD_SA has not yet been
	 * updated, but shows the old state. */
	encap = ike_sa->has_condition(ike_sa, COND_NAT_ANY);

	enumerator = ike_sa->create_child_sa_enumerator(ike_sa);
	while (enumerator->enumerate(enumerator, &child_sa))
	{
		if (handle_sa(child_sa))
		{
			ipth = init_handle();
			if (ipth)
			{
				if (remove_entry(this, ipth, child_sa) &&
					add_entry(this, ipth, lhost, rhost, child_sa, encap))
				{
					commit_handle(ipth);
				}
				iptc_free(ipth);
			}
		}
	}
	enumerator->destroy(enumerator);

	return TRUE;
}

CALLBACK(ts_filter, bool,
	entry_t *entry, enumerator_t *orig, va_list args)
{
	traffic_selector_t *ts, **out;
	uint32_t *mark;
	bool *reinject;

	VA_ARGS_VGET(args, out, mark, reinject);

	if (orig->enumerate(orig, &ts))
	{
		*out = ts;
		*mark = entry->mark;
		*reinject = entry->reinject;
		return TRUE;
	}
	return FALSE;
}

/**
 * Create inner enumerator over local traffic selectors
 */
static enumerator_t* create_inner_local(entry_t *entry, rwlock_t *lock)
{
	return enumerator_create_filter(array_create_enumerator(entry->lts),
									ts_filter, entry, NULL);
}

/**
 * Create inner enumerator over remote traffic selectors
 */
static enumerator_t* create_inner_remote(entry_t *entry, rwlock_t *lock)
{
	return enumerator_create_filter(array_create_enumerator(entry->rts),
									ts_filter, entry, NULL);
}

METHOD(forecast_listener_t, create_enumerator, enumerator_t*,
	private_forecast_listener_t *this, bool local)
{
	this->lock->read_lock(this->lock);
	return enumerator_create_nested(
					this->entries->create_enumerator(this->entries),
					(void*)(local ? create_inner_local : create_inner_remote),
					this->lock, (void*)this->lock->unlock);
}

METHOD(forecast_listener_t, set_broadcast, void,
	private_forecast_listener_t *this, host_t *bcast)
{
	if (bcast->get_family(bcast) == AF_INET)
	{
		struct sockaddr_in *in;

		in = (struct sockaddr_in*)bcast->get_sockaddr(bcast);
		this->broadcast = in->sin_addr.s_addr;
	}
}

METHOD(forecast_listener_t, destroy, void,
	private_forecast_listener_t *this)
{
	this->entries->destroy(this->entries);
	this->lock->destroy(this->lock);
	free(this);
}

/**
 * See header
 */
forecast_listener_t *forecast_listener_create()
{
	private_forecast_listener_t *this;

	INIT(this,
		.public = {
			.listener = {
				.ike_update = _ike_update,
				.child_updown = _child_updown,
				.child_rekey = _child_rekey,
			},
			.create_enumerator = _create_enumerator,
			.set_broadcast = _set_broadcast,
			.destroy = _destroy,
		},
		.entries = linked_list_create(),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
		.reinject_configs = lib->settings->get_str(lib->settings,
								"%s.plugins.forecast.reinject", "", lib->ns),
	);

	return &this->public;
}
