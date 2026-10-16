/*
 * (C) 2012 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This code has been sponsored by Sophos Astaro <http://www.sophos.com>
 */

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <errno.h>
#include <netdb.h>	/* getprotobynumber */
#include <time.h>
#include <stdarg.h>
#include <inttypes.h>
#include <assert.h>

#include <xtables.h>
#include <libiptc/libxtc.h>
#include <libiptc/xtcshared.h>

#include <stdlib.h>
#include <string.h>

#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv6/ip6_tables.h>
#include <netinet/ip6.h>

#include <linux/netlink.h>
#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nf_tables.h>
#include <linux/netfilter/nf_tables_compat.h>

#include <linux/netfilter/xt_limit.h>
#include <linux/netfilter/xt_NFLOG.h>
#include <linux/netfilter/xt_mark.h>

#include <libmnl/libmnl.h>
#include <libnftnl/gen.h>
#include <libnftnl/table.h>
#include <libnftnl/chain.h>
#include <libnftnl/rule.h>
#include <libnftnl/expr.h>
#include <libnftnl/set.h>
#include <libnftnl/udata.h>
#include <libnftnl/batch.h>

#include <netinet/in.h>	/* inet_ntoa */
#include <arpa/inet.h>

#include "nft.h"
#include "xshared.h" /* proto_to_name */
#include "nft-cache.h"
#include "nft-shared.h"
#include "nft-bridge.h" /* EBT_NOPROTO */

static void *nft_fn;

int mnl_talk(struct nft_handle *h, struct nlmsghdr *nlh,
	     int (*cb)(const struct nlmsghdr *nlh, void *data),
	     void *data)
{
	int ret;
	char buf[32768];

	if (mnl_socket_sendto(h->nl, nlh, nlh->nlmsg_len) < 0)
		return -1;

	ret = mnl_socket_recvfrom(h->nl, buf, sizeof(buf));
	while (ret > 0) {
		ret = mnl_cb_run(buf, ret, h->seq, h->portid, cb, data);
		if (ret <= 0)
			break;

		ret = mnl_socket_recvfrom(h->nl, buf, sizeof(buf));
	}
	if (ret == -1) {
		return -1;
	}

	return 0;
}

#define NFT_NLMSG_MAXSIZE (UINT16_MAX + getpagesize())

/* Selected batch page is 2 Mbytes long to support loading a ruleset of 3.5M
 * rules matching on source and destination address as well as input and output
 * interfaces. This is what legacy iptables supports.
 */
#define BATCH_PAGE_SIZE 2 * 1024 * 1024

static struct nftnl_batch *mnl_batch_init(void)
{
	struct nftnl_batch *batch;

	batch = nftnl_batch_alloc(BATCH_PAGE_SIZE, NFT_NLMSG_MAXSIZE);
	if (batch == NULL)
		return NULL;

	return batch;
}

static void mnl_nft_batch_continue(struct nftnl_batch *batch)
{
	int ret = nftnl_batch_update(batch);

	assert(ret >= 0);
}

static uint32_t mnl_batch_begin(struct nftnl_batch *batch, uint32_t genid, uint32_t seqnum)
{
	struct nlmsghdr *nlh;

	nlh = nftnl_batch_begin(nftnl_batch_buffer(batch), seqnum);

	mnl_attr_put_u32(nlh, NFTA_GEN_ID, htonl(genid));

	mnl_nft_batch_continue(batch);

	return seqnum;
}

static void mnl_batch_end(struct nftnl_batch *batch, uint32_t seqnum)
{
	nftnl_batch_end(nftnl_batch_buffer(batch), seqnum);
	mnl_nft_batch_continue(batch);
}

static void mnl_batch_reset(struct nftnl_batch *batch)
{
	nftnl_batch_free(batch);
}

struct mnl_err {
	struct list_head	head;
	int			err;
	uint32_t		seqnum;
};

static void mnl_err_list_node_add(struct list_head *err_list, int error,
				  int seqnum)
{
	struct mnl_err *err = xtables_malloc(sizeof(struct mnl_err));

	err->seqnum = seqnum;
	err->err = error;
	list_add_tail(&err->head, err_list);
}

static void mnl_err_list_free(struct mnl_err *err)
{
	list_del(&err->head);
	free(err);
}

static void mnl_set_sndbuffer(struct nft_handle *h)
{
	int newbuffsiz = nftnl_batch_iovec_len(h->batch) * BATCH_PAGE_SIZE;

	if (newbuffsiz <= h->nlsndbuffsiz)
		return;

	/* Rise sender buffer length to avoid hitting -EMSGSIZE */
	if (setsockopt(mnl_socket_get_fd(h->nl), SOL_SOCKET, SO_SNDBUFFORCE,
		       &newbuffsiz, sizeof(socklen_t)) < 0)
		return;

	h->nlsndbuffsiz = newbuffsiz;
}

static void mnl_set_rcvbuffer(struct nft_handle *h, int numcmds)
{
	int newbuffsiz = getpagesize() * numcmds;

	if (newbuffsiz <= h->nlrcvbuffsiz)
		return;

	/* Rise receiver buffer length to avoid hitting -ENOBUFS */
	if (setsockopt(mnl_socket_get_fd(h->nl), SOL_SOCKET, SO_RCVBUFFORCE,
		       &newbuffsiz, sizeof(socklen_t)) < 0)
		return;

	h->nlrcvbuffsiz = newbuffsiz;
}

static ssize_t mnl_nft_socket_sendmsg(struct nft_handle *h, int numcmds)
{
	static const struct sockaddr_nl snl = {
		.nl_family = AF_NETLINK
	};
	uint32_t iov_len = nftnl_batch_iovec_len(h->batch);
	struct iovec iov[iov_len];
	struct msghdr msg = {
		.msg_name	= (struct sockaddr *) &snl,
		.msg_namelen	= sizeof(snl),
		.msg_iov	= iov,
		.msg_iovlen	= iov_len,
	};

	mnl_set_sndbuffer(h);
	mnl_set_rcvbuffer(h, numcmds);
	nftnl_batch_iovec(h->batch, iov, iov_len);

	return sendmsg(mnl_socket_get_fd(h->nl), &msg, 0);
}

static int mnl_batch_talk(struct nft_handle *h, int numcmds)
{
	const struct mnl_socket *nl = h->nl;
	int ret, fd = mnl_socket_get_fd(nl), portid = mnl_socket_get_portid(nl);
	char rcv_buf[MNL_SOCKET_BUFFER_SIZE];
	fd_set readfds;
	struct timeval tv = {
		.tv_sec		= 0,
		.tv_usec	= 0
	};
	int err = 0;

	ret = mnl_nft_socket_sendmsg(h, numcmds);
	if (ret == -1) {
		fprintf(stderr, "sendmsg() failed: %s\n", strerror(errno));
		return -1;
	}

	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);

	/* receive and digest all the acknowledgments from the kernel. */
	ret = select(fd+1, &readfds, NULL, NULL, &tv);
	if (ret == -1)
		return -1;

	while (ret > 0 && FD_ISSET(fd, &readfds)) {
		struct nlmsghdr *nlh = (struct nlmsghdr *)rcv_buf;

		ret = mnl_socket_recvfrom(nl, rcv_buf, sizeof(rcv_buf));
		if (ret == -1)
			return -1;

		ret = mnl_cb_run(rcv_buf, ret, 0, portid, NULL, NULL);
		/* Continue on error, make sure we get all acknowledgments */
		if (ret == -1) {
			mnl_err_list_node_add(&h->err_list, errno,
					      nlh->nlmsg_seq);
			err = -1;
		}

		ret = select(fd+1, &readfds, NULL, NULL, &tv);
		if (ret == -1)
			return -1;

		FD_ZERO(&readfds);
		FD_SET(fd, &readfds);
	}
	return err;
}

enum obj_action {
	NFT_COMPAT_COMMIT,
	NFT_COMPAT_ABORT,
};

struct obj_update {
	struct list_head	head;
	enum obj_update_type	type:8;
	uint8_t			skip:1;
	unsigned int		seq;
	union {
		struct nftnl_table	*table;
		struct nftnl_chain	*chain;
		struct nftnl_rule	*rule;
		struct nftnl_set	*set;
		void			*ptr;
	};
	struct {
		unsigned int		lineno;
	} error;
};

static int mnl_append_error(const struct nft_handle *h,
			    const struct obj_update *o,
			    const struct mnl_err *err,
			    char *buf, unsigned int len)
{
	static const char *type_name[] = {
		[NFT_COMPAT_TABLE_ADD] = "TABLE_ADD",
		[NFT_COMPAT_TABLE_FLUSH] = "TABLE_FLUSH",
		[NFT_COMPAT_CHAIN_ADD] = "CHAIN_ADD",
		[NFT_COMPAT_CHAIN_USER_ADD] = "CHAIN_USER_ADD",
		[NFT_COMPAT_CHAIN_DEL] = "CHAIN_DEL",
		[NFT_COMPAT_CHAIN_USER_FLUSH] = "CHAIN_USER_FLUSH",
		[NFT_COMPAT_CHAIN_UPDATE] = "CHAIN_UPDATE",
		[NFT_COMPAT_CHAIN_RENAME] = "CHAIN_RENAME",
		[NFT_COMPAT_CHAIN_ZERO] = "CHAIN_ZERO",
		[NFT_COMPAT_RULE_APPEND] = "RULE_APPEND",
		[NFT_COMPAT_RULE_INSERT] = "RULE_INSERT",
		[NFT_COMPAT_RULE_REPLACE] = "RULE_REPLACE",
		[NFT_COMPAT_RULE_DELETE] = "RULE_DELETE",
		[NFT_COMPAT_RULE_FLUSH] = "RULE_FLUSH",
		[NFT_COMPAT_SET_ADD] = "SET_ADD",
	};
	char errmsg[256];
	char tcr[128];

	if (o->error.lineno)
		snprintf(errmsg, sizeof(errmsg), "\nline %u: %s failed (%s)",
			 o->error.lineno, type_name[o->type], strerror(err->err));
	else
		snprintf(errmsg, sizeof(errmsg), " %s failed (%s)",
			 type_name[o->type], strerror(err->err));

	switch (o->type) {
	case NFT_COMPAT_TABLE_ADD:
	case NFT_COMPAT_TABLE_FLUSH:
		snprintf(tcr, sizeof(tcr), "table %s",
			 nftnl_table_get_str(o->table, NFTNL_TABLE_NAME));
		break;
	case NFT_COMPAT_CHAIN_ADD:
	case NFT_COMPAT_CHAIN_ZERO:
	case NFT_COMPAT_CHAIN_USER_ADD:
	case NFT_COMPAT_CHAIN_DEL:
	case NFT_COMPAT_CHAIN_USER_FLUSH:
	case NFT_COMPAT_CHAIN_UPDATE:
	case NFT_COMPAT_CHAIN_RENAME:
		snprintf(tcr, sizeof(tcr), "chain %s",
			 nftnl_chain_get_str(o->chain, NFTNL_CHAIN_NAME));
		break;
	case NFT_COMPAT_RULE_APPEND:
	case NFT_COMPAT_RULE_INSERT:
	case NFT_COMPAT_RULE_REPLACE:
	case NFT_COMPAT_RULE_DELETE:
	case NFT_COMPAT_RULE_FLUSH:
	case NFT_COMPAT_RULE_CHANGE_COUNTERS:
		snprintf(tcr, sizeof(tcr), "rule in chain %s",
			 nftnl_rule_get_str(o->rule, NFTNL_RULE_CHAIN));
#if 0
		{
			nft_rule_print_save(h, o->rule, NFT_RULE_APPEND, FMT_NOCOUNTS);
		}
#endif
		break;
	case NFT_COMPAT_SET_ADD:
		snprintf(tcr, sizeof(tcr), "set %s",
			 nftnl_set_get_str(o->set, NFTNL_SET_NAME));
		break;
	case NFT_COMPAT_RULE_LIST:
	case NFT_COMPAT_RULE_CHECK:
	case NFT_COMPAT_CHAIN_RESTORE:
	case NFT_COMPAT_RULE_SAVE:
	case NFT_COMPAT_RULE_ZERO:
	case NFT_COMPAT_BRIDGE_USER_CHAIN_UPDATE:
		assert(0);
		break;
	}

	return snprintf(buf, len, "%s: %s", errmsg, tcr);
}

static struct obj_update *batch_add(struct nft_handle *h, enum obj_update_type type, void *ptr)
{
	struct obj_update *obj;

	obj = xtables_calloc(1, sizeof(struct obj_update));
	obj->ptr = ptr;
	obj->error.lineno = h->error.lineno;
	obj->type = type;
	list_add_tail(&obj->head, &h->obj_list);
	h->obj_list_num++;

	return obj;
}

static struct obj_update *
batch_table_add(struct nft_handle *h, enum obj_update_type type,
		struct nftnl_table *t)
{
	return batch_add(h, type, t);
}

static struct obj_update *
batch_set_add(struct nft_handle *h, enum obj_update_type type,
	      struct nftnl_set *s)
{
	return batch_add(h, type, s);
}

static struct obj_update *
batch_chain_add(struct nft_handle *h, enum obj_update_type type,
			   struct nftnl_chain *c)
{
	return batch_add(h, type, c);
}

static struct obj_update *
batch_rule_add(struct nft_handle *h, enum obj_update_type type,
			  struct nftnl_rule *r)
{
	return batch_add(h, type, r);
}

static void batch_obj_del(struct nft_handle *h, struct obj_update *o);

static void batch_chain_flush(struct nft_handle *h,
			      const char *table, const char *chain)
{
	struct obj_update *obj, *tmp;

	list_for_each_entry_safe(obj, tmp, &h->obj_list, head) {
		struct nftnl_rule *r = obj->ptr;

		switch (obj->type) {
		case NFT_COMPAT_RULE_APPEND:
		case NFT_COMPAT_RULE_INSERT:
		case NFT_COMPAT_RULE_REPLACE:
		case NFT_COMPAT_RULE_DELETE:
			break;
		default:
			continue;
		}

		if (table &&
		    strcmp(table, nftnl_rule_get_str(r, NFTNL_RULE_TABLE)))
			continue;

		if (chain &&
		    strcmp(chain, nftnl_rule_get_str(r, NFTNL_RULE_CHAIN)))
			continue;

		batch_obj_del(h, obj);
	}
}

