/*
* <:copyright-BRCM:2012-2015:proprietary:standard
*
*    Copyright (c) 2012-2015 Broadcom
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

/*
 * rdpa_xtm_ex.c
 *
 *  Created on: 2017
 *      Author: srinies
 */

#include "rdd.h"
#include <bdmf_dev.h>
#include "rdpa_api.h"
#include "rdpa_common.h"
#include "rdpa_int.h"
#include "rdpa_platform.h"
#include "rdp_drv_bbh_rx.h"
#include "rdd_runner_proj_defs.h"
#include "rdd_tcam_ic.h"
#include "rdp_drv_bbh_tx.h"
#include "rdpa_xtm_ex.h"
#include "rdp_drv_proj_cntr.h"
#include "xrdp_drv_qm_ag.h"

#define RDPA_XTM_FLOW_MAX_INDEX         2

/* xtmflow counters shadow */
static uint32_t accumulative_tx_xtmflow_counters[TX_FLOW_COUNTERS_NUM][RDPA_XTM_FLOW_MAX_INDEX] = {};

int _cfg_ds_xtm_channel(bdmf_index channel_idx)
{
    uint32_t cntr_id;
    RDD_RX_FLOW_ENTRY_DTS rx_flow_entry;

    rdpa_cntr_id_alloc(CNTR_GROUP_RX_FLOW, &cntr_id);

    rx_flow_entry.flow_dest = 0;
    rx_flow_entry.virtual_port = RDD_DSL_WAN_VPORT;
    rx_flow_entry.cntr_id = cntr_id;
    rdd_rx_flow_entry_set(RDD_DSL_RX_FLOW_ID(channel_idx), &rx_flow_entry);
    return 0;
}

/* "us_cfg" attribute "write" callback. */
int rdpa_us_wan_flow_config(uint32_t      wan_flow,
                            int           wan_channel,
                            uint32_t      wan_hdr_type,
                            uint32_t      wan_port_or_fstat,
                            bdmf_boolean  crc_calc,
                            int           ptm_bonding,
                            uint8_t       pbits_to_queue_table_index,
                            uint8_t       traffic_class_to_queue_table_index,
                            bdmf_boolean  enable)
{
    int rc = BDMF_ERR_OK;
    uint32_t data = 0;
    uint32_t cntr_id;

    bdmf_trace("******************************************************************************************\n");
    bdmf_trace("rdpa_us_wan_flow_config : wan_flow=%d, wan_channel=%d, wan_hdr_type=%d, fstat=%x, crc_calc=%d, enable=%d\n",
               wan_flow, wan_channel, wan_hdr_type, (unsigned int)wan_port_or_fstat, crc_calc, enable);
    bdmf_trace("******************************************************************************************\n");

    /* wan_hdr_type is necessary for padding and is not required now, so ignore
    ** the parameter.
    **/
    if (enable)
    {
        data = wan_port_or_fstat | (crc_calc << 16);
        rc = ag_drv_bbh_tx_wan_configurations_flow2port_set(BBH_TX_ID_DSL, data, wan_flow, 1);
        rdpa_cntr_id_alloc(CNTR_GROUP_TX_FLOW, &cntr_id);
        accumulative_tx_xtmflow_counters[cntr_id][RDPA_XTM_FLOW_PACKET_OFFSET] = 0;
        accumulative_tx_xtmflow_counters[cntr_id][RDPA_XTM_FLOW_BYTES_OFFSET]  = 0;
        rdd_tm_flow_cntr_cfg(RDD_TM_DSL_FLOW_ID(wan_flow), cntr_id);
        rdd_tx_flow_enable(RDD_TM_DSL_FLOW_ID(wan_flow), rdpa_dir_us, 1);
    }
    else
    {
        rdpa_cntr_id_dealloc(CNTR_GROUP_TX_FLOW, NONE_CNTR_SUB_GROUP_ID, RDD_TM_DSL_FLOW_ID(wan_flow));
        rdd_tm_flow_cntr_cfg(RDD_TM_DSL_FLOW_ID(wan_flow), TX_FLOW_CNTR_GROUP_INVLID_CNTR);
        rdd_tx_flow_enable(RDD_TM_DSL_FLOW_ID(wan_flow), rdpa_dir_us, 0);
    }

    return rc;
}

