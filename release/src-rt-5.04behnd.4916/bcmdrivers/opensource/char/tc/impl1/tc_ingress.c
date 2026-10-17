/*
   <:copyright-BRCM:2024:DUAL/GPL:standard

      Copyright (c) 2024 Broadcom 
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

#include <linux/etherdevice.h>
#include <net/pkt_cls.h>
#include <net/tc_act/tc_gact.h>
#include <linux/rhashtable.h>
#include "tc_int.h"
#include "bcm_tc.h"

enum {
    TC_CHAIN_ID_IF =           BCM_TC_CHAIN_ID_IC_INGRESS_FILTER,
    TC_CHAIN_ID_IC_RX =        BCM_TC_CHAIN_ID_IC_RX,
    TC_CHAIN_ID_IC_PRIMARY =   BCM_TC_CHAIN_ID_IC_PRIMARY,
    TC_CHAIN_ID_IC_FLOW_MISS = BCM_TC_CHAIN_ID_IC_FLOW_MISS,
};

static struct rhashtable filter_ht;

struct filter_entry {
    unsigned long key;
    struct rhash_head ht_node;
    rdpa_traffic_dir dir;
    bdmf_object_handle ingress_port;
    union {
        struct {
            rdpa_ic_cfg_t cfg;
            rdpa_ic_info_t flow;
        };
        struct {
           int filter_idx;
           rdpa_filter_ctrl_t filter_ctrl;
           uint32_t opaque;
        };
    };
};

static const struct rhashtable_params filter_ht_params = {
    .key_offset = offsetof(struct filter_entry, key),
    .key_len = sizeof(unsigned long),
    .head_offset = offsetof(struct filter_entry, ht_node),
    .automatic_shrinking = true,
};

struct filter_action {
    rdpa_forward_action action;
    int reason;
    int iq_priority;
};

static unsigned char tc_prio2filter_idx(unsigned char flower_index) 
{
    if (!flower_index)
        return 0xFF;
    flower_index--;
#ifndef XRDP_RGEN6
    /* RDPA_FILTER_DNS no suppoted */
    if (flower_index > RDPA_FILTER_L2CP)
        flower_index--;
#endif
    if (flower_index >= RDPA_FILTER_MAX)
        return RDPA_FILTER_MAX;
    return flower_index;
}


static bdmf_object_handle tc_dev2rdpa_port_obj(struct net_device *dev)
{
    bcm_netdev_priv_info_get_cb_fn_t priv_info_get = bcm_netdev_ext_field_get(dev, bcm_netdev_cb_fn);
    bcm_netdev_priv_info_out_t info_out = {};

    if (!priv_info_get || priv_info_get(dev, BCM_NETDEV_TO_RDPA_PORT_OBJ, &info_out))
        return NULL;

    return info_out.bcm_netdev_to_rdpa_port_obj.rdpa_port_obj;
}

static rdpa_traffic_dir tc_dev2rdpa_dir(struct net_device *dev)
{
    return ((is_netdev_wan(dev) || is_netdev_tc_all_ports(dev)) ? rdpa_dir_ds : rdpa_dir_us);
}

static int tc_update_rdpa_action(struct flow_action_entry *action, struct filter_action *a)
{
    switch (action->id)
    {
    case FLOW_ACTION_ACCEPT:
        a->action = rdpa_forward_action_skip;
        break;
    case FLOW_ACTION_DROP:
        a->action = rdpa_forward_action_drop;
        break;
    case FLOW_ACTION_TRAP:
        a->action = rdpa_forward_action_host;
        a->reason = action->rtrap.reason;
        break;
    case FLOW_ACTION_IQ_PRIO:
        a->iq_priority = action->iq_prio.priority;
        break;
    default:
        return -1;
    }

    return 0;
}

static bool tc_has_not_supported_keys(unsigned int keys)
{
    return keys &
        (BIT(FLOW_DISSECTOR_KEY_PORTS_RANGE) |
         BIT(FLOW_DISSECTOR_KEY_TIPC) |
         BIT(FLOW_DISSECTOR_KEY_GRE_KEYID) |
         BIT(FLOW_DISSECTOR_KEY_ENC_KEYID) |
         BIT(FLOW_DISSECTOR_KEY_ENC_IPV4_ADDRS) |
         BIT(FLOW_DISSECTOR_KEY_ENC_IPV6_ADDRS) |
         BIT(FLOW_DISSECTOR_KEY_ENC_CONTROL) |
         BIT(FLOW_DISSECTOR_KEY_ENC_PORTS) |
         BIT(FLOW_DISSECTOR_KEY_ENC_IP) |
         BIT(FLOW_DISSECTOR_KEY_ENC_OPTS) |
         BIT(FLOW_DISSECTOR_KEY_MPLS) |
         BIT(FLOW_DISSECTOR_KEY_MPLS_ENTROPY) |
         BIT(FLOW_DISSECTOR_KEY_CT) |
         BIT(FLOW_DISSECTOR_KEY_HASH));
}

static int tc_add_ic_rule(struct filter_entry *entry, unsigned long cookie)
{
    int rc;

    if (tc_runner_create_ic_object(entry->dir, &entry->cfg, &entry->flow))
    {
        rc = -1;
        goto leave;
    }

    entry->key = cookie;
    rc = rhashtable_insert_fast(&filter_ht, &entry->ht_node, filter_ht_params); 
    if (rc)
        tc_runner_del_ic_rule(entry->dir, &entry->cfg, &entry->flow);

leave:
    __debug("Adding ic rule %s.\n", rc ? "failed" : "succeeded");
    return rc;
}

static int tc_del_ic_rule(unsigned long cookie)
{
    struct filter_entry *entry;

    entry = rhashtable_lookup_fast(&filter_ht, &cookie, filter_ht_params);
    if (entry)
    {
        tc_runner_del_ic_rule(entry->dir, &entry->cfg, &entry->flow);

        rhashtable_remove_fast(&filter_ht, &entry->ht_node, filter_ht_params);
        kfree(entry);
    }
    return 0;
}

static int tc_update_ic_stats(int chain_index, unsigned long cookie, struct flow_stats *stats)
{
    int rc;
    struct filter_entry *entry;
    uint64_t bytes;
    uint64_t packets;

    if (chain_index == TC_CHAIN_ID_IF ||
        (chain_index != TC_CHAIN_ID_IC_RX &&
         chain_index != TC_CHAIN_ID_IC_PRIMARY &&
         chain_index != TC_CHAIN_ID_IC_FLOW_MISS))
    {
        return -EOPNOTSUPP;
    }

    entry = rhashtable_lookup_fast(&filter_ht, &cookie, filter_ht_params);
    if (!entry)
        return -ENOENT;

    rc = tc_runner_get_ic_rule_stats(entry->dir, &entry->cfg, &entry->flow, &bytes, &packets);
    if (!rc)
        flow_stats_update(stats, bytes, packets, 0, jiffies, FLOW_ACTION_HW_STATS_IMMEDIATE);

    return rc ? -1 : 0;
}

