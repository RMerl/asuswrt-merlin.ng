/*
 * Copyright (C) 2008-2016 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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
 * Copyright (c) 2012 Nanoteq Pty Ltd
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

#include "kernel_interface.h"

#include <utils/debug.h>
#include <threading/mutex.h>
#include <collections/linked_list.h>
#include <collections/hashtable.h>
#include <collections/array.h>

typedef struct private_kernel_interface_t private_kernel_interface_t;

typedef struct kernel_algorithm_t kernel_algorithm_t;

/**
 * Mapping of IKE algorithms to kernel-specific algorithm identifiers
 */
struct kernel_algorithm_t {

	/**
	 * Transform type of the algorithm
	 */
	transform_type_t type;

	/**
	 * Identifier specified in IKE
	 */
	uint16_t ike;

	/**
	 * Identifier as defined in pfkeyv2.h
	 */
	uint16_t kernel;

	/**
	 * Name of the algorithm in linux crypto API
	 */
	char *name;
};

/**
 * Private data of a kernel_interface_t object.
 */
struct private_kernel_interface_t {

	/**
	 * Public part of kernel_interface_t object.
	 */
	kernel_interface_t public;

	/**
	 * Registered IPsec constructor
	 */
	kernel_ipsec_constructor_t ipsec_constructor;

	/**
	 * Registered net constructor
	 */
	kernel_net_constructor_t net_constructor;

	/**
	 * ipsec interface
	 */
	kernel_ipsec_t *ipsec;

	/**
	 * network interface
	 */
	kernel_net_t *net;

	/**
	 * mutex for listeners
	 */
	mutex_t *mutex;

	/**
	 * list of registered listeners
	 */
	linked_list_t *listeners;

	/**
	 * Reqid entries indexed by reqids
	 */
	hashtable_t *reqids;

	/**
	 * Reqid entries indexed by traffic selectors
	 */
	hashtable_t *reqids_by_ts;

	/**
	 * mutex for algorithm mappings
	 */
	mutex_t *mutex_algs;

	/**
	 * List of algorithm mappings (kernel_algorithm_t*)
	 */
	linked_list_t *algorithms;

	/**
	 * List of interface names to include or exclude (char*), NULL if interfaces
	 * are not filtered
	 */
	linked_list_t *ifaces_filter;

	/**
	 * TRUE to exclude interfaces listed in ifaces_filter, FALSE to consider
	 * only those listed there
	 */
	bool ifaces_exclude;
};

METHOD(kernel_interface_t, get_features, kernel_feature_t,
	private_kernel_interface_t *this)
{
	kernel_feature_t features = 0;

	if (this->ipsec && this->ipsec->get_features)
	{
		features |= this->ipsec->get_features(this->ipsec);
	}
	if (this->net && this->net->get_features)
	{
		features |= this->net->get_features(this->net);
	}
	return features;
}

METHOD(kernel_interface_t, get_spi, status_t,
	private_kernel_interface_t *this, host_t *src, host_t *dst,
	uint8_t protocol, uint32_t *spi)
{
	if (!this->ipsec)
	{
		return NOT_SUPPORTED;
	}
	return this->ipsec->get_spi(this->ipsec, src, dst, protocol, spi);
}

METHOD(kernel_interface_t, get_cpi, status_t,
	private_kernel_interface_t *this, host_t *src, host_t *dst,
	uint16_t *cpi)
{
	if (!this->ipsec)
	{
		return NOT_SUPPORTED;
	}
	return this->ipsec->get_cpi(this->ipsec, src, dst, cpi);
}

/**
 * Reqid mapping entry
 */
typedef struct {
	/** allocated reqid */
	uint32_t reqid;
	/** references to this entry */
	u_int refs;
	/** inbound mark used for SA */
	mark_t mark_in;
	/** outbound mark used for SA */
	mark_t mark_out;
	/** local traffic selectors */
	array_t *local;
	/** remote traffic selectors */
	array_t *remote;
} reqid_entry_t;

/**
 * Destroy a reqid mapping entry
 */
static void reqid_entry_destroy(reqid_entry_t *entry)
{
	array_destroy_offset(entry->local, offsetof(traffic_selector_t, destroy));
	array_destroy_offset(entry->remote, offsetof(traffic_selector_t, destroy));
	free(entry);
}

/**
 * Hashtable hash function for reqid entries using reqid as key
 */
