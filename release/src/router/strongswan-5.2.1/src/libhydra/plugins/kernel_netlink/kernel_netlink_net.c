/*
 * Copyright (C) 2008-2014 Tobias Brunner
 * Copyright (C) 2005-2008 Martin Willi
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

/*
 * Copyright (C) 2010 secunet Security Networks AG
 * Copyright (C) 2010 Thomas Egerer
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

#include <sys/socket.h>
#include <sys/utsname.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <unistd.h>
#include <errno.h>
#include <net/if.h>
#ifdef HAVE_LINUX_FIB_RULES_H
#include <linux/fib_rules.h>
#endif

#include "kernel_netlink_net.h"
#include "kernel_netlink_shared.h"

#include <hydra.h>
#include <utils/debug.h>
#include <threading/mutex.h>
#include <threading/rwlock.h>
#include <threading/rwlock_condvar.h>
#include <threading/spinlock.h>
#include <collections/hashtable.h>
#include <collections/linked_list.h>
#include <processing/jobs/callback_job.h>

/** delay before firing roam events (ms) */
#define ROAM_DELAY 100

/** delay before reinstalling routes (ms) */
#define ROUTE_DELAY 100

/** maximum recursion when searching for addresses in get_route() */
#define MAX_ROUTE_RECURSION 2

#ifndef ROUTING_TABLE
#define ROUTING_TABLE 0
#endif

#ifndef ROUTING_TABLE_PRIO
#define ROUTING_TABLE_PRIO 0
#endif

ENUM(rt_msg_names, RTM_NEWLINK, RTM_GETRULE,
	"RTM_NEWLINK",
	"RTM_DELLINK",
	"RTM_GETLINK",
	"RTM_SETLINK",
	"RTM_NEWADDR",
	"RTM_DELADDR",
	"RTM_GETADDR",
	"31",
	"RTM_NEWROUTE",
	"RTM_DELROUTE",
	"RTM_GETROUTE",
	"35",
	"RTM_NEWNEIGH",
	"RTM_DELNEIGH",
	"RTM_GETNEIGH",
	"RTM_NEWRULE",
	"RTM_DELRULE",
	"RTM_GETRULE",
);

typedef struct addr_entry_t addr_entry_t;

/**
 * IP address in an iface_entry_t
 */
struct addr_entry_t {

	/** the ip address */
	host_t *ip;

	/** address flags */
	u_char flags;

	/** scope of the address */
	u_char scope;

	/** number of times this IP is used, if virtual (i.e. managed by us) */
	u_int refcount;

	/** TRUE once it is installed, if virtual */
	bool installed;
};

/**
 * destroy a addr_entry_t object
 */
static void addr_entry_destroy(addr_entry_t *this)
{
	this->ip->destroy(this->ip);
	free(this);
}

typedef struct iface_entry_t iface_entry_t;

/**
 * A network interface on this system, containing addr_entry_t's
 */
struct iface_entry_t {

	/** interface index */
	int ifindex;

	/** name of the interface */
	char ifname[IFNAMSIZ];

	/** interface flags, as in netdevice(7) SIOCGIFFLAGS */
	u_int flags;

	/** list of addresses as host_t */
	linked_list_t *addrs;

	/** TRUE if usable by config */
	bool usable;
};

/**
 * destroy an interface entry
 */
static void iface_entry_destroy(iface_entry_t *this)
{
	this->addrs->destroy_function(this->addrs, (void*)addr_entry_destroy);
	free(this);
}

/**
 * find an interface entry by index
 */
static bool iface_entry_by_index(iface_entry_t *this, int *ifindex)
{
	return this->ifindex == *ifindex;
}

/**
 * find an interface entry by name
 */
static bool iface_entry_by_name(iface_entry_t *this, char *ifname)
{
	return streq(this->ifname, ifname);
}

/**
 * check if an interface is up
 */
static inline bool iface_entry_up(iface_entry_t *iface)
{
	return (iface->flags & IFF_UP) == IFF_UP;
}

/**
 * check if an interface is up and usable
 */
static inline bool iface_entry_up_and_usable(iface_entry_t *iface)
{
	return iface->usable && iface_entry_up(iface);
}

typedef struct addr_map_entry_t addr_map_entry_t;

/**
 * Entry that maps an IP address to an interface entry
 */
struct addr_map_entry_t {
	/** The IP address */
	host_t *ip;

	/** The address entry for this IP address */
	addr_entry_t *addr;

	/** The interface this address is installed on */
	iface_entry_t *iface;
};

/**
 * Hash a addr_map_entry_t object, all entries with the same IP address
 * are stored in the same bucket
 */
static u_int addr_map_entry_hash(addr_map_entry_t *this)
{
	return chunk_hash(this->ip->get_address(this->ip));
}

/**
 * Compare two addr_map_entry_t objects, two entries are equal if they are
 * installed on the same interface
 */
static bool addr_map_entry_equals(addr_map_entry_t *a, addr_map_entry_t *b)
{
	return a->iface->ifindex == b->iface->ifindex &&
		   a->ip->ip_equals(a->ip, b->ip);
}

/**
 * Used with get_match this finds an address entry if it is installed on
 * an up and usable interface
 */
static bool addr_map_entry_match_up_and_usable(addr_map_entry_t *a,
											   addr_map_entry_t *b)
{
	return iface_entry_up_and_usable(b->iface) &&
		   a->ip->ip_equals(a->ip, b->ip);
}

/**
 * Used with get_match this finds an address entry if it is installed on
 * any active local interface
 */
static bool addr_map_entry_match_up(addr_map_entry_t *a, addr_map_entry_t *b)
{
	return iface_entry_up(b->iface) && a->ip->ip_equals(a->ip, b->ip);
}

/**
 * Used with get_match this finds an address entry if it is installed on
 * any local interface
 */
static bool addr_map_entry_match(addr_map_entry_t *a, addr_map_entry_t *b)
{
	return a->ip->ip_equals(a->ip, b->ip);
}

typedef struct route_entry_t route_entry_t;

/**
 * Installed routing entry
 */
struct route_entry_t {
	/** Name of the interface the route is bound to */
	char *if_name;

	/** Source ip of the route */
	host_t *src_ip;

	/** Gateway for this route */
	host_t *gateway;

	/** Destination net */
	chunk_t dst_net;

	/** Destination net prefixlen */
	u_int8_t prefixlen;
};

/**
 * Clone a route_entry_t object.
 */
static route_entry_t *route_entry_clone(route_entry_t *this)
{
	route_entry_t *route;

	INIT(route,
		.if_name = strdup(this->if_name),
		.src_ip = this->src_ip->clone(this->src_ip),
		.gateway = this->gateway ? this->gateway->clone(this->gateway) : NULL,
		.dst_net = chunk_clone(this->dst_net),
		.prefixlen = this->prefixlen,
	);
	return route;
}

/**
 * Destroy a route_entry_t object
 */
static void route_entry_destroy(route_entry_t *this)
{
	free(this->if_name);
	DESTROY_IF(this->src_ip);
	DESTROY_IF(this->gateway);
	chunk_free(&this->dst_net);
	free(this);
}

/**
 * Hash a route_entry_t object
 */
static u_int route_entry_hash(route_entry_t *this)
{
	return chunk_hash_inc(chunk_from_thing(this->prefixlen),
						  chunk_hash(this->dst_net));
}

/**
 * Compare two route_entry_t objects
 */
static bool route_entry_equals(route_entry_t *a, route_entry_t *b)
{
	if (a->if_name && b->if_name && streq(a->if_name, b->if_name) &&
		a->src_ip->ip_equals(a->src_ip, b->src_ip) &&
		chunk_equals(a->dst_net, b->dst_net) && a->prefixlen == b->prefixlen)
	{
		return (!a->gateway && !b->gateway) || (a->gateway && b->gateway &&
					a->gateway->ip_equals(a->gateway, b->gateway));
	}
	return FALSE;
}

typedef struct net_change_t net_change_t;

/**
 * Queued network changes
 */
struct net_change_t {
	/** Name of the interface that got activated (or an IP appeared on) */
	char *if_name;
};

/**
 * Destroy a net_change_t object
 */
static void net_change_destroy(net_change_t *this)
{
	free(this->if_name);
	free(this);
}

/**
 * Hash a net_change_t object
 */
static u_int net_change_hash(net_change_t *this)
{
	return chunk_hash(chunk_create(this->if_name, strlen(this->if_name)));
}

/**
 * Compare two net_change_t objects
 */
static bool net_change_equals(net_change_t *a, net_change_t *b)
{
	return streq(a->if_name, b->if_name);
}

typedef struct private_kernel_netlink_net_t private_kernel_netlink_net_t;

/**
 * Private variables and functions of kernel_netlink_net class.
 */
struct private_kernel_netlink_net_t {
	/**
	 * Public part of the kernel_netlink_net_t object.
	 */
	kernel_netlink_net_t public;

	/**
	 * lock to access various lists and maps
	 */
	rwlock_t *lock;

	/**
	 * condition variable to signal virtual IP add/removal
	 */
	rwlock_condvar_t *condvar;

	/**
	 * Cached list of interfaces and its addresses (iface_entry_t)
	 */
	linked_list_t *ifaces;

	/**
	 * Map for IP addresses to iface_entry_t objects (addr_map_entry_t)
	 */
	hashtable_t *addrs;

	/**
	 * Map for virtual IP addresses to iface_entry_t objects (addr_map_entry_t)
	 */
	hashtable_t *vips;

	/**
	 * netlink rt socket (routing)
	 */
	netlink_socket_t *socket;

