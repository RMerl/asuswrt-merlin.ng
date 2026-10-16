/*
 * (C) 2012-2013 by Pablo Neira Ayuso <pablo@netfilter.org>
 * (C) 2013 by Tomasz Bursztyka <tomasz.bursztyka@linux.intel.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This code has been sponsored by Sophos Astaro <http://www.sophos.com>
 */

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <netdb.h>
#include <errno.h>
#include <inttypes.h>

#include <xtables.h>

#include <libmnl/libmnl.h>
#include <libnftnl/rule.h>
#include <libnftnl/expr.h>

#include "nft-shared.h"
#include "nft-bridge.h"
#include "xshared.h"
#include "nft.h"

extern struct nft_family_ops nft_family_ops_ipv4;
extern struct nft_family_ops nft_family_ops_ipv6;
extern struct nft_family_ops nft_family_ops_arp;
extern struct nft_family_ops nft_family_ops_bridge;

static struct nftnl_expr *xt_nftnl_expr_alloc(const char *name)
{
	struct nftnl_expr *expr = nftnl_expr_alloc(name);

	if (expr)
		return expr;

	xtables_error(RESOURCE_PROBLEM,
		      "Failed to allocate nftnl expression '%s'", name);
}

void add_meta(struct nft_handle *h, struct nftnl_rule *r, uint32_t key,
	      uint8_t *dreg)
{
	struct nftnl_expr *expr;
	uint8_t reg;

	expr = xt_nftnl_expr_alloc("meta");

	reg = NFT_REG_1;
	nftnl_expr_set_u32(expr, NFTNL_EXPR_META_KEY, key);
	nftnl_expr_set_u32(expr, NFTNL_EXPR_META_DREG, reg);
	nftnl_rule_add_expr(r, expr);

	*dreg = reg;
}

void add_payload(struct nft_handle *h, struct nftnl_rule *r,
		 int offset, int len, uint32_t base, uint8_t *dreg)
{
	struct nftnl_expr *expr;
	uint8_t reg;

	expr = xt_nftnl_expr_alloc("payload");

	reg = NFT_REG_1;
	nftnl_expr_set_u32(expr, NFTNL_EXPR_PAYLOAD_BASE, base);
	nftnl_expr_set_u32(expr, NFTNL_EXPR_PAYLOAD_DREG, reg);
	nftnl_expr_set_u32(expr, NFTNL_EXPR_PAYLOAD_OFFSET, offset);
	nftnl_expr_set_u32(expr, NFTNL_EXPR_PAYLOAD_LEN, len);
	nftnl_rule_add_expr(r, expr);

	*dreg = reg;
}

/* bitwise operation is = sreg & mask ^ xor */
void add_bitwise_u16(struct nft_handle *h, struct nftnl_rule *r,
		     uint16_t mask, uint16_t xor, uint8_t sreg, uint8_t *dreg)
{
	struct nftnl_expr *expr;
	uint8_t reg;

	expr = xt_nftnl_expr_alloc("bitwise");

	reg = NFT_REG_1;
	nftnl_expr_set_u32(expr, NFTNL_EXPR_BITWISE_SREG, sreg);
	nftnl_expr_set_u32(expr, NFTNL_EXPR_BITWISE_DREG, reg);
	nftnl_expr_set_u32(expr, NFTNL_EXPR_BITWISE_LEN, sizeof(uint16_t));
	nftnl_expr_set(expr, NFTNL_EXPR_BITWISE_MASK, &mask, sizeof(uint16_t));
	nftnl_expr_set(expr, NFTNL_EXPR_BITWISE_XOR, &xor, sizeof(uint16_t));
	nftnl_rule_add_expr(r, expr);

	*dreg = reg;
}

