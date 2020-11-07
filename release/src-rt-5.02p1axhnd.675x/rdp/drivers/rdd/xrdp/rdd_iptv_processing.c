/*
 * <:copyright-BRCM:2019:DUAL/GPL:standard
 * 
 *    Copyright (c) 2019 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 */


#include "rdd.h"
#include "rdd_iptv_processing.h"

void rdd_iptv_processing_cfg(RDD_HW_IPTV_CONFIGURATION_DTS *iptv_hw_config)
{
    RDD_HW_IPTV_CONFIGURATION_DDR_SOP_OFFSET0_WRITE_G(iptv_hw_config->ddr_sop_offset0, RDD_IPTV_CONFIGURATION_TABLE_ADDRESS_ARR, 0);
    RDD_HW_IPTV_CONFIGURATION_DDR_SOP_OFFSET1_WRITE_G(iptv_hw_config->ddr_sop_offset1, RDD_IPTV_CONFIGURATION_TABLE_ADDRESS_ARR, 0);
    RDD_HW_IPTV_CONFIGURATION_FPM_BASE_TOKEN_SIZE_WRITE_G(iptv_hw_config->fpm_base_token_size, RDD_IPTV_CONFIGURATION_TABLE_ADDRESS_ARR, 0);
    RDD_HW_IPTV_CONFIGURATION_HN_SIZE0_WRITE_G(iptv_hw_config->hn_size0, RDD_IPTV_CONFIGURATION_TABLE_ADDRESS_ARR, 0);
    RDD_HW_IPTV_CONFIGURATION_HN_SIZE1_WRITE_G(iptv_hw_config->hn_size1, RDD_IPTV_CONFIGURATION_TABLE_ADDRESS_ARR, 0);
}