static int tc_add_if_rule(struct filter_entry *entry, unsigned long cookie)
{
    int rc;

    __debug("filter_idx %d, cookie 0x%X\n", entry->filter_idx, cookie);

    entry->key = cookie;
    rc = rhashtable_insert_fast(&filter_ht, &entry->ht_node, filter_ht_params); 
    if (!rc)
    {
        entry->filter_ctrl.enabled = 1;
        rc = tc_runner_set_if_rule(entry->ingress_port, entry->filter_idx, &entry->filter_ctrl, entry->opaque);
        __debug("filter_idx %d, cookie 0x%X rc %d\n", entry->filter_idx, cookie, rc);
        if (rc)
            rhashtable_remove_fast(&filter_ht, &entry->ht_node, filter_ht_params);
    }

    return rc;
}

static int tc_del_if_rule(unsigned long cookie)
{
    struct filter_entry *entry;

    __debug("cookie 0x%X\n", cookie);
    entry = rhashtable_lookup_fast(&filter_ht, &cookie, filter_ht_params);
    if (entry)
    {
        entry->filter_ctrl.enabled = 0;
        tc_runner_set_if_rule(entry->ingress_port, entry->filter_idx, &entry->filter_ctrl, entry->opaque);

        rhashtable_remove_fast(&filter_ht, &entry->ht_node, filter_ht_params);
        kfree(entry);
    }

    return 0;
}

static bool tc_set_generic_rule(rdpa_ic_cfg_t *cfg, rdpa_ic_info_t *flow, rdpa_offset_t layer,
    uint32_t offset, uint32_t key, uint32_t mask)
{
    rdpa_ic_gen_rule_cfg_t *gen_rule_cfg;
    uint32_t *generic_key = NULL;
    uint32_t *generic_mask;

    if (!(cfg->field_mask & RDPA_IC_MASK_DST_MAC) && !(cfg->field_mask & RDPA_IC_MASK_GENERIC_1))
    {
        cfg->field_mask |= RDPA_IC_MASK_GENERIC_1;
        generic_key = &flow->key.generic_key_1;
        generic_mask = &flow->key.generic_mask;
        gen_rule_cfg = &cfg->gen_rule_cfg1;
    }
    else if (!(cfg->field_mask & RDPA_IC_MASK_SRC_MAC) && !(cfg->field_mask & RDPA_IC_MASK_GENERIC_2))
    {
        cfg->field_mask |= RDPA_IC_MASK_GENERIC_2;
        generic_key = &flow->key.generic_key_2;
        generic_mask = &flow->key.generic_mask_2;
        gen_rule_cfg = &cfg->gen_rule_cfg2;
    }
#if defined(RDP_UFC)
    else if (!(cfg->field_mask & RDPA_IC_MASK_SRC_IP) && !(cfg->field_mask & RDPA_IC_MASK_GENERIC_3))
    {
        cfg->field_mask |= RDPA_IC_MASK_GENERIC_3;
        generic_key = &flow->key.generic_key_3;
        generic_mask = &flow->key.generic_mask_3;
        gen_rule_cfg = &cfg->gen_rule_cfg3;
    }
    else if (!(cfg->field_mask & RDPA_IC_MASK_DST_IP) && !(cfg->field_mask & RDPA_IC_MASK_GENERIC_3))
    {
        cfg->field_mask |= RDPA_IC_MASK_GENERIC_4;
        generic_key = &flow->key.generic_key_4;
        generic_mask = &flow->key.generic_mask_4;
        gen_rule_cfg = &cfg->gen_rule_cfg4;
    }
#endif

    if (generic_key)
    {
        __debug("layer %d, offset %d, field_mask 0x%X, key 0x%X, mask 0x%X\n", (int)layer, offset,
            cfg->field_mask, key, mask);
        gen_rule_cfg->type = layer;
        gen_rule_cfg->offset = offset;
        *generic_key = key;
        *generic_mask = mask;
        return true;
    }

    return false;
}

