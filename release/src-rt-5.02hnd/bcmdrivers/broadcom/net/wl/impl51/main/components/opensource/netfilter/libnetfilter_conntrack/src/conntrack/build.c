/*
 * (C) 2005-2011 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "internal/internal.h"

static void __build_tuple_ip(struct nfnlhdr *req,
			     size_t size,
			     const struct __nfct_tuple *t)
{
	struct nfattr *nest;

	nest = nfnl_nest(&req->nlh, size, CTA_TUPLE_IP);

	switch(t->l3protonum) {
	case AF_INET:
	        nfnl_addattr_l(&req->nlh, size, CTA_IP_V4_SRC, &t->src.v4,
			       sizeof(u_int32_t));
		nfnl_addattr_l(&req->nlh, size, CTA_IP_V4_DST, &t->dst.v4,
			       sizeof(u_int32_t));
		break;
	case AF_INET6:
		nfnl_addattr_l(&req->nlh, size, CTA_IP_V6_SRC, &t->src.v6,
			       sizeof(struct in6_addr));
		nfnl_addattr_l(&req->nlh, size, CTA_IP_V6_DST, &t->dst.v6,
			       sizeof(struct in6_addr));
		break;
	default:
		break;
	}

	nfnl_nest_end(&req->nlh, nest);
}

static void __build_tuple_proto(struct nfnlhdr *req,
				size_t size,
				const struct __nfct_tuple *t)
{
	struct nfattr *nest;

	nest = nfnl_nest(&req->nlh, size, CTA_TUPLE_PROTO);

	nfnl_addattr_l(&req->nlh, size, CTA_PROTO_NUM, &t->protonum,
		       sizeof(u_int8_t));

	switch(t->protonum) {
	case IPPROTO_UDP:
	case IPPROTO_TCP:
	case IPPROTO_SCTP:
	case IPPROTO_DCCP:
	case IPPROTO_GRE:
	case IPPROTO_UDPLITE:
		nfnl_addattr_l(&req->nlh, size, CTA_PROTO_SRC_PORT,
			       &t->l4src.tcp.port, sizeof(u_int16_t));
		nfnl_addattr_l(&req->nlh, size, CTA_PROTO_DST_PORT,
			       &t->l4dst.tcp.port, sizeof(u_int16_t));
		break;

	case IPPROTO_ICMP:
		nfnl_addattr_l(&req->nlh, size, CTA_PROTO_ICMP_CODE,
			       &t->l4dst.icmp.code, sizeof(u_int8_t));
		nfnl_addattr_l(&req->nlh, size, CTA_PROTO_ICMP_TYPE,
			       &t->l4dst.icmp.type, sizeof(u_int8_t));
		nfnl_addattr_l(&req->nlh, size, CTA_PROTO_ICMP_ID,
			       &t->l4src.icmp.id, sizeof(u_int16_t));
		break;

	case IPPROTO_ICMPV6:
		nfnl_addattr_l(&req->nlh, size, CTA_PROTO_ICMPV6_CODE,
			       &t->l4dst.icmp.code, sizeof(u_int8_t));
		nfnl_addattr_l(&req->nlh, size, CTA_PROTO_ICMPV6_TYPE,
			       &t->l4dst.icmp.type, sizeof(u_int8_t));
		nfnl_addattr_l(&req->nlh, size, CTA_PROTO_ICMPV6_ID,
			       &t->l4src.icmp.id, sizeof(u_int16_t));
		break;

	default:
		break;
	}

	nfnl_nest_end(&req->nlh, nest);
}

void __build_tuple(struct nfnlhdr *req, 
		   size_t size, 
		   const struct __nfct_tuple *t, 
		   const int type)
{
	struct nfattr *nest;

	nest = nfnl_nest(&req->nlh, size, type);

	__build_tuple_ip(req, size, t);
	__build_tuple_proto(req, size, t);

	nfnl_nest_end(&req->nlh, nest);
}

static void __build_protoinfo(struct nfnlhdr *req, size_t size,
			      const struct nf_conntrack *ct)
{
	struct nfattr *nest, *nest_proto;

	switch(ct->head.orig.protonum) {
	case IPPROTO_TCP:
		/* Preliminary attribute check to avoid sending an empty
		 * CTA_PROTOINFO_TCP nest, which results in EINVAL in
		 * Linux kernel <= 2.6.25. */
		if (!(test_bit(ATTR_TCP_STATE, ct->head.set) ||
		      test_bit(ATTR_TCP_FLAGS_ORIG, ct->head.set) ||
		      test_bit(ATTR_TCP_FLAGS_REPL, ct->head.set) ||
		      test_bit(ATTR_TCP_MASK_ORIG, ct->head.set) ||
		      test_bit(ATTR_TCP_MASK_REPL, ct->head.set) ||
		      test_bit(ATTR_TCP_WSCALE_ORIG, ct->head.set) ||
		      test_bit(ATTR_TCP_WSCALE_REPL, ct->head.set))) {
			break;
		}
		nest = nfnl_nest(&req->nlh, size, CTA_PROTOINFO);
		nest_proto = nfnl_nest(&req->nlh, size, CTA_PROTOINFO_TCP);
		if (test_bit(ATTR_TCP_STATE, ct->head.set))
			nfnl_addattr_l(&req->nlh, size,
				       CTA_PROTOINFO_TCP_STATE,
				       &ct->protoinfo.tcp.state,
				       sizeof(u_int8_t));
		if (test_bit(ATTR_TCP_FLAGS_ORIG, ct->head.set) &&
		    test_bit(ATTR_TCP_MASK_ORIG, ct->head.set))
			nfnl_addattr_l(&req->nlh, size,
				       CTA_PROTOINFO_TCP_FLAGS_ORIGINAL,
				       &ct->protoinfo.tcp.flags[0], 
				       sizeof(struct nf_ct_tcp_flags));
		if (test_bit(ATTR_TCP_FLAGS_REPL, ct->head.set) &&
		    test_bit(ATTR_TCP_MASK_REPL, ct->head.set))
			nfnl_addattr_l(&req->nlh, size,
				       CTA_PROTOINFO_TCP_FLAGS_REPLY,
				       &ct->protoinfo.tcp.flags[1], 
				       sizeof(struct nf_ct_tcp_flags));
		if (test_bit(ATTR_TCP_WSCALE_ORIG, ct->head.set))
			nfnl_addattr_l(&req->nlh, size,
				       CTA_PROTOINFO_TCP_WSCALE_ORIGINAL,
				       &ct->protoinfo.tcp.wscale[__DIR_ORIG],
				       sizeof(u_int8_t));
		if (test_bit(ATTR_TCP_WSCALE_REPL, ct->head.set))
			nfnl_addattr_l(&req->nlh, size,
				       CTA_PROTOINFO_TCP_WSCALE_REPLY,
				       &ct->protoinfo.tcp.wscale[__DIR_REPL],
				       sizeof(u_int8_t));
		nfnl_nest_end(&req->nlh, nest_proto);
		nfnl_nest_end(&req->nlh, nest);
		break;
	case IPPROTO_SCTP:
		/* See comment above on TCP. */
		if (!(test_bit(ATTR_SCTP_STATE, ct->head.set) ||
		      test_bit(ATTR_SCTP_VTAG_ORIG, ct->head.set) ||
		      test_bit(ATTR_SCTP_VTAG_REPL, ct->head.set))) {
			break;
		}
		nest = nfnl_nest(&req->nlh, size, CTA_PROTOINFO);
		nest_proto = nfnl_nest(&req->nlh, size, CTA_PROTOINFO_SCTP);
		if (test_bit(ATTR_SCTP_STATE, ct->head.set))
			nfnl_addattr_l(&req->nlh, size,
				       CTA_PROTOINFO_SCTP_STATE,
				       &ct->protoinfo.sctp.state,
				       sizeof(u_int8_t));
		if (test_bit(ATTR_SCTP_VTAG_ORIG, ct->head.set))
			nfnl_addattr32(&req->nlh, size,
				    CTA_PROTOINFO_SCTP_VTAG_ORIGINAL,
				    htonl(ct->protoinfo.sctp.vtag[__DIR_ORIG]));
		if (test_bit(ATTR_SCTP_VTAG_REPL, ct->head.set))
			nfnl_addattr32(&req->nlh, size,
				    CTA_PROTOINFO_SCTP_VTAG_REPLY,
				    htonl(ct->protoinfo.sctp.vtag[__DIR_REPL]));
		nfnl_nest_end(&req->nlh, nest_proto);
		nfnl_nest_end(&req->nlh, nest);
		break;
	case IPPROTO_DCCP:
		/* See comment above on TCP. */
		if (!(test_bit(ATTR_DCCP_STATE, ct->head.set) ||
		      test_bit(ATTR_DCCP_ROLE, ct->head.set) ||
		      test_bit(ATTR_DCCP_HANDSHAKE_SEQ, ct->head.set))) {
			break;
		}
		nest = nfnl_nest(&req->nlh, size, CTA_PROTOINFO);
		nest_proto = nfnl_nest(&req->nlh, size, CTA_PROTOINFO_DCCP);
		if (test_bit(ATTR_DCCP_STATE, ct->head.set))
			nfnl_addattr_l(&req->nlh, size,
				       CTA_PROTOINFO_DCCP_STATE,
				       &ct->protoinfo.dccp.state,
				       sizeof(u_int8_t));
		if (test_bit(ATTR_DCCP_ROLE, ct->head.set))
			nfnl_addattr_l(&req->nlh, size,
				       CTA_PROTOINFO_DCCP_ROLE,
				       &ct->protoinfo.dccp.role,
				       sizeof(u_int8_t));
		if (test_bit(ATTR_DCCP_HANDSHAKE_SEQ, ct->head.set)) {
			/* FIXME: use __cpu_to_be64() instead which is the
			 * correct operation. This is a semantic abuse but
			 * we have no function to do it in libnfnetlink. */
			u_int64_t handshake_seq =
				__be64_to_cpu(ct->protoinfo.dccp.handshake_seq);

			nfnl_addattr_l(&req->nlh, size,
				       CTA_PROTOINFO_DCCP_HANDSHAKE_SEQ,
				       &handshake_seq,
				       sizeof(u_int64_t));
		}
		nfnl_nest_end(&req->nlh, nest_proto);
		nfnl_nest_end(&req->nlh, nest);
	default:
		break;
	}
}

