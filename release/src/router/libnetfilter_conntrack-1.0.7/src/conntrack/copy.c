/*
 * (C) 2005-2011 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "internal/internal.h"

static void copy_attr_orig_ipv4_src(struct nf_conntrack *dest,
				    const struct nf_conntrack *orig)
{
	dest->head.orig.src.v4 = orig->head.orig.src.v4;
}

static void copy_attr_orig_ipv4_dst(struct nf_conntrack *dest,
				    const struct nf_conntrack *orig)
{
	dest->head.orig.dst.v4 = orig->head.orig.dst.v4;
}

static void copy_attr_repl_ipv4_src(struct nf_conntrack *dest,
				    const struct nf_conntrack *orig)
{
	dest->repl.src.v4 = orig->repl.src.v4;
}

static void copy_attr_repl_ipv4_dst(struct nf_conntrack *dest,
				    const struct nf_conntrack *orig)
{
	dest->repl.dst.v4 = orig->repl.dst.v4;
}

static void copy_attr_orig_ipv6_src(struct nf_conntrack *dest,
				    const struct nf_conntrack *orig)
{
	memcpy(&dest->head.orig.src,
	       &orig->head.orig.src,
	       sizeof(union __nfct_address));
}

static void copy_attr_orig_ipv6_dst(struct nf_conntrack *dest,
				    const struct nf_conntrack *orig)
{
	memcpy(&dest->head.orig.dst,
	       &orig->head.orig.dst,
	       sizeof(union __nfct_address));
}

static void copy_attr_repl_ipv6_src(struct nf_conntrack *dest,
				    const struct nf_conntrack *orig)
{
	memcpy(&dest->repl.src,
	       &orig->repl.src,
	       sizeof(union __nfct_address));
}

static void copy_attr_repl_ipv6_dst(struct nf_conntrack *dest,
				    const struct nf_conntrack *orig)
{
	memcpy(&dest->repl.dst,
	       &orig->repl.dst,
	       sizeof(union __nfct_address));
}

static void copy_attr_orig_port_src(struct nf_conntrack *dest,
				    const struct nf_conntrack *orig)
{
	dest->head.orig.l4src.all = orig->head.orig.l4src.all;
}

static void copy_attr_orig_port_dst(struct nf_conntrack *dest,
				    const struct nf_conntrack *orig)
{
	dest->head.orig.l4dst.all = orig->head.orig.l4dst.all;
}

static void copy_attr_repl_port_src(struct nf_conntrack *dest,
				    const struct nf_conntrack *orig)
{
	dest->repl.l4src.all = orig->repl.l4src.all;
}

static void copy_attr_repl_port_dst(struct nf_conntrack *dest,
				    const struct nf_conntrack *orig)
{
	dest->repl.l4dst.all = orig->repl.l4dst.all;
}

static void copy_attr_orig_zone(struct nf_conntrack *dest,
				const struct nf_conntrack *orig)
{
	dest->head.orig.zone = orig->head.orig.zone;
}

static void copy_attr_repl_zone(struct nf_conntrack *dest,
				const struct nf_conntrack *orig)
{
	dest->repl.zone = orig->repl.zone;
}

static void copy_attr_icmp_type(struct nf_conntrack *dest,
				const struct nf_conntrack *orig)
{
	dest->head.orig.l4dst.icmp.type = 
		orig->head.orig.l4dst.icmp.type;

}

static void copy_attr_icmp_code(struct nf_conntrack *dest,
				const struct nf_conntrack *orig)
{
	dest->head.orig.l4dst.icmp.code = 
		orig->head.orig.l4dst.icmp.code;

}

static void copy_attr_icmp_id(struct nf_conntrack *dest,
			      const struct nf_conntrack *orig)
{
	dest->head.orig.l4src.icmp.id = 
		orig->head.orig.l4src.icmp.id;
}

static void copy_attr_orig_l3proto(struct nf_conntrack *dest,
				   const struct nf_conntrack *orig)
{
	dest->head.orig.l3protonum = orig->head.orig.l3protonum;
}

static void copy_attr_repl_l3proto(struct nf_conntrack *dest,
				   const struct nf_conntrack *orig)
{
	dest->repl.l3protonum = orig->repl.l3protonum;
}

static void copy_attr_orig_l4proto(struct nf_conntrack *dest,
				   const struct nf_conntrack *orig)
{
	dest->head.orig.protonum = orig->head.orig.protonum;
}

static void copy_attr_repl_l4proto(struct nf_conntrack *dest,
				   const struct nf_conntrack *orig)
{
	dest->repl.protonum = orig->repl.protonum;
}

static void copy_attr_master_ipv4_src(struct nf_conntrack *dest,
				      const struct nf_conntrack *orig)
{
	dest->master.src.v4 = orig->master.src.v4;
}

static void copy_attr_master_ipv4_dst(struct nf_conntrack *dest,
				      const struct nf_conntrack *orig)
{
	dest->master.dst.v4 = orig->master.dst.v4;
}

static void copy_attr_master_ipv6_src(struct nf_conntrack *dest,
				      const struct nf_conntrack *orig)
{
	memcpy(&dest->master.src, &orig->master.src,
	       sizeof(union __nfct_address));
}

static void copy_attr_master_ipv6_dst(struct nf_conntrack *dest,
				      const struct nf_conntrack *orig)
{
	memcpy(&dest->master.dst, &orig->master.dst,
	       sizeof(union __nfct_address));
}

static void copy_attr_master_port_src(struct nf_conntrack *dest,
				      const struct nf_conntrack *orig)
{
	dest->master.l4src.all = orig->master.l4src.all;
}

static void copy_attr_master_port_dst(struct nf_conntrack *dest,
				      const struct nf_conntrack *orig)
{
	dest->master.l4dst.all = orig->master.l4dst.all;
}

static void copy_attr_master_l3proto(struct nf_conntrack *dest,
				     const struct nf_conntrack *orig)
{
	dest->master.l3protonum = orig->master.l3protonum;
}

static void copy_attr_master_l4proto(struct nf_conntrack *dest,
				     const struct nf_conntrack *orig)
{
	dest->master.protonum = orig->master.protonum;
}

static void copy_attr_tcp_state(struct nf_conntrack *dest,
				const struct nf_conntrack *orig)
{
	dest->protoinfo.tcp.state = orig->protoinfo.tcp.state;
}

static void copy_attr_tcp_flags_orig(struct nf_conntrack *dest,
				     const struct nf_conntrack *orig)
{
	dest->protoinfo.tcp.flags[__DIR_ORIG].value =
		orig->protoinfo.tcp.flags[__DIR_ORIG].value;
}

static void copy_attr_tcp_flags_repl(struct nf_conntrack *dest,
				     const struct nf_conntrack *orig)
{
	dest->protoinfo.tcp.flags[__DIR_REPL].value =
		orig->protoinfo.tcp.flags[__DIR_REPL].value;
}

static void copy_attr_tcp_mask_orig(struct nf_conntrack *dest,
				    const struct nf_conntrack *orig)
{
	dest->protoinfo.tcp.flags[__DIR_ORIG].mask =
		orig->protoinfo.tcp.flags[__DIR_ORIG].mask;
}

static void copy_attr_tcp_mask_repl(struct nf_conntrack *dest,
				    const struct nf_conntrack *orig)
{
	dest->protoinfo.tcp.flags[__DIR_REPL].mask =
		orig->protoinfo.tcp.flags[__DIR_REPL].mask;
}

static void copy_attr_tcp_wscale_orig(struct nf_conntrack *dest,
				      const struct nf_conntrack *orig)
{
	dest->protoinfo.tcp.wscale[__DIR_ORIG] =
		orig->protoinfo.tcp.wscale[__DIR_ORIG];
}

static void copy_attr_tcp_wscale_repl(struct nf_conntrack *dest,
				      const struct nf_conntrack *orig)
{
	dest->protoinfo.tcp.wscale[__DIR_REPL] =
		orig->protoinfo.tcp.wscale[__DIR_REPL];
}

static void copy_attr_sctp_state(struct nf_conntrack *dest,
				 const struct nf_conntrack *orig)
{
	dest->protoinfo.sctp.state = orig->protoinfo.sctp.state;
}

static void copy_attr_sctp_vtag_orig(struct nf_conntrack *dest,
				     const struct nf_conntrack *orig)
{
	dest->protoinfo.sctp.vtag[__DIR_ORIG] =
		orig->protoinfo.sctp.vtag[__DIR_ORIG];
}

static void copy_attr_sctp_vtag_repl(struct nf_conntrack *dest,
				     const struct nf_conntrack *orig)
{
	dest->protoinfo.sctp.vtag[__DIR_REPL] =
		orig->protoinfo.sctp.vtag[__DIR_REPL];
}

static void copy_attr_dccp_state(struct nf_conntrack *dest,
				 const struct nf_conntrack *orig)
{
	dest->protoinfo.dccp.state = orig->protoinfo.dccp.state;
}

static void copy_attr_dccp_role(struct nf_conntrack *dest,
				const struct nf_conntrack *orig)
{
	dest->protoinfo.dccp.role = orig->protoinfo.dccp.role;
}

static void copy_attr_dccp_handshake_seq(struct nf_conntrack *dest,
					 const struct nf_conntrack *orig)
{
	dest->protoinfo.dccp.handshake_seq = orig->protoinfo.dccp.handshake_seq;
}

static void copy_attr_snat_ipv4(struct nf_conntrack *dest,
				const struct nf_conntrack *orig)
{
	dest->snat.min_ip.v4 = orig->snat.min_ip.v4;
}

static void copy_attr_dnat_ipv4(struct nf_conntrack *dest,
				const struct nf_conntrack *orig)
{
	dest->dnat.min_ip.v4 = orig->dnat.min_ip.v4;
}

static void copy_attr_snat_ipv6(struct nf_conntrack *dest,
				const struct nf_conntrack *orig)
{
	memcpy(&dest->snat.min_ip.v6, &orig->snat.min_ip.v6,
	       sizeof(struct in6_addr));
}

static void copy_attr_dnat_ipv6(struct nf_conntrack *dest,
				const struct nf_conntrack *orig)
{
	memcpy(&dest->dnat.min_ip.v6, &orig->dnat.min_ip.v6,
	       sizeof(struct in6_addr));
}

static void copy_attr_snat_port(struct nf_conntrack *dest,
				const struct nf_conntrack *orig)
{
	dest->snat.l4min.all = orig->snat.l4min.all;
}

static void copy_attr_dnat_port(struct nf_conntrack *dest,
				const struct nf_conntrack *orig)
{
	dest->dnat.l4min.all = orig->dnat.l4min.all;
}

static void copy_attr_timeout(struct nf_conntrack *dest,
			      const struct nf_conntrack *orig)
{
	dest->timeout = orig->timeout;
}

static void copy_attr_mark(struct nf_conntrack *dest,
			   const struct nf_conntrack *orig)
{
	dest->mark = orig->mark;
}

static void copy_attr_secmark(struct nf_conntrack *dest,
			      const struct nf_conntrack *orig)
{
	dest->secmark = orig->secmark;
}

static void copy_attr_orig_counter_packets(struct nf_conntrack *dest,
					   const struct nf_conntrack *orig)
{
	dest->counters[__DIR_ORIG].packets = orig->counters[__DIR_ORIG].packets;
}

static void copy_attr_repl_counter_packets(struct nf_conntrack *dest,
					   const struct nf_conntrack *orig)
{
	dest->counters[__DIR_REPL].packets = orig->counters[__DIR_REPL].packets;
}

static void copy_attr_orig_counter_bytes(struct nf_conntrack *dest,
					 const struct nf_conntrack *orig)
{
	dest->counters[__DIR_ORIG].bytes = orig->counters[__DIR_ORIG].bytes;
}

static void copy_attr_repl_counter_bytes(struct nf_conntrack *dest,
					 const struct nf_conntrack *orig)
{
	dest->counters[__DIR_REPL].bytes = orig->counters[__DIR_REPL].bytes;
}

static void copy_attr_status(struct nf_conntrack *dest,
			     const struct nf_conntrack *orig)
{
	dest->status = orig->status;
}

static void copy_attr_use(struct nf_conntrack *dest,
			  const struct nf_conntrack *orig)
{
	dest->use = orig->use;
}

static void copy_attr_id(struct nf_conntrack *dest,
			 const struct nf_conntrack *orig)
{
	dest->id = orig->id;
}

static void copy_attr_orig_cor_pos(struct nf_conntrack *dest,
				   const struct nf_conntrack *orig)
{
	dest->natseq[__DIR_ORIG].correction_pos =
		orig->natseq[__DIR_ORIG].correction_pos;
}

static void copy_attr_orig_off_bfr(struct nf_conntrack *dest,
				   const struct nf_conntrack *orig)
{
	dest->natseq[__DIR_ORIG].offset_before =
		orig->natseq[__DIR_ORIG].offset_before;
}

static void copy_attr_orig_off_aft(struct nf_conntrack *dest,
				   const struct nf_conntrack *orig)
{
	dest->natseq[__DIR_ORIG].offset_after =
		orig->natseq[__DIR_ORIG].offset_after;
}

static void copy_attr_repl_cor_pos(struct nf_conntrack *dest,
				   const struct nf_conntrack *orig)
{
	dest->natseq[__DIR_REPL].correction_pos =
		orig->natseq[__DIR_REPL].correction_pos;
}

static void copy_attr_repl_off_bfr(struct nf_conntrack *dest,
				   const struct nf_conntrack *orig)
{
	dest->natseq[__DIR_REPL].offset_before =
		orig->natseq[__DIR_REPL].offset_before;
}

static void copy_attr_repl_off_aft(struct nf_conntrack *dest,
				   const struct nf_conntrack *orig)
{
	dest->natseq[__DIR_REPL].offset_after =
		orig->natseq[__DIR_REPL].offset_after;
}

static void copy_attr_helper_name(struct nf_conntrack *dest,
				  const struct nf_conntrack *orig)
{
	strncpy(dest->helper_name, orig->helper_name, NFCT_HELPER_NAME_MAX);
	dest->helper_name[NFCT_HELPER_NAME_MAX-1] = '\0';
}

static void copy_attr_zone(struct nf_conntrack *dest,
			   const struct nf_conntrack *orig)
{
	dest->zone = orig->zone;
}

static void copy_attr_secctx(struct nf_conntrack *dest,
			     const struct nf_conntrack *orig)
{
	if (dest->secctx) {
		free(dest->secctx);
		dest->secctx = NULL;
	}
	if (orig->secctx)
		dest->secctx = strdup(orig->secctx);
}

static void copy_attr_timestamp_start(struct nf_conntrack *dest,
				      const struct nf_conntrack *orig)
{
	dest->timestamp.start = orig->timestamp.start;
}

static void copy_attr_timestamp_stop(struct nf_conntrack *dest,
				     const struct nf_conntrack *orig)
{
	dest->timestamp.stop = orig->timestamp.stop;
}

static void copy_attr_help_info(struct nf_conntrack *dest,
				const struct nf_conntrack *orig)
{
	if (orig->helper_info == NULL)
		return;

	if (dest->helper_info != NULL)
		free(dest->helper_info);

	dest->helper_info = calloc(1, orig->helper_info_len);
	if (dest->helper_info == NULL)
		return;

	memcpy(dest->helper_info, orig->helper_info, orig->helper_info_len);
}

static void* do_copy_attr_connlabels(struct nfct_bitmask *dest,
				     const struct nfct_bitmask *orig)
{
	if (orig == NULL)
		return dest;
	if (dest)
		nfct_bitmask_destroy(dest);
	return nfct_bitmask_clone(orig);
}

static void copy_attr_connlabels(struct nf_conntrack *dest,
				 const struct nf_conntrack *orig)
{
	dest->connlabels = do_copy_attr_connlabels(dest->connlabels, orig->connlabels);
}

static void copy_attr_connlabels_mask(struct nf_conntrack *dest,
				 const struct nf_conntrack *orig)
{
	dest->connlabels_mask = do_copy_attr_connlabels(dest->connlabels_mask, orig->connlabels_mask);
}

static void copy_attr_synproxy_its(struct nf_conntrack *dest,
				   const struct nf_conntrack *orig)
{
	dest->synproxy.its = orig->synproxy.its;
}

static void copy_attr_synproxy_isn(struct nf_conntrack *dest,
				   const struct nf_conntrack *orig)
{
	dest->synproxy.isn = orig->synproxy.isn;
}

static void copy_attr_synproxy_tsoff(struct nf_conntrack *dest,
				     const struct nf_conntrack *orig)
{
	dest->synproxy.tsoff = orig->synproxy.tsoff;
}

const copy_attr copy_attr_array[ATTR_MAX] = {
	[ATTR_ORIG_IPV4_SRC]		= copy_attr_orig_ipv4_src,
	[ATTR_ORIG_IPV4_DST] 		= copy_attr_orig_ipv4_dst,
	[ATTR_REPL_IPV4_SRC]		= copy_attr_repl_ipv4_src,
	[ATTR_REPL_IPV4_DST]		= copy_attr_repl_ipv4_dst,
	[ATTR_ORIG_IPV6_SRC]		= copy_attr_orig_ipv6_src,
	[ATTR_ORIG_IPV6_DST]		= copy_attr_orig_ipv6_dst,
	[ATTR_REPL_IPV6_SRC]		= copy_attr_repl_ipv6_src,
	[ATTR_REPL_IPV6_DST]		= copy_attr_repl_ipv6_dst,
	[ATTR_ORIG_PORT_SRC]		= copy_attr_orig_port_src,
	[ATTR_ORIG_PORT_DST]		= copy_attr_orig_port_dst,
	[ATTR_REPL_PORT_SRC]		= copy_attr_repl_port_src,
	[ATTR_REPL_PORT_DST]		= copy_attr_repl_port_dst,
	[ATTR_ICMP_TYPE]		= copy_attr_icmp_type,
	[ATTR_ICMP_CODE]		= copy_attr_icmp_code,
	[ATTR_ICMP_ID]			= copy_attr_icmp_id,
	[ATTR_ORIG_L3PROTO]		= copy_attr_orig_l3proto,
	[ATTR_REPL_L3PROTO]		= copy_attr_repl_l3proto,
	[ATTR_ORIG_L4PROTO]		= copy_attr_orig_l4proto,
	[ATTR_REPL_L4PROTO]		= copy_attr_repl_l4proto,
	[ATTR_TCP_STATE]		= copy_attr_tcp_state,
	[ATTR_SNAT_IPV4]		= copy_attr_snat_ipv4,
	[ATTR_DNAT_IPV4]		= copy_attr_dnat_ipv4,
	[ATTR_SNAT_PORT]		= copy_attr_snat_port,
	[ATTR_DNAT_PORT]		= copy_attr_dnat_port,
	[ATTR_TIMEOUT]			= copy_attr_timeout,
	[ATTR_MARK]			= copy_attr_mark,
	[ATTR_ORIG_COUNTER_PACKETS] 	= copy_attr_orig_counter_packets,
	[ATTR_ORIG_COUNTER_BYTES]	= copy_attr_orig_counter_bytes,
	[ATTR_REPL_COUNTER_PACKETS]	= copy_attr_repl_counter_packets,
	[ATTR_REPL_COUNTER_BYTES]	= copy_attr_repl_counter_bytes,
	[ATTR_USE]			= copy_attr_use,
	[ATTR_ID]			= copy_attr_id,
	[ATTR_STATUS]			= copy_attr_status,
	[ATTR_TCP_FLAGS_ORIG]		= copy_attr_tcp_flags_orig,
	[ATTR_TCP_FLAGS_REPL]		= copy_attr_tcp_flags_repl,
	[ATTR_TCP_MASK_ORIG]		= copy_attr_tcp_mask_orig,
	[ATTR_TCP_MASK_REPL]		= copy_attr_tcp_mask_repl,
	[ATTR_MASTER_IPV4_SRC]		= copy_attr_master_ipv4_src,
	[ATTR_MASTER_IPV4_DST] 		= copy_attr_master_ipv4_dst,
	[ATTR_MASTER_IPV6_SRC]		= copy_attr_master_ipv6_src,
	[ATTR_MASTER_IPV6_DST]		= copy_attr_master_ipv6_dst,
	[ATTR_MASTER_PORT_SRC]		= copy_attr_master_port_src,
	[ATTR_MASTER_PORT_DST]		= copy_attr_master_port_dst,
	[ATTR_MASTER_L3PROTO]		= copy_attr_master_l3proto,
	[ATTR_MASTER_L4PROTO]		= copy_attr_master_l4proto,
	[ATTR_SECMARK]			= copy_attr_secmark,
	[ATTR_ORIG_NAT_SEQ_CORRECTION_POS]	= copy_attr_orig_cor_pos,
	[ATTR_ORIG_NAT_SEQ_OFFSET_BEFORE]	= copy_attr_orig_off_bfr,
	[ATTR_ORIG_NAT_SEQ_OFFSET_AFTER]	= copy_attr_orig_off_aft,
	[ATTR_REPL_NAT_SEQ_CORRECTION_POS]	= copy_attr_repl_cor_pos,
	[ATTR_REPL_NAT_SEQ_OFFSET_BEFORE]	= copy_attr_repl_off_bfr,
	[ATTR_REPL_NAT_SEQ_OFFSET_AFTER]	= copy_attr_repl_off_aft,
	[ATTR_SCTP_STATE]		= copy_attr_sctp_state,
	[ATTR_SCTP_VTAG_ORIG]		= copy_attr_sctp_vtag_orig,
	[ATTR_SCTP_VTAG_REPL]		= copy_attr_sctp_vtag_repl,
	[ATTR_HELPER_NAME]		= copy_attr_helper_name,
	[ATTR_DCCP_STATE]		= copy_attr_dccp_state,
	[ATTR_DCCP_ROLE]		= copy_attr_dccp_role,
	[ATTR_DCCP_HANDSHAKE_SEQ]	= copy_attr_dccp_handshake_seq,
	[ATTR_TCP_WSCALE_ORIG]		= copy_attr_tcp_wscale_orig,
	[ATTR_TCP_WSCALE_REPL]		= copy_attr_tcp_wscale_repl,
	[ATTR_ZONE]			= copy_attr_zone,
	[ATTR_ORIG_ZONE]		= copy_attr_orig_zone,
	[ATTR_REPL_ZONE]		= copy_attr_repl_zone,
	[ATTR_SECCTX]			= copy_attr_secctx,
	[ATTR_TIMESTAMP_START]		= copy_attr_timestamp_start,
	[ATTR_TIMESTAMP_STOP]		= copy_attr_timestamp_stop,
	[ATTR_HELPER_INFO]		= copy_attr_help_info,
	[ATTR_CONNLABELS]		= copy_attr_connlabels,
	[ATTR_CONNLABELS_MASK]		= copy_attr_connlabels_mask,
	[ATTR_SNAT_IPV6]		= copy_attr_snat_ipv6,
	[ATTR_DNAT_IPV6]		= copy_attr_dnat_ipv6,
	[ATTR_SYNPROXY_ITS]		= copy_attr_synproxy_its,
	[ATTR_SYNPROXY_ISN]		= copy_attr_synproxy_isn,
	[ATTR_SYNPROXY_TSOFF]		= copy_attr_synproxy_tsoff,
};

/* this is used by nfct_copy() with the NFCT_CP_OVERRIDE flag set. */
void __copy_fast(struct nf_conntrack *ct1, const struct nf_conntrack *ct2)
{
	memcpy(ct1, ct2, sizeof(*ct1));
	/* malloc'd attributes: don't free, do copy */
	ct1->secctx = NULL;
	ct1->helper_info = NULL;
	ct1->connlabels = NULL;
	ct1->connlabels_mask = NULL;

	copy_attr_secctx(ct1, ct2);
	copy_attr_help_info(ct1, ct2);
	copy_attr_connlabels(ct1, ct2);
	copy_attr_connlabels_mask(ct1, ct2);
}