static int tc_parse_if_flower_filter(struct flow_cls_offload *f, int *filter_idx, rdpa_filter_ctrl_t *filter_ctrl, uint32_t *filter_opaque)
{
    uint16_t addr_type = 0;
    uint16_t ip_proto = 0;
    uint16_t etype = 0;

    __debug("dissector 0x%X\n", f->rule->match.dissector->used_keys);

    if (tc_has_not_supported_keys(f->rule->match.dissector->used_keys))
    {
        NL_SET_ERR_MSG_MOD(f->common.extack, "Key not supported");
        return -EOPNOTSUPP;
    }

    if (flow_rule_match_key(f->rule, FLOW_DISSECTOR_KEY_BASIC))
    {
        struct flow_match_basic match;

        flow_rule_match_basic(f->rule, &match);

        ip_proto = match.key->ip_proto;
        etype = ntohs(match.key->n_proto);

        __debug("ip_proto %d, n_proto 0x%X\n", ip_proto, etype);

        switch (etype)
        {
        case ETH_P_ARP:
            *filter_idx = RDPA_FILTER_ETYPE_ARP;
            return 0;
        case ETH_P_PPP_DISC:
            *filter_idx = RDPA_FILTER_ETYPE_PPPOE_D;
            return 0;
        case ETH_P_PPP_SES:
            *filter_idx = RDPA_FILTER_ETYPE_PPPOE_S;
            return 0;
        case ETH_P_CFM:
            *filter_idx = RDPA_FILTER_ETYPE_802_1AG_CFM;
            return 0;
        case ETH_P_1588:
            *filter_idx = RDPA_FILTER_ETYPE_PTP_1588;
            return 0;
        case ETH_P_PAE:
            *filter_idx = RDPA_FILTER_ETYPE_802_1X;
            return 0;
        case ETH_P_IP:
            if (ip_proto == IPPROTO_IGMP)
            {
                *filter_idx = RDPA_FILTER_IGMP;
                return 0;
            }
            break;
        default:
            if (etype)
            {
                *filter_idx = tc_prio2filter_idx(f->common.prio);
                *filter_opaque = etype;
                return 0;
            }
        }
    }

    if (flow_rule_match_key(f->rule, FLOW_DISSECTOR_KEY_CONTROL))
    {
        struct flow_match_control match;

        flow_rule_match_control(f->rule, &match);

        __debug("thoff %d, addr_type %d, flags 0x%X\n", match.key->thoff, match.key->addr_type,
            match.key->flags);

        if (match.key->flags & FLOW_DIS_IS_FRAGMENT)
        {
            *filter_idx = RDPA_FILTER_IP_FRAG;
            return 0;
        }

        addr_type = match.key->addr_type;
    }

    if (addr_type == FLOW_DISSECTOR_KEY_IPV4_ADDRS)
    {
        static const uint32_t mcast_ctrl_addr = htonl(0xE0000000);
        static const uint32_t mcast_mask = 0xFFFFFFFF;
        struct flow_match_ipv4_addrs match;

        flow_rule_match_ipv4_addrs(f->rule, &match);

        __debug("src %pI4/%pI4, dst %pI4/%pI4\n", &match.key->src, &match.mask->src,
            &match.key->dst, &match.mask->dst);

        if (match.key->dst == mcast_ctrl_addr && match.mask->dst == mcast_mask)
        {
            *filter_idx = RDPA_FILTER_IP_MCAST_CONTROL;
            return 0;
        }

        if (match.key->dst & mcast_ctrl_addr)
        {
            *filter_idx = RDPA_FILTER_MCAST_IPV4;
            return 0;
        }
    }
    else if (addr_type == FLOW_DISSECTOR_KEY_IPV6_ADDRS)
    {
        uint32_t addr, mask;
        struct flow_match_ipv6_addrs match;

        flow_rule_match_ipv6_addrs(f->rule, &match);

        __debug("src %pI6/%pI6, dst %pI6/%pI6\n", &match.key->src, &match.mask->src,
            &match.key->dst, &match.mask->dst);

        addr = htonl(*(uint32_t *)&match.key->dst.in6_u.u6_addr32[0]);
        mask = htonl(*(uint32_t *)&match.mask->dst.in6_u.u6_addr32[0]);

        __debug("addr 0x%08X, mask 0x%08X\n", addr, mask);

        if((addr & mask & 0xFF02FFFF) >= 0xFF020000 )
        {
            *filter_idx = RDPA_FILTER_MCAST_IPV6;
            return 0;
        }
    }

    if (flow_rule_match_key(f->rule, FLOW_DISSECTOR_KEY_PORTS))
    {
        struct flow_match_ports match;
        uint16_t dst;

        flow_rule_match_ports(f->rule, &match);

        __debug("ip_proto %d, src %d, src mask 0x%X, dst %d, dst mask 0x%X\n", ip_proto,
            ntohs(match.key->src), ntohs(match.mask->src), ntohs(match.key->dst),
            ntohs(match.mask->dst));

        dst = ntohs(match.key->dst);
        if ((dst == 67 || dst == 68 || dst == 546 || dst == 547) && ip_proto == IPPROTO_UDP)
        {
            *filter_idx = RDPA_FILTER_DHCP;
            return 0;
        }
#if defined(XRDP_RGEN6)
        else if ((dst == 53 || ntohs(match.key->src) == 53) &&
            (ip_proto == IPPROTO_UDP || ip_proto == IPPROTO_TCP))
        {
            *filter_idx = RDPA_FILTER_DNS;
            return 0;
        }
#endif
        else if ((dst == 319 || dst == 320) && ip_proto == IPPROTO_UDP)
        {
            *filter_idx = RDPA_FILTER_L4_PTP_1588;
            return 0;
        }

        return -EOPNOTSUPP;
    }

    if (flow_rule_match_key(f->rule, FLOW_DISSECTOR_KEY_ICMP))
    {
        struct flow_match_icmp match;
        uint8_t type;

        flow_rule_match_icmp(f->rule, &match);

        type = match.key->type;

        __debug("icmp type %d, code %d, session %d\n", type, match.key->code, match.key->id);

        if (etype == ETH_P_IPV6)
        {
            if (type == 130 || type == 131 || type == 132)
                *filter_idx = RDPA_FILTER_MLD;
            else
                *filter_idx = RDPA_FILTER_ICMPV6;
            return 0;
        }
        else if (etype == ETH_P_IP)
        {
            if (type)
                *filter_idx = RDPA_FILTER_ICMPV4;
            else
                *filter_idx = RDPA_FILTER_ICMPV4_ECHO_REPLY;
            return 0;
        }
    }

    if (flow_rule_match_key(f->rule, FLOW_DISSECTOR_KEY_ETH_ADDRS))
    {
        static const char l2cp[] = { 0x01, 0x80, 0xC2, 0x00, 0x00 };
        static const char bcast[ETH_ALEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
        static const uint8_t mcast_l2_mask = 0x01;
        struct flow_match_eth_addrs match;

        flow_rule_match_eth_addrs(f->rule, &match);

        __debug("Src address %pM, mask %pM, Dst address %pM, mask %pM\n",
            &match.key->src, &match.mask->src, &match.key->dst, &match.mask->dst);

        if (!memcmp(&match.key->src, &match.key->dst, sizeof(match.key->src)))
        {
            *filter_idx = RDPA_FILTER_MAC_SPOOFING;
            return 0;
        }

        if (!memcmp(&match.key->dst, &l2cp, sizeof(l2cp)) && ((match.key->dst[5] & 0xF0) == 0))
        {
            *filter_idx = RDPA_FILTER_L2CP;
            return 0;
        }

        if (!memcmp(&match.key->dst, &bcast, sizeof(match.key->dst)))
        {
            *filter_idx = RDPA_FILTER_BCAST;
            return 0;
        }

        if (match.key->dst[0] == mcast_l2_mask)
        {
            *filter_idx = RDPA_FILTER_MCAST_L2;
            return 0;
        }

        if (match.key->dst[0] & match.mask->dst[0] & mcast_l2_mask)
        {
            *filter_idx = RDPA_FILTER_MCAST;
            return 0;
        }
    }

    return -EOPNOTSUPP;
}

static int tc_parse_ic_flower_filter(struct flow_cls_offload *f, rdpa_ic_cfg_t *cfg, rdpa_ic_info_t *flow)
{
    uint16_t addr_type = 0;
#ifdef CONFIG_BCM_KF_ENHANCED_TC
    int num_of_vlans = 0;
#endif

    __debug("dissector 0x%X\n", f->rule->match.dissector->used_keys);

    if (tc_has_not_supported_keys(f->rule->match.dissector->used_keys))
    {
        NL_SET_ERR_MSG_MOD(f->common.extack, "Key not supported");
        return -EOPNOTSUPP;
    }

    if (flow_rule_match_key(f->rule, FLOW_DISSECTOR_KEY_MARK))
    {
        struct flow_match_mark match;

        flow_rule_match_mark(f->rule, &match);

        cfg->field_mask |= RDPA_IC_MASK_GEM_FLOW;
        flow->key.gem_flow = SKBMARK_GET_PORT((match.key->mark & match.mask->mark));
    }

    if (flow_rule_match_key(f->rule, FLOW_DISSECTOR_KEY_BASIC))
    {
        struct flow_match_basic match;

        flow_rule_match_basic(f->rule, &match);

        __debug("ip_proto %d, n_proto 0x%X\n", match.key->ip_proto, ntohs(match.key->n_proto));

        if (match.mask->ip_proto)
        {
            cfg->field_mask |= RDPA_IC_MASK_IP_PROTOCOL;
            flow->key.protocol = match.key->ip_proto;
        }
        if (match.mask->n_proto)
        {
            cfg->field_mask |= RDPA_IC_MASK_ETHER_TYPE;
            flow->key.etype = ntohs(match.key->n_proto);
        }
    }

    if (flow_rule_match_key(f->rule, FLOW_DISSECTOR_KEY_CONTROL))
    {
        struct flow_match_control match;

        flow_rule_match_control(f->rule, &match);

        __debug("thoff %d, addr_type %d, flags 0x%X\n", match.key->thoff, match.key->addr_type,
            match.key->flags);

        addr_type = match.key->addr_type;
    }

    if (addr_type == FLOW_DISSECTOR_KEY_IPV4_ADDRS)
    {
        const uint32_t m_all = 0xFFFFFFFF;
        struct flow_match_ipv4_addrs match;

        flow_rule_match_ipv4_addrs(f->rule, &match);

        __debug("src %pI4/%pI4, dst %pI4/%pI4\n", &match.key->src, &match.mask->src,
            &match.key->dst, &match.mask->dst);

        if (match.mask->src)
        {
            cfg->field_mask |= RDPA_IC_MASK_SRC_IP;
            flow->key.src_ip.family = bdmf_ip_family_ipv4;
            flow->key.src_ip.addr.ipv4 = ntohl(match.key->src);
            if (match.mask->src != m_all)
            {
                flow->key.src_ip_mask.family = bdmf_ip_family_ipv4;
                flow->key.src_ip_mask.addr.ipv4 = ntohl(match.mask->src);
            }
        }
        if (match.mask->dst)
        {
            cfg->field_mask |= RDPA_IC_MASK_DST_IP;
            flow->key.dst_ip.family = bdmf_ip_family_ipv4;
            flow->key.dst_ip.addr.ipv4 = ntohl(match.key->dst);
            if (match.mask->dst != m_all)
            {
                flow->key.dst_ip_mask.family = bdmf_ip_family_ipv4;
                flow->key.dst_ip_mask.addr.ipv4 = ntohl(match.mask->dst);
            }
        }
    }
    else if (addr_type == FLOW_DISSECTOR_KEY_IPV6_ADDRS)
    {
        const char m_all[16] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
        struct flow_match_ipv6_addrs match;

        flow_rule_match_ipv6_addrs(f->rule, &match);

        __debug("src %pI6/%pI6, dst %pI6/%pI6\n", &match.key->src, &match.mask->src,
            &match.key->dst, &match.mask->dst);

        if (memcmp(&match.mask->src, (char[16]){0}, sizeof(match.key->src)))
        {
            cfg->field_mask |= RDPA_IC_MASK_SRC_IP;
            flow->key.src_ip.family = bdmf_ip_family_ipv6;
            be32_to_cpu_array((u32 *)&flow->key.src_ip.addr.ipv6.data,
                (const __be32 *)&match.key->src, sizeof(match.key->src));
            if (memcmp(&match.mask->src, m_all, sizeof(match.mask->src)))
            {
                flow->key.src_ip_mask.family = bdmf_ip_family_ipv6;
                be32_to_cpu_array((u32 *)&flow->key.src_ip_mask.addr.ipv6.data,
                    (const __be32 *)&match.mask->src, sizeof(match.mask->src));
            }
        }
        if (memcmp(&match.mask->dst, (char[16]){0}, sizeof(match.key->dst)))
        {
            cfg->field_mask |= RDPA_IC_MASK_DST_IP;
            flow->key.src_ip.family = bdmf_ip_family_ipv6;
            be32_to_cpu_array((u32 *)&flow->key.dst_ip.addr.ipv6.data,
                (const __be32 *)&match.key->dst, sizeof(match.key->dst));
            if (memcmp(&match.mask->dst, m_all, sizeof(match.mask->dst)))
            {
                flow->key.dst_ip_mask.family = bdmf_ip_family_ipv6;
                be32_to_cpu_array((u32 *)&flow->key.dst_ip_mask.addr.ipv6.data,
                    (const __be32 *)&match.mask->dst, sizeof(match.mask->dst));
            }
        }
    }

    if (flow_rule_match_key(f->rule, FLOW_DISSECTOR_KEY_PORTS))
    {
        struct flow_match_ports match;

        flow_rule_match_ports(f->rule, &match);

        __debug("src %d, src mask 0x%X, dst %d, dst mask 0x%X\n",
            ntohs(match.key->src), ntohs(match.mask->src), ntohs(match.key->dst),
            ntohs(match.mask->dst));

        if (ntohs(match.mask->src))
        {
            cfg->field_mask |= RDPA_IC_MASK_SRC_PORT;
            flow->key.src_port = ntohs(match.key->src);
        }
        if (ntohs(match.mask->dst))
        {
            cfg->field_mask |= RDPA_IC_MASK_DST_PORT;
            flow->key.dst_port = ntohs(match.key->dst);
        }
    }

    if (flow_rule_match_key(f->rule, FLOW_DISSECTOR_KEY_ETH_ADDRS))
    {
        static const uint8_t m_all[ETH_ALEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
#if defined(PARSER_BC_MC_SUPPORT)
        static const uint8_t mcast[ETH_ALEN] = { 0x01, 0, 0, 0, 0, 0 };
#endif
        struct flow_match_eth_addrs match;

        flow_rule_match_eth_addrs(f->rule, &match);

        __debug("Src address %pM, mask %pM, Dst address %pM, mask %pM\n",
            match.key->src, match.mask->src, match.key->dst, match.mask->dst);

        if (memcmp(match.mask->src, (char[ETH_ALEN]){0}, sizeof(match.mask->src)))
        {
            if (!memcmp(match.mask->src, m_all, sizeof(match.mask->src)))
            {
                cfg->field_mask |= RDPA_IC_MASK_SRC_MAC;
                memcpy(flow->key.src_mac.b, match.key->src, sizeof(match.key->src));
            }
            else
            {
                if (!tc_set_generic_rule(cfg, flow, RDPA_OFFSET_L2, ETH_ALEN, *(uint32_t *)match.key->src,
                                         *(uint32_t *)match.mask->src) ||
                    !tc_set_generic_rule(cfg, flow, RDPA_OFFSET_L2, ETH_ALEN + 4, *(uint16_t *)&match.key->src[4],
                                         *(uint16_t *)&match.mask->src[4]))
                {
                    return BDMF_ERR_TOO_MANY;
                }
            }
        }
        if (memcmp(match.mask->dst, (char[ETH_ALEN]){0}, sizeof(match.mask->dst)))
        {
            if (!memcmp(match.mask->dst, m_all, sizeof(match.mask->dst)))
            {
#if defined(PARSER_BC_MC_SUPPORT)
                if (!memcmp(match.key->dst, m_all, sizeof(match.key->dst)))
                {
                    cfg->field_mask |= RDPA_IC_MASK_BC;
                    flow->key.bc = 1;
                }
                else
#endif
                {
                    cfg->field_mask |= RDPA_IC_MASK_DST_MAC;
                    memcpy(flow->key.dst_mac.b, match.key->dst, sizeof(match.key->dst));
                }
            }
            else
            {
#if defined(PARSER_BC_MC_SUPPORT)
                if (!memcmp(match.key->dst, mcast, sizeof(match.key->dst)) &&
                    !memcmp(match.mask->dst, mcast, sizeof(match.mask->dst)))
                {
                    cfg->field_mask |= RDPA_IC_MASK_MC;
                    flow->key.mc = 1;
                }
                else if (!memcmp(match.key->dst, (char[ETH_ALEN]){0}, sizeof(match.key->dst)) &&
                    !memcmp(match.mask->dst, mcast, sizeof(match.mask->dst)))
                {
                    cfg->field_mask |= RDPA_IC_MASK_MC;
                    flow->key.mc = 0;
                }
                else
#endif
                {
                    if (!tc_set_generic_rule(cfg, flow, RDPA_OFFSET_L2, 0, *(uint32_t *)match.key->dst,
                                             *(uint32_t *)match.mask->dst) ||
                        !tc_set_generic_rule(cfg, flow, RDPA_OFFSET_L2, 4, *(uint16_t *)&match.key->dst[4],
                                             *(uint16_t *)&match.mask->dst[4]))
                    {
                        return BDMF_ERR_TOO_MANY;
                    }
                }
            }
        }
    }

#ifdef CONFIG_BCM_KF_ENHANCED_TC
    if (flow_rule_match_key(f->rule, FLOW_DISSECTOR_KEY_NUM_OF_VLANS))
    {
        struct flow_dissector_key_num_of_vlans *key = skb_flow_dissector_target(
            f->rule->match.dissector, FLOW_DISSECTOR_KEY_NUM_OF_VLANS, f->rule->match.key);

        num_of_vlans = key->num_of_vlans; 
        __debug("num_of_vlans %d\n", num_of_vlans);

        cfg->field_mask |= RDPA_IC_MASK_NUM_OF_VLANS;
        flow->key.number_of_vlans = key->num_of_vlans;
    }
#endif

    if (flow_rule_match_key(f->rule, FLOW_DISSECTOR_KEY_CVLAN))
    {
        struct flow_match_vlan match;

        flow_rule_match_cvlan(f->rule, &match);

        __debug("cvlan id %d, dei %d, prio %d, tci 0x%X, tpid %d, eth_type 0x%X\n",
            match.key->vlan_id, match.key->vlan_dei, match.key->vlan_priority,
            match.key->vlan_tci, match.key->vlan_tpid, match.key->vlan_eth_type);

        if (match.mask->vlan_id)
        {
            cfg->field_mask |= RDPA_IC_MASK_INNER_VID;
            flow->key.inner_vid = match.key->vlan_id;
        }
        if (match.mask->vlan_dei)
        {
#if !defined(RDP_UFC)
            uint16_t key = htons(match.key->vlan_dei<<12);
            uint16_t mask = htons(1<<12);
            if (!tc_set_generic_rule(cfg, flow, RDPA_OFFSET_L2, 18, key, mask))
                return BDMF_ERR_TOO_MANY;
#else
            cfg->field_mask |= RDPA_IC_MASK_INNER_CFI;
            flow->key.inner_cfi = match.key->vlan_dei;
#endif
        }
        if (match.mask->vlan_priority)
        {
            cfg->field_mask |= RDPA_IC_MASK_INNER_PBIT;
            flow->key.inner_pbits = match.key->vlan_priority;
            flow->key.inner_pbits_mask = match.mask->vlan_priority;
        }
        if (match.mask->vlan_tpid)
        {
            cfg->field_mask |= RDPA_IC_MASK_INNER_TPID;
            flow->key.inner_tpid = ntohs(match.key->vlan_tpid);
        }
        if (match.mask->vlan_eth_type)
        {
            cfg->field_mask |= RDPA_IC_MASK_ETHER_TYPE;
            flow->key.etype = ntohs(match.key->vlan_eth_type);
        }
    }

    if (flow_rule_match_key(f->rule, FLOW_DISSECTOR_KEY_VLAN))
    {
        struct flow_match_vlan match;

        flow_rule_match_vlan(f->rule, &match);

        __debug("vlan id %d, dei %d, prio %d, tci 0x%X, tpid %d, eth_type 0x%X\n",
            match.key->vlan_id, match.key->vlan_dei, match.key->vlan_priority,
            match.key->vlan_tci, match.key->vlan_tpid, match.key->vlan_eth_type);

        if (match.mask->vlan_id)
        {
            cfg->field_mask |= RDPA_IC_MASK_OUTER_VID;
            flow->key.outer_vid = match.key->vlan_id;
        }
        if (match.mask->vlan_dei)
        {
#if !defined(RDP_UFC)
            uint16_t key = htons(match.key->vlan_dei<<12);
            uint16_t mask = htons(1<<12);
            if (!tc_set_generic_rule(cfg, flow, RDPA_OFFSET_L2, 14, key, mask))
                return BDMF_ERR_TOO_MANY;
#else
            cfg->field_mask |= RDPA_IC_MASK_OUTER_CFI;
            flow->key.outer_cfi = match.key->vlan_dei;
#endif
        }
        if (match.mask->vlan_priority)
        {
            cfg->field_mask |= RDPA_IC_MASK_OUTER_PBIT;
            flow->key.outer_pbits = match.key->vlan_priority;
            flow->key.outer_pbits_mask = match.mask->vlan_priority;
        }
        if (match.mask->vlan_tpid)
        {
            cfg->field_mask |= RDPA_IC_MASK_OUTER_TPID;
            flow->key.outer_tpid = ntohs(match.key->vlan_tpid);
        }
        if (match.mask->vlan_eth_type)
        {
#ifdef CONFIG_BCM_KF_ENHANCED_TC
            if (num_of_vlans == 2)
            {
                cfg->field_mask |= RDPA_IC_MASK_INNER_TPID;
                flow->key.inner_tpid = ntohs(match.key->vlan_eth_type);
            }
            else
#endif
            {
                cfg->field_mask |= RDPA_IC_MASK_ETHER_TYPE;
                flow->key.etype = ntohs(match.key->vlan_eth_type);
            }
        }
    }

    if (flow_rule_match_key(f->rule, FLOW_DISSECTOR_KEY_FLOW_LABEL))
    {
        struct flow_dissector_key_tags *key = skb_flow_dissector_target(
            f->rule->match.dissector, FLOW_DISSECTOR_KEY_FLOW_LABEL, f->rule->match.key);

        __debug("flow label 0x%X\n",key->flow_label);

        cfg->field_mask |= RDPA_IC_MASK_IPV6_FLOW_LABEL;
        flow->key.ipv6_flow_label = key->flow_label;
    }

    if (flow_rule_match_key(f->rule, FLOW_DISSECTOR_KEY_IP))
    {
        struct flow_match_ip match;

        flow_rule_match_ip(f->rule, &match);

        __debug("key (tos %d, ttl %d) mask (tos 0x%X, ttl 0x%X)\n",
            match.key->tos, match.key->ttl, match.mask->tos, match.mask->ttl);

        if (match.mask->tos)
        {
            /* TOS covers DSCP hence no specific option for RDPA_IC_MASK_DSCP */
            cfg->field_mask |= RDPA_IC_MASK_TOS;
            flow->key.tos = match.key->tos;
        }
        if (match.mask->ttl)
        {
            if (!tc_set_generic_rule(cfg, flow, RDPA_OFFSET_L3, 8, match.key->ttl, 0xFF))
                return BDMF_ERR_TOO_MANY;
        }
    }

    if (flow_rule_match_key(f->rule, FLOW_DISSECTOR_KEY_ICMP))
    {
        struct flow_match_icmp match;

        flow_rule_match_icmp(f->rule, &match);

        __debug("icmp type %d, code %d, session %d\n",
            match.key->type, match.key->code, match.key->id);

        if (!tc_set_generic_rule(cfg, flow, RDPA_OFFSET_L4, 0, match.key->code | (match.key->type << 8), 0xFFFF))
            return BDMF_ERR_TOO_MANY;
    }

    if (flow_rule_match_key(f->rule, FLOW_DISSECTOR_KEY_ARP))
    {
        struct flow_dissector_key_arp *mask= skb_flow_dissector_target(f->rule->match.dissector,
            FLOW_DISSECTOR_KEY_ARP, f->rule->match.mask);
        struct flow_dissector_key_arp *key = skb_flow_dissector_target(f->rule->match.dissector,
            FLOW_DISSECTOR_KEY_ARP, f->rule->match.key);

        __debug("arp sip 0x%X, tip 0x%X, op %d\n",
            (uint32_t)key->sip, (uint32_t)key->tip, key->op);

        if (mask->sip)
        {
            if (!tc_set_generic_rule(cfg, flow, RDPA_OFFSET_L3, 14, key->sip, 0xFFFFFFFF))
                return BDMF_ERR_TOO_MANY;
        }
        if (mask->tip)
        {
            if (!tc_set_generic_rule(cfg, flow, RDPA_OFFSET_L3, 24, key->tip, 0xFFFFFFFF))
                return BDMF_ERR_TOO_MANY;
        }
        if (mask->op)
        {
            if (!tc_set_generic_rule(cfg, flow, RDPA_OFFSET_L3, 7, key->op, 0xFF))
                return BDMF_ERR_TOO_MANY;
        }
        if (memcmp(&mask->sha, (char[ETH_ALEN]){0}, sizeof(mask->sha)))
        {
            if (!tc_set_generic_rule(cfg, flow, RDPA_OFFSET_L3, 8, *((uint32_t *)&key->sha[0]), 0xFFFFFFFF))
                return BDMF_ERR_TOO_MANY;
            if (!tc_set_generic_rule(cfg, flow, RDPA_OFFSET_L3, 8+4, *((uint16_t *)&key->sha[4]), 0xFFFF))
                return BDMF_ERR_TOO_MANY;
        }
        if (memcmp(&mask->tha, (char[ETH_ALEN]){0}, sizeof(mask->tha)))
        {
            if (!tc_set_generic_rule(cfg, flow, RDPA_OFFSET_L3, 8, *((uint32_t *)&key->tha[0]), 0xFFFFFFFF))
                return BDMF_ERR_TOO_MANY;
            if (!tc_set_generic_rule(cfg, flow, RDPA_OFFSET_L3, 8+4, *((uint16_t *)&key->tha[4]), 0xFFFF))
                return BDMF_ERR_TOO_MANY;
        }
    }

    if (flow_rule_match_key(f->rule, FLOW_DISSECTOR_KEY_TCP))
    {
        struct flow_match_tcp match;

        flow_rule_match_tcp(f->rule, &match);

        __debug("flags 0x%X/0x%X\n", ntohs(match.key->flags), ntohs(match.mask->flags));

        if (!tc_set_generic_rule(cfg, flow, RDPA_OFFSET_L4, 12, match.key->flags, 0xFFF))
            return BDMF_ERR_TOO_MANY;
    }

    return BDMF_ERR_OK;
}

static int tc_validate_filter_input(struct netlink_ext_ack *extack, unsigned int num_actions,
    u32 chain_index, u32 priority)
{
    __debug("num_actions %d, chain_idx %d, priority %d\n", num_actions, chain_index, priority);

    if (chain_index != TC_CHAIN_ID_IC_RX &&
        chain_index != TC_CHAIN_ID_IC_PRIMARY &&
        chain_index != TC_CHAIN_ID_IC_FLOW_MISS &&
        chain_index != TC_CHAIN_ID_IF)
    {
        NL_SET_ERR_MSG(extack, "Chain not supported");
        return -EOPNOTSUPP;
    }

    if (num_actions != 1 && chain_index == TC_CHAIN_ID_IF)
    {
        NL_SET_ERR_MSG(extack, "Only single action is supported for chain");
        return -EOPNOTSUPP;
    }

    if (num_actions > 2 && chain_index != TC_CHAIN_ID_IF)
    {
        NL_SET_ERR_MSG(extack, "Up to 2 actions are supported for chain");
        return -EOPNOTSUPP;
    }

    if ((priority > 43 && chain_index != TC_CHAIN_ID_IF) ||
        (tc_prio2filter_idx(priority) == RDPA_FILTER_MAX && chain_index == TC_CHAIN_ID_IF))
    {
        NL_SET_ERR_MSG(extack, "Max priority supported exceeded");
        return -EOPNOTSUPP;
    }

    return 0;
}

static int tc_flow_actions_to_rdpa_action(int num_entries, struct flow_action_entry actions[],
    struct filter_action *action)
{
    int rc;
    int i;

    memset(action, 0, sizeof(*action));
    for (i = 0; i < num_entries; i++)
    {
        rc = tc_update_rdpa_action(&actions[i], action);
        if (rc)
            return rc;
    }

    return 0;
}

#if defined(CONFIG_BCM_GDX_HW)
extern bcm_netdev_priv_info_get_cb_fn_t blog_gdx_dev_cb_fn;
#endif
static int tc_dev2ingress_port(struct flow_cls_offload *f, struct net_device *dev,
                               bdmf_object_handle *ingress_port_handle, rdpa_traffic_dir *dir)
{
    /* flower uses meta to pass indev device ifindex */
    if (flow_rule_match_key(f->rule, FLOW_DISSECTOR_KEY_META))
    {
#if defined(CONFIG_BCM_GDX_HW)
        int rc;
        struct net_device *indev;
        struct flow_match_meta match;
        bcm_netdev_priv_info_out_t info_out = {};

        if (!is_netdev_tc_all_ports(dev))
        {
            NL_SET_ERR_MSG(f->common.extack, "Must be bcmsw to configure GDX");
            return -EOPNOTSUPP;
        }

        flow_rule_match_meta(f->rule, &match);

        indev = dev_get_by_index(&init_net, match.key->ingress_ifindex);
        if (!indev || !is_netdev_hw_accel_gdx(indev))
            return -EOPNOTSUPP;

        if (!blog_gdx_dev_cb_fn)
        {
            NL_SET_ERR_MSG(f->common.extack, "GDX not configured");
            return -EOPNOTSUPP;
        }

        rc = blog_gdx_dev_cb_fn(indev, BCM_NETDEV_TO_RDPA_PORT_OBJ, &info_out);
        if (rc)
        {
            NL_SET_ERR_MSG(f->common.extack, "Failed to get gdx port");
            return -1;
        }

        *ingress_port_handle = info_out.bcm_netdev_to_rdpa_port_obj.rdpa_port_obj;
        *dir = tc_dev2rdpa_dir(indev);
        return 0;
#else
        return -EOPNOTSUPP;
#endif
    }

    *ingress_port_handle = tc_dev2rdpa_port_obj(dev);
    *dir = tc_dev2rdpa_dir(dev);
    return 0;
}

static int tc_alloc_and_initialize_filter_entry(struct netlink_ext_ack *extack, bdmf_object_handle ingress_port,
    rdpa_traffic_dir dir, uint32_t chain_index, struct filter_action *act_ent, uint32_t priority,
    struct filter_entry **entry)
{
    struct filter_entry *e;

    __debug("action %d (reason %d, iq_priority %d), chain_idx %d, priority %d\n",
        (int)act_ent->action, act_ent->reason, act_ent->iq_priority, chain_index, priority);

    if (act_ent->action == rdpa_forward_action_none ||
        (act_ent->action == rdpa_forward_action_skip && chain_index == TC_CHAIN_ID_IF))
    {
        NL_SET_ERR_MSG(extack, "Action not supported for chain");
        return -EOPNOTSUPP;
    }

    if (act_ent->reason > 15 || act_ent->reason < 0)
    {
        NL_SET_ERR_MSG(extack, "Trap reason out of range");
        return -EINVAL;
    }

    e = kzalloc(sizeof(*e), GFP_KERNEL);
    if (!e)
        return -ENOMEM;

    e->dir = dir;
    e->ingress_port = ingress_port;

    if (chain_index == TC_CHAIN_ID_IF)
    {
        e->filter_idx = RDPA_FILTER_MAX;
        e->filter_ctrl.action = act_ent->action;
    }
    else
    {
        switch (chain_index)
        {
        case TC_CHAIN_ID_IC_RX:
            e->cfg.generic_filter_location = RDPA_ALL_TRAFFIC_RX;
            break;
        case TC_CHAIN_ID_IC_PRIMARY:
            e->cfg.generic_filter_location = RDPA_ALL_TRAFFIC;
            break;
        case TC_CHAIN_ID_IC_FLOW_MISS:
            e->cfg.generic_filter_location = RDPA_FLOW_MISSED_TRAFFIC;
            break;
        }

        e->cfg.type = RDPA_IC_TYPE_GENERIC_FILTER;
        /* TC 1 priority is highest (range 1-43). RDPA 0 priority is lowest */
        e->cfg.prty = 43 - priority; 
        if (ingress_port)
        {
            e->flow.key.port_ingress_obj = ingress_port;
            e->cfg.field_mask |=  RDPA_IC_MASK_INGRESS_PORT;
        }
        e->flow.result.action = act_ent->action;
        if (act_ent->action == rdpa_forward_action_host)
            e->flow.result.trap_reason = act_ent->reason + rdpa_cpu_rx_reason_udef_0;
        if (act_ent->iq_priority)
            e->flow.result.iq_priority = rdpa_iq_priority_high;
    }

    *entry = e;

    return 0;
}

static int tc_ingress_flower_replace(struct net_device *dev, struct flow_cls_offload *f)
{
    int rc;
    struct filter_entry *entry;
    int chain_index = f->common.chain_index;
    struct filter_action act_ent;
    bdmf_object_handle ingress_port;
    rdpa_traffic_dir dir;

    __debug("chain_idx %d, protocol 0x%X, prio %d, cookie 0x%X\n", chain_index,
        ntohs(f->common.protocol), f->common.prio, f->cookie);

    rc = tc_validate_filter_input(f->common.extack, f->rule->action.num_entries, chain_index, f->common.prio);
    rc = rc ? rc : tc_flow_actions_to_rdpa_action(f->rule->action.num_entries, f->rule->action.entries, &act_ent);
    rc = rc ? rc : tc_dev2ingress_port(f, dev, &ingress_port, &dir);
    rc = rc ? rc : tc_alloc_and_initialize_filter_entry(f->common.extack, ingress_port, dir, chain_index, &act_ent,
        f->common.prio, &entry);
    if (rc)
        return rc;

    if (chain_index == TC_CHAIN_ID_IF)
    {
        rc = tc_parse_if_flower_filter(f, &entry->filter_idx, &entry->filter_ctrl, &entry->opaque);
        if (!rc && entry->filter_idx != tc_prio2filter_idx(f->common.prio))
            rc = -EINVAL;
        rc = rc ? rc : tc_add_if_rule(entry, f->cookie);
    }
    else
    {
        rc = tc_parse_ic_flower_filter(f, &entry->cfg, &entry->flow);
        rc = rc ? rc : tc_add_ic_rule(entry, f->cookie);
    }

    if (rc)
    {
        __debug("failed %d\n", rc);
        kfree(entry);
    }
    return rc;
}

static int tc_ingress_flower_destroy(struct net_device *dev, struct flow_cls_offload *f)
{
    __debug("chain_idx %d, cookie 0x%X\n", f->common.chain_index, f->cookie);

    if (f->common.chain_index == TC_CHAIN_ID_IF)
        return tc_del_if_rule(f->cookie);

    return tc_del_ic_rule(f->cookie);
}

static int tc_ingress_matchall_replace(struct net_device *dev, struct tc_cls_matchall_offload *f)
{
    int rc;
    struct filter_entry *entry;
    int chain_index = f->common.chain_index;
    struct filter_action act_ent;

    __debug("chain_idx %d, protocol 0x%X, prio %d\n", chain_index, ntohs(f->common.protocol),
        f->common.prio);

    /* Skip chain 0 configuration. Do not return an error here to prevent the rollback of other chain 0 rules. */
    if (chain_index == 0)
        return 0;

    rc = tc_validate_filter_input(f->common.extack, f->rule->action.num_entries, chain_index, f->common.prio);
    if (rc)
        return rc;
    
    if (chain_index == TC_CHAIN_ID_IF)
    {
        NL_SET_ERR_MSG(f->common.extack, "Chain not supported");
        return -EOPNOTSUPP;
    }

    rc = tc_flow_actions_to_rdpa_action(f->rule->action.num_entries, f->rule->action.entries, &act_ent);
    if (rc)
        return rc;

    rc = tc_alloc_and_initialize_filter_entry(f->common.extack, tc_dev2rdpa_port_obj(dev), tc_dev2rdpa_dir(dev),
        chain_index, &act_ent, f->common.prio, &entry);
    if (rc)
        return rc;

#if defined(RDP_UFC)
    entry->cfg.field_mask |= RDPA_IC_MASK_ANY;
#else
    if (entry->cfg.field_mask == 0)
        entry->cfg.field_mask = RDPA_IC_MASK_ANY;
    else if (!tc_set_generic_rule(&entry->cfg, &entry->flow, RDPA_OFFSET_L2, 0, 0, 0))
        return -ENOENT;
#endif

    return tc_add_ic_rule(entry, f->cookie);
}

static int tc_action_to_rdpa_action(struct tc_action *act, struct filter_action *a)
{
    if (is_tcf_gact_ok(act)) 
        a->action = rdpa_forward_action_skip;
    else if (is_tcf_gact_shot(act))
        a->action = rdpa_forward_action_drop;
    else if (is_tcf_gact_trap(act))
    {
        a->action = rdpa_forward_action_host;
        a->reason = 0;
    }
    else if (is_tcf_gact_rtrap(act))
    {
        a->action = rdpa_forward_action_host;
        a->reason = tcf_gact_rtrap_reason(act);
    }
    else if (is_tcf_gact_iq_prio(act))
        a->iq_priority = tcf_gact_iq_prio_priority(act) ?  rdpa_iq_priority_high :  rdpa_iq_priority_low;
    else
        return -1;

    return 0;
}

static int tc_actions_to_rdpa_action(int num_entries, struct tc_action **actions, struct filter_action *action)
{
    int rc;
    int i;

    memset(action, 0, sizeof(*action));
    for (i = 0; i < num_entries; i++)
    {
        struct tc_action * a = actions[i];
        rc = tc_action_to_rdpa_action(a, action);
        if (rc)
        {
#if defined(CC_TC_DEBUG_ENABLE)
            if (!(a->ops && a->ops->id != TCA_ID_GACT))
            {
                struct tcf_gact *gact;
                gact = to_gact(a);
                __debug("unsupported gaction %X\n", gact->tcf_action);
            }
            else
            {
                __debug("unsupported action %i, num_entries %i\n", i, num_entries);
            }
#endif
            return rc;
        }
    }

    return 0;
}

static int tc_parse_u32_filter(struct tc_cls_u32_offload *f, rdpa_ic_cfg_t *cfg, rdpa_ic_info_t *flow)
{
    int i, off;
    __be32 val, m;
    rdpa_offset_t layer;

    if (f->knode.mask == SKBMARK_PORT_M)
    {
        cfg->field_mask |= RDPA_IC_MASK_GEM_FLOW;
        flow->key.gem_flow = SKBMARK_GET_PORT(f->knode.val);
    }

    __debug("protocol 0x%X\n",ntohs(f->common.protocol));

    switch (ntohs(f->common.protocol))
    {
    case ETH_P_ALL:
        layer = RDPA_OFFSET_L2;
        break;
    default:
        layer = RDPA_OFFSET_L2;
        val = f->common.protocol;
        m = 0xFFFF;
        off = 10;
        if (!tc_set_generic_rule(cfg, flow, layer, off, htons(val), htons(m)))
            return -ENODATA;
        layer = RDPA_OFFSET_L3;
        break;
    }

    for (i = 0; i < f->knode.sel->nkeys; i++)
    {
        off = f->knode.sel->keys[i].off;
        val = f->knode.sel->keys[i].val;
        m = f->knode.sel->keys[i].mask;
        if (!tc_set_generic_rule(cfg, flow, layer, off, htonl(val), htonl(m)))
            return -ENODATA;
    }
    
    return 0;
}

static int tc_ingress_u32_replace(struct net_device *dev, struct tc_cls_u32_offload *f)
{
    int rc;
    struct filter_entry *entry;
    int chain_index = f->common.chain_index;
    struct filter_action act_ent;

    __debug("chain_idx %d, protocol 0x%X, prio %d\n", chain_index, ntohs(f->common.protocol),
        f->common.prio);

    rc = tc_validate_filter_input(f->common.extack, tcf_exts_num_actions(f->knode.exts),
        chain_index, f->common.prio);
    if (rc)
    {
        __debug("Validation failed (%d)\n", rc);
        return rc;
    }

    if (chain_index == TC_CHAIN_ID_IF)
    {
        NL_SET_ERR_MSG(f->common.extack, "Chain not supported");
        return -EOPNOTSUPP;
    }

    rc = tc_actions_to_rdpa_action(tcf_exts_num_actions(f->knode.exts), f->knode.exts->actions, &act_ent);
    if (rc)
        return rc;

    rc = tc_alloc_and_initialize_filter_entry(f->common.extack, tc_dev2rdpa_port_obj(dev), tc_dev2rdpa_dir(dev),
        chain_index, &act_ent, f->common.prio, &entry);
    if (rc)
    {
        __debug("Rule allocation failed (%d)\n", rc);
        return rc;
    }

    __debug("handle 0x%X, val %d, mask 0x%X, l_handle %d, fshift %d\n",
        (uint32_t)f->knode.handle, f->knode.val, f->knode.mask, f->knode.link_handle, f->knode.fshift);

    rc = tc_parse_u32_filter(f, &entry->cfg, &entry->flow);
    if (!rc)
        rc = tc_add_ic_rule(entry, f->knode.handle);

    if (rc)
        kfree(entry);
    return rc;
}

static int tc_ingress_setup_block_cb(enum tc_setup_type type, void *type_data, void *cb_priv)
{
    __debug("filter type %d\n", type);
    switch (type)
    {
    case TC_SETUP_CLSFLOWER:
    {
        struct flow_cls_offload *f = (struct flow_cls_offload *)type_data;
        __debug("filter command %d\n", f->command);
        switch (f->command)
        {
        case FLOW_CLS_REPLACE:
            return tc_ingress_flower_replace(cb_priv, f);
        case FLOW_CLS_DESTROY:
            return tc_ingress_flower_destroy(cb_priv, f);
        case FLOW_CLS_STATS:
            return tc_update_ic_stats(f->common.chain_index, f->cookie, &f->stats);
        default:
            return -EOPNOTSUPP;
        }
    }
    case TC_SETUP_CLSMATCHALL:
    {
        struct tc_cls_matchall_offload *f = (struct tc_cls_matchall_offload *)type_data;
        __debug("filter command %d\n", f->command);
        switch (f->command)
        {
        case TC_CLSMATCHALL_REPLACE:
            return tc_ingress_matchall_replace(cb_priv, f);
        case TC_CLSMATCHALL_DESTROY:
            return tc_del_ic_rule(f->cookie);
        case TC_CLSMATCHALL_STATS:
            return tc_update_ic_stats(f->common.chain_index, f->cookie,
                &f->stats);
        default:
            return -EOPNOTSUPP;
        }
    }
    case TC_SETUP_CLSU32:
    {
        struct tc_cls_u32_offload *f = (struct tc_cls_u32_offload *)type_data;
        __debug("filter command %d\n", f->command);
        switch (f->command) 
        {
        case TC_CLSU32_NEW_KNODE:
        case TC_CLSU32_REPLACE_KNODE: 
            return tc_ingress_u32_replace(cb_priv, f);
        case TC_CLSU32_DELETE_KNODE:
            return tc_del_ic_rule(f->knode.handle);

        case TC_CLSU32_NEW_HNODE:
        case TC_CLSU32_REPLACE_HNODE:
            __debug("handle 0x%X, prio %d, divisor %d\n",
                (uint32_t)f->hnode.handle, f->hnode.prio, f->hnode.divisor);
            return -EOPNOTSUPP;
        case TC_CLSU32_DELETE_HNODE:
            __debug("handle 0x%X, prio %d, divisor %d\n",
                (uint32_t)f->hnode.handle, f->hnode.prio, f->hnode.divisor);
            return -EOPNOTSUPP;
        default:
            return -EOPNOTSUPP;
        }
    }
    default:
        /* TC requires all offload callbacks not to fail in case of skip_sw is used otherwise it fails the call. */
        return 0;
    }
}

int tc_ingress_init(void)
{
    return rhashtable_init(&filter_ht, &filter_ht_params);
}

void tc_ingress_exit(void)
{
    rhashtable_destroy(&filter_ht);
}

static LIST_HEAD(block_cb_list);

int tc_ingress_setup_block(struct net_device *dev, void *type_data)
{
    __debug("Registering ingress TC (dev %s)\n", dev->name);
    return flow_block_cb_setup_simple(type_data, &block_cb_list, tc_ingress_setup_block_cb, dev, dev, true);
}
EXPORT_SYMBOL(tc_ingress_setup_block);
