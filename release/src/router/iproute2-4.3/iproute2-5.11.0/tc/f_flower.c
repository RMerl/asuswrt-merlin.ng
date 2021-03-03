/*
 * f_flower.c		Flower Classifier
 *
 *		This program is free software; you can distribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:     Jiri Pirko <jiri@resnulli.us>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <net/if.h>
#include <linux/limits.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/tc_act/tc_vlan.h>
#include <linux/mpls.h>

#include "utils.h"
#include "tc_util.h"
#include "rt_names.h"

enum flower_matching_flags {
	FLOWER_IP_FLAGS,
};

enum flower_endpoint {
	FLOWER_ENDPOINT_SRC,
	FLOWER_ENDPOINT_DST
};

enum flower_icmp_field {
	FLOWER_ICMP_FIELD_TYPE,
	FLOWER_ICMP_FIELD_CODE
};

static void explain(void)
{
	fprintf(stderr,
		"Usage: ... flower	[ MATCH-LIST ] [ verbose ]\n"
		"			[ skip_sw | skip_hw ]\n"
		"			[ action ACTION-SPEC ] [ classid CLASSID ]\n"
		"\n"
		"Where: MATCH-LIST := [ MATCH-LIST ] MATCH\n"
		"       MATCH      := {	indev DEV-NAME |\n"
		"			vlan_id VID |\n"
		"			vlan_prio PRIORITY |\n"
		"			vlan_ethtype [ ipv4 | ipv6 | ETH-TYPE ] |\n"
		"			cvlan_id VID |\n"
		"			cvlan_prio PRIORITY |\n"
		"			cvlan_ethtype [ ipv4 | ipv6 | ETH-TYPE ] |\n"
		"			dst_mac MASKED-LLADDR |\n"
		"			src_mac MASKED-LLADDR |\n"
		"			ip_proto [tcp | udp | sctp | icmp | icmpv6 | IP-PROTO ] |\n"
		"			ip_tos MASKED-IP_TOS |\n"
		"			ip_ttl MASKED-IP_TTL |\n"
		"			mpls LSE-LIST |\n"
		"			mpls_label LABEL |\n"
		"			mpls_tc TC |\n"
		"			mpls_bos BOS |\n"
		"			mpls_ttl TTL |\n"
		"			dst_ip PREFIX |\n"
		"			src_ip PREFIX |\n"
		"			dst_port PORT-NUMBER |\n"
		"			src_port PORT-NUMBER |\n"
		"			tcp_flags MASKED-TCP_FLAGS |\n"
		"			type MASKED-ICMP-TYPE |\n"
		"			code MASKED-ICMP-CODE |\n"
		"			arp_tip IPV4-PREFIX |\n"
		"			arp_sip IPV4-PREFIX |\n"
		"			arp_op [ request | reply | OP ] |\n"
		"			arp_tha MASKED-LLADDR |\n"
		"			arp_sha MASKED-LLADDR |\n"
		"			enc_dst_ip [ IPV4-ADDR | IPV6-ADDR ] |\n"
		"			enc_src_ip [ IPV4-ADDR | IPV6-ADDR ] |\n"
		"			enc_key_id [ KEY-ID ] |\n"
		"			enc_tos MASKED-IP_TOS |\n"
		"			enc_ttl MASKED-IP_TTL |\n"
		"			geneve_opts MASKED-OPTIONS |\n"
		"			vxlan_opts MASKED-OPTIONS |\n"
		"                       erspan_opts MASKED-OPTIONS |\n"
		"			ip_flags IP-FLAGS | \n"
		"			enc_dst_port [ port_number ] |\n"
		"			ct_state MASKED_CT_STATE |\n"
		"			ct_label MASKED_CT_LABEL |\n"
		"			ct_mark MASKED_CT_MARK |\n"
		"			ct_zone MASKED_CT_ZONE }\n"
		"	LSE-LIST := [ LSE-LIST ] LSE\n"
		"	LSE := lse depth DEPTH { label LABEL | tc TC | bos BOS | ttl TTL }\n"
		"	FILTERID := X:Y:Z\n"
		"	MASKED_LLADDR := { LLADDR | LLADDR/MASK | LLADDR/BITS }\n"
		"	MASKED_CT_STATE := combination of {+|-} and flags trk,est,new\n"
		"	ACTION-SPEC := ... look at individual actions\n"
		"\n"
		"NOTE:	CLASSID, IP-PROTO are parsed as hexadecimal input.\n"
		"NOTE:	There can be only used one mask per one prio. If user needs\n"
		"	to specify different mask, he has to use different prio.\n");
}

static int flower_parse_eth_addr(char *str, int addr_type, int mask_type,
				 struct nlmsghdr *n)
{
	int ret, err = -1;
	char addr[ETH_ALEN], *slash;

	slash = strchr(str, '/');
	if (slash)
		*slash = '\0';

	ret = ll_addr_a2n(addr, sizeof(addr), str);
	if (ret < 0)
		goto err;
	addattr_l(n, MAX_MSG, addr_type, addr, sizeof(addr));

	if (slash) {
		unsigned bits;

		if (!get_unsigned(&bits, slash + 1, 10)) {
			uint64_t mask;

			/* Extra 16 bit shift to push mac address into
			 * high bits of uint64_t
			 */
			mask = htonll(0xffffffffffffULL << (16 + 48 - bits));
			memcpy(addr, &mask, ETH_ALEN);
		} else {
			ret = ll_addr_a2n(addr, sizeof(addr), slash + 1);
			if (ret < 0)
				goto err;
		}
	} else {
		memset(addr, 0xff, ETH_ALEN);
	}
	addattr_l(n, MAX_MSG, mask_type, addr, sizeof(addr));

	err = 0;
err:
	if (slash)
		*slash = '/';
	return err;
}

static bool eth_type_vlan(__be16 ethertype)
{
	return ethertype == htons(ETH_P_8021Q) ||
	       ethertype == htons(ETH_P_8021AD);
}

static int flower_parse_vlan_eth_type(char *str, __be16 eth_type, int type,
				      __be16 *p_vlan_eth_type,
				      struct nlmsghdr *n)
{
	__be16 vlan_eth_type;

	if (!eth_type_vlan(eth_type)) {
		fprintf(stderr, "Can't set \"%s\" if ethertype isn't 802.1Q or 802.1AD\n",
			type == TCA_FLOWER_KEY_VLAN_ETH_TYPE ? "vlan_ethtype" : "cvlan_ethtype");
		return -1;
	}

	if (ll_proto_a2n(&vlan_eth_type, str))
		invarg("invalid vlan_ethtype", str);
	addattr16(n, MAX_MSG, type, vlan_eth_type);
	*p_vlan_eth_type = vlan_eth_type;
	return 0;
}

struct flag_to_string {
	int flag;
	enum flower_matching_flags type;
	char *string;
};

static struct flag_to_string flags_str[] = {
	{ TCA_FLOWER_KEY_FLAGS_IS_FRAGMENT, FLOWER_IP_FLAGS, "frag" },
	{ TCA_FLOWER_KEY_FLAGS_FRAG_IS_FIRST, FLOWER_IP_FLAGS, "firstfrag" },
};

static int flower_parse_matching_flags(char *str,
				       enum flower_matching_flags type,
				       __u32 *mtf, __u32 *mtf_mask)
{
	char *token;
	bool no;
	bool found;
	int i;

	token = strtok(str, "/");

	while (token) {
		if (!strncmp(token, "no", 2)) {
			no = true;
			token += 2;
		} else
			no = false;

		found = false;
		for (i = 0; i < ARRAY_SIZE(flags_str); i++) {
			if (type != flags_str[i].type)
				continue;

			if (!strcmp(token, flags_str[i].string)) {
				if (no)
					*mtf &= ~flags_str[i].flag;
				else
					*mtf |= flags_str[i].flag;

				*mtf_mask |= flags_str[i].flag;
				found = true;
				break;
			}
		}
		if (!found)
			return -1;

		token = strtok(NULL, "/");
	}

	return 0;
}

static int flower_parse_u16(char *str, int value_type, int mask_type,
			    struct nlmsghdr *n, bool be)
{
	__u16 value, mask;
	char *slash;

	slash = strchr(str, '/');
	if (slash)
		*slash = '\0';

	if (get_u16(&value, str, 0))
		return -1;

	if (slash) {
		if (get_u16(&mask, slash + 1, 0))
			return -1;
	} else {
		mask = UINT16_MAX;
	}

	if (be) {
		value = htons(value);
		mask = htons(mask);
	}
	addattr16(n, MAX_MSG, value_type, value);
	addattr16(n, MAX_MSG, mask_type, mask);

	return 0;
}

static int flower_parse_u32(char *str, int value_type, int mask_type,
			    struct nlmsghdr *n)
{
	__u32 value, mask;
	char *slash;

	slash = strchr(str, '/');
	if (slash)
		*slash = '\0';

	if (get_u32(&value, str, 0))
		return -1;

	if (slash) {
		if (get_u32(&mask, slash + 1, 0))
			return -1;
	} else {
		mask = UINT32_MAX;
	}

	addattr32(n, MAX_MSG, value_type, value);
	addattr32(n, MAX_MSG, mask_type, mask);

	return 0;
}

static int flower_parse_ct_mark(char *str, struct nlmsghdr *n)
{
	return flower_parse_u32(str,
				TCA_FLOWER_KEY_CT_MARK,
				TCA_FLOWER_KEY_CT_MARK_MASK,
				n);
}

static int flower_parse_ct_zone(char *str, struct nlmsghdr *n)
{
	return flower_parse_u16(str,
				TCA_FLOWER_KEY_CT_ZONE,
				TCA_FLOWER_KEY_CT_ZONE_MASK,
				n,
				false);
}

static int flower_parse_ct_labels(char *str, struct nlmsghdr *n)
{
#define LABELS_SIZE	16
	uint8_t labels[LABELS_SIZE], lmask[LABELS_SIZE];
	char *slash, *mask = NULL;
	size_t slen, slen_mask = 0;

	slash = index(str, '/');
	if (slash) {
		*slash = 0;
		mask = slash + 1;
		slen_mask = strlen(mask);
	}

	slen = strlen(str);
	if (slen > LABELS_SIZE * 2 || slen_mask > LABELS_SIZE * 2) {
		char errmsg[128];

		snprintf(errmsg, sizeof(errmsg),
				"%zd Max allowed size %d",
				slen, LABELS_SIZE*2);
		invarg(errmsg, str);
	}

	if (hex2mem(str, labels, slen / 2) < 0)
		invarg("labels must be a hex string\n", str);
	addattr_l(n, MAX_MSG, TCA_FLOWER_KEY_CT_LABELS, labels, slen / 2);

	if (mask) {
		if (hex2mem(mask, lmask, slen_mask / 2) < 0)
			invarg("labels mask must be a hex string\n", mask);
	} else {
		memset(lmask, 0xff, sizeof(lmask));
		slen_mask = sizeof(lmask) * 2;
	}
	addattr_l(n, MAX_MSG, TCA_FLOWER_KEY_CT_LABELS_MASK, lmask,
		  slen_mask / 2);

	return 0;
}

static struct flower_ct_states {
	char *str;
	int flag;
} flower_ct_states[] = {
	{ "trk", TCA_FLOWER_KEY_CT_FLAGS_TRACKED },
	{ "new", TCA_FLOWER_KEY_CT_FLAGS_NEW },
	{ "est", TCA_FLOWER_KEY_CT_FLAGS_ESTABLISHED },
	{ "inv", TCA_FLOWER_KEY_CT_FLAGS_INVALID },
	{ "rpl", TCA_FLOWER_KEY_CT_FLAGS_REPLY },
};

static int flower_parse_ct_state(char *str, struct nlmsghdr *n)
{
	int flags = 0, mask = 0,  len, i;
	bool p;

	while (*str != '\0') {
		if (*str == '+')
			p = true;
		else if (*str == '-')
			p = false;
		else
			return -1;

		for (i = 0; i < ARRAY_SIZE(flower_ct_states); i++) {
			len = strlen(flower_ct_states[i].str);
			if (strncmp(str + 1, flower_ct_states[i].str, len))
				continue;

			if (p)
				flags |= flower_ct_states[i].flag;
			mask |= flower_ct_states[i].flag;
			break;
		}

		if (i == ARRAY_SIZE(flower_ct_states))
			return -1;

		str += len + 1;
	}

	addattr16(n, MAX_MSG, TCA_FLOWER_KEY_CT_STATE, flags);
	addattr16(n, MAX_MSG, TCA_FLOWER_KEY_CT_STATE_MASK, mask);
	return 0;
}

