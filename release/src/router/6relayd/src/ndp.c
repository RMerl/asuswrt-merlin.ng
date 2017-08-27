/**
 * Copyright (C) 2012-2013 Steven Barth <steven@midlink.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/ethernet.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <netpacket/packet.h>

#include <linux/rtnetlink.h>
#include <linux/filter.h>
#include "router.h"
#include "ndp.h"


static const struct relayd_config *config = NULL;

static void handle_solicit(void *addr, void *data, size_t len,
		struct relayd_interface *iface);
static void handle_advert(void *addr, void *data, size_t len,
		struct relayd_interface *iface);
static void handle_neighbor(void *addr, void *data, size_t len,
		struct relayd_interface *iface);
static void handle_rtnetlink(void *addr, void *data, size_t len,
		struct relayd_interface *iface);
static struct ndp_neighbor* find_neighbor(struct in6_addr *addr,
		struct relayd_interface *iface, uint8_t strict);
static void modify_neighbor(struct in6_addr *addr, struct relayd_interface *iface,
		bool add, bool is_addr);
static ssize_t send_solicit(struct in6_addr *addr,
		uint8_t *mac, const struct relayd_interface *iface);
static ssize_t send_advert(struct in6_addr *addr, struct in6_addr *source,
		uint8_t *mac, const struct relayd_interface *iface);
static ssize_t ping6(struct in6_addr *addr,
		const struct relayd_interface *iface);

static struct list_head neighbors = LIST_HEAD_INIT(neighbors);
static struct list_head addresses = LIST_HEAD_INIT(addresses);
static size_t neighbor_count = 0;
static uint32_t rtnl_seqid = 0;

static int ping_socket = -1;
static struct relayd_event ndp_event_solicit = {-1, NULL, handle_neighbor};
static struct relayd_event rtnl_event = {-1, NULL, handle_rtnetlink};


// Filter ICMPv6 messages of type neighbor soliciation
static struct sock_filter bpf[] = {
	BPF_STMT(BPF_LD | BPF_B | BPF_ABS, offsetof(struct ip6_hdr, ip6_nxt)),
	BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, IPPROTO_ICMPV6, 0, 5),
	BPF_STMT(BPF_LD | BPF_B | BPF_ABS, offsetof(struct ip6_hdr, ip6_hlim)),
	BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, 255, 0, 3),
	BPF_STMT(BPF_LD | BPF_B | BPF_ABS, sizeof(struct ip6_hdr) +
			offsetof(struct icmp6_hdr, icmp6_type)),
	BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, ND_NEIGHBOR_SOLICIT, 2, 0),
	BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, ND_NEIGHBOR_ADVERT, 1, 0),
	BPF_STMT(BPF_RET | BPF_K, 0),
	BPF_STMT(BPF_RET | BPF_K, 0x7fffffff)
};
static const struct sock_fprog bpf_prog = {sizeof(bpf) / sizeof(*bpf), bpf};


// Initialize NDP-proxy
int init_ndp_proxy(const struct relayd_config *relayd_config)
{
	config = relayd_config;
	if (config->slavecount < 1)
		return 0;

	// Setup netlink socket
	if ((rtnl_event.socket = relayd_open_rtnl_socket()) < 0)
		return -1;

	// Receive netlink neighbor and ip-address events
	uint32_t group = RTNLGRP_IPV6_IFADDR;
	setsockopt(rtnl_event.socket, SOL_NETLINK,
			NETLINK_ADD_MEMBERSHIP, &group, sizeof(group));
	group = RTNLGRP_IPV6_ROUTE;
	setsockopt(rtnl_event.socket, SOL_NETLINK,
			NETLINK_ADD_MEMBERSHIP, &group, sizeof(group));

	// Synthesize initial address events
	struct {
		struct nlmsghdr nh;
		struct ifaddrmsg ifa;
	} req2 = {
		{sizeof(req2), RTM_GETADDR, NLM_F_REQUEST | NLM_F_DUMP,
				++rtnl_seqid, 0},
		{.ifa_family = AF_INET6}
	};
	send(rtnl_event.socket, &req2, sizeof(req2), MSG_DONTWAIT);

	relayd_register_event(&rtnl_event);



	// Test if disabled
	if (!config->enable_ndp_relay || config->slavecount < 1)
		return 0;

	for (size_t i = 0; i < config->static_ndp_len; ++i) {
		struct ndp_neighbor *n = malloc(sizeof(*n));
		n->timeout = 0;

		char ipbuf[INET6_ADDRSTRLEN];
		char iface[16];

		if (sscanf(config->static_ndp[i], "%45s/%hhu:%15s", ipbuf, &n->len, iface) < 3
				|| n->len > 128 || inet_pton(AF_INET6, ipbuf, &n->addr) != 1 ||
				!(n->iface = relayd_get_interface_by_name(iface))) {
			syslog(LOG_ERR, "Invalid static NDP-prefix %s", config->static_ndp[i]);
			return -1;
		}

		list_add(&n->head, &neighbors);
	}

#if defined(SOCK_CLOEXEC) && defined(SOCK_NONBLOCK)
	// Create socket for intercepting NDP
	int sock = socket(AF_PACKET, SOCK_DGRAM | SOCK_CLOEXEC | SOCK_NONBLOCK,
			htons(ETH_P_ALL)); // ETH_P_ALL for ingress + egress
#else
	int sock = socket(AF_PACKET, SOCK_DGRAM,
			htons(ETH_P_ALL)); // ETH_P_ALL for ingress + egress
	sock = fflags(sock, O_CLOEXEC | O_NONBLOCK);
#endif
	if (sock < 0) {
		syslog(LOG_ERR, "Unable to open packet socket: %s",
				strerror(errno));
		return -1;
	}

	if (setsockopt(sock, SOL_SOCKET, SO_ATTACH_FILTER,
			&bpf_prog, sizeof(bpf_prog))) {
		syslog(LOG_ERR, "Failed to set BPF: %s", strerror(errno));
		return -1;
	}


	struct packet_mreq mreq = {config->master.ifindex,
			PACKET_MR_ALLMULTI, ETH_ALEN, {0}};
	setsockopt(sock, SOL_PACKET, PACKET_ADD_MEMBERSHIP,
			&mreq, sizeof(mreq));

	for (size_t i = 0; i < config->slavecount; ++i) {
		mreq.mr_ifindex = config->slaves[i].ifindex;
		setsockopt(sock, SOL_PACKET, PACKET_ADD_MEMBERSHIP,
				&mreq, sizeof(mreq));
	}

	ndp_event_solicit.socket = sock;
	relayd_register_event(&ndp_event_solicit);

	// Open ICMPv6 socket
#ifdef SOCK_CLOEXEC
	ping_socket = socket(AF_INET6, SOCK_RAW | SOCK_CLOEXEC, IPPROTO_ICMPV6);
#else
	ping_socket = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
	ping_socket = fflags(ping_socket, O_CLOEXEC);
#endif

	int val = 2;
	setsockopt(ping_socket, IPPROTO_RAW, IPV6_CHECKSUM, &val, sizeof(val));

	// This is required by RFC 4861
	val = 255;
	setsockopt(ping_socket, IPPROTO_IPV6, IPV6_MULTICAST_HOPS,
			&val, sizeof(val));
	setsockopt(ping_socket, IPPROTO_IPV6, IPV6_UNICAST_HOPS,
			&val, sizeof(val));

	// Filter all packages, we only want to send
	struct icmp6_filter filt;
	ICMP6_FILTER_SETBLOCKALL(&filt);
	setsockopt(ping_socket, IPPROTO_ICMPV6, ICMP6_FILTER,
			&filt, sizeof(filt));


	// Netlink socket, continued...
	relayd_receive_packets(&rtnl_event);

	group = RTNLGRP_NEIGH;
	setsockopt(rtnl_event.socket, SOL_NETLINK,
			NETLINK_ADD_MEMBERSHIP, &group, sizeof(group));

	// Synthesize initial neighbor events
	struct {
		struct nlmsghdr nh;
		struct ndmsg ndm;
	} req = {
		{sizeof(req), RTM_GETNEIGH, NLM_F_REQUEST | NLM_F_DUMP,
				++rtnl_seqid, 0},
		{.ndm_family = AF_INET6}
	};
	send(rtnl_event.socket, &req, sizeof(req), MSG_DONTWAIT);

	relayd_receive_packets(&rtnl_event);

	return 0;
}


// Deinitialize NDP proxy
void deinit_ndp_proxy()
{
	while (!list_empty(&neighbors)) {
		struct ndp_neighbor *c = list_first_entry(&neighbors,
				struct ndp_neighbor, head);
		modify_neighbor(&c->addr, c->iface, false, c->is_addr);
	}
	while (!list_empty(&addresses)) {
		struct ndp_neighbor *c = list_first_entry(&addresses,
				struct ndp_neighbor, head);
		modify_neighbor(&c->addr, c->iface, false, c->is_addr);
	}
}


// Send solicitation
static ssize_t send_solicit(struct in6_addr *addr,
		uint8_t *mac, const struct relayd_interface *iface)
{
	struct sockaddr_in6 dest = {AF_INET6, 0, 0, ALL_IPV6_NODES, 0};
	struct nd_neighbor_solicit solicit = {
		.nd_ns_hdr = {ND_NEIGHBOR_SOLICIT, 0, 0, {{0}}},
		.nd_ns_target = *addr
	};
	struct {
		struct nd_opt_hdr hdr;
		uint8_t mac[6];
	} nd_opt_ll = {
		.hdr = {ND_OPT_SOURCE_LINKADDR, 1}
	};
	struct iovec iov[2] = {
		{&solicit, sizeof(solicit)},
		{&nd_opt_ll, sizeof(nd_opt_ll)}
	};

	if (iface->nondp)
		iov[1].iov_len = 0;
	else if (mac)
		memcpy(nd_opt_ll.mac, mac, sizeof(nd_opt_ll.mac));
	else
		relayd_get_interface_mac(iface->ifname, nd_opt_ll.mac);

	// Linux seems to not honor IPV6_PKTINFO on raw-sockets, so work around
	setsockopt(ping_socket, SOL_SOCKET, SO_BINDTODEVICE,
			iface->ifname, sizeof(iface->ifname));
	return relayd_forward_packet(ping_socket, &dest, iov, ARRAY_SIZE(iov), iface);
}


// Send advert
static ssize_t send_advert(struct in6_addr *addr, struct in6_addr *source,
		uint8_t *mac, const struct relayd_interface *iface)
{
	struct sockaddr_in6 dest = {AF_INET6, 0, 0, ALL_IPV6_NODES, 0};
	struct nd_neighbor_advert advert = {
		.nd_na_hdr = {ND_NEIGHBOR_ADVERT, 0, 0, {{0}}},
		.nd_na_target = *addr
	};
	struct {
		struct nd_opt_hdr hdr;
		uint8_t mac[6];
	} nd_opt_ll = {
		.hdr = {ND_OPT_TARGET_LINKADDR, 1}
	};
	struct iovec iov[2] = {
		{&advert, sizeof(advert)},
		{&nd_opt_ll, sizeof(nd_opt_ll)}
	};

	advert.nd_na_flags_reserved = ND_NA_FLAG_ROUTER |
		(source ? ND_NA_FLAG_SOLICITED : 0);

	if (iface->nondp)
		iov[1].iov_len = 0;
	else if (mac)
		memcpy(nd_opt_ll.mac, mac, sizeof(nd_opt_ll.mac));
	else
		relayd_get_interface_mac(iface->ifname, nd_opt_ll.mac);

	// If not DAD, then unicast to source
	if (source && !IN6_IS_ADDR_UNSPECIFIED(source)) {
		dest.sin6_addr = *source;
		dest.sin6_scope_id = iface->ifindex;
	}

	// Linux seems to not honor IPV6_PKTINFO on raw-sockets, so work around
	setsockopt(ping_socket, SOL_SOCKET, SO_BINDTODEVICE,
				iface->ifname, sizeof(iface->ifname));
	return relayd_forward_packet(ping_socket, &dest, iov, ARRAY_SIZE(iov), iface);
}


// Send an ICMP-ECHO. This is less for actually pinging but for the
// neighbor cache to be kept up-to-date.
static ssize_t ping6(struct in6_addr *addr,
		const struct relayd_interface *iface)
{
	// Send solicit directly
	if (iface->nondp)
		return (send_solicit(addr, NULL, iface) > 0);

	struct sockaddr_in6 dest = {AF_INET6, 0, 0, *addr, iface->ifindex};
	struct icmp6_hdr echo = {.icmp6_type = ICMP6_ECHO_REQUEST};
	struct iovec iov = {&echo, sizeof(echo)};

	// Linux seems to not honor IPV6_PKTINFO on raw-sockets, so work around
	setsockopt(ping_socket, SOL_SOCKET, SO_BINDTODEVICE,
			iface->ifname, sizeof(iface->ifname));
	return (relayd_forward_packet(ping_socket, &dest, &iov, 1, iface) > 0);
}


// Handle solicitations
static void handle_solicit(void *addr, void *data, size_t len,
		struct relayd_interface *iface)
{
	struct ip6_hdr *ip6 = data;
	struct nd_neighbor_solicit *req = (struct nd_neighbor_solicit*)&ip6[1];
	struct sockaddr_ll *ll = addr;

	if (iface->nondp && ll->sll_pkttype == PACKET_OUTGOING)
		return; // Own solicitations to non-NDP link

	// Solicitation is for duplicate address detection
	bool ns_is_dad = IN6_IS_ADDR_UNSPECIFIED(&ip6->ip6_src);

	// Don't forward any non-DAD solicitation for external ifaces
	// TODO: check if we should even forward DADs for them
	if (iface->external && !ns_is_dad)
		return;

	if (len < sizeof(*ip6) + sizeof(*req))
		return; // Invalid reqicitation

	if (IN6_IS_ADDR_LINKLOCAL(&req->nd_ns_target) ||
			IN6_IS_ADDR_LOOPBACK(&req->nd_ns_target) ||
			IN6_IS_ADDR_MULTICAST(&req->nd_ns_target))
		return; // Invalid target

	char ipbuf[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET6, &req->nd_ns_target, ipbuf, sizeof(ipbuf));
	syslog(LOG_NOTICE, "Got a NS for %s", ipbuf);

	uint8_t mac[6];
	relayd_get_interface_mac(iface->ifname, mac);
	if (!memcmp(ll->sll_addr, mac, sizeof(mac)) &&
			ll->sll_pkttype != PACKET_OUTGOING)
		return; // Looped back

	time_t now = relayd_monotonic_time();

	struct ndp_neighbor *n = find_neighbor(&req->nd_ns_target, iface, 0);
	if (n && (n->iface || abs(n->timeout - now) < 5)) {
		syslog(LOG_NOTICE, "%s is on %s", ipbuf,
				(n->iface) ? n->iface->ifname : "<pending>");
		if (!n->iface || n->iface == iface ||
				ll->sll_pkttype == PACKET_OUTGOING)
			return;

		// Found on other interface, answer with advertisement
		send_advert(&req->nd_ns_target, &ip6->ip6_src, mac, iface);
	} else {
		// Send echo to all other interfaces to see where target is on
		// This will trigger neighbor discovery which is what we want.
		// We will observe the neighbor cache to see results.

		ssize_t sent = 0;
		if (iface != &config->master)
			sent += ping6(&req->nd_ns_target, &config->master);

		for (size_t i = 0; i < config->slavecount; ++i)
			if ((!ns_is_dad || config->slaves[i].external == false)
					&& iface != &config->slaves[i])
				sent += ping6(&req->nd_ns_target,
						&config->slaves[i]);

		if (sent > 0) // Sent a ping, add pending neighbor entry
			modify_neighbor(&req->nd_ns_target, NULL, true, false);
	}
}


// Handle advertisements
static void handle_advert(void *addr, void *data, size_t len,
		struct relayd_interface *iface)
{
	struct ip6_hdr *ip6 = data;
	struct nd_neighbor_advert *req = (struct nd_neighbor_advert*)&ip6[1];
	struct sockaddr_ll *ll = addr;

	if (!iface->nondp || ll->sll_pkttype == PACKET_OUTGOING)
		return; // Own or kernel-driven adverts

	if (len < sizeof(*ip6) + sizeof(*req))
		return; // Invalid reqicitation

	if (IN6_IS_ADDR_MULTICAST(&ip6->ip6_dst) &&
	    (req->nd_na_flags_reserved & ND_NA_FLAG_SOLICITED))
		return; // Invalid unsolicited

	if (IN6_IS_ADDR_LINKLOCAL(&req->nd_na_target) ||
			IN6_IS_ADDR_LOOPBACK(&req->nd_na_target) ||
			IN6_IS_ADDR_MULTICAST(&req->nd_na_target))
		return; // Invalid target

	char ipbuf[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET6, &req->nd_na_target, ipbuf, sizeof(ipbuf));
	syslog(LOG_NOTICE, "Got a NA for %s", ipbuf);

	uint8_t mac[6];
	relayd_get_interface_mac(iface->ifname, mac);
	if (!memcmp(ll->sll_addr, mac, sizeof(mac)) &&
			ll->sll_pkttype != PACKET_OUTGOING)
		return; // Looped back

	modify_neighbor(&req->nd_na_target, iface, true, false);
}


// Handle neighbor discovery
static void handle_neighbor(void *addr, void *data, size_t len,
		struct relayd_interface *iface)
{
	struct ip6_hdr *ip6 = data;
	struct icmp6_hdr *req = (struct icmp6_hdr*)&ip6[1];

	if (len < sizeof(*ip6) + sizeof(*req))
		return; // Invalid reqicitation

	switch (req->icmp6_type) {
	case ND_NEIGHBOR_SOLICIT:
		handle_solicit(addr, data, len, iface);
		break;
	case ND_NEIGHBOR_ADVERT:
		handle_advert(addr, data, len, iface);
		break;
	}
}


void relayd_setup_route(const struct in6_addr *addr, int prefixlen,
		const struct relayd_interface *iface, const struct in6_addr *gw,
		int metric, bool add)
{
	struct req {
		struct nlmsghdr nh;
		struct rtmsg rtm;
		struct rtattr rta_dst;
		struct in6_addr dst_addr;
		struct rtattr rta_oif;
		uint32_t ifindex;
		struct rtattr rta_table;
		uint32_t table;
		struct rtattr rta_prio;
		uint32_t prio;
		struct rtattr rta_gw;
		struct in6_addr gw;
	} req = {
		{sizeof(req), 0, NLM_F_REQUEST, ++rtnl_seqid, 0},
		{AF_INET6, prefixlen, 0, 0, 0, 0, 0, 0, 0},
		{sizeof(struct rtattr) + sizeof(struct in6_addr), RTA_DST},
		*addr,
		{sizeof(struct rtattr) + sizeof(uint32_t), RTA_OIF},
		iface->ifindex,
		{sizeof(struct rtattr) + sizeof(uint32_t), RTA_TABLE},
		RT_TABLE_MAIN,
		{sizeof(struct rtattr) + sizeof(uint32_t), RTA_PRIORITY},
		metric,
		{sizeof(struct rtattr) + sizeof(struct in6_addr), RTA_GATEWAY},
		IN6ADDR_ANY_INIT,
	};

	if (gw)
		req.gw = *gw;

	if (add) {
		req.nh.nlmsg_type = RTM_NEWROUTE;
		req.nh.nlmsg_flags |= (NLM_F_CREATE | NLM_F_REPLACE);
		req.rtm.rtm_protocol = RTPROT_STATIC;
		req.rtm.rtm_scope = (gw) ? RT_SCOPE_UNIVERSE : RT_SCOPE_LINK;
		req.rtm.rtm_type = RTN_UNICAST;
	} else {
		req.nh.nlmsg_type = RTM_DELROUTE;
		req.rtm.rtm_scope = RT_SCOPE_NOWHERE;
	}

	req.nh.nlmsg_len = (gw) ? sizeof(req) : offsetof(struct req, rta_gw);
	send(rtnl_event.socket, &req, req.nh.nlmsg_len, MSG_DONTWAIT);
}

// Use rtnetlink to modify kernel routes
static void setup_route(struct in6_addr *addr, struct relayd_interface *iface,
		bool add, bool is_addr)
{
	char namebuf[INET6_ADDRSTRLEN];
	inet_ntop(AF_INET6, addr, namebuf, sizeof(namebuf));
	syslog(LOG_NOTICE, "%s about %s on %s", (add) ? "Learned" : "Forgot",
			namebuf, (iface) ? iface->ifname : "<pending>");

	if (!iface || !config->enable_route_learning || is_addr)
		return;

	relayd_setup_route(addr, 128, iface, NULL, 1024, add);
}

static void free_neighbor(struct ndp_neighbor *n)
{
	setup_route(&n->addr, n->iface, false, n->is_addr);
	list_del(&n->head);
	free(n);
	--neighbor_count;
}


static bool match_neighbor(struct ndp_neighbor *n, struct in6_addr *addr)
{
	if (n->len <= 32)
		return ntohl(n->addr.s6_addr32[0]) >> (32 - n->len) ==
				ntohl(addr->s6_addr32[0]) >> (32 - n->len);

	if (n->addr.s6_addr32[0] != addr->s6_addr32[0])
		return false;

	if (n->len <= 64)
		return ntohl(n->addr.s6_addr32[1]) >> (64 - n->len) ==
				ntohl(addr->s6_addr32[1]) >> (64 - n->len);

	if (n->addr.s6_addr32[1] != addr->s6_addr32[1])
		return false;

	if (n->len <= 96)
		return ntohl(n->addr.s6_addr32[2]) >> (96 - n->len) ==
				ntohl(addr->s6_addr32[2]) >> (96 - n->len);

	if (n->addr.s6_addr32[2] != addr->s6_addr32[2])
		return false;

	return ntohl(n->addr.s6_addr32[3]) >> (128 - n->len) ==
			ntohl(addr->s6_addr32[3]) >> (128 - n->len);
}


static struct ndp_neighbor* find_neighbor(struct in6_addr *addr,
		struct relayd_interface *iface, uint8_t strict)
{
	time_t now = relayd_monotonic_time();
	struct ndp_neighbor *n, *e = NULL;

	if (!strict || (strict & NDP_F_ADDR))
	list_for_each_entry(n, &addresses, head) {
		if (n->len == 128 && IN6_ARE_ADDR_EQUAL(&n->addr, addr)) {
			if (n->iface == iface)
				return n;
			else if (!e)
				e = n;
		}
	}
	if (e)
		return e;

	if (!strict || (strict & NDP_F_NEIGH))
	list_for_each_entry_safe(n, e, &neighbors, head) {
		if ((!strict && match_neighbor(n, addr)) ||
				(n->len == 128 && IN6_ARE_ADDR_EQUAL(&n->addr, addr)))
			return n;

		if (!n->iface && abs(n->timeout - now) >= 5)
			free_neighbor(n);
	}
	return NULL;
}


// Modified our own neighbor-entries
static void modify_neighbor(struct in6_addr *addr,
		struct relayd_interface *iface, bool add, bool is_addr)
{
	if (!addr || (void*)addr == (void*)iface)
		return;

	struct ndp_neighbor *n = find_neighbor(addr, iface,
			is_addr ? NDP_F_ADDR : NDP_F_NEIGH);
	if (!add) { // Delete action
		if (n && (!n->iface || n->iface == iface))
			free_neighbor(n);
	} else if (!n || // No entry yet, add one if possible
			(n->is_addr && is_addr && n->iface != iface)) {
		if (neighbor_count >= NDP_MAX_NEIGHBORS ||
				!(n = malloc(sizeof(*n))))
			return;

		n->len = 128;
		n->addr = *addr;
		n->iface = iface;
		if (!n->iface)
			n->timeout = relayd_monotonic_time();
		n->is_addr = is_addr;
		list_add(&n->head, is_addr ? &addresses : &neighbors);
		++neighbor_count;
		setup_route(addr, n->iface, add, n->is_addr);
	} else if (n->iface == iface) {
		if (!n->iface)
			n->timeout = relayd_monotonic_time();
	} else if (iface && (!n->iface ||
			(!iface->external && n->iface->external))) {
		setup_route(addr, n->iface, false, n->is_addr);
		n->iface = iface;
		n->is_addr = is_addr;
		setup_route(addr, n->iface, add, n->is_addr);
	}
	// TODO: In case a host switches interfaces we might want
	// to set its old neighbor entry to NUD_STALE and ping it
	// on the old interface to confirm if the MACs match.
}


// Handler for neighbor cache entries from the kernel. This is our source
// to learn and unlearn hosts on interfaces.
static void handle_rtnetlink(_unused void *addr, void *data, size_t len,
		_unused struct relayd_interface *iface)
{
	for (struct nlmsghdr *nh = data; NLMSG_OK(nh, len);
			nh = NLMSG_NEXT(nh, len)) {
		struct rtmsg *rtm = NLMSG_DATA(nh);
		if (config->enable_router_discovery_server &&
				(nh->nlmsg_type == RTM_NEWROUTE ||
				nh->nlmsg_type == RTM_DELROUTE) &&
				rtm->rtm_dst_len == 0)
			raise(SIGUSR1); // Inform about a change in default route

		struct ndmsg *ndm = NLMSG_DATA(nh);
		struct ifaddrmsg *ifa = NLMSG_DATA(nh);
		if (nh->nlmsg_type != RTM_NEWNEIGH
				&& nh->nlmsg_type != RTM_DELNEIGH
				&& nh->nlmsg_type != RTM_NEWADDR
				&& nh->nlmsg_type != RTM_DELADDR)
			continue; // Unrelated message type
		bool is_addr = (nh->nlmsg_type == RTM_NEWADDR
				|| nh->nlmsg_type == RTM_DELADDR);

		// Family and ifindex are on the same offset for NEIGH and ADDR
		if (NLMSG_PAYLOAD(nh, 0) < sizeof(*ndm)
				|| ndm->ndm_family != AF_INET6)
			continue; //

		// Lookup interface
		struct relayd_interface *iface;
		if (!(iface = relayd_get_interface_by_index(ndm->ndm_ifindex)))
			continue;

		// Data to retrieve
		size_t rta_offset = (is_addr) ? sizeof(*ifa) : sizeof(*ndm);
		uint16_t atype = (is_addr) ? IFA_ADDRESS : NDA_DST;
		ssize_t alen = NLMSG_PAYLOAD(nh, rta_offset);
		struct in6_addr *addr = NULL;

		for (struct rtattr *rta = (void*)(((uint8_t*)ndm) + rta_offset);
				RTA_OK(rta, alen); rta = RTA_NEXT(rta, alen))
			if (rta->rta_type == atype &&
					RTA_PAYLOAD(rta) >= sizeof(*addr))
				addr = RTA_DATA(rta);

		// Address not specified or unrelated
		if (!addr || IN6_IS_ADDR_LINKLOCAL(addr) ||
				IN6_IS_ADDR_MULTICAST(addr))
			continue;

		// Check for states
		bool add;
		if (is_addr)
			add = (nh->nlmsg_type == RTM_NEWADDR) && !(ifa->ifa_flags &
				IFA_F_TENTATIVE);
		else
			add = (nh->nlmsg_type == RTM_NEWNEIGH && (ndm->ndm_state &
				(NUD_REACHABLE | NUD_STALE | NUD_DELAY | NUD_PROBE
				| NUD_PERMANENT | (iface->nondp ? 0 : NUD_NOARP))));

		if (config->enable_ndp_relay)
			modify_neighbor(addr, iface, add, is_addr);

		if (is_addr && config->enable_router_discovery_server)
			raise(SIGUSR1); // Inform about a change in addresses

		if (is_addr && config->enable_dhcpv6_server)
			iface->pd_reconf = true;

		if (config->enable_ndp_relay && is_addr && iface == &config->master) {
			// Replay address changes on all slave interfaces
			nh->nlmsg_flags = NLM_F_REQUEST;

			if (nh->nlmsg_type == RTM_NEWADDR)
				nh->nlmsg_flags |= NLM_F_CREATE | NLM_F_REPLACE;

			for (size_t i = 0; i < config->slavecount; ++i) {
				ifa->ifa_index = config->slaves[i].ifindex;
				send(rtnl_event.socket, nh, nh->nlmsg_len, MSG_DONTWAIT);
			}

			if (nh->nlmsg_type == RTM_NEWADDR && iface->nondp &&
					ifa->ifa_prefixlen > 0 &&
					ifa->ifa_prefixlen < 128) {
				// Drop on-link route from non-NDP link
				relayd_setup_route(addr, ifa->ifa_prefixlen,
						iface, NULL, 256, false);
			}
		}

		/* TODO: See if this is required for optimal operation
		// Keep neighbor entries alive so we don't loose routes
		if (add && (ndm->ndm_state & NUD_STALE))
			ping6(addr, iface);
		*/
	}
}
