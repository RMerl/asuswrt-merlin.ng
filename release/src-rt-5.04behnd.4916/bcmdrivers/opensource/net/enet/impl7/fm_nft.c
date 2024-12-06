/*
   <:copyright-BRCM:2015:DUAL/GPL:standard

      Copyright (c) 2015 Broadcom 
      All Rights Reserved

   Unless you and Broadcom execute a separate written software license
   agreement governing use of this software, this software is licensed
   to you under the terms of the GNU General Public License version 2
   (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
   with the following added to such license:

      As a special exception, the copyright holders of this software give
      you permission to link this software with independent modules, and
      to copy and distribute the resulting executable under terms of your
      choice, provided that you also meet, for each linked independent
      module, the terms and conditions of the license of that module.
      An independent module is a module which is not derived from this
      software.  The special exception does not apply to any modifications
      of the software.

   Not withstanding the above, under no circumstances may you combine
   this software in any way with any other Broadcom software provided
   under a license other than the GPL, without Broadcom's express prior
   written consent.

   :>
 */

/*
 *  Created on: Jan/2023
 *      Author: itai.weiss@broadcom.com
 */ 

#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_zones.h>
#include <net/netfilter/nf_conntrack_labels.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_acct.h>
#include <uapi/linux/tc_act/tc_pedit.h>
#include <net/tc_act/tc_ct.h>
#include <net/flow_offload.h>
#include <net/netfilter/nf_flow_table.h>
#include <linux/workqueue.h>
#include <linux/refcount.h>
#include <linux/xarray.h>
#include <linux/if_vlan.h>
#include <linux/rhashtable.h>

#include <net/flow_dissector.h>
#include <net/pkt_cls.h>
#include "fm_nft_priv.h"


int fm_nft_global_trace_level = LOGLEVEL_ERR;

const struct rhashtable_params fm_nft_flow_table_params = {
    .head_offset = offsetof(fm_nft_flow_node_t, node),
    .key_offset = offsetof(fm_nft_flow_node_t, cookie),
    .key_len = sizeof(((fm_nft_flow_node_t *)0)->cookie),
    .automatic_shrinking = true
};

static int fn_nft_hw_accel_get(void)
{
#ifdef CONFIG_BCM_XDP
    /* XXX: Temporary, need to add logic to select the right accelerator according to the flow params.
            Currently if XDP support is enabled its preferred. In final solution FN_NFT_HW_ACCEL_RNR should always be preferred and 
            XDP will serve as fallback */
    return FN_NFT_HW_ACCEL_XDP;
#endif
    return FN_NFT_HW_ACCEL_RNR;
}

const char *loglevel2kern(int level)
{
	switch (level) {
	case LOGLEVEL_EMERG:
		return KERN_EMERG;
	case LOGLEVEL_ALERT:
		return KERN_ALERT;
	case LOGLEVEL_CRIT:
		return KERN_CRIT;
	case LOGLEVEL_ERR:
		return KERN_ERR;
	case LOGLEVEL_WARNING:
		return KERN_WARNING;
	case LOGLEVEL_NOTICE:
		return KERN_NOTICE;
	case LOGLEVEL_INFO:
		return KERN_INFO;
	case LOGLEVEL_DEBUG:
		return KERN_DEBUG;
	case LOGLEVEL_VERBOSE:
		return KERN_DEBUG;
	}
	return "Unknown log level";
}

void nft_fm_dump_tuple(struct net_device *dev, nft_fw_tuple_t *tuple)
{
    fm_nft_printk(LOGLEVEL_DEBUG, dev, "addr_type: %d, n_proto: %d, ip_proto: %d\n", tuple->addr_type, tuple->n_proto,
        tuple->ip_proto);

    RETURN_ON_ERR(tuple->addr_type != FLOW_DISSECTOR_KEY_IPV4_ADDRS,,dev,"We got IPv6 packet - not printing\n");


    fm_nft_printk(LOGLEVEL_DEBUG, dev, "source ip %pI4\n", &tuple->ip.src_v4);
    fm_nft_printk(LOGLEVEL_DEBUG, dev, "source port %d\n", cpu_to_be16(tuple->port.src));
    fm_nft_printk(LOGLEVEL_DEBUG, dev, "dest ip %pI4\n", &tuple->ip.dst_v4);
    fm_nft_printk(LOGLEVEL_DEBUG, dev, "dest port %d\n", cpu_to_be16(tuple->port.dst));
}

