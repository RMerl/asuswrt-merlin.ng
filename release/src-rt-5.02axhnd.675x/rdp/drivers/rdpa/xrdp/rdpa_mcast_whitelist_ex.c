/*
 * <:copyright-BRCM:2017:proprietary:standard
 * 
 *    Copyright (c) 2017 Broadcom 
 *    All Rights Reserved
 * 
 *  This program is the proprietary software of Broadcom and/or its
 *  licensors, and may only be used, duplicated, modified or distributed pursuant
 *  to the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied), right
 *  to use, or waiver of any kind with respect to the Software, and Broadcom
 *  expressly reserves all rights in and to the Software and all intellectual
 *  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 *  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 *  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 * 
 *  Except as expressly set forth in the Authorized License,
 * 
 *  1. This program, including its structure, sequence and organization,
 *     constitutes the valuable trade secrets of Broadcom, and you shall use
 *     all reasonable efforts to protect the confidentiality thereof, and to
 *     use this information only in connection with your use of Broadcom
 *     integrated circuit products.
 * 
 *  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
 *     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
 *     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
 *     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
 *     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
 *     PERFORMANCE OF THE SOFTWARE.
 * 
 *  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
 *     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
 *     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 *     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
 *     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
 *     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
 *     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
 *     LIMITED REMEDY.
 * :> 
 */

#include <bdmf_dev.h>
#include <rdd.h>
#include <rdpa_api.h>
#include "rdp_drv_hash.h"
#include "rdpa_mcast_whitelist.h"
#include "rdpa_mcast_whitelist_ex.h"
#include "rdd_multicast_whitelist.h"
#include "rdpa_iptv_ex.h"
#include "rdpa_port_int.h"

int rdpa_mcast_whitelist_pre_init_ex(void)
{
    return BDMF_ERR_OK;
}

int rdpa_mcast_whitelist_post_init_ex(struct bdmf_object *mo)
{
    mcast_whitelist_drv_priv_t *mcast_whitelist = (mcast_whitelist_drv_priv_t *)bdmf_obj_data(mo);

    /* Make sure index pool is initialized */
    if (!mcast_whitelist->flow_idx_pool_p)
    {
        int err = 0;
        /* initialize the flow_idx_pool */
        mcast_whitelist->flow_idx_pool_p = (rdpa_flow_idx_pool_t *)bdmf_alloc(sizeof(rdpa_flow_idx_pool_t));
        if (!mcast_whitelist->flow_idx_pool_p)
        {
            BDMF_TRACE_ERR("Memory allocation failure for rdpa_flow_idx_pool\n");

            return BDMF_ERR_NOMEM;
        }

        err = rdpa_flow_idx_pool_init(mcast_whitelist->flow_idx_pool_p, RDPA_MCAST_MAX_WHITELIST, "mcast_whitelist");
        if (err)
        {
            bdmf_free(mcast_whitelist->flow_idx_pool_p);
            mcast_whitelist->flow_idx_pool_p = NULL;
            return err;
        }
    }

    return BDMF_ERR_OK;
}

void rdpa_mcast_whitelist_destroy_ex(void)
{
    return;
}