static inline void 
__nat_seq_adj(struct nfnlhdr *req,
	      size_t size,
	      const struct nf_conntrack *ct,
	      int dir)
{
	nfnl_addattr32(&req->nlh, 
		       size, 
		       CTA_NAT_SEQ_CORRECTION_POS,
		       htonl(ct->natseq[dir].correction_pos));
	nfnl_addattr32(&req->nlh, 
		       size, 
		       CTA_NAT_SEQ_OFFSET_BEFORE,
		       htonl(ct->natseq[dir].offset_before));
	nfnl_addattr32(&req->nlh, 
		       size, 
		       CTA_NAT_SEQ_OFFSET_AFTER,
		       htonl(ct->natseq[dir].offset_after));
}

static void 
__build_nat_seq_adj(struct nfnlhdr *req, 
		    size_t size,
		    const struct nf_conntrack *ct,
		    int dir)
{
	struct nfattr *nest;
	int type = (dir == __DIR_ORIG) ? CTA_NAT_SEQ_ADJ_ORIG : 
					 CTA_NAT_SEQ_ADJ_REPLY;

	nest = nfnl_nest(&req->nlh, size, type);
	__nat_seq_adj(req, size, ct, dir);
	nfnl_nest_end(&req->nlh, nest);
}

static void __build_protonat(struct nfnlhdr *req,
			     size_t size,
			     const struct nf_conntrack *ct,
			     const struct __nfct_nat *nat)
{
	struct nfattr *nest;

	nest = nfnl_nest(&req->nlh, size, CTA_NAT_PROTO);

	switch (ct->head.orig.protonum) {
	case IPPROTO_TCP:
	case IPPROTO_UDP:
		nfnl_addattr_l(&req->nlh, size, CTA_PROTONAT_PORT_MIN,
			       &nat->l4min.tcp.port, sizeof(u_int16_t));
		nfnl_addattr_l(&req->nlh, size, CTA_PROTONAT_PORT_MAX,
			       &nat->l4max.tcp.port, sizeof(u_int16_t));
		break;
	}
	nfnl_nest_end(&req->nlh, nest);
}