static u_int hash_reqid(reqid_entry_t *entry)
{
	return chunk_hash_inc(chunk_from_thing(entry->reqid),
				chunk_hash_inc(chunk_from_thing(entry->mark_in),
					chunk_hash(chunk_from_thing(entry->mark_out))));
}

/**
 * Hashtable equals function for reqid entries using reqid as key
 */
static bool equals_reqid(reqid_entry_t *a, reqid_entry_t *b)
{
	return a->reqid == b->reqid &&
		   a->mark_in.value == b->mark_in.value &&
		   a->mark_in.mask == b->mark_in.mask &&
		   a->mark_out.value == b->mark_out.value &&
		   a->mark_out.mask == b->mark_out.mask;
}

/**
 * Hash an array of traffic selectors
 */
static u_int hash_ts_array(array_t *array, u_int hash)
{
	enumerator_t *enumerator;
	traffic_selector_t *ts;

	enumerator = array_create_enumerator(array);
	while (enumerator->enumerate(enumerator, &ts))
	{
		hash = ts->hash(ts, hash);
	}
	enumerator->destroy(enumerator);

	return hash;
}

/**
 * Hashtable hash function for reqid entries using traffic selectors as key
 */
static u_int hash_reqid_by_ts(reqid_entry_t *entry)
{
	return hash_ts_array(entry->local, hash_ts_array(entry->remote,
			chunk_hash_inc(chunk_from_thing(entry->mark_in),
				chunk_hash(chunk_from_thing(entry->mark_out)))));
}

/**
 * Compare two array with traffic selectors for equality
 */
static bool ts_array_equals(array_t *a, array_t *b)
{
	traffic_selector_t *tsa, *tsb;
	enumerator_t *ae, *be;
	bool equal = TRUE;

	if (array_count(a) != array_count(b))
	{
		return FALSE;
	}

	ae = array_create_enumerator(a);
	be = array_create_enumerator(b);
	while (equal && ae->enumerate(ae, &tsa) && be->enumerate(be, &tsb))
	{
		equal = tsa->equals(tsa, tsb);
	}
	ae->destroy(ae);
	be->destroy(be);

	return equal;
}

/**
 * Hashtable equals function for reqid entries using traffic selectors as key
 */
static bool equals_reqid_by_ts(reqid_entry_t *a, reqid_entry_t *b)
{
	return ts_array_equals(a->local, b->local) &&
		   ts_array_equals(a->remote, b->remote) &&
		   a->mark_in.value == b->mark_in.value &&
		   a->mark_in.mask == b->mark_in.mask &&
		   a->mark_out.value == b->mark_out.value &&
		   a->mark_out.mask == b->mark_out.mask;
}

/**
 * Create an array from copied traffic selector list items
 */
static array_t *array_from_ts_list(linked_list_t *list)
{
	enumerator_t *enumerator;
	traffic_selector_t *ts;
	array_t *array;

	array = array_create(0, 0);

	enumerator = list->create_enumerator(list);
	while (enumerator->enumerate(enumerator, &ts))
	{
		array_insert(array, ARRAY_TAIL, ts->clone(ts));
	}
	enumerator->destroy(enumerator);

	return array;
}

METHOD(kernel_interface_t, alloc_reqid, status_t,
	private_kernel_interface_t *this,
	linked_list_t *local_ts, linked_list_t *remote_ts,
	mark_t mark_in, mark_t mark_out, uint32_t *reqid)
{
	static uint32_t counter = 0;
	reqid_entry_t *entry = NULL, *tmpl;
	status_t status = SUCCESS;

	INIT(tmpl,
		.local = array_from_ts_list(local_ts),
		.remote = array_from_ts_list(remote_ts),
		.mark_in = mark_in,
		.mark_out = mark_out,
		.reqid = *reqid,
	);

	this->mutex->lock(this->mutex);
	if (tmpl->reqid)
	{
		/* search by reqid if given */
		entry = this->reqids->get(this->reqids, tmpl);
	}
	if (entry)
	{
		/* we don't require a traffic selector match for explicit reqids,
		 * as we want to reuse a reqid for trap-triggered policies that
		 * got narrowed during negotiation. */
		reqid_entry_destroy(tmpl);
	}
	else
	{
		/* search by traffic selectors */
		entry = this->reqids_by_ts->get(this->reqids_by_ts, tmpl);
		if (entry)
		{
			reqid_entry_destroy(tmpl);
		}
		else
		{
			/* none found, create a new entry, allocating a reqid */
			entry = tmpl;
			entry->reqid = ++counter;
			this->reqids_by_ts->put(this->reqids_by_ts, entry, entry);
			this->reqids->put(this->reqids, entry, entry);
		}
		*reqid = entry->reqid;
	}
	entry->refs++;
	this->mutex->unlock(this->mutex);

	return status;
}

