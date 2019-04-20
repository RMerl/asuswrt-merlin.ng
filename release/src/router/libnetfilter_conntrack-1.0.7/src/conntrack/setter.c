/*
 * (C) 2005-2011 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "internal/internal.h"
#include <linux/icmp.h>
#include <linux/icmpv6.h>

static const uint8_t invmap_icmp[] = {
	[ICMP_ECHO]		= ICMP_ECHOREPLY + 1,
	[ICMP_ECHOREPLY]	= ICMP_ECHO + 1,
	[ICMP_TIMESTAMP]	= ICMP_TIMESTAMPREPLY + 1,
	[ICMP_TIMESTAMPREPLY]	= ICMP_TIMESTAMP + 1,
	[ICMP_INFO_REQUEST]	= ICMP_INFO_REPLY + 1,
	[ICMP_INFO_REPLY]	= ICMP_INFO_REQUEST + 1,
	[ICMP_ADDRESS]		= ICMP_ADDRESSREPLY + 1,
	[ICMP_ADDRESSREPLY]	= ICMP_ADDRESS + 1
};

#ifndef ICMPV6_NI_QUERY
#define ICMPV6_NI_QUERY 139
#endif

#ifndef ICMPV6_NI_REPLY
#define ICMPV6_NI_REPLY 140
#endif

static const uint8_t invmap_icmpv6[] = {
	[ICMPV6_ECHO_REQUEST - 128]	= ICMPV6_ECHO_REPLY + 1,
	[ICMPV6_ECHO_REPLY - 128]	= ICMPV6_ECHO_REQUEST + 1,
	[ICMPV6_NI_QUERY - 128]		= ICMPV6_NI_QUERY + 1,
	[ICMPV6_NI_REPLY - 128]		= ICMPV6_NI_REPLY + 1
};

static void
set_attr_orig_ipv4_src(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->head.orig.src.v4 = *((uint32_t *) value);
}

static void
set_attr_orig_ipv4_dst(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->head.orig.dst.v4 = *((uint32_t *) value);
}

static void
set_attr_repl_ipv4_src(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->repl.src.v4 = *((uint32_t *) value);
}

static void
set_attr_repl_ipv4_dst(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->repl.dst.v4 = *((uint32_t *) value);
}

static void
set_attr_orig_ipv6_src(struct nf_conntrack *ct, const void *value, size_t len)
{
	memcpy(&ct->head.orig.src.v6, value, sizeof(uint32_t)*4);
}

static void
set_attr_orig_ipv6_dst(struct nf_conntrack *ct, const void *value, size_t len)
{
	memcpy(&ct->head.orig.dst.v6, value, sizeof(uint32_t)*4);
}

static void
set_attr_repl_ipv6_src(struct nf_conntrack *ct, const void *value, size_t len)
{
	memcpy(&ct->repl.src.v6, value, sizeof(uint32_t)*4);
}

static void
set_attr_repl_ipv6_dst(struct nf_conntrack *ct, const void *value, size_t len)
{
	memcpy(&ct->repl.dst.v6, value, sizeof(uint32_t)*4);
}

static void
set_attr_orig_port_src(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->head.orig.l4src.all = *((uint16_t *) value);
}

static void
set_attr_orig_port_dst(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->head.orig.l4dst.all = *((uint16_t *) value);
}

static void
set_attr_repl_port_src(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->repl.l4src.all = *((uint16_t *) value);
}

static void
set_attr_repl_port_dst(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->repl.l4dst.all = *((uint16_t *) value);
}

static void
set_attr_orig_zone(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->head.orig.zone = *((uint16_t *) value);
}

static void
set_attr_repl_zone(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->repl.zone = *((uint16_t *) value);
}

static void
set_attr_icmp_type(struct nf_conntrack *ct, const void *value, size_t len)
{
	uint8_t rtype;

	ct->head.orig.l4dst.icmp.type = *((uint8_t *) value);

	switch(ct->head.orig.l3protonum) {
		case AF_INET:
			rtype = invmap_icmp[*((uint8_t *) value)];
			break;

		case AF_INET6:
			rtype = invmap_icmpv6[*((uint8_t *) value) - 128];
			break;

		default:
			rtype = 0;	/* not found */
	}

	if (rtype)
		ct->repl.l4dst.icmp.type = rtype - 1;
	else
		ct->repl.l4dst.icmp.type = 255;	/* will fail with -EINVAL */

}

static void
set_attr_icmp_code(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->head.orig.l4dst.icmp.code = *((uint8_t *) value);
	ct->repl.l4dst.icmp.code = *((uint8_t *) value);
}

static void
set_attr_icmp_id(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->head.orig.l4src.icmp.id = *((uint16_t *) value);
	ct->repl.l4src.icmp.id = *((uint16_t *) value);
}

