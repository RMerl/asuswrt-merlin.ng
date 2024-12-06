/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:

       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.

    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.

:>
*/

#ifndef _DRV_EPON_NCO_ADDR_AG_H_
#define _DRV_EPON_NCO_ADDR_AG_H_

#include "access_macros.h"
#if !defined(_CFE_)
#include "bdmf_interface.h"
#else
#include "bdmf_data_types.h"
#include "bdmf_errno.h"
#endif
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
typedef struct
{
    bdmf_boolean cfgbypass;
    uint8_t cfgsrcout10mhz;
    uint8_t cfgsrcout;
    uint8_t cfgsrcin;
    bdmf_boolean cfgncoclr;
} nco_addr_nco_cfg;

bdmf_error_t ag_drv_nco_addr_nco_cfg_set(const nco_addr_nco_cfg *nco_cfg);
bdmf_error_t ag_drv_nco_addr_nco_cfg_get(nco_addr_nco_cfg *nco_cfg);
bdmf_error_t ag_drv_nco_addr_nco_int_set(bdmf_boolean intnoncosync, bdmf_boolean intnoxifpps, bdmf_boolean intnolifpps);
bdmf_error_t ag_drv_nco_addr_nco_int_get(bdmf_boolean *intnoncosync, bdmf_boolean *intnoxifpps, bdmf_boolean *intnolifpps);
bdmf_error_t ag_drv_nco_addr_nco_msk_set(bdmf_boolean intnoncosyncmask, bdmf_boolean intnoxifppsmask, bdmf_boolean intnolifppsmask);
bdmf_error_t ag_drv_nco_addr_nco_msk_get(bdmf_boolean *intnoncosyncmask, bdmf_boolean *intnoxifppsmask, bdmf_boolean *intnolifppsmask);
bdmf_error_t ag_drv_nco_addr_nco_1pps_period_set(uint32_t cfg1ppsperiod);
bdmf_error_t ag_drv_nco_addr_nco_1pps_period_get(uint32_t *cfg1ppsperiod);
bdmf_error_t ag_drv_nco_addr_nco_8khz_period_set(uint32_t cfg8khzperiod);
bdmf_error_t ag_drv_nco_addr_nco_8khz_period_get(uint32_t *cfg8khzperiod);
bdmf_error_t ag_drv_nco_addr_nco_center_frequency_set(uint32_t cfgncodefault);
bdmf_error_t ag_drv_nco_addr_nco_center_frequency_get(uint32_t *cfgncodefault);
bdmf_error_t ag_drv_nco_addr_nco_int_gain_set(uint16_t cfgncogain);
bdmf_error_t ag_drv_nco_addr_nco_int_gain_get(uint16_t *cfgncogain);
bdmf_error_t ag_drv_nco_addr_nco_pro_gain_set(uint16_t cfgncopropgain);
bdmf_error_t ag_drv_nco_addr_nco_pro_gain_get(uint16_t *cfgncopropgain);
bdmf_error_t ag_drv_nco_addr_nco_cnt_get(uint32_t *ncocnt);
bdmf_error_t ag_drv_nco_addr_nco_1pps_half_set(uint32_t cfg1ppshalfperiod);
bdmf_error_t ag_drv_nco_addr_nco_1pps_half_get(uint32_t *cfg1ppshalfperiod);
bdmf_error_t ag_drv_nco_addr_nco_8khz_half_set(uint32_t cfg8khzhalfperiod);
bdmf_error_t ag_drv_nco_addr_nco_8khz_half_get(uint32_t *cfg8khzhalfperiod);
bdmf_error_t ag_drv_nco_addr_nco_period_cnt_get(uint32_t *periodcnt);
bdmf_error_t ag_drv_nco_addr_nco_phs_err_cnt_get(uint16_t *ncophserr);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_nco_addr_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