static void __build_nat(struct nfnlhdr *req,
			size_t size,
			const struct __nfct_nat *nat)
{
	nfnl_addattr_l(&req->nlh, size, CTA_NAT_MINIP,
		       &nat->min_ip, sizeof(u_int32_t));
}

static void __build_snat(struct nfnlhdr *req,
			 size_t size,
			 const struct nf_conntrack *ct)
{
	struct nfattr *nest;

	nest = nfnl_nest(&req->nlh, size, CTA_NAT_SRC);
	__build_nat(req, size, &ct->snat);
	__build_protonat(req, size, ct, &ct->snat);
	nfnl_nest_end(&req->nlh, nest);
}

static void __build_snat_ipv4(struct nfnlhdr *req,
			      size_t size,
			      const struct nf_conntrack *ct)
{
	struct nfattr *nest;

	nest = nfnl_nest(&req->nlh, size, CTA_NAT_SRC);
	__build_nat(req, size, &ct->snat);
	nfnl_nest_end(&req->nlh, nest);
}

static void __build_snat_port(struct nfnlhdr *req,
			      size_t size,
			      const struct nf_conntrack *ct)
{
	struct nfattr *nest;

	nest = nfnl_nest(&req->nlh, size, CTA_NAT_SRC);
	__build_protonat(req, size, ct, &ct->snat);
	nfnl_nest_end(&req->nlh, nest);
}

