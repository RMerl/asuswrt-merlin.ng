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
 * >:
 */

#include "../../../linux-5.15/net/openvswitch/flow.h"
#include "../../../linux-5.15/net/openvswitch/datapath.h"
#include "ovs_bcm_blog.h"

/* These masks are used for flow acceleration.
 * mask_attr_ignore: All the attributes that are configured to be ignored.
 * mask_attr_field_ignore: All the attributes fields that are configured to
 * be ignored.
 * mask_attr_not_allowed: If any of these attributes is set a flow is not
 * accelerated.
 */
u64 mask_attr_ignore;
u64 mask_attr_field_ignore;
u64 mask_attr_not_allowed;

/**
 * ovs_bcm_blog_notify - flow going away notify flow cache
 * @flow: megaflow
 */
void ovs_bcm_blog_notify(struct sw_flow *flow)
{
	blog_lock();

	/* flow going away, notify blog client */
	if (flow && flow->sf_ext.blog_key != BLOG_KEY_FC_INVALID)
		blog_notify(DESTROY_MEGA, (void *)flow, (u32)flow->sf_ext.blog_key, 0);

	blog_unlock();
}

/**
 * ovs_bcm_blog_output_ports - counts the number of output port actions
 * @attr: flow action attributes
 * @len: len of attributes
 */
int ovs_bcm_blog_output_ports(const struct nlattr *attr, int len)
{
	const struct nlattr *a;
	int rem;
	int count = 0;

	for (a = attr, rem = len; rem > 0;
		 a = nla_next(a, &rem)) {
		switch (nla_type(a)) {
		case OVS_ACTION_ATTR_OUTPUT: {
			count++;
			break;
		}
		default:
			break;
	}
}

return count;
}

/**
 * blog_chk_unknown_ucast - calls blog_skip() for
 * unknown ucast (i.e., output port count > 1) or cases where mirroring is done.
 * @skb: received skb
 * @output_count: #of output ports used in the flow
 */
inline void blog_chk_unknown_ucast(struct sk_buff *skb, int output_count)
{
	if (output_count > 1 && !skb->blog_p->rx.multicast)
		blog_skip(skb, blog_skip_reason_mega_multi_output_ports);
}

/**
 * ovs_bcm_blog_match_l2 - decides whether the blog_skip() be called
 * for the megaflow based on the flow key and mask and l2_keymap.
 * For a L2 flow if a mask for any of IPv4/6 header field is set and
 * if not ignored is considered mismatch and the flow cannnot be accelerated.
 * Returns: FALSE: if blog_skip() should be called, TRUE: if blog_skip() should
 * not be called
 */
bool ovs_bcm_blog_match_l2(struct sw_flow *flow)
{
	struct sw_flow_key *flow_key = &flow->key;
	struct sw_flow_key *mask_key = &flow->mask->key;

	if (mask_key->eth.type == htons(0xffff) &&
	    (flow_key->eth.type == htons(ETH_P_IP) ||
	     mask_key->eth.type == htons(ETH_P_IPV6))) {
		if (mask_attr_field_ignore != OVS_BLOG_IGNORE_ALL_PKT_KEY_ATTR_FIELD_MASK) {
			/* No support for IP Frag */
			if (!(mask_attr_field_ignore &
						(1ULL << OVS_BLOG_KEY_ATTR_IP_FIELD_FRAG))) {
				if (mask_key->ip.frag == 0xff)
					return false;
			}

			/* No support for TTL */
			if (!(mask_attr_field_ignore &
						(1ULL << OVS_BLOG_KEY_ATTR_IP_FIELD_TTL))) {
				if (mask_key->ip.ttl == 0xff)
					return false;
			}

			/* No support for ToS */
			if (!(mask_attr_field_ignore &
						(1ULL << OVS_BLOG_KEY_ATTR_IP_FIELD_TOS))) {
				if (mask_key->ip.tos == 0xff)
					return false;
			}

			/* No support for TCP Flags */
			if (!(mask_attr_field_ignore &
					(1ULL << OVS_BLOG_KEY_ATTR_IP_FIELD_TCP_FLAGS))) {
				if (mask_key->tp.flags == 0xff)
					return false;
			}
		}

		/* No support for proto, L4 sport and dport */
		if (mask_key->ip.proto || mask_key->tp.src || mask_key->tp.dst)
			return false;

		switch (flow_key->eth.type) {
		case htons(ETH_P_IP):
			/* No support for IPv4 src and/or dest */
			if (mask_key->ipv4.addr.src || mask_key->ipv4.addr.dst)
				return false;
			break;

		case htons(ETH_P_IPV6):
		{
			u32 *src = (u32 *)&mask_key->ipv6.addr.src;
			u32 *dst = (u32 *)&mask_key->ipv6.addr.dst;

			/* No support for IPv6 src and/or dest */
			if (src[0] || src[1] || src[2] || src[3] ||
			    dst[0] || dst[1] || dst[2] || dst[3])
				return false;

			/* No support for IPv6 flow label */
			if (!(mask_attr_field_ignore
						& (1ULL << OVS_BLOG_KEY_ATTR_IP_FIELD_LABEL)))
				if (mask_key->ipv6.label == htonl(0xffffffff))
					return false;
			break;
		}

		default:
			break;
		}
	}
	return true;
}

