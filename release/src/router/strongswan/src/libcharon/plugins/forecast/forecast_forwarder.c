/*
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

#include "forecast_forwarder.h"

#include <errno.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <linux/socket.h>
#include <netinet/if_ether.h>
#include <linux/if_packet.h>
#include <linux/filter.h>
#include <sys/ioctl.h>
#include <ifaddrs.h>
#include <net/if.h>

#include <daemon.h>
#include <threading/thread.h>
#include <processing/jobs/callback_job.h>

#define BOOTP_SERVER_PORT 67
#define BOOTP_CLIENT_PORT 68

typedef struct private_forecast_forwarder_t private_forecast_forwarder_t;
typedef struct private_kernel_listener_t private_kernel_listener_t;

/**
 * Private data of registered kernel listener
 */
struct private_kernel_listener_t {

	/**
	 * Implements kernel_listener_t
	 */
	kernel_listener_t listener;

	/**
	 * Listener that knows active addresses
	 */
	forecast_listener_t *fc;

	/**
	 * current broadcast address of internal network
	 */
	uint32_t broadcast;

	/**
	 * LAN interface index
	 */
	int ifindex;

	/**
	 * Packet socket
	 */
	int pkt;

	/**
	 * RAW socket
	 */
	int raw;
};

/**
 * Private data of an forecast_forwarder_t object.
 */
struct private_forecast_forwarder_t {

	/**
	 * Public forecast_forwarder_t interface.
	 */
	forecast_forwarder_t public;

	/**
	 * Public kernel_listener_t interface.
	 */
	private_kernel_listener_t kernel;
};

/**
 * Send a broadcast/multicast packet to a network
 */
static void send_net(private_forecast_forwarder_t *this,
					 struct sockaddr_ll *addr, void *buf, size_t len)
{
	if (sendto(this->kernel.pkt, buf, len, 0,
			   (struct sockaddr*)addr, sizeof(*addr)) != len)
	{
		DBG1(DBG_NET, "forecast send_net() failed: %s", strerror(errno));
	}
}

/**
 * Send a broadcast/multicast packet to a peer
 */
static void send_peer(private_forecast_forwarder_t *this, uint32_t dst,
					  void *buf, size_t len, int mark)
{
	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_addr.s_addr = dst,
	};

	if (setsockopt(this->kernel.raw, SOL_SOCKET, SO_MARK,
				   &mark, sizeof(mark)) != 0)
	{
		DBG1(DBG_NET, "forecast setting SO_MARK failed: %s", strerror(errno));
	}
	if (sendto(this->kernel.raw, buf, len, 0,
			   (struct sockaddr*)&addr, sizeof(addr)) != len)
	{
		DBG1(DBG_NET, "forecast send_peer() failed: %s", strerror(errno));
	}
}

/**
 * Check if an IP packet is BOOTP/DHCP
 */
