/*
 * ipl2tp.c	       "ip l2tp"
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Original Author:	James Chapman <jchapman@katalix.com>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/ip.h>

#include <linux/genetlink.h>
#include <linux/l2tp.h>
#include "libgenl.h"

#include "utils.h"
#include "ip_common.h"

enum {
	L2TP_ADD,
	L2TP_CHG,
	L2TP_DEL,
	L2TP_GET
};

struct l2tp_parm {
	uint32_t tunnel_id;
	uint32_t peer_tunnel_id;
	uint32_t session_id;
	uint32_t peer_session_id;
	enum l2tp_encap_type encap;
	uint16_t local_udp_port;
	uint16_t peer_udp_port;
	int cookie_len;
	uint8_t cookie[8];
	int peer_cookie_len;
	uint8_t peer_cookie[8];
	inet_prefix local_ip;
	inet_prefix peer_ip;

	uint16_t pw_type;
	unsigned int udp6_csum_tx:1;
	unsigned int udp6_csum_rx:1;
	unsigned int udp_csum:1;
	unsigned int recv_seq:1;
	unsigned int send_seq:1;
	unsigned int tunnel:1;
	unsigned int session:1;
	int reorder_timeout;
	const char *ifname;
	uint8_t l2spec_type;
	uint8_t l2spec_len;
};

struct l2tp_stats {
	uint64_t data_rx_packets;
	uint64_t data_rx_bytes;
	uint64_t data_rx_errors;
	uint64_t data_rx_oos_packets;
	uint64_t data_rx_oos_discards;
	uint64_t data_tx_packets;
	uint64_t data_tx_bytes;
	uint64_t data_tx_errors;
};

struct l2tp_data {
	struct l2tp_parm config;
	struct l2tp_stats stats;
};

/* netlink socket */
static struct rtnl_handle genl_rth;
static int genl_family = -1;

/*****************************************************************************
 * Netlink actions
 *****************************************************************************/

static int create_tunnel(struct l2tp_parm *p)
{
	uint32_t local_attr = L2TP_ATTR_IP_SADDR;
	uint32_t peer_attr = L2TP_ATTR_IP_DADDR;

	GENL_REQUEST(req, 1024, genl_family, 0, L2TP_GENL_VERSION,
		     L2TP_CMD_TUNNEL_CREATE, NLM_F_REQUEST | NLM_F_ACK);

	addattr32(&req.n, 1024, L2TP_ATTR_CONN_ID, p->tunnel_id);
	addattr32(&req.n, 1024, L2TP_ATTR_PEER_CONN_ID, p->peer_tunnel_id);
	addattr8(&req.n, 1024, L2TP_ATTR_PROTO_VERSION, 3);
	addattr16(&req.n, 1024, L2TP_ATTR_ENCAP_TYPE, p->encap);

	if (p->local_ip.family == AF_INET6)
		local_attr = L2TP_ATTR_IP6_SADDR;
	addattr_l(&req.n, 1024, local_attr, &p->local_ip.data,
		  p->local_ip.bytelen);

	if (p->peer_ip.family == AF_INET6)
		peer_attr = L2TP_ATTR_IP6_DADDR;
	addattr_l(&req.n, 1024, peer_attr, &p->peer_ip.data,
		  p->peer_ip.bytelen);

	if (p->encap == L2TP_ENCAPTYPE_UDP) {
		addattr16(&req.n, 1024, L2TP_ATTR_UDP_SPORT, p->local_udp_port);
		addattr16(&req.n, 1024, L2TP_ATTR_UDP_DPORT, p->peer_udp_port);
		if (p->udp_csum)
			addattr8(&req.n, 1024, L2TP_ATTR_UDP_CSUM, 1);
		if (!p->udp6_csum_tx)
			addattr(&req.n, 1024, L2TP_ATTR_UDP_ZERO_CSUM6_TX);
		if (!p->udp6_csum_rx)
			addattr(&req.n, 1024, L2TP_ATTR_UDP_ZERO_CSUM6_RX);
	}

	if (rtnl_talk(&genl_rth, &req.n, NULL) < 0)
		return -2;

	return 0;
}

