/*
 * (C) 2005-2012 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This code has been sponsored by Vyatta Inc. <http://www.vyatta.com>
 */

#include "internal/internal.h"
#include <libmnl/libmnl.h>
#include <endian.h>

static int
nfct_parse_ip_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	/* skip unsupported attribute in user-space */
	if (mnl_attr_type_valid(attr, CTA_IP_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case CTA_IP_V4_SRC:
	case CTA_IP_V4_DST:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			return MNL_CB_ERROR;
		break;
	case CTA_IP_V6_SRC:
	case CTA_IP_V6_DST:
		if (mnl_attr_validate2(attr, MNL_TYPE_UNSPEC,
				       sizeof(struct in6_addr)) < 0) {
			return MNL_CB_ERROR;
		}
		break;
	}
	tb[type] = attr;
	return MNL_CB_OK;
}

static int
nfct_parse_ip(const struct nlattr *attr, struct __nfct_tuple *tuple,
	     const int dir, u_int32_t *set)
{
	struct nlattr *tb[CTA_IP_MAX+1] = {};

	if (mnl_attr_parse_nested(attr, nfct_parse_ip_attr_cb, tb) < 0)
		return -1;

	if (tb[CTA_IP_V4_SRC]) {
		tuple->src.v4 = mnl_attr_get_u32(tb[CTA_IP_V4_SRC]);
		switch(dir) {
		case __DIR_ORIG:
			set_bit(ATTR_ORIG_IPV4_SRC, set);
			break;
		case __DIR_REPL:
			set_bit(ATTR_REPL_IPV4_SRC, set);
			break;
		case __DIR_MASTER:
			set_bit(ATTR_MASTER_IPV4_SRC, set);
			break;
		}
	}

	if (tb[CTA_IP_V4_DST]) {
		tuple->dst.v4 = mnl_attr_get_u32(tb[CTA_IP_V4_DST]);
		switch(dir) {
		case __DIR_ORIG:
			set_bit(ATTR_ORIG_IPV4_DST, set);
			break;
		case __DIR_REPL:
			set_bit(ATTR_REPL_IPV4_DST, set);
			break;
		case __DIR_MASTER:
			set_bit(ATTR_MASTER_IPV4_DST, set);
			break;
		}
	}

	if (tb[CTA_IP_V6_SRC]) {
		memcpy(&tuple->src.v6, mnl_attr_get_payload(tb[CTA_IP_V6_SRC]),
		       sizeof(struct in6_addr));
		switch(dir) {
		case __DIR_ORIG:
			set_bit(ATTR_ORIG_IPV6_SRC, set);
			break;
		case __DIR_REPL:
			set_bit(ATTR_REPL_IPV6_SRC, set);
			break;
		case __DIR_MASTER:
			set_bit(ATTR_MASTER_IPV6_SRC, set);
			break;
		}
	}

	if (tb[CTA_IP_V6_DST]) {
		memcpy(&tuple->dst.v6, mnl_attr_get_payload(tb[CTA_IP_V6_DST]),
		       sizeof(struct in6_addr));
		switch(dir) {
		case __DIR_ORIG:
			set_bit(ATTR_ORIG_IPV6_DST, set);
			break;
		case __DIR_REPL:
			set_bit(ATTR_REPL_IPV6_DST, set);
			break;
		case __DIR_MASTER:
			set_bit(ATTR_MASTER_IPV6_DST, set);
			break;
		}
	}
	return 0;
}

static int
nfct_parse_proto_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, CTA_PROTO_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case CTA_PROTO_SRC_PORT:
	case CTA_PROTO_DST_PORT:
	case CTA_PROTO_ICMP_ID:
	case CTA_PROTO_ICMPV6_ID:
		if (mnl_attr_validate(attr, MNL_TYPE_U16) < 0)
			return MNL_CB_ERROR;
		break;
	case CTA_PROTO_NUM:
	case CTA_PROTO_ICMP_TYPE:
	case CTA_PROTO_ICMP_CODE:
	case CTA_PROTO_ICMPV6_TYPE:
	case CTA_PROTO_ICMPV6_CODE:
		if (mnl_attr_validate(attr, MNL_TYPE_U8) < 0)
			return MNL_CB_ERROR;
		break;
	}
	tb[type] = attr;
	return MNL_CB_OK;
}

