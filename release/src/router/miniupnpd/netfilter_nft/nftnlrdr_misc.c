/* $Id: nftnlrdr_misc.c,v 1.4 2019/06/30 20:00:41 nanard Exp $ */
/*
 * MiniUPnP project
 * http://miniupnp.free.fr/ or https://miniupnp.tuxfamily.org/
 * (c) 2015 Tomofumi Hayashi
 * (c) 2019 Thomas Bernard
 *
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>
#include <string.h>
#include <stddef.h>
#include <syslog.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/queue.h>
#include <errno.h>

#include <linux/netfilter.h>
#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nf_tables.h>
#include <linux/ipv6.h>

#include <libmnl/libmnl.h>
#include <libnftnl/rule.h>
#include <libnftnl/expr.h>

#include "nftnlrdr_misc.h"
#include "../macros.h"
#include "../upnpglobalvars.h"

#ifdef DEBUG
#define d_printf(x) do { printf x; } while (0)
#else
#define d_printf(x)
#endif

#define RULE_CACHE_INVALID  0
#define RULE_CACHE_VALID    1

static struct mnl_socket *nl = NULL;
// FILTER
struct rule_list head_filter = LIST_HEAD_INITIALIZER(head_filter);
// DNAT
struct rule_list head_redirect = LIST_HEAD_INITIALIZER(head_redirect);
// SNAT
struct rule_list head_peer = LIST_HEAD_INITIALIZER(head_peer);

static uint32_t rule_list_filter_validate = RULE_CACHE_INVALID;
static uint32_t rule_list_redirect_validate = RULE_CACHE_INVALID;
static uint32_t rule_list_peer_validate = RULE_CACHE_INVALID;

#ifdef DEBUG
static const char *
get_family_string(uint32_t family)
{
	switch (family) {
	case NFPROTO_INET:
		return "ipv4/6";
	case NFPROTO_IPV4:
		return "ipv4";
	case NFPROTO_IPV6:
		return "ipv6";
	}

	return "unknown family";
}

static const char *
get_proto_string(uint32_t proto)
{
	switch (proto) {
	case IPPROTO_TCP:
		return "tcp";
	case IPPROTO_UDP:
		return "udp";
	}

	return "unknown proto";
}

static const char *
get_verdict_string(uint32_t val)
{
	switch (val) {
	case NF_ACCEPT:
		return "accept";
	case NF_DROP:
		return "drop";
	default:
		return "unknown verdict";
	}
}

void
print_rule(rule_t *r)
{
	struct in_addr addr;
	char *iaddr_str = NULL, *rhost_str = NULL, *eaddr_str = NULL;
	char iaddr6_str[INET6_ADDRSTRLEN];
	char rhost6_str[INET6_ADDRSTRLEN];
	char ifname_buf[IF_NAMESIZE];

	switch (r->type) {
	case RULE_NAT:
		if (r->iaddr != 0) {
			addr.s_addr = r->iaddr;
			iaddr_str = strdupa(inet_ntoa(addr));
		}
		if (r->rhost != 0) {
			addr.s_addr = r->rhost;
			rhost_str = strdupa(inet_ntoa(addr));
		}
		if (r->eaddr != 0) {
			addr.s_addr = r->eaddr;
			eaddr_str = strdupa(inet_ntoa(addr));
		}
		if (r->nat_type == NFT_NAT_DNAT) {
			printf("%"PRIu64":[%s/%s] iif %s, %s/%s, %d -> "
			       "%s:%d (%s)\n",
			       r->handle,
			       r->table, r->chain,
			       if_indextoname(r->ingress_ifidx, ifname_buf),
			       get_family_string(r->family),
			       get_proto_string(r->proto), r->eport,
			       iaddr_str, r->iport,
			       r->desc);
		} else if (r->nat_type == NFT_NAT_SNAT) {
			printf("%"PRIu64":[%s/%s] "
			       "nat type:%d, family:%d, ifidx: %d, "
			       "eaddr: %s, eport:%d, "
			       "proto:%d, iaddr: %s, "
			       "iport:%d, rhost:%s rport:%d (%s)\n",
			       r->handle, r->table, r->chain,
			       r->nat_type, r->family, r->ingress_ifidx,
			       eaddr_str, r->eport,
			       r->proto, iaddr_str, r->iport,
			       rhost_str, r->rport,
			       r->desc);
		} else {
			printf("%"PRIu64":[%s/%s] "
			       "nat type:%d, family:%d, ifidx: %d, "
			       "eaddr: %s, eport:%d, "
			       "proto:%d, iaddr: %s, iport:%d, rhost:%s (%s)\n",
			       r->handle, r->table, r->chain,
			       r->nat_type, r->family, r->ingress_ifidx,
			       eaddr_str, r->eport,
			       r->proto, iaddr_str, r->iport, rhost_str,
			       r->desc);
		}
		break;
	case RULE_FILTER:
		if (r->iaddr != 0) {
			addr.s_addr = r->iaddr;
			iaddr_str = strdupa(inet_ntoa(addr));
		}
		if (r->rhost != 0) {
			addr.s_addr = r->rhost;
			rhost_str = strdupa(inet_ntoa(addr));
		}
		inet_ntop(AF_INET6, &r->iaddr6, iaddr6_str, INET6_ADDRSTRLEN);
		inet_ntop(AF_INET6, &r->rhost6, rhost6_str, INET6_ADDRSTRLEN);

		if ( (r->iaddr != 0) || (r->rhost != 0) ) {
			printf("%"PRIu64":[%s/%s] %s/%s, %s %s:%d: %s (%s)\n",
			       r->handle, r->table, r->chain,
			       get_family_string(r->family), get_proto_string(r->proto),
			       rhost_str,
			       iaddr_str, r->eport,
			       get_verdict_string(r->filter_action),
			       r->desc);
		} else {
			printf("%"PRIu64":[%s/%s] %s/%s, %s %s:%d: %s (%s)\n",
			       r->handle, r->table, r->chain,
			       get_family_string(r->family), get_proto_string(r->proto),
			       rhost6_str,
			       iaddr6_str, r->eport,
			       get_verdict_string(r->filter_action),
			       r->desc);
		}
		break;
	case RULE_COUNTER:
		if (r->iaddr != 0) {
			addr.s_addr = r->iaddr;
			iaddr_str = strdupa(inet_ntoa(addr));
		}
		if (r->rhost != 0) {
			addr.s_addr = r->iaddr;
			rhost_str = strdupa(inet_ntoa(addr));
		}
		printf("%"PRIu64":[%s/%s] %s/%s, %s:%d: "
		       "packets:%"PRIu64", bytes:%"PRIu64"\n",
		       r->handle, r->table, r->chain,
		       get_family_string(r->family), get_proto_string(r->proto),
		       iaddr_str, r->eport, r->packets, r->bytes);
		break;
	default:
		printf("nftables: unknown type: %d\n", r->type);
	}
}
#endif

static enum rule_reg_type *
get_reg_type_ptr(rule_t *r, uint32_t dreg)
{
	switch (dreg) {
	case NFT_REG_1:
		return &r->reg1_type;
	case NFT_REG_2:
		return &r->reg2_type;
	default:
		return NULL;
	}
}

static uint32_t *
get_reg_val_ptr(rule_t *r, uint32_t dreg)
{
	switch (dreg) {
	case NFT_REG_1:
		return &r->reg1_val;
	case NFT_REG_2:
		return &r->reg2_val;
	default:
		return NULL;
	}
}

static void
set_reg (rule_t *r, uint32_t dreg, enum rule_reg_type type, uint32_t val)
{
	if (dreg == NFT_REG_1) {
		r->reg1_type = type;
		if (type == RULE_REG_IMM_VAL) {
			r->reg1_val = val;
		}
	} else if (dreg == NFT_REG_2) {
		r->reg2_type = type;
		if (type == RULE_REG_IMM_VAL) {
			r->reg2_val = val;
		}
	} else if (dreg == NFT_REG_VERDICT) {
		if (r->type == RULE_FILTER) {
			r->filter_action = val;
		}
	} else {
		syslog(LOG_ERR, "%s: unknown reg:%d", "set_reg", dreg);
	}
	return ;
}

static inline void
parse_rule_immediate(struct nftnl_expr *e, rule_t *r)
{
	uint32_t dreg, reg_val, reg_len;

	dreg = nftnl_expr_get_u32(e, NFTNL_EXPR_IMM_DREG);

	if (dreg == NFT_REG_VERDICT) {
		reg_val = nftnl_expr_get_u32(e, NFTNL_EXPR_IMM_VERDICT);
	} else {
		reg_val = *(uint32_t *)nftnl_expr_get(e,
							 NFTNL_EXPR_IMM_DATA,
							 &reg_len);
	}

	set_reg(r, dreg, RULE_REG_IMM_VAL, reg_val);
	return;
}

static inline void
parse_rule_counter(struct nftnl_expr *e, rule_t *r)
{
	r->type = RULE_COUNTER;
	r->bytes = nftnl_expr_get_u64(e, NFTNL_EXPR_CTR_BYTES);
	r->packets = nftnl_expr_get_u64(e, NFTNL_EXPR_CTR_PACKETS);

	return;
}

static inline void
parse_rule_meta(struct nftnl_expr *e, rule_t *r)
{
	uint32_t key = nftnl_expr_get_u32(e, NFTNL_EXPR_META_KEY);
	uint32_t dreg = nftnl_expr_get_u32(e, NFTNL_EXPR_META_DREG);
	enum rule_reg_type reg_type;

	switch (key) {
	case NFT_META_IIF:
		reg_type = RULE_REG_IIF;
		set_reg(r, dreg, reg_type, 0);
		return ;
		
	case NFT_META_OIF:
		reg_type = RULE_REG_IIF;
		set_reg(r, dreg, reg_type, 0);
		return ;
		
	}
	syslog(LOG_DEBUG, "parse_rule_meta :Not support key %d\n", key);

	return;
}

static inline void
parse_rule_nat(struct nftnl_expr *e, rule_t *r)
{
	uint32_t addr_min_reg, addr_max_reg, proto_min_reg, proto_max_reg;
	uint16_t proto_min_val;
	r->type = RULE_NAT;

	r->nat_type = nftnl_expr_get_u32(e, NFTNL_EXPR_NAT_TYPE);
	r->family = nftnl_expr_get_u32(e, NFTNL_EXPR_NAT_FAMILY);
	addr_min_reg = nftnl_expr_get_u32(e, NFTNL_EXPR_NAT_REG_ADDR_MIN);
	addr_max_reg = nftnl_expr_get_u32(e, NFTNL_EXPR_NAT_REG_ADDR_MAX);
	proto_min_reg = nftnl_expr_get_u32(e, NFTNL_EXPR_NAT_REG_PROTO_MIN);
	proto_max_reg = nftnl_expr_get_u32(e, NFTNL_EXPR_NAT_REG_PROTO_MAX);

	if (addr_min_reg != addr_max_reg ||
	    proto_min_reg != proto_max_reg) {
		syslog(LOG_ERR, "Unsupport proto/addr range for NAT");
	}

	proto_min_val = htons((uint16_t)*get_reg_val_ptr(r, proto_min_reg));
	if (r->nat_type == NFT_NAT_DNAT) {
		r->iaddr = (in_addr_t)*get_reg_val_ptr(r, addr_min_reg);
		r->iport = proto_min_val;
	} else if (r->nat_type == NFT_NAT_SNAT) {
		r->eaddr = (in_addr_t)*get_reg_val_ptr(r, addr_min_reg);
		if (proto_min_reg == NFT_REG_1) {
			r->eport = proto_min_val;
		}
	}

	set_reg(r, NFT_REG_1, RULE_REG_NONE, 0);
	set_reg(r, NFT_REG_2, RULE_REG_NONE, 0);
	return;
}

static inline void
parse_rule_payload(struct nftnl_expr *e, rule_t *r)
{
	uint32_t  base, dreg, offset, len;
	uint32_t  *regptr;

	dreg = nftnl_expr_get_u32(e, NFTNL_EXPR_PAYLOAD_DREG);
	base = nftnl_expr_get_u32(e, NFTNL_EXPR_PAYLOAD_BASE);
	offset = nftnl_expr_get_u32(e, NFTNL_EXPR_PAYLOAD_OFFSET);
	len = nftnl_expr_get_u32(e, NFTNL_EXPR_PAYLOAD_LEN);
	regptr = get_reg_type_ptr(r, dreg);

	switch (base) {
	case NFT_PAYLOAD_NETWORK_HEADER:
		if (offset == offsetof(struct iphdr, daddr) &&
		    len == sizeof(in_addr_t)) {
			*regptr = RULE_REG_IP_DEST_ADDR;
			return;
		} else if (offset == offsetof(struct iphdr, saddr) &&
			   len == sizeof(in_addr_t)) {
			*regptr = RULE_REG_IP_SRC_ADDR;
			return;
		} else if (offset == offsetof(struct iphdr, saddr) &&
			   len == sizeof(in_addr_t) * 2) {
			*regptr = RULE_REG_IP_SD_ADDR;
			return;
		} else if (offset == offsetof(struct iphdr, protocol) &&
			   len == sizeof(uint8_t)) {
			*regptr = RULE_REG_IP_PROTO;
			return;
		} else if (offset == offsetof(struct ipv6hdr, nexthdr) &&
			   len == sizeof(uint8_t)) {
			*regptr = RULE_REG_IP6_PROTO;
			return;
		} else if (offset == offsetof(struct ipv6hdr, daddr) &&
		    len == sizeof(struct in6_addr)) {
			*regptr = RULE_REG_IP6_DEST_ADDR;
			return;
		} else if (offset == offsetof(struct ipv6hdr, saddr) &&
			   len == sizeof(struct in6_addr)) {
			*regptr = RULE_REG_IP6_SRC_ADDR;
			return;
		} else if (offset == offsetof(struct ipv6hdr, saddr) &&
			   len == sizeof(struct in6_addr) * 2) {
			*regptr = RULE_REG_IP6_SD_ADDR;
			return;
		}
	case NFT_PAYLOAD_TRANSPORT_HEADER:
		if (offset == offsetof(struct tcphdr, dest) &&
		    len == sizeof(uint16_t)) {
			*regptr = RULE_REG_TCP_DPORT;
			return;
		} else if (offset == offsetof(struct tcphdr, source) &&
			   len == sizeof(uint16_t) * 2) {
			*regptr = RULE_REG_TCP_SD_PORT;
			return;
		}
	}
	syslog(LOG_DEBUG,
	       "Unsupport payload: (dreg:%d, base:%d, offset:%d, len:%d)",
	       dreg, base, offset, len);
	return;
}

/*
 *
 * Note: Currently support only NFT_REG_1
 */
