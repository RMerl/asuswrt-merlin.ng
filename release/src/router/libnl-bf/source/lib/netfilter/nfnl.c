/*
 * lib/netfilter/nfnl.c		Netfilter Netlink
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 * Copyright (c) 2007 Philip Craig <philipc@snapgear.com>
 * Copyright (c) 2007 Secure Computing Corporation
 */

/**
 * @defgroup nfnl Netfilter Netlink
 *
 * @par Message Format
 * @code
 *  <------- NLMSG_ALIGN(hlen) ------> <---- NLMSG_ALIGN(len) --->
 * +----------------------------+- - -+- - - - - - - - - - -+- - -+
 * |           Header           | Pad |       Payload       | Pad |
 * |      struct nlmsghdr       |     |                     |     |
 * +----------------------------+- - -+- - - - - - - - - - -+- - -+
 * @endcode
 * @code
 *  <-------- NFNL_HDRLEN --------->
 * +--------------------------+- - -+------------+
 * | Netfilter Netlink Header | Pad | Attributes |
 * |    struct nfgenmsg       |     |            |
 * +--------------------------+- - -+------------+
 * nfnlmsg_attrdata(nfg, hdrlen)-----^
 * @endcode
 *
 * @par 1) Creating a new netfilter netlink message
 * @code
 * struct nl_msg *msg;
 *
 * // Create a new empty netlink message
 * msg = nlmsg_alloc();
 *
 * // Append the netlink and netfilter netlink message header
 * hdr = nfnlmsg_put(msg, PID, SEQ, SUBSYS, TYPE, NLM_F_ECHO,
 *                   FAMILY, RES_ID);
 *
 * // Append the attributes.
 * nla_put_u32(msg, 1, 0x10);
 *
 * // Message is ready to be sent.
 * nl_send_auto_complete(sk, msg);
 *
 * // All done? Free the message.
 * nlmsg_free(msg);
 * @endcode
 *
 * @par 2) Sending of trivial messages
 * @code
 * // For trivial messages not requiring any subsys specific header or
 * // attributes, nfnl_send_simple() may be used to send messages directly.
 * nfnl_send_simple(sk, SUBSYS, TYPE, 0, FAMILY, RES_ID);
 * @endcode
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/netfilter/nfnl.h>

/**
 * @name Socket Creating
 * @{
 */

/**
 * Create and connect netfilter netlink socket.
 * @arg sk		Netlink socket.
 *
 * Creates a NETLINK_NETFILTER netlink socket, binds the socket and
 * issues a connection attempt.
 *
 * @see nl_connect()
 *
 * @return 0 on success or a negative error code.
 */
int nfnl_connect(struct nl_sock *sk)
{
	return nl_connect(sk, NETLINK_NETFILTER);
}

/** @} */

/**
 * @name Sending
 * @{
 */

/**
 * Send trivial netfilter netlink message
 * @arg sk		Netlink socket.
 * @arg subsys_id	nfnetlink subsystem
 * @arg type		nfnetlink message type
 * @arg flags		message flags
 * @arg family		nfnetlink address family
 * @arg res_id		nfnetlink resource id
 *
 * @return Newly allocated netlink message or NULL.
 */
int nfnl_send_simple(struct nl_sock *sk, uint8_t subsys_id, uint8_t type,
		     int flags, uint8_t family, uint16_t res_id)
{
	struct nfgenmsg hdr = {
		.nfgen_family = family,
		.version = NFNETLINK_V0,
		.res_id = htons(res_id),
	};

	return nl_send_simple(sk, NFNLMSG_TYPE(subsys_id, type), flags,
			      &hdr, sizeof(hdr));
}

/** @} */

/**
 * @name Message Parsing
 * @{
 */

/**
 * Get netfilter subsystem id from message
 * @arg nlh	netlink messsage header
 */
uint8_t nfnlmsg_subsys(struct nlmsghdr *nlh)
{
	return NFNL_SUBSYS_ID(nlh->nlmsg_type);
}

/**
 * Get netfilter message type from message
 * @arg nlh	netlink messsage header
 */
uint8_t nfnlmsg_subtype(struct nlmsghdr *nlh)
{
	return NFNL_MSG_TYPE(nlh->nlmsg_type);
}

/**
 * Get netfilter family from message
 * @arg nlh	netlink messsage header
 */
uint8_t nfnlmsg_family(struct nlmsghdr *nlh)
{
	struct nfgenmsg *nfg = nlmsg_data(nlh);

	return nfg->nfgen_family;
}

/**
 * Get netfilter resource id from message
 * @arg nlh	netlink messsage header
 */
uint16_t nfnlmsg_res_id(struct nlmsghdr *nlh)
{
	struct nfgenmsg *nfg = nlmsg_data(nlh);

	return ntohs(nfg->res_id);
}

/** @} */

/**
 * @name Message Building
 * @{
 */

static int nfnlmsg_append(struct nl_msg *msg, uint8_t family, uint16_t res_id)
{
	struct nfgenmsg *nfg;

	nfg = nlmsg_reserve(msg, sizeof(*nfg), NLMSG_ALIGNTO);
	if (nfg == NULL)
		return -NLE_NOMEM;

	nfg->nfgen_family = family;
	nfg->version = NFNETLINK_V0;
	nfg->res_id = htons(res_id);
	NL_DBG(2, "msg %p: Added nfnetlink header family=%d res_id=%d\n",
	       msg, family, res_id);
	return 0;
}

/**
 * Allocate a new netfilter netlink message
 * @arg subsys_id	nfnetlink subsystem
 * @arg type		nfnetlink message type
 * @arg flags		message flags
 * @arg family		nfnetlink address family
 * @arg res_id		nfnetlink resource id
 *
 * @return Newly allocated netlink message or NULL.
 */
struct nl_msg *nfnlmsg_alloc_simple(uint8_t subsys_id, uint8_t type, int flags,
				    uint8_t family, uint16_t res_id)
{
	struct nl_msg *msg;

	msg = nlmsg_alloc_simple(NFNLMSG_TYPE(subsys_id, type), flags);
	if (msg == NULL)
		return NULL;

	if (nfnlmsg_append(msg, family, res_id) < 0)
		goto nla_put_failure;

	return msg;

nla_put_failure:
	nlmsg_free(msg);
	return NULL;
}

/**
 * Add netlink and netfilter netlink headers to netlink message
 * @arg msg		netlink message
 * @arg pid		netlink process id
 * @arg seq		sequence number of message
 * @arg subsys_id	nfnetlink subsystem
 * @arg type		nfnetlink message type
 * @arg flags		message flags
 * @arg family		nfnetlink address family
 * @arg res_id		nfnetlink resource id
 */
int nfnlmsg_put(struct nl_msg *msg, uint32_t pid, uint32_t seq,
		uint8_t subsys_id, uint8_t type, int flags, uint8_t family,
		uint16_t res_id)
{
	struct nlmsghdr *nlh;

	nlh = nlmsg_put(msg, pid, seq, NFNLMSG_TYPE(subsys_id, type), 0, flags);
	if (nlh == NULL)
		return -NLE_MSGSIZE;

	return nfnlmsg_append(msg, family, res_id);
}

/** @} */

/** @} */
