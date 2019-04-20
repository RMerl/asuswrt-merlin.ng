/*
 * (C) 2012 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation (or any later at your option).
 *
 * This code has been sponsored by Vyatta Inc. <http://www.vyatta.com>
 */

#include "helper.h"

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dlfcn.h>

int
cthelper_expect_init(struct nf_expect *exp, struct nf_conntrack *master,
		  uint32_t class,
		  union nfct_attr_grp_addr *saddr,
		  union nfct_attr_grp_addr *daddr,
		  uint8_t l4proto, uint16_t *sport, uint16_t *dport,
		  uint32_t flags)
{
	struct nf_conntrack *expected, *mask;

	expected = nfct_new();
	if (!expected)
		return -1;

	mask = nfct_new();
	if (!mask)
		return -1;

	if (saddr) {
		switch(nfct_get_attr_u8(master, ATTR_L3PROTO)) {
		uint32_t addr[4];

		case AF_INET:
			nfct_set_attr_u8(expected, ATTR_L3PROTO, AF_INET);
			nfct_set_attr_u32(expected, ATTR_IPV4_SRC, saddr->ip);

			nfct_set_attr_u8(mask, ATTR_L3PROTO, AF_INET);
			nfct_set_attr_u32(mask, ATTR_IPV4_SRC, 0xffffffff);
			break;
		case AF_INET6:
			nfct_set_attr_u8(expected, ATTR_L3PROTO, AF_INET6);
			nfct_set_attr(expected, ATTR_IPV6_SRC, saddr->ip6);
			memset(addr, 0xff, sizeof(addr));
			nfct_set_attr_u8(mask, ATTR_L3PROTO, AF_INET6);
			nfct_set_attr(mask, ATTR_IPV6_SRC, addr);
			break;
		default:
			break;
		}
	} else {
		switch(nfct_get_attr_u8(master, ATTR_L3PROTO)) {
		uint32_t addr[4];

		case AF_INET:
			nfct_set_attr_u8(expected, ATTR_L3PROTO, AF_INET);
			nfct_set_attr_u32(expected, ATTR_IPV4_SRC, 0x00000000);

			nfct_set_attr_u8(mask, ATTR_L3PROTO, AF_INET);
			nfct_set_attr_u32(mask, ATTR_IPV4_SRC, 0x00000000);
			break;
		case AF_INET6:
			memset(addr, 0x00, sizeof(addr));
			nfct_set_attr_u8(expected, ATTR_L3PROTO, AF_INET6);
			nfct_set_attr(expected, ATTR_IPV6_SRC, addr);

			nfct_set_attr_u8(mask, ATTR_L3PROTO, AF_INET6);
			nfct_set_attr(mask, ATTR_IPV6_SRC, addr);
			break;
		default:
			break;
		}
	}

	if (sport) {
		switch(l4proto) {
		case IPPROTO_TCP:
		case IPPROTO_UDP:
			nfct_set_attr_u8(expected, ATTR_L4PROTO, l4proto);
			nfct_set_attr_u16(expected, ATTR_PORT_SRC, *sport);
			nfct_set_attr_u8(mask, ATTR_L4PROTO, l4proto);
			nfct_set_attr_u16(mask, ATTR_PORT_SRC, 0xffff);
			break;
		default:
			break;
		}
	} else {
		switch(l4proto) {
		case IPPROTO_TCP:
		case IPPROTO_UDP:
			nfct_set_attr_u8(expected, ATTR_L4PROTO, l4proto);
			nfct_set_attr_u16(expected, ATTR_PORT_SRC, 0x0000);
			nfct_set_attr_u8(mask, ATTR_L4PROTO, l4proto);
			nfct_set_attr_u16(mask, ATTR_PORT_SRC, 0x0000);
			break;
		default:
			break;
		}
	}

	switch(nfct_get_attr_u8(master, ATTR_L3PROTO)) {
	uint32_t addr[4];

	case AF_INET:
		nfct_set_attr_u8(expected, ATTR_L3PROTO, AF_INET);
		nfct_set_attr_u32(expected, ATTR_IPV4_DST, daddr->ip);
		nfct_set_attr_u32(mask, ATTR_IPV4_DST, 0xffffffff);
		break;
	case AF_INET6:
		nfct_set_attr_u8(expected, ATTR_L3PROTO, AF_INET6);
		nfct_set_attr(expected, ATTR_IPV6_DST, daddr->ip6);
		memset(addr, 0xff, sizeof(addr));
		nfct_set_attr(mask, ATTR_IPV6_DST, addr);
		break;
	default:
		break;
	}

	switch(l4proto) {
	case IPPROTO_TCP:
	case IPPROTO_UDP:
		nfct_set_attr_u8(expected, ATTR_L4PROTO, l4proto);
		nfct_set_attr_u16(expected, ATTR_PORT_DST, *dport);
		nfct_set_attr_u8(mask, ATTR_L4PROTO, l4proto);
		nfct_set_attr_u16(mask, ATTR_PORT_DST, 0xffff);
		break;
	default:
		break;
	}

	nfexp_set_attr(exp, ATTR_EXP_MASTER, master);
	nfexp_set_attr(exp, ATTR_EXP_EXPECTED, expected);
	nfexp_set_attr(exp, ATTR_EXP_MASK, mask);
	nfexp_set_attr_u32(exp, ATTR_EXP_FLAGS, flags);

	nfct_destroy(expected);
	nfct_destroy(mask);

	return 0;
}