METHOD(kernel_interface_t, release_reqid, status_t,
	private_kernel_interface_t *this, uint32_t reqid,
	mark_t mark_in, mark_t mark_out)
{
	reqid_entry_t *entry, tmpl = {
		.reqid = reqid,
		.mark_in = mark_in,
		.mark_out = mark_out,
	};

	this->mutex->lock(this->mutex);
	entry = this->reqids->remove(this->reqids, &tmpl);
	if (entry)
	{
		if (--entry->refs == 0)
		{
			entry = this->reqids_by_ts->remove(this->reqids_by_ts, entry);
			if (entry)
			{
				reqid_entry_destroy(entry);
			}
		}
		else
		{
			this->reqids->put(this->reqids, entry, entry);
		}
	}
	this->mutex->unlock(this->mutex);

	if (entry)
	{
		return SUCCESS;
	}
	return NOT_FOUND;
}

METHOD(kernel_interface_t, add_sa, status_t,
	private_kernel_interface_t *this, kernel_ipsec_sa_id_t *id,
	kernel_ipsec_add_sa_t *data)
{
	if (!this->ipsec)
	{
		return NOT_SUPPORTED;
	}
	return this->ipsec->add_sa(this->ipsec, id, data);
}

METHOD(kernel_interface_t, update_sa, status_t,
	private_kernel_interface_t *this, kernel_ipsec_sa_id_t *id,
	kernel_ipsec_update_sa_t *data)
{
	if (!this->ipsec)
	{
		return NOT_SUPPORTED;
	}
	return this->ipsec->update_sa(this->ipsec, id, data);
}

METHOD(kernel_interface_t, query_sa, status_t,
	private_kernel_interface_t *this, kernel_ipsec_sa_id_t *id,
	kernel_ipsec_query_sa_t *data, uint64_t *bytes, uint64_t *packets,
	time_t *time)
{
	if (!this->ipsec)
	{
		return NOT_SUPPORTED;
	}
	return this->ipsec->query_sa(this->ipsec, id, data, bytes, packets, time);
}

METHOD(kernel_interface_t, del_sa, status_t,
	private_kernel_interface_t *this, kernel_ipsec_sa_id_t *id,
	kernel_ipsec_del_sa_t *data)
{
	if (!this->ipsec)
	{
		return NOT_SUPPORTED;
	}
	return this->ipsec->del_sa(this->ipsec, id, data);
}

METHOD(kernel_interface_t, flush_sas, status_t,
	private_kernel_interface_t *this)
{
	if (!this->ipsec)
	{
		return NOT_SUPPORTED;
	}
	return this->ipsec->flush_sas(this->ipsec);
}

METHOD(kernel_interface_t, add_policy, status_t,
	private_kernel_interface_t *this, kernel_ipsec_policy_id_t *id,
	kernel_ipsec_manage_policy_t *data)
{
	if (!this->ipsec)
	{
		return NOT_SUPPORTED;
	}
	return this->ipsec->add_policy(this->ipsec, id, data);
}

METHOD(kernel_interface_t, query_policy, status_t,
	private_kernel_interface_t *this, kernel_ipsec_policy_id_t *id,
	kernel_ipsec_query_policy_t *data, time_t *use_time)
{
	if (!this->ipsec)
	{
		return NOT_SUPPORTED;
	}
	return this->ipsec->query_policy(this->ipsec, id, data, use_time);
}

METHOD(kernel_interface_t, del_policy, status_t,
	private_kernel_interface_t *this, kernel_ipsec_policy_id_t *id,
	kernel_ipsec_manage_policy_t *data)
{
	if (!this->ipsec)
	{
		return NOT_SUPPORTED;
	}
	return this->ipsec->del_policy(this->ipsec, id, data);
}

METHOD(kernel_interface_t, flush_policies, status_t,
	private_kernel_interface_t *this)
{
	if (!this->ipsec)
	{
		return NOT_SUPPORTED;
	}
	return this->ipsec->flush_policies(this->ipsec);
}