static const struct builtin_table xtables_ipv4[NFT_TABLE_MAX] = {
	[NFT_TABLE_RAW] = {
		.name	= "raw",
		.type	= NFT_TABLE_RAW,
		.chains = {
			{
				.name	= "PREROUTING",
				.type	= "filter",
				.prio	= -300,	/* NF_IP_PRI_RAW */
				.hook	= NF_INET_PRE_ROUTING,
			},
			{
				.name	= "OUTPUT",
				.type	= "filter",
				.prio	= -300,	/* NF_IP_PRI_RAW */
				.hook	= NF_INET_LOCAL_OUT,
			},
		},
	},
	[NFT_TABLE_MANGLE] = {
		.name	= "mangle",
		.type	= NFT_TABLE_MANGLE,
		.chains = {
			{
				.name	= "PREROUTING",
				.type	= "filter",
				.prio	= -150,	/* NF_IP_PRI_MANGLE */
				.hook	= NF_INET_PRE_ROUTING,
			},
			{
				.name	= "INPUT",
				.type	= "filter",
				.prio	= -150,	/* NF_IP_PRI_MANGLE */
				.hook	= NF_INET_LOCAL_IN,
			},
			{
				.name	= "FORWARD",
				.type	= "filter",
				.prio	= -150,	/* NF_IP_PRI_MANGLE */
				.hook	= NF_INET_FORWARD,
			},
			{
				.name	= "OUTPUT",
				.type	= "route",
				.prio	= -150,	/* NF_IP_PRI_MANGLE */
				.hook	= NF_INET_LOCAL_OUT,
			},
			{
				.name	= "POSTROUTING",
				.type	= "filter",
				.prio	= -150,	/* NF_IP_PRI_MANGLE */
				.hook	= NF_INET_POST_ROUTING,
			},
		},
	},
	[NFT_TABLE_FILTER] = {
		.name	= "filter",
		.type	= NFT_TABLE_FILTER,
		.chains = {
			{
				.name	= "INPUT",
				.type	= "filter",
				.prio	= 0,	/* NF_IP_PRI_FILTER */
				.hook	= NF_INET_LOCAL_IN,
			},
			{
				.name	= "FORWARD",
				.type	= "filter",
				.prio	= 0,	/* NF_IP_PRI_FILTER */
				.hook	= NF_INET_FORWARD,
			},
			{
				.name	= "OUTPUT",
				.type	= "filter",
				.prio	= 0,	/* NF_IP_PRI_FILTER */
				.hook	= NF_INET_LOCAL_OUT,
			},
		},
	},
	[NFT_TABLE_SECURITY] = {
		.name	= "security",
		.type	= NFT_TABLE_SECURITY,
		.chains = {
			{
				.name	= "INPUT",
				.type	= "filter",
				.prio	= 150,	/* NF_IP_PRI_SECURITY */
				.hook	= NF_INET_LOCAL_IN,
			},
			{
				.name	= "FORWARD",
				.type	= "filter",
				.prio	= 150,	/* NF_IP_PRI_SECURITY */
				.hook	= NF_INET_FORWARD,
			},
			{
				.name	= "OUTPUT",
				.type	= "filter",
				.prio	= 150,	/* NF_IP_PRI_SECURITY */
				.hook	= NF_INET_LOCAL_OUT,
			},
		},
	},
	[NFT_TABLE_NAT] = {
		.name	= "nat",
		.type	= NFT_TABLE_NAT,
		.chains = {
			{
				.name	= "PREROUTING",
				.type	= "nat",
				.prio	= -100, /* NF_IP_PRI_NAT_DST */
				.hook	= NF_INET_PRE_ROUTING,
			},
			{
				.name	= "INPUT",
				.type	= "nat",
				.prio	= 100, /* NF_IP_PRI_NAT_SRC */
				.hook	= NF_INET_LOCAL_IN,
			},
			{
				.name	= "POSTROUTING",
				.type	= "nat",
				.prio	= 100, /* NF_IP_PRI_NAT_SRC */
				.hook	= NF_INET_POST_ROUTING,
			},
			{
				.name	= "OUTPUT",
				.type	= "nat",
				.prio	= -100, /* NF_IP_PRI_NAT_DST */
				.hook	= NF_INET_LOCAL_OUT,
			},
		},
	},
};

#include <linux/netfilter_arp.h>

static const struct builtin_table xtables_arp[NFT_TABLE_MAX] = {
	[NFT_TABLE_FILTER] = {
	.name   = "filter",
	.type	= NFT_TABLE_FILTER,
	.chains = {
			{
				.name   = "INPUT",
				.type   = "filter",
				.prio   = NF_IP_PRI_FILTER,
				.hook   = NF_ARP_IN,
			},
			{
				.name   = "OUTPUT",
				.type   = "filter",
				.prio   = NF_IP_PRI_FILTER,
				.hook   = NF_ARP_OUT,
			},
		},
	},
};

#include <linux/netfilter_bridge.h>

static const struct builtin_table xtables_bridge[NFT_TABLE_MAX] = {
	[NFT_TABLE_FILTER] = {
		.name = "filter",
		.type	= NFT_TABLE_FILTER,
		.chains = {
			{
				.name   = "INPUT",
				.type   = "filter",
				.prio   = NF_BR_PRI_FILTER_BRIDGED,
				.hook   = NF_BR_LOCAL_IN,
			},
			{
				.name   = "FORWARD",
				.type   = "filter",
				.prio   = NF_BR_PRI_FILTER_BRIDGED,
				.hook   = NF_BR_FORWARD,
			},
			{
				.name   = "OUTPUT",
				.type   = "filter",
				.prio   = NF_BR_PRI_FILTER_BRIDGED,
				.hook   = NF_BR_LOCAL_OUT,
			},
		},
	},
	[NFT_TABLE_NAT] = {
		.name = "nat",
		.type	= NFT_TABLE_NAT,
		.chains = {
			{
				.name   = "PREROUTING",
				.type   = "filter",
				.prio   = NF_BR_PRI_NAT_DST_BRIDGED,
				.hook   = NF_BR_PRE_ROUTING,
			},
			{
				.name   = "OUTPUT",
				.type   = "filter",
				.prio   = NF_BR_PRI_NAT_DST_OTHER,
				.hook   = NF_BR_LOCAL_OUT,
			},
			{
				.name   = "POSTROUTING",
				.type   = "filter",
				.prio   = NF_BR_PRI_NAT_SRC,
				.hook   = NF_BR_POST_ROUTING,
			},
		},
	},
	[NFT_TABLE_BROUTE] = {
		.name = "broute",
		.type	= NFT_TABLE_BROUTE,
		.chains = {
			{
				.name   = "BROUTING",
				.type   = "filter",
				.prio   = NF_BR_PRI_FIRST,
				.hook   = NF_BR_PRE_ROUTING,
			},
		},
	},

};

static int nft_table_builtin_add(struct nft_handle *h,
				 const struct builtin_table *_t)
{
	struct nftnl_table *t;
	int ret;

	if (h->cache->table[_t->type].exists)
		return 0;

	t = nftnl_table_alloc();
	if (t == NULL)
		return -1;

	nftnl_table_set_u32(t, NFTNL_TABLE_FAMILY, h->family);
	nftnl_table_set_str(t, NFTNL_TABLE_NAME, _t->name);

	ret = batch_table_add(h, NFT_COMPAT_TABLE_ADD, t) ? 0 : - 1;

	return ret;
}

static struct nftnl_chain *
nft_chain_builtin_alloc(int family, const char *tname,
			const struct builtin_chain *chain, int policy)
{
	struct nftnl_chain *c;

	c = nftnl_chain_alloc();
	if (c == NULL)
		return NULL;

	nftnl_chain_set_u32(c, NFTNL_CHAIN_FAMILY, family);
	nftnl_chain_set_str(c, NFTNL_CHAIN_TABLE, tname);
	nftnl_chain_set_str(c, NFTNL_CHAIN_NAME, chain->name);
	nftnl_chain_set_u32(c, NFTNL_CHAIN_HOOKNUM, chain->hook);
	nftnl_chain_set_u32(c, NFTNL_CHAIN_PRIO, chain->prio);
	if (policy >= 0)
		nftnl_chain_set_u32(c, NFTNL_CHAIN_POLICY, policy);

	nftnl_chain_set_str(c, NFTNL_CHAIN_TYPE, chain->type);

	nftnl_chain_set_u64(c, NFTNL_CHAIN_PACKETS, 0);
	nftnl_chain_set_u64(c, NFTNL_CHAIN_BYTES, 0);

	return c;
}

static void nft_chain_builtin_add(struct nft_handle *h,
				  const struct builtin_table *table,
				  const struct builtin_chain *chain,
				  bool fake)
{
	struct nftnl_chain *c;

	c = nft_chain_builtin_alloc(h->family, table->name, chain, NF_ACCEPT);
	if (c == NULL)
		return;

	if (!fake)
		batch_chain_add(h, NFT_COMPAT_CHAIN_ADD, c);
	nft_cache_add_chain(h, table, c, fake);
}

/* find if built-in table already exists */
const struct builtin_table *
nft_table_builtin_find(struct nft_handle *h, const char *table)
{
	int i;
	bool found = false;

	for (i = 0; i < NFT_TABLE_MAX; i++) {
		if (h->tables[i].name == NULL)
			continue;

		if (strcmp(h->tables[i].name, table) != 0)
			continue;

		found = true;
		break;
	}

	return found ? &h->tables[i] : NULL;
}

/* find if built-in chain already exists */
const struct builtin_chain *
nft_chain_builtin_find(const struct builtin_table *t, const char *chain)
{
	int i;
	bool found = false;

	for (i=0; i<NF_IP_NUMHOOKS && t->chains[i].name != NULL; i++) {
		if (strcmp(t->chains[i].name, chain) != 0)
			continue;

		found = true;
		break;
	}
	return found ? &t->chains[i] : NULL;
}

static void nft_chain_builtin_init(struct nft_handle *h,
				   const struct builtin_table *table)
{
	struct nft_chain *c;
	int i;

	/* Initialize built-in chains if they don't exist yet */
	for (i=0; i < NF_INET_NUMHOOKS && table->chains[i].name != NULL; i++) {
		c = nft_chain_find(h, table->name, table->chains[i].name);
		if (!c) {
			nft_chain_builtin_add(h, table,
					      &table->chains[i], false);
		} else if (c->fake) {
			batch_chain_add(h, NFT_COMPAT_CHAIN_ADD, c->nftnl);
			c->fake = false;
		}
	}
}

static const struct builtin_table *
nft_xt_builtin_table_init(struct nft_handle *h, const char *table)
{
	const struct builtin_table *t;

	if (!h->cache_init)
		return NULL;

	t = nft_table_builtin_find(h, table);
	if (t == NULL)
		return NULL;

	if (nft_table_builtin_add(h, t) < 0)
		return NULL;

	return t;
}

static int nft_xt_builtin_init(struct nft_handle *h, const char *table,
			       const char *chain)
{
	const struct builtin_table *t;
	const struct builtin_chain *c;
	struct nft_chain *nc;

	if (!h->cache_init)
		return 0;

	t = nft_xt_builtin_table_init(h, table);
	if (!t)
		return -1;

	if (h->cache_req.level < NFT_CL_CHAINS)
		return 0;

	if (!chain) {
		nft_chain_builtin_init(h, t);
		return 0;
	}

	c = nft_chain_builtin_find(t, chain);
	if (!c)
		return -1;

	nc = h->cache->table[t->type].base_chains[c->hook];
	if (!nc) {
		nft_chain_builtin_add(h, t, c, false);
	} else if (nc->fake) {
		batch_chain_add(h, NFT_COMPAT_CHAIN_ADD, nc->nftnl);
		nc->fake = false;
	}
	return 0;
}

static bool nft_chain_builtin(struct nftnl_chain *c)
{
	/* Check if this chain has hook number, in that case is built-in.
	 * Should we better export the flags to user-space via nf_tables?
	 */
	return nftnl_chain_get(c, NFTNL_CHAIN_HOOKNUM) != NULL;
}

static int __nft_xt_fake_builtin_chains(struct nft_handle *h,
				        const char *table, void *data)
{
	const char *chain = data ? *(const char **)data : NULL;
	const struct builtin_table *t;
	struct nft_chain **bcp;
	int i;

	t = nft_table_builtin_find(h, table);
	if (!t)
		return -1;

	bcp = h->cache->table[t->type].base_chains;
	for (i = 0; i < NF_INET_NUMHOOKS && t->chains[i].name; i++) {
		if (bcp[t->chains[i].hook])
			continue;

		if (chain && strcmp(chain, t->chains[i].name))
			continue;

		nft_chain_builtin_add(h, t, &t->chains[i], true);
	}
	return 0;
}

int nft_xt_fake_builtin_chains(struct nft_handle *h,
			       const char *table, const char *chain)
{
	if (table)
		return __nft_xt_fake_builtin_chains(h, table, &chain);

	return nft_for_each_table(h, __nft_xt_fake_builtin_chains, &chain);
}

int nft_restart(struct nft_handle *h)
{
	mnl_socket_close(h->nl);

	h->nl = mnl_socket_open(NETLINK_NETFILTER);
	if (h->nl == NULL)
		return -1;

	if (mnl_socket_bind(h->nl, 0, MNL_SOCKET_AUTOPID) < 0)
		return -1;

	h->portid = mnl_socket_get_portid(h->nl);
	h->nlsndbuffsiz = 0;
	h->nlrcvbuffsiz = 0;

	return 0;
}

static const struct builtin_table *builtin_tables_lookup(int family)
{
	switch (family) {
	case AF_INET:
	case AF_INET6:
		return xtables_ipv4;
	case NFPROTO_ARP:
		return xtables_arp;
	case NFPROTO_BRIDGE:
		return xtables_bridge;
	default:
		return NULL;
	}
}

int nft_init(struct nft_handle *h, int family)
{
	memset(h, 0, sizeof(*h));

	h->nl = mnl_socket_open(NETLINK_NETFILTER);
	if (h->nl == NULL)
		return -1;

	if (mnl_socket_bind(h->nl, 0, MNL_SOCKET_AUTOPID) < 0) {
		mnl_socket_close(h->nl);
		return -1;
	}

	h->ops = nft_family_ops_lookup(family);
	if (!h->ops)
		xtables_error(PARAMETER_PROBLEM, "Unknown family");

	h->portid = mnl_socket_get_portid(h->nl);
	h->tables = builtin_tables_lookup(family);
	h->cache = &h->__cache[0];
	h->family = family;

	INIT_LIST_HEAD(&h->obj_list);
	INIT_LIST_HEAD(&h->err_list);
	INIT_LIST_HEAD(&h->cmd_list);
	INIT_LIST_HEAD(&h->cache_req.chain_list);

	return 0;
}

void nft_fini(struct nft_handle *h)
{
	struct list_head *pos, *n;

	list_for_each_safe(pos, n, &h->cmd_list)
		nft_cmd_free(list_entry(pos, struct nft_cmd, head));

	list_for_each_safe(pos, n, &h->obj_list)
		batch_obj_del(h, list_entry(pos, struct obj_update, head));

	list_for_each_safe(pos, n, &h->err_list)
		mnl_err_list_free(list_entry(pos, struct mnl_err, head));

	nft_release_cache(h);
	mnl_socket_close(h->nl);
}

static void nft_chain_print_debug(struct nft_handle *h,
				  struct nftnl_chain *c, struct nlmsghdr *nlh)
{
	if (h->verbose > 1) {
		nftnl_chain_fprintf(stdout, c, 0, 0);
		fprintf(stdout, "\n");
	}
	if (h->verbose > 2)
		mnl_nlmsg_fprintf(stdout, nlh, nlh->nlmsg_len,
				  sizeof(struct nfgenmsg));
}

static struct nftnl_chain *nft_chain_new(struct nft_handle *h,
				       const char *table, const char *chain,
				       int policy,
				       const struct xt_counters *counters)
{
	static const struct xt_counters zero = {};
	struct nftnl_chain *c;
	const struct builtin_table *_t;
	const struct builtin_chain *_c;

	_t = nft_table_builtin_find(h, table);
	if (!_t) {
		errno = ENXIO;
		return NULL;
	}

	/* if this built-in table does not exists, create it */
	nft_xt_builtin_init(h, table, chain);

	_c = nft_chain_builtin_find(_t, chain);
	if (_c != NULL) {
		/* This is a built-in chain */
		c = nft_chain_builtin_alloc(h->family, _t->name, _c, policy);
		if (c == NULL)
			return NULL;
	} else {
		errno = ENOENT;
		return NULL;
	}

	if (!counters)
		counters = &zero;
	nftnl_chain_set_u64(c, NFTNL_CHAIN_BYTES, counters->bcnt);
	nftnl_chain_set_u64(c, NFTNL_CHAIN_PACKETS, counters->pcnt);

	return c;
}

int nft_chain_set(struct nft_handle *h, const char *table,
		  const char *chain, const char *policy,
		  const struct xt_counters *counters)
{
	struct nftnl_chain *c = NULL;

	nft_fn = nft_chain_set;

	if (strcmp(policy, "DROP") == 0)
		c = nft_chain_new(h, table, chain, NF_DROP, counters);
	else if (strcmp(policy, "ACCEPT") == 0)
		c = nft_chain_new(h, table, chain, NF_ACCEPT, counters);
	else if (strcmp(policy, "-") == 0)
		c = nft_chain_new(h, table, chain, -1, counters);
	else
		errno = EINVAL;

	if (c == NULL)
		return 0;

	if (!batch_chain_add(h, NFT_COMPAT_CHAIN_UPDATE, c))
		return 0;

	/* the core expects 1 for success and 0 for error */
	return 1;
}

static int __add_match(struct nftnl_expr *e, struct xt_entry_match *m)
{
	void *info;

	nftnl_expr_set(e, NFTNL_EXPR_MT_NAME, m->u.user.name, strlen(m->u.user.name));
	nftnl_expr_set_u32(e, NFTNL_EXPR_MT_REV, m->u.user.revision);

	info = xtables_calloc(1, m->u.match_size);
	memcpy(info, m->data, m->u.match_size - sizeof(*m));
	nftnl_expr_set(e, NFTNL_EXPR_MT_INFO, info, m->u.match_size - sizeof(*m));

	return 0;
}

