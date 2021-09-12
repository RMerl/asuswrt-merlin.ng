/* -*- mode: c; c-file-style: "openbsd" -*- */
/*
 * Copyright (c) 2012 Vincent Bernat <bernat@luffy.cx>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* Grabbing interfaces information with netlink only. */

#include "lldpd.h"

#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <net/if_arp.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/if_bridge.h>

#define NETLINK_BUFFER 4096

struct netlink_req {
	struct nlmsghdr hdr;
	struct ifinfomsg ifm;
	/* attribute has to be NLMSG aligned */
	struct rtattr ext_req __attribute__ ((aligned(NLMSG_ALIGNTO)));
	__u32 ext_filter_mask;
};

struct lldpd_netlink {
	int nl_socket;
	int nl_socket_recv_size;
	/* Cache */
	struct interfaces_device_list *devices;
	struct interfaces_address_list *addresses;
};

/**
 * Set netlink socket buffer size.
 *
 * This returns the effective size on success. If the provided value is 0, this
 * returns the current size instead. It returns -1 on system errors and -2 if
 * the size was not changed appropriately (when reaching the max).
 */
static int
netlink_socket_set_buffer_size(int s, int optname, const char *optname_str, int bufsize)
{
	socklen_t size = sizeof(int);
	int got = 0;

	if (bufsize > 0 && setsockopt(s, SOL_SOCKET, optname, &bufsize, sizeof(bufsize)) < 0) {
		log_warn("netlink", "unable to set %s to '%d'", optname_str, bufsize);
		return -1;
	}

	/* Now read them back from kernel.
	 * SO_SNDBUF & SO_RCVBUF are cap-ed at sysctl `net.core.rmem_max` &
	 * `net.core.wmem_max`. This it the easiest [probably sanest too]
	 * to validate that our socket buffers were set properly.
	 */
	if (getsockopt(s, SOL_SOCKET, optname, &got, &size) < 0) {
		log_warn("netlink", "unable to get %s", optname_str);
		return -1;
	}
	if (bufsize > 0 && got < bufsize) {
		log_warnx("netlink", "tried to set %s to '%d' "
		    "but got '%d'", optname_str, bufsize, got);
		return -2;
	}

	return got;
}

/**
 * Connect to netlink.
 *
 * Open a Netlink socket and connect to it.
 *
 * @param protocol Which protocol to use (eg NETLINK_ROUTE).
 * @param groups   Which groups we want to subscribe to
 * @return 0 on success, -1 otherwise
 */
static int
netlink_connect(struct lldpd *cfg, int protocol, unsigned groups)
{
	int s;
	struct sockaddr_nl local = {
		.nl_family = AF_NETLINK,
		.nl_pid = 0,
		.nl_groups = groups
	};

	/* Open Netlink socket */
	log_debug("netlink", "opening netlink socket");
	s = socket(AF_NETLINK, SOCK_RAW, protocol);
	if (s == -1) {
		log_warn("netlink", "unable to open netlink socket");
		return -1;
	}
	if (NETLINK_SEND_BUFSIZE &&
	    netlink_socket_set_buffer_size(s,
	    SO_SNDBUF, "SO_SNDBUF", NETLINK_SEND_BUFSIZE) == -1) {
		close(s);
		return -1;
	}

	int rc = netlink_socket_set_buffer_size(s,
	    SO_RCVBUF, "SO_RCVBUF", NETLINK_RECEIVE_BUFSIZE);
	switch (rc) {
	case -1:
		close(s);
		return -1;
	case -2: cfg->g_netlink->nl_socket_recv_size = 0; break;
	default: cfg->g_netlink->nl_socket_recv_size = rc; break;
	}
	if (groups && bind(s, (struct sockaddr *)&local, sizeof(struct sockaddr_nl)) < 0) {
		log_warn("netlink", "unable to bind netlink socket");
		close(s);
		return -1;
	}
	cfg->g_netlink->nl_socket = s;
	return 0;
}