static int
nfct_parse_proto(const struct nlattr *attr, struct __nfct_tuple *tuple,
		const int dir, u_int32_t *set)
{
	struct nlattr *tb[CTA_PROTO_MAX+1] = {};

	if (mnl_attr_parse_nested(attr, nfct_parse_proto_attr_cb, tb) < 0)
		return -1;

	if (tb[CTA_PROTO_NUM]) {
		tuple->protonum = mnl_attr_get_u8(tb[CTA_PROTO_NUM]);
		switch(dir) {
		case __DIR_ORIG:
			set_bit(ATTR_ORIG_L4PROTO, set);
			break;
		case __DIR_REPL:
			set_bit(ATTR_REPL_L4PROTO, set);
			break;
		case __DIR_MASTER:
			set_bit(ATTR_MASTER_L4PROTO, set);
			break;
		}
	}

	if (tb[CTA_PROTO_SRC_PORT]) {
		tuple->l4src.tcp.port =
			mnl_attr_get_u16(tb[CTA_PROTO_SRC_PORT]);
		switch(dir) {
		case __DIR_ORIG:
			set_bit(ATTR_ORIG_PORT_SRC, set);
			break;
		case __DIR_REPL:
			set_bit(ATTR_REPL_PORT_SRC, set);
			break;
		case __DIR_MASTER:
			set_bit(ATTR_MASTER_PORT_SRC, set);
			break;
		}
	}

	if (tb[CTA_PROTO_DST_PORT]) {
		tuple->l4dst.tcp.port =
			mnl_attr_get_u16(tb[CTA_PROTO_DST_PORT]);
		switch(dir) {
		case __DIR_ORIG:
			set_bit(ATTR_ORIG_PORT_DST, set);
			break;
		case __DIR_REPL:
			set_bit(ATTR_REPL_PORT_DST, set);
			break;
		case __DIR_MASTER:
			set_bit(ATTR_MASTER_PORT_DST, set);
			break;
		}
	}

	if (tb[CTA_PROTO_ICMP_TYPE]) {
		tuple->l4dst.icmp.type =
			mnl_attr_get_u8(tb[CTA_PROTO_ICMP_TYPE]);
		set_bit(ATTR_ICMP_TYPE, set);
	}

	if (tb[CTA_PROTO_ICMP_CODE]) {
		tuple->l4dst.icmp.code =
			mnl_attr_get_u8(tb[CTA_PROTO_ICMP_CODE]);
		set_bit(ATTR_ICMP_CODE, set);
	}

	if (tb[CTA_PROTO_ICMP_ID]) {
		tuple->l4src.icmp.id =
			mnl_attr_get_u16(tb[CTA_PROTO_ICMP_ID]);
		set_bit(ATTR_ICMP_ID, set);
	}

	if (tb[CTA_PROTO_ICMPV6_TYPE]) {
		tuple->l4dst.icmp.type =
			mnl_attr_get_u8(tb[CTA_PROTO_ICMPV6_TYPE]);
		set_bit(ATTR_ICMP_TYPE, set);
	}

	if (tb[CTA_PROTO_ICMPV6_CODE]) {
		tuple->l4dst.icmp.code =
			mnl_attr_get_u8(tb[CTA_PROTO_ICMPV6_CODE]);
		set_bit(ATTR_ICMP_CODE, set);
	}

	if (tb[CTA_PROTO_ICMPV6_ID]) {
		tuple->l4src.icmp.id =
			mnl_attr_get_u16(tb[CTA_PROTO_ICMPV6_ID]);
		set_bit(ATTR_ICMP_ID, set);
	}

	return 0;
}

static int nfct_parse_tuple_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, CTA_TUPLE_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case CTA_TUPLE_IP:
	case CTA_TUPLE_PROTO:
		if (mnl_attr_validate(attr, MNL_TYPE_NESTED) < 0)
			return MNL_CB_ERROR;
		break;
	}
	tb[type] = attr;
	return MNL_CB_OK;
}

