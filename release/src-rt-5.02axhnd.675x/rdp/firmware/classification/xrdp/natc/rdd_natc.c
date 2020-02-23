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


#include "rdd_natc.h"

void rdd_natc_tbl_cfg(uint8_t tbl_idx, uint32_t key_addr_hi, uint32_t key_addr_lo, uint32_t res_addr_hi, uint32_t res_addr_lo)
{
#ifdef SW_CACHE_MISS_HANDLE
    RDD_NATC_TBL_CONFIGURATION_MISS_CACHE_ENABLE_WRITE_G(1, RDD_NATC_TBL_CFG_ADDRESS_ARR, tbl_idx);
#else
    /* already configured HW ddr_update_on_cache_miss (data_path_init)*/
#endif
    RDD_NATC_TBL_CONFIGURATION_KEY_SIZE_WRITE_G(NATC_TABLE_KEY_SIZE, RDD_NATC_TBL_CFG_ADDRESS_ARR, tbl_idx);
    RDD_NATC_TBL_CONFIGURATION_CONTEXT_SIZE_WRITE_G(NATC_TABLE_RES_SIZE, RDD_NATC_TBL_CFG_ADDRESS_ARR, tbl_idx);
    
    RDD_NATC_TBL_CONFIGURATION_KEY_ADDR_HIGH_WRITE_G(key_addr_hi, RDD_NATC_TBL_CFG_ADDRESS_ARR, tbl_idx);
    RDD_NATC_TBL_CONFIGURATION_KEY_ADDR_LOW_WRITE_G(key_addr_lo, RDD_NATC_TBL_CFG_ADDRESS_ARR, tbl_idx);
       
    RDD_NATC_TBL_CONFIGURATION_RES_ADDR_HIGH_WRITE_G(res_addr_hi, RDD_NATC_TBL_CFG_ADDRESS_ARR, tbl_idx);
    RDD_NATC_TBL_CONFIGURATION_RES_ADDR_LOW_WRITE_G(res_addr_lo, RDD_NATC_TBL_CFG_ADDRESS_ARR, tbl_idx);        
}

void rdd_natc_l2_fc_enable(int enable)
{
    RDD_SYSTEM_CONFIGURATION_ENTRY_L2_FLOW_CACHE_MODE_WRITE_G(enable, RDD_SYSTEM_CONFIGURATION_ADDRESS_ARR, 0);
}

int rdd_natc_l2_fc_get(void)
{
    int enable;

    RDD_SYSTEM_CONFIGURATION_ENTRY_L2_FLOW_CACHE_MODE_READ_G(enable, RDD_SYSTEM_CONFIGURATION_ADDRESS_ARR, 0);
    return enable;
}