static int add_nft_limit(struct nftnl_rule *r, struct xt_entry_match *m)
{
	struct xt_rateinfo *rinfo = (void *)m->data;
	static const uint32_t mult[] = {
		XT_LIMIT_SCALE*24*60*60,	/* day */
		XT_LIMIT_SCALE*60*60,		/* hour */
		XT_LIMIT_SCALE*60,		/* min */
		XT_LIMIT_SCALE,			/* sec */
	};
	struct nftnl_expr *expr;
	int i;

	expr = nftnl_expr_alloc("limit");
	if (!expr)
		return -ENOMEM;

	for (i = 1; i < ARRAY_SIZE(mult); i++) {
		if (rinfo->avg > mult[i] ||
		    mult[i] / rinfo->avg < mult[i] % rinfo->avg)
			break;
	}

	nftnl_expr_set_u32(expr, NFTNL_EXPR_LIMIT_TYPE, NFT_LIMIT_PKTS);
	nftnl_expr_set_u32(expr, NFTNL_EXPR_LIMIT_FLAGS, 0);

	nftnl_expr_set_u64(expr, NFTNL_EXPR_LIMIT_RATE,
			   mult[i - 1] / rinfo->avg);
        nftnl_expr_set_u64(expr, NFTNL_EXPR_LIMIT_UNIT,
			   mult[i - 1] / XT_LIMIT_SCALE);

	nftnl_expr_set_u32(expr, NFTNL_EXPR_LIMIT_BURST, rinfo->burst);

	nftnl_rule_add_expr(r, expr);
	return 0;
}

static struct nftnl_set *add_anon_set(struct nft_handle *h, const char *table,
				      uint32_t flags, uint32_t key_type,
				      uint32_t key_len, uint32_t size)
{
	static uint32_t set_id = 0;
	struct nftnl_set *s;
	struct nft_cmd *cmd;

	s = nftnl_set_alloc();
	if (!s)
		return NULL;

	nftnl_set_set_u32(s, NFTNL_SET_FAMILY, h->family);
	nftnl_set_set_str(s, NFTNL_SET_TABLE, table);
	nftnl_set_set_str(s, NFTNL_SET_NAME, "__set%d");
	nftnl_set_set_u32(s, NFTNL_SET_ID, ++set_id);
	nftnl_set_set_u32(s, NFTNL_SET_FLAGS,
			  NFT_SET_ANONYMOUS | NFT_SET_CONSTANT | flags);
	nftnl_set_set_u32(s, NFTNL_SET_KEY_TYPE, key_type);
	nftnl_set_set_u32(s, NFTNL_SET_KEY_LEN, key_len);
	nftnl_set_set_u32(s, NFTNL_SET_DESC_SIZE, size);

	cmd = nft_cmd_new(h, NFT_COMPAT_SET_ADD, table, NULL, NULL, -1, false);
	if (!cmd) {
		nftnl_set_free(s);
		return NULL;
	}
	cmd->obj.set = s;

	return s;
}

static struct nftnl_expr *
__gen_payload(uint32_t base, uint32_t offset, uint32_t len, uint8_t reg)
{
	struct nftnl_expr *e = nftnl_expr_alloc("payload");

	if (!e)
		return NULL;

	nftnl_expr_set_u32(e, NFTNL_EXPR_PAYLOAD_BASE, base);
	nftnl_expr_set_u32(e, NFTNL_EXPR_PAYLOAD_OFFSET, offset);
	nftnl_expr_set_u32(e, NFTNL_EXPR_PAYLOAD_LEN, len);
	nftnl_expr_set_u32(e, NFTNL_EXPR_PAYLOAD_DREG, reg);

	return e;
}

static struct nftnl_expr *
gen_payload(struct nft_handle *h, uint32_t base, uint32_t offset, uint32_t len,
	    uint8_t *dreg)
{
	struct nftnl_expr *e;
	uint8_t reg;

	reg = NFT_REG_1;
	e = __gen_payload(base, offset, len, reg);
	*dreg = reg;

	return e;
}

static struct nftnl_expr *
gen_lookup(uint32_t sreg, const char *set_name, uint32_t set_id, uint32_t flags)
{
	struct nftnl_expr *e = nftnl_expr_alloc("lookup");

	if (!e)
		return NULL;
	nftnl_expr_set_u32(e, NFTNL_EXPR_LOOKUP_SREG, sreg);
	nftnl_expr_set_str(e, NFTNL_EXPR_LOOKUP_SET, set_name);
	nftnl_expr_set_u32(e, NFTNL_EXPR_LOOKUP_SET_ID, set_id);
	nftnl_expr_set_u32(e, NFTNL_EXPR_LOOKUP_FLAGS, flags);
	return e;
}

/* from nftables:include/datatype.h, TYPE_BITS */
#define CONCAT_TYPE_BITS	6

/* from nftables:include/datatype.h, enum datatypes */
#define NFT_DATATYPE_IPADDR	7
#define NFT_DATATYPE_ETHERADDR	9

static int __add_nft_among(struct nft_handle *h, const char *table,
			   struct nftnl_rule *r, struct nft_among_pair *pairs,
			   int cnt, bool dst, bool inv, bool ip)
{
	uint32_t set_id, type = NFT_DATATYPE_ETHERADDR, len = ETH_ALEN;
	/* { !dst, dst } */
	static const int eth_addr_off[] = {
		offsetof(struct ether_header, ether_shost),
		offsetof(struct ether_header, ether_dhost)
	};
	static const int ip_addr_off[] = {
		offsetof(struct iphdr, saddr),
		offsetof(struct iphdr, daddr)
	};
	struct nftnl_expr *e;
	struct nftnl_set *s;
	uint32_t flags = 0;
	uint8_t reg;
	int idx = 0;

	if (ip) {
		type = type << CONCAT_TYPE_BITS | NFT_DATATYPE_IPADDR;
		len += sizeof(struct in_addr) + NETLINK_ALIGN - 1;
		len &= ~(NETLINK_ALIGN - 1);
		flags = NFT_SET_INTERVAL | NFT_SET_CONCAT;
	}

	s = add_anon_set(h, table, flags, type, len, cnt);
	if (!s)
		return -ENOMEM;
	set_id = nftnl_set_get_u32(s, NFTNL_SET_ID);

	if (ip) {
		uint8_t field_len[2] = { ETH_ALEN, sizeof(struct in_addr) };

		nftnl_set_set_data(s, NFTNL_SET_DESC_CONCAT,
				   field_len, sizeof(field_len));
	}

	for (idx = 0; idx < cnt; idx++) {
		struct nftnl_set_elem *elem = nftnl_set_elem_alloc();

		if (!elem)
			return -ENOMEM;
		nftnl_set_elem_set(elem, NFTNL_SET_ELEM_KEY,
				   &pairs[idx], len);
		if (ip) {
			struct in_addr tmp = pairs[idx].in;

			if (tmp.s_addr == INADDR_ANY)
				pairs[idx].in.s_addr = INADDR_BROADCAST;
			nftnl_set_elem_set(elem, NFTNL_SET_ELEM_KEY_END,
					   &pairs[idx], len);
			pairs[idx].in = tmp;
		}
		nftnl_set_elem_add(s, elem);
	}

	e = gen_payload(h, NFT_PAYLOAD_LL_HEADER,
			eth_addr_off[dst], ETH_ALEN, &reg);
	if (!e)
		return -ENOMEM;
	nftnl_rule_add_expr(r, e);

	if (ip) {
		reg = nft_get_next_reg(reg, ETH_ALEN);
		e = __gen_payload(NFT_PAYLOAD_NETWORK_HEADER, ip_addr_off[dst],
				  sizeof(struct in_addr), reg);
		if (!e)
			return -ENOMEM;
		nftnl_rule_add_expr(r, e);
	}

	reg = NFT_REG_1;
	e = gen_lookup(reg, "__set%d", set_id, inv);
	if (!e)
		return -ENOMEM;
	nftnl_rule_add_expr(r, e);

	return 0;
}

static int add_nft_among(struct nft_handle *h,
			 struct nftnl_rule *r, struct xt_entry_match *m)
{
	struct nft_among_data *data = (struct nft_among_data *)m->data;
	const char *table = nftnl_rule_get(r, NFTNL_RULE_TABLE);

	if ((data->src.cnt && data->src.ip) ||
	    (data->dst.cnt && data->dst.ip)) {
		uint16_t eth_p_ip = htons(ETH_P_IP);
		uint8_t reg;

		add_meta(h, r, NFT_META_PROTOCOL, &reg);
		add_cmp_ptr(r, NFT_CMP_EQ, &eth_p_ip, 2, reg);
	}

	if (data->src.cnt)
		__add_nft_among(h, table, r, data->pairs, data->src.cnt,
				false, data->src.inv, data->src.ip);
	if (data->dst.cnt)
		__add_nft_among(h, table, r, data->pairs + data->src.cnt,
				data->dst.cnt, true, data->dst.inv,
				data->dst.ip);
	return 0;
}

static int expr_gen_range_cmp16(struct nftnl_rule *r,
				uint16_t lo,
				uint16_t hi,
				bool invert, uint8_t reg)
{
	struct nftnl_expr *e;

	if (lo == hi) {
		add_cmp_u16(r, htons(lo), invert ? NFT_CMP_NEQ : NFT_CMP_EQ, reg);
		return 0;
	}

	if (lo == 0 && hi < 0xffff) {
		add_cmp_u16(r, htons(hi) , invert ? NFT_CMP_GT : NFT_CMP_LTE, reg);
		return 0;
	}

	e = nftnl_expr_alloc("range");
	if (!e)
		return -ENOMEM;

	nftnl_expr_set_u32(e, NFTNL_EXPR_RANGE_SREG, reg);
	nftnl_expr_set_u32(e, NFTNL_EXPR_RANGE_OP, invert ? NFT_RANGE_NEQ : NFT_RANGE_EQ);

	lo = htons(lo);
	nftnl_expr_set(e, NFTNL_EXPR_RANGE_FROM_DATA, &lo, sizeof(lo));
	hi = htons(hi);
	nftnl_expr_set(e, NFTNL_EXPR_RANGE_TO_DATA, &hi, sizeof(hi));

	nftnl_rule_add_expr(r, e);
	return 0;
}

static int add_nft_tcpudp(struct nft_handle *h,struct nftnl_rule *r,
			  uint16_t src[2], bool invert_src,
			  uint16_t dst[2], bool invert_dst)
{
	struct nftnl_expr *expr;
	uint8_t op = NFT_CMP_EQ;
	uint8_t reg;
	int ret;

	if (!invert_src &&
	    src[0] && src[0] == src[1] &&
	    dst[0] && dst[0] == dst[1] &&
	    invert_src == invert_dst) {
		uint32_t combined = dst[0] | (src[0] << 16);

		expr = gen_payload(h, NFT_PAYLOAD_TRANSPORT_HEADER, 0, 4, &reg);
		if (!expr)
			return -ENOMEM;

		nftnl_rule_add_expr(r, expr);
		add_cmp_u32(r, htonl(combined), op, reg);
		return 0;
	}

	if (src[0] || src[1] < UINT16_MAX || invert_src) {
		expr = gen_payload(h, NFT_PAYLOAD_TRANSPORT_HEADER, 0, 2, &reg);
		if (!expr)
			return -ENOMEM;

		nftnl_rule_add_expr(r, expr);
		ret = expr_gen_range_cmp16(r, src[0], src[1], invert_src, reg);
		if (ret)
			return ret;
	}

	if (dst[0] || dst[1] < UINT16_MAX || invert_dst) {
		expr = gen_payload(h, NFT_PAYLOAD_TRANSPORT_HEADER, 2, 2, &reg);
		if (!expr)
			return -ENOMEM;

		nftnl_rule_add_expr(r, expr);
		ret = expr_gen_range_cmp16(r, dst[0], dst[1], invert_dst, reg);
		if (ret)
			return ret;
	}

	return 0;
}

/* without this, "iptables -A INPUT -m udp" is
 * turned into "iptables -A INPUT", which isn't
 * compatible with iptables-legacy behaviour.
 */
static bool udp_all_zero(const struct xt_udp *u)
{
	static const struct xt_udp zero = {
		.spts[1] = 0xffff,
		.dpts[1] = 0xffff,
	};

	return memcmp(u, &zero, sizeof(*u)) == 0;
}

static int add_nft_udp(struct nft_handle *h, struct nftnl_rule *r,
		       struct xt_entry_match *m)
{
	struct xt_udp *udp = (void *)m->data;

	if (udp->invflags > XT_UDP_INV_MASK ||
	    udp_all_zero(udp)) {
		struct nftnl_expr *expr = nftnl_expr_alloc("match");
		int ret;

		ret = __add_match(expr, m);
		nftnl_rule_add_expr(r, expr);
		return ret;
	}

	if (nftnl_rule_get_u32(r, NFTNL_RULE_COMPAT_PROTO) != IPPROTO_UDP)
		xtables_error(PARAMETER_PROBLEM, "UDP match requires '-p udp'");

	return add_nft_tcpudp(h, r, udp->spts, udp->invflags & XT_UDP_INV_SRCPT,
			      udp->dpts, udp->invflags & XT_UDP_INV_DSTPT);
}

static int add_nft_tcpflags(struct nft_handle *h, struct nftnl_rule *r,
			    uint8_t cmp, uint8_t mask,
			    bool invert)
{
	struct nftnl_expr *e;
	uint8_t reg;

	e = gen_payload(h, NFT_PAYLOAD_TRANSPORT_HEADER, 13, 1, &reg);

	if (!e)
		return -ENOMEM;

	nftnl_rule_add_expr(r, e);

	add_bitwise(h, r, &mask, 1, reg, &reg);
	add_cmp_u8(r, cmp, invert ? NFT_CMP_NEQ : NFT_CMP_EQ, reg);

	return 0;
}

static bool tcp_all_zero(const struct xt_tcp *t)
{
	static const struct xt_tcp zero = {
		.spts[1] = 0xffff,
		.dpts[1] = 0xffff,
	};

	return memcmp(t, &zero, sizeof(*t)) == 0;
}

static int add_nft_tcp(struct nft_handle *h, struct nftnl_rule *r,
		       struct xt_entry_match *m)
{
	static const uint8_t supported = XT_TCP_INV_SRCPT | XT_TCP_INV_DSTPT | XT_TCP_INV_FLAGS;
	struct xt_tcp *tcp = (void *)m->data;

	if (tcp->invflags & ~supported || tcp->option ||
	    tcp_all_zero(tcp)) {
		struct nftnl_expr *expr = nftnl_expr_alloc("match");
		int ret;

		ret = __add_match(expr, m);
		nftnl_rule_add_expr(r, expr);
		return ret;
	}

	if (nftnl_rule_get_u32(r, NFTNL_RULE_COMPAT_PROTO) != IPPROTO_TCP)
		xtables_error(PARAMETER_PROBLEM, "TCP match requires '-p tcp'");

	if (tcp->flg_mask) {
		int ret = add_nft_tcpflags(h, r, tcp->flg_cmp, tcp->flg_mask,
					   tcp->invflags & XT_TCP_INV_FLAGS);

		if (ret < 0)
			return ret;
	}

	return add_nft_tcpudp(h, r, tcp->spts, tcp->invflags & XT_TCP_INV_SRCPT,
			      tcp->dpts, tcp->invflags & XT_TCP_INV_DSTPT);
}

static int add_nft_mark(struct nft_handle *h, struct nftnl_rule *r,
			struct xt_entry_match *m)
{
	struct xt_mark_mtinfo1 *mark = (void *)m->data;
	uint8_t reg;
	int op;

	add_meta(h, r, NFT_META_MARK, &reg);
	if (mark->mask != 0xffffffff)
		add_bitwise(h, r, (uint8_t *)&mark->mask, sizeof(uint32_t), reg, &reg);

	if (mark->invert)
		op = NFT_CMP_NEQ;
	else
		op = NFT_CMP_EQ;

	add_cmp_u32(r, mark->mark, op, reg);

	return 0;
}

int add_match(struct nft_handle *h, struct nft_rule_ctx *ctx,
	      struct nftnl_rule *r, struct xt_entry_match *m)
{
	struct nftnl_expr *expr;
	int ret;

