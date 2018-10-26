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

#include "rdp_common.h"
#include "xrdp_drv_drivers_common_ag.h"
#include "xrdp_drv_dqm_ag.h"

bdmf_error_t ag_drv_dqm_fpm_addr_set(uint32_t fpmaddress)
{
    uint32_t reg_fpm_addr=0;

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
    if(!fpmaddress)
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
    uint32_t reg_buf_size=0;

#ifdef VALIDATE_PARMS
    if((pool_0_size >= _2BITS_MAX_VAL_))
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
    if(!pool_0_size)
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
    uint32_t reg_buf_base=0;

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
    if(!base)
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
    uint32_t reg_tokens_used=0;

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
    if(!count)
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
    uint32_t reg_num_pushed=0;

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
    if(!count)
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
    uint32_t reg_num_popped=0;

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
    if(!count)
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
    uint32_t reg_diag_sel=0;

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
    if(!sel)
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
    if(!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DQM, DIAG_DATA, reg_diag_data);

    *data = RU_FIELD_GET(0, DQM, DIAG_DATA, DATA, reg_diag_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_irq_tst_set(bdmf_boolean pushfullqtst, bdmf_boolean popemptyqtst)
{
    uint32_t reg_irq_tst=0;

#ifdef VALIDATE_PARMS
    if((pushfullqtst >= _1BITS_MAX_VAL_) ||
       (popemptyqtst >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_irq_tst = RU_FIELD_SET(0, DQM, IRQ_TST, PUSHFULLQTST, reg_irq_tst, pushfullqtst);
    reg_irq_tst = RU_FIELD_SET(0, DQM, IRQ_TST, POPEMPTYQTST, reg_irq_tst, popemptyqtst);

    RU_REG_WRITE(0, DQM, IRQ_TST, reg_irq_tst);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_irq_tst_get(bdmf_boolean *pushfullqtst, bdmf_boolean *popemptyqtst)
{
    uint32_t reg_irq_tst;

#ifdef VALIDATE_PARMS
    if(!pushfullqtst || !popemptyqtst)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, DQM, IRQ_TST, reg_irq_tst);

    *pushfullqtst = RU_FIELD_GET(0, DQM, IRQ_TST, PUSHFULLQTST, reg_irq_tst);
    *popemptyqtst = RU_FIELD_GET(0, DQM, IRQ_TST, POPEMPTYQTST, reg_irq_tst);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_token_fifo_get(uint8_t fifo_idx, uint32_t *token)
{
    uint32_t reg_token_fifo;

#ifdef VALIDATE_PARMS
    if(!token)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((fifo_idx >= 16))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, fifo_idx, DQM, TOKEN_FIFO, reg_token_fifo);

    *token = RU_FIELD_GET(0, DQM, TOKEN_FIFO, TOKEN, reg_token_fifo);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_head_ptr_get(uint16_t queue_idx, uint32_t *q_head_ptr)
{
    uint32_t reg_head_ptr;

#ifdef VALIDATE_PARMS
    if(!q_head_ptr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((queue_idx >= 32))
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
    if(!q_tail_ptr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((queue_idx >= 32))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, queue_idx, DQM, TAIL_PTR, reg_tail_ptr);

    *q_tail_ptr = RU_FIELD_GET(0, DQM, TAIL_PTR, Q_TAIL_PTR, reg_tail_ptr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_dqmol_size_get(uint16_t queue_idx, uint32_t *max_entries, bdmf_boolean *q_disable_offload, uint8_t *q_tkn_size)
{
    uint32_t reg_dqmol_size;

#ifdef VALIDATE_PARMS
    if(!max_entries || !q_disable_offload || !q_tkn_size)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((queue_idx >= 288))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, queue_idx, DQM, DQMOL_SIZE, reg_dqmol_size);

    *max_entries = RU_FIELD_GET(0, DQM, DQMOL_SIZE, MAX_ENTRIES, reg_dqmol_size);
    *q_disable_offload = RU_FIELD_GET(0, DQM, DQMOL_SIZE, Q_DISABLE_OFFLOAD, reg_dqmol_size);
    *q_tkn_size = RU_FIELD_GET(0, DQM, DQMOL_SIZE, Q_TKN_SIZE, reg_dqmol_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_dqmol_cfga_get(uint16_t queue_idx, uint16_t *q_size, uint16_t *q_start_addr)
{
    uint32_t reg_dqmol_cfga;

#ifdef VALIDATE_PARMS
    if(!q_size || !q_start_addr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((queue_idx >= 288))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, queue_idx, DQM, DQMOL_CFGA, reg_dqmol_cfga);

    *q_size = RU_FIELD_GET(0, DQM, DQMOL_CFGA, Q_SIZE, reg_dqmol_cfga);
    *q_start_addr = RU_FIELD_GET(0, DQM, DQMOL_CFGA, Q_START_ADDR, reg_dqmol_cfga);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_dqm_dqmol_cfgb_set(uint16_t queue_idx, bdmf_boolean enable)
{
    uint32_t reg_dqmol_cfgb=0;

#ifdef VALIDATE_PARMS
    if((queue_idx >= 288) ||
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
    if(!enable)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((queue_idx >= 288))
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
    uint32_t reg_dqmol_pushtoken=0;

#ifdef VALIDATE_PARMS
    if((queue_idx >= 288))
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
    if(!token)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((queue_idx >= 288))
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
    uint32_t reg_dqmol_pushtokennext=0;

#ifdef VALIDATE_PARMS
    if((queue_idx >= 288))
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
    if(!token)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((queue_idx >= 288))
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
    uint32_t reg_dqmol_poptoken=0;

#ifdef VALIDATE_PARMS
    if((queue_idx >= 288))
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
    if(!token)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((queue_idx >= 288))
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
    uint32_t reg_dqmol_poptokennext=0;

#ifdef VALIDATE_PARMS
    if((queue_idx >= 288))
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
    if(!token)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((queue_idx >= 288))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, queue_idx, DQM, DQMOL_POPTOKENNEXT, reg_dqmol_poptokennext);

    *token = RU_FIELD_GET(0, DQM, DQMOL_POPTOKENNEXT, TOKEN, reg_dqmol_poptokennext);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
typedef enum
{
    bdmf_address_fpm_addr,
    bdmf_address_buf_size,
    bdmf_address_buf_base,
    bdmf_address_tokens_used,
    bdmf_address_num_pushed,
    bdmf_address_num_popped,
    bdmf_address_diag_sel,
    bdmf_address_diag_data,
    bdmf_address_irq_tst,
    bdmf_address_token_fifo,
    bdmf_address_head_ptr,
    bdmf_address_tail_ptr,
    bdmf_address_dqmol_size,
    bdmf_address_dqmol_cfga,
    bdmf_address_dqmol_cfgb,
    bdmf_address_dqmol_pushtoken,
    bdmf_address_dqmol_pushtokennext,
    bdmf_address_dqmol_poptoken,
    bdmf_address_dqmol_poptokennext,
}
bdmf_address;

static int bcm_dqm_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_dqm_fpm_addr:
        err = ag_drv_dqm_fpm_addr_set(parm[1].value.unumber);
        break;
    case cli_dqm_buf_size:
        err = ag_drv_dqm_buf_size_set(parm[1].value.unumber);
        break;
    case cli_dqm_buf_base:
        err = ag_drv_dqm_buf_base_set(parm[1].value.unumber);
        break;
    case cli_dqm_tokens_used:
        err = ag_drv_dqm_tokens_used_set(parm[1].value.unumber);
        break;
    case cli_dqm_num_pushed:
        err = ag_drv_dqm_num_pushed_set(parm[1].value.unumber);
        break;
    case cli_dqm_num_popped:
        err = ag_drv_dqm_num_popped_set(parm[1].value.unumber);
        break;
    case cli_dqm_diag_sel:
        err = ag_drv_dqm_diag_sel_set(parm[1].value.unumber);
        break;
    case cli_dqm_irq_tst:
        err = ag_drv_dqm_irq_tst_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dqm_dqmol_cfgb:
        err = ag_drv_dqm_dqmol_cfgb_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dqm_dqmol_pushtoken:
        err = ag_drv_dqm_dqmol_pushtoken_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dqm_dqmol_pushtokennext:
        err = ag_drv_dqm_dqmol_pushtokennext_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dqm_dqmol_poptoken:
        err = ag_drv_dqm_dqmol_poptoken_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_dqm_dqmol_poptokennext:
        err = ag_drv_dqm_dqmol_poptokennext_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

int bcm_dqm_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_dqm_fpm_addr:
    {
        uint32_t fpmaddress;
        err = ag_drv_dqm_fpm_addr_get(&fpmaddress);
        bdmf_session_print(session, "fpmaddress = %u (0x%x)\n", fpmaddress, fpmaddress);
        break;
    }
    case cli_dqm_buf_size:
    {
        uint8_t pool_0_size;
        err = ag_drv_dqm_buf_size_get(&pool_0_size);
        bdmf_session_print(session, "pool_0_size = %u (0x%x)\n", pool_0_size, pool_0_size);
        break;
    }
    case cli_dqm_buf_base:
    {
        uint32_t base;
        err = ag_drv_dqm_buf_base_get(&base);
        bdmf_session_print(session, "base = %u (0x%x)\n", base, base);
        break;
    }
    case cli_dqm_tokens_used:
    {
        uint32_t count;
        err = ag_drv_dqm_tokens_used_get(&count);
        bdmf_session_print(session, "count = %u (0x%x)\n", count, count);
        break;
    }
    case cli_dqm_num_pushed:
    {
        uint32_t count;
        err = ag_drv_dqm_num_pushed_get(&count);
        bdmf_session_print(session, "count = %u (0x%x)\n", count, count);
        break;
    }
    case cli_dqm_num_popped:
    {
        uint32_t count;
        err = ag_drv_dqm_num_popped_get(&count);
        bdmf_session_print(session, "count = %u (0x%x)\n", count, count);
        break;
    }
    case cli_dqm_diag_sel:
    {
        uint8_t sel;
        err = ag_drv_dqm_diag_sel_get(&sel);
        bdmf_session_print(session, "sel = %u (0x%x)\n", sel, sel);
        break;
    }
    case cli_dqm_diag_data:
    {
        uint32_t data;
        err = ag_drv_dqm_diag_data_get(&data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    case cli_dqm_irq_tst:
    {
        bdmf_boolean pushfullqtst;
        bdmf_boolean popemptyqtst;
        err = ag_drv_dqm_irq_tst_get(&pushfullqtst, &popemptyqtst);
        bdmf_session_print(session, "pushfullqtst = %u (0x%x)\n", pushfullqtst, pushfullqtst);
        bdmf_session_print(session, "popemptyqtst = %u (0x%x)\n", popemptyqtst, popemptyqtst);
        break;
    }
    case cli_dqm_token_fifo:
    {
        uint32_t token;
        err = ag_drv_dqm_token_fifo_get(parm[1].value.unumber, &token);
        bdmf_session_print(session, "token = %u (0x%x)\n", token, token);
        break;
    }
    case cli_dqm_head_ptr:
    {
        uint32_t q_head_ptr;
        err = ag_drv_dqm_head_ptr_get(parm[1].value.unumber, &q_head_ptr);
        bdmf_session_print(session, "q_head_ptr = %u (0x%x)\n", q_head_ptr, q_head_ptr);
        break;
    }
    case cli_dqm_tail_ptr:
    {
        uint32_t q_tail_ptr;
        err = ag_drv_dqm_tail_ptr_get(parm[1].value.unumber, &q_tail_ptr);
        bdmf_session_print(session, "q_tail_ptr = %u (0x%x)\n", q_tail_ptr, q_tail_ptr);
        break;
    }
    case cli_dqm_dqmol_size:
    {
        uint32_t max_entries;
        bdmf_boolean q_disable_offload;
        uint8_t q_tkn_size;
        err = ag_drv_dqm_dqmol_size_get(parm[1].value.unumber, &max_entries, &q_disable_offload, &q_tkn_size);
        bdmf_session_print(session, "max_entries = %u (0x%x)\n", max_entries, max_entries);
        bdmf_session_print(session, "q_disable_offload = %u (0x%x)\n", q_disable_offload, q_disable_offload);
        bdmf_session_print(session, "q_tkn_size = %u (0x%x)\n", q_tkn_size, q_tkn_size);
        break;
    }
    case cli_dqm_dqmol_cfga:
    {
        uint16_t q_size;
        uint16_t q_start_addr;
        err = ag_drv_dqm_dqmol_cfga_get(parm[1].value.unumber, &q_size, &q_start_addr);
        bdmf_session_print(session, "q_size = %u (0x%x)\n", q_size, q_size);
        bdmf_session_print(session, "q_start_addr = %u (0x%x)\n", q_start_addr, q_start_addr);
        break;
    }
    case cli_dqm_dqmol_cfgb:
    {
        bdmf_boolean enable;
        err = ag_drv_dqm_dqmol_cfgb_get(parm[1].value.unumber, &enable);
        bdmf_session_print(session, "enable = %u (0x%x)\n", enable, enable);
        break;
    }
    case cli_dqm_dqmol_pushtoken:
    {
        uint32_t token;
        err = ag_drv_dqm_dqmol_pushtoken_get(parm[1].value.unumber, &token);
        bdmf_session_print(session, "token = %u (0x%x)\n", token, token);
        break;
    }
    case cli_dqm_dqmol_pushtokennext:
    {
        uint32_t token;
        err = ag_drv_dqm_dqmol_pushtokennext_get(parm[1].value.unumber, &token);
        bdmf_session_print(session, "token = %u (0x%x)\n", token, token);
        break;
    }
    case cli_dqm_dqmol_poptoken:
    {
        uint32_t token;
        err = ag_drv_dqm_dqmol_poptoken_get(parm[1].value.unumber, &token);
        bdmf_session_print(session, "token = %u (0x%x)\n", token, token);
        break;
    }
    case cli_dqm_dqmol_poptokennext:
    {
        uint32_t token;
        err = ag_drv_dqm_dqmol_poptokennext_get(parm[1].value.unumber, &token);
        bdmf_session_print(session, "token = %u (0x%x)\n", token, token);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_dqm_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        uint32_t fpmaddress=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_dqm_fpm_addr_set( %u)\n", fpmaddress);
        if(!err) ag_drv_dqm_fpm_addr_set(fpmaddress);
        if(!err) ag_drv_dqm_fpm_addr_get( &fpmaddress);
        if(!err) bdmf_session_print(session, "ag_drv_dqm_fpm_addr_get( %u)\n", fpmaddress);
        if(err || fpmaddress!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t pool_0_size=gtmv(m, 2);
        if(!err) bdmf_session_print(session, "ag_drv_dqm_buf_size_set( %u)\n", pool_0_size);
        if(!err) ag_drv_dqm_buf_size_set(pool_0_size);
        if(!err) ag_drv_dqm_buf_size_get( &pool_0_size);
        if(!err) bdmf_session_print(session, "ag_drv_dqm_buf_size_get( %u)\n", pool_0_size);
        if(err || pool_0_size!=gtmv(m, 2))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t base=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_dqm_buf_base_set( %u)\n", base);
        if(!err) ag_drv_dqm_buf_base_set(base);
        if(!err) ag_drv_dqm_buf_base_get( &base);
        if(!err) bdmf_session_print(session, "ag_drv_dqm_buf_base_get( %u)\n", base);
        if(err || base!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t count=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_dqm_tokens_used_set( %u)\n", count);
        if(!err) ag_drv_dqm_tokens_used_set(count);
        if(!err) ag_drv_dqm_tokens_used_get( &count);
        if(!err) bdmf_session_print(session, "ag_drv_dqm_tokens_used_get( %u)\n", count);
        if(err || count!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t count=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_dqm_num_pushed_set( %u)\n", count);
        if(!err) ag_drv_dqm_num_pushed_set(count);
        if(!err) ag_drv_dqm_num_pushed_get( &count);
        if(!err) bdmf_session_print(session, "ag_drv_dqm_num_pushed_get( %u)\n", count);
        if(err || count!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t count=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_dqm_num_popped_set( %u)\n", count);
        if(!err) ag_drv_dqm_num_popped_set(count);
        if(!err) ag_drv_dqm_num_popped_get( &count);
        if(!err) bdmf_session_print(session, "ag_drv_dqm_num_popped_get( %u)\n", count);
        if(err || count!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t sel=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_dqm_diag_sel_set( %u)\n", sel);
        if(!err) ag_drv_dqm_diag_sel_set(sel);
        if(!err) ag_drv_dqm_diag_sel_get( &sel);
        if(!err) bdmf_session_print(session, "ag_drv_dqm_diag_sel_get( %u)\n", sel);
        if(err || sel!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t data=gtmv(m, 32);
        if(!err) ag_drv_dqm_diag_data_get( &data);
        if(!err) bdmf_session_print(session, "ag_drv_dqm_diag_data_get( %u)\n", data);
    }
    {
        bdmf_boolean pushfullqtst=gtmv(m, 1);
        bdmf_boolean popemptyqtst=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_dqm_irq_tst_set( %u %u)\n", pushfullqtst, popemptyqtst);
        if(!err) ag_drv_dqm_irq_tst_set(pushfullqtst, popemptyqtst);
        if(!err) ag_drv_dqm_irq_tst_get( &pushfullqtst, &popemptyqtst);
        if(!err) bdmf_session_print(session, "ag_drv_dqm_irq_tst_get( %u %u)\n", pushfullqtst, popemptyqtst);
        if(err || pushfullqtst!=gtmv(m, 1) || popemptyqtst!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t fifo_idx=gtmv(m, 4);
        uint32_t token=gtmv(m, 32);
        if(!err) ag_drv_dqm_token_fifo_get( fifo_idx, &token);
        if(!err) bdmf_session_print(session, "ag_drv_dqm_token_fifo_get( %u %u)\n", fifo_idx, token);
    }
    {
        uint16_t queue_idx=gtmv(m, 5);
        uint32_t q_head_ptr=gtmv(m, 28);
        if(!err) ag_drv_dqm_head_ptr_get( queue_idx, &q_head_ptr);
        if(!err) bdmf_session_print(session, "ag_drv_dqm_head_ptr_get( %u %u)\n", queue_idx, q_head_ptr);
    }
    {
        uint16_t queue_idx=gtmv(m, 5);
        uint32_t q_tail_ptr=gtmv(m, 28);
        if(!err) ag_drv_dqm_tail_ptr_get( queue_idx, &q_tail_ptr);
        if(!err) bdmf_session_print(session, "ag_drv_dqm_tail_ptr_get( %u %u)\n", queue_idx, q_tail_ptr);
    }
    {
        uint16_t queue_idx=gtmv(m, 5);
        uint32_t max_entries=gtmv(m, 19);
        bdmf_boolean q_disable_offload=gtmv(m, 1);
        uint8_t q_tkn_size=gtmv(m, 2);
        if(!err) ag_drv_dqm_dqmol_size_get( queue_idx, &max_entries, &q_disable_offload, &q_tkn_size);
        if(!err) bdmf_session_print(session, "ag_drv_dqm_dqmol_size_get( %u %u %u %u)\n", queue_idx, max_entries, q_disable_offload, q_tkn_size);
    }
    {
        uint16_t queue_idx=gtmv(m, 5);
        uint16_t q_size=gtmv(m, 16);
        uint16_t q_start_addr=gtmv(m, 16);
        if(!err) ag_drv_dqm_dqmol_cfga_get( queue_idx, &q_size, &q_start_addr);
        if(!err) bdmf_session_print(session, "ag_drv_dqm_dqmol_cfga_get( %u %u %u)\n", queue_idx, q_size, q_start_addr);
    }
    {
        uint16_t queue_idx=gtmv(m, 5);
        bdmf_boolean enable=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_dqm_dqmol_cfgb_set( %u %u)\n", queue_idx, enable);
        if(!err) ag_drv_dqm_dqmol_cfgb_set(queue_idx, enable);
        if(!err) ag_drv_dqm_dqmol_cfgb_get( queue_idx, &enable);
        if(!err) bdmf_session_print(session, "ag_drv_dqm_dqmol_cfgb_get( %u %u)\n", queue_idx, enable);
        if(err || enable!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t queue_idx=gtmv(m, 5);
        uint32_t token=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_dqm_dqmol_pushtoken_set( %u %u)\n", queue_idx, token);
        if(!err) ag_drv_dqm_dqmol_pushtoken_set(queue_idx, token);
        if(!err) ag_drv_dqm_dqmol_pushtoken_get( queue_idx, &token);
        if(!err) bdmf_session_print(session, "ag_drv_dqm_dqmol_pushtoken_get( %u %u)\n", queue_idx, token);
        if(err || token!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t queue_idx=gtmv(m, 5);
        uint32_t token=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_dqm_dqmol_pushtokennext_set( %u %u)\n", queue_idx, token);
        if(!err) ag_drv_dqm_dqmol_pushtokennext_set(queue_idx, token);
        if(!err) ag_drv_dqm_dqmol_pushtokennext_get( queue_idx, &token);
        if(!err) bdmf_session_print(session, "ag_drv_dqm_dqmol_pushtokennext_get( %u %u)\n", queue_idx, token);
        if(err || token!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t queue_idx=gtmv(m, 5);
        uint32_t token=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_dqm_dqmol_poptoken_set( %u %u)\n", queue_idx, token);
        if(!err) ag_drv_dqm_dqmol_poptoken_set(queue_idx, token);
        if(!err) ag_drv_dqm_dqmol_poptoken_get( queue_idx, &token);
        if(!err) bdmf_session_print(session, "ag_drv_dqm_dqmol_poptoken_get( %u %u)\n", queue_idx, token);
        if(err || token!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t queue_idx=gtmv(m, 5);
        uint32_t token=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_dqm_dqmol_poptokennext_set( %u %u)\n", queue_idx, token);
        if(!err) ag_drv_dqm_dqmol_poptokennext_set(queue_idx, token);
        if(!err) ag_drv_dqm_dqmol_poptokennext_get( queue_idx, &token);
        if(!err) bdmf_session_print(session, "ag_drv_dqm_dqmol_poptokennext_get( %u %u)\n", queue_idx, token);
        if(err || token!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    return err;
}

static int bcm_dqm_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t i;
    uint32_t j;
    uint32_t index1_start=0;
    uint32_t index1_stop;
    uint32_t index2_start=0;
    uint32_t index2_stop;
    bdmfmon_cmd_parm_t * bdmf_parm;
    const ru_reg_rec * reg;
    const ru_block_rec * blk;
    const char * enum_string = bdmfmon_enum_parm_stringval(session, 0, parm[0].value.unumber);

    if(!enum_string)
        return BDMF_ERR_INTERNAL;

    switch (parm[0].value.unumber)
    {
    case bdmf_address_fpm_addr : reg = &RU_REG(DQM, FPM_ADDR); blk = &RU_BLK(DQM); break;
    case bdmf_address_buf_size : reg = &RU_REG(DQM, BUF_SIZE); blk = &RU_BLK(DQM); break;
    case bdmf_address_buf_base : reg = &RU_REG(DQM, BUF_BASE); blk = &RU_BLK(DQM); break;
    case bdmf_address_tokens_used : reg = &RU_REG(DQM, TOKENS_USED); blk = &RU_BLK(DQM); break;
    case bdmf_address_num_pushed : reg = &RU_REG(DQM, NUM_PUSHED); blk = &RU_BLK(DQM); break;
    case bdmf_address_num_popped : reg = &RU_REG(DQM, NUM_POPPED); blk = &RU_BLK(DQM); break;
    case bdmf_address_diag_sel : reg = &RU_REG(DQM, DIAG_SEL); blk = &RU_BLK(DQM); break;
    case bdmf_address_diag_data : reg = &RU_REG(DQM, DIAG_DATA); blk = &RU_BLK(DQM); break;
    case bdmf_address_irq_tst : reg = &RU_REG(DQM, IRQ_TST); blk = &RU_BLK(DQM); break;
    case bdmf_address_token_fifo : reg = &RU_REG(DQM, TOKEN_FIFO); blk = &RU_BLK(DQM); break;
    case bdmf_address_head_ptr : reg = &RU_REG(DQM, HEAD_PTR); blk = &RU_BLK(DQM); break;
    case bdmf_address_tail_ptr : reg = &RU_REG(DQM, TAIL_PTR); blk = &RU_BLK(DQM); break;
    case bdmf_address_dqmol_size : reg = &RU_REG(DQM, DQMOL_SIZE); blk = &RU_BLK(DQM); break;
    case bdmf_address_dqmol_cfga : reg = &RU_REG(DQM, DQMOL_CFGA); blk = &RU_BLK(DQM); break;
    case bdmf_address_dqmol_cfgb : reg = &RU_REG(DQM, DQMOL_CFGB); blk = &RU_BLK(DQM); break;
    case bdmf_address_dqmol_pushtoken : reg = &RU_REG(DQM, DQMOL_PUSHTOKEN); blk = &RU_BLK(DQM); break;
    case bdmf_address_dqmol_pushtokennext : reg = &RU_REG(DQM, DQMOL_PUSHTOKENNEXT); blk = &RU_BLK(DQM); break;
    case bdmf_address_dqmol_poptoken : reg = &RU_REG(DQM, DQMOL_POPTOKEN); blk = &RU_BLK(DQM); break;
    case bdmf_address_dqmol_poptokennext : reg = &RU_REG(DQM, DQMOL_POPTOKENNEXT); blk = &RU_BLK(DQM); break;
    default :
        return BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    if((bdmf_parm = bdmfmon_find_named_parm(session,"index1")))
    {
        index1_start = bdmf_parm->value.unumber;
        index1_stop = index1_start + 1;
    }
    else
        index1_stop = blk->addr_count;
    if((bdmf_parm = bdmfmon_find_named_parm(session,"index2")))
    {
        index2_start = bdmf_parm->value.unumber;
        index2_stop = index2_start + 1;
    }
    else
        index2_stop = reg->ram_count + 1;
    if(index1_stop > blk->addr_count)
    {
        bdmf_session_print(session, "index1 (%u) is out of range (%u).\n", index1_stop, blk->addr_count);
        return BDMF_ERR_RANGE;
    }
    if(index2_stop > (reg->ram_count + 1))
    {
        bdmf_session_print(session, "index2 (%u) is out of range (%u).\n", index2_stop, reg->ram_count + 1);
        return BDMF_ERR_RANGE;
    }
    if(reg->ram_count)
        for (i = index1_start; i < index1_stop; i++)
        {
            bdmf_session_print(session, "index1 = %u\n", i);
            for (j = index2_start; j < index2_stop; j++)
                bdmf_session_print(session, 	 "(%5u) 0x%lX\n", j, (blk->addr[i] + reg->addr + j));
        }
    else
        for (i = index1_start; i < index1_stop; i++)
            bdmf_session_print(session, "(%3u) 0x%lX\n", i, blk->addr[i]+reg->addr);
    return 0;
}

bdmfmon_handle_t ag_drv_dqm_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "dqm"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "dqm", "dqm", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_fpm_addr[]={
            BDMFMON_MAKE_PARM("fpmaddress", "fpmaddress", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_buf_size[]={
            BDMFMON_MAKE_PARM("pool_0_size", "pool_0_size", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_buf_base[]={
            BDMFMON_MAKE_PARM("base", "base", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tokens_used[]={
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_num_pushed[]={
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_num_popped[]={
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_diag_sel[]={
            BDMFMON_MAKE_PARM("sel", "sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_irq_tst[]={
            BDMFMON_MAKE_PARM("pushfullqtst", "pushfullqtst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("popemptyqtst", "popemptyqtst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dqmol_cfgb[]={
            BDMFMON_MAKE_PARM("queue_idx", "queue_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("enable", "enable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dqmol_pushtoken[]={
            BDMFMON_MAKE_PARM("queue_idx", "queue_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("token", "token", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dqmol_pushtokennext[]={
            BDMFMON_MAKE_PARM("queue_idx", "queue_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("token", "token", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dqmol_poptoken[]={
            BDMFMON_MAKE_PARM("queue_idx", "queue_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("token", "token", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dqmol_poptokennext[]={
            BDMFMON_MAKE_PARM("queue_idx", "queue_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("token", "token", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="fpm_addr", .val=cli_dqm_fpm_addr, .parms=set_fpm_addr },
            { .name="buf_size", .val=cli_dqm_buf_size, .parms=set_buf_size },
            { .name="buf_base", .val=cli_dqm_buf_base, .parms=set_buf_base },
            { .name="tokens_used", .val=cli_dqm_tokens_used, .parms=set_tokens_used },
            { .name="num_pushed", .val=cli_dqm_num_pushed, .parms=set_num_pushed },
            { .name="num_popped", .val=cli_dqm_num_popped, .parms=set_num_popped },
            { .name="diag_sel", .val=cli_dqm_diag_sel, .parms=set_diag_sel },
            { .name="irq_tst", .val=cli_dqm_irq_tst, .parms=set_irq_tst },
            { .name="dqmol_cfgb", .val=cli_dqm_dqmol_cfgb, .parms=set_dqmol_cfgb },
            { .name="dqmol_pushtoken", .val=cli_dqm_dqmol_pushtoken, .parms=set_dqmol_pushtoken },
            { .name="dqmol_pushtokennext", .val=cli_dqm_dqmol_pushtokennext, .parms=set_dqmol_pushtokennext },
            { .name="dqmol_poptoken", .val=cli_dqm_dqmol_poptoken, .parms=set_dqmol_poptoken },
            { .name="dqmol_poptokennext", .val=cli_dqm_dqmol_poptokennext, .parms=set_dqmol_poptokennext },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_dqm_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_token_fifo[]={
            BDMFMON_MAKE_PARM("fifo_idx", "fifo_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_head_ptr[]={
            BDMFMON_MAKE_PARM("queue_idx", "queue_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tail_ptr[]={
            BDMFMON_MAKE_PARM("queue_idx", "queue_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dqmol_size[]={
            BDMFMON_MAKE_PARM("queue_idx", "queue_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dqmol_cfga[]={
            BDMFMON_MAKE_PARM("queue_idx", "queue_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dqmol_cfgb[]={
            BDMFMON_MAKE_PARM("queue_idx", "queue_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dqmol_pushtoken[]={
            BDMFMON_MAKE_PARM("queue_idx", "queue_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dqmol_pushtokennext[]={
            BDMFMON_MAKE_PARM("queue_idx", "queue_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dqmol_poptoken[]={
            BDMFMON_MAKE_PARM("queue_idx", "queue_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_dqmol_poptokennext[]={
            BDMFMON_MAKE_PARM("queue_idx", "queue_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="fpm_addr", .val=cli_dqm_fpm_addr, .parms=set_default },
            { .name="buf_size", .val=cli_dqm_buf_size, .parms=set_default },
            { .name="buf_base", .val=cli_dqm_buf_base, .parms=set_default },
            { .name="tokens_used", .val=cli_dqm_tokens_used, .parms=set_default },
            { .name="num_pushed", .val=cli_dqm_num_pushed, .parms=set_default },
            { .name="num_popped", .val=cli_dqm_num_popped, .parms=set_default },
            { .name="diag_sel", .val=cli_dqm_diag_sel, .parms=set_default },
            { .name="diag_data", .val=cli_dqm_diag_data, .parms=set_default },
            { .name="irq_tst", .val=cli_dqm_irq_tst, .parms=set_default },
            { .name="token_fifo", .val=cli_dqm_token_fifo, .parms=set_token_fifo },
            { .name="head_ptr", .val=cli_dqm_head_ptr, .parms=set_head_ptr },
            { .name="tail_ptr", .val=cli_dqm_tail_ptr, .parms=set_tail_ptr },
            { .name="dqmol_size", .val=cli_dqm_dqmol_size, .parms=set_dqmol_size },
            { .name="dqmol_cfga", .val=cli_dqm_dqmol_cfga, .parms=set_dqmol_cfga },
            { .name="dqmol_cfgb", .val=cli_dqm_dqmol_cfgb, .parms=set_dqmol_cfgb },
            { .name="dqmol_pushtoken", .val=cli_dqm_dqmol_pushtoken, .parms=set_dqmol_pushtoken },
            { .name="dqmol_pushtokennext", .val=cli_dqm_dqmol_pushtokennext, .parms=set_dqmol_pushtokennext },
            { .name="dqmol_poptoken", .val=cli_dqm_dqmol_poptoken, .parms=set_dqmol_poptoken },
            { .name="dqmol_poptokennext", .val=cli_dqm_dqmol_poptokennext, .parms=set_dqmol_poptokennext },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_dqm_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_dqm_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="FPM_ADDR" , .val=bdmf_address_fpm_addr },
            { .name="BUF_SIZE" , .val=bdmf_address_buf_size },
            { .name="BUF_BASE" , .val=bdmf_address_buf_base },
            { .name="TOKENS_USED" , .val=bdmf_address_tokens_used },
            { .name="NUM_PUSHED" , .val=bdmf_address_num_pushed },
            { .name="NUM_POPPED" , .val=bdmf_address_num_popped },
            { .name="DIAG_SEL" , .val=bdmf_address_diag_sel },
            { .name="DIAG_DATA" , .val=bdmf_address_diag_data },
            { .name="IRQ_TST" , .val=bdmf_address_irq_tst },
            { .name="TOKEN_FIFO" , .val=bdmf_address_token_fifo },
            { .name="HEAD_PTR" , .val=bdmf_address_head_ptr },
            { .name="TAIL_PTR" , .val=bdmf_address_tail_ptr },
            { .name="DQMOL_SIZE" , .val=bdmf_address_dqmol_size },
            { .name="DQMOL_CFGA" , .val=bdmf_address_dqmol_cfga },
            { .name="DQMOL_CFGB" , .val=bdmf_address_dqmol_cfgb },
            { .name="DQMOL_PUSHTOKEN" , .val=bdmf_address_dqmol_pushtoken },
            { .name="DQMOL_PUSHTOKENNEXT" , .val=bdmf_address_dqmol_pushtokennext },
            { .name="DQMOL_POPTOKEN" , .val=bdmf_address_dqmol_poptoken },
            { .name="DQMOL_POPTOKENNEXT" , .val=bdmf_address_dqmol_poptokennext },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_dqm_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