METHOD(kernel_interface_t, get_source_addr, host_t*,
	private_kernel_interface_t *this, host_t *dest, host_t *src)
{
	if (!this->net)
	{
		return NULL;
	}
	return this->net->get_source_addr(this->net, dest, src);
}

METHOD(kernel_interface_t, get_nexthop, host_t*,
	private_kernel_interface_t *this, host_t *dest, int prefix, host_t *src,
	char **iface)
{
	if (!this->net)
	{
		return NULL;
	}
	return this->net->get_nexthop(this->net, dest, prefix, src, iface);
}

METHOD(kernel_interface_t, get_interface, bool,
	private_kernel_interface_t *this, host_t *host, char **name)
{
	if (!this->net)
	{
		return NULL;
	}
	return this->net->get_interface(this->net, host, name);
}

METHOD(kernel_interface_t, create_address_enumerator, enumerator_t*,
	private_kernel_interface_t *this, kernel_address_type_t which)
{
	if (!this->net)
	{
		return enumerator_create_empty();
	}
	return this->net->create_address_enumerator(this->net, which);
}

METHOD(kernel_interface_t, create_local_subnet_enumerator, enumerator_t*,
	private_kernel_interface_t *this)
{
	if (!this->net || !this->net->create_local_subnet_enumerator)
	{
		return enumerator_create_empty();
	}
	return this->net->create_local_subnet_enumerator(this->net);
}

METHOD(kernel_interface_t, add_ip, status_t,
	private_kernel_interface_t *this, host_t *virtual_ip, int prefix,
	char *iface)
{
	if (!this->net)
	{
		return NOT_SUPPORTED;
	}
	return this->net->add_ip(this->net, virtual_ip, prefix, iface);
}

METHOD(kernel_interface_t, del_ip, status_t,
	private_kernel_interface_t *this, host_t *virtual_ip, int prefix, bool wait)
{
	if (!this->net)
	{
		return NOT_SUPPORTED;
	}
	return this->net->del_ip(this->net, virtual_ip, prefix, wait);
}

METHOD(kernel_interface_t, add_route, status_t,
	private_kernel_interface_t *this, chunk_t dst_net,
	uint8_t prefixlen, host_t *gateway, host_t *src_ip, char *if_name)
{
	if (!this->net)
	{
		return NOT_SUPPORTED;
	}
	return this->net->add_route(this->net, dst_net, prefixlen, gateway,
								src_ip, if_name);
}

METHOD(kernel_interface_t, del_route, status_t,
	private_kernel_interface_t *this, chunk_t dst_net,
	uint8_t prefixlen, host_t *gateway, host_t *src_ip, char *if_name)
{
	if (!this->net)
	{
		return NOT_SUPPORTED;
	}
	return this->net->del_route(this->net, dst_net, prefixlen, gateway,
								src_ip, if_name);
}

METHOD(kernel_interface_t, bypass_socket, bool,
	private_kernel_interface_t *this, int fd, int family)
{
	if (!this->ipsec)
	{
		return FALSE;
	}
	return this->ipsec->bypass_socket(this->ipsec, fd, family);
}

METHOD(kernel_interface_t, enable_udp_decap, bool,
	private_kernel_interface_t *this, int fd, int family, uint16_t port)
{
	if (!this->ipsec)
	{
		return FALSE;
	}
	return this->ipsec->enable_udp_decap(this->ipsec, fd, family, port);
}

METHOD(kernel_interface_t, is_interface_usable, bool,
	private_kernel_interface_t *this, const char *iface)
{
	if (!this->ifaces_filter)
	{
		return TRUE;
	}
	return this->ifaces_filter->find_first(this->ifaces_filter,
					linked_list_match_str, NULL, iface) != this->ifaces_exclude;
}

METHOD(kernel_interface_t, all_interfaces_usable, bool,
	private_kernel_interface_t *this)
{
	return !this->ifaces_filter;
}

