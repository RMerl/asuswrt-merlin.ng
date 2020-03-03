/*
 * lib/route/link.c	Links (Interfaces)
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2012 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup rtnl
 * @defgroup link Links (Interfaces)
 *
 * @details
 * @route_doc{route_link, Link Documentation}
 * @{
 */

#include <netlink-private/netlink.h>
#include <netlink/netlink.h>
#include <netlink/attr.h>
#include <netlink/utils.h>
#include <netlink/object.h>
#include <netlink/hashtable.h>
#include <netlink/data.h>
#include <netlink/route/rtnl.h>
#include <netlink/route/link.h>
#include <netlink-private/route/link/api.h>

/** @cond SKIP */
#define LINK_ATTR_MTU		(1 <<  0)
#define LINK_ATTR_LINK		(1 <<  1)
#define LINK_ATTR_TXQLEN	(1 <<  2)
#define LINK_ATTR_WEIGHT	(1 <<  3)
#define LINK_ATTR_MASTER	(1 <<  4)
#define LINK_ATTR_QDISC		(1 <<  5)
#define LINK_ATTR_MAP		(1 <<  6)
#define LINK_ATTR_ADDR		(1 <<  7)
#define LINK_ATTR_BRD		(1 <<  8)
#define LINK_ATTR_FLAGS		(1 <<  9)
#define LINK_ATTR_IFNAME	(1 << 10)
#define LINK_ATTR_IFINDEX	(1 << 11)
#define LINK_ATTR_FAMILY	(1 << 12)
#define LINK_ATTR_ARPTYPE	(1 << 13)
#define LINK_ATTR_STATS		(1 << 14)
#define LINK_ATTR_CHANGE	(1 << 15)
#define LINK_ATTR_OPERSTATE	(1 << 16)
#define LINK_ATTR_LINKMODE	(1 << 17)
#define LINK_ATTR_LINKINFO	(1 << 18)
#define LINK_ATTR_IFALIAS	(1 << 19)
#define LINK_ATTR_NUM_VF	(1 << 20)
#define LINK_ATTR_PROMISCUITY	(1 << 21)
#define LINK_ATTR_NUM_TX_QUEUES	(1 << 22)
#define LINK_ATTR_NUM_RX_QUEUES	(1 << 23)
#define LINK_ATTR_GROUP		(1 << 24)
#define LINK_ATTR_CARRIER	(1 << 25)
#define LINK_ATTR_PROTINFO	(1 << 26)
#define LINK_ATTR_AF_SPEC	(1 << 27)
#define LINK_ATTR_PHYS_PORT_ID	(1 << 28)
#define LINK_ATTR_NS_FD		(1 << 29)
#define LINK_ATTR_NS_PID	(1 << 30)

static struct nl_cache_ops rtnl_link_ops;
static struct nl_object_ops link_obj_ops;
/** @endcond */

static struct rtnl_link_af_ops *af_lookup_and_alloc(struct rtnl_link *link,
						    int family)
{
	struct rtnl_link_af_ops *af_ops;
	void *data;

	af_ops = rtnl_link_af_ops_lookup(family);
	if (!af_ops)
		return NULL;

	if (!(data = rtnl_link_af_alloc(link, af_ops))) {
		rtnl_link_af_ops_put(af_ops);
		return NULL;
	}

	return af_ops;
}

static int af_free(struct rtnl_link *link, struct rtnl_link_af_ops *ops,
		    void *data, void *arg)
{
	if (ops->ao_free)
		ops->ao_free(link, data);

	rtnl_link_af_ops_put(ops);

	return 0;
}

static int af_clone(struct rtnl_link *link, struct rtnl_link_af_ops *ops,
		    void *data, void *arg)
{
	struct rtnl_link *dst = arg;

	if (ops->ao_clone &&
	    !(dst->l_af_data[ops->ao_family] = ops->ao_clone(dst, data)))
		return -NLE_NOMEM;

	return 0;
}

static int af_fill(struct rtnl_link *link, struct rtnl_link_af_ops *ops,
		   void *data, void *arg)
{
	struct nl_msg *msg = arg;
	struct nlattr *af_attr;
	int err;

	if (!ops->ao_fill_af)
		return 0;

	if (!(af_attr = nla_nest_start(msg, ops->ao_family)))
		return -NLE_MSGSIZE;

	if ((err = ops->ao_fill_af(link, arg, data)) < 0)
		return err;

	nla_nest_end(msg, af_attr);

	return 0;
}

static int af_dump_line(struct rtnl_link *link, struct rtnl_link_af_ops *ops,
			 void *data, void *arg)
{
	struct nl_dump_params *p = arg;

	if (ops->ao_dump[NL_DUMP_LINE])
		ops->ao_dump[NL_DUMP_LINE](link, p, data);

	return 0;
}

static int af_dump_details(struct rtnl_link *link, struct rtnl_link_af_ops *ops,
			   void *data, void *arg)
{
	struct nl_dump_params *p = arg;

	if (ops->ao_dump[NL_DUMP_DETAILS])
		ops->ao_dump[NL_DUMP_DETAILS](link, p, data);

	return 0;
}

static int af_dump_stats(struct rtnl_link *link, struct rtnl_link_af_ops *ops,
			 void *data, void *arg)
{
	struct nl_dump_params *p = arg;

	if (ops->ao_dump[NL_DUMP_STATS])
		ops->ao_dump[NL_DUMP_STATS](link, p, data);

	return 0;
}

static int do_foreach_af(struct rtnl_link *link,
			 int (*cb)(struct rtnl_link *,
			 	   struct rtnl_link_af_ops *, void *, void *),
			 void *arg)
{
	int i, err;

	for (i = 0; i < AF_MAX; i++) {
		if (link->l_af_data[i]) {
			struct rtnl_link_af_ops *ops;

			if (!(ops = rtnl_link_af_ops_lookup(i)))
				BUG();

			err = cb(link, ops, link->l_af_data[i], arg);

			rtnl_link_af_ops_put(ops);

			if (err < 0)
				return err;
		}
	}

	return 0;
}

static void release_link_info(struct rtnl_link *link)
{
	struct rtnl_link_info_ops *io = link->l_info_ops;

	if (io != NULL) {
		if (io->io_free)
			io->io_free(link);
		else {
			/* Catch missing io_free() implementations */
			BUG_ON(link->l_info);
		}
		rtnl_link_info_ops_put(io);
		link->l_info_ops = NULL;
	}
}

static void link_free_data(struct nl_object *c)
{
	struct rtnl_link *link = nl_object_priv(c);

	if (link) {
		struct rtnl_link_info_ops *io;

		if ((io = link->l_info_ops) != NULL)
			release_link_info(link);

		/* proto info af reference */
		rtnl_link_af_ops_put(link->l_af_ops);

		nl_addr_put(link->l_addr);
		nl_addr_put(link->l_bcast);

		free(link->l_ifalias);
		free(link->l_info_kind);

		do_foreach_af(link, af_free, NULL);

		nl_data_free(link->l_phys_port_id);
	}
}

static int link_clone(struct nl_object *_dst, struct nl_object *_src)
{
	struct rtnl_link *dst = nl_object_priv(_dst);
	struct rtnl_link *src = nl_object_priv(_src);
	int err;

	if (src->l_addr)
		if (!(dst->l_addr = nl_addr_clone(src->l_addr)))
			return -NLE_NOMEM;

	if (src->l_bcast)
		if (!(dst->l_bcast = nl_addr_clone(src->l_bcast)))
			return -NLE_NOMEM;

	if (src->l_ifalias)
		if (!(dst->l_ifalias = strdup(src->l_ifalias)))
			return -NLE_NOMEM;

	if (src->l_info_kind)
		if (!(dst->l_info_kind = strdup(src->l_info_kind)))
			return -NLE_NOMEM;

	if (src->l_info_ops && src->l_info_ops->io_clone) {
		err = src->l_info_ops->io_clone(dst, src);
		if (err < 0)
			return err;
	}

	if ((err = do_foreach_af(src, af_clone, dst)) < 0)
		return err;

	if (src->l_phys_port_id)
		if (!(dst->l_phys_port_id = nl_data_clone(src->l_phys_port_id)))
			return -NLE_NOMEM;

	return 0;
}

struct nla_policy rtln_link_policy[IFLA_MAX+1] = {
	[IFLA_IFNAME]		= { .type = NLA_STRING,
				    .maxlen = IFNAMSIZ },
	[IFLA_MTU]		= { .type = NLA_U32 },
	[IFLA_TXQLEN]		= { .type = NLA_U32 },
	[IFLA_LINK]		= { .type = NLA_U32 },
	[IFLA_WEIGHT]		= { .type = NLA_U32 },
	[IFLA_MASTER]		= { .type = NLA_U32 },
	[IFLA_OPERSTATE]	= { .type = NLA_U8 },
	[IFLA_LINKMODE] 	= { .type = NLA_U8 },
	[IFLA_LINKINFO]		= { .type = NLA_NESTED },
	[IFLA_QDISC]		= { .type = NLA_STRING,
				    .maxlen = IFQDISCSIZ },
	[IFLA_STATS]		= { .minlen = sizeof(struct rtnl_link_stats) },
	[IFLA_STATS64]		= { .minlen = sizeof(struct rtnl_link_stats64)},
	[IFLA_MAP]		= { .minlen = sizeof(struct rtnl_link_ifmap) },
	[IFLA_IFALIAS]		= { .type = NLA_STRING, .maxlen = IFALIASZ },
	[IFLA_NUM_VF]		= { .type = NLA_U32 },
	[IFLA_AF_SPEC]		= { .type = NLA_NESTED },
	[IFLA_PROMISCUITY]	= { .type = NLA_U32 },
	[IFLA_NUM_TX_QUEUES]	= { .type = NLA_U32 },
	[IFLA_NUM_RX_QUEUES]	= { .type = NLA_U32 },
	[IFLA_GROUP]		= { .type = NLA_U32 },
	[IFLA_CARRIER]		= { .type = NLA_U8 },
	[IFLA_PHYS_PORT_ID]	= { .type = NLA_UNSPEC },
	[IFLA_NET_NS_PID]	= { .type = NLA_U32 },
	[IFLA_NET_NS_FD]	= { .type = NLA_U32 },
};

static struct nla_policy link_info_policy[IFLA_INFO_MAX+1] = {
	[IFLA_INFO_KIND]	= { .type = NLA_STRING },
	[IFLA_INFO_DATA]	= { .type = NLA_NESTED },
	[IFLA_INFO_XSTATS]	= { .type = NLA_NESTED },
};

