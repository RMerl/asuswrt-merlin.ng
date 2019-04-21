/*
 * (C) 2012 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This code has been sponsored by Vyatta Inc. <http://www.vyatta.com>
 */
#include "internal.h"

#include <time.h>
#include <endian.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

#include <libmnl/libmnl.h>
#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nfnetlink_cttimeout.h>

#include <libnetfilter_cttimeout/libnetfilter_cttimeout.h>

static const char *const tcp_state_to_name[] = {
	[NFCT_TIMEOUT_ATTR_TCP_SYN_SENT]	= "SYN_SENT",
	[NFCT_TIMEOUT_ATTR_TCP_SYN_RECV]	= "SYN_RECV",
	[NFCT_TIMEOUT_ATTR_TCP_ESTABLISHED]	= "ESTABLISHED",
	[NFCT_TIMEOUT_ATTR_TCP_FIN_WAIT]	= "FIN_WAIT",
	[NFCT_TIMEOUT_ATTR_TCP_CLOSE_WAIT]	= "CLOSE_WAIT",
	[NFCT_TIMEOUT_ATTR_TCP_LAST_ACK]	= "LAST_ACK",
	[NFCT_TIMEOUT_ATTR_TCP_TIME_WAIT]	= "TIME_WAIT",
	[NFCT_TIMEOUT_ATTR_TCP_CLOSE]		= "CLOSE",
	[NFCT_TIMEOUT_ATTR_TCP_SYN_SENT2]	= "SYN_SENT2",
	[NFCT_TIMEOUT_ATTR_TCP_RETRANS]		= "RETRANS",
	[NFCT_TIMEOUT_ATTR_TCP_UNACK]		= "UNACKNOWLEDGED",
};

static const char *const generic_state_to_name[] = {
	[NFCT_TIMEOUT_ATTR_GENERIC]		= "TIMEOUT",
};

static const char *const udp_state_to_name[] = {
	[NFCT_TIMEOUT_ATTR_UDP_UNREPLIED]	= "UNREPLIED",
	[NFCT_TIMEOUT_ATTR_UDP_REPLIED]		= "REPLIED",
};

static const char *const sctp_state_to_name[] = {
	[NFCT_TIMEOUT_ATTR_SCTP_CLOSED]			= "CLOSED",
	[NFCT_TIMEOUT_ATTR_SCTP_COOKIE_WAIT]		= "COOKIE_WAIT",
	[NFCT_TIMEOUT_ATTR_SCTP_COOKIE_ECHOED]		= "COOKIE_ECHOED",
	[NFCT_TIMEOUT_ATTR_SCTP_ESTABLISHED]		= "ESTABLISHED",
	[NFCT_TIMEOUT_ATTR_SCTP_SHUTDOWN_SENT]		= "SHUTDOWN_SENT",
	[NFCT_TIMEOUT_ATTR_SCTP_SHUTDOWN_RECD]		= "SHUTDOWN_RECD",
	[NFCT_TIMEOUT_ATTR_SCTP_SHUTDOWN_ACK_SENT]	= "SHUTDOWN_ACK_SENT",
};

static const char *const dccp_state_to_name[] = {
	[NFCT_TIMEOUT_ATTR_DCCP_REQUEST]	= "REQUEST",
	[NFCT_TIMEOUT_ATTR_DCCP_RESPOND]	= "RESPOND",
	[NFCT_TIMEOUT_ATTR_DCCP_PARTOPEN]	= "PARTOPEN",
	[NFCT_TIMEOUT_ATTR_DCCP_OPEN]		= "OPEN",
	[NFCT_TIMEOUT_ATTR_DCCP_CLOSEREQ]	= "CLOSEREQ",
	[NFCT_TIMEOUT_ATTR_DCCP_CLOSING]	= "CLOSING",
	[NFCT_TIMEOUT_ATTR_DCCP_TIMEWAIT]	= "TIMEWAIT",
};

static const char *const icmp_state_to_name[] = {
	[NFCT_TIMEOUT_ATTR_ICMP]		= "TIMEOUT",
};

static const char *const icmpv6_state_to_name[] = {
	[NFCT_TIMEOUT_ATTR_ICMPV6]		= "TIMEOUT",
};