	switch (ctx->command) {
	case NFT_COMPAT_RULE_APPEND:
	case NFT_COMPAT_RULE_INSERT:
	case NFT_COMPAT_RULE_REPLACE:
		if (!strcmp(m->u.user.name, "limit"))
			return add_nft_limit(r, m);
		else if (!strcmp(m->u.user.name, "among"))
			return add_nft_among(h, r, m);
		else if (!strcmp(m->u.user.name, "udp"))
			return add_nft_udp(h, r, m);
		else if (!strcmp(m->u.user.name, "tcp"))
			return add_nft_tcp(h, r, m);
		else if (!strcmp(m->u.user.name, "mark"))
			return add_nft_mark(h, r, m);
		break;
	default:
		break;
	}

	expr = nftnl_expr_alloc("match");
	if (expr == NULL)
		return -ENOMEM;

	ret = __add_match(expr, m);
	nftnl_rule_add_expr(r, expr);

	return ret;
}

static int __add_target(struct nftnl_expr *e, struct xt_entry_target *t)
{
	void *info;

	nftnl_expr_set(e, NFTNL_EXPR_TG_NAME, t->u.user.name,
			  strlen(t->u.user.name));
	nftnl_expr_set_u32(e, NFTNL_EXPR_TG_REV, t->u.user.revision);

	info = xtables_calloc(1, t->u.target_size);
	memcpy(info, t->data, t->u.target_size - sizeof(*t));
	nftnl_expr_set(e, NFTNL_EXPR_TG_INFO, info, t->u.target_size - sizeof(*t));

	return 0;
}

static int add_meta_nftrace(struct nftnl_rule *r)
{
	struct nftnl_expr *expr;

	expr = nftnl_expr_alloc("immediate");
	if (expr == NULL)
		return -ENOMEM;

	nftnl_expr_set_u32(expr, NFTNL_EXPR_IMM_DREG, NFT_REG32_01);
	nftnl_expr_set_u8(expr, NFTNL_EXPR_IMM_DATA, 1);
	nftnl_rule_add_expr(r, expr);

	expr = nftnl_expr_alloc("meta");
	if (expr == NULL)
		return -ENOMEM;
	nftnl_expr_set_u32(expr, NFTNL_EXPR_META_KEY, NFT_META_NFTRACE);
	nftnl_expr_set_u32(expr, NFTNL_EXPR_META_SREG, NFT_REG32_01);

	nftnl_rule_add_expr(r, expr);
	return 0;
}

int add_target(struct nftnl_rule *r, struct xt_entry_target *t)
{
	struct nftnl_expr *expr;
	int ret;

	if (strcmp(t->u.user.name, "TRACE") == 0)
		return add_meta_nftrace(r);

	expr = nftnl_expr_alloc("target");
	if (expr == NULL)
		return -ENOMEM;

	ret = __add_target(expr, t);
	nftnl_rule_add_expr(r, expr);

	return ret;
}

int add_jumpto(struct nftnl_rule *r, const char *name, int verdict)
{
	struct nftnl_expr *expr;

	expr = nftnl_expr_alloc("immediate");
	if (expr == NULL)
		return -ENOMEM;

	nftnl_expr_set_u32(expr, NFTNL_EXPR_IMM_DREG, NFT_REG_VERDICT);
	nftnl_expr_set_u32(expr, NFTNL_EXPR_IMM_VERDICT, verdict);
	nftnl_expr_set_str(expr, NFTNL_EXPR_IMM_CHAIN, (char *)name);
	nftnl_rule_add_expr(r, expr);

	return 0;
}

int add_verdict(struct nftnl_rule *r, int verdict)
{
	struct nftnl_expr *expr;

	expr = nftnl_expr_alloc("immediate");
	if (expr == NULL)
		return -ENOMEM;

	nftnl_expr_set_u32(expr, NFTNL_EXPR_IMM_DREG, NFT_REG_VERDICT);
	nftnl_expr_set_u32(expr, NFTNL_EXPR_IMM_VERDICT, verdict);
	nftnl_rule_add_expr(r, expr);

	return 0;
}

int add_action(struct nftnl_rule *r, struct iptables_command_state *cs,
	       bool goto_set)
{
	int ret = 0;

	/* If no target at all, add nothing (default to continue) */
	if (cs->target != NULL) {
		/* Standard target? */
		if (strcmp(cs->jumpto, XTC_LABEL_ACCEPT) == 0)
			ret = add_verdict(r, NF_ACCEPT);
		else if (strcmp(cs->jumpto, XTC_LABEL_DROP) == 0)
			ret = add_verdict(r, NF_DROP);
		else if (strcmp(cs->jumpto, XTC_LABEL_RETURN) == 0)
			ret = add_verdict(r, NFT_RETURN);
		else if (strcmp(cs->jumpto, "NFLOG") == 0)
			ret = add_log(r, cs);
		else
			ret = add_target(r, cs->target->t);
	} else if (strlen(cs->jumpto) > 0) {
		/* Not standard, then it's a go / jump to chain */
		if (goto_set)
			ret = add_jumpto(r, cs->jumpto, NFT_GOTO);
		else
			ret = add_jumpto(r, cs->jumpto, NFT_JUMP);
	}
	return ret;
}

int add_log(struct nftnl_rule *r, struct iptables_command_state *cs)
{
	struct nftnl_expr *expr;
	struct xt_nflog_info *info = (struct xt_nflog_info *)cs->target->t->data;

	expr = nftnl_expr_alloc("log");
	if (!expr)
		return -ENOMEM;

	if (info->prefix[0] != '\0')
		nftnl_expr_set_str(expr, NFTNL_EXPR_LOG_PREFIX,
				   cs->target->udata);

	nftnl_expr_set_u16(expr, NFTNL_EXPR_LOG_GROUP, info->group);
	if (info->flags & XT_NFLOG_F_COPY_LEN)
		nftnl_expr_set_u32(expr, NFTNL_EXPR_LOG_SNAPLEN,
				   info->len);
	if (info->threshold)
		nftnl_expr_set_u16(expr, NFTNL_EXPR_LOG_QTHRESHOLD,
				   info->threshold);

	nftnl_rule_add_expr(r, expr);
	return 0;
}

static void nft_rule_print_debug(struct nft_handle *h,
				 struct nftnl_rule *r, struct nlmsghdr *nlh)
{
	if (h->verbose > 1) {
		nftnl_rule_fprintf(stdout, r, 0, 0);
		fprintf(stdout, "\n");
	}
	if (h->verbose > 2)
		mnl_nlmsg_fprintf(stdout, nlh, nlh->nlmsg_len,
				  sizeof(struct nfgenmsg));
}

int add_counters(struct nftnl_rule *r, uint64_t packets, uint64_t bytes)
{
	struct nftnl_expr *expr;

	expr = nftnl_expr_alloc("counter");
	if (expr == NULL)
		return -ENOMEM;

	nftnl_expr_set_u64(expr, NFTNL_EXPR_CTR_PACKETS, packets);
	nftnl_expr_set_u64(expr, NFTNL_EXPR_CTR_BYTES, bytes);

	nftnl_rule_add_expr(r, expr);

	return 0;
}

enum udata_type {
	UDATA_TYPE_COMMENT,
	UDATA_TYPE_EBTABLES_POLICY,
	__UDATA_TYPE_MAX,
};
#define UDATA_TYPE_MAX (__UDATA_TYPE_MAX - 1)

static int parse_udata_cb(const struct nftnl_udata *attr, void *data)
{
	unsigned char *value = nftnl_udata_get(attr);
	uint8_t type = nftnl_udata_type(attr);
	uint8_t len = nftnl_udata_len(attr);
	const struct nftnl_udata **tb = data;

	switch (type) {
	case UDATA_TYPE_COMMENT:
		if (value[len - 1] != '\0')
			return -1;
		break;
	case UDATA_TYPE_EBTABLES_POLICY:
		break;
	default:
		return 0;
	}
	tb[type] = attr;
	return 0;
}

char *get_comment(const void *data, uint32_t data_len)
{
	const struct nftnl_udata *tb[UDATA_TYPE_MAX + 1] = {};

	if (nftnl_udata_parse(data, data_len, parse_udata_cb, tb) < 0)
		return NULL;

	if (!tb[UDATA_TYPE_COMMENT])
		return NULL;

	return nftnl_udata_get(tb[UDATA_TYPE_COMMENT]);
}

void add_compat(struct nftnl_rule *r, uint32_t proto, bool inv)
{
	nftnl_rule_set_u32(r, NFTNL_RULE_COMPAT_PROTO, proto);
	nftnl_rule_set_u32(r, NFTNL_RULE_COMPAT_FLAGS,
			      inv ? NFT_RULE_COMPAT_F_INV : 0);
}

struct nftnl_rule *
nft_rule_new(struct nft_handle *h, struct nft_rule_ctx *ctx,
	     const char *chain, const char *table,
	     struct iptables_command_state *cs)
{
	struct nftnl_rule *r;

	r = nftnl_rule_alloc();
	if (r == NULL)
		return NULL;

	nftnl_rule_set_u32(r, NFTNL_RULE_FAMILY, h->family);
	nftnl_rule_set_str(r, NFTNL_RULE_TABLE, table);
	nftnl_rule_set_str(r, NFTNL_RULE_CHAIN, chain);

	if (h->ops->add(h, ctx, r, cs) < 0)
		goto err;

	return r;
err:
	nftnl_rule_free(r);
	return NULL;
}

int
nft_rule_append(struct nft_handle *h, const char *chain, const char *table,
		struct nftnl_rule *r, struct nftnl_rule *ref, bool verbose)
{
	struct nft_chain *c;
	int type;

	nft_xt_builtin_init(h, table, chain);

	nft_fn = nft_rule_append;

	if (ref) {
		nftnl_rule_set_u64(r, NFTNL_RULE_HANDLE,
				   nftnl_rule_get_u64(ref, NFTNL_RULE_HANDLE));
		type = NFT_COMPAT_RULE_REPLACE;
	} else
		type = NFT_COMPAT_RULE_APPEND;

	if (batch_rule_add(h, type, r) == NULL)
		return 0;

	if (verbose)
		h->ops->print_rule(h, r, 0, FMT_PRINT_RULE);

	if (ref) {
		nftnl_chain_rule_insert_at(r, ref);
		nftnl_chain_rule_del(ref);
		nftnl_rule_free(ref);
	} else {
		c = nft_chain_find(h, table, chain);
		if (!c) {
			errno = ENOENT;
			return 0;
		}
		nftnl_chain_rule_add_tail(r, c->nftnl);
	}

	return 1;
}

bool
nft_rule_print_save(struct nft_handle *h, const struct nftnl_rule *r,
		    enum nft_rule_print type, unsigned int format)
{
	const char *chain = nftnl_rule_get_str(r, NFTNL_RULE_CHAIN);
	struct iptables_command_state cs = {};
	struct nft_family_ops *ops = h->ops;
	bool ret;

	if (ops->init_cs)
		ops->init_cs(&cs);
	ret = ops->rule_to_cs(h, r, &cs);

	if (!(format & (FMT_NOCOUNTS | FMT_C_COUNTS)))
		printf("[%llu:%llu] ", (unsigned long long)cs.counters.pcnt,
				       (unsigned long long)cs.counters.bcnt);

	/* print chain name */
	switch(type) {
	case NFT_RULE_APPEND:
		printf("-A %s", chain);
		break;
	case NFT_RULE_DEL:
		printf("-D %s", chain);
		break;
	}

	if (ops->save_rule)
		ops->save_rule(&cs, format);

	if (ops->clear_cs)
		ops->clear_cs(&cs);

	return ret;
}

bool nft_rule_is_policy_rule(struct nftnl_rule *r)
{
	const struct nftnl_udata *tb[UDATA_TYPE_MAX + 1] = {};
	const void *data;
	uint32_t len;

	if (!nftnl_rule_is_set(r, NFTNL_RULE_USERDATA))
		return false;

	data = nftnl_rule_get_data(r, NFTNL_RULE_USERDATA, &len);
	if (nftnl_udata_parse(data, len, parse_udata_cb, tb) < 0)
		return NULL;

	if (!tb[UDATA_TYPE_EBTABLES_POLICY] ||
	    nftnl_udata_get_u32(tb[UDATA_TYPE_EBTABLES_POLICY]) != 1)
		return false;

	return true;
}

static struct nftnl_rule *nft_chain_last_rule(struct nftnl_chain *c)
{
	struct nftnl_rule *r = NULL, *last;
	struct nftnl_rule_iter *iter;

	iter = nftnl_rule_iter_create(c);
	if (!iter)
		return NULL;

	do {
		last = r;
		r = nftnl_rule_iter_next(iter);
	} while (r);
	nftnl_rule_iter_destroy(iter);

	return last;
}

void nft_bridge_chain_postprocess(struct nft_handle *h,
				  struct nftnl_chain *c)
{
	struct nftnl_rule *last = nft_chain_last_rule(c);
	struct nftnl_expr_iter *iter;
	struct nftnl_expr *expr;
	int verdict;

	if (!last || !nft_rule_is_policy_rule(last))
		return;

	iter = nftnl_expr_iter_create(last);
	if (!iter)
		return;

	expr = nftnl_expr_iter_next(iter);
	if (!expr ||
	    strcmp("counter", nftnl_expr_get_str(expr, NFTNL_EXPR_NAME)))
		goto out_iter;

	expr = nftnl_expr_iter_next(iter);
	if (!expr ||
	    strcmp("immediate", nftnl_expr_get_str(expr, NFTNL_EXPR_NAME)) ||
	    !nftnl_expr_is_set(expr, NFTNL_EXPR_IMM_VERDICT))
		goto out_iter;

	verdict = nftnl_expr_get_u32(expr, NFTNL_EXPR_IMM_VERDICT);
	switch (verdict) {
	case NF_ACCEPT:
	case NF_DROP:
		break;
	default:
		goto out_iter;
	}

	nftnl_chain_set_u32(c, NFTNL_CHAIN_POLICY, verdict);
	if (batch_rule_add(h, NFT_COMPAT_RULE_DELETE, last) == NULL)
		fprintf(stderr, "Failed to delete old policy rule\n");
	nftnl_chain_rule_del(last);
out_iter:
	nftnl_expr_iter_destroy(iter);
}
static const char *policy_name[NF_ACCEPT+1] = {
	[NF_DROP] = "DROP",
	[NF_ACCEPT] = "ACCEPT",
};

int nft_chain_save(struct nft_chain *nc, void *data)
{
	struct nftnl_chain *c = nc->nftnl;
	struct nft_handle *h = data;
	const char *policy = NULL;

	if (nftnl_chain_is_set(c, NFTNL_CHAIN_POLICY)) {
		policy = policy_name[nftnl_chain_get_u32(c, NFTNL_CHAIN_POLICY)];
	} else if (nft_chain_builtin(c)) {
		policy = "ACCEPT";
	} else if (h->family == NFPROTO_BRIDGE) {
		policy = "RETURN";
	}

	if (h->ops->save_chain)
		h->ops->save_chain(c, policy);

	return 0;
}

struct nft_rule_save_data {
	struct nft_handle *h;
	unsigned int format;
	unsigned int errors;
};

static int nft_rule_save_cb(struct nft_chain *c, void *data)
{
	struct nft_rule_save_data *d = data;
	struct nftnl_rule_iter *iter;
	struct nftnl_rule *r;

	iter = nftnl_rule_iter_create(c->nftnl);
	if (iter == NULL)
		return 1;

	r = nftnl_rule_iter_next(iter);
	while (r != NULL) {
		bool ret = nft_rule_print_save(d->h, r, NFT_RULE_APPEND, d->format);

		if (!ret)
			d->errors++;

		r = nftnl_rule_iter_next(iter);
	}

	nftnl_rule_iter_destroy(iter);
	return 0;
}

int nft_rule_save(struct nft_handle *h, const char *table, unsigned int format)
{
	struct nft_rule_save_data d = {
		.h = h,
		.format = format,
	};
	int ret;

	ret = nft_chain_foreach(h, table, nft_rule_save_cb, &d);

	if (ret == 0 && d.errors)
		xtables_error(VERSION_PROBLEM, "Cannot decode all rules provided by kernel");

	/* the core expects 1 for success and 0 for error */
	return ret == 0 ? 1 : 0;
}

struct nftnl_set *nft_set_batch_lookup_byid(struct nft_handle *h,
					    uint32_t set_id)
{
	struct obj_update *n;

	list_for_each_entry(n, &h->obj_list, head) {
		if (n->type == NFT_COMPAT_SET_ADD &&
		    nftnl_set_get_u32(n->set, NFTNL_SET_ID) == set_id)
			return n->set;
	}

	return NULL;
}

