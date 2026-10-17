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
#ifndef _RDP_DRV_NATC_H_
#define _RDP_DRV_NATC_H_

#include "rdp_common.h"
#include "rdp_drv_natc_defs.h"
#include "xrdp_autogens.h"
#include "data_path_init.h"


#ifdef __cplusplus
extern "C"
{
#endif

#define NATC_DEFAULT_BIN_SIZE   (ddr_8_bins_per_bucket)
#if (CHIP_VER >= RDP_GEN_100)
typedef struct
{
    uint32_t data[8];
} natc_key_mask;

bdmf_error_t drv_natc_key_mask_set(uint8_t tbl_idx, uint8_t zero, const natc_key_mask *mask);
bdmf_error_t drv_natc_key_mask_get(uint8_t tbl_idx, uint8_t zero, natc_key_mask *mask);
bdmf_error_t drv_natc_tbl_key_addr_set(uint8_t tbl_idx, uint32_t key_lo, uint8_t key_hi);
bdmf_error_t drv_natc_tbl_res_addr_set(uint8_t tbl_idx, uint32_t res_lo, uint8_t res_hi);
#else
#define drv_natc_key_mask_set ag_drv_natc_key_mask_set
#define drv_natc_key_mask_get ag_drv_natc_key_mask_get
#define drv_natc_tbl_key_addr_set ag_drv_natc_tbl_key_addr_set
#define drv_natc_tbl_res_addr_set ag_drv_natc_tbl_res_addr_set
#endif

typedef struct natc_tbl_virt_addr
{
    void *key;
    void *res;
} natc_tbl_virt_addr_t;

typedef struct natc_tbl_phys_addr
{
    bdmf_phys_addr_t key;
    bdmf_phys_addr_t res;
} natc_tbl_phys_addr_t;

typedef struct natc_tbl_config
{
    uint8_t         tbl_idx;
#if (CHIP_VER < RDP_GEN_60)
    uint32_t        mask;
#else
    natc_key_mask   mask;
#endif
    uint32_t        key_len;
    uint32_t        res_len;
    uint32_t        key_tbl_size;
    uint32_t        res_tbl_size;
    uint32_t        tbl_entry_num;
    natc_tbl_virt_addr_t vir_addr;
    natc_tbl_phys_addr_t phy_addr;
    uint32_t        count;
    rdpa_natc_table_name_t table_name;
    uint32_t        memory_shared_mask;
} natc_tbl_config_t;

typedef struct natc_config
{
    uint32_t tbl_en_mask;
    natc_ctrl_status   ctrl_status;
    tbl_control_t      tbl_cntrl[NATC_MAX_TABLES_NUM];
    natc_tbl_config_t  tbl_cfg[NATC_MAX_TABLES_NUM];
} natc_config_t;

int drv_natc_entry_counters_get(uint8_t tbl_idx, uint32_t entry_index, uint64_t *hit_count,
    uint64_t *byte_count);
int drv_natc_key_entry_add(uint8_t tbl_idx, uint32_t entry_idx, uint8_t *key);
int drv_natc_key_entry_get(uint8_t tbl_idx, uint32_t entry_index, bdmf_boolean *valid, uint8_t *key);
int drv_natc_result_entry_add(uint8_t tbl_idx, uint32_t entry_idx, uint8_t *result);
int drv_natc_result_entry_modify(uint8_t tbl_idx, uint32_t entry_idx, uint8_t *result);
int drv_natc_result_entry_get(uint8_t tbl_idx, uint32_t entry_index, uint8_t *res);
int drv_natc_key_result_entry_var_size_ctx_add(uint8_t tbl_idx, uint8_t *hash_key, uint8_t *key, uint8_t *result, uint32_t *hash_idx);
int drv_natc_key_result_entry_add(uint8_t tbl_idx, uint8_t *key, uint8_t *result, uint32_t *hash_idx);
int drv_natc_key_idx_get(uint8_t tbl_idx, uint8_t *keyword, uint32_t *hash_idx, uint32_t *entry_idx);
int drv_natc_entry_delete(uint8_t tbl_idx, uint32_t entry_index, uint8_t count_delete);
int drv_natc_tbl_ctrl_set(uint8_t tbl_idx, tbl_control_t *tbl_ctrl);
int drv_natc_tbl_ctrl_get(uint8_t tbl_idx, tbl_control_t *tbl_ctrl);
int drv_natc_init(natc_config_t *cfg);
int drv_natc_cfg_get(natc_config_t *cfg);
void drv_natc_cfg_build_tables(natc_config_t *cfg, natc_ddr_cfg_natc_ddr_size* ddr_size, natc_ddr_cfg_total_len* total_len, int table_base_size,
    void* table_base_addr_virt, bdmf_phys_addr_t table_base_addr_phys);
ddr_nat_table_size_t _natc_tbl_ddr_size_enum(uint8_t tbl_idx, natc_ddr_cfg_natc_ddr_size *ddr_size);
int drv_natc_tbl_database_get_tbl_size(natc_ddr_cfg_natc_ddr_size *ddr_size, natc_ddr_cfg_total_len *entry_len_array,
    uint32_t tbl_en_mask, uint32_t mem_avail);
bdmf_boolean drv_natc_tos_mflows_get(void);

#if CHIP_VER >= RDP_GEN_60 || defined(BCM63158)
typedef struct {
    uint8_t var_cntx;
    uint8_t non_cacheable;
    key_len_t key_len;
    ddr_nat_table_size_t tbl_size;
    ddr_bins_per_bucket_t bins;
    total_length_t total_entry_len;
    void *tbl_base_addr;
#if defined(BCM63158)  
    uint32_t mask;
#else
    natc_key_mask mask;
#endif
} natc_table_cfg_t;
void drv_natc_init_build_table_cfg(natc_config_t *cfg, rdd_natc_tbl_idx tbl_idx, natc_table_cfg_t *tbl_cfg, dpi_params_t *p_dpi_cfg);
#endif

void drv_natc_basic_cfg_init(natc_config_t *cfg, uint32_t tbl_en_mask);

#if CHIP_VER < RDP_GEN_60
int drv_natc_set_key_mask(uint16_t tbl_idx, uint16_t mask);
#if !defined(BCM63158)
#define NATC_16BYTE_KEY_MASK                                   0x8000
#define NATC_16BYTE_KEY_UCAST_3_TUPLE_MASK                     0xef00

#if defined(CONFIG_RNR_UNKNOWN_UCAST_FLOODING)
#define NATC_16BYTE_KEY_UCAST_MASK                             0x0
#else
#define NATC_16BYTE_KEY_UCAST_MASK                             NATC_16BYTE_KEY_MASK
#endif

#endif

#define NATC_16BYTE_KEY_TOS_MASK        0x0200

#else /* CHIP_VER < RDP_GEN_60 */
int drv_natc_set_key_mask(uint16_t tbl_idx, natc_key_mask *mask);
int drv_natc_flow_cntr_cfg(void);

#define NATC_16BYTE_KEY_TOS_WORD_INDEX  3
#define NATC_16BYTE_KEY_TOS_WORD_MASK   0xFF000000

#endif

total_length_t _natc_tbl_entry_size_enum(uint8_t tbl_idx, natc_ddr_cfg_total_len *total_len);

#ifdef USE_BDMF_SHELL
void drv_natc_cli_init(bdmfmon_handle_t driver_dir);
void drv_natc_cli_exit(bdmfmon_handle_t driver_dir);
#endif


#ifdef __cplusplus
}
#endif


#endif