int
nfct_parse_tuple(const struct nlattr *attr, struct __nfct_tuple *tuple,
		int dir, u_int32_t *set)
{
	struct nlattr *tb[CTA_TUPLE_MAX+1] = {};

	if (mnl_attr_parse_nested(attr, nfct_parse_tuple_attr_cb, tb) < 0)
		return -1;

	if (tb[CTA_TUPLE_IP]) {
		if (nfct_parse_ip(tb[CTA_TUPLE_IP], tuple, dir, set) < 0)
			return -1;
	}

	if (tb[CTA_TUPLE_PROTO]) {
		if (nfct_parse_proto(tb[CTA_TUPLE_PROTO], tuple, dir, set) < 0)
			return -1;
	}

	return 0;
}

static int
nfct_parse_pinfo_tcp_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, CTA_PROTOINFO_TCP_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case CTA_PROTOINFO_TCP_STATE:
	case CTA_PROTOINFO_TCP_WSCALE_ORIGINAL:
	case CTA_PROTOINFO_TCP_WSCALE_REPLY:
		if (mnl_attr_validate(attr, MNL_TYPE_U8) < 0)
			return MNL_CB_ERROR;
		break;
	case CTA_PROTOINFO_TCP_FLAGS_ORIGINAL:
	case CTA_PROTOINFO_TCP_FLAGS_REPLY:
		if (mnl_attr_validate2(attr, MNL_TYPE_UNSPEC,
					sizeof(struct nf_ct_tcp_flags)) < 0) {
			return MNL_CB_ERROR;
		}
		break;
	}
	tb[type] = attr;
	return MNL_CB_OK;
}

static int
nfct_parse_protoinfo_tcp(const struct nlattr *attr, struct nf_conntrack *ct)
{
	struct nlattr *tb[CTA_PROTOINFO_TCP_MAX+1] = {};

	if (mnl_attr_parse_nested(attr, nfct_parse_pinfo_tcp_attr_cb, tb) < 0)
		return -1;

	if (tb[CTA_PROTOINFO_TCP_STATE]) {
		ct->protoinfo.tcp.state =
			mnl_attr_get_u8(tb[CTA_PROTOINFO_TCP_STATE]);
		set_bit(ATTR_TCP_STATE, ct->head.set);
	}

	if (tb[CTA_PROTOINFO_TCP_WSCALE_ORIGINAL]) {
		memcpy(&ct->protoinfo.tcp.wscale[__DIR_ORIG],
			mnl_attr_get_payload(tb[CTA_PROTOINFO_TCP_WSCALE_ORIGINAL]),
			sizeof(uint8_t));
		set_bit(ATTR_TCP_WSCALE_ORIG, ct->head.set);
	}

	if (tb[CTA_PROTOINFO_TCP_WSCALE_REPLY]) {
		memcpy(&ct->protoinfo.tcp.wscale[__DIR_REPL],
			mnl_attr_get_payload(tb[CTA_PROTOINFO_TCP_WSCALE_REPLY]),
			sizeof(uint8_t));
		set_bit(ATTR_TCP_WSCALE_REPL, ct->head.set);
	}

	if (tb[CTA_PROTOINFO_TCP_FLAGS_ORIGINAL]) {
		memcpy(&ct->protoinfo.tcp.flags[0],
			mnl_attr_get_payload(tb[CTA_PROTOINFO_TCP_FLAGS_ORIGINAL]),
			sizeof(struct nf_ct_tcp_flags));
		set_bit(ATTR_TCP_FLAGS_ORIG, ct->head.set);
		set_bit(ATTR_TCP_MASK_ORIG, ct->head.set);
	}

	if (tb[CTA_PROTOINFO_TCP_FLAGS_REPLY]) {
		memcpy(&ct->protoinfo.tcp.flags[1],
			mnl_attr_get_payload(tb[CTA_PROTOINFO_TCP_FLAGS_REPLY]),
			sizeof(struct nf_ct_tcp_flags));
		set_bit(ATTR_TCP_FLAGS_REPL, ct->head.set);
		set_bit(ATTR_TCP_MASK_REPL, ct->head.set);
	}

	return 0;
}