	/**
	 * Netlink rt socket to receive address change events
	 */
	int socket_events;

	/**
	 * earliest time of the next roam event
	 */
	timeval_t next_roam;

	/**
	 * roam event due to address change
	 */
	bool roam_address;

	/**
	 * lock to check and update roam event time
	 */
	spinlock_t *roam_lock;

	/**
	 * routing table to install routes
	 */
	int routing_table;

	/**
	 * priority of used routing table
	 */
	int routing_table_prio;

	/**
	 * installed routes
	 */
	hashtable_t *routes;

	/**
	 * mutex for routes
	 */
	mutex_t *routes_lock;

	/**
	 * interface changes which may trigger route reinstallation
	 */
	hashtable_t *net_changes;

	/**
	 * mutex for route reinstallation triggers
	 */
	mutex_t *net_changes_lock;

	/**
	 * time of last route reinstallation
	 */
	timeval_t last_route_reinstall;

	/**
	 * whether to react to RTM_NEWROUTE or RTM_DELROUTE events
	 */
	bool process_route;

	/**
	 * whether to trigger roam events
	 */
	bool roam_events;

	/**
	 * whether to actually install virtual IPs
	 */
	bool install_virtual_ip;

	/**
	 * the name of the interface virtual IP addresses are installed on
	 */
	char *install_virtual_ip_on;

	/**
	 * whether preferred source addresses can be specified for IPv6 routes
	 */
	bool rta_prefsrc_for_ipv6;

	/**
	 * whether to prefer temporary IPv6 addresses over public ones
	 */
	bool prefer_temporary_addrs;

	/**
	 * list with routing tables to be excluded from route lookup
	 */
	linked_list_t *rt_exclude;

	/**
	 * MTU to set on installed routes
	 */
	u_int32_t mtu;

	/**
	 * MSS to set on installed routes
	 */
	u_int32_t mss;
};

/**
 * Forward declaration
 */
static status_t manage_srcroute(private_kernel_netlink_net_t *this,
								int nlmsg_type, int flags, chunk_t dst_net,
								u_int8_t prefixlen, host_t *gateway,
								host_t *src_ip, char *if_name);

/**
 * Clear the queued network changes.
 */
static void net_changes_clear(private_kernel_netlink_net_t *this)
{
	enumerator_t *enumerator;
	net_change_t *change;

	enumerator = this->net_changes->create_enumerator(this->net_changes);
	while (enumerator->enumerate(enumerator, NULL, (void**)&change))
	{
		this->net_changes->remove_at(this->net_changes, enumerator);
		net_change_destroy(change);
	}
	enumerator->destroy(enumerator);
}

/**
 * Act upon queued network changes.
 */
static job_requeue_t reinstall_routes(private_kernel_netlink_net_t *this)
{
	enumerator_t *enumerator;
	route_entry_t *route;

	this->net_changes_lock->lock(this->net_changes_lock);
	this->routes_lock->lock(this->routes_lock);

	enumerator = this->routes->create_enumerator(this->routes);
	while (enumerator->enumerate(enumerator, NULL, (void**)&route))
	{
		net_change_t *change, lookup = {
			.if_name = route->if_name,
		};
		/* check if a change for the outgoing interface is queued */
		change = this->net_changes->get(this->net_changes, &lookup);
		if (!change)
		{	/* in case src_ip is not on the outgoing interface */
			if (this->public.interface.get_interface(&this->public.interface,
												route->src_ip, &lookup.if_name))
			{
				if (!streq(lookup.if_name, route->if_name))
				{
					change = this->net_changes->get(this->net_changes, &lookup);
				}
				free(lookup.if_name);
			}
		}
		if (change)
		{
			manage_srcroute(this, RTM_NEWROUTE, NLM_F_CREATE | NLM_F_EXCL,
							route->dst_net, route->prefixlen, route->gateway,
							route->src_ip, route->if_name);
		}
	}
	enumerator->destroy(enumerator);
	this->routes_lock->unlock(this->routes_lock);

	net_changes_clear(this);
	this->net_changes_lock->unlock(this->net_changes_lock);
	return JOB_REQUEUE_NONE;
}

/**
 * Queue route reinstallation caused by network changes for a given interface.
 *
 * The route reinstallation is delayed for a while and only done once for
 * several calls during this delay, in order to avoid doing it too often.
 * The interface name is freed.
 */
static void queue_route_reinstall(private_kernel_netlink_net_t *this,
								  char *if_name)
{
	net_change_t *update, *found;
	timeval_t now;
	job_t *job;

	INIT(update,
		.if_name = if_name
	);

	this->net_changes_lock->lock(this->net_changes_lock);
	found = this->net_changes->put(this->net_changes, update, update);
	if (found)
	{
		net_change_destroy(found);
	}
	time_monotonic(&now);
	if (timercmp(&now, &this->last_route_reinstall, >))
	{
		timeval_add_ms(&now, ROUTE_DELAY);
		this->last_route_reinstall = now;

		job = (job_t*)callback_job_create((callback_job_cb_t)reinstall_routes,
										  this, NULL, NULL);
		lib->scheduler->schedule_job_ms(lib->scheduler, job, ROUTE_DELAY);
	}
	this->net_changes_lock->unlock(this->net_changes_lock);
}

/**
 * check if the given IP is known as virtual IP and currently installed
 *
 * this function will also return TRUE if the virtual IP entry disappeared.
 * in that case the returned entry will be NULL.
 *
 * this->lock must be held when calling this function
 */
static bool is_vip_installed_or_gone(private_kernel_netlink_net_t *this,
									 host_t *ip, addr_map_entry_t **entry)
{
	addr_map_entry_t lookup = {
		.ip = ip,
	};

	*entry = this->vips->get_match(this->vips, &lookup,
								  (void*)addr_map_entry_match);
	if (*entry == NULL)
	{	/* the virtual IP disappeared */
		return TRUE;
	}
	return (*entry)->addr->installed;
}

/**
 * check if the given IP is known as virtual IP
 *
 * this->lock must be held when calling this function
 */
static bool is_known_vip(private_kernel_netlink_net_t *this, host_t *ip)
{
	addr_map_entry_t lookup = {
		.ip = ip,
	};

	return this->vips->get_match(this->vips, &lookup,
								(void*)addr_map_entry_match) != NULL;
}

/**
 * Add an address map entry
 */
static void addr_map_entry_add(hashtable_t *map, addr_entry_t *addr,
							   iface_entry_t *iface)
{
	addr_map_entry_t *entry;

	INIT(entry,
		.ip = addr->ip,
		.addr = addr,
		.iface = iface,
	);
	entry = map->put(map, entry, entry);
	free(entry);
}

/**
 * Remove an address map entry
 */
static void addr_map_entry_remove(hashtable_t *map, addr_entry_t *addr,
								  iface_entry_t *iface)
{
	addr_map_entry_t *entry, lookup = {
		.ip = addr->ip,
		.addr = addr,
		.iface = iface,
	};

	entry = map->remove(map, &lookup);
	free(entry);
}

/**
 * Determine the type or scope of the given unicast IP address.  This is not
 * the same thing returned in rtm_scope/ifa_scope.
 *
 * We use return values as defined in RFC 6724 (referring to RFC 4291).
 */
static u_char get_scope(host_t *ip)
{
	chunk_t addr;

	addr = ip->get_address(ip);
	switch (addr.len)
	{
		case 4:
			/* we use the mapping defined in RFC 6724, 3.2 */
			if (addr.ptr[0] == 127)
			{	/* link-local, same as the IPv6 loopback address */
				return 2;
			}
			if (addr.ptr[0] == 169 && addr.ptr[1] == 254)
			{	/* link-local */
				return 2;
			}
			break;
		case 16:
			if (IN6_IS_ADDR_LOOPBACK((struct in6_addr*)addr.ptr))
			{	/* link-local, according to RFC 4291, 2.5.3 */
				return 2;
			}
			if (IN6_IS_ADDR_LINKLOCAL((struct in6_addr*)addr.ptr))
			{
				return 2;
			}
			if (IN6_IS_ADDR_SITELOCAL((struct in6_addr*)addr.ptr))
			{	/* deprecated, according to RFC 4291, 2.5.7 */
				return 5;
			}
			break;
		default:
			break;
	}
	/* global */
	return 14;
}

/**
 * Returns the length of the common prefix in bits up to the length of a's
 * prefix, defined by RFC 6724 as the portion of the address not including the
 * interface ID, which is 64-bit for most unicast addresses (see RFC 4291).
 */
static u_char common_prefix(host_t *a, host_t *b)
{
	chunk_t aa, ba;
	u_char byte, bits = 0, match;

	aa = a->get_address(a);
	ba = b->get_address(b);
	for (byte = 0; byte < 8; byte++)
	{
		if (aa.ptr[byte] != ba.ptr[byte])
		{
			match = aa.ptr[byte] ^ ba.ptr[byte];
			for (bits = 8; match; match >>= 1)
			{
				bits--;
			}
			break;
		}
	}
	return byte * 8 + bits;
}

/**
 * Compare two IP addresses and return TRUE if the second address is the better
 * choice of the two to reach the destination.
 * For IPv6 we approximately follow RFC 6724.
 */
