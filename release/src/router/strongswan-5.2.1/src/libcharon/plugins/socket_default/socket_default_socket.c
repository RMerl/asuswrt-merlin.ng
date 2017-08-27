/*
 * Copyright (C) 2006-2013 Tobias Brunner
 * Copyright (C) 2006 Daniel Roethlisberger
 * Copyright (C) 2005-2010 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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

/* for struct in6_pktinfo */
#define _GNU_SOURCE
#ifdef __sun
#define _XPG4_2
#define __EXTENSIONS__
#endif
/* make sure to use the proper defs on Mac OS X */
#define __APPLE_USE_RFC_3542

#include "socket_default_socket.h"

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

/* these are not defined on some platforms */
#ifndef SOL_IP
#define SOL_IP IPPROTO_IP
#endif
#ifndef SOL_IPV6
#define SOL_IPV6 IPPROTO_IPV6
#endif
#ifndef IPV6_TCLASS
#define IPV6_TCLASS 67
#endif

/* IPV6_RECVPKTINFO is defined in RFC 3542 which obsoletes RFC 2292 that
 * previously defined IPV6_PKTINFO */
#ifndef IPV6_RECVPKTINFO
#define IPV6_RECVPKTINFO IPV6_PKTINFO
#endif

#ifndef IN6ADDR_ANY_INIT
#define IN6ADDR_ANY_INIT {{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}}}
#endif

#ifndef HAVE_IN6ADDR_ANY
static const struct in6_addr in6addr_any = IN6ADDR_ANY_INIT;
#endif

typedef struct private_socket_default_socket_t private_socket_default_socket_t;

/**
 * Private data of an socket_t object
 */
struct private_socket_default_socket_t {

	/**
	 * public functions
	 */
	socket_default_socket_t public;

	/**
	 * Configured port (or random, if initially 0)
	 */
	u_int16_t port;

	/**
	 * Configured port for NAT-T (or random, if initially 0)
	 */
	u_int16_t natt;

	/**
	 * IPv4 socket (500 or port)
	 */
	int ipv4;

	/**
	 * IPv4 socket for NAT-T (4500 or natt)
	 */
	int ipv4_natt;

	/**
	 * IPv6 socket (500 or port)
	 */
	int ipv6;

	/**
	 * IPv6 socket for NAT-T (4500 or natt)
	 */
	int ipv6_natt;

	/**
	 * DSCP value set on IPv4 socket
	 */
	u_int8_t dscp4;

	/**
	 * DSCP value set on IPv4 socket for NAT-T (4500 or natt)
	 */
	u_int8_t dscp4_natt;

	/**
	 * DSCP value set on IPv6 socket (500 or port)
	 */
	u_int8_t dscp6;

	/**
	 * DSCP value set on IPv6 socket for NAT-T (4500 or natt)
	 */
	u_int8_t dscp6_natt;

	/**
	 * Maximum packet size to receive
	 */
	int max_packet;

	/**
	 * TRUE if the source address should be set on outbound packets
	 */
	bool set_source;
};