int rtnl_link_info_parse(struct rtnl_link *link, struct nlattr **tb)
{
	if (tb[IFLA_IFNAME] == NULL)
		return -NLE_MISSING_ATTR;

	nla_strlcpy(link->l_name, tb[IFLA_IFNAME], IFNAMSIZ);

	if (tb[IFLA_STATS]) {
		struct rtnl_link_stats *st = nla_data(tb[IFLA_STATS]);

		link->l_stats[RTNL_LINK_RX_PACKETS]	= st->rx_packets;
		link->l_stats[RTNL_LINK_TX_PACKETS]	= st->tx_packets;
		link->l_stats[RTNL_LINK_RX_BYTES]	= st->rx_bytes;
		link->l_stats[RTNL_LINK_TX_BYTES]	= st->tx_bytes;
		link->l_stats[RTNL_LINK_RX_ERRORS]	= st->rx_errors;
		link->l_stats[RTNL_LINK_TX_ERRORS]	= st->tx_errors;
		link->l_stats[RTNL_LINK_RX_DROPPED]	= st->rx_dropped;
		link->l_stats[RTNL_LINK_TX_DROPPED]	= st->tx_dropped;
		link->l_stats[RTNL_LINK_MULTICAST]	= st->multicast;
		link->l_stats[RTNL_LINK_COLLISIONS]	= st->collisions;

		link->l_stats[RTNL_LINK_RX_LEN_ERR]	= st->rx_length_errors;
		link->l_stats[RTNL_LINK_RX_OVER_ERR]	= st->rx_over_errors;
		link->l_stats[RTNL_LINK_RX_CRC_ERR]	= st->rx_crc_errors;
		link->l_stats[RTNL_LINK_RX_FRAME_ERR]	= st->rx_frame_errors;
		link->l_stats[RTNL_LINK_RX_FIFO_ERR]	= st->rx_fifo_errors;
		link->l_stats[RTNL_LINK_RX_MISSED_ERR]	= st->rx_missed_errors;

		link->l_stats[RTNL_LINK_TX_ABORT_ERR]	= st->tx_aborted_errors;
		link->l_stats[RTNL_LINK_TX_CARRIER_ERR]	= st->tx_carrier_errors;
		link->l_stats[RTNL_LINK_TX_FIFO_ERR]	= st->tx_fifo_errors;
		link->l_stats[RTNL_LINK_TX_HBEAT_ERR]	= st->tx_heartbeat_errors;
		link->l_stats[RTNL_LINK_TX_WIN_ERR]	= st->tx_window_errors;

		link->l_stats[RTNL_LINK_RX_COMPRESSED]	= st->rx_compressed;
		link->l_stats[RTNL_LINK_TX_COMPRESSED]	= st->tx_compressed;

		link->ce_mask |= LINK_ATTR_STATS;
	}

	if (tb[IFLA_STATS64]) {
		/*
		 * This structure contains 64bit parameters, and per the
		 * documentation in lib/attr.c, must not be accessed
		 * directly (because of alignment to 4 instead of 8).
		 * Therefore, copy the data to the stack and access it from
		 * there, where it will be aligned to 8.
		 */
		struct rtnl_link_stats64 st;

		nla_memcpy(&st, tb[IFLA_STATS64],
			   sizeof(struct rtnl_link_stats64));

		link->l_stats[RTNL_LINK_RX_PACKETS]	= st.rx_packets;
		link->l_stats[RTNL_LINK_TX_PACKETS]	= st.tx_packets;
		link->l_stats[RTNL_LINK_RX_BYTES]	= st.rx_bytes;
		link->l_stats[RTNL_LINK_TX_BYTES]	= st.tx_bytes;
		link->l_stats[RTNL_LINK_RX_ERRORS]	= st.rx_errors;
		link->l_stats[RTNL_LINK_TX_ERRORS]	= st.tx_errors;
		link->l_stats[RTNL_LINK_RX_DROPPED]	= st.rx_dropped;
		link->l_stats[RTNL_LINK_TX_DROPPED]	= st.tx_dropped;
		link->l_stats[RTNL_LINK_MULTICAST]	= st.multicast;
		link->l_stats[RTNL_LINK_COLLISIONS]	= st.collisions;

		link->l_stats[RTNL_LINK_RX_LEN_ERR]	= st.rx_length_errors;
		link->l_stats[RTNL_LINK_RX_OVER_ERR]	= st.rx_over_errors;
		link->l_stats[RTNL_LINK_RX_CRC_ERR]	= st.rx_crc_errors;
		link->l_stats[RTNL_LINK_RX_FRAME_ERR]	= st.rx_frame_errors;
		link->l_stats[RTNL_LINK_RX_FIFO_ERR]	= st.rx_fifo_errors;
		link->l_stats[RTNL_LINK_RX_MISSED_ERR]	= st.rx_missed_errors;

		link->l_stats[RTNL_LINK_TX_ABORT_ERR]	= st.tx_aborted_errors;
		link->l_stats[RTNL_LINK_TX_CARRIER_ERR]	= st.tx_carrier_errors;
		link->l_stats[RTNL_LINK_TX_FIFO_ERR]	= st.tx_fifo_errors;
		link->l_stats[RTNL_LINK_TX_HBEAT_ERR]	= st.tx_heartbeat_errors;
		link->l_stats[RTNL_LINK_TX_WIN_ERR]	= st.tx_window_errors;

		link->l_stats[RTNL_LINK_RX_COMPRESSED]	= st.rx_compressed;
		link->l_stats[RTNL_LINK_TX_COMPRESSED]	= st.tx_compressed;

		link->ce_mask |= LINK_ATTR_STATS;
	}

	if (tb[IFLA_TXQLEN]) {
		link->l_txqlen = nla_get_u32(tb[IFLA_TXQLEN]);
		link->ce_mask |= LINK_ATTR_TXQLEN;
	}

	if (tb[IFLA_MTU]) {
		link->l_mtu = nla_get_u32(tb[IFLA_MTU]);
		link->ce_mask |= LINK_ATTR_MTU;
	}

	if (tb[IFLA_ADDRESS]) {
		link->l_addr = nl_addr_alloc_attr(tb[IFLA_ADDRESS], AF_UNSPEC);
		if (link->l_addr == NULL)
			return -NLE_NOMEM;
		nl_addr_set_family(link->l_addr,
				   nl_addr_guess_family(link->l_addr));
		link->ce_mask |= LINK_ATTR_ADDR;
	}

	if (tb[IFLA_BROADCAST]) {
		link->l_bcast = nl_addr_alloc_attr(tb[IFLA_BROADCAST],
						   AF_UNSPEC);
		if (link->l_bcast == NULL)
			return -NLE_NOMEM;
		nl_addr_set_family(link->l_bcast,
				   nl_addr_guess_family(link->l_bcast));
		link->ce_mask |= LINK_ATTR_BRD;
	}

	if (tb[IFLA_LINK]) {
		link->l_link = nla_get_u32(tb[IFLA_LINK]);
		link->ce_mask |= LINK_ATTR_LINK;
	}

	if (tb[IFLA_WEIGHT]) {
		link->l_weight = nla_get_u32(tb[IFLA_WEIGHT]);
		link->ce_mask |= LINK_ATTR_WEIGHT;
	}

	if (tb[IFLA_QDISC]) {
		nla_strlcpy(link->l_qdisc, tb[IFLA_QDISC], IFQDISCSIZ);
		link->ce_mask |= LINK_ATTR_QDISC;
	}

	if (tb[IFLA_MAP]) {
		nla_memcpy(&link->l_map, tb[IFLA_MAP],
			   sizeof(struct rtnl_link_ifmap));
		link->ce_mask |= LINK_ATTR_MAP;
	}

	if (tb[IFLA_MASTER]) {
		link->l_master = nla_get_u32(tb[IFLA_MASTER]);
		link->ce_mask |= LINK_ATTR_MASTER;
	}

	if (tb[IFLA_CARRIER]) {
		link->l_carrier = nla_get_u8(tb[IFLA_CARRIER]);
		link->ce_mask |= LINK_ATTR_CARRIER;
	}

	if (tb[IFLA_OPERSTATE]) {
		link->l_operstate = nla_get_u8(tb[IFLA_OPERSTATE]);
		link->ce_mask |= LINK_ATTR_OPERSTATE;
	}

	if (tb[IFLA_LINKMODE]) {
		link->l_linkmode = nla_get_u8(tb[IFLA_LINKMODE]);
		link->ce_mask |= LINK_ATTR_LINKMODE;
	}

	if (tb[IFLA_IFALIAS]) {
		link->l_ifalias = nla_strdup(tb[IFLA_IFALIAS]);
		if (link->l_ifalias == NULL)
			return -NLE_NOMEM;
		link->ce_mask |= LINK_ATTR_IFALIAS;
	}

	if (tb[IFLA_NET_NS_FD]) {
		link->l_ns_fd = nla_get_u32(tb[IFLA_NET_NS_FD]);
		link->ce_mask |= LINK_ATTR_NS_FD;
	}

	if (tb[IFLA_NET_NS_PID]) {
		link->l_ns_pid = nla_get_u32(tb[IFLA_NET_NS_PID]);
		link->ce_mask |= LINK_ATTR_NS_PID;
	}

	return 0;
}

static int link_msg_parser(struct nl_cache_ops *ops, struct sockaddr_nl *who,
			   struct nlmsghdr *n, struct nl_parser_param *pp)
{
	struct rtnl_link *link;
	struct ifinfomsg *ifi;
	struct nlattr *tb[IFLA_MAX+1];
	struct rtnl_link_af_ops *af_ops = NULL;
	int err, family;
	struct nla_policy real_link_policy[IFLA_MAX+1];

	memcpy(&real_link_policy, rtln_link_policy, sizeof(rtln_link_policy));

	link = rtnl_link_alloc();
	if (link == NULL) {
		err = -NLE_NOMEM;
		goto errout;
	}

	link->ce_msgtype = n->nlmsg_type;

	if (!nlmsg_valid_hdr(n, sizeof(*ifi)))
		return -NLE_MSG_TOOSHORT;

	ifi = nlmsg_data(n);
	link->l_family = family = ifi->ifi_family;
	link->l_arptype = ifi->ifi_type;
	link->l_index = ifi->ifi_index;
	link->l_flags = ifi->ifi_flags;
	link->l_change = ifi->ifi_change;
	link->ce_mask = (LINK_ATTR_IFNAME | LINK_ATTR_FAMILY |
			  LINK_ATTR_ARPTYPE| LINK_ATTR_IFINDEX |
			  LINK_ATTR_FLAGS | LINK_ATTR_CHANGE);

	if ((af_ops = af_lookup_and_alloc(link, family))) {
		if (af_ops->ao_protinfo_policy) {
			memcpy(&real_link_policy[IFLA_PROTINFO],
			       af_ops->ao_protinfo_policy,
			       sizeof(struct nla_policy));
		}

		link->l_af_ops = af_ops;
	}

	err = nlmsg_parse(n, sizeof(*ifi), tb, IFLA_MAX, real_link_policy);
	if (err < 0)
		return err;

	err = rtnl_link_info_parse(link, tb);
	if (err < 0)
		return err;

	if (tb[IFLA_NUM_VF]) {
		link->l_num_vf = nla_get_u32(tb[IFLA_NUM_VF]);
		link->ce_mask |= LINK_ATTR_NUM_VF;
	}

	if (tb[IFLA_LINKINFO]) {
		struct nlattr *li[IFLA_INFO_MAX+1];

		err = nla_parse_nested(li, IFLA_INFO_MAX, tb[IFLA_LINKINFO],
				       link_info_policy);
		if (err < 0)
			goto errout;

		if (li[IFLA_INFO_KIND]) {
			struct rtnl_link_info_ops *ops;
			char *kind = nla_get_string(li[IFLA_INFO_KIND]);
			int af;

			err = rtnl_link_set_type(link, kind);
			if (err < 0)
				goto errout;

			if ((af = nl_str2af(kind)) >= 0 &&
				!af_ops && (af_ops = af_lookup_and_alloc(link, af))) {

				if (af_ops->ao_protinfo_policy) {
					tb[IFLA_PROTINFO] = (struct nlattr *)af_ops->ao_protinfo_policy;
				}
				link->l_family = family = af;
				link->l_af_ops = af_ops;
			}

			ops = rtnl_link_info_ops_lookup(kind);
			link->l_info_ops = ops;

			if (ops) {
				if (ops->io_parse &&
				    (li[IFLA_INFO_DATA] || li[IFLA_INFO_XSTATS])) {
					err = ops->io_parse(link, li[IFLA_INFO_DATA],
							    li[IFLA_INFO_XSTATS]);
					if (err < 0)
						goto errout;
				} else {
				}
			}
		}
		link->ce_mask |= LINK_ATTR_LINKINFO;
	}

	if (tb[IFLA_PROTINFO] && af_ops && af_ops->ao_parse_protinfo) {
		err = af_ops->ao_parse_protinfo(link, tb[IFLA_PROTINFO],
						link->l_af_data[link->l_family]);
		if (err < 0)
			goto errout;
		link->ce_mask |= LINK_ATTR_PROTINFO;
	}

	if (tb[IFLA_AF_SPEC]) {
		struct nlattr *af_attr;
		int remaining;

		nla_for_each_nested(af_attr, tb[IFLA_AF_SPEC], remaining) {
			af_ops = af_lookup_and_alloc(link, nla_type(af_attr));
			if (af_ops && af_ops->ao_parse_af) {
				char *af_data = link->l_af_data[nla_type(af_attr)];

				err = af_ops->ao_parse_af(link, af_attr, af_data);
				if (err < 0)
					goto errout;
			}

		}
		link->ce_mask |= LINK_ATTR_AF_SPEC;
	}