static int delete_tunnel(struct l2tp_parm *p)
{
	GENL_REQUEST(req, 128, genl_family, 0, L2TP_GENL_VERSION,
		     L2TP_CMD_TUNNEL_DELETE, NLM_F_REQUEST | NLM_F_ACK);

	addattr32(&req.n, 128, L2TP_ATTR_CONN_ID, p->tunnel_id);

	if (rtnl_talk(&genl_rth, &req.n, NULL) < 0)
		return -2;

	return 0;
}

static int create_session(struct l2tp_parm *p)
{
	GENL_REQUEST(req, 1024, genl_family, 0, L2TP_GENL_VERSION,
		     L2TP_CMD_SESSION_CREATE, NLM_F_REQUEST | NLM_F_ACK);

	addattr32(&req.n, 1024, L2TP_ATTR_CONN_ID, p->tunnel_id);
	addattr32(&req.n, 1024, L2TP_ATTR_PEER_CONN_ID, p->peer_tunnel_id);
	addattr32(&req.n, 1024, L2TP_ATTR_SESSION_ID, p->session_id);
	addattr32(&req.n, 1024, L2TP_ATTR_PEER_SESSION_ID, p->peer_session_id);
	addattr16(&req.n, 1024, L2TP_ATTR_PW_TYPE, p->pw_type);
	addattr8(&req.n, 1024, L2TP_ATTR_L2SPEC_TYPE, p->l2spec_type);
	addattr8(&req.n, 1024, L2TP_ATTR_L2SPEC_LEN, p->l2spec_len);

	if (p->recv_seq)
		addattr8(&req.n, 1024, L2TP_ATTR_RECV_SEQ, 1);
	if (p->send_seq)
		addattr8(&req.n, 1024, L2TP_ATTR_SEND_SEQ, 1);
	if (p->reorder_timeout)
		addattr64(&req.n, 1024, L2TP_ATTR_RECV_TIMEOUT,
					  p->reorder_timeout);
	if (p->cookie_len)
		addattr_l(&req.n, 1024, L2TP_ATTR_COOKIE,
			  p->cookie, p->cookie_len);
	if (p->peer_cookie_len)
		addattr_l(&req.n, 1024, L2TP_ATTR_PEER_COOKIE,
			  p->peer_cookie,  p->peer_cookie_len);
	if (p->ifname)
		addattrstrz(&req.n, 1024, L2TP_ATTR_IFNAME, p->ifname);

	if (rtnl_talk(&genl_rth, &req.n, NULL) < 0)
		return -2;

	return 0;
}

static int delete_session(struct l2tp_parm *p)
{
	GENL_REQUEST(req, 1024, genl_family, 0, L2TP_GENL_VERSION,
		     L2TP_CMD_SESSION_DELETE, NLM_F_REQUEST | NLM_F_ACK);

	addattr32(&req.n, 1024, L2TP_ATTR_CONN_ID, p->tunnel_id);
	addattr32(&req.n, 1024, L2TP_ATTR_SESSION_ID, p->session_id);
	if (rtnl_talk(&genl_rth, &req.n, NULL) < 0)
		return -2;

	return 0;
}

static void print_cookie(const char *name, const char *fmt,
			 const uint8_t *cookie, int len)
{
	char abuf[32];
	size_t n;

	n = snprintf(abuf, sizeof(abuf),
		     "%02x%02x%02x%02x",
		     cookie[0], cookie[1], cookie[2], cookie[3]);
	if (len == 8)
		snprintf(abuf + n, sizeof(abuf) - n,
			 "%02x%02x%02x%02x",
			 cookie[4], cookie[5],
			 cookie[6], cookie[7]);

	print_string(PRINT_ANY, name, fmt, abuf);
}

