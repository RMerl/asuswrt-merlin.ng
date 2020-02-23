/*
* <:copyright-BRCM:2014:proprietary:standard
* 
*    Copyright (c) 2014 Broadcom 
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


#include "rdd_ingress_filter.h"
#include "rdd_ag_processing.h"
#include "rdp_drv_proj_cntr.h"

int rdd_ingress_filter_module_init(const rdd_module_t *module)
{
    uint8_t ix, l2_filter_reasons[INGRESS_FILTER_L2_REASON_TABLE_SIZE] = { };

    /* init L2 filter reasons */
    l2_filter_reasons[INGRESS_FILTER_ETYPE_PPPOE_D] = CPU_RX_REASON_ETYPE_PPPOE_D;
    l2_filter_reasons[INGRESS_FILTER_ETYPE_PPPOE_S] = CPU_RX_REASON_ETYPE_PPPOE_S;
    l2_filter_reasons[INGRESS_FILTER_ETYPE_UDEF_0] = CPU_RX_REASON_ETYPE_UDEF_0;
    l2_filter_reasons[INGRESS_FILTER_ETYPE_UDEF_1] = CPU_RX_REASON_ETYPE_UDEF_1;
    l2_filter_reasons[INGRESS_FILTER_ETYPE_UDEF_2] = CPU_RX_REASON_ETYPE_UDEF_2;
    l2_filter_reasons[INGRESS_FILTER_ETYPE_UDEF_3] = CPU_RX_REASON_ETYPE_UDEF_3;
    l2_filter_reasons[INGRESS_FILTER_ETYPE_ARP] = CPU_RX_REASON_ETYPE_ARP;
    l2_filter_reasons[INGRESS_FILTER_ETYPE_802_1X] = CPU_RX_REASON_ETYPE_802_1X;
    l2_filter_reasons[INGRESS_FILTER_ETYPE_802_1AG_CFM] = CPU_RX_REASON_ETYPE_802_1AG_CFM;
    l2_filter_reasons[INGRESS_FILTER_ETYPE_PTP_1588] = CPU_RX_REASON__1588;

    /* init res_offset in cfg... */
    RDD_INGRESS_FILTER_CFG_RES_OFFSET_WRITE_G((uint16_t)module->res_offset, RDD_INGRESS_FILTER_CFG_ADDRESS_ARR, 0);

    /* init profiles table */
    for (ix = 0; ix < RDD_INGRESS_FILTER_PROFILE_TABLE_SIZE; ix++)
    {
        RDD_INGRESS_FILTER_CTRL_ENABLE_VECTOR_WRITE_G(0, RDD_INGRESS_FILTER_PROFILE_TABLE_ADDRESS_ARR, ix);
        RDD_INGRESS_FILTER_CTRL_ACTION_VECTOR_WRITE_G(0, RDD_INGRESS_FILTER_PROFILE_TABLE_ADDRESS_ARR, ix);
    }

    for (ix = 0; ix < INGRESS_FILTER_L2_REASON_TABLE_SIZE; ix++)
        RDD_BYTE_1_BITS_WRITE_G(l2_filter_reasons[ix], RDD_INGRESS_FILTER_L2_REASON_TABLE_ADDRESS_ARR, ix);

    return 0;
}

int rdd_ingress_filter_vport_to_profile_set(uint8_t vport, uint8_t profile)
{
#if defined(BCM63158)
    return rdd_ag_processing_vport_cfg_table_ingress_filter_profile_set(vport, profile);
#else
    return rdd_ag_processing_vport_cfg_ex_table_ingress_filter_profile_set(vport, profile);
#endif
}

void rdd_ingress_filter_profile_cfg(uint8_t profile, uint32_t filter_mask, uint32_t action_mask)
{
    RDD_INGRESS_FILTER_CTRL_ENABLE_VECTOR_WRITE_G(filter_mask, RDD_INGRESS_FILTER_PROFILE_TABLE_ADDRESS_ARR, profile);
    RDD_INGRESS_FILTER_CTRL_ACTION_VECTOR_WRITE_G(action_mask, RDD_INGRESS_FILTER_PROFILE_TABLE_ADDRESS_ARR, profile);
}

void rdd_ingress_filter_1588_cfg(int enable)
{
    RDD_BYTE_1_BITS_WRITE_G(enable, RDD_INGRESS_FILTER_1588_CFG_ADDRESS_ARR, 0);
}

int rdd_ingress_filter_drop_counter_get(uint8_t filter, rdpa_traffic_dir dir, uint16_t *drop_counter)
{
    int counter_idx;

    if (dir == rdpa_dir_ds)
        counter_idx = filter + COUNTER_INGRESS_FILTER_DROP_FIRST_DS;
    else
        counter_idx = filter + COUNTER_INGRESS_FILTER_DROP_FIRST_US;

    return drv_cntr_various_counter_get(counter_idx, drop_counter);
}

void rdd_ingress_filter_cpu_bypass_cfg_set(uint8_t profile, bdmf_boolean cpu_bypass)
{
    RDD_INGRESS_FILTER_CTRL_CPU_BYPASS_WRITE_G((int)cpu_bypass, RDD_INGRESS_FILTER_PROFILE_TABLE_ADDRESS_ARR, profile);
}

