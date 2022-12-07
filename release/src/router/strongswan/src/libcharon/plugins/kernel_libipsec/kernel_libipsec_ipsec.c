/*
 * Copyright (C) 2012-2013 Tobias Brunner
 *
 * Copyright (C) secunet Security Networks AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.  *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include "kernel_libipsec_ipsec.h"
#include "kernel_libipsec_router.h"

#include <library.h>
#include <ipsec.h>
#include <daemon.h>
#include <networking/tun_device.h>
#include <threading/mutex.h>
#include <utils/debug.h>

typedef struct private_kernel_libipsec_ipsec_t private_kernel_libipsec_ipsec_t;

struct private_kernel_libipsec_ipsec_t {

	/**
	 * Public libipsec_ipsec interface
	 */
	kernel_libipsec_ipsec_t public;

	/**
	 * Listener for lifetime expire events
	 */
	ipsec_event_listener_t ipsec_listener;

	/**
	 * Mutex to lock access to various lists
	 */
	mutex_t *mutex;

	/**
	 * List of installed policies (policy_entry_t)
	 */
	linked_list_t *policies;

	/**
	 * List of exclude routes (exclude_route_t)
	 */
	linked_list_t *excludes;

	/**
	 * Whether the remote TS may equal the IKE peer
	 */
	bool allow_peer_ts;
};

typedef struct exclude_route_t exclude_route_t;

/**
 * Exclude route definition
 */
struct exclude_route_t {
	/** Destination address to exclude */
	host_t *dst;
	/** Source address for route */
	host_t *src;
	/** Nexthop exclude has been installed */
	host_t *gtw;
	/** References to this route */
	int refs;
};

/**
 * Clean up an exclude route entry
 */
static void exclude_route_destroy(exclude_route_t *this)
{
	this->dst->destroy(this->dst);
	this->src->destroy(this->src);
	this->gtw->destroy(this->gtw);
	free(this);
}

CALLBACK(exclude_route_match, bool,
	exclude_route_t *current, va_list args)
{
	host_t *dst;

	VA_ARGS_VGET(args, dst);
	return dst->ip_equals(dst, current->dst);
}

typedef struct route_entry_t route_entry_t;

/**
 * Installed routing entry
 */
struct route_entry_t {
	/** Name of the interface the route is bound to */
	char *if_name;
	/** Source IP of the route */
	host_t *src_ip;
	/** Gateway of the route */
	host_t *gateway;
	/** Destination net */
	chunk_t dst_net;
	/** Destination net prefixlen */
	uint8_t prefixlen;
	/** Reference to exclude route, if any */
	exclude_route_t *exclude;
};

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
 * Compare two route_entry_t objects
 */
static bool route_entry_equals(route_entry_t *a, route_entry_t *b)
{
	if ((!a->src_ip && !b->src_ip) || (a->src_ip && b->src_ip &&
		  a->src_ip->ip_equals(a->src_ip, b->src_ip)))
	{
		if ((!a->gateway && !b->gateway) || (a->gateway && b->gateway &&
			  a->gateway->ip_equals(a->gateway, b->gateway)))
		{
			return a->if_name && b->if_name && streq(a->if_name, b->if_name) &&
				   chunk_equals(a->dst_net, b->dst_net) &&
				   a->prefixlen == b->prefixlen;
		}
	}
	return FALSE;
}

typedef struct policy_entry_t policy_entry_t;

/**
 * Installed policy
 */
struct policy_entry_t {
	/** Direction of this policy: in, out, forward */
	uint8_t direction;
	/** Parameters of installed policy */
	struct {
		/** Subnet and port */
		host_t *net;
		/** Subnet mask */
		uint8_t mask;
		/** Protocol */
		uint8_t proto;
	} src, dst;
	/** Associated route installed for this policy */
	route_entry_t *route;
	/** References to this policy */
	int refs;
};

/**
 * Create a policy_entry_t object
 */
static policy_entry_t *create_policy_entry(traffic_selector_t *src_ts,
										   traffic_selector_t *dst_ts,
										   policy_dir_t dir)
{
	policy_entry_t *this;
	INIT(this,
		.direction = dir,
	);

	src_ts->to_subnet(src_ts, &this->src.net, &this->src.mask);
	dst_ts->to_subnet(dst_ts, &this->dst.net, &this->dst.mask);

	/* src or dest proto may be "any" (0), use more restrictive one */
	this->src.proto = max(src_ts->get_protocol(src_ts),
						  dst_ts->get_protocol(dst_ts));
	this->src.proto = this->src.proto ? this->src.proto : 0;
	this->dst.proto = this->src.proto;
	return this;
}