static void __build_dnat(struct nfnlhdr *req,
			 size_t size,
			 const struct nf_conntrack *ct)
{
	struct nfattr *nest;

	nest = nfnl_nest(&req->nlh, size, CTA_NAT_DST);
	__build_nat(req, size, &ct->dnat);
	__build_protonat(req, size, ct, &ct->dnat);
	nfnl_nest_end(&req->nlh, nest);
}

static void __build_dnat_ipv4(struct nfnlhdr *req,
			      size_t size,
			      const struct nf_conntrack *ct)
{
	struct nfattr *nest;

	nest = nfnl_nest(&req->nlh, size, CTA_NAT_DST);
	__build_nat(req, size, &ct->dnat);
	nfnl_nest_end(&req->nlh, nest);
}

static void __build_dnat_port(struct nfnlhdr *req,
			      size_t size,
			      const struct nf_conntrack *ct)
{
	struct nfattr *nest;

	nest = nfnl_nest(&req->nlh, size, CTA_NAT_DST);
        __build_protonat(req, size, ct, &ct->dnat);
	nfnl_nest_end(&req->nlh, nest);
}

static void __build_status(struct nfnlhdr *req,
			   size_t size,
			   const struct nf_conntrack *ct)
{
	nfnl_addattr32(&req->nlh, size, CTA_STATUS,
		       htonl(ct->status | IPS_CONFIRMED));
}

static void __build_timeout(struct nfnlhdr *req,
			    size_t size,
			    const struct nf_conntrack *ct)
{
	nfnl_addattr32(&req->nlh, size, CTA_TIMEOUT, htonl(ct->timeout));
}

static void __build_mark(struct nfnlhdr *req,
			 size_t size,
			 const struct nf_conntrack *ct)
{
	nfnl_addattr32(&req->nlh, size, CTA_MARK, htonl(ct->mark));
}

static void __build_secmark(struct nfnlhdr *req,
			    size_t size,
			    const struct nf_conntrack *ct)
{
	nfnl_addattr32(&req->nlh, size, CTA_SECMARK, htonl(ct->secmark));
}

static void __build_helper_name(struct nfnlhdr *req,
				size_t size,
				const struct nf_conntrack *ct)
{
	struct nfattr *nest;

	nest = nfnl_nest(&req->nlh, size, CTA_HELP);
	nfnl_addattr_l(&req->nlh,
		       size, 
		       CTA_HELP_NAME,
		       ct->helper_name,
		       strlen(ct->helper_name)+1);
	nfnl_nest_end(&req->nlh, nest);
}

static void __build_zone(struct nfnlhdr *req,
			 size_t size,
			 const struct nf_conntrack *ct)
{
	nfnl_addattr16(&req->nlh, size, CTA_ZONE, htons(ct->zone));
}