void add_bitwise(struct nft_handle *h, struct nftnl_rule *r,
		 uint8_t *mask, size_t len, uint8_t sreg, uint8_t *dreg)
{
	struct nftnl_expr *expr;
	uint32_t xor[4] = { 0 };
	uint8_t reg = *dreg;

	expr = xt_nftnl_expr_alloc("bitwise");

	nftnl_expr_set_u32(expr, NFTNL_EXPR_BITWISE_SREG, sreg);
	nftnl_expr_set_u32(expr, NFTNL_EXPR_BITWISE_DREG, reg);
	nftnl_expr_set_u32(expr, NFTNL_EXPR_BITWISE_LEN, len);
	nftnl_expr_set(expr, NFTNL_EXPR_BITWISE_MASK, mask, len);
	nftnl_expr_set(expr, NFTNL_EXPR_BITWISE_XOR, &xor, len);
	nftnl_rule_add_expr(r, expr);

	*dreg = reg;
}

void add_cmp_ptr(struct nftnl_rule *r, uint32_t op, void *data, size_t len,
		 uint8_t sreg)
{
	struct nftnl_expr *expr;

	expr = xt_nftnl_expr_alloc("cmp");

	nftnl_expr_set_u32(expr, NFTNL_EXPR_CMP_SREG, sreg);
	nftnl_expr_set_u32(expr, NFTNL_EXPR_CMP_OP, op);
	nftnl_expr_set(expr, NFTNL_EXPR_CMP_DATA, data, len);
	nftnl_rule_add_expr(r, expr);
}

void add_cmp_u8(struct nftnl_rule *r, uint8_t val, uint32_t op, uint8_t sreg)
{
	add_cmp_ptr(r, op, &val, sizeof(val), sreg);
}

void add_cmp_u16(struct nftnl_rule *r, uint16_t val, uint32_t op, uint8_t sreg)
{
	add_cmp_ptr(r, op, &val, sizeof(val), sreg);
}

void add_cmp_u32(struct nftnl_rule *r, uint32_t val, uint32_t op, uint8_t sreg)
{
	add_cmp_ptr(r, op, &val, sizeof(val), sreg);
}

void add_iface(struct nft_handle *h, struct nftnl_rule *r,
	       char *iface, uint32_t key, uint32_t op)
{
	int iface_len = strlen(iface);
	uint8_t reg;


	if (iface[iface_len - 1] == '+') {
		if (iface_len > 1) {
			iface_len -= 1;
		} else if (op != NFT_CMP_EQ) {
			op = NFT_CMP_EQ;
			iface = "INVAL/D";
			iface_len = strlen(iface) + 1;
		} else {
			return; /* -o + */
		}
	} else {
		iface_len += 1;
	}

	add_meta(h, r, key, &reg);
	add_cmp_ptr(r, op, iface, iface_len, reg);
}

void add_addr(struct nft_handle *h, struct nftnl_rule *r,
	      enum nft_payload_bases base, int offset,
	      void *data, void *mask, size_t len, uint32_t op)
{
	const unsigned char *m = mask;
	bool bitwise = false;
	uint8_t reg;
	int i, j;

	for (i = 0; i < len; i++) {
		if (m[i] != 0xff) {
			bitwise = m[i] != 0;
			break;
		}
	}
	for (j = i + 1; !bitwise && j < len; j++)
		bitwise = !!m[j];

	if (!bitwise)
		len = i;

	add_payload(h, r, offset, len, base, &reg);

	if (bitwise)
		add_bitwise(h, r, mask, len, reg, &reg);

	add_cmp_ptr(r, op, data, len, reg);
}

void add_proto(struct nft_handle *h, struct nftnl_rule *r,
	       int offset, size_t len, uint8_t proto, uint32_t op)
{
	uint8_t reg;

	add_payload(h, r, offset, len, NFT_PAYLOAD_NETWORK_HEADER, &reg);
	add_cmp_u8(r, proto, op, reg);
}

void add_l4proto(struct nft_handle *h, struct nftnl_rule *r,
		 uint8_t proto, uint32_t op)
{
	uint8_t reg;

	add_meta(h, r, NFT_META_L4PROTO, &reg);
	add_cmp_u8(r, proto, op, reg);
}