static struct {
	uint32_t nlattr_max;
	uint32_t attr_max;
	const char *const *state_to_name;
} timeout_protocol[IPPROTO_MAX] = {
	[IPPROTO_ICMP]	= {
		.nlattr_max	= __CTA_TIMEOUT_ICMP_MAX,
		.attr_max	= NFCT_TIMEOUT_ATTR_ICMP_MAX,
		.state_to_name	= icmp_state_to_name,
	},
	[IPPROTO_TCP]	= {
		.nlattr_max	= __CTA_TIMEOUT_TCP_MAX,
		.attr_max	= NFCT_TIMEOUT_ATTR_TCP_MAX,
		.state_to_name	= tcp_state_to_name,
	},
	[IPPROTO_UDP]	= {
		.nlattr_max	= __CTA_TIMEOUT_UDP_MAX,
		.attr_max	= NFCT_TIMEOUT_ATTR_UDP_MAX,
		.state_to_name	= udp_state_to_name,
	},
	[IPPROTO_GRE]	= {
		.nlattr_max	= __CTA_TIMEOUT_GRE_MAX,
		.attr_max	= NFCT_TIMEOUT_ATTR_GRE_MAX,
		.state_to_name	= udp_state_to_name,
	},
	[IPPROTO_SCTP]	= {
		.nlattr_max	= __CTA_TIMEOUT_SCTP_MAX,
		.attr_max	= NFCT_TIMEOUT_ATTR_SCTP_MAX,
		.state_to_name	= sctp_state_to_name,
	},
	[IPPROTO_DCCP]	= {
		.nlattr_max	= __CTA_TIMEOUT_DCCP_MAX,
		.attr_max	= NFCT_TIMEOUT_ATTR_DCCP_MAX,
		.state_to_name	= dccp_state_to_name,
	},
	[IPPROTO_UDPLITE]	= {
		.nlattr_max	= __CTA_TIMEOUT_UDPLITE_MAX,
		.attr_max	= NFCT_TIMEOUT_ATTR_UDPLITE_MAX,
		.state_to_name	= udp_state_to_name,
	},
	[IPPROTO_ICMPV6]	= {
		.nlattr_max	= __CTA_TIMEOUT_ICMPV6_MAX,
		.attr_max	= NFCT_TIMEOUT_ATTR_ICMPV6_MAX,
		.state_to_name	= icmpv6_state_to_name,
	},
	/* add your new supported protocol tracker here. */
	[IPPROTO_RAW]	= {
		.nlattr_max	= __CTA_TIMEOUT_GENERIC_MAX,
		.attr_max	= NFCT_TIMEOUT_ATTR_GENERIC_MAX,
		.state_to_name	= generic_state_to_name,
	},
};


struct nfct_timeout {
	char				name[32];	/* object name. */
	uint16_t			l3num;		/* AF_INET, ... */
	uint8_t				l4num;		/* UDP, TCP, ... */
	uint16_t			attrset;

	uint32_t			*timeout;	/* array of timeout. */
	uint16_t			polset;
};

/**
 * \mainpage
 *
 * libnetfilter_cttimeout is the userspace library that provides a programming
 * interface (API) to the in-kernel cttimeout infrastructure. This
 * infrastructure allows you to define fine-grain connection tracking timeout
 * policies that can be attached to traffic flows via iptables CT target.
 * Before the existence of this infrastructure, you could only set global
 * timeout policies per protocol. This library is currently used by the nfct
 * utility that is part of the conntrack-tools.
 *
 * libnetfilter_cttimeout homepage is:
 *      http://netfilter.org/projects/libnetfilter_cttimeout/
 *
 * \section Dependencies
 * libnetfilter_cttimeout requires libmnl and a kernel that includes the
 * nfnetlink_cttimeout subsystem (i.e. 3.4.0 or later).
 *
 * \section Main Features
 *  - listing/retrieving entries from the timeout policy table.
 *  - inserting/modifying/deleting entries from the timeout policy table.
 *
 * \section Git Tree
 * The current development version of libnetfilter_cttimeout can be accessed at
 * https://git.netfilter.org/cgi-bin/gitweb.cgi?p=libnetfilter_cttimeout.git
 *
 * \section Privileges
 * You need the CAP_NET_ADMIN capability in order to allow your application
 * to receive events from and to send commands to kernel-space, excepting
 * the timeout policy table dumping operation.
 *
 * \section Authors
 * libnetfilter_conntrack has been written by Pablo Neira Ayuso.
 */

