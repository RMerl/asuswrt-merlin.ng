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

#include "bdmf_dev.h"
#include "rdpa_api.h"
#include "rdpa_common.h"
#include "rdpa_int.h"
#include "rdpa_ingress_class_int.h"
#include "rdd.h"
#include "rdp_drv_bbh.h"
#include "rdd_ih_defs.h"
#include "rdpa_mllid.h"

/* MLLID port Counters  - accumulative */
static uint16_t accumulate_ih_packets_discard[RDPA_EPON_MLLID_NUM];

extern struct bdmf_object *mllid_objects[RDPA_EPON_MLLID_NUM];


int rdpa_mllid_flow_pm_counters_get(bdmf_index index, bdmf_index flow_id, rdpa_mllid_stat_t *stat)
{
    int rc = 0;
    uint16_t ih_packets_discard = 0;
    rdd_flow_pm_counters_t rdd_flow_counters = {};
    DRV_BBH_PORT_INDEX bbh_port = DRV_BBH_EPON;

    /* DS */
    rc = rdd_flow_pm_counters_get(flow_id, RDD_FLOW_PM_COUNTERS_DS, 0, &rdd_flow_counters);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Could not read RDD flow counters for MLLID flow %ld\n", flow_id);

    rc = fi_bl_drv_bbh_rx_get_per_flow_counters(bbh_port, flow_id, &ih_packets_discard);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Could not read BBH Rx flow counters for MLLID flow %ld\n", flow_id);
    accumulate_ih_packets_discard[index] += ih_packets_discard;

    /* Keep counters values for accumulate_mllid_stat */
    stat->rx_packets = rdd_flow_counters.good_rx_packet;
    stat->rx_bytes = rdd_flow_counters.good_rx_bytes;
    stat->rx_packets_discard = rdd_flow_counters.error_rx_packets_discard + accumulate_ih_packets_discard[index];

    return rc;
}

int mllid_attr_stat_read_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    int rc = 0;
    mllid_drv_priv_t *mllid = (mllid_drv_priv_t *)bdmf_obj_data(mo);
    rdpa_mllid_stat_t *stat = (rdpa_mllid_stat_t *)val;

    /* Unconfigured MLLID flow - silently return BDMF_ERR_NOENT */
    if ((unsigned)mllid->index >= RDPA_EPON_MLLID_NUM || mllid_objects[mllid->index] != mo)
        return BDMF_ERR_NOENT;

    rc = rdpa_mllid_flow_pm_counters_get(mllid->index, mllid->flow_id, stat);

    return rc;
}

int mllid_attr_stat_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size)
{
    int rc = 0;
    uint16_t ih_packets_discard = 0;
    rdd_flow_pm_counters_t rdd_flow_counters = {};
    mllid_drv_priv_t *mllid = (mllid_drv_priv_t *)bdmf_obj_data(mo);
    DRV_BBH_PORT_INDEX bbh_port = DRV_BBH_EPON;
    
    rc = rdd_flow_pm_counters_get(mllid->flow_id, RDD_FLOW_PM_COUNTERS_DS, 1, &rdd_flow_counters);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Could not clear RDD flow counters for MLLID flow %ld\n", mllid->flow_id);

    rc = fi_bl_drv_bbh_rx_get_per_flow_counters(bbh_port, mllid->flow_id, &ih_packets_discard);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Could not clear BBH Rx flow counters for MLLID flow %ld\n", mllid->flow_id);
    accumulate_ih_packets_discard[mllid->index] = 0;

    return rc;
}

int mllid_post_init_ex(bdmf_index index, bdmf_index flow_id)
{
    accumulate_ih_packets_discard[index] = 0;
    return BDMF_ERR_OK;
}