static inline void
parse_rule_cmp(struct nftnl_expr *e, rule_t *r) {
	uint32_t data_len;
	void *data_val;
	uint32_t op, sreg;
	uint16_t *ports;
	in_addr_t *addrp;
	struct in6_addr *addrp6;

	data_val = (void *)nftnl_expr_get(e, NFTNL_EXPR_CMP_DATA, &data_len);
	sreg = nftnl_expr_get_u32(e, NFTNL_EXPR_CMP_SREG);
	op = nftnl_expr_get_u32(e, NFTNL_EXPR_CMP_OP);

	if (sreg != NFT_REG_1) {
		syslog(LOG_ERR, "parse_rule_cmp: Unsupport reg:%d", sreg);
		return;
	}

	switch (r->reg1_type) {
	case RULE_REG_IIF:
		if (data_len == sizeof(uint32_t) && op == NFT_CMP_EQ) {
			r->ingress_ifidx = *(uint32_t *)data_val;
			r->reg1_type = RULE_REG_NONE;
			return;
		}
	case RULE_REG_IP_SRC_ADDR:
		if (data_len == sizeof(in_addr_t) && op == NFT_CMP_EQ) {
			r->rhost = *(in_addr_t *)data_val;
			r->reg1_type = RULE_REG_NONE;
			return;
		}
	case RULE_REG_IP6_SRC_ADDR:
		if (data_len == sizeof(struct in6_addr) && op == NFT_CMP_EQ) {
			r->rhost6 = *(struct in6_addr *)data_val;
			r->reg1_type = RULE_REG_NONE;
			return;
		}
	case RULE_REG_IP_DEST_ADDR:
		if (data_len == sizeof(in_addr_t) && op == NFT_CMP_EQ) {
			if (r->type == RULE_FILTER) {
				r->iaddr = *(in_addr_t *)data_val;
			} else {
				r->rhost = *(in_addr_t *)data_val;
			}
			r->reg1_type = RULE_REG_NONE;
			return;
		}
	case RULE_REG_IP6_DEST_ADDR:
		if (data_len == sizeof(struct in6_addr) && op == NFT_CMP_EQ) {
			if (r->type == RULE_FILTER) {
				r->iaddr6 = *(struct in6_addr *)data_val;
			} else {
				r->rhost6 = *(struct in6_addr *)data_val;
			}
			r->reg1_type = RULE_REG_NONE;
			return;
		}
	case RULE_REG_IP_SD_ADDR:
		if (data_len == sizeof(in_addr_t) * 2 && op == NFT_CMP_EQ) {
			addrp = (in_addr_t *)data_val;
			r->iaddr = addrp[0];
			r->rhost = addrp[1];
			r->reg1_type = RULE_REG_NONE;
			return;
		}
	case RULE_REG_IP6_SD_ADDR:
		if (data_len == sizeof(struct in6_addr) * 2 && op == NFT_CMP_EQ) {
			addrp6 = (struct in6_addr *)data_val;
			r->iaddr6 = addrp6[0];
			r->rhost6 = addrp6[1];
			r->reg1_type = RULE_REG_NONE;
			return;
		}
	case RULE_REG_IP_PROTO:
	case RULE_REG_IP6_PROTO:
		if (data_len == sizeof(uint8_t) && op == NFT_CMP_EQ) {
			r->proto = *(uint8_t *)data_val;
			r->reg1_type = RULE_REG_NONE;
			return;
		}
	case RULE_REG_TCP_DPORT:
		if (data_len == sizeof(uint16_t) && op == NFT_CMP_EQ) {
			r->eport = ntohs(*(uint16_t *)data_val);
			r->reg1_type = RULE_REG_NONE;
			return;
		}
	case RULE_REG_TCP_SD_PORT:
		if (data_len == sizeof(uint16_t) * 2 && op == NFT_CMP_EQ) {
			ports = (uint16_t *)data_val;
			r->eport = ntohs(ports[0]);
			r->rport = ntohs(ports[1]);
			r->reg1_type = RULE_REG_NONE;
			return;
		}
	default:
		break;
	}

	syslog(LOG_DEBUG, "Unknown cmp (r1type:%d, data_len:%d, op:%d)",
	       r->reg1_type, data_len, op);

	return;
}