static int
nfct_parse_pinfo_sctp_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, CTA_PROTOINFO_SCTP_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case CTA_PROTOINFO_SCTP_STATE:
		if (mnl_attr_validate(attr, MNL_TYPE_U8) < 0)
			return MNL_CB_ERROR;
		break;
	case CTA_PROTOINFO_SCTP_VTAG_ORIGINAL:
	case CTA_PROTOINFO_SCTP_VTAG_REPLY:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			return MNL_CB_ERROR;
		break;
	}
	tb[type] = attr;
	return MNL_CB_OK;
}

static int
nfct_parse_protoinfo_sctp(const struct nlattr *attr, struct nf_conntrack *ct)
{
	struct nlattr *tb[CTA_PROTOINFO_SCTP_MAX+1] = {};

	if (mnl_attr_parse_nested(attr, nfct_parse_pinfo_sctp_attr_cb, tb) < 0)
		return -1;

	if (tb[CTA_PROTOINFO_SCTP_STATE]) {
		ct->protoinfo.sctp.state =
			mnl_attr_get_u8(tb[CTA_PROTOINFO_SCTP_STATE]);
		set_bit(ATTR_SCTP_STATE, ct->head.set);
	}

	if (tb[CTA_PROTOINFO_SCTP_VTAG_ORIGINAL]) {
		ct->protoinfo.sctp.vtag[__DIR_ORIG] =
		ntohl(mnl_attr_get_u32(tb[CTA_PROTOINFO_SCTP_VTAG_ORIGINAL]));
		set_bit(ATTR_SCTP_VTAG_ORIG, ct->head.set);
	}

	if (tb[CTA_PROTOINFO_SCTP_VTAG_REPLY]) {
		ct->protoinfo.sctp.vtag[__DIR_REPL] =
		ntohl(mnl_attr_get_u32(tb[CTA_PROTOINFO_SCTP_VTAG_REPLY]));
		set_bit(ATTR_SCTP_VTAG_REPL, ct->head.set);
	}

	return 0;
}

static int
nfct_parse_pinfo_dccp_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, CTA_PROTOINFO_DCCP_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case CTA_PROTOINFO_DCCP_STATE:
	case CTA_PROTOINFO_DCCP_ROLE:
		if (mnl_attr_validate(attr, MNL_TYPE_U8) < 0)
			return MNL_CB_ERROR;
		break;
	case CTA_PROTOINFO_DCCP_HANDSHAKE_SEQ:
		if (mnl_attr_validate(attr, MNL_TYPE_U64) < 0)
			return MNL_CB_ERROR;
		break;
	}
	tb[type] = attr;
	return MNL_CB_OK;
}

static int
nfct_parse_protoinfo_dccp(const struct nlattr *attr, struct nf_conntrack *ct)
{
	struct nlattr *tb[CTA_PROTOINFO_DCCP_MAX+1] = {};

	if (mnl_attr_parse_nested(attr, nfct_parse_pinfo_dccp_attr_cb, tb) < 0)
		return -1;

	if (tb[CTA_PROTOINFO_DCCP_STATE]) {
		ct->protoinfo.dccp.state = mnl_attr_get_u8(tb[CTA_PROTOINFO_DCCP_STATE]);
		set_bit(ATTR_DCCP_STATE, ct->head.set);
	}
	if (tb[CTA_PROTOINFO_DCCP_ROLE]) {
		ct->protoinfo.dccp.role = mnl_attr_get_u8(tb[CTA_PROTOINFO_DCCP_ROLE]);
		set_bit(ATTR_DCCP_ROLE, ct->head.set);
	}
	if (tb[CTA_PROTOINFO_DCCP_HANDSHAKE_SEQ]) {
		ct->protoinfo.dccp.handshake_seq = be64toh(
		mnl_attr_get_u64(tb[CTA_PROTOINFO_DCCP_HANDSHAKE_SEQ]));
		set_bit(ATTR_DCCP_HANDSHAKE_SEQ, ct->head.set);
	}

	return 0;
}

