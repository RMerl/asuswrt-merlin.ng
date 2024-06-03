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


#ifndef _XRDP_DRV_DQM_AG_H_
#define _XRDP_DRV_DQM_AG_H_

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif


/**********************************************************************************************************************
 * max: 
 *     Represents the maximum number of entries the queue can hold (in words). This is a global settings.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dqm_max_entries_words_set(uint32_t max);
bdmf_error_t ag_drv_dqm_max_entries_words_get(uint32_t *max);

/**********************************************************************************************************************
 * fpmaddress: 
 *     This is the FPM address to be used by components in this module. The same address is used to alloc and free a
 *     token in the FPM. 
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dqm_fpm_addr_set(uint32_t fpmaddress);
bdmf_error_t ag_drv_dqm_fpm_addr_get(uint32_t *fpmaddress);

/**********************************************************************************************************************
 * pool_0_size: 
 *     Buffer Size. This is an encoded value. 
 *     0 =>  256 byte buffer, 
 *     1 =>  512 byte buffer, 
 *     2 => 1024 byte buffer, 
 *     3 => 2048 byte buffer.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dqm_buf_size_set(uint8_t pool_0_size);
bdmf_error_t ag_drv_dqm_buf_size_get(uint8_t *pool_0_size);

/**********************************************************************************************************************
 * base: 
 *     Buffer base address for bits[39:8]. Address bits [7:0] is always assumed to be 0. 
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dqm_buf_base_set(uint32_t base);
bdmf_error_t ag_drv_dqm_buf_base_get(uint32_t *base);

/**********************************************************************************************************************
 * count: 
 *     Represents the current number of tokens used by the queue data structure. This count does not include tokens
 *     that are prefetched. 
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dqm_tokens_used_set(uint32_t count);
bdmf_error_t ag_drv_dqm_tokens_used_get(uint32_t *count);

/**********************************************************************************************************************
 * count: 
 *     Represents the current number of pushed transaction across all queues 
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dqm_num_pushed_set(uint32_t count);
bdmf_error_t ag_drv_dqm_num_pushed_get(uint32_t *count);

/**********************************************************************************************************************
 * count: 
 *     Represents the current number of popped transaction across all queues 
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dqm_num_popped_set(uint32_t count);
bdmf_error_t ag_drv_dqm_num_popped_get(uint32_t *count);

/**********************************************************************************************************************
 * sel: 
 *     MUX Select for routing diag data to the Diag Data Register 
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dqm_diag_sel_set(uint8_t sel);
bdmf_error_t ag_drv_dqm_diag_sel_get(uint8_t *sel);

/**********************************************************************************************************************
 * data: 
 *     data presented as diag readback data.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dqm_diag_data_get(uint32_t *data);

/**********************************************************************************************************************
 * popemptyqtst: 
 *     Test the PopEmptyQ irq
 * pushfullqtst: 
 *     Test the PushFullQ irq
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dqm_irq_tst_set(bdmf_boolean popemptyqtst, bdmf_boolean pushfullqtst);
bdmf_error_t ag_drv_dqm_irq_tst_get(bdmf_boolean *popemptyqtst, bdmf_boolean *pushfullqtst);

/**********************************************************************************************************************
 * rd_loc: 
 *     token fifo read pointer 
 * level: 
 *     token fifo depth count 
 * empty: 
 *     token fifo empty 
 * full: 
 *     token fifo full
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dqm_token_fifo_status_get(uint8_t *rd_loc, uint8_t *level, bdmf_boolean *empty, bdmf_boolean *full);

/**********************************************************************************************************************
 * count: 
 *     Represents the current number of popped with no-commit transaction across all queues 
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dqm_num_popped_no_commit_set(uint32_t count);
bdmf_error_t ag_drv_dqm_num_popped_no_commit_get(uint32_t *count);

/**********************************************************************************************************************
 * q_head_ptr: 
 *     Queue Head Pointer (in words). This is a read-only field and will reset to 0 whenever CNTRL_CFGB is programmed 
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dqm_head_ptr_get(uint16_t queue_idx, uint32_t *q_head_ptr);

/**********************************************************************************************************************
 * q_tail_ptr: 
 *     Queue Tail Pointer (in words). This is a read-only field and will reset to 0 whenever CNTRL_CFGB is programmed 
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dqm_tail_ptr_get(uint16_t queue_idx, uint32_t *q_tail_ptr);

/**********************************************************************************************************************
 * q_tkn_size: 
 *     Queue Token Size (in words). This is a base-0 value. A value of 0 means the token is 1 word long. A value of 1
 *     means the token is 2 words long. This maxes out at a value of 3 to mean that a token is 4 words long. 
 * q_disable_offload: 
 *     When set, this puts  the DQM OL queue into legacy DQM mode, there's no offloading of data. All queue data are
 *     stored in the QSM memory.
 * max_entries: 
 *     Maximum number of entries allotted to the queue before it's full
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dqm_dqmol_size_get(uint16_t queue_idx, uint8_t *q_tkn_size, bdmf_boolean *q_disable_offload, uint32_t *max_entries);

/**********************************************************************************************************************
 * q_start_addr: 
 *     Queue Start Address (word addr). The hardware takes this word address and adds the base address of the Queue
 *     Shared Memory (0x4000 byte addr) to form the physical address for the Queue. 
 * q_size: 
 *     Queue Memory Size (in words). It is required that the Queue Memory Size be whole multiple of the
 *     QUEUE_x_CNTRL_SIZE.Q_TKN_SIZE. For example, if Q_TKN_SIZE == 2 (which represents a 3 word token), then the
 *     Queue Memory Size must be 3, 6, 9, 12, etc. 
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dqm_dqmol_cfga_get(uint16_t queue_idx, uint16_t *q_start_addr, uint16_t *q_size);

/**********************************************************************************************************************
 * enable: 
 *     When set, the DQMOL is enabled and ready for use.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dqm_dqmol_cfgb_set(uint16_t queue_idx, bdmf_boolean enable);
bdmf_error_t ag_drv_dqm_dqmol_cfgb_get(uint16_t queue_idx, bdmf_boolean *enable);

/**********************************************************************************************************************
 * token: 
 *     Queue Token. This is the current token the offload hardware is using for this queue. 
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dqm_dqmol_pushtoken_set(uint16_t queue_idx, uint32_t token);
bdmf_error_t ag_drv_dqm_dqmol_pushtoken_get(uint16_t queue_idx, uint32_t *token);

/**********************************************************************************************************************
 * token: 
 *     Queue Token. This is the current token the offload hardware is using for this queue. 
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dqm_dqmol_pushtokennext_set(uint16_t queue_idx, uint32_t token);
bdmf_error_t ag_drv_dqm_dqmol_pushtokennext_get(uint16_t queue_idx, uint32_t *token);

/**********************************************************************************************************************
 * token: 
 *     Queue Token. This is the current token the offload hardware is using for this queue. 
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dqm_dqmol_poptoken_set(uint16_t queue_idx, uint32_t token);
bdmf_error_t ag_drv_dqm_dqmol_poptoken_get(uint16_t queue_idx, uint32_t *token);

/**********************************************************************************************************************
 * token: 
 *     Queue Token. This is the current token the offload hardware is using for this queue. 
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dqm_dqmol_poptokennext_set(uint16_t queue_idx, uint32_t token);
bdmf_error_t ag_drv_dqm_dqmol_poptokennext_get(uint16_t queue_idx, uint32_t *token);

/**********************************************************************************************************************
 * data: 
 *     data 
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dqm_qsmdata_set(uint16_t queue_idx, uint32_t data);
bdmf_error_t ag_drv_dqm_qsmdata_get(uint16_t queue_idx, uint32_t *data);

#ifdef USE_BDMF_SHELL
enum
{
    cli_dqm_max_entries_words,
    cli_dqm_fpm_addr,
    cli_dqm_buf_size,
    cli_dqm_buf_base,
    cli_dqm_tokens_used,
    cli_dqm_num_pushed,
    cli_dqm_num_popped,
    cli_dqm_diag_sel,
    cli_dqm_diag_data,
    cli_dqm_irq_tst,
    cli_dqm_token_fifo_status,
    cli_dqm_num_popped_no_commit,
    cli_dqm_head_ptr,
    cli_dqm_tail_ptr,
    cli_dqm_dqmol_size,
    cli_dqm_dqmol_cfga,
    cli_dqm_dqmol_cfgb,
    cli_dqm_dqmol_pushtoken,
    cli_dqm_dqmol_pushtokennext,
    cli_dqm_dqmol_poptoken,
    cli_dqm_dqmol_poptokennext,
    cli_dqm_qsmdata,
};

int bcm_dqm_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_dqm_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