static void print_tunnel(const struct l2tp_data *data)
{
	const struct l2tp_parm *p = &data->config;
	char buf[INET6_ADDRSTRLEN];

	open_json_object(NULL);
	print_uint(PRINT_ANY, "tunnel_id", "Tunnel %u,", p->tunnel_id);
	print_string(PRINT_ANY, "encap", " encap %s",
		     p->encap == L2TP_ENCAPTYPE_UDP ? "UDP" :
		     p->encap == L2TP_ENCAPTYPE_IP ? "IP" : "??");
	print_nl();

	print_string(PRINT_ANY, "local", "  From %s ",
		     inet_ntop(p->local_ip.family, p->local_ip.data,
			       buf, sizeof(buf)));
	print_string(PRINT_ANY, "peer", "to %s",
		     inet_ntop(p->peer_ip.family, p->peer_ip.data,
			       buf, sizeof(buf)));
	print_nl();

	print_uint(PRINT_ANY, "peer_tunnel", "  Peer tunnel %u",
		   p->peer_tunnel_id);
	print_nl();

	if (p->encap == L2TP_ENCAPTYPE_UDP) {
		print_string(PRINT_FP, NULL,
			     "  UDP source / dest ports:", NULL);

		print_hu(PRINT_ANY, "local_port", " %hu",
			   p->local_udp_port);
		print_hu(PRINT_ANY, "peer_port", "/%hu",
			   p->peer_udp_port);
		print_nl();

		switch (p->local_ip.family) {
		case AF_INET:
			print_bool(PRINT_JSON, "checksum",
				   NULL, p->udp_csum);
			print_string(PRINT_FP, NULL,
				     "  UDP checksum: %s\n",
				     p->udp_csum ? "enabled" : "disabled");
			break;
		case AF_INET6:
			if (is_json_context()) {
				print_bool(PRINT_JSON, "checksum_tx",
					   NULL, p->udp6_csum_tx);

				print_bool(PRINT_JSON, "checksum_rx",
					   NULL, p->udp6_csum_tx);
			} else {
				printf("  UDP checksum: %s%s%s%s\n",
				       p->udp6_csum_tx && p->udp6_csum_rx
				       ? "enabled" : "",
				       p->udp6_csum_tx && !p->udp6_csum_rx
				       ? "tx" : "",
				       !p->udp6_csum_tx && p->udp6_csum_rx
				       ? "rx" : "",
				       !p->udp6_csum_tx && !p->udp6_csum_rx
				       ? "disabled" : "");
			}
			break;
		}
	}
	close_json_object();
}

static void print_session(struct l2tp_data *data)
{
	struct l2tp_parm *p = &data->config;

	open_json_object(NULL);

	print_uint(PRINT_ANY, "session_id", "Session %u", p->session_id);
	print_uint(PRINT_ANY, "tunnel_id",  " in tunnel %u", p->tunnel_id);
	print_nl();

	print_uint(PRINT_ANY, "peer_session_id",
		     "  Peer session %u,", p->peer_session_id);
	print_uint(PRINT_ANY, "peer_tunnel_id",
		     " tunnel %u",  p->peer_tunnel_id);
	print_nl();

	if (p->ifname != NULL) {
		print_color_string(PRINT_ANY, COLOR_IFNAME,
				   "interface", "  interface name: %s" , p->ifname);
		print_nl();
	}

	/* Show offsets only for plain console output (for legacy scripts) */
	print_uint(PRINT_FP, "offset", "  offset %u,", 0);
	print_uint(PRINT_FP, "peer_offset", " peer offset %u\n", 0);

	if (p->cookie_len > 0)
		print_cookie("cookie", "  cookie %s",
			     p->cookie, p->cookie_len);

	if (p->peer_cookie_len > 0)
		print_cookie("peer_cookie", "  peer cookie %s",
			     p->peer_cookie, p->peer_cookie_len);

	if (p->reorder_timeout != 0)
		print_uint(PRINT_ANY, "reorder_timeout",
			   "  reorder timeout: %u", p->reorder_timeout);


	if (p->send_seq || p->recv_seq) {
		print_string(PRINT_FP, NULL, "%s  sequence numbering:", _SL_);

		if (p->send_seq)
			print_null(PRINT_ANY, "send_seq", " send", NULL);
		if (p->recv_seq)
			print_null(PRINT_ANY, "recv_seq", " recv", NULL);

	}
	print_string(PRINT_FP, NULL, "\n", NULL);
	close_json_object();
}