static bool is_address_better(private_kernel_netlink_net_t *this,
							  addr_entry_t *a, addr_entry_t *b, host_t *d)
{
	u_char sa, sb, sd, pa, pb;

	/* rule 2: prefer appropriate scope */
	if (d)
	{
		sa = get_scope(a->ip);
		sb = get_scope(b->ip);
		sd = get_scope(d);
		if (sa < sb)
		{
			return sa < sd;
		}
		else if (sb < sa)
		{
			return sb >= sd;
		}
	}
	if (a->ip->get_family(a->ip) == AF_INET)
	{	/* stop here for IPv4, default to addresses found earlier */
		return FALSE;
	}
	/* rule 3: avoid deprecated addresses (RFC 4862) */
	if ((a->flags & IFA_F_DEPRECATED) != (b->flags & IFA_F_DEPRECATED))
	{
		return a->flags & IFA_F_DEPRECATED;
	}
	/* rule 4 is not applicable as we don't know if an address is a home or
	 * care-of addresses.
	 * rule 5 does not apply as we only compare addresses from one interface
	 * rule 6 requires a policy table (optionally configurable) to match
	 * configurable labels
	 */
	/* rule 7: prefer temporary addresses (WE REVERSE THIS BY DEFAULT!) */
	if ((a->flags & IFA_F_TEMPORARY) != (b->flags & IFA_F_TEMPORARY))
	{
		if (this->prefer_temporary_addrs)
		{
			return b->flags & IFA_F_TEMPORARY;
		}
		return a->flags & IFA_F_TEMPORARY;
	}
	/* rule 8: use longest matching prefix */
	if (d)
	{
		pa = common_prefix(a->ip, d);
		pb = common_prefix(b->ip, d);
		if (pa != pb)
		{
			return pb > pa;
		}
	}
	/* default to addresses found earlier */
	return FALSE;
}

/**
 * Get a non-virtual IP address on the given interface.
 *
 * If a candidate address is given, we first search for that address and if not
 * found return the address as above.
 * Returned host is a clone, has to be freed by caller.
 *
 * this->lock must be held when calling this function.
 */
static host_t *get_interface_address(private_kernel_netlink_net_t *this,
									 int ifindex, int family, host_t *dest,
									 host_t *candidate)
{
	iface_entry_t *iface;
	enumerator_t *addrs;
	addr_entry_t *addr, *best = NULL;

	if (this->ifaces->find_first(this->ifaces, (void*)iface_entry_by_index,
								 (void**)&iface, &ifindex) == SUCCESS)
	{
		if (iface->usable)
		{	/* only use interfaces not excluded by config */
			addrs = iface->addrs->create_enumerator(iface->addrs);
			while (addrs->enumerate(addrs, &addr))
			{
				if (addr->refcount ||
					addr->ip->get_family(addr->ip) != family)
				{	/* ignore virtual IP addresses and ensure family matches */
					continue;
				}
				if (candidate && candidate->ip_equals(candidate, addr->ip))
				{	/* stop if we find the candidate */
					best = addr;
					break;
				}
				else if (!best || is_address_better(this, best, addr, dest))
				{
					best = addr;
				}
			}
			addrs->destroy(addrs);
		}
	}
	return best ? best->ip->clone(best->ip) : NULL;
}

/**
 * callback function that raises the delayed roam event
 */
static job_requeue_t roam_event(private_kernel_netlink_net_t *this)
{
	bool address;

	this->roam_lock->lock(this->roam_lock);
	address = this->roam_address;
	this->roam_address = FALSE;
	this->roam_lock->unlock(this->roam_lock);
	hydra->kernel_interface->roam(hydra->kernel_interface, address);
	return JOB_REQUEUE_NONE;
}

/**
 * fire a roaming event. we delay it for a bit and fire only one event
 * for multiple calls. otherwise we would create too many events.
 */
static void fire_roam_event(private_kernel_netlink_net_t *this, bool address)
{
	timeval_t now;
	job_t *job;

	if (!this->roam_events)
	{
		return;
	}

	time_monotonic(&now);
	this->roam_lock->lock(this->roam_lock);
	this->roam_address |= address;
	if (!timercmp(&now, &this->next_roam, >))
	{
		this->roam_lock->unlock(this->roam_lock);
		return;
	}
	timeval_add_ms(&now, ROAM_DELAY);
	this->next_roam = now;
	this->roam_lock->unlock(this->roam_lock);

	job = (job_t*)callback_job_create((callback_job_cb_t)roam_event,
									  this, NULL, NULL);
	lib->scheduler->schedule_job_ms(lib->scheduler, job, ROAM_DELAY);
}

/**
 * check if an interface with a given index is up and usable
 *
 * this->lock must be locked when calling this function
 */
static bool is_interface_up_and_usable(private_kernel_netlink_net_t *this,
									   int index)
{
	iface_entry_t *iface;

	if (this->ifaces->find_first(this->ifaces, (void*)iface_entry_by_index,
								 (void**)&iface, &index) == SUCCESS)
	{
		return iface_entry_up_and_usable(iface);
	}
	return FALSE;
}

/**
 * unregister the current addr_entry_t from the hashtable it is stored in
 *
 * this->lock must be locked when calling this function
 */
static void addr_entry_unregister(addr_entry_t *addr, iface_entry_t *iface,
								  private_kernel_netlink_net_t *this)
{
	if (addr->refcount)
	{
		addr_map_entry_remove(this->vips, addr, iface);
		this->condvar->broadcast(this->condvar);
		return;
	}
	addr_map_entry_remove(this->addrs, addr, iface);
}

/**
 * process RTM_NEWLINK/RTM_DELLINK from kernel
 */
static void process_link(private_kernel_netlink_net_t *this,
						 struct nlmsghdr *hdr, bool event)
{
	struct ifinfomsg* msg = NLMSG_DATA(hdr);
	struct rtattr *rta = IFLA_RTA(msg);
	size_t rtasize = IFLA_PAYLOAD (hdr);
	enumerator_t *enumerator;
	iface_entry_t *current, *entry = NULL;
	char *name = NULL;
	bool update = FALSE, update_routes = FALSE;

	while (RTA_OK(rta, rtasize))
	{
		switch (rta->rta_type)
		{
			case IFLA_IFNAME:
				name = RTA_DATA(rta);
				break;
		}
		rta = RTA_NEXT(rta, rtasize);
	}
	if (!name)
	{
		name = "(unknown)";
	}

	this->lock->write_lock(this->lock);
	switch (hdr->nlmsg_type)
	{
		case RTM_NEWLINK:
		{
			if (this->ifaces->find_first(this->ifaces,
									(void*)iface_entry_by_index, (void**)&entry,
									&msg->ifi_index) != SUCCESS)
			{
				INIT(entry,
					.ifindex = msg->ifi_index,
					.addrs = linked_list_create(),
					.usable = hydra->kernel_interface->is_interface_usable(
												hydra->kernel_interface, name),
				);
				this->ifaces->insert_last(this->ifaces, entry);
			}
			strncpy(entry->ifname, name, IFNAMSIZ);
			entry->ifname[IFNAMSIZ-1] = '\0';
			if (event && entry->usable)
			{
				if (!(entry->flags & IFF_UP) && (msg->ifi_flags & IFF_UP))
				{
					update = update_routes = TRUE;
					DBG1(DBG_KNL, "interface %s activated", name);
				}
				if ((entry->flags & IFF_UP) && !(msg->ifi_flags & IFF_UP))
				{
					update = TRUE;
					DBG1(DBG_KNL, "interface %s deactivated", name);
				}
			}
			entry->flags = msg->ifi_flags;
			break;
		}
		case RTM_DELLINK:
		{
			enumerator = this->ifaces->create_enumerator(this->ifaces);
			while (enumerator->enumerate(enumerator, &current))
			{
				if (current->ifindex == msg->ifi_index)
				{
					if (event && current->usable)
					{
						update = TRUE;
						DBG1(DBG_KNL, "interface %s deleted", current->ifname);
					}
					/* TODO: move virtual IPs installed on this interface to
					 * another interface? */
					this->ifaces->remove_at(this->ifaces, enumerator);
					current->addrs->invoke_function(current->addrs,
								(void*)addr_entry_unregister, current, this);
					iface_entry_destroy(current);
					break;
				}
			}
			enumerator->destroy(enumerator);
			break;
		}
	}
	this->lock->unlock(this->lock);

	if (update_routes && event)
	{
		queue_route_reinstall(this, strdup(name));
	}

	if (update && event)
	{
		fire_roam_event(this, TRUE);
	}
}

/**
 * process RTM_NEWADDR/RTM_DELADDR from kernel
 */
static void process_addr(private_kernel_netlink_net_t *this,
						 struct nlmsghdr *hdr, bool event)
{
	struct ifaddrmsg* msg = NLMSG_DATA(hdr);
	struct rtattr *rta = IFA_RTA(msg);
	size_t rtasize = IFA_PAYLOAD (hdr);
	host_t *host = NULL;
	iface_entry_t *iface;
	chunk_t local = chunk_empty, address = chunk_empty;
	char *route_ifname = NULL;
	bool update = FALSE, found = FALSE, changed = FALSE;

	while (RTA_OK(rta, rtasize))
	{
		switch (rta->rta_type)
		{
			case IFA_LOCAL:
				local.ptr = RTA_DATA(rta);
				local.len = RTA_PAYLOAD(rta);
				break;
			case IFA_ADDRESS:
				address.ptr = RTA_DATA(rta);
				address.len = RTA_PAYLOAD(rta);
				break;
		}
		rta = RTA_NEXT(rta, rtasize);
	}

	/* For PPP interfaces, we need the IFA_LOCAL address,
	 * IFA_ADDRESS is the peers address. But IFA_LOCAL is
	 * not included in all cases (IPv6?), so fallback to IFA_ADDRESS. */
	if (local.ptr)
	{
		host = host_create_from_chunk(msg->ifa_family, local, 0);
	}
	else if (address.ptr)
	{
		host = host_create_from_chunk(msg->ifa_family, address, 0);
	}

