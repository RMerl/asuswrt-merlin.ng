/*
* <:copyright-BRCM:2013:DUAL/GPL:standard
*
*    Copyright (c) 2013 Broadcom
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

#include "bcm_OS_Deps.h"
#include <linux/bcm_log.h>
#include <rdpa_api.h>
#if defined(CONFIG_BLOG)
#include <linux/blog_rule.h>
#include <linux/vlanctl_bind.h>
#include "rdpa_mw_blog_parse.h"
#include "rdpa_mw_qos.h"
#endif
#include "rdpa_mw_vlan.h"
#include "br_private.h"


/* taken from bcm_vlan.h: */
#define BCM_VLAN_MAX_TPID_VALUES 4

/* taken from bcm_vlan_local.h: */
#define BCM_VLAN_PBITS_MASK 0xE000
#define BCM_VLAN_PBITS_SHIFT 13
#define BCM_VLAN_VID_MASK 0x0FFF
#define BCM_VLAN_CFI_MASK 0x1000
#define BCM_VLAN_FILTER_FLAGS_IS_UNICAST   0x0001
#define BCM_VLAN_FILTER_FLAGS_IS_MULTICAST 0x0002
#define BCM_VLAN_FILTER_FLAGS_IS_BROADCAST 0x0004


#define protoLogDebug bcmLog_logIsEnabled(BCM_LOG_ID_RDPA, BCM_LOG_LEVEL_DEBUG)
#define protoDebug(fmt, arg...) BCM_LOG_DEBUG(BCM_LOG_ID_RDPA, fmt, ##arg)
#define protoInfo(fmt, arg...) BCM_LOG_INFO(BCM_LOG_ID_RDPA, fmt, ##arg)

#define BLOG_KEY_FROM_DIR_IC_FLOW(dir, ingress_class_idx, flow_idx, egress_port) \
    (dir << 31 | egress_port << 24 | ingress_class_idx << 16 | flow_idx)
#define BLOG_HASH_FROM_KEY(blog_key) (blog_key & ~BLOG_KEY_FROM_DIR_IC_FLOW(0, 0, 0, 0x3F))
#define DIR_FROM_BLOG_KEY(blog_key) (blog_key >> 31)
#define EGRESS_PORT_FROM_BLOG_KEY(blog_key) ((blog_key >> 24) & 0x3F)
#define IC_IDX_FROM_BLOG_KEY(blog_key) ((blog_key >> 16) & 0xFF)
#define FLOW_IDX_FROM_BLOG_KEY(blog_key) (blog_key & 0xFFFF)

int rdpa_mw_packet_based_forward = 1; /* Packet based forwarding is always in use */
int rdpa_init_system(void);

static bdmf_object_handle rdpa_system = NULL;
static rdpa_ip_class_method method;

#if defined(CONFIG_BLOG)
struct blog_flow_entry
{
    DLIST_ENTRY(blog_flow_entry) list;
    uint32_t hash; /* blog_key without the egress_port set */
    uint8_t ref_cnt;
};

typedef struct blog_flow_entry blog_flow_entry_t;
DLIST_HEAD(blog_flow_list_t, blog_flow_entry);
static struct blog_flow_list_t blog_flow_list;

/* Compare ALL ic flow fields */
static int compare_ic_flows(rdpa_ic_info_t *a, rdpa_ic_info_t *b)
{
#define COMPARE(field) do \
    { \
        if (a->field != b->field) \
        { \
            protoDebug("mismatch at %s %ld %ld offset 0x%x", #field, (long)a->field, (long)b->field, \
                       (uint32_t)offsetof(rdpa_ic_info_t, field));      \
            return -1; \
        } \
    } while (0)

    if (bdmf_ip_cmp(&a->key.src_ip, &b->key.src_ip))
        return -1;
    if (bdmf_ip_cmp(&a->key.dst_ip, &b->key.dst_ip))
        return -1;
    COMPARE(key.src_port);
    COMPARE(key.dst_port);
    COMPARE(key.protocol);
    COMPARE(key.outer_vid);
    COMPARE(key.inner_vid);
    if (memcmp(&a->key.dst_mac, &b->key.dst_mac, sizeof(a->key.dst_mac)))
        return -1;
    if (memcmp(&a->key.src_mac, &b->key.src_mac, sizeof(a->key.src_mac)))
        return -1;
    COMPARE(key.etype);
    COMPARE(key.dscp);
    COMPARE(key.ssid);
    COMPARE(key.ingress_port);
    COMPARE(key.gem_flow);
    COMPARE(key.outer_pbits);
    COMPARE(key.inner_pbits);
    COMPARE(key.number_of_vlans);
    COMPARE(key.ipv6_flow_label);
    COMPARE(key.outer_tpid);
    COMPARE(key.inner_tpid);
    COMPARE(key.l3_protocol);
    COMPARE(result.qos_method);
    COMPARE(result.wan_flow);
    COMPARE(result.action);
    COMPARE(result.policer);
    COMPARE(result.forw_mode);
    COMPARE(result.egress_port);
    COMPARE(result.queue_id);
    COMPARE(result.vlan_action);
    COMPARE(result.opbit_remark);
    COMPARE(result.opbit_val);
    COMPARE(result.ipbit_remark);
    COMPARE(result.ipbit_val);
    COMPARE(result.dscp_remark);
    COMPARE(result.dscp_val);
    COMPARE(result.ecn_val);
    COMPARE(result.pbit_to_gem_table);

    return 0;
}

static int blog_rule_mapping_get(blogRule_t *blog_rule, blogRuleCommand_t req_cmd, int *out)
{
    int i;

    for (i = 0; i < blog_rule->actionCount; i++)
    {
        blogRuleCommand_t cmd;

        cmd = blog_rule->action[i].cmd;
        if (cmd != req_cmd)
            continue;

        switch (cmd)
        {
            case BLOG_RULE_CMD_SET_SKB_MARK_PORT:
                *out = blog_rule->action[i].skbMarkPort;
                return 0;
            case BLOG_RULE_CMD_SET_SKB_MARK_QUEUE:
                *out = blog_rule->action[i].skbMarkQueue;
                return 0;
            case BLOG_RULE_CMD_SET_DSCP:
                *out = blog_rule->action[i].dscp;
                return 0;
            case BLOG_RULE_CMD_DROP:
                return 0;
            case BLOG_RULE_CMD_SET_DEI:
                *out = blog_rule->action[i].dei == 1 ? RDPA_IC_DEI_SET : RDPA_IC_DEI_CLEAR;
                return 0;
            default:
                break;
        }
    }

    return -1;
}