/**
 * Destroy a policy_entry_t object
 */
static void policy_entry_destroy(policy_entry_t *this)
{
	if (this->route)
	{
		route_entry_destroy(this->route);
	}
	DESTROY_IF(this->src.net);
	DESTROY_IF(this->dst.net);
	free(this);
}

CALLBACK(policy_entry_equals, bool,
	policy_entry_t *a, va_list args)
{
	policy_entry_t *b;

	VA_ARGS_VGET(args, b);
	return a->direction == b->direction &&
		   a->src.proto == b->src.proto &&
		   a->dst.proto == b->dst.proto &&
		   a->src.mask == b->src.mask &&
		   a->dst.mask == b->dst.mask &&
		   a->src.net->equals(a->src.net, b->src.net) &&
		   a->dst.net->equals(a->dst.net, b->dst.net);
}

/**
 * Expiration callback
 */
static void expire(uint8_t protocol, uint32_t spi, host_t *dst, bool hard)
{
	charon->kernel->expire(charon->kernel, protocol, spi, dst, hard);
}

METHOD(kernel_ipsec_t, get_features, kernel_feature_t,
	private_kernel_libipsec_ipsec_t *this)
{
	return KERNEL_REQUIRE_UDP_ENCAPSULATION | KERNEL_ESP_V3_TFC;
}

METHOD(kernel_ipsec_t, get_spi, status_t,
	private_kernel_libipsec_ipsec_t *this, host_t *src, host_t *dst,
	uint8_t protocol, uint32_t *spi)
{
	return ipsec->sas->get_spi(ipsec->sas, src, dst, protocol, spi);
}

METHOD(kernel_ipsec_t, get_cpi, status_t,
	private_kernel_libipsec_ipsec_t *this, host_t *src, host_t *dst,
	uint16_t *cpi)
{
	return NOT_SUPPORTED;
}

METHOD(kernel_ipsec_t, add_sa, status_t,
	private_kernel_libipsec_ipsec_t *this, kernel_ipsec_sa_id_t *id,
	kernel_ipsec_add_sa_t *data)
{
	return ipsec->sas->add_sa(ipsec->sas, id->src, id->dst, id->spi, id->proto,
					data->reqid, id->mark, data->tfc, data->lifetime,
					data->enc_alg, data->enc_key, data->int_alg, data->int_key,
					data->mode, data->ipcomp, data->cpi, data->initiator,
					data->encap, data->esn, data->inbound, data->update);
}

METHOD(kernel_ipsec_t, update_sa, status_t,
	private_kernel_libipsec_ipsec_t *this, kernel_ipsec_sa_id_t *id,
	kernel_ipsec_update_sa_t *data)
{
	return NOT_SUPPORTED;
}

METHOD(kernel_ipsec_t, query_sa, status_t,
	private_kernel_libipsec_ipsec_t *this, kernel_ipsec_sa_id_t *id,
	kernel_ipsec_query_sa_t *data, uint64_t *bytes, uint64_t *packets,
	time_t *time)
{
	return ipsec->sas->query_sa(ipsec->sas, id->src, id->dst, id->spi,
								id->proto, id->mark, bytes, packets, time);
}

METHOD(kernel_ipsec_t, del_sa, status_t,
	private_kernel_libipsec_ipsec_t *this, kernel_ipsec_sa_id_t *id,
	kernel_ipsec_del_sa_t *data)
{
	return ipsec->sas->del_sa(ipsec->sas, id->src, id->dst, id->spi, id->proto,
							  data->cpi, id->mark);
}

METHOD(kernel_ipsec_t, flush_sas, status_t,
	private_kernel_libipsec_ipsec_t *this)
{
	return ipsec->sas->flush_sas(ipsec->sas);
}

/**
 * Add an explicit exclude route to a routing entry
 */
