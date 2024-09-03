/*
 * lib/idiag/idiagnl_msg_obj.c Inet Diag Message Object
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2013 Sassano Systems LLC <joe@sassanosystems.com>
 */

#include <netlink-private/netlink.h>
#include <netlink/idiag/msg.h>
#include <netlink/idiag/meminfo.h>
#include <netlink/idiag/vegasinfo.h>
#include <linux/inet_diag.h>

/**
 * @ingroup idiag
 * @defgroup idiagnl_msg Inet Diag Messages
 *
 * @details
 * @idiagnl_doc{idiagnl_msg, Inet Diag Message Documentation}
 * @{
 */
struct idiagnl_msg *idiagnl_msg_alloc(void)
{
	return (struct idiagnl_msg *) nl_object_alloc(&idiagnl_msg_obj_ops);
}

void idiagnl_msg_get(struct idiagnl_msg *msg)
{
	nl_object_get((struct nl_object *) msg);
}

void idiagnl_msg_put(struct idiagnl_msg *msg)
{
	nl_object_put((struct nl_object *) msg);
}

static struct nl_cache_ops idiagnl_msg_ops;

static int idiagnl_msg_parser(struct nl_cache_ops *ops, struct sockaddr_nl *who,
		struct nlmsghdr *nlh, struct nl_parser_param *pp)
{
	struct idiagnl_msg *msg = NULL;
	int err = 0;

	if ((err = idiagnl_msg_parse(nlh, &msg)) < 0)
		return err;

	err = pp->pp_cb((struct nl_object *) msg, pp);
	idiagnl_msg_put(msg);

	return err;
}

static int idiagnl_request_update(struct nl_cache *cache, struct nl_sock *sk)
{
	int family = cache->c_iarg1;
	int states = cache->c_iarg2;

	return idiagnl_send_simple(sk, 0, family, states, IDIAG_ATTR_ALL);
}

static struct nl_cache_ops idiagnl_msg_ops = {
	.co_name		= "idiag/idiag",
	.co_hdrsize		= sizeof(struct inet_diag_msg),
	.co_msgtypes		= {
		{ IDIAG_TCPDIAG_GETSOCK, NL_ACT_NEW, "new" },
		{ IDIAG_DCCPDIAG_GETSOCK, NL_ACT_NEW, "new" },
		END_OF_MSGTYPES_LIST,
	},
	.co_protocol		= NETLINK_INET_DIAG,
	.co_request_update	= idiagnl_request_update,
	.co_msg_parser		= idiagnl_msg_parser,
	.co_obj_ops		= &idiagnl_msg_obj_ops,
};

static void __init idiagnl_init(void)
{
	nl_cache_mngt_register(&idiagnl_msg_ops);
}

static void __exit idiagnl_exit(void)
{
	nl_cache_mngt_unregister(&idiagnl_msg_ops);
}

/**
 * @name Cache Management
 * @{
 */

/**
 * Build an inetdiag cache to hold socket state information.
 * @arg	sk      Netlink socket
 * @arg family  The address family to query
 * @arg states  Socket states to query
 * @arg result  Result pointer
 *
 * @note The caller is responsible for destroying and free the cache after using
 *  it.
 * @return 0 on success of a negative error code.
 */
int idiagnl_msg_alloc_cache(struct nl_sock *sk, int family, int states,
		struct nl_cache **result)
{
	struct nl_cache *cache = NULL;
	int err;

	if (!(cache = nl_cache_alloc(&idiagnl_msg_ops)))
		return -NLE_NOMEM;

	cache->c_iarg1 = family;
	cache->c_iarg2 = states;

	if (sk && (err = nl_cache_refill(sk, cache)) < 0) {
		free(cache);
		return err;
	}

	*result = cache;
	return 0;
}

/** @} */

/**
 * @name Attributes
 * @{
 */

uint8_t idiagnl_msg_get_family(const struct idiagnl_msg *msg)
{
	return msg->idiag_family;
}

void idiagnl_msg_set_family(struct idiagnl_msg *msg, uint8_t family)
{
	msg->idiag_family = family;
}

uint8_t idiagnl_msg_get_state(const struct idiagnl_msg *msg)
{
	return msg->idiag_state;
}

