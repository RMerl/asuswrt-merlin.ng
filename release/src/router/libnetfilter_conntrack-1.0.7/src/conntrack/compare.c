/*
 * (C) 2005-2011 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "internal/internal.h"
#include <stdbool.h>

static int __cmp(int attr,
		 const struct nf_conntrack *ct1, 
		 const struct nf_conntrack *ct2,
		 unsigned int flags,
		 int (*cmp)(const struct nf_conntrack *ct1,
		 	    const struct nf_conntrack *ct2,
			    unsigned int flags), bool strict)
{
	int a = test_bit(attr, ct1->head.set);
	int b = test_bit(attr, ct2->head.set);
	if (a && b) {
		return cmp(ct1, ct2, flags);
	} else if (!a && !b) {
		return 1;
	} else if (flags & NFCT_CMP_MASK &&
		   test_bit(attr, ct1->head.set)) {
		return strict ? 0 : cmp(ct1, ct2, flags);
	} else if (flags & NFCT_CMP_STRICT) {
		return strict ? 0 : cmp(ct1, ct2, flags);
	}
	return 1;
}

static int
cmp_orig_l3proto(const struct nf_conntrack *ct1,
		 const struct nf_conntrack *ct2,
		 unsigned int flags)
{
	return (ct1->head.orig.l3protonum == ct2->head.orig.l3protonum);
}

static int
cmp_icmp_id(const struct nf_conntrack *ct1,
	    const struct nf_conntrack *ct2,
	    unsigned int flags)
{
	return (ct1->head.orig.l4src.icmp.id == ct2->head.orig.l4src.icmp.id);
}

static int
cmp_icmp_type(const struct nf_conntrack *ct1,
	      const struct nf_conntrack *ct2,
	      unsigned int flags)
{
	return (ct1->head.orig.l4dst.icmp.type ==
		ct2->head.orig.l4dst.icmp.type);
}

static int
cmp_icmp_code(const struct nf_conntrack *ct1,
	      const struct nf_conntrack *ct2,
	      unsigned int flags)
{
	return (ct1->head.orig.l4dst.icmp.code ==
		ct2->head.orig.l4dst.icmp.code);
}

static int
cmp_orig_port_src(const struct nf_conntrack *ct1,
		  const struct nf_conntrack *ct2,
		  unsigned int flags)
{
	return (ct1->head.orig.l4src.all == ct2->head.orig.l4src.all);
}

static int
cmp_orig_port_dst(const struct nf_conntrack *ct1,
		  const struct nf_conntrack *ct2,
		  unsigned int flags)
{
	return (ct1->head.orig.l4dst.all == ct2->head.orig.l4dst.all);
}

static int 
cmp_orig_l4proto(const struct nf_conntrack *ct1,
		 const struct nf_conntrack *ct2,
		 unsigned int flags)
{
	if (ct1->head.orig.protonum != ct2->head.orig.protonum)
		return 0;

	switch(ct1->head.orig.protonum) {
	case IPPROTO_ICMP:
	case IPPROTO_ICMPV6:
		if (!__cmp(ATTR_ICMP_ID, ct1, ct2, flags, cmp_icmp_id, true))
			return 0;
		if (!__cmp(ATTR_ICMP_CODE, ct1, ct2, flags, cmp_icmp_code, true))
			return 0;
		if (!__cmp(ATTR_ICMP_TYPE, ct1, ct2, flags, cmp_icmp_type, true))
			return 0;
		break;
	case IPPROTO_TCP:
	case IPPROTO_UDP:
	case IPPROTO_UDPLITE:
	case IPPROTO_DCCP:
	case IPPROTO_SCTP:
		if (!__cmp(ATTR_ORIG_PORT_SRC, ct1, ct2,
			       flags, cmp_orig_port_src, true))
			return 0;
		if (!__cmp(ATTR_ORIG_PORT_DST, ct1, ct2,
			       flags, cmp_orig_port_dst, true))
			return 0;
		break;
	}
	return 1;
}

static int 
cmp_orig_ipv4_src(const struct nf_conntrack *ct1,
		  const struct nf_conntrack *ct2,
		  unsigned int flags)
{
	return (ct1->head.orig.src.v4 == ct2->head.orig.src.v4);}

static int 
cmp_orig_ipv4_dst(const struct nf_conntrack *ct1,
		  const struct nf_conntrack *ct2,
		  unsigned int flags)
{
	return (ct1->head.orig.dst.v4 == ct2->head.orig.dst.v4);}

static int 
cmp_orig_ipv6_src(const struct nf_conntrack *ct1,
		  const struct nf_conntrack *ct2,
		  unsigned int flags)
{
	return (memcmp(&ct1->head.orig.src.v6, &ct2->head.orig.src.v6,
		sizeof(struct in6_addr)) == 0);
}

static int 
cmp_orig_ipv6_dst(const struct nf_conntrack *ct1,
		  const struct nf_conntrack *ct2,
		  unsigned int flags)
{
	return (memcmp(&ct1->head.orig.dst.v6, &ct2->head.orig.dst.v6,
		sizeof(struct in6_addr)) == 0);
}

static int
cmp_orig_zone(const struct nf_conntrack *ct1,
	      const struct nf_conntrack *ct2,
	      unsigned int flags)
{
	return nfct_get_attr_u16(ct1, ATTR_ORIG_ZONE) ==
	       nfct_get_attr_u16(ct2, ATTR_ORIG_ZONE);
}

int __cmp_orig(const struct nf_conntrack *ct1,
	       const struct nf_conntrack *ct2,
	       unsigned int flags)
{
	if (!__cmp(ATTR_ORIG_L3PROTO, ct1, ct2, flags, cmp_orig_l3proto, true))
		return 0;
	if (!__cmp(ATTR_ORIG_L4PROTO, ct1, ct2, flags, cmp_orig_l4proto, true))
		return 0;
	if (!__cmp(ATTR_ORIG_IPV4_SRC, ct1, ct2, flags, cmp_orig_ipv4_src, true))
		return 0;
	if (!__cmp(ATTR_ORIG_IPV4_DST, ct1, ct2, flags, cmp_orig_ipv4_dst, true))
		return 0;
	if (!__cmp(ATTR_ORIG_IPV6_SRC, ct1, ct2, flags, cmp_orig_ipv6_src, true))
		return 0;
	if (!__cmp(ATTR_ORIG_IPV6_DST, ct1, ct2, flags, cmp_orig_ipv6_dst, true))
		return 0;
	if (!__cmp(ATTR_ORIG_ZONE, ct1, ct2, flags, cmp_orig_zone, false))
		return 0;

	return 1;
}

static int
cmp_repl_l3proto(const struct nf_conntrack *ct1,
		 const struct nf_conntrack *ct2,
		 unsigned int flags)
{
	return (ct1->repl.l3protonum == ct2->repl.l3protonum);
}

static int
cmp_repl_port_src(const struct nf_conntrack *ct1,
		  const struct nf_conntrack *ct2,
		  unsigned int flags)
{
	return (ct1->repl.l4src.all == ct2->repl.l4src.all);
}

static int
cmp_repl_port_dst(const struct nf_conntrack *ct1,
		  const struct nf_conntrack *ct2,
		  unsigned int flags)
{
	return (ct1->repl.l4dst.all == ct2->repl.l4dst.all);
}

static int 
cmp_repl_l4proto(const struct nf_conntrack *ct1,
		 const struct nf_conntrack *ct2,
		 unsigned int flags)
{
	if (ct1->repl.protonum != ct2->repl.protonum)
		return 0;

	switch(ct1->repl.protonum) {
	case IPPROTO_ICMP:
	case IPPROTO_ICMPV6:
		if (!__cmp(ATTR_ICMP_ID, ct1, ct2, flags, cmp_icmp_id, true))
			return 0;
		if (!__cmp(ATTR_ICMP_CODE, ct1, ct2, flags, cmp_icmp_code, true))
			return 0;
		if (!__cmp(ATTR_ICMP_TYPE, ct1, ct2, flags, cmp_icmp_type, true))
			return 0;
		break;
	case IPPROTO_TCP:
	case IPPROTO_UDP:
	case IPPROTO_UDPLITE:
	case IPPROTO_DCCP:
	case IPPROTO_SCTP:
		if (!__cmp(ATTR_REPL_PORT_SRC, ct1, ct2,
			       flags, cmp_repl_port_src, true))
			return 0;
		if (!__cmp(ATTR_REPL_PORT_DST, ct1, ct2,
			       flags, cmp_repl_port_dst, true))
			return 0;
		break;
	}
	return 1;
}

static int 
cmp_repl_ipv4_src(const struct nf_conntrack *ct1,
		  const struct nf_conntrack *ct2,
		  unsigned int flags)
{
	return (ct1->repl.src.v4 == ct2->repl.src.v4);}

static int 
cmp_repl_ipv4_dst(const struct nf_conntrack *ct1,
		  const struct nf_conntrack *ct2,
		  unsigned int flags)
{
	return (ct1->repl.dst.v4 == ct2->repl.dst.v4);}

static int 
cmp_repl_ipv6_src(const struct nf_conntrack *ct1,
		  const struct nf_conntrack *ct2,
		  unsigned int flags)
{
	return (memcmp(&ct1->repl.src.v6, &ct2->repl.src.v6,
		sizeof(struct in6_addr)) == 0);
}

static int 
cmp_repl_ipv6_dst(const struct nf_conntrack *ct1,
		  const struct nf_conntrack *ct2,
		  unsigned int flags)
{
	return (memcmp(&ct1->repl.dst.v6, &ct2->repl.dst.v6,
		sizeof(struct in6_addr)) == 0);
}

static int
cmp_repl_zone(const struct nf_conntrack *ct1,
	      const struct nf_conntrack *ct2,
	      unsigned int flags)
{
	return nfct_get_attr_u16(ct1, ATTR_REPL_ZONE) ==
	       nfct_get_attr_u16(ct2, ATTR_REPL_ZONE);
}

static int cmp_repl(const struct nf_conntrack *ct1,
		    const struct nf_conntrack *ct2,
		    unsigned int flags)
{
	if (!__cmp(ATTR_REPL_L3PROTO, ct1, ct2, flags, cmp_repl_l3proto, true))
		return 0;
	if (!__cmp(ATTR_REPL_L4PROTO, ct1, ct2, flags, cmp_repl_l4proto, true))
		return 0;
	if (!__cmp(ATTR_REPL_IPV4_SRC, ct1, ct2, flags, cmp_repl_ipv4_src, true))
		return 0;
	if (!__cmp(ATTR_REPL_IPV4_DST, ct1, ct2, flags, cmp_repl_ipv4_dst, true))
		return 0;
	if (!__cmp(ATTR_REPL_IPV6_SRC, ct1, ct2, flags, cmp_repl_ipv6_src, true))
		return 0;
	if (!__cmp(ATTR_REPL_IPV6_DST, ct1, ct2, flags, cmp_repl_ipv6_dst, true))
		return 0;
	if (!__cmp(ATTR_REPL_ZONE, ct1, ct2, flags, cmp_repl_zone, false))
		return 0;

	return 1;
}

static int 
cmp_id(const struct nf_conntrack *ct1,
       const struct nf_conntrack *ct2,
       unsigned int flags)
{
	return (ct1->id == ct2->id);
}

static int
cmp_mark(const struct nf_conntrack *ct1,
	 const struct nf_conntrack *ct2,
	 unsigned int flags)
{
	return nfct_get_attr_u32(ct1, ATTR_MARK) ==
	       nfct_get_attr_u32(ct2, ATTR_MARK);
}

static int 
cmp_timeout(const struct nf_conntrack *ct1,
	    const struct nf_conntrack *ct2,
	    unsigned int flags)
{
	int ret = 0;

#define __NFCT_CMP_TIMEOUT (NFCT_CMP_TIMEOUT_LE | NFCT_CMP_TIMEOUT_GT)

	if (!(flags & __NFCT_CMP_TIMEOUT) &&
	    ct1->timeout == ct2->timeout)
		return 1;
	else {
		if (flags & NFCT_CMP_TIMEOUT_GT &&
		    ct1->timeout > ct2->timeout)
			ret = 1;
		else if (flags & NFCT_CMP_TIMEOUT_LT &&
			 ct1->timeout < ct2->timeout)
		    	ret = 1;
		else if (flags & NFCT_CMP_TIMEOUT_EQ &&
			 ct1->timeout == ct2->timeout)
			ret = 1;
	}
	return ret;
}

static int 
cmp_status(const struct nf_conntrack *ct1,
	   const struct nf_conntrack *ct2,
	   unsigned int flags)
{
	return ((ct1->status & ct2->status) == ct1->status);
}

static int 
cmp_tcp_state(const struct nf_conntrack *ct1,
	      const struct nf_conntrack *ct2,
	      unsigned int flags)
{
	return (ct1->protoinfo.tcp.state == ct2->protoinfo.tcp.state);
}

static int 
cmp_sctp_state(const struct nf_conntrack *ct1,
	       const struct nf_conntrack *ct2,
	       unsigned int flags)
{
	return (ct1->protoinfo.sctp.state == ct2->protoinfo.sctp.state);
}

static int
cmp_dccp_state(const struct nf_conntrack *ct1,
	       const struct nf_conntrack *ct2,
	       unsigned int flags)
{
	return (ct1->protoinfo.dccp.state == ct2->protoinfo.dccp.state);
}

static int
cmp_zone(const struct nf_conntrack *ct1,
	 const struct nf_conntrack *ct2,
	 unsigned int flags)
{
	return nfct_get_attr_u16(ct1, ATTR_ZONE) ==
	       nfct_get_attr_u16(ct2, ATTR_ZONE);
}

static int
cmp_secctx(const struct nf_conntrack *ct1,
	   const struct nf_conntrack *ct2,
	   unsigned int flags)
{
	if (ct1->secctx == NULL || ct2->secctx == NULL)
		return ct1->secctx == ct2->secctx;
	return strcmp(ct1->secctx, ct2->secctx) == 0;
}

static int __cmp_clabel(const struct nfct_bitmask *a,
			const struct nfct_bitmask *b)
{
	unsigned int len, max;
	const uint32_t *bits;

	if (a == NULL || b == NULL)
		return a == b;

	if (a->words < b->words) {
		bits = b->bits;
		max = b->words;
		len = a->words;
	} else {
		bits = a->bits;
		max = a->words;
		len = b->words;
	}

	while (max > len) {
		if (bits[--max])
			return 0;
	}
	/* bitmask sizes are equal or extra bits are not set */
	return memcmp(a->bits, b->bits, len * sizeof(a->bits[0])) == 0;
}

