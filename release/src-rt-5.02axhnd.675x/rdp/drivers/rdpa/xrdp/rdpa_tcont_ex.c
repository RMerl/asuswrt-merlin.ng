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

/*
 * rdpa_tcont_ex.c
 *
 *  Created on: June 22, 2014
 *      Author: Yoni Itah
 */

#include "bdmf_dev.h"
#include "rdpa_api.h"
#include "rdpa_common.h"
#include "rdpa_int.h"

#include "xrdp_drv_bbh_rx_ag.h"
#include "rdp_drv_bbh_tx.h"

#ifndef RDD_US_CHANNEL_OFFSET_TCONT
#define RDD_US_CHANNEL_OFFSET_TCONT 0
#endif

extern int (*f_rdpa_wan_tx_bbh_flush_status_get)(uint8_t tcont_id, bdmf_boolean *bbh_flush_done_p);

#define get_tcont_indication(per_tcont_ind, tcont_id, state)  \
    do { \
            if (tcont_id < 32)  \
            { \
                state = ((per_tcont_ind[0] & (1 << tcont_id)) ? 1 : 0);   \
            } \
            else /* tconts 33-40 */ \
            { \
                state = ((per_tcont_ind[1] & (1 << (tcont_id-32))) ? 1 : 0); \
            } \
    } while (0)


int tcont_tx_bbh_flush_status_get(uint8_t tcont_id, bdmf_boolean *bbh_flush_done_p)
{
    bbh_tx_debug_counters_swrden debug_counters_swrden = {};
    uint32_t swrddata[2] = {0, 0};
    uint32_t pd_queue_empty = 1;
    int rc;
    uint32_t segmentation_inactive = 1;
#if defined(BCM6846) || defined(BCM6856) || defined(BCM6878)
    uint32_t in_segmentation[2] = {0, 0};
    uint32_t segmentation_active = 0;
#endif

    /* Read BBH flush status */
    /* 1. Enable PD Empty Select */
    debug_counters_swrden.pdemptysel = 1;

    rc = ag_drv_bbh_tx_debug_counters_swrden_set(BBH_TX_WAN_ID, &debug_counters_swrden);
    if (rc)
    {
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed to read flush status due to bbh driver error %d\n", rc);
    }

    /* 2. Set PD Empty Select re-direct adress - read  the first 32 tconts */

    rc = ag_drv_bbh_tx_debug_counters_swrdaddr_set(BBH_TX_WAN_ID, 0);
    if (rc)
    {
       BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed to read flush status due to bbh driver error %d\n", rc);
    }

    /* 3. Read PD Empty Select for tconts 1 - 32 */

    rc = ag_drv_bbh_tx_debug_counters_swrddata_get(BBH_TX_WAN_ID, &swrddata[0]);
    if (rc)
    {
       BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed to read flush status due to bbh driver error %d\n", rc);
    }

    /* 4. Set PD Empty Select re-direct adress - for tconts 33 - 40 */
    rc = ag_drv_bbh_tx_debug_counters_swrdaddr_set(BBH_TX_WAN_ID, 1);
    if (rc)
    {
       BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed to read flush status due to bbh driver error %d\n", rc);
    }

    /* 5. Read PD Empty Select for tconts 33 - 40 */

    rc = ag_drv_bbh_tx_debug_counters_swrddata_get(BBH_TX_WAN_ID, &swrddata[1]);
    if (rc)
    {
       BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed to read flush status due to bbh driver error %d\n", rc);
    }

    BDMF_TRACE_DBG("PD Empty                       0x%08X 0x%08X", swrddata[0], swrddata[1]);

#if defined(BCM6846) || defined(BCM6856) || defined(BCM6878)
    /* 
       In these SoCs BBH Tx has yet another indicator to be checked -  IN_SEGMENTATION 
       (Segmentation State Machine is currently handling a PD of a certain T-CONT)
    */
    /* Cleanup */
    rc = ag_drv_bbh_tx_debug_counters_in_segmentation_get(BBH_TX_WAN_ID, 0, &in_segmentation[0]);
    if (rc)
    {
       BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed to read IN_SEGMENT status due to bbh driver error %d\n", rc);
    }
    rc = ag_drv_bbh_tx_debug_counters_in_segmentation_get(BBH_TX_WAN_ID, 1, &in_segmentation[1]);
    if (rc)
    {
       BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed to read IN_SEGMENT status due to bbh driver error %d\n", rc);
    }
    BDMF_TRACE_DBG("IN_SEGMENTATION before cleanup 0x%08X 0x%08X", in_segmentation[0], in_segmentation[1]);

    /* 6. Now read indication only for first 32 tconts */
    rc = ag_drv_bbh_tx_debug_counters_in_segmentation_get(BBH_TX_WAN_ID, 0, &in_segmentation[0]);
    if (rc)
    {
       BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed to read IN_SEGMENT status due to bbh driver error %d\n", rc);
    }
    rc = ag_drv_bbh_tx_debug_counters_in_segmentation_get(BBH_TX_WAN_ID, 1, &in_segmentation[1]);
    if (rc)
    {
       BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed to read IN_SEGMENT status due to bbh driver error %d\n", rc);
    }

    BDMF_TRACE_DBG("IN_SEGMENTATION after  cleanup 0x%08X 0x%08X", in_segmentation[0], in_segmentation[1]);

    get_tcont_indication(in_segmentation, tcont_id, segmentation_active);

    segmentation_inactive = !segmentation_active;
#endif

    get_tcont_indication(swrddata, tcont_id, pd_queue_empty);
    *bbh_flush_done_p = pd_queue_empty && segmentation_inactive;

    return 0;
}

uint8_t tcont_tc_table_get(bdmf_object_handle tcont)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

uint8_t tcont_pbit_table_get(bdmf_object_handle tcont)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int rdpa_tcont_sr_dba_callback(uint32_t tcont_id, uint32_t *runner_ddr_occupancy)
{
    return BDMF_ERR_NOT_SUPPORTED;
}

int rdpa_tcont_tcont_id_to_channel_id(int tcont_id)
{
    if (tcont_id)
        return tcont_id - 1 + RDD_US_CHANNEL_OFFSET_TCONT;
    else
        return RDPA_MAX_TCONT - 1 + RDD_US_CHANNEL_OFFSET_TCONT;
}