METHOD(kernel_interface_t, get_address_by_ts, status_t,
	private_kernel_interface_t *this, traffic_selector_t *ts,
	host_t **ip, bool *vip)
{
	enumerator_t *addrs;
	host_t *host;
	int family;
	bool found = FALSE;

	DBG2(DBG_KNL, "getting a local address in traffic selector %R", ts);

	/* if we have a family which includes localhost, we do not
	 * search for an IP, we use the default */
	family = ts->get_type(ts) == TS_IPV4_ADDR_RANGE ? AF_INET : AF_INET6;

	if (family == AF_INET)
	{
		host = host_create_from_string("127.0.0.1", 0);
	}
	else
	{
		host = host_create_from_string("::1", 0);
	}

	if (ts->includes(ts, host))
	{
		*ip = host_create_any(family);
		if (vip)
		{
			*vip = FALSE;
		}
		host->destroy(host);
		DBG2(DBG_KNL, "using host %H", *ip);
		return SUCCESS;
	}
	host->destroy(host);

	/* try virtual IPs only first (on all interfaces) */
	addrs = create_address_enumerator(this,
									  ADDR_TYPE_ALL ^ ADDR_TYPE_REGULAR);
	while (addrs->enumerate(addrs, (void**)&host))
	{
		if (ts->includes(ts, host))
		{
			found = TRUE;
			*ip = host->clone(host);
			if (vip)
			{
				*vip = TRUE;
			}
			break;
		}
	}
	addrs->destroy(addrs);

	if (!found)
	{	/* then try the regular addresses (on all interfaces) */
		addrs = create_address_enumerator(this,
										  ADDR_TYPE_ALL ^ ADDR_TYPE_VIRTUAL);
		while (addrs->enumerate(addrs, (void**)&host))
		{
			if (ts->includes(ts, host))
			{
				found = TRUE;
				*ip = host->clone(host);
				if (vip)
				{
					*vip = FALSE;
				}
				break;
			}
		}
		addrs->destroy(addrs);
	}

	if (!found)
	{
		DBG2(DBG_KNL, "no local address found in traffic selector %R", ts);
		return FAILED;
	}

	DBG2(DBG_KNL, "using host %H", *ip);
	return SUCCESS;
}


METHOD(kernel_interface_t, add_ipsec_interface, bool,
	private_kernel_interface_t *this, kernel_ipsec_constructor_t constructor)
{
	if (!this->ipsec)
	{
		this->ipsec_constructor = constructor;
		this->ipsec = constructor();
		return this->ipsec != NULL;
	}
	return FALSE;
}

METHOD(kernel_interface_t, remove_ipsec_interface, bool,
	private_kernel_interface_t *this, kernel_ipsec_constructor_t constructor)
{
	if (constructor == this->ipsec_constructor && this->ipsec)
	{
		this->ipsec->destroy(this->ipsec);
		this->ipsec = NULL;
		return TRUE;
	}
	return FALSE;
}

METHOD(kernel_interface_t, add_net_interface, bool,
	private_kernel_interface_t *this, kernel_net_constructor_t constructor)
{
	if (!this->net)
	{
		this->net_constructor = constructor;
		this->net = constructor();
		return this->net != NULL;
	}
	return FALSE;
}

METHOD(kernel_interface_t, remove_net_interface, bool,
	private_kernel_interface_t *this, kernel_net_constructor_t constructor)
{
	if (constructor == this->net_constructor && this->net)
	{
		this->net->destroy(this->net);
		this->net = NULL;
		return TRUE;
	}
	return FALSE;
}

METHOD(kernel_interface_t, add_listener, void,
	private_kernel_interface_t *this, kernel_listener_t *listener)
{
	this->mutex->lock(this->mutex);
	this->listeners->insert_last(this->listeners, listener);
	this->mutex->unlock(this->mutex);
}

METHOD(kernel_interface_t, remove_listener, void,
	private_kernel_interface_t *this, kernel_listener_t *listener)
{
	this->mutex->lock(this->mutex);
	this->listeners->remove(this->listeners, listener, NULL);
	this->mutex->unlock(this->mutex);
}

METHOD(kernel_interface_t, acquire, void,
	private_kernel_interface_t *this, uint32_t reqid,
	traffic_selector_t *src_ts, traffic_selector_t *dst_ts)
{
	kernel_listener_t *listener;
	enumerator_t *enumerator;
	this->mutex->lock(this->mutex);
	enumerator = this->listeners->create_enumerator(this->listeners);
	while (enumerator->enumerate(enumerator, &listener))
	{
		if (listener->acquire &&
			!listener->acquire(listener, reqid, src_ts, dst_ts))
		{
			this->listeners->remove_at(this->listeners, enumerator);
		}
	}
	enumerator->destroy(enumerator);
	this->mutex->unlock(this->mutex);
}