/* lookup if we have an existing flow with the same key/result besides vlan_action. Assume vlan_action in flow is not
   set */
static void match_ic_flow(bdmf_object_handle ingress_class_obj, rdpa_ic_info_t *flow, bdmf_index *flow_idx)
{
    rdpa_ic_info_t flow_tmp;

    *flow_idx = BDMF_INDEX_UNASSIGNED;
    while (rdpa_ingress_class_flow_get_next(ingress_class_obj, flow_idx) != BDMF_ERR_NO_MORE)
    {
        rdpa_ingress_class_flow_get(ingress_class_obj, *flow_idx, &flow_tmp);
        flow_tmp.result.vlan_action = NULL;
        if (!compare_ic_flows(&flow_tmp, flow))
        {
            protoDebug("IC flow match found idx(%d)", (int)*flow_idx);
            return;
        }
    }

    *flow_idx = BDMF_INDEX_UNASSIGNED;
    protoDebug("IC flow no match found");
}

static int ds_dal_enabled_get(rdpa_if wan_port, int *enabled)
{
    bdmf_object_handle port_obj = NULL;
    rdpa_port_dp_cfg_t cfg;
    int rc;

    rc = rdpa_port_get(wan_port, &port_obj);
    if (rc)
        BDMF_TRACE_RET(rc, "Failed to get wan interface object\n");

    rdpa_port_cfg_get(port_obj, &cfg);
    *enabled = cfg.dal_enable;
    bdmf_put(port_obj);

    return 0;
}

static int configure_action_per_port(bdmf_object_handle ingress_class_obj, uint32_t flow_idx, rdpa_if port,
    bdmf_object_handle vlan_action_obj, int drop)
{
    rdpa_port_action_key_t pa_key = { .flow = flow_idx, .port = port };
    rdpa_port_action_t pa = { .vlan_action = vlan_action_obj, .drop = drop };
    int rc;

    protoDebug("per-port action config: flow_idx %d, port %d, drop %d vlan %p", (int)flow_idx, (int)port, drop,
        vlan_action_obj);
    rc = rdpa_ingress_class_port_action_set(ingress_class_obj, &pa_key, &pa);
    if (rc < 0)
        BDMF_TRACE_RET(rc, "Failed to update port action flow attribute\n");

    return 0;
}

static int deconfigure_action_per_port(bdmf_object_handle ingress_class_obj, uint32_t flow_idx, rdpa_if port)
{
    rdpa_port_action_key_t pa_key = { .flow = flow_idx, .port = port };
    rdpa_port_action_t pa ;
    int rc;

    rc = rdpa_ingress_class_port_action_get(ingress_class_obj, &pa_key, &pa);

    /* if no vlan_action is configured with this flow_idx and port */
    if (rc == BDMF_ERR_NOENT)
        return 0;

    /* if other error returned during find port action */
    if (rc < 0)
        BDMF_TRACE_RET(rc, "get port action returns %d\n", rc);

    /* if port action is found */
    if (pa.vlan_action)
        bdmf_put(pa.vlan_action);

    return 0;
}