static bool is_bootp(void *buf, size_t len)
{
	struct __attribute__((__packed__)) {
		struct iphdr ip;
		struct udphdr udp;
	} *pkt = buf;

	if (len > sizeof(*pkt))
	{
		if (ntohs(pkt->udp.source) == BOOTP_CLIENT_PORT &&
			ntohs(pkt->udp.dest) == BOOTP_SERVER_PORT)
		{
			return TRUE;
		}
		if (ntohs(pkt->udp.source) == BOOTP_SERVER_PORT &&
			ntohs(pkt->udp.dest) == BOOTP_CLIENT_PORT)
		{
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * Broadcast/Multicast receiver
 */
static bool receive_casts(private_forecast_forwarder_t *this)
{
	struct __attribute__((packed)) {
		struct iphdr hdr;
		char data[2048];
	} buf;
	char *type;
	ssize_t len;
	u_int mark, origin = 0;
	host_t *src, *dst;
	traffic_selector_t *ts;
	enumerator_t *enumerator;
	struct sockaddr_ll addr;
	socklen_t alen = sizeof(addr);
	bool reinject;

	len = recvfrom(this->kernel.pkt, &buf, sizeof(buf), MSG_DONTWAIT,
				   (struct sockaddr*)&addr, &alen);
	if (len < 0)
	{
		if (errno != EAGAIN && errno != EWOULDBLOCK)
		{
			DBG1(DBG_NET, "receiving from forecast socket failed: %s",
				 strerror(errno));
		}
		return TRUE;
	}
	else if (len < sizeof(struct iphdr))
	{
		DBG1(DBG_NET, "received short forecast packet: %zd bytes", len);
		return TRUE;
	}
	if (is_bootp(&buf, len))
	{	/* don't forward DHCP broadcasts */
		return TRUE;
	}

	src = host_create_from_chunk(AF_INET, chunk_from_thing(buf.hdr.saddr), 0);
	dst = host_create_from_chunk(AF_INET, chunk_from_thing(buf.hdr.daddr), 0);

	/* create valid broadcast/multicast MAC to send out */
	if (IN_MULTICAST(ntohl(buf.hdr.daddr)))
	{
		type = "multi";
		ETHER_MAP_IP_MULTICAST(&buf.hdr.daddr, addr.sll_addr);
	}
	else
	{
		type = "broad";
		memset(&addr.sll_addr, 0xFF, sizeof(addr.sll_addr));
	}
	DBG2(DBG_NET, "forecast intercepted packet: %H to %H", src, dst);

	/* find mark of originating tunnel */
	enumerator = this->kernel.fc->create_enumerator(this->kernel.fc, FALSE);
	while (enumerator->enumerate(enumerator, &ts, &mark, &reinject))
	{
		if (ts->includes(ts, src))
		{
			origin = mark;
			break;
		}
	}
	enumerator->destroy(enumerator);

	/* send packet over all tunnels, but not the packets origin */
	enumerator = this->kernel.fc->create_enumerator(this->kernel.fc, FALSE);
	while (enumerator->enumerate(enumerator, &ts, &mark, &reinject))
	{
		if (ts->includes(ts, dst))
		{
			if ((reinject && origin != mark) || origin == 0)
			{
				DBG2(DBG_NET, "forwarding a %H %scast from %H to peer %R (%u)",
					 dst, type, src, ts, mark);
				send_peer(this, buf.hdr.daddr, &buf, len, mark);
			}
		}
	}
	enumerator->destroy(enumerator);

	if (origin)
	{
		/* forward broadcast/multicast from client to network */
		DBG2(DBG_NET, "forwarding a %H %scast from peer %H to internal network",
			 dst, type, src);
		addr.sll_ifindex = this->kernel.ifindex;
		send_net(this, &addr, &buf, len);
	}

	dst->destroy(dst);
	src->destroy(src);

	return TRUE;
}

/**
 * Join a multicast group
 */
static void join_group(private_kernel_listener_t *this, char *group,
					   struct sockaddr *addr)
{
	struct sockaddr_in *in;
	struct ip_mreqn mreq;
	host_t *host;

	host = host_create_from_string(group, 0);
	if (host)
	{
		memset(&mreq, 0, sizeof(mreq));
		memcpy(&mreq.imr_multiaddr.s_addr, host->get_address(host).ptr, 4);
		if (addr->sa_family == AF_INET)
		{
			in = (struct sockaddr_in*)addr;
			memcpy(&mreq.imr_address, &in->sin_addr.s_addr,
				   sizeof(in->sin_addr.s_addr));
		}
		mreq.imr_ifindex = this->ifindex;
		if (setsockopt(this->raw, IPPROTO_IP, IP_ADD_MEMBERSHIP,
					   &mreq, sizeof(mreq)) == -1)
		{
			if (errno != EADDRINUSE)
			{
				DBG1(DBG_NET, "forecast multicast join to %s failed: %s",
					 group, strerror(errno));
			}
		}
		else
		{
			DBG2(DBG_NET, "forwarding multicast group %s", group);
		}
		host->destroy(host);
	}
}

/**
 * (Re-)Join all multicast groups we want to forward
 */
static void join_groups(private_kernel_listener_t *this, struct sockaddr *addr)
{
	enumerator_t *enumerator;
	char *groups, *group;
	static char *def =
		"224.0.0.1,"		/* host multicast */
		"224.0.0.22,"		/* IGMP */
		"224.0.0.251,"		/* mDNS */
		"224.0.0.252,"		/* LLMNR */
		"239.255.255.250";	/* SSDP/WS-discovery */

	groups = lib->settings->get_str(lib->settings,
									"%s.plugins.forecast.groups", def, lib->ns);
	DBG1(DBG_CFG, "joining forecast multicast groups: %s", groups);
	enumerator = enumerator_create_token(groups, ",", " ");
	while (enumerator->enumerate(enumerator, &group))
	{
		join_group(this, group, addr);
	}
	enumerator->destroy(enumerator);
}

/**
 * Attach the socket filter to the socket
 */
static bool attach_filter(int fd, uint32_t broadcast)
{
	struct sock_filter filter_code[] = {
		/* destination address: is ... */
		BPF_STMT(BPF_LD+BPF_W+BPF_ABS, offsetof(struct iphdr, daddr)),
		/* broadcast, as received from the local network */
		BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, ntohl(broadcast), 4, 0),
		/* broadcast, as Win7 sends them */
		BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, 0xFFFFFFFF, 3, 0),
		/* any multicast, 224.0.0.0/4 */
		BPF_STMT(BPF_ALU+BPF_AND+BPF_K, 0xF0000000),
		BPF_JUMP(BPF_JMP+BPF_JEQ+BPF_K, 0xE0000000, 1, 0),
		BPF_STMT(BPF_RET+BPF_K, 0),
		BPF_STMT(BPF_LD+BPF_W+BPF_LEN, 0),
		BPF_STMT(BPF_RET+BPF_A, 0),
	};
	struct sock_fprog filter = {
		sizeof(filter_code) / sizeof(struct sock_filter),
		filter_code,
	};

	if (setsockopt(fd, SOL_SOCKET, SO_ATTACH_FILTER,
				   &filter, sizeof(filter)) < 0)
	{
		DBG1(DBG_NET, "installing forecast PACKET socket filter failed: %s",
			 strerror(errno));
		return FALSE;
	}
	return TRUE;
}

/**
 * Get the interface index of an interface name
 */
static int get_ifindex(private_kernel_listener_t *this, char *ifname)
{
	struct ifreq ifr = {};

	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	if (ioctl(this->raw, SIOCGIFINDEX, &ifr) == 0)
	{
		return ifr.ifr_ifindex;
	}
	return 0;
}

/**
 * Set up the interface for broad/multicast forwarding
 */
static void setup_interface(private_kernel_listener_t *this)
{
	struct ifaddrs *addrs, *current;
	struct sockaddr_in *in;
	host_t *host;
	char *name;

	name = lib->settings->get_str(lib->settings,
							"%s.plugins.forecast.interface", NULL, lib->ns);
	if (getifaddrs(&addrs) == 0)
	{
		for (current = addrs; current; current = current->ifa_next)
		{
			if (name && !streq(name, current->ifa_name))
			{
				continue;
			}
			if (current->ifa_flags & IFF_BROADCAST &&
				current->ifa_broadaddr &&
				current->ifa_broadaddr->sa_family == AF_INET)
			{
				DBG1(DBG_NET, "using forecast interface %s", current->ifa_name);
				this->ifindex = get_ifindex(this, current->ifa_name);
				in = (struct sockaddr_in*)current->ifa_broadaddr;
				attach_filter(this->pkt, in->sin_addr.s_addr);
				join_groups(this, current->ifa_addr);
				host = host_create_from_sockaddr(current->ifa_broadaddr);
				if (host)
				{
					this->fc->set_broadcast(this->fc, host);
					host->destroy(host);
				}
				break;
			}
		}
	}
	freeifaddrs(addrs);
}

METHOD(kernel_listener_t, roam, bool,
	private_kernel_listener_t *this, bool address)
{
	if (address)
	{
		setup_interface(this);
	}
	return TRUE;
}

METHOD(forecast_forwarder_t, destroy, void,
	private_forecast_forwarder_t *this)
{
	if (this->kernel.raw != -1)
	{
		close(this->kernel.raw);
	}
	if (this->kernel.pkt != -1)
	{
		lib->watcher->remove(lib->watcher, this->kernel.pkt);
		close(this->kernel.pkt);
	}
	charon->kernel->remove_listener(charon->kernel, &this->kernel.listener);
	free(this);
}

/**
 * See header
 */
forecast_forwarder_t *forecast_forwarder_create(forecast_listener_t *listener)
{
	private_forecast_forwarder_t *this;
	int on = 1;

	INIT(this,
		.public = {
			.destroy = _destroy,
		},
		.kernel = {
			.listener = {
				.roam = _roam,
			},
			.raw = -1,
			.pkt = -1,
			.fc = listener,
		},
	);

	this->kernel.pkt = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_IP));
	if (this->kernel.pkt == -1)
	{
		DBG1(DBG_NET, "opening PACKET socket failed: %s", strerror(errno));
		destroy(this);
		return NULL;
	}
	this->kernel.raw = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
	if (this->kernel.raw == -1)
	{
		DBG1(DBG_NET, "opening RAW socket failed: %s", strerror(errno));
		destroy(this);
		return NULL;
	}
	if (setsockopt(this->kernel.raw, IPPROTO_IP, IP_HDRINCL,
				   &on, sizeof(on)) == -1)
	{
		DBG1(DBG_NET, "forecast socket HDRINCL failed: %s", strerror(errno));
		destroy(this);
		return NULL;
	}
	if (setsockopt(this->kernel.raw, SOL_SOCKET, SO_BROADCAST,
				   &on, sizeof(on)) == -1)
	{
		DBG1(DBG_NET, "forecast socket BROADCAST failed: %s", strerror(errno));
		destroy(this);
		return NULL;
	}

	setup_interface(&this->kernel);

	charon->kernel->add_listener(charon->kernel,
										   &this->kernel.listener);

	lib->watcher->add(lib->watcher, this->kernel.pkt, WATCHER_READ,
					  (watcher_cb_t)receive_casts, this);

	return &this->public;
}