/**
 * ovs_bcm_blog_match - decides whether blog_skip() be called
 * for the megaflow based on the match flag and output port count.
 * Also links the megaflow to blog.
 * @skb: received skb
 * @flow: flow found after key lookup
 */
void ovs_bcm_blog_match(struct sk_buff *skb, struct sw_flow *flow)
{
	struct blog_t *blog_p = skb->blog_p;

	if (!blog_ptr(skb))
		return;

	if (!flow->sf_ext.blog_match || !(blog_p->l2_keymap || blog_p->l3_keymap)) {
		blog_skip(skb, blog_skip_reason_mega_attr_mismatch);
	} else if (blog_p->l2_keymap && !ovs_bcm_blog_match_l2(flow)) {
		blog_skip(skb, blog_skip_reason_mega_field_mismatch);
	} else if (blog_ptr(skb)) {
		blog_chk_unknown_ucast(skb, flow->sf_ext.output_count);
		blog_link(MEGA, blog_ptr(skb), (void *)flow, 0, 0);

		/* fcache updates byte count of DP flow by rx pkt len of physical
		 * device. If the input port of DP flow is gretap iface, it needs
		 * to minus outer header length.
		 */
		if (flow->sf_ext.is_gretap && !flow->sf_ext.stats_adj)
			flow->sf_ext.stats_adj = BLOG_IPV4_HDR_LEN + blog_p->grerx.hlen +
					blog_p->grerx.l2_hlen;
	}
}

/**
 * ovs_bcm_blog_match_validate - decides whether the blog_skip() be called
 * for the megaflow based on the flow key and mask.
 * @match: receives the extracted flow match information.
 * @key_attrs: key_attributes (fields) of the flow of the packet that
 * triggered the creation.
 * @mask_attrs: mask of key_attributes (fields) of the flow of the packet that
 * triggered the creation.
 * Returns: FALSE: if blog_skip() should be called, TRUE: if blog_skip() should
 * not be called
 */
