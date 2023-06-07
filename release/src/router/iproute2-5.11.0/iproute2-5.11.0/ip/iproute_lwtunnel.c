/*
 * iproute_lwtunnel.c
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Roopa Prabhu, <roopa@cumulusnetworks.com>
 *		Thomas Graf <tgraf@suug.ch>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <linux/ila.h>
#include <linux/lwtunnel.h>
#include <linux/mpls_iptunnel.h>
#include <errno.h>

#include "rt_names.h"
#include "bpf_util.h"
#include "utils.h"
#include "ip_common.h"
#include "ila_common.h"

#include <linux/seg6.h>
#include <linux/seg6_iptunnel.h>
#include <linux/rpl.h>
#include <linux/rpl_iptunnel.h>
#include <linux/seg6_hmac.h>
#include <linux/seg6_local.h>
#include <linux/if_tunnel.h>

static const char *format_encap_type(int type)
{
	switch (type) {
	case LWTUNNEL_ENCAP_MPLS:
		return "mpls";
	case LWTUNNEL_ENCAP_IP:
		return "ip";
	case LWTUNNEL_ENCAP_IP6:
		return "ip6";
	case LWTUNNEL_ENCAP_ILA:
		return "ila";
	case LWTUNNEL_ENCAP_BPF:
		return "bpf";
	case LWTUNNEL_ENCAP_SEG6:
		return "seg6";
	case LWTUNNEL_ENCAP_SEG6_LOCAL:
		return "seg6local";
	case LWTUNNEL_ENCAP_RPL:
		return "rpl";
	default:
		return "unknown";
	}
}

static void encap_type_usage(void)
{
	int i;

	fprintf(stderr, "Usage: ip route ... encap TYPE [ OPTIONS ] [...]\n");

	for (i = 1; i <= LWTUNNEL_ENCAP_MAX; i++)
		fprintf(stderr, "%s %s\n", format_encap_type(i),
			i == 1 ? "TYPE := " : "      ");

	exit(-1);
}

static int read_encap_type(const char *name)
{
	if (strcmp(name, "mpls") == 0)
		return LWTUNNEL_ENCAP_MPLS;
	else if (strcmp(name, "ip") == 0)
		return LWTUNNEL_ENCAP_IP;
	else if (strcmp(name, "ip6") == 0)
		return LWTUNNEL_ENCAP_IP6;
	else if (strcmp(name, "ila") == 0)
		return LWTUNNEL_ENCAP_ILA;
	else if (strcmp(name, "bpf") == 0)
		return LWTUNNEL_ENCAP_BPF;
	else if (strcmp(name, "seg6") == 0)
		return LWTUNNEL_ENCAP_SEG6;
	else if (strcmp(name, "seg6local") == 0)
		return LWTUNNEL_ENCAP_SEG6_LOCAL;
	else if (strcmp(name, "rpl") == 0)
		return LWTUNNEL_ENCAP_RPL;
	else if (strcmp(name, "help") == 0)
		encap_type_usage();

	return LWTUNNEL_ENCAP_NONE;
}

static void print_srh(FILE *fp, struct ipv6_sr_hdr *srh)
{
	int i;

	if (is_json_context())
		open_json_array(PRINT_JSON, "segs");
	else
		fprintf(fp, "segs %d [ ", srh->first_segment + 1);

	for (i = srh->first_segment; i >= 0; i--)
		print_color_string(PRINT_ANY, COLOR_INET6,
				   NULL, "%s ",
				   rt_addr_n2a(AF_INET6, 16, &srh->segments[i]));

	if (is_json_context())
		close_json_array(PRINT_JSON, NULL);
	else
		fprintf(fp, "] ");

	if (sr_has_hmac(srh)) {
		unsigned int offset = ((srh->hdrlen + 1) << 3) - 40;
		struct sr6_tlv_hmac *tlv;

		tlv = (struct sr6_tlv_hmac *)((char *)srh + offset);
		print_0xhex(PRINT_ANY, "hmac",
			    "hmac %llX ", ntohl(tlv->hmackeyid));
	}
}

static const char *seg6_mode_types[] = {
	[SEG6_IPTUN_MODE_INLINE]	= "inline",
	[SEG6_IPTUN_MODE_ENCAP]		= "encap",
	[SEG6_IPTUN_MODE_L2ENCAP]	= "l2encap",
};

static const char *format_seg6mode_type(int mode)
{
	if (mode < 0 || mode > ARRAY_SIZE(seg6_mode_types))
		return "<unknown>";

	return seg6_mode_types[mode];
}

static int read_seg6mode_type(const char *mode)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(seg6_mode_types); i++) {
		if (strcmp(mode, seg6_mode_types[i]) == 0)
			return i;
	}

	return -1;
}

static void print_encap_seg6(FILE *fp, struct rtattr *encap)
{
	struct rtattr *tb[SEG6_IPTUNNEL_MAX+1];
	struct seg6_iptunnel_encap *tuninfo;

	parse_rtattr_nested(tb, SEG6_IPTUNNEL_MAX, encap);

	if (!tb[SEG6_IPTUNNEL_SRH])
		return;

	tuninfo = RTA_DATA(tb[SEG6_IPTUNNEL_SRH]);
	print_string(PRINT_ANY, "mode",
		     "mode %s ", format_seg6mode_type(tuninfo->mode));

	print_srh(fp, tuninfo->srh);
}

static void print_rpl_srh(FILE *fp, struct ipv6_rpl_sr_hdr *srh)
{
	int i;

	if (is_json_context())
		open_json_array(PRINT_JSON, "segs");
	else
		fprintf(fp, "segs %d [ ", srh->segments_left);

	for (i = srh->segments_left - 1; i >= 0; i--) {
		print_color_string(PRINT_ANY, COLOR_INET6,
				   NULL, "%s ",
				   rt_addr_n2a(AF_INET6, 16, &srh->rpl_segaddr[i]));
	}

	if (is_json_context())
		close_json_array(PRINT_JSON, NULL);
	else
		fprintf(fp, "] ");
}

static void print_encap_rpl(FILE *fp, struct rtattr *encap)
{
	struct rtattr *tb[RPL_IPTUNNEL_MAX + 1];
	struct ipv6_rpl_sr_hdr *srh;

	parse_rtattr_nested(tb, RPL_IPTUNNEL_MAX, encap);

	if (!tb[RPL_IPTUNNEL_SRH])
		return;

	srh = RTA_DATA(tb[RPL_IPTUNNEL_SRH]);

	print_rpl_srh(fp, srh);
}

static const char *seg6_action_names[SEG6_LOCAL_ACTION_MAX + 1] = {
	[SEG6_LOCAL_ACTION_END]			= "End",
	[SEG6_LOCAL_ACTION_END_X]		= "End.X",
	[SEG6_LOCAL_ACTION_END_T]		= "End.T",
	[SEG6_LOCAL_ACTION_END_DX2]		= "End.DX2",
	[SEG6_LOCAL_ACTION_END_DX6]		= "End.DX6",
	[SEG6_LOCAL_ACTION_END_DX4]		= "End.DX4",
	[SEG6_LOCAL_ACTION_END_DT6]		= "End.DT6",
	[SEG6_LOCAL_ACTION_END_DT4]		= "End.DT4",
	[SEG6_LOCAL_ACTION_END_B6]		= "End.B6",
	[SEG6_LOCAL_ACTION_END_B6_ENCAP]	= "End.B6.Encaps",
	[SEG6_LOCAL_ACTION_END_BM]		= "End.BM",
	[SEG6_LOCAL_ACTION_END_S]		= "End.S",
	[SEG6_LOCAL_ACTION_END_AS]		= "End.AS",
	[SEG6_LOCAL_ACTION_END_AM]		= "End.AM",
	[SEG6_LOCAL_ACTION_END_BPF]		= "End.BPF",
};

static const char *format_action_type(int action)
{
	if (action < 0 || action > SEG6_LOCAL_ACTION_MAX)
		return "<invalid>";

	return seg6_action_names[action] ?: "<unknown>";
}

static int read_action_type(const char *name)
{
	int i;

	for (i = 0; i < SEG6_LOCAL_ACTION_MAX + 1; i++) {
		if (!seg6_action_names[i])
			continue;

		if (strcmp(seg6_action_names[i], name) == 0)
			return i;
	}

	return SEG6_LOCAL_ACTION_UNSPEC;
}

static void print_encap_bpf_prog(FILE *fp, struct rtattr *encap,
				 const char *str)
{
	struct rtattr *tb[LWT_BPF_PROG_MAX+1];
	const char *progname = NULL;

	parse_rtattr_nested(tb, LWT_BPF_PROG_MAX, encap);

	if (tb[LWT_BPF_PROG_NAME])
		progname = rta_getattr_str(tb[LWT_BPF_PROG_NAME]);

	if (is_json_context())
		print_string(PRINT_JSON, str, NULL,
			     progname ? : "<unknown>");
	else {
		fprintf(fp, "%s ", str);
		if (progname)
			fprintf(fp, "%s ", progname);
	}
}

static void print_encap_seg6local(FILE *fp, struct rtattr *encap)
{
	struct rtattr *tb[SEG6_LOCAL_MAX + 1];
	int action;

	SPRINT_BUF(b1);

	parse_rtattr_nested(tb, SEG6_LOCAL_MAX, encap);

	if (!tb[SEG6_LOCAL_ACTION])
		return;

	action = rta_getattr_u32(tb[SEG6_LOCAL_ACTION]);

	print_string(PRINT_ANY, "action",
		     "action %s ", format_action_type(action));

	if (tb[SEG6_LOCAL_SRH]) {
		open_json_object("srh");
		print_srh(fp, RTA_DATA(tb[SEG6_LOCAL_SRH]));
		close_json_object();
	}

	if (tb[SEG6_LOCAL_TABLE])
		print_string(PRINT_ANY, "table", "table %s ",
			     rtnl_rttable_n2a(rta_getattr_u32(tb[SEG6_LOCAL_TABLE]),
			     b1, sizeof(b1)));

	if (tb[SEG6_LOCAL_VRFTABLE])
		print_string(PRINT_ANY, "vrftable", "vrftable %s ",
			     rtnl_rttable_n2a(rta_getattr_u32(tb[SEG6_LOCAL_VRFTABLE]),
			     b1, sizeof(b1)));

	if (tb[SEG6_LOCAL_NH4]) {
		print_string(PRINT_ANY, "nh4",
			     "nh4 %s ", rt_addr_n2a_rta(AF_INET, tb[SEG6_LOCAL_NH4]));
	}

	if (tb[SEG6_LOCAL_NH6]) {
		print_string(PRINT_ANY, "nh6",
			     "nh6 %s ", rt_addr_n2a_rta(AF_INET6, tb[SEG6_LOCAL_NH6]));
	}

	if (tb[SEG6_LOCAL_IIF]) {
		int iif = rta_getattr_u32(tb[SEG6_LOCAL_IIF]);

		print_string(PRINT_ANY, "iif",
			     "iif %s ", ll_index_to_name(iif));
	}

	if (tb[SEG6_LOCAL_OIF]) {
		int oif = rta_getattr_u32(tb[SEG6_LOCAL_OIF]);

		print_string(PRINT_ANY, "oif",
			     "oif %s ", ll_index_to_name(oif));
	}

	if (tb[SEG6_LOCAL_BPF])
		print_encap_bpf_prog(fp, tb[SEG6_LOCAL_BPF], "endpoint");
}

static void print_encap_mpls(FILE *fp, struct rtattr *encap)
{
	struct rtattr *tb[MPLS_IPTUNNEL_MAX+1];

	parse_rtattr_nested(tb, MPLS_IPTUNNEL_MAX, encap);

	if (tb[MPLS_IPTUNNEL_DST])
		print_string(PRINT_ANY, "dst", " %s ",
			format_host_rta(AF_MPLS, tb[MPLS_IPTUNNEL_DST]));
	if (tb[MPLS_IPTUNNEL_TTL])
		print_uint(PRINT_ANY, "ttl", "ttl %u ",
			rta_getattr_u8(tb[MPLS_IPTUNNEL_TTL]));
}

static void lwtunnel_print_geneve_opts(struct rtattr *attr)
{
	struct rtattr *tb[LWTUNNEL_IP_OPT_GENEVE_MAX + 1];
	struct rtattr *i = RTA_DATA(attr);
	int rem = RTA_PAYLOAD(attr);
	char *name = "geneve_opts";
	int data_len, offset = 0;
	char data[rem * 2 + 1];
	__u16 class;
	__u8 type;

	print_nl();
	print_string(PRINT_FP, name, "\t%s ", name);
	open_json_array(PRINT_JSON, name);

	while (rem) {
		parse_rtattr(tb, LWTUNNEL_IP_OPT_GENEVE_MAX, i, rem);
		class = rta_getattr_be16(tb[LWTUNNEL_IP_OPT_GENEVE_CLASS]);
		type = rta_getattr_u8(tb[LWTUNNEL_IP_OPT_GENEVE_TYPE]);
		data_len = RTA_PAYLOAD(tb[LWTUNNEL_IP_OPT_GENEVE_DATA]);
		hexstring_n2a(RTA_DATA(tb[LWTUNNEL_IP_OPT_GENEVE_DATA]),
			      data_len, data, sizeof(data));
		offset += data_len + 20;
		rem -= data_len + 20;
		i = RTA_DATA(attr) + offset;

		open_json_object(NULL);
		print_uint(PRINT_ANY, "class", "%u", class);
		print_uint(PRINT_ANY, "type", ":%u", type);
		if (rem)
			print_string(PRINT_ANY, "data", ":%s,", data);
		else
			print_string(PRINT_ANY, "data", ":%s ", data);
		close_json_object();
	}

	close_json_array(PRINT_JSON, name);
}

static void lwtunnel_print_vxlan_opts(struct rtattr *attr)
{
	struct rtattr *tb[LWTUNNEL_IP_OPT_VXLAN_MAX + 1];
	struct rtattr *i = RTA_DATA(attr);
	int rem = RTA_PAYLOAD(attr);
	char *name = "vxlan_opts";
	__u32 gbp;

	parse_rtattr(tb, LWTUNNEL_IP_OPT_VXLAN_MAX, i, rem);
	gbp = rta_getattr_u32(tb[LWTUNNEL_IP_OPT_VXLAN_GBP]);

	print_nl();
	print_string(PRINT_FP, name, "\t%s ", name);
	open_json_array(PRINT_JSON, name);
	open_json_object(NULL);
	print_uint(PRINT_ANY, "gbp", "%u ", gbp);
	close_json_object();
	close_json_array(PRINT_JSON, name);
}

static void lwtunnel_print_erspan_opts(struct rtattr *attr)
{
	struct rtattr *tb[LWTUNNEL_IP_OPT_ERSPAN_MAX + 1];
	struct rtattr *i = RTA_DATA(attr);
	char *name = "erspan_opts";
	__u8 ver, hwid, dir;
	__u32 idx;

	parse_rtattr(tb, LWTUNNEL_IP_OPT_ERSPAN_MAX, i, RTA_PAYLOAD(attr));
	ver = rta_getattr_u8(tb[LWTUNNEL_IP_OPT_ERSPAN_VER]);
	if (ver == 1) {
		idx = rta_getattr_be32(tb[LWTUNNEL_IP_OPT_ERSPAN_INDEX]);
		dir = 0;
		hwid = 0;
	} else {
		idx = 0;
		dir = rta_getattr_u8(tb[LWTUNNEL_IP_OPT_ERSPAN_DIR]);
		hwid = rta_getattr_u8(tb[LWTUNNEL_IP_OPT_ERSPAN_HWID]);
	}

	print_nl();
	print_string(PRINT_FP, name, "\t%s ", name);
	open_json_array(PRINT_JSON, name);
	open_json_object(NULL);
	print_uint(PRINT_ANY, "ver", "%u", ver);
	print_uint(PRINT_ANY, "index", ":%u", idx);
	print_uint(PRINT_ANY, "dir", ":%u", dir);
	print_uint(PRINT_ANY, "hwid", ":%u ", hwid);
	close_json_object();
	close_json_array(PRINT_JSON, name);
}

static void lwtunnel_print_opts(struct rtattr *attr)
{
	struct rtattr *tb_opt[LWTUNNEL_IP_OPTS_MAX + 1];

	parse_rtattr_nested(tb_opt, LWTUNNEL_IP_OPTS_MAX, attr);
	if (tb_opt[LWTUNNEL_IP_OPTS_GENEVE])
		lwtunnel_print_geneve_opts(tb_opt[LWTUNNEL_IP_OPTS_GENEVE]);
	else if (tb_opt[LWTUNNEL_IP_OPTS_VXLAN])
		lwtunnel_print_vxlan_opts(tb_opt[LWTUNNEL_IP_OPTS_VXLAN]);
	else if (tb_opt[LWTUNNEL_IP_OPTS_ERSPAN])
		lwtunnel_print_erspan_opts(tb_opt[LWTUNNEL_IP_OPTS_ERSPAN]);
}

static void print_encap_ip(FILE *fp, struct rtattr *encap)
{
	struct rtattr *tb[LWTUNNEL_IP_MAX+1];
	__u16 flags;

	parse_rtattr_nested(tb, LWTUNNEL_IP_MAX, encap);

	if (tb[LWTUNNEL_IP_ID])
		print_u64(PRINT_ANY, "id", "id %llu ",
			   ntohll(rta_getattr_u64(tb[LWTUNNEL_IP_ID])));

	if (tb[LWTUNNEL_IP_SRC])
		print_color_string(PRINT_ANY, COLOR_INET,
				   "src", "src %s ",
				   rt_addr_n2a_rta(AF_INET, tb[LWTUNNEL_IP_SRC]));

	if (tb[LWTUNNEL_IP_DST])
		print_color_string(PRINT_ANY, COLOR_INET,
				   "dst", "dst %s ",
				   rt_addr_n2a_rta(AF_INET, tb[LWTUNNEL_IP_DST]));

	if (tb[LWTUNNEL_IP_TTL])
		print_uint(PRINT_ANY, "ttl",
			   "ttl %u ", rta_getattr_u8(tb[LWTUNNEL_IP_TTL]));

	if (tb[LWTUNNEL_IP_TOS])
		print_uint(PRINT_ANY, "tos",
			   "tos %d ", rta_getattr_u8(tb[LWTUNNEL_IP_TOS]));

	if (tb[LWTUNNEL_IP_FLAGS]) {
		flags = rta_getattr_u16(tb[LWTUNNEL_IP_FLAGS]);
		if (flags & TUNNEL_KEY)
			print_bool(PRINT_ANY, "key", "key ", true);
		if (flags & TUNNEL_CSUM)
			print_bool(PRINT_ANY, "csum", "csum ", true);
		if (flags & TUNNEL_SEQ)
			print_bool(PRINT_ANY, "seq", "seq ", true);
	}

	if (tb[LWTUNNEL_IP_OPTS])
		lwtunnel_print_opts(tb[LWTUNNEL_IP_OPTS]);
}

static void print_encap_ila(FILE *fp, struct rtattr *encap)
{
	struct rtattr *tb[ILA_ATTR_MAX+1];

	parse_rtattr_nested(tb, ILA_ATTR_MAX, encap);

	if (tb[ILA_ATTR_LOCATOR]) {
		char abuf[ADDR64_BUF_SIZE];

		addr64_n2a(rta_getattr_u64(tb[ILA_ATTR_LOCATOR]),
			   abuf, sizeof(abuf));
		print_string(PRINT_ANY, "locator",
			     " %s ", abuf);
	}

	if (tb[ILA_ATTR_CSUM_MODE])
		print_string(PRINT_ANY, "csum_mode",
			     " csum-mode %s ",
			     ila_csum_mode2name(rta_getattr_u8(tb[ILA_ATTR_CSUM_MODE])));

	if (tb[ILA_ATTR_IDENT_TYPE])
		print_string(PRINT_ANY, "ident_type",
			     " ident-type %s ",
			     ila_ident_type2name(rta_getattr_u8(tb[ILA_ATTR_IDENT_TYPE])));

	if (tb[ILA_ATTR_HOOK_TYPE])
		print_string(PRINT_ANY, "hook_type",
			     " hook-type %s ",
			     ila_hook_type2name(rta_getattr_u8(tb[ILA_ATTR_HOOK_TYPE])));
}

static void print_encap_ip6(FILE *fp, struct rtattr *encap)
{
	struct rtattr *tb[LWTUNNEL_IP6_MAX+1];
	__u16 flags;

	parse_rtattr_nested(tb, LWTUNNEL_IP6_MAX, encap);

	if (tb[LWTUNNEL_IP6_ID])
		print_u64(PRINT_ANY, "id", "id %llu ",
			    ntohll(rta_getattr_u64(tb[LWTUNNEL_IP6_ID])));

	if (tb[LWTUNNEL_IP6_SRC])
		print_color_string(PRINT_ANY, COLOR_INET6,
				   "src", "src %s ",
				   rt_addr_n2a_rta(AF_INET6, tb[LWTUNNEL_IP6_SRC]));

	if (tb[LWTUNNEL_IP6_DST])
		print_color_string(PRINT_ANY, COLOR_INET6,
				   "dst", "dst %s ",
				   rt_addr_n2a_rta(AF_INET6, tb[LWTUNNEL_IP6_DST]));

	if (tb[LWTUNNEL_IP6_HOPLIMIT])
		print_u64(PRINT_ANY, "hoplimit",
			   "hoplimit %u ",
			   rta_getattr_u8(tb[LWTUNNEL_IP6_HOPLIMIT]));

	if (tb[LWTUNNEL_IP6_TC])
		print_uint(PRINT_ANY, "tc",
			   "tc %u ", rta_getattr_u8(tb[LWTUNNEL_IP6_TC]));

	if (tb[LWTUNNEL_IP6_FLAGS]) {
		flags = rta_getattr_u16(tb[LWTUNNEL_IP6_FLAGS]);
		if (flags & TUNNEL_KEY)
			print_bool(PRINT_ANY, "key", "key ", true);
		if (flags & TUNNEL_CSUM)
			print_bool(PRINT_ANY, "csum", "csum ", true);
		if (flags & TUNNEL_SEQ)
			print_bool(PRINT_ANY, "seq", "seq ", true);
	}

	if (tb[LWTUNNEL_IP6_OPTS])
		lwtunnel_print_opts(tb[LWTUNNEL_IP6_OPTS]);
}

static void print_encap_bpf(FILE *fp, struct rtattr *encap)
{
	struct rtattr *tb[LWT_BPF_MAX+1];

	parse_rtattr_nested(tb, LWT_BPF_MAX, encap);

	if (tb[LWT_BPF_IN])
		print_encap_bpf_prog(fp, tb[LWT_BPF_IN], "in");
	if (tb[LWT_BPF_OUT])
		print_encap_bpf_prog(fp, tb[LWT_BPF_OUT], "out");
	if (tb[LWT_BPF_XMIT])
		print_encap_bpf_prog(fp, tb[LWT_BPF_XMIT], "xmit");
	if (tb[LWT_BPF_XMIT_HEADROOM])
		print_uint(PRINT_ANY, "headroom",
			   " %u ", rta_getattr_u32(tb[LWT_BPF_XMIT_HEADROOM]));
}

void lwt_print_encap(FILE *fp, struct rtattr *encap_type,
			  struct rtattr *encap)
{
	int et;

	if (!encap_type)
		return;

	et = rta_getattr_u16(encap_type);

	print_string(PRINT_ANY, "encap", " encap %s ", format_encap_type(et));

	switch (et) {
	case LWTUNNEL_ENCAP_MPLS:
		print_encap_mpls(fp, encap);
		break;
	case LWTUNNEL_ENCAP_IP:
		print_encap_ip(fp, encap);
		break;
	case LWTUNNEL_ENCAP_ILA:
		print_encap_ila(fp, encap);
		break;
	case LWTUNNEL_ENCAP_IP6:
		print_encap_ip6(fp, encap);
		break;
	case LWTUNNEL_ENCAP_BPF:
		print_encap_bpf(fp, encap);
		break;
	case LWTUNNEL_ENCAP_SEG6:
		print_encap_seg6(fp, encap);
		break;
	case LWTUNNEL_ENCAP_SEG6_LOCAL:
		print_encap_seg6local(fp, encap);
		break;
	case LWTUNNEL_ENCAP_RPL:
		print_encap_rpl(fp, encap);
		break;
	}
}

static struct ipv6_sr_hdr *parse_srh(char *segbuf, int hmac, bool encap)
{
	struct ipv6_sr_hdr *srh;
	int nsegs = 0;
	int srhlen;
	char *s;
	int i;

	s = segbuf;
	for (i = 0; *s; *s++ == ',' ? i++ : *s);
	nsegs = i + 1;

	if (!encap)
		nsegs++;

	srhlen = 8 + 16*nsegs;

	if (hmac)
		srhlen += 40;

	srh = malloc(srhlen);
	memset(srh, 0, srhlen);

	srh->hdrlen = (srhlen >> 3) - 1;
	srh->type = 4;
	srh->segments_left = nsegs - 1;
	srh->first_segment = nsegs - 1;

	if (hmac)
		srh->flags |= SR6_FLAG1_HMAC;

	i = srh->first_segment;
	for (s = strtok(segbuf, ","); s; s = strtok(NULL, ",")) {
		inet_prefix addr;

		get_addr(&addr, s, AF_INET6);
		memcpy(&srh->segments[i], addr.data, sizeof(struct in6_addr));
		i--;
	}

	if (hmac) {
		struct sr6_tlv_hmac *tlv;

		tlv = (struct sr6_tlv_hmac *)((char *)srh + srhlen - 40);
		tlv->tlvhdr.type = SR6_TLV_HMAC;
		tlv->tlvhdr.len = 38;
		tlv->hmackeyid = htonl(hmac);
	}

	return srh;
}

static int parse_encap_seg6(struct rtattr *rta, size_t len, int *argcp,
			    char ***argvp)
{
	int mode_ok = 0, segs_ok = 0, hmac_ok = 0;
	struct seg6_iptunnel_encap *tuninfo;
	struct ipv6_sr_hdr *srh;
	char **argv = *argvp;
	char segbuf[1024] = "";
	int argc = *argcp;
	int encap = -1;
	__u32 hmac = 0;
	int ret = 0;
	int srhlen;

	while (argc > 0) {
		if (strcmp(*argv, "mode") == 0) {
			NEXT_ARG();
			if (mode_ok++)
				duparg2("mode", *argv);
			encap = read_seg6mode_type(*argv);
			if (encap < 0)
				invarg("\"mode\" value is invalid\n", *argv);
		} else if (strcmp(*argv, "segs") == 0) {
			NEXT_ARG();
			if (segs_ok++)
				duparg2("segs", *argv);
			if (encap == -1)
				invarg("\"segs\" provided before \"mode\"\n",
				       *argv);

			strlcpy(segbuf, *argv, 1024);
		} else if (strcmp(*argv, "hmac") == 0) {
			NEXT_ARG();
			if (hmac_ok++)
				duparg2("hmac", *argv);
			get_u32(&hmac, *argv, 0);
		} else {
			break;
		}
		argc--; argv++;
	}

	srh = parse_srh(segbuf, hmac, encap);
	srhlen = (srh->hdrlen + 1) << 3;

	tuninfo = malloc(sizeof(*tuninfo) + srhlen);
	memset(tuninfo, 0, sizeof(*tuninfo) + srhlen);

	tuninfo->mode = encap;

	memcpy(tuninfo->srh, srh, srhlen);

	if (rta_addattr_l(rta, len, SEG6_IPTUNNEL_SRH, tuninfo,
			  sizeof(*tuninfo) + srhlen)) {
		ret = -1;
		goto out;
	}

	*argcp = argc + 1;
	*argvp = argv - 1;

out:
	free(tuninfo);
	free(srh);

	return ret;
}

static struct ipv6_rpl_sr_hdr *parse_rpl_srh(char *segbuf)
{
	struct ipv6_rpl_sr_hdr *srh;
	int nsegs = 0;
	int srhlen;
	char *s;
	int i;

	s = segbuf;
	for (i = 0; *s; *s++ == ',' ? i++ : *s);
	nsegs = i + 1;

	srhlen = 8 + 16 * nsegs;

	srh = calloc(1, srhlen);

	srh->hdrlen = (srhlen >> 3) - 1;
	srh->type = 3;
	srh->segments_left = nsegs;

	for (s = strtok(segbuf, ","); s; s = strtok(NULL, ",")) {
		inet_prefix addr;

		get_addr(&addr, s, AF_INET6);
		memcpy(&srh->rpl_segaddr[i], addr.data, sizeof(struct in6_addr));
		i--;
	}

	return srh;
}

static int parse_encap_rpl(struct rtattr *rta, size_t len, int *argcp,
			   char ***argvp)
{
	struct ipv6_rpl_sr_hdr *srh;
	char **argv = *argvp;
	char segbuf[1024] = "";
	int argc = *argcp;
	int segs_ok = 0;
	int ret = 0;
	int srhlen;

	while (argc > 0) {
		if (strcmp(*argv, "segs") == 0) {
			NEXT_ARG();
			if (segs_ok++)
				duparg2("segs", *argv);

			strlcpy(segbuf, *argv, 1024);
		} else {
			break;
		}
		argc--; argv++;
	}

	srh = parse_rpl_srh(segbuf);
	srhlen = (srh->hdrlen + 1) << 3;

	if (rta_addattr_l(rta, len, RPL_IPTUNNEL_SRH, srh,
			  srhlen)) {
		ret = -1;
		goto out;
	}

	*argcp = argc + 1;
	*argvp = argv - 1;

out:
	free(srh);

	return ret;
}

struct lwt_x {
	struct rtattr *rta;
	size_t len;
};

static void bpf_lwt_cb(void *lwt_ptr, int fd, const char *annotation)
{
	struct lwt_x *x = lwt_ptr;

	rta_addattr32(x->rta, x->len, LWT_BPF_PROG_FD, fd);
	rta_addattr_l(x->rta, x->len, LWT_BPF_PROG_NAME, annotation,
		      strlen(annotation) + 1);
}

static const struct bpf_cfg_ops bpf_cb_ops = {
	.ebpf_cb = bpf_lwt_cb,
};

static int lwt_parse_bpf(struct rtattr *rta, size_t len,
			 int *argcp, char ***argvp,
			 int attr, const enum bpf_prog_type bpf_type)
{
	struct bpf_cfg_in cfg = {
		.type = bpf_type,
		.argc = *argcp,
		.argv = *argvp,
	};
	struct lwt_x x = {
		.rta = rta,
		.len = len,
	};
	struct rtattr *nest;
	int err;

	nest = rta_nest(rta, len, attr);
	err = bpf_parse_and_load_common(&cfg, &bpf_cb_ops, &x);
	if (err < 0) {
		fprintf(stderr, "Failed to parse eBPF program: %s\n",
			strerror(-err));
		return -1;
	}
	rta_nest_end(rta, nest);

	*argcp = cfg.argc;
	*argvp = cfg.argv;

	return 0;
}

static int parse_encap_seg6local(struct rtattr *rta, size_t len, int *argcp,
				 char ***argvp)
{
	int segs_ok = 0, hmac_ok = 0, table_ok = 0, vrftable_ok = 0;
	int nh4_ok = 0, nh6_ok = 0, iif_ok = 0, oif_ok = 0;
	__u32 action = 0, table, vrftable, iif, oif;
	int action_ok = 0, srh_ok = 0, bpf_ok = 0;
	struct ipv6_sr_hdr *srh;
	char **argv = *argvp;
	int argc = *argcp;
	char segbuf[1024];
	inet_prefix addr;
	__u32 hmac = 0;
	int ret = 0;

	while (argc > 0) {
		if (strcmp(*argv, "action") == 0) {
			NEXT_ARG();
			if (action_ok++)
				duparg2("action", *argv);
			action = read_action_type(*argv);
			if (!action)
				invarg("\"action\" value is invalid\n", *argv);
			ret = rta_addattr32(rta, len, SEG6_LOCAL_ACTION,
					    action);
		} else if (strcmp(*argv, "table") == 0) {
			NEXT_ARG();
			if (table_ok++)
				duparg2("table", *argv);
			if (rtnl_rttable_a2n(&table, *argv))
				invarg("invalid table id\n", *argv);
			ret = rta_addattr32(rta, len, SEG6_LOCAL_TABLE, table);
		} else if (strcmp(*argv, "vrftable") == 0) {
			NEXT_ARG();
			if (vrftable_ok++)
				duparg2("vrftable", *argv);
			if (rtnl_rttable_a2n(&vrftable, *argv))
				invarg("invalid vrf table id\n", *argv);
			ret = rta_addattr32(rta, len, SEG6_LOCAL_VRFTABLE,
					    vrftable);
		} else if (strcmp(*argv, "nh4") == 0) {
			NEXT_ARG();
			if (nh4_ok++)
				duparg2("nh4", *argv);
			get_addr(&addr, *argv, AF_INET);
			ret = rta_addattr_l(rta, len, SEG6_LOCAL_NH4,
					    &addr.data, addr.bytelen);
		} else if (strcmp(*argv, "nh6") == 0) {
			NEXT_ARG();
			if (nh6_ok++)
				duparg2("nh6", *argv);
			get_addr(&addr, *argv, AF_INET6);
			ret = rta_addattr_l(rta, len, SEG6_LOCAL_NH6,
					    &addr.data, addr.bytelen);
		} else if (strcmp(*argv, "iif") == 0) {
			NEXT_ARG();
			if (iif_ok++)
				duparg2("iif", *argv);
			iif = ll_name_to_index(*argv);
			if (!iif)
				exit(nodev(*argv));
			ret = rta_addattr32(rta, len, SEG6_LOCAL_IIF, iif);
		} else if (strcmp(*argv, "oif") == 0) {
			NEXT_ARG();
			if (oif_ok++)
				duparg2("oif", *argv);
			oif = ll_name_to_index(*argv);
			if (!oif)
				exit(nodev(*argv));
			ret = rta_addattr32(rta, len, SEG6_LOCAL_OIF, oif);
		} else if (strcmp(*argv, "srh") == 0) {
			NEXT_ARG();
			if (srh_ok++)
				duparg2("srh", *argv);
			if (strcmp(*argv, "segs") != 0)
				invarg("missing \"segs\" attribute for srh\n",
					*argv);
			NEXT_ARG();
			if (segs_ok++)
				duparg2("segs", *argv);
			strncpy(segbuf, *argv, 1024);
			segbuf[1023] = 0;
			if (!NEXT_ARG_OK())
				break;
			NEXT_ARG();
			if (strcmp(*argv, "hmac") == 0) {
				NEXT_ARG();
				if (hmac_ok++)
					duparg2("hmac", *argv);
				get_u32(&hmac, *argv, 0);
			} else {
				continue;
			}
		} else if (strcmp(*argv, "endpoint") == 0) {
			NEXT_ARG();
			if (bpf_ok++)
				duparg2("endpoint", *argv);

			if (lwt_parse_bpf(rta, len, &argc, &argv, SEG6_LOCAL_BPF,
			    BPF_PROG_TYPE_LWT_SEG6LOCAL) < 0)
				exit(-1);
		} else {
			break;
		}
		if (ret)
			return ret;
		argc--; argv++;
	}

	if (!action) {
		fprintf(stderr, "Missing action type\n");
		exit(-1);
	}

	if (srh_ok) {
		int srhlen;

		srh = parse_srh(segbuf, hmac,
				action == SEG6_LOCAL_ACTION_END_B6_ENCAP);
		srhlen = (srh->hdrlen + 1) << 3;
		ret = rta_addattr_l(rta, len, SEG6_LOCAL_SRH, srh, srhlen);
		free(srh);
	}

	*argcp = argc + 1;
	*argvp = argv - 1;

	return ret;
}

static int parse_encap_mpls(struct rtattr *rta, size_t len,
			    int *argcp, char ***argvp)
{
	inet_prefix addr;
	int argc = *argcp;
	char **argv = *argvp;
	int ttl_ok = 0;

	if (get_addr(&addr, *argv, AF_MPLS)) {
		fprintf(stderr,
			"Error: an inet address is expected rather than \"%s\".\n",
			*argv);
		exit(1);
	}

	if (rta_addattr_l(rta, len, MPLS_IPTUNNEL_DST,
			  &addr.data, addr.bytelen))
		return -1;

	argc--;
	argv++;

	while (argc > 0) {
		if (strcmp(*argv, "ttl") == 0) {
			__u8 ttl;

			NEXT_ARG();
			if (ttl_ok++)
				duparg2("ttl", *argv);
			if (get_u8(&ttl, *argv, 0))
				invarg("\"ttl\" value is invalid\n", *argv);
			if (rta_addattr8(rta, len, MPLS_IPTUNNEL_TTL, ttl))
				return -1;
		} else {
			break;
		}
		argc--; argv++;
	}

	/* argv is currently the first unparsed argument,
	 * but the lwt_parse_encap() caller will move to the next,
	 * so step back
	 */
	*argcp = argc + 1;
	*argvp = argv - 1;

	return 0;
}

