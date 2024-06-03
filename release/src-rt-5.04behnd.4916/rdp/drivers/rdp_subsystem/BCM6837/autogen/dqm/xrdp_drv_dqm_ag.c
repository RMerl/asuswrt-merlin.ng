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


#include "xrdp_drv_drivers_common_ag.h"
#include "xrdp_drv_dqm_ag.h"

#define BLOCK_ADDR_COUNT_BITS 0
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_dqm_max_entries_words_set(uint32_t max)
{
    uint32_t reg_max_entries_words = 0;

#ifdef VALIDATE_PARMS
    if ((max >= _19BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_max_entries_words = RU_FIELD_SET(0, DQM, MAX_ENTRIES_WORDS, MAX, reg_max_entries_words, max);

    RU_REG_WRITE(0, DQM, MAX_ENTRIES_WORDS, reg_max_entries_words);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_max_entries_words_get(uint32_t *max)
{
    uint32_t reg_max_entries_words;

#ifdef VALIDATE_PARMS
    if (!max)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DQM, MAX_ENTRIES_WORDS, reg_max_entries_words);

    *max = RU_FIELD_GET(0, DQM, MAX_ENTRIES_WORDS, MAX, reg_max_entries_words);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_fpm_addr_set(uint32_t fpmaddress)
{
    uint32_t reg_fpm_addr = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_fpm_addr = RU_FIELD_SET(0, DQM, FPM_ADDR, FPMADDRESS, reg_fpm_addr, fpmaddress);

    RU_REG_WRITE(0, DQM, FPM_ADDR, reg_fpm_addr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_fpm_addr_get(uint32_t *fpmaddress)
{
    uint32_t reg_fpm_addr;

#ifdef VALIDATE_PARMS
    if (!fpmaddress)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DQM, FPM_ADDR, reg_fpm_addr);

    *fpmaddress = RU_FIELD_GET(0, DQM, FPM_ADDR, FPMADDRESS, reg_fpm_addr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_buf_size_set(uint8_t pool_0_size)
{
    uint32_t reg_buf_size = 0;

#ifdef VALIDATE_PARMS
    if ((pool_0_size >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_buf_size = RU_FIELD_SET(0, DQM, BUF_SIZE, POOL_0_SIZE, reg_buf_size, pool_0_size);

    RU_REG_WRITE(0, DQM, BUF_SIZE, reg_buf_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_buf_size_get(uint8_t *pool_0_size)
{
    uint32_t reg_buf_size;

#ifdef VALIDATE_PARMS
    if (!pool_0_size)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DQM, BUF_SIZE, reg_buf_size);

    *pool_0_size = RU_FIELD_GET(0, DQM, BUF_SIZE, POOL_0_SIZE, reg_buf_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_buf_base_set(uint32_t base)
{
    uint32_t reg_buf_base = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_buf_base = RU_FIELD_SET(0, DQM, BUF_BASE, BASE, reg_buf_base, base);

    RU_REG_WRITE(0, DQM, BUF_BASE, reg_buf_base);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_buf_base_get(uint32_t *base)
{
    uint32_t reg_buf_base;

#ifdef VALIDATE_PARMS
    if (!base)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DQM, BUF_BASE, reg_buf_base);

    *base = RU_FIELD_GET(0, DQM, BUF_BASE, BASE, reg_buf_base);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_tokens_used_set(uint32_t count)
{
    uint32_t reg_tokens_used = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_tokens_used = RU_FIELD_SET(0, DQM, TOKENS_USED, COUNT, reg_tokens_used, count);

    RU_REG_WRITE(0, DQM, TOKENS_USED, reg_tokens_used);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_tokens_used_get(uint32_t *count)
{
    uint32_t reg_tokens_used;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DQM, TOKENS_USED, reg_tokens_used);

    *count = RU_FIELD_GET(0, DQM, TOKENS_USED, COUNT, reg_tokens_used);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_num_pushed_set(uint32_t count)
{
    uint32_t reg_num_pushed = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_num_pushed = RU_FIELD_SET(0, DQM, NUM_PUSHED, COUNT, reg_num_pushed, count);

    RU_REG_WRITE(0, DQM, NUM_PUSHED, reg_num_pushed);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_num_pushed_get(uint32_t *count)
{
    uint32_t reg_num_pushed;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DQM, NUM_PUSHED, reg_num_pushed);

    *count = RU_FIELD_GET(0, DQM, NUM_PUSHED, COUNT, reg_num_pushed);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_num_popped_set(uint32_t count)
{
    uint32_t reg_num_popped = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_num_popped = RU_FIELD_SET(0, DQM, NUM_POPPED, COUNT, reg_num_popped, count);

    RU_REG_WRITE(0, DQM, NUM_POPPED, reg_num_popped);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_num_popped_get(uint32_t *count)
{
    uint32_t reg_num_popped;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DQM, NUM_POPPED, reg_num_popped);

    *count = RU_FIELD_GET(0, DQM, NUM_POPPED, COUNT, reg_num_popped);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_diag_sel_set(uint8_t sel)
{
    uint32_t reg_diag_sel = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_diag_sel = RU_FIELD_SET(0, DQM, DIAG_SEL, SEL, reg_diag_sel, sel);

    RU_REG_WRITE(0, DQM, DIAG_SEL, reg_diag_sel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_diag_sel_get(uint8_t *sel)
{
    uint32_t reg_diag_sel;

#ifdef VALIDATE_PARMS
    if (!sel)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DQM, DIAG_SEL, reg_diag_sel);

    *sel = RU_FIELD_GET(0, DQM, DIAG_SEL, SEL, reg_diag_sel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_diag_data_get(uint32_t *data)
{
    uint32_t reg_diag_data;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DQM, DIAG_DATA, reg_diag_data);

    *data = RU_FIELD_GET(0, DQM, DIAG_DATA, DATA, reg_diag_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_irq_tst_set(bdmf_boolean popemptyqtst, bdmf_boolean pushfullqtst)
{
    uint32_t reg_irq_tst = 0;

#ifdef VALIDATE_PARMS
    if ((popemptyqtst >= _1BITS_MAX_VAL_) ||
       (pushfullqtst >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_irq_tst = RU_FIELD_SET(0, DQM, IRQ_TST, POPEMPTYQTST, reg_irq_tst, popemptyqtst);
    reg_irq_tst = RU_FIELD_SET(0, DQM, IRQ_TST, PUSHFULLQTST, reg_irq_tst, pushfullqtst);

    RU_REG_WRITE(0, DQM, IRQ_TST, reg_irq_tst);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_irq_tst_get(bdmf_boolean *popemptyqtst, bdmf_boolean *pushfullqtst)
{
    uint32_t reg_irq_tst;

#ifdef VALIDATE_PARMS
    if (!popemptyqtst || !pushfullqtst)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DQM, IRQ_TST, reg_irq_tst);

    *popemptyqtst = RU_FIELD_GET(0, DQM, IRQ_TST, POPEMPTYQTST, reg_irq_tst);
    *pushfullqtst = RU_FIELD_GET(0, DQM, IRQ_TST, PUSHFULLQTST, reg_irq_tst);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_token_fifo_status_get(uint8_t *rd_loc, uint8_t *level, bdmf_boolean *empty, bdmf_boolean *full)
{
    uint32_t reg_token_fifo_status;

#ifdef VALIDATE_PARMS
    if (!rd_loc || !level || !empty || !full)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DQM, TOKEN_FIFO_STATUS, reg_token_fifo_status);

    *rd_loc = RU_FIELD_GET(0, DQM, TOKEN_FIFO_STATUS, RD_LOC, reg_token_fifo_status);
    *level = RU_FIELD_GET(0, DQM, TOKEN_FIFO_STATUS, LEVEL, reg_token_fifo_status);
    *empty = RU_FIELD_GET(0, DQM, TOKEN_FIFO_STATUS, EMPTY, reg_token_fifo_status);
    *full = RU_FIELD_GET(0, DQM, TOKEN_FIFO_STATUS, FULL, reg_token_fifo_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_num_popped_no_commit_set(uint32_t count)
{
    uint32_t reg_num_popped_no_commit = 0;

#ifdef VALIDATE_PARMS
#endif

    reg_num_popped_no_commit = RU_FIELD_SET(0, DQM, NUM_POPPED_NO_COMMIT, COUNT, reg_num_popped_no_commit, count);

    RU_REG_WRITE(0, DQM, NUM_POPPED_NO_COMMIT, reg_num_popped_no_commit);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_num_popped_no_commit_get(uint32_t *count)
{
    uint32_t reg_num_popped_no_commit;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DQM, NUM_POPPED_NO_COMMIT, reg_num_popped_no_commit);

    *count = RU_FIELD_GET(0, DQM, NUM_POPPED_NO_COMMIT, COUNT, reg_num_popped_no_commit);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_head_ptr_get(uint16_t queue_idx, uint32_t *q_head_ptr)
{
    uint32_t reg_head_ptr;

#ifdef VALIDATE_PARMS
    if (!q_head_ptr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((queue_idx >= 160))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, queue_idx, DQM, HEAD_PTR, reg_head_ptr);

    *q_head_ptr = RU_FIELD_GET(0, DQM, HEAD_PTR, Q_HEAD_PTR, reg_head_ptr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_tail_ptr_get(uint16_t queue_idx, uint32_t *q_tail_ptr)
{
    uint32_t reg_tail_ptr;

#ifdef VALIDATE_PARMS
    if (!q_tail_ptr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((queue_idx >= 160))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, queue_idx, DQM, TAIL_PTR, reg_tail_ptr);

    *q_tail_ptr = RU_FIELD_GET(0, DQM, TAIL_PTR, Q_TAIL_PTR, reg_tail_ptr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_dqmol_size_get(uint16_t queue_idx, uint8_t *q_tkn_size, bdmf_boolean *q_disable_offload, uint32_t *max_entries)
{
    uint32_t reg_dqmol_size;

#ifdef VALIDATE_PARMS
    if (!q_tkn_size || !q_disable_offload || !max_entries)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((queue_idx >= 160))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, queue_idx, DQM, DQMOL_SIZE, reg_dqmol_size);

    *q_tkn_size = RU_FIELD_GET(0, DQM, DQMOL_SIZE, Q_TKN_SIZE, reg_dqmol_size);
    *q_disable_offload = RU_FIELD_GET(0, DQM, DQMOL_SIZE, Q_DISABLE_OFFLOAD, reg_dqmol_size);
    *max_entries = RU_FIELD_GET(0, DQM, DQMOL_SIZE, MAX_ENTRIES, reg_dqmol_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_dqmol_cfga_get(uint16_t queue_idx, uint16_t *q_start_addr, uint16_t *q_size)
{
    uint32_t reg_dqmol_cfga;

#ifdef VALIDATE_PARMS
    if (!q_start_addr || !q_size)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((queue_idx >= 160))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, queue_idx, DQM, DQMOL_CFGA, reg_dqmol_cfga);

    *q_start_addr = RU_FIELD_GET(0, DQM, DQMOL_CFGA, Q_START_ADDR, reg_dqmol_cfga);
    *q_size = RU_FIELD_GET(0, DQM, DQMOL_CFGA, Q_SIZE, reg_dqmol_cfga);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_dqmol_cfgb_set(uint16_t queue_idx, bdmf_boolean enable)
{
    uint32_t reg_dqmol_cfgb = 0;

#ifdef VALIDATE_PARMS
    if ((queue_idx >= 160) ||
       (enable >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_dqmol_cfgb = RU_FIELD_SET(0, DQM, DQMOL_CFGB, ENABLE, reg_dqmol_cfgb, enable);

    RU_REG_RAM_WRITE(0, queue_idx, DQM, DQMOL_CFGB, reg_dqmol_cfgb);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_dqmol_cfgb_get(uint16_t queue_idx, bdmf_boolean *enable)
{
    uint32_t reg_dqmol_cfgb;

#ifdef VALIDATE_PARMS
    if (!enable)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((queue_idx >= 160))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, queue_idx, DQM, DQMOL_CFGB, reg_dqmol_cfgb);

    *enable = RU_FIELD_GET(0, DQM, DQMOL_CFGB, ENABLE, reg_dqmol_cfgb);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_dqmol_pushtoken_set(uint16_t queue_idx, uint32_t token)
{
    uint32_t reg_dqmol_pushtoken = 0;

#ifdef VALIDATE_PARMS
    if ((queue_idx >= 160))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_dqmol_pushtoken = RU_FIELD_SET(0, DQM, DQMOL_PUSHTOKEN, TOKEN, reg_dqmol_pushtoken, token);

    RU_REG_RAM_WRITE(0, queue_idx, DQM, DQMOL_PUSHTOKEN, reg_dqmol_pushtoken);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_dqmol_pushtoken_get(uint16_t queue_idx, uint32_t *token)
{
    uint32_t reg_dqmol_pushtoken;

#ifdef VALIDATE_PARMS
    if (!token)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((queue_idx >= 160))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, queue_idx, DQM, DQMOL_PUSHTOKEN, reg_dqmol_pushtoken);

    *token = RU_FIELD_GET(0, DQM, DQMOL_PUSHTOKEN, TOKEN, reg_dqmol_pushtoken);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_dqmol_pushtokennext_set(uint16_t queue_idx, uint32_t token)
{
    uint32_t reg_dqmol_pushtokennext = 0;

#ifdef VALIDATE_PARMS
    if ((queue_idx >= 160))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_dqmol_pushtokennext = RU_FIELD_SET(0, DQM, DQMOL_PUSHTOKENNEXT, TOKEN, reg_dqmol_pushtokennext, token);

    RU_REG_RAM_WRITE(0, queue_idx, DQM, DQMOL_PUSHTOKENNEXT, reg_dqmol_pushtokennext);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_dqmol_pushtokennext_get(uint16_t queue_idx, uint32_t *token)
{
    uint32_t reg_dqmol_pushtokennext;

#ifdef VALIDATE_PARMS
    if (!token)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((queue_idx >= 160))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, queue_idx, DQM, DQMOL_PUSHTOKENNEXT, reg_dqmol_pushtokennext);

    *token = RU_FIELD_GET(0, DQM, DQMOL_PUSHTOKENNEXT, TOKEN, reg_dqmol_pushtokennext);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_dqmol_poptoken_set(uint16_t queue_idx, uint32_t token)
{
    uint32_t reg_dqmol_poptoken = 0;

#ifdef VALIDATE_PARMS
    if ((queue_idx >= 160))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_dqmol_poptoken = RU_FIELD_SET(0, DQM, DQMOL_POPTOKEN, TOKEN, reg_dqmol_poptoken, token);

    RU_REG_RAM_WRITE(0, queue_idx, DQM, DQMOL_POPTOKEN, reg_dqmol_poptoken);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_dqmol_poptoken_get(uint16_t queue_idx, uint32_t *token)
{
    uint32_t reg_dqmol_poptoken;

#ifdef VALIDATE_PARMS
    if (!token)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((queue_idx >= 160))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, queue_idx, DQM, DQMOL_POPTOKEN, reg_dqmol_poptoken);

    *token = RU_FIELD_GET(0, DQM, DQMOL_POPTOKEN, TOKEN, reg_dqmol_poptoken);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_dqmol_poptokennext_set(uint16_t queue_idx, uint32_t token)
{
    uint32_t reg_dqmol_poptokennext = 0;

#ifdef VALIDATE_PARMS
    if ((queue_idx >= 160))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_dqmol_poptokennext = RU_FIELD_SET(0, DQM, DQMOL_POPTOKENNEXT, TOKEN, reg_dqmol_poptokennext, token);

    RU_REG_RAM_WRITE(0, queue_idx, DQM, DQMOL_POPTOKENNEXT, reg_dqmol_poptokennext);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_dqmol_poptokennext_get(uint16_t queue_idx, uint32_t *token)
{
    uint32_t reg_dqmol_poptokennext;

#ifdef VALIDATE_PARMS
    if (!token)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((queue_idx >= 160))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, queue_idx, DQM, DQMOL_POPTOKENNEXT, reg_dqmol_poptokennext);

    *token = RU_FIELD_GET(0, DQM, DQMOL_POPTOKENNEXT, TOKEN, reg_dqmol_poptokennext);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_qsmdata_set(uint16_t queue_idx, uint32_t data)
{
    uint32_t reg_qsmdata = 0;

#ifdef VALIDATE_PARMS
    if ((queue_idx >= 15360))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_qsmdata = RU_FIELD_SET(0, DQM, QSMDATA, DATA, reg_qsmdata, data);

    RU_REG_RAM_WRITE(0, queue_idx, DQM, QSMDATA, reg_qsmdata);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_qsmdata_get(uint16_t queue_idx, uint32_t *data)
{
    uint32_t reg_qsmdata;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((queue_idx >= 15360))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, queue_idx, DQM, QSMDATA, reg_qsmdata);

    *data = RU_FIELD_GET(0, DQM, QSMDATA, DATA, reg_qsmdata);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL

typedef enum
{
    bdmf_address_max_entries_words,
    bdmf_address_fpm_addr,
    bdmf_address_irq_sts,
    bdmf_address_irq_msk,
    bdmf_address_buf_size,
    bdmf_address_buf_base,
    bdmf_address_tokens_used,
    bdmf_address_num_pushed,
    bdmf_address_num_popped,
    bdmf_address_diag_sel,
    bdmf_address_diag_data,
    bdmf_address_irq_tst,
    bdmf_address_token_fifo_status,
    bdmf_address_num_popped_no_commit,
    bdmf_address_status,
    bdmf_address_head_ptr,
    bdmf_address_tail_ptr,
    bdmf_address_dqmol_size,
    bdmf_address_dqmol_cfga,
    bdmf_address_dqmol_cfgb,
    bdmf_address_dqmol_pushtoken,
    bdmf_address_dqmol_pushtokennext,
    bdmf_address_dqmol_poptoken,
    bdmf_address_dqmol_poptokennext,
    bdmf_address_word0,
    bdmf_address_word1,
    bdmf_address_word2,
    bdmf_address_qsmdata,
}
bdmf_address;

static int ag_drv_dqm_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_dqm_max_entries_words:
        ag_err = ag_drv_dqm_max_entries_words_set(parm[1].value.unumber);
        break;
    case cli_dqm_fpm_addr:
        ag_err = ag_drv_dqm_fpm_addr_set(parm[1].value.unumber);
        break;
    case cli_dqm_buf_size:
        ag_err = ag_drv_dqm_buf_size_set(parm[1].value.unumber);
        break;
    case cli_dqm_buf_base:
        ag_err = ag_drv_dqm_buf_base_set(parm[1].value.unumber);
        break;
    case cli_dqm_tokens_used:
        ag_err = ag_drv_dqm_tokens_used_set(parm[1].value.unumber);
        break;
    case cli_dqm_num_pushed:
        ag_err = ag_drv_dqm_num_pushed_set(parm[1].value.unumber);
        break;
    case cli_dqm_num_popped:
        ag_err = ag_drv_dqm_num_popped_set(parm[1].value.unumber);
        break;
    case cli_dqm_diag_sel:
        ag_err = ag_drv_dqm_diag_sel_set(parm[1].value.unumber);
        break;
    case cli_dqm_irq_tst:
        ag_err = ag_drv_dqm_irq_tst_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dqm_num_popped_no_commit:
        ag_err = ag_drv_dqm_num_popped_no_commit_set(parm[1].value.unumber);
        break;
    case cli_dqm_dqmol_cfgb:
        ag_err = ag_drv_dqm_dqmol_cfgb_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dqm_dqmol_pushtoken:
        ag_err = ag_drv_dqm_dqmol_pushtoken_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dqm_dqmol_pushtokennext:
        ag_err = ag_drv_dqm_dqmol_pushtokennext_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dqm_dqmol_poptoken:
        ag_err = ag_drv_dqm_dqmol_poptoken_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dqm_dqmol_poptokennext:
        ag_err = ag_drv_dqm_dqmol_poptokennext_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dqm_qsmdata:
        ag_err = ag_drv_dqm_qsmdata_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

int bcm_dqm_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_dqm_max_entries_words:
    {
        uint32_t max;
        ag_err = ag_drv_dqm_max_entries_words_get(&max);
        bdmf_session_print(session, "max = %u = 0x%x\n", max, max);
        break;
    }
    case cli_dqm_fpm_addr:
    {
        uint32_t fpmaddress;
        ag_err = ag_drv_dqm_fpm_addr_get(&fpmaddress);
        bdmf_session_print(session, "fpmaddress = %u = 0x%x\n", fpmaddress, fpmaddress);
        break;
    }
    case cli_dqm_buf_size:
    {
        uint8_t pool_0_size;
        ag_err = ag_drv_dqm_buf_size_get(&pool_0_size);
        bdmf_session_print(session, "pool_0_size = %u = 0x%x\n", pool_0_size, pool_0_size);
        break;
    }
    case cli_dqm_buf_base:
    {
        uint32_t base;
        ag_err = ag_drv_dqm_buf_base_get(&base);
        bdmf_session_print(session, "base = %u = 0x%x\n", base, base);
        break;
    }
    case cli_dqm_tokens_used:
    {
        uint32_t count;
        ag_err = ag_drv_dqm_tokens_used_get(&count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_dqm_num_pushed:
    {
        uint32_t count;
        ag_err = ag_drv_dqm_num_pushed_get(&count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_dqm_num_popped:
    {
        uint32_t count;
        ag_err = ag_drv_dqm_num_popped_get(&count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_dqm_diag_sel:
    {
        uint8_t sel;
        ag_err = ag_drv_dqm_diag_sel_get(&sel);
        bdmf_session_print(session, "sel = %u = 0x%x\n", sel, sel);
        break;
    }
    case cli_dqm_diag_data:
    {
        uint32_t data;
        ag_err = ag_drv_dqm_diag_data_get(&data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    case cli_dqm_irq_tst:
    {
        bdmf_boolean popemptyqtst;
        bdmf_boolean pushfullqtst;
        ag_err = ag_drv_dqm_irq_tst_get(&popemptyqtst, &pushfullqtst);
        bdmf_session_print(session, "popemptyqtst = %u = 0x%x\n", popemptyqtst, popemptyqtst);
        bdmf_session_print(session, "pushfullqtst = %u = 0x%x\n", pushfullqtst, pushfullqtst);
        break;
    }
    case cli_dqm_token_fifo_status:
    {
        uint8_t rd_loc;
        uint8_t level;
        bdmf_boolean empty;
        bdmf_boolean full;
        ag_err = ag_drv_dqm_token_fifo_status_get(&rd_loc, &level, &empty, &full);
        bdmf_session_print(session, "rd_loc = %u = 0x%x\n", rd_loc, rd_loc);
        bdmf_session_print(session, "level = %u = 0x%x\n", level, level);
        bdmf_session_print(session, "empty = %u = 0x%x\n", empty, empty);
        bdmf_session_print(session, "full = %u = 0x%x\n", full, full);
        break;
    }
    case cli_dqm_num_popped_no_commit:
    {
        uint32_t count;
        ag_err = ag_drv_dqm_num_popped_no_commit_get(&count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_dqm_head_ptr:
    {
        uint32_t q_head_ptr;
        ag_err = ag_drv_dqm_head_ptr_get(parm[1].value.unumber, &q_head_ptr);
        bdmf_session_print(session, "q_head_ptr = %u = 0x%x\n", q_head_ptr, q_head_ptr);
        break;
    }
    case cli_dqm_tail_ptr:
    {
        uint32_t q_tail_ptr;
        ag_err = ag_drv_dqm_tail_ptr_get(parm[1].value.unumber, &q_tail_ptr);
        bdmf_session_print(session, "q_tail_ptr = %u = 0x%x\n", q_tail_ptr, q_tail_ptr);
        break;
    }
    case cli_dqm_dqmol_size:
    {
        uint8_t q_tkn_size;
        bdmf_boolean q_disable_offload;
        uint32_t max_entries;
        ag_err = ag_drv_dqm_dqmol_size_get(parm[1].value.unumber, &q_tkn_size, &q_disable_offload, &max_entries);
        bdmf_session_print(session, "q_tkn_size = %u = 0x%x\n", q_tkn_size, q_tkn_size);
        bdmf_session_print(session, "q_disable_offload = %u = 0x%x\n", q_disable_offload, q_disable_offload);
        bdmf_session_print(session, "max_entries = %u = 0x%x\n", max_entries, max_entries);
        break;
    }
    case cli_dqm_dqmol_cfga:
    {
        uint16_t q_start_addr;
        uint16_t q_size;
        ag_err = ag_drv_dqm_dqmol_cfga_get(parm[1].value.unumber, &q_start_addr, &q_size);
        bdmf_session_print(session, "q_start_addr = %u = 0x%x\n", q_start_addr, q_start_addr);
        bdmf_session_print(session, "q_size = %u = 0x%x\n", q_size, q_size);
        break;
    }
    case cli_dqm_dqmol_cfgb:
    {
        bdmf_boolean enable;
        ag_err = ag_drv_dqm_dqmol_cfgb_get(parm[1].value.unumber, &enable);
        bdmf_session_print(session, "enable = %u = 0x%x\n", enable, enable);
        break;
    }
    case cli_dqm_dqmol_pushtoken:
    {
        uint32_t token;
        ag_err = ag_drv_dqm_dqmol_pushtoken_get(parm[1].value.unumber, &token);
        bdmf_session_print(session, "token = %u = 0x%x\n", token, token);
        break;
    }
    case cli_dqm_dqmol_pushtokennext:
    {
        uint32_t token;
        ag_err = ag_drv_dqm_dqmol_pushtokennext_get(parm[1].value.unumber, &token);
        bdmf_session_print(session, "token = %u = 0x%x\n", token, token);
        break;
    }
    case cli_dqm_dqmol_poptoken:
    {
        uint32_t token;
        ag_err = ag_drv_dqm_dqmol_poptoken_get(parm[1].value.unumber, &token);
        bdmf_session_print(session, "token = %u = 0x%x\n", token, token);
        break;
    }
    case cli_dqm_dqmol_poptokennext:
    {
        uint32_t token;
        ag_err = ag_drv_dqm_dqmol_poptokennext_get(parm[1].value.unumber, &token);
        bdmf_session_print(session, "token = %u = 0x%x\n", token, token);
        break;
    }
    case cli_dqm_qsmdata:
    {
        uint32_t data;
        ag_err = ag_drv_dqm_qsmdata_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u = 0x%x\n", data, data);
        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_dqm_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t ag_err = BDMF_ERR_OK;
    uint32_t test_success_cnt = 0;
    uint32_t test_failure_cnt = 0;

    {
        uint32_t max = gtmv(m, 19);
        bdmf_session_print(session, "ag_drv_dqm_max_entries_words_set( %u)\n",
            max);
        ag_err = ag_drv_dqm_max_entries_words_set(max);
        if (!ag_err)
            ag_err = ag_drv_dqm_max_entries_words_get(&max);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dqm_max_entries_words_get( %u)\n",
                max);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (max != gtmv(m, 19))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t fpmaddress = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_dqm_fpm_addr_set( %u)\n",
            fpmaddress);
        ag_err = ag_drv_dqm_fpm_addr_set(fpmaddress);
        if (!ag_err)
            ag_err = ag_drv_dqm_fpm_addr_get(&fpmaddress);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dqm_fpm_addr_get( %u)\n",
                fpmaddress);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (fpmaddress != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t pool_0_size = gtmv(m, 2);
        bdmf_session_print(session, "ag_drv_dqm_buf_size_set( %u)\n",
            pool_0_size);
        ag_err = ag_drv_dqm_buf_size_set(pool_0_size);
        if (!ag_err)
            ag_err = ag_drv_dqm_buf_size_get(&pool_0_size);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dqm_buf_size_get( %u)\n",
                pool_0_size);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pool_0_size != gtmv(m, 2))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t base = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_dqm_buf_base_set( %u)\n",
            base);
        ag_err = ag_drv_dqm_buf_base_set(base);
        if (!ag_err)
            ag_err = ag_drv_dqm_buf_base_get(&base);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dqm_buf_base_get( %u)\n",
                base);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (base != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_dqm_tokens_used_set( %u)\n",
            count);
        ag_err = ag_drv_dqm_tokens_used_set(count);
        if (!ag_err)
            ag_err = ag_drv_dqm_tokens_used_get(&count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dqm_tokens_used_get( %u)\n",
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_dqm_num_pushed_set( %u)\n",
            count);
        ag_err = ag_drv_dqm_num_pushed_set(count);
        if (!ag_err)
            ag_err = ag_drv_dqm_num_pushed_get(&count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dqm_num_pushed_get( %u)\n",
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_dqm_num_popped_set( %u)\n",
            count);
        ag_err = ag_drv_dqm_num_popped_set(count);
        if (!ag_err)
            ag_err = ag_drv_dqm_num_popped_get(&count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dqm_num_popped_get( %u)\n",
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t sel = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_dqm_diag_sel_set( %u)\n",
            sel);
        ag_err = ag_drv_dqm_diag_sel_set(sel);
        if (!ag_err)
            ag_err = ag_drv_dqm_diag_sel_get(&sel);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dqm_diag_sel_get( %u)\n",
                sel);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (sel != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t data = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_dqm_diag_data_get(&data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dqm_diag_data_get( %u)\n",
                data);
        }
    }

    {
        bdmf_boolean popemptyqtst = gtmv(m, 1);
        bdmf_boolean pushfullqtst = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_dqm_irq_tst_set( %u %u)\n",
            popemptyqtst, pushfullqtst);
        ag_err = ag_drv_dqm_irq_tst_set(popemptyqtst, pushfullqtst);
        if (!ag_err)
            ag_err = ag_drv_dqm_irq_tst_get(&popemptyqtst, &pushfullqtst);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dqm_irq_tst_get( %u %u)\n",
                popemptyqtst, pushfullqtst);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (popemptyqtst != gtmv(m, 1) || pushfullqtst != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t rd_loc = gtmv(m, 4);
        uint8_t level = gtmv(m, 5);
        bdmf_boolean empty = gtmv(m, 1);
        bdmf_boolean full = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_dqm_token_fifo_status_get(&rd_loc, &level, &empty, &full);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dqm_token_fifo_status_get( %u %u %u %u)\n",
                rd_loc, level, empty, full);
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_dqm_num_popped_no_commit_set( %u)\n",
            count);
        ag_err = ag_drv_dqm_num_popped_no_commit_set(count);
        if (!ag_err)
            ag_err = ag_drv_dqm_num_popped_no_commit_get(&count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dqm_num_popped_no_commit_get( %u)\n",
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t queue_idx = gtmv(m, 7);
        uint32_t q_head_ptr = gtmv(m, 28);
        if (!ag_err)
            ag_err = ag_drv_dqm_head_ptr_get(queue_idx, &q_head_ptr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dqm_head_ptr_get( %u %u)\n", queue_idx,
                q_head_ptr);
        }
    }

    {
        uint16_t queue_idx = gtmv(m, 7);
        uint32_t q_tail_ptr = gtmv(m, 28);
        if (!ag_err)
            ag_err = ag_drv_dqm_tail_ptr_get(queue_idx, &q_tail_ptr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dqm_tail_ptr_get( %u %u)\n", queue_idx,
                q_tail_ptr);
        }
    }

    {
        uint16_t queue_idx = gtmv(m, 7);
        uint8_t q_tkn_size = gtmv(m, 2);
        bdmf_boolean q_disable_offload = gtmv(m, 1);
        uint32_t max_entries = gtmv(m, 19);
        if (!ag_err)
            ag_err = ag_drv_dqm_dqmol_size_get(queue_idx, &q_tkn_size, &q_disable_offload, &max_entries);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dqm_dqmol_size_get( %u %u %u %u)\n", queue_idx,
                q_tkn_size, q_disable_offload, max_entries);
        }
    }

    {
        uint16_t queue_idx = gtmv(m, 7);
        uint16_t q_start_addr = gtmv(m, 16);
        uint16_t q_size = gtmv(m, 16);
        if (!ag_err)
            ag_err = ag_drv_dqm_dqmol_cfga_get(queue_idx, &q_start_addr, &q_size);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dqm_dqmol_cfga_get( %u %u %u)\n", queue_idx,
                q_start_addr, q_size);
        }
    }

    {
        uint16_t queue_idx = gtmv(m, 7);
        bdmf_boolean enable = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_dqm_dqmol_cfgb_set( %u %u)\n", queue_idx,
            enable);
        ag_err = ag_drv_dqm_dqmol_cfgb_set(queue_idx, enable);
        if (!ag_err)
            ag_err = ag_drv_dqm_dqmol_cfgb_get(queue_idx, &enable);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dqm_dqmol_cfgb_get( %u %u)\n", queue_idx,
                enable);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (enable != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t queue_idx = gtmv(m, 7);
        uint32_t token = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_dqm_dqmol_pushtoken_set( %u %u)\n", queue_idx,
            token);
        ag_err = ag_drv_dqm_dqmol_pushtoken_set(queue_idx, token);
        if (!ag_err)
            ag_err = ag_drv_dqm_dqmol_pushtoken_get(queue_idx, &token);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dqm_dqmol_pushtoken_get( %u %u)\n", queue_idx,
                token);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (token != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t queue_idx = gtmv(m, 7);
        uint32_t token = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_dqm_dqmol_pushtokennext_set( %u %u)\n", queue_idx,
            token);
        ag_err = ag_drv_dqm_dqmol_pushtokennext_set(queue_idx, token);
        if (!ag_err)
            ag_err = ag_drv_dqm_dqmol_pushtokennext_get(queue_idx, &token);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dqm_dqmol_pushtokennext_get( %u %u)\n", queue_idx,
                token);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (token != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t queue_idx = gtmv(m, 7);
        uint32_t token = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_dqm_dqmol_poptoken_set( %u %u)\n", queue_idx,
            token);
        ag_err = ag_drv_dqm_dqmol_poptoken_set(queue_idx, token);
        if (!ag_err)
            ag_err = ag_drv_dqm_dqmol_poptoken_get(queue_idx, &token);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dqm_dqmol_poptoken_get( %u %u)\n", queue_idx,
                token);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (token != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t queue_idx = gtmv(m, 7);
        uint32_t token = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_dqm_dqmol_poptokennext_set( %u %u)\n", queue_idx,
            token);
        ag_err = ag_drv_dqm_dqmol_poptokennext_set(queue_idx, token);
        if (!ag_err)
            ag_err = ag_drv_dqm_dqmol_poptokennext_get(queue_idx, &token);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dqm_dqmol_poptokennext_get( %u %u)\n", queue_idx,
                token);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (token != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t queue_idx = gtmv(m, 13);
        uint32_t data = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_dqm_qsmdata_set( %u %u)\n", queue_idx,
            data);
        ag_err = ag_drv_dqm_qsmdata_set(queue_idx, data);
        if (!ag_err)
            ag_err = ag_drv_dqm_qsmdata_get(queue_idx, &data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_dqm_qsmdata_get( %u %u)\n", queue_idx,
                data);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (data != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    bdmf_session_print(session, "successes=%u failures=%u\n", test_success_cnt, test_failure_cnt);

    return ag_err;
}
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_dqm_cli_ext_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m, input_method = parm[0].value.unumber;
    bdmfmon_cmd_parm_t *p_start, *p_stop;
    bdmf_error_t ag_err = BDMF_ERR_OK;
    uint32_t ext_test_success_cnt = 0;
    uint32_t ext_test_failure_cnt = 0;
    uint32_t start_idx = 0;
    uint32_t stop_idx = 0;

    p_start = bdmfmon_cmd_find(session, "start_idx");
    p_stop = bdmfmon_cmd_find(session, "stop_idx");

    if (p_start)
        start_idx = p_start->value.unumber;
    if (p_stop)
        stop_idx = p_stop->value.unumber;

    if ((start_idx > stop_idx) && (stop_idx != 0))
    {
        bdmf_session_print(session, "ERROR: start_idx must be less than stop_idx\n");
        return BDMF_ERR_PARM;
    }

    m = bdmf_test_method_high; /* "Initialization" method */
    switch (parm[1].value.unumber)
    {
    case cli_dqm_dqmol_cfgb:
    {
        uint16_t max_queue_idx = 160;
        uint16_t queue_idx = gtmv(m, 7);
        bdmf_boolean enable = gtmv(m, 1);

        if ((start_idx >= max_queue_idx) || (stop_idx >= max_queue_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_queue_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (queue_idx = 0; queue_idx < max_queue_idx; queue_idx++)
        {
            bdmf_session_print(session, "ag_drv_dqm_dqmol_cfgb_set( %u %u)\n", queue_idx,
                enable);
            ag_err = ag_drv_dqm_dqmol_cfgb_set(queue_idx, enable);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", queue_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        enable = gtmv(m, 1);

        for (queue_idx = start_idx; queue_idx <= stop_idx; queue_idx++)
        {
            bdmf_session_print(session, "ag_drv_dqm_dqmol_cfgb_set( %u %u)\n", queue_idx,
                enable);
            ag_err = ag_drv_dqm_dqmol_cfgb_set(queue_idx, enable);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", queue_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (queue_idx = 0; queue_idx < max_queue_idx; queue_idx++)
        {
            if (queue_idx < start_idx || queue_idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_dqm_dqmol_cfgb_get(queue_idx, &enable);

            bdmf_session_print(session, "ag_drv_dqm_dqmol_cfgb_get( %u %u)\n", queue_idx,
                enable);

            if (enable != gtmv(m, 1) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", queue_idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of dqmol_cfgb completed. Number of tested entries %u.\n", max_queue_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_dqm_dqmol_pushtoken:
    {
        uint16_t max_queue_idx = 160;
        uint16_t queue_idx = gtmv(m, 7);
        uint32_t token = gtmv(m, 32);

        if ((start_idx >= max_queue_idx) || (stop_idx >= max_queue_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_queue_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (queue_idx = 0; queue_idx < max_queue_idx; queue_idx++)
        {
            bdmf_session_print(session, "ag_drv_dqm_dqmol_pushtoken_set( %u %u)\n", queue_idx,
                token);
            ag_err = ag_drv_dqm_dqmol_pushtoken_set(queue_idx, token);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", queue_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        token = gtmv(m, 32);

        for (queue_idx = start_idx; queue_idx <= stop_idx; queue_idx++)
        {
            bdmf_session_print(session, "ag_drv_dqm_dqmol_pushtoken_set( %u %u)\n", queue_idx,
                token);
            ag_err = ag_drv_dqm_dqmol_pushtoken_set(queue_idx, token);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", queue_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (queue_idx = 0; queue_idx < max_queue_idx; queue_idx++)
        {
            if (queue_idx < start_idx || queue_idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_dqm_dqmol_pushtoken_get(queue_idx, &token);

            bdmf_session_print(session, "ag_drv_dqm_dqmol_pushtoken_get( %u %u)\n", queue_idx,
                token);

            if (token != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", queue_idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of dqmol_pushtoken completed. Number of tested entries %u.\n", max_queue_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_dqm_dqmol_pushtokennext:
    {
        uint16_t max_queue_idx = 160;
        uint16_t queue_idx = gtmv(m, 7);
        uint32_t token = gtmv(m, 32);

        if ((start_idx >= max_queue_idx) || (stop_idx >= max_queue_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_queue_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (queue_idx = 0; queue_idx < max_queue_idx; queue_idx++)
        {
            bdmf_session_print(session, "ag_drv_dqm_dqmol_pushtokennext_set( %u %u)\n", queue_idx,
                token);
            ag_err = ag_drv_dqm_dqmol_pushtokennext_set(queue_idx, token);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", queue_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        token = gtmv(m, 32);

        for (queue_idx = start_idx; queue_idx <= stop_idx; queue_idx++)
        {
            bdmf_session_print(session, "ag_drv_dqm_dqmol_pushtokennext_set( %u %u)\n", queue_idx,
                token);
            ag_err = ag_drv_dqm_dqmol_pushtokennext_set(queue_idx, token);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", queue_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (queue_idx = 0; queue_idx < max_queue_idx; queue_idx++)
        {
            if (queue_idx < start_idx || queue_idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_dqm_dqmol_pushtokennext_get(queue_idx, &token);

            bdmf_session_print(session, "ag_drv_dqm_dqmol_pushtokennext_get( %u %u)\n", queue_idx,
                token);

            if (token != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", queue_idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of dqmol_pushtokennext completed. Number of tested entries %u.\n", max_queue_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_dqm_dqmol_poptoken:
    {
        uint16_t max_queue_idx = 160;
        uint16_t queue_idx = gtmv(m, 7);
        uint32_t token = gtmv(m, 32);

        if ((start_idx >= max_queue_idx) || (stop_idx >= max_queue_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_queue_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (queue_idx = 0; queue_idx < max_queue_idx; queue_idx++)
        {
            bdmf_session_print(session, "ag_drv_dqm_dqmol_poptoken_set( %u %u)\n", queue_idx,
                token);
            ag_err = ag_drv_dqm_dqmol_poptoken_set(queue_idx, token);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", queue_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        token = gtmv(m, 32);

        for (queue_idx = start_idx; queue_idx <= stop_idx; queue_idx++)
        {
            bdmf_session_print(session, "ag_drv_dqm_dqmol_poptoken_set( %u %u)\n", queue_idx,
                token);
            ag_err = ag_drv_dqm_dqmol_poptoken_set(queue_idx, token);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", queue_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (queue_idx = 0; queue_idx < max_queue_idx; queue_idx++)
        {
            if (queue_idx < start_idx || queue_idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_dqm_dqmol_poptoken_get(queue_idx, &token);

            bdmf_session_print(session, "ag_drv_dqm_dqmol_poptoken_get( %u %u)\n", queue_idx,
                token);

            if (token != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", queue_idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of dqmol_poptoken completed. Number of tested entries %u.\n", max_queue_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_dqm_dqmol_poptokennext:
    {
        uint16_t max_queue_idx = 160;
        uint16_t queue_idx = gtmv(m, 7);
        uint32_t token = gtmv(m, 32);

        if ((start_idx >= max_queue_idx) || (stop_idx >= max_queue_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_queue_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (queue_idx = 0; queue_idx < max_queue_idx; queue_idx++)
        {
            bdmf_session_print(session, "ag_drv_dqm_dqmol_poptokennext_set( %u %u)\n", queue_idx,
                token);
            ag_err = ag_drv_dqm_dqmol_poptokennext_set(queue_idx, token);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", queue_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        token = gtmv(m, 32);

        for (queue_idx = start_idx; queue_idx <= stop_idx; queue_idx++)
        {
            bdmf_session_print(session, "ag_drv_dqm_dqmol_poptokennext_set( %u %u)\n", queue_idx,
                token);
            ag_err = ag_drv_dqm_dqmol_poptokennext_set(queue_idx, token);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", queue_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (queue_idx = 0; queue_idx < max_queue_idx; queue_idx++)
        {
            if (queue_idx < start_idx || queue_idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_dqm_dqmol_poptokennext_get(queue_idx, &token);

            bdmf_session_print(session, "ag_drv_dqm_dqmol_poptokennext_get( %u %u)\n", queue_idx,
                token);

            if (token != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", queue_idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of dqmol_poptokennext completed. Number of tested entries %u.\n", max_queue_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_dqm_qsmdata:
    {
        uint16_t max_queue_idx = 15360;
        uint16_t queue_idx = gtmv(m, 13);
        uint32_t data = gtmv(m, 32);

        if ((start_idx >= max_queue_idx) || (stop_idx >= max_queue_idx))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_queue_idx);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (queue_idx = 0; queue_idx < max_queue_idx; queue_idx++)
        {
            bdmf_session_print(session, "ag_drv_dqm_qsmdata_set( %u %u)\n", queue_idx,
                data);
            ag_err = ag_drv_dqm_qsmdata_set(queue_idx, data);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", queue_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        data = gtmv(m, 32);

        for (queue_idx = start_idx; queue_idx <= stop_idx; queue_idx++)
        {
            bdmf_session_print(session, "ag_drv_dqm_qsmdata_set( %u %u)\n", queue_idx,
                data);
            ag_err = ag_drv_dqm_qsmdata_set(queue_idx, data);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", queue_idx);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (queue_idx = 0; queue_idx < max_queue_idx; queue_idx++)
        {
            if (queue_idx < start_idx || queue_idx > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_dqm_qsmdata_get(queue_idx, &data);

            bdmf_session_print(session, "ag_drv_dqm_qsmdata_get( %u %u)\n", queue_idx,
                data);

            if (data != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", queue_idx);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of qsmdata completed. Number of tested entries %u.\n", max_queue_idx);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }

    return ag_err;
}
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_dqm_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    int chip_rev_idx = RU_CHIP_REV_IDX_GET();
    uint32_t i;
    uint32_t j;
    uint32_t index1_start = 0;
    uint32_t index1_stop;
    uint32_t index2_start = 0;
    uint32_t index2_stop;
    bdmfmon_cmd_parm_t *cliparm;
    const ru_reg_rec *reg;
    const ru_block_rec *blk;
    const char *enum_string = bdmfmon_enum_parm_stringval(session, &parm[0]);

    if(!enum_string)
        return BDMF_ERR_INTERNAL;

    switch (parm[0].value.unumber)
    {
    case bdmf_address_max_entries_words: reg = &RU_REG(DQM, MAX_ENTRIES_WORDS); blk = &RU_BLK(DQM); break;
    case bdmf_address_fpm_addr: reg = &RU_REG(DQM, FPM_ADDR); blk = &RU_BLK(DQM); break;
    case bdmf_address_irq_sts: reg = &RU_REG(DQM, IRQ_STS); blk = &RU_BLK(DQM); break;
    case bdmf_address_irq_msk: reg = &RU_REG(DQM, IRQ_MSK); blk = &RU_BLK(DQM); break;
    case bdmf_address_buf_size: reg = &RU_REG(DQM, BUF_SIZE); blk = &RU_BLK(DQM); break;
    case bdmf_address_buf_base: reg = &RU_REG(DQM, BUF_BASE); blk = &RU_BLK(DQM); break;
    case bdmf_address_tokens_used: reg = &RU_REG(DQM, TOKENS_USED); blk = &RU_BLK(DQM); break;
    case bdmf_address_num_pushed: reg = &RU_REG(DQM, NUM_PUSHED); blk = &RU_BLK(DQM); break;
    case bdmf_address_num_popped: reg = &RU_REG(DQM, NUM_POPPED); blk = &RU_BLK(DQM); break;
    case bdmf_address_diag_sel: reg = &RU_REG(DQM, DIAG_SEL); blk = &RU_BLK(DQM); break;
    case bdmf_address_diag_data: reg = &RU_REG(DQM, DIAG_DATA); blk = &RU_BLK(DQM); break;
    case bdmf_address_irq_tst: reg = &RU_REG(DQM, IRQ_TST); blk = &RU_BLK(DQM); break;
    case bdmf_address_token_fifo_status: reg = &RU_REG(DQM, TOKEN_FIFO_STATUS); blk = &RU_BLK(DQM); break;
    case bdmf_address_num_popped_no_commit: reg = &RU_REG(DQM, NUM_POPPED_NO_COMMIT); blk = &RU_BLK(DQM); break;
    case bdmf_address_status: reg = &RU_REG(DQM, STATUS); blk = &RU_BLK(DQM); break;
    case bdmf_address_head_ptr: reg = &RU_REG(DQM, HEAD_PTR); blk = &RU_BLK(DQM); break;
    case bdmf_address_tail_ptr: reg = &RU_REG(DQM, TAIL_PTR); blk = &RU_BLK(DQM); break;
    case bdmf_address_dqmol_size: reg = &RU_REG(DQM, DQMOL_SIZE); blk = &RU_BLK(DQM); break;
    case bdmf_address_dqmol_cfga: reg = &RU_REG(DQM, DQMOL_CFGA); blk = &RU_BLK(DQM); break;
    case bdmf_address_dqmol_cfgb: reg = &RU_REG(DQM, DQMOL_CFGB); blk = &RU_BLK(DQM); break;
    case bdmf_address_dqmol_pushtoken: reg = &RU_REG(DQM, DQMOL_PUSHTOKEN); blk = &RU_BLK(DQM); break;
    case bdmf_address_dqmol_pushtokennext: reg = &RU_REG(DQM, DQMOL_PUSHTOKENNEXT); blk = &RU_BLK(DQM); break;
    case bdmf_address_dqmol_poptoken: reg = &RU_REG(DQM, DQMOL_POPTOKEN); blk = &RU_BLK(DQM); break;
    case bdmf_address_dqmol_poptokennext: reg = &RU_REG(DQM, DQMOL_POPTOKENNEXT); blk = &RU_BLK(DQM); break;
    case bdmf_address_word0: reg = &RU_REG(DQM, WORD0); blk = &RU_BLK(DQM); break;
    case bdmf_address_word1: reg = &RU_REG(DQM, WORD1); blk = &RU_BLK(DQM); break;
    case bdmf_address_word2: reg = &RU_REG(DQM, WORD2); blk = &RU_BLK(DQM); break;
    case bdmf_address_qsmdata: reg = &RU_REG(DQM, QSMDATA); blk = &RU_BLK(DQM); break;
    default :
        return BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    if ((cliparm = bdmfmon_cmd_find(session, "index1")))
    {
        index1_start = cliparm->value.unumber;
        index1_stop = index1_start + 1;
    }
    else
        index1_stop = blk->addr_count;
    if ((cliparm = bdmfmon_cmd_find(session, "index2")))
    {
        index2_start = cliparm->value.unumber;
        index2_stop = index2_start + 1;
    }
    else
        index2_stop = reg->ram_count;
    if (index1_stop > blk->addr_count)
    {
        bdmf_session_print(session, "index1 (%u) is out of range (%u).\n", index1_stop, blk->addr_count);
        return BDMF_ERR_RANGE;
    }
    if (index2_stop > (reg->ram_count))
    {
        bdmf_session_print(session, "index2 (%u) is out of range (%u).\n", index2_stop, reg->ram_count);
        return BDMF_ERR_RANGE;
    }
    if (reg->ram_count)
        for (i = index1_start; i < index1_stop; i++)
        {
            bdmf_session_print(session, "index1 = %u\n", i);
            for (j = index2_start; j < index2_stop; j++)
                bdmf_session_print(session, TAB "(%5u) 0x%08X\n", j, ((blk->addr[i] + reg->addr[chip_rev_idx]) + j));
        }
    else
        for (i = index1_start; i < index1_stop; i++)
            bdmf_session_print(session, "(%3u) 0x%08X\n", i, blk->addr[i]+reg->addr[chip_rev_idx]);
    return 0;
}
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

bdmfmon_handle_t ag_drv_dqm_cli_init(bdmfmon_handle_t root_dir)
{
    bdmfmon_handle_t dir;

    dir = bdmfmon_dir_add(root_dir, "dqm", "dqm", BDMF_ACCESS_ADMIN, NULL);
    BUG_ON(dir == NULL);

    {
        static bdmfmon_cmd_parm_t set_max_entries_words[] = {
            BDMFMON_MAKE_PARM("max", "max", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_fpm_addr[] = {
            BDMFMON_MAKE_PARM("fpmaddress", "fpmaddress", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_buf_size[] = {
            BDMFMON_MAKE_PARM("pool_0_size", "pool_0_size", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_buf_base[] = {
            BDMFMON_MAKE_PARM("base", "base", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tokens_used[] = {
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_num_pushed[] = {
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_num_popped[] = {
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_diag_sel[] = {
            BDMFMON_MAKE_PARM("sel", "sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_irq_tst[] = {
            BDMFMON_MAKE_PARM("popemptyqtst", "popemptyqtst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pushfullqtst", "pushfullqtst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_num_popped_no_commit[] = {
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dqmol_cfgb[] = {
            BDMFMON_MAKE_PARM("queue_idx", "queue_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable", "enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dqmol_pushtoken[] = {
            BDMFMON_MAKE_PARM("queue_idx", "queue_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("token", "token", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dqmol_pushtokennext[] = {
            BDMFMON_MAKE_PARM("queue_idx", "queue_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("token", "token", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dqmol_poptoken[] = {
            BDMFMON_MAKE_PARM("queue_idx", "queue_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("token", "token", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dqmol_poptokennext[] = {
            BDMFMON_MAKE_PARM("queue_idx", "queue_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("token", "token", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_qsmdata[] = {
            BDMFMON_MAKE_PARM("queue_idx", "queue_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("data", "data", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "max_entries_words", .val = cli_dqm_max_entries_words, .parms = set_max_entries_words },
            { .name = "fpm_addr", .val = cli_dqm_fpm_addr, .parms = set_fpm_addr },
            { .name = "buf_size", .val = cli_dqm_buf_size, .parms = set_buf_size },
            { .name = "buf_base", .val = cli_dqm_buf_base, .parms = set_buf_base },
            { .name = "tokens_used", .val = cli_dqm_tokens_used, .parms = set_tokens_used },
            { .name = "num_pushed", .val = cli_dqm_num_pushed, .parms = set_num_pushed },
            { .name = "num_popped", .val = cli_dqm_num_popped, .parms = set_num_popped },
            { .name = "diag_sel", .val = cli_dqm_diag_sel, .parms = set_diag_sel },
            { .name = "irq_tst", .val = cli_dqm_irq_tst, .parms = set_irq_tst },
            { .name = "num_popped_no_commit", .val = cli_dqm_num_popped_no_commit, .parms = set_num_popped_no_commit },
            { .name = "dqmol_cfgb", .val = cli_dqm_dqmol_cfgb, .parms = set_dqmol_cfgb },
            { .name = "dqmol_pushtoken", .val = cli_dqm_dqmol_pushtoken, .parms = set_dqmol_pushtoken },
            { .name = "dqmol_pushtokennext", .val = cli_dqm_dqmol_pushtokennext, .parms = set_dqmol_pushtokennext },
            { .name = "dqmol_poptoken", .val = cli_dqm_dqmol_poptoken, .parms = set_dqmol_poptoken },
            { .name = "dqmol_poptokennext", .val = cli_dqm_dqmol_poptokennext, .parms = set_dqmol_poptokennext },
            { .name = "qsmdata", .val = cli_dqm_qsmdata, .parms = set_qsmdata },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", ag_drv_dqm_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t get_default[] = {
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_head_ptr[] = {
            BDMFMON_MAKE_PARM("queue_idx", "queue_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_tail_ptr[] = {
            BDMFMON_MAKE_PARM("queue_idx", "queue_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_dqmol_size[] = {
            BDMFMON_MAKE_PARM("queue_idx", "queue_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_dqmol_cfga[] = {
            BDMFMON_MAKE_PARM("queue_idx", "queue_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_dqmol_cfgb[] = {
            BDMFMON_MAKE_PARM("queue_idx", "queue_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_dqmol_pushtoken[] = {
            BDMFMON_MAKE_PARM("queue_idx", "queue_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_dqmol_pushtokennext[] = {
            BDMFMON_MAKE_PARM("queue_idx", "queue_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_dqmol_poptoken[] = {
            BDMFMON_MAKE_PARM("queue_idx", "queue_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_dqmol_poptokennext[] = {
            BDMFMON_MAKE_PARM("queue_idx", "queue_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_qsmdata[] = {
            BDMFMON_MAKE_PARM("queue_idx", "queue_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "max_entries_words", .val = cli_dqm_max_entries_words, .parms = get_default },
            { .name = "fpm_addr", .val = cli_dqm_fpm_addr, .parms = get_default },
            { .name = "buf_size", .val = cli_dqm_buf_size, .parms = get_default },
            { .name = "buf_base", .val = cli_dqm_buf_base, .parms = get_default },
            { .name = "tokens_used", .val = cli_dqm_tokens_used, .parms = get_default },
            { .name = "num_pushed", .val = cli_dqm_num_pushed, .parms = get_default },
            { .name = "num_popped", .val = cli_dqm_num_popped, .parms = get_default },
            { .name = "diag_sel", .val = cli_dqm_diag_sel, .parms = get_default },
            { .name = "diag_data", .val = cli_dqm_diag_data, .parms = get_default },
            { .name = "irq_tst", .val = cli_dqm_irq_tst, .parms = get_default },
            { .name = "token_fifo_status", .val = cli_dqm_token_fifo_status, .parms = get_default },
            { .name = "num_popped_no_commit", .val = cli_dqm_num_popped_no_commit, .parms = get_default },
            { .name = "head_ptr", .val = cli_dqm_head_ptr, .parms = get_head_ptr },
            { .name = "tail_ptr", .val = cli_dqm_tail_ptr, .parms = get_tail_ptr },
            { .name = "dqmol_size", .val = cli_dqm_dqmol_size, .parms = get_dqmol_size },
            { .name = "dqmol_cfga", .val = cli_dqm_dqmol_cfga, .parms = get_dqmol_cfga },
            { .name = "dqmol_cfgb", .val = cli_dqm_dqmol_cfgb, .parms = get_dqmol_cfgb },
            { .name = "dqmol_pushtoken", .val = cli_dqm_dqmol_pushtoken, .parms = get_dqmol_pushtoken },
            { .name = "dqmol_pushtokennext", .val = cli_dqm_dqmol_pushtokennext, .parms = get_dqmol_pushtokennext },
            { .name = "dqmol_poptoken", .val = cli_dqm_dqmol_poptoken, .parms = get_dqmol_poptoken },
            { .name = "dqmol_poptokennext", .val = cli_dqm_dqmol_poptokennext, .parms = get_dqmol_poptokennext },
            { .name = "qsmdata", .val = cli_dqm_qsmdata, .parms = get_qsmdata },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_dqm_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name = "low", .val = ag_drv_cli_test_method_low },
            { .name = "mid", .val = ag_drv_cli_test_method_mid },
            { .name = "high", .val = ag_drv_cli_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "test", "test", ag_drv_dqm_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_cmd_parm_t ext_test_default[] = {
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "dqmol_cfgb", .val = cli_dqm_dqmol_cfgb, .parms = ext_test_default},
            { .name = "dqmol_pushtoken", .val = cli_dqm_dqmol_pushtoken, .parms = ext_test_default},
            { .name = "dqmol_pushtokennext", .val = cli_dqm_dqmol_pushtokennext, .parms = ext_test_default},
            { .name = "dqmol_poptoken", .val = cli_dqm_dqmol_poptoken, .parms = ext_test_default},
            { .name = "dqmol_poptokennext", .val = cli_dqm_dqmol_poptokennext, .parms = ext_test_default},
            { .name = "qsmdata", .val = cli_dqm_qsmdata, .parms = ext_test_default},
            BDMFMON_ENUM_LAST
        };
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name = "low", .val = ag_drv_cli_test_method_low },
            { .name = "mid", .val = ag_drv_cli_test_method_mid },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "ext_test", "ext_test", ag_drv_dqm_cli_ext_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0),
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0),
            BDMFMON_MAKE_PARM("start_idx", "array index to start test", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL),
            BDMFMON_MAKE_PARM("stop_idx", "array index to stop test", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name = "MAX_ENTRIES_WORDS", .val = bdmf_address_max_entries_words },
            { .name = "FPM_ADDR", .val = bdmf_address_fpm_addr },
            { .name = "IRQ_STS", .val = bdmf_address_irq_sts },
            { .name = "IRQ_MSK", .val = bdmf_address_irq_msk },
            { .name = "BUF_SIZE", .val = bdmf_address_buf_size },
            { .name = "BUF_BASE", .val = bdmf_address_buf_base },
            { .name = "TOKENS_USED", .val = bdmf_address_tokens_used },
            { .name = "NUM_PUSHED", .val = bdmf_address_num_pushed },
            { .name = "NUM_POPPED", .val = bdmf_address_num_popped },
            { .name = "DIAG_SEL", .val = bdmf_address_diag_sel },
            { .name = "DIAG_DATA", .val = bdmf_address_diag_data },
            { .name = "IRQ_TST", .val = bdmf_address_irq_tst },
            { .name = "TOKEN_FIFO_STATUS", .val = bdmf_address_token_fifo_status },
            { .name = "NUM_POPPED_NO_COMMIT", .val = bdmf_address_num_popped_no_commit },
            { .name = "STATUS", .val = bdmf_address_status },
            { .name = "HEAD_PTR", .val = bdmf_address_head_ptr },
            { .name = "TAIL_PTR", .val = bdmf_address_tail_ptr },
            { .name = "DQMOL_SIZE", .val = bdmf_address_dqmol_size },
            { .name = "DQMOL_CFGA", .val = bdmf_address_dqmol_cfga },
            { .name = "DQMOL_CFGB", .val = bdmf_address_dqmol_cfgb },
            { .name = "DQMOL_PUSHTOKEN", .val = bdmf_address_dqmol_pushtoken },
            { .name = "DQMOL_PUSHTOKENNEXT", .val = bdmf_address_dqmol_pushtokennext },
            { .name = "DQMOL_POPTOKEN", .val = bdmf_address_dqmol_poptoken },
            { .name = "DQMOL_POPTOKENNEXT", .val = bdmf_address_dqmol_poptokennext },
            { .name = "WORD0", .val = bdmf_address_word0 },
            { .name = "WORD1", .val = bdmf_address_word1 },
            { .name = "WORD2", .val = bdmf_address_word2 },
            { .name = "QSMDATA", .val = bdmf_address_qsmdata },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "address", "address", bcm_dqm_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

    return dir;
}
#endif /* #ifdef USE_BDMF_SHELL */
