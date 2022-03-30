/*
 * lib/netfilter/queue_msg.c	Netfilter Queue Messages
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2007, 2008 Patrick McHardy <kaber@trash.net>
 * Copyright (c) 2010       Karl Hiramoto <karl@hiramoto.org>
 */

/**
 * @ingroup nfnl
 * @defgroup queue Queue
 * @brief
 * @{
 */

#include <sys/types.h>
#include <linux/netfilter/nfnetlink_queue.h>

#include <netlink-private/netlink.h>
#include <netlink/attr.h>
#include <netlink/netfilter/nfnl.h>
#include <netlink/netfilter/queue_msg.h>
#include <byteswap.h>

static struct nl_cache_ops nfnl_queue_msg_ops;

#if __BYTE_ORDER == __BIG_ENDIAN
static uint64_t ntohll(uint64_t x)
{
	return x;
}
#elif __BYTE_ORDER == __LITTLE_ENDIAN
static uint64_t ntohll(uint64_t x)
{
	return bswap_64(x);
}
#endif

static struct nla_policy queue_policy[NFQA_MAX+1] = {
	[NFQA_PACKET_HDR]		= {
		.minlen	= sizeof(struct nfqnl_msg_packet_hdr),
	},
	[NFQA_VERDICT_HDR]		= {
		.minlen	= sizeof(struct nfqnl_msg_verdict_hdr),
	},
	[NFQA_MARK]			= { .type = NLA_U32 },
	[NFQA_TIMESTAMP]		= {
		.minlen = sizeof(struct nfqnl_msg_packet_timestamp),
	},
	[NFQA_IFINDEX_INDEV]		= { .type = NLA_U32 },
	[NFQA_IFINDEX_OUTDEV]		= { .type = NLA_U32 },
	[NFQA_IFINDEX_PHYSINDEV]	= { .type = NLA_U32 },
	[NFQA_IFINDEX_PHYSOUTDEV]	= { .type = NLA_U32 },
	[NFQA_HWADDR]			= {
		.minlen	= sizeof(struct nfqnl_msg_packet_hw),
	},
};

int nfnlmsg_queue_msg_parse(struct nlmsghdr *nlh,
			    struct nfnl_queue_msg **result)
{
	struct nfnl_queue_msg *msg;
	struct nlattr *tb[NFQA_MAX+1];
	struct nlattr *attr;
	int err;

	msg = nfnl_queue_msg_alloc();
	if (!msg)
		return -NLE_NOMEM;

	msg->ce_msgtype = nlh->nlmsg_type;

	err = nlmsg_parse(nlh, sizeof(struct nfgenmsg), tb, NFQA_MAX,
			  queue_policy);
	if (err < 0)
		goto errout;

	nfnl_queue_msg_set_group(msg, nfnlmsg_res_id(nlh));
	nfnl_queue_msg_set_family(msg, nfnlmsg_family(nlh));

	attr = tb[NFQA_PACKET_HDR];
	if (attr) {
		struct nfqnl_msg_packet_hdr *hdr = nla_data(attr);

		nfnl_queue_msg_set_packetid(msg, ntohl(hdr->packet_id));
		if (hdr->hw_protocol)
			nfnl_queue_msg_set_hwproto(msg, hdr->hw_protocol);
		nfnl_queue_msg_set_hook(msg, hdr->hook);
	}

	attr = tb[NFQA_MARK];
	if (attr)
		nfnl_queue_msg_set_mark(msg, ntohl(nla_get_u32(attr)));

	attr = tb[NFQA_TIMESTAMP];
	if (attr) {
		struct nfqnl_msg_packet_timestamp *timestamp = nla_data(attr);
		struct timeval tv;

		tv.tv_sec = ntohll(timestamp->sec);
		tv.tv_usec = ntohll(timestamp->usec);
		nfnl_queue_msg_set_timestamp(msg, &tv);
	}

	attr = tb[NFQA_IFINDEX_INDEV];
	if (attr)
		nfnl_queue_msg_set_indev(msg, ntohl(nla_get_u32(attr)));

	attr = tb[NFQA_IFINDEX_OUTDEV];
	if (attr)
		nfnl_queue_msg_set_outdev(msg, ntohl(nla_get_u32(attr)));

	attr = tb[NFQA_IFINDEX_PHYSINDEV];
	if (attr)
		nfnl_queue_msg_set_physindev(msg, ntohl(nla_get_u32(attr)));

	attr = tb[NFQA_IFINDEX_PHYSOUTDEV];
	if (attr)
		nfnl_queue_msg_set_physoutdev(msg, ntohl(nla_get_u32(attr)));

	attr = tb[NFQA_HWADDR];
	if (attr) {
		struct nfqnl_msg_packet_hw *hw = nla_data(attr);

		nfnl_queue_msg_set_hwaddr(msg, hw->hw_addr,
					  ntohs(hw->hw_addrlen));
	}

	attr = tb[NFQA_PAYLOAD];
	if (attr) {
		err = nfnl_queue_msg_set_payload(msg, nla_data(attr),
						 nla_len(attr));
		if (err < 0)
			goto errout;
	}

	*result = msg;
	return 0;

errout:
	nfnl_queue_msg_put(msg);
	return err;
}

static int queue_msg_parser(struct nl_cache_ops *ops, struct sockaddr_nl *who,
			    struct nlmsghdr *nlh, struct nl_parser_param *pp)
{
	struct nfnl_queue_msg *msg;
	int err;

	if ((err = nfnlmsg_queue_msg_parse(nlh, &msg)) < 0)
		return err;