/**
 * Send a netlink message.
 *
 * The type of the message can be chosen as well the route family. The
 * mesage will always be NLM_F_REQUEST | NLM_F_DUMP.
 *
 * @param s      the netlink socket
 * @param type   the request type (eg RTM_GETLINK)
 * @param family the rt family (eg AF_PACKET)
 * @return 0 on success, -1 otherwise
 */
static int
netlink_send(int s, int type, int family, int seq)
{
	struct netlink_req req = {
		.hdr = {
			.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg)),
			.nlmsg_type = type,
			.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP,
			.nlmsg_seq = seq,
			.nlmsg_pid = getpid() },
		.ifm = { .ifi_family = family }
	};
	struct iovec iov = {
		.iov_base = &req,
		.iov_len = req.hdr.nlmsg_len
	};
	struct sockaddr_nl peer = { .nl_family = AF_NETLINK };
	struct msghdr rtnl_msg = {
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_name = &peer,
		.msg_namelen = sizeof(struct sockaddr_nl)
	};

	if (family == AF_BRIDGE) {
		unsigned int len = RTA_LENGTH(sizeof(__u32));
		/* request bridge vlan attributes */
		req.ext_req.rta_type = IFLA_EXT_MASK;
		req.ext_req.rta_len = len;
		req.ext_filter_mask = RTEXT_FILTER_BRVLAN;
		req.hdr.nlmsg_len = NLMSG_ALIGN(req.hdr.nlmsg_len) + RTA_ALIGN(len);
		iov.iov_len = req.hdr.nlmsg_len;
	}

	/* Send netlink message. This is synchronous but we are guaranteed
	 * to not block. */
	log_debug("netlink", "sending netlink message");
	if (sendmsg(s, (struct msghdr *)&rtnl_msg, 0) == -1) {
		log_warn("netlink", "unable to send netlink message");
		return -1;
	}

	return 0;
}

static void
netlink_parse_rtattr(struct rtattr *tb[], int max, struct rtattr *rta, int len)
{
	while (RTA_OK(rta, len)) {
		if ((rta->rta_type <= max) && (!tb[rta->rta_type]))
			tb[rta->rta_type] = rta;
		rta = RTA_NEXT(rta,len);
	}
}

/**
 * Parse a `linkinfo` attributes.
 *
 * @param iff where to put the result
 * @param rta linkinfo attribute
 * @param len length of attributes
 */
static void
netlink_parse_linkinfo(struct interfaces_device *iff, struct rtattr *rta, int len)
{
	struct rtattr *link_info_attrs[IFLA_INFO_MAX+1] = {};
	char *kind = NULL;
	uint16_t vlan_id;

	netlink_parse_rtattr(link_info_attrs, IFLA_INFO_MAX, rta, len);

	if (link_info_attrs[IFLA_INFO_KIND]) {
		kind = strdup(RTA_DATA(link_info_attrs[IFLA_INFO_KIND]));
		if (kind) {
			if (!strcmp(kind, "vlan")) {
				log_debug("netlink", "interface %s is a VLAN",
				    iff->name);
				iff->type |= IFACE_VLAN_T;
			} else if (!strcmp(kind, "bridge")) {
				log_debug("netlink", "interface %s is a bridge",
				    iff->name);
				iff->type |= IFACE_BRIDGE_T;
			} else if (!strcmp(kind, "bond")) {
				log_debug("netlink", "interface %s is a bond",
				    iff->name);
				iff->type |= IFACE_BOND_T;
			} else if (!strcmp(kind, "team")) {
				log_debug("netlink", "interface %s is a team",
				    iff->name);
				iff->type |= IFACE_BOND_T;
			}
		}
	}

	if (kind && !strcmp(kind, "vlan") && link_info_attrs[IFLA_INFO_DATA]) {
		struct rtattr *vlan_link_info_data_attrs[IFLA_VLAN_MAX+1] = {};
		netlink_parse_rtattr(vlan_link_info_data_attrs, IFLA_VLAN_MAX,
		    RTA_DATA(link_info_attrs[IFLA_INFO_DATA]),
		    RTA_PAYLOAD(link_info_attrs[IFLA_INFO_DATA]));

		if (vlan_link_info_data_attrs[IFLA_VLAN_ID]) {
			vlan_id = *(uint16_t *)RTA_DATA(vlan_link_info_data_attrs[IFLA_VLAN_ID]);
			bitmap_set(iff->vlan_bmap, vlan_id);
			log_debug("netlink", "VLAN ID for interface %s is %d",
			    iff->name, vlan_id);
		}
	}

	if (kind && !strcmp(kind, "bridge") && link_info_attrs[IFLA_INFO_DATA]) {
		struct rtattr *bridge_link_info_data_attrs[IFLA_BR_MAX+1] = {};
		netlink_parse_rtattr(bridge_link_info_data_attrs, IFLA_BR_MAX,
		    RTA_DATA(link_info_attrs[IFLA_INFO_DATA]),
		    RTA_PAYLOAD(link_info_attrs[IFLA_INFO_DATA]));

		if (bridge_link_info_data_attrs[IFLA_BR_VLAN_FILTERING] &&
		    *(uint8_t *)RTA_DATA(bridge_link_info_data_attrs[IFLA_BR_VLAN_FILTERING]) > 0) {
			iff->type |= IFACE_BRIDGE_VLAN_T;
		}
	}

	free(kind);
}