bool ovs_bcm_blog_match_validate(const struct sw_flow_match *match,
				 u64 key_attrs, u64 mask_attrs)
{
	u64 key_expected = 0;
	u64 mask_allowed;
	u64 mask_attr_all_ignore = OVS_BLOG_IGNORE_ALL_METADATA_KEY_ATTR_MASK;

	key_attrs &= ~mask_attr_ignore;
	mask_attrs &= ~mask_attr_ignore;
	mask_allowed = key_attrs; /* At most allow all key attributes, after ignore */

	if (mask_attrs & mask_attr_not_allowed)
		return false;

	if (mask_attrs & mask_attr_all_ignore)
		return false;

	/* The following mask attributes allowed only if they
	 * pass the validation tests.
	 */
	mask_allowed &= ~((1ULL << OVS_KEY_ATTR_IPV4)
			| (1ULL << OVS_KEY_ATTR_CT_ORIG_TUPLE_IPV4)
			| (1ULL << OVS_KEY_ATTR_IPV6)
			| (1ULL << OVS_KEY_ATTR_CT_ORIG_TUPLE_IPV6)
			| (1ULL << OVS_KEY_ATTR_TCP)
			| (1ULL << OVS_KEY_ATTR_TCP_FLAGS)
			| (1ULL << OVS_KEY_ATTR_UDP));

	/* Always allowed mask fields. */
	mask_allowed |= ((1ULL << OVS_KEY_ATTR_TUNNEL)
			   | (1ULL << OVS_KEY_ATTR_IN_PORT)
			   | (1ULL << OVS_KEY_ATTR_ETHERTYPE)
			   | (1ULL << OVS_KEY_ATTR_ETHERNET)
			   | (1ULL << OVS_KEY_ATTR_VLAN));

	if (match->key->tun_proto != 0)
		return false;

	if (match->key->mac_proto != MAC_PROTO_ETHERNET)
		return false;

	/* Check key attributes. */
	if (match->key->eth.type == htons(ETH_P_IP)) {
		key_expected |= 1ULL << OVS_KEY_ATTR_IPV4;
		if (match->mask && match->mask->key.eth.type == htons(0xffff)) {
			mask_allowed |= 1ULL << OVS_KEY_ATTR_IPV4;
			mask_allowed |= 1ULL << OVS_KEY_ATTR_CT_ORIG_TUPLE_IPV4;
		}

		/* No support for TTL/HopLimit */
		if (!(mask_attr_field_ignore
					& (1ULL << OVS_BLOG_KEY_ATTR_IP_FIELD_TTL))) {
			if (match->mask && match->mask->key.ip.ttl == 0xff)
				return false;
		}

		/* No support for ToS */
		if (!(mask_attr_field_ignore
					& (1ULL << OVS_BLOG_KEY_ATTR_IP_FIELD_TOS))) {
			if (match->mask && match->mask->key.ip.tos == 0xff)
				return false;
		}

		if (match->key->ip.frag != OVS_FRAG_TYPE_LATER) {
			if (match->key->ip.proto == IPPROTO_UDP) {
				key_expected |= 1ULL << OVS_KEY_ATTR_UDP;
				if (match->mask && match->mask->key.ip.proto == 0xff)
					mask_allowed |= 1ULL << OVS_KEY_ATTR_UDP;
			}

			if (match->key->ip.proto == IPPROTO_TCP) {
				key_expected |= 1ULL << OVS_KEY_ATTR_TCP;
				key_expected |= 1ULL << OVS_KEY_ATTR_TCP_FLAGS;
				if (match->mask && match->mask->key.ip.proto == 0xff) {
					mask_allowed |= 1ULL << OVS_KEY_ATTR_TCP;
					mask_allowed |= 1ULL << OVS_KEY_ATTR_TCP_FLAGS;
				}
			}
		}
	}

	if (match->key->eth.type == htons(ETH_P_IPV6)) {
		key_expected |= 1ULL << OVS_KEY_ATTR_IPV6;
		if (match->mask && match->mask->key.eth.type == htons(0xffff)) {
			mask_allowed |= 1ULL << OVS_KEY_ATTR_IPV6;
			mask_allowed |= 1ULL << OVS_KEY_ATTR_CT_ORIG_TUPLE_IPV6;
		}

		/* No support for IPv6 hop limit */
		if (!(mask_attr_field_ignore
					& (1ULL << OVS_BLOG_KEY_ATTR_IP_FIELD_TTL))) {
			if (match->mask && match->mask->key.ip.ttl == 0xff)
				return false;
		}

		/* No support for IPv6 ToS */
		if (!(mask_attr_field_ignore
					& (1ULL << OVS_BLOG_KEY_ATTR_IP_FIELD_TOS))) {
			if (match->mask && match->mask->key.ip.tos == 0xff)
				return false;
		}

		/* No support for IPv6 flow label */
		if (!(mask_attr_field_ignore
					& (1ULL << OVS_BLOG_KEY_ATTR_IP_FIELD_LABEL))) {
			if (match->mask->key.ipv6.label == htonl(0xffffffff))
				return false;
		}

		if (match->key->ip.frag != OVS_FRAG_TYPE_LATER) {
			if (match->key->ip.proto == IPPROTO_UDP) {
				key_expected |= 1ULL << OVS_KEY_ATTR_UDP;
				if (match->mask && match->mask->key.ip.proto == 0xff)
					mask_allowed |= 1ULL << OVS_KEY_ATTR_UDP;
			}

			if (match->key->ip.proto == IPPROTO_TCP) {
				key_expected |= 1ULL << OVS_KEY_ATTR_TCP;
				key_expected |= 1ULL << OVS_KEY_ATTR_TCP_FLAGS;
				if (match->mask && match->mask->key.ip.proto == 0xff) {
					mask_allowed |= 1ULL << OVS_KEY_ATTR_TCP;
					mask_allowed |= 1ULL << OVS_KEY_ATTR_TCP_FLAGS;
				}
			}
		}
	}

	if ((key_attrs & key_expected) != key_expected)
		/* Key attributes check failed. */
		return false;

	if ((mask_attrs & mask_allowed) != mask_attrs)
		/* Mask attributes check failed. */
		return false;

	return true;
}