	err = pp->pp_cb((struct nl_object *) msg, pp);
	nfnl_queue_msg_put(msg);
	return err;
}

/** @} */

static struct nl_msg *
__nfnl_queue_msg_build_verdict(const struct nfnl_queue_msg *msg,
							   uint8_t type)
{
	struct nl_msg *nlmsg;
	struct nfqnl_msg_verdict_hdr verdict;

	nlmsg = nfnlmsg_alloc_simple(NFNL_SUBSYS_QUEUE, type, 0,
				     nfnl_queue_msg_get_family(msg),
				     nfnl_queue_msg_get_group(msg));
	if (nlmsg == NULL)
		return NULL;

	verdict.id = htonl(nfnl_queue_msg_get_packetid(msg));
	verdict.verdict = htonl(nfnl_queue_msg_get_verdict(msg));
	if (nla_put(nlmsg, NFQA_VERDICT_HDR, sizeof(verdict), &verdict) < 0)
		goto nla_put_failure;

	if (nfnl_queue_msg_test_mark(msg) &&
	    nla_put_u32(nlmsg, NFQA_MARK,
			ntohl(nfnl_queue_msg_get_mark(msg))) < 0)
		goto nla_put_failure;

	return nlmsg;

nla_put_failure:
	nlmsg_free(nlmsg);
	return NULL;
}

struct nl_msg *
nfnl_queue_msg_build_verdict(const struct nfnl_queue_msg *msg)
{
	return __nfnl_queue_msg_build_verdict(msg, NFQNL_MSG_VERDICT);
}

struct nl_msg *
nfnl_queue_msg_build_verdict_batch(const struct nfnl_queue_msg *msg)
{
	return __nfnl_queue_msg_build_verdict(msg, NFQNL_MSG_VERDICT_BATCH);
}

/**
* Send a message verdict/mark
* @arg nlh            netlink messsage header
* @arg msg            queue msg
* @return 0 on OK or error code
*/
int nfnl_queue_msg_send_verdict(struct nl_sock *nlh,
				const struct nfnl_queue_msg *msg)
{
	struct nl_msg *nlmsg;
	int err;

	nlmsg = nfnl_queue_msg_build_verdict(msg);
	if (nlmsg == NULL)
		return -NLE_NOMEM;

	err = nl_send_auto_complete(nlh, nlmsg);
	nlmsg_free(nlmsg);
	if (err < 0)
		return err;
	return wait_for_ack(nlh);
}

/**
* Send a message batched verdict/mark
* @arg nlh            netlink messsage header
* @arg msg            queue msg
* @return 0 on OK or error code
*/
int nfnl_queue_msg_send_verdict_batch(struct nl_sock *nlh,
									  const struct nfnl_queue_msg *msg)
{
	struct nl_msg *nlmsg;
	int err;

	nlmsg = nfnl_queue_msg_build_verdict_batch(msg);
	if (nlmsg == NULL)
		return -NLE_NOMEM;

	err = nl_send_auto_complete(nlh, nlmsg);
	nlmsg_free(nlmsg);
	if (err < 0)
		return err;
	return wait_for_ack(nlh);
}

/**
* Send a message verdict including the payload
* @arg nlh            netlink messsage header
* @arg msg            queue msg
* @arg payload_data   packet payload data
* @arg payload_len    payload length
* @return 0 on OK or error code
*/
int nfnl_queue_msg_send_verdict_payload(struct nl_sock *nlh,
				const struct nfnl_queue_msg *msg,
				const void *payload_data, unsigned payload_len)
{
	struct nl_msg *nlmsg;
	int err;
	struct iovec iov[3];
	struct nlattr nla;

	nlmsg = nfnl_queue_msg_build_verdict(msg);
	if (nlmsg == NULL)
		return -NLE_NOMEM;

	memset(iov, 0, sizeof(iov));

	iov[0].iov_base = (void *) nlmsg_hdr(nlmsg);
	iov[0].iov_len = nlmsg_hdr(nlmsg)->nlmsg_len;

	nla.nla_type = NFQA_PAYLOAD;
	nla.nla_len = payload_len + sizeof(nla);
	nlmsg_hdr(nlmsg)->nlmsg_len += nla.nla_len;

	iov[1].iov_base = (void *) &nla;
	iov[1].iov_len = sizeof(nla);

	iov[2].iov_base = (void *) payload_data;
	iov[2].iov_len = NLA_ALIGN(payload_len);

	nl_complete_msg(nlh, nlmsg);
	err = nl_send_iovec(nlh, nlmsg, iov, 3);

	nlmsg_free(nlmsg);
	if (err < 0)
		return err;
	return wait_for_ack(nlh);
}

#define NFNLMSG_QUEUE_TYPE(type) NFNLMSG_TYPE(NFNL_SUBSYS_QUEUE, (type))
static struct nl_cache_ops nfnl_queue_msg_ops = {
	.co_name		= "netfilter/queue_msg",
	.co_hdrsize		= NFNL_HDRLEN,
	.co_msgtypes		= {
		{ NFNLMSG_QUEUE_TYPE(NFQNL_MSG_PACKET), NL_ACT_NEW, "new" },
		END_OF_MSGTYPES_LIST,
	},
	.co_protocol		= NETLINK_NETFILTER,
	.co_msg_parser		= queue_msg_parser,
	.co_obj_ops		= &queue_msg_obj_ops,
};

static void __init nfnl_msg_queue_init(void)
{
	nl_cache_mngt_register(&nfnl_queue_msg_ops);
}

static void __exit nfnl_queue_msg_exit(void)
{
	nl_cache_mngt_unregister(&nfnl_queue_msg_ops);
}

/** @} */