/**
 * Parse a `afspec` attributes.
 *
 * @param iff where to put the result
 * @param rta afspec attribute
 * @param len length of attributes
 */
static void
netlink_parse_afspec(struct interfaces_device *iff, struct rtattr *rta, int len)
{
	while (RTA_OK(rta, len)) {
		struct bridge_vlan_info *vinfo;
		switch (rta->rta_type) {
		case IFLA_BRIDGE_VLAN_INFO:
			vinfo = RTA_DATA(rta);
			log_debug("netlink", "found VLAN %d on interface %s",
			    vinfo->vid, iff->name ? iff->name : "(unknown)");

			bitmap_set(iff->vlan_bmap, vinfo->vid);
			if (vinfo->flags & (BRIDGE_VLAN_INFO_PVID | BRIDGE_VLAN_INFO_UNTAGGED))
				iff->pvid = vinfo->vid;
			break;
		default:
			log_debug("netlink", "unknown afspec attribute type %d for iface %s",
			    rta->rta_type, iff->name ? iff->name : "(unknown)");
			break;
		}
		rta = RTA_NEXT(rta, len);
	}
	/* All enbridged interfaces will have VLAN 1 by default, ignore it */
	if (iff->vlan_bmap[0] == 2 && (bitmap_numbits(iff->vlan_bmap) == 1)
			&& iff->pvid == 1) {
		log_debug("netlink", "found only default VLAN 1 on interface %s, removing",
		    iff->name ? iff->name : "(unknown)");
		iff->vlan_bmap[0] = iff->pvid = 0;
	}
}

/**
 * Parse a `link` netlink message.
 *
 * @param msg  message to be parsed
 * @param iff  where to put the result
 * return 0 if the interface is worth it, -1 otherwise
 */
static int
netlink_parse_link(struct nlmsghdr *msg,
    struct interfaces_device *iff)
{
	struct ifinfomsg *ifi;
	struct rtattr *attribute;
	int len;
	ifi = NLMSG_DATA(msg);
	len = msg->nlmsg_len - NLMSG_LENGTH(sizeof(struct ifinfomsg));

	if (ifi->ifi_type != ARPHRD_ETHER) {
		log_debug("netlink", "skip non Ethernet interface at index %d",
		    ifi->ifi_index);
		return -1;
	}

	iff->index = ifi->ifi_index;
	iff->flags = ifi->ifi_flags;
	iff->lower_idx = -1;
	iff->upper_idx = -1;

