/*
 * Copyright (C) 2012-2015 Tobias Brunner
 * HSR Hochschule fuer Technik Rapperswil
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
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

#include "android_net.h"

#include "../android_jni.h"
#include "../charonservice.h"
#include <daemon.h>
#include <processing/jobs/callback_job.h>
#include <threading/mutex.h>

/** delay before firing roam events (ms) */
#define ROAM_DELAY 100
#define ROAM_DELAY_RECHECK 1000

typedef struct private_android_net_t private_android_net_t;

struct private_android_net_t {

	/**
	 * Public kernel interface
	 */
	kernel_net_t public;

	/**
	 * Reference to NetworkManager object
	 */
	network_manager_t *network_manager;

	/**
	 * Earliest time of the next roam event
	 */
	timeval_t next_roam;

	/**
	 * Mutex to check and update roam event time, and other private members
	 */
	mutex_t *mutex;

	/**
	 * List of virtual IPs
	 */
	linked_list_t *vips;

	/**
	 * Socket used to determine source address
	 */
	int socket_v4;

	/**
	 * Whether the device is currently connected
	 */
	bool connected;
};

/**
 * callback function that raises the delayed roam event
 */
static job_requeue_t roam_event()
{
	/* this will fail if no connection is up */
	charonservice->bypass_socket(charonservice, -1, 0);
	charon->kernel->roam(charon->kernel, TRUE);
	return JOB_REQUEUE_NONE;
}

/**
 * Listen for connectivity change events and queue a roam event
 */
static void connectivity_cb(private_android_net_t *this,
							bool disconnected)
{
	timeval_t now;
	job_t *job;

	time_monotonic(&now);
	this->mutex->lock(this->mutex);
	this->connected = !disconnected;
	if (!timercmp(&now, &this->next_roam, >))
	{
		this->mutex->unlock(this->mutex);
		return;
	}
	timeval_add_ms(&now, ROAM_DELAY);
	this->next_roam = now;
	this->mutex->unlock(this->mutex);

	job = (job_t*)callback_job_create((callback_job_cb_t)roam_event, NULL,
									   NULL, NULL);
	lib->scheduler->schedule_job_ms(lib->scheduler, job, ROAM_DELAY);
}

METHOD(kernel_net_t, get_source_addr, host_t*,
	private_android_net_t *this, host_t *dest, host_t *src)
{
	union {
		struct sockaddr sockaddr;
		struct sockaddr_in sin;
		struct sockaddr_in6 sin6;
	} addr;
	socklen_t addrlen;
	timeval_t now;
	job_t *job;

	addrlen = *dest->get_sockaddr_len(dest);
	addr.sockaddr.sa_family = AF_UNSPEC;
	if (connect(this->socket_v4, &addr.sockaddr, addrlen) < 0)
	{
		DBG1(DBG_KNL, "failed to disconnect socket: %s", strerror(errno));
		return NULL;
	}
	if (android_sdk_version <= ANDROID_JELLY_BEAN_MR2)
	{	/* this seems to help avoiding the VIP, unless there is no connectivity
		 * at all */
		charonservice->bypass_socket(charonservice, -1, 0);
	}
	if (connect(this->socket_v4, dest->get_sockaddr(dest), addrlen) < 0)
	{
		/* don't report an error if we are not connected (ENETUNREACH) */
		if (errno != ENETUNREACH)
		{
			DBG1(DBG_KNL, "failed to connect socket: %s", strerror(errno));
		}
		else
		{
			time_monotonic(&now);
			this->mutex->lock(this->mutex);
			if (this->connected && timercmp(&now, &this->next_roam, >))
			{	/* we were not able to find a source address but reportedly are
				 * connected, trigger a recheck in case an IP address appears
				 * delayed but the callback is not triggered again */
				timeval_add_ms(&now, ROAM_DELAY_RECHECK);
				this->next_roam = now;
				this->mutex->unlock(this->mutex);
				job = (job_t*)callback_job_create((callback_job_cb_t)roam_event,
												  NULL, NULL, NULL);
				lib->scheduler->schedule_job_ms(lib->scheduler, job,
												ROAM_DELAY_RECHECK);
			}
			else
			{
				this->mutex->unlock(this->mutex);
			}
		}
		return NULL;
	}
	if (getsockname(this->socket_v4, &addr.sockaddr, &addrlen) < 0)
	{
		DBG1(DBG_KNL, "failed to determine src address: %s", strerror(errno));
		return NULL;
	}
	return host_create_from_sockaddr((sockaddr_t*)&addr);
}

CALLBACK(vip_equals, bool,
	host_t *vip, va_list args)
{
	host_t *host;

	VA_ARGS_VGET(args, host);
	return host->ip_equals(host, vip);
}

