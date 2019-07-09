/*
 * Copyright (C) 2014 Tobias Brunner
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

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "android_dns_proxy.h"

#include <daemon.h>
#include <threading/rwlock.h>
#include <collections/hashtable.h>
#include <processing/jobs/callback_job.h>

/**
 * Timeout in seconds for sockets (i.e. not used for x seconds -> delete)
 */
#define SOCKET_TIMEOUT 30

typedef struct private_android_dns_proxy_t private_android_dns_proxy_t;

struct private_android_dns_proxy_t {

	/**
	 * Public interface
	 */
	android_dns_proxy_t public;

	/**
	 * Mapping from source address to sockets
	 */
	hashtable_t *sockets;

	/**
	 * Registered callback
	 */
	dns_proxy_response_cb_t cb;

	/**
	 * Data passed to callback
	 */
	void *data;

	/**
	 * Lock used to synchronize access to the private members
	 */
	rwlock_t *lock;

	/**
	 * Hostnames to filter queries by
	 */
	hashtable_t *hostnames;
};

/**
 * Data for proxy sockets
 */
typedef struct {
	private_android_dns_proxy_t *proxy;
	time_t last_use;
	host_t *src;
	int fd;
} proxy_socket_t;

/**
 * Destroy a socket
 */
static void socket_destroy(proxy_socket_t *this)
{
	this->src->destroy(this->src);
	if (this->fd != -1)
	{
		close(this->fd);
	}
	free(this);
}

/**
 * Hash a proxy socket by src address
 */
static u_int socket_hash(host_t *src)
{
	uint16_t port = src->get_port(src);
	return chunk_hash_inc(src->get_address(src),
						  chunk_hash(chunk_from_thing(port)));
}

/**
 * Compare proxy sockets by src address
 */
static bool socket_equals(host_t *a, host_t *b)
{
	return a->equals(a, b);
}

/**
 * Opens a UDP socket for the given address family
 */
static int open_socket(int family)
{
	int skt;

	skt = socket(family, SOCK_DGRAM, IPPROTO_UDP);
	if (skt < 0)
	{
		DBG1(DBG_NET, "could not open proxy socket: %s", strerror(errno));
		return -1;
	}
	if (!charon->kernel->bypass_socket(charon->kernel, skt, family))
	{
		DBG1(DBG_NET, "installing bypass policy for proxy socket failed");
		close(skt);
		return -1;
	}
	return skt;
}

/**
 * Create a proxy socket for the given source
 */
static proxy_socket_t *create_socket(private_android_dns_proxy_t *this,
									 host_t *src)
{
	proxy_socket_t *skt;

	INIT(skt,
		.proxy = this,
		.src = src->clone(src),
		.fd = open_socket(src->get_family(src)),
	);
	if (skt->fd == -1)
	{
		socket_destroy(skt);
		return NULL;
	}
	return skt;
}

CALLBACK(handle_response, bool,
	proxy_socket_t *this, int fd, watcher_event_t event)
{
	struct sockaddr_storage addr;
	socklen_t addr_len = sizeof(addr);
	char buf[4096];
	ssize_t len;
	host_t *src;

	len = recvfrom(fd, buf, sizeof(buf), MSG_DONTWAIT, (struct sockaddr*)&addr,
				   &addr_len);
	if (len > 0)
	{
		ip_packet_t *packet;

		src = host_create_from_sockaddr((sockaddr_t*)&addr);
		if (!src)
		{
			DBG1(DBG_NET, "failed to parse source address");
			return TRUE;
		}
		packet = ip_packet_create_udp_from_data(src, this->src,
												chunk_create(buf, len));
		if (!packet)
		{
			DBG1(DBG_NET, "failed to parse DNS response");
			return TRUE;
		}
		this->proxy->lock->read_lock(this->proxy->lock);
		this->last_use = time_monotonic(NULL);
		if (this->proxy->cb)
		{
			this->proxy->cb(this->proxy->data, packet);
		}
		else
		{
			packet->destroy(packet);
		}
		this->proxy->lock->unlock(this->proxy->lock);
	}
	else if (errno != EWOULDBLOCK)
	{
		DBG1(DBG_NET, "receiving DNS response failed: %s", strerror(errno));
	}
	return TRUE;
}

CALLBACK(handle_timeout, job_requeue_t,
	proxy_socket_t *this)
{
	time_t now, diff;

	now = time_monotonic(NULL);
	this->proxy->lock->write_lock(this->proxy->lock);
	diff = now - this->last_use;
	if (diff >= SOCKET_TIMEOUT)
	{
		this->proxy->sockets->remove(this->proxy->sockets, this->src);
		lib->watcher->remove(lib->watcher, this->fd);
		this->proxy->lock->unlock(this->proxy->lock);
		socket_destroy(this);
		return JOB_REQUEUE_NONE;
	}
	this->proxy->lock->unlock(this->proxy->lock);
	return JOB_RESCHEDULE(SOCKET_TIMEOUT - diff);
}

/**
 * DNS header and masks to access flags
 */
typedef struct __attribute__((packed)) {
	uint16_t id;
	uint16_t flags;
#define DNS_QR_MASK 0x8000
#define DNS_OPCODE_MASK 0x7800
	uint16_t qdcount;
	uint16_t ancount;
	uint16_t nscount;
	uint16_t arcount;
} dns_header_t;