static int add_modify_ic_flow(uint32_t field_mask, Blog_t *blog, bdmf_object_handle ingress_class_obj, rdpa_traffic_dir dir,
    uint32_t *flow_idx_p, rdpa_if *egress_port)
{
    int rc = BDMF_ERR_OK, is_drop_rule, blog_out;
    bdmf_index flow_idx = BDMF_INDEX_UNASSIGNED;
    rdpa_ic_info_t flow = {};
    blogRule_t *blog_rule = blog->blogRule_p;
    blogRuleFilter_t *filter = &blog_rule->filter;
    bdmf_object_handle vlan_action_obj = NULL;
    uint32_t blog_queue_id = 0;
    rdpa_if ingress_port;

    is_drop_rule = !blog_rule_mapping_get(blog_rule, BLOG_RULE_CMD_DROP, NULL);

    if (field_mask & RDPA_IC_MASK_DST_MAC)
        memcpy(&flow.key.dst_mac, filter->eth.value.h_dest, sizeof(flow.key.dst_mac));

    if (field_mask & RDPA_IC_MASK_SRC_MAC)
        memcpy(&flow.key.src_mac, filter->eth.value.h_source, sizeof(flow.key.src_mac));

    if (field_mask & RDPA_IC_MASK_ETHER_TYPE)
    {
        if (filter->nbrOfVlanTags == 2)
            flow.key.etype = filter->vlan[1].value.h_vlan_encapsulated_proto;
        else if (filter->nbrOfVlanTags == 1)
            flow.key.etype = filter->vlan[0].value.h_vlan_encapsulated_proto;
        else
            flow.key.etype = filter->eth.value.h_proto;
    }

    if (field_mask & RDPA_IC_MASK_IP_PROTOCOL)
        flow.key.protocol = filter->ipv4.value.ip_proto;

    if (field_mask & RDPA_IC_MASK_DSCP)
        flow.key.dscp = blog_rule_filterInUse(filter->ipv6.mask.tclass) ? filter->ipv6.value.tclass : filter->ipv4.value.tos;

    if (field_mask & RDPA_IC_MASK_OUTER_PBIT)
        flow.key.outer_pbits = (filter->vlan[0].value.h_vlan_TCI & BCM_VLAN_PBITS_MASK) >> BCM_VLAN_PBITS_SHIFT;

    if (field_mask & RDPA_IC_MASK_INNER_PBIT)
        flow.key.inner_pbits = (filter->vlan[1].value.h_vlan_TCI & BCM_VLAN_PBITS_MASK) >> BCM_VLAN_PBITS_SHIFT;

    if (field_mask & RDPA_IC_MASK_OUTER_VID)
        flow.key.outer_vid = filter->vlan[0].value.h_vlan_TCI & BCM_VLAN_VID_MASK;

    if (field_mask & RDPA_IC_MASK_INNER_VID)
        flow.key.inner_vid = filter->vlan[1].value.h_vlan_TCI & BCM_VLAN_VID_MASK;

    if (field_mask & RDPA_IC_MASK_OUTER_TPID)
        flow.key.outer_tpid = filter->eth.value.h_proto;

    if (field_mask & RDPA_IC_MASK_INNER_TPID)
        flow.key.inner_tpid = filter->vlan[0].value.h_vlan_encapsulated_proto;

    flow.key.number_of_vlans = filter->nbrOfVlanTags;
    flow.result.policer = NULL;
    flow.result.qos_method = rdpa_qos_method_flow;
    flow.result.queue_id = 0; /* XXX: insert qos queue mapping function here */
    if (!blog_rule_mapping_get(blog_rule, BLOG_RULE_CMD_SET_DEI, &blog_out))
        flow.result.dei_command = blog_out;
    else
        flow.result.dei_command = RDPA_IC_DEI_COPY;

    if (!is_drop_rule && (blog->tx.info.phyHdrType == BLOG_GPONPHY || blog->tx.info.phyHdrType == BLOG_EPONPHY))
    {
        if (blog_rule_mapping_get(blog_rule, BLOG_RULE_CMD_SET_SKB_MARK_PORT, &blog_out))
        {
            if (blog->tx.info.phyHdrType == BLOG_GPONPHY)
            {
                rc = BDMF_ERR_PARM;
                BDMF_TRACE_ERR("skb port not set for xPON, skipping blog (0x%p) rule\n", blog);
                goto Error;
            }
            else
                blog_out = 0;
        }

        flow.result.wan_flow = blog_out;
    }

    /* DS will use per-port vlan_action drop instead of forward_action_drop */
    flow.result.action = (dir == rdpa_dir_ds || !is_drop_rule) ? rdpa_forward_action_forward : rdpa_forward_action_drop;

    if (!blog_rule_mapping_get(blog_rule, BLOG_RULE_CMD_SET_SKB_MARK_QUEUE, &blog_out))
    {
        blog_queue_id = blog_out;
        protoDebug("Queue %d specified in BLOG, dir %d\n", blog_queue_id, dir);
    }
    else
    {
        blog_queue_id = 0; /* Defaults to queue 0 if not specified in blog */
        protoDebug("Queue %d not specified in BLOG, dir %d\n", blog_queue_id, dir);
    }

    if (dir == rdpa_dir_us)
    {
        flow.key.ingress_port = blog_parse_ingress_port_get(blog);
        if (rdpa_if_is_wifi(flow.key.ingress_port))
        {
            flow.key.ssid = flow.key.ingress_port;
            flow.key.ingress_port = rdpa_if_wlan0;
        }

        *egress_port = 0; /* egress_port is passed on for the blog_key mask */
        flow.result.queue_id = blog_queue_id;
        flow.result.forw_mode = rdpa_forwarding_mode_flow;
    }
    else /* dir = rdpa_dir_ds */
    {
        if (field_mask & RDPA_IC_MASK_GEM_FLOW && blog->rx.info.phyHdrType == BLOG_GPONPHY) /* XXX: check EPON LLID */
        {
            /* port mark filter value is offset by 1 because
             * 0 is reserved to indicate filter not in use.
             * Therefore use 16-bit to cover the supported
             * port range [0 to 255].
             */
            flow.key.gem_flow = filter->skb.markPort - 1;
        }

        *egress_port = blog_parse_egress_port_get(blog);
        ingress_port = blog_parse_ingress_port_get(blog);

        flow.result.queue_id = blog_queue_id;

        /* XXX: ds_dal is not supposed to change during runtime, should be moved to init after rdpa script is eliminated */
        if (ds_dal_enabled_get(ingress_port, &rdpa_mw_packet_based_forward))
            return -1;
        if (rdpa_mw_packet_based_forward)
        {
            flow.result.forw_mode = rdpa_forwarding_mode_pkt;
            /* XXX: rdpa_ingress_class should ignore egress_port when rdpa_forwarding_mode_pkt */
            flow.result.egress_port = rdpa_if_lan0;

            /* This should always be the last line after flow configuration, check if the same flow exists besides the
               vlan_action */
            match_ic_flow(ingress_class_obj, &flow, &flow_idx);
        }
        else
        {
            flow.result.forw_mode = rdpa_forwarding_mode_flow;
            flow.result.egress_port = *egress_port;
        }
    }

    if (!is_drop_rule)
    {
        rc = blog_rule_to_vlan_action(blog_rule, dir, &vlan_action_obj);
        if (rc < 0)
        {
            BDMF_TRACE_ERR("Failed to assign a valid vlan_action\n");
            goto Error;
        }
    }

    /* US flows and new DS flows have vlan_action set, otherwise set port_action */
    if (flow_idx != BDMF_INDEX_UNASSIGNED)
    {
        if (dir == rdpa_dir_ds)
        {
            if (!rdpa_mw_packet_based_forward)
            {
                rc = BDMF_ERR_INTERNAL;
                BDMF_TRACE_ERR("Flow based forwarding cannot assign multiple ports to same flow (%d)\n", (int)flow_idx);
                goto Error;
            }

            protoDebug("Reusing flow_idx %d:", (int)flow_idx);
            rc = configure_action_per_port(ingress_class_obj, flow_idx, *egress_port, vlan_action_obj, is_drop_rule);
            if (rc)
            {
                BDMF_TRACE_ERR("configure_action_per_port fail with rc %d, port %d\n", rc, *egress_port);
                goto Error;
            }
        }
        else
        {
            rc = BDMF_ERR_INTERNAL;
            BDMF_TRACE_ERR("US flow already assigned to flow %d\n", (int)flow_idx);
            goto Error;
        }
    }
    else
    {
        /* Packet based forwarding sets vlan_action per-port only */
        if (!rdpa_mw_packet_based_forward || dir == rdpa_dir_us)
            flow.result.vlan_action = vlan_action_obj;

        rc = rdpa_ingress_class_flow_add(ingress_class_obj, &flow_idx, &flow);
        if (rc < 0)
        {
            BDMF_TRACE_ERR("rdpa_ingress_class_flow_add fail with rc %d, flow_idx %d\n", rc, (int)flow_idx);
            goto Error;
        }

        if (rdpa_mw_packet_based_forward && dir == rdpa_dir_ds)
        {
            rdpa_if lan_port;

            /* Since this is a new flow, initialize all non-configured lan egress ports to drop */
#if defined(CONFIG_BCM_XRDP)
            for (lan_port = rdpa_if_lan0; lan_port <= rdpa_if_wlan2; ++lan_port)
#else
            for (lan_port = rdpa_if_lan0; lan_port <= rdpa_if_ssid0; ++lan_port)
#endif
            {
                int this_rule_port = (lan_port == *egress_port);

                /* Per-port vlan action can set rdpa_if_lan + wlan port (only one action for all ssids) */
                if (!rdpa_if_is_lan(lan_port) || rdpa_if_is_wlan(lan_port))
                    continue;

                protoDebug("this_rule_port %d: ", this_rule_port);
                rc = configure_action_per_port(ingress_class_obj, flow_idx, lan_port,
                    this_rule_port ? vlan_action_obj : NULL, is_drop_rule);
                if (rc)
                {
                    BDMF_TRACE_ERR("configure_action_per_port fail with rc %d, port %d\n", rc, lan_port);
                    goto Error;
                }
            }
        }
    }

Error:
    if (rc)
    {
        if (vlan_action_obj)
            bdmf_put(vlan_action_obj);
        return rc;
    }

    *flow_idx_p = flow_idx;
    return 0;
}