	if (tb[IFLA_PROMISCUITY]) {
		link->l_promiscuity = nla_get_u32(tb[IFLA_PROMISCUITY]);
		link->ce_mask |= LINK_ATTR_PROMISCUITY;
	}

	if (tb[IFLA_NUM_TX_QUEUES]) {
		link->l_num_tx_queues = nla_get_u32(tb[IFLA_NUM_TX_QUEUES]);
		link->ce_mask |= LINK_ATTR_NUM_TX_QUEUES;
	}

	if (tb[IFLA_NUM_RX_QUEUES]) {
		link->l_num_rx_queues = nla_get_u32(tb[IFLA_NUM_RX_QUEUES]);
		link->ce_mask |= LINK_ATTR_NUM_RX_QUEUES;
	}

	if (tb[IFLA_GROUP]) {
		link->l_group = nla_get_u32(tb[IFLA_GROUP]);
		link->ce_mask |= LINK_ATTR_GROUP;
	}

	if (tb[IFLA_PHYS_PORT_ID]) {
		link->l_phys_port_id = nl_data_alloc_attr(tb[IFLA_PHYS_PORT_ID]);
		if (link->l_phys_port_id == NULL) {
			err = -NLE_NOMEM;
			goto errout;
		}
		link->ce_mask |= LINK_ATTR_PHYS_PORT_ID;
	}

	err = pp->pp_cb((struct nl_object *) link, pp);
errout:
	rtnl_link_af_ops_put(af_ops);
	rtnl_link_put(link);
	return err;
}

static int link_request_update(struct nl_cache *cache, struct nl_sock *sk)
{
	int family = cache->c_iarg1;

	return nl_rtgen_request(sk, RTM_GETLINK, family, NLM_F_DUMP);
}

static void link_dump_line(struct nl_object *obj, struct nl_dump_params *p)
{
	char buf[128];
	struct nl_cache *cache = obj->ce_cache;
	struct rtnl_link *link = (struct rtnl_link *) obj;
	int fetched_cache = 0;

	if (!cache) {
		cache = nl_cache_mngt_require_safe("route/link");
		fetched_cache = 1;
	}

	nl_dump_line(p, "%s %s ", link->l_name,
		     nl_llproto2str(link->l_arptype, buf, sizeof(buf)));

	if (link->l_addr && !nl_addr_iszero(link->l_addr))
		nl_dump(p, "%s ", nl_addr2str(link->l_addr, buf, sizeof(buf)));

	if (link->ce_mask & LINK_ATTR_MASTER) {
		if (cache) {
			struct rtnl_link *master = rtnl_link_get(cache, link->l_master);
			nl_dump(p, "master %s ", master ? master->l_name : "inv");
			if (master)
				rtnl_link_put(master);
		} else
			nl_dump(p, "master %d ", link->l_master);
	}

	rtnl_link_flags2str(link->l_flags, buf, sizeof(buf));
	if (buf[0])
		nl_dump(p, "<%s> ", buf);

	if (link->ce_mask & LINK_ATTR_LINK) {
		if (cache) {
			struct rtnl_link *ll = rtnl_link_get(cache, link->l_link);
			nl_dump(p, "slave-of %s ", ll ? ll->l_name : "NONE");
			if (ll)
				rtnl_link_put(ll);
		} else
			nl_dump(p, "slave-of %d ", link->l_link);
	}

	if (link->ce_mask & LINK_ATTR_GROUP)
		nl_dump(p, "group %u ", link->l_group);

	if (link->l_info_ops && link->l_info_ops->io_dump[NL_DUMP_LINE])
		link->l_info_ops->io_dump[NL_DUMP_LINE](link, p);

	do_foreach_af(link, af_dump_line, p);

	nl_dump(p, "\n");

	if (fetched_cache)
		nl_cache_put(cache);
}

static void link_dump_details(struct nl_object *obj, struct nl_dump_params *p)
{
	struct rtnl_link *link = (struct rtnl_link *) obj;
	char buf[64];

	link_dump_line(obj, p);

	nl_dump_line(p, "    mtu %u ", link->l_mtu);
	nl_dump(p, "txqlen %u weight %u ", link->l_txqlen, link->l_weight);

	if (link->ce_mask & LINK_ATTR_QDISC)
		nl_dump(p, "qdisc %s ", link->l_qdisc);

	if (link->ce_mask & LINK_ATTR_MAP && link->l_map.lm_irq)
		nl_dump(p, "irq %u ", link->l_map.lm_irq);

	if (link->ce_mask & LINK_ATTR_IFINDEX)
		nl_dump(p, "index %u ", link->l_index);

	if (link->ce_mask & LINK_ATTR_PROMISCUITY && link->l_promiscuity > 0)
		nl_dump(p, "promisc-mode (%u users) ", link->l_promiscuity);

	nl_dump(p, "\n");

	if (link->ce_mask & LINK_ATTR_IFALIAS)
		nl_dump_line(p, "    alias %s\n", link->l_ifalias);

	nl_dump_line(p, "    ");

	if (link->ce_mask & LINK_ATTR_NUM_TX_QUEUES)
		nl_dump(p, "txq %u ", link->l_num_tx_queues);

	if (link->ce_mask & LINK_ATTR_NUM_RX_QUEUES)
		nl_dump(p, "rxq %u ", link->l_num_rx_queues);

	if (link->ce_mask & LINK_ATTR_BRD)
		nl_dump(p, "brd %s ", nl_addr2str(link->l_bcast, buf,
						   sizeof(buf)));

	if ((link->ce_mask & LINK_ATTR_OPERSTATE) &&
	    link->l_operstate != IF_OPER_UNKNOWN) {
		rtnl_link_operstate2str(link->l_operstate, buf, sizeof(buf));
		nl_dump(p, "state %s ", buf);
	}

	if (link->ce_mask & LINK_ATTR_NUM_VF)
		nl_dump(p, "num-vf %u ", link->l_num_vf);

	nl_dump(p, "mode %s ",
		rtnl_link_mode2str(link->l_linkmode, buf, sizeof(buf)));

	nl_dump(p, "carrier %s",
		rtnl_link_carrier2str(link->l_carrier, buf, sizeof(buf)));

	nl_dump(p, "\n");

	if (link->l_info_ops && link->l_info_ops->io_dump[NL_DUMP_DETAILS])
		link->l_info_ops->io_dump[NL_DUMP_DETAILS](link, p);

	do_foreach_af(link, af_dump_details, p);
}

static void link_dump_stats(struct nl_object *obj, struct nl_dump_params *p)
{
	struct rtnl_link *link = (struct rtnl_link *) obj;
	char *unit, fmt[64];
	float res;

	link_dump_details(obj, p);

	nl_dump_line(p, "    Stats:    bytes    packets     errors "
			"   dropped   fifo-err compressed\n");

	res = nl_cancel_down_bytes(link->l_stats[RTNL_LINK_RX_BYTES], &unit);

	strcpy(fmt, "     RX %X.2f %s %10" PRIu64 " %10" PRIu64 " %10" PRIu64 " %10" PRIu64 " %10" PRIu64 "\n");
	fmt[9] = *unit == 'B' ? '9' : '7';

	nl_dump_line(p, fmt, res, unit,
		link->l_stats[RTNL_LINK_RX_PACKETS],
		link->l_stats[RTNL_LINK_RX_ERRORS],
		link->l_stats[RTNL_LINK_RX_DROPPED],
		link->l_stats[RTNL_LINK_RX_FIFO_ERR],
		link->l_stats[RTNL_LINK_RX_COMPRESSED]);

	res = nl_cancel_down_bytes(link->l_stats[RTNL_LINK_TX_BYTES], &unit);

	strcpy(fmt, "     TX %X.2f %s %10" PRIu64 " %10" PRIu64 " %10" PRIu64 " %10" PRIu64 " %10" PRIu64 "\n");
	fmt[9] = *unit == 'B' ? '9' : '7';

	nl_dump_line(p, fmt, res, unit,
		link->l_stats[RTNL_LINK_TX_PACKETS],
		link->l_stats[RTNL_LINK_TX_ERRORS],
		link->l_stats[RTNL_LINK_TX_DROPPED],
		link->l_stats[RTNL_LINK_TX_FIFO_ERR],
		link->l_stats[RTNL_LINK_TX_COMPRESSED]);

	nl_dump_line(p, "    Errors:  length       over        crc "
			"     frame     missed  multicast\n");

	nl_dump_line(p, "     RX  %10" PRIu64 " %10" PRIu64 " %10"
				PRIu64 " %10" PRIu64 " %10" PRIu64 " %10"
				PRIu64 "\n",
		link->l_stats[RTNL_LINK_RX_LEN_ERR],
		link->l_stats[RTNL_LINK_RX_OVER_ERR],
		link->l_stats[RTNL_LINK_RX_CRC_ERR],
		link->l_stats[RTNL_LINK_RX_FRAME_ERR],
		link->l_stats[RTNL_LINK_RX_MISSED_ERR],
		link->l_stats[RTNL_LINK_MULTICAST]);

	nl_dump_line(p, "            aborted    carrier  heartbeat "
			"    window  collision\n");

	nl_dump_line(p, "     TX  %10" PRIu64 " %10" PRIu64 " %10"
			PRIu64 " %10" PRIu64 " %10" PRIu64 "\n",
		link->l_stats[RTNL_LINK_TX_ABORT_ERR],
		link->l_stats[RTNL_LINK_TX_CARRIER_ERR],
		link->l_stats[RTNL_LINK_TX_HBEAT_ERR],
		link->l_stats[RTNL_LINK_TX_WIN_ERR],
		link->l_stats[RTNL_LINK_COLLISIONS]);

	if (link->l_info_ops && link->l_info_ops->io_dump[NL_DUMP_STATS])
		link->l_info_ops->io_dump[NL_DUMP_STATS](link, p);

	do_foreach_af(link, af_dump_stats, p);
}

static void link_keygen(struct nl_object *obj, uint32_t *hashkey,
        uint32_t table_sz)
{
	struct rtnl_link *link = (struct rtnl_link *) obj;
	unsigned int lkey_sz;
	struct link_hash_key {
		uint32_t	l_index;
		uint32_t	l_family;
	} __attribute__((packed)) lkey;

	lkey_sz = sizeof(lkey);
	lkey.l_index = link->l_index;
	lkey.l_family = link->l_family;

	*hashkey = nl_hash(&lkey, lkey_sz, 0) % table_sz;

	NL_DBG(5, "link %p key (dev %d fam %d) keysz %d, hash 0x%x\n",
	       link, lkey.l_index, lkey.l_family, lkey_sz, *hashkey);

	return;
}

