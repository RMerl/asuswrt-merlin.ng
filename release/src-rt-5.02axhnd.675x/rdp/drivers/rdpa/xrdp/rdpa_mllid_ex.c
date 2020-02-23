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
 :>
*/

#include "rdd.h"
#include "bdmf_dev.h"
#include "rdpa_api.h"
#include "rdpa_int.h"
#include "rdpa_ingress_class_int.h"
#include "rdp_drv_bbh_rx.h"
#include "rdd_runner_proj_defs.h"
#include "rdd_tcam_ic.h"
#include "rdp_drv_bbh_tx.h"
#include "rdpa_mllid.h"
#include "rdp_drv_proj_cntr.h"
#include "xrdp_drv_qm_ag.h"


extern struct bdmf_object *mllid_objects[RDPA_EPON_MLLID_NUM];

/* MLLID counters shadow */
static uint32_t accumulative_rx_mllid_counters[RX_FLOW_COUNTERS_NUM][3] = {};


int mllid_attr_stat_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    int rc = 0;
    uint32_t cntr_id;
    uint32_t cntr_arr[MAX_NUM_OF_COUNTERS_PER_READ] = {};
    rdpa_mllid_stat_t *stat = (rdpa_mllid_stat_t *)val;
    mllid_drv_priv_t *mllid = (mllid_drv_priv_t *)bdmf_obj_data(mo);

    /* Unconfigured MLLID flow - silently return BDMF_ERR_NOENT */
    if ((unsigned)mllid->index >= RDPA_EPON_MLLID_NUM || mllid_objects[mllid->index] != mo)
        return BDMF_ERR_NOENT;

    memset(stat, 0, sizeof(rdpa_mllid_stat_t));

    /* RX counters */
    cntr_id = rdd_rx_flow_cntr_id_get(mllid->flow_id);
    rc = drv_cntr_counter_read(CNTR_GROUP_RX_FLOW, cntr_id, cntr_arr);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Could not read CNTR flow counters for mllid flow %ld. err: %d\n", mllid->index, rc);

    rdpa_common_update_cntr_results_uint32(&(stat->rx_packets), &accumulative_rx_mllid_counters[cntr_id][0], 0, cntr_arr[0]);
    rdpa_common_update_cntr_results_uint32(&(stat->rx_bytes), &accumulative_rx_mllid_counters[cntr_id][1], 0, cntr_arr[1]);
    rdpa_common_update_cntr_results_uint32(&(stat->rx_packets_discard), &accumulative_rx_mllid_counters[cntr_id][2], 0, cntr_arr[2]);
 
    return rc;
}

int mllid_attr_stat_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size)
{
    int rc = 0;
    uint32_t cntr_id;
    uint32_t cntr_arr[MAX_NUM_OF_COUNTERS_PER_READ] = {};
    mllid_drv_priv_t *mllid = (mllid_drv_priv_t *)bdmf_obj_data(mo);

    cntr_id = rdd_rx_flow_cntr_id_get(mllid->flow_id);
    /* read the counters make sure to clear them */
    drv_cntr_counter_read(CNTR_GROUP_RX_FLOW, cntr_id, cntr_arr);

    accumulative_rx_mllid_counters[cntr_id][0] = 0;
    accumulative_rx_mllid_counters[cntr_id][1] = 0;
    accumulative_rx_mllid_counters[cntr_id][2] = 0;


    return rc;
}

int mllid_post_init_ex(bdmf_index index, bdmf_index flow_id)
{
    uint32_t cntr_id;

    cntr_id = rdd_rx_flow_cntr_id_get(flow_id);
    memset(accumulative_rx_mllid_counters[cntr_id], 0, 3 * sizeof(uint32_t));

    return BDMF_ERR_OK;
}