static int lwtunnel_parse_geneve_opt(char *str, size_t len, struct rtattr *rta)
{
	struct rtattr *nest;
	char *token;
	int i, err;

	nest = rta_nest(rta, len, LWTUNNEL_IP_OPTS_GENEVE | NLA_F_NESTED);
	i = 1;
	token = strsep(&str, ":");
	while (token) {
		switch (i) {
		case LWTUNNEL_IP_OPT_GENEVE_CLASS:
		{
			__be16 opt_class;

			if (!strlen(token))
				break;
			err = get_be16(&opt_class, token, 0);
			if (err)
				return err;

			rta_addattr16(rta, len, i, opt_class);
			break;
		}
		case LWTUNNEL_IP_OPT_GENEVE_TYPE:
		{
			__u8 opt_type;

			if (!strlen(token))
				break;
			err = get_u8(&opt_type, token, 0);
			if (err)
				return err;

			rta_addattr8(rta, len, i, opt_type);
			break;
		}
		case LWTUNNEL_IP_OPT_GENEVE_DATA:
		{
			size_t token_len = strlen(token);
			__u8 *opts;

			if (!token_len)
				break;
			opts = malloc(token_len / 2);
			if (!opts)
				return -1;
			if (hex2mem(token, opts, token_len / 2) < 0) {
				free(opts);
				return -1;
			}
			rta_addattr_l(rta, len, i, opts, token_len / 2);
			free(opts);

			break;
		}
		default:
			fprintf(stderr, "Unknown \"geneve_opts\" type\n");
			return -1;
		}

		token = strsep(&str, ":");
		i++;
	}
	rta_nest_end(rta, nest);

	return 0;
}

