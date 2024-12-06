/*
   Copyright (c) 2015 Broadcom
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


#ifndef _XRDP_DRV_CNPL_AG_H_
#define _XRDP_DRV_CNPL_AG_H_

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif

typedef struct
{
    bdmf_boolean cn_double;
    uint8_t cn0_byts;
    uint16_t ba;
    bdmf_boolean wrap;
    bdmf_boolean clr;
} cnpl_counter_cfg;

typedef struct
{
    uint16_t bk_ba;
    uint16_t pa_ba;
    bdmf_boolean pl_double;
    bdmf_boolean fc;
    uint8_t pl_st;
    uint8_t pl_end;
    uint8_t n;
} cnpl_policer_cfg;

typedef struct
{
    bdmf_boolean cn_rd_st;
    bdmf_boolean pl_plc_st;
    bdmf_boolean pl_rd_st;
} cnpl_sw_stat;


/**********************************************************************************************************************
 * cn_double: 
 *     1:each counter of the group is double sub-cntr (for 4B-double the format is 36b+28b)
 *     0:each counter of the group is single
 * cn0_byts: 
 *     number of bytes that will hold each sub-counter.
 *     0: 1B
 *     1: 2B
 *     2: 4B
 *     3: 8B (only single, no double supported)
 * ba: 
 *     counters group base address (in 8B resolution: 0 is 0x0, 1 is 0x8, ...)
 * wrap: 
 *     1:wrap at max value
 *     0:freeze at max value
 * clr: 
 *     1:read clear when reading
 *     0:read no-clear when reading
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_counter_cfg_set(uint32_t cnt_loc_profile, const cnpl_counter_cfg *counter_cfg);
bdmf_error_t ag_drv_cnpl_counter_cfg_get(uint32_t cnt_loc_profile, cnpl_counter_cfg *counter_cfg);

/**********************************************************************************************************************
 * bk_ba: 
 *     buckets base address(in 8B resolution: 0 is 0x0, 1 is 0x8, ...)
 * pa_ba: 
 *     params base address(in 8B resolution: 0 is 0x0, 1 is 0x8, ...)
 * pl_double: 
 *     1:each policer is dual bucket
 *     0:each policer is single bucket
 * fc: 
 *     1:the policers are flow control type
 *     0:the policers are normal
 *     
 *     When it is fc, the double field has no effect.
 * pl_st: 
 *     Index of 1st policer of the group.
 *     In default: pol_start>pol_end, so the group is not valid
 * pl_end: 
 *     Index of last policer of the group.
 *     In default: pol_start>pol_end, so the group is not valid
 * n: 
 *     shift left of 8k cycles quanta. 4b lsb only are valid.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_policer_cfg_set(uint32_t conf_idx, const cnpl_policer_cfg *policer_cfg);
bdmf_error_t ag_drv_cnpl_policer_cfg_get(uint32_t conf_idx, cnpl_policer_cfg *policer_cfg);

/**********************************************************************************************************************
 * cn_rd_st: 
 *     0: DONE (ready)
 *     1: PROC(not ready)
 * pl_plc_st: 
 *     0: DONE (ready)
 *     1: PROC(not ready)
 * pl_rd_st: 
 *     0: DONE (ready)
 *     1: PROC(not ready)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_sw_stat_get(cnpl_sw_stat *sw_stat);

/**********************************************************************************************************************
 * data: 
 *     data
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_memory_data_set(uint32_t mem_idx, uint32_t data);
bdmf_error_t ag_drv_cnpl_memory_data_get(uint32_t mem_idx, uint32_t *data);

/**********************************************************************************************************************
 * vec: 
 *     32b vector for 32 policers
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_policers_configurations_pl_calc_type_set(uint32_t conf_idx, uint32_t vec);
bdmf_error_t ag_drv_cnpl_policers_configurations_pl_calc_type_get(uint32_t conf_idx, uint32_t *vec);

/**********************************************************************************************************************
 * prf0: 
 *     16b profile
 * prf1: 
 *     16b profile
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_policers_configurations_pl_size_prof_set(uint32_t conf_idx, uint16_t prf0, uint16_t prf1);
bdmf_error_t ag_drv_cnpl_policers_configurations_pl_size_prof_get(uint32_t conf_idx, uint16_t *prf0, uint16_t *prf1);

/**********************************************************************************************************************
 * en: 
 *     1:enable periodic update
 *     0:disable periodic update
 * mtu: 
 *     MTU for calculation of bucket size
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_policers_configurations_per_up_set(bdmf_boolean en, uint16_t mtu);
bdmf_error_t ag_drv_cnpl_policers_configurations_per_up_get(bdmf_boolean *en, uint16_t *mtu);

/**********************************************************************************************************************
 * val: 
 *     value of register
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_sw_if_sw_cmd_set(uint32_t val);
bdmf_error_t ag_drv_cnpl_sw_if_sw_cmd_get(uint32_t *val);

/**********************************************************************************************************************
 * col: 
 *     red, yellow, green, non-active
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_sw_if_sw_pl_rslt_get(uint8_t *col);

/**********************************************************************************************************************
 * rd: 
 *     value of read data
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_sw_if_sw_pl_rd_get(uint32_t bucket, uint32_t *rd);

/**********************************************************************************************************************
 * rd: 
 *     value of read data
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_sw_if_sw_cnt_rd_get(uint32_t rd_idx, uint32_t *rd);

/**********************************************************************************************************************
 * sw_prio: 
 *     0: fixed lower
 *     1: rr with fw (default)
 *     2: fixed higher
 * mem_addr_bit_sel: 
 *     15 possible options to select which address bit decodes to which cnpl memory the transaction goes.
 *     Currently only 9 bits are needed (for 20K memory).
 *     Value of 0xF is kept to disable this feature (legacy mode), and all accesses will be directed to memory0.
 *     Bits [0..8] are valid values for 20K memory.
 *     Note: if mem_addr_bit_sel (in CNPL_BLOCK.CNPL_Misc.ARB_PRM) > 8, only half the address space is accessible
 *     (10MB out of 20MB).
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_misc_arb_prm_set(uint8_t sw_prio, uint8_t mem_addr_bit_sel);
bdmf_error_t ag_drv_cnpl_misc_arb_prm_get(uint8_t *sw_prio, uint8_t *mem_addr_bit_sel);

/**********************************************************************************************************************
 * en: 
 *     0: dis
 *     1: en
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_misc_col_awr_en_set(bdmf_boolean en);
bdmf_error_t ag_drv_cnpl_misc_col_awr_en_get(bdmf_boolean *en);

/**********************************************************************************************************************
 * init0: 
 *     init for mem 0
 * init1: 
 *     init for mem 1
 **********************************************************************************************************************/
bdmf_error_t ag_drv_cnpl_misc_mem_init_set(bdmf_boolean init0, bdmf_boolean init1);
bdmf_error_t ag_drv_cnpl_misc_mem_init_get(bdmf_boolean *init0, bdmf_boolean *init1);

#ifdef USE_BDMF_SHELL
enum
{
    cli_cnpl_counter_cfg,
    cli_cnpl_policer_cfg,
    cli_cnpl_sw_stat,
    cli_cnpl_memory_data,
    cli_cnpl_policers_configurations_pl_calc_type,
    cli_cnpl_policers_configurations_pl_size_prof,
    cli_cnpl_policers_configurations_per_up,
    cli_cnpl_sw_if_sw_cmd,
    cli_cnpl_sw_if_sw_pl_rslt,
    cli_cnpl_sw_if_sw_pl_rd,
    cli_cnpl_sw_if_sw_cnt_rd,
    cli_cnpl_misc_arb_prm,
    cli_cnpl_misc_col_awr_en,
    cli_cnpl_misc_mem_init,
};

int bcm_cnpl_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_cnpl_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