	for (attribute = IFLA_RTA(ifi);
	     RTA_OK(attribute, len);
	     attribute = RTA_NEXT(attribute, len)) {
		switch(attribute->rta_type) {
		case IFLA_IFNAME:
			/* Interface name */
			iff->name = strdup(RTA_DATA(attribute));
			break;
		case IFLA_IFALIAS:
			/* Interface alias */
			iff->alias = strdup(RTA_DATA(attribute));
			break;
		case IFLA_ADDRESS:
			/* Interface MAC address */
			iff->address = malloc(RTA_PAYLOAD(attribute));
			if (iff->address)
				memcpy(iff->address, RTA_DATA(attribute), RTA_PAYLOAD(attribute));
			break;
		case IFLA_LINK:
			/* Index of "lower" interface */
			if (iff->lower_idx == -1) {
				iff->lower_idx = *(int*)RTA_DATA(attribute);
				log_debug("netlink", "attribute IFLA_LINK for %s: %d",
				    iff->name ? iff->name : "(unknown)", iff->lower_idx);
			} else {
				log_debug("netlink", "attribute IFLA_LINK for %s: %d (ignored)",
				    iff->name ? iff->name : "(unknown)", iff->lower_idx);
			}
			break;
		case IFLA_LINK_NETNSID:
			/* Is the lower interface into another namesapce? */
			iff->lower_idx = -2;
			log_debug("netlink", "attribute IFLA_LINK_NETNSID received for %s",
			    iff->name ? iff->name : "(unknown)");
			break;
		case IFLA_MASTER:
			/* Index of master interface */
			iff->upper_idx = *(int*)RTA_DATA(attribute);
			break;
		case IFLA_MTU:
			/* Maximum Transmission Unit */
			iff->mtu = *(int*)RTA_DATA(attribute);
			break;
		case IFLA_LINKINFO:
			netlink_parse_linkinfo(iff, RTA_DATA(attribute), RTA_PAYLOAD(attribute));
			break;
		case IFLA_AF_SPEC:
			if (ifi->ifi_family != AF_BRIDGE) break;
			netlink_parse_afspec(iff, RTA_DATA(attribute), RTA_PAYLOAD(attribute));
			break;
		default:
			log_debug("netlink", "unhandled link attribute type %d for iface %s",
			    attribute->rta_type, iff->name ? iff->name : "(unknown)");
			break;
		}
	}
	if (!iff->name || !iff->address) {
		log_debug("netlink", "interface %d does not have a name or an address, skip",
		    iff->index);
		return -1;
	}
	if (iff->upper_idx == -1) {
		/* No upper interface, we cannot be enslaved. We need to clear
		 * the flag because the appropriate information may come later
		 * and we don't want to miss it. */
		iff->flags &= ~IFF_SLAVE;
	}
	if (iff->lower_idx == -2)
		iff->lower_idx = -1;

	if (ifi->ifi_family == AF_BRIDGE && msg->nlmsg_type == RTM_DELLINK && iff->upper_idx != -1) {
		log_debug("netlink", "removal of %s from bridge %d",
		    iff->name, iff->upper_idx);
		msg->nlmsg_type = RTM_NEWLINK;
		iff->upper_idx = -1;
	}

	log_debug("netlink", "parsed link %d (%s, flags: %d)",
	    iff->index, iff->name, iff->flags);
	return 0;
}

/**
 * Parse a `address` netlink message.
 *
 * @param msg  message to be parsed
 * @param ifa  where to put the result
 * return 0 if the address is worth it, -1 otherwise
 */
static int
netlink_parse_address(struct nlmsghdr *msg,
    struct interfaces_address *ifa)
{
	struct ifaddrmsg *ifi;
	struct rtattr *attribute;
	int len;
	ifi = NLMSG_DATA(msg);
	len = msg->nlmsg_len - NLMSG_LENGTH(sizeof(struct ifaddrmsg));

	ifa->index = ifi->ifa_index;
	ifa->flags = ifi->ifa_flags;
	switch (ifi->ifa_family) {
	case AF_INET:
	case AF_INET6: break;
	default:
		log_debug("netlink", "got a non IP address on if %d (family: %d)",
		    ifa->index, ifi->ifa_family);
		return -1;
	}

