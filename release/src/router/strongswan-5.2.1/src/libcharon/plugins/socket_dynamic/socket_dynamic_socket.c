/*
 * Copyright (C) 2006-2013 Tobias Brunner
 * Copyright (C) 2006 Daniel Roethlisberger
 * Copyright (C) 2005-2010 Martin Willi
 * Copyright (C) 2005 Jan Hutter
 * Hochschule fuer Technik Rapperswil
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

/* for struct in6_pktinfo */
#define _GNU_SOURCE

#include "socket_dynamic_socket.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <net/if.h>

#include <hydra.h>
#include <daemon.h>
#include <threading/thread.h>
#include <threading/rwlock.h>
#include <collections/hashtable.h>

/* these are not defined on some platforms */
#ifndef SOL_IP
#define SOL_IP IPPROTO_IP
#endif
#ifndef SOL_IPV6
#define SOL_IPV6 IPPROTO_IPV6
#endif

/* IPV6_RECVPKTINFO is defined in RFC 3542 which obsoletes RFC 2292 that
 * previously defined IPV6_PKTINFO */
#ifndef IPV6_RECVPKTINFO
#define IPV6_RECVPKTINFO IPV6_PKTINFO
#endif

typedef struct private_socket_dynamic_socket_t private_socket_dynamic_socket_t;
typedef struct dynsock_t dynsock_t;

/**
 * Private data of an socket_t object
 */
struct private_socket_dynamic_socket_t {

	/**
	 * public functions
	 */
	socket_dynamic_socket_t public;

	/**
	 * Hashtable of bound sockets
	 */
	hashtable_t *sockets;

	/**
	 * Lock for sockets hashtable
	 */
	rwlock_t *lock;

	/**
	 * Notification pipe to signal receiver
	 */
	int notify[2];

	/**
	 * Maximum packet size to receive
	 */
	int max_packet;
};

/**
 * Struct for a dynamically allocated socket
 */
struct dynsock_t {

	/**
	 * File descriptor of socket
	 */
	int fd;

	/**
	 * Address family
	 */
	int family;

	/**
	 * Bound source port
	 */
	u_int16_t port;
};

/**
 * Hash function for hashtable
 */
static u_int hash(dynsock_t *key)
{
	return (key->family << 16) | key->port;
}

/**
 * Equals function for hashtable
 */
static bool equals(dynsock_t *a, dynsock_t *b)
{
	return a->family == b->family && a->port == b->port;
}

/**
 * Create a fd_set from all bound sockets
 */
static int build_fds(private_socket_dynamic_socket_t *this, fd_set *fds)
{
	enumerator_t *enumerator;
	dynsock_t *key, *value;
	int maxfd;

	FD_ZERO(fds);
	FD_SET(this->notify[0], fds);
	maxfd = this->notify[0];

	this->lock->read_lock(this->lock);
	enumerator = this->sockets->create_enumerator(this->sockets);
	while (enumerator->enumerate(enumerator, &key, &value))
	{
		FD_SET(value->fd, fds);
		maxfd = max(maxfd, value->fd);
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);

	return maxfd + 1;
}

/**
 * Find the socket select()ed
 */