/**
 * \defgroup nfcttimeout Timeout policy object handling
 * @{
 */

/**
 * nfct_timeout_alloc - allocate a new conntrack timeout object
 * \param protonum layer 4 protocol number (use IPPROTO_* constants)
 *
 * You can use IPPROTO_MAX to set the timeout for the generic protocol tracker.
 *
 * In case of success, this function returns a valid pointer, otherwise NULL
 * s returned and errno is appropriately set.
 */
struct nfct_timeout *nfct_timeout_alloc(void)
{
	struct nfct_timeout *t;

	t = calloc(1, sizeof(struct nfct_timeout));
	if (t == NULL)
		return NULL;

	return t;
}
EXPORT_SYMBOL(nfct_timeout_alloc);

/**
 * nfct_timeout_free - release one conntrack timeout object
 * \param t pointer to the conntrack timeout object
 */
void nfct_timeout_free(struct nfct_timeout *t)
{
	if (t->timeout)
		free(t->timeout);
	free(t);
}
EXPORT_SYMBOL(nfct_timeout_free);

/**
 * nfct_timeout_attr_set - set one attribute of the conntrack timeout object
 * \param t pointer to the conntrack timeout object
 * \param type attribute type you want to set
 * \param data pointer to data that will be used to set this attribute
 */
int
nfct_timeout_attr_set(struct nfct_timeout *t, uint32_t type, const void *data)
{
	switch(type) {
	case NFCT_TIMEOUT_ATTR_NAME:
		strncpy(t->name, data, sizeof(t->name));
		t->name[sizeof(t->name)-1] = '\0';
		break;
	case NFCT_TIMEOUT_ATTR_L3PROTO:
		t->l3num = *((uint16_t *) data);
		break;
	case NFCT_TIMEOUT_ATTR_L4PROTO:
		t->l4num = *((uint8_t *) data);
		break;
	/* NFCT_TIMEOUT_ATTR_POLICY is set by nfct_timeout_policy_attr_set. */
	}
	t->attrset |= (1 << type);
	return 0;
}
EXPORT_SYMBOL(nfct_timeout_attr_set);

/**
 * nfct_timeout_attr_set_u8 - set one attribute of the conntrack timeout object
 * \param t pointer to the conntrack timeout object
 * \param type attribute type you want to set
 * \param data pointer to data that will be used to set this attribute
 */
int
nfct_timeout_attr_set_u8(struct nfct_timeout *t, uint32_t type, uint8_t data)
{
	return nfct_timeout_attr_set(t, type, &data);
}
EXPORT_SYMBOL(nfct_timeout_attr_set_u8);

/**
 * nfct_timeout_attr_set_u16 - set one attribute of the conntrack timeout object
 * \param t pointer to the conntrack timeout object
 * \param type attribute type you want to set
 * \param data pointer to data that will be used to set this attribute
 */
int
nfct_timeout_attr_set_u16(struct nfct_timeout *t, uint32_t type, uint16_t data)
{
	return nfct_timeout_attr_set(t, type, &data);
}
EXPORT_SYMBOL(nfct_timeout_attr_set_u16);

/**
 * nfct_timeout_attr_unset - unset one attribute of the conntrack timeout object
 * \param t pointer to the conntrack timeout object
 * \param type attribute type you want to set
 */
void nfct_timeout_attr_unset(struct nfct_timeout *t, uint32_t type)
{
	t->attrset &= ~(1 << type);
}
EXPORT_SYMBOL(nfct_timeout_attr_unset);

/**
 * nfct_timeout_policy_attr_set_u32 - set one attribute of the policy
 * \param t pointer to the conntrack timeout object
 * \param type attribute type you want to set
 * \param data data that will be used to set this attribute
 */