/**
 * Extract the hostname in the question section data points to.
 * Hostnames can be at most 255 characters (including dots separating labels),
 * each label must be between 1 and 63 characters.
 * The format is [len][label][len][label], followed by a null byte to indicate
 * the null label of the root.
 */
static char *extract_hostname(chunk_t data)
{
	char *hostname, *pos, *end;
	uint8_t label;

	if (!data.len || data.len > 255)
	{
		return NULL;
	}
	label = *data.ptr;
	data = chunk_skip(data, 1);
	hostname = strndup(data.ptr, data.len);
	/* replace all label lengths with dots */
	pos = hostname + label;
	end = hostname + strlen(hostname);
	while (pos < end)
	{
		label = *pos;
		*pos++ = '.';
		pos += label;
	}
	return hostname;
}

/**
 * Check if the DNS query is for one of the allowed hostnames
 */
static bool check_hostname(private_android_dns_proxy_t *this, chunk_t data)
{
	dns_header_t *dns;
	char *hostname;
	bool success = FALSE;

	this->lock->read_lock(this->lock);
	if (!this->hostnames->get_count(this->hostnames))
	{
		this->lock->unlock(this->lock);
		return TRUE;
	}
	if (data.len < sizeof(dns_header_t))
	{
		this->lock->unlock(this->lock);
		return FALSE;
	}
	dns = (dns_header_t*)data.ptr;
	if ((ntohs(dns->flags) & DNS_QR_MASK) == 0 &&
		(ntohs(dns->flags) & DNS_OPCODE_MASK) == 0 &&
		 dns->qdcount)
	{
		data = chunk_skip(data, sizeof(dns_header_t));
		hostname = extract_hostname(data);
		if (hostname && this->hostnames->get(this->hostnames, hostname))
		{
			success = TRUE;
		}
		free(hostname);
	}
	this->lock->unlock(this->lock);
	return success;
}

METHOD(android_dns_proxy_t, handle, bool,
	private_android_dns_proxy_t *this, ip_packet_t *packet)
{
	proxy_socket_t *skt;
	host_t *dst, *src;
	chunk_t data;

	if (packet->get_next_header(packet) != IPPROTO_UDP)
	{
		return FALSE;
	}
	dst = packet->get_destination(packet);
	if (dst->get_port(dst) != 53)
	{	/* no DNS packet */
		return FALSE;
	}
	data = packet->get_payload(packet);
	/* remove UDP header */
	data = chunk_skip(data, 8);
	if (!check_hostname(this, data))
	{
		return FALSE;
	}
	src = packet->get_source(packet);
	this->lock->write_lock(this->lock);
	skt = this->sockets->get(this->sockets, src);
	if (!skt)
	{
		skt = create_socket(this, src);
		if (!skt)
		{
			this->lock->unlock(this->lock);
			return FALSE;
		}
		this->sockets->put(this->sockets, skt->src, skt);
		lib->watcher->add(lib->watcher, skt->fd, WATCHER_READ, handle_response,
						  skt);
		lib->scheduler->schedule_job(lib->scheduler,
			(job_t*)callback_job_create(handle_timeout, skt,
					NULL, (callback_job_cancel_t)return_false), SOCKET_TIMEOUT);
	}
	skt->last_use = time_monotonic(NULL);
	if (sendto(skt->fd, data.ptr, data.len, 0, dst->get_sockaddr(dst),
			   *dst->get_sockaddr_len(dst)) != data.len)
	{
		DBG1(DBG_NET, "sending DNS request failed: %s", strerror(errno));
	}
	this->lock->unlock(this->lock);
	return TRUE;
}

METHOD(android_dns_proxy_t, register_cb, void,
	private_android_dns_proxy_t *this, dns_proxy_response_cb_t cb, void *data)
{
	this->lock->write_lock(this->lock);
	this->cb = cb;
	this->data = data;
	this->lock->unlock(this->lock);
}

METHOD(android_dns_proxy_t, unregister_cb, void,
	private_android_dns_proxy_t *this, dns_proxy_response_cb_t cb)
{
	this->lock->write_lock(this->lock);
	if (this->cb == cb)
	{
		this->cb = NULL;
	}
	this->lock->unlock(this->lock);
}

METHOD(android_dns_proxy_t, add_hostname, void,
	private_android_dns_proxy_t *this, char *hostname)
{
	char *existing;

	hostname = strdup(hostname);
	this->lock->write_lock(this->lock);
	existing = this->hostnames->put(this->hostnames, hostname, hostname);
	this->lock->unlock(this->lock);
	free(existing);
}

METHOD(android_dns_proxy_t, destroy, void,
	private_android_dns_proxy_t *this)
{
	this->hostnames->destroy_function(this->hostnames, (void*)free);
	this->sockets->destroy_function(this->sockets, (void*)socket_destroy);
	this->lock->destroy(this->lock);
	free(this);
}

/**
 * Described in header.
 */
android_dns_proxy_t *android_dns_proxy_create()
{
	private_android_dns_proxy_t *this;

	INIT(this,
		.public = {
			.handle = _handle,
			.register_cb = _register_cb,
			.unregister_cb = _unregister_cb,
			.add_hostname = _add_hostname,
			.destroy = _destroy,
		},
		.sockets = hashtable_create((hashtable_hash_t)socket_hash,
									(hashtable_equals_t)socket_equals, 4),
		.hostnames = hashtable_create(hashtable_hash_str,
									  hashtable_equals_str, 4),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
	);

	return &this->public;
}