METHOD(kernel_interface_t, expire, void,
	private_kernel_interface_t *this, uint8_t protocol, uint32_t spi,
	host_t *dst, bool hard)
{
	kernel_listener_t *listener;
	enumerator_t *enumerator;

	this->mutex->lock(this->mutex);
	enumerator = this->listeners->create_enumerator(this->listeners);
	while (enumerator->enumerate(enumerator, &listener))
	{
		if (listener->expire &&
			!listener->expire(listener, protocol, spi, dst, hard))
		{
			this->listeners->remove_at(this->listeners, enumerator);
		}
	}
	enumerator->destroy(enumerator);
	this->mutex->unlock(this->mutex);
}

METHOD(kernel_interface_t, mapping, void,
	private_kernel_interface_t *this, uint8_t protocol, uint32_t spi,
	host_t *dst, host_t *remote)
{
	kernel_listener_t *listener;
	enumerator_t *enumerator;

	this->mutex->lock(this->mutex);
	enumerator = this->listeners->create_enumerator(this->listeners);
	while (enumerator->enumerate(enumerator, &listener))
	{
		if (listener->mapping &&
			!listener->mapping(listener, protocol, spi, dst, remote))
		{
			this->listeners->remove_at(this->listeners, enumerator);
		}
	}
	enumerator->destroy(enumerator);
	this->mutex->unlock(this->mutex);
}

METHOD(kernel_interface_t, migrate, void,
	private_kernel_interface_t *this, uint32_t reqid,
	traffic_selector_t *src_ts, traffic_selector_t *dst_ts,
	policy_dir_t direction, host_t *local, host_t *remote)
{
	kernel_listener_t *listener;
	enumerator_t *enumerator;
	this->mutex->lock(this->mutex);
	enumerator = this->listeners->create_enumerator(this->listeners);
	while (enumerator->enumerate(enumerator, &listener))
	{
		if (listener->migrate &&
			!listener->migrate(listener, reqid, src_ts, dst_ts, direction,
							   local, remote))
		{
			this->listeners->remove_at(this->listeners, enumerator);
		}
	}
	enumerator->destroy(enumerator);
	this->mutex->unlock(this->mutex);
}

static bool call_roam(kernel_listener_t *listener, bool *roam)
{
	return listener->roam && !listener->roam(listener, *roam);
}

METHOD(kernel_interface_t, roam, void,
	private_kernel_interface_t *this, bool address)
{
	this->mutex->lock(this->mutex);
	this->listeners->remove(this->listeners, &address, (void*)call_roam);
	this->mutex->unlock(this->mutex);
}

METHOD(kernel_interface_t, tun, void,
	private_kernel_interface_t *this, tun_device_t *tun, bool created)
{
	kernel_listener_t *listener;
	enumerator_t *enumerator;
	this->mutex->lock(this->mutex);
	enumerator = this->listeners->create_enumerator(this->listeners);
	while (enumerator->enumerate(enumerator, &listener))
	{
		if (listener->tun &&
			!listener->tun(listener, tun, created))
		{
			this->listeners->remove_at(this->listeners, enumerator);
		}
	}
	enumerator->destroy(enumerator);
	this->mutex->unlock(this->mutex);
}

METHOD(kernel_interface_t, register_algorithm, void,
	private_kernel_interface_t *this, uint16_t alg_id, transform_type_t type,
	uint16_t kernel_id, char *kernel_name)
{
	kernel_algorithm_t *algorithm;

	INIT(algorithm,
		.type = type,
		.ike = alg_id,
		.kernel = kernel_id,
		.name = strdup(kernel_name),
	);

	this->mutex_algs->lock(this->mutex_algs);
	this->algorithms->insert_first(this->algorithms, algorithm);
	this->mutex_algs->unlock(this->mutex_algs);
}