void idiagnl_msg_set_state(struct idiagnl_msg *msg, uint8_t state)
{
	msg->idiag_state = state;
}

uint8_t idiagnl_msg_get_timer(const struct idiagnl_msg *msg)
{
	return msg->idiag_timer;
}

void idiagnl_msg_set_timer(struct idiagnl_msg *msg, uint8_t timer)
{
	msg->idiag_timer = timer;
}

uint8_t idiagnl_msg_get_retrans(const struct idiagnl_msg *msg)
{
	return msg->idiag_retrans;
}

void idiagnl_msg_set_retrans(struct idiagnl_msg *msg, uint8_t retrans)
{
	msg->idiag_retrans = retrans;
}

uint16_t idiagnl_msg_get_sport(struct idiagnl_msg *msg)
{
	return msg->idiag_sport;
}

void idiagnl_msg_set_sport(struct idiagnl_msg *msg, uint16_t port)
{
	msg->idiag_sport = port;
}

uint16_t idiagnl_msg_get_dport(struct idiagnl_msg *msg)
{
	return msg->idiag_dport;
}

void idiagnl_msg_set_dport(struct idiagnl_msg *msg, uint16_t port)
{
	msg->idiag_dport = port;
}

struct nl_addr *idiagnl_msg_get_src(const struct idiagnl_msg *msg)
{
	return msg->idiag_src;
}

int idiagnl_msg_set_src(struct idiagnl_msg *msg, struct nl_addr *addr)
{
	if (msg->idiag_src)
		nl_addr_put(msg->idiag_src);

	nl_addr_get(addr);
	msg->idiag_src = addr;

	return 0;
}

struct nl_addr *idiagnl_msg_get_dst(const struct idiagnl_msg *msg)
{
	return msg->idiag_dst;
}

int idiagnl_msg_set_dst(struct idiagnl_msg *msg, struct nl_addr *addr)
{
	if (msg->idiag_dst)
		nl_addr_put(msg->idiag_dst);

	nl_addr_get(addr);
	msg->idiag_dst = addr;

	return 0;
}

uint32_t idiagnl_msg_get_ifindex(const struct idiagnl_msg *msg)
{
	return msg->idiag_ifindex;
}

void idiagnl_msg_set_ifindex(struct idiagnl_msg *msg, uint32_t ifindex)
{
	msg->idiag_ifindex = ifindex;
}

uint32_t idiagnl_msg_get_expires(const struct idiagnl_msg *msg)
{
	return msg->idiag_expires;
}

void idiagnl_msg_set_expires(struct idiagnl_msg *msg, uint32_t expires)
{
	msg->idiag_expires = expires;
}

uint32_t idiagnl_msg_get_rqueue(const struct idiagnl_msg *msg)
{
	return msg->idiag_rqueue;
}

void idiagnl_msg_set_rqueue(struct idiagnl_msg *msg, uint32_t rqueue)
{
	msg->idiag_rqueue = rqueue;
}

uint32_t idiagnl_msg_get_wqueue(const struct idiagnl_msg *msg)
{
	return msg->idiag_wqueue;
}

void idiagnl_msg_set_wqueue(struct idiagnl_msg *msg, uint32_t wqueue)
{
	msg->idiag_wqueue = wqueue;
}

uint32_t idiagnl_msg_get_uid(const struct idiagnl_msg *msg)
{
	return msg->idiag_uid;
}

void idiagnl_msg_set_uid(struct idiagnl_msg *msg, uint32_t uid)
{
	msg->idiag_uid = uid;
}

uint32_t idiagnl_msg_get_inode(const struct idiagnl_msg *msg)
{
	return msg->idiag_inode;
}

void idiagnl_msg_set_inode(struct idiagnl_msg *msg, uint32_t inode)
{
	msg->idiag_inode = inode;
}

uint8_t idiagnl_msg_get_tos(const struct idiagnl_msg *msg)
{
	return msg->idiag_tos;
}

void idiagnl_msg_set_tos(struct idiagnl_msg *msg, uint8_t tos)
{
	msg->idiag_tos = tos;
}

uint8_t idiagnl_msg_get_tclass(const struct idiagnl_msg *msg)
{
	return msg->idiag_tclass;
}