static void
__nft_rule_flush(struct nft_handle *h, const char *table,
		 const char *chain, bool verbose, bool skip)
{
	struct obj_update *obj;
	struct nftnl_rule *r;

	if (verbose && chain)
		fprintf(stdout, "Flushing chain `%s'\n", chain);

	r = nftnl_rule_alloc();
	if (r == NULL)
		return;

	nftnl_rule_set_str(r, NFTNL_RULE_TABLE, table);
	if (chain)
		nftnl_rule_set_str(r, NFTNL_RULE_CHAIN, chain);

	obj = batch_rule_add(h, NFT_COMPAT_RULE_FLUSH, r);
	if (!obj) {
		nftnl_rule_free(r);
		return;
	}

	obj->skip = skip;
}

struct nft_rule_flush_data {
	struct nft_handle *h;
	const char *table;
	bool verbose;
};

static int nft_rule_flush_cb(struct nft_chain *c, void *data)
{
	const char *chain = nftnl_chain_get_str(c->nftnl, NFTNL_CHAIN_NAME);
	struct nft_rule_flush_data *d = data;

	batch_chain_flush(d->h, d->table, chain);
	__nft_rule_flush(d->h, d->table, chain, d->verbose, false);
	flush_rule_cache(d->h, d->table, c);
	return 0;
}

int nft_rule_flush(struct nft_handle *h, const char *chain, const char *table,
		   bool verbose)
{
	struct nft_rule_flush_data d = {
		.h = h,
		.table = table,
		.verbose = verbose,
	};
	struct nft_chain *c = NULL;
	int ret = 0;

	nft_fn = nft_rule_flush;

	if (chain || verbose)
		nft_xt_builtin_init(h, table, chain);
	else if (!nft_table_find(h, table))
		return 1;

	if (chain) {
		c = nft_chain_find(h, table, chain);
		if (!c) {
			errno = ENOENT;
			return 0;
		}
	}

	if (chain || !verbose) {
		batch_chain_flush(h, table, chain);
		__nft_rule_flush(h, table, chain, verbose, false);
		flush_rule_cache(h, table, c);
		return 1;
	}

	nft_cache_sort_chains(h, table);

	ret = nft_chain_foreach(h, table, nft_rule_flush_cb, &d);

	/* the core expects 1 for success and 0 for error */
	return ret == 0 ? 1 : 0;
}

int nft_chain_user_add(struct nft_handle *h, const char *chain, const char *table)
{
	const struct builtin_table *t;
	struct nftnl_chain *c;

	nft_fn = nft_chain_user_add;

	t = nft_xt_builtin_table_init(h, table);

	if (nft_chain_exists(h, table, chain)) {
		errno = EEXIST;
		return 0;
	}

	c = nftnl_chain_alloc();
	if (c == NULL)
		return 0;

	nftnl_chain_set_u32(c, NFTNL_CHAIN_FAMILY, h->family);
	nftnl_chain_set_str(c, NFTNL_CHAIN_TABLE, table);
	nftnl_chain_set_str(c, NFTNL_CHAIN_NAME, chain);
	if (h->family == NFPROTO_BRIDGE)
		nftnl_chain_set_u32(c, NFTNL_CHAIN_POLICY, NF_ACCEPT);

	if (!batch_chain_add(h, NFT_COMPAT_CHAIN_USER_ADD, c))
		return 0;

	nft_cache_add_chain(h, t, c, false);

	/* the core expects 1 for success and 0 for error */
	return 1;
}

int nft_chain_restore(struct nft_handle *h, const char *chain, const char *table)
{
	const struct builtin_table *t;
	struct obj_update *obj;
	struct nftnl_chain *c;
	struct nft_chain *nc;
	bool created = false;

	t = nft_xt_builtin_table_init(h, table);

	nc = nft_chain_find(h, table, chain);
	if (!nc) {
		c = nftnl_chain_alloc();
		if (!c)
			return 0;

		nftnl_chain_set_u32(c, NFTNL_CHAIN_FAMILY, h->family);
		nftnl_chain_set_str(c, NFTNL_CHAIN_TABLE, table);
		nftnl_chain_set_str(c, NFTNL_CHAIN_NAME, chain);
		created = true;

		nft_cache_add_chain(h, t, c, false);
	} else {
		c = nc->nftnl;

		/* If the chain should vanish meanwhile, kernel genid changes
		 * and the transaction is refreshed enabling the chain add
		 * object. With the handle still set, kernel interprets it as a
		 * chain replace job and errors since it is not found anymore.
		 */
		nftnl_chain_unset(c, NFTNL_CHAIN_HANDLE);
	}

	__nft_rule_flush(h, table, chain, false, created);

	obj = batch_chain_add(h, NFT_COMPAT_CHAIN_USER_ADD, c);
	if (!obj)
		return 0;

	obj->skip = !created;

	/* the core expects 1 for success and 0 for error */
	return 1;
}

/* From linux/netlink.h */
#ifndef NLM_F_NONREC
#define NLM_F_NONREC	0x100	/* Do not delete recursively    */
#endif

struct chain_del_data {
	struct nft_handle	*handle;
	const char		*chain;
	bool			verbose;
};

static bool nft_may_delete_chain(struct nftnl_chain *c)
{
	if (nftnl_chain_is_set(c, NFTNL_CHAIN_POLICY) &&
	    nftnl_chain_get_u32(c, NFTNL_CHAIN_POLICY) != NF_ACCEPT)
		return false;

	return nftnl_rule_lookup_byindex(c, 0) == NULL;
}

static int __nft_chain_del(struct nft_chain *nc, void *data)
{
	struct chain_del_data *d = data;
	struct nftnl_chain *c = nc->nftnl;
	struct nft_handle *h = d->handle;
	bool builtin = nft_chain_builtin(c);
	struct obj_update *obj;
	int ret = 0;

	if (d->verbose && !builtin)
		fprintf(stdout, "Deleting chain `%s'\n",
			nftnl_chain_get_str(c, NFTNL_CHAIN_NAME));


	/* XXX This triggers a fast lookup from the kernel. */
	nftnl_chain_unset(c, NFTNL_CHAIN_HANDLE);
	obj = batch_chain_add(h, NFT_COMPAT_CHAIN_DEL, c);
	if (!obj)
		return -1;

	if (builtin) {
		obj->skip = !nft_may_delete_chain(c);
		if (obj->skip && d->chain) {
			/* complain if explicitly requested */
			errno = EBUSY;
			ret = -1;
		}
		*nc->base_slot = NULL;
	}

	/* nftnl_chain is freed when deleting the batch object */
	nc->nftnl = NULL;

	nft_chain_list_del(nc);
	nft_chain_free(nc);
	return ret;
}

int nft_chain_del(struct nft_handle *h, const char *chain,
		  const char *table, bool verbose)
{
	struct chain_del_data d = {
		.handle = h,
		.chain = chain,
		.verbose = verbose,
	};
	struct nft_chain *c;
	int ret = 0;

	nft_fn = nft_chain_del;

	if (chain) {
		c = nft_chain_find(h, table, chain);
		if (!c) {
			errno = ENOENT;
			return 0;
		}

		ret = __nft_chain_del(c, &d);
		goto out;
	}

	if (verbose)
		nft_cache_sort_chains(h, table);

	ret = nft_chain_foreach(h, table, __nft_chain_del, &d);
out:
	/* the core expects 1 for success and 0 for error */
	return ret == 0 ? 1 : 0;
}

bool nft_chain_exists(struct nft_handle *h,
		      const char *table, const char *chain)
{
	const struct builtin_table *t = nft_table_builtin_find(h, table);

	/* xtables does not support custom tables */
	if (!t)
		return false;

	if (nft_chain_builtin_find(t, chain))
		return true;

	return !!nft_chain_find(h, table, chain);
}

int nft_chain_user_rename(struct nft_handle *h,const char *chain,
			  const char *table, const char *newname)
{
	struct nftnl_chain *c;
	struct nft_chain *nc;
	uint64_t handle;

	nft_fn = nft_chain_user_rename;

	if (nft_chain_exists(h, table, newname)) {
		errno = EEXIST;
		return 0;
	}

	/* Find the old chain to be renamed */
	nc = nft_chain_find(h, table, chain);
	if (nc == NULL) {
		errno = ENOENT;
		return 0;
	}
	handle = nftnl_chain_get_u64(nc->nftnl, NFTNL_CHAIN_HANDLE);

	/* Now prepare the new name for the chain */
	c = nftnl_chain_alloc();
	if (c == NULL)
		return 0;

	nftnl_chain_set_u32(c, NFTNL_CHAIN_FAMILY, h->family);
	nftnl_chain_set_str(c, NFTNL_CHAIN_TABLE, table);
	nftnl_chain_set_str(c, NFTNL_CHAIN_NAME, newname);
	nftnl_chain_set_u64(c, NFTNL_CHAIN_HANDLE, handle);

	if (!batch_chain_add(h, NFT_COMPAT_CHAIN_RENAME, c))
		return 0;

	/* the core expects 1 for success and 0 for error */
	return 1;
}

bool nft_table_find(struct nft_handle *h, const char *tablename)
{
	const struct builtin_table *t;

	t = nft_table_builtin_find(h, tablename);
	return t ? h->cache->table[t->type].exists : false;
}

int nft_for_each_table(struct nft_handle *h,
		       int (*func)(struct nft_handle *h, const char *tablename, void *data),
		       void *data)
{
	int i;

	for (i = 0; i < NFT_TABLE_MAX; i++) {
		if (h->tables[i].name == NULL)
			continue;

		if (!h->cache->table[h->tables[i].type].exists)
			continue;

		func(h, h->tables[i].name, data);
	}

	return 0;
}

static int __nft_table_flush(struct nft_handle *h, const char *table, bool exists)
{
	const struct builtin_table *_t;
	struct obj_update *obj;
	struct nftnl_table *t;

	t = nftnl_table_alloc();
	if (t == NULL)
		return -1;

	nftnl_table_set_u32(t, NFTNL_TABLE_FAMILY, h->family);
	nftnl_table_set_str(t, NFTNL_TABLE_NAME, table);

	obj = batch_table_add(h, NFT_COMPAT_TABLE_FLUSH, t);
	if (!obj) {
		nftnl_table_free(t);
		return -1;
	}

	if (!exists)
		obj->skip = 1;

	_t = nft_table_builtin_find(h, table);
	assert(_t);
	h->cache->table[_t->type].exists = false;

	flush_chain_cache(h, table);

	return 0;
}

int nft_table_flush(struct nft_handle *h, const char *table)
{
	const struct builtin_table *t;
	int ret = 0;

	nft_fn = nft_table_flush;

	t = nft_table_builtin_find(h, table);
	if (!t)
		return 0;

	ret = __nft_table_flush(h, table, h->cache->table[t->type].exists);

	/* the core expects 1 for success and 0 for error */
	return ret == 0 ? 1 : 0;
}

static int __nft_rule_del(struct nft_handle *h, struct nftnl_rule *r)
{
	struct obj_update *obj;

	nftnl_rule_list_del(r);

	if (!nftnl_rule_get_u64(r, NFTNL_RULE_HANDLE) &&
	    !nftnl_rule_get_u32(r, NFTNL_RULE_ID))
		nftnl_rule_set_u32(r, NFTNL_RULE_ID, ++h->rule_id);

	obj = batch_rule_add(h, NFT_COMPAT_RULE_DELETE, r);
	if (!obj) {
		nftnl_rule_free(r);
		return -1;
	}
	return 1;
}

static bool nft_rule_cmp(struct nft_handle *h, struct nftnl_rule *r,
			 struct iptables_command_state *cs)
{
	struct iptables_command_state this = {};
	bool ret = false, ret_this;

	if (h->ops->init_cs)
		h->ops->init_cs(&this);

	ret_this = h->ops->rule_to_cs(h, r, &this);

	DEBUGP("with ... ");
#ifdef DEBUG_DEL
	nft_rule_print_save(h, r, NFT_RULE_APPEND, 0);
#endif
	if (!ret_this)
		DEBUGP("Cannot convert rule: %d\n", ret_this);

	if (!h->ops->is_same(cs, &this))
		goto out;

	if (!compare_matches(cs->matches, this.matches)) {
		DEBUGP("Different matches\n");
		goto out;
	}

	if (!compare_targets(cs->target, this.target)) {
		DEBUGP("Different target\n");
		goto out;
	}

	if ((!cs->target || !this.target) &&
	    strcmp(cs->jumpto, this.jumpto) != 0) {
		DEBUGP("Different verdict\n");
		goto out;
	}

	ret = true;
out:
	h->ops->clear_cs(&this);
	return ret;
}

static struct nftnl_rule *
nft_rule_find(struct nft_handle *h, struct nft_chain *nc,
	      struct nftnl_rule *rule, int rulenum)
{
	struct iptables_command_state cs = {};
	struct nftnl_chain *c = nc->nftnl;
	struct nftnl_rule *r;
	struct nftnl_rule_iter *iter;
	bool found = false;

	if (rulenum >= 0)
		/* Delete by rule number case */
		return nftnl_rule_lookup_byindex(c, rulenum);

	iter = nftnl_rule_iter_create(c);
	if (iter == NULL)
		return 0;

	if (h->ops->init_cs)
		h->ops->init_cs(&cs);

	if (!h->ops->rule_to_cs(h, rule, &cs))
		return NULL;

	DEBUGP("comparing ... ");
#ifdef DEBUG_DEL
	nft_rule_print_save(h, rule, NFT_RULE_APPEND, 0);
#endif

	r = nftnl_rule_iter_next(iter);
	while (r != NULL) {
		found = nft_rule_cmp(h, r, &cs);
		if (found)
			break;
		r = nftnl_rule_iter_next(iter);
	}

	nftnl_rule_iter_destroy(iter);

	h->ops->clear_cs(&cs);

	return found ? r : NULL;
}

int nft_rule_check(struct nft_handle *h, const char *chain,
		   const char *table, struct nftnl_rule *rule, bool verbose)
{
	struct nftnl_rule *r;
	struct nft_chain *c;

	nft_fn = nft_rule_check;

	c = nft_chain_find(h, table, chain);
	if (!c)
		goto fail_enoent;

	r = nft_rule_find(h, c, rule, -1);
	if (r == NULL)
		goto fail_enoent;

	if (verbose)
		h->ops->print_rule(h, r, 0, FMT_PRINT_RULE);

	return 1;
fail_enoent:
	errno = ENOENT;
	return 0;
}

int nft_rule_delete(struct nft_handle *h, const char *chain,
		    const char *table, struct nftnl_rule *rule, bool verbose)
{
	int ret = 0;
	struct nftnl_rule *r;
	struct nft_chain *c;

	nft_fn = nft_rule_delete;

	c = nft_chain_find(h, table, chain);
	if (!c) {
		errno = ENOENT;
		return 0;
	}

	r = nft_rule_find(h, c, rule, -1);
	if (r != NULL) {
		ret =__nft_rule_del(h, r);
		if (ret < 0)
			errno = ENOMEM;
		if (verbose)
			h->ops->print_rule(h, r, 0, FMT_PRINT_RULE);
	} else
		errno = ENOENT;

	return ret;
}

static struct nftnl_rule *
nft_rule_add(struct nft_handle *h, const char *chain,
	     const char *table, struct nftnl_rule *r,
	     struct nftnl_rule *ref, bool verbose)
{
	uint64_t ref_id;

	if (ref) {
		ref_id = nftnl_rule_get_u64(ref, NFTNL_RULE_HANDLE);
		if (ref_id > 0) {
			nftnl_rule_set_u64(r, NFTNL_RULE_POSITION, ref_id);
			DEBUGP("adding after rule handle %"PRIu64"\n", ref_id);
		} else {
			ref_id = nftnl_rule_get_u32(ref, NFTNL_RULE_ID);
			if (!ref_id) {
				ref_id = ++h->rule_id;
				nftnl_rule_set_u32(ref, NFTNL_RULE_ID, ref_id);
			}
			nftnl_rule_set_u32(r, NFTNL_RULE_POSITION_ID, ref_id);
			DEBUGP("adding after rule ID %"PRIu64"\n", ref_id);
		}
	}

	if (!batch_rule_add(h, NFT_COMPAT_RULE_INSERT, r))
		return NULL;

	if (verbose)
		h->ops->print_rule(h, r, 0, FMT_PRINT_RULE);