static int
rule_expr_cb(struct nftnl_expr *e, void *data)
{
	rule_t *r = data;
	const char *attr_name = nftnl_expr_get_str(e, NFTNL_EXPR_NAME);

	if (strncmp("cmp", attr_name, sizeof("cmp")) == 0) {
		parse_rule_cmp(e, r);
	} else if (strncmp("nat", attr_name, sizeof("nat")) == 0) {
		parse_rule_nat(e, r);
	} else if (strncmp("meta", attr_name, sizeof("meta")) == 0) {
		parse_rule_meta(e, r);
	} else if (strncmp("counter", attr_name, sizeof("counter")) == 0) {
		parse_rule_counter(e, r);
	} else if (strncmp("payload", attr_name, sizeof("payload")) == 0) {
		parse_rule_payload(e, r);
	} else if (strncmp("immediate", attr_name, sizeof("immediate")) == 0) {
		parse_rule_immediate(e, r);
	} else {
		syslog(LOG_DEBUG, "unknown attr: %s\n", attr_name);
	}

	return MNL_CB_OK;
}


static int
table_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nftnl_rule *t;
	uint32_t len;
	struct nftnl_expr *expr;
	struct nftnl_expr_iter *itr;
	rule_t *r;
	char *chain;
	char *descr;
	int index_filter, index_peer, index_redirect;
	UNUSED(data);

	index_filter = index_peer = index_redirect = 0;

	r = malloc(sizeof(rule_t));

	memset(r, 0, sizeof(rule_t));
	t = nftnl_rule_alloc();
	if (t == NULL) {
		syslog(LOG_ERR, "nftnl_rule_alloc() FAILED");
		goto err;
	}

	if (nftnl_rule_nlmsg_parse(nlh, t) < 0) {
		syslog(LOG_ERR, "nftnl_rule_nlmsg_parse FAILED");
		goto err_free;
	}

	chain = (char *)nftnl_rule_get_data(t, NFTNL_RULE_CHAIN, &len);
	if (strcmp(chain, miniupnpd_nat_chain) != 0 &&
	    strcmp(chain, miniupnpd_nat_postrouting_chain) != 0 &&
	    strcmp(chain, miniupnpd_forward_chain) != 0) {
		goto rule_skip;
	}

	r->table = strdup(
		(char *)nftnl_rule_get_data(t, NFTNL_RULE_TABLE, &len));
	r->chain = strdup(chain);
	r->family = *(uint32_t*)nftnl_rule_get_data(t, NFTNL_RULE_FAMILY,
						       &len);
	descr = (char *)nftnl_rule_get_data(t, NFTNL_RULE_USERDATA,
						 &r->desc_len);
	if (r->desc_len > 0)
		r->desc = strdup(descr);

	r->handle = *(uint32_t*)nftnl_rule_get_data(t,
						       NFTNL_RULE_HANDLE,
						       &len);
	if (strcmp(r->table, NFT_TABLE_NAT) == 0) {
		r->type = RULE_NAT;
	} else if (strcmp(r->table, NFT_TABLE_FILTER) == 0) {
		r->type = RULE_FILTER;
	}

	itr = nftnl_expr_iter_create(t);

	while ((expr = nftnl_expr_iter_next(itr)) != NULL) {
		rule_expr_cb(expr, r);
	}

	if (r->type == RULE_NONE) {
		free(r);
	} else if (r->type == RULE_NAT && r->nat_type == NFT_NAT_SNAT) {
		r->index = index_peer;
		LIST_INSERT_HEAD(&head_peer, r, entry);
		index_peer++;
	} else if (r->type == RULE_NAT && r->nat_type == NFT_NAT_DNAT) {
		r->index = index_redirect;
		LIST_INSERT_HEAD(&head_redirect, r, entry);
		index_redirect++;
	} else if (r->type == RULE_FILTER) {
		r->index = index_filter;
		LIST_INSERT_HEAD(&head_filter, r, entry);
		index_filter++;
	}