METHOD(socket_t, receiver, status_t,
	private_socket_default_socket_t *this, packet_t **packet)
{
	char buffer[this->max_packet];
	chunk_t data;
	packet_t *pkt;
	host_t *source = NULL, *dest = NULL;
	int bytes_read = 0;
	bool oldstate;

	fd_set rfds;
	int max_fd = 0, selected = 0;
	u_int16_t port = 0;

	FD_ZERO(&rfds);

	if (this->ipv4 != -1)
	{
		FD_SET(this->ipv4, &rfds);
		max_fd = max(max_fd, this->ipv4);
	}
	if (this->ipv4_natt != -1)
	{
		FD_SET(this->ipv4_natt, &rfds);
		max_fd = max(max_fd, this->ipv4_natt);
	}
	if (this->ipv6 != -1)
	{
		FD_SET(this->ipv6, &rfds);
		max_fd = max(max_fd, this->ipv6);
	}
	if (this->ipv6_natt != -1)
	{
		FD_SET(this->ipv6_natt, &rfds);
		max_fd = max(max_fd, this->ipv6_natt);
	}

	DBG2(DBG_NET, "waiting for data on sockets");
	oldstate = thread_cancelability(TRUE);
	if (select(max_fd + 1, &rfds, NULL, NULL, NULL) <= 0)
	{
		thread_cancelability(oldstate);
		return FAILED;
	}
	thread_cancelability(oldstate);

	if (this->ipv4 != -1 && FD_ISSET(this->ipv4, &rfds))
	{
		port = this->port;
		selected = this->ipv4;
	}
	if (this->ipv4_natt != -1 && FD_ISSET(this->ipv4_natt, &rfds))
	{
		port = this->natt;
		selected = this->ipv4_natt;
	}
	if (this->ipv6 != -1 && FD_ISSET(this->ipv6, &rfds))
	{
		port = this->port;
		selected = this->ipv6;
	}
	if (this->ipv6_natt != -1 && FD_ISSET(this->ipv6_natt, &rfds))
	{
		port = this->natt;
		selected = this->ipv6_natt;
	}
	if (selected)
	{
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
		bytes_read = recvmsg(selected, &msg, 0);
		if (bytes_read < 0)
		{
			DBG1(DBG_NET, "error reading socket: %s", strerror(errno));
			return FAILED;
		}
		if (msg.msg_flags & MSG_TRUNC)
		{
			DBG1(DBG_NET, "receive buffer too small, packet discarded");
			return FAILED;
		}
		DBG3(DBG_NET, "received packet %b", buffer, bytes_read);

		/* read ancillary data to get destination address */
		for (cmsgptr = CMSG_FIRSTHDR(&msg); cmsgptr != NULL;
			 cmsgptr = CMSG_NXTHDR(&msg, cmsgptr))
		{
			if (cmsgptr->cmsg_len == 0)
			{
				DBG1(DBG_NET, "error reading ancillary data");
				return FAILED;
			}

#ifdef HAVE_IN6_PKTINFO
			if (cmsgptr->cmsg_level == SOL_IPV6 &&
				cmsgptr->cmsg_type == IPV6_PKTINFO)
			{
				struct in6_pktinfo *pktinfo;
				pktinfo = (struct in6_pktinfo*)CMSG_DATA(cmsgptr);
				struct sockaddr_in6 dst;

				memset(&dst, 0, sizeof(dst));
				memcpy(&dst.sin6_addr, &pktinfo->ipi6_addr, sizeof(dst.sin6_addr));
				dst.sin6_family = AF_INET6;
				dst.sin6_port = htons(port);
				dest = host_create_from_sockaddr((sockaddr_t*)&dst);
			}
#endif /* HAVE_IN6_PKTINFO */
			if (cmsgptr->cmsg_level == SOL_IP &&
#ifdef IP_PKTINFO
				cmsgptr->cmsg_type == IP_PKTINFO
#elif defined(IP_RECVDSTADDR)
				cmsgptr->cmsg_type == IP_RECVDSTADDR
#else
				FALSE
#endif
				)
			{
				struct in_addr *addr;
				struct sockaddr_in dst;

#ifdef IP_PKTINFO
				struct in_pktinfo *pktinfo;
				pktinfo = (struct in_pktinfo*)CMSG_DATA(cmsgptr);
				addr = &pktinfo->ipi_addr;
#elif defined(IP_RECVDSTADDR)
				addr = (struct in_addr*)CMSG_DATA(cmsgptr);
#endif
				memset(&dst, 0, sizeof(dst));
				memcpy(&dst.sin_addr, addr, sizeof(dst.sin_addr));

				dst.sin_family = AF_INET;
				dst.sin_port = htons(port);
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
			return FAILED;
		}
		source = host_create_from_sockaddr((sockaddr_t*)&src);

		pkt = packet_create();
		pkt->set_source(pkt, source);
		pkt->set_destination(pkt, dest);
		DBG2(DBG_NET, "received packet: from %#H to %#H", source, dest);
		data = chunk_create(buffer, bytes_read);
		pkt->set_data(pkt, chunk_clone(data));
	}
	else
	{
		/* oops, shouldn't happen */
		return FAILED;
	}
	/* return packet */
	*packet = pkt;
	return SUCCESS;
}

METHOD(socket_t, sender, status_t,
	private_socket_default_socket_t *this, packet_t *packet)
{
	int sport, skt = -1, family;
	ssize_t bytes_sent;
	chunk_t data;
	host_t *src, *dst;
	struct msghdr msg;
	struct cmsghdr *cmsg;
	struct iovec iov;
	u_int8_t *dscp;

	src = packet->get_source(packet);
	dst = packet->get_destination(packet);
	data = packet->get_data(packet);

	DBG2(DBG_NET, "sending packet: from %#H to %#H", src, dst);

	/* send data */
	sport = src->get_port(src);
	family = dst->get_family(dst);
	if (sport == 0 || sport == this->port)
	{
		switch (family)
		{
			case AF_INET:
				skt = this->ipv4;
				dscp = &this->dscp4;
				break;
			case AF_INET6:
				skt = this->ipv6;
				dscp = &this->dscp6;
				break;
			default:
				return FAILED;
		}
	}
	else if (sport == this->natt)
	{
		switch (family)
		{
			case AF_INET:
				skt = this->ipv4_natt;
				dscp = &this->dscp4_natt;
				break;
			case AF_INET6:
				skt = this->ipv6_natt;
				dscp = &this->dscp6_natt;
				break;
			default:
				return FAILED;
		}
	}
	if (skt == -1)
	{
		DBG1(DBG_NET, "no socket found to send IPv%d packet from port %d",
			 family == AF_INET ? 4 : 6, sport);
		return FAILED;
	}

	/* setting DSCP values per-packet in a cmsg seems not to be supported
	 * on Linux. We instead setsockopt() before sending it, this should be
	 * safe as only a single thread calls send(). */
	if (*dscp != packet->get_dscp(packet))
	{
		if (family == AF_INET)
		{
			u_int8_t ds4;

			ds4 = packet->get_dscp(packet) << 2;
			if (setsockopt(skt, SOL_IP, IP_TOS, &ds4, sizeof(ds4)) == 0)
			{
				*dscp = packet->get_dscp(packet);
			}
			else
			{
				DBG1(DBG_NET, "unable to set IP_TOS on socket: %s",
					 strerror(errno));
			}
		}
		else
		{
			u_int ds6;

			ds6 = packet->get_dscp(packet) << 2;
			if (setsockopt(skt, SOL_IPV6, IPV6_TCLASS, &ds6, sizeof(ds6)) == 0)
			{
				*dscp = packet->get_dscp(packet);
			}
			else
			{
				DBG1(DBG_NET, "unable to set IPV6_TCLASS on socket: %s",
					 strerror(errno));
			}
		}
	}

	memset(&msg, 0, sizeof(struct msghdr));
	msg.msg_name = dst->get_sockaddr(dst);;
	msg.msg_namelen = *dst->get_sockaddr_len(dst);
	iov.iov_base = data.ptr;
	iov.iov_len = data.len;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_flags = 0;

	if (this->set_source && !src->is_anyaddr(src))
	{
		if (family == AF_INET)
		{
#if defined(IP_PKTINFO) || defined(IP_SENDSRCADDR)
			struct in_addr *addr;
			struct sockaddr_in *sin;
#ifdef IP_PKTINFO
			char buf[CMSG_SPACE(sizeof(struct in_pktinfo))];
			struct in_pktinfo *pktinfo;
#elif defined(IP_SENDSRCADDR)
			char buf[CMSG_SPACE(sizeof(struct in_addr))];
#endif
			memset(buf, 0, sizeof(buf));
			msg.msg_control = buf;
			msg.msg_controllen = sizeof(buf);
			cmsg = CMSG_FIRSTHDR(&msg);
			cmsg->cmsg_level = SOL_IP;
#ifdef IP_PKTINFO
			cmsg->cmsg_type = IP_PKTINFO;
			cmsg->cmsg_len = CMSG_LEN(sizeof(struct in_pktinfo));
			pktinfo = (struct in_pktinfo*)CMSG_DATA(cmsg);
			addr = &pktinfo->ipi_spec_dst;
#elif defined(IP_SENDSRCADDR)
			cmsg->cmsg_type = IP_SENDSRCADDR;
			cmsg->cmsg_len = CMSG_LEN(sizeof(struct in_addr));
			addr = (struct in_addr*)CMSG_DATA(cmsg);
#endif
			sin = (struct sockaddr_in*)src->get_sockaddr(src);
			memcpy(addr, &sin->sin_addr, sizeof(struct in_addr));
#endif /* IP_PKTINFO || IP_SENDSRCADDR */
		}
#ifdef HAVE_IN6_PKTINFO
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
#endif /* HAVE_IN6_PKTINFO */
	}

	bytes_sent = sendmsg(skt, &msg, 0);

	if (bytes_sent != data.len)
	{
		DBG1(DBG_NET, "error writing to socket: %s", strerror(errno));
		return FAILED;
	}
	return SUCCESS;
}

METHOD(socket_t, get_port, u_int16_t,
	private_socket_default_socket_t *this, bool nat_t)
{
	return nat_t ? this->natt : this->port;
}

METHOD(socket_t, supported_families, socket_family_t,
	private_socket_default_socket_t *this)
{
	socket_family_t families = SOCKET_FAMILY_NONE;

	if (this->ipv4 != -1 || this->ipv4_natt != -1)
	{
		families |= SOCKET_FAMILY_IPV4;
	}
	if (this->ipv6 != -1 || this->ipv6_natt != -1)
	{
		families |= SOCKET_FAMILY_IPV6;
	}
	return families;
}

/**
 * open a socket to send and receive packets
 */
static int open_socket(private_socket_default_socket_t *this,
					   int family, u_int16_t *port)
{
	int on = TRUE;
	union {
		struct sockaddr sockaddr;
		struct sockaddr_in sin;
		struct sockaddr_in6 sin6;
	} addr;
	socklen_t addrlen;
	u_int sol, pktinfo = 0;
	int skt;

	memset(&addr, 0, sizeof(addr));
	addr.sockaddr.sa_family = family;
	/* precalculate constants depending on address family */
	switch (family)
	{
		case AF_INET:
			addr.sin.sin_addr.s_addr = htonl(INADDR_ANY);
			addr.sin.sin_port = htons(*port);
			addrlen = sizeof(addr.sin);
			sol = SOL_IP;
#ifdef IP_PKTINFO
			pktinfo = IP_PKTINFO;
#elif defined(IP_RECVDSTADDR)
			pktinfo = IP_RECVDSTADDR;
#endif
			break;
		case AF_INET6:
			memcpy(&addr.sin6.sin6_addr, &in6addr_any, sizeof(in6addr_any));
			addr.sin6.sin6_port = htons(*port);
			addrlen = sizeof(addr.sin6);
			sol = SOL_IPV6;
			pktinfo = IPV6_RECVPKTINFO;
			break;
		default:
			return -1;
	}

	skt = socket(family, SOCK_DGRAM, IPPROTO_UDP);
	if (skt < 0)
	{
		DBG1(DBG_NET, "could not open socket: %s", strerror(errno));
		return -1;
	}
	if (setsockopt(skt, SOL_SOCKET, SO_REUSEADDR, (void*)&on, sizeof(on)) < 0)
	{
		DBG1(DBG_NET, "unable to set SO_REUSEADDR on socket: %s", strerror(errno));
		close(skt);
		return -1;
	}

	/* bind the socket */
	if (bind(skt, &addr.sockaddr, addrlen) < 0)
	{
		DBG1(DBG_NET, "unable to bind socket: %s", strerror(errno));
		close(skt);
		return -1;
	}

	/* retrieve randomly allocated port if needed */
	if (*port == 0)
	{
		if (getsockname(skt, &addr.sockaddr, &addrlen) < 0)
		{
			DBG1(DBG_NET, "unable to determine port: %s", strerror(errno));
			close(skt);
			return -1;
		}
		switch (family)
		{
			case AF_INET:
				*port = ntohs(addr.sin.sin_port);
				break;
			case AF_INET6:
				*port = ntohs(addr.sin6.sin6_port);
				break;
		}
	}

	/* get additional packet info on receive */
	if (pktinfo > 0)
	{
		if (setsockopt(skt, sol, pktinfo, &on, sizeof(on)) < 0)
		{
			DBG1(DBG_NET, "unable to set IP_PKTINFO on socket: %s", strerror(errno));
			close(skt);
			return -1;
		}
	}
#ifdef SO_MARK
	{	/* set optional MARK on socket (requires CAP_NET_ADMIN) */
		char *fwmark;
		mark_t mark;

		fwmark = lib->settings->get_str(lib->settings,
							"%s.plugins.socket-default.fwmark", NULL, lib->ns);
		if (fwmark && mark_from_string(fwmark, &mark))
		{
			if (setsockopt(skt, SOL_SOCKET, SO_MARK, &mark.value,
						   sizeof(mark.value)) < 0)
			{
				DBG1(DBG_NET, "unable to set SO_MARK on socket: %s",
					 strerror(errno));
			}
		}
	}
#endif

	if (!hydra->kernel_interface->bypass_socket(hydra->kernel_interface,
												skt, family))
	{
		DBG1(DBG_NET, "installing IKE bypass policy failed");
	}

	/* enable UDP decapsulation for NAT-T sockets */
	if (port == &this->natt &&
		!hydra->kernel_interface->enable_udp_decap(hydra->kernel_interface,
												   skt, family, this->natt))
	{
		DBG1(DBG_NET, "enabling UDP decapsulation for %s on port %d failed",
			 family == AF_INET ? "IPv4" : "IPv6", this->natt);
	}

	return skt;
}

/**
 * Check if we should use the given family
 */
static bool use_family(int family)
{
	switch (family)
	{
		case AF_INET:
			return lib->settings->get_bool(lib->settings,
						"%s.plugins.socket-default.use_ipv4", TRUE, lib->ns);
		case AF_INET6:
			return lib->settings->get_bool(lib->settings,
						"%s.plugins.socket-default.use_ipv6", TRUE, lib->ns);
		default:
			return FALSE;
	}
}

/**
 * Open a socket pair (normal and NAT traversal) for a given address family
 */
static void open_socketpair(private_socket_default_socket_t *this, int family,
							int *skt, int *skt_natt, char *label)
{
	if (!use_family(family))
	{
		*skt = -1;
		*skt_natt = -1;
		return;
	}

	*skt = open_socket(this, family, &this->port);
	if (*skt == -1)
	{
		*skt_natt = -1;
		DBG1(DBG_NET, "could not open %s socket, %s disabled", label, label);
	}
	else
	{
		*skt_natt = open_socket(this, family, &this->natt);
		if (*skt_natt == -1)
		{
			DBG1(DBG_NET, "could not open %s NAT-T socket", label);
		}
	}
}

METHOD(socket_t, destroy, void,
	private_socket_default_socket_t *this)
{
	if (this->ipv4 != -1)
	{
		close(this->ipv4);
	}
	if (this->ipv4_natt != -1)
	{
		close(this->ipv4_natt);
	}
	if (this->ipv6 != -1)
	{
		close(this->ipv6);
	}
	if (this->ipv6_natt != -1)
	{
		close(this->ipv6_natt);
	}
	free(this);
}

/*
 * See header for description
 */
socket_default_socket_t *socket_default_socket_create()
{
	private_socket_default_socket_t *this;

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
		.port = lib->settings->get_int(lib->settings,
							"%s.port", CHARON_UDP_PORT, lib->ns),
		.natt = lib->settings->get_int(lib->settings,
							"%s.port_nat_t", CHARON_NATT_PORT, lib->ns),
		.max_packet = lib->settings->get_int(lib->settings,
							"%s.max_packet", PACKET_MAX_DEFAULT, lib->ns),
		.set_source = lib->settings->get_bool(lib->settings,
							"%s.plugins.socket-default.set_source", TRUE,
							lib->ns),
	);

	if (this->port && this->port == this->natt)
	{
		DBG1(DBG_NET, "IKE ports can't be equal, will allocate NAT-T "
			 "port randomly");
		this->natt = 0;
	}

	if ((this->port && this->port < 1024) || (this->natt && this->natt < 1024))
	{
		if (!lib->caps->check(lib->caps, CAP_NET_BIND_SERVICE))
		{
			/* required to bind ports < 1024 */
			DBG1(DBG_NET, "socket-default plugin requires CAP_NET_BIND_SERVICE "
				 "capability");
			destroy(this);
			return NULL;
		}
	}

	/* we allocate IPv6 sockets first as that will reserve randomly allocated
	 * ports also for IPv4. On OS X, we have to do it the other way round
	 * for the same effect. */
#ifdef __APPLE__
	open_socketpair(this, AF_INET, &this->ipv4, &this->ipv4_natt, "IPv4");
	open_socketpair(this, AF_INET6, &this->ipv6, &this->ipv6_natt, "IPv6");
#else /* !__APPLE__ */
	open_socketpair(this, AF_INET6, &this->ipv6, &this->ipv6_natt, "IPv6");
	open_socketpair(this, AF_INET, &this->ipv4, &this->ipv4_natt, "IPv4");
#endif /* __APPLE__ */

	if (this->ipv4 == -1 && this->ipv6 == -1)
	{
		DBG1(DBG_NET, "could not create any sockets");
		destroy(this);
		return NULL;
	}

	return &this->public;
}