void idiagnl_msg_set_tclass(struct idiagnl_msg *msg, uint8_t tclass)
{
	msg->idiag_tclass = tclass;
}

uint8_t	idiagnl_msg_get_shutdown(const struct idiagnl_msg *msg)
{
	return msg->idiag_shutdown;
}

void  idiagnl_msg_set_shutdown(struct idiagnl_msg *msg, uint8_t shutdown)
{
	msg->idiag_shutdown = shutdown;
}

char *idiagnl_msg_get_cong(const struct idiagnl_msg *msg)
{
	return msg->idiag_cong;
}

void idiagnl_msg_set_cong(struct idiagnl_msg *msg, char *cong)
{
	msg->idiag_cong = strdup(cong);
}

struct idiagnl_meminfo *idiagnl_msg_get_meminfo(const struct idiagnl_msg *msg)
{
	return msg->idiag_meminfo;
}

void idiagnl_msg_set_meminfo(struct idiagnl_msg *msg, struct idiagnl_meminfo
		*minfo)
{
	if (msg->idiag_meminfo)
		idiagnl_meminfo_put(msg->idiag_meminfo);

	idiagnl_meminfo_get(minfo);
	msg->idiag_meminfo = minfo;
}

struct idiagnl_vegasinfo *idiagnl_msg_get_vegasinfo(const struct idiagnl_msg *msg)
{
	return msg->idiag_vegasinfo;
}

void idiagnl_msg_set_vegasinfo(struct idiagnl_msg *msg, struct idiagnl_vegasinfo
		*vinfo)
{
	if (msg->idiag_vegasinfo)
		idiagnl_vegasinfo_put(msg->idiag_vegasinfo);

	idiagnl_vegasinfo_get(vinfo);
	msg->idiag_vegasinfo = vinfo;
}

struct tcp_info idiagnl_msg_get_tcpinfo(const struct idiagnl_msg *msg)
{
	return msg->idiag_tcpinfo;
}

void idiagnl_msg_set_tcpinfo(struct idiagnl_msg *msg, struct tcp_info *tinfo)
{
	memcpy(&msg->idiag_tcpinfo, tinfo, sizeof(struct tcp_info));
}

/** @} */

static void idiag_msg_dump_line(struct nl_object *a, struct nl_dump_params *p)
{
	struct idiagnl_msg *msg = (struct idiagnl_msg *) a;
	char buf[64] = { 0 };

	nl_dump_line(p, "family: %s ", nl_af2str(msg->idiag_family, buf, sizeof(buf)));
	nl_dump(p, "src: %s:%d ", nl_addr2str(msg->idiag_src, buf, sizeof(buf)),
			ntohs(msg->idiag_sport));
	nl_dump(p, "dst: %s:%d ", nl_addr2str(msg->idiag_dst, buf, sizeof(buf)),
			ntohs(msg->idiag_dport));
	nl_dump(p, "iif: %d ", msg->idiag_ifindex);
	nl_dump(p, "\n");
}

static void idiag_msg_dump_details(struct nl_object *a, struct nl_dump_params *p)
{
	struct idiagnl_msg *msg = (struct idiagnl_msg *) a;
	char buf[64], buf2[64];

	nl_dump(p, "\nfamily: %s\n", nl_af2str(msg->idiag_family, buf, sizeof(buf)));
	nl_dump(p, "state: %s\n",
			idiagnl_state2str(msg->idiag_state, buf, sizeof(buf)));
	nl_dump(p, "timer (%s, %s, retransmits: %d)\n",
			idiagnl_timer2str(msg->idiag_timer, buf, sizeof(buf)),
			nl_msec2str(msg->idiag_expires, buf2, sizeof(buf2)),
			msg->idiag_retrans);

	nl_dump(p, "source: %s:%d\n", nl_addr2str(msg->idiag_src, buf, sizeof(buf)),
			ntohs(msg->idiag_sport));
	nl_dump(p, "destination: %s:%d\n", nl_addr2str(msg->idiag_dst, buf, sizeof(buf)),
			ntohs(msg->idiag_dport));

	nl_dump(p, "ifindex: %d\n", msg->idiag_ifindex);
	nl_dump(p, "rqueue: %-6d wqueue: %-6d\n", msg->idiag_rqueue, msg->idiag_wqueue);
	nl_dump(p, "uid %d\n", msg->idiag_uid);
	nl_dump(p, "inode %d\n", msg->idiag_inode);
	if (msg->idiag_shutdown) {
		nl_dump(p, "socket shutdown: %s\n",
				idiagnl_shutdown2str(msg->idiag_shutdown,
					buf, sizeof(buf)));
	}

	nl_dump(p, "tos: 0x%x\n", msg->idiag_tos);
	nl_dump(p, "traffic class: %d\n", msg->idiag_tclass);
	nl_dump(p, "congestion algorithm: %s\n", msg->idiag_cong);
}