	if (host == NULL)
	{	/* bad family? */
		return;
	}

	this->lock->write_lock(this->lock);
	if (this->ifaces->find_first(this->ifaces, (void*)iface_entry_by_index,
								 (void**)&iface, &msg->ifa_index) == SUCCESS)
	{
		addr_map_entry_t *entry, lookup = {
			.ip = host,
			.iface = iface,
		};
		addr_entry_t *addr;

		entry = this->vips->get(this->vips, &lookup);
		if (entry)
		{
			if (hdr->nlmsg_type == RTM_NEWADDR)
			{	/* mark as installed and signal waiting threads */
				entry->addr->installed = TRUE;
			}
			else
			{	/* the address was already marked as uninstalled */
				addr = entry->addr;
				iface->addrs->remove(iface->addrs, addr, NULL);
				addr_map_entry_remove(this->vips, addr, iface);
				addr_entry_destroy(addr);
			}
			/* no roam events etc. for virtual IPs */
			this->condvar->broadcast(this->condvar);
			this->lock->unlock(this->lock);
			host->destroy(host);
			return;
		}
		entry = this->addrs->get(this->addrs, &lookup);
		if (entry)
		{
			if (hdr->nlmsg_type == RTM_DELADDR)
			{
				found = TRUE;
				addr = entry->addr;
				iface->addrs->remove(iface->addrs, addr, NULL);
				if (iface->usable)
				{
					changed = TRUE;
					DBG1(DBG_KNL, "%H disappeared from %s", host,
						 iface->ifname);
				}
				addr_map_entry_remove(this->addrs, addr, iface);
				addr_entry_destroy(addr);
			}
		}
		else
		{
			if (hdr->nlmsg_type == RTM_NEWADDR)
			{
				found = TRUE;
				changed = TRUE;
				route_ifname = strdup(iface->ifname);
				INIT(addr,
					.ip = host->clone(host),
					.flags = msg->ifa_flags,
					.scope = msg->ifa_scope,
				);
				iface->addrs->insert_last(iface->addrs, addr);
				addr_map_entry_add(this->addrs, addr, iface);
				if (event && iface->usable)
				{
					DBG1(DBG_KNL, "%H appeared on %s", host, iface->ifname);
				}
			}
		}
		if (found && (iface->flags & IFF_UP))
		{
			update = TRUE;
		}
		if (!iface->usable)
		{	/* ignore events for interfaces excluded by config */
			update = changed = FALSE;
		}
	}
	this->lock->unlock(this->lock);

	if (update && event && route_ifname)
	{
		queue_route_reinstall(this, route_ifname);
	}
	else
	{
		free(route_ifname);
	}
	host->destroy(host);

	/* send an update to all IKE_SAs */
	if (update && event && changed)
	{
		fire_roam_event(this, TRUE);
	}
}

/**
 * process RTM_NEWROUTE and RTM_DELROUTE from kernel
 */
static void process_route(private_kernel_netlink_net_t *this, struct nlmsghdr *hdr)
{
	struct rtmsg* msg = NLMSG_DATA(hdr);
	struct rtattr *rta = RTM_RTA(msg);
	size_t rtasize = RTM_PAYLOAD(hdr);
	u_int32_t rta_oif = 0;
	host_t *host = NULL;

	/* ignore routes added by us or in the local routing table (local addrs) */
	if (msg->rtm_table && (msg->rtm_table == this->routing_table ||
						   msg->rtm_table == RT_TABLE_LOCAL))
	{
		return;
	}
	else if (msg->rtm_flags & RTM_F_CLONED)
	{	/* ignore cached routes, seem to be created a lot for IPv6 */
		return;
	}

	while (RTA_OK(rta, rtasize))
	{
		switch (rta->rta_type)
		{
			case RTA_PREFSRC:
				DESTROY_IF(host);
				host = host_create_from_chunk(msg->rtm_family,
							chunk_create(RTA_DATA(rta), RTA_PAYLOAD(rta)), 0);
				break;
			case RTA_OIF:
				if (RTA_PAYLOAD(rta) == sizeof(rta_oif))
				{
					rta_oif = *(u_int32_t*)RTA_DATA(rta);
				}
				break;
		}
		rta = RTA_NEXT(rta, rtasize);
	}
	this->lock->read_lock(this->lock);
	if (rta_oif && !is_interface_up_and_usable(this, rta_oif))
	{	/* ignore route changes for interfaces that are ignored or down */
		this->lock->unlock(this->lock);
		DESTROY_IF(host);
		return;
	}
	if (!host && rta_oif)
	{
		host = get_interface_address(this, rta_oif, msg->rtm_family,
									 NULL, NULL);
	}
	if (!host || is_known_vip(this, host))
	{	/* ignore routes added for virtual IPs */
		this->lock->unlock(this->lock);
		DESTROY_IF(host);
		return;
	}
	this->lock->unlock(this->lock);
	fire_roam_event(this, FALSE);
	host->destroy(host);
}

/**
 * Receives events from kernel
 */
static bool receive_events(private_kernel_netlink_net_t *this, int fd,
						   watcher_event_t event)
{
	char response[1536];
	struct nlmsghdr *hdr = (struct nlmsghdr*)response;
	struct sockaddr_nl addr;
	socklen_t addr_len = sizeof(addr);
	int len;

	len = recvfrom(this->socket_events, response, sizeof(response),
				   MSG_DONTWAIT, (struct sockaddr*)&addr, &addr_len);
	if (len < 0)
	{
		switch (errno)
		{
			case EINTR:
				/* interrupted, try again */
				return TRUE;
			case EAGAIN:
				/* no data ready, select again */
				return TRUE;
			default:
				DBG1(DBG_KNL, "unable to receive from rt event socket");
				sleep(1);
				return TRUE;
		}
	}

	if (addr.nl_pid != 0)
	{	/* not from kernel. not interested, try another one */
		return TRUE;
	}

	while (NLMSG_OK(hdr, len))
	{
		/* looks good so far, dispatch netlink message */
		switch (hdr->nlmsg_type)
		{
			case RTM_NEWADDR:
			case RTM_DELADDR:
				process_addr(this, hdr, TRUE);
				break;
			case RTM_NEWLINK:
			case RTM_DELLINK:
				process_link(this, hdr, TRUE);
				break;
			case RTM_NEWROUTE:
			case RTM_DELROUTE:
				if (this->process_route)
				{
					process_route(this, hdr);
				}
				break;
			default:
				break;
		}
		hdr = NLMSG_NEXT(hdr, len);
	}
	return TRUE;
}

/** enumerator over addresses */
typedef struct {
	private_kernel_netlink_net_t* this;
	/** which addresses to enumerate */
	kernel_address_type_t which;
} address_enumerator_t;

/**
 * cleanup function for address enumerator
 */
static void address_enumerator_destroy(address_enumerator_t *data)
{
	data->this->lock->unlock(data->this->lock);
	free(data);
}

/**
 * filter for addresses
 */
static bool filter_addresses(address_enumerator_t *data,
							 addr_entry_t** in, host_t** out)
{
	if (!(data->which & ADDR_TYPE_VIRTUAL) && (*in)->refcount)
	{	/* skip virtual interfaces added by us */
		return FALSE;
	}
	if (!(data->which & ADDR_TYPE_REGULAR) && !(*in)->refcount)
	{	/* address is regular, but not requested */
		return FALSE;
	}
	if ((*in)->scope >= RT_SCOPE_LINK)
	{	/* skip addresses with a unusable scope */
		return FALSE;
	}
	*out = (*in)->ip;
	return TRUE;
}

/**
 * enumerator constructor for interfaces
 */
static enumerator_t *create_iface_enumerator(iface_entry_t *iface,
											 address_enumerator_t *data)
{
	return enumerator_create_filter(
				iface->addrs->create_enumerator(iface->addrs),
				(void*)filter_addresses, data, NULL);
}

/**
 * filter for interfaces
 */
static bool filter_interfaces(address_enumerator_t *data, iface_entry_t** in,
							  iface_entry_t** out)
{
	if (!(data->which & ADDR_TYPE_IGNORED) && !(*in)->usable)
	{	/* skip interfaces excluded by config */
		return FALSE;
	}
	if (!(data->which & ADDR_TYPE_LOOPBACK) && ((*in)->flags & IFF_LOOPBACK))
	{	/* ignore loopback devices */
		return FALSE;
	}
	if (!(data->which & ADDR_TYPE_DOWN) && !((*in)->flags & IFF_UP))
	{	/* skip interfaces not up */
		return FALSE;
	}
	*out = *in;
	return TRUE;
}

METHOD(kernel_net_t, create_address_enumerator, enumerator_t*,
	private_kernel_netlink_net_t *this, kernel_address_type_t which)
{
	address_enumerator_t *data;

	INIT(data,
		.this = this,
		.which = which,
	);

	this->lock->read_lock(this->lock);
	return enumerator_create_nested(
				enumerator_create_filter(
					this->ifaces->create_enumerator(this->ifaces),
					(void*)filter_interfaces, data, NULL),
				(void*)create_iface_enumerator, data,
				(void*)address_enumerator_destroy);
}