static void _mcast_wlist_rdpa2rdd(rdpa_mcast_whitelist_t *rdpa_mc_wlist,
                                  RDD_MULTICAST_WHITELIST_LKP_ENTRY_DTS *rdd_mc_wlist,
                                  RDD_MULTICAST_WHITELIST_CONTEXT_ENTRY_DTS *rdd_mc_wlist_ctxt)
{
    uint32_t ipv6_dst_ip_crc;

    memset(rdd_mc_wlist, 0x0, sizeof(RDD_MULTICAST_WHITELIST_LKP_ENTRY_DTS));
    memset(rdd_mc_wlist_ctxt, 0x0, sizeof(RDD_MULTICAST_WHITELIST_CONTEXT_ENTRY_DTS));

    rdd_mc_wlist->valid = 1;
    rdd_mc_wlist_ctxt->valid = 1;
    rdd_mc_wlist_ctxt->num_of_vlans = rdpa_mc_wlist->num_vlan_tags;
    rdd_mc_wlist_ctxt->vlan_0_union = 0xfff;
    rdd_mc_wlist_ctxt->vlan_1_union = 0xfff;
    if (rdpa_mc_wlist->num_vlan_tags >= 1)
        rdd_mc_wlist_ctxt->vlan_0_union = rdpa_mc_wlist->outer_vlan_id;
    if (rdpa_mc_wlist->num_vlan_tags == 2)
        rdd_mc_wlist_ctxt->vlan_1_union = rdpa_mc_wlist->inner_vlan_id;
    if (rdpa_mc_wlist->dst_ip.family == bdmf_ip_family_ipv4)
    {
        rdd_mc_wlist->is_ipv6 = 0;
        rdd_mc_wlist->dst_ip = rdpa_mc_wlist->dst_ip.addr.ipv4;
        memcpy(&rdd_mc_wlist_ctxt->dst_ip, &rdpa_mc_wlist->dst_ip.addr.ipv4, 4);
        if (rdpa_mc_wlist->src_ip.addr.ipv4 != 0)
        {
            rdd_mc_wlist_ctxt->check_src_ip = 1;
            memcpy(&rdd_mc_wlist_ctxt->src_ip, &rdpa_mc_wlist->src_ip.addr.ipv4, 4);
        }
    }
    else
    {
        rdd_mc_wlist->is_ipv6 = 1;
        rdd_crc_ipv6_addr_calc(&rdpa_mc_wlist->dst_ip, &ipv6_dst_ip_crc);
        rdd_mc_wlist->dst_ip = ipv6_dst_ip_crc;
        memcpy(rdd_mc_wlist_ctxt->dst_ip, &rdpa_mc_wlist->dst_ip.addr.ipv6, 16);

        if (!bdmf_ipv6_is_zero(&rdpa_mc_wlist->src_ip.addr.ipv6))
        {
            rdd_mc_wlist_ctxt->check_src_ip = 1;
            memcpy(rdd_mc_wlist_ctxt->src_ip, &rdpa_mc_wlist->src_ip.addr.ipv6, 16);
        }
    }
}

static void _mcast_wlist_rdd2rdpa(RDD_MULTICAST_WHITELIST_LKP_ENTRY_DTS *rdd_mc_wlist,
                                  RDD_MULTICAST_WHITELIST_CONTEXT_ENTRY_DTS *rdd_mc_wlist_ctxt,
                                  rdpa_mcast_whitelist_t *rdpa_mc_wlist)
{
    rdpa_mc_wlist->num_vlan_tags = rdd_mc_wlist_ctxt->num_of_vlans;
    rdpa_mc_wlist->outer_vlan_id = rdd_mc_wlist_ctxt->vlan_0_union;
    rdpa_mc_wlist->inner_vlan_id = rdd_mc_wlist_ctxt->vlan_1_union;

    if (rdd_mc_wlist->is_ipv6)
    {
        rdpa_mc_wlist->src_ip.family = bdmf_ip_family_ipv6;
        rdpa_mc_wlist->dst_ip.family = bdmf_ip_family_ipv6;
    }
    else
    {
        rdpa_mc_wlist->src_ip.family = bdmf_ip_family_ipv4;
        rdpa_mc_wlist->dst_ip.family = bdmf_ip_family_ipv4;
    }
    memcpy(&rdpa_mc_wlist->dst_ip.addr.ipv6, rdd_mc_wlist_ctxt->dst_ip, 16);
    memcpy(&rdpa_mc_wlist->src_ip.addr.ipv6, rdd_mc_wlist_ctxt->src_ip, 16);
}

