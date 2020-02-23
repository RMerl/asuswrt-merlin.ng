/*
 * <:copyright-BRCM:2013:proprietary:standard
 * 
 *    Copyright (c) 2013 Broadcom 
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

#include "rdpa_api.h"
#include "rdpa_int.h"
#include "rdp_drv_cntr.h"
#include "rdd_tcam_ic.h"
#include "rdd_common.h"

extern unsigned int util_get_chip_id(void);

int rdpa_cntr_id_alloc(uint32_t group_id, uint32_t *cntr_id)
{
    int rc = 0;
    rc = drv_cntr_counter_alloc(group_id, cntr_id);
    if (rc)
    {
        BDMF_TRACE_ERR("Cannot allocate counter for Group: %d ; error %d\n", group_id, rc);
    }
    return rc;
}

void rdpa_cntr_id_dealloc(int group_id, cntr_sub_group_id_t sub_group, uint32_t entry_idx)
{
    int rc = 0;
    uint32_t cntr_id = 0;

    switch (group_id)
    {
    case CNTR_GROUP_RX_FLOW:
        cntr_id = rdd_rx_flow_cntr_id_get(entry_idx);
        if (cntr_id == RX_FLOW_CNTR_GROUP_INVLID_CNTR)
        {
            BDMF_TRACE_ERR("RX_FLOW Can't deallocate counter for Group: %d ; counter: %d\n", group_id, cntr_id);
            return;
        }
        break;
    case CNTR_GROUP_TX_FLOW:
        cntr_id = rdd_tm_flow_cntr_id_get(entry_idx);
        if (cntr_id == TX_FLOW_CNTR_GROUP_INVLID_CNTR)
        {
            BDMF_TRACE_ERR("TX_FLOW Can't deallocate counter for Group: %d ; counter: %d\n", group_id, cntr_id);
            return;
        }
        break;
    case CNTR_GROUP_IPTV_NATC:
        cntr_id = entry_idx;
        break;
    case CNTR_GROUP_TCAM_DEF:
        if (sub_group == DEF_FLOW_CNTR_SUB_GROUP_ID)
            cntr_id = rdd_rx_default_flow_cntr_id_get(entry_idx);
        else if (sub_group == IPTV_CNTR_SUB_GROUP_ID)
            cntr_id = entry_idx;
        /* else if (sub_group == TCAM_CNTR_SUB_GROUP_ID)
           dealloc is done in rdpa_ic_rdd_rule_delete "ingress_class_ex.c"
           cntr_id is taken from RDPA database
        */
        else
            cntr_id = TCAM_DEF_CNTR_GROUP_INVLID_CNTR;

        if (cntr_id == TCAM_DEF_CNTR_GROUP_INVLID_CNTR)
        {
            BDMF_TRACE_ERR("Can't deallocate counter for Group: %d ; sub_group: %d ; counter: %d\n", group_id, sub_group, cntr_id);
            return;
        }
        break;
    default:
        return;
    }

    rc = drv_cntr_counter_dealloc(group_id, cntr_id);
    if (rc)
        BDMF_TRACE_ERR("Can't deallocate counter for Group: %d ; sub_group: %d ; counter: %d. error %d\n", group_id, sub_group, cntr_id, rc);
}

bdmf_boolean rdpa_is_bcm6836_chip(void)
{
#ifndef RDP_SIM
    return ((util_get_chip_id() & ~0xF) == 0x68360);
#else
    return 0;
#endif
}