METHOD(kernel_net_t, get_source_addr_old, host_t*,
	private_android_net_t *this, host_t *dest, host_t *src)
{
	host_t *host;

	/* on older Android versions we might get the virtual IP back because
	 * the protect() implementation there and connect() don't properly work
	 * together, on newer releases (using fwmarks) that's not a problem */
	host = get_source_addr(this, dest, src);
	if (host)
	{
		this->mutex->lock(this->mutex);
		if (this->vips->find_first(this->vips, vip_equals, NULL, host))
		{
			host->destroy(host);
			host = NULL;
		}
		this->mutex->unlock(this->mutex);
	}
	return host;
}

METHOD(kernel_net_t, get_nexthop, host_t*,
	private_android_net_t *this, host_t *dest, int prefix, host_t *src,
	char **iface)
{
	return NULL;
}

METHOD(kernel_net_t, get_interface, bool,
	private_android_net_t *this, host_t *host, char **name)
{
	if (name)
	{	/* the actual name does not matter in our case */
		*name = strdup("strongswan");
	}
	return TRUE;
}

METHOD(kernel_net_t, create_address_enumerator, enumerator_t*,
	private_android_net_t *this, kernel_address_type_t which)
{
	/* return virtual IPs if requested, nothing otherwise */
	if (which & ADDR_TYPE_VIRTUAL)
	{
		this->mutex->lock(this->mutex);
		return enumerator_create_cleaner(
					this->vips->create_enumerator(this->vips),
					(void*)this->mutex->unlock, this->mutex);
	}
	return enumerator_create_empty();
}

METHOD(kernel_net_t, add_ip, status_t,
	private_android_net_t *this, host_t *virtual_ip, int prefix, char *iface)
{
	this->mutex->lock(this->mutex);
	this->vips->insert_last(this->vips, virtual_ip->clone(virtual_ip));
	this->mutex->unlock(this->mutex);
	return SUCCESS;
}

METHOD(kernel_net_t, del_ip, status_t,
	private_android_net_t *this, host_t *virtual_ip, int prefix, bool wait)
{
	host_t *vip;

	this->mutex->lock(this->mutex);
	if (this->vips->find_first(this->vips, vip_equals, (void**)&vip,
							   virtual_ip))
	{
		this->vips->remove(this->vips, vip, NULL);
		vip->destroy(vip);
	}
	this->mutex->unlock(this->mutex);
	return SUCCESS;
}

METHOD(kernel_net_t, add_route, status_t,
	private_android_net_t *this, chunk_t dst_net, uint8_t prefixlen,
	host_t *gateway, host_t *src_ip, char *if_name)
{
	return NOT_SUPPORTED;
}

METHOD(kernel_net_t, del_route, status_t,
	private_android_net_t *this, chunk_t dst_net, uint8_t prefixlen,
	host_t *gateway, host_t *src_ip, char *if_name)
{
	return NOT_SUPPORTED;
}

METHOD(kernel_net_t, destroy, void,
	private_android_net_t *this)
{
	this->network_manager->remove_connectivity_cb(this->network_manager,
												 (void*)connectivity_cb);
	this->mutex->destroy(this->mutex);
	this->vips->destroy(this->vips);
	close(this->socket_v4);
	free(this);
}

kernel_net_t *kernel_android_net_create()
{
	private_android_net_t *this;

	INIT(this,
		.public = {
			.get_source_addr = _get_source_addr,
			.get_nexthop = _get_nexthop,
			.get_interface = _get_interface,
			.create_address_enumerator = _create_address_enumerator,
			.add_ip = _add_ip,
			.del_ip = _del_ip,
			.add_route = _add_route,
			.del_route = _del_route,
			.destroy = _destroy,
		},
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.vips = linked_list_create(),
		.network_manager = charonservice->get_network_manager(charonservice),
	);
	timerclear(&this->next_roam);

	if (android_sdk_version <= ANDROID_JELLY_BEAN_MR2)
	{
		this->public.get_source_addr = _get_source_addr_old;
	}

	this->socket_v4 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (this->socket_v4 < 0)
	{
		DBG1(DBG_KNL, "failed to create socket to lookup src addresses: %s",
			 strerror(errno));
	}
	charonservice->bypass_socket(charonservice, this->socket_v4, AF_INET);

	this->mutex->lock(this->mutex);
	this->network_manager->add_connectivity_cb(
						this->network_manager, (void*)connectivity_cb, this);
	this->connected = this->network_manager->is_connected(this->network_manager);
	this->mutex->unlock(this->mutex);
	return &this->public;
}