static int add_ic_cfg(uint32_t field_mask, rdpa_traffic_dir dir, int prty, bdmf_object_handle *ingress_class_obj)
{
    rdpa_ic_cfg_t cfg = { .type = RDPA_IC_TYPE_FLOW, .field_mask = field_mask, .prty = prty };
    int rc;
    BDMF_MATTR(ingress_class_attrs, rdpa_ingress_class_drv());

    rdpa_ingress_class_dir_set(ingress_class_attrs, dir);
    rdpa_ingress_class_cfg_set(ingress_class_attrs, &cfg);
    rc = bdmf_new_and_set(rdpa_ingress_class_drv(), NULL, ingress_class_attrs, ingress_class_obj);
    if (rc < 0)
        BDMF_TRACE_RET(rc, "Failed to create ingress_class object\n");

    return 0;
}

static int filter_to_mask(blogRuleFilter_t *filter, rdpa_traffic_dir dir, int is_us_wifi, uint32_t *mask)
{
    *mask = RDPA_IC_MASK_NUM_OF_VLANS;

    if (filter->nbrOfVlanTags > 3 ||
        blog_rule_filterInUse(filter->skb.priority) ||
        blog_rule_filterInUse(filter->skb.markFlowId) || /* Used by linux tc - will not be in use by rdpa_mw */
        (filter->flags && filter->flags != (BCM_VLAN_FILTER_FLAGS_IS_UNICAST | BCM_VLAN_FILTER_FLAGS_IS_BROADCAST))
/* It appears sometimes blogs are send with CFI filter (always set to 0). We ignore them since we do not want to fail blog creation:
        || filter->vlan[0].mask.h_vlan_TCI & BCM_VLAN_CFI_MASK || filter->vlan[1].mask.h_vlan_TCI & BCM_VLAN_CFI_MASK ||
*/
        )
    {
        return -1; /* Unsupported filter */
    }

    if ((filter->nbrOfVlanTags == 2 && blog_rule_filterInUse(filter->vlan[1].mask.h_vlan_encapsulated_proto)) ||
        (filter->nbrOfVlanTags == 1 && blog_rule_filterInUse(filter->vlan[0].mask.h_vlan_encapsulated_proto)) ||
        (blog_rule_filterInUse(filter->eth.mask.h_proto) && !is_tpid(filter->eth.value.h_proto)))
    {
        *mask |= RDPA_IC_MASK_ETHER_TYPE;
    }

    *mask |= blog_rule_filterInUse(filter->eth.mask.h_dest) ? RDPA_IC_MASK_DST_MAC : 0;
    *mask |= blog_rule_filterInUse(filter->eth.mask.h_source) ? RDPA_IC_MASK_SRC_MAC : 0;
    *mask |= blog_rule_filterInUse(filter->ipv4.mask.ip_proto) ? RDPA_IC_MASK_IP_PROTOCOL : 0;
    *mask |= blog_rule_filterInUse(filter->ipv4.mask.tos) ? RDPA_IC_MASK_DSCP : 0;
    *mask |= blog_rule_filterInUse(filter->ipv6.mask.tclass) ? RDPA_IC_MASK_DSCP : 0;
    *mask |= dir == rdpa_dir_us ? RDPA_IC_MASK_INGRESS_PORT : 0;
    *mask |= blog_rule_filterInUse(filter->skb.markPort) ? RDPA_IC_MASK_GEM_FLOW : 0;
    *mask |= is_us_wifi ? RDPA_IC_MASK_SSID : 0;
    if (filter->nbrOfVlanTags > 0)
    {
        *mask |= filter->vlan[0].mask.h_vlan_TCI & BCM_VLAN_PBITS_MASK ? RDPA_IC_MASK_OUTER_PBIT : 0;
        *mask |= filter->vlan[0].mask.h_vlan_TCI & BCM_VLAN_VID_MASK ? RDPA_IC_MASK_OUTER_VID : 0;
    }
    if (filter->nbrOfVlanTags > 1)
    {
        *mask |= filter->vlan[1].mask.h_vlan_TCI & BCM_VLAN_PBITS_MASK ? RDPA_IC_MASK_INNER_PBIT : 0;
        *mask |= filter->vlan[1].mask.h_vlan_TCI & BCM_VLAN_VID_MASK ? RDPA_IC_MASK_INNER_VID : 0;
    }

    if (filter->nbrOfVlanTags >= 1 && blog_rule_filterInUse(filter->eth.mask.h_proto))
        *mask |= RDPA_IC_MASK_OUTER_TPID;

    if (filter->nbrOfVlanTags >= 2 && filter->vlan[0].mask.h_vlan_encapsulated_proto)
        *mask |= RDPA_IC_MASK_INNER_TPID;

    return 0;
}