bool is_same_interfaces(const char *a_iniface, const char *a_outiface,
			unsigned const char *a_iniface_mask,
			unsigned const char *a_outiface_mask,
			const char *b_iniface, const char *b_outiface,
			unsigned const char *b_iniface_mask,
			unsigned const char *b_outiface_mask)
{
	int i;

	for (i = 0; i < IFNAMSIZ; i++) {
		if (a_iniface_mask[i] != b_iniface_mask[i]) {
			DEBUGP("different iniface mask %x, %x (%d)\n",
			a_iniface_mask[i] & 0xff, b_iniface_mask[i] & 0xff, i);
			return false;
		}
		if ((a_iniface[i] & a_iniface_mask[i])
		    != (b_iniface[i] & b_iniface_mask[i])) {
			DEBUGP("different iniface\n");
			return false;
		}
		if (a_outiface_mask[i] != b_outiface_mask[i]) {
			DEBUGP("different outiface mask\n");
			return false;
		}
		if ((a_outiface[i] & a_outiface_mask[i])
		    != (b_outiface[i] & b_outiface_mask[i])) {
			DEBUGP("different outiface\n");
			return false;
		}
	}

	return true;
}

void __get_cmp_data(struct nftnl_expr *e, void *data, size_t dlen, uint8_t *op)
{
	uint32_t len;

	memcpy(data, nftnl_expr_get(e, NFTNL_EXPR_CMP_DATA, &len), dlen);
	*op = nftnl_expr_get_u32(e, NFTNL_EXPR_CMP_OP);
}

void get_cmp_data(struct nftnl_expr *e, void *data, size_t dlen, bool *inv)
{
	uint8_t op;

	__get_cmp_data(e, data, dlen, &op);
	*inv = (op == NFT_CMP_NEQ);
}

void nft_ipv46_save_chain(const struct nftnl_chain *c, const char *policy)
{
	const char *chain = nftnl_chain_get_str(c, NFTNL_CHAIN_NAME);
	uint64_t pkts = nftnl_chain_get_u64(c, NFTNL_CHAIN_PACKETS);
	uint64_t bytes = nftnl_chain_get_u64(c, NFTNL_CHAIN_BYTES);

	printf(":%s %s [%"PRIu64":%"PRIu64"]\n",
	       chain, policy ?: "-", pkts, bytes);
}

void save_matches_and_target(const struct iptables_command_state *cs,
			     bool goto_flag, const void *fw,
			     unsigned int format)
{
	struct xtables_rule_match *matchp;

	for (matchp = cs->matches; matchp; matchp = matchp->next) {
		if (matchp->match->alias) {
			printf(" -m %s",
			       matchp->match->alias(matchp->match->m));
		} else
			printf(" -m %s", matchp->match->name);

		if (matchp->match->save != NULL) {
			/* cs->fw union makes the trick */
			matchp->match->save(fw, matchp->match->m);
		}
	}

	if ((format & (FMT_NOCOUNTS | FMT_C_COUNTS)) == FMT_C_COUNTS)
		printf(" -c %llu %llu",
		       (unsigned long long)cs->counters.pcnt,
		       (unsigned long long)cs->counters.bcnt);

	if (cs->target != NULL) {
		if (cs->target->alias) {
			printf(" -j %s", cs->target->alias(cs->target->t));
		} else
			printf(" -j %s", cs->jumpto);

		if (cs->target->save != NULL) {
			cs->target->save(fw, cs->target->t);
		}
	} else if (strlen(cs->jumpto) > 0) {
		printf(" -%c %s", goto_flag ? 'g' : 'j', cs->jumpto);
	}

	printf("\n");
}

void print_matches_and_target(struct iptables_command_state *cs,
			      unsigned int format)
{
	struct xtables_rule_match *matchp;

	for (matchp = cs->matches; matchp; matchp = matchp->next) {
		if (matchp->match->print != NULL) {
			matchp->match->print(&cs->fw, matchp->match->m,
					     format & FMT_NUMERIC);
		}
	}

	if (cs->target != NULL) {
		if (cs->target->print != NULL) {
			cs->target->print(&cs->fw, cs->target->t,
					  format & FMT_NUMERIC);
		}
	}
}

