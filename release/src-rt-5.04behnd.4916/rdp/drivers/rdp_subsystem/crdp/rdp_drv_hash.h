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
#ifndef _RDP_DRV_HASH_H_
#define _RDP_DRV_HASH_H_

#include "xrdp_autogens.h"
#include "rdp_platform.h"
#include "rdd_data_structures_auto.h"


typedef struct hash_result
{
        uint32_t                        match;
        uint16_t                        match_index;
        uint16_t                        context_addr;
        uint16_t                        first_free_idx;
        uint16_t                        duplicate_idx;
} hash_result_t;






bdmf_error_t drv_hash_find(uint8_t table_id, uint8_t *key , hash_result_t *hash_res);
bdmf_error_t drv_hash_key_get(uint8_t table_id, uint16_t idx , uint8_t *key);
bdmf_error_t drv_hash_rule_add(uint32_t table_id, uint8_t *key_p, uint8_t *ext_ctx, uint8_t *int_ctx,
    uint16_t *key_idx);
bdmf_error_t drv_hash_rule_remove_index(uint32_t table_id, uint16_t idx);
bdmf_error_t drv_hash_set_aging(uint32_t table_id, uint16_t idx, bdmf_boolean *prev_age);
bdmf_error_t drv_hash_modify_context(uint8_t table_id, uint16_t idx, uint8_t *ext_ctx, uint8_t *int_ctx);
bdmf_error_t drv_hash_get_context(uint16_t idx, uint8_t table_id, 
    uint8_t *ext_ctx, uint8_t *int_ctx, bdmf_boolean *skp, uint8_t *cfg);


int drv_hash_cam_max_threshold_set(uint32_t table_id, uint32_t max_cam_threshold);

#ifdef USE_BDMF_SHELL
void drv_hash_cli_init(bdmfmon_handle_t driver_dir);
void drv_hash_cli_exit(bdmfmon_handle_t driver_dir);
#endif
#endif
