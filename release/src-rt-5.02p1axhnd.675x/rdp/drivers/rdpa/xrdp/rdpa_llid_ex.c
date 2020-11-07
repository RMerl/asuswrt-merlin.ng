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
#include "rdpa_ingress_class_int.h"
#include "rdd.h"
#include "rdpa_llid_ex.h"
#include "rdd_runner_proj_defs.h"
#include "rdp_drv_bbh_tx.h"
#include "rdp_drv_proj_cntr.h"

/* LLID counters shadow */
static uint32_t accumulative_rx_llid_couners[RX_FLOW_COUNTERS_NUM][3] = {};
static uint32_t accumulative_tx_llid_couners[TX_FLOW_COUNTERS_NUM][2] = {};

int gem_attr_ds_def_flow_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size);

extern bdmf_object_handle llid_objects[RDPA_EPON_MAX_LLID];

bdmf_boolean llid_has_dedicated_data_channel(llid_drv_priv_t *llid);

epon_l2_l1_map epon_l2_l1_alloc[] = {
    {0, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}, {6, 6}, {7, 7},
    {8, 8}, {9, 9}, {10, 10}, {11, 11}, {12, 12}, {13, 13}, {14, 14}, {15, 15},
    {16, 16}, {17, 17}, {18, 18}, {19, 19}, {20, 20}, {21, 21}, {22, 22}, {23, 23},
    {24, 24}, {25, 25}, {26, 26}, {27, 27}, {28, 28}, {29, 29}, {30, 30}, {31, 31}
};

int rdpa_rdd_default_flow_del(llid_drv_priv_t *llid)
{
    int rc = 0;
    rdd_ic_context_t context = {.cntr_id = TCAM_DEF_CNTR_GROUP_INVLID_CNTR};

    rdpa_cntr_id_dealloc(CNTR_GROUP_TCAM_DEF, DEF_FLOW_CNTR_SUB_GROUP_ID, llid->index);
    rdd_rx_default_flow_cfg(llid->index, llid->ds_def_flow, &context);
    classification_ctx_index_put(rdpa_dir_ds, llid->ds_def_flow);

    llid->ds_def_flow = RDPA_UNMATCHED_DS_IC_RESULT_ID;

    return rc;
}

static int rdpa_rdd_tx_flow_cfg(uint32_t wan_flow,  uint32_t wan_port, bdmf_boolean crc_calc_en, bdmf_boolean enable)
{
    int rc = BDMF_ERR_OK;
    uint32_t data = 0;
    uint32_t cntr_id;

    rdd_tx_flow_enable(wan_flow, rdpa_dir_us, enable);

    if (enable)
    {
        data = wan_port | (crc_calc_en << 16);
        rc = ag_drv_bbh_tx_wan_configurations_flow2port_set(BBH_TX_WAN_ID, data, wan_flow, 1);
        rdpa_cntr_id_alloc(CNTR_GROUP_TX_FLOW, &cntr_id);
        rdd_tm_flow_cntr_cfg(wan_flow, cntr_id);
    }
    else
    {
        rdpa_cntr_id_dealloc(CNTR_GROUP_TX_FLOW, NONE_CNTR_SUB_GROUP_ID, wan_flow);
        rdd_tm_flow_cntr_cfg(wan_flow, TX_FLOW_CNTR_GROUP_INVLID_CNTR);
    }
    return rc;
}

/* Set llid channel */
int llid_set_channel_ex(llid_drv_priv_t *llid)
{
     int rc, i;

    /* assign free channel */
    rc = llid_set_l1_channel(llid);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "llid_set_l1_channel()--> %d\n", rc);

    for (i = 0; (i < llid_total_chan_num) && !rc; i++)
    {
        if (llid->channels[i] >= 0)
        {
            rc = rdpa_rdd_tx_flow_cfg(llid->channels[i], 0, 1, 1);
        }
    }

    return rc;
}

/********************************************************************************************
 * _if_channel_destroyed - check if the same channel was already deleted, assumed that      *
 *                         function called from for loop  where loop counter is incremented *
 *  index - loop index,                                                                     *
 *  llid - llid object                                                              *
 * ******************************************************************************************/
static bdmf_boolean _if_channel_destroyed(int index, llid_drv_priv_t *llid)
{
    int i;

    for (i = 0; i < index; i++)
    {
        if (llid->deleted_channels[i] == llid->channels[index]) 
            return 1;
    }
    
    llid->deleted_channels[index] = llid->channels[index];
    return 0;
}