static int link_compare(struct nl_object *_a, struct nl_object *_b,
			uint32_t attrs, int flags)
{
	struct rtnl_link *a = (struct rtnl_link *) _a;
	struct rtnl_link *b = (struct rtnl_link *) _b;
	int diff = 0;

#define LINK_DIFF(ATTR, EXPR) ATTR_DIFF(attrs, LINK_ATTR_##ATTR, a, b, EXPR)

	diff |= LINK_DIFF(IFINDEX,	a->l_index != b->l_index);
	diff |= LINK_DIFF(MTU,		a->l_mtu != b->l_mtu);
	diff |= LINK_DIFF(LINK,		a->l_link != b->l_link);
	diff |= LINK_DIFF(TXQLEN,	a->l_txqlen != b->l_txqlen);
	diff |= LINK_DIFF(WEIGHT,	a->l_weight != b->l_weight);
	diff |= LINK_DIFF(MASTER,	a->l_master != b->l_master);
	diff |= LINK_DIFF(FAMILY,	a->l_family != b->l_family);
	diff |= LINK_DIFF(OPERSTATE,	a->l_operstate != b->l_operstate);
	diff |= LINK_DIFF(LINKMODE,	a->l_linkmode != b->l_linkmode);
	diff |= LINK_DIFF(QDISC,	strcmp(a->l_qdisc, b->l_qdisc));
	diff |= LINK_DIFF(IFNAME,	strcmp(a->l_name, b->l_name));
	diff |= LINK_DIFF(ADDR,		nl_addr_cmp(a->l_addr, b->l_addr));
	diff |= LINK_DIFF(BRD,		nl_addr_cmp(a->l_bcast, b->l_bcast));
	diff |= LINK_DIFF(IFALIAS,	strcmp(a->l_ifalias, b->l_ifalias));
	diff |= LINK_DIFF(NUM_VF,	a->l_num_vf != b->l_num_vf);
	diff |= LINK_DIFF(PROMISCUITY,	a->l_promiscuity != b->l_promiscuity);
	diff |= LINK_DIFF(NUM_TX_QUEUES,a->l_num_tx_queues != b->l_num_tx_queues);
	diff |= LINK_DIFF(NUM_RX_QUEUES,a->l_num_rx_queues != b->l_num_rx_queues);
	diff |= LINK_DIFF(GROUP,	a->l_group != b->l_group);

	if (flags & LOOSE_COMPARISON)
		diff |= LINK_DIFF(FLAGS,
				  (a->l_flags ^ b->l_flags) & b->l_flag_mask);
	else
		diff |= LINK_DIFF(FLAGS, a->l_flags != b->l_flags);

	/*
	 * Compare LINK_ATTR_PROTINFO af_data
	 */
	if (a->l_family == b->l_family) {
		if (rtnl_link_af_data_compare(a, b, a->l_family) != 0)
			goto protinfo_mismatch;
	}

out:
	return diff;

protinfo_mismatch:
	diff |= LINK_DIFF(PROTINFO, 1);
	goto out;

#undef LINK_DIFF
}

static const struct trans_tbl link_attrs[] = {
	__ADD(LINK_ATTR_MTU, mtu)
	__ADD(LINK_ATTR_LINK, link)
	__ADD(LINK_ATTR_TXQLEN, txqlen)
	__ADD(LINK_ATTR_WEIGHT, weight)
	__ADD(LINK_ATTR_MASTER, master)
	__ADD(LINK_ATTR_QDISC, qdisc)
	__ADD(LINK_ATTR_MAP, map)
	__ADD(LINK_ATTR_ADDR, address)
	__ADD(LINK_ATTR_BRD, broadcast)
	__ADD(LINK_ATTR_FLAGS, flags)
	__ADD(LINK_ATTR_IFNAME, name)
	__ADD(LINK_ATTR_IFINDEX, ifindex)
	__ADD(LINK_ATTR_FAMILY, family)
	__ADD(LINK_ATTR_ARPTYPE, arptype)
	__ADD(LINK_ATTR_STATS, stats)
	__ADD(LINK_ATTR_CHANGE, change)
	__ADD(LINK_ATTR_OPERSTATE, operstate)
	__ADD(LINK_ATTR_LINKMODE, linkmode)
	__ADD(LINK_ATTR_IFALIAS, ifalias)
	__ADD(LINK_ATTR_NUM_VF, num_vf)
	__ADD(LINK_ATTR_PROMISCUITY, promiscuity)
	__ADD(LINK_ATTR_NUM_TX_QUEUES, num_tx_queues)
	__ADD(LINK_ATTR_NUM_RX_QUEUES, num_rx_queues)
	__ADD(LINK_ATTR_GROUP, group)
	__ADD(LINK_ATTR_CARRIER, carrier)
	__ADD(LINK_ATTR_PHYS_PORT_ID, phys_port_id)
};

static char *link_attrs2str(int attrs, char *buf, size_t len)
{
	return __flags2str(attrs, buf, len, link_attrs,
			   ARRAY_SIZE(link_attrs));
}

/**
 * @name Get / List
 * @{
 */

/**
 * Allocate link cache and fill in all configured links.
 * @arg sk		Netlink socket.
 * @arg family		Link address family or AF_UNSPEC
 * @arg result		Pointer to store resulting cache.
 *
 * Allocates and initializes a new link cache. If \c sk is valid, a netlink
 * message is sent to the kernel requesting a full dump of all configured
 * links. The returned messages are parsed and filled into the cache. If
 * the operation succeeds, the resulting cache will contain a link object for
 * each link configured in the kernel. If \c sk is NULL, returns 0 but the
 * cache is still empty.
 *
 * If \c family is set to an address family other than \c AF_UNSPEC the
 * contents of the cache can be limited to a specific address family.
 * Currently the following address families are supported:
 * - AF_BRIDGE
 * - AF_INET6
 *
 * @route_doc{link_list, Get List of Links}
 * @see rtnl_link_get()
 * @see rtnl_link_get_by_name()
 * @return 0 on success or a negative error code.
 */
int rtnl_link_alloc_cache(struct nl_sock *sk, int family, struct nl_cache **result)
{
	struct nl_cache * cache;
	int err;

	cache = nl_cache_alloc(&rtnl_link_ops);
	if (!cache)
		return -NLE_NOMEM;

	cache->c_iarg1 = family;

	if (sk && (err = nl_cache_refill(sk, cache)) < 0) {
		nl_cache_free(cache);
		return err;
	}

	*result = cache;
	return 0;
}

/**
 * Lookup link in cache by interface index
 * @arg cache		Link cache
 * @arg ifindex		Interface index
 *
 * Searches through the provided cache looking for a link with matching
 * interface index.
 *
 * @attention The reference counter of the returned link object will be
 *            incremented. Use rtnl_link_put() to release the reference.
 *
 * @route_doc{link_list, Get List of Links}
 * @see rtnl_link_get_by_name()
 * @return Link object or NULL if no match was found.
 */
struct rtnl_link *rtnl_link_get(struct nl_cache *cache, int ifindex)
{
	struct rtnl_link *link;

	if (cache->c_ops != &rtnl_link_ops)
		return NULL;

	nl_list_for_each_entry(link, &cache->c_items, ce_list) {
		if (link->l_index == ifindex) {
			nl_object_get((struct nl_object *) link);
			return link;
		}
	}

	return NULL;
}

/**
 * Lookup link in cache by link name
 * @arg cache		Link cache
 * @arg name		Name of link
 *
 * Searches through the provided cache looking for a link with matching
 * link name
 *
 * @attention The reference counter of the returned link object will be
 *            incremented. Use rtnl_link_put() to release the reference.
 *
 * @route_doc{link_list, Get List of Links}
 * @see rtnl_link_get()
 * @return Link object or NULL if no match was found.
 */
struct rtnl_link *rtnl_link_get_by_name(struct nl_cache *cache,
					 const char *name)
{
	struct rtnl_link *link;

	if (cache->c_ops != &rtnl_link_ops)
		return NULL;

	nl_list_for_each_entry(link, &cache->c_items, ce_list) {
		if (!strcmp(name, link->l_name)) {
			nl_object_get((struct nl_object *) link);
			return link;
		}
	}

	return NULL;
}

/**
 * Construct RTM_GETLINK netlink message
 * @arg ifindex		Interface index
 * @arg name		Name of link
 * @arg result		Pointer to store resulting netlink message
 *
 * The behaviour of this function is identical to rtnl_link_get_kernel()
 * with the exception that it will not send the message but return it in
 * the provided return pointer instead.
 *
 * @see rtnl_link_get_kernel()
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_link_build_get_request(int ifindex, const char *name,
				struct nl_msg **result)
{
	struct ifinfomsg ifi;
	struct nl_msg *msg;

	if (ifindex <= 0 && !name) {
		APPBUG("ifindex or name must be specified");
		return -NLE_MISSING_ATTR;
	}

	memset(&ifi, 0, sizeof(ifi));

	if (!(msg = nlmsg_alloc_simple(RTM_GETLINK, 0)))
		return -NLE_NOMEM;

	if (ifindex > 0)
		ifi.ifi_index = ifindex;

	if (nlmsg_append(msg, &ifi, sizeof(ifi), NLMSG_ALIGNTO) < 0)
		goto nla_put_failure;

	if (name)
		NLA_PUT_STRING(msg, IFLA_IFNAME, name);

	*result = msg;
	return 0;

nla_put_failure:
	nlmsg_free(msg);
	return -NLE_MSGSIZE;
}

/**
 * Get a link object directly from kernel
 * @arg sk		Netlink socket
 * @arg ifindex		Interface index
 * @arg name		Name of link
 * @arg result		Pointer to store resulting link object
 *
 * This function builds a \c RTM_GETLINK netlink message to request
 * a specific link directly from the kernel. The returned answer is
 * parsed into a struct rtnl_link object and returned via the result
 * pointer or -NLE_OBJ_NOTFOUND is returned if no matching link was
 * found.
 *
 * @route_doc{link_direct_lookup, Lookup Single Link (Direct Lookup)}
 * @return 0 on success or a negative error code.
 */
int rtnl_link_get_kernel(struct nl_sock *sk, int ifindex, const char *name,
			 struct rtnl_link **result)
{
	struct nl_msg *msg = NULL;
	struct nl_object *obj;
	int err;

	if ((err = rtnl_link_build_get_request(ifindex, name, &msg)) < 0)
		return err;

	err = nl_send_auto(sk, msg);
	nlmsg_free(msg);
	if (err < 0)
		return err;

	if ((err = nl_pickup(sk, link_msg_parser, &obj)) < 0)
		return err;

	/* We have used link_msg_parser(), object is definitely a link */
	*result = (struct rtnl_link *) obj;

	/* If an object has been returned, we also need to wait for the ACK */
	 if (err == 0 && obj)
		 wait_for_ack(sk);

	return 0;
}

/**
 * Translate interface index to corresponding link name
 * @arg cache		Link cache
 * @arg ifindex		Interface index
 * @arg dst		String to store name
 * @arg len		Length of destination string
 *
 * Translates the specified interface index to the corresponding
 * link name and stores the name in the destination string.
 *
 * @route_doc{link_translate_ifindex, Translating interface index to link name}
 * @see rtnl_link_name2i()
 * @return Name of link or NULL if no match was found.
 */
char * rtnl_link_i2name(struct nl_cache *cache, int ifindex, char *dst,
			size_t len)
{
	struct rtnl_link *link = rtnl_link_get(cache, ifindex);

	if (link) {
		strncpy(dst, link->l_name, len - 1);
		rtnl_link_put(link);
		return dst;
	}

	return NULL;
}

/**
 * Translate link name to corresponding interface index
 * @arg cache		Link cache
 * @arg name		Name of link
 *
 * @route_doc{link_translate_ifindex, Translating interface index to link name}
 * @see rtnl_link_i2name()
 * @return Interface index or 0 if no match was found.
 */
int rtnl_link_name2i(struct nl_cache *cache, const char *name)
{
	int ifindex = 0;
	struct rtnl_link *link;

	link = rtnl_link_get_by_name(cache, name);
	if (link) {
		ifindex = link->l_index;
		rtnl_link_put(link);
	}

	return ifindex;
}

/** @} */