void nft_fm_dump_macs(struct net_device *dev, struct ethhdr *eth)
{
    fm_nft_printk(LOGLEVEL_DEBUG, dev, "NAT source MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", eth->h_source[0],
        eth->h_source[1], eth->h_source[2], eth->h_source[3], eth->h_source[4], eth->h_source[5]);
    fm_nft_printk(LOGLEVEL_DEBUG, dev, "NAT dest MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", eth->h_dest[0], eth->h_dest[1],
        eth->h_dest[2], eth->h_dest[3], eth->h_dest[4], eth->h_dest[5]);
    fm_nft_printk(LOGLEVEL_DEBUG, dev, "NAT h proto 0x%x (0x%x) sizeof ethhdr %ld\n", eth->h_proto, htons(eth->h_proto),
        sizeof(struct ethhdr));
}

int nft_fm_tc_rule_to_key_tuple(struct net_device *dev, struct nft_fw_tuple *tuple, struct flow_rule *rule)
{
    struct flow_match_control control;
    struct flow_match_basic basic;

    flow_rule_match_basic(rule, &basic);
    flow_rule_match_control(rule, &control);

    tuple->n_proto = basic.key->n_proto;
    tuple->ip_proto = basic.key->ip_proto;
    tuple->addr_type = control.key->addr_type;

    if (tuple->addr_type == FLOW_DISSECTOR_KEY_IPV4_ADDRS)
    {
        struct flow_match_ipv4_addrs match;

        flow_rule_match_ipv4_addrs(rule, &match);
        tuple->ip.src_v4 = match.key->src;
        tuple->ip.dst_v4 = match.key->dst;
    }
    else if (tuple->addr_type == FLOW_DISSECTOR_KEY_IPV6_ADDRS)
    {
        struct flow_match_ipv6_addrs match;

        flow_rule_match_ipv6_addrs(rule, &match);
        tuple->ip.src_v6 = match.key->src;
        tuple->ip.dst_v6 = match.key->dst;
    }
    else
    {
        fm_nft_printk(LOGLEVEL_ERR, dev, "%s: Unsupported addr type %d\n", __FUNCTION__, tuple->addr_type);
        return -EOPNOTSUPP;
    }

    if (flow_rule_match_key(rule, FLOW_DISSECTOR_KEY_PORTS))
    {
        struct flow_match_ports match;

        flow_rule_match_ports(rule, &match);
        switch (tuple->ip_proto)
        {
        case IPPROTO_TCP:
        case IPPROTO_UDP:
            tuple->port.src = match.key->src;
            tuple->port.dst = match.key->dst;
            break;
        default:
            fm_nft_printk(LOGLEVEL_ERR, dev, "%s: Unknown protocol %d\n", __FUNCTION__, tuple->ip_proto);
            return -EOPNOTSUPP;
        }
    }

    return 0;
}

static int nft_fm_tc_parse_redirect(nft_fm_tc_info_t *ctx, const struct flow_action_entry *act)
{
    struct net_device *ingress_dev = ctx->fm_nft_ctx->dev;
    struct net_device *egress_dev = act->dev;

    RETURN_ON_ERR(!egress_dev, -EINVAL, ingress_dev, "no dev in mirred action\n");

    fm_nft_printk(LOGLEVEL_DEBUG, ingress_dev, "Got tx_ifindex %d\n", egress_dev->ifindex);
    ctx->tx_ifindex = egress_dev->ifindex;

    return 0;
}

/* The ETH addresses arrives in iterations of 4 bytes */
static void flow_offload_mangle_eth(const struct flow_action_entry *act, void *eth)
{
    uint32_t *dest = eth + act->mangle.offset;

    BUG_ON(act->mangle.offset > 8);
    BUG_ON((act->mangle.val & act->mangle.mask) != 0);

    *dest = (*dest & act->mangle.mask) | act->mangle.val;
}

