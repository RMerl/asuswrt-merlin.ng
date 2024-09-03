/*
 * <:copyright-BRCM:2019:DUAL/GPL:standard
 * 
 *    Copyright (c) 2019 Broadcom 
 *    All Rights Reserved
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * :>
 */


#include "rdd.h"
#include "rdd_iptv_processing.h"

void rdd_iptv_processing_cfg(HW_IPTV_CONFIGURATION_STRUCT *iptv_hw_config)
{
    RDD_HW_IPTV_CONFIGURATION_DDR_SOP_OFFSET0_WRITE_G(iptv_hw_config->ddr_sop_offset0, RDD_IPTV_CONFIGURATION_TABLE_ADDRESS_ARR, 0);
    RDD_HW_IPTV_CONFIGURATION_DDR_SOP_OFFSET1_WRITE_G(iptv_hw_config->ddr_sop_offset1, RDD_IPTV_CONFIGURATION_TABLE_ADDRESS_ARR, 0);
    RDD_HW_IPTV_CONFIGURATION_FPM_BASE_TOKEN_SIZE_WRITE_G(iptv_hw_config->fpm_base_token_size, RDD_IPTV_CONFIGURATION_TABLE_ADDRESS_ARR, 0);
    RDD_HW_IPTV_CONFIGURATION_HN_SIZE0_WRITE_G(iptv_hw_config->hn_size0, RDD_IPTV_CONFIGURATION_TABLE_ADDRESS_ARR, 0);
    RDD_HW_IPTV_CONFIGURATION_HN_SIZE1_WRITE_G(iptv_hw_config->hn_size1, RDD_IPTV_CONFIGURATION_TABLE_ADDRESS_ARR, 0);
#if defined(OPERATION_MODE_PRV)
    RDD_HW_IPTV_CONFIGURATION_FPM_POOL1_STAT2_ADDR_WRITE_G(iptv_hw_config->fpm_pool1_stat2_addr, RDD_IPTV_CONFIGURATION_TABLE_ADDRESS_ARR, 0);
#endif
}

