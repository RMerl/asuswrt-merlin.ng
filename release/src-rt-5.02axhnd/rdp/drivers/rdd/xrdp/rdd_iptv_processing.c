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


#include "rdd.h"
#include "rdd_iptv_processing.h"
#ifdef G9991
#include "XRDP_AG.h"

extern uintptr_t xrdp_virt2phys(const ru_block_rec *ru_block, uint8_t addr_idx);
#endif

void rdd_iptv_processing_cfg(RDD_HW_IPTV_CONFIGURATION_DTS *iptv_hw_config)
{
    RDD_HW_IPTV_CONFIGURATION_DDR_SOP_OFFSET0_WRITE_G(iptv_hw_config->ddr_sop_offset0, RDD_IPTV_CONFIGURATION_TABLE_ADDRESS_ARR, 0);
    RDD_HW_IPTV_CONFIGURATION_DDR_SOP_OFFSET1_WRITE_G(iptv_hw_config->ddr_sop_offset1, RDD_IPTV_CONFIGURATION_TABLE_ADDRESS_ARR, 0);
    RDD_HW_IPTV_CONFIGURATION_FPM_BASE_TOKEN_SIZE_WRITE_G(iptv_hw_config->fpm_base_token_size, RDD_IPTV_CONFIGURATION_TABLE_ADDRESS_ARR, 0);
    RDD_HW_IPTV_CONFIGURATION_HN_SIZE0_WRITE_G(iptv_hw_config->hn_size0, RDD_IPTV_CONFIGURATION_TABLE_ADDRESS_ARR, 0);
    RDD_HW_IPTV_CONFIGURATION_HN_SIZE1_WRITE_G(iptv_hw_config->hn_size1, RDD_IPTV_CONFIGURATION_TABLE_ADDRESS_ARR, 0);
#ifdef G9991
    RDD_HW_IPTV_CONFIGURATION_FPM_POOL1_STAT2_ADDR_WRITE_G(xrdp_virt2phys(&RU_BLK(FPM), 0) + RU_REG_OFFSET(FPM, POOL1_STAT2), RDD_IPTV_CONFIGURATION_TABLE_ADDRESS_ARR, 0);
#endif
}