	for (attribute = IFA_RTA(ifi);
	     RTA_OK(attribute, len);
	     attribute = RTA_NEXT(attribute, len)) {
		switch(attribute->rta_type) {
		case IFA_ADDRESS:
			/* Address */
			if (ifi->ifa_family == AF_INET) {
				struct sockaddr_in ip;
				memset(&ip, 0, sizeof(struct sockaddr_in));
				ip.sin_family = AF_INET;
				memcpy(&ip.sin_addr, RTA_DATA(attribute),
				    sizeof(struct in_addr));
				memcpy(&ifa->address, &ip, sizeof(struct sockaddr_in));
			} else {
				struct sockaddr_in6 ip6;
				memset(&ip6, 0, sizeof(struct sockaddr_in6));
				ip6.sin6_family = AF_INET6;
				memcpy(&ip6.sin6_addr, RTA_DATA(attribute),
				    sizeof(struct in6_addr));
				memcpy(&ifa->address, &ip6, sizeof(struct sockaddr_in6));
			}
			break;
		default:
			log_debug("netlink", "unhandled address attribute type %d for iface %d",
			    attribute->rta_type, ifa->index);
			break;
		}
	}
	if (ifa->address.ss_family == AF_UNSPEC) {
		log_debug("netlink", "no IP for interface %d",
		    ifa->index);
		return -1;
	}
	return 0;
}

/**
 * Merge an old interface with a new one.
 *
 * Some properties may be absent in the new interface that should be copied over
 * from the old one.
 */
void
netlink_merge(struct interfaces_device *old, struct interfaces_device *new)
{
	if (new->alias == NULL) {
		new->alias = old->alias;
		old->alias = NULL;
	}
	if (new->address == NULL) {
		new->address = old->address;
		old->address = NULL;
	}
	if (new->mtu == 0)
		new->mtu = old->mtu;
	if (new->type == 0)
		new->type = old->type;

	if (bitmap_isempty(new->vlan_bmap) && new->type == IFACE_VLAN_T)
		memcpy((void *)new->vlan_bmap, (void *)old->vlan_bmap,
				sizeof(uint32_t) * VLAN_BITMAP_LEN);

	/* It's not possible for lower link to change */
	new->lower_idx = old->lower_idx;
}

/**
 * Receive netlink answer from the kernel.
 *
 * @param ifs  list to store interface list or NULL if we don't
 * @param ifas list to store address list or NULL if we don't
 * @return     0 on success, -1 on error
 */
static int
netlink_recv(struct lldpd *cfg,
    struct interfaces_device_list *ifs,
    struct interfaces_address_list *ifas)
{
	int end = 0, ret = 0, flags, retry = 0;
	struct iovec iov;
	int link_update = 0;
	int s = cfg->g_netlink->nl_socket;

	struct interfaces_device *ifdold;
	struct interfaces_device *ifdnew;
	struct interfaces_address *ifaold;
	struct interfaces_address *ifanew;
	char addr[INET6_ADDRSTRLEN + 1];

	iov.iov_len = NETLINK_BUFFER;
	iov.iov_base = malloc(iov.iov_len);
	if (!iov.iov_base) {
		log_warn("netlink", "not enough memory");
		return -1;
	}

