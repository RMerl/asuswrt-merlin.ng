/*
 * (C) 2005-2011 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "internal/internal.h"

static const void *get_attr_orig_ipv4_src(const struct nf_conntrack *ct)
{
	return &ct->head.orig.src.v4;
}

static const void *get_attr_orig_ipv4_dst(const struct nf_conntrack *ct)
{
	return &ct->head.orig.dst.v4;
}

static const void *get_attr_repl_ipv4_src(const struct nf_conntrack *ct)
{
	return &ct->repl.src.v4;
}

static const void *get_attr_repl_ipv4_dst(const struct nf_conntrack *ct)
{
	return &ct->repl.dst.v4;
}

static const void *get_attr_orig_ipv6_src(const struct nf_conntrack *ct)
{
	return &ct->head.orig.src.v6;
}

static const void *get_attr_orig_ipv6_dst(const struct nf_conntrack *ct)
{
	return &ct->head.orig.dst.v6;
}

static const void *get_attr_repl_ipv6_src(const struct nf_conntrack *ct)
{
	return &ct->repl.src.v6;
}

static const void *get_attr_repl_ipv6_dst(const struct nf_conntrack *ct)
{
	return &ct->repl.dst.v6;
}

static const void *get_attr_orig_port_src(const struct nf_conntrack *ct)
{
	return &ct->head.orig.l4src.all;
}

static const void *get_attr_orig_port_dst(const struct nf_conntrack *ct)
{
	return &ct->head.orig.l4dst.all;
}

static const void *get_attr_repl_port_src(const struct nf_conntrack *ct)
{
	return &ct->repl.l4src.all;
}

static const void *get_attr_repl_port_dst(const struct nf_conntrack *ct)
{
	return &ct->repl.l4dst.all;
}

static const void *get_attr_orig_zone(const struct nf_conntrack *ct)
{
	return &ct->head.orig.zone;
}

static const void *get_attr_repl_zone(const struct nf_conntrack *ct)
{
	return &ct->repl.zone;
}

static const void *get_attr_icmp_type(const struct nf_conntrack *ct)
{
	return &ct->head.orig.l4dst.icmp.type;
}

static const void *get_attr_icmp_code(const struct nf_conntrack *ct)
{
	return &ct->head.orig.l4dst.icmp.code;
}

static const void *get_attr_icmp_id(const struct nf_conntrack *ct)
{
	return &ct->head.orig.l4src.icmp.id;
}

static const void *get_attr_orig_l3proto(const struct nf_conntrack *ct)
{
	return &ct->head.orig.l3protonum;
}

static const void *get_attr_repl_l3proto(const struct nf_conntrack *ct)
{
	return &ct->repl.l3protonum;
}

static const void *get_attr_orig_l4proto(const struct nf_conntrack *ct)
{
	return &ct->head.orig.protonum;
}

static const void *get_attr_repl_l4proto(const struct nf_conntrack *ct)
{
	return &ct->repl.protonum;
}

static const void *get_attr_master_ipv4_src(const struct nf_conntrack *ct)
{
	return &ct->master.src.v4;
}

static const void *get_attr_master_ipv4_dst(const struct nf_conntrack *ct)
{
	return &ct->master.dst.v4;
}

static const void *get_attr_master_ipv6_src(const struct nf_conntrack *ct)
{
	return &ct->master.src.v6;
}

static const void *get_attr_master_ipv6_dst(const struct nf_conntrack *ct)
{
	return &ct->master.dst.v6;
}

static const void *get_attr_master_port_src(const struct nf_conntrack *ct)
{
	return &ct->master.l4src.all;
}

static const void *get_attr_master_port_dst(const struct nf_conntrack *ct)
{
	return &ct->master.l4dst.all;
}

static const void *get_attr_master_l3proto(const struct nf_conntrack *ct)
{
	return &ct->master.l3protonum;
}

static const void *get_attr_master_l4proto(const struct nf_conntrack *ct)
{
	return &ct->master.protonum;
}

static const void *get_attr_tcp_state(const struct nf_conntrack *ct)
{
	return &ct->protoinfo.tcp.state;
}

static const void *get_attr_tcp_flags_orig(const struct nf_conntrack *ct)
{
	return &ct->protoinfo.tcp.flags[__DIR_ORIG].value;
}

static const void *get_attr_tcp_mask_orig(const struct nf_conntrack *ct)
{
	return &ct->protoinfo.tcp.flags[__DIR_ORIG].mask;
}

static const void *get_attr_tcp_flags_repl(const struct nf_conntrack *ct)
{
	return &ct->protoinfo.tcp.flags[__DIR_REPL].value;
}

static const void *get_attr_tcp_mask_repl(const struct nf_conntrack *ct)
{
	return &ct->protoinfo.tcp.flags[__DIR_REPL].mask;
}

static const void *get_attr_tcp_wscale_orig(const struct nf_conntrack *ct)
{
	return &ct->protoinfo.tcp.wscale[__DIR_ORIG];
}

static const void *get_attr_tcp_wscale_repl(const struct nf_conntrack *ct)
{
	return &ct->protoinfo.tcp.wscale[__DIR_REPL];
}

static const void *get_attr_sctp_state(const struct nf_conntrack *ct)
{
	return &ct->protoinfo.sctp.state;
}

static const void *get_attr_sctp_vtag_orig(const struct nf_conntrack *ct)
{
	return &ct->protoinfo.sctp.vtag[__DIR_ORIG];
}

static const void *get_attr_sctp_vtag_repl(const struct nf_conntrack *ct)
{
	return &ct->protoinfo.sctp.vtag[__DIR_REPL];
}

static const void *get_attr_snat_ipv4(const struct nf_conntrack *ct)
{
	return &ct->snat.min_ip.v4;
}

static const void *get_attr_dnat_ipv4(const struct nf_conntrack *ct)
{
	return &ct->dnat.min_ip.v4;
}

static const void *get_attr_snat_ipv6(const struct nf_conntrack *ct)
{
	return &ct->snat.min_ip.v6;
}

static const void *get_attr_dnat_ipv6(const struct nf_conntrack *ct)
{
	return &ct->dnat.min_ip.v6;
}

static const void *get_attr_snat_port(const struct nf_conntrack *ct)
{
	return &ct->snat.l4min.all;
}

static const void *get_attr_dnat_port(const struct nf_conntrack *ct)
{
	return &ct->dnat.l4min.all;
}

static const void *get_attr_timeout(const struct nf_conntrack *ct)
{
	return &ct->timeout;
}

static const void *get_attr_mark(const struct nf_conntrack *ct)
{
	return &ct->mark;
}

static const void *get_attr_secmark(const struct nf_conntrack *ct)
{
	return &ct->secmark;
}

static const void *get_attr_orig_counter_packets(const struct nf_conntrack *ct)
{
	return &ct->counters[__DIR_ORIG].packets;
}

static const void *get_attr_orig_counter_bytes(const struct nf_conntrack *ct)
{
	return &ct->counters[__DIR_ORIG].bytes;
}

static const void *get_attr_repl_counter_packets(const struct nf_conntrack *ct)
{
	return &ct->counters[__DIR_REPL].packets;
}

static const void *get_attr_repl_counter_bytes(const struct nf_conntrack *ct)
{
	return &ct->counters[__DIR_REPL].bytes;
}

static const void *get_attr_status(const struct nf_conntrack *ct)
{
	return &ct->status;
}

static const void *get_attr_use(const struct nf_conntrack *ct)
{
	return &ct->use;
}

static const void *get_attr_id(const struct nf_conntrack *ct)
{
	return &ct->id;
}

static const void *get_attr_orig_cor_pos(const struct nf_conntrack *ct)
{
	return &ct->natseq[__DIR_ORIG].correction_pos;
}

static const void *get_attr_orig_off_bfr(const struct nf_conntrack *ct)
{
	return &ct->natseq[__DIR_ORIG].offset_before;
}

static const void *get_attr_orig_off_aft(const struct nf_conntrack *ct)
{
	return &ct->natseq[__DIR_ORIG].offset_after;
}

static const void *get_attr_repl_cor_pos(const struct nf_conntrack *ct)
{
	return &ct->natseq[__DIR_REPL].correction_pos;
}

static const void *get_attr_repl_off_bfr(const struct nf_conntrack *ct)
{
	return &ct->natseq[__DIR_REPL].offset_before;
}

static const void *get_attr_repl_off_aft(const struct nf_conntrack *ct)
{
	return &ct->natseq[__DIR_REPL].offset_after;
}

static const void *get_attr_helper_name(const struct nf_conntrack *ct)
{
	return ct->helper_name;
}

static const void *get_attr_dccp_state(const struct nf_conntrack *ct)
{
	return &ct->protoinfo.dccp.state;
}

static const void *get_attr_dccp_role(const struct nf_conntrack *ct)
{
	return &ct->protoinfo.dccp.role;
}

static const void *get_attr_dccp_handshake_seq(const struct nf_conntrack *ct)
{
	return &ct->protoinfo.dccp.handshake_seq;
}

static const void *get_attr_zone(const struct nf_conntrack *ct)
{
	return &ct->zone;
}

static const void *get_attr_secctx(const struct nf_conntrack *ct)
{
	return ct->secctx;
}

static const void *get_attr_timestamp_start(const struct nf_conntrack *ct)
{
	return &ct->timestamp.start;
}

static const void *get_attr_timestamp_stop(const struct nf_conntrack *ct)
{
	return &ct->timestamp.stop;
}

static const void *get_attr_helper_info(const struct nf_conntrack *ct)
{
	return ct->helper_info;
}

static const void *get_attr_connlabels(const struct nf_conntrack *ct)
{
	return ct->connlabels;
}

static const void *get_attr_connlabels_mask(const struct nf_conntrack *ct)
{
	return ct->connlabels_mask;
}

static const void *get_attr_synproxy_isn(const struct nf_conntrack *ct)
{
	return &ct->synproxy.isn;
}

static const void *get_attr_synproxy_its(const struct nf_conntrack *ct)
{
	return &ct->synproxy.its;
}

static const void *get_attr_synproxy_tsoff(const struct nf_conntrack *ct)
{
	return &ct->synproxy.tsoff;
}

//Andrew add
static const void *get_attr_tcp_state_bit(const struct nf_conntrack *ct)
{
	return &ct->protoinfo.tcp.state_bit;
}

static const void *get_attr_sctp_state_bit(const struct nf_conntrack *ct)
{
	return &ct->protoinfo.sctp.state_bit;
}

static const void *get_attr_dccp_state_bit(const struct nf_conntrack *ct)
{
	return &ct->protoinfo.dccp.state_bit;
}
//Andrew end

const get_attr get_attr_array[ATTR_MAX] = {
	[ATTR_ORIG_IPV4_SRC]		= get_attr_orig_ipv4_src,
	[ATTR_ORIG_IPV4_DST] 		= get_attr_orig_ipv4_dst,
	[ATTR_REPL_IPV4_SRC]		= get_attr_repl_ipv4_src,
	[ATTR_REPL_IPV4_DST]		= get_attr_repl_ipv4_dst,
	[ATTR_ORIG_IPV6_SRC]		= get_attr_orig_ipv6_src,
	[ATTR_ORIG_IPV6_DST]		= get_attr_orig_ipv6_dst,
	[ATTR_REPL_IPV6_SRC]		= get_attr_repl_ipv6_src,
	[ATTR_REPL_IPV6_DST]		= get_attr_repl_ipv6_dst,
	[ATTR_ORIG_PORT_SRC]		= get_attr_orig_port_src,
	[ATTR_ORIG_PORT_DST]		= get_attr_orig_port_dst,
	[ATTR_REPL_PORT_SRC]		= get_attr_repl_port_src,
	[ATTR_REPL_PORT_DST]		= get_attr_repl_port_dst,
	[ATTR_ICMP_TYPE]		= get_attr_icmp_type,
	[ATTR_ICMP_CODE]		= get_attr_icmp_code,
	[ATTR_ICMP_ID]			= get_attr_icmp_id,
	[ATTR_ORIG_L3PROTO]		= get_attr_orig_l3proto,
	[ATTR_REPL_L3PROTO]		= get_attr_repl_l3proto,
	[ATTR_ORIG_L4PROTO]		= get_attr_orig_l4proto,
	[ATTR_REPL_L4PROTO]		= get_attr_repl_l4proto,
	[ATTR_TCP_STATE]		= get_attr_tcp_state,
	[ATTR_SNAT_IPV4]		= get_attr_snat_ipv4,
	[ATTR_DNAT_IPV4]		= get_attr_dnat_ipv4,
	[ATTR_SNAT_PORT]		= get_attr_snat_port,
	[ATTR_DNAT_PORT]		= get_attr_dnat_port,
	[ATTR_TIMEOUT]			= get_attr_timeout,
	[ATTR_MARK]			= get_attr_mark,
	[ATTR_ORIG_COUNTER_PACKETS] 	= get_attr_orig_counter_packets,
	[ATTR_ORIG_COUNTER_BYTES]	= get_attr_orig_counter_bytes,
	[ATTR_REPL_COUNTER_PACKETS]	= get_attr_repl_counter_packets,
	[ATTR_REPL_COUNTER_BYTES]	= get_attr_repl_counter_bytes,
	[ATTR_USE]			= get_attr_use,
	[ATTR_ID]			= get_attr_id,
	[ATTR_STATUS]			= get_attr_status,
	[ATTR_TCP_FLAGS_ORIG]		= get_attr_tcp_flags_orig,
	[ATTR_TCP_FLAGS_REPL]		= get_attr_tcp_flags_repl,
	[ATTR_TCP_MASK_ORIG]		= get_attr_tcp_mask_orig,
	[ATTR_TCP_MASK_REPL]		= get_attr_tcp_mask_repl,
	[ATTR_MASTER_IPV4_SRC]		= get_attr_master_ipv4_src,
	[ATTR_MASTER_IPV4_DST] 		= get_attr_master_ipv4_dst,
	[ATTR_MASTER_IPV6_SRC]		= get_attr_master_ipv6_src,
	[ATTR_MASTER_IPV6_DST]		= get_attr_master_ipv6_dst,
	[ATTR_MASTER_PORT_SRC]		= get_attr_master_port_src,
	[ATTR_MASTER_PORT_DST]		= get_attr_master_port_dst,
	[ATTR_MASTER_L3PROTO]		= get_attr_master_l3proto,
	[ATTR_MASTER_L4PROTO]		= get_attr_master_l4proto,
	[ATTR_SECMARK]			= get_attr_secmark,
	[ATTR_ORIG_NAT_SEQ_CORRECTION_POS]	= get_attr_orig_cor_pos,
	[ATTR_ORIG_NAT_SEQ_OFFSET_BEFORE]	= get_attr_orig_off_bfr,
	[ATTR_ORIG_NAT_SEQ_OFFSET_AFTER]	= get_attr_orig_off_aft,
	[ATTR_REPL_NAT_SEQ_CORRECTION_POS]	= get_attr_repl_cor_pos,
	[ATTR_REPL_NAT_SEQ_OFFSET_BEFORE]	= get_attr_repl_off_bfr,
	[ATTR_REPL_NAT_SEQ_OFFSET_AFTER]	= get_attr_repl_off_aft,
	[ATTR_SCTP_STATE]		= get_attr_sctp_state,
	[ATTR_SCTP_VTAG_ORIG]		= get_attr_sctp_vtag_orig,
	[ATTR_SCTP_VTAG_REPL]		= get_attr_sctp_vtag_repl,
	[ATTR_HELPER_NAME]		= get_attr_helper_name,
	[ATTR_DCCP_STATE]		= get_attr_dccp_state,
	[ATTR_DCCP_ROLE]		= get_attr_dccp_role,
	[ATTR_DCCP_HANDSHAKE_SEQ]	= get_attr_dccp_handshake_seq,
	[ATTR_TCP_WSCALE_ORIG]		= get_attr_tcp_wscale_orig,
	[ATTR_TCP_WSCALE_REPL]		= get_attr_tcp_wscale_repl,
	[ATTR_ZONE]			= get_attr_zone,
	[ATTR_ORIG_ZONE]		= get_attr_orig_zone,
	[ATTR_REPL_ZONE]		= get_attr_repl_zone,
	[ATTR_SECCTX]			= get_attr_secctx,
	[ATTR_TIMESTAMP_START]		= get_attr_timestamp_start,
	[ATTR_TIMESTAMP_STOP]		= get_attr_timestamp_stop,
	[ATTR_HELPER_INFO]		= get_attr_helper_info,
	[ATTR_CONNLABELS]		= get_attr_connlabels,
	[ATTR_CONNLABELS_MASK]		= get_attr_connlabels_mask,
	[ATTR_SNAT_IPV6]		= get_attr_snat_ipv6,
	[ATTR_DNAT_IPV6]		= get_attr_dnat_ipv6,
	[ATTR_SYNPROXY_ISN]		= get_attr_synproxy_isn,
	[ATTR_SYNPROXY_ITS]		= get_attr_synproxy_its,
	[ATTR_SYNPROXY_TSOFF]		= get_attr_synproxy_tsoff,
	//Andrew add
	[ATTR_TCP_STATE_BIT]		= get_attr_tcp_state_bit,
	[ATTR_SCTP_STATE_BIT]		= get_attr_sctp_state_bit,
	[ATTR_DCCP_STATE_BIT]		= get_attr_dccp_state_bit,
	//Andrew end
};