static int flower_parse_ip_proto(char *str, __be16 eth_type, int type,
				 __u8 *p_ip_proto, struct nlmsghdr *n)
{
	int ret;
	__u8 ip_proto;

	if (eth_type != htons(ETH_P_IP) && eth_type != htons(ETH_P_IPV6))
		goto err;

	if (matches(str, "tcp") == 0) {
		ip_proto = IPPROTO_TCP;
	} else if (matches(str, "udp") == 0) {
		ip_proto = IPPROTO_UDP;
	} else if (matches(str, "sctp") == 0) {
		ip_proto = IPPROTO_SCTP;
	} else if (matches(str, "icmp") == 0) {
		if (eth_type != htons(ETH_P_IP))
			goto err;
		ip_proto = IPPROTO_ICMP;
	} else if (matches(str, "icmpv6") == 0) {
		if (eth_type != htons(ETH_P_IPV6))
			goto err;
		ip_proto = IPPROTO_ICMPV6;
	} else {
		ret = get_u8(&ip_proto, str, 16);
		if (ret)
			return -1;
	}
	addattr8(n, MAX_MSG, type, ip_proto);
	*p_ip_proto = ip_proto;
	return 0;

err:
	fprintf(stderr, "Illegal \"eth_type\" for ip proto\n");
	return -1;
}

static int __flower_parse_ip_addr(char *str, int family,
				  int addr4_type, int mask4_type,
				  int addr6_type, int mask6_type,
				  struct nlmsghdr *n)
{
	int ret;
	inet_prefix addr;
	int bits;
	int i;

	ret = get_prefix(&addr, str, family);
	if (ret)
		return -1;

	if (family && (addr.family != family)) {
		fprintf(stderr, "Illegal \"eth_type\" for ip address\n");
		return -1;
	}

	addattr_l(n, MAX_MSG, addr.family == AF_INET ? addr4_type : addr6_type,
		  addr.data, addr.bytelen);

	memset(addr.data, 0xff, addr.bytelen);
	bits = addr.bitlen;
	for (i = 0; i < addr.bytelen / 4; i++) {
		if (!bits) {
			addr.data[i] = 0;
		} else if (bits / 32 >= 1) {
			bits -= 32;
		} else {
			addr.data[i] <<= 32 - bits;
			addr.data[i] = htonl(addr.data[i]);
			bits = 0;
		}
	}

	addattr_l(n, MAX_MSG, addr.family == AF_INET ? mask4_type : mask6_type,
		  addr.data, addr.bytelen);

	return 0;
}

static int flower_parse_ip_addr(char *str, __be16 eth_type,
				int addr4_type, int mask4_type,
				int addr6_type, int mask6_type,
				struct nlmsghdr *n)
{
	int family;

	if (eth_type == htons(ETH_P_IP)) {
		family = AF_INET;
	} else if (eth_type == htons(ETH_P_IPV6)) {
		family = AF_INET6;
	} else if (!eth_type) {
		family = AF_UNSPEC;
	} else {
		return -1;
	}

	return __flower_parse_ip_addr(str, family, addr4_type, mask4_type,
				      addr6_type, mask6_type, n);
}

static bool flower_eth_type_arp(__be16 eth_type)
{
	return eth_type == htons(ETH_P_ARP) || eth_type == htons(ETH_P_RARP);
}

static int flower_parse_arp_ip_addr(char *str, __be16 eth_type,
				    int addr_type, int mask_type,
				    struct nlmsghdr *n)
{
	if (!flower_eth_type_arp(eth_type))
		return -1;

	return __flower_parse_ip_addr(str, AF_INET, addr_type, mask_type,
				      TCA_FLOWER_UNSPEC, TCA_FLOWER_UNSPEC, n);
}

static int flower_parse_u8(char *str, int value_type, int mask_type,
			   int (*value_from_name)(const char *str,
						 __u8 *value),
			   bool (*value_validate)(__u8 value),
			   struct nlmsghdr *n)
{
	char *slash;
	int ret, err = -1;
	__u8 value, mask;

	slash = strchr(str, '/');
	if (slash)
		*slash = '\0';

	ret = value_from_name ? value_from_name(str, &value) : -1;
	if (ret < 0) {
		ret = get_u8(&value, str, 10);
		if (ret)
			goto err;
	}

	if (value_validate && !value_validate(value))
		goto err;

	if (slash) {
		ret = get_u8(&mask, slash + 1, 10);
		if (ret)
			goto err;
	}
	else {
		mask = UINT8_MAX;
	}

	addattr8(n, MAX_MSG, value_type, value);
	addattr8(n, MAX_MSG, mask_type, mask);

	err = 0;
err:
	if (slash)
		*slash = '/';
	return err;
}

static const char *flower_print_arp_op_to_name(__u8 op)
{
	switch (op) {
	case ARPOP_REQUEST:
		return "request";
	case ARPOP_REPLY:
		return "reply";
	default:
		return NULL;
	}
}

static int flower_arp_op_from_name(const char *name, __u8 *op)
{
	if (!strcmp(name, "request"))
		*op = ARPOP_REQUEST;
	else if (!strcmp(name, "reply"))
		*op = ARPOP_REPLY;
	else
		return -1;

	return 0;
}

static bool flow_arp_op_validate(__u8 op)
{
	return !op || op == ARPOP_REQUEST || op == ARPOP_REPLY;
}

static int flower_parse_arp_op(char *str, __be16 eth_type,
			       int op_type, int mask_type,
			       struct nlmsghdr *n)
{
	if (!flower_eth_type_arp(eth_type))
		return -1;

	return flower_parse_u8(str, op_type, mask_type, flower_arp_op_from_name,
			       flow_arp_op_validate, n);
}

static int flower_icmp_attr_type(__be16 eth_type, __u8 ip_proto,
				 enum flower_icmp_field field)
{
	if (eth_type == htons(ETH_P_IP) && ip_proto == IPPROTO_ICMP)
		return field == FLOWER_ICMP_FIELD_CODE ?
			TCA_FLOWER_KEY_ICMPV4_CODE :
			TCA_FLOWER_KEY_ICMPV4_TYPE;
	else if (eth_type == htons(ETH_P_IPV6) && ip_proto == IPPROTO_ICMPV6)
		return field == FLOWER_ICMP_FIELD_CODE ?
			TCA_FLOWER_KEY_ICMPV6_CODE :
			TCA_FLOWER_KEY_ICMPV6_TYPE;

	return -1;
}

static int flower_icmp_attr_mask_type(__be16 eth_type, __u8 ip_proto,
				      enum flower_icmp_field field)
{
	if (eth_type == htons(ETH_P_IP) && ip_proto == IPPROTO_ICMP)
		return field == FLOWER_ICMP_FIELD_CODE ?
			TCA_FLOWER_KEY_ICMPV4_CODE_MASK :
			TCA_FLOWER_KEY_ICMPV4_TYPE_MASK;
	else if (eth_type == htons(ETH_P_IPV6) && ip_proto == IPPROTO_ICMPV6)
		return field == FLOWER_ICMP_FIELD_CODE ?
			TCA_FLOWER_KEY_ICMPV6_CODE_MASK :
			TCA_FLOWER_KEY_ICMPV6_TYPE_MASK;

	return -1;
}

static int flower_parse_icmp(char *str, __u16 eth_type, __u8 ip_proto,
			     enum flower_icmp_field field, struct nlmsghdr *n)
{
	int value_type, mask_type;

	value_type = flower_icmp_attr_type(eth_type, ip_proto, field);
	mask_type = flower_icmp_attr_mask_type(eth_type, ip_proto, field);
	if (value_type < 0 || mask_type < 0)
		return -1;

	return flower_parse_u8(str, value_type, mask_type, NULL, NULL, n);
}

static int flower_port_attr_type(__u8 ip_proto, enum flower_endpoint endpoint)
{
	if (ip_proto == IPPROTO_TCP)
		return endpoint == FLOWER_ENDPOINT_SRC ?
			TCA_FLOWER_KEY_TCP_SRC :
			TCA_FLOWER_KEY_TCP_DST;
	else if (ip_proto == IPPROTO_UDP)
		return endpoint == FLOWER_ENDPOINT_SRC ?
			TCA_FLOWER_KEY_UDP_SRC :
			TCA_FLOWER_KEY_UDP_DST;
	else if (ip_proto == IPPROTO_SCTP)
		return endpoint == FLOWER_ENDPOINT_SRC ?
			TCA_FLOWER_KEY_SCTP_SRC :
			TCA_FLOWER_KEY_SCTP_DST;
	else
		return -1;
}

static int flower_port_attr_mask_type(__u8 ip_proto,
				      enum flower_endpoint endpoint)
{
	switch (ip_proto) {
	case IPPROTO_TCP:
		return endpoint == FLOWER_ENDPOINT_SRC ?
			TCA_FLOWER_KEY_TCP_SRC_MASK :
			TCA_FLOWER_KEY_TCP_DST_MASK;
	case IPPROTO_UDP:
		return endpoint == FLOWER_ENDPOINT_SRC ?
			TCA_FLOWER_KEY_UDP_SRC_MASK :
			TCA_FLOWER_KEY_UDP_DST_MASK;
	case IPPROTO_SCTP:
		return endpoint == FLOWER_ENDPOINT_SRC ?
			TCA_FLOWER_KEY_SCTP_SRC_MASK :
			TCA_FLOWER_KEY_SCTP_DST_MASK;
	default:
		return -1;
	}
}

static int flower_port_range_attr_type(__u8 ip_proto, enum flower_endpoint type,
				       __be16 *min_port_type,
				       __be16 *max_port_type)
{
	if (ip_proto == IPPROTO_TCP || ip_proto == IPPROTO_UDP ||
	    ip_proto == IPPROTO_SCTP) {
		if (type == FLOWER_ENDPOINT_SRC) {
			*min_port_type = TCA_FLOWER_KEY_PORT_SRC_MIN;
			*max_port_type = TCA_FLOWER_KEY_PORT_SRC_MAX;
		} else {
			*min_port_type = TCA_FLOWER_KEY_PORT_DST_MIN;
			*max_port_type = TCA_FLOWER_KEY_PORT_DST_MAX;
		}
	} else {
		return -1;
	}
	return 0;
}

/* parse range args in format 10-20 */
static int parse_range(char *str, __be16 *min, __be16 *max)
{
	char *sep;

	sep = strchr(str, '-');
	if (sep) {
		*sep = '\0';

		if (get_be16(min, str, 10))
			return -1;

		if (get_be16(max, sep + 1, 10))
			return -1;
	} else {
		if (get_be16(min, str, 10))
			return -1;
	}
	return 0;
}

static int flower_parse_port(char *str, __u8 ip_proto,
			     enum flower_endpoint endpoint,
			     struct nlmsghdr *n)
{
	char *slash = NULL;
	__be16 min = 0;
	__be16 max = 0;
	int ret;

	ret = parse_range(str, &min, &max);
	if (ret) {
		slash = strchr(str, '/');
		if (!slash)
			return -1;
	}

	if (min && max) {
		__be16 min_port_type, max_port_type;

		if (max <= min) {
			fprintf(stderr, "max value should be greater than min value\n");
			return -1;
		}
		if (flower_port_range_attr_type(ip_proto, endpoint,
						&min_port_type, &max_port_type))
			return -1;

		addattr16(n, MAX_MSG, min_port_type, min);
		addattr16(n, MAX_MSG, max_port_type, max);
	} else if (slash || (min && !max)) {
		int type;

		type = flower_port_attr_type(ip_proto, endpoint);
		if (type < 0)
			return -1;

		if (!slash) {
			addattr16(n, MAX_MSG, type, min);
		} else {
			int mask_type;

			mask_type = flower_port_attr_mask_type(ip_proto,
							       endpoint);
			if (mask_type < 0)
				return -1;
			return flower_parse_u16(str, type, mask_type, n, true);
		}
	} else {
		return -1;
	}
	return 0;
}

#define TCP_FLAGS_MAX_MASK 0xfff

static int flower_parse_tcp_flags(char *str, int flags_type, int mask_type,
				  struct nlmsghdr *n)
{
	char *slash;
	int ret, err = -1;
	__u16 flags;

	slash = strchr(str, '/');
	if (slash)
		*slash = '\0';

	ret = get_u16(&flags, str, 16);
	if (ret < 0 || flags & ~TCP_FLAGS_MAX_MASK)
		goto err;

	addattr16(n, MAX_MSG, flags_type, htons(flags));

