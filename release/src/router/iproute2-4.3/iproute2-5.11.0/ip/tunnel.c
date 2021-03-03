/*
 * Copyright (C)2006 USAGI/WIDE Project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses>.
 */
/*
 * split from ip_tunnel.c
 */
/*
 * Author:
 *	Masahide NAKAMURA @USAGI
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <linux/if.h>
#include <linux/ip.h>
#include <linux/if_tunnel.h>
#include <linux/if_arp.h>

#include "utils.h"
#include "tunnel.h"
#include "json_print.h"

const char *tnl_strproto(__u8 proto)
{
	switch (proto) {
	case IPPROTO_IPIP:
		return "ip";
	case IPPROTO_GRE:
		return "gre";
	case IPPROTO_IPV6:
		return "ipv6";
	case IPPROTO_ESP:
		return "esp";
	case IPPROTO_MPLS:
		return "mpls";
	case 0:
		return "any";
	default:
		return "unknown";
	}
}

int tnl_get_ioctl(const char *basedev, void *p)
{
	struct ifreq ifr;
	int fd;
	int err;

	strlcpy(ifr.ifr_name, basedev, IFNAMSIZ);
	ifr.ifr_ifru.ifru_data = (void *)p;

	fd = socket(preferred_family, SOCK_DGRAM, 0);
	if (fd < 0) {
		fprintf(stderr, "create socket failed: %s\n", strerror(errno));
		return -1;
	}

	err = ioctl(fd, SIOCGETTUNNEL, &ifr);
	if (err)
		fprintf(stderr, "get tunnel \"%s\" failed: %s\n", basedev,
			strerror(errno));

	close(fd);
	return err;
}

int tnl_add_ioctl(int cmd, const char *basedev, const char *name, void *p)
{
	struct ifreq ifr;
	int fd;
	int err;

	if (cmd == SIOCCHGTUNNEL && name[0])
		strlcpy(ifr.ifr_name, name, IFNAMSIZ);
	else
		strlcpy(ifr.ifr_name, basedev, IFNAMSIZ);
	ifr.ifr_ifru.ifru_data = p;

	fd = socket(preferred_family, SOCK_DGRAM, 0);
	if (fd < 0) {
		fprintf(stderr, "create socket failed: %s\n", strerror(errno));
		return -1;
	}

	err = ioctl(fd, cmd, &ifr);
	if (err)
		fprintf(stderr, "add tunnel \"%s\" failed: %s\n", ifr.ifr_name,
			strerror(errno));
	close(fd);
	return err;
}

int tnl_del_ioctl(const char *basedev, const char *name, void *p)
{
	struct ifreq ifr;
	int fd;
	int err;

	if (name[0])
		strlcpy(ifr.ifr_name, name, IFNAMSIZ);
	else
		strlcpy(ifr.ifr_name, basedev, IFNAMSIZ);

	ifr.ifr_ifru.ifru_data = p;

	fd = socket(preferred_family, SOCK_DGRAM, 0);
	if (fd < 0) {
		fprintf(stderr, "create socket failed: %s\n", strerror(errno));
		return -1;
	}

	err = ioctl(fd, SIOCDELTUNNEL, &ifr);
	if (err)
		fprintf(stderr, "delete tunnel \"%s\" failed: %s\n",
			ifr.ifr_name, strerror(errno));
	close(fd);
	return err;
}

static int tnl_gen_ioctl(int cmd, const char *name,
			 void *p, int skiperr)
{
	struct ifreq ifr;
	int fd;
	int err;

	strlcpy(ifr.ifr_name, name, IFNAMSIZ);
	ifr.ifr_ifru.ifru_data = p;

	fd = socket(preferred_family, SOCK_DGRAM, 0);
	if (fd < 0) {
		fprintf(stderr, "create socket failed: %s\n", strerror(errno));
		return -1;
	}

	err = ioctl(fd, cmd, &ifr);
	if (err && errno != skiperr)
		fprintf(stderr, "%s: ioctl %x failed: %s\n", name,
			cmd, strerror(errno));
	close(fd);
	return err;
}

int tnl_prl_ioctl(int cmd, const char *name, void *p)
{
	return tnl_gen_ioctl(cmd, name, p, -1);
}

int tnl_6rd_ioctl(int cmd, const char *name, void *p)
{
	return tnl_gen_ioctl(cmd, name, p, -1);
}

int tnl_ioctl_get_6rd(const char *name, void *p)
{
	return tnl_gen_ioctl(SIOCGET6RD, name, p, EINVAL);
}

__be32 tnl_parse_key(const char *name, const char *key)
{
	unsigned int uval;

	if (strchr(key, '.'))
		return get_addr32(key);

	if (get_unsigned(&uval, key, 0) < 0) {
		fprintf(stderr,
			"invalid value for \"%s\": \"%s\"; it should be an unsigned integer\n",
			name, key);
		exit(-1);
	}
	return htonl(uval);
}

static const char *tnl_encap_str(const char *name, int enabled, int port)
{
	static const char ne[][sizeof("no")] = {
		[0] = "no",
		[1] = "",
	};
	static char buf[32];
	char b1[16];
	const char *val;

	if (!port) {
		val = "auto ";
	} else if (port < 0) {
		val = "";
	} else {
		snprintf(b1, sizeof(b1), "%u ", port - 1);
		val = b1;
	}

	snprintf(buf, sizeof(buf), "%sencap-%s %s", ne[!!enabled], name, val);
	return buf;
}

void tnl_print_encap(struct rtattr *tb[],
		     int encap_type, int encap_flags,
		     int encap_sport, int encap_dport)
{
	__u16 type, flags, sport, dport;

	if (!tb[encap_type])
		return;

	type = rta_getattr_u16(tb[encap_type]);
	if (type == TUNNEL_ENCAP_NONE)
		return;

	flags = rta_getattr_u16(tb[encap_flags]);
	sport = rta_getattr_u16(tb[encap_sport]);
	dport = rta_getattr_u16(tb[encap_dport]);

	open_json_object("encap");
	print_string(PRINT_FP, NULL, "encap ", NULL);

	switch (type) {
	case TUNNEL_ENCAP_FOU:
		print_string(PRINT_ANY, "type", "%s ", "fou");
		break;
	case TUNNEL_ENCAP_GUE:
		print_string(PRINT_ANY, "type", "%s ", "gue");
		break;
	default:
		print_null(PRINT_ANY, "type", "%s ", "unknown");
		break;
	}

	if (is_json_context()) {
		print_uint(PRINT_JSON, "sport", NULL, ntohs(sport));
		print_uint(PRINT_JSON, "dport", NULL, ntohs(dport));
		print_bool(PRINT_JSON, "csum", NULL,
			   flags & TUNNEL_ENCAP_FLAG_CSUM);
		print_bool(PRINT_JSON, "csum6", NULL,
			   flags & TUNNEL_ENCAP_FLAG_CSUM6);
		print_bool(PRINT_JSON, "remcsum", NULL,
			   flags & TUNNEL_ENCAP_FLAG_REMCSUM);
		close_json_object();
	} else {
		int t;

		t = sport ? ntohs(sport) + 1 : 0;
		print_string(PRINT_FP, NULL, "%s",
			     tnl_encap_str("sport", 1, t));

		t = ntohs(dport) + 1;
		print_string(PRINT_FP, NULL, "%s",
			     tnl_encap_str("dport", 1, t));

		t = flags & TUNNEL_ENCAP_FLAG_CSUM;
		print_string(PRINT_FP, NULL, "%s",
			     tnl_encap_str("csum", t, -1));

		t = flags & TUNNEL_ENCAP_FLAG_CSUM6;
		print_string(PRINT_FP, NULL, "%s",
			     tnl_encap_str("csum6", t, -1));

		t = flags & TUNNEL_ENCAP_FLAG_REMCSUM;
		print_string(PRINT_FP, NULL, "%s",
			     tnl_encap_str("remcsum", t, -1));
	}
}

void tnl_print_endpoint(const char *name, const struct rtattr *rta, int family)
{
	const char *value;
	inet_prefix dst;

	if (!rta) {
		value = "any";
	} else if (get_addr_rta(&dst, rta, family)) {
		value = "unknown";
	} else if (dst.flags & ADDRTYPE_UNSPEC) {
		value = "any";
	} else {
		value = format_host(family, dst.bytelen, dst.data);
		if (!value)
			value = "unknown";
	}

	if (is_json_context()) {
		print_string(PRINT_JSON, name, NULL, value);
	} else {
		SPRINT_BUF(b1);

		snprintf(b1, sizeof(b1), "%s %%s ", name);
		print_string(PRINT_FP, NULL, b1, value);
	}
}

void tnl_print_gre_flags(__u8 proto,
			 __be16 i_flags, __be16 o_flags,
			 __be32 i_key, __be32 o_key)
{
	if ((i_flags & GRE_KEY) && (o_flags & GRE_KEY) &&
	    o_key == i_key) {
		print_uint(PRINT_ANY, "key", " key %u", ntohl(i_key));
	} else {
		if (i_flags & GRE_KEY)
			print_uint(PRINT_ANY, "ikey", " ikey %u", ntohl(i_key));
		if (o_flags & GRE_KEY)
			print_uint(PRINT_ANY, "okey", " okey %u", ntohl(o_key));
	}

	if (proto != IPPROTO_GRE)
		return;

	open_json_array(PRINT_JSON, "flags");
	if (i_flags & GRE_SEQ) {
		if (is_json_context())
			print_string(PRINT_JSON, NULL, "%s", "rx_drop_ooseq");
		else
			printf("%s  Drop packets out of sequence.", _SL_);
	}
	if (i_flags & GRE_CSUM) {
		if (is_json_context())
			print_string(PRINT_JSON, NULL, "%s", "rx_csum");
		else
			printf("%s  Checksum in received packet is required.", _SL_);
	}
	if (o_flags & GRE_SEQ) {
		if (is_json_context())
			print_string(PRINT_JSON, NULL, "%s", "tx_seq");
		else
			printf("%s  Sequence packets on output.", _SL_);
	}
	if (o_flags & GRE_CSUM) {
		if (is_json_context())
			print_string(PRINT_JSON, NULL, "%s", "tx_csum");
		else
			printf("%s  Checksum output packets.", _SL_);
	}
	close_json_array(PRINT_JSON, NULL);
}

static void tnl_print_stats(const struct rtnl_link_stats64 *s)
{
	printf("%s", _SL_);
	printf("RX: Packets    Bytes        Errors CsumErrs OutOfSeq Mcasts%s", _SL_);
	printf("    %-10lld %-12lld %-6lld %-8lld %-8lld %-8lld%s",
	       s->rx_packets, s->rx_bytes, s->rx_errors, s->rx_frame_errors,
	       s->rx_fifo_errors, s->multicast, _SL_);
	printf("TX: Packets    Bytes        Errors DeadLoop NoRoute  NoBufs%s", _SL_);
	printf("    %-10lld %-12lld %-6lld %-8lld %-8lld %-6lld",
	       s->tx_packets, s->tx_bytes, s->tx_errors, s->collisions,
	       s->tx_carrier_errors, s->tx_dropped);
}

static int print_nlmsg_tunnel(struct nlmsghdr *n, void *arg)
{
	struct tnl_print_nlmsg_info *info = arg;
	struct ifinfomsg *ifi = NLMSG_DATA(n);
	struct rtattr *tb[IFLA_MAX+1];
	const char *name, *n1;

	if (n->nlmsg_type != RTM_NEWLINK && n->nlmsg_type != RTM_DELLINK)
		return 0;

	if (n->nlmsg_len < NLMSG_LENGTH(sizeof(*ifi)))
		return -1;

	if (preferred_family == AF_INET) {
		switch (ifi->ifi_type) {
		case ARPHRD_TUNNEL:
		case ARPHRD_IPGRE:
		case ARPHRD_SIT:
			break;
		default:
			return 0;
		}
	} else {
		switch (ifi->ifi_type) {
		case ARPHRD_TUNNEL6:
		case ARPHRD_IP6GRE:
			break;
		default:
			return 0;
		}
	}

	parse_rtattr(tb, IFLA_MAX, IFLA_RTA(ifi), IFLA_PAYLOAD(n));

	if (!tb[IFLA_IFNAME])
		return 0;

	name = rta_getattr_str(tb[IFLA_IFNAME]);

	/* Assume p1->name[IFNAMSIZ] is first field of structure */
	n1 = info->p1;
	if (n1[0] && strcmp(n1, name))
		return 0;

	info->ifi = ifi;
	info->init(info);

	/* TODO: parse netlink attributes */
	if (tnl_get_ioctl(name, info->p2))
		return 0;

	if (!info->match(info))
		return 0;

	info->print(info->p2);
	if (show_stats) {
		struct rtnl_link_stats64 s;

		if (get_rtnl_link_stats_rta(&s, tb) <= 0)
			return -1;

		tnl_print_stats(&s);
	}
	fputc('\n', stdout);

	return 0;
}

int do_tunnels_list(struct tnl_print_nlmsg_info *info)
{
	new_json_obj(json);
	if (rtnl_linkdump_req(&rth, preferred_family) < 0) {
		perror("Cannot send dump request\n");
		return -1;
	}

	if (rtnl_dump_filter(&rth, print_nlmsg_tunnel, info) < 0) {
		fprintf(stderr, "Dump terminated\n");
		return -1;
	}
	delete_json_obj();

	return 0;
}