int rtnl_link_fill_info(struct nl_msg *msg, struct rtnl_link *link)
{
	if (link->ce_mask & LINK_ATTR_ADDR)
		NLA_PUT_ADDR(msg, IFLA_ADDRESS, link->l_addr);

	if (link->ce_mask & LINK_ATTR_BRD)
		NLA_PUT_ADDR(msg, IFLA_BROADCAST, link->l_bcast);

	if (link->ce_mask & LINK_ATTR_MTU)
		NLA_PUT_U32(msg, IFLA_MTU, link->l_mtu);

	if (link->ce_mask & LINK_ATTR_TXQLEN)
		NLA_PUT_U32(msg, IFLA_TXQLEN, link->l_txqlen);

	if (link->ce_mask & LINK_ATTR_WEIGHT)
		NLA_PUT_U32(msg, IFLA_WEIGHT, link->l_weight);

	if (link->ce_mask & LINK_ATTR_IFNAME)
		NLA_PUT_STRING(msg, IFLA_IFNAME, link->l_name);

	if (link->ce_mask & LINK_ATTR_OPERSTATE)
		NLA_PUT_U8(msg, IFLA_OPERSTATE, link->l_operstate);

	if (link->ce_mask & LINK_ATTR_CARRIER)
		NLA_PUT_U8(msg, IFLA_CARRIER, link->l_carrier);

	if (link->ce_mask & LINK_ATTR_LINKMODE)
		NLA_PUT_U8(msg, IFLA_LINKMODE, link->l_linkmode);

	if (link->ce_mask & LINK_ATTR_IFALIAS)
		NLA_PUT_STRING(msg, IFLA_IFALIAS, link->l_ifalias);

	if (link->ce_mask & LINK_ATTR_LINK)
		NLA_PUT_U32(msg, IFLA_LINK, link->l_link);

	if (link->ce_mask & LINK_ATTR_MASTER)
		NLA_PUT_U32(msg, IFLA_MASTER, link->l_master);

	if (link->ce_mask & LINK_ATTR_NUM_TX_QUEUES)
		NLA_PUT_U32(msg, IFLA_NUM_TX_QUEUES, link->l_num_tx_queues);

	if (link->ce_mask & LINK_ATTR_NUM_RX_QUEUES)
		NLA_PUT_U32(msg, IFLA_NUM_RX_QUEUES, link->l_num_rx_queues);

	if (link->ce_mask & LINK_ATTR_NS_FD)
		NLA_PUT_U32(msg, IFLA_NET_NS_FD, link->l_ns_fd);

	if (link->ce_mask & LINK_ATTR_NS_PID)
		NLA_PUT_U32(msg, IFLA_NET_NS_PID, link->l_ns_pid);

	return 0;

nla_put_failure:
	return -NLE_MSGSIZE;
}

static int build_link_msg(int cmd, struct ifinfomsg *hdr,
			  struct rtnl_link *link, int flags, struct nl_msg **result)
{
	struct nl_msg *msg;
	struct nlattr *af_spec;

	msg = nlmsg_alloc_simple(cmd, flags);
	if (!msg)
		return -NLE_NOMEM;

	if (nlmsg_append(msg, hdr, sizeof(*hdr), NLMSG_ALIGNTO) < 0)
		goto nla_put_failure;

	if (rtnl_link_fill_info(msg, link))
		goto nla_put_failure;

	if (link->ce_mask & LINK_ATTR_GROUP)
		NLA_PUT_U32(msg, IFLA_GROUP, link->l_group);

	if (link->ce_mask & LINK_ATTR_LINKINFO) {
		struct nlattr *info;

		if (!(info = nla_nest_start(msg, IFLA_LINKINFO)))
			goto nla_put_failure;

		NLA_PUT_STRING(msg, IFLA_INFO_KIND, link->l_info_kind);

		if (link->l_info_ops) {
			if (link->l_info_ops->io_put_attrs &&
			    link->l_info_ops->io_put_attrs(msg, link) < 0)
				goto nla_put_failure;
		}

		nla_nest_end(msg, info);
	}

	if (!(af_spec = nla_nest_start(msg, IFLA_AF_SPEC)))
		goto nla_put_failure;

	if (do_foreach_af(link, af_fill, msg) < 0)
		goto nla_put_failure;

	nla_nest_end(msg, af_spec);

	*result = msg;
	return 0;

nla_put_failure:
	nlmsg_free(msg);
	return -NLE_MSGSIZE;
}

/**
 * @name Add / Modify
 * @{
 */

/**
 * Build a netlink message requesting the addition of new virtual link
 * @arg link		new link to add
 * @arg flags		additional netlink message flags
 * @arg result		pointer to store resulting netlink message
 *
 * The behaviour of this function is identical to rtnl_link_add() with
 * the exception that it will not send the message but return it in the
 * provided return pointer instead.
 *
 * @see rtnl_link_add()
 *
 * @note This operation is not supported on all kernel versions.
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_link_build_add_request(struct rtnl_link *link, int flags,
				struct nl_msg **result)
{
	struct ifinfomsg ifi = {
		.ifi_family = link->l_family,
		.ifi_index = link->l_index,
		.ifi_flags = link->l_flags,
	};

	return build_link_msg(RTM_NEWLINK, &ifi, link, flags, result);
}

/**
 * Add virtual link
 * @arg sk		netlink socket.
 * @arg link		new link to add
 * @arg flags		additional netlink message flags
 *
 * Builds a \c RTM_NEWLINK netlink message requesting the addition of
 * a new virtual link.
 *
 * After sending, the function will wait for the ACK or an eventual
 * error message to be received and will therefore block until the
 * operation has been completed.
 *
 * @copydoc auto_ack_warning
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_link_add(struct nl_sock *sk, struct rtnl_link *link, int flags)
{
	struct nl_msg *msg;
	int err;

	err = rtnl_link_build_add_request(link, flags, &msg);
	if (err < 0)
		return err;

	return nl_send_sync(sk, msg);
}

/**
 * Build a netlink message requesting the modification of link
 * @arg orig		original link to change
 * @arg changes		link containing the changes to be made
 * @arg flags		additional netlink message flags
 * @arg result		pointer to store resulting netlink message
 *
 * The behaviour of this function is identical to rtnl_link_change() with
 * the exception that it will not send the message but return it in the
 * provided return pointer instead.
 *
 * @see rtnl_link_change()
 *
 * @note The resulting message will have message type set to RTM_NEWLINK
 *       which may not work with older kernels. You may have to modify it
 *       to RTM_SETLINK (does not allow changing link info attributes) to
 *       have the change request work with older kernels.
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_link_build_change_request(struct rtnl_link *orig,
				   struct rtnl_link *changes, int flags,
				   struct nl_msg **result)
{
	struct ifinfomsg ifi = {
		.ifi_family = orig->l_family,
		.ifi_index = orig->l_index,
	};
	int err;

	if (changes->ce_mask & LINK_ATTR_FLAGS) {
		ifi.ifi_flags = orig->l_flags & ~changes->l_flag_mask;
		ifi.ifi_flags |= changes->l_flags;
	}

	if (changes->l_family && changes->l_family != orig->l_family) {
		APPBUG("link change: family is immutable");
		return -NLE_IMMUTABLE;
	}

	/* Avoid unnecessary name change requests */
	if (orig->ce_mask & LINK_ATTR_IFINDEX &&
	    orig->ce_mask & LINK_ATTR_IFNAME &&
	    changes->ce_mask & LINK_ATTR_IFNAME &&
	    !strcmp(orig->l_name, changes->l_name))
		changes->ce_mask &= ~LINK_ATTR_IFNAME;

	if ((err = build_link_msg(RTM_NEWLINK, &ifi, changes, flags, result)) < 0)
		goto errout;

	return 0;

errout:
	return err;
}

/**
 * Change link
 * @arg sk		netlink socket.
 * @arg orig		original link to be changed
 * @arg changes		link containing the changes to be made
 * @arg flags		additional netlink message flags
 *
 * Builds a \c RTM_NEWLINK netlink message requesting the change of
 * a network link. If -EOPNOTSUPP is returned by the kernel, the
 * message type will be changed to \c RTM_SETLINK and the message is
 * resent to work around older kernel versions.
 *
 * The link to be changed is looked up based on the interface index
 * supplied in the \p orig link. Optionaly the link name is used but
 * only if no interface index is provided, otherwise providing an
 * link name will result in the link name being changed.
 *
 * If no matching link exists, the function will return
 * -NLE_OBJ_NOTFOUND.
 *
 * After sending, the function will wait for the ACK or an eventual
 * error message to be received and will therefore block until the
 * operation has been completed.
 *
 * @copydoc auto_ack_warning
 *
 * @note The link name can only be changed if the link has been put
 *       in opertional down state. (~IF_UP)
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_link_change(struct nl_sock *sk, struct rtnl_link *orig,
		     struct rtnl_link *changes, int flags)
{
	struct nl_msg *msg;
	int err;

	err = rtnl_link_build_change_request(orig, changes, flags, &msg);
	if (err < 0)
		return err;

retry:
	err = nl_send_auto_complete(sk, msg);
	if (err < 0)
		goto errout;

	err = wait_for_ack(sk);
	if (err == -NLE_OPNOTSUPP && msg->nm_nlh->nlmsg_type == RTM_NEWLINK) {
		msg->nm_nlh->nlmsg_type = RTM_SETLINK;
		goto retry;
	}

errout:
	nlmsg_free(msg);
	return err;
}

/** @} */

/**
 * @name Delete
 * @{
 */

/**
 * Build a netlink message requesting the deletion of a link
 * @arg link		Link to delete
 * @arg result		Pointer to store resulting netlink message
 *
 * The behaviour of this function is identical to rtnl_link_delete() with
 * the exception that it will not send the message but return it in the
 * provided return pointer instead.
 *
 * @see rtnl_link_delete()
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_link_build_delete_request(const struct rtnl_link *link,
				   struct nl_msg **result)
{
	struct nl_msg *msg;
	struct ifinfomsg ifi = {
		.ifi_index = link->l_index,
	};

	if (!(link->ce_mask & (LINK_ATTR_IFINDEX | LINK_ATTR_IFNAME))) {
		APPBUG("ifindex or name must be specified");
		return -NLE_MISSING_ATTR;
	}

	if (!(msg = nlmsg_alloc_simple(RTM_DELLINK, 0)))
		return -NLE_NOMEM;

	if (nlmsg_append(msg, &ifi, sizeof(ifi), NLMSG_ALIGNTO) < 0)
		goto nla_put_failure;

	if (link->ce_mask & LINK_ATTR_IFNAME)
		NLA_PUT_STRING(msg, IFLA_IFNAME, link->l_name);

	*result = msg;
	return 0;

nla_put_failure:
	nlmsg_free(msg);
	return -NLE_MSGSIZE;
}

/**
 * Delete link
 * @arg sk		Netlink socket
 * @arg link		Link to delete
 *
 * Builds a \c RTM_DELLINK netlink message requesting the deletion of
 * a network link which has been previously added to the kernel and
 * sends the message to the kernel.
 *
 * If no matching link exists, the function will return
 * -NLE_OBJ_NOTFOUND.
 *
 * After sending, the function will wait for the ACK or an eventual
 * error message to be received and will therefore block until the
 * operation has been completed.
 *
 * @copydoc auto_ack_warning
 *
 * @note Only virtual links such as dummy interface or vlan interfaces
 *       can be deleted. It is not possible to delete physical interfaces
 *       such as ethernet interfaces or the loopback device.
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_link_delete(struct nl_sock *sk, const struct rtnl_link *link)
{
	struct nl_msg *msg;
	int err;

	if ((err = rtnl_link_build_delete_request(link, &msg)) < 0)
		return err;

	return nl_send_sync(sk, msg);
}

/** @} */

/**
 * @name Link Object
 * @{
 */

/**
 * Allocate link object
 *
 * @see rtnl_link_put()
 * @return New link object or NULL if allocation failed
 */
struct rtnl_link *rtnl_link_alloc(void)
{
	return (struct rtnl_link *) nl_object_alloc(&link_obj_ops);
}

/**
 * Return a link object reference
 * @arg link		Link object
 */
void rtnl_link_put(struct rtnl_link *link)
{
	nl_object_put((struct nl_object *) link);
}

/**
 * Set name of link object
 * @arg link		Link object
 * @arg name		New name
 *
 * @note To change the name of a link in the kernel, set the interface
 *       index to the link you wish to change, modify the link name using
 *       this function and pass the link object to rtnl_link_change() or
 *       rtnl_link_add().
 *
 * @route_doc{link_attr_name, Link Name}
 * @see rtnl_link_get_name()
 * @see rtnl_link_set_ifindex()
 */
void rtnl_link_set_name(struct rtnl_link *link, const char *name)
{
	strncpy(link->l_name, name, sizeof(link->l_name) - 1);
	link->ce_mask |= LINK_ATTR_IFNAME;
}

/**
 * Return name of link object
 * @arg link		Link object
 *
 * @route_doc{link_attr_name, Link Name}
 * @see rtnl_link_set_name()
 * @return Link name or NULL if name is not specified
 */