rule_skip:
err_free:
	nftnl_rule_free(t);
err:
	return MNL_CB_OK;
}

void
reflesh_nft_cache_filter()
{

	if (rule_list_filter_validate == RULE_CACHE_VALID) {
		return;
	}

	reflesh_nft_cache(&head_filter, NFT_TABLE_FILTER, miniupnpd_forward_chain, NFPROTO_INET);

	rule_list_filter_validate = RULE_CACHE_VALID;

	return;
}

void
reflesh_nft_cache_peer()
{
	if (rule_list_peer_validate == RULE_CACHE_VALID) {
		return;
	}

	reflesh_nft_cache(&head_peer, NFT_TABLE_NAT, miniupnpd_nat_postrouting_chain, NFPROTO_IPV4);

	rule_list_peer_validate = RULE_CACHE_VALID;

	return;
}

void
reflesh_nft_cache_redirect()
{
	if (rule_list_redirect_validate == RULE_CACHE_VALID) {
		return;
	}

	reflesh_nft_cache(&head_redirect, NFT_TABLE_NAT, miniupnpd_nat_chain, NFPROTO_IPV4);

	rule_list_redirect_validate = RULE_CACHE_VALID;

	return;
}

void
reflesh_nft_cache(struct rule_list *head, char *table, const char *chain, uint32_t family)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	uint32_t portid, seq, type = NFTNL_OUTPUT_DEFAULT;
	struct nftnl_rule *t;
	rule_t *p1, *p2;
	int ret;

	t = NULL;
	p1 = LIST_FIRST(head);
	if (p1 != NULL) {
		while(p1 != NULL) {
			p2 = (rule_t *)LIST_NEXT(p1, entry);
			if (p1->desc != NULL) {
				free(p1->desc);
			}
			if (p1->table != NULL) {
				free(p1->table);
			}
			if (p1->chain != NULL) {
				free(p1->chain);
			}
			free(p1);
			p1 = p2;
		}
	}
	LIST_INIT(head);

	if (nl == NULL) {
		nl = mnl_socket_open(NETLINK_NETFILTER);
		if (nl == NULL) {
			syslog(LOG_ERR, "%s: mnl_socket_open() FAILED: %m", "reflesh_nft_cache()");
			return;
		}

		if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0) {
			syslog(LOG_ERR, "%s: mnl_socket_bind() FAILED: %m", "reflesh_nft_cache()");
			return;
		}
	}
	portid = mnl_socket_get_portid(nl);

	t = nftnl_rule_alloc();
	if (t == NULL) {
		syslog(LOG_ERR, "%s: nftnl_rule_alloc() FAILED", "reflesh_nft_cache()");
		return;
	}

	seq = time(NULL);
	nlh = nftnl_rule_nlmsg_build_hdr(buf, NFT_MSG_GETRULE, family,
					NLM_F_DUMP, seq);
	nftnl_rule_set(t, NFTNL_RULE_TABLE, table);
	nftnl_rule_set(t, NFTNL_RULE_CHAIN, chain);
	nftnl_rule_nlmsg_build_payload(nlh, t);
	nftnl_rule_free(t);

	if (mnl_socket_sendto(nl, nlh, nlh->nlmsg_len) < 0) {
		syslog(LOG_ERR, "%s: mnl_socket_sendto() FAILED: %m", "reflesh_nft_cache()");
		return;
	}

	ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	while (ret > 0) {
		ret = mnl_cb_run(buf, ret, seq, portid, table_cb, &type);
		if (ret <= 0)
			break;
		ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	}

	if (ret == -1) {
		syslog(LOG_ERR, "%s: mnl_socket_recvfrom() FAILED: %m", "reflesh_nft_cache()");
	}

	/* mnl_socket_close(nl); */

	return;
}