int __build_conntrack(struct nfnl_subsys_handle *ssh,
		      struct nfnlhdr *req,
		      size_t size,
		      u_int16_t type,
		      u_int16_t flags,
		      const struct nf_conntrack *ct)
{
	u_int8_t l3num = ct->head.orig.l3protonum;

	if (!test_bit(ATTR_ORIG_L3PROTO, ct->head.set)) {
		errno = EINVAL;
		return -1;
	}

	memset(req, 0, size);

	nfnl_fill_hdr(ssh, &req->nlh, 0, l3num, 0, type, flags);

	if (test_bit(ATTR_ORIG_IPV4_SRC, ct->head.set) ||
	    test_bit(ATTR_ORIG_IPV4_DST, ct->head.set) ||
	    test_bit(ATTR_ORIG_IPV6_SRC, ct->head.set) ||
	    test_bit(ATTR_ORIG_IPV6_DST, ct->head.set) ||
	    test_bit(ATTR_ORIG_PORT_SRC, ct->head.set) ||
	    test_bit(ATTR_ORIG_PORT_DST, ct->head.set) ||
	    test_bit(ATTR_ORIG_L3PROTO, ct->head.set)  ||
	    test_bit(ATTR_ORIG_L4PROTO, ct->head.set)  ||
	    test_bit(ATTR_ICMP_TYPE, ct->head.set) 	  ||
	    test_bit(ATTR_ICMP_CODE, ct->head.set)	  ||
	    test_bit(ATTR_ICMP_ID, ct->head.set))
		__build_tuple(req, size, &ct->head.orig, CTA_TUPLE_ORIG);

	if (test_bit(ATTR_REPL_IPV4_SRC, ct->head.set) ||
	    test_bit(ATTR_REPL_IPV4_DST, ct->head.set) ||
	    test_bit(ATTR_REPL_IPV6_SRC, ct->head.set) ||
	    test_bit(ATTR_REPL_IPV6_DST, ct->head.set) ||
	    test_bit(ATTR_REPL_PORT_SRC, ct->head.set) ||
	    test_bit(ATTR_REPL_PORT_DST, ct->head.set) ||
	    test_bit(ATTR_REPL_L3PROTO, ct->head.set)  ||
	    test_bit(ATTR_REPL_L4PROTO, ct->head.set))
		__build_tuple(req, size, &ct->repl, CTA_TUPLE_REPLY);

	if (test_bit(ATTR_MASTER_IPV4_SRC, ct->head.set) ||
	    test_bit(ATTR_MASTER_IPV4_DST, ct->head.set) ||
	    test_bit(ATTR_MASTER_IPV6_SRC, ct->head.set) ||
	    test_bit(ATTR_MASTER_IPV6_DST, ct->head.set) ||
	    test_bit(ATTR_MASTER_PORT_SRC, ct->head.set) ||
	    test_bit(ATTR_MASTER_PORT_DST, ct->head.set) ||
	    test_bit(ATTR_MASTER_L3PROTO, ct->head.set) ||
	    test_bit(ATTR_MASTER_L4PROTO, ct->head.set))
	    	__build_tuple(req, size, &ct->master, CTA_TUPLE_MASTER);

	if (test_bit(ATTR_STATUS, ct->head.set))
		__build_status(req, size, ct);
	else {
		/* build IPS_CONFIRMED if we're creating a new conntrack */
		if (type == IPCTNL_MSG_CT_NEW && flags & NLM_F_CREATE)
			__build_status(req, size, ct);
	}

	if (test_bit(ATTR_TIMEOUT, ct->head.set))
		__build_timeout(req, size, ct);

	if (test_bit(ATTR_MARK, ct->head.set))
		__build_mark(req, size, ct);

	if (test_bit(ATTR_SECMARK, ct->head.set))
		__build_secmark(req, size, ct);

	__build_protoinfo(req, size, ct);

	if (test_bit(ATTR_SNAT_IPV4, ct->head.set) && 
	    test_bit(ATTR_SNAT_PORT, ct->head.set))
		__build_snat(req, size, ct);
	else if (test_bit(ATTR_SNAT_IPV4, ct->head.set))
		__build_snat_ipv4(req, size, ct);
	else if (test_bit(ATTR_SNAT_PORT, ct->head.set))
		__build_snat_port(req, size, ct);

	if (test_bit(ATTR_DNAT_IPV4, ct->head.set) &&
	    test_bit(ATTR_DNAT_PORT, ct->head.set))
		__build_dnat(req, size, ct);
	else if (test_bit(ATTR_DNAT_IPV4, ct->head.set))
		__build_dnat_ipv4(req, size, ct);
	else if (test_bit(ATTR_DNAT_PORT, ct->head.set))
		__build_dnat_port(req, size, ct);

	if (test_bit(ATTR_ORIG_NAT_SEQ_CORRECTION_POS, ct->head.set) &&
	    test_bit(ATTR_ORIG_NAT_SEQ_OFFSET_BEFORE, ct->head.set) &&
	    test_bit(ATTR_ORIG_NAT_SEQ_OFFSET_AFTER, ct->head.set))
	    	__build_nat_seq_adj(req, size, ct, __DIR_ORIG);

	if (test_bit(ATTR_REPL_NAT_SEQ_CORRECTION_POS, ct->head.set) &&
	    test_bit(ATTR_REPL_NAT_SEQ_OFFSET_BEFORE, ct->head.set) &&
	    test_bit(ATTR_REPL_NAT_SEQ_OFFSET_AFTER, ct->head.set))
	    	__build_nat_seq_adj(req, size, ct, __DIR_REPL);

	if (test_bit(ATTR_HELPER_NAME, ct->head.set))
		__build_helper_name(req, size, ct);

	if (test_bit(ATTR_ZONE, ct->head.set))
		__build_zone(req, size, ct);

	return 0;
}