static int nft_fm_tc_rule_to_tuple_nat(nft_fm_tc_info_t *ctx, struct flow_action_entry *act)
{
    nft_fw_tuple_t *tuple_ctx = &ctx->tuple_ctx;
    u32 offset, val, ip6_offset;

    fm_nft_printk(LOGLEVEL_DEBUG,ctx->fm_nft_ctx->dev, "nft_fm_tc_rule_to_tuple_nat: Entered, action %d, offset %d, value %x (%d), \
        mask %x (%d)\n", act->mangle.htype, act->mangle.offset ,act->mangle.val, act->mangle.val, act->mangle.mask,
        act->mangle.mask);

    offset = act->mangle.offset;
    val = act->mangle.val;
    switch (act->mangle.htype) {
    case FLOW_ACT_MANGLE_HDR_TYPE_IP4:
        if (offset == offsetof(struct iphdr, saddr))
        {
            ctx->is_snat = true;
            tuple_ctx->ip.src_v4 = val;
        }
        else if (offset == offsetof(struct iphdr, daddr))
        {
            ctx->is_snat = false;
            tuple_ctx->ip.dst_v4 = val;
        }
        else
            return -EOPNOTSUPP;
        break;

    case FLOW_ACT_MANGLE_HDR_TYPE_IP6:
        ip6_offset = (offset - offsetof(struct ipv6hdr, saddr));
        ip6_offset /= 4;
        if (ip6_offset < 4)
        {
            ctx->is_snat = true;
            tuple_ctx->ip.src_v6.s6_addr32[ip6_offset] = val;
        }
        else if (ip6_offset < 8)
        {
            ctx->is_snat = false;
            tuple_ctx->ip.dst_v6.s6_addr32[ip6_offset - 4] = val;
        }
        else
            return -EOPNOTSUPP;
        break;

    case FLOW_ACT_MANGLE_HDR_TYPE_TCP:
    case FLOW_ACT_MANGLE_HDR_TYPE_UDP:
        if (ctx->is_snat)
            tuple_ctx->port.src = htons(ntohl(val) >> 16);
        else
            tuple_ctx->port.dst = htons(ntohl(val));
        break;

    case FLOW_ACT_MANGLE_HDR_TYPE_ETH:
        flow_offload_mangle_eth(act, &ctx->eth);
        ctx->l2_hdr_size = ETHHDR_ADDRS_LEN;
        break;
    default:
        return -EOPNOTSUPP;
    }

    return 0;
}

#define ACTION_NAME_SIZE 16

static const char flowact2str[NUM_FLOW_ACTIONS][ACTION_NAME_SIZE] = {
    [FLOW_ACTION_ACCEPT]    = "ACCEPT",
    [FLOW_ACTION_DROP]  = "DROP",
    [FLOW_ACTION_TRAP]  = "TRAP",
    [FLOW_ACTION_GOTO]  = "GOTO",
    [FLOW_ACTION_REDIRECT]  = "REDIRECT",
    [FLOW_ACTION_MIRRED]    = "MIRRED",
    [FLOW_ACTION_VLAN_PUSH] = "VLAN_PUSH",
    [FLOW_ACTION_VLAN_POP]  = "VLAN_POP",
    [FLOW_ACTION_VLAN_MANGLE]   = "VLAN_MANGLE",
    [FLOW_ACTION_TUNNEL_ENCAP]  = "TUNNEL_ENCAP",
    [FLOW_ACTION_TUNNEL_DECAP]  = "TUNNEL_DECAP",
    [FLOW_ACTION_MANGLE]    = "MANGLE",
    [FLOW_ACTION_ADD]   = "ADD",
    [FLOW_ACTION_CSUM]  = "CSUM",
    [FLOW_ACTION_MARK]  = "MARK",
    [FLOW_ACTION_WAKE]  = "WAKE",
    [FLOW_ACTION_QUEUE] = "QUEUE",
    [FLOW_ACTION_SAMPLE]    = "SAMPLE",
    [FLOW_ACTION_POLICE]    = "POLICE",
    [FLOW_ACTION_CT]    = "CT",
};