/* "whitelist" attribute "read" callback */
int mcast_attr_whitelist_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
                                 bdmf_index index, void *val, uint32_t size)
{
    mcast_whitelist_drv_priv_t *mcast_whitelist = (mcast_whitelist_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_mcast_whitelist_t *rdpa_mc_wlist = (rdpa_mcast_whitelist_t *)val;
    RDD_MULTICAST_WHITELIST_LKP_ENTRY_DTS rdd_mc_wlist;
    RDD_MULTICAST_WHITELIST_CONTEXT_ENTRY_DTS rdd_mc_wlist_ctxt;
    uint32_t rdd_flow_id;
    bdmf_error_t rc = BDMF_ERR_OK;

    if (rdpa_flow_idx_pool_get_id(mcast_whitelist->flow_idx_pool_p, (uint32_t)index, &rdd_flow_id))
        return BDMF_ERR_NOENT;

    rc = rdd_multicast_whitelist_entry_get(rdd_flow_id, &rdd_mc_wlist, &rdd_mc_wlist_ctxt);

    if (rc == BDMF_ERR_OK)
        _mcast_wlist_rdd2rdpa(&rdd_mc_wlist, &rdd_mc_wlist_ctxt, rdpa_mc_wlist);

    return rc;
}

/* "whitelist" attribute "add" callback */
int mcast_attr_whitelist_add_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
                                bdmf_index *index, const void *val, uint32_t size)
{
    mcast_whitelist_drv_priv_t *mcast_whitelist = (mcast_whitelist_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_mcast_whitelist_t *rdpa_mc_wlist = (rdpa_mcast_whitelist_t *)val;
    RDD_MULTICAST_WHITELIST_LKP_ENTRY_DTS rdd_mc_wlist;
    RDD_MULTICAST_WHITELIST_CONTEXT_ENTRY_DTS rdd_mc_wlist_ctxt;
    uint32_t rdd_flow_id, rdpa_flow_idx;
    bdmf_error_t rc = BDMF_ERR_OK;

    _mcast_wlist_rdpa2rdd(rdpa_mc_wlist, &rdd_mc_wlist, &rdd_mc_wlist_ctxt);

    rc = rdd_multicast_whitelist_entry_add(&rdd_mc_wlist, &rdd_mc_wlist_ctxt, &rdd_flow_id);

    if (rc)
        return rc;

    if (rdpa_flow_idx_pool_get_index(mcast_whitelist->flow_idx_pool_p, &rdpa_flow_idx))
    {
        rdd_multicast_whitelist_delete(rdd_flow_id);
        return BDMF_ERR_NORES;
    }

    if (rdpa_flow_idx_pool_set_id(mcast_whitelist->flow_idx_pool_p, rdpa_flow_idx, rdd_flow_id) != 0)
    {
        rdpa_flow_idx_pool_return_index(mcast_whitelist->flow_idx_pool_p, rdpa_flow_idx);
        rdd_multicast_whitelist_delete(rdd_flow_id);
        return BDMF_ERR_NORES;
    }

    *index = (bdmf_index)rdpa_flow_idx;

    return rc;
}

/* "whitelist" attribute "del" callback */
int mcast_attr_whitelist_delete_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
                                   bdmf_index index)
{
    mcast_whitelist_drv_priv_t *mcast_whitelist = (mcast_whitelist_drv_priv_t *)bdmf_obj_data(mo);
    uint32_t rdd_flow_id;
    bdmf_error_t rc = BDMF_ERR_OK;

    if (rdpa_flow_idx_pool_get_id(mcast_whitelist->flow_idx_pool_p, (uint32_t)index, &rdd_flow_id))
        return BDMF_ERR_NOENT;

    rc = rdd_multicast_whitelist_delete(rdd_flow_id);
    if (rc)
        return rc;

    rdpa_flow_idx_pool_return_index(mcast_whitelist->flow_idx_pool_p, (uint32_t)index);

    return rc;
}

/* "whitelist" attribute "find" callback */
int mcast_attr_whitelist_find_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
                                 bdmf_index *index, void *val, uint32_t size)
{
    mcast_whitelist_drv_priv_t *mcast_whitelist = (mcast_whitelist_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_mcast_whitelist_t *rdpa_mc_wlist = (rdpa_mcast_whitelist_t *)val;
    RDD_MULTICAST_WHITELIST_LKP_ENTRY_DTS rdd_mc_wlist;
    RDD_MULTICAST_WHITELIST_CONTEXT_ENTRY_DTS rdd_mc_wlist_ctxt;
    uint32_t rdd_flow_id, rdpa_flow_idx = 0;
    bdmf_error_t rc = BDMF_ERR_OK;

    _mcast_wlist_rdpa2rdd(rdpa_mc_wlist, &rdd_mc_wlist, &rdd_mc_wlist_ctxt);

    rc = rdd_multicast_whitelist_entry_find(&rdd_mc_wlist, &rdd_mc_wlist_ctxt, &rdd_flow_id);

    if (!rc && rdpa_flow_idx_pool_reverse_get_index(mcast_whitelist->flow_idx_pool_p, &rdpa_flow_idx, rdd_flow_id))
        return BDMF_ERR_NOENT;

    *index = (bdmf_index)rdpa_flow_idx;

    return rc;
}

/* "whitelist enable_port" attribute "read" callback */
int mcast_attr_whitelist_enable_port_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
                                             bdmf_index index, void *val, uint32_t size)
{
    bdmf_error_t rc;
    uint32_t port_mask = 0;
    rdd_vport_id_t rdd_vport = rdpa_port_rdpa_if_to_vport(index);
    bdmf_boolean *enable_ptr = (bdmf_boolean *)val;

    rc = rdd_multicast_whitelist_enable_port_mask_get(&port_mask);

    if (port_mask & RDD_VPORT_ID(rdd_vport))
        *enable_ptr = 1;
    else
        *enable_ptr = 0;

    return rc;
}