struct nft_family_ops *nft_family_ops_lookup(int family)
{
	switch (family) {
	case AF_INET:
		return &nft_family_ops_ipv4;
	case AF_INET6:
		return &nft_family_ops_ipv6;
	case NFPROTO_ARP:
		return &nft_family_ops_arp;
	case NFPROTO_BRIDGE:
		return &nft_family_ops_bridge;
	default:
		break;
	}

	return NULL;
}

bool compare_matches(struct xtables_rule_match *mt1,
		     struct xtables_rule_match *mt2)
{
	struct xtables_rule_match *mp1;
	struct xtables_rule_match *mp2;

	for (mp1 = mt1, mp2 = mt2; mp1 && mp2; mp1 = mp1->next, mp2 = mp2->next) {
		struct xt_entry_match *m1 = mp1->match->m;
		struct xt_entry_match *m2 = mp2->match->m;
		size_t cmplen = mp1->match->userspacesize;

		if (strcmp(m1->u.user.name, m2->u.user.name) != 0) {
			DEBUGP("mismatching match name\n");
			return false;
		}

		if (m1->u.user.match_size != m2->u.user.match_size) {
			DEBUGP("mismatching match size\n");
			return false;
		}

		if (!strcmp(m1->u.user.name, "among"))
			cmplen = m1->u.match_size - sizeof(*m1);

		if (memcmp(m1->data, m2->data, cmplen) != 0) {
			DEBUGP("mismatch match data\n");
			DEBUG_HEXDUMP("m1->data", m1->data, cmplen);
			DEBUG_HEXDUMP("m2->data", m2->data, cmplen);
			return false;
		}
	}

	/* Both cursors should be NULL */
	if (mp1 != mp2) {
		DEBUGP("mismatch matches amount\n");
		return false;
	}

	return true;
}

bool compare_targets(struct xtables_target *tg1, struct xtables_target *tg2)
{
	if (tg1 == NULL && tg2 == NULL)
		return true;

	if (tg1 == NULL || tg2 == NULL)
		return false;
	if (tg1->userspacesize != tg2->userspacesize)
		return false;

	if (strcmp(tg1->t->u.user.name, tg2->t->u.user.name) != 0)
		return false;

	if (memcmp(tg1->t->data, tg2->t->data, tg1->userspacesize) != 0)
		return false;

	return true;
}

void nft_check_xt_legacy(int family, bool is_ipt_save)
{
	static const char tables6[] = "/proc/net/ip6_tables_names";
	static const char tables4[] = "/proc/net/ip_tables_names";
	static const char tablesa[] = "/proc/net/arp_tables_names";
	const char *prefix = "ip";
	FILE *fp = NULL;
	char buf[1024];

	switch (family) {
	case NFPROTO_IPV4:
		fp = fopen(tables4, "r");
		break;
	case NFPROTO_IPV6:
		fp = fopen(tables6, "r");
		prefix = "ip6";
		break;
	case NFPROTO_ARP:
		fp = fopen(tablesa, "r");
		prefix = "arp";
		break;
	default:
		break;
	}

	if (!fp)
		return;

	if (fgets(buf, sizeof(buf), fp))
		fprintf(stderr, "# Warning: %stables-legacy tables present, use %stables-legacy%s to see them\n",
			prefix, prefix, is_ipt_save ? "-save" : "");
	fclose(fp);
}

enum nft_registers nft_get_next_reg(enum nft_registers reg, size_t size)
{
	/* convert size to NETLINK_ALIGN-sized chunks */
	size = (size + NETLINK_ALIGN - 1) / NETLINK_ALIGN;

	/* map 16byte reg to 4byte one */
	if (reg < __NFT_REG_MAX)
		reg = NFT_REG32_00 + (reg - 1) * NFT_REG_SIZE / NFT_REG32_SIZE;

	reg += size;
	assert(reg <= NFT_REG32_15);

	return reg;
}