static int get_response(struct nlmsghdr *n, void *arg)
{
	struct genlmsghdr *ghdr;
	struct l2tp_data *data = arg;
	struct l2tp_parm *p = &data->config;
	struct rtattr *attrs[L2TP_ATTR_MAX + 1];
	struct rtattr *nla_stats, *rta;
	int len;

	/* Validate message and parse attributes */
	if (n->nlmsg_type == NLMSG_ERROR)
		return -EBADMSG;

	ghdr = NLMSG_DATA(n);
	len = n->nlmsg_len - NLMSG_LENGTH(sizeof(*ghdr));
	if (len < 0)
		return -1;

	parse_rtattr(attrs, L2TP_ATTR_MAX, (void *)ghdr + GENL_HDRLEN, len);

	if (attrs[L2TP_ATTR_PW_TYPE])
		p->pw_type = rta_getattr_u16(attrs[L2TP_ATTR_PW_TYPE]);
	if (attrs[L2TP_ATTR_ENCAP_TYPE])
		p->encap = rta_getattr_u16(attrs[L2TP_ATTR_ENCAP_TYPE]);
	if (attrs[L2TP_ATTR_CONN_ID])
		p->tunnel_id = rta_getattr_u32(attrs[L2TP_ATTR_CONN_ID]);
	if (attrs[L2TP_ATTR_PEER_CONN_ID])
		p->peer_tunnel_id = rta_getattr_u32(attrs[L2TP_ATTR_PEER_CONN_ID]);
	if (attrs[L2TP_ATTR_SESSION_ID])
		p->session_id = rta_getattr_u32(attrs[L2TP_ATTR_SESSION_ID]);
	if (attrs[L2TP_ATTR_PEER_SESSION_ID])
		p->peer_session_id = rta_getattr_u32(attrs[L2TP_ATTR_PEER_SESSION_ID]);
	if (attrs[L2TP_ATTR_L2SPEC_TYPE])
		p->l2spec_type = rta_getattr_u8(attrs[L2TP_ATTR_L2SPEC_TYPE]);
	if (attrs[L2TP_ATTR_L2SPEC_LEN])
		p->l2spec_len = rta_getattr_u8(attrs[L2TP_ATTR_L2SPEC_LEN]);

	if (attrs[L2TP_ATTR_UDP_CSUM])
		p->udp_csum = !!rta_getattr_u8(attrs[L2TP_ATTR_UDP_CSUM]);

	p->udp6_csum_tx = !attrs[L2TP_ATTR_UDP_ZERO_CSUM6_TX];
	p->udp6_csum_rx = !attrs[L2TP_ATTR_UDP_ZERO_CSUM6_RX];

	if (attrs[L2TP_ATTR_COOKIE])
		memcpy(p->cookie, RTA_DATA(attrs[L2TP_ATTR_COOKIE]),
		       p->cookie_len = RTA_PAYLOAD(attrs[L2TP_ATTR_COOKIE]));

	if (attrs[L2TP_ATTR_PEER_COOKIE])
		memcpy(p->peer_cookie, RTA_DATA(attrs[L2TP_ATTR_PEER_COOKIE]),
		       p->peer_cookie_len = RTA_PAYLOAD(attrs[L2TP_ATTR_PEER_COOKIE]));

	if (attrs[L2TP_ATTR_RECV_SEQ])
		p->recv_seq = !!rta_getattr_u8(attrs[L2TP_ATTR_RECV_SEQ]);
	if (attrs[L2TP_ATTR_SEND_SEQ])
		p->send_seq = !!rta_getattr_u8(attrs[L2TP_ATTR_SEND_SEQ]);

	if (attrs[L2TP_ATTR_RECV_TIMEOUT])
		p->reorder_timeout = rta_getattr_u64(attrs[L2TP_ATTR_RECV_TIMEOUT]);

	rta = attrs[L2TP_ATTR_IP_SADDR];
	p->local_ip.family = AF_INET;
	if (!rta) {
		rta = attrs[L2TP_ATTR_IP6_SADDR];
		p->local_ip.family = AF_INET6;
	}
	if (rta && get_addr_rta(&p->local_ip, rta, p->local_ip.family))
		return -1;

	rta = attrs[L2TP_ATTR_IP_DADDR];
	p->peer_ip.family = AF_INET;
	if (!rta) {
		rta = attrs[L2TP_ATTR_IP6_DADDR];
		p->peer_ip.family = AF_INET6;
	}
	if (rta && get_addr_rta(&p->peer_ip, rta, p->peer_ip.family))
		return -1;

	if (attrs[L2TP_ATTR_UDP_SPORT])
		p->local_udp_port = rta_getattr_u16(attrs[L2TP_ATTR_UDP_SPORT]);
	if (attrs[L2TP_ATTR_UDP_DPORT])
		p->peer_udp_port = rta_getattr_u16(attrs[L2TP_ATTR_UDP_DPORT]);
	if (attrs[L2TP_ATTR_IFNAME])
		p->ifname = rta_getattr_str(attrs[L2TP_ATTR_IFNAME]);

	nla_stats = attrs[L2TP_ATTR_STATS];
	if (nla_stats) {
		struct rtattr *tb[L2TP_ATTR_STATS_MAX + 1];

		parse_rtattr_nested(tb, L2TP_ATTR_STATS_MAX, nla_stats);

		if (tb[L2TP_ATTR_TX_PACKETS])
			data->stats.data_tx_packets = rta_getattr_u64(tb[L2TP_ATTR_TX_PACKETS]);
		if (tb[L2TP_ATTR_TX_BYTES])
			data->stats.data_tx_bytes = rta_getattr_u64(tb[L2TP_ATTR_TX_BYTES]);
		if (tb[L2TP_ATTR_TX_ERRORS])
			data->stats.data_tx_errors = rta_getattr_u64(tb[L2TP_ATTR_TX_ERRORS]);
		if (tb[L2TP_ATTR_RX_PACKETS])
			data->stats.data_rx_packets = rta_getattr_u64(tb[L2TP_ATTR_RX_PACKETS]);
		if (tb[L2TP_ATTR_RX_BYTES])
			data->stats.data_rx_bytes = rta_getattr_u64(tb[L2TP_ATTR_RX_BYTES]);
		if (tb[L2TP_ATTR_RX_ERRORS])
			data->stats.data_rx_errors = rta_getattr_u64(tb[L2TP_ATTR_RX_ERRORS]);
		if (tb[L2TP_ATTR_RX_SEQ_DISCARDS])
			data->stats.data_rx_oos_discards = rta_getattr_u64(tb[L2TP_ATTR_RX_SEQ_DISCARDS]);
		if (tb[L2TP_ATTR_RX_OOS_PACKETS])
			data->stats.data_rx_oos_packets = rta_getattr_u64(tb[L2TP_ATTR_RX_OOS_PACKETS]);
	}

	return 0;
}