METHOD(kernel_net_t, get_interface_name, bool,
	private_kernel_netlink_net_t *this, host_t* ip, char **name)
{
	addr_map_entry_t *entry, lookup = {
		.ip = ip,
	};

	if (ip->is_anyaddr(ip))
	{
		return FALSE;
	}
	this->lock->read_lock(this->lock);
	/* first try to find it on an up and usable interface */
	entry = this->addrs->get_match(this->addrs, &lookup,
								  (void*)addr_map_entry_match_up_and_usable);
	if (entry)
	{
		if (name)
		{
			*name = strdup(entry->iface->ifname);
			DBG2(DBG_KNL, "%H is on interface %s", ip, *name);
		}
		this->lock->unlock(this->lock);
		return TRUE;
	}
	/* in a second step, consider virtual IPs installed by us */
	entry = this->vips->get_match(this->vips, &lookup,
								  (void*)addr_map_entry_match_up_and_usable);
	if (entry)
	{
		if (name)
		{
			*name = strdup(entry->iface->ifname);
			DBG2(DBG_KNL, "virtual IP %H is on interface %s", ip, *name);
		}
		this->lock->unlock(this->lock);
		return TRUE;
	}
	/* maybe it is installed on an ignored interface */
	entry = this->addrs->get_match(this->addrs, &lookup,
								  (void*)addr_map_entry_match_up);
	if (!entry)
	{
		DBG2(DBG_KNL, "%H is not a local address or the interface is down", ip);
	}
	this->lock->unlock(this->lock);
	return FALSE;
}

/**
 * get the index of an interface by name
 */
static int get_interface_index(private_kernel_netlink_net_t *this, char* name)
{
	iface_entry_t *iface;
	int ifindex = 0;

	DBG2(DBG_KNL, "getting iface index for %s", name);

	this->lock->read_lock(this->lock);
	if (this->ifaces->find_first(this->ifaces, (void*)iface_entry_by_name,
								(void**)&iface, name) == SUCCESS)
	{
		ifindex = iface->ifindex;
	}
	this->lock->unlock(this->lock);

	if (ifindex == 0)
	{
		DBG1(DBG_KNL, "unable to get interface index for %s", name);
	}
	return ifindex;
}

/**
 * check if an address or net (addr with prefix net bits) is in
 * subnet (net with net_len net bits)
 */
static bool addr_in_subnet(chunk_t addr, int prefix, chunk_t net, int net_len)
{
	static const u_char mask[] = { 0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe };
	int byte = 0;

	if (net_len == 0)
	{	/* any address matches a /0 network */
		return TRUE;
	}
	if (addr.len != net.len || net_len > 8 * net.len || prefix < net_len)
	{
		return FALSE;
	}
	/* scan through all bytes in network order */
	while (net_len > 0)
	{
		if (net_len < 8)
		{
			return (mask[net_len] & addr.ptr[byte]) == (mask[net_len] & net.ptr[byte]);
		}
		else
		{
			if (addr.ptr[byte] != net.ptr[byte])
			{
				return FALSE;
			}
			byte++;
			net_len -= 8;
		}
	}
	return TRUE;
}

/**
 * Store information about a route retrieved via RTNETLINK
 */
typedef struct {
	chunk_t gtw;
	chunk_t src;
	chunk_t dst;
	host_t *src_host;
	u_int8_t dst_len;
	u_int32_t table;
	u_int32_t oif;
} rt_entry_t;

/**
 * Free a route entry
 */
static void rt_entry_destroy(rt_entry_t *this)
{
	DESTROY_IF(this->src_host);
	free(this);
}

/**
 * Parse route received with RTM_NEWROUTE. The given rt_entry_t object will be
 * reused if not NULL.
 *
 * Returned chunks point to internal data of the Netlink message.
 */
static rt_entry_t *parse_route(struct nlmsghdr *hdr, rt_entry_t *route)
{
	struct rtattr *rta;
	struct rtmsg *msg;
	size_t rtasize;

	msg = NLMSG_DATA(hdr);
	rta = RTM_RTA(msg);
	rtasize = RTM_PAYLOAD(hdr);

	if (route)
	{
		route->gtw = chunk_empty;
		route->src = chunk_empty;
		route->dst = chunk_empty;
		route->dst_len = msg->rtm_dst_len;
		route->table = msg->rtm_table;
		route->oif = 0;
	}
	else
	{
		INIT(route,
			.dst_len = msg->rtm_dst_len,
			.table = msg->rtm_table,
		);
	}

	while (RTA_OK(rta, rtasize))
	{
		switch (rta->rta_type)
		{
			case RTA_PREFSRC:
				route->src = chunk_create(RTA_DATA(rta), RTA_PAYLOAD(rta));
				break;
			case RTA_GATEWAY:
				route->gtw = chunk_create(RTA_DATA(rta), RTA_PAYLOAD(rta));
				break;
			case RTA_DST:
				route->dst = chunk_create(RTA_DATA(rta), RTA_PAYLOAD(rta));
				break;
			case RTA_OIF:
				if (RTA_PAYLOAD(rta) == sizeof(route->oif))
				{
					route->oif = *(u_int32_t*)RTA_DATA(rta);
				}
				break;
#ifdef HAVE_RTA_TABLE
			case RTA_TABLE:
				if (RTA_PAYLOAD(rta) == sizeof(route->table))
				{
					route->table = *(u_int32_t*)RTA_DATA(rta);
				}
				break;
#endif /* HAVE_RTA_TABLE*/
		}
		rta = RTA_NEXT(rta, rtasize);
	}
	return route;
}

/**
 * Get a route: If "nexthop", the nexthop is returned. source addr otherwise.
 */
static host_t *get_route(private_kernel_netlink_net_t *this, host_t *dest,
						 int prefix, bool nexthop, host_t *candidate,
						 u_int recursion)
{
	netlink_buf_t request;
	struct nlmsghdr *hdr, *out, *current;
	struct rtmsg *msg;
	chunk_t chunk;
	size_t len;
	linked_list_t *routes;
	rt_entry_t *route = NULL, *best = NULL;
	enumerator_t *enumerator;
	host_t *addr = NULL;
	bool match_net;
	int family;

	if (recursion > MAX_ROUTE_RECURSION)
	{
		return NULL;
	}
	chunk = dest->get_address(dest);
	len = chunk.len * 8;
	prefix = prefix < 0 ? len : min(prefix, len);
	match_net = prefix != len;

	memset(&request, 0, sizeof(request));

	family = dest->get_family(dest);
	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST;
	if (family == AF_INET || this->rta_prefsrc_for_ipv6 ||
		this->routing_table || match_net)
	{	/* kernels prior to 3.0 do not support RTA_PREFSRC for IPv6 routes.
		 * as we want to ignore routes with virtual IPs we cannot use DUMP
		 * if these routes are not installed in a separate table */
		hdr->nlmsg_flags |= NLM_F_DUMP;
	}
	hdr->nlmsg_type = RTM_GETROUTE;
	hdr->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));

	msg = NLMSG_DATA(hdr);
	msg->rtm_family = family;
	if (candidate)
	{
		chunk = candidate->get_address(candidate);
		netlink_add_attribute(hdr, RTA_PREFSRC, chunk, sizeof(request));
	}
	if (!match_net)
	{
		chunk = dest->get_address(dest);
		netlink_add_attribute(hdr, RTA_DST, chunk, sizeof(request));
	}

	if (this->socket->send(this->socket, hdr, &out, &len) != SUCCESS)
	{
		DBG2(DBG_KNL, "getting %s to reach %H/%d failed",
			 nexthop ? "nexthop" : "address", dest, prefix);
		return NULL;
	}
	routes = linked_list_create();
	this->lock->read_lock(this->lock);

	for (current = out; NLMSG_OK(current, len);
		 current = NLMSG_NEXT(current, len))
	{
		switch (current->nlmsg_type)
		{
			case NLMSG_DONE:
				break;
			case RTM_NEWROUTE:
			{
				rt_entry_t *other;
				uintptr_t table;

				route = parse_route(current, route);

				table = (uintptr_t)route->table;
				if (this->rt_exclude->find_first(this->rt_exclude, NULL,
												 (void**)&table) == SUCCESS)
				{	/* route is from an excluded routing table */
					continue;
				}
				if (this->routing_table != 0 &&
					route->table == this->routing_table)
				{	/* route is from our own ipsec routing table */
					continue;
				}
				if (route->oif && !is_interface_up_and_usable(this, route->oif))
				{	/* interface is down */
					continue;
				}
				if (!addr_in_subnet(chunk, prefix, route->dst, route->dst_len))
				{	/* route destination does not contain dest */
					continue;
				}
				if (route->src.ptr)
				{	/* verify source address, if any */
					host_t *src = host_create_from_chunk(msg->rtm_family,
														 route->src, 0);
					if (src && is_known_vip(this, src))
					{	/* ignore routes installed by us */
						src->destroy(src);
						continue;
					}
					route->src_host = src;
				}
				/* insert route, sorted by decreasing network prefix */
				enumerator = routes->create_enumerator(routes);
				while (enumerator->enumerate(enumerator, &other))
				{
					if (route->dst_len > other->dst_len)
					{
						break;
					}
				}
				routes->insert_before(routes, enumerator, route);
				enumerator->destroy(enumerator);
				route = NULL;
				continue;
			}
			default:
				continue;
		}
		break;
	}
	if (route)
	{
		rt_entry_destroy(route);
	}

	/* now we have a list of routes matching dest, sorted by net prefix.
	 * we will look for source addresses for these routes and select the one
	 * with the preferred source address, if possible */
	enumerator = routes->create_enumerator(routes);
	while (enumerator->enumerate(enumerator, &route))
	{
		if (route->src_host)
		{	/* got a source address with the route, if no preferred source
			 * is given or it matches we are done, as this is the best route */
			if (!candidate || candidate->ip_equals(candidate, route->src_host))
			{
				best = route;
				break;
			}
			else if (route->oif)
			{	/* no match yet, maybe it is assigned to the same interface */
				host_t *src = get_interface_address(this, route->oif,
											msg->rtm_family, dest, candidate);
				if (src && src->ip_equals(src, candidate))
				{
					route->src_host->destroy(route->src_host);
					route->src_host = src;
					best = route;
					break;
				}
				DESTROY_IF(src);
			}
			/* no luck yet with the source address. if this is the best (first)
			 * route we store it as fallback in case we don't find a route with
			 * the preferred source */
			best = best ?: route;
			continue;
		}
		if (route->oif)
		{	/* no src, but an interface - get address from it */
			route->src_host = get_interface_address(this, route->oif,
											msg->rtm_family, dest, candidate);
			if (route->src_host)
			{	/* we handle this address the same as the one above */
				if (!candidate ||
					 candidate->ip_equals(candidate, route->src_host))
				{
					best = route;
					break;
				}
				best = best ?: route;
				continue;
			}
		}
		if (route->gtw.ptr)
		{	/* no src, no iface, but a gateway - lookup src to reach gtw */
			host_t *gtw;

			gtw = host_create_from_chunk(msg->rtm_family, route->gtw, 0);
			if (gtw && !gtw->ip_equals(gtw, dest))
			{
				route->src_host = get_route(this, gtw, -1, FALSE, candidate,
											recursion + 1);
			}
			DESTROY_IF(gtw);
			if (route->src_host)
			{	/* more of the same */
				if (!candidate ||
					 candidate->ip_equals(candidate, route->src_host))
				{
					best = route;
					break;
				}
				best = best ?: route;
			}
		}
	}
	enumerator->destroy(enumerator);

	if (nexthop)
	{	/* nexthop lookup, return gateway if any */
		if (best || routes->get_first(routes, (void**)&best) == SUCCESS)
		{
			addr = host_create_from_chunk(msg->rtm_family, best->gtw, 0);
		}
		if (!addr && !match_net)
		{	/* fallback to destination address */
			addr = dest->clone(dest);
		}
	}
	else
	{
		if (best)
		{
			addr = best->src_host->clone(best->src_host);
		}
	}
	this->lock->unlock(this->lock);
	routes->destroy_function(routes, (void*)rt_entry_destroy);
	free(out);

	if (addr)
	{
		DBG2(DBG_KNL, "using %H as %s to reach %H/%d", addr,
			 nexthop ? "nexthop" : "address", dest, prefix);
	}
	else if (!recursion)
	{
		DBG2(DBG_KNL, "no %s found to reach %H/%d",
			 nexthop ? "nexthop" : "address", dest, prefix);
	}
	return addr;
}