static inline rdpa_traffic_dir blog_get_direction(Blog_t *blog)
{
    return ((struct net_device *)blog->rx_dev_p)->priv_flags & IFF_WANDEV ? rdpa_dir_ds : rdpa_dir_us;
}

static int ic_idx_exists_in_ref_list(bdmf_number ic_idx)
{
    blog_flow_entry_t *entry = NULL, *tmp_entry;

    DLIST_FOREACH_SAFE(entry, &blog_flow_list, list, tmp_entry)
    {
        if (IC_IDX_FROM_BLOG_KEY(entry->hash) == (int)ic_idx)
            return 1;
    }

    return 0;
}

/*
 *   Find the IC object and next free priority number based on rule field mask
 *   and direction.
 */
static void find_prty_and_ic_by_field_mask(uint32_t field_mask, rdpa_traffic_dir dir,
    bdmf_object_handle *ingress_class_obj, int *next_free_prty)
{
    bdmf_object_handle obj = NULL;
    rdpa_traffic_dir dir_temp;
    rdpa_ic_cfg_t cfg;
    bdmf_number idx;
    int used_prio[RDPA_IC_MAX_PRIORITY + 1] = {};
    int last_used_prty = RDPA_IC_MAX_PRIORITY;
    int i;

    *ingress_class_obj = NULL;
    while ((obj = bdmf_get_next(rdpa_ingress_class_drv(), obj, NULL)))
    {
        rdpa_ingress_class_dir_get(obj, &dir_temp);
        if (dir != dir_temp)
            continue;

        rdpa_ingress_class_cfg_get(obj, &cfg);
        if (cfg.type != RDPA_IC_TYPE_FLOW)
            continue;

        used_prio[cfg.prty] = 1;

        if (ingress_class_obj && !*ingress_class_obj && cfg.field_mask == field_mask)
        {
            bdmf_number idx;

            rdpa_ingress_class_index_get(obj, &idx);
            /* Reuse only ic's which rdpa_mw created */
            if (ic_idx_exists_in_ref_list(idx))
                *ingress_class_obj = obj;
        }

        /* XXX: should pick the lowest number from all IC's (except 0 and 1), not only rdpa_mw's */
        rdpa_ingress_class_index_get(obj, &idx);
        if ((ic_idx_exists_in_ref_list(idx)) && (cfg.prty < last_used_prty) 
            && (cfg.prty != 0) && (cfg.prty != 1))
        {
            last_used_prty = cfg.prty;
        }
    }

    /* 
    * Did't find it in the IC list, assign a priority.
    * For VLAN rules with VLAN NUM only, assign the lowest priority 0. 
    * For VLAN rules with VLAN NUM + PORT, assign from second lowest priority 1.
    * For others, assign from 63. 
    */ 
    if (!*ingress_class_obj)
    {
        *next_free_prty = -1;
    
        if (field_mask == RDPA_IC_MASK_NUM_OF_VLANS)
        {   
            if (!used_prio[0]) 
            {
                *next_free_prty = 0;
            }
        }
        else if ((field_mask == (RDPA_IC_MASK_NUM_OF_VLANS | RDPA_IC_MASK_INGRESS_PORT)) ||
          (field_mask == (RDPA_IC_MASK_NUM_OF_VLANS | RDPA_IC_MASK_GEM_FLOW)))
        {
            if (!used_prio[1])
            {
                *next_free_prty = 1;
            }
            else if (!used_prio[0]) 
            {
                *next_free_prty = 0;
            }
        }
        else
        {
            for (i = last_used_prty; i >= 0; i--)
            {
                if (!used_prio[i])
                {
                    *next_free_prty = i;
                    break;
                }
            }
        }
    }
}

static uint8_t *find_blog_flow_hash(uint32_t key, int add)
{
    blog_flow_entry_t *entry = NULL, *tmp_entry;

    DLIST_FOREACH_SAFE(entry, &blog_flow_list, list, tmp_entry)
    {
        if (entry->hash == key)
            return &entry->ref_cnt;
    }

    if (!add)
        return NULL;

    entry = bdmf_alloc(sizeof(blog_flow_entry_t));
    if (!entry)
        return NULL;

    entry->ref_cnt = 0;
    entry->hash = key;
    DLIST_INSERT_HEAD(&blog_flow_list, entry, list);

    return &entry->ref_cnt;
}

static void remove_blog_flow_hash_by_ref_cnt(uint8_t *ref_cnt)
{
    blog_flow_entry_t *entry = NULL, *tmp_entry;

    DLIST_FOREACH_SAFE(entry, &blog_flow_list, list, tmp_entry)
    {
        if (&entry->ref_cnt == ref_cnt)
        {
            DLIST_REMOVE(entry, list);
            bdmf_free(entry);
        }
    }
}

static void ic_clean(bdmf_object_handle ingress_class_obj)
{
    bdmf_number nflows;

    rdpa_ingress_class_nflow_get(ingress_class_obj, &nflows);
    if (!nflows)
        bdmf_destroy(ingress_class_obj);
}