static int lwtunnel_parse_geneve_opts(char *str, size_t len, struct rtattr *rta)
{
	char *token;
	int err;

	token = strsep(&str, ",");
	while (token) {
		err = lwtunnel_parse_geneve_opt(token, len, rta);
		if (err)
			return err;

		token = strsep(&str, ",");
	}

	return 0;
}

static int lwtunnel_parse_vxlan_opts(char *str, size_t len, struct rtattr *rta)
{
	struct rtattr *nest;
	__u32 gbp;
	int err;

	nest = rta_nest(rta, len, LWTUNNEL_IP_OPTS_VXLAN | NLA_F_NESTED);
	err = get_u32(&gbp, str, 0);
	if (err)
		return err;
	rta_addattr32(rta, len, LWTUNNEL_IP_OPT_VXLAN_GBP, gbp);

	rta_nest_end(rta, nest);
	return 0;
}

static int lwtunnel_parse_erspan_opts(char *str, size_t len, struct rtattr *rta)
{
	struct rtattr *nest;
	char *token;
	int i, err;

	nest = rta_nest(rta, len, LWTUNNEL_IP_OPTS_ERSPAN | NLA_F_NESTED);
	i = 1;
	token = strsep(&str, ":");
	while (token) {
		switch (i) {
		case LWTUNNEL_IP_OPT_ERSPAN_VER:
		{
			__u8 opt_type;

			if (!strlen(token))
				break;
			err = get_u8(&opt_type, token, 0);
			if (err)
				return err;

			rta_addattr8(rta, len, i, opt_type);
			break;
		}
		case LWTUNNEL_IP_OPT_ERSPAN_INDEX:
		{
			__be32 opt_class;

			if (!strlen(token))
				break;
			err = get_be32(&opt_class, token, 0);
			if (err)
				return err;

			rta_addattr32(rta, len, i, opt_class);
			break;
		}
		case LWTUNNEL_IP_OPT_ERSPAN_DIR:
		{
			__u8 opt_type;

			if (!strlen(token))
				break;
			err = get_u8(&opt_type, token, 0);
			if (err)
				return err;

			rta_addattr8(rta, len, i, opt_type);
			break;
		}
		case LWTUNNEL_IP_OPT_ERSPAN_HWID:
		{
			__u8 opt_type;

			if (!strlen(token))
				break;
			err = get_u8(&opt_type, token, 0);
			if (err)
				return err;

			rta_addattr8(rta, len, i, opt_type);
			break;
		}
		default:
			fprintf(stderr, "Unknown \"geneve_opts\" type\n");
			return -1;
		}

		token = strsep(&str, ":");
		i++;
	}

	rta_nest_end(rta, nest);
	return 0;
}