static int session_nlmsg(struct nlmsghdr *n, void *arg)
{
	int ret = get_response(n, arg);

	if (ret == 0)
		print_session(arg);

	return ret;
}

static int get_session(struct l2tp_data *p)
{
	GENL_REQUEST(req, 128, genl_family, 0, L2TP_GENL_VERSION,
		     L2TP_CMD_SESSION_GET,
		     NLM_F_ROOT | NLM_F_MATCH | NLM_F_REQUEST);

	req.n.nlmsg_seq = genl_rth.dump = ++genl_rth.seq;

	if (p->config.tunnel_id && p->config.session_id) {
		addattr32(&req.n, 128, L2TP_ATTR_CONN_ID, p->config.tunnel_id);
		addattr32(&req.n, 128, L2TP_ATTR_SESSION_ID,
			  p->config.session_id);
	}

	if (rtnl_send(&genl_rth, &req, req.n.nlmsg_len) < 0)
		return -2;

	new_json_obj(json);
	if (rtnl_dump_filter(&genl_rth, session_nlmsg, p) < 0) {
		fprintf(stderr, "Dump terminated\n");
		exit(1);
	}
	delete_json_obj();
	fflush(stdout);

	return 0;
}

static int tunnel_nlmsg(struct nlmsghdr *n, void *arg)
{
	int ret = get_response(n, arg);

	if (ret == 0)
		print_tunnel(arg);

	return ret;
}