static uint32_t activate_blogRule(Blog_t *blog)
{
    uint32_t field_mask, flow_idx;
    bdmf_object_handle ingress_class_obj;
    rdpa_traffic_dir dir = blog_get_direction(blog);
    int next_prty, rc, reuse = 0;
    uint8_t *ref_cnt;
    bdmf_number ic_idx;
    rdpa_if egress_port;

    rc = filter_to_mask(&((blogRule_t*)blog->blogRule_p)->filter, dir,
        rdpa_if_is_wifi(blog_parse_ingress_port_get(blog)), &field_mask);
    if (rc < 0)
        BDMF_TRACE_RET(BLOG_KEY_INVALID, "blogRule filter not supported for blog %p\n", blog);

    find_prty_and_ic_by_field_mask(field_mask, dir, &ingress_class_obj, &next_prty);
    if (!ingress_class_obj)
    {
        /*
         * Priority of the rule to be added will be lower than the lowest
         * in use, as the blog rules are activated in the order of priority,
         * the highest priority rule is activated first, the lowest priority
         * is activated in the end.
         * XXX: if a blog rule/classifier in the "middle" will be removed/inserted,
         * this would not be handled correctly.
        */
        if (next_prty == -1)
        {
            BDMF_TRACE_RET(BLOG_KEY_INVALID, "Cannot create IC: all priorities are in use\n");
        }

        protoDebug("<prty=%d>", next_prty);
        rc = add_ic_cfg(field_mask, dir, next_prty, &ingress_class_obj);
        if (rc)
            return BLOG_KEY_INVALID;
    }
    else
        reuse = 1;

    rdpa_ingress_class_index_get(ingress_class_obj, &ic_idx);
    protoDebug("%s ingress_class/dir=%s,index=%d", reuse ? "Reusing" : "Created", dir == rdpa_dir_ds ? "ds" : "us",
        (int)ic_idx);

    rc = add_modify_ic_flow(field_mask, blog, ingress_class_obj, dir, &flow_idx, &egress_port);
    if (rc)
    {
        BDMF_TRACE_ERR("Failed to create ic flow. blogRule not supported, rc=%d\n", rc);
        goto Error;
    }

    protoDebug("Created flow dir %d, ic_idx %d, flow_idx %d, blog_key %x", dir, (int)ic_idx, (int)flow_idx,
        (uint32_t)BLOG_KEY_FROM_DIR_IC_FLOW(dir, ic_idx, flow_idx, egress_port));

    /* inc. ref_cnt of blog_flow_hash - matching flows with different egress_port share same entry */
    ref_cnt = find_blog_flow_hash(BLOG_KEY_FROM_DIR_IC_FLOW(dir, ic_idx, flow_idx, 0), 1);
    if (!ref_cnt)
    {
        rdpa_ingress_class_flow_delete(ingress_class_obj, flow_idx);
        BDMF_TRACE_ERR("malloc failed for find_blog_flow_hash\n");
        goto Error;
    }

    ++(*ref_cnt);
    return BLOG_KEY_FROM_DIR_IC_FLOW(dir, ic_idx, flow_idx, egress_port);

Error:
    if (!reuse)
        ic_clean(ingress_class_obj);

    return BLOG_KEY_INVALID;
}

static uint32_t rdpa_config_fp(Blog_t *blog_p, BlogTraffic_t traffic)
{
    blogRule_t *blog_rule = blog_p->blogRule_p;

    BCM_ASSERT(blog_p != BLOG_NULL && traffic < BlogTraffic_MAX);
    protoDebug("Activating Blog rule (%p), Blog (%p)", blog_rule, blog_p);
    if (protoLogDebug)
    {
        /* blog_dump(blog_p); */
        blog_rule_dump(blog_rule);
    }

    if (traffic != BlogTraffic_Layer2_Flow)
        BDMF_TRACE_RET(BLOG_KEY_INVALID, "rdpa_mw handles only L2 blog rule flows\n");

    return activate_blogRule(blog_p);
}

/* Always return BLOG_NULL, even if error */
static Blog_t *deactivate_blogRule(uint32_t blog_key)
{
    rdpa_ingress_class_key_t key = { .dir = DIR_FROM_BLOG_KEY(blog_key), .index = IC_IDX_FROM_BLOG_KEY(blog_key) };
    bdmf_object_handle ingress_class_obj = NULL;
    int rc;
    char *dir_str = key.dir == rdpa_dir_ds ? "ds" : "us";
    uint8_t *ref_cnt;
    rdpa_ic_info_t  ic_flow;

    /* dec. ref_cnt of blog_flow_hash entry */
    ref_cnt = find_blog_flow_hash(BLOG_HASH_FROM_KEY(blog_key), 0);
    if (!ref_cnt)
    {
        /* Matching blog_key entry not found */
        BUG();
        return BLOG_NULL;
    }

    rdpa_ingress_class_get(&key, &ingress_class_obj);
    if (!ingress_class_obj)
    {
        BDMF_TRACE_ERR("Cannot find ingress_class/dir=%s,index=%d\n", dir_str, (int)key.index);
        return BLOG_NULL;
    }
    bdmf_put(ingress_class_obj); /* above rdpa_ingress_class_get, obj still locked by 'blog-flow' so its safe to put */

    deconfigure_action_per_port(ingress_class_obj, FLOW_IDX_FROM_BLOG_KEY(blog_key), EGRESS_PORT_FROM_BLOG_KEY(blog_key));
    if (--(*ref_cnt))
    {
        protoDebug("IC flow still in use, setting port-action to drop, still in use by %d ports", *ref_cnt);
        configure_action_per_port(ingress_class_obj, FLOW_IDX_FROM_BLOG_KEY(blog_key),
            EGRESS_PORT_FROM_BLOG_KEY(blog_key), NULL, 1);
        return BLOG_NULL; /* Do nothing because there are still ports using the same flow */
    }

    rc = rdpa_ingress_class_flow_get(ingress_class_obj, FLOW_IDX_FROM_BLOG_KEY(blog_key), &ic_flow);
    if ((!rc) && (ic_flow.result.vlan_action))
    {
        bdmf_put(ic_flow.result.vlan_action);
    }

    rc = rdpa_ingress_class_flow_delete(ingress_class_obj, FLOW_IDX_FROM_BLOG_KEY(blog_key));
    if (rc)
    {
        BDMF_TRACE_ERR("Cannot delete ingress_class/dir=%s,index=%d flow[%d], rc=%d\n",
            dir_str, (int)key.index, FLOW_IDX_FROM_BLOG_KEY(blog_key), rc);
    }

    remove_blog_flow_hash_by_ref_cnt(ref_cnt);
    ic_clean(ingress_class_obj);

    return BLOG_NULL;
}