/* "whitelist enable_port" attribute "write" callback */
int mcast_attr_whitelist_enable_port_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
                                              bdmf_index index, const void *val, uint32_t size)
{
    bdmf_error_t rc;
    rdd_vport_id_t rdd_vport = rdpa_port_rdpa_if_to_vport(index);
    uint32_t port_mask = 0;
    bdmf_boolean enable = *(bdmf_boolean *)val;

    if (rdd_vport > 31)
        return BDMF_ERR_RANGE;

    rc = rdd_multicast_whitelist_enable_port_mask_get(&port_mask);
    if (rc != BDMF_ERR_OK)
        return rc;

    port_mask &= ~RDD_VPORT_ID(rdd_vport);
    port_mask |= enable << rdd_vport;

    return rdd_multicast_whitelist_enable_port_mask_set(port_mask);
}

/* "whitelist stat" attribute "read" callback */
int mcast_attr_whitelist_stats_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
                                       bdmf_index index, void *val, uint32_t size)
{
#if 0
    /* TODO! temporarily disable this section, as the result is not yet correct */
    mcast_whitelist_drv_priv_t *mcast_whitelist = (mcast_whitelist_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_stat_t *stat = (rdpa_stat_t *)val;
    uint32_t rdd_flow_id;
    uint64_t packets = 0;
    uint64_t bytes = 0;
    bdmf_error_t rc;

    if (rdpa_flow_idx_pool_get_id(mcast_whitelist->flow_idx_pool_p, (uint32_t)index, &rdd_flow_id))
        return BDMF_ERR_NOENT;

    rc = rdd_multicast_whitelist_counter_get(rdd_flow_id, &packets, &bytes);
    if (rc == BDMF_ERR_OK)
    {
        stat->packets = (uint32_t)packets;
        stat->bytes = (uint32_t)bytes;
        return 0;
    }
    return rc;
#else
    return BDMF_ERR_NOT_SUPPORTED;
#endif
}

/* "whitelist global_stat" attribute read callback */
int mcast_attr_whitelist_global_stat_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
                                             bdmf_index index, void *val, uint32_t size)
{
    rdpa_iptv_stat_t rdpa_iptv_stat = {};
    rdpa_mcast_whitelist_stat_t *stat = (rdpa_mcast_whitelist_stat_t *)val;
    bdmf_error_t rc;

    rc = rdpa_iptv_stat_read_ex(ad, index, (void *)&rdpa_iptv_stat, sizeof(rdpa_iptv_stat_t));
    if (rc)
        return rc;

    /* rdpa_iptv_stat.discard_[pkt/bytes]
     *      => packet/byte count of total iptv_lookup miss -> enter mcast_whitelist_lookup
     * rdpa_iptv_stat.iptv_lkp_miss_drop
     *   + rdpa_iptv_stat.iptv_src_ip_vid_lkp_miss_drop
     *   + rdpa_iptv_stat.iptv_invalid_ctx_entry_drop => packet count of total
     * mcast_whitelist_lookup miss */
    stat->rx_pkt = rdpa_iptv_stat.discard_pkt;
    stat->rx_byte = rdpa_iptv_stat.discard_bytes;
    stat->dropped_pkt = rdpa_iptv_stat.iptv_lkp_miss_drop +
                        rdpa_iptv_stat.iptv_src_ip_vid_lkp_miss_drop +
                        rdpa_iptv_stat.iptv_invalid_ctx_entry_drop;

    return rc;
}