/**
 * ovs_bcm_blog_mega_get_key - gets the blog_key from a megaflow
 * @mega: megaflow
 */
unsigned long ovs_bcm_blog_mega_get_key(void *mega)
{
	struct sw_flow *flow = (struct sw_flow *)mega;

	if (flow)
		return (flow->sf_ext.blog_key);
	else
		return BLOG_KEY_FC_INVALID;
}

/**
 * ovs_bcm_blog_mega_set_key - sets the blog_key in a megaflow
 * @mega: megaflow
 * @blog_key: new blog_key to be set
 */
void ovs_bcm_blog_mega_set_key(void *mega, unsigned long blog_key)
{
	struct sw_flow *flow = (struct sw_flow *)mega;

	/* check megaflow connections for any corruptions*/
	if (blog_key != BLOG_KEY_FC_INVALID) {
		/*check blog_key should be BLOG_KEY_FC_INVALID */
		if (flow->sf_ext.blog_key != BLOG_KEY_FC_INVALID) {
			/* split to 2 printk to pass checkpatch.pl */
			pr_warn("MEGA: blog_key corruption when adding flow ");
			pr_warn("mega flow=%p ld_key=0x%08x new_key=0x%08lx\n",
				flow, flow->sf_ext.blog_key, blog_key);
		}
	} else {
		/*check blog_key should be BLOG_KEY_FC_INVALID */
		if (flow->sf_ext.blog_key == BLOG_KEY_FC_INVALID) {
			pr_warn("MEGA: blog_key corruption when deleting flow ");
			pr_warn("mega flow=%p old_key=0x%08x new_key=0x%08lx\n",
				flow, flow->sf_ext.blog_key, blog_key);
		}
	}

	flow->sf_ext.blog_key = (u32)blog_key;
}

/**
 * ovs_bcm_blog_mega_update_fast_stats - update megaflow fast stats
 * @flow: megaflow
 * @ovs_stats: megaflows stats
 * @used: last packet forward time
 */
bool ovs_bcm_blog_mega_update_fast_stats(struct sw_flow *flow,
					 struct ovs_flow_stats *ovs_stats)
{
	blog_fast_stats_t fast_stats = {0};
	blog_fast_stats_t *fast_stats_p = &fast_stats;

	if (flow->sf_ext.blog_key != BLOG_KEY_FC_INVALID) {
		blog_notify_async_wait(FETCH_MEGA_STATS, (void *)flow,
				       (unsigned long)flow->sf_ext.blog_key,
				       (unsigned long)fast_stats_p);
		ovs_stats->n_packets += fast_stats_p->packet_count;
		ovs_stats->n_bytes += fast_stats_p->byte_count -
				(fast_stats_p->packet_count * flow->sf_ext.stats_adj);
	}