	while (!end) {
		ssize_t len;
		struct nlmsghdr *msg;
		struct sockaddr_nl peer = { .nl_family = AF_NETLINK };
		struct msghdr rtnl_reply = {
			.msg_iov = &iov,
			.msg_iovlen = 1,
			.msg_name = &peer,
			.msg_namelen = sizeof(struct sockaddr_nl)
		};
		flags = MSG_PEEK | MSG_TRUNC;
retry:
		len = recvmsg(s, &rtnl_reply, flags);
		if (len == -1) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				if (retry++ == 0) {
					levent_recv_error(s, "netlink socket");
					goto retry;
				}
				log_warnx("netlink", "should have received something, but didn't");
				ret = 0;
				goto out;
			}
			int rsize = cfg->g_netlink->nl_socket_recv_size;
			if (errno == ENOBUFS &&
			    rsize > 0 && rsize < NETLINK_MAX_RECEIVE_BUFSIZE) {
				/* Try to increase buffer size */
				rsize *= 2;
				if (rsize > NETLINK_MAX_RECEIVE_BUFSIZE) {
					rsize = NETLINK_MAX_RECEIVE_BUFSIZE;
				}
				int rc = netlink_socket_set_buffer_size(s,
				    SO_RCVBUF, "SO_RCVBUF",
				    rsize);
				if (rc < 0)
					cfg->g_netlink->nl_socket_recv_size = 0;
				else
					cfg->g_netlink->nl_socket_recv_size = rsize;
				if (rc > 0 || rc == -2) {
					log_info("netlink",
					    "netlink receive buffer too small, retry with larger one (%d)",
					    rsize);
					flags = 0;
					goto retry;
				}
			}
			log_warn("netlink", "unable to receive netlink answer");
			ret = -1;
			goto out;
		}
		if (!len) {
			ret = 0;
			goto out;
		}

		if (iov.iov_len < len || (rtnl_reply.msg_flags & MSG_TRUNC)) {
			void *tmp;

			/* Provided buffer is not large enough, enlarge it
			 * to size of len (which should be total length of the message)
			 * and try again. */
			iov.iov_len = len;
			tmp = realloc(iov.iov_base, iov.iov_len);
			if (!tmp) {
				log_warn("netlink", "not enough memory");
				ret = -1;
				goto out;
			}
			log_debug("netlink", "enlarge message size to %zu bytes", len);
			iov.iov_base = tmp;
			flags = 0;
			goto retry;
		}

		if (flags != 0) {
			/* Buffer is big enough, do the actual reading */
			flags = 0;
			goto retry;
		}

		for (msg = (struct nlmsghdr*)(void*)(iov.iov_base);
		     NLMSG_OK(msg, len);
		     msg = NLMSG_NEXT(msg, len)) {
			if (!(msg->nlmsg_flags & NLM_F_MULTI))
				end = 1;
			switch (msg->nlmsg_type) {
			case NLMSG_DONE:
				log_debug("netlink", "received done message");
				end = 1;
				break;
			case RTM_NEWLINK:
			case RTM_DELLINK:
				if (!ifs) break;
				log_debug("netlink", "received link information");
				ifdnew = calloc(1, sizeof(struct interfaces_device));
				if (ifdnew == NULL) {
					log_warn("netlink", "not enough memory for another interface, give up what we have");
					goto end;
				}
				if (netlink_parse_link(msg, ifdnew) == 0) {
					/* We need to find if we already have this interface */
					TAILQ_FOREACH(ifdold, ifs, next) {
						if (ifdold->index == ifdnew->index) break;
					}

					if (msg->nlmsg_type == RTM_NEWLINK) {
						if (ifdold == NULL) {
							log_debug("netlink", "interface %s is new",
							    ifdnew->name);
							TAILQ_INSERT_TAIL(ifs, ifdnew, next);
						} else {
							log_debug("netlink", "interface %s/%s is updated",
							    ifdold->name, ifdnew->name);
							netlink_merge(ifdold, ifdnew);
							TAILQ_INSERT_AFTER(ifs, ifdold, ifdnew, next);
							TAILQ_REMOVE(ifs, ifdold, next);
							interfaces_free_device(ifdold);
						}
					} else {
						if (ifdold == NULL) {
							log_warnx("netlink",
							    "removal request for %s, but no knowledge of it",
							    ifdnew->name);
						} else {
							log_debug("netlink", "interface %s is to be removed",
							    ifdold->name);
							TAILQ_REMOVE(ifs, ifdold, next);
							interfaces_free_device(ifdold);
						}
						interfaces_free_device(ifdnew);
					}
					link_update = 1;
				} else {
					interfaces_free_device(ifdnew);
				}
				break;
			case RTM_NEWADDR:
			case RTM_DELADDR:
				if (!ifas) break;
				log_debug("netlink", "received address information");
				ifanew = calloc(1, sizeof(struct interfaces_address));
				if (ifanew == NULL) {
					log_warn("netlink", "not enough memory for another address, give what we have");
					goto end;
				}
				if (netlink_parse_address(msg, ifanew) == 0) {
					TAILQ_FOREACH(ifaold, ifas, next) {
						if ((ifaold->index == ifanew->index) &&
						    !memcmp(&ifaold->address, &ifanew->address,
							sizeof(ifaold->address))) break;
					}
					if (getnameinfo((struct sockaddr *)&ifanew->address,
						sizeof(ifanew->address),
						addr, sizeof(addr),
						NULL, 0, NI_NUMERICHOST) != 0) {
						strlcpy(addr, "(unknown)", sizeof(addr));
					}

					if (msg->nlmsg_type == RTM_NEWADDR) {
						if (ifaold == NULL) {
							log_debug("netlink", "new address %s%%%d",
							    addr, ifanew->index);
							TAILQ_INSERT_TAIL(ifas, ifanew, next);
						} else {
							log_debug("netlink", "updated address %s%%%d",
							    addr, ifaold->index);
							TAILQ_INSERT_AFTER(ifas, ifaold, ifanew, next);
							TAILQ_REMOVE(ifas, ifaold, next);
							interfaces_free_address(ifaold);
						}
					} else {
						if (ifaold == NULL) {
							log_info("netlink",
							    "removal request for address of %s%%%d, but no knowledge of it",
							    addr, ifanew->index);
						} else {
							log_debug("netlink", "address %s%%%d is to be removed",
							    addr, ifaold->index);
							TAILQ_REMOVE(ifas, ifaold, next);
							interfaces_free_address(ifaold);
						}
						interfaces_free_address(ifanew);
					}
				} else {
					interfaces_free_address(ifanew);
				}
				break;
			default:
				log_debug("netlink",
				    "received unhandled message type %d (len: %d)",
				    msg->nlmsg_type, msg->nlmsg_len);
			}
		}
	}
