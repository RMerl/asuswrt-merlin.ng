/*
* <:copyright-BRCM:2014-2015:proprietary:standard
*
*    Copyright (c) 2014-2015 Broadcom
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

#include <bdmf_dev.h>
#include "rdpa_api.h"
#include "rdpa_tcpspdtest.h"
#include "rdd_tcpspdtest.h"
#include "rdp_drv_sbpm.h"
#include "rdpa_tcpspdtest_ex.h"
#include "rdd_ag_pktgen_tx.h"
#include "rdd_ag_tcpspdtest.h"
#include "xrdp_drv_rnr_regs_ag.h"
#include "xrdp_drv_qm_ag.h"
#include "rdpa_cpu_ex.h"
#include "rdpa_spdtest_common_ex.h"

int tcpspdtest_pre_init_ex(struct bdmf_object *mo)
{
    tcpspdtest_drv_priv_t *tcpspdtest = (tcpspdtest_drv_priv_t *)bdmf_obj_data(mo);
    
    spdt_tx_sbpm_bn_reset(&tcpspdtest->engine_ref_pkt);
    return 0;
}

int tcpspdtest_post_init_ex(struct bdmf_object *mo)
{
    return 0;
}

void rdpa_tcpspdtest_ut_pktgen_tx_wakeup_ex(void)
{
    int core_id = get_runner_idx(tcpspdtest_engine_runner_image);
   
#ifdef CONFIG_PKTGEN_UT

#if defined(BCM6858)
    /* 4 comes for LAN3 */
    RDD_PKTGEN_TX_STREAM_ENTRY_WAN_FLOW_WRITE_G(0x84, RDD_PKTGEN_TX_STREAM_TABLE_ADDRESS_ARR, 2);
    /* 0xa8 comes for a QM queue that configured for LAN3 egress_tm object (egress_tm/dir=ds,index=2). */
    RDD_PKTGEN_TX_STREAM_ENTRY_TX_QM_QUEUE_WRITE_G(0xa8, RDD_PKTGEN_TX_STREAM_TABLE_ADDRESS_ARR, 2);
#endif
#if defined(BCM6846)
    /* 4 comes for LAN3 */
    RDD_PKTGEN_TX_STREAM_ENTRY_WAN_FLOW_WRITE_G(0x84, RDD_PKTGEN_TX_STREAM_TABLE_ADDRESS_ARR, 2);
    /* 0x36 comes for a QM queue that configured for LAN3 egress_tm object (egress_tm/dir=ds,index=2). */
    RDD_PKTGEN_TX_STREAM_ENTRY_TX_QM_QUEUE_WRITE_G(0x36, RDD_PKTGEN_TX_STREAM_TABLE_ADDRESS_ARR, 2);
#endif
    
    spdt_tx_defs_init(CPU_RX_COPY_THREAD_NUMBER);

    spdt_tx_fpm_ug_budget_set(1); /* We transmit to LAN, UG = DS */
    RDD_SPDTEST_GEN_CFG_IS_ON_WRITE_G(1, RDD_SPDTEST_GEN_PARAM_ADDRESS_ARR, 0); /* Tell NATC that test is running */

#endif /* CONFIG_PKTGEN_UT */

#ifndef RDP_SIM   
    ag_drv_rnr_regs_cfg_cpu_wakeup_set(core_id, CPU_RX_COPY_THREAD_NUMBER);
#else 
    rdp_cpu_runner_wakeup(core_id, CPU_RX_COPY_THREAD_NUMBER);
#endif   
}

int rdpa_tcpspdtest_attr_engine_conn_info_write_ex(struct bdmf_object *mo, bdmf_index index,
    rdpa_tcpspdtest_engine_conn_info_t *conn_info)
{
    RDD_TCPSPDTEST_ENGINE_CONN_INFO_DTS rdd_conn_info;
    RDD_PKTGEN_TX_STREAM_ENTRY_DTS rdd_pktgen_tx_stream = {};
    int tx_hdr_len;
    uint32_t fpm_ug;

    spdt_tx_defs_init(CPU_RX_COPY_THREAD_NUMBER);

    if (rdpa_if_is_wan(conn_info->port))
    {
        /* Upstream */
        fpm_ug = 1;
        rdd_pktgen_tx_stream.entry_parms_wan_flow = conn_info->wan_flow;
    }
    else
    {
        /* Downstream */
        fpm_ug = 0;
        rdd_pktgen_tx_stream.entry_parms_wan_flow = 0;
    }
    rdd_pktgen_tx_stream.entry_parms_wan_flow = BB_ID_CPU0; 
    spdt_tx_fpm_ug_budget_set(fpm_ug);

    rdd_pktgen_tx_stream.entry_parms_l2_hdr_len = conn_info->l2_hdr_len;
    rdd_pktgen_tx_stream.entry_parms_l3_hdr_len = conn_info->l3_hdr_len;
    rdd_pktgen_tx_stream.entry_parms_l3_protocol = conn_info->l3_protocol;
    tx_hdr_len = rdd_pktgen_tx_stream.entry_parms_l2_hdr_len + rdd_pktgen_tx_stream.entry_parms_l3_hdr_len + conn_info->l4_hdr_len;
    rdd_pktgen_tx_stream.entry_parms_tx_hdr_len = (RDPA_TCPSPDTEST_MIN_TX_PD_LEN <= tx_hdr_len ? tx_hdr_len :
        RDPA_TCPSPDTEST_MIN_TX_PD_LEN);
    rdd_conn_info.cpu_rx_rdd_queue = conn_info->cpu_rx_rdd_queue;

    rdd_pktgen_tx_stream.entry_parms_tx_qm_queue = 0; /* Send to dispatcher */
    rdd_conn_info.up_tx_mss = conn_info->up_tx_mss;
    rdd_conn_info.up_tx_max_pd_len = conn_info->up_tx_max_pd_len;
    rdd_conn_info.up_pppoe_hdr_ofs = conn_info->up_pppoe_hdr_ofs;
    rdd_conn_info.up_peer_rx_scale = conn_info->up_peer_rx_scale;
    rdd_conn_info.sack_permitted = conn_info->sack_permitted;

    return rdd_tcpspdtest_engine_conn_info_set(&rdd_conn_info, &rdd_pktgen_tx_stream, (uint8_t)index);
}

int tcpspdtest_attr_is_on_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad,
    bdmf_index index, const void *val, uint32_t size)
{
    bdmf_boolean is_on = *(bdmf_boolean *)val;
    tcpspdtest_drv_priv_t *tcpspdtest = (tcpspdtest_drv_priv_t *)bdmf_obj_data(mo);

    RDD_SPDTEST_GEN_CFG_IS_ON_WRITE_G(is_on, RDD_SPDTEST_GEN_PARAM_ADDRESS_ARR, 0);
    tcpspdtest->is_on = is_on;
    return 0;
}