void llid_destroy_ex(llid_drv_priv_t *llid)
{
    int i;

    for (i = 0; i < llid_total_chan_num; i++)
    {
        if (llid->channels[i] >= 0 && !_if_channel_destroyed(i, llid))
        {
            rdpa_rdd_tx_flow_cfg(llid->channels[i], 0, 0, 0);
        }
    }
}

int llid_attr_ds_def_flow_read_ex(struct bdmf_object *mo, 
    struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    llid_drv_priv_t *priv = (llid_drv_priv_t *)bdmf_obj_data(mo);

    if (priv->ds_def_flow == RDPA_UNMATCHED_DS_IC_RESULT_ID)
        return BDMF_ERR_NOENT;

    return gem_attr_ds_def_flow_read_ex(mo, ad, index, val, size);
}

int llid_set_def_flow_ex(llid_drv_priv_t *llid, rdpa_ic_result_t *cfg)
{
    int rc, ctx_idx;
    rdd_ic_context_t context = {};
    
    rc = classification_ctx_index_get(rdpa_dir_ds, rdpa_flow_def_flow_type, &ctx_idx);
    if (rc < 0)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Can't get free context index\n");

    rc = rdpa_map_to_rdd_classifier(rdpa_dir_ds, cfg, &context, 0, 0, RDPA_IC_TYPE_FLOW, 0);
    if (rc)
        return rc;

    rdpa_cntr_id_alloc(CNTR_GROUP_TCAM_DEF, &context.cntr_id);
    rdd_rx_default_flow_cfg(llid->index, ctx_idx, &context);
    rc = rdpa_ic_rdd_context_cfg(rdpa_dir_ds, ctx_idx, &context);
    if (!rc)
        llid->ds_def_flow = ctx_idx;

    return rc;
}

int llid_attr_counters_read(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    llid_drv_priv_t *llid = (llid_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_gem_stat_t *stat = (rdpa_gem_stat_t *)val;
    int rc = 0;
    uint32_t cntr_id;
    uint32_t cntr_arr[MAX_NUM_OF_COUNTERS_PER_READ] = {};

    if ((unsigned)index >= llid_total_chan_num)
        return BDMF_ERR_NO_MORE;
    if (llid->channels[index] < 0)
        return BDMF_ERR_NOENT;


    if (RDPA_EPON_CTRL_CH_INDEX == index)
    {
        cntr_id = rdd_rx_flow_cntr_id_get(llid->index);
        rc = drv_cntr_counter_read(CNTR_GROUP_RX_FLOW, cntr_id, cntr_arr);
        if (rc)
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Could not read CNTR flow counters for LLID flow %ld. err: %d\n", llid->index, rc);

        rdpa_common_update_cntr_results_uint32(&(stat->rx_packets), &accumulative_rx_llid_couners[cntr_id][0],
            0, cntr_arr[0]);
        rdpa_common_update_cntr_results_uint32(&(stat->rx_bytes), &accumulative_rx_llid_couners[cntr_id][1],
            0, cntr_arr[1]);
        rdpa_common_update_cntr_results_uint32(&(stat->rx_packets_discard), &accumulative_rx_llid_couners[cntr_id][2],
            0, cntr_arr[2]);
    }

    cntr_id = rdd_tm_flow_cntr_id_get(llid->channels[index]);
    rc = drv_cntr_counter_read(CNTR_GROUP_TX_FLOW, cntr_id, cntr_arr);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Could not read CNTR flow counters for LLID flow %ld. err: %d\n", llid->index, rc);

    rdpa_common_update_cntr_results_uint32(&(stat->tx_packets), &accumulative_tx_llid_couners[cntr_id][0],
        0, cntr_arr[0]);
    rdpa_common_update_cntr_results_uint32(&(stat->tx_bytes), &accumulative_tx_llid_couners[cntr_id][1],
        0, cntr_arr[1]);
    stat->tx_packets_discard = 0;

    return rc;
}

int llid_link_tc_to_queue(struct bdmf_object *mo, bdmf_boolean enable)
{
    int rc = BDMF_ERR_OK;

    /* 63158 does not support TC to queue */
#if !defined(BCM63158)
    bdmf_object_handle tc_to_q_obj = NULL;
    rdpa_tc_to_queue_key_t t2q_key;
    
    t2q_key.dir = rdpa_dir_us; 
    t2q_key.table = 0; 
    
    rc = rdpa_tc_to_queue_get(&t2q_key, &tc_to_q_obj);
    if (rc < 0)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Can't get tc_to_queue(dir:us, table:0)\n");
    
    if (enable)
        bdmf_link(mo, tc_to_q_obj, NULL);
    else
        bdmf_unlink(mo, tc_to_q_obj);
    
    bdmf_put(tc_to_q_obj);
#endif

    return rc;
}