static void add_exclude_route(private_kernel_libipsec_ipsec_t *this,
							  route_entry_t *route, host_t *src, host_t *dst)
{
	exclude_route_t *exclude;
	host_t *gtw;

	if (this->excludes->find_first(this->excludes, exclude_route_match,
								  (void**)&exclude, dst))
	{
		route->exclude = exclude;
		exclude->refs++;
	}

	if (!route->exclude)
	{
		DBG2(DBG_KNL, "installing new exclude route for %H src %H", dst, src);
		gtw = charon->kernel->get_nexthop(charon->kernel, dst, -1, NULL, NULL);
		if (gtw)
		{
			char *if_name = NULL;

			if (charon->kernel->get_interface(charon->kernel, src, &if_name) &&
				charon->kernel->add_route(charon->kernel, dst->get_address(dst),
									dst->get_family(dst) == AF_INET ? 32 : 128,
									gtw, src, if_name, TRUE) == SUCCESS)
			{
				INIT(exclude,
					.dst = dst->clone(dst),
					.src = src->clone(src),
					.gtw = gtw->clone(gtw),
					.refs = 1,
				);
				route->exclude = exclude;
				this->excludes->insert_last(this->excludes, exclude);
			}
			else
			{
				DBG1(DBG_KNL, "installing exclude route for %H failed", dst);
			}
			gtw->destroy(gtw);
			free(if_name);
		}
		else
		{
			DBG1(DBG_KNL, "gateway lookup for %H failed", dst);
		}
	}
}

/**
 * Remove an exclude route attached to a routing entry
 */
static void remove_exclude_route(private_kernel_libipsec_ipsec_t *this,
								 route_entry_t *route)
{
	char *if_name = NULL;
	host_t *dst;

	if (!route->exclude || --route->exclude->refs > 0)
	{
		return;
	}
	this->excludes->remove(this->excludes, route->exclude, NULL);

	dst = route->exclude->dst;
	DBG2(DBG_KNL, "uninstalling exclude route for %H src %H",
		 dst, route->exclude->src);
	if (charon->kernel->get_interface(charon->kernel, route->exclude->src,
									  &if_name) &&
		charon->kernel->del_route(charon->kernel, dst->get_address(dst),
								  dst->get_family(dst) == AF_INET ? 32 : 128,
								  route->exclude->gtw, route->exclude->src,
								  if_name, TRUE) != SUCCESS)
	{
		DBG1(DBG_KNL, "uninstalling exclude route for %H failed", dst);
	}
	exclude_route_destroy(route->exclude);
	route->exclude = NULL;
	free(if_name);
}

/**
 * Install a route for the given policy
 *
 * this->mutex is released by this function
 */