static int
nfct_parse_protoinfo_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, CTA_PROTOINFO_TCP_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case CTA_PROTOINFO_TCP:
	case CTA_PROTOINFO_SCTP:
	case CTA_PROTOINFO_DCCP:
		if (mnl_attr_validate(attr, MNL_TYPE_NESTED) < 0)
			return MNL_CB_ERROR;
		break;
	}
	tb[type] = attr;
	return MNL_CB_OK;
}

static int
nfct_parse_protoinfo(const struct nlattr *attr, struct nf_conntrack *ct)
{
	struct nlattr *tb[CTA_PROTOINFO_MAX+1] = {};

	if (mnl_attr_parse_nested(attr, nfct_parse_protoinfo_attr_cb, tb) < 0)
		return -1;

	if (tb[CTA_PROTOINFO_TCP])
		nfct_parse_protoinfo_tcp(tb[CTA_PROTOINFO_TCP], ct);

	if (tb[CTA_PROTOINFO_SCTP])
		nfct_parse_protoinfo_sctp(tb[CTA_PROTOINFO_SCTP], ct);

	if (tb[CTA_PROTOINFO_DCCP])
		nfct_parse_protoinfo_dccp(tb[CTA_PROTOINFO_DCCP], ct);

	return 0;
}

static int nfct_parse_counters_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, CTA_COUNTERS_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case CTA_COUNTERS_PACKETS:
	case CTA_COUNTERS_BYTES:
		if (mnl_attr_validate(attr, MNL_TYPE_U64) < 0)
			return MNL_CB_ERROR;
		break;
	case CTA_COUNTERS32_PACKETS:
	case CTA_COUNTERS32_BYTES:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			return MNL_CB_ERROR;
		break;
	}
	tb[type] = attr;
	return MNL_CB_OK;
}

static int
nfct_parse_counters(const struct nlattr *attr, struct nf_conntrack *ct,
		   int dir)
{
	struct nlattr *tb[CTA_COUNTERS_MAX+1] = {};

	if (mnl_attr_parse_nested(attr, nfct_parse_counters_attr_cb, tb) < 0)
		return -1;

	if (tb[CTA_COUNTERS_PACKETS] || tb[CTA_COUNTERS32_PACKETS]) {
		if (tb[CTA_COUNTERS32_PACKETS]) {
			ct->counters[dir].packets =
			ntohl(mnl_attr_get_u32(tb[CTA_COUNTERS32_PACKETS]));
		}
		if (tb[CTA_COUNTERS_PACKETS]) {
			ct->counters[dir].packets =
			be64toh(mnl_attr_get_u64(tb[CTA_COUNTERS_PACKETS]));
		}
		switch(dir) {
		case __DIR_ORIG:
			set_bit(ATTR_ORIG_COUNTER_PACKETS, ct->head.set);
			break;
		case __DIR_REPL:
			set_bit(ATTR_REPL_COUNTER_PACKETS, ct->head.set);
			break;
		}
	}
	if (tb[CTA_COUNTERS_BYTES] || tb[CTA_COUNTERS32_BYTES]) {
		if (tb[CTA_COUNTERS32_BYTES]) {
			ct->counters[dir].bytes =
			ntohl(mnl_attr_get_u32(tb[CTA_COUNTERS32_BYTES]));
		}
		if (tb[CTA_COUNTERS_BYTES]) {
			ct->counters[dir].bytes =
			be64toh(mnl_attr_get_u64(tb[CTA_COUNTERS_BYTES]));
		}

		switch(dir) {
		case __DIR_ORIG:
			set_bit(ATTR_ORIG_COUNTER_BYTES, ct->head.set);
			break;
		case __DIR_REPL:
			set_bit(ATTR_REPL_COUNTER_BYTES, ct->head.set);
			break;
		}
	}

	return 0;
}