	return r;
}

int nft_rule_insert(struct nft_handle *h, const char *chain,
		    const char *table, struct nftnl_rule *new_rule, int rulenum,
		    bool verbose)
{
	struct nftnl_rule *r = NULL;
	struct nft_chain *c;

	nft_xt_builtin_init(h, table, chain);

	nft_fn = nft_rule_insert;

	c = nft_chain_find(h, table, chain);
	if (!c) {
		errno = ENOENT;
		goto err;
	}

	if (rulenum > 0) {
		r = nft_rule_find(h, c, new_rule, rulenum);
		if (r == NULL) {
			/* special case: iptables allows to insert into
			 * rule_count + 1 position.
			 */
			r = nft_rule_find(h, c, new_rule, rulenum - 1);
			if (r != NULL)
				return nft_rule_append(h, chain, table,
						       new_rule, NULL, verbose);

			errno = E2BIG;
			goto err;
		}
	}

	new_rule = nft_rule_add(h, chain, table, new_rule, r, verbose);
	if (!new_rule)
		goto err;

	if (r)
		nftnl_chain_rule_insert_at(new_rule, r);
	else
		nftnl_chain_rule_add(new_rule, c->nftnl);

	return 1;
err:
	return 0;
}

int nft_rule_delete_num(struct nft_handle *h, const char *chain,
			const char *table, int rulenum, bool verbose)
{
	int ret = 0;
	struct nftnl_rule *r;
	struct nft_chain *c;

	nft_fn = nft_rule_delete_num;

	c = nft_chain_find(h, table, chain);
	if (!c) {
		errno = ENOENT;
		return 0;
	}

	r = nft_rule_find(h, c, NULL, rulenum);
	if (r != NULL) {
		DEBUGP("deleting rule by number %d\n", rulenum);
		ret = __nft_rule_del(h, r);
		if (ret < 0)
			errno = ENOMEM;
	} else
		errno = E2BIG;

	return ret;
}

int nft_rule_replace(struct nft_handle *h, const char *chain,
		     const char *table, struct nftnl_rule *rule,
		     int rulenum, bool verbose)
{
	int ret = 0;
	struct nftnl_rule *r;
	struct nft_chain *c;

	nft_fn = nft_rule_replace;

	c = nft_chain_find(h, table, chain);
	if (!c) {
		errno = ENOENT;
		return 0;
	}

	r = nft_rule_find(h, c, rule, rulenum);
	if (r != NULL) {
		DEBUGP("replacing rule with handle=%llu\n",
			(unsigned long long)
			nftnl_rule_get_u64(r, NFTNL_RULE_HANDLE));

		ret = nft_rule_append(h, chain, table, rule, r, verbose);
	} else
		errno = E2BIG;

	return ret;
}

static int nft_rule_change_counters(struct nft_handle *h, const char *table,
				    const char *chain, struct nftnl_rule *rule,
				    int rulenum, struct xt_counters *counters,
				    uint8_t counter_op, bool verbose)
{
	struct iptables_command_state cs = {};
	struct nftnl_rule *r, *new_rule;
	struct nft_rule_ctx ctx = {
		.command = NFT_COMPAT_RULE_APPEND,
	};
	struct nft_chain *c;

	nft_fn = nft_rule_change_counters;

	c = nft_chain_find(h, table, chain);
	if (!c) {
		errno = ENOENT;
		return 0;
	}

	r = nft_rule_find(h, c, rule, rulenum);
	if (!r) {
		errno = E2BIG;
		return 0;
	}

	DEBUGP("changing counters of rule with handle=%llu\n",
		(unsigned long long)
		nftnl_rule_get_u64(r, NFTNL_RULE_HANDLE));

	if (h->ops->init_cs)
		h->ops->init_cs(&cs);
	h->ops->rule_to_cs(h, r, &cs);

	if (counter_op & CTR_OP_INC_PKTS)
		cs.counters.pcnt += counters->pcnt;
	else if (counter_op & CTR_OP_DEC_PKTS)
		cs.counters.pcnt -= counters->pcnt;
	else
		cs.counters.pcnt = counters->pcnt;

	if (counter_op & CTR_OP_INC_BYTES)
		cs.counters.bcnt += counters->bcnt;
	else if (counter_op & CTR_OP_DEC_BYTES)
		cs.counters.bcnt -= counters->bcnt;
	else
		cs.counters.bcnt = counters->bcnt;

	new_rule = nft_rule_new(h, &ctx, chain, table, &cs);
	h->ops->clear_cs(&cs);

	return nft_rule_append(h, chain, table, new_rule, r, verbose);
}

static int
__nft_rule_list(struct nft_handle *h, struct nftnl_chain *c,
		int rulenum, unsigned int format,
		void (*cb)(struct nft_handle *h, struct nftnl_rule *r,
			   unsigned int num, unsigned int format))
{
	struct nftnl_rule_iter *iter;
	struct nftnl_rule *r;
	int rule_ctr = 0;

	if (rulenum > 0) {
		r = nftnl_rule_lookup_byindex(c, rulenum - 1);
		if (!r)
			/* iptables-legacy returns 0 when listing for
			 * valid chain but invalid rule number
			 */
			return 1;
		cb(h, r, rulenum, format);
		return 1;
	}

	iter = nftnl_rule_iter_create(c);
	if (iter == NULL)
		return 0;

	r = nftnl_rule_iter_next(iter);
	while (r != NULL) {
		cb(h, r, ++rule_ctr, format);
		r = nftnl_rule_iter_next(iter);
	}

	nftnl_rule_iter_destroy(iter);
	return 1;
}

static int nft_rule_count(struct nft_handle *h, struct nftnl_chain *c)
{
	struct nftnl_rule_iter *iter;
	struct nftnl_rule *r;
	int rule_ctr = 0;

	iter = nftnl_rule_iter_create(c);
	if (iter == NULL)
		return 0;

	r = nftnl_rule_iter_next(iter);
	while (r != NULL) {
		rule_ctr++;
		r = nftnl_rule_iter_next(iter);
	}

	nftnl_rule_iter_destroy(iter);
	return rule_ctr;
}

static void __nft_print_header(struct nft_handle *h,
			       struct nft_chain *nc, unsigned int format)
{
	struct nftnl_chain *c = nc->nftnl;
	const char *chain_name = nftnl_chain_get_str(c, NFTNL_CHAIN_NAME);
	uint32_t refs = nftnl_chain_get_u32(c, NFTNL_CHAIN_USE);
	uint32_t entries = nft_rule_count(h, c);
	struct xt_counters ctrs = {
		.pcnt = nftnl_chain_get_u64(c, NFTNL_CHAIN_PACKETS),
		.bcnt = nftnl_chain_get_u64(c, NFTNL_CHAIN_BYTES),
	};
	const char *pname = NULL;

	if (nftnl_chain_get(c, NFTNL_CHAIN_HOOKNUM) &&
	    nftnl_chain_is_set(c, NFTNL_CHAIN_POLICY))
		pname = policy_name[nftnl_chain_get_u32(c, NFTNL_CHAIN_POLICY)];

	h->ops->print_header(format, chain_name, pname,
			     &ctrs, refs - entries, entries);
}

struct nft_rule_list_cb_data {
	struct nft_handle *h;
	unsigned int format;
	int rulenum;
	bool found;
	bool save_fmt;
	void (*cb)(struct nft_handle *h, struct nftnl_rule *r,
		   unsigned int num, unsigned int format);
};

static int nft_rule_list_cb(struct nft_chain *c, void *data)
{
	struct nft_rule_list_cb_data *d = data;

	if (!d->save_fmt) {
		if (d->found)
			printf("\n");
		d->found = true;

		__nft_print_header(d->h, c, d->format);
	}

	return __nft_rule_list(d->h, c->nftnl, d->rulenum, d->format, d->cb);
}

int nft_rule_list(struct nft_handle *h, const char *chain, const char *table,
		  int rulenum, unsigned int format)
{
	const struct nft_family_ops *ops = h->ops;
	struct nft_rule_list_cb_data d = {
		.h = h,
		.format = format,
		.rulenum = rulenum,
		.cb = ops->print_rule,
	};
	struct nft_chain *c;

	nft_xt_fake_builtin_chains(h, table, chain);
	nft_assert_table_compatible(h, table, chain);

	if (chain) {
		c = nft_chain_find(h, table, chain);
		if (!c) {
			errno = ENOENT;
			return 0;
		}

		if (rulenum)
			d.save_fmt = true;	/* skip header printing */
		else if (ops->print_table_header)
			ops->print_table_header(table);

		nft_rule_list_cb(c, &d);
		return 1;
	}

	nft_cache_sort_chains(h, table);

	if (ops->print_table_header)
		ops->print_table_header(table);

	nft_chain_foreach(h, table, nft_rule_list_cb, &d);
	return 1;
}

static void
list_save(struct nft_handle *h, struct nftnl_rule *r,
	  unsigned int num, unsigned int format)
{
	nft_rule_print_save(h, r, NFT_RULE_APPEND, format);
}

int nft_chain_foreach(struct nft_handle *h, const char *table,
		      int (*cb)(struct nft_chain *c, void *data),
		      void *data)
{
	const struct builtin_table *t;
	struct nft_chain_list *list;
	struct nft_chain *c, *c_bak;
	int i, ret;

	t = nft_table_builtin_find(h, table);
	if (!t)
		return -1;

	for (i = 0; i < NF_INET_NUMHOOKS; i++) {
		c = h->cache->table[t->type].base_chains[i];
		if (!c)
			continue;

		ret = cb(c, data);
		if (ret < 0)
			return ret;
	}

	list = h->cache->table[t->type].chains;
	if (!list)
		return -1;

	list_for_each_entry_safe(c, c_bak, &list->list, head) {
		ret = cb(c, data);
		if (ret < 0)
			return ret;
	}
	return 0;
}

static int nft_rule_list_chain_save(struct nft_chain *nc, void *data)
{
	struct nftnl_chain *c = nc->nftnl;
	const char *chain_name = nftnl_chain_get_str(c, NFTNL_CHAIN_NAME);
	uint32_t policy = nftnl_chain_get_u32(c, NFTNL_CHAIN_POLICY);
	int *counters = data;

	if (!nft_chain_builtin(c)) {
		printf("-N %s\n", chain_name);
		return 0;
	}

	/* this is a base chain */

	printf("-P %s %s", chain_name, policy_name[policy]);
	if (*counters)
		printf(" -c %"PRIu64" %"PRIu64,
		       nftnl_chain_get_u64(c, NFTNL_CHAIN_PACKETS),
		       nftnl_chain_get_u64(c, NFTNL_CHAIN_BYTES));
	printf("\n");
	return 0;
}

int nft_rule_list_save(struct nft_handle *h, const char *chain,
		       const char *table, int rulenum, int counters)
{
	struct nft_rule_list_cb_data d = {
		.h = h,
		.rulenum = rulenum,
		.save_fmt = true,
		.cb = list_save,
	};
	struct nft_chain *c;
	int ret = 0;

	nft_xt_fake_builtin_chains(h, table, chain);
	nft_assert_table_compatible(h, table, chain);

	if (counters < 0)
		d.format = FMT_C_COUNTS;
	else if (counters == 0)
		d.format = FMT_NOCOUNTS;

	if (chain) {
		c = nft_chain_find(h, table, chain);
		if (!c) {
			errno = ENOENT;
			return 0;
		}

		if (!rulenum)
			nft_rule_list_chain_save(c, &counters);

		return nft_rule_list_cb(c, &d);
	}

	nft_cache_sort_chains(h, table);

	/* Dump policies and custom chains first */
	nft_chain_foreach(h, table, nft_rule_list_chain_save, &counters);

	/* Now dump out rules in this table */
	ret = nft_chain_foreach(h, table, nft_rule_list_cb, &d);
	return ret == 0 ? 1 : 0;
}

int nft_rule_zero_counters(struct nft_handle *h, const char *chain,
			   const char *table, int rulenum)
{
	struct iptables_command_state cs = {};
	struct nftnl_rule *r, *new_rule;
	struct nft_rule_ctx ctx = {
		.command = NFT_COMPAT_RULE_APPEND,
	};
	struct nft_chain *c;

	nft_fn = nft_rule_delete;

	c = nft_chain_find(h, table, chain);
	if (!c) {
		errno = ENOENT;
		return 0;
	}

	r = nft_rule_find(h, c, NULL, rulenum);
	if (r == NULL) {
		errno = ENOENT;
		return 0;
	}

	if (h->ops->init_cs)
		h->ops->init_cs(&cs);
	h->ops->rule_to_cs(h, r, &cs);
	cs.counters.pcnt = cs.counters.bcnt = 0;
	new_rule = nft_rule_new(h, &ctx, chain, table, &cs);
	h->ops->clear_cs(&cs);

	if (!new_rule)
		return 1;

	return nft_rule_append(h, chain, table, new_rule, r, false);
}

static void nft_table_print_debug(struct nft_handle *h,
				  struct nftnl_table *t, struct nlmsghdr *nlh)
{
	if (h->verbose > 1) {
		nftnl_table_fprintf(stdout, t, 0, 0);
		fprintf(stdout, "\n");
	}
	if (h->verbose > 2)
		mnl_nlmsg_fprintf(stdout, nlh, nlh->nlmsg_len,
				  sizeof(struct nfgenmsg));
}

static void nft_compat_table_batch_add(struct nft_handle *h, uint16_t type,
				       uint16_t flags, uint32_t seq,
				       struct nftnl_table *table)
{
	struct nlmsghdr *nlh;

	nlh = nftnl_nlmsg_build_hdr(nftnl_batch_buffer(h->batch),
				    type, h->family, flags, seq);
	nftnl_table_nlmsg_build_payload(nlh, table);
	nft_table_print_debug(h, table, nlh);
}

static void nft_compat_set_batch_add(struct nft_handle *h, uint16_t type,
				     uint16_t flags, uint32_t seq,
				     struct nftnl_set *set)
{
	struct nlmsghdr *nlh;

	nlh = nftnl_nlmsg_build_hdr(nftnl_batch_buffer(h->batch),
					type, h->family, flags, seq);
	nftnl_set_nlmsg_build_payload(nlh, set);
}

static void nft_compat_setelem_batch_add(struct nft_handle *h, uint16_t type,
					 uint16_t flags, uint32_t *seq,
					 struct nftnl_set *set)
{
	struct nftnl_set_elems_iter *iter;
	struct nlmsghdr *nlh;

	iter = nftnl_set_elems_iter_create(set);
	if (!iter)
		return;

	while (nftnl_set_elems_iter_cur(iter)) {
		(*seq)++;
		mnl_nft_batch_continue(h->batch);
		nlh = nftnl_nlmsg_build_hdr(nftnl_batch_buffer(h->batch),
					    type, h->family, flags, *seq);
		if (nftnl_set_elems_nlmsg_build_payload_iter(nlh, iter) <= 0)
			break;
	}
	nftnl_set_elems_iter_destroy(iter);

	if (h->verbose > 1) {
		fprintf(stdout, "set ");
		nftnl_set_fprintf(stdout, set, 0, 0);
		fprintf(stdout, "\n");
	}
}

static void nft_compat_chain_batch_add(struct nft_handle *h, uint16_t type,
				       uint16_t flags, uint32_t seq,
				       struct nftnl_chain *chain)
{
	struct nlmsghdr *nlh;

	nlh = nftnl_nlmsg_build_hdr(nftnl_batch_buffer(h->batch),
				    type, h->family, flags, seq);
	nftnl_chain_nlmsg_build_payload(nlh, chain);
	nft_chain_print_debug(h, chain, nlh);
}

static void nft_compat_rule_batch_add(struct nft_handle *h, uint16_t type,
				      uint16_t flags, uint32_t seq,
				      struct nftnl_rule *rule)
{
	struct nlmsghdr *nlh;

	nlh = nftnl_nlmsg_build_hdr(nftnl_batch_buffer(h->batch),
				    type, h->family, flags, seq);
	nftnl_rule_nlmsg_build_payload(nlh, rule);
	nft_rule_print_debug(h, rule, nlh);
}