static void
expr_add_payload(struct nftnl_rule *r, uint32_t base, uint32_t dreg,
                 uint32_t offset, uint32_t len)
{
	struct nftnl_expr *e;

	e = nftnl_expr_alloc("payload");
	if (e == NULL) {
		syslog(LOG_ERR, "%s: nftnl_expr_alloc(\"%s\") FAILED", "expr_add_payload()", "payload");
		return;
	}

	nftnl_expr_set_u32(e, NFTNL_EXPR_PAYLOAD_BASE, base);
	nftnl_expr_set_u32(e, NFTNL_EXPR_PAYLOAD_DREG, dreg);
	nftnl_expr_set_u32(e, NFTNL_EXPR_PAYLOAD_OFFSET, offset);
	nftnl_expr_set_u32(e, NFTNL_EXPR_PAYLOAD_LEN, len);

	nftnl_rule_add_expr(r, e);
}

#if 0
static void
expr_add_bitwise(struct nftnl_rule *r, uint32_t sreg, uint32_t dreg,
		 uint32_t len, uint32_t mask, uint32_t xor)
{
	struct nftnl_expr *e;

	e = nftnl_expr_alloc("bitwise");
	if (e == NULL) {
		syslog(LOG_ERR, "%s: nftnl_expr_alloc(\"%s\") FAILED", "expr_add_bitwise()", "bitwise");
		return;
	}

	nftnl_expr_set_u32(e, NFTNL_EXPR_BITWISE_SREG, sreg);
	nftnl_expr_set_u32(e, NFTNL_EXPR_BITWISE_DREG, dreg);
	nftnl_expr_set_u32(e, NFTNL_EXPR_BITWISE_LEN, len);
	nftnl_expr_set(e, NFTNL_EXPR_BITWISE_MASK, &mask, sizeof(mask));
	nftnl_expr_set(e, NFTNL_EXPR_BITWISE_XOR, &xor, sizeof(xor));

	nftnl_rule_add_expr(r, e);
}
#endif

static void
expr_add_cmp(struct nftnl_rule *r, uint32_t sreg, uint32_t op,
	     const void *data, uint32_t data_len)
{
	struct nftnl_expr *e;

	e = nftnl_expr_alloc("cmp");
	if (e == NULL) {
		syslog(LOG_ERR, "%s: nftnl_expr_alloc(\"%s\") FAILED", "expr_add_cmp()", "cmp");
		return;
	}

	nftnl_expr_set_u32(e, NFTNL_EXPR_CMP_SREG, sreg);
	nftnl_expr_set_u32(e, NFTNL_EXPR_CMP_OP, op);
	nftnl_expr_set(e, NFTNL_EXPR_CMP_DATA, data, data_len);

	nftnl_rule_add_expr(r, e);
}

static void
expr_add_meta(struct nftnl_rule *r, uint32_t meta_key, uint32_t dreg)
{
	struct nftnl_expr *e;

	e = nftnl_expr_alloc("meta");
	if (e == NULL) {
		syslog(LOG_ERR, "%s: nftnl_expr_alloc(\"%s\") FAILED", "expr_add_meta()", "meta");
		return;
	}

	nftnl_expr_set_u32(e, NFTNL_EXPR_META_KEY, meta_key);
	nftnl_expr_set_u32(e, NFTNL_EXPR_META_DREG, dreg);

	nftnl_rule_add_expr(r, e);
}

static void
expr_set_reg_val_u32(struct nftnl_rule *r, enum nft_registers dreg, uint32_t val)
{
	struct nftnl_expr *e;
	e = nftnl_expr_alloc("immediate");
	if (e == NULL) {
		syslog(LOG_ERR, "%s: nftnl_expr_alloc(\"%s\") FAILED", "expr_set_reg_val_u32()", "immediate");
		return;
	}
	nftnl_expr_set_u32(e, NFTNL_EXPR_IMM_DREG, dreg);
	nftnl_expr_set_u32(e, NFTNL_EXPR_IMM_DATA, val);
	nftnl_rule_add_expr(r, e);
}

static void
expr_set_reg_val_u16(struct nftnl_rule *r, enum nft_registers dreg, uint32_t val)
{
	struct nftnl_expr *e;
	e = nftnl_expr_alloc("immediate");
	if (e == NULL) {
		syslog(LOG_ERR, "%s: nftnl_expr_alloc(\"%s\") FAILED", "expr_set_reg_val_u16()", "immediate");
		return;
	}
	nftnl_expr_set_u32(e, NFTNL_EXPR_IMM_DREG, dreg);
	nftnl_expr_set_u16(e, NFTNL_EXPR_IMM_DATA, val);
	nftnl_rule_add_expr(r, e);
}