static int parse_encap_ip(struct rtattr *rta, size_t len,
			  int *argcp, char ***argvp)
{
	int id_ok = 0, dst_ok = 0, src_ok = 0, tos_ok = 0, ttl_ok = 0;
	int key_ok = 0, csum_ok = 0, seq_ok = 0, opts_ok = 0;
	char **argv = *argvp;
	int argc = *argcp;
	int ret = 0;
	__u16 flags = 0;

	while (argc > 0) {
		if (strcmp(*argv, "id") == 0) {
			__u64 id;

			NEXT_ARG();
			if (id_ok++)
				duparg2("id", *argv);
			if (get_be64(&id, *argv, 0))
				invarg("\"id\" value is invalid\n", *argv);
			ret = rta_addattr64(rta, len, LWTUNNEL_IP_ID, id);
		} else if (strcmp(*argv, "dst") == 0) {
			inet_prefix addr;

			NEXT_ARG();
			if (dst_ok++)
				duparg2("dst", *argv);
			get_addr(&addr, *argv, AF_INET);
			ret = rta_addattr_l(rta, len, LWTUNNEL_IP_DST,
					    &addr.data, addr.bytelen);
		} else if (strcmp(*argv, "src") == 0) {
			inet_prefix addr;

			NEXT_ARG();
			if (src_ok++)
				duparg2("src", *argv);
			get_addr(&addr, *argv, AF_INET);
			ret = rta_addattr_l(rta, len, LWTUNNEL_IP_SRC,
					    &addr.data, addr.bytelen);
		} else if (strcmp(*argv, "tos") == 0) {
			__u32 tos;

			NEXT_ARG();
			if (tos_ok++)
				duparg2("tos", *argv);
			if (rtnl_dsfield_a2n(&tos, *argv))
				invarg("\"tos\" value is invalid\n", *argv);
			ret = rta_addattr8(rta, len, LWTUNNEL_IP_TOS, tos);
		} else if (strcmp(*argv, "ttl") == 0) {
			__u8 ttl;

			NEXT_ARG();
			if (ttl_ok++)
				duparg2("ttl", *argv);
			if (get_u8(&ttl, *argv, 0))
				invarg("\"ttl\" value is invalid\n", *argv);
			ret = rta_addattr8(rta, len, LWTUNNEL_IP_TTL, ttl);
		} else if (strcmp(*argv, "geneve_opts") == 0) {
			struct rtattr *nest;

			if (opts_ok++)
				duparg2("opts", *argv);

			NEXT_ARG();

			nest = rta_nest(rta, len,
					LWTUNNEL_IP_OPTS | NLA_F_NESTED);
			ret = lwtunnel_parse_geneve_opts(*argv, len, rta);
			if (ret)
				invarg("\"geneve_opts\" value is invalid\n",
				       *argv);
			rta_nest_end(rta, nest);
		} else if (strcmp(*argv, "vxlan_opts") == 0) {
			struct rtattr *nest;

			if (opts_ok++)
				duparg2("opts", *argv);

			NEXT_ARG();

			nest = rta_nest(rta, len,
					LWTUNNEL_IP_OPTS | NLA_F_NESTED);
			ret = lwtunnel_parse_vxlan_opts(*argv, len, rta);
			if (ret)
				invarg("\"vxlan_opts\" value is invalid\n",
				       *argv);
			rta_nest_end(rta, nest);
		} else if (strcmp(*argv, "erspan_opts") == 0) {
			struct rtattr *nest;

			if (opts_ok++)
				duparg2("opts", *argv);

			NEXT_ARG();

			nest = rta_nest(rta, len,
					LWTUNNEL_IP_OPTS | NLA_F_NESTED);
			ret = lwtunnel_parse_erspan_opts(*argv, len, rta);
			if (ret)
				invarg("\"erspan_opts\" value is invalid\n",
				       *argv);
			rta_nest_end(rta, nest);
		} else if (strcmp(*argv, "key") == 0) {
			if (key_ok++)
				duparg2("key", *argv);
			flags |= TUNNEL_KEY;
		} else if (strcmp(*argv, "csum") == 0) {
			if (csum_ok++)
				duparg2("csum", *argv);
			flags |= TUNNEL_CSUM;
		} else if (strcmp(*argv, "seq") == 0) {
			if (seq_ok++)
				duparg2("seq", *argv);
			flags |= TUNNEL_SEQ;
		} else {
			break;
		}
		if (ret)
			break;
		argc--; argv++;
	}

	if (flags)
		ret = rta_addattr16(rta, len,  LWTUNNEL_IP_FLAGS, flags);

	/* argv is currently the first unparsed argument,
	 * but the lwt_parse_encap() caller will move to the next,
	 * so step back
	 */
	*argcp = argc + 1;
	*argvp = argv - 1;

	return ret;
}