char *rtnl_link_get_name(struct rtnl_link *link)
{
	return link->ce_mask & LINK_ATTR_IFNAME ? link->l_name : NULL;
}

/**
 * Set the group identifier of a link object
 * @arg link		Link object
 * @arg group		Group identifier
 */
void rtnl_link_set_group(struct rtnl_link *link, uint32_t group)
{
	link->l_group = group;
	link->ce_mask |= LINK_ATTR_GROUP;
}

/**
 * Return the group identifier of link object
 * @arg link		Link object
 *
 * @return Group identifier or 0 if not set.
 */
uint32_t rtnl_link_get_group(struct rtnl_link *link)
{
	return link->l_group;
}

static inline void __assign_addr(struct rtnl_link *link, struct nl_addr **pos,
				 struct nl_addr *new, int flag)
{
	if (*pos)
		nl_addr_put(*pos);

	nl_addr_get(new);
	*pos = new;

	link->ce_mask |= flag;
}

/**
 * Set link layer address of link object
 * @arg link		Link object
 * @arg addr		New link layer address
 *
 * The function increments the reference counter of the address object
 * and overwrites any existing link layer address previously assigned.
 *
 * @route_doc{link_attr_address, Link layer address}
 * @see rtnl_link_get_addr()
 */
void rtnl_link_set_addr(struct rtnl_link *link, struct nl_addr *addr)
{
	__assign_addr(link, &link->l_addr, addr, LINK_ATTR_ADDR);
}

/**
 * Return link layer address of link object
 * @arg link		Link object
 *
 * @copydoc pointer_lifetime_warning
 * @route_doc{link_attr_address, Link Layer Address}
 * @see rtnl_link_set_addr()
 * @return Link layer address or NULL if not set.
 */
struct nl_addr *rtnl_link_get_addr(struct rtnl_link *link)
{
	return link->ce_mask & LINK_ATTR_ADDR ? link->l_addr : NULL;
}

/**
 * Set link layer broadcast address of link object
 * @arg link		Link object
 * @arg addr		New broadcast address
 *
 * The function increments the reference counter of the address object
 * and overwrites any existing link layer broadcast address previously
 * assigned.
 *
 * @route_doc{link_attr_broadcast, Link Layer Broadcast Address}
 * @see rtnl_link_get_broadcast()
 */
void rtnl_link_set_broadcast(struct rtnl_link *link, struct nl_addr *addr)
{
	__assign_addr(link, &link->l_bcast, addr, LINK_ATTR_BRD);
}

/**
 * Return link layer broadcast address of link object
 * @arg link		Link object
 *
 * @copydoc pointer_lifetime_warning
 * @route_doc{link_attr_address, Link Layer Address}
 * @see rtnl_link_set_broadcast()
 * @return Link layer address or NULL if not set.
 */
struct nl_addr *rtnl_link_get_broadcast(struct rtnl_link *link)
{
	return link->ce_mask & LINK_ATTR_BRD ? link->l_bcast : NULL;
}

/**
 * Set flags of link object
 * @arg link		Link object
 * @arg flags		Flags
 *
 * @see rtnl_link_get_flags()
 * @see rtnl_link_unset_flags()
 */
void rtnl_link_set_flags(struct rtnl_link *link, unsigned int flags)
{
	link->l_flag_mask |= flags;
	link->l_flags |= flags;
	link->ce_mask |= LINK_ATTR_FLAGS;
}

/**
 * Unset flags of link object
 * @arg link		Link object
 * @arg flags		Flags
 *
 * @see rtnl_link_set_flags()
 * @see rtnl_link_get_flags()
 */
void rtnl_link_unset_flags(struct rtnl_link *link, unsigned int flags)
{
	link->l_flag_mask |= flags;
	link->l_flags &= ~flags;
	link->ce_mask |= LINK_ATTR_FLAGS;
}

/**
 * Return flags of link object
 * @arg link		Link object
 *
 * @route_doc{link_attr_flags, Link Flags}
 * @see rtnl_link_set_flags()
 * @see rtnl_link_unset_flags()
 * @return Link flags or 0 if none have been set.
 */
unsigned int rtnl_link_get_flags(struct rtnl_link *link)
{
	return link->l_flags;
}

/**
 * Set address family of link object
 *
 * @see rtnl_link_get_family()
 */
void rtnl_link_set_family(struct rtnl_link *link, int family)
{
	link->l_family = family;
	link->ce_mask |= LINK_ATTR_FAMILY;

	if (link->l_af_ops) {
		af_free(link, link->l_af_ops,
			link->l_af_data[link->l_af_ops->ao_family], NULL);
		link->l_af_data[link->l_af_ops->ao_family] = NULL;
	}

	link->l_af_ops = af_lookup_and_alloc(link, family);
}

/**
 * Return address family of link object
 * @arg link		Link object
 *
 * @see rtnl_link_set_family()
 * @return Address family or \c AF_UNSPEC if not specified.
 */
int rtnl_link_get_family(struct rtnl_link *link)
{
	return link->ce_mask & LINK_ATTR_FAMILY ? link->l_family : AF_UNSPEC;
}

/**
 * Set hardware type of link object
 * @arg link		Link object
 * @arg arptype		New hardware type \c (ARPHRD_*)
 *
 * @route_doc{link_attr_arptype, Hardware Type}
 * @copydoc read_only_attribute
 * @see rtnl_link_get_arptype()
 */
void rtnl_link_set_arptype(struct rtnl_link *link, unsigned int arptype)
{
	link->l_arptype = arptype;
	link->ce_mask |= LINK_ATTR_ARPTYPE;
}

/**
 * Get hardware type of link object
 * @arg link		Link object
 *
 * @route_doc{link_attr_arptype, Hardware Type}
 * @see rtnl_link_set_arptype()
 * @return Hardware type \c (ARPHRD_ETHER *) or \c ARPHRD_VOID
 */
unsigned int rtnl_link_get_arptype(struct rtnl_link *link)
{
	if (link->ce_mask & LINK_ATTR_ARPTYPE)
		return link->l_arptype;
	else
		return ARPHRD_VOID;
}

/**
 * Set interface index of link object
 * @arg link		Link object
 * @arg ifindex		Interface index
 *
 * @route_doc{link_attr_ifindex, Interface Index}
 * @see rtnl_link_get_ifindex()
 */
void rtnl_link_set_ifindex(struct rtnl_link *link, int ifindex)
{
	link->l_index = ifindex;
	link->ce_mask |= LINK_ATTR_IFINDEX;
}

/**
 * Return interface index of link object
 * @arg link		Link object
 *
 * @route_doc{link_attr_ifindex, Interface Index}
 * @see rtnl_link_set_ifindex()
 * @return Interface index or 0 if not set.
 */
int rtnl_link_get_ifindex(struct rtnl_link *link)
{
	return link->l_index;
}

/**
 * Set Maximum Transmission Unit of link object
 * @arg link		Link object
 * @arg mtu		New MTU value in number of bytes
 *
 * @route_doc{link_attr_mtu, Maximum Transmission Unit}
 * @see rtnl_link_get_mtu()
 */
void rtnl_link_set_mtu(struct rtnl_link *link, unsigned int mtu)
{
	link->l_mtu = mtu;
	link->ce_mask |= LINK_ATTR_MTU;
}

/**
 * Return maximum transmission unit of link object
 * @arg link		Link object
 *
 * @route_doc{link_attr_mtu, Maximum Transmission Unit}
 * @see rtnl_link_set_mtu()
 * @return MTU in bytes or 0 if not set
 */
unsigned int rtnl_link_get_mtu(struct rtnl_link *link)
{
	return link->l_mtu;
}

/**
 * Set transmission queue length
 * @arg link		Link object
 * @arg txqlen		New queue length
 *
 * The unit is dependant on the link type. The most common units is number
 * of packets.
 *
 * @route_doc{link_attr_txqlen, Transmission Queue Length}
 */
void rtnl_link_set_txqlen(struct rtnl_link *link, unsigned int txqlen)
{
	link->l_txqlen = txqlen;
	link->ce_mask |= LINK_ATTR_TXQLEN;
}

/**
 * Return transmission queue length
 * @arg link		Link object
 *
 * The unit is dependant on the link type. The most common units is number
 * of packets.
 *
 * @route_doc{link_attr_txqlen, Transmission Queue Length}
 * @return queue length or 0 if not specified.
 */
unsigned int rtnl_link_get_txqlen(struct rtnl_link *link)
{
	return link->ce_mask & LINK_ATTR_TXQLEN ? link->l_txqlen : 0;
}

void rtnl_link_set_link(struct rtnl_link *link, int ifindex)
{
	link->l_link = ifindex;
	link->ce_mask |= LINK_ATTR_LINK;
}

int rtnl_link_get_link(struct rtnl_link *link)
{
	return link->l_link;
}

/**
 * Set master link of link object
 * @arg link		Link object
 * @arg ifindex		Interface index of master link
 *
 * @see rtnl_link_get_master()
 */
void rtnl_link_set_master(struct rtnl_link *link, int ifindex)
{
	link->l_master = ifindex;
	link->ce_mask |= LINK_ATTR_MASTER;
}

/**
 * Return master link of link object
 * @arg link		Link object
 *
 * @see rtnl_link_set_master()
 * @return Interface index of master link or 0 if not specified
 */
int rtnl_link_get_master(struct rtnl_link *link)
{
	return link->l_master;
}

/**
 * Set carrier of link object
 * @arg link		Link object
 * @arg status		New carrier status
 *
 * @see rtnl_link_get_carrier()
 */
void rtnl_link_set_carrier(struct rtnl_link *link, uint8_t status)
{
	link->l_carrier = status;
	link->ce_mask |= LINK_ATTR_CARRIER;
}

/**
 * Return carrier status of link object
 * @arg link		Link object
 *
 * @see rtnl_link_set_master()
 * @return Carrier state.
 */
uint8_t rtnl_link_get_carrier(struct rtnl_link *link)
{
	return link->l_carrier;
}

/**
 * Set operational status of link object
 * @arg link		Link object
 * @arg status		New opertional status
 *
 * @route_doc{link_attr_operstate, Operational Status}}
 * @see rtnl_link_get_operstate()
 */
void rtnl_link_set_operstate(struct rtnl_link *link, uint8_t status)
{
	link->l_operstate = status;
	link->ce_mask |= LINK_ATTR_OPERSTATE;
}

/**
 * Return operational status of link object
 * @arg link		Link object
 *
 * @route_doc{link_attr_operstate, Operational Status}
 * @see rtnl_link_set_operstate()
 * @return Opertional state or \c IF_OPER_UNKNOWN
 */
uint8_t rtnl_link_get_operstate(struct rtnl_link *link)
{
	return link->l_operstate;
}

/**
 * Set link mode of link object
 * @arg link		Link object
 * @arg mode		New link mode
 *
 * @route_doc{link_attr_mode, Mode}
 * @see rtnl_link_get_linkmode()
 */
void rtnl_link_set_linkmode(struct rtnl_link *link, uint8_t mode)
{
	link->l_linkmode = mode;
	link->ce_mask |= LINK_ATTR_LINKMODE;
}

/**
 * Return link mode of link object
 * @arg link		Link object
 *
 * @route_doc{link_attr_mode, Mode}
 * @see rtnl_link_get_linkmode()
 * @return Link mode or \c IF_LINK_MODE_DEFAULT
 */
uint8_t rtnl_link_get_linkmode(struct rtnl_link *link)
{
	return link->l_linkmode;
}

/**
 * Return alias name of link object (SNMP IfAlias)
 * @arg link		Link object
 *
 * @route_doc{link_attr_alias, Alias}
 * @see rtnl_link_set_ifalias()
 * @return Alias name or NULL if not set.
 */
const char *rtnl_link_get_ifalias(struct rtnl_link *link)
{
	return link->l_ifalias;
}