METHOD(kernel_net_t, get_source_addr, host_t*,
	private_kernel_netlink_net_t *this, host_t *dest, host_t *src)
{
	return get_route(this, dest, -1, FALSE, src, 0);
}

METHOD(kernel_net_t, get_nexthop, host_t*,
	private_kernel_netlink_net_t *this, host_t *dest, int prefix, host_t *src)
{
	return get_route(this, dest, prefix, TRUE, src, 0);
}

/**
 * Manages the creation and deletion of ip addresses on an interface.
 * By setting the appropriate nlmsg_type, the ip will be set or unset.
 */
static status_t manage_ipaddr(private_kernel_netlink_net_t *this, int nlmsg_type,
							  int flags, int if_index, host_t *ip, int prefix)
{
	netlink_buf_t request;
	struct nlmsghdr *hdr;
	struct ifaddrmsg *msg;
	chunk_t chunk;

	memset(&request, 0, sizeof(request));

	chunk = ip->get_address(ip);

	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | flags;
	hdr->nlmsg_type = nlmsg_type;
	hdr->nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg));

	msg = NLMSG_DATA(hdr);
	msg->ifa_family = ip->get_family(ip);
	msg->ifa_flags = 0;
	msg->ifa_prefixlen = prefix < 0 ? chunk.len * 8 : prefix;
	msg->ifa_scope = RT_SCOPE_UNIVERSE;
	msg->ifa_index = if_index;

	netlink_add_attribute(hdr, IFA_LOCAL, chunk, sizeof(request));

	if (ip->get_family(ip) == AF_INET6 && this->rta_prefsrc_for_ipv6)
	{	/* if source routes are possible we let the virtual IP get deprecated
		 * immediately (but mark it as valid forever) so it gets only used if
		 * forced by our route, and not by the default IPv6 address selection */
		struct ifa_cacheinfo cache = {
			.ifa_valid = 0xFFFFFFFF,
			.ifa_prefered = 0,
		};
		netlink_add_attribute(hdr, IFA_CACHEINFO, chunk_from_thing(cache),
							  sizeof(request));
	}
	return this->socket->send_ack(this->socket, hdr);
}

METHOD(kernel_net_t, add_ip, status_t,
	private_kernel_netlink_net_t *this, host_t *virtual_ip, int prefix,
	char *iface_name)
{
	addr_map_entry_t *entry, lookup = {
		.ip = virtual_ip,
	};
	iface_entry_t *iface = NULL;

	if (!this->install_virtual_ip)
	{	/* disabled by config */
		return SUCCESS;
	}

	this->lock->write_lock(this->lock);
	/* the virtual IP might actually be installed as regular IP, in which case
	 * we don't track it as virtual IP */
	entry = this->addrs->get_match(this->addrs, &lookup,
								  (void*)addr_map_entry_match);
	if (!entry)
	{	/* otherwise it might already be installed as virtual IP */
		entry = this->vips->get_match(this->vips, &lookup,
									 (void*)addr_map_entry_match);
		if (entry)
		{	/* the vip we found can be in one of three states: 1) installed and
			 * ready, 2) just added by another thread, but not yet confirmed to
			 * be installed by the kernel, 3) just deleted, but not yet gone.
			 * Then while we wait below, several things could happen (as we
			 * release the lock).  For instance, the interface could disappear,
			 * or the IP is finally deleted, and it reappears on a different
			 * interface. All these cases are handled by the call below. */
			while (!is_vip_installed_or_gone(this, virtual_ip, &entry))
			{
				this->condvar->wait(this->condvar, this->lock);
			}
			if (entry)
			{
				entry->addr->refcount++;
			}
		}
	}
	if (entry)
	{
		DBG2(DBG_KNL, "virtual IP %H is already installed on %s", virtual_ip,
			 entry->iface->ifname);
		this->lock->unlock(this->lock);
		return SUCCESS;
	}
	/* try to find the target interface, either by config or via src ip */
	if (!this->install_virtual_ip_on ||
		 this->ifaces->find_first(this->ifaces, (void*)iface_entry_by_name,
						(void**)&iface, this->install_virtual_ip_on) != SUCCESS)
	{
		if (this->ifaces->find_first(this->ifaces, (void*)iface_entry_by_name,
									 (void**)&iface, iface_name) != SUCCESS)
		{	/* if we don't find the requested interface we just use the first */
			this->ifaces->get_first(this->ifaces, (void**)&iface);
		}
	}
	if (iface)
	{
		addr_entry_t *addr;

		INIT(addr,
			.ip = virtual_ip->clone(virtual_ip),
			.refcount = 1,
			.scope = RT_SCOPE_UNIVERSE,
		);
		iface->addrs->insert_last(iface->addrs, addr);
		addr_map_entry_add(this->vips, addr, iface);
		if (manage_ipaddr(this, RTM_NEWADDR, NLM_F_CREATE | NLM_F_EXCL,
						  iface->ifindex, virtual_ip, prefix) == SUCCESS)
		{
			while (!is_vip_installed_or_gone(this, virtual_ip, &entry))
			{	/* wait until address appears */
				this->condvar->wait(this->condvar, this->lock);
			}
			if (entry)
			{	/* we fail if the interface got deleted in the meantime */
				DBG2(DBG_KNL, "virtual IP %H installed on %s", virtual_ip,
					 entry->iface->ifname);
				this->lock->unlock(this->lock);
				/* during IKEv1 reauthentication, children get moved from
				 * old the new SA before the virtual IP is available. This
				 * kills the route for our virtual IP, reinstall. */
				queue_route_reinstall(this, strdup(entry->iface->ifname));
				return SUCCESS;
			}
		}
		this->lock->unlock(this->lock);
		DBG1(DBG_KNL, "adding virtual IP %H failed", virtual_ip);
		return FAILED;
	}
	this->lock->unlock(this->lock);
	DBG1(DBG_KNL, "no interface available, unable to install virtual IP %H",
		 virtual_ip);
	return FAILED;
}

METHOD(kernel_net_t, del_ip, status_t,
	private_kernel_netlink_net_t *this, host_t *virtual_ip, int prefix,
	bool wait)
{
	addr_map_entry_t *entry, lookup = {
		.ip = virtual_ip,
	};

	if (!this->install_virtual_ip)
	{	/* disabled by config */
		return SUCCESS;
	}

	DBG2(DBG_KNL, "deleting virtual IP %H", virtual_ip);

	this->lock->write_lock(this->lock);
	entry = this->vips->get_match(this->vips, &lookup,
								 (void*)addr_map_entry_match);
	if (!entry)
	{	/* we didn't install this IP as virtual IP */
		entry = this->addrs->get_match(this->addrs, &lookup,
									  (void*)addr_map_entry_match);
		if (entry)
		{
			DBG2(DBG_KNL, "not deleting existing IP %H on %s", virtual_ip,
				 entry->iface->ifname);
			this->lock->unlock(this->lock);
			return SUCCESS;
		}
		DBG2(DBG_KNL, "virtual IP %H not cached, unable to delete", virtual_ip);
		this->lock->unlock(this->lock);
		return FAILED;
	}
	if (entry->addr->refcount == 1)
	{
		status_t status;

		/* we set this flag so that threads calling add_ip will block and wait
		 * until the entry is gone, also so we can wait below */
		entry->addr->installed = FALSE;
		status = manage_ipaddr(this, RTM_DELADDR, 0, entry->iface->ifindex,
							   virtual_ip, prefix);
		if (status == SUCCESS && wait)
		{	/* wait until the address is really gone */
			while (is_known_vip(this, virtual_ip))
			{
				this->condvar->wait(this->condvar, this->lock);
			}
		}
		this->lock->unlock(this->lock);
		return status;
	}
	else
	{
		entry->addr->refcount--;
	}
	DBG2(DBG_KNL, "virtual IP %H used by other SAs, not deleting",
		 virtual_ip);
	this->lock->unlock(this->lock);
	return SUCCESS;
}