METHOD(kernel_interface_t, lookup_algorithm, bool,
	private_kernel_interface_t *this, uint16_t alg_id, transform_type_t type,
	uint16_t *kernel_id, char **kernel_name)
{
	kernel_algorithm_t *algorithm;
	enumerator_t *enumerator;
	bool found = FALSE;

	this->mutex_algs->lock(this->mutex_algs);
	enumerator = this->algorithms->create_enumerator(this->algorithms);
	while (enumerator->enumerate(enumerator, &algorithm))
	{
		if (algorithm->type == type && algorithm->ike == alg_id)
		{
			if (kernel_id)
			{
				*kernel_id = algorithm->kernel;
			}
			if (kernel_name)
			{
				*kernel_name = algorithm->name;
			}
			found = TRUE;
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->mutex_algs->unlock(this->mutex_algs);
	return found;
}

METHOD(kernel_interface_t, destroy, void,
	private_kernel_interface_t *this)
{
	kernel_algorithm_t *algorithm;

	while (this->algorithms->remove_first(this->algorithms,
										 (void**)&algorithm) == SUCCESS)
	{
		free(algorithm->name);
		free(algorithm);
	}
	this->algorithms->destroy(this->algorithms);
	this->mutex_algs->destroy(this->mutex_algs);
	DESTROY_IF(this->ipsec);
	DESTROY_IF(this->net);
	DESTROY_FUNCTION_IF(this->ifaces_filter, (void*)free);
	this->reqids->destroy(this->reqids);
	this->reqids_by_ts->destroy(this->reqids_by_ts);
	this->listeners->destroy(this->listeners);
	this->mutex->destroy(this->mutex);
	free(this);
}

/*
 * Described in header-file
 */
kernel_interface_t *kernel_interface_create()
{
	private_kernel_interface_t *this;
	char *ifaces;

	INIT(this,
		.public = {
			.get_features = _get_features,
			.get_spi = _get_spi,
			.get_cpi = _get_cpi,
			.alloc_reqid = _alloc_reqid,
			.release_reqid = _release_reqid,
			.add_sa = _add_sa,
			.update_sa = _update_sa,
			.query_sa = _query_sa,
			.del_sa = _del_sa,
			.flush_sas = _flush_sas,
			.add_policy = _add_policy,
			.query_policy = _query_policy,
			.del_policy = _del_policy,
			.flush_policies = _flush_policies,
			.get_source_addr = _get_source_addr,
			.get_nexthop = _get_nexthop,
			.get_interface = _get_interface,
			.create_address_enumerator = _create_address_enumerator,
			.create_local_subnet_enumerator = _create_local_subnet_enumerator,
			.add_ip = _add_ip,
			.del_ip = _del_ip,
			.add_route = _add_route,
			.del_route = _del_route,
			.bypass_socket = _bypass_socket,
			.enable_udp_decap = _enable_udp_decap,

			.is_interface_usable = _is_interface_usable,
			.all_interfaces_usable = _all_interfaces_usable,
			.get_address_by_ts = _get_address_by_ts,
			.add_ipsec_interface = _add_ipsec_interface,
			.remove_ipsec_interface = _remove_ipsec_interface,
			.add_net_interface = _add_net_interface,
			.remove_net_interface = _remove_net_interface,

			.add_listener = _add_listener,
			.remove_listener = _remove_listener,
			.register_algorithm = _register_algorithm,
			.lookup_algorithm = _lookup_algorithm,
			.acquire = _acquire,
			.expire = _expire,
			.mapping = _mapping,
			.migrate = _migrate,
			.roam = _roam,
			.tun = _tun,
			.destroy = _destroy,
		},
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.listeners = linked_list_create(),
		.mutex_algs = mutex_create(MUTEX_TYPE_DEFAULT),
		.algorithms = linked_list_create(),
		.reqids = hashtable_create((hashtable_hash_t)hash_reqid,
								   (hashtable_equals_t)equals_reqid, 8),
		.reqids_by_ts = hashtable_create((hashtable_hash_t)hash_reqid_by_ts,
								   (hashtable_equals_t)equals_reqid_by_ts, 8),
	);

	ifaces = lib->settings->get_str(lib->settings,
									"%s.interfaces_use", NULL, lib->ns);
	if (!ifaces)
	{
		this->ifaces_exclude = TRUE;
		ifaces = lib->settings->get_str(lib->settings,
									"%s.interfaces_ignore", NULL, lib->ns);
	}
	if (ifaces)
	{
		enumerator_t *enumerator;
		char *iface;

		enumerator = enumerator_create_token(ifaces, ",", " ");
		while (enumerator->enumerate(enumerator, &iface))
		{
			if (!this->ifaces_filter)
			{
				this->ifaces_filter = linked_list_create();
			}
			this->ifaces_filter->insert_last(this->ifaces_filter,
											 strdup(iface));
		}
		enumerator->destroy(enumerator);
	}

	return &this->public;
}