static void
set_attr_orig_l3proto(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->head.orig.l3protonum = *((uint8_t *) value);
}

static void
set_attr_repl_l3proto(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->repl.l3protonum = *((uint8_t *) value);
}

static void
set_attr_orig_l4proto(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->head.orig.protonum = *((uint8_t *) value);
}

static void
set_attr_repl_l4proto(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->repl.protonum = *((uint8_t *) value);
}

static void
set_attr_tcp_state(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->protoinfo.tcp.state = *((uint8_t *) value);
}

static void
set_attr_tcp_flags_orig(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->protoinfo.tcp.flags[__DIR_ORIG].value = *((uint8_t *) value);
}

static void
set_attr_tcp_mask_orig(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->protoinfo.tcp.flags[__DIR_ORIG].mask = *((uint8_t *) value);
}

static void
set_attr_tcp_flags_repl(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->protoinfo.tcp.flags[__DIR_REPL].value = *((uint8_t *) value);
}

static void
set_attr_tcp_mask_repl(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->protoinfo.tcp.flags[__DIR_REPL].mask = *((uint8_t *) value);
}

static void
set_attr_sctp_state(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->protoinfo.sctp.state = *((uint8_t *) value);
}

static void
set_attr_sctp_vtag_orig(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->protoinfo.sctp.vtag[__DIR_ORIG] = *((uint32_t *) value);
}

static void
set_attr_sctp_vtag_repl(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->protoinfo.sctp.vtag[__DIR_REPL] = *((uint32_t *) value);
}

static void
set_attr_snat_ipv4(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->snat.min_ip.v4 = ct->snat.max_ip.v4 = *((uint32_t *) value);
}

static void
set_attr_dnat_ipv4(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->dnat.min_ip.v4 = ct->dnat.max_ip.v4 = *((uint32_t *) value);
}

static void
set_attr_snat_ipv6(struct nf_conntrack *ct, const void *value, size_t len)
{
	memcpy(&ct->snat.min_ip.v6, value, sizeof(struct in6_addr));
	memcpy(&ct->snat.max_ip.v6, value, sizeof(struct in6_addr));
}

static void
set_attr_dnat_ipv6(struct nf_conntrack *ct, const void *value, size_t len)
{
	memcpy(&ct->dnat.min_ip.v6, value, sizeof(struct in6_addr));
	memcpy(&ct->dnat.max_ip.v6, value, sizeof(struct in6_addr));
}

static void
set_attr_snat_port(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->snat.l4min.all = ct->snat.l4max.all = *((uint16_t *) value);
}

static void
set_attr_dnat_port(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->dnat.l4min.all = ct->dnat.l4max.all = *((uint16_t *) value);
}

static void
set_attr_timeout(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->timeout = *((uint32_t *) value);
}

static void
set_attr_mark(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->mark = *((uint32_t *) value);
}

static void
set_attr_secmark(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->secmark = *((uint32_t *) value);
}

static void
set_attr_status(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->status = *((uint32_t *) value);
}

static void
set_attr_id(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->id = *((uint32_t *) value);
}

static void
set_attr_master_ipv4_src(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->master.src.v4 = *((uint32_t *) value);
}

static void
set_attr_master_ipv4_dst(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->master.dst.v4 = *((uint32_t *) value);
}

static void
set_attr_master_ipv6_src(struct nf_conntrack *ct, const void *value, size_t len)
{
	memcpy(&ct->master.src.v6, value, sizeof(uint32_t)*4);
}

static void
set_attr_master_ipv6_dst(struct nf_conntrack *ct, const void *value, size_t len)
{
	memcpy(&ct->master.dst.v6, value, sizeof(uint32_t)*4);
}

static void
set_attr_master_port_src(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->master.l4src.all = *((uint16_t *) value);
}

static void
set_attr_master_port_dst(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->master.l4dst.all = *((uint16_t *) value);
}

static void
set_attr_master_l3proto(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->master.l3protonum = *((uint8_t *) value);
}

static void
set_attr_master_l4proto(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->master.protonum = *((uint8_t *) value);
}

static void
set_attr_orig_cor_pos(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->natseq[__DIR_ORIG].correction_pos = *((uint32_t *) value);
}

static void
set_attr_orig_off_bfr(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->natseq[__DIR_ORIG].offset_before = *((uint32_t *) value);
}

static void
set_attr_orig_off_aft(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->natseq[__DIR_ORIG].offset_after = *((uint32_t *) value);
}

static void
set_attr_repl_cor_pos(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->natseq[__DIR_REPL].correction_pos = *((uint32_t *) value);
}