	if (slash) {
		ret = get_u16(&flags, slash + 1, 16);
		if (ret < 0 || flags & ~TCP_FLAGS_MAX_MASK)
			goto err;
	} else {
		flags = TCP_FLAGS_MAX_MASK;
	}
	addattr16(n, MAX_MSG, mask_type, htons(flags));

	err = 0;
err:
	if (slash)
		*slash = '/';
	return err;
}

static int flower_parse_ip_tos_ttl(char *str, int key_type, int mask_type,
				   struct nlmsghdr *n)
{
	char *slash;
	int ret, err = -1;
	__u8 tos_ttl;

	slash = strchr(str, '/');
	if (slash)
		*slash = '\0';

	ret = get_u8(&tos_ttl, str, 10);
	if (ret < 0)
		ret = get_u8(&tos_ttl, str, 16);
	if (ret < 0)
		goto err;

	addattr8(n, MAX_MSG, key_type, tos_ttl);

	if (slash) {
		ret = get_u8(&tos_ttl, slash + 1, 16);
		if (ret < 0)
			goto err;
	} else {
		tos_ttl = 0xff;
	}
	addattr8(n, MAX_MSG, mask_type, tos_ttl);

	err = 0;
err:
	if (slash)
		*slash = '/';
	return err;
}

static int flower_parse_key_id(const char *str, int type, struct nlmsghdr *n)
{
	int ret;
	__be32 key_id;

	ret = get_be32(&key_id, str, 10);
	if (!ret)
		addattr32(n, MAX_MSG, type, key_id);

	return ret;
}

static int flower_parse_enc_port(char *str, int type, struct nlmsghdr *n)
{
	int ret;
	__be16 port;

	ret = get_be16(&port, str, 10);
	if (ret)
		return -1;

	addattr16(n, MAX_MSG, type, port);

	return 0;
}