static void idiag_msg_dump_stats(struct nl_object *obj, struct nl_dump_params *p)
{
	struct idiagnl_msg *msg = (struct idiagnl_msg *) obj;
	char buf[64];

	idiag_msg_dump_details(obj, p);

	nl_dump(p, "tcp info:  [\n");
	nl_dump(p, "\tsocket state: %s\n",
			idiagnl_state2str(msg->idiag_tcpinfo.tcpi_state,
				buf, sizeof(buf)));
	nl_dump(p, "\ttcp state: %s\n",
			idiagnl_tcpstate2str(msg->idiag_tcpinfo.tcpi_ca_state,
				buf, sizeof(buf)));
	nl_dump(p, "\tretransmits: %d\n",
			msg->idiag_tcpinfo.tcpi_retransmits);
	nl_dump(p, "\tprobes: %d\n",
			msg->idiag_tcpinfo.tcpi_probes);
	nl_dump(p, "\tbackoff: %d\n",
			msg->idiag_tcpinfo.tcpi_backoff);
	nl_dump(p, "\toptions: %s\n",
			idiagnl_tcpopts2str(msg->idiag_tcpinfo.tcpi_options,
				buf, sizeof(buf)));
	nl_dump(p, "\tsnd_wscale: %d\n", msg->idiag_tcpinfo.tcpi_snd_wscale);
	nl_dump(p, "\trcv_wscale: %d\n", msg->idiag_tcpinfo.tcpi_rcv_wscale);
	nl_dump(p, "\trto: %d\n", msg->idiag_tcpinfo.tcpi_rto);
	nl_dump(p, "\tato: %d\n", msg->idiag_tcpinfo.tcpi_ato);
	nl_dump(p, "\tsnd_mss: %s\n", nl_size2str(msg->idiag_tcpinfo.tcpi_snd_mss,
				buf, sizeof(buf)));
	nl_dump(p, "\trcv_mss: %s\n", nl_size2str(msg->idiag_tcpinfo.tcpi_rcv_mss,
				buf, sizeof(buf)));
	nl_dump(p, "\tunacked: %d\n", msg->idiag_tcpinfo.tcpi_unacked);
	nl_dump(p, "\tsacked: %d\n", msg->idiag_tcpinfo.tcpi_sacked);

	nl_dump(p, "\tlost: %d\n", msg->idiag_tcpinfo.tcpi_lost);
	nl_dump(p, "\tretransmit segments: %d\n",
			msg->idiag_tcpinfo.tcpi_retrans);
	nl_dump(p, "\tfackets: %d\n",
			msg->idiag_tcpinfo.tcpi_fackets);
	nl_dump(p, "\tlast data sent: %s\n",
			nl_msec2str(msg->idiag_tcpinfo.tcpi_last_data_sent, buf,
				sizeof(buf)));
	nl_dump(p, "\tlast ack sent: %s\n",
			nl_msec2str(msg->idiag_tcpinfo.tcpi_last_ack_sent, buf, sizeof(buf)));
	nl_dump(p, "\tlast data recv: %s\n",
			nl_msec2str(msg->idiag_tcpinfo.tcpi_last_data_recv, buf,
				sizeof(buf)));
	nl_dump(p, "\tlast ack recv: %s\n",
			nl_msec2str(msg->idiag_tcpinfo.tcpi_last_ack_recv, buf,
				sizeof(buf)));
	nl_dump(p, "\tpath mtu: %s\n",
			nl_size2str(msg->idiag_tcpinfo.tcpi_pmtu, buf,
				sizeof(buf)));
	nl_dump(p, "\trcv ss threshold: %d\n",
			msg->idiag_tcpinfo.tcpi_rcv_ssthresh);
	nl_dump(p, "\tsmoothed round trip time: %d\n",
			msg->idiag_tcpinfo.tcpi_rtt);
	nl_dump(p, "\tround trip time variation: %d\n",
			msg->idiag_tcpinfo.tcpi_rttvar);
	nl_dump(p, "\tsnd ss threshold: %s\n",
			nl_size2str(msg->idiag_tcpinfo.tcpi_snd_ssthresh, buf,
				sizeof(buf)));
	nl_dump(p, "\tsend congestion window: %d\n",
			msg->idiag_tcpinfo.tcpi_snd_cwnd);
	nl_dump(p, "\tadvertised mss: %s\n",
			nl_size2str(msg->idiag_tcpinfo.tcpi_advmss, buf,
				sizeof(buf)));
	nl_dump(p, "\treordering: %d\n",
			msg->idiag_tcpinfo.tcpi_reordering);
	nl_dump(p, "\trcv rround trip time: %d\n",
			msg->idiag_tcpinfo.tcpi_rcv_rtt);
	nl_dump(p, "\treceive queue space: %s\n",
			nl_size2str(msg->idiag_tcpinfo.tcpi_rcv_space, buf,
				sizeof(buf)));
	nl_dump(p, "\ttotal retransmits: %d\n",
			msg->idiag_tcpinfo.tcpi_total_retrans);
	nl_dump(p, "]\n");

	if (msg->idiag_meminfo) {
		nl_dump(p, "meminfo:  [\n");
		nl_dump(p, "\trmem: %s\n",
				nl_size2str(msg->idiag_meminfo->idiag_rmem,
					    buf,
					    sizeof(buf)));
		nl_dump(p, "\twmem: %s\n",
				nl_size2str(msg->idiag_meminfo->idiag_wmem,
					    buf,
					    sizeof(buf)));
		nl_dump(p, "\tfmem: %s\n",
				nl_size2str(msg->idiag_meminfo->idiag_fmem,
					    buf,
					    sizeof(buf)));
		nl_dump(p, "\ttmem: %s\n",
				nl_size2str(msg->idiag_meminfo->idiag_tmem,
					    buf,
					    sizeof(buf)));
		nl_dump(p, "]\n");
	}

	if (msg->idiag_vegasinfo) {
		nl_dump(p, "vegasinfo:  [\n");
		nl_dump(p, "\tvegas enabled: %d\n",
				msg->idiag_vegasinfo->tcpv_enabled);
		if (msg->idiag_vegasinfo->tcpv_enabled) {
			nl_dump(p, "\trtt cnt: %d",
					msg->idiag_vegasinfo->tcpv_rttcnt);
			nl_dump(p, "\trtt (propagation delay): %d",
					msg->idiag_vegasinfo->tcpv_rtt);
			nl_dump(p, "\tmin rtt: %d",
					msg->idiag_vegasinfo->tcpv_minrtt);
		}
		nl_dump(p, "]\n");
	}

	nl_dump(p, "skmeminfo:  [\n");
	nl_dump(p, "\trmem alloc: %d\n",
			msg->idiag_skmeminfo[IDIAG_SK_MEMINFO_RMEM_ALLOC]);
	nl_dump(p, "\trcv buf: %s\n",
			nl_size2str(msg->idiag_skmeminfo[IDIAG_SK_MEMINFO_RCVBUF],
				buf, sizeof(buf)));
	nl_dump(p, "\twmem alloc: %d\n",
			msg->idiag_skmeminfo[IDIAG_SK_MEMINFO_WMEM_ALLOC]);
	nl_dump(p, "\tsnd buf: %s\n",
			nl_size2str(msg->idiag_skmeminfo[IDIAG_SK_MEMINFO_SNDBUF],
				buf, sizeof(buf)));
	nl_dump(p, "\tfwd alloc: %d\n",
			msg->idiag_skmeminfo[IDIAG_SK_MEMINFO_FWD_ALLOC]);
	nl_dump(p, "\twmem queued: %s\n",
			nl_size2str(msg->idiag_skmeminfo[IDIAG_SK_MEMINFO_WMEM_QUEUED],
				buf, sizeof(buf)));
	nl_dump(p, "\topt mem: %d\n",
			msg->idiag_skmeminfo[IDIAG_SK_MEMINFO_OPTMEM]);
	nl_dump(p, "\tbacklog: %d\n",
			msg->idiag_skmeminfo[IDIAG_SK_MEMINFO_BACKLOG]);
	nl_dump(p, "]\n\n");
}