end:
	if (link_update) {
		/* Fill out lower/upper */
		struct interfaces_device *iface1, *iface2;
		TAILQ_FOREACH(iface1, ifs, next) {
			if (iface1->upper_idx != -1 && iface1->upper_idx != iface1->index) {
				TAILQ_FOREACH(iface2, ifs, next) {
					if (iface1->upper_idx == iface2->index) {
						log_debug("netlink",
						    "upper interface for %s is %s",
						    iface1->name, iface2->name);
						iface1->upper = iface2;
						break;
					}
				}
				if (iface2 == NULL)
					iface1->upper = NULL;
			} else {
				iface1->upper = NULL;
			}
			if (iface1->lower_idx != -1 && iface1->lower_idx != iface1->index) {
				TAILQ_FOREACH(iface2, ifs, next) {
					if (iface1->lower_idx == iface2->index) {
						/* Workaround a bug introduced
						 * in Linux 4.1: a pair of veth
						 * will be lower interface of
						 * each other. Do not modify
						 * index as if one of them is
						 * updated, we will loose the
						 * information about the
						 * loop. */
						if (iface2->lower_idx == iface1->index) {
							iface1->lower = NULL;
							log_debug("netlink",
							    "link loop detected between %s(%d) and %s(%d)",
							    iface1->name,
							    iface1->index,
							    iface2->name,
							    iface2->index);
						} else {
							log_debug("netlink",
							    "lower interface for %s is %s",
							    iface1->name, iface2->name);
							iface1->lower = iface2;
						}
						break;
					}
				}
			} else {
				iface1->lower = NULL;
			}
		}
	}

out:
	free(iov.iov_base);
	return ret;
}

static int
netlink_group_mask(int group)
{
	return group ? (1 << (group - 1)) : 0;
}

/**
 * Subscribe to link changes.
 *
 * @return 0 on success, -1 otherwise
 */
static int
netlink_subscribe_changes(struct lldpd *cfg)
{
	unsigned int groups;

	log_debug("netlink", "listening on interface changes");

	groups = netlink_group_mask(RTNLGRP_LINK) |
	    netlink_group_mask(RTNLGRP_IPV4_IFADDR) |
	    netlink_group_mask(RTNLGRP_IPV6_IFADDR);

	return netlink_connect(cfg, NETLINK_ROUTE, groups);
}

/**
 * Receive changes from netlink */