static int
nfct_parse_nat_seq_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, CTA_NAT_SEQ_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case CTA_NAT_SEQ_CORRECTION_POS:
	case CTA_NAT_SEQ_OFFSET_BEFORE:
	case CTA_NAT_SEQ_OFFSET_AFTER:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			return MNL_CB_ERROR;
		break;
	}
	tb[type] = attr;
	return MNL_CB_OK;
}

static int
nfct_parse_nat_seq(const struct nlattr *attr, struct nf_conntrack *ct, int dir)
{
	struct nlattr *tb[CTA_NAT_SEQ_MAX+1] = {};

	if (mnl_attr_parse_nested(attr, nfct_parse_nat_seq_attr_cb, tb) < 0)
		return -1;

	if (tb[CTA_NAT_SEQ_CORRECTION_POS]) {
		ct->natseq[dir].correction_pos =
			ntohl(mnl_attr_get_u32(tb[CTA_NAT_SEQ_CORRECTION_POS]));
		switch(dir) {
		case __DIR_ORIG:
			set_bit(ATTR_ORIG_NAT_SEQ_CORRECTION_POS, ct->head.set);
			break;
		case __DIR_REPL:
			set_bit(ATTR_REPL_NAT_SEQ_CORRECTION_POS, ct->head.set);
			break;
		}
	}

	if (tb[CTA_NAT_SEQ_OFFSET_BEFORE]) {
		ct->natseq[dir].offset_before =
			ntohl(mnl_attr_get_u32(tb[CTA_NAT_SEQ_OFFSET_BEFORE]));
		switch(dir) {
		case __DIR_ORIG:
			set_bit(ATTR_ORIG_NAT_SEQ_OFFSET_BEFORE, ct->head.set);
			break;
		case __DIR_REPL:
			set_bit(ATTR_REPL_NAT_SEQ_OFFSET_BEFORE, ct->head.set);
			break;
		}
	}

	if (tb[CTA_NAT_SEQ_OFFSET_AFTER]) {
		ct->natseq[dir].offset_after =
			ntohl(mnl_attr_get_u32(tb[CTA_NAT_SEQ_OFFSET_AFTER]));
		switch(dir) {
		case __DIR_ORIG:
			set_bit(ATTR_ORIG_NAT_SEQ_OFFSET_AFTER, ct->head.set);
			break;
		case __DIR_REPL:
			set_bit(ATTR_REPL_NAT_SEQ_OFFSET_AFTER, ct->head.set);
			break;
		}
	}

	return 0;
}

static int
nfct_parse_helper_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, CTA_HELP_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case CTA_HELP_NAME:
		if (mnl_attr_validate(attr, MNL_TYPE_STRING) < 0)
			return MNL_CB_ERROR;
		break;
	}
	tb[type] = attr;
	return MNL_CB_OK;
}

static int
nfct_parse_helper(const struct nlattr *attr, struct nf_conntrack *ct)
{
	struct nlattr *tb[CTA_HELP_MAX+1] = {};

	if (mnl_attr_parse_nested(attr, nfct_parse_helper_attr_cb, tb) < 0)
		return -1;

	if (!tb[CTA_HELP_NAME])
		return 0;

	strncpy(ct->helper_name, mnl_attr_get_str(tb[CTA_HELP_NAME]),
		NFCT_HELPER_NAME_MAX);
	ct->helper_name[NFCT_HELPER_NAME_MAX-1] = '\0';
	set_bit(ATTR_HELPER_NAME, ct->head.set);

	if (!tb[CTA_HELP_INFO])
		return 0;

	ct->helper_info_len = mnl_attr_get_payload_len(tb[CTA_HELP_INFO]);
	ct->helper_info = calloc(1, ct->helper_info_len);
	if (ct->helper_info == NULL)
		return -1;

	memcpy(ct->helper_info, mnl_attr_get_payload(tb[CTA_HELP_INFO]),
		ct->helper_info_len);
	set_bit(ATTR_HELPER_INFO, ct->head.set);

	return 0;
}

static int
nfct_parse_secctx_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, CTA_SECCTX_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case CTA_SECCTX_NAME:
		if (mnl_attr_validate(attr, MNL_TYPE_STRING) < 0)
			return MNL_CB_ERROR;
		break;
	}
	tb[type] = attr;
	return MNL_CB_OK;
}