static void idiagnl_msg_free(struct nl_object *a)
{
	struct idiagnl_msg *msg = (struct idiagnl_msg *) a;
	if (a == NULL)
		return;

	free(msg->idiag_cong);
	nl_addr_put(msg->idiag_src);
	nl_addr_put(msg->idiag_dst);
	idiagnl_meminfo_put(msg->idiag_meminfo);
	idiagnl_vegasinfo_put(msg->idiag_vegasinfo);
}

static int idiagnl_msg_clone(struct nl_object *_dst, struct nl_object *_src)
{
	struct idiagnl_msg *dst = (struct idiagnl_msg *) _dst;
	struct idiagnl_msg *src = (struct idiagnl_msg *) _src;

	if (src->idiag_src)
		if (!(dst->idiag_src = nl_addr_clone(src->idiag_src)))
			return -NLE_NOMEM;

	if (src->idiag_dst)
		if (!(dst->idiag_dst = nl_addr_clone(src->idiag_dst)))
			return -NLE_NOMEM;

	return 0;
}

static struct nla_policy ext_policy[IDIAG_ATTR_MAX] = {
	[IDIAG_ATTR_MEMINFO]    = { .minlen = sizeof(struct inet_diag_meminfo) },
	[IDIAG_ATTR_INFO]       = { .minlen = sizeof(struct tcp_info)	},
	[IDIAG_ATTR_VEGASINFO]  = { .minlen = sizeof(struct tcpvegas_info) },
	[IDIAG_ATTR_CONG]       = { .type = NLA_STRING },
	[IDIAG_ATTR_TOS]        = { .type = NLA_U8 },
	[IDIAG_ATTR_TCLASS]     = { .type = NLA_U8 },
	[IDIAG_ATTR_SKMEMINFO]  = { .minlen = (sizeof(uint32_t) * IDIAG_SK_MEMINFO_VARS)  },
	[IDIAG_ATTR_SHUTDOWN]   = { .type = NLA_U8 },
};