static void
expr_set_reg_verdict(struct nftnl_rule *r, uint32_t val)
{
	struct nftnl_expr *e;
	e = nftnl_expr_alloc("immediate");
	if (e == NULL) {
		syslog(LOG_ERR, "%s: nftnl_expr_alloc(\"%s\") FAILED", "expr_set_reg_verdict()", "immediate");
		return;
	}
	nftnl_expr_set_u32(e, NFTNL_EXPR_IMM_DREG, NFT_REG_VERDICT);
	nftnl_expr_set_u32(e, NFTNL_EXPR_IMM_VERDICT, val);
	nftnl_rule_add_expr(r, e);
}

static void
expr_add_nat(struct nftnl_rule *r, uint32_t t, uint32_t family,
	     in_addr_t addr_min, uint32_t proto_min, uint32_t flags)
{
	struct nftnl_expr *e;
	UNUSED(flags);

	e = nftnl_expr_alloc("nat");
	if (e == NULL) {
		syslog(LOG_ERR, "%s: nftnl_expr_alloc(\"%s\") FAILED", "expr_add_nat()", "nat");
		return;
	}
	
	nftnl_expr_set_u32(e, NFTNL_EXPR_NAT_TYPE, t);
	nftnl_expr_set_u32(e, NFTNL_EXPR_NAT_FAMILY, family);

	/* To IP Address */
	expr_set_reg_val_u32(r, NFT_REG_1, (uint32_t)addr_min);
	nftnl_expr_set_u32(e, NFTNL_EXPR_NAT_REG_ADDR_MIN, NFT_REG_1);
	nftnl_expr_set_u32(e, NFTNL_EXPR_NAT_REG_ADDR_MAX, NFT_REG_1);
	/* To Port */
	expr_set_reg_val_u16(r, NFT_REG_2, proto_min);
	nftnl_expr_set_u32(e, NFTNL_EXPR_NAT_REG_PROTO_MIN, NFT_REG_2);
	nftnl_expr_set_u32(e, NFTNL_EXPR_NAT_REG_PROTO_MAX, NFT_REG_2);

	nftnl_rule_add_expr(r, e);
}

struct nftnl_rule *
rule_set_snat(uint8_t family, uint8_t proto,
	      in_addr_t rhost, unsigned short rport,
	      in_addr_t ehost, unsigned short eport,
	      in_addr_t ihost, unsigned short iport,
	      const char *descr,
	      const char *handle)
{
	struct nftnl_rule *r = NULL;
	uint16_t dport, sport;
	uint32_t descr_len;
	#ifdef DEBUG
	char buf[8192];
	#endif
	UNUSED(handle);

	r = nftnl_rule_alloc();
	if (r == NULL) {
		syslog(LOG_ERR, "nftnl_rule_alloc() FAILED");
		return NULL;
	}

	nftnl_rule_set(r, NFTNL_RULE_TABLE, NFT_TABLE_NAT);
	nftnl_rule_set(r, NFTNL_RULE_CHAIN, miniupnpd_nat_postrouting_chain);
	nftnl_rule_set_u32(r, NFTNL_RULE_FAMILY, family);

	if (descr != NULL) {
		descr_len = strlen(descr);
		nftnl_rule_set_data(r, NFTNL_RULE_USERDATA,
				       descr, descr_len);
	}

	/* Destination IP */
	expr_add_payload(r, NFT_PAYLOAD_NETWORK_HEADER, NFT_REG_1,
	                 offsetof(struct iphdr, daddr), sizeof(uint32_t));
	expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &ihost, sizeof(uint32_t));

	/* Source IP */
	expr_add_payload(r, NFT_PAYLOAD_NETWORK_HEADER, NFT_REG_1,
	                 offsetof(struct iphdr, saddr), sizeof(in_addr_t));
	expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &rhost, sizeof(in_addr_t));

	/* Protocol */
	expr_add_payload(r, NFT_PAYLOAD_NETWORK_HEADER, NFT_REG_1,
	                 offsetof(struct iphdr, protocol), sizeof(uint8_t));
	expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &proto, sizeof(uint8_t));

	/* Source and Destination Port of Protocol */
	if (proto == IPPROTO_TCP) {
		/* Destination Port */
		dport = htons(iport);
		expr_add_payload(r, NFT_PAYLOAD_TRANSPORT_HEADER, NFT_REG_1,
		                 offsetof(struct tcphdr, dest), sizeof(uint16_t));
		expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &dport, sizeof(uint16_t));

		/* Source Port */
		sport = htons(rport);
		expr_add_payload(r, NFT_PAYLOAD_TRANSPORT_HEADER, NFT_REG_1,
		                 offsetof(struct tcphdr, source), sizeof(uint16_t));
		expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &sport, sizeof(uint16_t));
	} else if (proto == IPPROTO_UDP) {
		/* Destination Port */
		dport = htons(iport);
		expr_add_payload(r, NFT_PAYLOAD_TRANSPORT_HEADER, NFT_REG_1,
		                 offsetof(struct udphdr, dest), sizeof(uint16_t));
		expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &dport, sizeof(uint16_t));

		/* Source Port */
		sport = htons(rport);
		expr_add_payload(r, NFT_PAYLOAD_TRANSPORT_HEADER, NFT_REG_1,
		                 offsetof(struct udphdr, source), sizeof(uint16_t));
		expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &sport, sizeof(uint16_t));
	}

	expr_add_nat(r, NFT_NAT_SNAT, family, ehost, htons(eport), 0);

	#ifdef DEBUG
	nftnl_rule_snprintf(buf, sizeof(buf), r, NFTNL_OUTPUT_DEFAULT, 0);
	fprintf(stdout, "%s\n", buf);
	#endif

	return r;
}