/**
 * Manages source routes in the routing table.
 * By setting the appropriate nlmsg_type, the route gets added or removed.
 */
static status_t manage_srcroute(private_kernel_netlink_net_t *this,
								int nlmsg_type, int flags, chunk_t dst_net,
								u_int8_t prefixlen, host_t *gateway,
								host_t *src_ip, char *if_name)
{
	netlink_buf_t request;
	struct nlmsghdr *hdr;
	struct rtmsg *msg;
	struct rtattr *rta;
	int ifindex;
	chunk_t chunk;

	/* if route is 0.0.0.0/0, we can't install it, as it would
	 * overwrite the default route. Instead, we add two routes:
	 * 0.0.0.0/1 and 128.0.0.0/1 */
	if (this->routing_table == 0 && prefixlen == 0)
	{
		chunk_t half_net;
		u_int8_t half_prefixlen;
		status_t status;

		half_net = chunk_alloca(dst_net.len);
		memset(half_net.ptr, 0, half_net.len);
		half_prefixlen = 1;

		status = manage_srcroute(this, nlmsg_type, flags, half_net, half_prefixlen,
					gateway, src_ip, if_name);
		half_net.ptr[0] |= 0x80;
		status = manage_srcroute(this, nlmsg_type, flags, half_net, half_prefixlen,
					gateway, src_ip, if_name);
		return status;
	}

	memset(&request, 0, sizeof(request));

	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK | flags;
	hdr->nlmsg_type = nlmsg_type;
	hdr->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));

	msg = NLMSG_DATA(hdr);
	msg->rtm_family = src_ip->get_family(src_ip);
	msg->rtm_dst_len = prefixlen;
	msg->rtm_table = this->routing_table;
	msg->rtm_protocol = RTPROT_STATIC;
	msg->rtm_type = RTN_UNICAST;
	msg->rtm_scope = RT_SCOPE_UNIVERSE;

	netlink_add_attribute(hdr, RTA_DST, dst_net, sizeof(request));
	chunk = src_ip->get_address(src_ip);
	netlink_add_attribute(hdr, RTA_PREFSRC, chunk, sizeof(request));
	if (gateway && gateway->get_family(gateway) == src_ip->get_family(src_ip))
	{
		chunk = gateway->get_address(gateway);
		netlink_add_attribute(hdr, RTA_GATEWAY, chunk, sizeof(request));
	}
	ifindex = get_interface_index(this, if_name);
	chunk.ptr = (char*)&ifindex;
	chunk.len = sizeof(ifindex);
	netlink_add_attribute(hdr, RTA_OIF, chunk, sizeof(request));

	if (this->mtu || this->mss)
	{
		chunk = chunk_alloca(RTA_LENGTH((sizeof(struct rtattr) +
										 sizeof(u_int32_t)) * 2));
		chunk.len = 0;
		rta = (struct rtattr*)chunk.ptr;
		if (this->mtu)
		{
			rta->rta_type = RTAX_MTU;
			rta->rta_len = RTA_LENGTH(sizeof(u_int32_t));
			memcpy(RTA_DATA(rta), &this->mtu, sizeof(u_int32_t));
			chunk.len = rta->rta_len;
		}
		if (this->mss)
		{
			rta = (struct rtattr*)(chunk.ptr + RTA_ALIGN(chunk.len));
			rta->rta_type = RTAX_ADVMSS;
			rta->rta_len = RTA_LENGTH(sizeof(u_int32_t));
			memcpy(RTA_DATA(rta), &this->mss, sizeof(u_int32_t));
			chunk.len = RTA_ALIGN(chunk.len) + rta->rta_len;
		}
		netlink_add_attribute(hdr, RTA_METRICS, chunk, sizeof(request));
	}

	return this->socket->send_ack(this->socket, hdr);
}

METHOD(kernel_net_t, add_route, status_t,
	private_kernel_netlink_net_t *this, chunk_t dst_net, u_int8_t prefixlen,
	host_t *gateway, host_t *src_ip, char *if_name)
{
	status_t status;
	route_entry_t *found, route = {
		.dst_net = dst_net,
		.prefixlen = prefixlen,
		.gateway = gateway,
		.src_ip = src_ip,
		.if_name = if_name,
	};

	this->routes_lock->lock(this->routes_lock);
	found = this->routes->get(this->routes, &route);
	if (found)
	{
		this->routes_lock->unlock(this->routes_lock);
		return ALREADY_DONE;
	}
	status = manage_srcroute(this, RTM_NEWROUTE, NLM_F_CREATE | NLM_F_EXCL,
							 dst_net, prefixlen, gateway, src_ip, if_name);
	if (status == SUCCESS)
	{
		found = route_entry_clone(&route);
		this->routes->put(this->routes, found, found);
	}
	this->routes_lock->unlock(this->routes_lock);
	return status;
}

METHOD(kernel_net_t, del_route, status_t,
	private_kernel_netlink_net_t *this, chunk_t dst_net, u_int8_t prefixlen,
	host_t *gateway, host_t *src_ip, char *if_name)
{
	status_t status;
	route_entry_t *found, route = {
		.dst_net = dst_net,
		.prefixlen = prefixlen,
		.gateway = gateway,
		.src_ip = src_ip,
		.if_name = if_name,
	};

	this->routes_lock->lock(this->routes_lock);
	found = this->routes->get(this->routes, &route);
	if (!found)
	{
		this->routes_lock->unlock(this->routes_lock);
		return NOT_FOUND;
	}
	this->routes->remove(this->routes, found);
	route_entry_destroy(found);
	status = manage_srcroute(this, RTM_DELROUTE, 0, dst_net, prefixlen,
							 gateway, src_ip, if_name);
	this->routes_lock->unlock(this->routes_lock);
	return status;
}

/**
 * Initialize a list of local addresses.
 */
static status_t init_address_list(private_kernel_netlink_net_t *this)
{
	netlink_buf_t request;
	struct nlmsghdr *out, *current, *in;
	struct rtgenmsg *msg;
	size_t len;
	enumerator_t *ifaces, *addrs;
	iface_entry_t *iface;
	addr_entry_t *addr;

	DBG2(DBG_KNL, "known interfaces and IP addresses:");

	memset(&request, 0, sizeof(request));

	in = &request.hdr;
	in->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtgenmsg));
	in->nlmsg_flags = NLM_F_REQUEST | NLM_F_MATCH | NLM_F_ROOT;
	msg = NLMSG_DATA(in);
	msg->rtgen_family = AF_UNSPEC;

	/* get all links */
	in->nlmsg_type = RTM_GETLINK;
	if (this->socket->send(this->socket, in, &out, &len) != SUCCESS)
	{
		return FAILED;
	}
	current = out;
	while (NLMSG_OK(current, len))
	{
		switch (current->nlmsg_type)
		{
			case NLMSG_DONE:
				break;
			case RTM_NEWLINK:
				process_link(this, current, FALSE);
				/* fall through */
			default:
				current = NLMSG_NEXT(current, len);
				continue;
		}
		break;
	}
	free(out);

	/* get all interface addresses */
	in->nlmsg_type = RTM_GETADDR;
	if (this->socket->send(this->socket, in, &out, &len) != SUCCESS)
	{
		return FAILED;
	}
	current = out;
	while (NLMSG_OK(current, len))
	{
		switch (current->nlmsg_type)
		{
			case NLMSG_DONE:
				break;
			case RTM_NEWADDR:
				process_addr(this, current, FALSE);
				/* fall through */
			default:
				current = NLMSG_NEXT(current, len);
				continue;
		}
		break;
	}
	free(out);

	this->lock->read_lock(this->lock);
	ifaces = this->ifaces->create_enumerator(this->ifaces);
	while (ifaces->enumerate(ifaces, &iface))
	{
		if (iface_entry_up_and_usable(iface))
		{
			DBG2(DBG_KNL, "  %s", iface->ifname);
			addrs = iface->addrs->create_enumerator(iface->addrs);
			while (addrs->enumerate(addrs, (void**)&addr))
			{
				DBG2(DBG_KNL, "    %H", addr->ip);
			}
			addrs->destroy(addrs);
		}
	}
	ifaces->destroy(ifaces);
	this->lock->unlock(this->lock);
	return SUCCESS;
}

/**
 * create or delete a rule to use our routing table
 */