static void batch_obj_del(struct nft_handle *h, struct obj_update *o)
{
	switch (o->type) {
	case NFT_COMPAT_TABLE_ADD:
	case NFT_COMPAT_TABLE_FLUSH:
		nftnl_table_free(o->table);
		break;
	case NFT_COMPAT_CHAIN_ZERO:
	case NFT_COMPAT_CHAIN_USER_ADD:
	case NFT_COMPAT_CHAIN_ADD:
		break;
	case NFT_COMPAT_CHAIN_DEL:
	case NFT_COMPAT_CHAIN_USER_FLUSH:
	case NFT_COMPAT_CHAIN_UPDATE:
	case NFT_COMPAT_CHAIN_RENAME:
		nftnl_chain_free(o->chain);
		break;
	case NFT_COMPAT_RULE_APPEND:
	case NFT_COMPAT_RULE_INSERT:
	case NFT_COMPAT_RULE_REPLACE:
	case NFT_COMPAT_RULE_CHANGE_COUNTERS:
		break;
	case NFT_COMPAT_RULE_DELETE:
	case NFT_COMPAT_RULE_FLUSH:
		nftnl_rule_free(o->rule);
		break;
	case NFT_COMPAT_SET_ADD:
		nftnl_set_free(o->set);
		break;
	case NFT_COMPAT_RULE_LIST:
	case NFT_COMPAT_RULE_CHECK:
	case NFT_COMPAT_CHAIN_RESTORE:
	case NFT_COMPAT_RULE_SAVE:
	case NFT_COMPAT_RULE_ZERO:
	case NFT_COMPAT_BRIDGE_USER_CHAIN_UPDATE:
		assert(0);
		break;
	}
	h->obj_list_num--;
	list_del(&o->head);
	free(o);
}

static void nft_refresh_transaction(struct nft_handle *h)
{
	const char *tablename, *chainname;
	const struct nft_chain *c;
	struct obj_update *n, *tmp;
	bool exists;

	h->error.lineno = 0;

	list_for_each_entry_safe(n, tmp, &h->obj_list, head) {
		switch (n->type) {
		case NFT_COMPAT_TABLE_FLUSH:
			tablename = nftnl_table_get_str(n->table, NFTNL_TABLE_NAME);
			if (!tablename)
				continue;
			exists = nft_table_find(h, tablename);
			if (exists)
				n->skip = 0;
			else
				n->skip = 1;
			break;
		case NFT_COMPAT_CHAIN_USER_ADD:
			tablename = nftnl_chain_get_str(n->chain, NFTNL_CHAIN_TABLE);
			if (!tablename)
				continue;

			chainname = nftnl_chain_get_str(n->chain, NFTNL_CHAIN_NAME);
			if (!chainname)
				continue;

			if (!h->noflush)
				break;

			c = nft_chain_find(h, tablename, chainname);
			if (c) {
				n->skip = 1;
			} else if (!c) {
				n->skip = 0;
			}
			break;
		case NFT_COMPAT_RULE_FLUSH:
			tablename = nftnl_rule_get_str(n->rule, NFTNL_RULE_TABLE);
			if (!tablename)
				continue;

			chainname = nftnl_rule_get_str(n->rule, NFTNL_RULE_CHAIN);
			if (!chainname)
				continue;

			n->skip = !nft_chain_find(h, tablename, chainname);
			break;
		case NFT_COMPAT_CHAIN_DEL:
			if (!nftnl_chain_get(n->chain, NFTNL_CHAIN_HOOKNUM))
				break;
			n->skip = !nft_may_delete_chain(n->chain);
			break;
		case NFT_COMPAT_CHAIN_ZERO:
			tablename = nftnl_chain_get_str(n->chain, NFTNL_CHAIN_TABLE);
			if (!tablename)
				continue;

			chainname = nftnl_chain_get_str(n->chain, NFTNL_CHAIN_NAME);
			if (!chainname)
				continue;

			n->skip = nftnl_chain_is_set(n->chain,
						     NFTNL_CHAIN_HOOKNUM) &&
				  !nft_chain_find(h, tablename, chainname);
			break;
		case NFT_COMPAT_TABLE_ADD:
		case NFT_COMPAT_CHAIN_ADD:
		case NFT_COMPAT_CHAIN_USER_FLUSH:
		case NFT_COMPAT_CHAIN_UPDATE:
		case NFT_COMPAT_CHAIN_RENAME:
		case NFT_COMPAT_RULE_APPEND:
		case NFT_COMPAT_RULE_INSERT:
		case NFT_COMPAT_RULE_REPLACE:
		case NFT_COMPAT_RULE_CHANGE_COUNTERS:
		case NFT_COMPAT_RULE_DELETE:
		case NFT_COMPAT_SET_ADD:
		case NFT_COMPAT_RULE_LIST:
		case NFT_COMPAT_RULE_CHECK:
		case NFT_COMPAT_CHAIN_RESTORE:
		case NFT_COMPAT_RULE_SAVE:
		case NFT_COMPAT_RULE_ZERO:
		case NFT_COMPAT_BRIDGE_USER_CHAIN_UPDATE:
			break;
		}
	}
}

static int nft_action(struct nft_handle *h, int action)
{
	struct obj_update *n, *tmp;
	struct mnl_err *err, *ne;
	unsigned int buflen, i, len;
	bool show_errors = true;
	char errmsg[1024];
	uint32_t seq;
	int ret = 0;

retry:
	seq = 1;
	h->batch = mnl_batch_init();

	mnl_batch_begin(h->batch, h->nft_genid, seq++);
	h->nft_genid++;

	list_for_each_entry(n, &h->obj_list, head) {
		if (n->skip) {
			n->seq = 0;
			continue;
		}

		n->seq = seq++;
		switch (n->type) {
		case NFT_COMPAT_TABLE_ADD:
			nft_compat_table_batch_add(h, NFT_MSG_NEWTABLE,
						   NLM_F_CREATE, n->seq,
						   n->table);
			break;
		case NFT_COMPAT_TABLE_FLUSH:
			nft_compat_table_batch_add(h, NFT_MSG_DELTABLE,
						   0,
						   n->seq, n->table);
			break;
		case NFT_COMPAT_CHAIN_ADD:
		case NFT_COMPAT_CHAIN_ZERO:
			nft_compat_chain_batch_add(h, NFT_MSG_NEWCHAIN,
						   NLM_F_CREATE, n->seq,
						   n->chain);
			break;
		case NFT_COMPAT_CHAIN_USER_ADD:
			nft_compat_chain_batch_add(h, NFT_MSG_NEWCHAIN,
						   NLM_F_EXCL, n->seq,
						   n->chain);
			break;
		case NFT_COMPAT_CHAIN_DEL:
			nft_compat_chain_batch_add(h, NFT_MSG_DELCHAIN,
						   NLM_F_NONREC, n->seq,
						   n->chain);
			break;
		case NFT_COMPAT_CHAIN_USER_FLUSH:
			nft_compat_chain_batch_add(h, NFT_MSG_DELCHAIN,
						   0, n->seq,
						   n->chain);
			break;
		case NFT_COMPAT_CHAIN_UPDATE:
			nft_compat_chain_batch_add(h, NFT_MSG_NEWCHAIN,
						   h->restore ?
						     NLM_F_CREATE : 0,
						   n->seq, n->chain);
			break;
		case NFT_COMPAT_CHAIN_RENAME:
			nft_compat_chain_batch_add(h, NFT_MSG_NEWCHAIN, 0,
						   n->seq, n->chain);
			break;
		case NFT_COMPAT_RULE_APPEND:
			nft_compat_rule_batch_add(h, NFT_MSG_NEWRULE,
						  NLM_F_CREATE | NLM_F_APPEND,
						  n->seq, n->rule);
			break;
		case NFT_COMPAT_RULE_INSERT:
			nft_compat_rule_batch_add(h, NFT_MSG_NEWRULE,
						  NLM_F_CREATE, n->seq,
						  n->rule);
			break;
		case NFT_COMPAT_RULE_REPLACE:
		case NFT_COMPAT_RULE_CHANGE_COUNTERS:
			nft_compat_rule_batch_add(h, NFT_MSG_NEWRULE,
						  NLM_F_CREATE | NLM_F_REPLACE,
						  n->seq, n->rule);
			break;
		case NFT_COMPAT_RULE_DELETE:
		case NFT_COMPAT_RULE_FLUSH:
			nft_compat_rule_batch_add(h, NFT_MSG_DELRULE, 0,
						  n->seq, n->rule);
			break;
		case NFT_COMPAT_SET_ADD:
			nft_compat_set_batch_add(h, NFT_MSG_NEWSET,
						 NLM_F_CREATE, n->seq, n->set);
			nft_compat_setelem_batch_add(h, NFT_MSG_NEWSETELEM,
						     NLM_F_CREATE, &n->seq, n->set);
			seq = n->seq;
			break;
		case NFT_COMPAT_RULE_LIST:
		case NFT_COMPAT_RULE_CHECK:
		case NFT_COMPAT_CHAIN_RESTORE:
		case NFT_COMPAT_RULE_SAVE:
		case NFT_COMPAT_RULE_ZERO:
		case NFT_COMPAT_BRIDGE_USER_CHAIN_UPDATE:
			assert(0);
			return 0;
		}

		mnl_nft_batch_continue(h->batch);
	}

	switch (action) {
	case NFT_COMPAT_COMMIT:
		mnl_batch_end(h->batch, seq++);
		break;
	case NFT_COMPAT_ABORT:
		break;
	}

	errno = 0;
	ret = mnl_batch_talk(h, seq);
	if (ret && errno == ERESTART) {
		nft_rebuild_cache(h);

		nft_refresh_transaction(h);

		list_for_each_entry_safe(err, ne, &h->err_list, head)
			mnl_err_list_free(err);

		mnl_batch_reset(h->batch);
		goto retry;
	}

	i = 0;
	buflen = sizeof(errmsg);

	list_for_each_entry_safe(n, tmp, &h->obj_list, head) {
		list_for_each_entry_safe(err, ne, &h->err_list, head) {
			if (err->seqnum > n->seq)
				break;

			if (err->seqnum == n->seq && show_errors) {
				if (n->error.lineno == 0)
					show_errors = false;
				len = mnl_append_error(h, n, err, errmsg + i, buflen);
				if (len > 0 && len <= buflen) {
					buflen -= len;
					i += len;
				}
			}
			mnl_err_list_free(err);
		}
		batch_obj_del(h, n);
	}

	nft_release_cache(h);
	mnl_batch_reset(h->batch);

	if (i)
		xtables_error(RESOURCE_PROBLEM, "%s", errmsg);

	return ret == 0 ? 1 : 0;
}

static int ebt_add_policy_rule(struct nftnl_chain *c, void *data)
{
	uint32_t policy = nftnl_chain_get_u32(c, NFTNL_CHAIN_POLICY);
	struct iptables_command_state cs = {
		.eb.bitmask = EBT_NOPROTO,
	};
	struct nftnl_udata_buf *udata;
	struct nft_rule_ctx ctx = {
		.command	= NFT_COMPAT_RULE_APPEND,
	};
	struct nft_handle *h = data;
	struct nftnl_rule *r;
	const char *pname;

	if (nftnl_chain_get(c, NFTNL_CHAIN_HOOKNUM))
		return 0; /* ignore base chains */

	if (!nftnl_chain_is_set(c, NFTNL_CHAIN_POLICY))
		return 0;

	nftnl_chain_unset(c, NFTNL_CHAIN_POLICY);

	switch (policy) {
	case NFT_RETURN:
		return 0; /* return policy is default for nft chains */
	case NF_ACCEPT:
		pname = "ACCEPT";
		break;
	case NF_DROP:
		pname = "DROP";
		break;
	default:
		return -1;
	}

	command_jump(&cs, pname);

	r = nft_rule_new(h, &ctx, nftnl_chain_get_str(c, NFTNL_CHAIN_NAME),
			 nftnl_chain_get_str(c, NFTNL_CHAIN_TABLE), &cs);
	ebt_cs_clean(&cs);

	if (!r)
		return -1;

	udata = nftnl_udata_buf_alloc(NFT_USERDATA_MAXLEN);
	if (!udata)
		goto err_free_rule;

	if (!nftnl_udata_put_u32(udata, UDATA_TYPE_EBTABLES_POLICY, 1))
		goto err_free_rule;

	nftnl_rule_set_data(r, NFTNL_RULE_USERDATA,
			    nftnl_udata_buf_data(udata),
			    nftnl_udata_buf_len(udata));
	nftnl_udata_buf_free(udata);

	if (!batch_rule_add(h, NFT_COMPAT_RULE_APPEND, r))
		goto err_free_rule;

	/* add the rule to chain so it is freed later */
	nftnl_chain_rule_add_tail(r, c);

	return 0;
err_free_rule:
	nftnl_rule_free(r);
	return -1;
}

int ebt_set_user_chain_policy(struct nft_handle *h, const char *table,
			      const char *chain, const char *policy)
{
	struct nft_chain *c = nft_chain_find(h, table, chain);
	int pval;

	if (!c)
		return 0;

	if (!strcmp(policy, "DROP"))
		pval = NF_DROP;
	else if (!strcmp(policy, "ACCEPT"))
		pval = NF_ACCEPT;
	else if (!strcmp(policy, "RETURN"))
		pval = NFT_RETURN;
	else
		return 0;

	nftnl_chain_set_u32(c->nftnl, NFTNL_CHAIN_POLICY, pval);
	return 1;
}

static void nft_bridge_commit_prepare(struct nft_handle *h)
{
	const struct builtin_table *t;
	struct nft_chain_list *list;
	struct nft_chain *c;
	int i;

	for (i = 0; i < NFT_TABLE_MAX; i++) {
		t = &h->tables[i];

		if (!t->name)
			continue;

		list = h->cache->table[t->type].chains;
		if (!list)
			continue;

		list_for_each_entry(c, &list->list, head) {
			ebt_add_policy_rule(c->nftnl, h);
		}
	}
}

static void assert_chain_exists(struct nft_handle *h,
				const char *table, const char *chain)
{
	if (chain && !nft_chain_exists(h, table, chain))
		xtables_error(PARAMETER_PROBLEM,
			      "Chain '%s' does not exist", chain);
}

static int nft_prepare(struct nft_handle *h)
{
	struct nft_cmd *cmd, *next;
	int ret = 1;

	nft_cache_build(h);

	list_for_each_entry_safe(cmd, next, &h->cmd_list, head) {
		h->error.lineno = cmd->error.lineno;

		switch (cmd->command) {
		case NFT_COMPAT_TABLE_FLUSH:
			ret = nft_table_flush(h, cmd->table);
			break;
		case NFT_COMPAT_CHAIN_USER_ADD:
			ret = nft_chain_user_add(h, cmd->chain, cmd->table);
			break;
		case NFT_COMPAT_CHAIN_DEL:
			ret = nft_chain_del(h, cmd->chain, cmd->table,
					    cmd->verbose);
			break;
		case NFT_COMPAT_CHAIN_RESTORE:
			ret = nft_chain_restore(h, cmd->chain, cmd->table);
			break;
		case NFT_COMPAT_CHAIN_UPDATE:
			ret = nft_chain_set(h, cmd->table, cmd->chain,
					    cmd->policy, &cmd->counters);
			break;
		case NFT_COMPAT_CHAIN_RENAME:
			ret = nft_chain_user_rename(h, cmd->chain, cmd->table,
						    cmd->rename);
			break;
		case NFT_COMPAT_CHAIN_ZERO:
			ret = nft_chain_zero_counters(h, cmd->chain, cmd->table,
						      cmd->verbose);
			break;
		case NFT_COMPAT_RULE_APPEND:
			assert_chain_exists(h, cmd->table, cmd->jumpto);
			ret = nft_rule_append(h, cmd->chain, cmd->table,
					      cmd->obj.rule, NULL, cmd->verbose);
			break;
		case NFT_COMPAT_RULE_INSERT:
			assert_chain_exists(h, cmd->table, cmd->jumpto);
			ret = nft_rule_insert(h, cmd->chain, cmd->table,
					      cmd->obj.rule, cmd->rulenum,
					      cmd->verbose);
			break;
		case NFT_COMPAT_RULE_REPLACE:
			assert_chain_exists(h, cmd->table, cmd->jumpto);
			ret = nft_rule_replace(h, cmd->chain, cmd->table,
					      cmd->obj.rule, cmd->rulenum,
					      cmd->verbose);
			break;
		case NFT_COMPAT_RULE_DELETE:
			assert_chain_exists(h, cmd->table, cmd->jumpto);
			if (cmd->rulenum >= 0)
				ret = nft_rule_delete_num(h, cmd->chain,
							  cmd->table,
							  cmd->rulenum,
							  cmd->verbose);
			else
				ret = nft_rule_delete(h, cmd->chain, cmd->table,
						      cmd->obj.rule, cmd->verbose);
			break;
		case NFT_COMPAT_RULE_FLUSH:
			ret = nft_rule_flush(h, cmd->chain, cmd->table,
					     cmd->verbose);
			break;
		case NFT_COMPAT_RULE_LIST:
			ret = nft_rule_list(h, cmd->chain, cmd->table,
					    cmd->rulenum, cmd->format);
			break;
		case NFT_COMPAT_RULE_CHECK:
			assert_chain_exists(h, cmd->table, cmd->jumpto);
			ret = nft_rule_check(h, cmd->chain, cmd->table,
					     cmd->obj.rule, cmd->verbose);
			break;
		case NFT_COMPAT_RULE_ZERO:
			ret = nft_rule_zero_counters(h, cmd->chain, cmd->table,
                                                     cmd->rulenum);
			break;
		case NFT_COMPAT_RULE_SAVE:
			ret = nft_rule_list_save(h, cmd->chain, cmd->table,
						 cmd->rulenum,
						 cmd->counters_save);
			break;
		case NFT_COMPAT_BRIDGE_USER_CHAIN_UPDATE:
			ret = ebt_set_user_chain_policy(h, cmd->table,
							cmd->chain, cmd->policy);
			break;
		case NFT_COMPAT_SET_ADD:
			nft_xt_builtin_table_init(h, cmd->table);
			batch_set_add(h, NFT_COMPAT_SET_ADD, cmd->obj.set);
			ret = 1;
			break;
		case NFT_COMPAT_TABLE_ADD:
		case NFT_COMPAT_CHAIN_ADD:
			assert(0);
			return 0;
		case NFT_COMPAT_RULE_CHANGE_COUNTERS:
			ret = nft_rule_change_counters(h, cmd->table,
						       cmd->chain,
						       cmd->obj.rule,
						       cmd->rulenum,
						       &cmd->counters,
						       cmd->counter_op,
						       cmd->verbose);
			break;
		}

		nft_cmd_free(cmd);

		if (ret == 0)
			return 0;
	}

	return 1;
}