static int parse_encap_ila(struct rtattr *rta, size_t len,
			   int *argcp, char ***argvp)
{
	__u64 locator;
	int argc = *argcp;
	char **argv = *argvp;
	int ret = 0;

	if (get_addr64(&locator, *argv) < 0) {
		fprintf(stderr, "Bad locator: %s\n", *argv);
		exit(1);
	}

	argc--; argv++;

	if (rta_addattr64(rta, len, ILA_ATTR_LOCATOR, locator))
		return -1;

	while (argc > 0) {
		if (strcmp(*argv, "csum-mode") == 0) {
			int csum_mode;

			NEXT_ARG();

			csum_mode = ila_csum_name2mode(*argv);
			if (csum_mode < 0)
				invarg("\"csum-mode\" value is invalid\n",
				       *argv);

			ret = rta_addattr8(rta, len, ILA_ATTR_CSUM_MODE,
					   (__u8)csum_mode);

			argc--; argv++;
		} else if (strcmp(*argv, "ident-type") == 0) {
			int ident_type;

			NEXT_ARG();

			ident_type = ila_ident_name2type(*argv);
			if (ident_type < 0)
				invarg("\"ident-type\" value is invalid\n",
				       *argv);

			ret = rta_addattr8(rta, len, ILA_ATTR_IDENT_TYPE,
					   (__u8)ident_type);

			argc--; argv++;
		} else if (strcmp(*argv, "hook-type") == 0) {
			int hook_type;

			NEXT_ARG();

			hook_type = ila_hook_name2type(*argv);
			if (hook_type < 0)
				invarg("\"hook-type\" value is invalid\n",
				       *argv);

			ret = rta_addattr8(rta, len, ILA_ATTR_HOOK_TYPE,
					   (__u8)hook_type);

			argc--; argv++;
		} else {
			break;
		}
		if (ret)
			break;
	}

	/* argv is currently the first unparsed argument,
	 * but the lwt_parse_encap() caller will move to the next,
	 * so step back
	 */
	*argcp = argc + 1;
	*argvp = argv - 1;

	return ret;
}