static int get_tunnel(struct l2tp_data *p)
{
	GENL_REQUEST(req, 1024, genl_family, 0, L2TP_GENL_VERSION,
		     L2TP_CMD_TUNNEL_GET,
		     NLM_F_ROOT | NLM_F_MATCH | NLM_F_REQUEST);

	req.n.nlmsg_seq = genl_rth.dump = ++genl_rth.seq;

	if (p->config.tunnel_id)
		addattr32(&req.n, 1024, L2TP_ATTR_CONN_ID, p->config.tunnel_id);

	if (rtnl_send(&genl_rth, &req, req.n.nlmsg_len) < 0)
		return -2;

	new_json_obj(json);
	if (rtnl_dump_filter(&genl_rth, tunnel_nlmsg, p) < 0) {
		fprintf(stderr, "Dump terminated\n");
		exit(1);
	}
	delete_json_obj();
	fflush(stdout);

	return 0;
}

/*****************************************************************************
 * Command parser
 *****************************************************************************/

static void usage(void) __attribute__((noreturn));

static void usage(void)
{
	fprintf(stderr, "Usage: ip l2tp add tunnel\n"
		"          remote ADDR local ADDR\n"
		"          tunnel_id ID peer_tunnel_id ID\n"
		"          [ encap { ip | udp } ]\n"
		"          [ udp_sport PORT ] [ udp_dport PORT ]\n"
		"          [ udp_csum { on | off } ]\n"
		"          [ udp6_csum_tx { on | off } ]\n"
		"          [ udp6_csum_rx { on | off } ]\n"
		"Usage: ip l2tp add session [ name NAME ]\n"
		"          tunnel_id ID\n"
		"          session_id ID peer_session_id ID\n"
		"          [ cookie HEXSTR ] [ peer_cookie HEXSTR ]\n"
		"          [ seq { none | send | recv | both } ]\n"
		"          [ l2spec_type L2SPEC ]\n"
		"       ip l2tp del tunnel tunnel_id ID\n"
		"       ip l2tp del session tunnel_id ID session_id ID\n"
		"       ip l2tp show tunnel [ tunnel_id ID ]\n"
		"       ip l2tp show session [ tunnel_id ID ] [ session_id ID ]\n"
		"\n"
		"Where: NAME   := STRING\n"
		"       ADDR   := { IP_ADDRESS | any }\n"
		"       PORT   := { 0..65535 }\n"
		"       ID     := { 1..4294967295 }\n"
		"       HEXSTR := { 8 or 16 hex digits (4 / 8 bytes) }\n"
		"       L2SPEC := { none | default }\n");

	exit(-1);
}