static int cthelper_expect_cmd(struct nf_expect *exp, int cmd)
{
	int ret;
	struct nfct_handle *h;

	h = nfct_open(EXPECT, 0);
	if (!h)
		return -1;

	ret = nfexp_query(h, cmd, exp);

	nfct_close(h);
	return ret;
}

int cthelper_add_expect(struct nf_expect *exp)
{
	return cthelper_expect_cmd(exp, NFCT_Q_CREATE_UPDATE);
}

int cthelper_del_expect(struct nf_expect *exp)
{
	return cthelper_expect_cmd(exp, NFCT_Q_DESTROY);
}

void
cthelper_get_addr_src(struct nf_conntrack *ct, int dir,
		      union nfct_attr_grp_addr *addr)
{
	switch (dir) {
	case MYCT_DIR_ORIG:
		nfct_get_attr_grp(ct, ATTR_GRP_ORIG_ADDR_SRC, addr);
		break;
	case MYCT_DIR_REPL:
		nfct_get_attr_grp(ct, ATTR_GRP_REPL_ADDR_SRC, addr);
		break;
	}
}

void
cthelper_get_addr_dst(struct nf_conntrack *ct, int dir,
		      union nfct_attr_grp_addr *addr)
{
	switch (dir) {
	case MYCT_DIR_ORIG:
		nfct_get_attr_grp(ct, ATTR_GRP_ORIG_ADDR_DST, addr);
		break;
	case MYCT_DIR_REPL:
		nfct_get_attr_grp(ct, ATTR_GRP_REPL_ADDR_DST, addr);
		break;
	}
}

void cthelper_get_port_src(struct nf_conntrack *ct, int dir, uint16_t *port)
{
	switch (dir) {
	case MYCT_DIR_ORIG:
		*port = nfct_get_attr_u16(ct, ATTR_PORT_SRC);
		break;
	case MYCT_DIR_REPL:
		*port = nfct_get_attr_u16(ct, ATTR_REPL_PORT_SRC);
		break;
	}
}

void cthelper_get_port_dst(struct nf_conntrack *ct, int dir, uint16_t *port)
{
	switch (dir) {
	case MYCT_DIR_ORIG:
		*port = nfct_get_attr_u16(ct, ATTR_PORT_DST);
		break;
	case MYCT_DIR_REPL:
		*port = nfct_get_attr_u16(ct, ATTR_REPL_PORT_DST);
		break;
	}
}
