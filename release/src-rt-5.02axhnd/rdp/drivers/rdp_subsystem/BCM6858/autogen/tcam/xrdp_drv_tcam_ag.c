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
#include "xrdp_drv_tcam_ag.h"

bdmf_error_t ag_drv_tcam_counters_get(uint32_t *srch_256, uint32_t *hit_256, uint32_t *srch_512, uint32_t *hit_512)
{
    uint32_t reg_counters_tcam_tcam_counters_srch_256;
    uint32_t reg_counters_tcam_tcam_counters_hit_256;
    uint32_t reg_counters_tcam_tcam_counters_srch_512;
    uint32_t reg_counters_tcam_tcam_counters_hit_512;

#ifdef VALIDATE_PARMS
    if(!srch_256 || !hit_256 || !srch_512 || !hit_512)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, TCAM, COUNTERS_TCAM_TCAM_COUNTERS_SRCH_256, reg_counters_tcam_tcam_counters_srch_256);
    RU_REG_READ(0, TCAM, COUNTERS_TCAM_TCAM_COUNTERS_HIT_256, reg_counters_tcam_tcam_counters_hit_256);
    RU_REG_READ(0, TCAM, COUNTERS_TCAM_TCAM_COUNTERS_SRCH_512, reg_counters_tcam_tcam_counters_srch_512);
    RU_REG_READ(0, TCAM, COUNTERS_TCAM_TCAM_COUNTERS_HIT_512, reg_counters_tcam_tcam_counters_hit_512);

    *srch_256 = RU_FIELD_GET(0, TCAM, COUNTERS_TCAM_TCAM_COUNTERS_SRCH_256, CNT, reg_counters_tcam_tcam_counters_srch_256);
    *hit_256 = RU_FIELD_GET(0, TCAM, COUNTERS_TCAM_TCAM_COUNTERS_HIT_256, CNT, reg_counters_tcam_tcam_counters_hit_256);
    *srch_512 = RU_FIELD_GET(0, TCAM, COUNTERS_TCAM_TCAM_COUNTERS_SRCH_512, CNT, reg_counters_tcam_tcam_counters_srch_512);
    *hit_512 = RU_FIELD_GET(0, TCAM, COUNTERS_TCAM_TCAM_COUNTERS_HIT_512, CNT, reg_counters_tcam_tcam_counters_hit_512);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_context_set(uint16_t ctx_idx, uint32_t data)
{
    uint32_t reg_context_ram_tcam_tcam_context_ram_context=0;

#ifdef VALIDATE_PARMS
    if((ctx_idx >= 2048))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_context_ram_tcam_tcam_context_ram_context = RU_FIELD_SET(0, TCAM, CONTEXT_RAM_TCAM_TCAM_CONTEXT_RAM_CONTEXT, DATA, reg_context_ram_tcam_tcam_context_ram_context, data);

    RU_REG_RAM_WRITE(0, ctx_idx, TCAM, CONTEXT_RAM_TCAM_TCAM_CONTEXT_RAM_CONTEXT, reg_context_ram_tcam_tcam_context_ram_context);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_context_get(uint16_t ctx_idx, uint32_t *data)
{
    uint32_t reg_context_ram_tcam_tcam_context_ram_context;

#ifdef VALIDATE_PARMS
    if(!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((ctx_idx >= 2048))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, ctx_idx, TCAM, CONTEXT_RAM_TCAM_TCAM_CONTEXT_RAM_CONTEXT, reg_context_ram_tcam_tcam_context_ram_context);

    *data = RU_FIELD_GET(0, TCAM, CONTEXT_RAM_TCAM_TCAM_CONTEXT_RAM_CONTEXT, DATA, reg_context_ram_tcam_tcam_context_ram_context);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_bank_enable_set(uint8_t value)
{
    uint32_t reg_cfg_tcam_tcam_cfg_bank_en=0;

#ifdef VALIDATE_PARMS
#endif

    reg_cfg_tcam_tcam_cfg_bank_en = RU_FIELD_SET(0, TCAM, CFG_TCAM_TCAM_CFG_BANK_EN, VALUE, reg_cfg_tcam_tcam_cfg_bank_en, value);

    RU_REG_WRITE(0, TCAM, CFG_TCAM_TCAM_CFG_BANK_EN, reg_cfg_tcam_tcam_cfg_bank_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_bank_enable_get(uint8_t *value)
{
    uint32_t reg_cfg_tcam_tcam_cfg_bank_en;

#ifdef VALIDATE_PARMS
    if(!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, TCAM, CFG_TCAM_TCAM_CFG_BANK_EN, reg_cfg_tcam_tcam_cfg_bank_en);

    *value = RU_FIELD_GET(0, TCAM, CFG_TCAM_TCAM_CFG_BANK_EN, VALUE, reg_cfg_tcam_tcam_cfg_bank_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_global_mask_set(uint8_t word_idx, uint32_t value)
{
    uint32_t reg_cfg_tcam_tcam_cfg_global_mask=0;

#ifdef VALIDATE_PARMS
    if((word_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cfg_tcam_tcam_cfg_global_mask = RU_FIELD_SET(0, TCAM, CFG_TCAM_TCAM_CFG_GLOBAL_MASK, VALUE, reg_cfg_tcam_tcam_cfg_global_mask, value);

    RU_REG_RAM_WRITE(0, word_idx, TCAM, CFG_TCAM_TCAM_CFG_GLOBAL_MASK, reg_cfg_tcam_tcam_cfg_global_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_global_mask_get(uint8_t word_idx, uint32_t *value)
{
    uint32_t reg_cfg_tcam_tcam_cfg_global_mask;

#ifdef VALIDATE_PARMS
    if(!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((word_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, word_idx, TCAM, CFG_TCAM_TCAM_CFG_GLOBAL_MASK, reg_cfg_tcam_tcam_cfg_global_mask);

    *value = RU_FIELD_GET(0, TCAM, CFG_TCAM_TCAM_CFG_GLOBAL_MASK, VALUE, reg_cfg_tcam_tcam_cfg_global_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_op_set(uint8_t cmd)
{
    uint32_t reg_indirect_tcam_tcam_indirect_op=0;

#ifdef VALIDATE_PARMS
    if((cmd >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_indirect_tcam_tcam_indirect_op = RU_FIELD_SET(0, TCAM, INDIRECT_TCAM_TCAM_INDIRECT_OP, CMD, reg_indirect_tcam_tcam_indirect_op, cmd);

    RU_REG_WRITE(0, TCAM, INDIRECT_TCAM_TCAM_INDIRECT_OP, reg_indirect_tcam_tcam_indirect_op);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_op_get(uint8_t *cmd)
{
    uint32_t reg_indirect_tcam_tcam_indirect_op;

#ifdef VALIDATE_PARMS
    if(!cmd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, TCAM, INDIRECT_TCAM_TCAM_INDIRECT_OP, reg_indirect_tcam_tcam_indirect_op);

    *cmd = RU_FIELD_GET(0, TCAM, INDIRECT_TCAM_TCAM_INDIRECT_OP, CMD, reg_indirect_tcam_tcam_indirect_op);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_op_done_get(bdmf_boolean *done)
{
    uint32_t reg_indirect_tcam_tcam_indirect_op_done;

#ifdef VALIDATE_PARMS
    if(!done)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, TCAM, INDIRECT_TCAM_TCAM_INDIRECT_OP_DONE, reg_indirect_tcam_tcam_indirect_op_done);

    *done = RU_FIELD_GET(0, TCAM, INDIRECT_TCAM_TCAM_INDIRECT_OP_DONE, DONE, reg_indirect_tcam_tcam_indirect_op_done);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_address_set(bdmf_boolean key1_ind, uint16_t entry_addr)
{
    uint32_t reg_indirect_tcam_tcam_indirect_addr=0;

#ifdef VALIDATE_PARMS
    if((key1_ind >= _1BITS_MAX_VAL_) ||
       (entry_addr >= _10BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_indirect_tcam_tcam_indirect_addr = RU_FIELD_SET(0, TCAM, INDIRECT_TCAM_TCAM_INDIRECT_ADDR, KEY1_IND, reg_indirect_tcam_tcam_indirect_addr, key1_ind);
    reg_indirect_tcam_tcam_indirect_addr = RU_FIELD_SET(0, TCAM, INDIRECT_TCAM_TCAM_INDIRECT_ADDR, ENTRY_ADDR, reg_indirect_tcam_tcam_indirect_addr, entry_addr);

    RU_REG_WRITE(0, TCAM, INDIRECT_TCAM_TCAM_INDIRECT_ADDR, reg_indirect_tcam_tcam_indirect_addr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_address_get(bdmf_boolean *key1_ind, uint16_t *entry_addr)
{
    uint32_t reg_indirect_tcam_tcam_indirect_addr;

#ifdef VALIDATE_PARMS
    if(!key1_ind || !entry_addr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, TCAM, INDIRECT_TCAM_TCAM_INDIRECT_ADDR, reg_indirect_tcam_tcam_indirect_addr);

    *key1_ind = RU_FIELD_GET(0, TCAM, INDIRECT_TCAM_TCAM_INDIRECT_ADDR, KEY1_IND, reg_indirect_tcam_tcam_indirect_addr);
    *entry_addr = RU_FIELD_GET(0, TCAM, INDIRECT_TCAM_TCAM_INDIRECT_ADDR, ENTRY_ADDR, reg_indirect_tcam_tcam_indirect_addr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_valid_in_set(bdmf_boolean valid)
{
    uint32_t reg_indirect_tcam_tcam_indirect_vlid_in=0;

#ifdef VALIDATE_PARMS
    if((valid >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_indirect_tcam_tcam_indirect_vlid_in = RU_FIELD_SET(0, TCAM, INDIRECT_TCAM_TCAM_INDIRECT_VLID_IN, VALID, reg_indirect_tcam_tcam_indirect_vlid_in, valid);

    RU_REG_WRITE(0, TCAM, INDIRECT_TCAM_TCAM_INDIRECT_VLID_IN, reg_indirect_tcam_tcam_indirect_vlid_in);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_valid_in_get(bdmf_boolean *valid)
{
    uint32_t reg_indirect_tcam_tcam_indirect_vlid_in;

#ifdef VALIDATE_PARMS
    if(!valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, TCAM, INDIRECT_TCAM_TCAM_INDIRECT_VLID_IN, reg_indirect_tcam_tcam_indirect_vlid_in);

    *valid = RU_FIELD_GET(0, TCAM, INDIRECT_TCAM_TCAM_INDIRECT_VLID_IN, VALID, reg_indirect_tcam_tcam_indirect_vlid_in);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_valid_out_set(bdmf_boolean valid)
{
    uint32_t reg_indirect_tcam_tcam_indirect_vlid_out=0;

#ifdef VALIDATE_PARMS
    if((valid >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_indirect_tcam_tcam_indirect_vlid_out = RU_FIELD_SET(0, TCAM, INDIRECT_TCAM_TCAM_INDIRECT_VLID_OUT, VALID, reg_indirect_tcam_tcam_indirect_vlid_out, valid);

    RU_REG_WRITE(0, TCAM, INDIRECT_TCAM_TCAM_INDIRECT_VLID_OUT, reg_indirect_tcam_tcam_indirect_vlid_out);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_valid_out_get(bdmf_boolean *valid)
{
    uint32_t reg_indirect_tcam_tcam_indirect_vlid_out;

#ifdef VALIDATE_PARMS
    if(!valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, TCAM, INDIRECT_TCAM_TCAM_INDIRECT_VLID_OUT, reg_indirect_tcam_tcam_indirect_vlid_out);

    *valid = RU_FIELD_GET(0, TCAM, INDIRECT_TCAM_TCAM_INDIRECT_VLID_OUT, VALID, reg_indirect_tcam_tcam_indirect_vlid_out);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_result_get(bdmf_boolean *match, uint16_t *index)
{
    uint32_t reg_indirect_tcam_tcam_indirect_rslt;

#ifdef VALIDATE_PARMS
    if(!match || !index)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, TCAM, INDIRECT_TCAM_TCAM_INDIRECT_RSLT, reg_indirect_tcam_tcam_indirect_rslt);

    *match = RU_FIELD_GET(0, TCAM, INDIRECT_TCAM_TCAM_INDIRECT_RSLT, MATCH, reg_indirect_tcam_tcam_indirect_rslt);
    *index = RU_FIELD_GET(0, TCAM, INDIRECT_TCAM_TCAM_INDIRECT_RSLT, INDEX, reg_indirect_tcam_tcam_indirect_rslt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_key_in_set(uint8_t word_idx, uint32_t value)
{
    uint32_t reg_indirect_tcam_tcam_indirect_key_in=0;

#ifdef VALIDATE_PARMS
    if((word_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_indirect_tcam_tcam_indirect_key_in = RU_FIELD_SET(0, TCAM, INDIRECT_TCAM_TCAM_INDIRECT_KEY_IN, VALUE, reg_indirect_tcam_tcam_indirect_key_in, value);

    RU_REG_RAM_WRITE(0, word_idx, TCAM, INDIRECT_TCAM_TCAM_INDIRECT_KEY_IN, reg_indirect_tcam_tcam_indirect_key_in);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_key_in_get(uint8_t word_idx, uint32_t *value)
{
    uint32_t reg_indirect_tcam_tcam_indirect_key_in;

#ifdef VALIDATE_PARMS
    if(!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((word_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, word_idx, TCAM, INDIRECT_TCAM_TCAM_INDIRECT_KEY_IN, reg_indirect_tcam_tcam_indirect_key_in);

    *value = RU_FIELD_GET(0, TCAM, INDIRECT_TCAM_TCAM_INDIRECT_KEY_IN, VALUE, reg_indirect_tcam_tcam_indirect_key_in);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_key_out_get(uint8_t word_idx, uint32_t *value)
{
    uint32_t reg_indirect_tcam_tcam_indirect_key_out;

#ifdef VALIDATE_PARMS
    if(!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((word_idx >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, word_idx, TCAM, INDIRECT_TCAM_TCAM_INDIRECT_KEY_OUT, reg_indirect_tcam_tcam_indirect_key_out);

    *value = RU_FIELD_GET(0, TCAM, INDIRECT_TCAM_TCAM_INDIRECT_KEY_OUT, VALUE, reg_indirect_tcam_tcam_indirect_key_out);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_debug_bus_tcam_debug_bus_select_set(uint8_t select_module)
{
    uint32_t reg_debug_bus_tcam_debug_bus_select=0;

#ifdef VALIDATE_PARMS
    if((select_module >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_debug_bus_tcam_debug_bus_select = RU_FIELD_SET(0, TCAM, DEBUG_BUS_TCAM_DEBUG_BUS_SELECT, SELECT_MODULE, reg_debug_bus_tcam_debug_bus_select, select_module);

    RU_REG_WRITE(0, TCAM, DEBUG_BUS_TCAM_DEBUG_BUS_SELECT, reg_debug_bus_tcam_debug_bus_select);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_debug_bus_tcam_debug_bus_select_get(uint8_t *select_module)
{
    uint32_t reg_debug_bus_tcam_debug_bus_select;

#ifdef VALIDATE_PARMS
    if(!select_module)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, TCAM, DEBUG_BUS_TCAM_DEBUG_BUS_SELECT, reg_debug_bus_tcam_debug_bus_select);

    *select_module = RU_FIELD_GET(0, TCAM, DEBUG_BUS_TCAM_DEBUG_BUS_SELECT, SELECT_MODULE, reg_debug_bus_tcam_debug_bus_select);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
typedef enum
{
    bdmf_address_context_ram_tcam_tcam_context_ram_context,
    bdmf_address_cfg_tcam_tcam_cfg_bank_en,
    bdmf_address_cfg_tcam_tcam_cfg_global_mask,
    bdmf_address_counters_tcam_tcam_counters_srch_256,
    bdmf_address_counters_tcam_tcam_counters_hit_256,
    bdmf_address_counters_tcam_tcam_counters_srch_512,
    bdmf_address_counters_tcam_tcam_counters_hit_512,
    bdmf_address_indirect_tcam_tcam_indirect_op,
    bdmf_address_indirect_tcam_tcam_indirect_op_done,
    bdmf_address_indirect_tcam_tcam_indirect_addr,
    bdmf_address_indirect_tcam_tcam_indirect_vlid_in,
    bdmf_address_indirect_tcam_tcam_indirect_vlid_out,
    bdmf_address_indirect_tcam_tcam_indirect_rslt,
    bdmf_address_indirect_tcam_tcam_indirect_key_in,
    bdmf_address_indirect_tcam_tcam_indirect_key_out,
    bdmf_address_debug_bus_tcam_debug_bus_select,
}
bdmf_address;

static int bcm_tcam_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_tcam_context:
        err = ag_drv_tcam_context_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_tcam_bank_enable:
        err = ag_drv_tcam_bank_enable_set(parm[1].value.unumber);
        break;
    case cli_tcam_global_mask:
        err = ag_drv_tcam_global_mask_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_tcam_op:
        err = ag_drv_tcam_op_set(parm[1].value.unumber);
        break;
    case cli_tcam_address:
        err = ag_drv_tcam_address_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_tcam_valid_in:
        err = ag_drv_tcam_valid_in_set(parm[1].value.unumber);
        break;
    case cli_tcam_valid_out:
        err = ag_drv_tcam_valid_out_set(parm[1].value.unumber);
        break;
    case cli_tcam_key_in:
        err = ag_drv_tcam_key_in_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_tcam_debug_bus_tcam_debug_bus_select:
        err = ag_drv_tcam_debug_bus_tcam_debug_bus_select_set(parm[1].value.unumber);
        break;
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

int bcm_tcam_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_tcam_counters:
    {
        uint32_t srch_256;
        uint32_t hit_256;
        uint32_t srch_512;
        uint32_t hit_512;
        err = ag_drv_tcam_counters_get(&srch_256, &hit_256, &srch_512, &hit_512);
        bdmf_session_print(session, "srch_256 = %u (0x%x)\n", srch_256, srch_256);
        bdmf_session_print(session, "hit_256 = %u (0x%x)\n", hit_256, hit_256);
        bdmf_session_print(session, "srch_512 = %u (0x%x)\n", srch_512, srch_512);
        bdmf_session_print(session, "hit_512 = %u (0x%x)\n", hit_512, hit_512);
        break;
    }
    case cli_tcam_context:
    {
        uint32_t data;
        err = ag_drv_tcam_context_get(parm[1].value.unumber, &data);
        bdmf_session_print(session, "data = %u (0x%x)\n", data, data);
        break;
    }
    case cli_tcam_bank_enable:
    {
        uint8_t value;
        err = ag_drv_tcam_bank_enable_get(&value);
        bdmf_session_print(session, "value = %u (0x%x)\n", value, value);
        break;
    }
    case cli_tcam_global_mask:
    {
        uint32_t value;
        err = ag_drv_tcam_global_mask_get(parm[1].value.unumber, &value);
        bdmf_session_print(session, "value = %u (0x%x)\n", value, value);
        break;
    }
    case cli_tcam_op:
    {
        uint8_t cmd;
        err = ag_drv_tcam_op_get(&cmd);
        bdmf_session_print(session, "cmd = %u (0x%x)\n", cmd, cmd);
        break;
    }
    case cli_tcam_op_done:
    {
        bdmf_boolean done;
        err = ag_drv_tcam_op_done_get(&done);
        bdmf_session_print(session, "done = %u (0x%x)\n", done, done);
        break;
    }
    case cli_tcam_address:
    {
        bdmf_boolean key1_ind;
        uint16_t entry_addr;
        err = ag_drv_tcam_address_get(&key1_ind, &entry_addr);
        bdmf_session_print(session, "key1_ind = %u (0x%x)\n", key1_ind, key1_ind);
        bdmf_session_print(session, "entry_addr = %u (0x%x)\n", entry_addr, entry_addr);
        break;
    }
    case cli_tcam_valid_in:
    {
        bdmf_boolean valid;
        err = ag_drv_tcam_valid_in_get(&valid);
        bdmf_session_print(session, "valid = %u (0x%x)\n", valid, valid);
        break;
    }
    case cli_tcam_valid_out:
    {
        bdmf_boolean valid;
        err = ag_drv_tcam_valid_out_get(&valid);
        bdmf_session_print(session, "valid = %u (0x%x)\n", valid, valid);
        break;
    }
    case cli_tcam_result:
    {
        bdmf_boolean match;
        uint16_t index;
        err = ag_drv_tcam_result_get(&match, &index);
        bdmf_session_print(session, "match = %u (0x%x)\n", match, match);
        bdmf_session_print(session, "index = %u (0x%x)\n", index, index);
        break;
    }
    case cli_tcam_key_in:
    {
        uint32_t value;
        err = ag_drv_tcam_key_in_get(parm[1].value.unumber, &value);
        bdmf_session_print(session, "value = %u (0x%x)\n", value, value);
        break;
    }
    case cli_tcam_key_out:
    {
        uint32_t value;
        err = ag_drv_tcam_key_out_get(parm[1].value.unumber, &value);
        bdmf_session_print(session, "value = %u (0x%x)\n", value, value);
        break;
    }
    case cli_tcam_debug_bus_tcam_debug_bus_select:
    {
        uint8_t select_module;
        err = ag_drv_tcam_debug_bus_tcam_debug_bus_select_get(&select_module);
        bdmf_session_print(session, "select_module = %u (0x%x)\n", select_module, select_module);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_tcam_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        uint32_t srch_256=gtmv(m, 32);
        uint32_t hit_256=gtmv(m, 32);
        uint32_t srch_512=gtmv(m, 32);
        uint32_t hit_512=gtmv(m, 32);
        if(!err) ag_drv_tcam_counters_get( &srch_256, &hit_256, &srch_512, &hit_512);
        if(!err) bdmf_session_print(session, "ag_drv_tcam_counters_get( %u %u %u %u)\n", srch_256, hit_256, srch_512, hit_512);
    }
    {
        uint16_t ctx_idx=gtmv(m, 11);
        uint32_t data=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_tcam_context_set( %u %u)\n", ctx_idx, data);
        if(!err) ag_drv_tcam_context_set(ctx_idx, data);
        if(!err) ag_drv_tcam_context_get( ctx_idx, &data);
        if(!err) bdmf_session_print(session, "ag_drv_tcam_context_get( %u %u)\n", ctx_idx, data);
        if(err || data!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t value=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_tcam_bank_enable_set( %u)\n", value);
        if(!err) ag_drv_tcam_bank_enable_set(value);
        if(!err) ag_drv_tcam_bank_enable_get( &value);
        if(!err) bdmf_session_print(session, "ag_drv_tcam_bank_enable_get( %u)\n", value);
        if(err || value!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t word_idx=gtmv(m, 3);
        uint32_t value=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_tcam_global_mask_set( %u %u)\n", word_idx, value);
        if(!err) ag_drv_tcam_global_mask_set(word_idx, value);
        if(!err) ag_drv_tcam_global_mask_get( word_idx, &value);
        if(!err) bdmf_session_print(session, "ag_drv_tcam_global_mask_get( %u %u)\n", word_idx, value);
        if(err || value!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t cmd=gtmv(m, 4);
        if(!err) bdmf_session_print(session, "ag_drv_tcam_op_set( %u)\n", cmd);
        if(!err) ag_drv_tcam_op_set(cmd);
        if(!err) ag_drv_tcam_op_get( &cmd);
        if(!err) bdmf_session_print(session, "ag_drv_tcam_op_get( %u)\n", cmd);
        if(err || cmd!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean done=gtmv(m, 1);
        if(!err) ag_drv_tcam_op_done_get( &done);
        if(!err) bdmf_session_print(session, "ag_drv_tcam_op_done_get( %u)\n", done);
    }
    {
        bdmf_boolean key1_ind=gtmv(m, 1);
        uint16_t entry_addr=gtmv(m, 10);
        if(!err) bdmf_session_print(session, "ag_drv_tcam_address_set( %u %u)\n", key1_ind, entry_addr);
        if(!err) ag_drv_tcam_address_set(key1_ind, entry_addr);
        if(!err) ag_drv_tcam_address_get( &key1_ind, &entry_addr);
        if(!err) bdmf_session_print(session, "ag_drv_tcam_address_get( %u %u)\n", key1_ind, entry_addr);
        if(err || key1_ind!=gtmv(m, 1) || entry_addr!=gtmv(m, 10))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean valid=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_tcam_valid_in_set( %u)\n", valid);
        if(!err) ag_drv_tcam_valid_in_set(valid);
        if(!err) ag_drv_tcam_valid_in_get( &valid);
        if(!err) bdmf_session_print(session, "ag_drv_tcam_valid_in_get( %u)\n", valid);
        if(err || valid!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean valid=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_tcam_valid_out_set( %u)\n", valid);
        if(!err) ag_drv_tcam_valid_out_set(valid);
        if(!err) ag_drv_tcam_valid_out_get( &valid);
        if(!err) bdmf_session_print(session, "ag_drv_tcam_valid_out_get( %u)\n", valid);
        if(err || valid!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean match=gtmv(m, 1);
        uint16_t index=gtmv(m, 10);
        if(!err) ag_drv_tcam_result_get( &match, &index);
        if(!err) bdmf_session_print(session, "ag_drv_tcam_result_get( %u %u)\n", match, index);
    }
    {
        uint8_t word_idx=gtmv(m, 3);
        uint32_t value=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_tcam_key_in_set( %u %u)\n", word_idx, value);
        if(!err) ag_drv_tcam_key_in_set(word_idx, value);
        if(!err) ag_drv_tcam_key_in_get( word_idx, &value);
        if(!err) bdmf_session_print(session, "ag_drv_tcam_key_in_get( %u %u)\n", word_idx, value);
        if(err || value!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t word_idx=gtmv(m, 3);
        uint32_t value=gtmv(m, 32);
        if(!err) ag_drv_tcam_key_out_get( word_idx, &value);
        if(!err) bdmf_session_print(session, "ag_drv_tcam_key_out_get( %u %u)\n", word_idx, value);
    }
    {
        uint8_t select_module=gtmv(m, 2);
        if(!err) bdmf_session_print(session, "ag_drv_tcam_debug_bus_tcam_debug_bus_select_set( %u)\n", select_module);
        if(!err) ag_drv_tcam_debug_bus_tcam_debug_bus_select_set(select_module);
        if(!err) ag_drv_tcam_debug_bus_tcam_debug_bus_select_get( &select_module);
        if(!err) bdmf_session_print(session, "ag_drv_tcam_debug_bus_tcam_debug_bus_select_get( %u)\n", select_module);
        if(err || select_module!=gtmv(m, 2))
            return err ? err : BDMF_ERR_IO;
    }
    return err;
}

static int bcm_tcam_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_context_ram_tcam_tcam_context_ram_context : reg = &RU_REG(TCAM, CONTEXT_RAM_TCAM_TCAM_CONTEXT_RAM_CONTEXT); blk = &RU_BLK(TCAM); break;
    case bdmf_address_cfg_tcam_tcam_cfg_bank_en : reg = &RU_REG(TCAM, CFG_TCAM_TCAM_CFG_BANK_EN); blk = &RU_BLK(TCAM); break;
    case bdmf_address_cfg_tcam_tcam_cfg_global_mask : reg = &RU_REG(TCAM, CFG_TCAM_TCAM_CFG_GLOBAL_MASK); blk = &RU_BLK(TCAM); break;
    case bdmf_address_counters_tcam_tcam_counters_srch_256 : reg = &RU_REG(TCAM, COUNTERS_TCAM_TCAM_COUNTERS_SRCH_256); blk = &RU_BLK(TCAM); break;
    case bdmf_address_counters_tcam_tcam_counters_hit_256 : reg = &RU_REG(TCAM, COUNTERS_TCAM_TCAM_COUNTERS_HIT_256); blk = &RU_BLK(TCAM); break;
    case bdmf_address_counters_tcam_tcam_counters_srch_512 : reg = &RU_REG(TCAM, COUNTERS_TCAM_TCAM_COUNTERS_SRCH_512); blk = &RU_BLK(TCAM); break;
    case bdmf_address_counters_tcam_tcam_counters_hit_512 : reg = &RU_REG(TCAM, COUNTERS_TCAM_TCAM_COUNTERS_HIT_512); blk = &RU_BLK(TCAM); break;
    case bdmf_address_indirect_tcam_tcam_indirect_op : reg = &RU_REG(TCAM, INDIRECT_TCAM_TCAM_INDIRECT_OP); blk = &RU_BLK(TCAM); break;
    case bdmf_address_indirect_tcam_tcam_indirect_op_done : reg = &RU_REG(TCAM, INDIRECT_TCAM_TCAM_INDIRECT_OP_DONE); blk = &RU_BLK(TCAM); break;
    case bdmf_address_indirect_tcam_tcam_indirect_addr : reg = &RU_REG(TCAM, INDIRECT_TCAM_TCAM_INDIRECT_ADDR); blk = &RU_BLK(TCAM); break;
    case bdmf_address_indirect_tcam_tcam_indirect_vlid_in : reg = &RU_REG(TCAM, INDIRECT_TCAM_TCAM_INDIRECT_VLID_IN); blk = &RU_BLK(TCAM); break;
    case bdmf_address_indirect_tcam_tcam_indirect_vlid_out : reg = &RU_REG(TCAM, INDIRECT_TCAM_TCAM_INDIRECT_VLID_OUT); blk = &RU_BLK(TCAM); break;
    case bdmf_address_indirect_tcam_tcam_indirect_rslt : reg = &RU_REG(TCAM, INDIRECT_TCAM_TCAM_INDIRECT_RSLT); blk = &RU_BLK(TCAM); break;
    case bdmf_address_indirect_tcam_tcam_indirect_key_in : reg = &RU_REG(TCAM, INDIRECT_TCAM_TCAM_INDIRECT_KEY_IN); blk = &RU_BLK(TCAM); break;
    case bdmf_address_indirect_tcam_tcam_indirect_key_out : reg = &RU_REG(TCAM, INDIRECT_TCAM_TCAM_INDIRECT_KEY_OUT); blk = &RU_BLK(TCAM); break;
    case bdmf_address_debug_bus_tcam_debug_bus_select : reg = &RU_REG(TCAM, DEBUG_BUS_TCAM_DEBUG_BUS_SELECT); blk = &RU_BLK(TCAM); break;
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

bdmfmon_handle_t ag_drv_tcam_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "tcam"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "tcam", "tcam", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_context[]={
            BDMFMON_MAKE_PARM("ctx_idx", "ctx_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("data", "data", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_bank_enable[]={
            BDMFMON_MAKE_PARM("value", "value", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_global_mask[]={
            BDMFMON_MAKE_PARM("word_idx", "word_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("value", "value", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_op[]={
            BDMFMON_MAKE_PARM("cmd", "cmd", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_address[]={
            BDMFMON_MAKE_PARM("key1_ind", "key1_ind", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("entry_addr", "entry_addr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_valid_in[]={
            BDMFMON_MAKE_PARM("valid", "valid", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_valid_out[]={
            BDMFMON_MAKE_PARM("valid", "valid", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_key_in[]={
            BDMFMON_MAKE_PARM("word_idx", "word_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("value", "value", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_debug_bus_tcam_debug_bus_select[]={
            BDMFMON_MAKE_PARM("select_module", "select_module", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="context", .val=cli_tcam_context, .parms=set_context },
            { .name="bank_enable", .val=cli_tcam_bank_enable, .parms=set_bank_enable },
            { .name="global_mask", .val=cli_tcam_global_mask, .parms=set_global_mask },
            { .name="op", .val=cli_tcam_op, .parms=set_op },
            { .name="address", .val=cli_tcam_address, .parms=set_address },
            { .name="valid_in", .val=cli_tcam_valid_in, .parms=set_valid_in },
            { .name="valid_out", .val=cli_tcam_valid_out, .parms=set_valid_out },
            { .name="key_in", .val=cli_tcam_key_in, .parms=set_key_in },
            { .name="debug_bus_tcam_debug_bus_select", .val=cli_tcam_debug_bus_tcam_debug_bus_select, .parms=set_debug_bus_tcam_debug_bus_select },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_tcam_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_context[]={
            BDMFMON_MAKE_PARM("ctx_idx", "ctx_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_global_mask[]={
            BDMFMON_MAKE_PARM("word_idx", "word_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_key_in[]={
            BDMFMON_MAKE_PARM("word_idx", "word_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_key_out[]={
            BDMFMON_MAKE_PARM("word_idx", "word_idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="counters", .val=cli_tcam_counters, .parms=set_default },
            { .name="context", .val=cli_tcam_context, .parms=set_context },
            { .name="bank_enable", .val=cli_tcam_bank_enable, .parms=set_default },
            { .name="global_mask", .val=cli_tcam_global_mask, .parms=set_global_mask },
            { .name="op", .val=cli_tcam_op, .parms=set_default },
            { .name="op_done", .val=cli_tcam_op_done, .parms=set_default },
            { .name="address", .val=cli_tcam_address, .parms=set_default },
            { .name="valid_in", .val=cli_tcam_valid_in, .parms=set_default },
            { .name="valid_out", .val=cli_tcam_valid_out, .parms=set_default },
            { .name="result", .val=cli_tcam_result, .parms=set_default },
            { .name="key_in", .val=cli_tcam_key_in, .parms=set_key_in },
            { .name="key_out", .val=cli_tcam_key_out, .parms=set_key_out },
            { .name="debug_bus_tcam_debug_bus_select", .val=cli_tcam_debug_bus_tcam_debug_bus_select, .parms=set_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_tcam_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_tcam_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="CONTEXT_RAM_TCAM_TCAM_CONTEXT_RAM_CONTEXT" , .val=bdmf_address_context_ram_tcam_tcam_context_ram_context },
            { .name="CFG_TCAM_TCAM_CFG_BANK_EN" , .val=bdmf_address_cfg_tcam_tcam_cfg_bank_en },
            { .name="CFG_TCAM_TCAM_CFG_GLOBAL_MASK" , .val=bdmf_address_cfg_tcam_tcam_cfg_global_mask },
            { .name="COUNTERS_TCAM_TCAM_COUNTERS_SRCH_256" , .val=bdmf_address_counters_tcam_tcam_counters_srch_256 },
            { .name="COUNTERS_TCAM_TCAM_COUNTERS_HIT_256" , .val=bdmf_address_counters_tcam_tcam_counters_hit_256 },
            { .name="COUNTERS_TCAM_TCAM_COUNTERS_SRCH_512" , .val=bdmf_address_counters_tcam_tcam_counters_srch_512 },
            { .name="COUNTERS_TCAM_TCAM_COUNTERS_HIT_512" , .val=bdmf_address_counters_tcam_tcam_counters_hit_512 },
            { .name="INDIRECT_TCAM_TCAM_INDIRECT_OP" , .val=bdmf_address_indirect_tcam_tcam_indirect_op },
            { .name="INDIRECT_TCAM_TCAM_INDIRECT_OP_DONE" , .val=bdmf_address_indirect_tcam_tcam_indirect_op_done },
            { .name="INDIRECT_TCAM_TCAM_INDIRECT_ADDR" , .val=bdmf_address_indirect_tcam_tcam_indirect_addr },
            { .name="INDIRECT_TCAM_TCAM_INDIRECT_VLID_IN" , .val=bdmf_address_indirect_tcam_tcam_indirect_vlid_in },
            { .name="INDIRECT_TCAM_TCAM_INDIRECT_VLID_OUT" , .val=bdmf_address_indirect_tcam_tcam_indirect_vlid_out },
            { .name="INDIRECT_TCAM_TCAM_INDIRECT_RSLT" , .val=bdmf_address_indirect_tcam_tcam_indirect_rslt },
            { .name="INDIRECT_TCAM_TCAM_INDIRECT_KEY_IN" , .val=bdmf_address_indirect_tcam_tcam_indirect_key_in },
            { .name="INDIRECT_TCAM_TCAM_INDIRECT_KEY_OUT" , .val=bdmf_address_indirect_tcam_tcam_indirect_key_out },
            { .name="DEBUG_BUS_TCAM_DEBUG_BUS_SELECT" , .val=bdmf_address_debug_bus_tcam_debug_bus_select },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_tcam_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