int nft_commit(struct nft_handle *h)
{
	if (!nft_prepare(h))
		return 0;

	return nft_action(h, NFT_COMPAT_COMMIT);
}

int nft_bridge_commit(struct nft_handle *h)
{
	if (!nft_prepare(h))
		return 0;

	nft_bridge_commit_prepare(h);

	return nft_action(h, NFT_COMPAT_COMMIT);
}

int nft_abort(struct nft_handle *h)
{
	struct nft_cmd *cmd, *next;

	list_for_each_entry_safe(cmd, next, &h->cmd_list, head)
		nft_cmd_free(cmd);

	return nft_action(h, NFT_COMPAT_ABORT);
}

int nft_compatible_revision(const char *name, uint8_t rev, int opt)
{
	struct mnl_socket *nl;
	char buf[16536];
	struct nlmsghdr *nlh;
	uint32_t portid, seq, type = 0;
	uint32_t pf = AF_INET;
	int ret = 0;

	switch (opt) {
	case IPT_SO_GET_REVISION_MATCH:
		break;
	case IP6T_SO_GET_REVISION_MATCH:
		pf = AF_INET6;
		break;
	case IPT_SO_GET_REVISION_TARGET:
		type = 1;
		break;
	case IP6T_SO_GET_REVISION_TARGET:
		type = 1;
		pf = AF_INET6;
		break;
	default:
		/* No revision support (arp, ebtables), assume latest version ok */
		return 1;
	}

	nlh = mnl_nlmsg_put_header(buf);
	nlh->nlmsg_type = (NFNL_SUBSYS_NFT_COMPAT << 8) | NFNL_MSG_COMPAT_GET;
	nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
	nlh->nlmsg_seq = seq = time(NULL);

	struct nfgenmsg *nfg = mnl_nlmsg_put_extra_header(nlh, sizeof(*nfg));
	nfg->nfgen_family = pf;
	nfg->version = NFNETLINK_V0;
	nfg->res_id = 0;

	mnl_attr_put_strz(nlh, NFTA_COMPAT_NAME, name);
	mnl_attr_put_u32(nlh, NFTA_COMPAT_REV, htonl(rev));
	mnl_attr_put_u32(nlh, NFTA_COMPAT_TYPE, htonl(type));

	DEBUGP("requesting `%s' rev=%d type=%d via nft_compat\n",
		name, rev, type);

	nl = mnl_socket_open(NETLINK_NETFILTER);
	if (nl == NULL)
		return 0;

	if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0)
		goto err;

	portid = mnl_socket_get_portid(nl);

	if (mnl_socket_sendto(nl, nlh, nlh->nlmsg_len) < 0)
		goto err;

	ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	if (ret == -1)
		goto err;

	ret = mnl_cb_run(buf, ret, seq, portid, NULL, NULL);
	if (ret == -1)
		goto err;

err:
	mnl_socket_close(nl);

	/* ignore EPERM and errors for revision 0 -
	 * this is required for printing extension help texts as user, also
	 * helps error messaging on unavailable kernel extension */
	if (ret < 0) {
		if (errno == EPERM)
			return 1;
		if (rev == 0) {
			fprintf(stderr,
				"Warning: Extension %s revision 0 not supported, missing kernel module?\n",
				name);
			return 1;
		}
	}

	return ret < 0 ? 0 : 1;
}

/* Translates errno numbers into more human-readable form than strerror. */
const char *nft_strerror(int err)
{
	unsigned int i;
	static struct table_struct {
		void *fn;
		int err;
		const char *message;
	} table[] =
	  {
	    { nft_chain_del, ENOTEMPTY, "Chain is not empty" },
	    { nft_chain_del, EBUSY, "Directory not empty" },
	    { nft_chain_del, EMLINK,
	      "Can't delete chain with references left" },
	    { nft_chain_user_add, EEXIST, "Chain already exists" },
	    { nft_chain_user_rename, EEXIST, "File exists" },
	    { nft_rule_insert, E2BIG, "Index of insertion too big" },
	    { nft_rule_check, ENOENT, "Bad rule (does a matching rule exist in that chain?)" },
	    { nft_rule_replace, E2BIG, "Index of replacement too big" },
	    { nft_rule_delete_num, E2BIG, "Index of deletion too big" },
/*	    { TC_READ_COUNTER, E2BIG, "Index of counter too big" },
	    { TC_ZERO_COUNTER, E2BIG, "Index of counter too big" }, */
	    /* ENOENT for DELETE probably means no matching rule */
	    { nft_rule_delete, ENOENT,
	      "Bad rule (does a matching rule exist in that chain?)" },
	    { nft_chain_set, ENOENT, "Bad built-in chain name" },
	    { nft_chain_set, EINVAL, "Bad policy name" },
	    { nft_chain_set, ENXIO, "Bad table name" },
	    { NULL, ELOOP, "Loop found in table" },
	    { NULL, EPERM, "Permission denied (you must be root)" },
	    { NULL, 0, "Incompatible with this kernel" },
	    { NULL, ENOPROTOOPT, "iptables who? (do you need to insmod?)" },
	    { NULL, ENOSYS, "Will be implemented real soon.  I promise ;)" },
	    { NULL, ENOMEM, "Memory allocation problem" },
	    { NULL, ENOENT, "No chain/target/match by that name" },
	  };

	for (i = 0; i < ARRAY_SIZE(table); i++) {
		if ((!table[i].fn || table[i].fn == nft_fn)
		    && table[i].err == err)
			return table[i].message;
	}

	return strerror(err);
}

static int l4proto_expr_get_dreg(struct nftnl_expr *e, uint32_t *dregp)
{
	const char *name = nftnl_expr_get_str(e, NFTNL_EXPR_NAME);
	uint32_t poff = offsetof(struct iphdr, protocol);
	uint32_t pbase = NFT_PAYLOAD_NETWORK_HEADER;

	if (!strcmp(name, "payload") &&
	    nftnl_expr_get_u32(e, NFTNL_EXPR_PAYLOAD_BASE) == pbase &&
	    nftnl_expr_get_u32(e, NFTNL_EXPR_PAYLOAD_OFFSET) == poff &&
	    nftnl_expr_get_u32(e, NFTNL_EXPR_PAYLOAD_LEN) == sizeof(uint8_t)) {
		*dregp = nftnl_expr_get_u32(e, NFTNL_EXPR_PAYLOAD_DREG);
		return 0;
	}
	if (!strcmp(name, "meta") &&
	    nftnl_expr_get_u32(e, NFTNL_EXPR_META_KEY) == NFT_META_L4PROTO) {
		*dregp = nftnl_expr_get_u32(e, NFTNL_EXPR_META_DREG);
		return 0;
	}
	return -1;
}

static int recover_rule_compat(struct nftnl_rule *r)
{
	struct nftnl_expr_iter *iter;
	struct nftnl_expr *e;
	uint32_t reg;
	int ret = -1;

	iter = nftnl_expr_iter_create(r);
	if (!iter)
		return -1;

next_expr:
	e = nftnl_expr_iter_next(iter);
	if (!e)
		goto out;

	/* may be 'ip protocol' or 'meta l4proto' with identical RHS */
	if (l4proto_expr_get_dreg(e, &reg) < 0)
		goto next_expr;

	e = nftnl_expr_iter_next(iter);
	if (!e)
		goto out;

	if (strcmp("cmp", nftnl_expr_get_str(e, NFTNL_EXPR_NAME)) ||
	    reg != nftnl_expr_get_u32(e, NFTNL_EXPR_CMP_SREG))
		goto next_expr;

	add_compat(r, nftnl_expr_get_u8(e, NFTNL_EXPR_CMP_DATA),
		   nftnl_expr_get_u32(e, NFTNL_EXPR_CMP_OP) == NFT_CMP_NEQ);
	ret = 0;
out:
	nftnl_expr_iter_destroy(iter);
	return ret;
}

struct chain_zero_data {
	struct nft_handle	*handle;
	bool			verbose;
};

static int __nft_chain_zero_counters(struct nft_chain *nc, void *data)
{
	struct nftnl_chain *c = nc->nftnl;
	struct chain_zero_data *d = data;
	struct nft_handle *h = d->handle;
	struct nftnl_rule_iter *iter;
	struct nftnl_rule *r;
	struct obj_update *o;

	if (d->verbose)
		fprintf(stdout, "Zeroing chain `%s'\n",
		        nftnl_chain_get_str(c, NFTNL_CHAIN_NAME));

	if (nftnl_chain_is_set(c, NFTNL_CHAIN_HOOKNUM)) {
		/* zero base chain counters. */
		nftnl_chain_set_u64(c, NFTNL_CHAIN_PACKETS, 0);
		nftnl_chain_set_u64(c, NFTNL_CHAIN_BYTES, 0);
		nftnl_chain_unset(c, NFTNL_CHAIN_HANDLE);
		o = batch_chain_add(h, NFT_COMPAT_CHAIN_ZERO, c);
		if (!o)
			return -1;
		/* may skip if it is a fake entry */
		o->skip = nc->fake;
	}

	iter = nftnl_rule_iter_create(c);
	if (iter == NULL)
		return -1;

	r = nftnl_rule_iter_next(iter);
	while (r != NULL) {
		struct nftnl_expr_iter *ei;
		struct nftnl_expr *e;
		bool zero_needed;

		ei = nftnl_expr_iter_create(r);
		if (!ei)
			break;

		e = nftnl_expr_iter_next(ei);
	        zero_needed = false;
		while (e != NULL) {
			const char *en = nftnl_expr_get_str(e, NFTNL_EXPR_NAME);

			if (strcmp(en, "counter") == 0 && (
			    nftnl_expr_get_u64(e, NFTNL_EXPR_CTR_PACKETS) ||
			    nftnl_expr_get_u64(e, NFTNL_EXPR_CTR_BYTES))) {
				nftnl_expr_set_u64(e, NFTNL_EXPR_CTR_PACKETS, 0);
				nftnl_expr_set_u64(e, NFTNL_EXPR_CTR_BYTES, 0);
				zero_needed = true;
			}

			e = nftnl_expr_iter_next(ei);
		}

		nftnl_expr_iter_destroy(ei);

		if (zero_needed) {
			/*
			 * Unset RULE_POSITION for older kernels, we want to replace
			 * rule based on its handle only.
			 */
			recover_rule_compat(r);
			nftnl_rule_unset(r, NFTNL_RULE_POSITION);
			if (!batch_rule_add(h, NFT_COMPAT_RULE_REPLACE, r)) {
				nftnl_rule_iter_destroy(iter);
				return -1;
			}
		}
		r = nftnl_rule_iter_next(iter);
	}

	nftnl_rule_iter_destroy(iter);
	return 0;
}

int nft_chain_zero_counters(struct nft_handle *h, const char *chain,
			    const char *table, bool verbose)
{
	struct chain_zero_data d = {
		.handle = h,
		.verbose = verbose,
	};
	struct nft_chain *c;
	int ret = 0;

	nft_xt_fake_builtin_chains(h, table, chain);

	if (chain) {
		c = nft_chain_find(h, table, chain);
		if (!c) {
			errno = ENOENT;
			return 0;
		}

		ret = __nft_chain_zero_counters(c, &d);
		goto err;
	}

	if (verbose)
		nft_cache_sort_chains(h, table);

	ret = nft_chain_foreach(h, table, __nft_chain_zero_counters, &d);
err:
	/* the core expects 1 for success and 0 for error */
	return ret == 0 ? 1 : 0;
}

uint32_t nft_invflags2cmp(uint32_t invflags, uint32_t flag)
{
	if (invflags & flag)
		return NFT_CMP_NEQ;

	return NFT_CMP_EQ;
}

static const char *supported_exprs[] = {
	"match",
	"target",
	"payload",
	"meta",
	"cmp",
	"bitwise",
	"counter",
	"immediate",
	"lookup",
	"range",
};


static int nft_is_expr_compatible(struct nftnl_expr *expr, void *data)
{
	const char *name = nftnl_expr_get_str(expr, NFTNL_EXPR_NAME);
	int i;

	for (i = 0; i < ARRAY_SIZE(supported_exprs); i++) {
		if (strcmp(supported_exprs[i], name) == 0)
			return 0;
	}

	if (!strcmp(name, "limit") &&
	    nftnl_expr_get_u32(expr, NFTNL_EXPR_LIMIT_TYPE) == NFT_LIMIT_PKTS &&
	    nftnl_expr_get_u32(expr, NFTNL_EXPR_LIMIT_FLAGS) == 0)
		return 0;

	if (!strcmp(name, "log") &&
	    nftnl_expr_is_set(expr, NFTNL_EXPR_LOG_GROUP))
		return 0;

	return -1;
}

static int nft_is_rule_compatible(struct nftnl_rule *rule, void *data)
{
	return nftnl_expr_foreach(rule, nft_is_expr_compatible, NULL);
}

static int nft_is_chain_compatible(struct nft_chain *nc, void *data)
{
	struct nftnl_chain *c = nc->nftnl;

	return nftnl_rule_foreach(c, nft_is_rule_compatible, NULL);
}

bool nft_is_table_compatible(struct nft_handle *h,
			     const char *table, const char *chain)
{
	if (chain) {
		struct nft_chain *c = nft_chain_find(h, table, chain);

		return !c || !nft_is_chain_compatible(c, h);
	}

	return !nft_chain_foreach(h, table, nft_is_chain_compatible, h);
}

bool nft_is_table_tainted(struct nft_handle *h, const char *table)
{
	const struct builtin_table *t = nft_table_builtin_find(h, table);

	return t ? h->cache->table[t->type].tainted : false;
}

void nft_assert_table_compatible(struct nft_handle *h,
				 const char *table, const char *chain)
{
	const char *pfx = "", *sfx = "";

	if (nft_is_table_compatible(h, table, chain)) {
		if (nft_is_table_tainted(h, table))
			printf("# Table `%s' contains incompatible base-chains, use 'nft' tool to list them.\n",
			       table);
		return;
	}

	if (chain) {
		pfx = "chain `";
		sfx = "' in ";
	} else {
		chain = "";
	}
	xtables_error(OTHER_PROBLEM,
		      "%s%s%stable `%s' is incompatible, use 'nft' tool.",
		      pfx, chain, sfx, table);
}