/**
 * Set alias name of link object (SNMP IfAlias)
 * @arg link		Link object
 * @arg alias		Alias name or NULL to unset
 *
 * Sets the alias name of the link to the specified name. The alias
 * name can be unset by specyfing NULL as the alias. The name will
 * be strdup()ed, so no need to provide a persistent character string.
 *
 * @route_doc{link_attr_alias, Alias}
 * @see rtnl_link_get_ifalias()
 */
void rtnl_link_set_ifalias(struct rtnl_link *link, const char *alias)
{
	free(link->l_ifalias);

	if (alias) {
		link->l_ifalias = strdup(alias);
		link->ce_mask |= LINK_ATTR_IFALIAS;
	} else {
		link->l_ifalias = NULL;
		link->ce_mask &= ~LINK_ATTR_IFALIAS;
	}
}

/**
 * Set queueing discipline name of link object
 * @arg link		Link object
 * @arg name		Name of queueing discipline
 *
 * @copydoc read_only_attribute
 *
 * For more information on how to modify the qdisc of a link, see section
 * @ref_route{route_tc, Traffic Control}.
 *
 * @route_doc{link_attr_qdisc, Queueing Discipline Name}
 * @see rtnl_link_get_qdisc()
 */
void rtnl_link_set_qdisc(struct rtnl_link *link, const char *name)
{
	strncpy(link->l_qdisc, name, sizeof(link->l_qdisc) - 1);
	link->ce_mask |= LINK_ATTR_QDISC;
}

/**
 * Return name of queueing discipline of link object
 * @arg link		Link object
 *
 * @route_doc{link_attr_qdisc, Queueing Discipline Name}
 * @see rtnl_link_set_qdisc()
 * @return Name of qdisc or NULL if not specified.
 */
char *rtnl_link_get_qdisc(struct rtnl_link *link)
{
	return link->ce_mask & LINK_ATTR_QDISC ? link->l_qdisc : NULL;
}

/**
 * Return number of PCI virtual functions of link object
 * @arg link		Link object
 * @arg num_vf		Pointer to store number of VFs
 *
 * @return 0 on success or -NLE_OPNOTSUPP if not available
 */
int rtnl_link_get_num_vf(struct rtnl_link *link, uint32_t *num_vf)
{
	if (link->ce_mask & LINK_ATTR_NUM_VF) {
		*num_vf = link->l_num_vf;
		return 0;
	} else
		return -NLE_OPNOTSUPP;
}

/**
 * Return value of link statistics counter
 * @arg link		Link object
 * @arg id		Identifier of statistical counter
 *
 * @return Value of counter or 0 if not specified.
 */
uint64_t rtnl_link_get_stat(struct rtnl_link *link, rtnl_link_stat_id_t id)
{
	if (id > RTNL_LINK_STATS_MAX)
		return 0;

	return link->l_stats[id];
}

/**
 * Set value of link statistics counter
 * @arg link		Link object
 * @arg id		Identifier of statistical counter
 * @arg value		New value
 *
 * \note Changing the value of a statistical counter will not change the
 *       value in the kernel.
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_set_stat(struct rtnl_link *link, rtnl_link_stat_id_t id,
		       const uint64_t value)
{
	if (id > RTNL_LINK_STATS_MAX)
		return -NLE_INVAL;

	link->l_stats[id] = value;

	return 0;
}

/**
 * Set type of link object
 * @arg link		Link object
 * @arg type		Name of link type
 *
 * Looks up the link type module and prepares the link to store type
 * specific attributes. If a type has been assigned already it will
 * be released with all link type specific attributes lost.
 *
 * @route_doc{link_modules, Link Modules}
 * @return 0 on success or a negative errror code.
 */
int rtnl_link_set_type(struct rtnl_link *link, const char *type)
{
	struct rtnl_link_info_ops *io;
	int err;
	char *kind;

	free(link->l_info_kind);
	link->ce_mask &= ~LINK_ATTR_LINKINFO;
	if (link->l_info_ops)
		release_link_info(link);

	if (!type)
		return 0;

	kind = strdup(type);
	if (!kind)
		return -NLE_NOMEM;

	io = rtnl_link_info_ops_lookup(type);
	if (io) {
		if (io->io_alloc && (err = io->io_alloc(link)) < 0)
			goto errout;

		link->l_info_ops = io;
	}

	link->l_info_kind = kind;
	link->ce_mask |= LINK_ATTR_LINKINFO;

	return 0;

errout:
	free(kind);
	return err;
}

/**
 * Return type of link
 * @arg link		Link object
 *
 * @route_doc{link_modules, Link Modules}
 * @return Name of link type or NULL if not specified.
 */
char *rtnl_link_get_type(struct rtnl_link *link)
{
	return link->l_info_kind;
}

/**
 * Set link promiscuity count
 * @arg link		Link object
 * @arg count		New promiscuity count
 *
 * @copydoc read_only_attribute
 *
 * @see rtnl_link_get_promiscuity()
 */
void rtnl_link_set_promiscuity(struct rtnl_link *link, uint32_t count)
{
	link->l_promiscuity = count;
	link->ce_mask |= LINK_ATTR_PROMISCUITY;
}

/**
 * Return link promiscuity count
 * @arg link		Link object
 *
 * @see rtnl_link_set_promiscuity()
 * @return Link promiscuity count or 0
 */
uint32_t rtnl_link_get_promiscuity(struct rtnl_link *link)
{
	return link->l_promiscuity;
}

/**
 * Set number of TX queues
 * @arg link		Link object
 * @arg nqueues		Number of queues
 *
 * Sets the number of TX queues of the link object. The value is considered
 * by the kernel when creating network devices that can be created via
 * netlink. The value will be passed on to alloc_netdev_mqs()
 *
 * Therefore use of rtnl_link_set_num_tx_queues() only makes sense in
 * combination with rtnl_link_add() or if the link object is used as a filter.
 *
 * @see rtnl_link_get_num_tx_queues()
 */
void rtnl_link_set_num_tx_queues(struct rtnl_link *link, uint32_t nqueues)
{
	link->l_num_tx_queues = nqueues;
	link->ce_mask |= LINK_ATTR_NUM_TX_QUEUES;
}

/**
 * Return number of TX queues
 * @arg link		Link object
 *
 * @return Number of TX queues or 0
 */
uint32_t rtnl_link_get_num_tx_queues(struct rtnl_link *link)
{
	return link->l_num_tx_queues;
}

/**
 * Set number of RX queues
 * @arg link		Link object
 * @arg nqueues		Number of queues
 *
 * Sets the number of RX queues of the link object. The value is considered
 * by the kernel when creating network devices that can be created via
 * netlink. The value will be passed on to alloc_netdev_mqs()
 *
 * Therefore use of rtnl_link_set_num_rx_queues() only makes sense in
 * combination with rtnl_link_add() or if the link object is used as a filter.
 *
 * @see rtnl_link_get_num_rx_queues()
 */
void rtnl_link_set_num_rx_queues(struct rtnl_link *link, uint32_t nqueues)
{
	link->l_num_rx_queues = nqueues;
	link->ce_mask |= LINK_ATTR_NUM_RX_QUEUES;
}

/**
 * Return number of RX queues
 * @arg link		Link object
 *
 * @return Number of RX queues or 0
 */
uint32_t rtnl_link_get_num_rx_queues(struct rtnl_link *link)
{
	return link->l_num_rx_queues;
}

/**
 * Return physical port id of link object
 * @arg link		Link object
 *
 * @return Physical port id or NULL if not set.
 */
struct nl_data *rtnl_link_get_phys_port_id(struct rtnl_link *link)
{
	return link->l_phys_port_id;
}

void rtnl_link_set_ns_fd(struct rtnl_link *link, int fd)
{
	link->l_ns_fd = fd;
	link->ce_mask |= LINK_ATTR_NS_FD;
}

int rtnl_link_get_ns_fd(struct rtnl_link *link)
{
	return link->l_ns_fd;
}

void rtnl_link_set_ns_pid(struct rtnl_link *link, pid_t pid)
{
	link->l_ns_pid = pid;
	link->ce_mask |= LINK_ATTR_NS_PID;
}

pid_t rtnl_link_get_ns_pid(struct rtnl_link *link)
{
	return link->l_ns_pid;
}

/** @} */

/**
 * @name Master/Slave
 * @{
 */

/**
 * Enslave slave link to master link
 * @arg sock		netlink socket
 * @arg master		ifindex of master link
 * @arg slave		ifindex of slave link
 *
 * This function is identical to rtnl_link_enslave() except that
 * it takes interface indices instead of rtnl_link objects.
 *
 * @see rtnl_link_enslave()
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_link_enslave_ifindex(struct nl_sock *sock, int master, int slave)
{
	struct rtnl_link *link;
	int err;

	if (!(link = rtnl_link_alloc()))
		return -NLE_NOMEM;

	rtnl_link_set_ifindex(link, slave);
	rtnl_link_set_master(link, master);

	if ((err = rtnl_link_change(sock, link, link, 0)) < 0)
		goto errout;

	rtnl_link_put(link);

	/*
	 * Due to the kernel not signaling whether this opertion is
	 * supported or not, we will retrieve the attribute to see  if the
	 * request was successful. If the master assigned remains unchanged
	 * we will return NLE_OPNOTSUPP to allow performing backwards
	 * compatibility of some sort.
	 */
	if ((err = rtnl_link_get_kernel(sock, slave, NULL, &link)) < 0)
		return err;

	if (rtnl_link_get_master(link) != master)
		err = -NLE_OPNOTSUPP;

errout:
	rtnl_link_put(link);

	return err;
}

/**
 * Enslave slave link to master link
 * @arg sock		netlink socket
 * @arg master		master link
 * @arg slave		slave link
 *
 * Constructs a RTM_NEWLINK or RTM_SETLINK message adding the slave to
 * the master and sends the request via the specified netlink socket.
 *
 * @note The feature of enslaving/releasing via netlink has only been added
 *       recently to the kernel (Feb 2011). Also, the kernel does not signal
 *       if the operation is not supported. Therefore this function will
 *       verify if the master assignment has changed and will return
 *       -NLE_OPNOTSUPP if it did not.
 *
 * @see rtnl_link_enslave_ifindex()
 * @see rtnl_link_release()
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_link_enslave(struct nl_sock *sock, struct rtnl_link *master,
		      struct rtnl_link *slave)
{
	return rtnl_link_enslave_ifindex(sock, rtnl_link_get_ifindex(master),
					 rtnl_link_get_ifindex(slave));
}

/**
 * Release slave link from its master
 * @arg sock		netlink socket
 * @arg slave		slave link
 *
 * This function is identical to rtnl_link_release() except that
 * it takes an interface index instead of a rtnl_link object.
 *
 * @see rtnl_link_release()
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_link_release_ifindex(struct nl_sock *sock, int slave)
{
	return rtnl_link_enslave_ifindex(sock, 0, slave);
}

/**
 * Release slave link from its master
 * @arg sock		netlink socket
 * @arg slave		slave link
 *
 * Constructs a RTM_NEWLINK or RTM_SETLINK message releasing the slave from
 * its master and sends the request via the specified netlink socket.
 *
 * @note The feature of enslaving/releasing via netlink has only been added
 *       recently to the kernel (Feb 2011). Also, the kernel does not signal
 *       if the operation is not supported. Therefore this function will
 *       verify if the master assignment has changed and will return
 *       -NLE_OPNOTSUPP if it did not.
 *
 * @see rtnl_link_release_ifindex()
 * @see rtnl_link_enslave()
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_link_release(struct nl_sock *sock, struct rtnl_link *slave)
{
	return rtnl_link_release_ifindex(sock, rtnl_link_get_ifindex(slave));
}

/** @} */

/**
 * @name Utilities
 * @{
 */