int
nfct_timeout_policy_attr_set_u32(struct nfct_timeout *t,
				 uint32_t type, uint32_t data)
{
	size_t timeout_array_size;

	/* Layer 4 protocol needs to be already set. */
	if (!(t->attrset & (1 << NFCT_TIMEOUT_ATTR_L4PROTO)))
		return -1;

	if (t->timeout == NULL) {
		/* if not supported, default to generic protocol tracker. */
		if (timeout_protocol[t->l4num].attr_max != 0) {
			timeout_array_size =
				sizeof(uint32_t) *
					timeout_protocol[t->l4num].attr_max;
		} else {
			timeout_array_size =
				sizeof(uint32_t) *
					timeout_protocol[IPPROTO_RAW].attr_max;
		}
		t->timeout = calloc(1, timeout_array_size);
		if (t->timeout == NULL)
			return -1;
	}

	/* this state does not exists in this protocol tracker. */
	if (type > timeout_protocol[t->l4num].attr_max)
		return -1;

	t->timeout[type] = data;
	t->polset |= (1 << type);

	if (!(t->attrset & (1 << NFCT_TIMEOUT_ATTR_POLICY)))
		t->attrset |= (1 << NFCT_TIMEOUT_ATTR_POLICY);

	return 0;
}
EXPORT_SYMBOL(nfct_timeout_policy_attr_set_u32);

/**
 * nfct_timeout_policy_attr_unset - unset one attribute of the policy
 * \param t pointer to the conntrack timeout object
 * \param type attribute type you want to set
 */
void nfct_timeout_policy_attr_unset(struct nfct_timeout *t, uint32_t type)
{
	t->attrset &= ~(1 << type);
}
EXPORT_SYMBOL(nfct_timeout_policy_attr_unset);

/**
 * nfct_timeout_policy_attr_to_name - get state name from protocol state number
 * \param l4proto protocol, ie. IPPROTO_*
 * \param state state number that you want to get the state name
 *
 * This function returns NULL if unsupported protocol or state number is passed.
 * Otherwise, a pointer to valid string is returned.
 */
const char *nfct_timeout_policy_attr_to_name(uint8_t l4proto, uint32_t state)
{
	if (timeout_protocol[l4proto].state_to_name == NULL) {
		printf("no array state name\n");
		return NULL;
	}

	if (timeout_protocol[l4proto].state_to_name[state] == NULL) {
		printf("state %d does not exists\n", state);
		return NULL;
	}

	return timeout_protocol[l4proto].state_to_name[state];
}
EXPORT_SYMBOL(nfct_timeout_policy_attr_to_name);

/**
 * @}
 */

/**
 * \defgroup nfcttimeout_output Timeout policy object output
 * @{
 */

static int
nfct_timeout_snprintf_default(char *buf, size_t size,
			      const struct nfct_timeout *t,
			      unsigned int flags)
{
	int ret = 0;
	unsigned int offset = 0;

	if (t->attrset & (1 << NFCT_TIMEOUT_ATTR_NAME)) {
		ret = snprintf(buf+offset, size, ".%s = {\n", t->name);
		offset += ret;
		size -= ret;
	}
	if (t->attrset & (1 << NFCT_TIMEOUT_ATTR_L3PROTO)) {
		ret = snprintf(buf+offset, size, "\t.l3proto = %u,\n",
				t->l3num);
		offset += ret;
		size -= ret;
	}
	if (t->attrset & (1 << NFCT_TIMEOUT_ATTR_L4PROTO)) {
		ret = snprintf(buf+offset, size, "\t.l4proto = %u,\n",
				t->l4num);
		offset += ret;
		size -= ret;
	}
	if (t->attrset & (1 << NFCT_TIMEOUT_ATTR_POLICY)) {
		uint8_t l4num = t->l4num;
		int i;

		/* default to generic protocol tracker. */
		if (timeout_protocol[t->l4num].attr_max == 0)
			l4num = IPPROTO_RAW;

		ret = snprintf(buf+offset, size, "\t.policy = {\n");
		offset += ret;
		size -= ret;

		for (i=0; i<timeout_protocol[l4num].attr_max; i++) {
			const char *state_name =
				timeout_protocol[l4num].state_to_name[i][0] ?
				timeout_protocol[l4num].state_to_name[i] :
				"UNKNOWN";

			ret = snprintf(buf+offset, size,
				"\t\t.%s = %u,\n", state_name, t->timeout[i]);
			offset += ret;
			size -= ret;
		}

		ret = snprintf(buf+offset, size, "\t},\n");
		offset += ret;
		size -= ret;
	}
	ret = snprintf(buf+offset, size, "};");
	offset += ret;
	size -= ret;

	buf[offset]='\0';

	return ret;
}