struct nftnl_rule *
rule_set_dnat(uint8_t family, const char * ifname, uint8_t proto,
	      in_addr_t rhost, unsigned short eport,
	      in_addr_t ihost, uint32_t iport,
	      const char *descr,
	      const char *handle)
{
	struct nftnl_rule *r = NULL;
	uint16_t dport;
	uint64_t handle_num;
	uint32_t if_idx;
	uint32_t descr_len;
	#ifdef DEBUG
	char buf[8192];
	#endif

	UNUSED(handle);

	r = nftnl_rule_alloc();
	if (r == NULL) {
		syslog(LOG_ERR, "nftnl_rule_alloc() FAILED");
		return NULL;
	}

	nftnl_rule_set(r, NFTNL_RULE_TABLE, NFT_TABLE_NAT);
	nftnl_rule_set(r, NFTNL_RULE_CHAIN, miniupnpd_nat_chain);
	nftnl_rule_set_u32(r, NFTNL_RULE_FAMILY, family);

	if (descr != NULL) {
		descr_len = strlen(descr);
		nftnl_rule_set_data(r, NFTNL_RULE_USERDATA,
				       descr, descr_len);
	}

	if (handle != NULL) {
		handle_num = atoll(handle);
		nftnl_rule_set_u64(r, NFTNL_RULE_POSITION, handle_num);
	}

	if (ifname != NULL) {
		if_idx = (uint32_t)if_nametoindex(ifname);
		expr_add_meta(r, NFT_META_IIF, NFT_REG_1);
		expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &if_idx,
			     sizeof(uint32_t));
	}

	/* Source IP */
	if (rhost != 0) {
		expr_add_payload(r, NFT_PAYLOAD_NETWORK_HEADER, NFT_REG_1,
		                 offsetof(struct iphdr, saddr), sizeof(in_addr_t));
		expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &rhost, sizeof(in_addr_t));
	}

	/* Protocol */
	expr_add_payload(r, NFT_PAYLOAD_NETWORK_HEADER, NFT_REG_1,
	                 offsetof(struct iphdr, protocol), sizeof(uint8_t));
	expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &proto, sizeof(uint8_t));

	if (proto == IPPROTO_TCP) {
		dport = htons(eport);
		expr_add_payload(r, NFT_PAYLOAD_TRANSPORT_HEADER, NFT_REG_1,
		                 offsetof(struct tcphdr, dest), sizeof(uint16_t));
		expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &dport, sizeof(uint16_t));
	} else if (proto == IPPROTO_UDP) {
		dport = htons(eport);
		expr_add_payload(r, NFT_PAYLOAD_TRANSPORT_HEADER, NFT_REG_1,
		                 offsetof(struct udphdr, dest), sizeof(uint16_t));
		expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &dport, sizeof(uint16_t));
	}

	expr_add_nat(r, NFT_NAT_DNAT, family, ihost, htons(iport), 0);

	#ifdef DEBUG
	nftnl_rule_snprintf(buf, sizeof(buf), r, NFTNL_OUTPUT_DEFAULT, 0);
	fprintf(stdout, "%s\n", buf);
	#endif

	return r;
}

struct nftnl_rule *
rule_set_filter(uint8_t family, const char * ifname, uint8_t proto,
		in_addr_t rhost, in_addr_t iaddr,
		unsigned short eport, unsigned short iport,
		unsigned short rport, const char *descr, const char *handle)
{
	struct nftnl_rule *r = NULL;
	#ifdef DEBUG
	char buf[8192];
	#endif
	UNUSED(eport);

	r = nftnl_rule_alloc();
	if (r == NULL) {
		syslog(LOG_ERR, "nftnl_rule_alloc() FAILED");
		return NULL;
	}

	r = rule_set_filter_common(r, family, ifname, proto, eport, iport, rport, descr, handle);

	/* Destination IP */
	expr_add_payload(r, NFT_PAYLOAD_NETWORK_HEADER, NFT_REG_1,
	                 offsetof(struct iphdr, daddr), sizeof(uint32_t));
	expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &iaddr, sizeof(uint32_t));

	/* Source IP */
	if (rhost != 0) {
		expr_add_payload(r, NFT_PAYLOAD_NETWORK_HEADER, NFT_REG_1,
		                 offsetof(struct iphdr, saddr), sizeof(in_addr_t));
		expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &rhost,
			     sizeof(in_addr_t));
	}

	/* Protocol */
	expr_add_payload(r, NFT_PAYLOAD_NETWORK_HEADER, NFT_REG_1,
	                 offsetof(struct iphdr, protocol), sizeof(uint8_t));
	expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &proto, sizeof(uint8_t));

	expr_set_reg_verdict(r, NF_ACCEPT);

	#ifdef DEBUG
	nftnl_rule_snprintf(buf, sizeof(buf), r, NFTNL_OUTPUT_DEFAULT, 0);
	fprintf(stdout, "%s\n", buf);
	#endif

	return r;
}

struct nftnl_rule *
rule_set_filter6(uint8_t family, const char * ifname, uint8_t proto,
		struct in6_addr *rhost6, struct in6_addr *iaddr6,
		unsigned short eport, unsigned short iport,
		unsigned short rport, const char *descr, const char *handle)
{
	struct nftnl_rule *r = NULL;
	#ifdef DEBUG
	char buf[8192];
	#endif
	UNUSED(eport);

	r = nftnl_rule_alloc();
	if (r == NULL) {
		syslog(LOG_ERR, "nftnl_rule_alloc() FAILED");
		return NULL;
	}

	r = rule_set_filter_common(r, family, ifname, proto, eport, iport, rport, descr, handle);

	/* Destination IP */
	expr_add_payload(r, NFT_PAYLOAD_NETWORK_HEADER, NFT_REG_1,
	                 offsetof(struct ipv6hdr, daddr), sizeof(struct in6_addr));
	expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, iaddr6, sizeof(struct in6_addr));

	/* Source IP */
	if (rhost6) {
		expr_add_payload(r, NFT_PAYLOAD_NETWORK_HEADER, NFT_REG_1,
		                 offsetof(struct ipv6hdr, saddr), sizeof(struct in6_addr));
		expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, rhost6, sizeof(struct in6_addr));
	}

	/* Protocol */
	expr_add_payload(r, NFT_PAYLOAD_NETWORK_HEADER, NFT_REG_1,
	                 offsetof(struct ipv6hdr, nexthdr), sizeof(uint8_t));
	expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &proto, sizeof(uint8_t));

	expr_set_reg_verdict(r, NF_ACCEPT);

	#ifdef DEBUG
	nftnl_rule_snprintf(buf, sizeof(buf), r, NFTNL_OUTPUT_DEFAULT, 0);
	fprintf(stdout, "%s\n", buf);
	#endif

	return r;
}