static int parse_encap_ip6(struct rtattr *rta, size_t len,
			   int *argcp, char ***argvp)
{
	int id_ok = 0, dst_ok = 0, src_ok = 0, tos_ok = 0, ttl_ok = 0;
	int key_ok = 0, csum_ok = 0, seq_ok = 0, opts_ok = 0;
	char **argv = *argvp;
	int argc = *argcp;
	int ret = 0;
	__u16 flags = 0;

	while (argc > 0) {
		if (strcmp(*argv, "id") == 0) {
			__u64 id;

			NEXT_ARG();
			if (id_ok++)
				duparg2("id", *argv);
			if (get_be64(&id, *argv, 0))
				invarg("\"id\" value is invalid\n", *argv);
			ret = rta_addattr64(rta, len, LWTUNNEL_IP6_ID, id);
		} else if (strcmp(*argv, "dst") == 0) {
			inet_prefix addr;

			NEXT_ARG();
			if (dst_ok++)
				duparg2("dst", *argv);
			get_addr(&addr, *argv, AF_INET6);
			ret = rta_addattr_l(rta, len, LWTUNNEL_IP6_DST,
					    &addr.data, addr.bytelen);
		} else if (strcmp(*argv, "src") == 0) {
			inet_prefix addr;

			NEXT_ARG();
			if (src_ok++)
				duparg2("src", *argv);
			get_addr(&addr, *argv, AF_INET6);
			ret = rta_addattr_l(rta, len, LWTUNNEL_IP6_SRC,
					    &addr.data, addr.bytelen);
		} else if (strcmp(*argv, "tc") == 0) {
			__u32 tc;

			NEXT_ARG();
			if (tos_ok++)
				duparg2("tc", *argv);
			if (rtnl_dsfield_a2n(&tc, *argv))
				invarg("\"tc\" value is invalid\n", *argv);
			ret = rta_addattr8(rta, len, LWTUNNEL_IP6_TC, tc);
		} else if (strcmp(*argv, "hoplimit") == 0) {
			__u8 hoplimit;

			NEXT_ARG();
			if (ttl_ok++)
				duparg2("hoplimit", *argv);
			if (get_u8(&hoplimit, *argv, 0))
				invarg("\"hoplimit\" value is invalid\n",
				       *argv);
			ret = rta_addattr8(rta, len, LWTUNNEL_IP6_HOPLIMIT,
					   hoplimit);
		} else if (strcmp(*argv, "geneve_opts") == 0) {
			struct rtattr *nest;

			if (opts_ok++)
				duparg2("opts", *argv);

			NEXT_ARG();

			nest = rta_nest(rta, len,
					LWTUNNEL_IP_OPTS | NLA_F_NESTED);
			ret = lwtunnel_parse_geneve_opts(*argv, len, rta);
			if (ret)
				invarg("\"geneve_opts\" value is invalid\n",
				       *argv);
			rta_nest_end(rta, nest);
		} else if (strcmp(*argv, "vxlan_opts") == 0) {
			struct rtattr *nest;

			if (opts_ok++)
				duparg2("opts", *argv);

			NEXT_ARG();

			nest = rta_nest(rta, len,
					LWTUNNEL_IP_OPTS | NLA_F_NESTED);
			ret = lwtunnel_parse_vxlan_opts(*argv, len, rta);
			if (ret)
				invarg("\"vxlan_opts\" value is invalid\n",
				       *argv);
			rta_nest_end(rta, nest);
		} else if (strcmp(*argv, "erspan_opts") == 0) {
			struct rtattr *nest;

			if (opts_ok++)
				duparg2("opts", *argv);

			NEXT_ARG();

			nest = rta_nest(rta, len,
					LWTUNNEL_IP_OPTS | NLA_F_NESTED);
			ret = lwtunnel_parse_erspan_opts(*argv, len, rta);
			if (ret)
				invarg("\"erspan_opts\" value is invalid\n",
				       *argv);
			rta_nest_end(rta, nest);
		} else if (strcmp(*argv, "key") == 0) {
			if (key_ok++)
				duparg2("key", *argv);
			flags |= TUNNEL_KEY;
		} else if (strcmp(*argv, "csum") == 0) {
			if (csum_ok++)
				duparg2("csum", *argv);
			flags |= TUNNEL_CSUM;
		} else if (strcmp(*argv, "seq") == 0) {
			if (seq_ok++)
				duparg2("seq", *argv);
			flags |= TUNNEL_SEQ;
		} else {
			break;
		}
		if (ret)
			break;
		argc--; argv++;
	}

	if (flags)
		ret = rta_addattr16(rta, len,  LWTUNNEL_IP6_FLAGS, flags);

	/* argv is currently the first unparsed argument,
	 * but the lwt_parse_encap() caller will move to the next,
	 * so step back
	 */
	*argcp = argc + 1;
	*argvp = argv - 1;

	return ret;
}