static void vlan_update(struct net_device *dev, nft_fm_vlan_t *vlan, struct flow_action_entry *act)
{
    vlan->vhdr[vlan->vlan_tag_num].h_vlan_TCI = htons(act->vlan.vid);
    vlan->vhdr[vlan->vlan_tag_num].h_vlan_encapsulated_proto = act->vlan.proto;
    vlan->vlan_tag_num++;

    fm_nft_printk(LOGLEVEL_DEBUG, dev, "Received: action %s: vid %d, proto 0x%x, prio %d\n", flowact2str[act->id],
        act->vlan.vid, htons(act->vlan.proto), act->vlan.prio);
}

int nft_fm_tc_parse_actions(nft_fm_tc_info_t *ctx, struct flow_action *flow_action,
    struct netlink_ext_ack *extack)
{
    struct flow_action_entry *act;
    int i, rc;
    struct net_device * dev = ctx->fm_nft_ctx->dev;

    RETURN_ON_ERR(!flow_action_has_entries(flow_action),-EINVAL,dev,"no actions\n");

    RETURN_ON_ERR(!flow_action_basic_hw_stats_check(flow_action, extack), -EOPNOTSUPP, dev, "flow_action_basic_hw_stats_check fails"); 

    memcpy(&ctx->tuple_ctx, &ctx->tuple_key, sizeof(nft_fw_tuple_t));

    flow_action_for_each(i, act, flow_action)
    {
        fm_nft_printk(LOGLEVEL_DEBUG, dev, "Got flow action %s (id %d)\n", flowact2str[act->id], act->id);

        switch (act->id)
        {
        case FLOW_ACTION_DROP:
            return 0; /* don't bother with other actions */
        case FLOW_ACTION_REDIRECT:
            rc = nft_fm_tc_parse_redirect(ctx, act);
            RETURN_ON_ERR(rc,rc,dev,"nft_fm_tc_parse_redirect failed\n");
            break;
        case FLOW_ACTION_VLAN_POP:
            BUG_ON(ctx->l2_hdr_size == 0);
            ctx->l2_hdr_delta -= sizeof(struct vlan_hdr);
            break;
        case FLOW_ACTION_VLAN_PUSH:
	    BUG_ON(ctx->l2_hdr_size == 0);
            vlan_update(dev, &ctx->vlan, act);
            ctx->l2_hdr_delta += sizeof(struct vlan_hdr);
            ctx->l2_hdr_size += sizeof(struct vlan_hdr);
            break;
        case FLOW_ACTION_VLAN_MANGLE:
            break;
        case FLOW_ACTION_TUNNEL_ENCAP:
            break;
        case FLOW_ACTION_TUNNEL_DECAP:
            break;
            /* Packet edit: L2 rewrite, NAT, NAPT */
        case FLOW_ACTION_MANGLE:
            rc = nft_fm_tc_rule_to_tuple_nat(ctx, act);
            RETURN_ON_ERR(rc,rc,dev, "Failed to parse nat action tuple. rc %d\n", rc);
            break;
        case FLOW_ACTION_CSUM:
            break;
        default:
            fm_nft_printk(LOGLEVEL_ERR, dev, "Unexpected action - id %d\n", act->id);
            break;
        }
    }

    return 0;
}

/* Check if device ifindex match the received ifindex.
 * Return 1 if they are the same, 0 otherwise and -1 if there is an error */
static ifindex_match_t nft_tc_match_ifindex(struct flow_rule *flow_rule, struct net_device *dev)
{
    if (flow_rule_match_key(flow_rule, FLOW_DISSECTOR_KEY_META))
    {
        struct flow_match_meta match;

        flow_rule_match_meta(flow_rule, &match);
        fm_nft_printk(LOGLEVEL_DEBUG, dev, "ingress_ifindex %d, ingress_iftype %d\n", match.key->ingress_ifindex,
            match.key->ingress_iftype);

        /* Filter out irrelevant calls by comparing ifindex */
        RETURN_ON_ERR_LVL(dev->ifindex != match.key->ingress_ifindex, IFINDEX_DIFFER, LOGLEVEL_VERBOSE, dev, "Mixed ifindex %d - skip...\n",dev->ifindex);

        return IFINDEX_MATCH;
    }

    return IFINDEX_ERR;
}