struct nftnl_rule *
rule_set_filter_common(struct nftnl_rule *r, uint8_t family, const char * ifname,
		uint8_t proto, unsigned short eport, unsigned short iport,
		unsigned short rport, const char *descr, const char *handle)
{
	uint16_t dport, sport;
	uint64_t handle_num;
	uint32_t if_idx;
	uint32_t descr_len;
	UNUSED(eport);

	nftnl_rule_set(r, NFTNL_RULE_TABLE, NFT_TABLE_FILTER);
	nftnl_rule_set(r, NFTNL_RULE_CHAIN, miniupnpd_forward_chain);
	nftnl_rule_set_u32(r, NFTNL_RULE_FAMILY, family);

	if (descr != NULL) {
		descr_len = strlen(descr);
		nftnl_rule_set_data(r, NFTNL_RULE_USERDATA,
				       descr, descr_len);
	}

	if (handle != NULL) {
		handle_num = atoll(handle);
		nftnl_rule_set_u64(r, NFTNL_RULE_POSITION, handle_num);
	}

	if (ifname != NULL) {
		if_idx = (uint32_t)if_nametoindex(ifname);
		expr_add_meta(r, NFT_META_IIF, NFT_REG_1);
		expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &if_idx,
			     sizeof(uint32_t));
	}

	/* Destination Port */
	dport = htons(iport);
	if (proto == IPPROTO_TCP) {
		expr_add_payload(r, NFT_PAYLOAD_TRANSPORT_HEADER, NFT_REG_1,
		                 offsetof(struct tcphdr, dest), sizeof(uint16_t));
	} else if (proto == IPPROTO_UDP) {
		expr_add_payload(r, NFT_PAYLOAD_TRANSPORT_HEADER, NFT_REG_1,
		                 offsetof(struct udphdr, dest), sizeof(uint16_t));
	}
	expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &dport, sizeof(uint16_t));

	/* Source Port */
	if (rport != 0) {
		sport = htons(rport);
		if (proto == IPPROTO_TCP) {
			expr_add_payload(r, NFT_PAYLOAD_TRANSPORT_HEADER, NFT_REG_1,
			                 offsetof(struct tcphdr, source), sizeof(uint16_t));
		} else if (proto == IPPROTO_UDP) {
			expr_add_payload(r, NFT_PAYLOAD_TRANSPORT_HEADER, NFT_REG_1,
			                 offsetof(struct udphdr, source), sizeof(uint16_t));
		}
		expr_add_cmp(r, NFT_REG_1, NFT_CMP_EQ, &sport, sizeof(uint16_t));
	}

	return r;
}

struct nftnl_rule *
rule_del_handle(rule_t *rule)
{
	struct nftnl_rule *r = NULL;

	r = nftnl_rule_alloc();
	if (r == NULL) {
		syslog(LOG_ERR, "nftnl_rule_alloc() FAILED");
		return NULL;
	}

	nftnl_rule_set(r, NFTNL_RULE_TABLE, rule->table);
	nftnl_rule_set(r, NFTNL_RULE_CHAIN, rule->chain);
	nftnl_rule_set_u32(r, NFTNL_RULE_FAMILY, rule->family);
	nftnl_rule_set_u64(r, NFTNL_RULE_HANDLE, rule->handle);

	return r;
}

static void
nft_mnl_batch_put(char *buf, uint16_t type, uint32_t seq)
{
	struct nlmsghdr *nlh;
	struct nfgenmsg *nfg;

	nlh = mnl_nlmsg_put_header(buf);
	nlh->nlmsg_type = type;
	nlh->nlmsg_flags = NLM_F_REQUEST;
	nlh->nlmsg_seq = seq;

	nfg = mnl_nlmsg_put_extra_header(nlh, sizeof(*nfg));
	nfg->nfgen_family = AF_INET;
	nfg->version = NFNETLINK_V0;
	nfg->res_id = NFNL_SUBSYS_NFTABLES;
}

int
nft_send_request(struct nftnl_rule * rule, uint16_t cmd, enum rule_chain_type chain_type)
{
	struct nlmsghdr *nlh;
	struct mnl_nlmsg_batch *batch;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	uint32_t seq = time(NULL);
	int ret;

	if (chain_type == RULE_CHAIN_FILTER)
		rule_list_filter_validate = RULE_CACHE_INVALID;
	else if (chain_type == RULE_CHAIN_PEER)
		rule_list_peer_validate = RULE_CACHE_INVALID;
	else if (chain_type == RULE_CHAIN_REDIRECT)
		rule_list_redirect_validate = RULE_CACHE_INVALID;

	if (nl == NULL) {
		nl = mnl_socket_open(NETLINK_NETFILTER);
		if (nl == NULL) {
			syslog(LOG_ERR, "%s: mnl_socket_open() FAILED: %m", "nft_send_request()");
			return -1;
		}

		if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0) {
			syslog(LOG_ERR, "%s: mnl_socket_bind() FAILED: %m", "nft_send_request()");
			return -1;
		}
	}

	batch = mnl_nlmsg_batch_start(buf, sizeof(buf));

	nft_mnl_batch_put(mnl_nlmsg_batch_current(batch),
			  NFNL_MSG_BATCH_BEGIN, seq++);
	mnl_nlmsg_batch_next(batch);

	nlh = nftnl_rule_nlmsg_build_hdr(mnl_nlmsg_batch_current(batch),
				       cmd,
				       nftnl_rule_get_u32(rule, NFTNL_RULE_FAMILY),
				       NLM_F_APPEND|NLM_F_CREATE|NLM_F_ACK,
				       seq++);

	nftnl_rule_nlmsg_build_payload(nlh, rule);
	nftnl_rule_free(rule);
	mnl_nlmsg_batch_next(batch);

	nft_mnl_batch_put(mnl_nlmsg_batch_current(batch), NFNL_MSG_BATCH_END,
			  seq++);
	mnl_nlmsg_batch_next(batch);

	ret = mnl_socket_sendto(nl, mnl_nlmsg_batch_head(batch),
				mnl_nlmsg_batch_size(batch));
	if (ret == -1) {
		syslog(LOG_ERR, "%s: mnl_socket_sendto() FAILED: %m", "nft_send_request()");
		return -1;
	}

	mnl_nlmsg_batch_stop(batch);

	ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	if (ret == -1) {
		syslog(LOG_ERR, "%s: mnl_socket_recvfrom() FAILED: %m", "nft_send_request()");
		return -1;	
	}

	ret = mnl_cb_run(buf, ret, 0, mnl_socket_get_portid(nl), NULL, NULL);
	if (ret < 0) {
		syslog(LOG_ERR, "%s: mnl_cb_run() FAILED: %m", "nft_send_request()");
		return -1;	
	}

	/* mnl_socket_close(nl); */
	return 0;
}
