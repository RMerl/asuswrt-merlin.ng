#if defined(CONFIG_BCM_OVS) && defined(CONFIG_BLOG)
/*
 * <:copyright-BRCM:2019:DUAL/GPL:standard
 *
 *    Copyright (c) 2019 Broadcom
 *    All Rights Reserved
 *
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 *
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 *
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 *
 * :>
 */

#ifndef OVS_BCM_BLOG_H
#define OVS_BCM_BLOG_H 1

#include <net/gre.h>

/* These key attributes are always set. Flow Cache will always ignore them */
#define OVS_BLOG_ALWAYS_ALLOWED_KEY_ATTR_MASK \
				((1ULL << OVS_KEY_ATTR_TUNNEL) \
				| (1ULL << OVS_KEY_ATTR_IN_PORT) \
				| (1ULL << OVS_KEY_ATTR_ETHERTYPE) \
				| (1ULL << OVS_KEY_ATTR_ETHERNET) \
				| (1ULL << OVS_KEY_ATTR_VLAN))

/* These key attributes if set blog_skip will be called. Flow cache will
 * not accelerate these flows.
 */
#define OVS_BLOG_NOT_ALLOWED_PKT_KEY_ATTR_MASK \
				 ((1ULL << OVS_KEY_ATTR_ENCAP) \
				| (1ULL << OVS_KEY_ATTR_ICMP) \
				| (1ULL << OVS_KEY_ATTR_ICMPV6) \
				| (1ULL << OVS_KEY_ATTR_ARP) \
				| (1ULL << OVS_KEY_ATTR_ND) \
				| (1ULL << OVS_KEY_ATTR_SCTP) \
				| (1ULL << OVS_KEY_ATTR_MPLS) \
				| (1ULL << OVS_KEY_ATTR_TUNNEL_INFO))

/* These key attributes even if set, flow cache will ignore them (don't care).
 * If any of these attributes is set for the flow, flow cache will accelerate
 * the packets and use the learnt values for these attributes in all the
 * subsequent accelerated packets of the flow.
 *
 * CAUTION: If any of these attributes(s) are set for the flow and not ignored in
 * the mask below, flow cache will not learn that flow and hence not accelerate
 * the packets of that flow.
 *
 * Note:- These attribute masks were observed to be set for disable-megaflows
 */
#define OVS_BLOG_IGNORE_METADATA_KEY_ATTR_MASK \
				 ((1ULL << OVS_KEY_ATTR_PRIORITY) \
				| (1ULL << OVS_KEY_ATTR_SKB_MARK) \
				| (1ULL << OVS_KEY_ATTR_DP_HASH) \
				| (1ULL << OVS_KEY_ATTR_RECIRC_ID) \
				| (1ULL << OVS_KEY_ATTR_CT_STATE) \
				| (1ULL << OVS_KEY_ATTR_CT_ZONE) \
				| (1ULL << OVS_KEY_ATTR_CT_MARK) \
				| (1ULL << OVS_KEY_ATTR_CT_LABELS))

/* All the attributes that can be ignored.
 * CAUTION: Do NOT change this mask
 */
#define OVS_BLOG_IGNORE_ALL_METADATA_KEY_ATTR_MASK \
				 ((1ULL << OVS_KEY_ATTR_PRIORITY) \
				| (1ULL << OVS_KEY_ATTR_SKB_MARK) \
				| (1ULL << OVS_KEY_ATTR_DP_HASH) \
				| (1ULL << OVS_KEY_ATTR_RECIRC_ID) \
				| (1ULL << OVS_KEY_ATTR_CT_STATE) \
				| (1ULL << OVS_KEY_ATTR_CT_ZONE) \
				| (1ULL << OVS_KEY_ATTR_CT_MARK) \
				| (1ULL << OVS_KEY_ATTR_CT_LABELS))

enum ovs_bcm_blog_key_attr_field {
	OVS_BLOG_KEY_ATTR_IP_FIELD_FRAG,
	OVS_BLOG_KEY_ATTR_IP_FIELD_TTL,
	OVS_BLOG_KEY_ATTR_IP_FIELD_TOS,
	OVS_BLOG_KEY_ATTR_IP_FIELD_TCP_FLAGS,
	OVS_BLOG_KEY_ATTR_IP_FIELD_LABEL,
	__OVS_BLOG_KEY_ATTR_FIELD_MAX
};

#define OVS_BLOG_KEY_ATTR_FIELD_MAX (__OVS_BLOG_KEY_ATTR_FIELD_MAX - 1)

/* If a attribute is set, we can still allow to ignore specific fields
 * of that attribute (e.g. if IPV4 attribute is set we can still decide
 * to ignore TTL field).
 * If any of these attributes is set for the flow and the field is ignored,
 * flow cache will accelerate the packets and use the learnt values for these
 * attribute field in accelerated packets.
 *
 * CAUTION: If any of these field(s) are set for the flow and not ignored in
 * the mask below, flow cache will not learn that flow and hence not accelerate
 * the packets of that flow.
 */
#define OVS_BLOG_IGNORE_PKT_KEY_ATTR_FIELD_MASK \
				 ((1ULL << OVS_BLOG_KEY_ATTR_IP_FIELD_FRAG) \
				| (1ULL << OVS_BLOG_KEY_ATTR_IP_FIELD_TTL) \
				| (1ULL << OVS_BLOG_KEY_ATTR_IP_FIELD_TOS) \
				| (1ULL << OVS_BLOG_KEY_ATTR_IP_FIELD_TCP_FLAGS) \
				| (1ULL << OVS_BLOG_KEY_ATTR_IP_FIELD_LABEL))

/* All the attributes fields that can be ignored.
 * CAUTION: Do NOT change this mask
 */
#define OVS_BLOG_IGNORE_ALL_PKT_KEY_ATTR_FIELD_MASK \
				 ((1ULL << OVS_BLOG_KEY_ATTR_IP_FIELD_FRAG) \
				| (1ULL << OVS_BLOG_KEY_ATTR_IP_FIELD_TTL) \
				| (1ULL << OVS_BLOG_KEY_ATTR_IP_FIELD_TOS) \
				| (1ULL << OVS_BLOG_KEY_ATTR_IP_FIELD_TCP_FLAGS) \
				| (1ULL << OVS_BLOG_KEY_ATTR_IP_FIELD_LABEL))

void ovs_bcm_blog_notify(struct sw_flow *flow);
int ovs_bcm_blog_output_ports(const struct nlattr *attr, int len);
void ovs_bcm_blog_match(struct sk_buff *skb, struct sw_flow *flow);
bool ovs_bcm_blog_match_validate(const struct sw_flow_match *match,
				 u64 key_attrs, u64 mask_attrs);
unsigned long ovs_bcm_blog_mega_get_key(void *flow);
void ovs_bcm_blog_mega_set_key(void *flow, unsigned long new_blog_key);

bool  ovs_bcm_blog_mega_update_fast_stats(struct sw_flow *flow,
					  struct ovs_flow_stats *ovs_stats);
void ovs_bcm_blog_mega_clear_fast_stats(struct sw_flow *flow);

void ovs_bcm_blog_init(void);
void ovs_bcm_blog_exit(void);

#endif /* OVS_BCM_BLOG_H */
#endif /* defined(CONFIG_BLOG) */