static bool install_route(private_kernel_libipsec_ipsec_t *this,
	host_t *src, host_t *dst, traffic_selector_t *src_ts,
	traffic_selector_t *dst_ts, policy_entry_t *policy)
{
	route_entry_t *route, *old;
	host_t *src_ip;
	bool is_virtual;

	if (policy->direction != POLICY_OUT)
	{
		this->mutex->unlock(this->mutex);
		return TRUE;
	}

	if (charon->kernel->get_address_by_ts(charon->kernel, src_ts, &src_ip,
										  &is_virtual) != SUCCESS)
	{
		traffic_selector_t *multicast, *broadcast = NULL;
		bool ignore = FALSE;

		this->mutex->unlock(this->mutex);
		switch (src_ts->get_type(src_ts))
		{
			case TS_IPV4_ADDR_RANGE:
				multicast = traffic_selector_create_from_cidr("224.0.0.0/4",
															  0, 0, 0xffff);
				broadcast = traffic_selector_create_from_cidr("255.255.255.255/32",
															  0, 0, 0xffff);
				break;
			case TS_IPV6_ADDR_RANGE:
				multicast = traffic_selector_create_from_cidr("ff00::/8",
															  0, 0, 0xffff);
				break;
			default:
				return FALSE;
		}
		ignore = src_ts->is_contained_in(src_ts, multicast);
		ignore |= broadcast && src_ts->is_contained_in(src_ts, broadcast);
		multicast->destroy(multicast);
		DESTROY_IF(broadcast);
		if (!ignore)
		{
			DBG1(DBG_KNL, "error installing route with policy %R === %R %N",
				 src_ts, dst_ts, policy_dir_names, policy->direction);
		}
		return ignore;
	}

	INIT(route,
		.if_name = router->get_tun_name(router, is_virtual ? src_ip : NULL),
		.src_ip = src_ip,
		.dst_net = chunk_clone(policy->dst.net->get_address(policy->dst.net)),
		.prefixlen = policy->dst.mask,
	);
#ifndef __linux__
	/* on Linux we can't install a gateway */
	route->gateway = charon->kernel->get_nexthop(charon->kernel, dst, -1, src,
												 NULL);
#endif

	if (policy->route)
	{
		old = policy->route;

		if (route_entry_equals(old, route))
		{	/* such a route already exists */
			route_entry_destroy(route);
			this->mutex->unlock(this->mutex);
			return TRUE;
		}
		/* uninstall previously installed route */
		if (charon->kernel->del_route(charon->kernel, old->dst_net,
									  old->prefixlen, old->gateway, old->src_ip,
									  old->if_name, FALSE) != SUCCESS)
		{
			DBG1(DBG_KNL, "error uninstalling route installed with policy "
				 "%R === %R %N", src_ts, dst_ts, policy_dir_names,
				 policy->direction);
		}
		route_entry_destroy(old);
		policy->route = NULL;
	}

	if (!this->allow_peer_ts && dst_ts->is_host(dst_ts, dst))
	{
		DBG1(DBG_KNL, "can't install route for %R === %R %N, conflicts with "
			 "IKE traffic", src_ts, dst_ts, policy_dir_names,
			 policy->direction);
		route_entry_destroy(route);
		this->mutex->unlock(this->mutex);
		return FALSE;
	}
	/* if remote traffic selector covers the IKE peer, add an exclude route */
	if (!this->allow_peer_ts && dst_ts->includes(dst_ts, dst))
	{
		/* add exclude route for peer */
		add_exclude_route(this, route, src, dst);
	}

	DBG2(DBG_KNL, "installing route: %R src %H dev %s",
		 dst_ts, route->src_ip, route->if_name);

	switch (charon->kernel->add_route(charon->kernel, route->dst_net,
									  route->prefixlen, route->gateway,
									  route->src_ip, route->if_name, FALSE))
	{
		case ALREADY_DONE:
			/* route exists, do not uninstall */
			remove_exclude_route(this, route);
			route_entry_destroy(route);
			this->mutex->unlock(this->mutex);
			return TRUE;
		case SUCCESS:
			/* cache the installed route */
			policy->route = route;
			this->mutex->unlock(this->mutex);
			return TRUE;
		default:
			DBG1(DBG_KNL, "installing route failed: %R src %H dev %s",
				 dst_ts, route->src_ip, route->if_name);
			remove_exclude_route(this, route);
			route_entry_destroy(route);
			this->mutex->unlock(this->mutex);
			return FALSE;
	}
}

METHOD(kernel_ipsec_t, add_policy, status_t,
	private_kernel_libipsec_ipsec_t *this, kernel_ipsec_policy_id_t *id,
	kernel_ipsec_manage_policy_t *data)
{
	policy_entry_t *policy, *found = NULL;
	status_t status;

	status = ipsec->policies->add_policy(ipsec->policies, data->src, data->dst,
										 id->src_ts, id->dst_ts, id->dir,
										 data->type, data->sa, id->mark,
										 data->prio);
	if (status != SUCCESS)
	{
		return status;
	}
	/* we track policies in order to install routes */
	policy = create_policy_entry(id->src_ts, id->dst_ts, id->dir);

	this->mutex->lock(this->mutex);
	if (this->policies->find_first(this->policies, policy_entry_equals,
								  (void**)&found, policy))
	{
		policy_entry_destroy(policy);
		policy = found;
	}
	else
	{	/* use the new one, if we have no such policy */
		this->policies->insert_last(this->policies, policy);
	}
	policy->refs++;

	if (!install_route(this, data->src, data->dst, id->src_ts, id->dst_ts,
					   policy))
	{
		return FAILED;
	}
	return SUCCESS;
}

METHOD(kernel_ipsec_t, query_policy, status_t,
	private_kernel_libipsec_ipsec_t *this, kernel_ipsec_policy_id_t *id,
	kernel_ipsec_query_policy_t *data, time_t *use_time)
{
	return NOT_SUPPORTED;
}