static int flower_parse_geneve_opt(char *str, struct nlmsghdr *n)
{
	struct rtattr *nest;
	char *token;
	int i, err;

	nest = addattr_nest(n, MAX_MSG, TCA_FLOWER_KEY_ENC_OPTS_GENEVE);

	i = 1;
	token = strsep(&str, ":");
	while (token) {
		switch (i) {
		case TCA_FLOWER_KEY_ENC_OPT_GENEVE_CLASS:
		{
			__be16 opt_class;

			if (!strlen(token))
				break;
			err = get_be16(&opt_class, token, 16);
			if (err)
				return err;

			addattr16(n, MAX_MSG, i, opt_class);
			break;
		}
		case TCA_FLOWER_KEY_ENC_OPT_GENEVE_TYPE:
		{
			__u8 opt_type;

			if (!strlen(token))
				break;
			err = get_u8(&opt_type, token, 16);
			if (err)
				return err;

			addattr8(n, MAX_MSG, i, opt_type);
			break;
		}
		case TCA_FLOWER_KEY_ENC_OPT_GENEVE_DATA:
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
			addattr_l(n, MAX_MSG, i, opts, token_len / 2);
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
	addattr_nest_end(n, nest);

	return 0;
}

static int flower_parse_vxlan_opt(char *str, struct nlmsghdr *n)
{
	struct rtattr *nest;
	__u32 gbp;
	int err;

	nest = addattr_nest(n, MAX_MSG,
			    TCA_FLOWER_KEY_ENC_OPTS_VXLAN | NLA_F_NESTED);

	err = get_u32(&gbp, str, 0);
	if (err)
		return err;
	addattr32(n, MAX_MSG, TCA_FLOWER_KEY_ENC_OPT_VXLAN_GBP, gbp);

	addattr_nest_end(n, nest);

	return 0;
}

static int flower_parse_erspan_opt(char *str, struct nlmsghdr *n)
{
	struct rtattr *nest;
	char *token;
	int i, err;

	nest = addattr_nest(n, MAX_MSG,
			    TCA_FLOWER_KEY_ENC_OPTS_ERSPAN | NLA_F_NESTED);

	i = 1;
	token = strsep(&str, ":");
	while (token) {
		switch (i) {
		case TCA_FLOWER_KEY_ENC_OPT_ERSPAN_VER:
		{
			__u8 opt_type;

			if (!strlen(token))
				break;
			err = get_u8(&opt_type, token, 0);
			if (err)
				return err;

			addattr8(n, MAX_MSG, i, opt_type);
			break;
		}
		case TCA_FLOWER_KEY_ENC_OPT_ERSPAN_INDEX:
		{
			__be32 opt_index;

			if (!strlen(token))
				break;
			err = get_be32(&opt_index, token, 0);
			if (err)
				return err;

			addattr32(n, MAX_MSG, i, opt_index);
			break;
		}
		case TCA_FLOWER_KEY_ENC_OPT_ERSPAN_DIR:
		{
			__u8 opt_type;

			if (!strlen(token))
				break;
			err = get_u8(&opt_type, token, 0);
			if (err)
				return err;

			addattr8(n, MAX_MSG, i, opt_type);
			break;
		}
		case TCA_FLOWER_KEY_ENC_OPT_ERSPAN_HWID:
		{
			__u8 opt_type;

			if (!strlen(token))
				break;
			err = get_u8(&opt_type, token, 0);
			if (err)
				return err;

			addattr8(n, MAX_MSG, i, opt_type);
			break;
		}
		default:
			fprintf(stderr, "Unknown \"geneve_opts\" type\n");
			return -1;
		}

		token = strsep(&str, ":");
		i++;
	}
	addattr_nest_end(n, nest);

	return 0;
}

static int flower_parse_geneve_opts(char *str, struct nlmsghdr *n)
{
	char *token;
	int err;

	token = strsep(&str, ",");
	while (token) {
		err = flower_parse_geneve_opt(token, n);
		if (err)
			return err;

		token = strsep(&str, ",");
	}

	return 0;
}

static int flower_check_enc_opt_key(char *key)
{
	int key_len, col_cnt = 0;

	key_len = strlen(key);
	while ((key = strchr(key, ':'))) {
		if (strlen(key) == key_len)
			return -1;

		key_len = strlen(key) - 1;
		col_cnt++;
		key++;
	}

	if (col_cnt != 2 || !key_len)
		return -1;

	return 0;
}

static int flower_parse_enc_opts_geneve(char *str, struct nlmsghdr *n)
{
	char key[XATTR_SIZE_MAX], mask[XATTR_SIZE_MAX];
	int data_len, key_len, mask_len, err;
	char *token, *slash;
	struct rtattr *nest;

	key_len = 0;
	mask_len = 0;
	token = strsep(&str, ",");
	while (token) {
		slash = strchr(token, '/');
		if (slash)
			*slash = '\0';

		if ((key_len + strlen(token) > XATTR_SIZE_MAX) ||
		    flower_check_enc_opt_key(token))
			return -1;

		strcpy(&key[key_len], token);
		key_len += strlen(token) + 1;
		key[key_len - 1] = ',';

		if (!slash) {
			/* Pad out mask when not provided */
			if (mask_len + strlen(token) > XATTR_SIZE_MAX)
				return -1;

			data_len = strlen(rindex(token, ':'));
			sprintf(&mask[mask_len], "ffff:ff:");
			mask_len += 8;
			memset(&mask[mask_len], 'f', data_len - 1);
			mask_len += data_len;
			mask[mask_len - 1] = ',';
			token = strsep(&str, ",");
			continue;
		}

		if (mask_len + strlen(slash + 1) > XATTR_SIZE_MAX)
			return -1;

		strcpy(&mask[mask_len], slash + 1);
		mask_len += strlen(slash + 1) + 1;
		mask[mask_len - 1] = ',';

		*slash = '/';
		token = strsep(&str, ",");
	}
	key[key_len - 1] = '\0';
	mask[mask_len - 1] = '\0';

	nest = addattr_nest(n, MAX_MSG, TCA_FLOWER_KEY_ENC_OPTS);
	err = flower_parse_geneve_opts(key, n);
	if (err)
		return err;
	addattr_nest_end(n, nest);

	nest = addattr_nest(n, MAX_MSG, TCA_FLOWER_KEY_ENC_OPTS_MASK);
	err = flower_parse_geneve_opts(mask, n);
	if (err)
		return err;
	addattr_nest_end(n, nest);

	return 0;
}

static int flower_parse_enc_opts_vxlan(char *str, struct nlmsghdr *n)
{
	char key[XATTR_SIZE_MAX], mask[XATTR_SIZE_MAX];
	struct rtattr *nest;
	char *slash;
	int err;

	slash = strchr(str, '/');
	if (slash) {
		*slash++ = '\0';
		if (strlen(slash) > XATTR_SIZE_MAX)
			return -1;
		strcpy(mask, slash);
	} else {
		strcpy(mask, "0xffffffff");
	}

	if (strlen(str) > XATTR_SIZE_MAX)
		return -1;
	strcpy(key, str);

	nest = addattr_nest(n, MAX_MSG, TCA_FLOWER_KEY_ENC_OPTS | NLA_F_NESTED);
	err = flower_parse_vxlan_opt(str, n);
	if (err)
		return err;
	addattr_nest_end(n, nest);

	nest = addattr_nest(n, MAX_MSG,
			    TCA_FLOWER_KEY_ENC_OPTS_MASK | NLA_F_NESTED);
	err = flower_parse_vxlan_opt(mask, n);
	if (err)
		return err;
	addattr_nest_end(n, nest);

	return 0;
}

static int flower_parse_enc_opts_erspan(char *str, struct nlmsghdr *n)
{
	char key[XATTR_SIZE_MAX], mask[XATTR_SIZE_MAX];
	struct rtattr *nest;
	char *slash;
	int err;


	slash = strchr(str, '/');
	if (slash) {
		*slash++ = '\0';
		if (strlen(slash) > XATTR_SIZE_MAX)
			return -1;
		strcpy(mask, slash);
	} else {
		int index;

		slash = strchr(str, ':');
		index = (int)(slash - str);
		memcpy(mask, str, index);
		strcpy(mask + index, ":0xffffffff:0xff:0xff");
	}

	if (strlen(str) > XATTR_SIZE_MAX)
		return -1;
	strcpy(key, str);

	nest = addattr_nest(n, MAX_MSG, TCA_FLOWER_KEY_ENC_OPTS | NLA_F_NESTED);
	err = flower_parse_erspan_opt(key, n);
	if (err)
		return err;
	addattr_nest_end(n, nest);

	nest = addattr_nest(n, MAX_MSG,
			    TCA_FLOWER_KEY_ENC_OPTS_MASK | NLA_F_NESTED);
	err = flower_parse_erspan_opt(mask, n);
	if (err)
		return err;
	addattr_nest_end(n, nest);

	return 0;
}

static int flower_parse_mpls_lse(int *argc_p, char ***argv_p,
				 struct nlmsghdr *nlh)
{
	struct rtattr *lse_attr;
	char **argv = *argv_p;
	int argc = *argc_p;
	__u8 depth = 0;
	int ret;

	lse_attr = addattr_nest(nlh, MAX_MSG,
				TCA_FLOWER_KEY_MPLS_OPTS_LSE | NLA_F_NESTED);

	while (argc > 0) {
		if (matches(*argv, "depth") == 0) {
			NEXT_ARG();
			ret = get_u8(&depth, *argv, 10);
			if (ret < 0 || depth < 1) {
				fprintf(stderr, "Illegal \"depth\"\n");
				return -1;
			}
			addattr8(nlh, MAX_MSG,
				 TCA_FLOWER_KEY_MPLS_OPT_LSE_DEPTH, depth);
		} else if (matches(*argv, "label") == 0) {
			__u32 label;

			NEXT_ARG();
			ret = get_u32(&label, *argv, 10);
			if (ret < 0 ||
			    label & ~(MPLS_LS_LABEL_MASK >> MPLS_LS_LABEL_SHIFT)) {
				fprintf(stderr, "Illegal \"label\"\n");
				return -1;
			}
			addattr32(nlh, MAX_MSG,
				  TCA_FLOWER_KEY_MPLS_OPT_LSE_LABEL, label);
		} else if (matches(*argv, "tc") == 0) {
			__u8 tc;

			NEXT_ARG();
			ret = get_u8(&tc, *argv, 10);
			if (ret < 0 ||
			    tc & ~(MPLS_LS_TC_MASK >> MPLS_LS_TC_SHIFT)) {
				fprintf(stderr, "Illegal \"tc\"\n");
				return -1;
			}
			addattr8(nlh, MAX_MSG, TCA_FLOWER_KEY_MPLS_OPT_LSE_TC,
				 tc);
		} else if (matches(*argv, "bos") == 0) {
			__u8 bos;

			NEXT_ARG();
			ret = get_u8(&bos, *argv, 10);
			if (ret < 0 || bos & ~(MPLS_LS_S_MASK >> MPLS_LS_S_SHIFT)) {
				fprintf(stderr, "Illegal \"bos\"\n");
				return -1;
			}
			addattr8(nlh, MAX_MSG, TCA_FLOWER_KEY_MPLS_OPT_LSE_BOS,
				 bos);
		} else if (matches(*argv, "ttl") == 0) {
			__u8 ttl;

			NEXT_ARG();
			ret = get_u8(&ttl, *argv, 10);
			if (ret < 0 || ttl & ~(MPLS_LS_TTL_MASK >> MPLS_LS_TTL_SHIFT)) {
				fprintf(stderr, "Illegal \"ttl\"\n");
				return -1;
			}
			addattr8(nlh, MAX_MSG, TCA_FLOWER_KEY_MPLS_OPT_LSE_TTL,
				 ttl);
		} else {
			break;
		}
		argc--; argv++;
	}

	if (!depth) {
		missarg("depth");
		return -1;
	}

	addattr_nest_end(nlh, lse_attr);

	*argc_p = argc;
	*argv_p = argv;

	return 0;
}

static int flower_parse_mpls(int *argc_p, char ***argv_p, struct nlmsghdr *nlh)
{
	struct rtattr *mpls_attr;
	char **argv = *argv_p;
	int argc = *argc_p;

	mpls_attr = addattr_nest(nlh, MAX_MSG,
				 TCA_FLOWER_KEY_MPLS_OPTS | NLA_F_NESTED);

	while (argc > 0) {
		if (matches(*argv, "lse") == 0) {
			NEXT_ARG();
			if (flower_parse_mpls_lse(&argc, &argv, nlh) < 0)
				return -1;
		} else {
			break;
		}
	}

	addattr_nest_end(nlh, mpls_attr);

	*argc_p = argc;
	*argv_p = argv;

	return 0;
}

static int flower_parse_opt(struct filter_util *qu, char *handle,
			    int argc, char **argv, struct nlmsghdr *n)
{
	int ret;
	struct tcmsg *t = NLMSG_DATA(n);
	bool mpls_format_old = false;
	bool mpls_format_new = false;
	struct rtattr *tail;
	__be16 tc_proto = TC_H_MIN(t->tcm_info);
	__be16 eth_type = tc_proto;
	__be16 vlan_ethtype = 0;
	__u8 ip_proto = 0xff;
	__u32 flags = 0;
	__u32 mtf = 0;
	__u32 mtf_mask = 0;

	if (handle) {
		ret = get_u32(&t->tcm_handle, handle, 0);
		if (ret) {
			fprintf(stderr, "Illegal \"handle\"\n");
			return -1;
		}
	}

	tail = (struct rtattr *) (((void *) n) + NLMSG_ALIGN(n->nlmsg_len));
	addattr_l(n, MAX_MSG, TCA_OPTIONS, NULL, 0);

	if (argc == 0) {
		/*at minimal we will match all ethertype packets */
		goto parse_done;
	}

	while (argc > 0) {
		if (matches(*argv, "classid") == 0 ||
		    matches(*argv, "flowid") == 0) {
			unsigned int handle;

			NEXT_ARG();
			ret = get_tc_classid(&handle, *argv);
			if (ret) {
				fprintf(stderr, "Illegal \"classid\"\n");
				return -1;
			}
			addattr_l(n, MAX_MSG, TCA_FLOWER_CLASSID, &handle, 4);
		} else if (matches(*argv, "hw_tc") == 0) {
			unsigned int handle;
			__u32 tc;
			char *end;

			NEXT_ARG();
			tc = strtoul(*argv, &end, 0);
			if (*end) {
				fprintf(stderr, "Illegal TC index\n");
				return -1;
			}
			if (tc >= TC_QOPT_MAX_QUEUE) {
				fprintf(stderr, "TC index exceeds max range\n");
				return -1;
			}
			handle = TC_H_MAKE(TC_H_MAJ(t->tcm_parent),
					   TC_H_MIN(tc + TC_H_MIN_PRIORITY));
			addattr_l(n, MAX_MSG, TCA_FLOWER_CLASSID, &handle,
				  sizeof(handle));
		} else if (matches(*argv, "ip_flags") == 0) {
			NEXT_ARG();
			ret = flower_parse_matching_flags(*argv,
							  FLOWER_IP_FLAGS,
							  &mtf,
							  &mtf_mask);
			if (ret < 0) {
				fprintf(stderr, "Illegal \"ip_flags\"\n");
				return -1;
			}
		} else if (matches(*argv, "verbose") == 0) {
			flags |= TCA_CLS_FLAGS_VERBOSE;
		} else if (matches(*argv, "skip_hw") == 0) {
			flags |= TCA_CLS_FLAGS_SKIP_HW;
		} else if (matches(*argv, "skip_sw") == 0) {
			flags |= TCA_CLS_FLAGS_SKIP_SW;
		} else if (matches(*argv, "ct_state") == 0) {
			NEXT_ARG();
			ret = flower_parse_ct_state(*argv, n);
			if (ret < 0) {
				fprintf(stderr, "Illegal \"ct_state\"\n");
				return -1;
			}
		} else if (matches(*argv, "ct_zone") == 0) {
			NEXT_ARG();
			ret = flower_parse_ct_zone(*argv, n);
			if (ret < 0) {
				fprintf(stderr, "Illegal \"ct_zone\"\n");
				return -1;
			}
		} else if (matches(*argv, "ct_mark") == 0) {
			NEXT_ARG();
			ret = flower_parse_ct_mark(*argv, n);
			if (ret < 0) {
				fprintf(stderr, "Illegal \"ct_mark\"\n");
				return -1;
			}
		} else if (matches(*argv, "ct_label") == 0) {
			NEXT_ARG();
			ret = flower_parse_ct_labels(*argv, n);
			if (ret < 0) {
				fprintf(stderr, "Illegal \"ct_label\"\n");
				return -1;
			}
		} else if (matches(*argv, "indev") == 0) {
			NEXT_ARG();
			if (check_ifname(*argv))
				invarg("\"indev\" not a valid ifname", *argv);
			addattrstrz(n, MAX_MSG, TCA_FLOWER_INDEV, *argv);
		} else if (matches(*argv, "vlan_id") == 0) {
			__u16 vid;

			NEXT_ARG();
			if (!eth_type_vlan(tc_proto)) {
				fprintf(stderr, "Can't set \"vlan_id\" if ethertype isn't 802.1Q or 802.1AD\n");
				return -1;
			}
			ret = get_u16(&vid, *argv, 10);
			if (ret < 0 || vid & ~0xfff) {
				fprintf(stderr, "Illegal \"vlan_id\"\n");
				return -1;
			}
			addattr16(n, MAX_MSG, TCA_FLOWER_KEY_VLAN_ID, vid);
		} else if (matches(*argv, "vlan_prio") == 0) {
			__u8 vlan_prio;

			NEXT_ARG();
			if (!eth_type_vlan(tc_proto)) {
				fprintf(stderr, "Can't set \"vlan_prio\" if ethertype isn't 802.1Q or 802.1AD\n");
				return -1;
			}
			ret = get_u8(&vlan_prio, *argv, 10);
			if (ret < 0 || vlan_prio & ~0x7) {
				fprintf(stderr, "Illegal \"vlan_prio\"\n");
				return -1;
			}
			addattr8(n, MAX_MSG,
				 TCA_FLOWER_KEY_VLAN_PRIO, vlan_prio);
		} else if (matches(*argv, "vlan_ethtype") == 0) {
			NEXT_ARG();
			ret = flower_parse_vlan_eth_type(*argv, eth_type,
						 TCA_FLOWER_KEY_VLAN_ETH_TYPE,
						 &vlan_ethtype, n);
			if (ret < 0)
				return -1;
			/* get new ethtype for later parsing  */
			eth_type = vlan_ethtype;
		} else if (matches(*argv, "cvlan_id") == 0) {
			__u16 vid;

			NEXT_ARG();
			if (!eth_type_vlan(vlan_ethtype)) {
				fprintf(stderr, "Can't set \"cvlan_id\" if inner vlan ethertype isn't 802.1Q or 802.1AD\n");
				return -1;
			}
			ret = get_u16(&vid, *argv, 10);
			if (ret < 0 || vid & ~0xfff) {
				fprintf(stderr, "Illegal \"cvlan_id\"\n");
				return -1;
			}
			addattr16(n, MAX_MSG, TCA_FLOWER_KEY_CVLAN_ID, vid);
		} else if (matches(*argv, "cvlan_prio") == 0) {
			__u8 cvlan_prio;

			NEXT_ARG();
			if (!eth_type_vlan(vlan_ethtype)) {
				fprintf(stderr, "Can't set \"cvlan_prio\" if inner vlan ethertype isn't 802.1Q or 802.1AD\n");
				return -1;
			}
			ret = get_u8(&cvlan_prio, *argv, 10);
			if (ret < 0 || cvlan_prio & ~0x7) {
				fprintf(stderr, "Illegal \"cvlan_prio\"\n");
				return -1;
			}
			addattr8(n, MAX_MSG,
				 TCA_FLOWER_KEY_CVLAN_PRIO, cvlan_prio);
		} else if (matches(*argv, "cvlan_ethtype") == 0) {
			NEXT_ARG();
			/* get new ethtype for later parsing */
			ret = flower_parse_vlan_eth_type(*argv, vlan_ethtype,
						 TCA_FLOWER_KEY_CVLAN_ETH_TYPE,
						 &eth_type, n);
			if (ret < 0)
				return -1;
		} else if (matches(*argv, "mpls") == 0) {
			NEXT_ARG();
			if (eth_type != htons(ETH_P_MPLS_UC) &&
			    eth_type != htons(ETH_P_MPLS_MC)) {
				fprintf(stderr,
					"Can't set \"mpls\" if ethertype isn't MPLS\n");
				return -1;
			}
			if (mpls_format_old) {
				fprintf(stderr,
					"Can't set \"mpls\" if \"mpls_label\", \"mpls_tc\", \"mpls_bos\" or \"mpls_ttl\" is set\n");
				return -1;
			}
			mpls_format_new = true;
			if (flower_parse_mpls(&argc, &argv, n) < 0)
				return -1;
			continue;
		} else if (matches(*argv, "mpls_label") == 0) {
			__u32 label;

			NEXT_ARG();
			if (eth_type != htons(ETH_P_MPLS_UC) &&
			    eth_type != htons(ETH_P_MPLS_MC)) {
				fprintf(stderr,
					"Can't set \"mpls_label\" if ethertype isn't MPLS\n");
				return -1;
			}
			if (mpls_format_new) {
				fprintf(stderr,
					"Can't set \"mpls_label\" if \"mpls\" is set\n");
				return -1;
			}
			mpls_format_old = true;
			ret = get_u32(&label, *argv, 10);
			if (ret < 0 || label & ~(MPLS_LS_LABEL_MASK >> MPLS_LS_LABEL_SHIFT)) {
				fprintf(stderr, "Illegal \"mpls_label\"\n");
				return -1;
			}
			addattr32(n, MAX_MSG, TCA_FLOWER_KEY_MPLS_LABEL, label);
		} else if (matches(*argv, "mpls_tc") == 0) {
			__u8 tc;

			NEXT_ARG();
			if (eth_type != htons(ETH_P_MPLS_UC) &&
			    eth_type != htons(ETH_P_MPLS_MC)) {
				fprintf(stderr,
					"Can't set \"mpls_tc\" if ethertype isn't MPLS\n");
				return -1;
			}
			if (mpls_format_new) {
				fprintf(stderr,
					"Can't set \"mpls_tc\" if \"mpls\" is set\n");
				return -1;
			}
			mpls_format_old = true;
			ret = get_u8(&tc, *argv, 10);
			if (ret < 0 || tc & ~(MPLS_LS_TC_MASK >> MPLS_LS_TC_SHIFT)) {
				fprintf(stderr, "Illegal \"mpls_tc\"\n");
				return -1;
			}
			addattr8(n, MAX_MSG, TCA_FLOWER_KEY_MPLS_TC, tc);
		} else if (matches(*argv, "mpls_bos") == 0) {
			__u8 bos;

			NEXT_ARG();
			if (eth_type != htons(ETH_P_MPLS_UC) &&
			    eth_type != htons(ETH_P_MPLS_MC)) {
				fprintf(stderr,
					"Can't set \"mpls_bos\" if ethertype isn't MPLS\n");
				return -1;
			}
			if (mpls_format_new) {
				fprintf(stderr,
					"Can't set \"mpls_bos\" if \"mpls\" is set\n");
				return -1;
			}
			mpls_format_old = true;
			ret = get_u8(&bos, *argv, 10);
			if (ret < 0 || bos & ~(MPLS_LS_S_MASK >> MPLS_LS_S_SHIFT)) {
				fprintf(stderr, "Illegal \"mpls_bos\"\n");
				return -1;
			}
			addattr8(n, MAX_MSG, TCA_FLOWER_KEY_MPLS_BOS, bos);
		} else if (matches(*argv, "mpls_ttl") == 0) {
			__u8 ttl;

			NEXT_ARG();
			if (eth_type != htons(ETH_P_MPLS_UC) &&
			    eth_type != htons(ETH_P_MPLS_MC)) {
				fprintf(stderr,
					"Can't set \"mpls_ttl\" if ethertype isn't MPLS\n");
				return -1;
			}
			if (mpls_format_new) {
				fprintf(stderr,
					"Can't set \"mpls_ttl\" if \"mpls\" is set\n");
				return -1;
			}
			mpls_format_old = true;
			ret = get_u8(&ttl, *argv, 10);
			if (ret < 0 || ttl & ~(MPLS_LS_TTL_MASK >> MPLS_LS_TTL_SHIFT)) {
				fprintf(stderr, "Illegal \"mpls_ttl\"\n");
				return -1;
			}
			addattr8(n, MAX_MSG, TCA_FLOWER_KEY_MPLS_TTL, ttl);
		} else if (matches(*argv, "dst_mac") == 0) {
			NEXT_ARG();
			ret = flower_parse_eth_addr(*argv,
						    TCA_FLOWER_KEY_ETH_DST,
						    TCA_FLOWER_KEY_ETH_DST_MASK,
						    n);
			if (ret < 0) {
				fprintf(stderr, "Illegal \"dst_mac\"\n");
				return -1;
			}
		} else if (matches(*argv, "src_mac") == 0) {
			NEXT_ARG();
			ret = flower_parse_eth_addr(*argv,
						    TCA_FLOWER_KEY_ETH_SRC,
						    TCA_FLOWER_KEY_ETH_SRC_MASK,
						    n);
			if (ret < 0) {
				fprintf(stderr, "Illegal \"src_mac\"\n");
				return -1;
			}
		} else if (matches(*argv, "ip_proto") == 0) {
			NEXT_ARG();
			ret = flower_parse_ip_proto(*argv, eth_type,
						    TCA_FLOWER_KEY_IP_PROTO,
						    &ip_proto, n);
			if (ret < 0) {
				fprintf(stderr, "Illegal \"ip_proto\"\n");
				return -1;
			}
		} else if (matches(*argv, "ip_tos") == 0) {
			NEXT_ARG();
			ret = flower_parse_ip_tos_ttl(*argv,
						      TCA_FLOWER_KEY_IP_TOS,
						      TCA_FLOWER_KEY_IP_TOS_MASK,
						      n);
			if (ret < 0) {
				fprintf(stderr, "Illegal \"ip_tos\"\n");
				return -1;
			}
		} else if (matches(*argv, "ip_ttl") == 0) {
			NEXT_ARG();
			ret = flower_parse_ip_tos_ttl(*argv,
						      TCA_FLOWER_KEY_IP_TTL,
						      TCA_FLOWER_KEY_IP_TTL_MASK,
						      n);
			if (ret < 0) {
				fprintf(stderr, "Illegal \"ip_ttl\"\n");
				return -1;
			}
		} else if (matches(*argv, "dst_ip") == 0) {
			NEXT_ARG();
			ret = flower_parse_ip_addr(*argv, eth_type,
						   TCA_FLOWER_KEY_IPV4_DST,
						   TCA_FLOWER_KEY_IPV4_DST_MASK,
						   TCA_FLOWER_KEY_IPV6_DST,
						   TCA_FLOWER_KEY_IPV6_DST_MASK,
						   n);
			if (ret < 0) {
				fprintf(stderr, "Illegal \"dst_ip\"\n");
				return -1;
			}
		} else if (matches(*argv, "src_ip") == 0) {
			NEXT_ARG();
			ret = flower_parse_ip_addr(*argv, eth_type,
						   TCA_FLOWER_KEY_IPV4_SRC,
						   TCA_FLOWER_KEY_IPV4_SRC_MASK,
						   TCA_FLOWER_KEY_IPV6_SRC,
						   TCA_FLOWER_KEY_IPV6_SRC_MASK,
						   n);
			if (ret < 0) {
				fprintf(stderr, "Illegal \"src_ip\"\n");
				return -1;
			}
		} else if (matches(*argv, "dst_port") == 0) {
			NEXT_ARG();
			ret = flower_parse_port(*argv, ip_proto,
						FLOWER_ENDPOINT_DST, n);
			if (ret < 0) {
				fprintf(stderr, "Illegal \"dst_port\"\n");
				return -1;
			}
		} else if (matches(*argv, "src_port") == 0) {
			NEXT_ARG();
			ret = flower_parse_port(*argv, ip_proto,
						FLOWER_ENDPOINT_SRC, n);
			if (ret < 0) {
				fprintf(stderr, "Illegal \"src_port\"\n");
				return -1;
			}
		} else if (matches(*argv, "tcp_flags") == 0) {
			NEXT_ARG();
			ret = flower_parse_tcp_flags(*argv,
						     TCA_FLOWER_KEY_TCP_FLAGS,
						     TCA_FLOWER_KEY_TCP_FLAGS_MASK,
						     n);
			if (ret < 0) {
				fprintf(stderr, "Illegal \"tcp_flags\"\n");
				return -1;
			}
		} else if (matches(*argv, "type") == 0) {
			NEXT_ARG();
			ret = flower_parse_icmp(*argv, eth_type, ip_proto,
						FLOWER_ICMP_FIELD_TYPE, n);
			if (ret < 0) {
				fprintf(stderr, "Illegal \"icmp type\"\n");
				return -1;
			}
		} else if (matches(*argv, "code") == 0) {
			NEXT_ARG();
			ret = flower_parse_icmp(*argv, eth_type, ip_proto,
						FLOWER_ICMP_FIELD_CODE, n);
			if (ret < 0) {
				fprintf(stderr, "Illegal \"icmp code\"\n");
				return -1;
			}
		} else if (matches(*argv, "arp_tip") == 0) {
			NEXT_ARG();
			ret = flower_parse_arp_ip_addr(*argv, eth_type,
						TCA_FLOWER_KEY_ARP_TIP,
						TCA_FLOWER_KEY_ARP_TIP_MASK,
						n);
			if (ret < 0) {
				fprintf(stderr, "Illegal \"arp_tip\"\n");
				return -1;
			}
		} else if (matches(*argv, "arp_sip") == 0) {
			NEXT_ARG();
			ret = flower_parse_arp_ip_addr(*argv, eth_type,
						TCA_FLOWER_KEY_ARP_SIP,
						TCA_FLOWER_KEY_ARP_SIP_MASK,
						n);
			if (ret < 0) {
				fprintf(stderr, "Illegal \"arp_sip\"\n");
				return -1;
			}
		} else if (matches(*argv, "arp_op") == 0) {
			NEXT_ARG();
			ret = flower_parse_arp_op(*argv, eth_type,
						TCA_FLOWER_KEY_ARP_OP,
						TCA_FLOWER_KEY_ARP_OP_MASK,
						n);
			if (ret < 0) {
				fprintf(stderr, "Illegal \"arp_op\"\n");
				return -1;
			}
		} else if (matches(*argv, "arp_tha") == 0) {
			NEXT_ARG();
			ret = flower_parse_eth_addr(*argv,
						    TCA_FLOWER_KEY_ARP_THA,
						    TCA_FLOWER_KEY_ARP_THA_MASK,
						    n);
			if (ret < 0) {
				fprintf(stderr, "Illegal \"arp_tha\"\n");
				return -1;
			}
		} else if (matches(*argv, "arp_sha") == 0) {
			NEXT_ARG();
			ret = flower_parse_eth_addr(*argv,
						    TCA_FLOWER_KEY_ARP_SHA,
						    TCA_FLOWER_KEY_ARP_SHA_MASK,
						    n);
			if (ret < 0) {
				fprintf(stderr, "Illegal \"arp_sha\"\n");
				return -1;
			}
		} else if (matches(*argv, "enc_dst_ip") == 0) {
			NEXT_ARG();
			ret = flower_parse_ip_addr(*argv, 0,
						   TCA_FLOWER_KEY_ENC_IPV4_DST,
						   TCA_FLOWER_KEY_ENC_IPV4_DST_MASK,
						   TCA_FLOWER_KEY_ENC_IPV6_DST,
						   TCA_FLOWER_KEY_ENC_IPV6_DST_MASK,
						   n);
			if (ret < 0) {
				fprintf(stderr, "Illegal \"enc_dst_ip\"\n");
				return -1;
			}
		} else if (matches(*argv, "enc_src_ip") == 0) {
			NEXT_ARG();
			ret = flower_parse_ip_addr(*argv, 0,
						   TCA_FLOWER_KEY_ENC_IPV4_SRC,
						   TCA_FLOWER_KEY_ENC_IPV4_SRC_MASK,
						   TCA_FLOWER_KEY_ENC_IPV6_SRC,
						   TCA_FLOWER_KEY_ENC_IPV6_SRC_MASK,
						   n);
			if (ret < 0) {
				fprintf(stderr, "Illegal \"enc_src_ip\"\n");
				return -1;
			}
		} else if (matches(*argv, "enc_key_id") == 0) {
			NEXT_ARG();
			ret = flower_parse_key_id(*argv,
						  TCA_FLOWER_KEY_ENC_KEY_ID, n);
			if (ret < 0) {
				fprintf(stderr, "Illegal \"enc_key_id\"\n");
				return -1;
			}
		} else if (matches(*argv, "enc_dst_port") == 0) {
			NEXT_ARG();
			ret = flower_parse_enc_port(*argv,
						    TCA_FLOWER_KEY_ENC_UDP_DST_PORT, n);
			if (ret < 0) {
				fprintf(stderr, "Illegal \"enc_dst_port\"\n");
				return -1;
			}
		} else if (matches(*argv, "enc_tos") == 0) {
			NEXT_ARG();
			ret = flower_parse_ip_tos_ttl(*argv,
						      TCA_FLOWER_KEY_ENC_IP_TOS,
						      TCA_FLOWER_KEY_ENC_IP_TOS_MASK,
						      n);
			if (ret < 0) {
				fprintf(stderr, "Illegal \"enc_tos\"\n");
				return -1;
			}
		} else if (matches(*argv, "enc_ttl") == 0) {
			NEXT_ARG();
			ret = flower_parse_ip_tos_ttl(*argv,
						      TCA_FLOWER_KEY_ENC_IP_TTL,
						      TCA_FLOWER_KEY_ENC_IP_TTL_MASK,
						      n);
			if (ret < 0) {
				fprintf(stderr, "Illegal \"enc_ttl\"\n");
				return -1;
			}
		} else if (matches(*argv, "geneve_opts") == 0) {
			NEXT_ARG();
			ret = flower_parse_enc_opts_geneve(*argv, n);
			if (ret < 0) {
				fprintf(stderr, "Illegal \"geneve_opts\"\n");
				return -1;
			}
		} else if (matches(*argv, "vxlan_opts") == 0) {
			NEXT_ARG();
			ret = flower_parse_enc_opts_vxlan(*argv, n);
			if (ret < 0) {
				fprintf(stderr, "Illegal \"vxlan_opts\"\n");
				return -1;
			}
		} else if (matches(*argv, "erspan_opts") == 0) {
			NEXT_ARG();
			ret = flower_parse_enc_opts_erspan(*argv, n);
			if (ret < 0) {
				fprintf(stderr, "Illegal \"erspan_opts\"\n");
				return -1;
			}
		} else if (matches(*argv, "action") == 0) {
			NEXT_ARG();
			ret = parse_action(&argc, &argv, TCA_FLOWER_ACT, n);
			if (ret) {
				fprintf(stderr, "Illegal \"action\"\n");
				return -1;
			}
			continue;
		} else if (strcmp(*argv, "help") == 0) {
			explain();
			return -1;
		} else {
			fprintf(stderr, "What is \"%s\"?\n", *argv);
			explain();
			return -1;
		}
		argc--; argv++;
	}

parse_done:
	ret = addattr32(n, MAX_MSG, TCA_FLOWER_FLAGS, flags);
	if (ret)
		return ret;

	if (mtf_mask) {
		ret = addattr32(n, MAX_MSG, TCA_FLOWER_KEY_FLAGS, htonl(mtf));
		if (ret)
			return ret;

		ret = addattr32(n, MAX_MSG, TCA_FLOWER_KEY_FLAGS_MASK, htonl(mtf_mask));
		if (ret)
			return ret;
	}

	if (tc_proto != htons(ETH_P_ALL)) {
		ret = addattr16(n, MAX_MSG, TCA_FLOWER_KEY_ETH_TYPE, tc_proto);
		if (ret)
			return ret;
	}

	tail->rta_len = (((void *)n)+n->nlmsg_len) - (void *)tail;

	return 0;
}

static int __mask_bits(char *addr, size_t len)
{
	int bits = 0;
	bool hole = false;
	int i;
	int j;

	for (i = 0; i < len; i++, addr++) {
		for (j = 7; j >= 0; j--) {
			if (((*addr) >> j) & 0x1) {
				if (hole)
					return -1;
				bits++;
			} else if (bits) {
				hole = true;
			} else{
				return -1;
			}
		}
	}
	return bits;
}

static void flower_print_eth_addr(char *name, struct rtattr *addr_attr,
				  struct rtattr *mask_attr)
{
	SPRINT_BUF(namefrm);
	SPRINT_BUF(out);
	SPRINT_BUF(b1);
	size_t done;
	int bits;

	if (!addr_attr || RTA_PAYLOAD(addr_attr) != ETH_ALEN)
		return;
	done = sprintf(out, "%s",
		       ll_addr_n2a(RTA_DATA(addr_attr), ETH_ALEN,
				   0, b1, sizeof(b1)));
	if (mask_attr && RTA_PAYLOAD(mask_attr) == ETH_ALEN) {
		bits = __mask_bits(RTA_DATA(mask_attr), ETH_ALEN);
		if (bits < 0)
			sprintf(out + done, "/%s",
				ll_addr_n2a(RTA_DATA(mask_attr), ETH_ALEN,
					    0, b1, sizeof(b1)));
		else if (bits < ETH_ALEN * 8)
			sprintf(out + done, "/%d", bits);
	}

	print_nl();
	sprintf(namefrm, "  %s %%s", name);
	print_string(PRINT_ANY, name, namefrm, out);
}

static void flower_print_eth_type(__be16 *p_eth_type,
				  struct rtattr *eth_type_attr)
{
	SPRINT_BUF(out);
	__be16 eth_type;

	if (!eth_type_attr)
		return;

	eth_type = rta_getattr_u16(eth_type_attr);
	if (eth_type == htons(ETH_P_IP))
		sprintf(out, "ipv4");
	else if (eth_type == htons(ETH_P_IPV6))
		sprintf(out, "ipv6");
	else if (eth_type == htons(ETH_P_ARP))
		sprintf(out, "arp");
	else if (eth_type == htons(ETH_P_RARP))
		sprintf(out, "rarp");
	else
		sprintf(out, "%04x", ntohs(eth_type));

	print_nl();
	print_string(PRINT_ANY, "eth_type", "  eth_type %s", out);
	*p_eth_type = eth_type;
}

static void flower_print_ip_proto(__u8 *p_ip_proto,
				  struct rtattr *ip_proto_attr)
{
	SPRINT_BUF(out);
	__u8 ip_proto;

	if (!ip_proto_attr)
		return;

	ip_proto = rta_getattr_u8(ip_proto_attr);
	if (ip_proto == IPPROTO_TCP)
		sprintf(out, "tcp");
	else if (ip_proto == IPPROTO_UDP)
		sprintf(out, "udp");
	else if (ip_proto == IPPROTO_SCTP)
		sprintf(out, "sctp");
	else if (ip_proto == IPPROTO_ICMP)
		sprintf(out, "icmp");
	else if (ip_proto == IPPROTO_ICMPV6)
		sprintf(out, "icmpv6");
	else
		sprintf(out, "%02x", ip_proto);

	print_nl();
	print_string(PRINT_ANY, "ip_proto", "  ip_proto %s", out);
	*p_ip_proto = ip_proto;
}

static void flower_print_ip_attr(const char *name, struct rtattr *key_attr,
				 struct rtattr *mask_attr)
{
	print_masked_u8(name, key_attr, mask_attr, true);
}

static void flower_print_matching_flags(char *name,
					enum flower_matching_flags type,
					struct rtattr *attr,
					struct rtattr *mask_attr)
{
	int i;
	int count = 0;
	__u32 mtf;
	__u32 mtf_mask;

	if (!mask_attr || RTA_PAYLOAD(mask_attr) != 4)
		return;

	mtf = ntohl(rta_getattr_u32(attr));
	mtf_mask = ntohl(rta_getattr_u32(mask_attr));

	for (i = 0; i < ARRAY_SIZE(flags_str); i++) {
		if (type != flags_str[i].type)
			continue;
		if (mtf_mask & flags_str[i].flag) {
			if (++count == 1) {
				print_nl();
				print_string(PRINT_FP, NULL, "  %s ", name);
				open_json_object(name);
			} else {
				print_string(PRINT_FP, NULL, "/", NULL);
			}

			print_bool(PRINT_JSON, flags_str[i].string, NULL,
				   mtf & flags_str[i].flag);
			if (mtf & flags_str[i].flag)
				print_string(PRINT_FP, NULL, "%s",
					     flags_str[i].string);
			else
				print_string(PRINT_FP, NULL, "no%s",
					     flags_str[i].string);
		}
	}
	if (count)
		close_json_object();
}

static void flower_print_ip_addr(char *name, __be16 eth_type,
				 struct rtattr *addr4_attr,
				 struct rtattr *mask4_attr,
				 struct rtattr *addr6_attr,
				 struct rtattr *mask6_attr)
{
	struct rtattr *addr_attr;
	struct rtattr *mask_attr;
	SPRINT_BUF(namefrm);
	SPRINT_BUF(out);
	size_t done;
	int family;
	size_t len;
	int bits;

	if (eth_type == htons(ETH_P_IP)) {
		family = AF_INET;
		addr_attr = addr4_attr;
		mask_attr = mask4_attr;
		len = 4;
	} else if (eth_type == htons(ETH_P_IPV6)) {
		family = AF_INET6;
		addr_attr = addr6_attr;
		mask_attr = mask6_attr;
		len = 16;
	} else {
		return;
	}
	if (!addr_attr || RTA_PAYLOAD(addr_attr) != len)
		return;
	if (!mask_attr || RTA_PAYLOAD(mask_attr) != len)
		return;
	done = sprintf(out, "%s", rt_addr_n2a_rta(family, addr_attr));
	bits = __mask_bits(RTA_DATA(mask_attr), len);
	if (bits < 0)
		sprintf(out + done, "/%s", rt_addr_n2a_rta(family, mask_attr));
	else if (bits < len * 8)
		sprintf(out + done, "/%d", bits);

	print_nl();
	sprintf(namefrm, "  %s %%s", name);
	print_string(PRINT_ANY, name, namefrm, out);
}
static void flower_print_ip4_addr(char *name, struct rtattr *addr_attr,
				  struct rtattr *mask_attr)
{
	return flower_print_ip_addr(name, htons(ETH_P_IP),
				    addr_attr, mask_attr, 0, 0);
}

static void flower_print_port(char *name, struct rtattr *attr,
			      struct rtattr *mask_attr)
{
	print_masked_be16(name, attr, mask_attr, true);
}

static void flower_print_port_range(char *name, struct rtattr *min_attr,
				    struct rtattr *max_attr)
{
	if (!min_attr || !max_attr)
		return;

	if (is_json_context()) {
		open_json_object(name);
		print_hu(PRINT_JSON, "start", NULL, rta_getattr_be16(min_attr));
		print_hu(PRINT_JSON, "end", NULL, rta_getattr_be16(max_attr));
		close_json_object();
	} else {
		SPRINT_BUF(namefrm);
		SPRINT_BUF(out);
		size_t done;

		done = sprintf(out, "%u", rta_getattr_be16(min_attr));
		sprintf(out + done, "-%u", rta_getattr_be16(max_attr));
		print_nl();
		sprintf(namefrm, "  %s %%s", name);
		print_string(PRINT_ANY, name, namefrm, out);
	}
}

static void flower_print_tcp_flags(const char *name, struct rtattr *flags_attr,
				   struct rtattr *mask_attr)
{
	SPRINT_BUF(namefrm);
	SPRINT_BUF(out);
	size_t done;

	if (!flags_attr)
		return;

	done = sprintf(out, "0x%x", rta_getattr_be16(flags_attr));
	if (mask_attr)
		sprintf(out + done, "/%x", rta_getattr_be16(mask_attr));

	print_nl();
	sprintf(namefrm, "  %s %%s", name);
	print_string(PRINT_ANY, name, namefrm, out);
}

static void flower_print_ct_state(struct rtattr *flags_attr,
				  struct rtattr *mask_attr)
{
	SPRINT_BUF(out);
	uint16_t state;
	uint16_t state_mask;
	size_t done = 0;
	int i;

	if (!flags_attr)
		return;

	state = rta_getattr_u16(flags_attr);
	if (mask_attr)
		state_mask = rta_getattr_u16(mask_attr);
	else
		state_mask = UINT16_MAX;

	for (i = 0; i < ARRAY_SIZE(flower_ct_states); i++) {
		if (!(state_mask & flower_ct_states[i].flag))
			continue;

		if (state & flower_ct_states[i].flag)
			done += sprintf(out + done, "+%s",
					flower_ct_states[i].str);
		else
			done += sprintf(out + done, "-%s",
					flower_ct_states[i].str);
	}

	print_nl();
	print_string(PRINT_ANY, "ct_state", "  ct_state %s", out);
}

static void flower_print_ct_label(struct rtattr *attr,
				  struct rtattr *mask_attr)
{
	const unsigned char *str;
	bool print_mask = false;
	int data_len, i;
	SPRINT_BUF(out);
	char *p;

	if (!attr)
		return;

	data_len = RTA_PAYLOAD(attr);
	hexstring_n2a(RTA_DATA(attr), data_len, out, sizeof(out));
	p = out + data_len*2;

	data_len = RTA_PAYLOAD(attr);
	str = RTA_DATA(mask_attr);
	if (data_len != 16)
		print_mask = true;
	for (i = 0; !print_mask && i < data_len; i++) {
		if (str[i] != 0xff)
			print_mask = true;
	}
	if (print_mask) {
		*p++ = '/';
		hexstring_n2a(RTA_DATA(mask_attr), data_len, p,
			      sizeof(out)-(p-out));
		p += data_len*2;
	}
	*p = '\0';

	print_nl();
	print_string(PRINT_ANY, "ct_label", "  ct_label %s", out);
}

static void flower_print_ct_zone(struct rtattr *attr,
				 struct rtattr *mask_attr)
{
	print_masked_u16("ct_zone", attr, mask_attr, true);
}

static void flower_print_ct_mark(struct rtattr *attr,
				 struct rtattr *mask_attr)
{
	print_masked_u32("ct_mark", attr, mask_attr, true);
}

static void flower_print_key_id(const char *name, struct rtattr *attr)
{
	SPRINT_BUF(namefrm);

	if (!attr)
		return;

	print_nl();
	sprintf(namefrm, "  %s %%u", name);
	print_uint(PRINT_ANY, name, namefrm, rta_getattr_be32(attr));
}

static void flower_print_geneve_opts(const char *name, struct rtattr *attr,
				     char *strbuf)
{
	struct rtattr *tb[TCA_FLOWER_KEY_ENC_OPT_GENEVE_MAX + 1];
	int ii, data_len, offset = 0, slen = 0;
	struct rtattr *i = RTA_DATA(attr);
	int rem = RTA_PAYLOAD(attr);
	__u8 type, data_r[rem];
	char data[rem * 2 + 1];
	__u16 class;

	open_json_array(PRINT_JSON, name);
	while (rem) {
		parse_rtattr(tb, TCA_FLOWER_KEY_ENC_OPT_GENEVE_MAX, i, rem);
		class = rta_getattr_be16(tb[TCA_FLOWER_KEY_ENC_OPT_GENEVE_CLASS]);
		type = rta_getattr_u8(tb[TCA_FLOWER_KEY_ENC_OPT_GENEVE_TYPE]);
		data_len = RTA_PAYLOAD(tb[TCA_FLOWER_KEY_ENC_OPT_GENEVE_DATA]);
		hexstring_n2a(RTA_DATA(tb[TCA_FLOWER_KEY_ENC_OPT_GENEVE_DATA]),
			      data_len, data, sizeof(data));
		hex2mem(data, data_r, data_len);
		offset += data_len + 20;
		rem -= data_len + 20;
		i = RTA_DATA(attr) + offset;

		open_json_object(NULL);
		print_uint(PRINT_JSON, "class", NULL, class);
		print_uint(PRINT_JSON, "type", NULL, type);
		open_json_array(PRINT_JSON, "data");
		for (ii = 0; ii < data_len; ii++)
			print_uint(PRINT_JSON, NULL, NULL, data_r[ii]);
		close_json_array(PRINT_JSON, "data");
		close_json_object();

		slen += sprintf(strbuf + slen, "%04x:%02x:%s",
				class, type, data);
		if (rem)
			slen += sprintf(strbuf + slen, ",");
	}
	close_json_array(PRINT_JSON, name);
}

static void flower_print_vxlan_opts(const char *name, struct rtattr *attr,
				    char *strbuf)
{
	struct rtattr *tb[TCA_FLOWER_KEY_ENC_OPT_VXLAN_MAX + 1];
	struct rtattr *i = RTA_DATA(attr);
	int rem = RTA_PAYLOAD(attr);
	__u32 gbp;

	parse_rtattr(tb, TCA_FLOWER_KEY_ENC_OPT_VXLAN_MAX, i, rem);
	gbp = rta_getattr_u32(tb[TCA_FLOWER_KEY_ENC_OPT_VXLAN_GBP]);

	open_json_array(PRINT_JSON, name);
	open_json_object(NULL);
	print_uint(PRINT_JSON, "gbp", NULL, gbp);
	close_json_object();
	close_json_array(PRINT_JSON, name);

	sprintf(strbuf, "%u", gbp);
}

static void flower_print_erspan_opts(const char *name, struct rtattr *attr,
				     char *strbuf)
{
	struct rtattr *tb[TCA_FLOWER_KEY_ENC_OPT_ERSPAN_MAX + 1];
	__u8 ver, hwid, dir;
	__u32 idx;

	parse_rtattr(tb, TCA_FLOWER_KEY_ENC_OPT_ERSPAN_MAX, RTA_DATA(attr),
		     RTA_PAYLOAD(attr));
	ver = rta_getattr_u8(tb[TCA_FLOWER_KEY_ENC_OPT_ERSPAN_VER]);
	if (ver == 1) {
		idx = rta_getattr_be32(tb[TCA_FLOWER_KEY_ENC_OPT_ERSPAN_INDEX]);
		hwid = 0;
		dir = 0;
	} else {
		idx = 0;
		hwid = rta_getattr_u8(tb[TCA_FLOWER_KEY_ENC_OPT_ERSPAN_HWID]);
		dir = rta_getattr_u8(tb[TCA_FLOWER_KEY_ENC_OPT_ERSPAN_DIR]);
	}

	open_json_array(PRINT_JSON, name);
	open_json_object(NULL);
	print_uint(PRINT_JSON, "ver", NULL, ver);
	print_uint(PRINT_JSON, "index", NULL, idx);
	print_uint(PRINT_JSON, "dir", NULL, dir);
	print_uint(PRINT_JSON, "hwid", NULL, hwid);
	close_json_object();
	close_json_array(PRINT_JSON, name);

	sprintf(strbuf, "%u:%u:%u:%u", ver, idx, dir, hwid);
}

static void flower_print_enc_parts(const char *name, const char *namefrm,
				   struct rtattr *attr, char *key, char *mask)
{
	char *key_token, *mask_token, *out;
	int len;

	out = malloc(RTA_PAYLOAD(attr) * 4 + 3);
	if (!out)
		return;

	len = 0;
	key_token = strsep(&key, ",");
	mask_token = strsep(&mask, ",");
	while (key_token) {
		len += sprintf(&out[len], "%s/%s,", key_token, mask_token);
		mask_token = strsep(&mask, ",");
		key_token = strsep(&key, ",");
	}

	out[len - 1] = '\0';
	print_nl();
	print_string(PRINT_FP, name, namefrm, out);
	free(out);
}

static void flower_print_enc_opts(const char *name, struct rtattr *attr,
				  struct rtattr *mask_attr)
{
	struct rtattr *key_tb[TCA_FLOWER_KEY_ENC_OPTS_MAX + 1];
	struct rtattr *msk_tb[TCA_FLOWER_KEY_ENC_OPTS_MAX + 1];
	char *key, *msk;

	if (!attr)
		return;

	key = malloc(RTA_PAYLOAD(attr) * 2 + 1);
	if (!key)
		return;

	msk = malloc(RTA_PAYLOAD(attr) * 2 + 1);
	if (!msk)
		goto err_key_free;

	parse_rtattr_nested(key_tb, TCA_FLOWER_KEY_ENC_OPTS_MAX, attr);
	parse_rtattr_nested(msk_tb, TCA_FLOWER_KEY_ENC_OPTS_MAX, mask_attr);

	if (key_tb[TCA_FLOWER_KEY_ENC_OPTS_GENEVE]) {
		flower_print_geneve_opts("geneve_opt_key",
				key_tb[TCA_FLOWER_KEY_ENC_OPTS_GENEVE], key);

		if (msk_tb[TCA_FLOWER_KEY_ENC_OPTS_GENEVE])
			flower_print_geneve_opts("geneve_opt_mask",
				msk_tb[TCA_FLOWER_KEY_ENC_OPTS_GENEVE], msk);

		flower_print_enc_parts(name, "  geneve_opts %s", attr, key,
				       msk);
	} else if (key_tb[TCA_FLOWER_KEY_ENC_OPTS_VXLAN]) {
		flower_print_vxlan_opts("vxlan_opt_key",
				key_tb[TCA_FLOWER_KEY_ENC_OPTS_VXLAN], key);

		if (msk_tb[TCA_FLOWER_KEY_ENC_OPTS_VXLAN])
			flower_print_vxlan_opts("vxlan_opt_mask",
				msk_tb[TCA_FLOWER_KEY_ENC_OPTS_VXLAN], msk);

		flower_print_enc_parts(name, "  vxlan_opts %s", attr, key,
				       msk);
	} else if (key_tb[TCA_FLOWER_KEY_ENC_OPTS_ERSPAN]) {
		flower_print_erspan_opts("erspan_opt_key",
				key_tb[TCA_FLOWER_KEY_ENC_OPTS_ERSPAN], key);

		if (msk_tb[TCA_FLOWER_KEY_ENC_OPTS_ERSPAN])
			flower_print_erspan_opts("erspan_opt_mask",
				msk_tb[TCA_FLOWER_KEY_ENC_OPTS_ERSPAN], msk);

		flower_print_enc_parts(name, "  erspan_opts %s", attr, key,
				       msk);
	}

	free(msk);
err_key_free:
	free(key);
}

static void flower_print_masked_u8(const char *name, struct rtattr *attr,
				   struct rtattr *mask_attr,
				   const char *(*value_to_str)(__u8 value))
{
	const char *value_str = NULL;
	__u8 value, mask;
	SPRINT_BUF(namefrm);
	SPRINT_BUF(out);
	size_t done;

	if (!attr)
		return;

	value = rta_getattr_u8(attr);
	mask = mask_attr ? rta_getattr_u8(mask_attr) : UINT8_MAX;
	if (mask == UINT8_MAX && value_to_str)
		value_str = value_to_str(value);

	if (value_str)
		done = sprintf(out, "%s", value_str);
	else
		done = sprintf(out, "%d", value);

	if (mask != UINT8_MAX)
		sprintf(out + done, "/%d", mask);

	print_nl();
	sprintf(namefrm, "  %s %%s", name);
	print_string(PRINT_ANY, name, namefrm, out);
}

static void flower_print_u8(const char *name, struct rtattr *attr)
{
	flower_print_masked_u8(name, attr, NULL, NULL);
}

static void flower_print_u32(const char *name, struct rtattr *attr)
{
	SPRINT_BUF(namefrm);

	if (!attr)
		return;

	print_nl();
	sprintf(namefrm, "  %s %%u", name);
	print_uint(PRINT_ANY, name, namefrm, rta_getattr_u32(attr));
}

static void flower_print_mpls_opt_lse(struct rtattr *lse)
{
	struct rtattr *tb[TCA_FLOWER_KEY_MPLS_OPT_LSE_MAX + 1];
	struct rtattr *attr;

	if (lse->rta_type != (TCA_FLOWER_KEY_MPLS_OPTS_LSE | NLA_F_NESTED)) {
		fprintf(stderr, "rta_type 0x%x, expecting 0x%x (0x%x & 0x%x)\n",
		       lse->rta_type,
		       TCA_FLOWER_KEY_MPLS_OPTS_LSE & NLA_F_NESTED,
		       TCA_FLOWER_KEY_MPLS_OPTS_LSE, NLA_F_NESTED);
		return;
	}

	parse_rtattr(tb, TCA_FLOWER_KEY_MPLS_OPT_LSE_MAX, RTA_DATA(lse),
		     RTA_PAYLOAD(lse));

	print_nl();
	print_string(PRINT_FP, NULL, "    lse", NULL);
	open_json_object(NULL);
	attr = tb[TCA_FLOWER_KEY_MPLS_OPT_LSE_DEPTH];
	if (attr)
		print_hhu(PRINT_ANY, "depth", " depth %u",
			  rta_getattr_u8(attr));
	attr = tb[TCA_FLOWER_KEY_MPLS_OPT_LSE_LABEL];
	if (attr)
		print_uint(PRINT_ANY, "label", " label %u",
			   rta_getattr_u32(attr));
	attr = tb[TCA_FLOWER_KEY_MPLS_OPT_LSE_TC];
	if (attr)
		print_hhu(PRINT_ANY, "tc", " tc %u", rta_getattr_u8(attr));
	attr = tb[TCA_FLOWER_KEY_MPLS_OPT_LSE_BOS];
	if (attr)
		print_hhu(PRINT_ANY, "bos", " bos %u", rta_getattr_u8(attr));
	attr = tb[TCA_FLOWER_KEY_MPLS_OPT_LSE_TTL];
	if (attr)
		print_hhu(PRINT_ANY, "ttl", " ttl %u", rta_getattr_u8(attr));
	close_json_object();
}

static void flower_print_mpls_opts(struct rtattr *attr)
{
	struct rtattr *lse;
	int rem;

	if (!attr || !(attr->rta_type & NLA_F_NESTED))
		return;

	print_nl();
	print_string(PRINT_FP, NULL, "  mpls", NULL);
	open_json_array(PRINT_JSON, "mpls");
	rem = RTA_PAYLOAD(attr);
	lse = RTA_DATA(attr);
	while (RTA_OK(lse, rem)) {
		flower_print_mpls_opt_lse(lse);
		lse = RTA_NEXT(lse, rem);
	};
	if (rem)
		fprintf(stderr, "!!!Deficit %d, rta_len=%d\n",
			rem, lse->rta_len);
	close_json_array(PRINT_JSON, NULL);
}

static void flower_print_arp_op(const char *name,
				struct rtattr *op_attr,
				struct rtattr *mask_attr)
{
	flower_print_masked_u8(name, op_attr, mask_attr,
			       flower_print_arp_op_to_name);
}

static int flower_print_opt(struct filter_util *qu, FILE *f,
			    struct rtattr *opt, __u32 handle)
{
	struct rtattr *tb[TCA_FLOWER_MAX + 1];
	__be16 min_port_type, max_port_type;
	int nl_type, nl_mask_type;
	__be16 eth_type = 0;
	__u8 ip_proto = 0xff;

	if (!opt)
		return 0;

	parse_rtattr_nested(tb, TCA_FLOWER_MAX, opt);

	if (handle)
		print_uint(PRINT_ANY, "handle", "handle 0x%x ", handle);

	if (tb[TCA_FLOWER_CLASSID]) {
		__u32 h = rta_getattr_u32(tb[TCA_FLOWER_CLASSID]);

		if (TC_H_MIN(h) < TC_H_MIN_PRIORITY ||
		    TC_H_MIN(h) > (TC_H_MIN_PRIORITY + TC_QOPT_MAX_QUEUE - 1)) {
			SPRINT_BUF(b1);
			print_string(PRINT_ANY, "classid", "classid %s ",
				     sprint_tc_classid(h, b1));
		} else {
			print_uint(PRINT_ANY, "hw_tc", "hw_tc %u ",
				   TC_H_MIN(h) - TC_H_MIN_PRIORITY);
		}
	}

	if (tb[TCA_FLOWER_INDEV]) {
		struct rtattr *attr = tb[TCA_FLOWER_INDEV];

		print_nl();
		print_string(PRINT_ANY, "indev", "  indev %s",
			     rta_getattr_str(attr));
	}

	open_json_object("keys");

	if (tb[TCA_FLOWER_KEY_VLAN_ID]) {
		struct rtattr *attr = tb[TCA_FLOWER_KEY_VLAN_ID];

		print_nl();
		print_uint(PRINT_ANY, "vlan_id", "  vlan_id %u",
			   rta_getattr_u16(attr));
	}

	if (tb[TCA_FLOWER_KEY_VLAN_PRIO]) {
		struct rtattr *attr = tb[TCA_FLOWER_KEY_VLAN_PRIO];

		print_nl();
		print_uint(PRINT_ANY, "vlan_prio", "  vlan_prio %d",
			   rta_getattr_u8(attr));
	}

	if (tb[TCA_FLOWER_KEY_VLAN_ETH_TYPE]) {
		SPRINT_BUF(buf);
		struct rtattr *attr = tb[TCA_FLOWER_KEY_VLAN_ETH_TYPE];

		print_nl();
		print_string(PRINT_ANY, "vlan_ethtype", "  vlan_ethtype %s",
			     ll_proto_n2a(rta_getattr_u16(attr),
			     buf, sizeof(buf)));
	}

	if (tb[TCA_FLOWER_KEY_CVLAN_ID]) {
		struct rtattr *attr = tb[TCA_FLOWER_KEY_CVLAN_ID];

		print_nl();
		print_uint(PRINT_ANY, "cvlan_id", "  cvlan_id %u",
			   rta_getattr_u16(attr));
	}

	if (tb[TCA_FLOWER_KEY_CVLAN_PRIO]) {
		struct rtattr *attr = tb[TCA_FLOWER_KEY_CVLAN_PRIO];

		print_nl();
		print_uint(PRINT_ANY, "cvlan_prio", "  cvlan_prio %d",
			   rta_getattr_u8(attr));
	}

	if (tb[TCA_FLOWER_KEY_CVLAN_ETH_TYPE]) {
		SPRINT_BUF(buf);
		struct rtattr *attr = tb[TCA_FLOWER_KEY_CVLAN_ETH_TYPE];

		print_nl();
		print_string(PRINT_ANY, "cvlan_ethtype", "  cvlan_ethtype %s",
			     ll_proto_n2a(rta_getattr_u16(attr),
			     buf, sizeof(buf)));
	}

	flower_print_eth_addr("dst_mac", tb[TCA_FLOWER_KEY_ETH_DST],
			      tb[TCA_FLOWER_KEY_ETH_DST_MASK]);
	flower_print_eth_addr("src_mac", tb[TCA_FLOWER_KEY_ETH_SRC],
			      tb[TCA_FLOWER_KEY_ETH_SRC_MASK]);

	flower_print_eth_type(&eth_type, tb[TCA_FLOWER_KEY_ETH_TYPE]);
	flower_print_ip_proto(&ip_proto, tb[TCA_FLOWER_KEY_IP_PROTO]);

	flower_print_ip_attr("ip_tos", tb[TCA_FLOWER_KEY_IP_TOS],
			    tb[TCA_FLOWER_KEY_IP_TOS_MASK]);
	flower_print_ip_attr("ip_ttl", tb[TCA_FLOWER_KEY_IP_TTL],
			    tb[TCA_FLOWER_KEY_IP_TTL_MASK]);

	flower_print_mpls_opts(tb[TCA_FLOWER_KEY_MPLS_OPTS]);
	flower_print_u32("mpls_label", tb[TCA_FLOWER_KEY_MPLS_LABEL]);
	flower_print_u8("mpls_tc", tb[TCA_FLOWER_KEY_MPLS_TC]);
	flower_print_u8("mpls_bos", tb[TCA_FLOWER_KEY_MPLS_BOS]);
	flower_print_u8("mpls_ttl", tb[TCA_FLOWER_KEY_MPLS_TTL]);

	flower_print_ip_addr("dst_ip", eth_type,
			     tb[TCA_FLOWER_KEY_IPV4_DST],
			     tb[TCA_FLOWER_KEY_IPV4_DST_MASK],
			     tb[TCA_FLOWER_KEY_IPV6_DST],
			     tb[TCA_FLOWER_KEY_IPV6_DST_MASK]);

	flower_print_ip_addr("src_ip", eth_type,
			     tb[TCA_FLOWER_KEY_IPV4_SRC],
			     tb[TCA_FLOWER_KEY_IPV4_SRC_MASK],
			     tb[TCA_FLOWER_KEY_IPV6_SRC],
			     tb[TCA_FLOWER_KEY_IPV6_SRC_MASK]);

	nl_type = flower_port_attr_type(ip_proto, FLOWER_ENDPOINT_DST);
	nl_mask_type = flower_port_attr_mask_type(ip_proto, FLOWER_ENDPOINT_DST);
	if (nl_type >= 0)
		flower_print_port("dst_port", tb[nl_type], tb[nl_mask_type]);
	nl_type = flower_port_attr_type(ip_proto, FLOWER_ENDPOINT_SRC);
	nl_mask_type = flower_port_attr_mask_type(ip_proto, FLOWER_ENDPOINT_SRC);
	if (nl_type >= 0)
		flower_print_port("src_port", tb[nl_type], tb[nl_mask_type]);

	if (!flower_port_range_attr_type(ip_proto, FLOWER_ENDPOINT_DST,
					 &min_port_type, &max_port_type))
		flower_print_port_range("dst_port",
					tb[min_port_type], tb[max_port_type]);

	if (!flower_port_range_attr_type(ip_proto, FLOWER_ENDPOINT_SRC,
					 &min_port_type, &max_port_type))
		flower_print_port_range("src_port",
					tb[min_port_type], tb[max_port_type]);

	flower_print_tcp_flags("tcp_flags", tb[TCA_FLOWER_KEY_TCP_FLAGS],
			       tb[TCA_FLOWER_KEY_TCP_FLAGS_MASK]);

	nl_type = flower_icmp_attr_type(eth_type, ip_proto,
					FLOWER_ICMP_FIELD_TYPE);
	nl_mask_type = flower_icmp_attr_mask_type(eth_type, ip_proto,
						  FLOWER_ICMP_FIELD_TYPE);
	if (nl_type >= 0 && nl_mask_type >= 0)
		flower_print_masked_u8("icmp_type", tb[nl_type],
				       tb[nl_mask_type], NULL);

	nl_type = flower_icmp_attr_type(eth_type, ip_proto,
					FLOWER_ICMP_FIELD_CODE);
	nl_mask_type = flower_icmp_attr_mask_type(eth_type, ip_proto,
						  FLOWER_ICMP_FIELD_CODE);
	if (nl_type >= 0 && nl_mask_type >= 0)
		flower_print_masked_u8("icmp_code", tb[nl_type],
				       tb[nl_mask_type], NULL);

	flower_print_ip4_addr("arp_sip", tb[TCA_FLOWER_KEY_ARP_SIP],
			     tb[TCA_FLOWER_KEY_ARP_SIP_MASK]);
	flower_print_ip4_addr("arp_tip", tb[TCA_FLOWER_KEY_ARP_TIP],
			     tb[TCA_FLOWER_KEY_ARP_TIP_MASK]);
	flower_print_arp_op("arp_op", tb[TCA_FLOWER_KEY_ARP_OP],
			    tb[TCA_FLOWER_KEY_ARP_OP_MASK]);
	flower_print_eth_addr("arp_sha", tb[TCA_FLOWER_KEY_ARP_SHA],
			      tb[TCA_FLOWER_KEY_ARP_SHA_MASK]);
	flower_print_eth_addr("arp_tha", tb[TCA_FLOWER_KEY_ARP_THA],
			      tb[TCA_FLOWER_KEY_ARP_THA_MASK]);

	flower_print_ip_addr("enc_dst_ip",
			     tb[TCA_FLOWER_KEY_ENC_IPV4_DST_MASK] ?
			     htons(ETH_P_IP) : htons(ETH_P_IPV6),
			     tb[TCA_FLOWER_KEY_ENC_IPV4_DST],
			     tb[TCA_FLOWER_KEY_ENC_IPV4_DST_MASK],
			     tb[TCA_FLOWER_KEY_ENC_IPV6_DST],
			     tb[TCA_FLOWER_KEY_ENC_IPV6_DST_MASK]);

	flower_print_ip_addr("enc_src_ip",
			     tb[TCA_FLOWER_KEY_ENC_IPV4_SRC_MASK] ?
			     htons(ETH_P_IP) : htons(ETH_P_IPV6),
			     tb[TCA_FLOWER_KEY_ENC_IPV4_SRC],
			     tb[TCA_FLOWER_KEY_ENC_IPV4_SRC_MASK],
			     tb[TCA_FLOWER_KEY_ENC_IPV6_SRC],
			     tb[TCA_FLOWER_KEY_ENC_IPV6_SRC_MASK]);

	flower_print_key_id("enc_key_id", tb[TCA_FLOWER_KEY_ENC_KEY_ID]);

	flower_print_port("enc_dst_port", tb[TCA_FLOWER_KEY_ENC_UDP_DST_PORT],
			  tb[TCA_FLOWER_KEY_ENC_UDP_DST_PORT_MASK]);

	flower_print_ip_attr("enc_tos", tb[TCA_FLOWER_KEY_ENC_IP_TOS],
			    tb[TCA_FLOWER_KEY_ENC_IP_TOS_MASK]);
	flower_print_ip_attr("enc_ttl", tb[TCA_FLOWER_KEY_ENC_IP_TTL],
			    tb[TCA_FLOWER_KEY_ENC_IP_TTL_MASK]);
	flower_print_enc_opts("enc_opt", tb[TCA_FLOWER_KEY_ENC_OPTS],
			      tb[TCA_FLOWER_KEY_ENC_OPTS_MASK]);

	flower_print_matching_flags("ip_flags", FLOWER_IP_FLAGS,
				    tb[TCA_FLOWER_KEY_FLAGS],
				    tb[TCA_FLOWER_KEY_FLAGS_MASK]);

	flower_print_ct_state(tb[TCA_FLOWER_KEY_CT_STATE],
			      tb[TCA_FLOWER_KEY_CT_STATE_MASK]);
	flower_print_ct_zone(tb[TCA_FLOWER_KEY_CT_ZONE],
			     tb[TCA_FLOWER_KEY_CT_ZONE_MASK]);
	flower_print_ct_mark(tb[TCA_FLOWER_KEY_CT_MARK],
			     tb[TCA_FLOWER_KEY_CT_MARK_MASK]);
	flower_print_ct_label(tb[TCA_FLOWER_KEY_CT_LABELS],
			      tb[TCA_FLOWER_KEY_CT_LABELS_MASK]);

	close_json_object();

	if (tb[TCA_FLOWER_FLAGS]) {
		__u32 flags = rta_getattr_u32(tb[TCA_FLOWER_FLAGS]);

		if (flags & TCA_CLS_FLAGS_SKIP_HW) {
			print_nl();
			print_bool(PRINT_ANY, "skip_hw", "  skip_hw", true);
		}
		if (flags & TCA_CLS_FLAGS_SKIP_SW) {
			print_nl();
			print_bool(PRINT_ANY, "skip_sw", "  skip_sw", true);
		}
		if (flags & TCA_CLS_FLAGS_IN_HW) {
			print_nl();
			print_bool(PRINT_ANY, "in_hw", "  in_hw", true);

			if (tb[TCA_FLOWER_IN_HW_COUNT]) {
				__u32 count = rta_getattr_u32(tb[TCA_FLOWER_IN_HW_COUNT]);

				print_uint(PRINT_ANY, "in_hw_count",
					   " in_hw_count %u", count);
			}
		}
		else if (flags & TCA_CLS_FLAGS_NOT_IN_HW) {
			print_nl();
			print_bool(PRINT_ANY, "not_in_hw", "  not_in_hw", true);
		}
	}

	if (tb[TCA_FLOWER_ACT])
		tc_print_action(f, tb[TCA_FLOWER_ACT], 0);

	return 0;
}

struct filter_util flower_filter_util = {
	.id = "flower",
	.parse_fopt = flower_parse_opt,
	.print_fopt = flower_print_opt,
};