	ovs_stats->n_packets += flow->sf_ext.cumm_fast_stats.packet_count;
	ovs_stats->n_bytes += flow->sf_ext.cumm_fast_stats.byte_count;

	if (flow->sf_ext.blog_key != BLOG_KEY_FC_INVALID &&
	    flow->sf_ext.prev_packet_count != fast_stats_p->packet_count) {
		flow->sf_ext.prev_packet_count = fast_stats_p->packet_count;
		return true;
	}

	return false;
}

/**
 * ovs_bcm_blog_mega_put_fast_stats - puts the stats in a megaflow
 * @mega: megaflow
 * @fast_stats_p: flows stats from flow cache
 */
void ovs_bcm_blog_mega_put_fast_stats(void *mega, const blog_fast_stats_t *fast_stats_p)
{
	struct sw_flow *flow = (struct sw_flow *)mega;

	if (flow->sf_ext.blog_key != BLOG_KEY_FC_INVALID) {
		flow->sf_ext.cumm_fast_stats.packet_count += fast_stats_p->packet_count;
		flow->sf_ext.cumm_fast_stats.byte_count += fast_stats_p->byte_count -
				(fast_stats_p->packet_count * flow->sf_ext.stats_adj);
	}
}

/**
 * ovs_bcm_blog_mega_clear_fast_stats - clears/resets the megaflow fast stats
 * @flow: megaflow
 */
void ovs_bcm_blog_mega_clear_fast_stats(struct sw_flow *flow)
{
	if (flow->sf_ext.blog_key != BLOG_KEY_FC_INVALID)
		blog_notify_async_wait(CLEAR_MEGA_STATS, (void *)flow,
				       (unsigned long)flow->sf_ext.blog_key, (unsigned long)NULL);

	flow->sf_ext.cumm_fast_stats.packet_count = 0;
	flow->sf_ext.cumm_fast_stats.byte_count = 0;
}

/**
 * ovs_bcm_blog_is_internal_dev - Is the device an OVS internal device?
 * @dev: net device
 */
int ovs_bcm_blog_is_ovs_internal_dev(void *dev)
{
	struct vport *vport;

	vport = ovs_internal_dev_get_vport((struct net_device *)dev);
	return vport ? 1 : 0;
}

void ovs_bcm_blog_init(void)
{
	blog_ovs_hooks_t blog_ovs_hooks = {};

	mask_attr_ignore = OVS_BLOG_IGNORE_METADATA_KEY_ATTR_MASK;
	mask_attr_field_ignore = OVS_BLOG_IGNORE_PKT_KEY_ATTR_FIELD_MASK;
	mask_attr_not_allowed = OVS_BLOG_NOT_ALLOWED_PKT_KEY_ATTR_MASK;

	blog_ovs_hooks.is_ovs_internal_dev = ovs_bcm_blog_is_ovs_internal_dev;
	blog_ovs_hooks.mega_get_key = ovs_bcm_blog_mega_get_key;
	blog_ovs_hooks.mega_set_key = ovs_bcm_blog_mega_set_key;
	blog_ovs_hooks.mega_put_fast_stats = ovs_bcm_blog_mega_put_fast_stats;

	blog_bind_ovs(&blog_ovs_hooks);
}

void ovs_bcm_blog_exit(void)
{
	blog_ovs_hooks_t blog_ovs_hooks = {};

	blog_ovs_hooks.is_ovs_internal_dev = NULL;
	blog_ovs_hooks.mega_get_key = NULL;
	blog_ovs_hooks.mega_set_key = NULL;
	blog_ovs_hooks.mega_put_fast_stats = NULL;

	blog_bind_ovs(&blog_ovs_hooks);

	mask_attr_ignore = 0;
	mask_attr_field_ignore = 0;
	mask_attr_not_allowed = 0;
}

#endif /* defined(CONFIG_BLOG) */