static int parse_args(int argc, char **argv, int cmd, struct l2tp_parm *p)
{
	memset(p, 0, sizeof(*p));

	if (argc == 0)
		usage();

	/* Defaults */
	p->l2spec_type = L2TP_L2SPECTYPE_DEFAULT;
	p->l2spec_len = 4;
	p->udp6_csum_rx = 1;
	p->udp6_csum_tx = 1;

	while (argc > 0) {
		if (strcmp(*argv, "encap") == 0) {
			NEXT_ARG();
			if (strcmp(*argv, "ip") == 0) {
				p->encap = L2TP_ENCAPTYPE_IP;
			} else if (strcmp(*argv, "udp") == 0) {
				p->encap = L2TP_ENCAPTYPE_UDP;
			} else {
				fprintf(stderr, "Unknown tunnel encapsulation \"%s\"\n", *argv);
				exit(-1);
			}
		} else if (strcmp(*argv, "name") == 0) {
			NEXT_ARG();
			if (check_ifname(*argv))
				invarg("\"name\" not a valid ifname", *argv);
			p->ifname = *argv;
		} else if (strcmp(*argv, "remote") == 0) {
			NEXT_ARG();
			if (get_addr(&p->peer_ip, *argv, AF_UNSPEC))
				invarg("invalid remote address\n", *argv);
		} else if (strcmp(*argv, "local") == 0) {
			NEXT_ARG();
			if (get_addr(&p->local_ip, *argv, AF_UNSPEC))
				invarg("invalid local address\n", *argv);
		} else if ((strcmp(*argv, "tunnel_id") == 0) ||
			   (strcmp(*argv, "tid") == 0)) {
			__u32 uval;

			NEXT_ARG();
			if (get_u32(&uval, *argv, 0))
				invarg("invalid ID\n", *argv);
			p->tunnel_id = uval;
		} else if ((strcmp(*argv, "peer_tunnel_id") == 0) ||
			   (strcmp(*argv, "ptid") == 0)) {
			__u32 uval;

			NEXT_ARG();
			if (get_u32(&uval, *argv, 0))
				invarg("invalid ID\n", *argv);
			p->peer_tunnel_id = uval;
		} else if ((strcmp(*argv, "session_id") == 0) ||
			   (strcmp(*argv, "sid") == 0)) {
			__u32 uval;

			NEXT_ARG();
			if (get_u32(&uval, *argv, 0))
				invarg("invalid ID\n", *argv);
			p->session_id = uval;
		} else if ((strcmp(*argv, "peer_session_id") == 0) ||
			   (strcmp(*argv, "psid") == 0)) {
			__u32 uval;

			NEXT_ARG();
			if (get_u32(&uval, *argv, 0))
				invarg("invalid ID\n", *argv);
			p->peer_session_id = uval;
		} else if (strcmp(*argv, "udp_sport") == 0) {
			__u16 uval;

			NEXT_ARG();
			if (get_u16(&uval, *argv, 0))
				invarg("invalid port\n", *argv);
			p->local_udp_port = uval;
		} else if (strcmp(*argv, "udp_dport") == 0) {
			__u16 uval;

			NEXT_ARG();
			if (get_u16(&uval, *argv, 0))
				invarg("invalid port\n", *argv);
			p->peer_udp_port = uval;
		} else if (strcmp(*argv, "udp_csum") == 0) {
			NEXT_ARG();
			if (strcmp(*argv, "on") == 0)
				p->udp_csum = 1;
			else if (strcmp(*argv, "off") == 0)
				p->udp_csum = 0;
			else
				invarg("invalid option for udp_csum\n", *argv);
		} else if (strcmp(*argv, "udp6_csum_rx") == 0) {
			NEXT_ARG();
			if (strcmp(*argv, "on") == 0)
				p->udp6_csum_rx = 1;
			else if (strcmp(*argv, "off") == 0)
				p->udp6_csum_rx = 0;
			else
				invarg("invalid option for udp6_csum_rx\n"
						, *argv);
		} else if (strcmp(*argv, "udp6_csum_tx") == 0) {
			NEXT_ARG();
			if (strcmp(*argv, "on") == 0)
				p->udp6_csum_tx = 1;
			else if (strcmp(*argv, "off") == 0)
				p->udp6_csum_tx = 0;
			else
				invarg("invalid option for udp6_csum_tx\n"
						, *argv);
		} else if (strcmp(*argv, "offset") == 0) {
			fprintf(stderr, "Ignoring option \"offset\"\n");
			NEXT_ARG();
		} else if (strcmp(*argv, "peer_offset") == 0) {
			fprintf(stderr, "Ignoring option \"peer_offset\"\n");
			NEXT_ARG();
		} else if (strcmp(*argv, "cookie") == 0) {
			int slen;

			NEXT_ARG();
			slen = strlen(*argv);
			if ((slen != 8) && (slen != 16))
				invarg("cookie must be either 8 or 16 hex digits\n", *argv);

			p->cookie_len = slen / 2;
			if (hex2mem(*argv, p->cookie, p->cookie_len) < 0)
				invarg("cookie must be a hex string\n", *argv);
		} else if (strcmp(*argv, "peer_cookie") == 0) {
			int slen;

			NEXT_ARG();
			slen = strlen(*argv);
			if ((slen != 8) && (slen != 16))
				invarg("cookie must be either 8 or 16 hex digits\n", *argv);

			p->peer_cookie_len = slen / 2;
			if (hex2mem(*argv, p->peer_cookie, p->peer_cookie_len) < 0)
				invarg("cookie must be a hex string\n", *argv);
		} else if (strcmp(*argv, "l2spec_type") == 0) {
			NEXT_ARG();
			if (strcasecmp(*argv, "default") == 0) {
				p->l2spec_type = L2TP_L2SPECTYPE_DEFAULT;
				p->l2spec_len = 4;
			} else if (strcasecmp(*argv, "none") == 0) {
				p->l2spec_type = L2TP_L2SPECTYPE_NONE;
				p->l2spec_len = 0;
			} else {
				fprintf(stderr,
					"Unknown layer2specific header type \"%s\"\n",
					*argv);
				exit(-1);
			}
		} else if (strcmp(*argv, "seq") == 0) {
			NEXT_ARG();
			if (strcasecmp(*argv, "both") == 0) {
				p->recv_seq = 1;
				p->send_seq = 1;
			} else if (strcasecmp(*argv, "recv") == 0) {
				p->recv_seq = 1;
			} else if (strcasecmp(*argv, "send") == 0) {
				p->send_seq = 1;
			} else if (strcasecmp(*argv, "none") == 0) {
				p->recv_seq = 0;
				p->send_seq = 0;
			} else {
				fprintf(stderr,
					"Unknown seq value \"%s\"\n", *argv);
				exit(-1);
			}
		} else if (strcmp(*argv, "tunnel") == 0) {
			p->tunnel = 1;
		} else if (strcmp(*argv, "session") == 0) {
			p->session = 1;
		} else if (matches(*argv, "help") == 0) {
			usage();
		} else {
			fprintf(stderr, "Unknown command: %s\n", *argv);
			usage();
		}

		argc--; argv++;
	}

	return 0;
}