static int nft_tc_block_add_flow(struct flow_cls_offload *f, fm_nft_ctx_t * fm_nft_ctx)
{
    int err = 0;
    ifindex_match_t ret;
    nft_fm_tc_info_t nft_fm_tc_info_ctx = { .fm_nft_ctx = fm_nft_ctx };
    struct net_device * dev = fm_nft_ctx->dev;
    struct flow_rule *flow_rule = flow_cls_offload_flow_rule(f);
    fm_nft_flow_node_t * flow_node = NULL;

    ret = nft_tc_match_ifindex(flow_rule, dev);
    RETURN_ON_ERR(ret == IFINDEX_ERR,-EINVAL,dev,"Failed to parse key meta.\n");

    if (ret == IFINDEX_DIFFER)
    {
        return 0;
    }

    err = nft_fm_tc_rule_to_key_tuple(dev, &nft_fm_tc_info_ctx.tuple_key, flow_rule);
    RETURN_ON_ERR(err,err,dev, "Failed to parse key tuple.\n");

    fm_nft_printk(LOGLEVEL_DEBUG, dev, "Got the following key tuple:\n");
    nft_fm_dump_tuple(dev, &nft_fm_tc_info_ctx.tuple_key);

    err = nft_fm_tc_parse_actions(&nft_fm_tc_info_ctx, &flow_rule->action, f->common.extack);
    RETURN_ON_ERR(err,err,dev, "Failed to parse action\n");

    nft_fm_dump_macs(dev, &nft_fm_tc_info_ctx.eth);

    fm_nft_printk(LOGLEVEL_DEBUG, dev, "Got the following nat action tuple:\n");
    nft_fm_dump_tuple(dev, &nft_fm_tc_info_ctx.tuple_ctx);


    flow_node = kzalloc(sizeof(*flow_node), GFP_KERNEL);

    RETURN_ON_ERR(!flow_node,-ENOMEM,dev,"Memory allocation failed\n"); 

    flow_node->cookie = f->cookie;
    flow_node->tx_ifindex = nft_fm_tc_info_ctx.tx_ifindex;

    nft_fm_tc_info_ctx.cookie = f->cookie;


    /* try runner if accell is not set to XDP */
    err = -EOPNOTSUPP;
    if (fn_nft_hw_accel_get() == FN_NFT_HW_ACCEL_RNR)
      err = nft_fm_to_rnr_lookup_entry_add(&nft_fm_tc_info_ctx, flow_node);

#ifdef CONFIG_BCM_XDP
    /* try XDP if runner is not supporting it, or accelleration is forced to XDP */
    if (err == -EOPNOTSUPP)
        err = nft_fm_to_xdp_lookup_entry_add(&nft_fm_tc_info_ctx, flow_node);
#endif

    RETURN_ON_ERR(err,err,dev,"Failed to add flow\n"); 

    fm_nft_printk(LOGLEVEL_VERBOSE, dev, "adding cookie %lu to dev %s\n",flow_node->cookie,dev->name);

    err = rhashtable_insert_fast(&fm_nft_ctx->fm_nft_flow_table, &flow_node->node, fm_nft_ctx->fm_nft_flow_params);
    GOTO_ON_ERR(err, nft_tc_block_add_flow_fail, dev, "rhashtable_insert_fast failed\n");

    return 0;

nft_tc_block_add_flow_fail:
    kfree(flow_node);
    return -1;
}

