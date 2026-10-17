/*
    <:copyright-BRCM:2015:DUAL/GPL:standard
    
       Copyright (c) 2015 Broadcom 
       All Rights Reserved
    
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
#ifndef _RDP_DRV_SBPM_H_
#define _RDP_DRV_SBPM_H_

#include "xrdp_drv_drivers_common_ag.h"
#include "xrdp_autogens.h"

typedef enum {
  SBPM_TYPE_INGRESS,
  SBPM_TYPE_EGRESS
} sbpm_type_e;

#define SBPM_BUF_SIZE 128

bdmf_error_t drv_sbpm_alloc_list(uint32_t size, uint32_t headroom, uint8_t *data,
    uint16_t *bn0, uint16_t *bn1, uint8_t *bns_num);

bdmf_error_t drv_sbpm_alloc_and_connect_next_bn(uint32_t size, uint32_t headroom, uint8_t *data, uint16_t next_bn,
    bdmf_boolean inc_next_mc_counter, uint16_t *bn);

bdmf_error_t drv_sbpm_free_list(sbpm_type_e sbpm_type, uint8_t sbpm_id, uint16_t head_bn, uint16_t last_bn, uint8_t num_bns);

bdmf_error_t drv_sbpm_init(void);

#ifdef USE_BDMF_SHELL
int drv_sbpm_cli_sanity_check(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
void drv_sbpm_cli_init(bdmfmon_handle_t driver_dir);
#endif

#endif