int rdpa_wan_dsl_channel_base(void)
{
   return 0; /* until the multi-wan support */
}

int rdpa_flow_pm_counters_get(int index, rdpa_xtmflow_stat_t *stat)
{
    int rc = 0;
    uint32_t cntr_id;
    uint32_t cntr_arr[MAX_NUM_OF_COUNTERS_PER_READ] = {};

    memset(stat, 0, sizeof(rdpa_xtmflow_stat_t));

    /* Unconfigured xtmflow - silently return BDMF_ERR_NOENT */
    if ((unsigned)index >= RDPA_MAX_XTMFLOW)
        return BDMF_ERR_NOENT;

    /* US - Tx Counters */
    cntr_id = rdd_tm_flow_cntr_id_get(RDD_TM_DSL_FLOW_ID(index));
    rc = drv_cntr_counter_read(CNTR_GROUP_TX_FLOW, cntr_id, cntr_arr);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Could not read CNTR flow counters for XTM flow %d. err: %d\n", index, rc);

    /* Keep counters values for accumulative_gem_stat */
    /* No transmit discards support due to the HW limitation */
    rdpa_common_update_cntr_results_uint32(&(stat->tx_packets), &(accumulative_tx_xtmflow_counters[cntr_id][RDPA_XTM_FLOW_PACKET_OFFSET]),
        0, cntr_arr[0]);

    rdpa_common_update_cntr_results_uint32(&(stat->tx_bytes), &(accumulative_tx_xtmflow_counters[cntr_id][RDPA_XTM_FLOW_BYTES_OFFSET]),
        0, cntr_arr[1]);

    return rc;
}

int rdpa_xtm_xtmchannel_id_to_tm_channel_id(int xtmchannel_id)
{
    return xtmchannel_id + RDD_US_CHANNEL_OFFSET_DSL;
}

int rdpa_xtm_txdrop_stats_get(rdpa_stat_t *tx_drop_stats)
{
    rdpa_stat_1way_t stat = {};
    int rc, i, tm_channel_id, rc_id, rdpa_queue;

    memset(tx_drop_stats, 0, sizeof(rdpa_stat_t));

    for (i = 0; i < RDPA_MAX_XTMCHANNEL; i++)
    {
        tm_channel_id = rdpa_xtm_xtmchannel_id_to_tm_channel_id(i);
        rc = _rdpa_egress_tm_channel_queue_to_rdd(rdpa_dir_us, tm_channel_id, 0, &rc_id, &rdpa_queue);
        if (rc)
            continue;

        BDMF_TRACE_DBG("xtm_chan=%d, tm_chan=%d, Tx_Queue = %d: ", i, tm_channel_id, rdpa_queue);

        rc = _rdpa_egress_tm_tx_queue_drop_stat_read(rdpa_queue, &stat);
        if (!rc)
            tx_drop_stats->packets += stat.discarded.packets;
    }
    return BDMF_ERR_OK;
}

int rdpa_xtmflow_attr_stat_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index,
    const void *val, uint32_t size)
{
    xtmflow_drv_priv_t *xtmflow = (xtmflow_drv_priv_t *)bdmf_obj_data(mo);

    /* Unconfigured xtmflow - silently return BDMF_ERR_NOENT */
    if ((unsigned)xtmflow->index >= RDPA_MAX_XTMFLOW)
        return BDMF_ERR_NOENT;

    accumulative_tx_xtmflow_counters[xtmflow->index][RDPA_XTM_FLOW_PACKET_OFFSET] = 0;
    accumulative_tx_xtmflow_counters[xtmflow->index][RDPA_XTM_FLOW_BYTES_OFFSET]  = 0;

    return BDMF_ERR_OK;
}