static int
nfct_parse_secctx(const struct nlattr *attr, struct nf_conntrack *ct)
{
	struct nlattr *tb[CTA_SECCTX_MAX+1] = {};

	if (mnl_attr_parse_nested(attr, nfct_parse_secctx_attr_cb, tb) < 0)
		return -1;

	if (!tb[CTA_SECCTX_NAME])
		return 0;

	ct->secctx = strdup(NFA_DATA(tb[CTA_SECCTX_NAME]));
	if (ct->secctx)
		set_bit(ATTR_SECCTX, ct->head.set);

	return 0;
}

static int
nfct_parse_timestamp_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, CTA_TIMESTAMP_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case CTA_TIMESTAMP_START:
	case CTA_TIMESTAMP_STOP:
		if (mnl_attr_validate(attr, MNL_TYPE_U64) < 0)
			return MNL_CB_ERROR;
		break;
	}
	tb[type] = attr;
	return MNL_CB_OK;
}

static int
nfct_parse_timestamp(const struct nlattr *attr, struct nf_conntrack *ct)
{
	struct nlattr *tb[CTA_TIMESTAMP_MAX+1] = {};

	if (mnl_attr_parse_nested(attr, nfct_parse_timestamp_attr_cb, tb) < 0)
		return -1;

	if (tb[CTA_TIMESTAMP_START]) {
		ct->timestamp.start =
			be64toh(mnl_attr_get_u64(tb[CTA_TIMESTAMP_START]));
		set_bit(ATTR_TIMESTAMP_START, ct->head.set);
	}
	if (tb[CTA_TIMESTAMP_STOP]) {
		ct->timestamp.stop =
			be64toh(mnl_attr_get_u64(tb[CTA_TIMESTAMP_START]));
		set_bit(ATTR_TIMESTAMP_STOP, ct->head.set);
	}

	return 0;
}

static int
nfct_parse_conntrack_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, CTA_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case CTA_TUPLE_ORIG:
	case CTA_TUPLE_REPLY:
	case CTA_TUPLE_MASTER:
	case CTA_NAT_SEQ_ADJ_ORIG:
	case CTA_NAT_SEQ_ADJ_REPLY:
	case CTA_PROTOINFO:
	case CTA_COUNTERS_ORIG:
	case CTA_COUNTERS_REPLY:
	case CTA_HELP:
	case CTA_SECCTX:
	case CTA_TIMESTAMP:
		if (mnl_attr_validate(attr, MNL_TYPE_NESTED) < 0)
			return MNL_CB_ERROR;
		break;
	case CTA_STATUS:
	case CTA_TIMEOUT:
	case CTA_MARK:
	case CTA_SECMARK:
	case CTA_USE:
	case CTA_ID:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			return MNL_CB_ERROR;
		break;
	case CTA_ZONE:
		if (mnl_attr_validate(attr, MNL_TYPE_U16) < 0)
			return MNL_CB_ERROR;
		break;
	case CTA_NAT_SRC:
	case CTA_NAT_DST:
		/* deprecated */
		break;
	}
	tb[type] = attr;
	return MNL_CB_OK;
}