static dynsock_t* scan_fds(private_socket_dynamic_socket_t *this, fd_set *fds)
{
	enumerator_t *enumerator;
	dynsock_t *key, *value, *selected = NULL;

	this->lock->read_lock(this->lock);
	enumerator = this->sockets->create_enumerator(this->sockets);
	while (enumerator->enumerate(enumerator, &key, &value))
	{
		if (FD_ISSET(value->fd, fds))
		{
			selected = value;
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);

	return selected;
}

/**
 * Receive a packet from a given socket fd
 */
static packet_t *receive_packet(private_socket_dynamic_socket_t *this,
								dynsock_t *skt)
{
	host_t *source = NULL, *dest = NULL;
	ssize_t len;
	char buffer[this->max_packet];
	chunk_t data;
	packet_t *packet;
	struct msghdr msg;
	struct cmsghdr *cmsgptr;
	struct iovec iov;
	char ancillary[64];
	union {
		struct sockaddr_in in4;
		struct sockaddr_in6 in6;
	} src;

	msg.msg_name = &src;
	msg.msg_namelen = sizeof(src);
	iov.iov_base = buffer;
	iov.iov_len = this->max_packet;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = ancillary;
	msg.msg_controllen = sizeof(ancillary);
	msg.msg_flags = 0;
	len = recvmsg(skt->fd, &msg, 0);
	if (len < 0)
	{
		DBG1(DBG_NET, "error reading socket: %s", strerror(errno));
		return NULL;
	}
	if (msg.msg_flags & MSG_TRUNC)
	{
		DBG1(DBG_NET, "receive buffer too small, packet discarded");
		return NULL;
	}
	DBG3(DBG_NET, "received packet %b", buffer, (u_int)len);

	/* read ancillary data to get destination address */
	for (cmsgptr = CMSG_FIRSTHDR(&msg); cmsgptr != NULL;
		 cmsgptr = CMSG_NXTHDR(&msg, cmsgptr))
	{
		if (cmsgptr->cmsg_len == 0)
		{
			DBG1(DBG_NET, "error reading ancillary data");
			return NULL;
		}

		if (cmsgptr->cmsg_level == SOL_IPV6 &&
			cmsgptr->cmsg_type == IPV6_PKTINFO)
		{
			struct in6_pktinfo *pktinfo;
			struct sockaddr_in6 dst;

			pktinfo = (struct in6_pktinfo*)CMSG_DATA(cmsgptr);
			memset(&dst, 0, sizeof(dst));
			memcpy(&dst.sin6_addr, &pktinfo->ipi6_addr, sizeof(dst.sin6_addr));
			dst.sin6_family = AF_INET6;
			dst.sin6_port = htons(skt->port);
			dest = host_create_from_sockaddr((sockaddr_t*)&dst);
		}
		if (cmsgptr->cmsg_level == SOL_IP &&
			cmsgptr->cmsg_type == IP_PKTINFO)
		{
			struct in_pktinfo *pktinfo;
			struct sockaddr_in dst;

			pktinfo = (struct in_pktinfo*)CMSG_DATA(cmsgptr);
			memset(&dst, 0, sizeof(dst));
			memcpy(&dst.sin_addr, &pktinfo->ipi_addr, sizeof(dst.sin_addr));

			dst.sin_family = AF_INET;
			dst.sin_port = htons(skt->port);
			dest = host_create_from_sockaddr((sockaddr_t*)&dst);
		}
		if (dest)
		{
			break;
		}
	}
	if (dest == NULL)
	{
		DBG1(DBG_NET, "error reading IP header");
		return NULL;
	}
	source = host_create_from_sockaddr((sockaddr_t*)&src);
	DBG2(DBG_NET, "received packet: from %#H to %#H", source, dest);
	data = chunk_create(buffer, len);

	packet = packet_create();
	packet->set_source(packet, source);
	packet->set_destination(packet, dest);
	packet->set_data(packet, chunk_clone(data));
	return packet;
}

METHOD(socket_t, receiver, status_t,
	private_socket_dynamic_socket_t *this, packet_t **packet)
{
	dynsock_t *selected;
	packet_t *pkt;
	bool oldstate;
	fd_set fds;
	int maxfd;

	while (TRUE)
	{
		maxfd = build_fds(this, &fds);

		DBG2(DBG_NET, "waiting for data on sockets");
		oldstate = thread_cancelability(TRUE);
		if (select(maxfd, &fds, NULL, NULL, NULL) <= 0)
		{
			thread_cancelability(oldstate);
			return FAILED;
		}
		thread_cancelability(oldstate);

		if (FD_ISSET(this->notify[0], &fds))
		{	/* got notified, read garbage, rebuild fdset */
			char buf[1];

			ignore_result(read(this->notify[0], buf, sizeof(buf)));
			DBG2(DBG_NET, "rebuilding fdset due to newly bound ports");
			continue;
		}
		selected = scan_fds(this, &fds);
		if (selected)
		{
			break;
		}
	}
	pkt = receive_packet(this, selected);
	if (pkt)
	{
		*packet = pkt;
		return SUCCESS;
	}
	return FAILED;
}

/**
 * Get the port allocated dynamically using bind()
 */
static bool get_dynamic_port(int fd, int family, u_int16_t *port)
{
	union {
		struct sockaddr_storage ss;
		struct sockaddr s;
		struct sockaddr_in sin;
		struct sockaddr_in6 sin6;
	} addr;
	socklen_t addrlen;

	addrlen = sizeof(addr);
	if (getsockname(fd, &addr.s, &addrlen) != 0)
	{
		DBG1(DBG_NET, "unable to getsockname: %s", strerror(errno));
		return FALSE;
	}
	switch (family)
	{
		case AF_INET:
			if (addrlen != sizeof(addr.sin) || addr.sin.sin_family != family)
			{
				break;
			}
			*port = ntohs(addr.sin.sin_port);
			return TRUE;
		case AF_INET6:
			if (addrlen != sizeof(addr.sin6) || addr.sin6.sin6_family != family)
			{
				break;
			}
			*port = ntohs(addr.sin6.sin6_port);
			return TRUE;
		default:
			return FALSE;
	}
	DBG1(DBG_NET, "received invalid getsockname() result");
	return FALSE;
}

/**
 * open a socket to send and receive packets
 */
static int open_socket(private_socket_dynamic_socket_t *this,
					   int family, u_int16_t *port)
{
	union {
		struct sockaddr_storage ss;
		struct sockaddr s;
		struct sockaddr_in sin;
		struct sockaddr_in6 sin6;
	} addr;
	int on = TRUE;
	socklen_t addrlen;
	u_int sol, pktinfo = 0;
	int fd;

	memset(&addr, 0, sizeof(addr));
	/* precalculate constants depending on address family */
	switch (family)
	{
		case AF_INET:
			addr.sin.sin_family = AF_INET;
			addr.sin.sin_addr.s_addr = INADDR_ANY;
			addr.sin.sin_port = htons(*port);
			addrlen = sizeof(addr.sin);
			sol = SOL_IP;
			pktinfo = IP_PKTINFO;
			break;
		case AF_INET6:
			addr.sin6.sin6_family = AF_INET6;
			memset(&addr.sin6.sin6_addr, 0, sizeof(addr.sin6.sin6_addr));
			addr.sin6.sin6_port = htons(*port);
			addrlen = sizeof(addr.sin6);
			sol = SOL_IPV6;
			pktinfo = IPV6_RECVPKTINFO;
			break;
		default:
			return 0;
	}

	fd = socket(family, SOCK_DGRAM, IPPROTO_UDP);
	if (fd < 0)
	{
		DBG1(DBG_NET, "could not open socket: %s", strerror(errno));
		return 0;
	}
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void*)&on, sizeof(on)) < 0)
	{
		DBG1(DBG_NET, "unable to set SO_REUSEADDR on socket: %s", strerror(errno));
		close(fd);
		return 0;
	}

	if (bind(fd, &addr.s, addrlen) < 0)
	{
		DBG1(DBG_NET, "unable to bind socket: %s", strerror(errno));
		close(fd);
		return 0;
	}
	if (*port == 0 && !get_dynamic_port(fd, family, port))
	{
		close(fd);
		return 0;
	}

	/* get additional packet info on receive */
	if (setsockopt(fd, sol, pktinfo, &on, sizeof(on)) < 0)
	{
		DBG1(DBG_NET, "unable to set IP_PKTINFO on socket: %s", strerror(errno));
		close(fd);
		return 0;
	}

	if (!hydra->kernel_interface->bypass_socket(hydra->kernel_interface,
												fd, family))
	{
		DBG1(DBG_NET, "installing IKE bypass policy failed");
	}

	/* enable UDP decapsulation on each socket */
	if (!hydra->kernel_interface->enable_udp_decap(hydra->kernel_interface,
												   fd, family, *port))
	{
		DBG1(DBG_NET, "enabling UDP decapsulation for %s on port %d failed",
			 family == AF_INET ? "IPv4" : "IPv6", *port);
	}

	return fd;
}