static void
netlink_change_cb(struct lldpd *cfg)
{
	if (cfg->g_netlink == NULL)
		return;
	netlink_recv(cfg,
	    cfg->g_netlink->devices,
	    cfg->g_netlink->addresses);
}

/**
 * Initialize netlink subsystem.
 *
 * This can be called several times but will have effect only the first time.
 *
 * @return 0 on success, -1 otherwise
 */
static int
netlink_initialize(struct lldpd *cfg)
{
#ifdef ENABLE_DOT1
	struct interfaces_device *iff;
#endif

	if (cfg->g_netlink) return 0;

	log_debug("netlink", "initialize netlink subsystem");
	if ((cfg->g_netlink = calloc(sizeof(struct lldpd_netlink), 1)) == NULL) {
		log_warn("netlink", "unable to allocate memory for netlink subsystem");
		goto end;
	}

	/* Connect to netlink (by requesting to get notified on updates) and
	 * request updated information right now */
	if (netlink_subscribe_changes(cfg) == -1)
		goto end;

	struct interfaces_address_list *ifaddrs = cfg->g_netlink->addresses =
	    malloc(sizeof(struct interfaces_address_list));
	if (ifaddrs == NULL) {
		log_warn("netlink", "not enough memory for address list");
		goto end;
	}
	TAILQ_INIT(ifaddrs);

	struct interfaces_device_list *ifs = cfg->g_netlink->devices =
	    malloc(sizeof(struct interfaces_device_list));
	if (ifs == NULL) {
		log_warn("netlink", "not enough memory for interface list");
		goto end;
	}
	TAILQ_INIT(ifs);

	if (netlink_send(cfg->g_netlink->nl_socket, RTM_GETADDR, AF_UNSPEC, 1) == -1)
		goto end;
	netlink_recv(cfg, NULL, ifaddrs);
	if (netlink_send(cfg->g_netlink->nl_socket, RTM_GETLINK, AF_PACKET, 2) == -1)
		goto end;
	netlink_recv(cfg, ifs, NULL);
#ifdef ENABLE_DOT1
	/* If we have a bridge, search for VLAN-aware bridges */
	TAILQ_FOREACH(iff, ifs, next) {
		if (iff->type & IFACE_BRIDGE_T) {
			log_debug("netlink", "interface %s is a bridge, check for VLANs", iff->name);
			if (netlink_send(cfg->g_netlink->nl_socket, RTM_GETLINK, AF_BRIDGE, 3) == -1)
				goto end;
			netlink_recv(cfg, ifs, NULL);
			break;
		}
	}
#endif

	/* Listen to any future change */
	cfg->g_iface_cb = netlink_change_cb;
	if (levent_iface_subscribe(cfg, cfg->g_netlink->nl_socket) == -1) {
		goto end;
	}

	return 0;
end:
	netlink_cleanup(cfg);
	return -1;
}

/**
 * Cleanup netlink subsystem.
 */
void
netlink_cleanup(struct lldpd *cfg)
{
	if (cfg->g_netlink == NULL) return;
	if (cfg->g_netlink->nl_socket != -1)
		close(cfg->g_netlink->nl_socket);
	interfaces_free_devices(cfg->g_netlink->devices);
	interfaces_free_addresses(cfg->g_netlink->addresses);

	free(cfg->g_netlink);
	cfg->g_netlink = NULL;
}

/**
 * Receive the list of interfaces.
 *
 * @return a list of interfaces.
 */
struct interfaces_device_list*
netlink_get_interfaces(struct lldpd *cfg)
{
	if (netlink_initialize(cfg) == -1) return NULL;
	struct interfaces_device *ifd;
	TAILQ_FOREACH(ifd, cfg->g_netlink->devices, next) {
		ifd->ignore = 0;
	}
	return cfg->g_netlink->devices;
}

/**
 * Receive the list of addresses.
 *
 * @return a list of addresses.
 */
struct interfaces_address_list*
netlink_get_addresses(struct lldpd *cfg)
{
	if (netlink_initialize(cfg) == -1) return NULL;
	return cfg->g_netlink->addresses;
}