METHOD(kernel_ipsec_t, del_policy, status_t,
	private_kernel_libipsec_ipsec_t *this, kernel_ipsec_policy_id_t *id,
	kernel_ipsec_manage_policy_t *data)
{
	policy_entry_t *policy, *found = NULL;
	status_t status;

	status = ipsec->policies->del_policy(ipsec->policies, data->src, data->dst,
										 id->src_ts, id->dst_ts, id->dir,
										 data->type, data->sa, id->mark,
										 data->prio);

	policy = create_policy_entry(id->src_ts, id->dst_ts, id->dir);

	this->mutex->lock(this->mutex);
	if (!this->policies->find_first(this->policies, policy_entry_equals,
									(void**)&found, policy))
	{
		policy_entry_destroy(policy);
		this->mutex->unlock(this->mutex);
		return status;
	}
	policy_entry_destroy(policy);
	policy = found;

	if (--policy->refs > 0)
	{	/* policy is still in use */
		this->mutex->unlock(this->mutex);
		return status;
	}

	if (policy->route)
	{
		route_entry_t *route = policy->route;

		if (charon->kernel->del_route(charon->kernel, route->dst_net,
								route->prefixlen, route->gateway, route->src_ip,
								route->if_name, FALSE) != SUCCESS)
		{
			DBG1(DBG_KNL, "error uninstalling route installed with "
				 "policy %R === %R %N", id->src_ts, id->dst_ts,
				 policy_dir_names, id->dir);
		}
		remove_exclude_route(this, route);
	}
	this->policies->remove(this->policies, policy, NULL);
	policy_entry_destroy(policy);
	this->mutex->unlock(this->mutex);
	return status;
}

METHOD(kernel_ipsec_t, flush_policies, status_t,
	private_kernel_libipsec_ipsec_t *this)
{
	policy_entry_t *pol;
	status_t status;

	status = ipsec->policies->flush_policies(ipsec->policies);

	this->mutex->lock(this->mutex);
	while (this->policies->remove_first(this->policies, (void*)&pol) == SUCCESS)
	{
		if (pol->route)
		{
			route_entry_t *route = pol->route;

			charon->kernel->del_route(charon->kernel, route->dst_net,
									  route->prefixlen, route->gateway,
									  route->src_ip, route->if_name, FALSE);
			remove_exclude_route(this, route);
		}
		policy_entry_destroy(pol);
	}
	this->mutex->unlock(this->mutex);
	return status;
}

METHOD(kernel_ipsec_t, bypass_socket, bool,
	private_kernel_libipsec_ipsec_t *this, int fd, int family)
{
	/* we use exclude routes for this */
	return NOT_SUPPORTED;
}

METHOD(kernel_ipsec_t, enable_udp_decap, bool,
	private_kernel_libipsec_ipsec_t *this, int fd, int family, uint16_t port)
{
	return NOT_SUPPORTED;
}

METHOD(kernel_ipsec_t, destroy, void,
	private_kernel_libipsec_ipsec_t *this)
{
	ipsec->events->unregister_listener(ipsec->events, &this->ipsec_listener);
	this->policies->destroy_function(this->policies, (void*)policy_entry_destroy);
	this->excludes->destroy(this->excludes);
	this->mutex->destroy(this->mutex);
	free(this);
}

/*
 * Described in header.
 */
kernel_libipsec_ipsec_t *kernel_libipsec_ipsec_create()
{
	private_kernel_libipsec_ipsec_t *this;

	INIT(this,
		.public = {
			.interface = {
				.get_features = _get_features,
				.get_spi = _get_spi,
				.get_cpi = _get_cpi,
				.add_sa  = _add_sa,
				.update_sa = _update_sa,
				.query_sa = _query_sa,
				.del_sa = _del_sa,
				.flush_sas = _flush_sas,
				.add_policy = _add_policy,
				.query_policy = _query_policy,
				.del_policy = _del_policy,
				.flush_policies = _flush_policies,
				.bypass_socket = _bypass_socket,
				.enable_udp_decap = _enable_udp_decap,
				.destroy = _destroy,
			},
		},
		.ipsec_listener = {
			.expire = expire,
		},
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.policies = linked_list_create(),
		.excludes = linked_list_create(),
		.allow_peer_ts = lib->settings->get_bool(lib->settings,
					"%s.plugins.kernel-libipsec.allow_peer_ts", FALSE, lib->ns),
	);

	ipsec->events->register_listener(ipsec->events, &this->ipsec_listener);

	return &this->public;
};