static int nft_tc_block_delete_flow(struct flow_cls_offload *f, fm_nft_ctx_t * fm_nft_ctx)
{
    int rc;
    fm_nft_flow_node_t *flow_node = rhashtable_lookup_fast(&fm_nft_ctx->fm_nft_flow_table, &f->cookie, fm_nft_ctx->fm_nft_flow_params);
    RETURN_ON_ERR(!flow_node, -1, fm_nft_ctx->dev, "rhashtable_lookup_fast: node not found %lu\n",f->cookie);

#ifdef CONFIG_BCM_XDP
    if (flow_node->xdp_accellerated)
      rc = nft_fm_to_xdp_lookup_entry_delete(fm_nft_ctx, flow_node);
#endif
    rc = nft_fm_to_rnr_lookup_entry_delete(fm_nft_ctx, flow_node);

    RETURN_ON_ERR(rc,rc,fm_nft_ctx->dev,"failed to delete flow\n");

    rc = rhashtable_remove_fast(&fm_nft_ctx->fm_nft_flow_table, &flow_node->node, fm_nft_ctx->fm_nft_flow_params);
    RETURN_ON_ERR(rc,rc,fm_nft_ctx->dev,"%s: rhashtable_remove_fast failed. rc %d\n",__FUNCTION__,rc);

    return 0;
}

int fm_nft_stats_update(fm_nft_ctx_t *fm_nft_ctx, unsigned long cookie, u64 packets, u64 bytes, u64 drops)
{
    fm_nft_flow_node_t *flow_node = NULL;
    struct net_device *dev;

    if (!fm_nft_ctx)
        return -1;

    dev = fm_nft_ctx->dev;

    fm_nft_printk(LOGLEVEL_VERBOSE, dev, "%s: Entered. cookie %lu, packets %llu, bytes %llu, drops %llu\n", __FUNCTION__,
        cookie, packets, bytes, drops);

    if (!cookie)
        return 0;

    flow_node = rhashtable_lookup_fast(&fm_nft_ctx->fm_nft_flow_table, &cookie, fm_nft_ctx->fm_nft_flow_params);
    RETURN_ON_ERR(!flow_node, -1, dev, "%s: rhashtable_lookup_fast - node not found %lu\n", __FUNCTION__,cookie);

    flow_node->stats.packets += packets;
    flow_node->stats.bytes += bytes;
    flow_node->stats.drops += drops;
    flow_node->lastused = jiffies;

    fm_nft_printk(LOGLEVEL_VERBOSE, dev, "%s: Done. cookie %lu, current stats: packets %llu, bytes %llu, drops %llu, \
        lastused %lu\n", __FUNCTION__, cookie, flow_node->stats.packets, flow_node->stats.bytes, flow_node->stats.drops,
        flow_node->lastused);

    return 0;
}

static int nft_tc_block_get_flow_stats(struct flow_cls_offload *f, fm_nft_ctx_t *fm_nft_ctx)
{
    fm_nft_flow_node_t *flow_node = NULL;
    struct net_device *dev = fm_nft_ctx->dev;
    fm_nft_stats_t stats, *curr_stats, *prev_stats;
    unsigned long lastused;

    if (!fm_nft_ctx)
        return -1;

    flow_node = rhashtable_lookup_fast(&fm_nft_ctx->fm_nft_flow_table, &f->cookie, fm_nft_ctx->fm_nft_flow_params);
    if (!flow_node)
        return -1;

#ifdef CONFIG_BCM_XDP
    if (!flow_node->xdp_accellerated)
#endif
    {
        nft_rnr_update_flow_stat(fm_nft_ctx,flow_node);
    }

    curr_stats = &flow_node->stats;
    prev_stats = &flow_node->prev_stats;

    stats.packets = curr_stats->packets - prev_stats->packets;
    stats.bytes = curr_stats->bytes - prev_stats->bytes;
    stats.drops = curr_stats->drops - prev_stats->drops;
    *prev_stats = *curr_stats;
    lastused = flow_node->lastused;

    if (stats.bytes)
    {
        fm_nft_printk(LOGLEVEL_DEBUG, dev, "%s: flow cookie %lu, bytes %llu, packets %llu, drops %llu, lastused %lu\n",
            __FUNCTION__, f->cookie, stats.bytes, stats.packets, stats.drops, lastused);
    }

    flow_stats_update(&f->stats, stats.bytes, stats.packets, stats.drops, lastused, FLOW_ACTION_HW_STATS_DELAYED);

    return 0;
}