static const struct trans_tbl link_flags[] = {
	__ADD(IFF_LOOPBACK, loopback)
	__ADD(IFF_BROADCAST, broadcast)
	__ADD(IFF_POINTOPOINT, pointopoint)
	__ADD(IFF_MULTICAST, multicast)
	__ADD(IFF_NOARP, noarp)
	__ADD(IFF_ALLMULTI, allmulti)
	__ADD(IFF_PROMISC, promisc)
	__ADD(IFF_MASTER, master)
	__ADD(IFF_SLAVE, slave)
	__ADD(IFF_DEBUG, debug)
	__ADD(IFF_DYNAMIC, dynamic)
	__ADD(IFF_AUTOMEDIA, automedia)
	__ADD(IFF_PORTSEL, portsel)
	__ADD(IFF_NOTRAILERS, notrailers)
	__ADD(IFF_UP, up)
	__ADD(IFF_RUNNING, running)
	__ADD(IFF_LOWER_UP, lowerup)
	__ADD(IFF_DORMANT, dormant)
	__ADD(IFF_ECHO, echo)
};

char *rtnl_link_flags2str(int flags, char *buf, size_t len)
{
	return __flags2str(flags, buf, len, link_flags,
			   ARRAY_SIZE(link_flags));
}

int rtnl_link_str2flags(const char *name)
{
	return __str2flags(name, link_flags, ARRAY_SIZE(link_flags));
}

static const struct trans_tbl link_stats[] = {
	__ADD(RTNL_LINK_RX_PACKETS, rx_packets)
	__ADD(RTNL_LINK_TX_PACKETS, tx_packets)
	__ADD(RTNL_LINK_RX_BYTES, rx_bytes)
	__ADD(RTNL_LINK_TX_BYTES, tx_bytes)
	__ADD(RTNL_LINK_RX_ERRORS, rx_errors)
	__ADD(RTNL_LINK_TX_ERRORS, tx_errors)
	__ADD(RTNL_LINK_RX_DROPPED, rx_dropped)
	__ADD(RTNL_LINK_TX_DROPPED, tx_dropped)
	__ADD(RTNL_LINK_RX_COMPRESSED, rx_compressed)
	__ADD(RTNL_LINK_TX_COMPRESSED, tx_compressed)
	__ADD(RTNL_LINK_RX_FIFO_ERR, rx_fifo_err)
	__ADD(RTNL_LINK_TX_FIFO_ERR, tx_fifo_err)
	__ADD(RTNL_LINK_RX_LEN_ERR, rx_len_err)
	__ADD(RTNL_LINK_RX_OVER_ERR, rx_over_err)
	__ADD(RTNL_LINK_RX_CRC_ERR, rx_crc_err)
	__ADD(RTNL_LINK_RX_FRAME_ERR, rx_frame_err)
	__ADD(RTNL_LINK_RX_MISSED_ERR, rx_missed_err)
	__ADD(RTNL_LINK_TX_ABORT_ERR, tx_abort_err)
	__ADD(RTNL_LINK_TX_CARRIER_ERR, tx_carrier_err)
	__ADD(RTNL_LINK_TX_HBEAT_ERR, tx_hbeat_err)
	__ADD(RTNL_LINK_TX_WIN_ERR, tx_win_err)
	__ADD(RTNL_LINK_COLLISIONS, collisions)
	__ADD(RTNL_LINK_MULTICAST, multicast)
	__ADD(RTNL_LINK_IP6_INPKTS, Ip6InReceives)
	__ADD(RTNL_LINK_IP6_INHDRERRORS, Ip6InHdrErrors)
	__ADD(RTNL_LINK_IP6_INTOOBIGERRORS, Ip6InTooBigErrors)
	__ADD(RTNL_LINK_IP6_INNOROUTES, Ip6InNoRoutes)
	__ADD(RTNL_LINK_IP6_INADDRERRORS, Ip6InAddrErrors)
	__ADD(RTNL_LINK_IP6_INUNKNOWNPROTOS, Ip6InUnknownProtos)
	__ADD(RTNL_LINK_IP6_INTRUNCATEDPKTS, Ip6InTruncatedPkts)
	__ADD(RTNL_LINK_IP6_INDISCARDS, Ip6InDiscards)
	__ADD(RTNL_LINK_IP6_INDELIVERS, Ip6InDelivers)
	__ADD(RTNL_LINK_IP6_OUTFORWDATAGRAMS, Ip6OutForwDatagrams)
	__ADD(RTNL_LINK_IP6_OUTPKTS, Ip6OutRequests)
	__ADD(RTNL_LINK_IP6_OUTDISCARDS, Ip6OutDiscards)
	__ADD(RTNL_LINK_IP6_OUTNOROUTES, Ip6OutNoRoutes)
	__ADD(RTNL_LINK_IP6_REASMTIMEOUT, Ip6ReasmTimeout)
	__ADD(RTNL_LINK_IP6_REASMREQDS, Ip6ReasmReqds)
	__ADD(RTNL_LINK_IP6_REASMOKS, Ip6ReasmOKs)
	__ADD(RTNL_LINK_IP6_REASMFAILS, Ip6ReasmFails)
	__ADD(RTNL_LINK_IP6_FRAGOKS, Ip6FragOKs)
	__ADD(RTNL_LINK_IP6_FRAGFAILS, Ip6FragFails)
	__ADD(RTNL_LINK_IP6_FRAGCREATES, Ip6FragCreates)
	__ADD(RTNL_LINK_IP6_INMCASTPKTS, Ip6InMcastPkts)
	__ADD(RTNL_LINK_IP6_OUTMCASTPKTS, Ip6OutMcastPkts)
	__ADD(RTNL_LINK_IP6_INBCASTPKTS, Ip6InBcastPkts)
	__ADD(RTNL_LINK_IP6_OUTBCASTPKTS, Ip6OutBcastPkts)
	__ADD(RTNL_LINK_IP6_INOCTETS, Ip6InOctets)
	__ADD(RTNL_LINK_IP6_OUTOCTETS, Ip6OutOctets)
	__ADD(RTNL_LINK_IP6_INMCASTOCTETS, Ip6InMcastOctets)
	__ADD(RTNL_LINK_IP6_OUTMCASTOCTETS, Ip6OutMcastOctets)
	__ADD(RTNL_LINK_IP6_INBCASTOCTETS, Ip6InBcastOctets)
	__ADD(RTNL_LINK_IP6_OUTBCASTOCTETS, Ip6OutBcastOctets)
	__ADD(RTNL_LINK_ICMP6_INMSGS, ICMP6_InMsgs)
	__ADD(RTNL_LINK_ICMP6_INERRORS, ICMP6_InErrors)
	__ADD(RTNL_LINK_ICMP6_OUTMSGS, ICMP6_OutMsgs)
	__ADD(RTNL_LINK_ICMP6_OUTERRORS, ICMP6_OutErrors)
	__ADD(RTNL_LINK_ICMP6_CSUMERRORS, ICMP6_InCsumErrors)
	__ADD(RTNL_LINK_IP6_CSUMERRORS, Ip6_InCsumErrors)
	__ADD(RTNL_LINK_IP6_NOECTPKTS, Ip6_InNoECTPkts)
	__ADD(RTNL_LINK_IP6_ECT1PKTS, Ip6_InECT1Pkts)
	__ADD(RTNL_LINK_IP6_ECT0PKTS, Ip6_InECT0Pkts)
	__ADD(RTNL_LINK_IP6_CEPKTS, Ip6_InCEPkts)
};

char *rtnl_link_stat2str(int st, char *buf, size_t len)
{
	return __type2str(st, buf, len, link_stats, ARRAY_SIZE(link_stats));
}

int rtnl_link_str2stat(const char *name)
{
	return __str2type(name, link_stats, ARRAY_SIZE(link_stats));
}

static const struct trans_tbl link_operstates[] = {
	__ADD(IF_OPER_UNKNOWN, unknown)
	__ADD(IF_OPER_NOTPRESENT, notpresent)
	__ADD(IF_OPER_DOWN, down)
	__ADD(IF_OPER_LOWERLAYERDOWN, lowerlayerdown)
	__ADD(IF_OPER_TESTING, testing)
	__ADD(IF_OPER_DORMANT, dormant)
	__ADD(IF_OPER_UP, up)
};

char *rtnl_link_operstate2str(uint8_t st, char *buf, size_t len)
{
	return __type2str(st, buf, len, link_operstates,
			  ARRAY_SIZE(link_operstates));
}

int rtnl_link_str2operstate(const char *name)
{
	return __str2type(name, link_operstates,
			  ARRAY_SIZE(link_operstates));
}

static const struct trans_tbl link_modes[] = {
	__ADD(IF_LINK_MODE_DEFAULT, default)
	__ADD(IF_LINK_MODE_DORMANT, dormant)
};

static const struct trans_tbl carrier_states[] = {
	__ADD(IF_CARRIER_DOWN, down)
	__ADD(IF_CARRIER_UP, up)
};

char *rtnl_link_mode2str(uint8_t st, char *buf, size_t len)
{
	return __type2str(st, buf, len, link_modes, ARRAY_SIZE(link_modes));
}

int rtnl_link_str2mode(const char *name)
{
	return __str2type(name, link_modes, ARRAY_SIZE(link_modes));
}

char *rtnl_link_carrier2str(uint8_t st, char *buf, size_t len)
{
	return __type2str(st, buf, len, carrier_states,
			  ARRAY_SIZE(carrier_states));
}

int rtnl_link_str2carrier(const char *name)
{
	return __str2type(name, carrier_states, ARRAY_SIZE(carrier_states));
}

/** @} */

/**
 * @name Deprecated Functions
 */

/**
 * @deprecated Use of this function is deprecated, use rtnl_link_set_type()
 */
int rtnl_link_set_info_type(struct rtnl_link *link, const char *type)
{
	return rtnl_link_set_type(link, type);
}

/**
 * @deprecated Use of this function is deprecated, use rtnl_link_get_type()
 */
char *rtnl_link_get_info_type(struct rtnl_link *link)
{
	return rtnl_link_get_type(link);
}

/**
 * @deprecated The weight attribute is unused and obsoleted in all recent kernels
 */
void rtnl_link_set_weight(struct rtnl_link *link, unsigned int weight)
{
	link->l_weight = weight;
	link->ce_mask |= LINK_ATTR_WEIGHT;
}

/**
 * @deprecated The weight attribute is unused and obsoleted in all recent kernels
 */
unsigned int rtnl_link_get_weight(struct rtnl_link *link)
{
	return link->l_weight;
}

/** @} */

static struct nl_object_ops link_obj_ops = {
	.oo_name		= "route/link",
	.oo_size		= sizeof(struct rtnl_link),
	.oo_free_data		= link_free_data,
	.oo_clone		= link_clone,
	.oo_dump = {
	    [NL_DUMP_LINE]	= link_dump_line,
	    [NL_DUMP_DETAILS]	= link_dump_details,
	    [NL_DUMP_STATS]	= link_dump_stats,
	},
	.oo_compare		= link_compare,
	.oo_keygen		= link_keygen,
	.oo_attrs2str		= link_attrs2str,
	.oo_id_attrs		= LINK_ATTR_IFINDEX | LINK_ATTR_FAMILY,
};

static struct nl_af_group link_groups[] = {
	{ AF_UNSPEC,	RTNLGRP_LINK },
	{ AF_BRIDGE,    RTNLGRP_LINK },
	{ END_OF_GROUP_LIST },
};

static struct nl_cache_ops rtnl_link_ops = {
	.co_name		= "route/link",
	.co_hdrsize		= sizeof(struct ifinfomsg),
	.co_msgtypes		= {
					{ RTM_NEWLINK, NL_ACT_NEW, "new" },
					{ RTM_DELLINK, NL_ACT_DEL, "del" },
					{ RTM_GETLINK, NL_ACT_GET, "get" },
					{ RTM_SETLINK, NL_ACT_CHANGE, "set" },
					END_OF_MSGTYPES_LIST,
				  },
	.co_protocol		= NETLINK_ROUTE,
	.co_groups		= link_groups,
	.co_request_update	= link_request_update,
	.co_msg_parser		= link_msg_parser,
	.co_obj_ops		= &link_obj_ops,
};

static void __init link_init(void)
{
	nl_cache_mngt_register(&rtnl_link_ops);
}

static void __exit link_exit(void)
{
	nl_cache_mngt_unregister(&rtnl_link_ops);
}

/** @} */