static void
set_attr_repl_off_bfr(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->natseq[__DIR_REPL].offset_before = *((uint32_t *) value);
}

static void
set_attr_repl_off_aft(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->natseq[__DIR_REPL].offset_after = *((uint32_t *) value);
}

static void
set_attr_helper_name(struct nf_conntrack *ct, const void *value, size_t len)
{
	strncpy(ct->helper_name, value, NFCT_HELPER_NAME_MAX);
	ct->helper_name[NFCT_HELPER_NAME_MAX-1] = '\0';
}

static void
set_attr_dccp_state(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->protoinfo.dccp.state = *((uint8_t *) value);
}

static void
set_attr_dccp_role(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->protoinfo.dccp.role = *((uint8_t *) value);
}

static void
set_attr_dccp_handshake_seq(struct nf_conntrack *ct, const void *value,
				size_t len)
{
	ct->protoinfo.dccp.handshake_seq = *((uint64_t *) value);
}

static void
set_attr_tcp_wscale_orig(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->protoinfo.tcp.wscale[__DIR_ORIG] = *((uint8_t *) value);
}

static void
set_attr_tcp_wscale_repl(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->protoinfo.tcp.wscale[__DIR_REPL] = *((uint8_t *) value);
}

static void
set_attr_zone(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->zone = *((uint16_t *) value);
}

static void
set_attr_helper_info(struct nf_conntrack *ct, const void *value, size_t len)
{
	if (ct->helper_info == NULL) {
retry:
		ct->helper_info = calloc(1, len);
		if (ct->helper_info == NULL)
			return;

		memcpy(ct->helper_info, value, len);
	} else {
		free(ct->helper_info);
		goto retry;
	}
}

static void
do_set_attr_connlabels(struct nfct_bitmask *current, const void *value)
{
	if (current && current != value)
		nfct_bitmask_destroy(current);
}

static void
set_attr_connlabels(struct nf_conntrack *ct, const void *value, size_t len)
{
	do_set_attr_connlabels(ct->connlabels, value);
	ct->connlabels = (void *) value;
}

static void
set_attr_connlabels_mask(struct nf_conntrack *ct, const void *value, size_t len)
{
	do_set_attr_connlabels(ct->connlabels_mask, value);
	ct->connlabels_mask = (void *) value;
}

static void
set_attr_synproxy_isn(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->synproxy.isn = *((uint32_t *) value);
}

static void
set_attr_synproxy_its(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->synproxy.its = *((uint32_t *) value);
}

static void
set_attr_synproxy_tsoff(struct nf_conntrack *ct, const void *value, size_t len)
{
	ct->synproxy.tsoff = *((uint32_t *) value);
}

static void
set_attr_do_nothing(struct nf_conntrack *ct, const void *value, size_t len) {}