/* Deactivate the FP configuration, recognized by 'key'. */
static Blog_t *rdpa_deconf_fp(uint32_t blog_key, BlogTraffic_t traffic)
{
    protoDebug("rdpa_deconf_fp, blog key %x", blog_key);
    if (traffic != BlogTraffic_Layer2_Flow)
        return BLOG_NULL;

    return deactivate_blogRule(blog_key);
}

static void rdpa_disable_all_tpid(void)
{
    rdpa_tpid_detect_cfg_t entry = {};
    rdpa_tpid_detect_t tpid;

    for(tpid=0; tpid< rdpa_tpid_detect__num_of; ++tpid)
        rdpa_system_tpid_detect_set(rdpa_system, tpid, &entry);
}

static int rdpa_vlan_add(struct net_device *vlan_dev)
{
    BDMF_MATTR(vlan_attr, rdpa_vlan_drv());
    bdmf_object_handle vlan_object, port_object = NULL;
    rdpa_if port;
    int ret = -1;

    port = rdpa_mw_root_dev2rdpa_if(netdev_path_get_root(vlan_dev));
    if (port == rdpa_if_none)
    {
        BDMF_TRACE_ERR("Can't find RDPA port interface for %s\n", vlan_dev->name);
        goto Exit;
    }

    rdpa_port_get(port, &port_object);
    if (!port_object)
    {
        BDMF_TRACE_ERR("Can't find RDPA port object for %s\n", vlan_dev->name);
        goto Exit;
    }

    rdpa_vlan_name_set(vlan_attr, vlan_dev->name);

    if (bdmf_new_and_set(rdpa_vlan_drv(), port_object, vlan_attr, &vlan_object))
    {
        BDMF_TRACE_ERR("Failed to add RDPA vlan object for %s\n", vlan_dev->name);
        goto Exit;
    }

    ret = 0;

Exit:
    if (port_object)
        bdmf_put(port_object);

    return ret;
}

static int rdpa_vlan_del(struct net_device *vlan_dev)
{
    bdmf_object_handle vlan_object = NULL;
    int ret = -1;

    if (rdpa_vlan_get(vlan_dev->name, &vlan_object))
    {
        BDMF_TRACE_ERR("Can't find RDPA vlan object for %s\n", vlan_dev->name);
        goto Exit;
    }

    if (bdmf_destroy(vlan_object))
    {
        BDMF_TRACE_ERR("Failed to remove RDPA vlan object for %s\n", vlan_dev->name);
        goto Exit;
    }

    ret = 0;

Exit:
    if (vlan_object)
        bdmf_put(vlan_object);

    return ret;
}

static int rdpa_vlan_vid_set(const char *name, unsigned int vid, int enable)
{
    bdmf_object_handle vlan_object;

    if (rdpa_vlan_get(name, &vlan_object))
    {
        BDMF_TRACE_ERR("Can't find RDPA vlan object for %s\n", name);
        return -1;
    }

    bdmf_put(vlan_object);
    rdpa_vlan_vid_enable_set(vlan_object, vid, enable ? 1 : 0);

    return 0;
}

#define BCM_VLAN_DONT_CARE         ~0

static int rdpa_vlan_set(vlanctl_vlan_t *vlanctl_vlan)
{
    BDMF_TRACE_DBG("rdpa_vlan_set: op=%s dev=%s vid=%u\n",
        vlanctl_vlan->enable ? "add" : "del",
        vlanctl_vlan->vlan_dev->name,
        vlanctl_vlan->vid);

    if (vlanctl_vlan->vid == BCM_VLAN_DONT_CARE)
    {
        if (vlanctl_vlan->enable)
            return rdpa_vlan_add(vlanctl_vlan->vlan_dev);
        else
            return rdpa_vlan_del(vlanctl_vlan->vlan_dev);
    }
    else
    {
        return rdpa_vlan_vid_set(vlanctl_vlan->vlan_dev->name, vlanctl_vlan->vid, vlanctl_vlan->enable);
    }
}

#if defined(CONFIG_BCM_PON)
static int rdpa_route_mac_add(bdmf_mac_t *p_mac)
{

    bdmf_error_t rc = 0;
    bdmf_object_handle ip_class = NULL;
    bdmf_mac_t wan_mac = {};
    int is_found = 0;
    int loop;

    bdmf_lock();

    rc = rdpa_ip_class_get(&ip_class);
    if (rc)
    {
        bdmf_unlock();
        BDMF_TRACE_RET(rc, "get ip_class object return errors\n");
    }

    for (loop = 0; !rc && loop < RDPA_MAX_ROUTED_MAC; loop ++)
    {
        rc = rdpa_ip_class_routed_mac_get(ip_class, loop, &wan_mac);
        if (!rc && bdmf_mac_is_zero(&wan_mac))
        {
            is_found = 1;
            memcpy(&wan_mac, p_mac, sizeof(wan_mac));
            rc = rc ? rc : rdpa_ip_class_routed_mac_set(ip_class, loop, &wan_mac);
            break;
        }
    }

    if (!rc && !is_found)
    {
        rc = BDMF_ERR_NORES;
        BDMF_TRACE_ERR("ip_class route mac table full\n");
    }

    if (ip_class)
        bdmf_put(ip_class);

    bdmf_unlock();
    return rc;
}

static int rdpa_route_mac_del(bdmf_mac_t *p_mac)
{

    bdmf_error_t rc = 0;
    bdmf_object_handle ip_class = NULL;
    bdmf_mac_t wan_mac = {};
    bdmf_mac_t zero_mac = {};
    int is_found = 0;
    int loop;

    bdmf_lock();
    rc = rdpa_ip_class_get(&ip_class);
    if(rc)
    {
        bdmf_unlock();
        BDMF_TRACE_RET(rc, "get ip_class object return error\n");
    }

    for (loop = 0; !rc && loop < RDPA_MAX_ROUTED_MAC; loop ++)
    {
        rc = rc ? rc : rdpa_ip_class_routed_mac_get(ip_class, loop, &wan_mac);
        if (!rc && !memcmp(&wan_mac, p_mac, sizeof(wan_mac)))
        {
            is_found = 1;
            rc = rc ? rc : rdpa_ip_class_routed_mac_set(ip_class, loop, &zero_mac);
            break;
        }
    }

    if (!rc && !is_found)
    {
        rc = BDMF_ERR_NOENT;
        BDMF_TRACE_ERR("mac not found in ip_class\n");
    }

    if (ip_class)
        bdmf_put(ip_class);

    bdmf_unlock();
    return rc;
}