static int cmp_clabel(const struct nf_conntrack *ct1,
		      const struct nf_conntrack *ct2,
		      unsigned int flags)
{
	return __cmp_clabel(nfct_get_attr(ct1, ATTR_CONNLABELS),
			    nfct_get_attr(ct2, ATTR_CONNLABELS));

}

static int cmp_clabel_mask(const struct nf_conntrack *ct1,
		      const struct nf_conntrack *ct2,
		      unsigned int flags)
{
	return __cmp_clabel(nfct_get_attr(ct1, ATTR_CONNLABELS_MASK),
			    nfct_get_attr(ct2, ATTR_CONNLABELS_MASK));

}

static int cmp_meta(const struct nf_conntrack *ct1,
		    const struct nf_conntrack *ct2,
		    unsigned int flags)
{
	if (!__cmp(ATTR_ID, ct1, ct2, flags, cmp_id, true))
		return 0;
	if (!__cmp(ATTR_MARK, ct1, ct2, flags, cmp_mark, false))
		return 0;
	if (!__cmp(ATTR_TIMEOUT, ct1, ct2, flags, cmp_timeout, true))
		return 0;
	if (!__cmp(ATTR_STATUS, ct1, ct2, flags, cmp_status, true))
		return 0;
	if (!__cmp(ATTR_TCP_STATE, ct1, ct2, flags, cmp_tcp_state, true))
		return 0;
	if (!__cmp(ATTR_SCTP_STATE, ct1, ct2, flags, cmp_sctp_state, true))
		return 0;
	if (!__cmp(ATTR_DCCP_STATE, ct1, ct2, flags, cmp_dccp_state, true))
		return 0;
	if (!__cmp(ATTR_ZONE, ct1, ct2, flags, cmp_zone, false))
		return 0;
	if (!__cmp(ATTR_SECCTX, ct1, ct2, flags, cmp_secctx, true))
		return 0;
	if (!__cmp(ATTR_CONNLABELS, ct1, ct2, flags, cmp_clabel, true))
		return 0;
	if (!__cmp(ATTR_CONNLABELS_MASK, ct1, ct2, flags, cmp_clabel_mask, true))
		return 0;

	return 1;
}

int __compare(const struct nf_conntrack *ct1,
	      const struct nf_conntrack *ct2,
	      unsigned int flags)
{
	if ((flags & ~(NFCT_CMP_MASK|NFCT_CMP_STRICT)) == NFCT_CMP_ALL)
		return cmp_meta(ct1, ct2, flags) &&
		       __cmp_orig(ct1, ct2, flags) &&
		       cmp_repl(ct1, ct2, flags);

	if (flags & NFCT_CMP_ORIG && !__cmp_orig(ct1, ct2, flags))
		return 0;

	if (flags & NFCT_CMP_REPL && !cmp_repl(ct1, ct2, flags))
		return 0;

	return 1;
}