static void lwt_bpf_usage(void)
{
	fprintf(stderr, "Usage: ip route ... encap bpf [ in BPF ] [ out BPF ] [ xmit BPF ] [...]\n");
	fprintf(stderr, "BPF := obj FILE [ section NAME ] [ verbose ]\n");
	exit(-1);
}

static int parse_encap_bpf(struct rtattr *rta, size_t len, int *argcp,
			   char ***argvp)
{
	char **argv = *argvp;
	int argc = *argcp;
	int headroom_set = 0;

	while (argc > 0) {
		if (strcmp(*argv, "in") == 0) {
			NEXT_ARG();
			if (lwt_parse_bpf(rta, len, &argc, &argv, LWT_BPF_IN,
					  BPF_PROG_TYPE_LWT_IN) < 0)
				return -1;
		} else if (strcmp(*argv, "out") == 0) {
			NEXT_ARG();
			if (lwt_parse_bpf(rta, len, &argc, &argv, LWT_BPF_OUT,
					  BPF_PROG_TYPE_LWT_OUT) < 0)
				return -1;
		} else if (strcmp(*argv, "xmit") == 0) {
			NEXT_ARG();
			if (lwt_parse_bpf(rta, len, &argc, &argv, LWT_BPF_XMIT,
					  BPF_PROG_TYPE_LWT_XMIT) < 0)
				return -1;
		} else if (strcmp(*argv, "headroom") == 0) {
			unsigned int headroom;

			NEXT_ARG();
			if (get_unsigned(&headroom, *argv, 0) || headroom == 0)
				invarg("headroom is invalid\n", *argv);
			if (!headroom_set)
				rta_addattr32(rta, len, LWT_BPF_XMIT_HEADROOM,
					      headroom);
			headroom_set = 1;
		} else if (strcmp(*argv, "help") == 0) {
			lwt_bpf_usage();
		} else {
			break;
		}
		NEXT_ARG_FWD();
	}

	/* argv is currently the first unparsed argument,
	 * but the lwt_parse_encap() caller will move to the next,
	 * so step back
	 */
	*argcp = argc + 1;
	*argvp = argv - 1;

	return 0;
}