static int rdpa_route_mac_set(vlanctl_route_mac_t *vlanctl_route_mac)
{
    bdmf_mac_t mac;

    memcpy(mac.b, vlanctl_route_mac->mac, sizeof(mac.b));
    return (vlanctl_route_mac->enable)?rdpa_route_mac_add(&mac):rdpa_route_mac_del(&mac);
}

#endif

static void rdpa_setTpidTable(unsigned int *tpidTable)
{
    rdpa_tpid_detect_cfg_t entry = {.val_udef=0, .otag_en=1, .itag_en=1};
    rdpa_tpid_detect_t tpid;
    int is_udef1_used = 0;
    int rc, i;

    rdpa_disable_all_tpid();

    for(i=0; i<BCM_VLAN_MAX_TPID_VALUES; ++i)
    {
        switch(tpidTable[i])
        {
        case 0x8100:
            tpid = rdpa_tpid_detect_0x8100;
            break;
        case 0x88A8:
            tpid = rdpa_tpid_detect_0x88A8;
            break;
        case 0x9100:
            tpid = rdpa_tpid_detect_0x9100;
            break;
        case 0x9200:
            tpid = rdpa_tpid_detect_0x9200;
            break;
        default:
            if(!is_udef1_used)
            {
                tpid = rdpa_tpid_detect_udef_1;
                is_udef1_used = 1;
            }
            else
                tpid = rdpa_tpid_detect_udef_2;
            break;
        }

        entry.val_udef = tpidTable[i];		
        rc = rdpa_system_tpid_detect_set(rdpa_system, tpid, &entry);
        if (rc)
            protoDebug("Set tpid error\n");
    }
}

static void rdpa_notify_fp( vlanctl_bind_Notify_t event, void *ptr)
{
    switch(event)
    {
    case VLANCTL_BIND_NOTIFY_TPID:
        protoDebug("Activating blog tpid set");
        rdpa_setTpidTable(ptr);
        break;
    case VLANCTL_BIND_NOTIFY_VLAN:
        protoDebug("Setting vlan");
        rdpa_vlan_set(ptr);
        break;
    case VLANCTL_BIND_NOTIFY_ROUTE_MAC:
        protoDebug("Setting route mac");
#if defined(CONFIG_BCM_PON)
        rdpa_route_mac_set(ptr);
#endif
        break;
    case VLANCTL_BIND_DROP_PRECEDENCE_SET:
    {
        const struct rdpa_mw_drop_precedence_args *args = ptr;
        rdpa_mw_drop_precedence_set(args->rdpa_dir, args->dp_code);
        break;
    }
    default:
        break;
    }
}

static void rdpa_bind_vlanctl(int enabled)
{
    vlanctl_bind_t hook_info;

    protoDebug("rdpa_bind_vlanctl %s", enabled ? "enable" : "disable");

    /* Register for all possible calls from vlanctl_bind */
    hook_info.hook_info = 0xFF;
    if (enabled)
    {
        if (method == rdpa_method_fc)
            vlanctl_bind_config((vlanctl_bind_ScHook_t)NULL, (vlanctl_bind_SdHook_t)NULL,
                 (vlanctl_bind_SnHook_t)rdpa_notify_fp,  VLANCTL_BIND_CLIENT_RUNNER, hook_info);
        else
            vlanctl_bind_config((vlanctl_bind_ScHook_t)rdpa_config_fp, (vlanctl_bind_SdHook_t)rdpa_deconf_fp,
                 (vlanctl_bind_SnHook_t)rdpa_notify_fp,  VLANCTL_BIND_CLIENT_RUNNER, hook_info);
    }
    else
        vlanctl_bind_config(NULL, NULL, NULL, VLANCTL_BIND_CLIENT_RUNNER, hook_info);

    protoInfo("vlanctl_bind rules reflection to Runner %s.", enabled ? "enabled" : "disabled");
}
#endif

int __init rdpa_mw_init(void)
{
    rdpa_system_init_cfg_t sys_init_cfg;

    bcmLog_setLogLevel(BCM_LOG_ID_RDPA, BCM_LOG_LEVEL_ERROR);

    rdpa_init_system();

    if (rdpa_system_get(&rdpa_system))
        return -1;

    rdpa_system_init_cfg_get(rdpa_system, &sys_init_cfg);
    method = sys_init_cfg.ip_class_method;
    BCM_LOG_INFO(BCM_LOG_ID_RDPA, "Loading rdpa_mw, RDPA ip class method = %d\n", method);

#if defined(CONFIG_BLOG)
    DLIST_INIT(&blog_flow_list);
    rdpa_bind_vlanctl(1);
#endif

    return 0;
}

void __exit rdpa_mw_exit(void)
{
#if defined(CONFIG_BLOG)
    blog_flow_entry_t *entry = NULL, *tmp_entry;
#endif

    BCM_LOG_INFO(BCM_LOG_ID_RDPA, "Exit\n");

#if defined(CONFIG_BLOG)
    rdpa_bind_vlanctl(0);

    DLIST_FOREACH_SAFE(entry, &blog_flow_list, list, tmp_entry)
    {
        DLIST_REMOVE(entry, list);
        bdmf_free(entry);
    }
#endif
    bdmf_put(rdpa_system);
}

module_init(rdpa_mw_init);
module_exit(rdpa_mw_exit);

MODULE_DESCRIPTION("RDPA MW layer");
MODULE_VERSION("0.9");
MODULE_LICENSE("GPL");