/**
 * nfct_timeout_snprintf - print conntrack timeout object into one buffer
 * \param buf: pointer to buffer that is used to print the object
 * \param size: size of the buffer (or remaining room in it).
 * \param t: pointer to a valid conntrack timeout object.
 * \param type: output type (see NFCT_TIMEOUT_O_*)
 * \param flags: output flags (always set this to zero).
 *
 * This function returns -1 in case that some mandatory attributes are
 * missing. On sucess, it returns 0.
 */
int nfct_timeout_snprintf(char *buf, size_t size, const struct nfct_timeout *t,
			  unsigned int type, unsigned int flags)
{
	int ret = 0;

	switch(type) {
	case NFCT_TIMEOUT_O_DEFAULT:
		ret = nfct_timeout_snprintf_default(buf, size, t, flags);
		break;
	/* add your new output here. */
	default:
		break;
	}

	return ret;
}
EXPORT_SYMBOL(nfct_timeout_snprintf);

/**
 * @}
 */

/**
 * \defgroup nlmsg Netlink message helper functions
 * @{
 */

/**
 * nfct_timeout_nlmsg_build_hdr - build netlink message header for ct timeout
 * \param buf: buffer where this function outputs the netlink message.
 * \param cmd: nfct_timeout nfnetlink command.
 * \param flags: netlink flags.
 * \param seq: sequence number for this message.
 *
 * Possible commands:
 * - CTNL_MSG_TIMEOUT_NEW: new conntrack timeout object.
 * - CTNL_MSG_TIMEOUT_GET: get conntrack timeout object.
 * - CTNL_MSG_TIMEOUT_DEL: delete conntrack timeout object.
 */
struct nlmsghdr *
nfct_timeout_nlmsg_build_hdr(char *buf, uint8_t cmd,
			     uint16_t flags, uint32_t seq)
{
	struct nlmsghdr *nlh;
	struct nfgenmsg *nfh;

	nlh = mnl_nlmsg_put_header(buf);
	nlh->nlmsg_type = (NFNL_SUBSYS_CTNETLINK_TIMEOUT << 8) | cmd;
	nlh->nlmsg_flags = NLM_F_REQUEST | flags;
	nlh->nlmsg_seq = seq;

	nfh = mnl_nlmsg_put_extra_header(nlh, sizeof(struct nfgenmsg));
	nfh->nfgen_family = AF_UNSPEC;
	nfh->version = NFNETLINK_V0;
	nfh->res_id = 0;

	return nlh;
}
EXPORT_SYMBOL(nfct_timeout_nlmsg_build_hdr);

/**
 * nfct_timeout_nlmsg_build_payload - build payload from ct timeout object
 * \param nlh: netlink message that you want to use to add the payload.
 * \param t: pointer to a conntrack timeout object
 */
void
nfct_timeout_nlmsg_build_payload(struct nlmsghdr *nlh,
				 const struct nfct_timeout *t)
{
	int i;
	struct nlattr *nest;

	if (t->attrset & (1 << NFCT_TIMEOUT_ATTR_NAME))
		mnl_attr_put_strz(nlh, CTA_TIMEOUT_NAME, t->name);

	if (t->attrset & (1 << NFCT_TIMEOUT_ATTR_L3PROTO))
		mnl_attr_put_u16(nlh, CTA_TIMEOUT_L3PROTO, htons(t->l3num));

	if (t->attrset & (1 << NFCT_TIMEOUT_ATTR_L4PROTO))
		mnl_attr_put_u8(nlh, CTA_TIMEOUT_L4PROTO, t->l4num);

	if (t->attrset & (1 << NFCT_TIMEOUT_ATTR_POLICY) && t->polset) {
		nest = mnl_attr_nest_start(nlh, CTA_TIMEOUT_DATA);

		for (i=0; i<timeout_protocol[t->l4num].attr_max; i++) {
			if (t->polset & (1 << i)) {
				mnl_attr_put_u32(nlh, i+1,
						 htonl(t->timeout[i]));
			}
		}
		mnl_attr_nest_end(nlh, nest);
	}

}
EXPORT_SYMBOL(nfct_timeout_nlmsg_build_payload);