static int do_add(int argc, char **argv)
{
	struct l2tp_parm p;
	int ret = 0;

	if (parse_args(argc, argv, L2TP_ADD, &p) < 0)
		return -1;

	if (!p.tunnel && !p.session)
		missarg("tunnel or session");

	if (p.tunnel_id == 0)
		missarg("tunnel_id");

	/* session_id and peer_session_id must be provided for sessions */
	if ((p.session) && (p.peer_session_id == 0))
		missarg("peer_session_id");
	if ((p.session) && (p.session_id == 0))
		missarg("session_id");

	/* peer_tunnel_id is needed for tunnels */
	if ((p.tunnel) && (p.peer_tunnel_id == 0))
		missarg("peer_tunnel_id");

	if (p.tunnel) {
		if (p.local_ip.family == AF_UNSPEC)
			missarg("local");

		if (p.peer_ip.family == AF_UNSPEC)
			missarg("remote");

		if (p.encap == L2TP_ENCAPTYPE_UDP) {
			if (p.local_udp_port == 0)
				missarg("udp_sport");
			if (p.peer_udp_port == 0)
				missarg("udp_dport");
		}

		ret = create_tunnel(&p);
	}

	if (p.session) {
		/* Only ethernet pseudowires supported */
		p.pw_type = L2TP_PWTYPE_ETH;

		ret = create_session(&p);
	}

	return ret;
}

static int do_del(int argc, char **argv)
{
	struct l2tp_parm p;

	if (parse_args(argc, argv, L2TP_DEL, &p) < 0)
		return -1;

	if (!p.tunnel && !p.session)
		missarg("tunnel or session");

	if ((p.tunnel) && (p.tunnel_id == 0))
		missarg("tunnel_id");
	if ((p.session) && (p.session_id == 0))
		missarg("session_id");

	if (p.session_id)
		return delete_session(&p);
	else
		return delete_tunnel(&p);

	return -1;
}

static int do_show(int argc, char **argv)
{
	struct l2tp_data data;
	struct l2tp_parm *p = &data.config;

	if (parse_args(argc, argv, L2TP_GET, p) < 0)
		return -1;

	if (!p->tunnel && !p->session)
		missarg("tunnel or session");

	if (p->session)
		get_session(&data);
	else
		get_tunnel(&data);

	return 0;
}

int do_ipl2tp(int argc, char **argv)
{
	if (argc < 1 || !matches(*argv, "help"))
		usage();

	if (genl_init_handle(&genl_rth, L2TP_GENL_NAME, &genl_family))
		exit(1);

	if (matches(*argv, "add") == 0)
		return do_add(argc-1, argv+1);
	if (matches(*argv, "delete") == 0)
		return do_del(argc-1, argv+1);
	if (matches(*argv, "show") == 0 ||
	    matches(*argv, "lst") == 0 ||
	    matches(*argv, "list") == 0)
		return do_show(argc-1, argv+1);

	fprintf(stderr,
		"Command \"%s\" is unknown, try \"ip l2tp help\".\n", *argv);
	exit(-1);
}