int idiagnl_msg_parse(struct nlmsghdr *nlh, struct idiagnl_msg **result)
{
	struct idiagnl_msg *msg = NULL;
	struct inet_diag_msg *raw_msg = NULL;
	struct nl_addr *src = NULL, *dst = NULL;
	struct nlattr *tb[IDIAG_ATTR_MAX];
	int err = 0;

	msg = idiagnl_msg_alloc();
	if (!msg)
		goto errout_nomem;

	err = nlmsg_parse(nlh, sizeof(struct inet_diag_msg), tb, IDIAG_ATTR_MAX,
			ext_policy);
	if (err < 0)
		goto errout;

	raw_msg = nlmsg_data(nlh);
	msg->idiag_family = raw_msg->idiag_family;
	msg->idiag_state = raw_msg->idiag_state;
	msg->idiag_timer = raw_msg->idiag_timer;
	msg->idiag_retrans = raw_msg->idiag_retrans;
	msg->idiag_expires = raw_msg->idiag_expires;
	msg->idiag_rqueue = raw_msg->idiag_rqueue;
	msg->idiag_wqueue = raw_msg->idiag_wqueue;
	msg->idiag_uid = raw_msg->idiag_uid;
	msg->idiag_inode = raw_msg->idiag_inode;
	msg->idiag_sport = raw_msg->id.idiag_sport;
	msg->idiag_dport = raw_msg->id.idiag_dport;
	msg->idiag_ifindex = raw_msg->id.idiag_if;

	dst = nl_addr_build(raw_msg->idiag_family, raw_msg->id.idiag_dst,
			sizeof(raw_msg->id.idiag_dst));
	if (!dst)
		goto errout_nomem;

	err = idiagnl_msg_set_dst(msg, dst);
	if (err < 0)
		goto errout;

	nl_addr_put(dst);

	src = nl_addr_build(raw_msg->idiag_family, raw_msg->id.idiag_src,
			sizeof(raw_msg->id.idiag_src));
	if (!src)
		goto errout_nomem;

	err = idiagnl_msg_set_src(msg, src);
	if (err < 0)
		goto errout;

	nl_addr_put(src);

	if (tb[IDIAG_ATTR_TOS])
		msg->idiag_tos = nla_get_u8(tb[IDIAG_ATTR_TOS]);

	if (tb[IDIAG_ATTR_TCLASS])
		msg->idiag_tclass = nla_get_u8(tb[IDIAG_ATTR_TCLASS]);

	if (tb[IDIAG_ATTR_SHUTDOWN])
		msg->idiag_shutdown = nla_get_u8(tb[IDIAG_ATTR_SHUTDOWN]);

	if (tb[IDIAG_ATTR_CONG])
		msg->idiag_cong = nla_strdup(tb[IDIAG_ATTR_CONG]);

	if (tb[IDIAG_ATTR_INFO])
		nla_memcpy(&msg->idiag_tcpinfo, tb[IDIAG_ATTR_INFO],
				sizeof(msg->idiag_tcpinfo));

	if (tb[IDIAG_ATTR_MEMINFO]) {
		struct idiagnl_meminfo *minfo = idiagnl_meminfo_alloc();
		struct inet_diag_meminfo *raw_minfo = NULL;

		if (!minfo)
			goto errout_nomem;

		raw_minfo = (struct inet_diag_meminfo *)
			nla_data(tb[IDIAG_ATTR_MEMINFO]);

		idiagnl_meminfo_set_rmem(minfo, raw_minfo->idiag_rmem);
		idiagnl_meminfo_set_wmem(minfo, raw_minfo->idiag_wmem);
		idiagnl_meminfo_set_fmem(minfo, raw_minfo->idiag_fmem);
		idiagnl_meminfo_set_tmem(minfo, raw_minfo->idiag_tmem);

		msg->idiag_meminfo = minfo;
	}

	if (tb[IDIAG_ATTR_VEGASINFO]) {
		struct idiagnl_vegasinfo *vinfo = idiagnl_vegasinfo_alloc();
		struct tcpvegas_info *raw_vinfo = NULL;

		if (!vinfo)
			goto errout_nomem;

		raw_vinfo = (struct tcpvegas_info *)
			nla_data(tb[IDIAG_ATTR_VEGASINFO]);

		idiagnl_vegasinfo_set_enabled(vinfo, raw_vinfo->tcpv_enabled);
		idiagnl_vegasinfo_set_rttcnt(vinfo, raw_vinfo->tcpv_rttcnt);
		idiagnl_vegasinfo_set_rtt(vinfo, raw_vinfo->tcpv_rtt);
		idiagnl_vegasinfo_set_minrtt(vinfo, raw_vinfo->tcpv_minrtt);

		msg->idiag_vegasinfo = vinfo;
	}

	if (tb[IDIAG_ATTR_SKMEMINFO])
		nla_memcpy(&msg->idiag_skmeminfo, tb[IDIAG_ATTR_SKMEMINFO],
				sizeof(msg->idiag_skmeminfo));

	*result = msg;
	return 0;

errout:
	idiagnl_msg_put(msg);
	return err;

errout_nomem:
	err = -NLE_NOMEM;
	goto errout;
}

/** @cond SKIP */
struct nl_object_ops idiagnl_msg_obj_ops = {
	.oo_name			 = "idiag/idiag_msg",
	.oo_size			 = sizeof(struct idiagnl_msg),
	.oo_free_data			 = idiagnl_msg_free,
	.oo_clone			 = idiagnl_msg_clone,
	.oo_dump			 = {
		[NL_DUMP_LINE]		 = idiag_msg_dump_line,
		[NL_DUMP_DETAILS]	 = idiag_msg_dump_details,
		[NL_DUMP_STATS]		 = idiag_msg_dump_stats,
	},
	.oo_attrs2str			= idiagnl_attrs2str,
	.oo_id_attrs			= (IDIAG_ATTR_INFO)
};
/** @endcond */

/** @} */