void nft_uninit(fm_nft_ctx_t *fm_nft_ctx)
{
    if (!fm_nft_ctx)
        return;

    printk(KERN_DEBUG "nft_uninit: Entered %s.\n",fm_nft_ctx->dev->name);

    rhashtable_destroy(&fm_nft_ctx->fm_nft_flow_table);

    kfree(fm_nft_ctx);

    rnr_nft_uninit();
}

int nft_init(fm_nft_ctx_t **_ctx, struct net_device *dev, xdp_nft_get_progs_by_dev_cb get_xdp_prog_cb)
{
    fm_nft_ctx_t *ctx;
    int rc;

    printk(KERN_DEBUG "nft_init: Entered %s.\n",dev->name);

    ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
    RETURN_ON_ERR(!ctx,-ENOMEM,dev, "nft_init: Memory allocation failed\n");

    ctx->fm_nft_flow_params = fm_nft_flow_table_params;

    rc = rhashtable_init(&ctx->fm_nft_flow_table, &ctx->fm_nft_flow_params);
    GOTO_ON_ERR(rc,nft_init_fail,dev,"nft_init: init hash table failed. rc: %d\n", rc);

    ctx->dev = dev;

    *_ctx = ctx;

#if defined(CONFIG_BCM_XDP)
    xdp_nft_init(ctx, get_xdp_prog_cb);
#endif
    rnr_nft_init(dev);

    return 0;

nft_init_fail:
    kfree(ctx);
    return -1;
}

/* This is the HW offload CB that is called from nftables offload flow. */
static int fm_setup_tc_nft_cb(enum tc_setup_type type, void *type_data, void *cb_priv)
{
    struct flow_cls_offload *f = type_data;
    fm_nft_ctx_t *fm_nft_ctx = cb_priv;
    struct net_device *dev = fm_nft_ctx->dev;
    fm_nft_printk(LOGLEVEL_DEBUG, dev, "fm_setup_tc_nft_cb: type %d f->command %d\n",type,f->command);

    switch (f->command)
    {
    case FLOW_CLS_STATS:
        return nft_tc_block_get_flow_stats(f, fm_nft_ctx);
    case FLOW_CLS_REPLACE:
        {
            fm_nft_printk(LOGLEVEL_DEBUG, dev, "fm_setup_tc_nft_cb: FLOW_CLS_REPLACE\n");
            return nft_tc_block_add_flow(f, fm_nft_ctx);
        }
    case FLOW_CLS_DESTROY:
        {
            fm_nft_printk(LOGLEVEL_DEBUG, dev, "fm_setup_tc_nft_cb: FLOW_CLS_DESTROY\n");
            return nft_tc_block_delete_flow(f, fm_nft_ctx);
        }
    default:
        fm_nft_printk(LOGLEVEL_ERR, dev, "fm_setup_tc_nft_cb: Unsupported command %d\n", f->command);
        return -EOPNOTSUPP;
    }

    return 0;
}

static LIST_HEAD(fm_block_ft_cb_list);
int fm_setup_tc(fm_nft_ctx_t *fm_nft_ctx, enum tc_setup_type type, void *type_data)
{
    struct net_device *dev = fm_nft_ctx->dev;
    fm_nft_printk(LOGLEVEL_DEBUG, dev, "fm_setup_tc: HW offload cb is set for device %s, type %d ifindex %d\n", dev->name, type, dev->ifindex);

    switch (type) {
    case TC_SETUP_FT:
        return flow_block_cb_setup_simple(type_data, &fm_block_ft_cb_list, fm_setup_tc_nft_cb, dev, fm_nft_ctx, true);
    case TC_SETUP_BLOCK:
    case TC_SETUP_CLSU32:
    case TC_SETUP_CLSMATCHALL:
    case TC_SETUP_CLSFLOWER:
        return 0;
    default:
        fm_nft_printk(LOGLEVEL_ERR, dev, "fm_setup_tc: Unsupported type, type %d\n", type);
        return -EOPNOTSUPP;
    }

    return 0;
}
