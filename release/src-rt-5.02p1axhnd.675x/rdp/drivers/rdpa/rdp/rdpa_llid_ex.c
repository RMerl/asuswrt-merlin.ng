/*
* <:copyright-BRCM:2013-2015:proprietary:standard
* 
*    Copyright (c) 2013-2015 Broadcom 
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

/*
 * rdpa_llid.c
 *
 * EPON LLID driver
 */

#include <bdmf_dev.h>
#include "rdpa_api.h"
#include "rdpa_common.h"
#include "rdpa_int.h"
#include "rdd.h"
#include "rdpa_llid_ex.h"
#include "rdpa_ingress_class_int.h"

extern bdmf_object_handle llid_objects[RDPA_EPON_MAX_LLID];

epon_l2_l1_map epon_l2_l1_alloc[] = {
    {0, 8}, {1, 9}, {2, 10}, {3, 11}, {4, 12}, {5, 13}, {6, 14}, {7, 15}
};

/* pm_counters attribute "read" callback */
int llid_attr_counters_read(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, void *val, uint32_t size)
{
    llid_drv_priv_t *llid = (llid_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_gem_stat_t *stat = (rdpa_gem_stat_t *)val;
    rdd_flow_pm_counters_t rdd_flow_counters = {};
    int rc = 0;

    if ((unsigned)index >= llid_total_chan_num)
        return BDMF_ERR_NO_MORE;
    if (llid->channels[index] < 0)
        return BDMF_ERR_NOENT;

    rc = rdd_flow_pm_counters_get(llid->channels[index], RDD_FLOW_PM_COUNTERS_US, 0, &rdd_flow_counters);
    if (rc)
    {
        BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "rdd_flow_pm_counters_get(%d, 2, &counters) --> %d\n",
            llid->channels[index], rc);
    }

    stat->tx_packets = rdd_flow_counters.good_tx_packet;
    stat->tx_bytes = rdd_flow_counters.good_tx_bytes;
    stat->tx_packets_discard = rdd_flow_counters.error_tx_packets_discard;

    if (RDPA_EPON_CTRL_CH_INDEX == index)
    {
        memset(&rdd_flow_counters, 0, sizeof(rdd_flow_counters));
        rc = rdd_flow_pm_counters_get(llid->index, RDD_FLOW_PM_COUNTERS_DS, 0, &rdd_flow_counters);
        if (rc)
        {
            BDMF_TRACE_RET_OBJ(BDMF_ERR_INTERNAL, mo, "rdd_flow_pm_counters_get(%ld, 1, &counters) --> %d\n",
                llid->index, rc);
        }
        stat->rx_packets = rdd_flow_counters.good_rx_packet;
        stat->rx_bytes = rdd_flow_counters.good_rx_bytes;
        stat->rx_packets_discard = rdd_flow_counters.error_rx_packets_discard;
    }
    return rc;
}

static int rdpa_rdd_rx_default_flow_context_get(uint32_t flow_index, rdd_ic_context_t *context)
{
    return rdd_ic_context_get(rdpa_dir_ds, flow_index, context);
}

int rdpa_rdd_default_flow_del(llid_drv_priv_t *llid)
{
    return rdd_ds_wan_flow_cfg(llid->ds_def_flow, 0, 1, RDPA_UNMATCHED_DS_IC_RESULT_ID);
}

int rdpa_rdd_default_flow_cfg(uint32_t flow_idx, rdd_ic_context_t *context)
{
    return rdd_ic_context_cfg(rdpa_dir_ds, flow_idx, context);
}

/* Set llid channel */
int llid_set_channel_ex(llid_drv_priv_t *llid)
{
     int i, rc;

    /* assign free channel */
    rc = llid_set_l1_channel(llid);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "llid_set_l1_channel()--> %d\n", rc);
    /* config rdd path */
    for (i = 0; i < llid_total_chan_num; i++)
    {
        /* Set RDD mapping */
        rdd_us_wan_flow_cfg(llid->channels[i], llid->channels[i], 0, 0, 1, 0, 0, 0);
    }
    return rc;
}

void llid_destroy_ex(llid_drv_priv_t *llid)
{
    int i;

    for (i = 0; i < llid_total_chan_num; i++)
    {
        if (llid_epon_l1_channels_get(i) == llid->index)
        {
            /* Clear RDD mapping */
            rdd_us_wan_flow_cfg(llid->channels[i], 0, 0, 0, 0, 0, 0, 0);
            llid_epon_l1_channels_set(i, BDMF_INDEX_UNASSIGNED);
        }
    }
}

int llid_attr_ds_def_flow_read_ex(struct bdmf_object *mo, 
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    llid_drv_priv_t *priv = (llid_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_ic_result_t *cfg = (rdpa_ic_result_t *)val;
    rdd_ic_context_t context;
    int rc = BDMF_ERR_OK;

    if (priv->ds_def_flow == RDPA_UNMATCHED_DS_IC_RESULT_ID)
        return BDMF_ERR_NOENT;

    rc = rdpa_rdd_rx_default_flow_context_get(priv->ds_def_flow, &context);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "failed reading flow index %d\n",
            (int)index);

    return rdpa_map_from_rdd_classifier(rdpa_dir_ds, cfg, &context, 0);
}

int llid_set_def_flow_ex(llid_drv_priv_t *llid, rdpa_ic_result_t *cfg)
{
    int i;
    rdd_ic_context_t new_context = {};
    rdd_ic_context_t context = {};
    int idx, rc;

    rc = rdpa_map_to_rdd_classifier(rdpa_dir_ds, cfg, &new_context, 0, 0, RDPA_IC_TYPE_FLOW, 0);
    if (rc)
        return rc;

    for (i = 0; i < RDPA_EPON_MAX_LLID; i++)
    {
        if (llid_objects[i])
        {
            llid_drv_priv_t *other_llid = 
                (llid_drv_priv_t *)bdmf_obj_data(llid_objects[i]);

            if (other_llid->index == llid->index)
                continue;

            rdpa_rdd_rx_default_flow_context_get(other_llid->ds_def_flow, &context);

            /* if default configuration already exist - use it */
            if (!memcmp(&context, &new_context, sizeof(rdd_ic_context_t)))
            {
                rc = rdd_ds_wan_flow_cfg(llid->index, 0, 1, other_llid->ds_def_flow);
                if (rc)
                {
                    BDMF_TRACE_RET(BDMF_ERR_INTERNAL,
                        "Can't set llid %d ds default flow configuration, "
                        "error %d", (int)llid->index, rc);
                }

                llid->ds_def_flow = other_llid->ds_def_flow;
                return 0;
            }
        }
    }

    /*new ds default flow configuration */
    rc = classification_ctx_index_get(rdpa_dir_ds, 0, &idx);
    if (rc < 0)
        goto exit2;

    rc = rc ? rc : rdpa_rdd_default_flow_cfg(idx, &new_context);
    if (rc)
        goto exit1;

    rc = rdd_ds_wan_flow_cfg(llid->index, 0, 1, idx);
    if (rc)
        goto exit;

    llid->ds_def_flow = idx;
    return 0;
exit:
    rdpa_ic_result_delete(idx, rdpa_dir_ds);
exit1:
    classification_ctx_index_put(rdpa_dir_ds, idx);
exit2:
    BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Cannot set default flow. error %d\n",
        rc);
    return 0;
}

int llid_link_tc_to_queue(struct bdmf_object *mo, bdmf_boolean enable)
{
    return BDMF_ERR_OK;
}