int lwt_parse_encap(struct rtattr *rta, size_t len, int *argcp, char ***argvp,
		    int encap_attr, int encap_type_attr)
{
	struct rtattr *nest;
	int argc = *argcp;
	char **argv = *argvp;
	__u16 type;
	int ret = 0;

	NEXT_ARG();
	type = read_encap_type(*argv);
	if (!type)
		invarg("\"encap type\" value is invalid\n", *argv);

	NEXT_ARG();
	if (argc <= 1) {
		fprintf(stderr,
			"Error: unexpected end of line after \"encap\"\n");
		exit(-1);
	}

	nest = rta_nest(rta, len, encap_attr);
	switch (type) {
	case LWTUNNEL_ENCAP_MPLS:
		ret = parse_encap_mpls(rta, len, &argc, &argv);
		break;
	case LWTUNNEL_ENCAP_IP:
		ret = parse_encap_ip(rta, len, &argc, &argv);
		break;
	case LWTUNNEL_ENCAP_ILA:
		ret = parse_encap_ila(rta, len, &argc, &argv);
		break;
	case LWTUNNEL_ENCAP_IP6:
		ret = parse_encap_ip6(rta, len, &argc, &argv);
		break;
	case LWTUNNEL_ENCAP_BPF:
		if (parse_encap_bpf(rta, len, &argc, &argv) < 0)
			exit(-1);
		break;
	case LWTUNNEL_ENCAP_SEG6:
		ret = parse_encap_seg6(rta, len, &argc, &argv);
		break;
	case LWTUNNEL_ENCAP_SEG6_LOCAL:
		ret = parse_encap_seg6local(rta, len, &argc, &argv);
		break;
	case LWTUNNEL_ENCAP_RPL:
		ret = parse_encap_rpl(rta, len, &argc, &argv);
		break;
	default:
		fprintf(stderr, "Error: unsupported encap type\n");
		break;
	}
	if (ret)
		return ret;

	rta_nest_end(rta, nest);

	ret = rta_addattr16(rta, len, encap_type_attr, type);

	*argcp = argc;
	*argvp = argv;

	return ret;
}