static int
timeout_nlmsg_parse_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	uint16_t type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, CTA_TIMEOUT_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case CTA_TIMEOUT_NAME:
		if (mnl_attr_validate(attr, MNL_TYPE_STRING) < 0) {
			perror("mnl_attr_validate");
			return MNL_CB_ERROR;
		}
		break;
	case CTA_TIMEOUT_L3PROTO:
		if (mnl_attr_validate(attr, MNL_TYPE_U16) < 0) {
			perror("mnl_attr_validate");
			return MNL_CB_ERROR;
		}
		break;
	case CTA_TIMEOUT_L4PROTO:
		if (mnl_attr_validate(attr, MNL_TYPE_U8) < 0) {
			perror("mnl_attr_validate");
			return MNL_CB_ERROR;
		}
		break;
	case CTA_TIMEOUT_DATA:
		if (mnl_attr_validate(attr, MNL_TYPE_NESTED) < 0) {
			perror("mnl_attr_validate");
			return MNL_CB_ERROR;
		}
		break;
	}
	tb[type] = attr;
	return MNL_CB_OK;
}

struct _container_policy_cb {
	unsigned int nlattr_max;
	void *tb;
};

static int
parse_timeout_attr_policy_cb(const struct nlattr *attr, void *data)
{
	struct _container_policy_cb *data_cb = data;
        const struct nlattr **tb = data_cb->tb;
	uint16_t type = mnl_attr_get_type(attr);

        if (mnl_attr_type_valid(attr, data_cb->nlattr_max) < 0)
                return MNL_CB_OK;

	if (type <= data_cb->nlattr_max) {
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0) {
			perror("mnl_attr_validate");
			return MNL_CB_ERROR;
		}
		tb[type] = attr;
	}
	return MNL_CB_OK;
}

static void
timeout_parse_attr_data(struct nfct_timeout *t, const struct nlattr *nest)
{
	unsigned int nlattr_max = timeout_protocol[t->l4num].nlattr_max;
	struct nlattr *tb[nlattr_max];
	struct _container_policy_cb cnt = {
		.nlattr_max = nlattr_max,
		.tb = tb,
	};
	unsigned int i;

	memset(tb, 0, sizeof(struct nlattr *) * nlattr_max);

	mnl_attr_parse_nested(nest, parse_timeout_attr_policy_cb, &cnt);

	for (i=1; i<nlattr_max; i++) {
		if (tb[i]) {
			nfct_timeout_policy_attr_set_u32(t, i-1,
				ntohl(mnl_attr_get_u32(tb[i])));
		}
	}
}

/**
 * nfct_timeout_nlmsg_parse_payload - set timeout object attributes from message
 * \param nlh: netlink message that you want to use to add the payload.
 * \param t: pointer to a conntrack timeout object
 *
 * This function returns -1 in case that some mandatory attributes are
 * missing. On sucess, it returns 0.
 */
int
nfct_timeout_nlmsg_parse_payload(const struct nlmsghdr *nlh,
				 struct nfct_timeout *t)
{
	struct nlattr *tb[CTA_TIMEOUT_MAX+1] = {};
	struct nfgenmsg *nfg = mnl_nlmsg_get_payload(nlh);

	mnl_attr_parse(nlh, sizeof(*nfg), timeout_nlmsg_parse_attr_cb, tb);
	if (tb[CTA_TIMEOUT_NAME]) {
		nfct_timeout_attr_set(t, NFCT_TIMEOUT_ATTR_NAME,
			mnl_attr_get_str(tb[CTA_TIMEOUT_NAME]));
	}
	if (tb[CTA_TIMEOUT_L3PROTO]) {
		nfct_timeout_attr_set_u16(t, NFCT_TIMEOUT_ATTR_L3PROTO,
			ntohs(mnl_attr_get_u16(tb[CTA_TIMEOUT_L3PROTO])));
	}
	if (tb[CTA_TIMEOUT_L4PROTO]) {
		nfct_timeout_attr_set_u8(t, NFCT_TIMEOUT_ATTR_L4PROTO,
			mnl_attr_get_u8(tb[CTA_TIMEOUT_L4PROTO]));
	}
	if (tb[CTA_TIMEOUT_DATA]) {
		timeout_parse_attr_data(t, tb[CTA_TIMEOUT_DATA]);
	}
	return 0;
}
EXPORT_SYMBOL(nfct_timeout_nlmsg_parse_payload);

/**
 * @}
 */