int
nfct_payload_parse(const void *payload, size_t payload_len,
		   uint16_t l3num, struct nf_conntrack *ct)
{
	struct nlattr *tb[CTA_MAX+1] = {};

	if (mnl_attr_parse_payload(payload, payload_len,
				   nfct_parse_conntrack_attr_cb, tb) < 0)
		return -1;

	if (tb[CTA_TUPLE_ORIG]) {
		ct->head.orig.l3protonum = l3num;
		set_bit(ATTR_ORIG_L3PROTO, ct->head.set);

		if (nfct_parse_tuple(tb[CTA_TUPLE_ORIG], &ct->head.orig,
				     __DIR_ORIG, ct->head.set) < 0)
			return -1;
	}

	if (tb[CTA_TUPLE_REPLY]) {
		ct->repl.l3protonum = l3num;
		set_bit(ATTR_REPL_L3PROTO, ct->head.set);

		if (nfct_parse_tuple(tb[CTA_TUPLE_REPLY], &ct->repl,
				     __DIR_REPL, ct->head.set) < 0)
			return -1;
	}

	if (tb[CTA_TUPLE_MASTER]) {
		ct->master.l3protonum = l3num;
		set_bit(ATTR_MASTER_L3PROTO, ct->head.set);

		if (nfct_parse_tuple(tb[CTA_TUPLE_MASTER], &ct->master,
				     __DIR_MASTER, ct->head.set) < 0)
			return -1;
	}

	if (tb[CTA_NAT_SEQ_ADJ_ORIG]) {
		if (nfct_parse_nat_seq(tb[CTA_NAT_SEQ_ADJ_ORIG],
					ct, __DIR_ORIG) < 0)
			return -1;
	}

	if (tb[CTA_NAT_SEQ_ADJ_REPLY]) {
		if (nfct_parse_nat_seq(tb[CTA_NAT_SEQ_ADJ_REPLY],
					ct, __DIR_REPL) < 0)
			return -1;
	}

	if (tb[CTA_STATUS]) {
		ct->status = ntohl(mnl_attr_get_u32(tb[CTA_STATUS]));
		set_bit(ATTR_STATUS, ct->head.set);
	}

	if (tb[CTA_PROTOINFO]) {
		if (nfct_parse_protoinfo(tb[CTA_PROTOINFO], ct) < 0)
			return -1;
	}

	if (tb[CTA_TIMEOUT]) {
		ct->timeout = ntohl(mnl_attr_get_u32(tb[CTA_TIMEOUT]));
		set_bit(ATTR_TIMEOUT, ct->head.set);
	}

	if (tb[CTA_MARK]) {
		ct->mark = ntohl(mnl_attr_get_u32(tb[CTA_MARK]));
		set_bit(ATTR_MARK, ct->head.set);
	}

	if (tb[CTA_SECMARK]) {
		ct->secmark = ntohl(mnl_attr_get_u32(tb[CTA_SECMARK]));
		set_bit(ATTR_SECMARK, ct->head.set);
	}

	if (tb[CTA_COUNTERS_ORIG]) {
		if (nfct_parse_counters(tb[CTA_COUNTERS_ORIG],
					ct, __DIR_ORIG) < 0)
			return -1;
	}

	if (tb[CTA_COUNTERS_REPLY]) {
		if (nfct_parse_counters(tb[CTA_COUNTERS_REPLY],
					ct, __DIR_REPL) < 0)
			return -1;
	}

	if (tb[CTA_USE]) {
		ct->use = ntohl(mnl_attr_get_u32(tb[CTA_USE]));
		set_bit(ATTR_USE, ct->head.set);
	}

	if (tb[CTA_ID]) {
		ct->id = ntohl(mnl_attr_get_u32(tb[CTA_ID]));
		set_bit(ATTR_ID, ct->head.set);
	}

	if (tb[CTA_HELP]) {
		if (nfct_parse_helper(tb[CTA_HELP], ct) < 0)
			return -1;
	}

	if (tb[CTA_ZONE]) {
		ct->zone = ntohs(mnl_attr_get_u16(tb[CTA_ZONE]));
		set_bit(ATTR_ZONE, ct->head.set);
	}

	if (tb[CTA_SECCTX]) {
		if (nfct_parse_secctx(tb[CTA_SECCTX], ct) < 0)
			return -1;
	}

	if (tb[CTA_TIMESTAMP]) {
		if (nfct_parse_timestamp(tb[CTA_TIMESTAMP], ct) < 0)
			return -1;
	}

	return 0;
}

int nfct_nlmsg_parse(const struct nlmsghdr *nlh, struct nf_conntrack *ct)
{
	struct nfgenmsg *nfhdr = mnl_nlmsg_get_payload(nlh);

	return nfct_payload_parse((uint8_t *)nfhdr + sizeof(struct nfgenmsg),
				  nlh->nlmsg_len, nfhdr->nfgen_family, ct);
}
