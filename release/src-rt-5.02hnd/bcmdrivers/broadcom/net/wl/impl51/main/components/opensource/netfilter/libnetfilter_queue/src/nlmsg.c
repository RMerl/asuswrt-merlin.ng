/*
 * (C) 2012 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include <arpa/inet.h>
#include <time.h>
#include <endian.h>
#include <stdlib.h>
#include <string.h>

#include <libmnl/libmnl.h>

#ifndef __aligned_be64
#define __aligned_be64 __be64 __attribute__((aligned(8)))
#define __aligned_le64 __le64 __attribute__((aligned(8)))
#endif

#include <linux/netfilter/nfnetlink_queue.h>

#include <libnetfilter_queue/libnetfilter_queue.h>

#include "internal.h"

/**
 * \defgroup nfq_verd Verdict helpers
 * @{
 */

void nfq_nlmsg_verdict_put(struct nlmsghdr *nlh, int id, int verdict)
{
	struct nfqnl_msg_verdict_hdr vh = {
		.verdict	= htonl(verdict),
		.id		= htonl(id),
	};
	mnl_attr_put(nlh, NFQA_VERDICT_HDR, sizeof(vh), &vh);
}
EXPORT_SYMBOL(nfq_nlmsg_verdict_put);

void nfq_nlmsg_verdict_put_mark(struct nlmsghdr *nlh, uint32_t mark)
{
	mnl_attr_put_u32(nlh, NFQA_MARK, htonl(mark));
}
EXPORT_SYMBOL(nfq_nlmsg_verdict_put_mark);

void
nfq_nlmsg_verdict_put_pkt(struct nlmsghdr *nlh, const void *pkt, uint32_t plen)
{
	mnl_attr_put(nlh, NFQA_PAYLOAD, plen, pkt);
}
EXPORT_SYMBOL(nfq_nlmsg_verdict_put_pkt);

/**
 * @}
 */

/**
 * \defgroup nfq_cfg Config helpers
 * @{
 */

/**
 * nfq_nlmsg_cfg_build_request- build netlink config message
 * \param buf Buffer where netlink message is going to be written.
 * \param cfg Structure that contains the config parameters.
 * \param command nfqueue nfnetlink command.
 *
 * This function returns a pointer to the netlink message. If something goes
 * wrong it returns NULL.
 *
 * Possible commands are:
 *
 * - NFQNL_CFG_CMD_NONE: Do nothing. It can be useful to know if the queue
 *   subsystem is working.
 * - NFQNL_CFG_CMD_BIND: Binds the program to a specific queue.
 * - NFQNL_CFG_CMD_UNBIND: Unbinds the program to a specifiq queue.
 * - NFQNL_CFG_CMD_PF_BIND: Binds to process packets belonging to the given
 *   protocol family (ie. PF_INET, PF_INET6, etc).
 * - NFQNL_CFG_CMD_PF_UNBIND: Unbinds from processing packets belonging to the
 *   given protocol family.
 */
void nfq_nlmsg_cfg_put_cmd(struct nlmsghdr *nlh, uint16_t pf, uint8_t cmd)
{
	struct nfqnl_msg_config_cmd command = {
		.command = cmd,
		.pf = htons(pf),
	};
	mnl_attr_put(nlh, NFQA_CFG_CMD, sizeof(command), &command);
}
EXPORT_SYMBOL(nfq_nlmsg_cfg_put_cmd);

void nfq_nlmsg_cfg_put_params(struct nlmsghdr *nlh, uint8_t mode, int range)
{
	struct nfqnl_msg_config_params params = {
		.copy_range = htonl(range),
		.copy_mode = mode,
	};
	mnl_attr_put(nlh, NFQA_CFG_PARAMS, sizeof(params), &params);
}
EXPORT_SYMBOL(nfq_nlmsg_cfg_put_params);

void nfq_nlmsg_cfg_put_qmaxlen(struct nlmsghdr *nlh, uint32_t queue_maxlen)
{
	mnl_attr_put_u32(nlh, NFQA_CFG_QUEUE_MAXLEN, htonl(queue_maxlen));
}
EXPORT_SYMBOL(nfq_nlmsg_cfg_put_qmaxlen);

/**
 * @}
 */

/**
 * \defgroup nlmsg Netlink message helper functions
 * @{
 */

static int nfq_pkt_parse_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	/* skip unsupported attribute in user-space */
	if (mnl_attr_type_valid(attr, NFQA_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case NFQA_MARK:
	case NFQA_IFINDEX_INDEV:
	case NFQA_IFINDEX_OUTDEV:
	case NFQA_IFINDEX_PHYSINDEV:
	case NFQA_IFINDEX_PHYSOUTDEV:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			return MNL_CB_ERROR;
		break;
	case NFQA_TIMESTAMP:
		if (mnl_attr_validate2(attr, MNL_TYPE_UNSPEC,
		    sizeof(struct nfqnl_msg_packet_timestamp)) < 0) {
			return MNL_CB_ERROR;
		}
		break;
	case NFQA_HWADDR:
		if (mnl_attr_validate2(attr, MNL_TYPE_UNSPEC,
		    sizeof(struct nfqnl_msg_packet_hw)) < 0) {
			return MNL_CB_ERROR;
		}
		break;
	case NFQA_PAYLOAD:
		break;
	}
	tb[type] = attr;
	return MNL_CB_OK;
}

/**
 * nfq_pkt_parse - set packet attributes from netlink message
 * \param nlh netlink message that you want to read.
 * \param pkt pointer to the packet to set.
 *
 * This function returns MNL_CB_ERROR if any error occurs, or MNL_CB_OK on
 * success.
 */
int nfq_nlmsg_parse(const struct nlmsghdr *nlh, struct nlattr **attr)
{
	return mnl_attr_parse(nlh, sizeof(struct nfgenmsg),
			      nfq_pkt_parse_attr_cb, attr);
}
EXPORT_SYMBOL(nfq_nlmsg_parse);

/**
 * @}
 */