/**
 * Get the first usable socket for an address family
 */
static dynsock_t *get_any_socket(private_socket_dynamic_socket_t *this,
								 int family)
{
	dynsock_t *key, *value, *found = NULL;
	enumerator_t *enumerator;

	this->lock->read_lock(this->lock);
	enumerator = this->sockets->create_enumerator(this->sockets);
	while (enumerator->enumerate(enumerator, &key, &value))
	{
		if (value->family == family)
		{
			found = value;
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);

	return found;
}

/**
 * Find/Create a socket to send from host
 */
static dynsock_t *find_socket(private_socket_dynamic_socket_t *this,
							  int family, u_int16_t port)
{
	dynsock_t *skt, lookup = {
		.family = family,
		.port = port,
	};
	char buf[] = {0x01};
	int fd;

	this->lock->read_lock(this->lock);
	skt = this->sockets->get(this->sockets, &lookup);
	this->lock->unlock(this->lock);
	if (skt)
	{
		return skt;
	}
	if (!port)
	{
		skt = get_any_socket(this, family);
		if (skt)
		{
			return skt;
		}
	}
	fd = open_socket(this, family, &port);
	if (!fd)
	{
		return NULL;
	}
	INIT(skt,
		.family = family,
		.port = port,
		.fd = fd,
	);
	this->lock->write_lock(this->lock);
	this->sockets->put(this->sockets, skt, skt);
	this->lock->unlock(this->lock);
	/* notify receiver thread to reread socket list */
	ignore_result(write(this->notify[1], buf, sizeof(buf)));

	return skt;
}

METHOD(socket_t, sender, status_t,
	private_socket_dynamic_socket_t *this, packet_t *packet)
{
	dynsock_t *skt;
	host_t *src, *dst;
	int family;
	ssize_t len;
	chunk_t data;
	struct msghdr msg;
	struct cmsghdr *cmsg;
	struct iovec iov;

	src = packet->get_source(packet);
	dst = packet->get_destination(packet);
	family = src->get_family(src);
	skt = find_socket(this, family, src->get_port(src));
	if (!skt)
	{
		return FAILED;
	}

	data = packet->get_data(packet);
	DBG2(DBG_NET, "sending packet: from %#H to %#H", src, dst);

	memset(&msg, 0, sizeof(struct msghdr));
	msg.msg_name = dst->get_sockaddr(dst);;
	msg.msg_namelen = *dst->get_sockaddr_len(dst);
	iov.iov_base = data.ptr;
	iov.iov_len = data.len;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_flags = 0;

	if (!src->is_anyaddr(src))
	{
		if (family == AF_INET)
		{
			struct in_addr *addr;
			struct sockaddr_in *sin;
			char buf[CMSG_SPACE(sizeof(struct in_pktinfo))];
			struct in_pktinfo *pktinfo;

			memset(buf, 0, sizeof(buf));
			msg.msg_control = buf;
			msg.msg_controllen = sizeof(buf);
			cmsg = CMSG_FIRSTHDR(&msg);
			cmsg->cmsg_level = SOL_IP;
			cmsg->cmsg_type = IP_PKTINFO;
			cmsg->cmsg_len = CMSG_LEN(sizeof(struct in_pktinfo));
			pktinfo = (struct in_pktinfo*)CMSG_DATA(cmsg);
			addr = &pktinfo->ipi_spec_dst;
			sin = (struct sockaddr_in*)src->get_sockaddr(src);
			memcpy(addr, &sin->sin_addr, sizeof(struct in_addr));
		}
		else
		{
			char buf[CMSG_SPACE(sizeof(struct in6_pktinfo))];
			struct in6_pktinfo *pktinfo;
			struct sockaddr_in6 *sin;

			memset(buf, 0, sizeof(buf));
			msg.msg_control = buf;
			msg.msg_controllen = sizeof(buf);
			cmsg = CMSG_FIRSTHDR(&msg);
			cmsg->cmsg_level = SOL_IPV6;
			cmsg->cmsg_type = IPV6_PKTINFO;
			cmsg->cmsg_len = CMSG_LEN(sizeof(struct in6_pktinfo));
			pktinfo = (struct in6_pktinfo*)CMSG_DATA(cmsg);
			sin = (struct sockaddr_in6*)src->get_sockaddr(src);
			memcpy(&pktinfo->ipi6_addr, &sin->sin6_addr, sizeof(struct in6_addr));
		}
	}

	len = sendmsg(skt->fd, &msg, 0);
	if (len != data.len)
	{
		DBG1(DBG_NET, "error writing to socket: %s", strerror(errno));
		return FAILED;
	}
	return SUCCESS;
}

METHOD(socket_t, get_port, u_int16_t,
	private_socket_dynamic_socket_t *this, bool nat_t)
{
	/* we return 0 here for users that have no explicit port configured, the
	 * sender will default to the default port in this case */
	return 0;
}

METHOD(socket_t, supported_families, socket_family_t,
	private_socket_dynamic_socket_t *this)
{
	/* we could return only the families of the opened sockets, but it could
	 * be that both families are supported even if no socket is yet open */
	return SOCKET_FAMILY_BOTH;
}

METHOD(socket_t, destroy, void,
	private_socket_dynamic_socket_t *this)
{
	enumerator_t *enumerator;
	dynsock_t *key, *value;

	enumerator = this->sockets->create_enumerator(this->sockets);
	while (enumerator->enumerate(enumerator, &key, &value))
	{
		close(value->fd);
		free(value);
	}
	enumerator->destroy(enumerator);
	this->sockets->destroy(this->sockets);
	this->lock->destroy(this->lock);

	close(this->notify[0]);
	close(this->notify[1]);
	free(this);
}

/*
 * See header for description
 */
socket_dynamic_socket_t *socket_dynamic_socket_create()
{
	private_socket_dynamic_socket_t *this;

	INIT(this,
		.public = {
			.socket = {
				.send = _sender,
				.receive = _receiver,
				.get_port = _get_port,
				.supported_families = _supported_families,
				.destroy = _destroy,
			},
		},
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
		.max_packet = lib->settings->get_int(lib->settings,
								"%s.max_packet", PACKET_MAX_DEFAULT, lib->ns),
	);

	if (pipe(this->notify) != 0)
	{
		DBG1(DBG_NET, "creating notify pipe for dynamic socket failed");
		free(this);
		return NULL;
	}

	this->sockets = hashtable_create((void*)hash, (void*)equals, 8);

	return &this->public;
}