static status_t manage_rule(private_kernel_netlink_net_t *this, int nlmsg_type,
							int family, u_int32_t table, u_int32_t prio)
{
	netlink_buf_t request;
	struct nlmsghdr *hdr;
	struct rtmsg *msg;
	chunk_t chunk;
	char *fwmark;

	memset(&request, 0, sizeof(request));
	hdr = &request.hdr;
	hdr->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
	hdr->nlmsg_type = nlmsg_type;
	if (nlmsg_type == RTM_NEWRULE)
	{
		hdr->nlmsg_flags |= NLM_F_CREATE | NLM_F_EXCL;
	}
	hdr->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));

	msg = NLMSG_DATA(hdr);
	msg->rtm_table = table;
	msg->rtm_family = family;
	msg->rtm_protocol = RTPROT_BOOT;
	msg->rtm_scope = RT_SCOPE_UNIVERSE;
	msg->rtm_type = RTN_UNICAST;

	chunk = chunk_from_thing(prio);
	netlink_add_attribute(hdr, RTA_PRIORITY, chunk, sizeof(request));

	fwmark = lib->settings->get_str(lib->settings,
							"%s.plugins.kernel-netlink.fwmark", NULL, lib->ns);
	if (fwmark)
	{
#ifdef HAVE_LINUX_FIB_RULES_H
		mark_t mark;

		if (fwmark[0] == '!')
		{
			msg->rtm_flags |= FIB_RULE_INVERT;
			fwmark++;
		}
		if (mark_from_string(fwmark, &mark))
		{
			chunk = chunk_from_thing(mark.value);
			netlink_add_attribute(hdr, FRA_FWMARK, chunk, sizeof(request));
			chunk = chunk_from_thing(mark.mask);
			netlink_add_attribute(hdr, FRA_FWMASK, chunk, sizeof(request));
		}
#else
		DBG1(DBG_KNL, "setting firewall mark on routing rule is not supported");
#endif
	}
	return this->socket->send_ack(this->socket, hdr);
}

/**
 * check for kernel features (currently only via version number)
 */
static void check_kernel_features(private_kernel_netlink_net_t *this)
{
	struct utsname utsname;
	int a, b, c;

	if (uname(&utsname) == 0)
	{
		switch(sscanf(utsname.release, "%d.%d.%d", &a, &b, &c))
		{
			case 3:
				if (a == 2)
				{
					DBG2(DBG_KNL, "detected Linux %d.%d.%d, no support for "
						 "RTA_PREFSRC for IPv6 routes", a, b, c);
					break;
				}
				/* fall-through */
			case 2:
				/* only 3.x+ uses two part version numbers */
				this->rta_prefsrc_for_ipv6 = TRUE;
				break;
			default:
				break;
		}
	}
}

/**
 * Destroy an address to iface map
 */
static void addr_map_destroy(hashtable_t *map)
{
	enumerator_t *enumerator;
	addr_map_entry_t *addr;

	enumerator = map->create_enumerator(map);
	while (enumerator->enumerate(enumerator, NULL, (void**)&addr))
	{
		free(addr);
	}
	enumerator->destroy(enumerator);
	map->destroy(map);
}

METHOD(kernel_net_t, destroy, void,
	private_kernel_netlink_net_t *this)
{
	enumerator_t *enumerator;
	route_entry_t *route;

	if (this->routing_table)
	{
		manage_rule(this, RTM_DELRULE, AF_INET, this->routing_table,
					this->routing_table_prio);
		manage_rule(this, RTM_DELRULE, AF_INET6, this->routing_table,
					this->routing_table_prio);
	}
	if (this->socket_events > 0)
	{
		lib->watcher->remove(lib->watcher, this->socket_events);
		close(this->socket_events);
	}
	enumerator = this->routes->create_enumerator(this->routes);
	while (enumerator->enumerate(enumerator, NULL, (void**)&route))
	{
		manage_srcroute(this, RTM_DELROUTE, 0, route->dst_net, route->prefixlen,
						route->gateway, route->src_ip, route->if_name);
		route_entry_destroy(route);
	}
	enumerator->destroy(enumerator);
	this->routes->destroy(this->routes);
	this->routes_lock->destroy(this->routes_lock);
	DESTROY_IF(this->socket);

	net_changes_clear(this);
	this->net_changes->destroy(this->net_changes);
	this->net_changes_lock->destroy(this->net_changes_lock);

	addr_map_destroy(this->addrs);
	addr_map_destroy(this->vips);

	this->ifaces->destroy_function(this->ifaces, (void*)iface_entry_destroy);
	this->rt_exclude->destroy(this->rt_exclude);
	this->roam_lock->destroy(this->roam_lock);
	this->condvar->destroy(this->condvar);
	this->lock->destroy(this->lock);
	free(this);
}

/*
 * Described in header.
 */
kernel_netlink_net_t *kernel_netlink_net_create()
{
	private_kernel_netlink_net_t *this;
	enumerator_t *enumerator;
	bool register_for_events = TRUE;
	char *exclude;

	INIT(this,
		.public = {
			.interface = {
				.get_interface = _get_interface_name,
				.create_address_enumerator = _create_address_enumerator,
				.get_source_addr = _get_source_addr,
				.get_nexthop = _get_nexthop,
				.add_ip = _add_ip,
				.del_ip = _del_ip,
				.add_route = _add_route,
				.del_route = _del_route,
				.destroy = _destroy,
			},
		},
		.socket = netlink_socket_create(NETLINK_ROUTE, rt_msg_names),
		.rt_exclude = linked_list_create(),
		.routes = hashtable_create((hashtable_hash_t)route_entry_hash,
								   (hashtable_equals_t)route_entry_equals, 16),
		.net_changes = hashtable_create(
								   (hashtable_hash_t)net_change_hash,
								   (hashtable_equals_t)net_change_equals, 16),
		.addrs = hashtable_create(
								(hashtable_hash_t)addr_map_entry_hash,
								(hashtable_equals_t)addr_map_entry_equals, 16),
		.vips = hashtable_create((hashtable_hash_t)addr_map_entry_hash,
								 (hashtable_equals_t)addr_map_entry_equals, 16),
		.routes_lock = mutex_create(MUTEX_TYPE_DEFAULT),
		.net_changes_lock = mutex_create(MUTEX_TYPE_DEFAULT),
		.ifaces = linked_list_create(),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
		.condvar = rwlock_condvar_create(),
		.roam_lock = spinlock_create(),
		.routing_table = lib->settings->get_int(lib->settings,
						"%s.routing_table", ROUTING_TABLE, lib->ns),
		.routing_table_prio = lib->settings->get_int(lib->settings,
						"%s.routing_table_prio", ROUTING_TABLE_PRIO, lib->ns),
		.process_route = lib->settings->get_bool(lib->settings,
						"%s.process_route", TRUE, lib->ns),
		.install_virtual_ip = lib->settings->get_bool(lib->settings,
						"%s.install_virtual_ip", TRUE, lib->ns),
		.install_virtual_ip_on = lib->settings->get_str(lib->settings,
						"%s.install_virtual_ip_on", NULL, lib->ns),
		.prefer_temporary_addrs = lib->settings->get_bool(lib->settings,
						"%s.prefer_temporary_addrs", FALSE, lib->ns),
		.roam_events = lib->settings->get_bool(lib->settings,
						"%s.plugins.kernel-netlink.roam_events", TRUE, lib->ns),
		.mtu = lib->settings->get_int(lib->settings,
						"%s.plugins.kernel-netlink.mtu", 0, lib->ns),
		.mss = lib->settings->get_int(lib->settings,
						"%s.plugins.kernel-netlink.mss", 0, lib->ns),
	);
	timerclear(&this->last_route_reinstall);
	timerclear(&this->next_roam);

	check_kernel_features(this);

	if (streq(lib->ns, "starter"))
	{	/* starter has no threads, so we do not register for kernel events */
		register_for_events = FALSE;
	}

	exclude = lib->settings->get_str(lib->settings,
									 "%s.ignore_routing_tables", NULL, lib->ns);
	if (exclude)
	{
		char *token;
		uintptr_t table;

		enumerator = enumerator_create_token(exclude, " ", " ");
		while (enumerator->enumerate(enumerator, &token))
		{
			errno = 0;
			table = strtoul(token, NULL, 10);

			if (errno == 0)
			{
				this->rt_exclude->insert_last(this->rt_exclude, (void*)table);
			}
		}
		enumerator->destroy(enumerator);
	}

	if (register_for_events)
	{
		struct sockaddr_nl addr;

		memset(&addr, 0, sizeof(addr));
		addr.nl_family = AF_NETLINK;

		/* create and bind RT socket for events (address/interface/route changes) */
		this->socket_events = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
		if (this->socket_events < 0)
		{
			DBG1(DBG_KNL, "unable to create RT event socket");
			destroy(this);
			return NULL;
		}
		addr.nl_groups = RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR |
						 RTMGRP_IPV4_ROUTE | RTMGRP_IPV6_ROUTE | RTMGRP_LINK;
		if (bind(this->socket_events, (struct sockaddr*)&addr, sizeof(addr)))
		{
			DBG1(DBG_KNL, "unable to bind RT event socket");
			destroy(this);
			return NULL;
		}

		lib->watcher->add(lib->watcher, this->socket_events, WATCHER_READ,
						  (watcher_cb_t)receive_events, this);
	}

	if (init_address_list(this) != SUCCESS)
	{
		DBG1(DBG_KNL, "unable to get interface list");
		destroy(this);
		return NULL;
	}

	if (this->routing_table)
	{
		if (manage_rule(this, RTM_NEWRULE, AF_INET, this->routing_table,
						this->routing_table_prio) != SUCCESS)
		{
			DBG1(DBG_KNL, "unable to create IPv4 routing table rule");
		}
		if (manage_rule(this, RTM_NEWRULE, AF_INET6, this->routing_table,
						this->routing_table_prio) != SUCCESS)
		{
			DBG1(DBG_KNL, "unable to create IPv6 routing table rule");
		}
	}

	return &this->public;
}