const set_attr set_attr_array[ATTR_MAX] = {
	[ATTR_ORIG_IPV4_SRC]	= set_attr_orig_ipv4_src,
	[ATTR_ORIG_IPV4_DST] 	= set_attr_orig_ipv4_dst,
	[ATTR_REPL_IPV4_SRC]	= set_attr_repl_ipv4_src,
	[ATTR_REPL_IPV4_DST]	= set_attr_repl_ipv4_dst,
	[ATTR_ORIG_IPV6_SRC]	= set_attr_orig_ipv6_src,
	[ATTR_ORIG_IPV6_DST]	= set_attr_orig_ipv6_dst,
	[ATTR_REPL_IPV6_SRC]	= set_attr_repl_ipv6_src,
	[ATTR_REPL_IPV6_DST]	= set_attr_repl_ipv6_dst,
	[ATTR_ORIG_PORT_SRC]	= set_attr_orig_port_src,
	[ATTR_ORIG_PORT_DST]	= set_attr_orig_port_dst,
	[ATTR_REPL_PORT_SRC]	= set_attr_repl_port_src,
	[ATTR_REPL_PORT_DST]	= set_attr_repl_port_dst,
	[ATTR_ICMP_TYPE]	= set_attr_icmp_type,
	[ATTR_ICMP_CODE]	= set_attr_icmp_code,
	[ATTR_ICMP_ID]		= set_attr_icmp_id,
	[ATTR_ORIG_L3PROTO]	= set_attr_orig_l3proto,
	[ATTR_REPL_L3PROTO]	= set_attr_repl_l3proto,
	[ATTR_ORIG_L4PROTO]	= set_attr_orig_l4proto,
	[ATTR_REPL_L4PROTO]	= set_attr_repl_l4proto,
	[ATTR_TCP_STATE]	= set_attr_tcp_state,
	[ATTR_SNAT_IPV4]	= set_attr_snat_ipv4,
	[ATTR_DNAT_IPV4]	= set_attr_dnat_ipv4,
	[ATTR_SNAT_PORT]	= set_attr_snat_port,
	[ATTR_DNAT_PORT]	= set_attr_dnat_port,
	[ATTR_TIMEOUT]		= set_attr_timeout,
	[ATTR_MARK]		= set_attr_mark,
	[ATTR_ORIG_COUNTER_PACKETS]	= set_attr_do_nothing,
	[ATTR_REPL_COUNTER_PACKETS]	= set_attr_do_nothing,
	[ATTR_ORIG_COUNTER_BYTES]	= set_attr_do_nothing,
	[ATTR_REPL_COUNTER_BYTES]	= set_attr_do_nothing,
	[ATTR_USE]		= set_attr_do_nothing,
	[ATTR_ID]		= set_attr_id,
	[ATTR_STATUS]		= set_attr_status,
	[ATTR_TCP_FLAGS_ORIG]	= set_attr_tcp_flags_orig,
	[ATTR_TCP_FLAGS_REPL]	= set_attr_tcp_flags_repl,
	[ATTR_TCP_MASK_ORIG]	= set_attr_tcp_mask_orig,
	[ATTR_TCP_MASK_REPL]	= set_attr_tcp_mask_repl,
	[ATTR_MASTER_IPV4_SRC]	= set_attr_master_ipv4_src,
	[ATTR_MASTER_IPV4_DST]	= set_attr_master_ipv4_dst,
	[ATTR_MASTER_IPV6_SRC]	= set_attr_master_ipv6_src,
	[ATTR_MASTER_IPV6_DST]	= set_attr_master_ipv6_dst,
	[ATTR_MASTER_PORT_SRC]	= set_attr_master_port_src,
	[ATTR_MASTER_PORT_DST]	= set_attr_master_port_dst,
	[ATTR_MASTER_L3PROTO]	= set_attr_master_l3proto,
	[ATTR_MASTER_L4PROTO]	= set_attr_master_l4proto,
	[ATTR_SECMARK]		= set_attr_secmark,
	[ATTR_ORIG_NAT_SEQ_CORRECTION_POS] 	= set_attr_orig_cor_pos,
	[ATTR_ORIG_NAT_SEQ_OFFSET_BEFORE] 	= set_attr_orig_off_bfr,
	[ATTR_ORIG_NAT_SEQ_OFFSET_AFTER] 	= set_attr_orig_off_aft,
	[ATTR_REPL_NAT_SEQ_CORRECTION_POS] 	= set_attr_repl_cor_pos,
	[ATTR_REPL_NAT_SEQ_OFFSET_BEFORE] 	= set_attr_repl_off_bfr,
	[ATTR_REPL_NAT_SEQ_OFFSET_AFTER] 	= set_attr_repl_off_aft,
	[ATTR_SCTP_STATE]	= set_attr_sctp_state,
	[ATTR_SCTP_VTAG_ORIG]	= set_attr_sctp_vtag_orig,
	[ATTR_SCTP_VTAG_REPL]	= set_attr_sctp_vtag_repl,
	[ATTR_HELPER_NAME]	= set_attr_helper_name,
	[ATTR_DCCP_STATE]	= set_attr_dccp_state,
	[ATTR_DCCP_ROLE]	= set_attr_dccp_role,
	[ATTR_DCCP_HANDSHAKE_SEQ] = set_attr_dccp_handshake_seq,
	[ATTR_TCP_WSCALE_ORIG]	= set_attr_tcp_wscale_orig,
	[ATTR_TCP_WSCALE_REPL]	= set_attr_tcp_wscale_repl,
	[ATTR_ZONE]		= set_attr_zone,
	[ATTR_ORIG_ZONE]	= set_attr_orig_zone,
	[ATTR_REPL_ZONE]	= set_attr_repl_zone,
	[ATTR_SECCTX]		= set_attr_do_nothing,
	[ATTR_TIMESTAMP_START]	= set_attr_do_nothing,
	[ATTR_TIMESTAMP_STOP]	= set_attr_do_nothing,
	[ATTR_HELPER_INFO]	= set_attr_helper_info,
	[ATTR_CONNLABELS]	= set_attr_connlabels,
	[ATTR_CONNLABELS_MASK]	= set_attr_connlabels_mask,
	[ATTR_SNAT_IPV6]	= set_attr_snat_ipv6,
	[ATTR_DNAT_IPV6]	= set_attr_dnat_ipv6,
	[ATTR_SYNPROXY_ISN]	= set_attr_synproxy_isn,
	[ATTR_SYNPROXY_ITS]	= set_attr_synproxy_its,
	[ATTR_SYNPROXY_TSOFF]	= set_attr_synproxy_tsoff,
};
