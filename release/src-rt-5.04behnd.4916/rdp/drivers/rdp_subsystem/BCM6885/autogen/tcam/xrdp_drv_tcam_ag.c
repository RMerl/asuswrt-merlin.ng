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
#include "xrdp_drv_tcam_ag.h"

#define BLOCK_ADDR_COUNT_BITS 0
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_tcam_cfg_dinm_set(uint8_t tcam_id, uint32_t cfg_dinm, uint32_t ram_line)
{
    uint32_t reg_cfg_dinm = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (cfg_dinm >= 64))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cfg_dinm = RU_FIELD_SET(tcam_id, TCAM, CFG_DINM, RAM_LINE, reg_cfg_dinm, ram_line);

    RU_REG_RAM_WRITE(tcam_id, cfg_dinm, TCAM, CFG_DINM, reg_cfg_dinm);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_cfg_dinm_get(uint8_t tcam_id, uint32_t cfg_dinm, uint32_t *ram_line)
{
    uint32_t reg_cfg_dinm;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (cfg_dinm >= 64))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, cfg_dinm, TCAM, CFG_DINM, reg_cfg_dinm);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, CFG_DINM, RAM_LINE, reg_cfg_dinm);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_cfg_blksel_set(uint8_t tcam_id, uint32_t cfg_blksel, uint32_t ram_line)
{
    uint32_t reg_cfg_blksel = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (cfg_blksel >= 2))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cfg_blksel = RU_FIELD_SET(tcam_id, TCAM, CFG_BLKSEL, RAM_LINE, reg_cfg_blksel, ram_line);

    RU_REG_RAM_WRITE(tcam_id, cfg_blksel, TCAM, CFG_BLKSEL, reg_cfg_blksel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_cfg_blksel_get(uint8_t tcam_id, uint32_t cfg_blksel, uint32_t *ram_line)
{
    uint32_t reg_cfg_blksel;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (cfg_blksel >= 2))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, cfg_blksel, TCAM, CFG_BLKSEL, reg_cfg_blksel);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, CFG_BLKSEL, RAM_LINE, reg_cfg_blksel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_cfg_tm_set(uint8_t tcam_id, uint32_t cfg_tm, uint32_t ram_line)
{
    uint32_t reg_cfg_tm = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (cfg_tm >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cfg_tm = RU_FIELD_SET(tcam_id, TCAM, CFG_TM, RAM_LINE, reg_cfg_tm, ram_line);

    RU_REG_RAM_WRITE(tcam_id, cfg_tm, TCAM, CFG_TM, reg_cfg_tm);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_cfg_tm_get(uint8_t tcam_id, uint32_t cfg_tm, uint32_t *ram_line)
{
    uint32_t reg_cfg_tm;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (cfg_tm >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, cfg_tm, TCAM, CFG_TM, reg_cfg_tm);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, CFG_TM, RAM_LINE, reg_cfg_tm);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_cfg_vreset_set(uint8_t tcam_id, uint16_t val)
{
    uint32_t reg_cfg_vreset = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cfg_vreset = RU_FIELD_SET(tcam_id, TCAM, CFG_VRESET, VAL, reg_cfg_vreset, val);

    RU_REG_WRITE(tcam_id, TCAM, CFG_VRESET, reg_cfg_vreset);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_cfg_vreset_get(uint8_t tcam_id, uint16_t *val)
{
    uint32_t reg_cfg_vreset;

#ifdef VALIDATE_PARMS
    if (!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(tcam_id, TCAM, CFG_VRESET, reg_cfg_vreset);

    *val = RU_FIELD_GET(tcam_id, TCAM, CFG_VRESET, VAL, reg_cfg_vreset);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_counters_searches_get(uint8_t tcam_id, uint32_t counters_searches, uint32_t *cnt)
{
    uint32_t reg_counters_searches;

#ifdef VALIDATE_PARMS
    if (!cnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (counters_searches >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, counters_searches, TCAM, COUNTERS_SEARCHES, reg_counters_searches);

    *cnt = RU_FIELD_GET(tcam_id, TCAM, COUNTERS_SEARCHES, CNT, reg_counters_searches);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_counters_hits_get(uint8_t tcam_id, uint8_t tcam_idx, uint32_t *cnt)
{
    uint32_t reg_counters_hits;

#ifdef VALIDATE_PARMS
    if (!cnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (tcam_idx >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, tcam_idx, TCAM, COUNTERS_HITS, reg_counters_hits);

    *cnt = RU_FIELD_GET(tcam_id, TCAM, COUNTERS_HITS, CNT, reg_counters_hits);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_tcam_mem_indirect_op_set(uint8_t tcam_id, uint8_t cmd)
{
    uint32_t reg_tcam_mem_indirect_op = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (cmd >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tcam_mem_indirect_op = RU_FIELD_SET(tcam_id, TCAM, TCAM_MEM_INDIRECT_OP, CMD, reg_tcam_mem_indirect_op, cmd);

    RU_REG_WRITE(tcam_id, TCAM, TCAM_MEM_INDIRECT_OP, reg_tcam_mem_indirect_op);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_tcam_mem_indirect_op_get(uint8_t tcam_id, uint8_t *cmd)
{
    uint32_t reg_tcam_mem_indirect_op;

#ifdef VALIDATE_PARMS
    if (!cmd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(tcam_id, TCAM, TCAM_MEM_INDIRECT_OP, reg_tcam_mem_indirect_op);

    *cmd = RU_FIELD_GET(tcam_id, TCAM, TCAM_MEM_INDIRECT_OP, CMD, reg_tcam_mem_indirect_op);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_tcam_mem_indirect_op_done_get(uint8_t tcam_id, bdmf_boolean *done)
{
    uint32_t reg_tcam_mem_indirect_op_done;

#ifdef VALIDATE_PARMS
    if (!done)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(tcam_id, TCAM, TCAM_MEM_INDIRECT_OP_DONE, reg_tcam_mem_indirect_op_done);

    *done = RU_FIELD_GET(tcam_id, TCAM, TCAM_MEM_INDIRECT_OP_DONE, DONE, reg_tcam_mem_indirect_op_done);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_tcam_mem_indirect_addr_set(uint8_t tcam_id, uint16_t entry_addr, uint8_t mem_id)
{
    uint32_t reg_tcam_mem_indirect_addr = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (entry_addr >= _10BITS_MAX_VAL_) ||
       (mem_id >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tcam_mem_indirect_addr = RU_FIELD_SET(tcam_id, TCAM, TCAM_MEM_INDIRECT_ADDR, ENTRY_ADDR, reg_tcam_mem_indirect_addr, entry_addr);
    reg_tcam_mem_indirect_addr = RU_FIELD_SET(tcam_id, TCAM, TCAM_MEM_INDIRECT_ADDR, MEM_ID, reg_tcam_mem_indirect_addr, mem_id);

    RU_REG_WRITE(tcam_id, TCAM, TCAM_MEM_INDIRECT_ADDR, reg_tcam_mem_indirect_addr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_tcam_mem_indirect_addr_get(uint8_t tcam_id, uint16_t *entry_addr, uint8_t *mem_id)
{
    uint32_t reg_tcam_mem_indirect_addr;

#ifdef VALIDATE_PARMS
    if (!entry_addr || !mem_id)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(tcam_id, TCAM, TCAM_MEM_INDIRECT_ADDR, reg_tcam_mem_indirect_addr);

    *entry_addr = RU_FIELD_GET(tcam_id, TCAM, TCAM_MEM_INDIRECT_ADDR, ENTRY_ADDR, reg_tcam_mem_indirect_addr);
    *mem_id = RU_FIELD_GET(tcam_id, TCAM, TCAM_MEM_INDIRECT_ADDR, MEM_ID, reg_tcam_mem_indirect_addr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_tcam_mem_indirect_valid_in_set(uint8_t tcam_id, uint8_t valid)
{
    uint32_t reg_tcam_mem_indirect_valid_in = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (valid >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tcam_mem_indirect_valid_in = RU_FIELD_SET(tcam_id, TCAM, TCAM_MEM_INDIRECT_VALID_IN, VALID, reg_tcam_mem_indirect_valid_in, valid);

    RU_REG_WRITE(tcam_id, TCAM, TCAM_MEM_INDIRECT_VALID_IN, reg_tcam_mem_indirect_valid_in);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_tcam_mem_indirect_valid_in_get(uint8_t tcam_id, uint8_t *valid)
{
    uint32_t reg_tcam_mem_indirect_valid_in;

#ifdef VALIDATE_PARMS
    if (!valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(tcam_id, TCAM, TCAM_MEM_INDIRECT_VALID_IN, reg_tcam_mem_indirect_valid_in);

    *valid = RU_FIELD_GET(tcam_id, TCAM, TCAM_MEM_INDIRECT_VALID_IN, VALID, reg_tcam_mem_indirect_valid_in);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_tcam_mem_indirect_valid_out_get(uint8_t tcam_id, uint8_t *valid)
{
    uint32_t reg_tcam_mem_indirect_valid_out;

#ifdef VALIDATE_PARMS
    if (!valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(tcam_id, TCAM, TCAM_MEM_INDIRECT_VALID_OUT, reg_tcam_mem_indirect_valid_out);

    *valid = RU_FIELD_GET(tcam_id, TCAM, TCAM_MEM_INDIRECT_VALID_OUT, VALID, reg_tcam_mem_indirect_valid_out);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_tcam_mem_indirect_data_out_get(uint8_t tcam_id, uint8_t indirect_data, uint32_t *value)
{
    uint32_t reg_tcam_mem_indirect_data_out;

#ifdef VALIDATE_PARMS
    if (!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (indirect_data >= 8))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, indirect_data, TCAM, TCAM_MEM_INDIRECT_DATA_OUT, reg_tcam_mem_indirect_data_out);

    *value = RU_FIELD_GET(tcam_id, TCAM, TCAM_MEM_INDIRECT_DATA_OUT, VALUE, reg_tcam_mem_indirect_data_out);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_tcam_mem_indirect_data_in_set(uint8_t tcam_id, uint8_t indirect_data_in, uint32_t value)
{
    uint32_t reg_tcam_mem_indirect_data_in = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (indirect_data_in >= 16))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tcam_mem_indirect_data_in = RU_FIELD_SET(tcam_id, TCAM, TCAM_MEM_INDIRECT_DATA_IN, VALUE, reg_tcam_mem_indirect_data_in, value);

    RU_REG_RAM_WRITE(tcam_id, indirect_data_in, TCAM, TCAM_MEM_INDIRECT_DATA_IN, reg_tcam_mem_indirect_data_in);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_tcam_mem_indirect_data_in_get(uint8_t tcam_id, uint8_t indirect_data_in, uint32_t *value)
{
    uint32_t reg_tcam_mem_indirect_data_in;

#ifdef VALIDATE_PARMS
    if (!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (indirect_data_in >= 16))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, indirect_data_in, TCAM, TCAM_MEM_INDIRECT_DATA_IN, reg_tcam_mem_indirect_data_in);

    *value = RU_FIELD_GET(tcam_id, TCAM, TCAM_MEM_INDIRECT_DATA_IN, VALUE, reg_tcam_mem_indirect_data_in);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_tcam_debug_bus_select_set(uint8_t tcam_id, uint8_t select_module)
{
    uint32_t reg_tcam_debug_bus_select = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (select_module >= _2BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tcam_debug_bus_select = RU_FIELD_SET(tcam_id, TCAM, TCAM_DEBUG_BUS_SELECT, SELECT_MODULE, reg_tcam_debug_bus_select, select_module);

    RU_REG_WRITE(tcam_id, TCAM, TCAM_DEBUG_BUS_SELECT, reg_tcam_debug_bus_select);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_tcam_debug_bus_select_get(uint8_t tcam_id, uint8_t *select_module)
{
    uint32_t reg_tcam_debug_bus_select;

#ifdef VALIDATE_PARMS
    if (!select_module)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(tcam_id, TCAM, TCAM_DEBUG_BUS_SELECT, reg_tcam_debug_bus_select);

    *select_module = RU_FIELD_GET(tcam_id, TCAM, TCAM_DEBUG_BUS_SELECT, SELECT_MODULE, reg_tcam_debug_bus_select);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_0_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_0 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_0 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_0, RAM_LINE, reg_profiles_key_builder_offsets_0, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_0, reg_profiles_key_builder_offsets_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_0_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_0;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_0, reg_profiles_key_builder_offsets_0);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_0, RAM_LINE, reg_profiles_key_builder_offsets_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_1_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_1 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_1 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_1, RAM_LINE, reg_profiles_key_builder_offsets_1, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_1, reg_profiles_key_builder_offsets_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_1_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_1;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_1, reg_profiles_key_builder_offsets_1);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_1, RAM_LINE, reg_profiles_key_builder_offsets_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_2_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_2 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_2 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_2, RAM_LINE, reg_profiles_key_builder_offsets_2, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_2, reg_profiles_key_builder_offsets_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_2_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_2;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_2, reg_profiles_key_builder_offsets_2);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_2, RAM_LINE, reg_profiles_key_builder_offsets_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_3_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_3 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_3 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_3, RAM_LINE, reg_profiles_key_builder_offsets_3, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_3, reg_profiles_key_builder_offsets_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_3_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_3;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_3, reg_profiles_key_builder_offsets_3);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_3, RAM_LINE, reg_profiles_key_builder_offsets_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_4_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_4 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_4 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_4, RAM_LINE, reg_profiles_key_builder_offsets_4, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_4, reg_profiles_key_builder_offsets_4);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_4_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_4;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_4, reg_profiles_key_builder_offsets_4);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_4, RAM_LINE, reg_profiles_key_builder_offsets_4);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_5_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_5 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_5 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_5, RAM_LINE, reg_profiles_key_builder_offsets_5, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_5, reg_profiles_key_builder_offsets_5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_5_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_5;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_5, reg_profiles_key_builder_offsets_5);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_5, RAM_LINE, reg_profiles_key_builder_offsets_5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_6_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_6 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_6 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_6, RAM_LINE, reg_profiles_key_builder_offsets_6, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_6, reg_profiles_key_builder_offsets_6);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_6_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_6;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_6, reg_profiles_key_builder_offsets_6);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_6, RAM_LINE, reg_profiles_key_builder_offsets_6);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_7_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_7 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_7 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_7, RAM_LINE, reg_profiles_key_builder_offsets_7, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_7, reg_profiles_key_builder_offsets_7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_7_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_7;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_7, reg_profiles_key_builder_offsets_7);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_7, RAM_LINE, reg_profiles_key_builder_offsets_7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_8_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_8 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_8 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_8, RAM_LINE, reg_profiles_key_builder_offsets_8, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_8, reg_profiles_key_builder_offsets_8);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_8_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_8;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_8, reg_profiles_key_builder_offsets_8);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_8, RAM_LINE, reg_profiles_key_builder_offsets_8);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_9_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_9 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_9 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_9, RAM_LINE, reg_profiles_key_builder_offsets_9, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_9, reg_profiles_key_builder_offsets_9);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_9_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_9;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_9, reg_profiles_key_builder_offsets_9);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_9, RAM_LINE, reg_profiles_key_builder_offsets_9);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_10_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_10 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_10 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_10, RAM_LINE, reg_profiles_key_builder_offsets_10, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_10, reg_profiles_key_builder_offsets_10);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_10_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_10;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_10, reg_profiles_key_builder_offsets_10);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_10, RAM_LINE, reg_profiles_key_builder_offsets_10);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_11_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_11 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_11 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_11, RAM_LINE, reg_profiles_key_builder_offsets_11, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_11, reg_profiles_key_builder_offsets_11);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_11_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_11;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_11, reg_profiles_key_builder_offsets_11);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_11, RAM_LINE, reg_profiles_key_builder_offsets_11);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_12_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_12 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_12 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_12, RAM_LINE, reg_profiles_key_builder_offsets_12, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_12, reg_profiles_key_builder_offsets_12);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_12_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_12;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_12, reg_profiles_key_builder_offsets_12);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_12, RAM_LINE, reg_profiles_key_builder_offsets_12);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_13_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_13 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_13 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_13, RAM_LINE, reg_profiles_key_builder_offsets_13, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_13, reg_profiles_key_builder_offsets_13);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_13_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_13;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_13, reg_profiles_key_builder_offsets_13);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_13, RAM_LINE, reg_profiles_key_builder_offsets_13);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_14_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_14 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_14 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_14, RAM_LINE, reg_profiles_key_builder_offsets_14, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_14, reg_profiles_key_builder_offsets_14);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_14_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_14;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_14, reg_profiles_key_builder_offsets_14);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_14, RAM_LINE, reg_profiles_key_builder_offsets_14);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_15_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_15 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_15 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_15, RAM_LINE, reg_profiles_key_builder_offsets_15, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_15, reg_profiles_key_builder_offsets_15);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_15_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_15;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_15, reg_profiles_key_builder_offsets_15);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_15, RAM_LINE, reg_profiles_key_builder_offsets_15);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_16_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_16 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_16 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_16, RAM_LINE, reg_profiles_key_builder_offsets_16, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_16, reg_profiles_key_builder_offsets_16);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_16_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_16;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_16, reg_profiles_key_builder_offsets_16);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_16, RAM_LINE, reg_profiles_key_builder_offsets_16);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_17_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_17 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_17 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_17, RAM_LINE, reg_profiles_key_builder_offsets_17, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_17, reg_profiles_key_builder_offsets_17);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_17_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_17;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_17, reg_profiles_key_builder_offsets_17);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_17, RAM_LINE, reg_profiles_key_builder_offsets_17);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_18_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_18 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_18 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_18, RAM_LINE, reg_profiles_key_builder_offsets_18, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_18, reg_profiles_key_builder_offsets_18);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_18_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_18;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_18, reg_profiles_key_builder_offsets_18);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_18, RAM_LINE, reg_profiles_key_builder_offsets_18);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_19_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_19 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_19 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_19, RAM_LINE, reg_profiles_key_builder_offsets_19, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_19, reg_profiles_key_builder_offsets_19);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_19_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_19;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_19, reg_profiles_key_builder_offsets_19);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_19, RAM_LINE, reg_profiles_key_builder_offsets_19);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_20_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_20 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_20 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_20, RAM_LINE, reg_profiles_key_builder_offsets_20, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_20, reg_profiles_key_builder_offsets_20);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_20_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_20;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_20, reg_profiles_key_builder_offsets_20);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_20, RAM_LINE, reg_profiles_key_builder_offsets_20);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_21_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_21 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_21 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_21, RAM_LINE, reg_profiles_key_builder_offsets_21, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_21, reg_profiles_key_builder_offsets_21);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_21_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_21;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_21, reg_profiles_key_builder_offsets_21);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_21, RAM_LINE, reg_profiles_key_builder_offsets_21);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_22_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_22 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_22 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_22, RAM_LINE, reg_profiles_key_builder_offsets_22, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_22, reg_profiles_key_builder_offsets_22);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_22_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_22;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_22, reg_profiles_key_builder_offsets_22);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_22, RAM_LINE, reg_profiles_key_builder_offsets_22);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_23_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_23 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_23 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_23, RAM_LINE, reg_profiles_key_builder_offsets_23, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_23, reg_profiles_key_builder_offsets_23);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_23_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_23;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_23, reg_profiles_key_builder_offsets_23);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_23, RAM_LINE, reg_profiles_key_builder_offsets_23);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_24_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_24 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_24 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_24, RAM_LINE, reg_profiles_key_builder_offsets_24, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_24, reg_profiles_key_builder_offsets_24);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_24_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_24;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_24, reg_profiles_key_builder_offsets_24);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_24, RAM_LINE, reg_profiles_key_builder_offsets_24);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_25_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_25 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_25 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_25, RAM_LINE, reg_profiles_key_builder_offsets_25, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_25, reg_profiles_key_builder_offsets_25);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_25_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_25;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_25, reg_profiles_key_builder_offsets_25);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_25, RAM_LINE, reg_profiles_key_builder_offsets_25);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_26_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_26 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_26 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_26, RAM_LINE, reg_profiles_key_builder_offsets_26, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_26, reg_profiles_key_builder_offsets_26);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_26_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_26;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_26, reg_profiles_key_builder_offsets_26);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_26, RAM_LINE, reg_profiles_key_builder_offsets_26);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_27_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_27 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_27 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_27, RAM_LINE, reg_profiles_key_builder_offsets_27, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_27, reg_profiles_key_builder_offsets_27);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_27_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_27;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_27, reg_profiles_key_builder_offsets_27);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_27, RAM_LINE, reg_profiles_key_builder_offsets_27);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_28_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_28 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_28 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_28, RAM_LINE, reg_profiles_key_builder_offsets_28, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_28, reg_profiles_key_builder_offsets_28);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_28_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_28;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_28, reg_profiles_key_builder_offsets_28);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_28, RAM_LINE, reg_profiles_key_builder_offsets_28);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_29_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_29 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_29 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_29, RAM_LINE, reg_profiles_key_builder_offsets_29, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_29, reg_profiles_key_builder_offsets_29);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_29_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_29;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_29, reg_profiles_key_builder_offsets_29);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_29, RAM_LINE, reg_profiles_key_builder_offsets_29);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_30_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_30 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_30 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_30, RAM_LINE, reg_profiles_key_builder_offsets_30, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_30, reg_profiles_key_builder_offsets_30);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_30_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_30;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_30, reg_profiles_key_builder_offsets_30);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_30, RAM_LINE, reg_profiles_key_builder_offsets_30);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_31_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_31 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_31 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_31, RAM_LINE, reg_profiles_key_builder_offsets_31, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_31, reg_profiles_key_builder_offsets_31);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_31_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_31;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_31, reg_profiles_key_builder_offsets_31);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_31, RAM_LINE, reg_profiles_key_builder_offsets_31);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_32_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_32 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_32 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_32, RAM_LINE, reg_profiles_key_builder_offsets_32, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_32, reg_profiles_key_builder_offsets_32);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_32_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_32;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_32, reg_profiles_key_builder_offsets_32);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_32, RAM_LINE, reg_profiles_key_builder_offsets_32);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_33_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_33 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_33 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_33, RAM_LINE, reg_profiles_key_builder_offsets_33, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_33, reg_profiles_key_builder_offsets_33);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_33_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_33;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_33, reg_profiles_key_builder_offsets_33);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_33, RAM_LINE, reg_profiles_key_builder_offsets_33);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_34_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_34 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_34 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_34, RAM_LINE, reg_profiles_key_builder_offsets_34, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_34, reg_profiles_key_builder_offsets_34);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_34_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_34;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_34, reg_profiles_key_builder_offsets_34);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_34, RAM_LINE, reg_profiles_key_builder_offsets_34);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_35_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_35 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_35 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_35, RAM_LINE, reg_profiles_key_builder_offsets_35, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_35, reg_profiles_key_builder_offsets_35);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_35_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_35;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_35, reg_profiles_key_builder_offsets_35);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_35, RAM_LINE, reg_profiles_key_builder_offsets_35);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_36_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_36 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_36 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_36, RAM_LINE, reg_profiles_key_builder_offsets_36, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_36, reg_profiles_key_builder_offsets_36);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_36_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_36;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_36, reg_profiles_key_builder_offsets_36);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_36, RAM_LINE, reg_profiles_key_builder_offsets_36);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_37_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_37 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_37 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_37, RAM_LINE, reg_profiles_key_builder_offsets_37, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_37, reg_profiles_key_builder_offsets_37);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_37_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_37;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_37, reg_profiles_key_builder_offsets_37);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_37, RAM_LINE, reg_profiles_key_builder_offsets_37);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_38_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_38 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_38 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_38, RAM_LINE, reg_profiles_key_builder_offsets_38, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_38, reg_profiles_key_builder_offsets_38);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_38_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_38;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_38, reg_profiles_key_builder_offsets_38);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_38, RAM_LINE, reg_profiles_key_builder_offsets_38);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_39_set(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_39 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_key_builder_offsets_39 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_39, RAM_LINE, reg_profiles_key_builder_offsets_39, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_39, reg_profiles_key_builder_offsets_39);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_key_builder_offsets_39_get(uint8_t tcam_id, uint8_t profile_key_builder_offset, uint32_t *ram_line)
{
    uint32_t reg_profiles_key_builder_offsets_39;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_key_builder_offset >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_key_builder_offset, TCAM, PROFILES_KEY_BUILDER_OFFSETS_39, reg_profiles_key_builder_offsets_39);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_KEY_BUILDER_OFFSETS_39, RAM_LINE, reg_profiles_key_builder_offsets_39);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_0_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line)
{
    uint32_t reg_profiles_inner_shift_0 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_inner_shift_0 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_INNER_SHIFT_0, RAM_LINE, reg_profiles_inner_shift_0, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_0, reg_profiles_inner_shift_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_0_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line)
{
    uint32_t reg_profiles_inner_shift_0;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_0, reg_profiles_inner_shift_0);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_INNER_SHIFT_0, RAM_LINE, reg_profiles_inner_shift_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_1_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line)
{
    uint32_t reg_profiles_inner_shift_1 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_inner_shift_1 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_INNER_SHIFT_1, RAM_LINE, reg_profiles_inner_shift_1, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_1, reg_profiles_inner_shift_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_1_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line)
{
    uint32_t reg_profiles_inner_shift_1;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_1, reg_profiles_inner_shift_1);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_INNER_SHIFT_1, RAM_LINE, reg_profiles_inner_shift_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_2_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line)
{
    uint32_t reg_profiles_inner_shift_2 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_inner_shift_2 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_INNER_SHIFT_2, RAM_LINE, reg_profiles_inner_shift_2, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_2, reg_profiles_inner_shift_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_2_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line)
{
    uint32_t reg_profiles_inner_shift_2;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_2, reg_profiles_inner_shift_2);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_INNER_SHIFT_2, RAM_LINE, reg_profiles_inner_shift_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_3_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line)
{
    uint32_t reg_profiles_inner_shift_3 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_inner_shift_3 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_INNER_SHIFT_3, RAM_LINE, reg_profiles_inner_shift_3, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_3, reg_profiles_inner_shift_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_3_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line)
{
    uint32_t reg_profiles_inner_shift_3;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_3, reg_profiles_inner_shift_3);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_INNER_SHIFT_3, RAM_LINE, reg_profiles_inner_shift_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_4_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line)
{
    uint32_t reg_profiles_inner_shift_4 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_inner_shift_4 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_INNER_SHIFT_4, RAM_LINE, reg_profiles_inner_shift_4, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_4, reg_profiles_inner_shift_4);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_4_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line)
{
    uint32_t reg_profiles_inner_shift_4;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_4, reg_profiles_inner_shift_4);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_INNER_SHIFT_4, RAM_LINE, reg_profiles_inner_shift_4);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_5_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line)
{
    uint32_t reg_profiles_inner_shift_5 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_inner_shift_5 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_INNER_SHIFT_5, RAM_LINE, reg_profiles_inner_shift_5, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_5, reg_profiles_inner_shift_5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_5_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line)
{
    uint32_t reg_profiles_inner_shift_5;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_5, reg_profiles_inner_shift_5);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_INNER_SHIFT_5, RAM_LINE, reg_profiles_inner_shift_5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_6_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line)
{
    uint32_t reg_profiles_inner_shift_6 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_inner_shift_6 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_INNER_SHIFT_6, RAM_LINE, reg_profiles_inner_shift_6, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_6, reg_profiles_inner_shift_6);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_6_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line)
{
    uint32_t reg_profiles_inner_shift_6;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_6, reg_profiles_inner_shift_6);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_INNER_SHIFT_6, RAM_LINE, reg_profiles_inner_shift_6);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_7_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line)
{
    uint32_t reg_profiles_inner_shift_7 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_inner_shift_7 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_INNER_SHIFT_7, RAM_LINE, reg_profiles_inner_shift_7, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_7, reg_profiles_inner_shift_7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_7_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line)
{
    uint32_t reg_profiles_inner_shift_7;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_7, reg_profiles_inner_shift_7);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_INNER_SHIFT_7, RAM_LINE, reg_profiles_inner_shift_7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_8_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line)
{
    uint32_t reg_profiles_inner_shift_8 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_inner_shift_8 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_INNER_SHIFT_8, RAM_LINE, reg_profiles_inner_shift_8, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_8, reg_profiles_inner_shift_8);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_8_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line)
{
    uint32_t reg_profiles_inner_shift_8;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_8, reg_profiles_inner_shift_8);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_INNER_SHIFT_8, RAM_LINE, reg_profiles_inner_shift_8);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_9_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line)
{
    uint32_t reg_profiles_inner_shift_9 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_inner_shift_9 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_INNER_SHIFT_9, RAM_LINE, reg_profiles_inner_shift_9, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_9, reg_profiles_inner_shift_9);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_9_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line)
{
    uint32_t reg_profiles_inner_shift_9;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_9, reg_profiles_inner_shift_9);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_INNER_SHIFT_9, RAM_LINE, reg_profiles_inner_shift_9);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_10_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line)
{
    uint32_t reg_profiles_inner_shift_10 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_inner_shift_10 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_INNER_SHIFT_10, RAM_LINE, reg_profiles_inner_shift_10, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_10, reg_profiles_inner_shift_10);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_10_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line)
{
    uint32_t reg_profiles_inner_shift_10;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_10, reg_profiles_inner_shift_10);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_INNER_SHIFT_10, RAM_LINE, reg_profiles_inner_shift_10);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_11_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line)
{
    uint32_t reg_profiles_inner_shift_11 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_inner_shift_11 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_INNER_SHIFT_11, RAM_LINE, reg_profiles_inner_shift_11, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_11, reg_profiles_inner_shift_11);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_11_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line)
{
    uint32_t reg_profiles_inner_shift_11;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_11, reg_profiles_inner_shift_11);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_INNER_SHIFT_11, RAM_LINE, reg_profiles_inner_shift_11);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_12_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line)
{
    uint32_t reg_profiles_inner_shift_12 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_inner_shift_12 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_INNER_SHIFT_12, RAM_LINE, reg_profiles_inner_shift_12, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_12, reg_profiles_inner_shift_12);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_12_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line)
{
    uint32_t reg_profiles_inner_shift_12;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_12, reg_profiles_inner_shift_12);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_INNER_SHIFT_12, RAM_LINE, reg_profiles_inner_shift_12);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_13_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line)
{
    uint32_t reg_profiles_inner_shift_13 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_inner_shift_13 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_INNER_SHIFT_13, RAM_LINE, reg_profiles_inner_shift_13, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_13, reg_profiles_inner_shift_13);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_13_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line)
{
    uint32_t reg_profiles_inner_shift_13;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_13, reg_profiles_inner_shift_13);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_INNER_SHIFT_13, RAM_LINE, reg_profiles_inner_shift_13);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_14_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line)
{
    uint32_t reg_profiles_inner_shift_14 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_inner_shift_14 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_INNER_SHIFT_14, RAM_LINE, reg_profiles_inner_shift_14, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_14, reg_profiles_inner_shift_14);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_14_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line)
{
    uint32_t reg_profiles_inner_shift_14;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_14, reg_profiles_inner_shift_14);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_INNER_SHIFT_14, RAM_LINE, reg_profiles_inner_shift_14);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_15_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line)
{
    uint32_t reg_profiles_inner_shift_15 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_inner_shift_15 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_INNER_SHIFT_15, RAM_LINE, reg_profiles_inner_shift_15, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_15, reg_profiles_inner_shift_15);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_15_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line)
{
    uint32_t reg_profiles_inner_shift_15;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_15, reg_profiles_inner_shift_15);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_INNER_SHIFT_15, RAM_LINE, reg_profiles_inner_shift_15);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_16_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line)
{
    uint32_t reg_profiles_inner_shift_16 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_inner_shift_16 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_INNER_SHIFT_16, RAM_LINE, reg_profiles_inner_shift_16, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_16, reg_profiles_inner_shift_16);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_16_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line)
{
    uint32_t reg_profiles_inner_shift_16;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_16, reg_profiles_inner_shift_16);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_INNER_SHIFT_16, RAM_LINE, reg_profiles_inner_shift_16);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_17_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line)
{
    uint32_t reg_profiles_inner_shift_17 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_inner_shift_17 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_INNER_SHIFT_17, RAM_LINE, reg_profiles_inner_shift_17, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_17, reg_profiles_inner_shift_17);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_17_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line)
{
    uint32_t reg_profiles_inner_shift_17;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_17, reg_profiles_inner_shift_17);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_INNER_SHIFT_17, RAM_LINE, reg_profiles_inner_shift_17);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_18_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line)
{
    uint32_t reg_profiles_inner_shift_18 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_inner_shift_18 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_INNER_SHIFT_18, RAM_LINE, reg_profiles_inner_shift_18, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_18, reg_profiles_inner_shift_18);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_18_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line)
{
    uint32_t reg_profiles_inner_shift_18;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_18, reg_profiles_inner_shift_18);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_INNER_SHIFT_18, RAM_LINE, reg_profiles_inner_shift_18);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_19_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line)
{
    uint32_t reg_profiles_inner_shift_19 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_inner_shift_19 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_INNER_SHIFT_19, RAM_LINE, reg_profiles_inner_shift_19, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_19, reg_profiles_inner_shift_19);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_19_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line)
{
    uint32_t reg_profiles_inner_shift_19;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_19, reg_profiles_inner_shift_19);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_INNER_SHIFT_19, RAM_LINE, reg_profiles_inner_shift_19);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_20_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line)
{
    uint32_t reg_profiles_inner_shift_20 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_inner_shift_20 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_INNER_SHIFT_20, RAM_LINE, reg_profiles_inner_shift_20, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_20, reg_profiles_inner_shift_20);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_20_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line)
{
    uint32_t reg_profiles_inner_shift_20;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_20, reg_profiles_inner_shift_20);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_INNER_SHIFT_20, RAM_LINE, reg_profiles_inner_shift_20);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_21_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line)
{
    uint32_t reg_profiles_inner_shift_21 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_inner_shift_21 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_INNER_SHIFT_21, RAM_LINE, reg_profiles_inner_shift_21, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_21, reg_profiles_inner_shift_21);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_21_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line)
{
    uint32_t reg_profiles_inner_shift_21;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_21, reg_profiles_inner_shift_21);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_INNER_SHIFT_21, RAM_LINE, reg_profiles_inner_shift_21);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_22_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line)
{
    uint32_t reg_profiles_inner_shift_22 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_inner_shift_22 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_INNER_SHIFT_22, RAM_LINE, reg_profiles_inner_shift_22, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_22, reg_profiles_inner_shift_22);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_22_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line)
{
    uint32_t reg_profiles_inner_shift_22;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_22, reg_profiles_inner_shift_22);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_INNER_SHIFT_22, RAM_LINE, reg_profiles_inner_shift_22);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_23_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line)
{
    uint32_t reg_profiles_inner_shift_23 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_inner_shift_23 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_INNER_SHIFT_23, RAM_LINE, reg_profiles_inner_shift_23, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_23, reg_profiles_inner_shift_23);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_23_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line)
{
    uint32_t reg_profiles_inner_shift_23;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_23, reg_profiles_inner_shift_23);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_INNER_SHIFT_23, RAM_LINE, reg_profiles_inner_shift_23);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_24_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line)
{
    uint32_t reg_profiles_inner_shift_24 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_inner_shift_24 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_INNER_SHIFT_24, RAM_LINE, reg_profiles_inner_shift_24, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_24, reg_profiles_inner_shift_24);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_24_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line)
{
    uint32_t reg_profiles_inner_shift_24;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_24, reg_profiles_inner_shift_24);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_INNER_SHIFT_24, RAM_LINE, reg_profiles_inner_shift_24);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_25_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line)
{
    uint32_t reg_profiles_inner_shift_25 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_inner_shift_25 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_INNER_SHIFT_25, RAM_LINE, reg_profiles_inner_shift_25, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_25, reg_profiles_inner_shift_25);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_25_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line)
{
    uint32_t reg_profiles_inner_shift_25;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_25, reg_profiles_inner_shift_25);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_INNER_SHIFT_25, RAM_LINE, reg_profiles_inner_shift_25);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_26_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line)
{
    uint32_t reg_profiles_inner_shift_26 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_inner_shift_26 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_INNER_SHIFT_26, RAM_LINE, reg_profiles_inner_shift_26, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_26, reg_profiles_inner_shift_26);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_26_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line)
{
    uint32_t reg_profiles_inner_shift_26;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_26, reg_profiles_inner_shift_26);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_INNER_SHIFT_26, RAM_LINE, reg_profiles_inner_shift_26);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_27_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line)
{
    uint32_t reg_profiles_inner_shift_27 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_inner_shift_27 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_INNER_SHIFT_27, RAM_LINE, reg_profiles_inner_shift_27, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_27, reg_profiles_inner_shift_27);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_27_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line)
{
    uint32_t reg_profiles_inner_shift_27;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_27, reg_profiles_inner_shift_27);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_INNER_SHIFT_27, RAM_LINE, reg_profiles_inner_shift_27);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_28_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line)
{
    uint32_t reg_profiles_inner_shift_28 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_inner_shift_28 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_INNER_SHIFT_28, RAM_LINE, reg_profiles_inner_shift_28, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_28, reg_profiles_inner_shift_28);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_28_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line)
{
    uint32_t reg_profiles_inner_shift_28;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_28, reg_profiles_inner_shift_28);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_INNER_SHIFT_28, RAM_LINE, reg_profiles_inner_shift_28);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_29_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line)
{
    uint32_t reg_profiles_inner_shift_29 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_inner_shift_29 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_INNER_SHIFT_29, RAM_LINE, reg_profiles_inner_shift_29, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_29, reg_profiles_inner_shift_29);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_29_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line)
{
    uint32_t reg_profiles_inner_shift_29;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_29, reg_profiles_inner_shift_29);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_INNER_SHIFT_29, RAM_LINE, reg_profiles_inner_shift_29);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_30_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line)
{
    uint32_t reg_profiles_inner_shift_30 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_inner_shift_30 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_INNER_SHIFT_30, RAM_LINE, reg_profiles_inner_shift_30, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_30, reg_profiles_inner_shift_30);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_30_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line)
{
    uint32_t reg_profiles_inner_shift_30;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_30, reg_profiles_inner_shift_30);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_INNER_SHIFT_30, RAM_LINE, reg_profiles_inner_shift_30);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_31_set(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t ram_line)
{
    uint32_t reg_profiles_inner_shift_31 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_inner_shift_31 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_INNER_SHIFT_31, RAM_LINE, reg_profiles_inner_shift_31, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_31, reg_profiles_inner_shift_31);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_inner_shift_31_get(uint8_t tcam_id, uint8_t profile_inner_shift, uint32_t *ram_line)
{
    uint32_t reg_profiles_inner_shift_31;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_inner_shift >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_inner_shift, TCAM, PROFILES_INNER_SHIFT_31, reg_profiles_inner_shift_31);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_INNER_SHIFT_31, RAM_LINE, reg_profiles_inner_shift_31);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_cntxin_sp_id_0_set(uint8_t tcam_id, uint8_t profile_sp_id, uint32_t ram_line)
{
    uint32_t reg_profiles_cntxin_sp_id_0 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_sp_id >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_cntxin_sp_id_0 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_CNTXIN_SP_ID_0, RAM_LINE, reg_profiles_cntxin_sp_id_0, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_sp_id, TCAM, PROFILES_CNTXIN_SP_ID_0, reg_profiles_cntxin_sp_id_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_cntxin_sp_id_0_get(uint8_t tcam_id, uint8_t profile_sp_id, uint32_t *ram_line)
{
    uint32_t reg_profiles_cntxin_sp_id_0;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_sp_id >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_sp_id, TCAM, PROFILES_CNTXIN_SP_ID_0, reg_profiles_cntxin_sp_id_0);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_CNTXIN_SP_ID_0, RAM_LINE, reg_profiles_cntxin_sp_id_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_cntxin_sp_id_1_set(uint8_t tcam_id, uint8_t profile_sp_id, uint32_t ram_line)
{
    uint32_t reg_profiles_cntxin_sp_id_1 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_sp_id >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_cntxin_sp_id_1 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_CNTXIN_SP_ID_1, RAM_LINE, reg_profiles_cntxin_sp_id_1, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_sp_id, TCAM, PROFILES_CNTXIN_SP_ID_1, reg_profiles_cntxin_sp_id_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_cntxin_sp_id_1_get(uint8_t tcam_id, uint8_t profile_sp_id, uint32_t *ram_line)
{
    uint32_t reg_profiles_cntxin_sp_id_1;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_sp_id >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_sp_id, TCAM, PROFILES_CNTXIN_SP_ID_1, reg_profiles_cntxin_sp_id_1);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_CNTXIN_SP_ID_1, RAM_LINE, reg_profiles_cntxin_sp_id_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_cntxin_mask_0_set(uint8_t tcam_id, uint8_t profile_cntxin_mask, uint32_t ram_line)
{
    uint32_t reg_profiles_cntxin_mask_0 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_cntxin_mask >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_cntxin_mask_0 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_CNTXIN_MASK_0, RAM_LINE, reg_profiles_cntxin_mask_0, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_cntxin_mask, TCAM, PROFILES_CNTXIN_MASK_0, reg_profiles_cntxin_mask_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_cntxin_mask_0_get(uint8_t tcam_id, uint8_t profile_cntxin_mask, uint32_t *ram_line)
{
    uint32_t reg_profiles_cntxin_mask_0;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_cntxin_mask >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_cntxin_mask, TCAM, PROFILES_CNTXIN_MASK_0, reg_profiles_cntxin_mask_0);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_CNTXIN_MASK_0, RAM_LINE, reg_profiles_cntxin_mask_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_cntxin_mask_1_set(uint8_t tcam_id, uint8_t profile_cntxin_mask, uint32_t ram_line)
{
    uint32_t reg_profiles_cntxin_mask_1 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_cntxin_mask >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_cntxin_mask_1 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_CNTXIN_MASK_1, RAM_LINE, reg_profiles_cntxin_mask_1, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_cntxin_mask, TCAM, PROFILES_CNTXIN_MASK_1, reg_profiles_cntxin_mask_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_cntxin_mask_1_get(uint8_t tcam_id, uint8_t profile_cntxin_mask, uint32_t *ram_line)
{
    uint32_t reg_profiles_cntxin_mask_1;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_cntxin_mask >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_cntxin_mask, TCAM, PROFILES_CNTXIN_MASK_1, reg_profiles_cntxin_mask_1);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_CNTXIN_MASK_1, RAM_LINE, reg_profiles_cntxin_mask_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_cntxin_mask_2_set(uint8_t tcam_id, uint8_t profile_cntxin_mask, uint32_t ram_line)
{
    uint32_t reg_profiles_cntxin_mask_2 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_cntxin_mask >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_cntxin_mask_2 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_CNTXIN_MASK_2, RAM_LINE, reg_profiles_cntxin_mask_2, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_cntxin_mask, TCAM, PROFILES_CNTXIN_MASK_2, reg_profiles_cntxin_mask_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_cntxin_mask_2_get(uint8_t tcam_id, uint8_t profile_cntxin_mask, uint32_t *ram_line)
{
    uint32_t reg_profiles_cntxin_mask_2;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_cntxin_mask >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_cntxin_mask, TCAM, PROFILES_CNTXIN_MASK_2, reg_profiles_cntxin_mask_2);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_CNTXIN_MASK_2, RAM_LINE, reg_profiles_cntxin_mask_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_cntxin_mask_3_set(uint8_t tcam_id, uint8_t profile_cntxin_mask, uint32_t ram_line)
{
    uint32_t reg_profiles_cntxin_mask_3 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_cntxin_mask >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_cntxin_mask_3 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_CNTXIN_MASK_3, RAM_LINE, reg_profiles_cntxin_mask_3, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_cntxin_mask, TCAM, PROFILES_CNTXIN_MASK_3, reg_profiles_cntxin_mask_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_cntxin_mask_3_get(uint8_t tcam_id, uint8_t profile_cntxin_mask, uint32_t *ram_line)
{
    uint32_t reg_profiles_cntxin_mask_3;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_cntxin_mask >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_cntxin_mask, TCAM, PROFILES_CNTXIN_MASK_3, reg_profiles_cntxin_mask_3);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_CNTXIN_MASK_3, RAM_LINE, reg_profiles_cntxin_mask_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_cntxin_mask_4_set(uint8_t tcam_id, uint8_t profile_cntxin_mask, uint32_t ram_line)
{
    uint32_t reg_profiles_cntxin_mask_4 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_cntxin_mask >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_cntxin_mask_4 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_CNTXIN_MASK_4, RAM_LINE, reg_profiles_cntxin_mask_4, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_cntxin_mask, TCAM, PROFILES_CNTXIN_MASK_4, reg_profiles_cntxin_mask_4);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_cntxin_mask_4_get(uint8_t tcam_id, uint8_t profile_cntxin_mask, uint32_t *ram_line)
{
    uint32_t reg_profiles_cntxin_mask_4;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_cntxin_mask >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_cntxin_mask, TCAM, PROFILES_CNTXIN_MASK_4, reg_profiles_cntxin_mask_4);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_CNTXIN_MASK_4, RAM_LINE, reg_profiles_cntxin_mask_4);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_cntxin_mask_5_set(uint8_t tcam_id, uint8_t profile_cntxin_mask, uint32_t ram_line)
{
    uint32_t reg_profiles_cntxin_mask_5 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_cntxin_mask >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_cntxin_mask_5 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_CNTXIN_MASK_5, RAM_LINE, reg_profiles_cntxin_mask_5, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_cntxin_mask, TCAM, PROFILES_CNTXIN_MASK_5, reg_profiles_cntxin_mask_5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_cntxin_mask_5_get(uint8_t tcam_id, uint8_t profile_cntxin_mask, uint32_t *ram_line)
{
    uint32_t reg_profiles_cntxin_mask_5;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_cntxin_mask >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_cntxin_mask, TCAM, PROFILES_CNTXIN_MASK_5, reg_profiles_cntxin_mask_5);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_CNTXIN_MASK_5, RAM_LINE, reg_profiles_cntxin_mask_5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_cntxin_mask_6_set(uint8_t tcam_id, uint8_t profile_cntxin_mask, uint32_t ram_line)
{
    uint32_t reg_profiles_cntxin_mask_6 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_cntxin_mask >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_cntxin_mask_6 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_CNTXIN_MASK_6, RAM_LINE, reg_profiles_cntxin_mask_6, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_cntxin_mask, TCAM, PROFILES_CNTXIN_MASK_6, reg_profiles_cntxin_mask_6);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_cntxin_mask_6_get(uint8_t tcam_id, uint8_t profile_cntxin_mask, uint32_t *ram_line)
{
    uint32_t reg_profiles_cntxin_mask_6;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_cntxin_mask >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_cntxin_mask, TCAM, PROFILES_CNTXIN_MASK_6, reg_profiles_cntxin_mask_6);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_CNTXIN_MASK_6, RAM_LINE, reg_profiles_cntxin_mask_6);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_cntxin_mask_7_set(uint8_t tcam_id, uint8_t profile_cntxin_mask, uint32_t ram_line)
{
    uint32_t reg_profiles_cntxin_mask_7 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_cntxin_mask >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_cntxin_mask_7 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_CNTXIN_MASK_7, RAM_LINE, reg_profiles_cntxin_mask_7, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_cntxin_mask, TCAM, PROFILES_CNTXIN_MASK_7, reg_profiles_cntxin_mask_7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_cntxin_mask_7_get(uint8_t tcam_id, uint8_t profile_cntxin_mask, uint32_t *ram_line)
{
    uint32_t reg_profiles_cntxin_mask_7;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_cntxin_mask >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_cntxin_mask, TCAM, PROFILES_CNTXIN_MASK_7, reg_profiles_cntxin_mask_7);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_CNTXIN_MASK_7, RAM_LINE, reg_profiles_cntxin_mask_7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_sp_mask_0_set(uint8_t tcam_id, uint8_t profile_sp_mask, uint32_t ram_line)
{
    uint32_t reg_profiles_sp_mask_0 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_sp_mask >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_sp_mask_0 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_SP_MASK_0, RAM_LINE, reg_profiles_sp_mask_0, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_sp_mask, TCAM, PROFILES_SP_MASK_0, reg_profiles_sp_mask_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_sp_mask_0_get(uint8_t tcam_id, uint8_t profile_sp_mask, uint32_t *ram_line)
{
    uint32_t reg_profiles_sp_mask_0;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_sp_mask >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_sp_mask, TCAM, PROFILES_SP_MASK_0, reg_profiles_sp_mask_0);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_SP_MASK_0, RAM_LINE, reg_profiles_sp_mask_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_sp_mask_1_set(uint8_t tcam_id, uint8_t profile_sp_mask, uint32_t ram_line)
{
    uint32_t reg_profiles_sp_mask_1 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_sp_mask >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_sp_mask_1 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_SP_MASK_1, RAM_LINE, reg_profiles_sp_mask_1, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_sp_mask, TCAM, PROFILES_SP_MASK_1, reg_profiles_sp_mask_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_sp_mask_1_get(uint8_t tcam_id, uint8_t profile_sp_mask, uint32_t *ram_line)
{
    uint32_t reg_profiles_sp_mask_1;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_sp_mask >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_sp_mask, TCAM, PROFILES_SP_MASK_1, reg_profiles_sp_mask_1);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_SP_MASK_1, RAM_LINE, reg_profiles_sp_mask_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_sp_mask_2_set(uint8_t tcam_id, uint8_t profile_sp_mask, uint32_t ram_line)
{
    uint32_t reg_profiles_sp_mask_2 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_sp_mask >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_sp_mask_2 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_SP_MASK_2, RAM_LINE, reg_profiles_sp_mask_2, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_sp_mask, TCAM, PROFILES_SP_MASK_2, reg_profiles_sp_mask_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_sp_mask_2_get(uint8_t tcam_id, uint8_t profile_sp_mask, uint32_t *ram_line)
{
    uint32_t reg_profiles_sp_mask_2;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_sp_mask >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_sp_mask, TCAM, PROFILES_SP_MASK_2, reg_profiles_sp_mask_2);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_SP_MASK_2, RAM_LINE, reg_profiles_sp_mask_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_sp_mask_3_set(uint8_t tcam_id, uint8_t profile_sp_mask, uint32_t ram_line)
{
    uint32_t reg_profiles_sp_mask_3 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_sp_mask >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_sp_mask_3 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_SP_MASK_3, RAM_LINE, reg_profiles_sp_mask_3, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_sp_mask, TCAM, PROFILES_SP_MASK_3, reg_profiles_sp_mask_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_sp_mask_3_get(uint8_t tcam_id, uint8_t profile_sp_mask, uint32_t *ram_line)
{
    uint32_t reg_profiles_sp_mask_3;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_sp_mask >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_sp_mask, TCAM, PROFILES_SP_MASK_3, reg_profiles_sp_mask_3);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_SP_MASK_3, RAM_LINE, reg_profiles_sp_mask_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_rslt_mux_0_set(uint8_t tcam_id, uint8_t profile_rslt_mux, uint32_t ram_line)
{
    uint32_t reg_profiles_rslt_mux_0 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_rslt_mux >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_rslt_mux_0 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_RSLT_MUX_0, RAM_LINE, reg_profiles_rslt_mux_0, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_rslt_mux, TCAM, PROFILES_RSLT_MUX_0, reg_profiles_rslt_mux_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_rslt_mux_0_get(uint8_t tcam_id, uint8_t profile_rslt_mux, uint32_t *ram_line)
{
    uint32_t reg_profiles_rslt_mux_0;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_rslt_mux >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_rslt_mux, TCAM, PROFILES_RSLT_MUX_0, reg_profiles_rslt_mux_0);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_RSLT_MUX_0, RAM_LINE, reg_profiles_rslt_mux_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_rslt_mux_1_set(uint8_t tcam_id, uint8_t profile_rslt_mux, uint32_t ram_line)
{
    uint32_t reg_profiles_rslt_mux_1 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_rslt_mux >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_rslt_mux_1 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_RSLT_MUX_1, RAM_LINE, reg_profiles_rslt_mux_1, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_rslt_mux, TCAM, PROFILES_RSLT_MUX_1, reg_profiles_rslt_mux_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_rslt_mux_1_get(uint8_t tcam_id, uint8_t profile_rslt_mux, uint32_t *ram_line)
{
    uint32_t reg_profiles_rslt_mux_1;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_rslt_mux >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_rslt_mux, TCAM, PROFILES_RSLT_MUX_1, reg_profiles_rslt_mux_1);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_RSLT_MUX_1, RAM_LINE, reg_profiles_rslt_mux_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_rslt_mux_2_set(uint8_t tcam_id, uint8_t profile_rslt_mux, uint32_t ram_line)
{
    uint32_t reg_profiles_rslt_mux_2 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_rslt_mux >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_rslt_mux_2 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_RSLT_MUX_2, RAM_LINE, reg_profiles_rslt_mux_2, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_rslt_mux, TCAM, PROFILES_RSLT_MUX_2, reg_profiles_rslt_mux_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_rslt_mux_2_get(uint8_t tcam_id, uint8_t profile_rslt_mux, uint32_t *ram_line)
{
    uint32_t reg_profiles_rslt_mux_2;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_rslt_mux >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_rslt_mux, TCAM, PROFILES_RSLT_MUX_2, reg_profiles_rslt_mux_2);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_RSLT_MUX_2, RAM_LINE, reg_profiles_rslt_mux_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_rslt_mux_3_set(uint8_t tcam_id, uint8_t profile_rslt_mux, uint32_t ram_line)
{
    uint32_t reg_profiles_rslt_mux_3 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_rslt_mux >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_rslt_mux_3 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_RSLT_MUX_3, RAM_LINE, reg_profiles_rslt_mux_3, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_rslt_mux, TCAM, PROFILES_RSLT_MUX_3, reg_profiles_rslt_mux_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_rslt_mux_3_get(uint8_t tcam_id, uint8_t profile_rslt_mux, uint32_t *ram_line)
{
    uint32_t reg_profiles_rslt_mux_3;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_rslt_mux >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_rslt_mux, TCAM, PROFILES_RSLT_MUX_3, reg_profiles_rslt_mux_3);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_RSLT_MUX_3, RAM_LINE, reg_profiles_rslt_mux_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_rslt_builder_0_set(uint8_t tcam_id, uint8_t profile_rslt_builder, uint32_t ram_line)
{
    uint32_t reg_profiles_rslt_builder_0 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_rslt_builder >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_rslt_builder_0 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_RSLT_BUILDER_0, RAM_LINE, reg_profiles_rslt_builder_0, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_rslt_builder, TCAM, PROFILES_RSLT_BUILDER_0, reg_profiles_rslt_builder_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_rslt_builder_0_get(uint8_t tcam_id, uint8_t profile_rslt_builder, uint32_t *ram_line)
{
    uint32_t reg_profiles_rslt_builder_0;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_rslt_builder >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_rslt_builder, TCAM, PROFILES_RSLT_BUILDER_0, reg_profiles_rslt_builder_0);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_RSLT_BUILDER_0, RAM_LINE, reg_profiles_rslt_builder_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_rslt_builder_1_set(uint8_t tcam_id, uint8_t profile_rslt_builder, uint32_t ram_line)
{
    uint32_t reg_profiles_rslt_builder_1 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_rslt_builder >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_rslt_builder_1 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_RSLT_BUILDER_1, RAM_LINE, reg_profiles_rslt_builder_1, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_rslt_builder, TCAM, PROFILES_RSLT_BUILDER_1, reg_profiles_rslt_builder_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_rslt_builder_1_get(uint8_t tcam_id, uint8_t profile_rslt_builder, uint32_t *ram_line)
{
    uint32_t reg_profiles_rslt_builder_1;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_rslt_builder >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_rslt_builder, TCAM, PROFILES_RSLT_BUILDER_1, reg_profiles_rslt_builder_1);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_RSLT_BUILDER_1, RAM_LINE, reg_profiles_rslt_builder_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_rslt_builder_2_set(uint8_t tcam_id, uint8_t profile_rslt_builder, uint32_t ram_line)
{
    uint32_t reg_profiles_rslt_builder_2 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_rslt_builder >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_rslt_builder_2 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_RSLT_BUILDER_2, RAM_LINE, reg_profiles_rslt_builder_2, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_rslt_builder, TCAM, PROFILES_RSLT_BUILDER_2, reg_profiles_rslt_builder_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_rslt_builder_2_get(uint8_t tcam_id, uint8_t profile_rslt_builder, uint32_t *ram_line)
{
    uint32_t reg_profiles_rslt_builder_2;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_rslt_builder >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_rslt_builder, TCAM, PROFILES_RSLT_BUILDER_2, reg_profiles_rslt_builder_2);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_RSLT_BUILDER_2, RAM_LINE, reg_profiles_rslt_builder_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_rslt_builder_3_set(uint8_t tcam_id, uint8_t profile_rslt_builder, uint32_t ram_line)
{
    uint32_t reg_profiles_rslt_builder_3 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_rslt_builder >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_rslt_builder_3 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_RSLT_BUILDER_3, RAM_LINE, reg_profiles_rslt_builder_3, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_rslt_builder, TCAM, PROFILES_RSLT_BUILDER_3, reg_profiles_rslt_builder_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_rslt_builder_3_get(uint8_t tcam_id, uint8_t profile_rslt_builder, uint32_t *ram_line)
{
    uint32_t reg_profiles_rslt_builder_3;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_rslt_builder >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_rslt_builder, TCAM, PROFILES_RSLT_BUILDER_3, reg_profiles_rslt_builder_3);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_RSLT_BUILDER_3, RAM_LINE, reg_profiles_rslt_builder_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_rslt_builder_4_set(uint8_t tcam_id, uint8_t profile_rslt_builder, uint32_t ram_line)
{
    uint32_t reg_profiles_rslt_builder_4 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_rslt_builder >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_rslt_builder_4 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_RSLT_BUILDER_4, RAM_LINE, reg_profiles_rslt_builder_4, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_rslt_builder, TCAM, PROFILES_RSLT_BUILDER_4, reg_profiles_rslt_builder_4);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_rslt_builder_4_get(uint8_t tcam_id, uint8_t profile_rslt_builder, uint32_t *ram_line)
{
    uint32_t reg_profiles_rslt_builder_4;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_rslt_builder >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_rslt_builder, TCAM, PROFILES_RSLT_BUILDER_4, reg_profiles_rslt_builder_4);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_RSLT_BUILDER_4, RAM_LINE, reg_profiles_rslt_builder_4);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_rslt_builder_5_set(uint8_t tcam_id, uint8_t profile_rslt_builder, uint32_t ram_line)
{
    uint32_t reg_profiles_rslt_builder_5 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_rslt_builder >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_rslt_builder_5 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_RSLT_BUILDER_5, RAM_LINE, reg_profiles_rslt_builder_5, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_rslt_builder, TCAM, PROFILES_RSLT_BUILDER_5, reg_profiles_rslt_builder_5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_rslt_builder_5_get(uint8_t tcam_id, uint8_t profile_rslt_builder, uint32_t *ram_line)
{
    uint32_t reg_profiles_rslt_builder_5;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_rslt_builder >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_rslt_builder, TCAM, PROFILES_RSLT_BUILDER_5, reg_profiles_rslt_builder_5);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_RSLT_BUILDER_5, RAM_LINE, reg_profiles_rslt_builder_5);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_rslt_builder_6_set(uint8_t tcam_id, uint8_t profile_rslt_builder, uint32_t ram_line)
{
    uint32_t reg_profiles_rslt_builder_6 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_rslt_builder >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_rslt_builder_6 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_RSLT_BUILDER_6, RAM_LINE, reg_profiles_rslt_builder_6, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_rslt_builder, TCAM, PROFILES_RSLT_BUILDER_6, reg_profiles_rslt_builder_6);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_rslt_builder_6_get(uint8_t tcam_id, uint8_t profile_rslt_builder, uint32_t *ram_line)
{
    uint32_t reg_profiles_rslt_builder_6;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_rslt_builder >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_rslt_builder, TCAM, PROFILES_RSLT_BUILDER_6, reg_profiles_rslt_builder_6);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_RSLT_BUILDER_6, RAM_LINE, reg_profiles_rslt_builder_6);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_rslt_builder_7_set(uint8_t tcam_id, uint8_t profile_rslt_builder, uint32_t ram_line)
{
    uint32_t reg_profiles_rslt_builder_7 = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_rslt_builder >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_rslt_builder_7 = RU_FIELD_SET(tcam_id, TCAM, PROFILES_RSLT_BUILDER_7, RAM_LINE, reg_profiles_rslt_builder_7, ram_line);

    RU_REG_RAM_WRITE(tcam_id, profile_rslt_builder, TCAM, PROFILES_RSLT_BUILDER_7, reg_profiles_rslt_builder_7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_rslt_builder_7_get(uint8_t tcam_id, uint8_t profile_rslt_builder, uint32_t *ram_line)
{
    uint32_t reg_profiles_rslt_builder_7;

#ifdef VALIDATE_PARMS
    if (!ram_line)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_rslt_builder >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_rslt_builder, TCAM, PROFILES_RSLT_BUILDER_7, reg_profiles_rslt_builder_7);

    *ram_line = RU_FIELD_GET(tcam_id, TCAM, PROFILES_RSLT_BUILDER_7, RAM_LINE, reg_profiles_rslt_builder_7);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_tcam_concat_set(uint8_t tcam_id, uint8_t profile_tcam_concat, uint16_t en)
{
    uint32_t reg_profiles_tcam_concat = 0;

#ifdef VALIDATE_PARMS
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_tcam_concat >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_profiles_tcam_concat = RU_FIELD_SET(tcam_id, TCAM, PROFILES_TCAM_CONCAT, EN, reg_profiles_tcam_concat, en);

    RU_REG_RAM_WRITE(tcam_id, profile_tcam_concat, TCAM, PROFILES_TCAM_CONCAT, reg_profiles_tcam_concat);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tcam_profiles_tcam_concat_get(uint8_t tcam_id, uint8_t profile_tcam_concat, uint16_t *en)
{
    uint32_t reg_profiles_tcam_concat;

#ifdef VALIDATE_PARMS
    if (!en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tcam_id >= BLOCK_ADDR_COUNT) ||
       (profile_tcam_concat >= 4))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tcam_id, profile_tcam_concat, TCAM, PROFILES_TCAM_CONCAT, reg_profiles_tcam_concat);

    *en = RU_FIELD_GET(tcam_id, TCAM, PROFILES_TCAM_CONCAT, EN, reg_profiles_tcam_concat);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL

typedef enum
{
    bdmf_address_cfg_dinm,
    bdmf_address_cfg_blksel,
    bdmf_address_cfg_tm,
    bdmf_address_cfg_vreset,
    bdmf_address_counters_searches,
    bdmf_address_counters_hits,
    bdmf_address_tcam_mem_indirect_op,
    bdmf_address_tcam_mem_indirect_op_done,
    bdmf_address_tcam_mem_indirect_addr,
    bdmf_address_tcam_mem_indirect_valid_in,
    bdmf_address_tcam_mem_indirect_valid_out,
    bdmf_address_tcam_mem_indirect_data_out,
    bdmf_address_tcam_mem_indirect_data_in,
    bdmf_address_tcam_debug_bus_select,
    bdmf_address_profiles_key_builder_offsets_0,
    bdmf_address_profiles_key_builder_offsets_1,
    bdmf_address_profiles_key_builder_offsets_2,
    bdmf_address_profiles_key_builder_offsets_3,
    bdmf_address_profiles_key_builder_offsets_4,
    bdmf_address_profiles_key_builder_offsets_5,
    bdmf_address_profiles_key_builder_offsets_6,
    bdmf_address_profiles_key_builder_offsets_7,
    bdmf_address_profiles_key_builder_offsets_8,
    bdmf_address_profiles_key_builder_offsets_9,
    bdmf_address_profiles_key_builder_offsets_10,
    bdmf_address_profiles_key_builder_offsets_11,
    bdmf_address_profiles_key_builder_offsets_12,
    bdmf_address_profiles_key_builder_offsets_13,
    bdmf_address_profiles_key_builder_offsets_14,
    bdmf_address_profiles_key_builder_offsets_15,
    bdmf_address_profiles_key_builder_offsets_16,
    bdmf_address_profiles_key_builder_offsets_17,
    bdmf_address_profiles_key_builder_offsets_18,
    bdmf_address_profiles_key_builder_offsets_19,
    bdmf_address_profiles_key_builder_offsets_20,
    bdmf_address_profiles_key_builder_offsets_21,
    bdmf_address_profiles_key_builder_offsets_22,
    bdmf_address_profiles_key_builder_offsets_23,
    bdmf_address_profiles_key_builder_offsets_24,
    bdmf_address_profiles_key_builder_offsets_25,
    bdmf_address_profiles_key_builder_offsets_26,
    bdmf_address_profiles_key_builder_offsets_27,
    bdmf_address_profiles_key_builder_offsets_28,
    bdmf_address_profiles_key_builder_offsets_29,
    bdmf_address_profiles_key_builder_offsets_30,
    bdmf_address_profiles_key_builder_offsets_31,
    bdmf_address_profiles_key_builder_offsets_32,
    bdmf_address_profiles_key_builder_offsets_33,
    bdmf_address_profiles_key_builder_offsets_34,
    bdmf_address_profiles_key_builder_offsets_35,
    bdmf_address_profiles_key_builder_offsets_36,
    bdmf_address_profiles_key_builder_offsets_37,
    bdmf_address_profiles_key_builder_offsets_38,
    bdmf_address_profiles_key_builder_offsets_39,
    bdmf_address_profiles_inner_shift_0,
    bdmf_address_profiles_inner_shift_1,
    bdmf_address_profiles_inner_shift_2,
    bdmf_address_profiles_inner_shift_3,
    bdmf_address_profiles_inner_shift_4,
    bdmf_address_profiles_inner_shift_5,
    bdmf_address_profiles_inner_shift_6,
    bdmf_address_profiles_inner_shift_7,
    bdmf_address_profiles_inner_shift_8,
    bdmf_address_profiles_inner_shift_9,
    bdmf_address_profiles_inner_shift_10,
    bdmf_address_profiles_inner_shift_11,
    bdmf_address_profiles_inner_shift_12,
    bdmf_address_profiles_inner_shift_13,
    bdmf_address_profiles_inner_shift_14,
    bdmf_address_profiles_inner_shift_15,
    bdmf_address_profiles_inner_shift_16,
    bdmf_address_profiles_inner_shift_17,
    bdmf_address_profiles_inner_shift_18,
    bdmf_address_profiles_inner_shift_19,
    bdmf_address_profiles_inner_shift_20,
    bdmf_address_profiles_inner_shift_21,
    bdmf_address_profiles_inner_shift_22,
    bdmf_address_profiles_inner_shift_23,
    bdmf_address_profiles_inner_shift_24,
    bdmf_address_profiles_inner_shift_25,
    bdmf_address_profiles_inner_shift_26,
    bdmf_address_profiles_inner_shift_27,
    bdmf_address_profiles_inner_shift_28,
    bdmf_address_profiles_inner_shift_29,
    bdmf_address_profiles_inner_shift_30,
    bdmf_address_profiles_inner_shift_31,
    bdmf_address_profiles_cntxin_sp_id_0,
    bdmf_address_profiles_cntxin_sp_id_1,
    bdmf_address_profiles_cntxin_mask_0,
    bdmf_address_profiles_cntxin_mask_1,
    bdmf_address_profiles_cntxin_mask_2,
    bdmf_address_profiles_cntxin_mask_3,
    bdmf_address_profiles_cntxin_mask_4,
    bdmf_address_profiles_cntxin_mask_5,
    bdmf_address_profiles_cntxin_mask_6,
    bdmf_address_profiles_cntxin_mask_7,
    bdmf_address_profiles_sp_mask_0,
    bdmf_address_profiles_sp_mask_1,
    bdmf_address_profiles_sp_mask_2,
    bdmf_address_profiles_sp_mask_3,
    bdmf_address_profiles_rslt_mux_0,
    bdmf_address_profiles_rslt_mux_1,
    bdmf_address_profiles_rslt_mux_2,
    bdmf_address_profiles_rslt_mux_3,
    bdmf_address_profiles_rslt_builder_0,
    bdmf_address_profiles_rslt_builder_1,
    bdmf_address_profiles_rslt_builder_2,
    bdmf_address_profiles_rslt_builder_3,
    bdmf_address_profiles_rslt_builder_4,
    bdmf_address_profiles_rslt_builder_5,
    bdmf_address_profiles_rslt_builder_6,
    bdmf_address_profiles_rslt_builder_7,
    bdmf_address_profiles_tcam_concat,
}
bdmf_address;

static int ag_drv_tcam_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_tcam_cfg_dinm:
        ag_err = ag_drv_tcam_cfg_dinm_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_cfg_blksel:
        ag_err = ag_drv_tcam_cfg_blksel_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_cfg_tm:
        ag_err = ag_drv_tcam_cfg_tm_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_cfg_vreset:
        ag_err = ag_drv_tcam_cfg_vreset_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_tcam_tcam_mem_indirect_op:
        ag_err = ag_drv_tcam_tcam_mem_indirect_op_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_tcam_tcam_mem_indirect_addr:
        ag_err = ag_drv_tcam_tcam_mem_indirect_addr_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_tcam_mem_indirect_valid_in:
        ag_err = ag_drv_tcam_tcam_mem_indirect_valid_in_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_tcam_tcam_mem_indirect_data_in:
        ag_err = ag_drv_tcam_tcam_mem_indirect_data_in_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_tcam_debug_bus_select:
        ag_err = ag_drv_tcam_tcam_debug_bus_select_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_0:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_0_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_1:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_2:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_2_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_3:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_3_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_4:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_4_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_5:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_5_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_6:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_6_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_7:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_7_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_8:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_8_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_9:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_9_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_10:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_10_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_11:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_11_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_12:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_12_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_13:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_13_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_14:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_14_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_15:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_15_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_16:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_16_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_17:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_17_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_18:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_18_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_19:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_19_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_20:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_20_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_21:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_21_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_22:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_22_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_23:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_23_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_24:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_24_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_25:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_25_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_26:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_26_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_27:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_27_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_28:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_28_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_29:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_29_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_30:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_30_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_31:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_31_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_32:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_32_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_33:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_33_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_34:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_34_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_35:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_35_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_36:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_36_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_37:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_37_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_38:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_38_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_key_builder_offsets_39:
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_39_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_inner_shift_0:
        ag_err = ag_drv_tcam_profiles_inner_shift_0_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_inner_shift_1:
        ag_err = ag_drv_tcam_profiles_inner_shift_1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_inner_shift_2:
        ag_err = ag_drv_tcam_profiles_inner_shift_2_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_inner_shift_3:
        ag_err = ag_drv_tcam_profiles_inner_shift_3_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_inner_shift_4:
        ag_err = ag_drv_tcam_profiles_inner_shift_4_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_inner_shift_5:
        ag_err = ag_drv_tcam_profiles_inner_shift_5_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_inner_shift_6:
        ag_err = ag_drv_tcam_profiles_inner_shift_6_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_inner_shift_7:
        ag_err = ag_drv_tcam_profiles_inner_shift_7_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_inner_shift_8:
        ag_err = ag_drv_tcam_profiles_inner_shift_8_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_inner_shift_9:
        ag_err = ag_drv_tcam_profiles_inner_shift_9_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_inner_shift_10:
        ag_err = ag_drv_tcam_profiles_inner_shift_10_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_inner_shift_11:
        ag_err = ag_drv_tcam_profiles_inner_shift_11_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_inner_shift_12:
        ag_err = ag_drv_tcam_profiles_inner_shift_12_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_inner_shift_13:
        ag_err = ag_drv_tcam_profiles_inner_shift_13_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_inner_shift_14:
        ag_err = ag_drv_tcam_profiles_inner_shift_14_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_inner_shift_15:
        ag_err = ag_drv_tcam_profiles_inner_shift_15_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_inner_shift_16:
        ag_err = ag_drv_tcam_profiles_inner_shift_16_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_inner_shift_17:
        ag_err = ag_drv_tcam_profiles_inner_shift_17_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_inner_shift_18:
        ag_err = ag_drv_tcam_profiles_inner_shift_18_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_inner_shift_19:
        ag_err = ag_drv_tcam_profiles_inner_shift_19_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_inner_shift_20:
        ag_err = ag_drv_tcam_profiles_inner_shift_20_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_inner_shift_21:
        ag_err = ag_drv_tcam_profiles_inner_shift_21_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_inner_shift_22:
        ag_err = ag_drv_tcam_profiles_inner_shift_22_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_inner_shift_23:
        ag_err = ag_drv_tcam_profiles_inner_shift_23_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_inner_shift_24:
        ag_err = ag_drv_tcam_profiles_inner_shift_24_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_inner_shift_25:
        ag_err = ag_drv_tcam_profiles_inner_shift_25_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_inner_shift_26:
        ag_err = ag_drv_tcam_profiles_inner_shift_26_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_inner_shift_27:
        ag_err = ag_drv_tcam_profiles_inner_shift_27_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_inner_shift_28:
        ag_err = ag_drv_tcam_profiles_inner_shift_28_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_inner_shift_29:
        ag_err = ag_drv_tcam_profiles_inner_shift_29_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_inner_shift_30:
        ag_err = ag_drv_tcam_profiles_inner_shift_30_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_inner_shift_31:
        ag_err = ag_drv_tcam_profiles_inner_shift_31_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_cntxin_sp_id_0:
        ag_err = ag_drv_tcam_profiles_cntxin_sp_id_0_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_cntxin_sp_id_1:
        ag_err = ag_drv_tcam_profiles_cntxin_sp_id_1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_cntxin_mask_0:
        ag_err = ag_drv_tcam_profiles_cntxin_mask_0_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_cntxin_mask_1:
        ag_err = ag_drv_tcam_profiles_cntxin_mask_1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_cntxin_mask_2:
        ag_err = ag_drv_tcam_profiles_cntxin_mask_2_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_cntxin_mask_3:
        ag_err = ag_drv_tcam_profiles_cntxin_mask_3_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_cntxin_mask_4:
        ag_err = ag_drv_tcam_profiles_cntxin_mask_4_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_cntxin_mask_5:
        ag_err = ag_drv_tcam_profiles_cntxin_mask_5_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_cntxin_mask_6:
        ag_err = ag_drv_tcam_profiles_cntxin_mask_6_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_cntxin_mask_7:
        ag_err = ag_drv_tcam_profiles_cntxin_mask_7_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_sp_mask_0:
        ag_err = ag_drv_tcam_profiles_sp_mask_0_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_sp_mask_1:
        ag_err = ag_drv_tcam_profiles_sp_mask_1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_sp_mask_2:
        ag_err = ag_drv_tcam_profiles_sp_mask_2_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_sp_mask_3:
        ag_err = ag_drv_tcam_profiles_sp_mask_3_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_rslt_mux_0:
        ag_err = ag_drv_tcam_profiles_rslt_mux_0_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_rslt_mux_1:
        ag_err = ag_drv_tcam_profiles_rslt_mux_1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_rslt_mux_2:
        ag_err = ag_drv_tcam_profiles_rslt_mux_2_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_rslt_mux_3:
        ag_err = ag_drv_tcam_profiles_rslt_mux_3_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_rslt_builder_0:
        ag_err = ag_drv_tcam_profiles_rslt_builder_0_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_rslt_builder_1:
        ag_err = ag_drv_tcam_profiles_rslt_builder_1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_rslt_builder_2:
        ag_err = ag_drv_tcam_profiles_rslt_builder_2_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_rslt_builder_3:
        ag_err = ag_drv_tcam_profiles_rslt_builder_3_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_rslt_builder_4:
        ag_err = ag_drv_tcam_profiles_rslt_builder_4_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_rslt_builder_5:
        ag_err = ag_drv_tcam_profiles_rslt_builder_5_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_rslt_builder_6:
        ag_err = ag_drv_tcam_profiles_rslt_builder_6_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_rslt_builder_7:
        ag_err = ag_drv_tcam_profiles_rslt_builder_7_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_tcam_profiles_tcam_concat:
        ag_err = ag_drv_tcam_profiles_tcam_concat_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

int bcm_tcam_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_tcam_cfg_dinm:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_cfg_dinm_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_cfg_blksel:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_cfg_blksel_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_cfg_tm:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_cfg_tm_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_cfg_vreset:
    {
        uint16_t val;
        ag_err = ag_drv_tcam_cfg_vreset_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u = 0x%x\n", val, val);
        break;
    }
    case cli_tcam_counters_searches:
    {
        uint32_t cnt;
        ag_err = ag_drv_tcam_counters_searches_get(parm[1].value.unumber, parm[2].value.unumber, &cnt);
        bdmf_session_print(session, "cnt = %u = 0x%x\n", cnt, cnt);
        break;
    }
    case cli_tcam_counters_hits:
    {
        uint32_t cnt;
        ag_err = ag_drv_tcam_counters_hits_get(parm[1].value.unumber, parm[2].value.unumber, &cnt);
        bdmf_session_print(session, "cnt = %u = 0x%x\n", cnt, cnt);
        break;
    }
    case cli_tcam_tcam_mem_indirect_op:
    {
        uint8_t cmd;
        ag_err = ag_drv_tcam_tcam_mem_indirect_op_get(parm[1].value.unumber, &cmd);
        bdmf_session_print(session, "cmd = %u = 0x%x\n", cmd, cmd);
        break;
    }
    case cli_tcam_tcam_mem_indirect_op_done:
    {
        bdmf_boolean done;
        ag_err = ag_drv_tcam_tcam_mem_indirect_op_done_get(parm[1].value.unumber, &done);
        bdmf_session_print(session, "done = %u = 0x%x\n", done, done);
        break;
    }
    case cli_tcam_tcam_mem_indirect_addr:
    {
        uint16_t entry_addr;
        uint8_t mem_id;
        ag_err = ag_drv_tcam_tcam_mem_indirect_addr_get(parm[1].value.unumber, &entry_addr, &mem_id);
        bdmf_session_print(session, "entry_addr = %u = 0x%x\n", entry_addr, entry_addr);
        bdmf_session_print(session, "mem_id = %u = 0x%x\n", mem_id, mem_id);
        break;
    }
    case cli_tcam_tcam_mem_indirect_valid_in:
    {
        uint8_t valid;
        ag_err = ag_drv_tcam_tcam_mem_indirect_valid_in_get(parm[1].value.unumber, &valid);
        bdmf_session_print(session, "valid = %u = 0x%x\n", valid, valid);
        break;
    }
    case cli_tcam_tcam_mem_indirect_valid_out:
    {
        uint8_t valid;
        ag_err = ag_drv_tcam_tcam_mem_indirect_valid_out_get(parm[1].value.unumber, &valid);
        bdmf_session_print(session, "valid = %u = 0x%x\n", valid, valid);
        break;
    }
    case cli_tcam_tcam_mem_indirect_data_out:
    {
        uint32_t value;
        ag_err = ag_drv_tcam_tcam_mem_indirect_data_out_get(parm[1].value.unumber, parm[2].value.unumber, &value);
        bdmf_session_print(session, "value = %u = 0x%x\n", value, value);
        break;
    }
    case cli_tcam_tcam_mem_indirect_data_in:
    {
        uint32_t value;
        ag_err = ag_drv_tcam_tcam_mem_indirect_data_in_get(parm[1].value.unumber, parm[2].value.unumber, &value);
        bdmf_session_print(session, "value = %u = 0x%x\n", value, value);
        break;
    }
    case cli_tcam_tcam_debug_bus_select:
    {
        uint8_t select_module;
        ag_err = ag_drv_tcam_tcam_debug_bus_select_get(parm[1].value.unumber, &select_module);
        bdmf_session_print(session, "select_module = %u = 0x%x\n", select_module, select_module);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_0:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_0_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_1:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_1_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_2:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_2_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_3:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_3_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_4:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_4_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_5:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_5_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_6:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_6_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_7:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_7_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_8:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_8_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_9:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_9_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_10:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_10_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_11:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_11_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_12:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_12_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_13:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_13_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_14:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_14_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_15:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_15_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_16:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_16_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_17:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_17_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_18:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_18_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_19:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_19_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_20:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_20_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_21:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_21_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_22:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_22_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_23:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_23_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_24:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_24_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_25:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_25_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_26:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_26_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_27:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_27_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_28:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_28_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_29:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_29_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_30:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_30_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_31:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_31_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_32:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_32_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_33:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_33_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_34:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_34_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_35:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_35_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_36:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_36_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_37:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_37_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_38:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_38_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_key_builder_offsets_39:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_39_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_inner_shift_0:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_inner_shift_0_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_inner_shift_1:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_inner_shift_1_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_inner_shift_2:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_inner_shift_2_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_inner_shift_3:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_inner_shift_3_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_inner_shift_4:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_inner_shift_4_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_inner_shift_5:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_inner_shift_5_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_inner_shift_6:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_inner_shift_6_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_inner_shift_7:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_inner_shift_7_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_inner_shift_8:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_inner_shift_8_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_inner_shift_9:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_inner_shift_9_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_inner_shift_10:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_inner_shift_10_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_inner_shift_11:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_inner_shift_11_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_inner_shift_12:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_inner_shift_12_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_inner_shift_13:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_inner_shift_13_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_inner_shift_14:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_inner_shift_14_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_inner_shift_15:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_inner_shift_15_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_inner_shift_16:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_inner_shift_16_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_inner_shift_17:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_inner_shift_17_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_inner_shift_18:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_inner_shift_18_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_inner_shift_19:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_inner_shift_19_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_inner_shift_20:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_inner_shift_20_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_inner_shift_21:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_inner_shift_21_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_inner_shift_22:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_inner_shift_22_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_inner_shift_23:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_inner_shift_23_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_inner_shift_24:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_inner_shift_24_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_inner_shift_25:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_inner_shift_25_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_inner_shift_26:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_inner_shift_26_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_inner_shift_27:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_inner_shift_27_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_inner_shift_28:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_inner_shift_28_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_inner_shift_29:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_inner_shift_29_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_inner_shift_30:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_inner_shift_30_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_inner_shift_31:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_inner_shift_31_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_cntxin_sp_id_0:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_cntxin_sp_id_0_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_cntxin_sp_id_1:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_cntxin_sp_id_1_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_cntxin_mask_0:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_cntxin_mask_0_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_cntxin_mask_1:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_cntxin_mask_1_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_cntxin_mask_2:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_cntxin_mask_2_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_cntxin_mask_3:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_cntxin_mask_3_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_cntxin_mask_4:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_cntxin_mask_4_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_cntxin_mask_5:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_cntxin_mask_5_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_cntxin_mask_6:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_cntxin_mask_6_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_cntxin_mask_7:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_cntxin_mask_7_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_sp_mask_0:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_sp_mask_0_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_sp_mask_1:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_sp_mask_1_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_sp_mask_2:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_sp_mask_2_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_sp_mask_3:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_sp_mask_3_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_rslt_mux_0:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_rslt_mux_0_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_rslt_mux_1:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_rslt_mux_1_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_rslt_mux_2:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_rslt_mux_2_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_rslt_mux_3:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_rslt_mux_3_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_rslt_builder_0:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_rslt_builder_0_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_rslt_builder_1:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_rslt_builder_1_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_rslt_builder_2:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_rslt_builder_2_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_rslt_builder_3:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_rslt_builder_3_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_rslt_builder_4:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_rslt_builder_4_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_rslt_builder_5:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_rslt_builder_5_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_rslt_builder_6:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_rslt_builder_6_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_rslt_builder_7:
    {
        uint32_t ram_line;
        ag_err = ag_drv_tcam_profiles_rslt_builder_7_get(parm[1].value.unumber, parm[2].value.unumber, &ram_line);
        bdmf_session_print(session, "ram_line = %u = 0x%x\n", ram_line, ram_line);
        break;
    }
    case cli_tcam_profiles_tcam_concat:
    {
        uint16_t en;
        ag_err = ag_drv_tcam_profiles_tcam_concat_get(parm[1].value.unumber, parm[2].value.unumber, &en);
        bdmf_session_print(session, "en = %u = 0x%x\n", en, en);
        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_tcam_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    uint8_t tcam_id = parm[1].value.unumber;
    bdmf_error_t ag_err = BDMF_ERR_OK;
    uint32_t test_success_cnt = 0;
    uint32_t test_failure_cnt = 0;

    {
        uint32_t cfg_dinm = gtmv(m, 6);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_cfg_dinm_set(%u %u %u)\n", tcam_id, cfg_dinm,
            ram_line);
        ag_err = ag_drv_tcam_cfg_dinm_set(tcam_id, cfg_dinm, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_cfg_dinm_get(tcam_id, cfg_dinm, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_cfg_dinm_get(%u %u %u)\n", tcam_id, cfg_dinm,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t cfg_blksel = gtmv(m, 1);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_cfg_blksel_set(%u %u %u)\n", tcam_id, cfg_blksel,
            ram_line);
        ag_err = ag_drv_tcam_cfg_blksel_set(tcam_id, cfg_blksel, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_cfg_blksel_get(tcam_id, cfg_blksel, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_cfg_blksel_get(%u %u %u)\n", tcam_id, cfg_blksel,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t cfg_tm = gtmv(m, 3);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_cfg_tm_set(%u %u %u)\n", tcam_id, cfg_tm,
            ram_line);
        ag_err = ag_drv_tcam_cfg_tm_set(tcam_id, cfg_tm, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_cfg_tm_get(tcam_id, cfg_tm, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_cfg_tm_get(%u %u %u)\n", tcam_id, cfg_tm,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t val = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_tcam_cfg_vreset_set(%u %u)\n", tcam_id,
            val);
        ag_err = ag_drv_tcam_cfg_vreset_set(tcam_id, val);
        if (!ag_err)
            ag_err = ag_drv_tcam_cfg_vreset_get(tcam_id, &val);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_cfg_vreset_get(%u %u)\n", tcam_id,
                val);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (val != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t counters_searches = gtmv(m, 2);
        uint32_t cnt = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_tcam_counters_searches_get(tcam_id, counters_searches, &cnt);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_counters_searches_get(%u %u %u)\n", tcam_id, counters_searches,
                cnt);
        }
    }

    {
        uint8_t tcam_idx = gtmv(m, 2);
        uint32_t cnt = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_tcam_counters_hits_get(tcam_id, tcam_idx, &cnt);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_counters_hits_get(%u %u %u)\n", tcam_id, tcam_idx,
                cnt);
        }
    }

    {
        uint8_t cmd = gtmv(m, 4);
        bdmf_session_print(session, "ag_drv_tcam_tcam_mem_indirect_op_set(%u %u)\n", tcam_id,
            cmd);
        ag_err = ag_drv_tcam_tcam_mem_indirect_op_set(tcam_id, cmd);
        if (!ag_err)
            ag_err = ag_drv_tcam_tcam_mem_indirect_op_get(tcam_id, &cmd);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_tcam_mem_indirect_op_get(%u %u)\n", tcam_id,
                cmd);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (cmd != gtmv(m, 4))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean done = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_tcam_tcam_mem_indirect_op_done_get(tcam_id, &done);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_tcam_mem_indirect_op_done_get(%u %u)\n", tcam_id,
                done);
        }
    }

    {
        uint16_t entry_addr = gtmv(m, 10);
        uint8_t mem_id = gtmv(m, 4);
        bdmf_session_print(session, "ag_drv_tcam_tcam_mem_indirect_addr_set(%u %u %u)\n", tcam_id,
            entry_addr, mem_id);
        ag_err = ag_drv_tcam_tcam_mem_indirect_addr_set(tcam_id, entry_addr, mem_id);
        if (!ag_err)
            ag_err = ag_drv_tcam_tcam_mem_indirect_addr_get(tcam_id, &entry_addr, &mem_id);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_tcam_mem_indirect_addr_get(%u %u %u)\n", tcam_id,
                entry_addr, mem_id);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (entry_addr != gtmv(m, 10) || mem_id != gtmv(m, 4))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t valid = gtmv(m, 2);
        bdmf_session_print(session, "ag_drv_tcam_tcam_mem_indirect_valid_in_set(%u %u)\n", tcam_id,
            valid);
        ag_err = ag_drv_tcam_tcam_mem_indirect_valid_in_set(tcam_id, valid);
        if (!ag_err)
            ag_err = ag_drv_tcam_tcam_mem_indirect_valid_in_get(tcam_id, &valid);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_tcam_mem_indirect_valid_in_get(%u %u)\n", tcam_id,
                valid);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (valid != gtmv(m, 2))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t valid = gtmv(m, 2);
        if (!ag_err)
            ag_err = ag_drv_tcam_tcam_mem_indirect_valid_out_get(tcam_id, &valid);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_tcam_mem_indirect_valid_out_get(%u %u)\n", tcam_id,
                valid);
        }
    }

    {
        uint8_t indirect_data = gtmv(m, 3);
        uint32_t value = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_tcam_tcam_mem_indirect_data_out_get(tcam_id, indirect_data, &value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_tcam_mem_indirect_data_out_get(%u %u %u)\n", tcam_id, indirect_data,
                value);
        }
    }

    {
        uint8_t indirect_data_in = gtmv(m, 4);
        uint32_t value = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_tcam_mem_indirect_data_in_set(%u %u %u)\n", tcam_id, indirect_data_in,
            value);
        ag_err = ag_drv_tcam_tcam_mem_indirect_data_in_set(tcam_id, indirect_data_in, value);
        if (!ag_err)
            ag_err = ag_drv_tcam_tcam_mem_indirect_data_in_get(tcam_id, indirect_data_in, &value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_tcam_mem_indirect_data_in_get(%u %u %u)\n", tcam_id, indirect_data_in,
                value);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (value != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t select_module = gtmv(m, 2);
        bdmf_session_print(session, "ag_drv_tcam_tcam_debug_bus_select_set(%u %u)\n", tcam_id,
            select_module);
        ag_err = ag_drv_tcam_tcam_debug_bus_select_set(tcam_id, select_module);
        if (!ag_err)
            ag_err = ag_drv_tcam_tcam_debug_bus_select_get(tcam_id, &select_module);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_tcam_debug_bus_select_get(%u %u)\n", tcam_id,
                select_module);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (select_module != gtmv(m, 2))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_0_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_0_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_0_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_0_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_1_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_1_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_1_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_1_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_2_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_2_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_2_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_2_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_3_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_3_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_3_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_3_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_4_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_4_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_4_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_4_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_5_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_5_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_5_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_5_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_6_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_6_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_6_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_6_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_7_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_7_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_7_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_7_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_8_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_8_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_8_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_8_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_9_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_9_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_9_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_9_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_10_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_10_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_10_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_10_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_11_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_11_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_11_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_11_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_12_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_12_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_12_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_12_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_13_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_13_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_13_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_13_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_14_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_14_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_14_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_14_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_15_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_15_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_15_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_15_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_16_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_16_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_16_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_16_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_17_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_17_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_17_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_17_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_18_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_18_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_18_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_18_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_19_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_19_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_19_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_19_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_20_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_20_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_20_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_20_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_21_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_21_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_21_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_21_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_22_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_22_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_22_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_22_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_23_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_23_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_23_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_23_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_24_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_24_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_24_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_24_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_25_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_25_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_25_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_25_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_26_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_26_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_26_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_26_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_27_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_27_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_27_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_27_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_28_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_28_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_28_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_28_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_29_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_29_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_29_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_29_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_30_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_30_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_30_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_30_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_31_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_31_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_31_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_31_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_32_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_32_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_32_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_32_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_33_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_33_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_33_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_33_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_34_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_34_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_34_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_34_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_35_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_35_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_35_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_35_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_36_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_36_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_36_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_36_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_37_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_37_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_37_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_37_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_38_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_38_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_38_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_38_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_39_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
            ram_line);
        ag_err = ag_drv_tcam_profiles_key_builder_offsets_39_set(tcam_id, profile_key_builder_offset, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_39_get(tcam_id, profile_key_builder_offset, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_39_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_0_set(%u %u %u)\n", tcam_id, profile_inner_shift,
            ram_line);
        ag_err = ag_drv_tcam_profiles_inner_shift_0_set(tcam_id, profile_inner_shift, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_inner_shift_0_get(tcam_id, profile_inner_shift, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_0_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_1_set(%u %u %u)\n", tcam_id, profile_inner_shift,
            ram_line);
        ag_err = ag_drv_tcam_profiles_inner_shift_1_set(tcam_id, profile_inner_shift, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_inner_shift_1_get(tcam_id, profile_inner_shift, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_1_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_2_set(%u %u %u)\n", tcam_id, profile_inner_shift,
            ram_line);
        ag_err = ag_drv_tcam_profiles_inner_shift_2_set(tcam_id, profile_inner_shift, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_inner_shift_2_get(tcam_id, profile_inner_shift, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_2_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_3_set(%u %u %u)\n", tcam_id, profile_inner_shift,
            ram_line);
        ag_err = ag_drv_tcam_profiles_inner_shift_3_set(tcam_id, profile_inner_shift, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_inner_shift_3_get(tcam_id, profile_inner_shift, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_3_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_4_set(%u %u %u)\n", tcam_id, profile_inner_shift,
            ram_line);
        ag_err = ag_drv_tcam_profiles_inner_shift_4_set(tcam_id, profile_inner_shift, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_inner_shift_4_get(tcam_id, profile_inner_shift, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_4_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_5_set(%u %u %u)\n", tcam_id, profile_inner_shift,
            ram_line);
        ag_err = ag_drv_tcam_profiles_inner_shift_5_set(tcam_id, profile_inner_shift, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_inner_shift_5_get(tcam_id, profile_inner_shift, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_5_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_6_set(%u %u %u)\n", tcam_id, profile_inner_shift,
            ram_line);
        ag_err = ag_drv_tcam_profiles_inner_shift_6_set(tcam_id, profile_inner_shift, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_inner_shift_6_get(tcam_id, profile_inner_shift, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_6_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_7_set(%u %u %u)\n", tcam_id, profile_inner_shift,
            ram_line);
        ag_err = ag_drv_tcam_profiles_inner_shift_7_set(tcam_id, profile_inner_shift, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_inner_shift_7_get(tcam_id, profile_inner_shift, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_7_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_8_set(%u %u %u)\n", tcam_id, profile_inner_shift,
            ram_line);
        ag_err = ag_drv_tcam_profiles_inner_shift_8_set(tcam_id, profile_inner_shift, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_inner_shift_8_get(tcam_id, profile_inner_shift, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_8_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_9_set(%u %u %u)\n", tcam_id, profile_inner_shift,
            ram_line);
        ag_err = ag_drv_tcam_profiles_inner_shift_9_set(tcam_id, profile_inner_shift, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_inner_shift_9_get(tcam_id, profile_inner_shift, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_9_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_10_set(%u %u %u)\n", tcam_id, profile_inner_shift,
            ram_line);
        ag_err = ag_drv_tcam_profiles_inner_shift_10_set(tcam_id, profile_inner_shift, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_inner_shift_10_get(tcam_id, profile_inner_shift, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_10_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_11_set(%u %u %u)\n", tcam_id, profile_inner_shift,
            ram_line);
        ag_err = ag_drv_tcam_profiles_inner_shift_11_set(tcam_id, profile_inner_shift, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_inner_shift_11_get(tcam_id, profile_inner_shift, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_11_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_12_set(%u %u %u)\n", tcam_id, profile_inner_shift,
            ram_line);
        ag_err = ag_drv_tcam_profiles_inner_shift_12_set(tcam_id, profile_inner_shift, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_inner_shift_12_get(tcam_id, profile_inner_shift, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_12_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_13_set(%u %u %u)\n", tcam_id, profile_inner_shift,
            ram_line);
        ag_err = ag_drv_tcam_profiles_inner_shift_13_set(tcam_id, profile_inner_shift, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_inner_shift_13_get(tcam_id, profile_inner_shift, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_13_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_14_set(%u %u %u)\n", tcam_id, profile_inner_shift,
            ram_line);
        ag_err = ag_drv_tcam_profiles_inner_shift_14_set(tcam_id, profile_inner_shift, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_inner_shift_14_get(tcam_id, profile_inner_shift, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_14_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_15_set(%u %u %u)\n", tcam_id, profile_inner_shift,
            ram_line);
        ag_err = ag_drv_tcam_profiles_inner_shift_15_set(tcam_id, profile_inner_shift, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_inner_shift_15_get(tcam_id, profile_inner_shift, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_15_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_16_set(%u %u %u)\n", tcam_id, profile_inner_shift,
            ram_line);
        ag_err = ag_drv_tcam_profiles_inner_shift_16_set(tcam_id, profile_inner_shift, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_inner_shift_16_get(tcam_id, profile_inner_shift, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_16_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_17_set(%u %u %u)\n", tcam_id, profile_inner_shift,
            ram_line);
        ag_err = ag_drv_tcam_profiles_inner_shift_17_set(tcam_id, profile_inner_shift, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_inner_shift_17_get(tcam_id, profile_inner_shift, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_17_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_18_set(%u %u %u)\n", tcam_id, profile_inner_shift,
            ram_line);
        ag_err = ag_drv_tcam_profiles_inner_shift_18_set(tcam_id, profile_inner_shift, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_inner_shift_18_get(tcam_id, profile_inner_shift, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_18_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_19_set(%u %u %u)\n", tcam_id, profile_inner_shift,
            ram_line);
        ag_err = ag_drv_tcam_profiles_inner_shift_19_set(tcam_id, profile_inner_shift, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_inner_shift_19_get(tcam_id, profile_inner_shift, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_19_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_20_set(%u %u %u)\n", tcam_id, profile_inner_shift,
            ram_line);
        ag_err = ag_drv_tcam_profiles_inner_shift_20_set(tcam_id, profile_inner_shift, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_inner_shift_20_get(tcam_id, profile_inner_shift, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_20_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_21_set(%u %u %u)\n", tcam_id, profile_inner_shift,
            ram_line);
        ag_err = ag_drv_tcam_profiles_inner_shift_21_set(tcam_id, profile_inner_shift, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_inner_shift_21_get(tcam_id, profile_inner_shift, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_21_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_22_set(%u %u %u)\n", tcam_id, profile_inner_shift,
            ram_line);
        ag_err = ag_drv_tcam_profiles_inner_shift_22_set(tcam_id, profile_inner_shift, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_inner_shift_22_get(tcam_id, profile_inner_shift, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_22_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_23_set(%u %u %u)\n", tcam_id, profile_inner_shift,
            ram_line);
        ag_err = ag_drv_tcam_profiles_inner_shift_23_set(tcam_id, profile_inner_shift, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_inner_shift_23_get(tcam_id, profile_inner_shift, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_23_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_24_set(%u %u %u)\n", tcam_id, profile_inner_shift,
            ram_line);
        ag_err = ag_drv_tcam_profiles_inner_shift_24_set(tcam_id, profile_inner_shift, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_inner_shift_24_get(tcam_id, profile_inner_shift, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_24_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_25_set(%u %u %u)\n", tcam_id, profile_inner_shift,
            ram_line);
        ag_err = ag_drv_tcam_profiles_inner_shift_25_set(tcam_id, profile_inner_shift, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_inner_shift_25_get(tcam_id, profile_inner_shift, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_25_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_26_set(%u %u %u)\n", tcam_id, profile_inner_shift,
            ram_line);
        ag_err = ag_drv_tcam_profiles_inner_shift_26_set(tcam_id, profile_inner_shift, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_inner_shift_26_get(tcam_id, profile_inner_shift, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_26_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_27_set(%u %u %u)\n", tcam_id, profile_inner_shift,
            ram_line);
        ag_err = ag_drv_tcam_profiles_inner_shift_27_set(tcam_id, profile_inner_shift, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_inner_shift_27_get(tcam_id, profile_inner_shift, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_27_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_28_set(%u %u %u)\n", tcam_id, profile_inner_shift,
            ram_line);
        ag_err = ag_drv_tcam_profiles_inner_shift_28_set(tcam_id, profile_inner_shift, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_inner_shift_28_get(tcam_id, profile_inner_shift, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_28_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_29_set(%u %u %u)\n", tcam_id, profile_inner_shift,
            ram_line);
        ag_err = ag_drv_tcam_profiles_inner_shift_29_set(tcam_id, profile_inner_shift, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_inner_shift_29_get(tcam_id, profile_inner_shift, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_29_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_30_set(%u %u %u)\n", tcam_id, profile_inner_shift,
            ram_line);
        ag_err = ag_drv_tcam_profiles_inner_shift_30_set(tcam_id, profile_inner_shift, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_inner_shift_30_get(tcam_id, profile_inner_shift, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_30_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_31_set(%u %u %u)\n", tcam_id, profile_inner_shift,
            ram_line);
        ag_err = ag_drv_tcam_profiles_inner_shift_31_set(tcam_id, profile_inner_shift, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_inner_shift_31_get(tcam_id, profile_inner_shift, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_31_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_sp_id = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_sp_id_0_set(%u %u %u)\n", tcam_id, profile_sp_id,
            ram_line);
        ag_err = ag_drv_tcam_profiles_cntxin_sp_id_0_set(tcam_id, profile_sp_id, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_cntxin_sp_id_0_get(tcam_id, profile_sp_id, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_sp_id_0_get(%u %u %u)\n", tcam_id, profile_sp_id,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_sp_id = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_sp_id_1_set(%u %u %u)\n", tcam_id, profile_sp_id,
            ram_line);
        ag_err = ag_drv_tcam_profiles_cntxin_sp_id_1_set(tcam_id, profile_sp_id, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_cntxin_sp_id_1_get(tcam_id, profile_sp_id, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_sp_id_1_get(%u %u %u)\n", tcam_id, profile_sp_id,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_cntxin_mask = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_0_set(%u %u %u)\n", tcam_id, profile_cntxin_mask,
            ram_line);
        ag_err = ag_drv_tcam_profiles_cntxin_mask_0_set(tcam_id, profile_cntxin_mask, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_cntxin_mask_0_get(tcam_id, profile_cntxin_mask, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_0_get(%u %u %u)\n", tcam_id, profile_cntxin_mask,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_cntxin_mask = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_1_set(%u %u %u)\n", tcam_id, profile_cntxin_mask,
            ram_line);
        ag_err = ag_drv_tcam_profiles_cntxin_mask_1_set(tcam_id, profile_cntxin_mask, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_cntxin_mask_1_get(tcam_id, profile_cntxin_mask, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_1_get(%u %u %u)\n", tcam_id, profile_cntxin_mask,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_cntxin_mask = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_2_set(%u %u %u)\n", tcam_id, profile_cntxin_mask,
            ram_line);
        ag_err = ag_drv_tcam_profiles_cntxin_mask_2_set(tcam_id, profile_cntxin_mask, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_cntxin_mask_2_get(tcam_id, profile_cntxin_mask, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_2_get(%u %u %u)\n", tcam_id, profile_cntxin_mask,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_cntxin_mask = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_3_set(%u %u %u)\n", tcam_id, profile_cntxin_mask,
            ram_line);
        ag_err = ag_drv_tcam_profiles_cntxin_mask_3_set(tcam_id, profile_cntxin_mask, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_cntxin_mask_3_get(tcam_id, profile_cntxin_mask, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_3_get(%u %u %u)\n", tcam_id, profile_cntxin_mask,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_cntxin_mask = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_4_set(%u %u %u)\n", tcam_id, profile_cntxin_mask,
            ram_line);
        ag_err = ag_drv_tcam_profiles_cntxin_mask_4_set(tcam_id, profile_cntxin_mask, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_cntxin_mask_4_get(tcam_id, profile_cntxin_mask, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_4_get(%u %u %u)\n", tcam_id, profile_cntxin_mask,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_cntxin_mask = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_5_set(%u %u %u)\n", tcam_id, profile_cntxin_mask,
            ram_line);
        ag_err = ag_drv_tcam_profiles_cntxin_mask_5_set(tcam_id, profile_cntxin_mask, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_cntxin_mask_5_get(tcam_id, profile_cntxin_mask, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_5_get(%u %u %u)\n", tcam_id, profile_cntxin_mask,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_cntxin_mask = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_6_set(%u %u %u)\n", tcam_id, profile_cntxin_mask,
            ram_line);
        ag_err = ag_drv_tcam_profiles_cntxin_mask_6_set(tcam_id, profile_cntxin_mask, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_cntxin_mask_6_get(tcam_id, profile_cntxin_mask, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_6_get(%u %u %u)\n", tcam_id, profile_cntxin_mask,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_cntxin_mask = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_7_set(%u %u %u)\n", tcam_id, profile_cntxin_mask,
            ram_line);
        ag_err = ag_drv_tcam_profiles_cntxin_mask_7_set(tcam_id, profile_cntxin_mask, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_cntxin_mask_7_get(tcam_id, profile_cntxin_mask, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_7_get(%u %u %u)\n", tcam_id, profile_cntxin_mask,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_sp_mask = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_sp_mask_0_set(%u %u %u)\n", tcam_id, profile_sp_mask,
            ram_line);
        ag_err = ag_drv_tcam_profiles_sp_mask_0_set(tcam_id, profile_sp_mask, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_sp_mask_0_get(tcam_id, profile_sp_mask, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_sp_mask_0_get(%u %u %u)\n", tcam_id, profile_sp_mask,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_sp_mask = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_sp_mask_1_set(%u %u %u)\n", tcam_id, profile_sp_mask,
            ram_line);
        ag_err = ag_drv_tcam_profiles_sp_mask_1_set(tcam_id, profile_sp_mask, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_sp_mask_1_get(tcam_id, profile_sp_mask, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_sp_mask_1_get(%u %u %u)\n", tcam_id, profile_sp_mask,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_sp_mask = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_sp_mask_2_set(%u %u %u)\n", tcam_id, profile_sp_mask,
            ram_line);
        ag_err = ag_drv_tcam_profiles_sp_mask_2_set(tcam_id, profile_sp_mask, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_sp_mask_2_get(tcam_id, profile_sp_mask, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_sp_mask_2_get(%u %u %u)\n", tcam_id, profile_sp_mask,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_sp_mask = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_sp_mask_3_set(%u %u %u)\n", tcam_id, profile_sp_mask,
            ram_line);
        ag_err = ag_drv_tcam_profiles_sp_mask_3_set(tcam_id, profile_sp_mask, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_sp_mask_3_get(tcam_id, profile_sp_mask, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_sp_mask_3_get(%u %u %u)\n", tcam_id, profile_sp_mask,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_rslt_mux = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_mux_0_set(%u %u %u)\n", tcam_id, profile_rslt_mux,
            ram_line);
        ag_err = ag_drv_tcam_profiles_rslt_mux_0_set(tcam_id, profile_rslt_mux, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_rslt_mux_0_get(tcam_id, profile_rslt_mux, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_mux_0_get(%u %u %u)\n", tcam_id, profile_rslt_mux,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_rslt_mux = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_mux_1_set(%u %u %u)\n", tcam_id, profile_rslt_mux,
            ram_line);
        ag_err = ag_drv_tcam_profiles_rslt_mux_1_set(tcam_id, profile_rslt_mux, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_rslt_mux_1_get(tcam_id, profile_rslt_mux, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_mux_1_get(%u %u %u)\n", tcam_id, profile_rslt_mux,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_rslt_mux = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_mux_2_set(%u %u %u)\n", tcam_id, profile_rslt_mux,
            ram_line);
        ag_err = ag_drv_tcam_profiles_rslt_mux_2_set(tcam_id, profile_rslt_mux, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_rslt_mux_2_get(tcam_id, profile_rslt_mux, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_mux_2_get(%u %u %u)\n", tcam_id, profile_rslt_mux,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_rslt_mux = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_mux_3_set(%u %u %u)\n", tcam_id, profile_rslt_mux,
            ram_line);
        ag_err = ag_drv_tcam_profiles_rslt_mux_3_set(tcam_id, profile_rslt_mux, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_rslt_mux_3_get(tcam_id, profile_rslt_mux, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_mux_3_get(%u %u %u)\n", tcam_id, profile_rslt_mux,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_rslt_builder = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_0_set(%u %u %u)\n", tcam_id, profile_rslt_builder,
            ram_line);
        ag_err = ag_drv_tcam_profiles_rslt_builder_0_set(tcam_id, profile_rslt_builder, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_rslt_builder_0_get(tcam_id, profile_rslt_builder, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_0_get(%u %u %u)\n", tcam_id, profile_rslt_builder,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_rslt_builder = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_1_set(%u %u %u)\n", tcam_id, profile_rslt_builder,
            ram_line);
        ag_err = ag_drv_tcam_profiles_rslt_builder_1_set(tcam_id, profile_rslt_builder, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_rslt_builder_1_get(tcam_id, profile_rslt_builder, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_1_get(%u %u %u)\n", tcam_id, profile_rslt_builder,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_rslt_builder = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_2_set(%u %u %u)\n", tcam_id, profile_rslt_builder,
            ram_line);
        ag_err = ag_drv_tcam_profiles_rslt_builder_2_set(tcam_id, profile_rslt_builder, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_rslt_builder_2_get(tcam_id, profile_rslt_builder, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_2_get(%u %u %u)\n", tcam_id, profile_rslt_builder,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_rslt_builder = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_3_set(%u %u %u)\n", tcam_id, profile_rslt_builder,
            ram_line);
        ag_err = ag_drv_tcam_profiles_rslt_builder_3_set(tcam_id, profile_rslt_builder, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_rslt_builder_3_get(tcam_id, profile_rslt_builder, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_3_get(%u %u %u)\n", tcam_id, profile_rslt_builder,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_rslt_builder = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_4_set(%u %u %u)\n", tcam_id, profile_rslt_builder,
            ram_line);
        ag_err = ag_drv_tcam_profiles_rslt_builder_4_set(tcam_id, profile_rslt_builder, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_rslt_builder_4_get(tcam_id, profile_rslt_builder, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_4_get(%u %u %u)\n", tcam_id, profile_rslt_builder,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_rslt_builder = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_5_set(%u %u %u)\n", tcam_id, profile_rslt_builder,
            ram_line);
        ag_err = ag_drv_tcam_profiles_rslt_builder_5_set(tcam_id, profile_rslt_builder, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_rslt_builder_5_get(tcam_id, profile_rslt_builder, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_5_get(%u %u %u)\n", tcam_id, profile_rslt_builder,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_rslt_builder = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_6_set(%u %u %u)\n", tcam_id, profile_rslt_builder,
            ram_line);
        ag_err = ag_drv_tcam_profiles_rslt_builder_6_set(tcam_id, profile_rslt_builder, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_rslt_builder_6_get(tcam_id, profile_rslt_builder, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_6_get(%u %u %u)\n", tcam_id, profile_rslt_builder,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_rslt_builder = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_7_set(%u %u %u)\n", tcam_id, profile_rslt_builder,
            ram_line);
        ag_err = ag_drv_tcam_profiles_rslt_builder_7_set(tcam_id, profile_rslt_builder, ram_line);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_rslt_builder_7_get(tcam_id, profile_rslt_builder, &ram_line);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_7_get(%u %u %u)\n", tcam_id, profile_rslt_builder,
                ram_line);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ram_line != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t profile_tcam_concat = gtmv(m, 2);
        uint16_t en = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_tcam_profiles_tcam_concat_set(%u %u %u)\n", tcam_id, profile_tcam_concat,
            en);
        ag_err = ag_drv_tcam_profiles_tcam_concat_set(tcam_id, profile_tcam_concat, en);
        if (!ag_err)
            ag_err = ag_drv_tcam_profiles_tcam_concat_get(tcam_id, profile_tcam_concat, &en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_tcam_concat_get(%u %u %u)\n", tcam_id, profile_tcam_concat,
                en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (en != gtmv(m, 16))
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
static int ag_drv_tcam_cli_ext_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m, input_method = parm[0].value.unumber;
    uint8_t tcam_id = parm[2].value.unumber;
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
    case cli_tcam_cfg_dinm:
    {
        uint32_t max_cfg_dinm = 64;
        uint32_t cfg_dinm = gtmv(m, 6);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_cfg_dinm) || (stop_idx >= max_cfg_dinm))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_cfg_dinm);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (cfg_dinm = 0; cfg_dinm < max_cfg_dinm; cfg_dinm++)
        {
            bdmf_session_print(session, "ag_drv_tcam_cfg_dinm_set(%u %u %u)\n", tcam_id, cfg_dinm,
                ram_line);
            ag_err = ag_drv_tcam_cfg_dinm_set(tcam_id, cfg_dinm, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", cfg_dinm);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (cfg_dinm = start_idx; cfg_dinm <= stop_idx; cfg_dinm++)
        {
            bdmf_session_print(session, "ag_drv_tcam_cfg_dinm_set(%u %u %u)\n", tcam_id, cfg_dinm,
                ram_line);
            ag_err = ag_drv_tcam_cfg_dinm_set(tcam_id, cfg_dinm, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", cfg_dinm);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (cfg_dinm = 0; cfg_dinm < max_cfg_dinm; cfg_dinm++)
        {
            if (cfg_dinm < start_idx || cfg_dinm > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_cfg_dinm_get(tcam_id, cfg_dinm, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_cfg_dinm_get(%u %u %u)\n", tcam_id, cfg_dinm,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", cfg_dinm);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of cfg_dinm completed. Number of tested entries %u.\n", max_cfg_dinm);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_cfg_blksel:
    {
        uint32_t max_cfg_blksel = 2;
        uint32_t cfg_blksel = gtmv(m, 1);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_cfg_blksel) || (stop_idx >= max_cfg_blksel))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_cfg_blksel);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (cfg_blksel = 0; cfg_blksel < max_cfg_blksel; cfg_blksel++)
        {
            bdmf_session_print(session, "ag_drv_tcam_cfg_blksel_set(%u %u %u)\n", tcam_id, cfg_blksel,
                ram_line);
            ag_err = ag_drv_tcam_cfg_blksel_set(tcam_id, cfg_blksel, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", cfg_blksel);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (cfg_blksel = start_idx; cfg_blksel <= stop_idx; cfg_blksel++)
        {
            bdmf_session_print(session, "ag_drv_tcam_cfg_blksel_set(%u %u %u)\n", tcam_id, cfg_blksel,
                ram_line);
            ag_err = ag_drv_tcam_cfg_blksel_set(tcam_id, cfg_blksel, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", cfg_blksel);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (cfg_blksel = 0; cfg_blksel < max_cfg_blksel; cfg_blksel++)
        {
            if (cfg_blksel < start_idx || cfg_blksel > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_cfg_blksel_get(tcam_id, cfg_blksel, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_cfg_blksel_get(%u %u %u)\n", tcam_id, cfg_blksel,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", cfg_blksel);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of cfg_blksel completed. Number of tested entries %u.\n", max_cfg_blksel);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_cfg_tm:
    {
        uint32_t max_cfg_tm = 8;
        uint32_t cfg_tm = gtmv(m, 3);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_cfg_tm) || (stop_idx >= max_cfg_tm))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_cfg_tm);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (cfg_tm = 0; cfg_tm < max_cfg_tm; cfg_tm++)
        {
            bdmf_session_print(session, "ag_drv_tcam_cfg_tm_set(%u %u %u)\n", tcam_id, cfg_tm,
                ram_line);
            ag_err = ag_drv_tcam_cfg_tm_set(tcam_id, cfg_tm, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", cfg_tm);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (cfg_tm = start_idx; cfg_tm <= stop_idx; cfg_tm++)
        {
            bdmf_session_print(session, "ag_drv_tcam_cfg_tm_set(%u %u %u)\n", tcam_id, cfg_tm,
                ram_line);
            ag_err = ag_drv_tcam_cfg_tm_set(tcam_id, cfg_tm, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", cfg_tm);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (cfg_tm = 0; cfg_tm < max_cfg_tm; cfg_tm++)
        {
            if (cfg_tm < start_idx || cfg_tm > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_cfg_tm_get(tcam_id, cfg_tm, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_cfg_tm_get(%u %u %u)\n", tcam_id, cfg_tm,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", cfg_tm);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of cfg_tm completed. Number of tested entries %u.\n", max_cfg_tm);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_tcam_mem_indirect_data_in:
    {
        uint8_t max_indirect_data_in = 16;
        uint8_t indirect_data_in = gtmv(m, 4);
        uint32_t value = gtmv(m, 32);

        if ((start_idx >= max_indirect_data_in) || (stop_idx >= max_indirect_data_in))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_indirect_data_in);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (indirect_data_in = 0; indirect_data_in < max_indirect_data_in; indirect_data_in++)
        {
            bdmf_session_print(session, "ag_drv_tcam_tcam_mem_indirect_data_in_set(%u %u %u)\n", tcam_id, indirect_data_in,
                value);
            ag_err = ag_drv_tcam_tcam_mem_indirect_data_in_set(tcam_id, indirect_data_in, value);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", indirect_data_in);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        value = gtmv(m, 32);

        for (indirect_data_in = start_idx; indirect_data_in <= stop_idx; indirect_data_in++)
        {
            bdmf_session_print(session, "ag_drv_tcam_tcam_mem_indirect_data_in_set(%u %u %u)\n", tcam_id, indirect_data_in,
                value);
            ag_err = ag_drv_tcam_tcam_mem_indirect_data_in_set(tcam_id, indirect_data_in, value);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", indirect_data_in);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (indirect_data_in = 0; indirect_data_in < max_indirect_data_in; indirect_data_in++)
        {
            if (indirect_data_in < start_idx || indirect_data_in > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_tcam_mem_indirect_data_in_get(tcam_id, indirect_data_in, &value);

            bdmf_session_print(session, "ag_drv_tcam_tcam_mem_indirect_data_in_get(%u %u %u)\n", tcam_id, indirect_data_in,
                value);

            if (value != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", indirect_data_in);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of tcam_mem_indirect_data_in completed. Number of tested entries %u.\n", max_indirect_data_in);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_0:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_0_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_0_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_0_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_0_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_0_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_0_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_0 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_1:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_1_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_1_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_1_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_1_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_1_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_1_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_1 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_2:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_2_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_2_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_2_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_2_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_2_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_2_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_2 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_3:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_3_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_3_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_3_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_3_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_3_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_3_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_3 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_4:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_4_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_4_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_4_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_4_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_4_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_4_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_4 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_5:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_5_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_5_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_5_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_5_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_5_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_5_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_5 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_6:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_6_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_6_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_6_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_6_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_6_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_6_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_6 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_7:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_7_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_7_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_7_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_7_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_7_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_7_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_7 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_8:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_8_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_8_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_8_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_8_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_8_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_8_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_8 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_9:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_9_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_9_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_9_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_9_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_9_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_9_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_9 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_10:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_10_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_10_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_10_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_10_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_10_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_10_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_10 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_11:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_11_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_11_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_11_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_11_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_11_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_11_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_11 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_12:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_12_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_12_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_12_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_12_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_12_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_12_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_12 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_13:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_13_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_13_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_13_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_13_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_13_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_13_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_13 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_14:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_14_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_14_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_14_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_14_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_14_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_14_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_14 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_15:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_15_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_15_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_15_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_15_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_15_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_15_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_15 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_16:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_16_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_16_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_16_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_16_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_16_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_16_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_16 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_17:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_17_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_17_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_17_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_17_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_17_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_17_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_17 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_18:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_18_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_18_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_18_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_18_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_18_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_18_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_18 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_19:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_19_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_19_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_19_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_19_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_19_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_19_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_19 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_20:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_20_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_20_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_20_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_20_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_20_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_20_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_20 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_21:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_21_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_21_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_21_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_21_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_21_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_21_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_21 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_22:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_22_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_22_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_22_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_22_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_22_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_22_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_22 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_23:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_23_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_23_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_23_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_23_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_23_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_23_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_23 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_24:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_24_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_24_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_24_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_24_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_24_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_24_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_24 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_25:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_25_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_25_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_25_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_25_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_25_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_25_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_25 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_26:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_26_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_26_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_26_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_26_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_26_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_26_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_26 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_27:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_27_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_27_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_27_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_27_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_27_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_27_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_27 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_28:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_28_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_28_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_28_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_28_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_28_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_28_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_28 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_29:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_29_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_29_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_29_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_29_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_29_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_29_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_29 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_30:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_30_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_30_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_30_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_30_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_30_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_30_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_30 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_31:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_31_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_31_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_31_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_31_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_31_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_31_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_31 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_32:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_32_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_32_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_32_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_32_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_32_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_32_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_32 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_33:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_33_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_33_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_33_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_33_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_33_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_33_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_33 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_34:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_34_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_34_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_34_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_34_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_34_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_34_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_34 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_35:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_35_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_35_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_35_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_35_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_35_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_35_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_35 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_36:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_36_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_36_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_36_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_36_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_36_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_36_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_36 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_37:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_37_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_37_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_37_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_37_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_37_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_37_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_37 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_38:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_38_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_38_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_38_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_38_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_38_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_38_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_38 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_key_builder_offsets_39:
    {
        uint8_t max_profile_key_builder_offset = 4;
        uint8_t profile_key_builder_offset = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_key_builder_offset) || (stop_idx >= max_profile_key_builder_offset))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_key_builder_offset);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_39_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_39_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_key_builder_offset = start_idx; profile_key_builder_offset <= stop_idx; profile_key_builder_offset++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_39_set(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);
            ag_err = ag_drv_tcam_profiles_key_builder_offsets_39_set(tcam_id, profile_key_builder_offset, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_key_builder_offset);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_key_builder_offset = 0; profile_key_builder_offset < max_profile_key_builder_offset; profile_key_builder_offset++)
        {
            if (profile_key_builder_offset < start_idx || profile_key_builder_offset > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_key_builder_offsets_39_get(tcam_id, profile_key_builder_offset, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_key_builder_offsets_39_get(%u %u %u)\n", tcam_id, profile_key_builder_offset,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_key_builder_offset);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_key_builder_offsets_39 completed. Number of tested entries %u.\n", max_profile_key_builder_offset);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_inner_shift_0:
    {
        uint8_t max_profile_inner_shift = 4;
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_inner_shift) || (stop_idx >= max_profile_inner_shift))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_inner_shift);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_0_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_0_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_inner_shift = start_idx; profile_inner_shift <= stop_idx; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_0_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_0_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            if (profile_inner_shift < start_idx || profile_inner_shift > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_inner_shift_0_get(tcam_id, profile_inner_shift, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_0_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_inner_shift);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_inner_shift_0 completed. Number of tested entries %u.\n", max_profile_inner_shift);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_inner_shift_1:
    {
        uint8_t max_profile_inner_shift = 4;
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_inner_shift) || (stop_idx >= max_profile_inner_shift))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_inner_shift);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_1_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_1_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_inner_shift = start_idx; profile_inner_shift <= stop_idx; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_1_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_1_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            if (profile_inner_shift < start_idx || profile_inner_shift > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_inner_shift_1_get(tcam_id, profile_inner_shift, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_1_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_inner_shift);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_inner_shift_1 completed. Number of tested entries %u.\n", max_profile_inner_shift);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_inner_shift_2:
    {
        uint8_t max_profile_inner_shift = 4;
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_inner_shift) || (stop_idx >= max_profile_inner_shift))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_inner_shift);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_2_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_2_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_inner_shift = start_idx; profile_inner_shift <= stop_idx; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_2_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_2_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            if (profile_inner_shift < start_idx || profile_inner_shift > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_inner_shift_2_get(tcam_id, profile_inner_shift, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_2_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_inner_shift);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_inner_shift_2 completed. Number of tested entries %u.\n", max_profile_inner_shift);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_inner_shift_3:
    {
        uint8_t max_profile_inner_shift = 4;
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_inner_shift) || (stop_idx >= max_profile_inner_shift))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_inner_shift);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_3_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_3_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_inner_shift = start_idx; profile_inner_shift <= stop_idx; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_3_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_3_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            if (profile_inner_shift < start_idx || profile_inner_shift > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_inner_shift_3_get(tcam_id, profile_inner_shift, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_3_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_inner_shift);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_inner_shift_3 completed. Number of tested entries %u.\n", max_profile_inner_shift);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_inner_shift_4:
    {
        uint8_t max_profile_inner_shift = 4;
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_inner_shift) || (stop_idx >= max_profile_inner_shift))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_inner_shift);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_4_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_4_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_inner_shift = start_idx; profile_inner_shift <= stop_idx; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_4_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_4_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            if (profile_inner_shift < start_idx || profile_inner_shift > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_inner_shift_4_get(tcam_id, profile_inner_shift, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_4_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_inner_shift);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_inner_shift_4 completed. Number of tested entries %u.\n", max_profile_inner_shift);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_inner_shift_5:
    {
        uint8_t max_profile_inner_shift = 4;
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_inner_shift) || (stop_idx >= max_profile_inner_shift))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_inner_shift);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_5_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_5_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_inner_shift = start_idx; profile_inner_shift <= stop_idx; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_5_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_5_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            if (profile_inner_shift < start_idx || profile_inner_shift > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_inner_shift_5_get(tcam_id, profile_inner_shift, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_5_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_inner_shift);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_inner_shift_5 completed. Number of tested entries %u.\n", max_profile_inner_shift);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_inner_shift_6:
    {
        uint8_t max_profile_inner_shift = 4;
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_inner_shift) || (stop_idx >= max_profile_inner_shift))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_inner_shift);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_6_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_6_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_inner_shift = start_idx; profile_inner_shift <= stop_idx; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_6_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_6_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            if (profile_inner_shift < start_idx || profile_inner_shift > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_inner_shift_6_get(tcam_id, profile_inner_shift, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_6_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_inner_shift);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_inner_shift_6 completed. Number of tested entries %u.\n", max_profile_inner_shift);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_inner_shift_7:
    {
        uint8_t max_profile_inner_shift = 4;
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_inner_shift) || (stop_idx >= max_profile_inner_shift))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_inner_shift);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_7_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_7_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_inner_shift = start_idx; profile_inner_shift <= stop_idx; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_7_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_7_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            if (profile_inner_shift < start_idx || profile_inner_shift > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_inner_shift_7_get(tcam_id, profile_inner_shift, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_7_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_inner_shift);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_inner_shift_7 completed. Number of tested entries %u.\n", max_profile_inner_shift);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_inner_shift_8:
    {
        uint8_t max_profile_inner_shift = 4;
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_inner_shift) || (stop_idx >= max_profile_inner_shift))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_inner_shift);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_8_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_8_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_inner_shift = start_idx; profile_inner_shift <= stop_idx; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_8_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_8_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            if (profile_inner_shift < start_idx || profile_inner_shift > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_inner_shift_8_get(tcam_id, profile_inner_shift, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_8_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_inner_shift);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_inner_shift_8 completed. Number of tested entries %u.\n", max_profile_inner_shift);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_inner_shift_9:
    {
        uint8_t max_profile_inner_shift = 4;
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_inner_shift) || (stop_idx >= max_profile_inner_shift))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_inner_shift);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_9_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_9_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_inner_shift = start_idx; profile_inner_shift <= stop_idx; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_9_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_9_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            if (profile_inner_shift < start_idx || profile_inner_shift > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_inner_shift_9_get(tcam_id, profile_inner_shift, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_9_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_inner_shift);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_inner_shift_9 completed. Number of tested entries %u.\n", max_profile_inner_shift);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_inner_shift_10:
    {
        uint8_t max_profile_inner_shift = 4;
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_inner_shift) || (stop_idx >= max_profile_inner_shift))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_inner_shift);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_10_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_10_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_inner_shift = start_idx; profile_inner_shift <= stop_idx; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_10_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_10_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            if (profile_inner_shift < start_idx || profile_inner_shift > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_inner_shift_10_get(tcam_id, profile_inner_shift, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_10_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_inner_shift);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_inner_shift_10 completed. Number of tested entries %u.\n", max_profile_inner_shift);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_inner_shift_11:
    {
        uint8_t max_profile_inner_shift = 4;
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_inner_shift) || (stop_idx >= max_profile_inner_shift))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_inner_shift);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_11_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_11_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_inner_shift = start_idx; profile_inner_shift <= stop_idx; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_11_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_11_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            if (profile_inner_shift < start_idx || profile_inner_shift > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_inner_shift_11_get(tcam_id, profile_inner_shift, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_11_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_inner_shift);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_inner_shift_11 completed. Number of tested entries %u.\n", max_profile_inner_shift);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_inner_shift_12:
    {
        uint8_t max_profile_inner_shift = 4;
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_inner_shift) || (stop_idx >= max_profile_inner_shift))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_inner_shift);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_12_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_12_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_inner_shift = start_idx; profile_inner_shift <= stop_idx; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_12_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_12_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            if (profile_inner_shift < start_idx || profile_inner_shift > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_inner_shift_12_get(tcam_id, profile_inner_shift, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_12_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_inner_shift);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_inner_shift_12 completed. Number of tested entries %u.\n", max_profile_inner_shift);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_inner_shift_13:
    {
        uint8_t max_profile_inner_shift = 4;
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_inner_shift) || (stop_idx >= max_profile_inner_shift))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_inner_shift);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_13_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_13_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_inner_shift = start_idx; profile_inner_shift <= stop_idx; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_13_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_13_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            if (profile_inner_shift < start_idx || profile_inner_shift > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_inner_shift_13_get(tcam_id, profile_inner_shift, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_13_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_inner_shift);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_inner_shift_13 completed. Number of tested entries %u.\n", max_profile_inner_shift);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_inner_shift_14:
    {
        uint8_t max_profile_inner_shift = 4;
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_inner_shift) || (stop_idx >= max_profile_inner_shift))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_inner_shift);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_14_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_14_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_inner_shift = start_idx; profile_inner_shift <= stop_idx; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_14_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_14_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            if (profile_inner_shift < start_idx || profile_inner_shift > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_inner_shift_14_get(tcam_id, profile_inner_shift, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_14_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_inner_shift);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_inner_shift_14 completed. Number of tested entries %u.\n", max_profile_inner_shift);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_inner_shift_15:
    {
        uint8_t max_profile_inner_shift = 4;
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_inner_shift) || (stop_idx >= max_profile_inner_shift))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_inner_shift);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_15_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_15_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_inner_shift = start_idx; profile_inner_shift <= stop_idx; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_15_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_15_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            if (profile_inner_shift < start_idx || profile_inner_shift > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_inner_shift_15_get(tcam_id, profile_inner_shift, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_15_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_inner_shift);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_inner_shift_15 completed. Number of tested entries %u.\n", max_profile_inner_shift);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_inner_shift_16:
    {
        uint8_t max_profile_inner_shift = 4;
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_inner_shift) || (stop_idx >= max_profile_inner_shift))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_inner_shift);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_16_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_16_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_inner_shift = start_idx; profile_inner_shift <= stop_idx; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_16_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_16_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            if (profile_inner_shift < start_idx || profile_inner_shift > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_inner_shift_16_get(tcam_id, profile_inner_shift, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_16_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_inner_shift);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_inner_shift_16 completed. Number of tested entries %u.\n", max_profile_inner_shift);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_inner_shift_17:
    {
        uint8_t max_profile_inner_shift = 4;
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_inner_shift) || (stop_idx >= max_profile_inner_shift))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_inner_shift);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_17_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_17_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_inner_shift = start_idx; profile_inner_shift <= stop_idx; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_17_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_17_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            if (profile_inner_shift < start_idx || profile_inner_shift > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_inner_shift_17_get(tcam_id, profile_inner_shift, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_17_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_inner_shift);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_inner_shift_17 completed. Number of tested entries %u.\n", max_profile_inner_shift);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_inner_shift_18:
    {
        uint8_t max_profile_inner_shift = 4;
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_inner_shift) || (stop_idx >= max_profile_inner_shift))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_inner_shift);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_18_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_18_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_inner_shift = start_idx; profile_inner_shift <= stop_idx; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_18_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_18_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            if (profile_inner_shift < start_idx || profile_inner_shift > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_inner_shift_18_get(tcam_id, profile_inner_shift, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_18_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_inner_shift);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_inner_shift_18 completed. Number of tested entries %u.\n", max_profile_inner_shift);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_inner_shift_19:
    {
        uint8_t max_profile_inner_shift = 4;
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_inner_shift) || (stop_idx >= max_profile_inner_shift))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_inner_shift);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_19_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_19_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_inner_shift = start_idx; profile_inner_shift <= stop_idx; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_19_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_19_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            if (profile_inner_shift < start_idx || profile_inner_shift > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_inner_shift_19_get(tcam_id, profile_inner_shift, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_19_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_inner_shift);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_inner_shift_19 completed. Number of tested entries %u.\n", max_profile_inner_shift);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_inner_shift_20:
    {
        uint8_t max_profile_inner_shift = 4;
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_inner_shift) || (stop_idx >= max_profile_inner_shift))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_inner_shift);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_20_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_20_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_inner_shift = start_idx; profile_inner_shift <= stop_idx; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_20_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_20_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            if (profile_inner_shift < start_idx || profile_inner_shift > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_inner_shift_20_get(tcam_id, profile_inner_shift, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_20_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_inner_shift);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_inner_shift_20 completed. Number of tested entries %u.\n", max_profile_inner_shift);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_inner_shift_21:
    {
        uint8_t max_profile_inner_shift = 4;
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_inner_shift) || (stop_idx >= max_profile_inner_shift))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_inner_shift);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_21_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_21_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_inner_shift = start_idx; profile_inner_shift <= stop_idx; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_21_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_21_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            if (profile_inner_shift < start_idx || profile_inner_shift > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_inner_shift_21_get(tcam_id, profile_inner_shift, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_21_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_inner_shift);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_inner_shift_21 completed. Number of tested entries %u.\n", max_profile_inner_shift);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_inner_shift_22:
    {
        uint8_t max_profile_inner_shift = 4;
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_inner_shift) || (stop_idx >= max_profile_inner_shift))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_inner_shift);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_22_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_22_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_inner_shift = start_idx; profile_inner_shift <= stop_idx; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_22_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_22_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            if (profile_inner_shift < start_idx || profile_inner_shift > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_inner_shift_22_get(tcam_id, profile_inner_shift, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_22_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_inner_shift);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_inner_shift_22 completed. Number of tested entries %u.\n", max_profile_inner_shift);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_inner_shift_23:
    {
        uint8_t max_profile_inner_shift = 4;
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_inner_shift) || (stop_idx >= max_profile_inner_shift))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_inner_shift);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_23_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_23_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_inner_shift = start_idx; profile_inner_shift <= stop_idx; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_23_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_23_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            if (profile_inner_shift < start_idx || profile_inner_shift > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_inner_shift_23_get(tcam_id, profile_inner_shift, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_23_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_inner_shift);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_inner_shift_23 completed. Number of tested entries %u.\n", max_profile_inner_shift);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_inner_shift_24:
    {
        uint8_t max_profile_inner_shift = 4;
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_inner_shift) || (stop_idx >= max_profile_inner_shift))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_inner_shift);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_24_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_24_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_inner_shift = start_idx; profile_inner_shift <= stop_idx; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_24_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_24_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            if (profile_inner_shift < start_idx || profile_inner_shift > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_inner_shift_24_get(tcam_id, profile_inner_shift, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_24_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_inner_shift);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_inner_shift_24 completed. Number of tested entries %u.\n", max_profile_inner_shift);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_inner_shift_25:
    {
        uint8_t max_profile_inner_shift = 4;
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_inner_shift) || (stop_idx >= max_profile_inner_shift))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_inner_shift);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_25_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_25_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_inner_shift = start_idx; profile_inner_shift <= stop_idx; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_25_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_25_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            if (profile_inner_shift < start_idx || profile_inner_shift > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_inner_shift_25_get(tcam_id, profile_inner_shift, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_25_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_inner_shift);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_inner_shift_25 completed. Number of tested entries %u.\n", max_profile_inner_shift);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_inner_shift_26:
    {
        uint8_t max_profile_inner_shift = 4;
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_inner_shift) || (stop_idx >= max_profile_inner_shift))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_inner_shift);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_26_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_26_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_inner_shift = start_idx; profile_inner_shift <= stop_idx; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_26_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_26_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            if (profile_inner_shift < start_idx || profile_inner_shift > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_inner_shift_26_get(tcam_id, profile_inner_shift, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_26_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_inner_shift);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_inner_shift_26 completed. Number of tested entries %u.\n", max_profile_inner_shift);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_inner_shift_27:
    {
        uint8_t max_profile_inner_shift = 4;
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_inner_shift) || (stop_idx >= max_profile_inner_shift))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_inner_shift);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_27_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_27_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_inner_shift = start_idx; profile_inner_shift <= stop_idx; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_27_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_27_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            if (profile_inner_shift < start_idx || profile_inner_shift > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_inner_shift_27_get(tcam_id, profile_inner_shift, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_27_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_inner_shift);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_inner_shift_27 completed. Number of tested entries %u.\n", max_profile_inner_shift);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_inner_shift_28:
    {
        uint8_t max_profile_inner_shift = 4;
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_inner_shift) || (stop_idx >= max_profile_inner_shift))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_inner_shift);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_28_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_28_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_inner_shift = start_idx; profile_inner_shift <= stop_idx; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_28_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_28_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            if (profile_inner_shift < start_idx || profile_inner_shift > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_inner_shift_28_get(tcam_id, profile_inner_shift, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_28_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_inner_shift);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_inner_shift_28 completed. Number of tested entries %u.\n", max_profile_inner_shift);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_inner_shift_29:
    {
        uint8_t max_profile_inner_shift = 4;
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_inner_shift) || (stop_idx >= max_profile_inner_shift))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_inner_shift);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_29_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_29_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_inner_shift = start_idx; profile_inner_shift <= stop_idx; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_29_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_29_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            if (profile_inner_shift < start_idx || profile_inner_shift > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_inner_shift_29_get(tcam_id, profile_inner_shift, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_29_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_inner_shift);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_inner_shift_29 completed. Number of tested entries %u.\n", max_profile_inner_shift);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_inner_shift_30:
    {
        uint8_t max_profile_inner_shift = 4;
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_inner_shift) || (stop_idx >= max_profile_inner_shift))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_inner_shift);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_30_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_30_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_inner_shift = start_idx; profile_inner_shift <= stop_idx; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_30_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_30_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            if (profile_inner_shift < start_idx || profile_inner_shift > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_inner_shift_30_get(tcam_id, profile_inner_shift, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_30_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_inner_shift);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_inner_shift_30 completed. Number of tested entries %u.\n", max_profile_inner_shift);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_inner_shift_31:
    {
        uint8_t max_profile_inner_shift = 4;
        uint8_t profile_inner_shift = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_inner_shift) || (stop_idx >= max_profile_inner_shift))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_inner_shift);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_31_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_31_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_inner_shift = start_idx; profile_inner_shift <= stop_idx; profile_inner_shift++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_31_set(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);
            ag_err = ag_drv_tcam_profiles_inner_shift_31_set(tcam_id, profile_inner_shift, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_inner_shift);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_inner_shift = 0; profile_inner_shift < max_profile_inner_shift; profile_inner_shift++)
        {
            if (profile_inner_shift < start_idx || profile_inner_shift > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_inner_shift_31_get(tcam_id, profile_inner_shift, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_inner_shift_31_get(%u %u %u)\n", tcam_id, profile_inner_shift,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_inner_shift);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_inner_shift_31 completed. Number of tested entries %u.\n", max_profile_inner_shift);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_cntxin_sp_id_0:
    {
        uint8_t max_profile_sp_id = 4;
        uint8_t profile_sp_id = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_sp_id) || (stop_idx >= max_profile_sp_id))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_sp_id);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_sp_id = 0; profile_sp_id < max_profile_sp_id; profile_sp_id++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_sp_id_0_set(%u %u %u)\n", tcam_id, profile_sp_id,
                ram_line);
            ag_err = ag_drv_tcam_profiles_cntxin_sp_id_0_set(tcam_id, profile_sp_id, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_sp_id);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_sp_id = start_idx; profile_sp_id <= stop_idx; profile_sp_id++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_sp_id_0_set(%u %u %u)\n", tcam_id, profile_sp_id,
                ram_line);
            ag_err = ag_drv_tcam_profiles_cntxin_sp_id_0_set(tcam_id, profile_sp_id, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_sp_id);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_sp_id = 0; profile_sp_id < max_profile_sp_id; profile_sp_id++)
        {
            if (profile_sp_id < start_idx || profile_sp_id > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_cntxin_sp_id_0_get(tcam_id, profile_sp_id, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_sp_id_0_get(%u %u %u)\n", tcam_id, profile_sp_id,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_sp_id);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_cntxin_sp_id_0 completed. Number of tested entries %u.\n", max_profile_sp_id);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_cntxin_sp_id_1:
    {
        uint8_t max_profile_sp_id = 4;
        uint8_t profile_sp_id = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_sp_id) || (stop_idx >= max_profile_sp_id))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_sp_id);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_sp_id = 0; profile_sp_id < max_profile_sp_id; profile_sp_id++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_sp_id_1_set(%u %u %u)\n", tcam_id, profile_sp_id,
                ram_line);
            ag_err = ag_drv_tcam_profiles_cntxin_sp_id_1_set(tcam_id, profile_sp_id, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_sp_id);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_sp_id = start_idx; profile_sp_id <= stop_idx; profile_sp_id++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_sp_id_1_set(%u %u %u)\n", tcam_id, profile_sp_id,
                ram_line);
            ag_err = ag_drv_tcam_profiles_cntxin_sp_id_1_set(tcam_id, profile_sp_id, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_sp_id);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_sp_id = 0; profile_sp_id < max_profile_sp_id; profile_sp_id++)
        {
            if (profile_sp_id < start_idx || profile_sp_id > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_cntxin_sp_id_1_get(tcam_id, profile_sp_id, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_sp_id_1_get(%u %u %u)\n", tcam_id, profile_sp_id,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_sp_id);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_cntxin_sp_id_1 completed. Number of tested entries %u.\n", max_profile_sp_id);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_cntxin_mask_0:
    {
        uint8_t max_profile_cntxin_mask = 4;
        uint8_t profile_cntxin_mask = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_cntxin_mask) || (stop_idx >= max_profile_cntxin_mask))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_cntxin_mask);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_cntxin_mask = 0; profile_cntxin_mask < max_profile_cntxin_mask; profile_cntxin_mask++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_0_set(%u %u %u)\n", tcam_id, profile_cntxin_mask,
                ram_line);
            ag_err = ag_drv_tcam_profiles_cntxin_mask_0_set(tcam_id, profile_cntxin_mask, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_cntxin_mask);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_cntxin_mask = start_idx; profile_cntxin_mask <= stop_idx; profile_cntxin_mask++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_0_set(%u %u %u)\n", tcam_id, profile_cntxin_mask,
                ram_line);
            ag_err = ag_drv_tcam_profiles_cntxin_mask_0_set(tcam_id, profile_cntxin_mask, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_cntxin_mask);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_cntxin_mask = 0; profile_cntxin_mask < max_profile_cntxin_mask; profile_cntxin_mask++)
        {
            if (profile_cntxin_mask < start_idx || profile_cntxin_mask > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_cntxin_mask_0_get(tcam_id, profile_cntxin_mask, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_0_get(%u %u %u)\n", tcam_id, profile_cntxin_mask,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_cntxin_mask);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_cntxin_mask_0 completed. Number of tested entries %u.\n", max_profile_cntxin_mask);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_cntxin_mask_1:
    {
        uint8_t max_profile_cntxin_mask = 4;
        uint8_t profile_cntxin_mask = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_cntxin_mask) || (stop_idx >= max_profile_cntxin_mask))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_cntxin_mask);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_cntxin_mask = 0; profile_cntxin_mask < max_profile_cntxin_mask; profile_cntxin_mask++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_1_set(%u %u %u)\n", tcam_id, profile_cntxin_mask,
                ram_line);
            ag_err = ag_drv_tcam_profiles_cntxin_mask_1_set(tcam_id, profile_cntxin_mask, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_cntxin_mask);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_cntxin_mask = start_idx; profile_cntxin_mask <= stop_idx; profile_cntxin_mask++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_1_set(%u %u %u)\n", tcam_id, profile_cntxin_mask,
                ram_line);
            ag_err = ag_drv_tcam_profiles_cntxin_mask_1_set(tcam_id, profile_cntxin_mask, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_cntxin_mask);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_cntxin_mask = 0; profile_cntxin_mask < max_profile_cntxin_mask; profile_cntxin_mask++)
        {
            if (profile_cntxin_mask < start_idx || profile_cntxin_mask > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_cntxin_mask_1_get(tcam_id, profile_cntxin_mask, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_1_get(%u %u %u)\n", tcam_id, profile_cntxin_mask,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_cntxin_mask);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_cntxin_mask_1 completed. Number of tested entries %u.\n", max_profile_cntxin_mask);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_cntxin_mask_2:
    {
        uint8_t max_profile_cntxin_mask = 4;
        uint8_t profile_cntxin_mask = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_cntxin_mask) || (stop_idx >= max_profile_cntxin_mask))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_cntxin_mask);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_cntxin_mask = 0; profile_cntxin_mask < max_profile_cntxin_mask; profile_cntxin_mask++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_2_set(%u %u %u)\n", tcam_id, profile_cntxin_mask,
                ram_line);
            ag_err = ag_drv_tcam_profiles_cntxin_mask_2_set(tcam_id, profile_cntxin_mask, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_cntxin_mask);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_cntxin_mask = start_idx; profile_cntxin_mask <= stop_idx; profile_cntxin_mask++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_2_set(%u %u %u)\n", tcam_id, profile_cntxin_mask,
                ram_line);
            ag_err = ag_drv_tcam_profiles_cntxin_mask_2_set(tcam_id, profile_cntxin_mask, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_cntxin_mask);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_cntxin_mask = 0; profile_cntxin_mask < max_profile_cntxin_mask; profile_cntxin_mask++)
        {
            if (profile_cntxin_mask < start_idx || profile_cntxin_mask > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_cntxin_mask_2_get(tcam_id, profile_cntxin_mask, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_2_get(%u %u %u)\n", tcam_id, profile_cntxin_mask,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_cntxin_mask);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_cntxin_mask_2 completed. Number of tested entries %u.\n", max_profile_cntxin_mask);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_cntxin_mask_3:
    {
        uint8_t max_profile_cntxin_mask = 4;
        uint8_t profile_cntxin_mask = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_cntxin_mask) || (stop_idx >= max_profile_cntxin_mask))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_cntxin_mask);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_cntxin_mask = 0; profile_cntxin_mask < max_profile_cntxin_mask; profile_cntxin_mask++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_3_set(%u %u %u)\n", tcam_id, profile_cntxin_mask,
                ram_line);
            ag_err = ag_drv_tcam_profiles_cntxin_mask_3_set(tcam_id, profile_cntxin_mask, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_cntxin_mask);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_cntxin_mask = start_idx; profile_cntxin_mask <= stop_idx; profile_cntxin_mask++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_3_set(%u %u %u)\n", tcam_id, profile_cntxin_mask,
                ram_line);
            ag_err = ag_drv_tcam_profiles_cntxin_mask_3_set(tcam_id, profile_cntxin_mask, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_cntxin_mask);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_cntxin_mask = 0; profile_cntxin_mask < max_profile_cntxin_mask; profile_cntxin_mask++)
        {
            if (profile_cntxin_mask < start_idx || profile_cntxin_mask > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_cntxin_mask_3_get(tcam_id, profile_cntxin_mask, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_3_get(%u %u %u)\n", tcam_id, profile_cntxin_mask,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_cntxin_mask);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_cntxin_mask_3 completed. Number of tested entries %u.\n", max_profile_cntxin_mask);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_cntxin_mask_4:
    {
        uint8_t max_profile_cntxin_mask = 4;
        uint8_t profile_cntxin_mask = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_cntxin_mask) || (stop_idx >= max_profile_cntxin_mask))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_cntxin_mask);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_cntxin_mask = 0; profile_cntxin_mask < max_profile_cntxin_mask; profile_cntxin_mask++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_4_set(%u %u %u)\n", tcam_id, profile_cntxin_mask,
                ram_line);
            ag_err = ag_drv_tcam_profiles_cntxin_mask_4_set(tcam_id, profile_cntxin_mask, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_cntxin_mask);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_cntxin_mask = start_idx; profile_cntxin_mask <= stop_idx; profile_cntxin_mask++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_4_set(%u %u %u)\n", tcam_id, profile_cntxin_mask,
                ram_line);
            ag_err = ag_drv_tcam_profiles_cntxin_mask_4_set(tcam_id, profile_cntxin_mask, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_cntxin_mask);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_cntxin_mask = 0; profile_cntxin_mask < max_profile_cntxin_mask; profile_cntxin_mask++)
        {
            if (profile_cntxin_mask < start_idx || profile_cntxin_mask > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_cntxin_mask_4_get(tcam_id, profile_cntxin_mask, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_4_get(%u %u %u)\n", tcam_id, profile_cntxin_mask,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_cntxin_mask);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_cntxin_mask_4 completed. Number of tested entries %u.\n", max_profile_cntxin_mask);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_cntxin_mask_5:
    {
        uint8_t max_profile_cntxin_mask = 4;
        uint8_t profile_cntxin_mask = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_cntxin_mask) || (stop_idx >= max_profile_cntxin_mask))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_cntxin_mask);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_cntxin_mask = 0; profile_cntxin_mask < max_profile_cntxin_mask; profile_cntxin_mask++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_5_set(%u %u %u)\n", tcam_id, profile_cntxin_mask,
                ram_line);
            ag_err = ag_drv_tcam_profiles_cntxin_mask_5_set(tcam_id, profile_cntxin_mask, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_cntxin_mask);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_cntxin_mask = start_idx; profile_cntxin_mask <= stop_idx; profile_cntxin_mask++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_5_set(%u %u %u)\n", tcam_id, profile_cntxin_mask,
                ram_line);
            ag_err = ag_drv_tcam_profiles_cntxin_mask_5_set(tcam_id, profile_cntxin_mask, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_cntxin_mask);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_cntxin_mask = 0; profile_cntxin_mask < max_profile_cntxin_mask; profile_cntxin_mask++)
        {
            if (profile_cntxin_mask < start_idx || profile_cntxin_mask > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_cntxin_mask_5_get(tcam_id, profile_cntxin_mask, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_5_get(%u %u %u)\n", tcam_id, profile_cntxin_mask,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_cntxin_mask);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_cntxin_mask_5 completed. Number of tested entries %u.\n", max_profile_cntxin_mask);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_cntxin_mask_6:
    {
        uint8_t max_profile_cntxin_mask = 4;
        uint8_t profile_cntxin_mask = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_cntxin_mask) || (stop_idx >= max_profile_cntxin_mask))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_cntxin_mask);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_cntxin_mask = 0; profile_cntxin_mask < max_profile_cntxin_mask; profile_cntxin_mask++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_6_set(%u %u %u)\n", tcam_id, profile_cntxin_mask,
                ram_line);
            ag_err = ag_drv_tcam_profiles_cntxin_mask_6_set(tcam_id, profile_cntxin_mask, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_cntxin_mask);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_cntxin_mask = start_idx; profile_cntxin_mask <= stop_idx; profile_cntxin_mask++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_6_set(%u %u %u)\n", tcam_id, profile_cntxin_mask,
                ram_line);
            ag_err = ag_drv_tcam_profiles_cntxin_mask_6_set(tcam_id, profile_cntxin_mask, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_cntxin_mask);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_cntxin_mask = 0; profile_cntxin_mask < max_profile_cntxin_mask; profile_cntxin_mask++)
        {
            if (profile_cntxin_mask < start_idx || profile_cntxin_mask > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_cntxin_mask_6_get(tcam_id, profile_cntxin_mask, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_6_get(%u %u %u)\n", tcam_id, profile_cntxin_mask,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_cntxin_mask);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_cntxin_mask_6 completed. Number of tested entries %u.\n", max_profile_cntxin_mask);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_cntxin_mask_7:
    {
        uint8_t max_profile_cntxin_mask = 4;
        uint8_t profile_cntxin_mask = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_cntxin_mask) || (stop_idx >= max_profile_cntxin_mask))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_cntxin_mask);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_cntxin_mask = 0; profile_cntxin_mask < max_profile_cntxin_mask; profile_cntxin_mask++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_7_set(%u %u %u)\n", tcam_id, profile_cntxin_mask,
                ram_line);
            ag_err = ag_drv_tcam_profiles_cntxin_mask_7_set(tcam_id, profile_cntxin_mask, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_cntxin_mask);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_cntxin_mask = start_idx; profile_cntxin_mask <= stop_idx; profile_cntxin_mask++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_7_set(%u %u %u)\n", tcam_id, profile_cntxin_mask,
                ram_line);
            ag_err = ag_drv_tcam_profiles_cntxin_mask_7_set(tcam_id, profile_cntxin_mask, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_cntxin_mask);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_cntxin_mask = 0; profile_cntxin_mask < max_profile_cntxin_mask; profile_cntxin_mask++)
        {
            if (profile_cntxin_mask < start_idx || profile_cntxin_mask > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_cntxin_mask_7_get(tcam_id, profile_cntxin_mask, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_cntxin_mask_7_get(%u %u %u)\n", tcam_id, profile_cntxin_mask,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_cntxin_mask);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_cntxin_mask_7 completed. Number of tested entries %u.\n", max_profile_cntxin_mask);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_sp_mask_0:
    {
        uint8_t max_profile_sp_mask = 4;
        uint8_t profile_sp_mask = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_sp_mask) || (stop_idx >= max_profile_sp_mask))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_sp_mask);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_sp_mask = 0; profile_sp_mask < max_profile_sp_mask; profile_sp_mask++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_sp_mask_0_set(%u %u %u)\n", tcam_id, profile_sp_mask,
                ram_line);
            ag_err = ag_drv_tcam_profiles_sp_mask_0_set(tcam_id, profile_sp_mask, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_sp_mask);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_sp_mask = start_idx; profile_sp_mask <= stop_idx; profile_sp_mask++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_sp_mask_0_set(%u %u %u)\n", tcam_id, profile_sp_mask,
                ram_line);
            ag_err = ag_drv_tcam_profiles_sp_mask_0_set(tcam_id, profile_sp_mask, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_sp_mask);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_sp_mask = 0; profile_sp_mask < max_profile_sp_mask; profile_sp_mask++)
        {
            if (profile_sp_mask < start_idx || profile_sp_mask > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_sp_mask_0_get(tcam_id, profile_sp_mask, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_sp_mask_0_get(%u %u %u)\n", tcam_id, profile_sp_mask,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_sp_mask);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_sp_mask_0 completed. Number of tested entries %u.\n", max_profile_sp_mask);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_sp_mask_1:
    {
        uint8_t max_profile_sp_mask = 4;
        uint8_t profile_sp_mask = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_sp_mask) || (stop_idx >= max_profile_sp_mask))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_sp_mask);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_sp_mask = 0; profile_sp_mask < max_profile_sp_mask; profile_sp_mask++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_sp_mask_1_set(%u %u %u)\n", tcam_id, profile_sp_mask,
                ram_line);
            ag_err = ag_drv_tcam_profiles_sp_mask_1_set(tcam_id, profile_sp_mask, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_sp_mask);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_sp_mask = start_idx; profile_sp_mask <= stop_idx; profile_sp_mask++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_sp_mask_1_set(%u %u %u)\n", tcam_id, profile_sp_mask,
                ram_line);
            ag_err = ag_drv_tcam_profiles_sp_mask_1_set(tcam_id, profile_sp_mask, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_sp_mask);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_sp_mask = 0; profile_sp_mask < max_profile_sp_mask; profile_sp_mask++)
        {
            if (profile_sp_mask < start_idx || profile_sp_mask > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_sp_mask_1_get(tcam_id, profile_sp_mask, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_sp_mask_1_get(%u %u %u)\n", tcam_id, profile_sp_mask,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_sp_mask);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_sp_mask_1 completed. Number of tested entries %u.\n", max_profile_sp_mask);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_sp_mask_2:
    {
        uint8_t max_profile_sp_mask = 4;
        uint8_t profile_sp_mask = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_sp_mask) || (stop_idx >= max_profile_sp_mask))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_sp_mask);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_sp_mask = 0; profile_sp_mask < max_profile_sp_mask; profile_sp_mask++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_sp_mask_2_set(%u %u %u)\n", tcam_id, profile_sp_mask,
                ram_line);
            ag_err = ag_drv_tcam_profiles_sp_mask_2_set(tcam_id, profile_sp_mask, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_sp_mask);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_sp_mask = start_idx; profile_sp_mask <= stop_idx; profile_sp_mask++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_sp_mask_2_set(%u %u %u)\n", tcam_id, profile_sp_mask,
                ram_line);
            ag_err = ag_drv_tcam_profiles_sp_mask_2_set(tcam_id, profile_sp_mask, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_sp_mask);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_sp_mask = 0; profile_sp_mask < max_profile_sp_mask; profile_sp_mask++)
        {
            if (profile_sp_mask < start_idx || profile_sp_mask > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_sp_mask_2_get(tcam_id, profile_sp_mask, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_sp_mask_2_get(%u %u %u)\n", tcam_id, profile_sp_mask,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_sp_mask);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_sp_mask_2 completed. Number of tested entries %u.\n", max_profile_sp_mask);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_sp_mask_3:
    {
        uint8_t max_profile_sp_mask = 4;
        uint8_t profile_sp_mask = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_sp_mask) || (stop_idx >= max_profile_sp_mask))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_sp_mask);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_sp_mask = 0; profile_sp_mask < max_profile_sp_mask; profile_sp_mask++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_sp_mask_3_set(%u %u %u)\n", tcam_id, profile_sp_mask,
                ram_line);
            ag_err = ag_drv_tcam_profiles_sp_mask_3_set(tcam_id, profile_sp_mask, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_sp_mask);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_sp_mask = start_idx; profile_sp_mask <= stop_idx; profile_sp_mask++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_sp_mask_3_set(%u %u %u)\n", tcam_id, profile_sp_mask,
                ram_line);
            ag_err = ag_drv_tcam_profiles_sp_mask_3_set(tcam_id, profile_sp_mask, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_sp_mask);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_sp_mask = 0; profile_sp_mask < max_profile_sp_mask; profile_sp_mask++)
        {
            if (profile_sp_mask < start_idx || profile_sp_mask > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_sp_mask_3_get(tcam_id, profile_sp_mask, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_sp_mask_3_get(%u %u %u)\n", tcam_id, profile_sp_mask,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_sp_mask);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_sp_mask_3 completed. Number of tested entries %u.\n", max_profile_sp_mask);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_rslt_mux_0:
    {
        uint8_t max_profile_rslt_mux = 4;
        uint8_t profile_rslt_mux = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_rslt_mux) || (stop_idx >= max_profile_rslt_mux))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_rslt_mux);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_rslt_mux = 0; profile_rslt_mux < max_profile_rslt_mux; profile_rslt_mux++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_mux_0_set(%u %u %u)\n", tcam_id, profile_rslt_mux,
                ram_line);
            ag_err = ag_drv_tcam_profiles_rslt_mux_0_set(tcam_id, profile_rslt_mux, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_rslt_mux);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_rslt_mux = start_idx; profile_rslt_mux <= stop_idx; profile_rslt_mux++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_mux_0_set(%u %u %u)\n", tcam_id, profile_rslt_mux,
                ram_line);
            ag_err = ag_drv_tcam_profiles_rslt_mux_0_set(tcam_id, profile_rslt_mux, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_rslt_mux);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_rslt_mux = 0; profile_rslt_mux < max_profile_rslt_mux; profile_rslt_mux++)
        {
            if (profile_rslt_mux < start_idx || profile_rslt_mux > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_rslt_mux_0_get(tcam_id, profile_rslt_mux, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_mux_0_get(%u %u %u)\n", tcam_id, profile_rslt_mux,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_rslt_mux);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_rslt_mux_0 completed. Number of tested entries %u.\n", max_profile_rslt_mux);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_rslt_mux_1:
    {
        uint8_t max_profile_rslt_mux = 4;
        uint8_t profile_rslt_mux = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_rslt_mux) || (stop_idx >= max_profile_rslt_mux))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_rslt_mux);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_rslt_mux = 0; profile_rslt_mux < max_profile_rslt_mux; profile_rslt_mux++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_mux_1_set(%u %u %u)\n", tcam_id, profile_rslt_mux,
                ram_line);
            ag_err = ag_drv_tcam_profiles_rslt_mux_1_set(tcam_id, profile_rslt_mux, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_rslt_mux);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_rslt_mux = start_idx; profile_rslt_mux <= stop_idx; profile_rslt_mux++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_mux_1_set(%u %u %u)\n", tcam_id, profile_rslt_mux,
                ram_line);
            ag_err = ag_drv_tcam_profiles_rslt_mux_1_set(tcam_id, profile_rslt_mux, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_rslt_mux);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_rslt_mux = 0; profile_rslt_mux < max_profile_rslt_mux; profile_rslt_mux++)
        {
            if (profile_rslt_mux < start_idx || profile_rslt_mux > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_rslt_mux_1_get(tcam_id, profile_rslt_mux, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_mux_1_get(%u %u %u)\n", tcam_id, profile_rslt_mux,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_rslt_mux);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_rslt_mux_1 completed. Number of tested entries %u.\n", max_profile_rslt_mux);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_rslt_mux_2:
    {
        uint8_t max_profile_rslt_mux = 4;
        uint8_t profile_rslt_mux = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_rslt_mux) || (stop_idx >= max_profile_rslt_mux))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_rslt_mux);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_rslt_mux = 0; profile_rslt_mux < max_profile_rslt_mux; profile_rslt_mux++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_mux_2_set(%u %u %u)\n", tcam_id, profile_rslt_mux,
                ram_line);
            ag_err = ag_drv_tcam_profiles_rslt_mux_2_set(tcam_id, profile_rslt_mux, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_rslt_mux);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_rslt_mux = start_idx; profile_rslt_mux <= stop_idx; profile_rslt_mux++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_mux_2_set(%u %u %u)\n", tcam_id, profile_rslt_mux,
                ram_line);
            ag_err = ag_drv_tcam_profiles_rslt_mux_2_set(tcam_id, profile_rslt_mux, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_rslt_mux);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_rslt_mux = 0; profile_rslt_mux < max_profile_rslt_mux; profile_rslt_mux++)
        {
            if (profile_rslt_mux < start_idx || profile_rslt_mux > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_rslt_mux_2_get(tcam_id, profile_rslt_mux, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_mux_2_get(%u %u %u)\n", tcam_id, profile_rslt_mux,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_rslt_mux);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_rslt_mux_2 completed. Number of tested entries %u.\n", max_profile_rslt_mux);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_rslt_mux_3:
    {
        uint8_t max_profile_rslt_mux = 4;
        uint8_t profile_rslt_mux = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_rslt_mux) || (stop_idx >= max_profile_rslt_mux))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_rslt_mux);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_rslt_mux = 0; profile_rslt_mux < max_profile_rslt_mux; profile_rslt_mux++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_mux_3_set(%u %u %u)\n", tcam_id, profile_rslt_mux,
                ram_line);
            ag_err = ag_drv_tcam_profiles_rslt_mux_3_set(tcam_id, profile_rslt_mux, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_rslt_mux);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_rslt_mux = start_idx; profile_rslt_mux <= stop_idx; profile_rslt_mux++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_mux_3_set(%u %u %u)\n", tcam_id, profile_rslt_mux,
                ram_line);
            ag_err = ag_drv_tcam_profiles_rslt_mux_3_set(tcam_id, profile_rslt_mux, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_rslt_mux);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_rslt_mux = 0; profile_rslt_mux < max_profile_rslt_mux; profile_rslt_mux++)
        {
            if (profile_rslt_mux < start_idx || profile_rslt_mux > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_rslt_mux_3_get(tcam_id, profile_rslt_mux, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_mux_3_get(%u %u %u)\n", tcam_id, profile_rslt_mux,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_rslt_mux);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_rslt_mux_3 completed. Number of tested entries %u.\n", max_profile_rslt_mux);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_rslt_builder_0:
    {
        uint8_t max_profile_rslt_builder = 4;
        uint8_t profile_rslt_builder = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_rslt_builder) || (stop_idx >= max_profile_rslt_builder))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_rslt_builder);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_rslt_builder = 0; profile_rslt_builder < max_profile_rslt_builder; profile_rslt_builder++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_0_set(%u %u %u)\n", tcam_id, profile_rslt_builder,
                ram_line);
            ag_err = ag_drv_tcam_profiles_rslt_builder_0_set(tcam_id, profile_rslt_builder, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_rslt_builder);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_rslt_builder = start_idx; profile_rslt_builder <= stop_idx; profile_rslt_builder++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_0_set(%u %u %u)\n", tcam_id, profile_rslt_builder,
                ram_line);
            ag_err = ag_drv_tcam_profiles_rslt_builder_0_set(tcam_id, profile_rslt_builder, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_rslt_builder);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_rslt_builder = 0; profile_rslt_builder < max_profile_rslt_builder; profile_rslt_builder++)
        {
            if (profile_rslt_builder < start_idx || profile_rslt_builder > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_rslt_builder_0_get(tcam_id, profile_rslt_builder, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_0_get(%u %u %u)\n", tcam_id, profile_rslt_builder,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_rslt_builder);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_rslt_builder_0 completed. Number of tested entries %u.\n", max_profile_rslt_builder);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_rslt_builder_1:
    {
        uint8_t max_profile_rslt_builder = 4;
        uint8_t profile_rslt_builder = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_rslt_builder) || (stop_idx >= max_profile_rslt_builder))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_rslt_builder);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_rslt_builder = 0; profile_rslt_builder < max_profile_rslt_builder; profile_rslt_builder++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_1_set(%u %u %u)\n", tcam_id, profile_rslt_builder,
                ram_line);
            ag_err = ag_drv_tcam_profiles_rslt_builder_1_set(tcam_id, profile_rslt_builder, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_rslt_builder);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_rslt_builder = start_idx; profile_rslt_builder <= stop_idx; profile_rslt_builder++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_1_set(%u %u %u)\n", tcam_id, profile_rslt_builder,
                ram_line);
            ag_err = ag_drv_tcam_profiles_rslt_builder_1_set(tcam_id, profile_rslt_builder, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_rslt_builder);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_rslt_builder = 0; profile_rslt_builder < max_profile_rslt_builder; profile_rslt_builder++)
        {
            if (profile_rslt_builder < start_idx || profile_rslt_builder > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_rslt_builder_1_get(tcam_id, profile_rslt_builder, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_1_get(%u %u %u)\n", tcam_id, profile_rslt_builder,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_rslt_builder);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_rslt_builder_1 completed. Number of tested entries %u.\n", max_profile_rslt_builder);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_rslt_builder_2:
    {
        uint8_t max_profile_rslt_builder = 4;
        uint8_t profile_rslt_builder = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_rslt_builder) || (stop_idx >= max_profile_rslt_builder))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_rslt_builder);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_rslt_builder = 0; profile_rslt_builder < max_profile_rslt_builder; profile_rslt_builder++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_2_set(%u %u %u)\n", tcam_id, profile_rslt_builder,
                ram_line);
            ag_err = ag_drv_tcam_profiles_rslt_builder_2_set(tcam_id, profile_rslt_builder, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_rslt_builder);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_rslt_builder = start_idx; profile_rslt_builder <= stop_idx; profile_rslt_builder++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_2_set(%u %u %u)\n", tcam_id, profile_rslt_builder,
                ram_line);
            ag_err = ag_drv_tcam_profiles_rslt_builder_2_set(tcam_id, profile_rslt_builder, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_rslt_builder);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_rslt_builder = 0; profile_rslt_builder < max_profile_rslt_builder; profile_rslt_builder++)
        {
            if (profile_rslt_builder < start_idx || profile_rslt_builder > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_rslt_builder_2_get(tcam_id, profile_rslt_builder, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_2_get(%u %u %u)\n", tcam_id, profile_rslt_builder,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_rslt_builder);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_rslt_builder_2 completed. Number of tested entries %u.\n", max_profile_rslt_builder);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_rslt_builder_3:
    {
        uint8_t max_profile_rslt_builder = 4;
        uint8_t profile_rslt_builder = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_rslt_builder) || (stop_idx >= max_profile_rslt_builder))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_rslt_builder);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_rslt_builder = 0; profile_rslt_builder < max_profile_rslt_builder; profile_rslt_builder++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_3_set(%u %u %u)\n", tcam_id, profile_rslt_builder,
                ram_line);
            ag_err = ag_drv_tcam_profiles_rslt_builder_3_set(tcam_id, profile_rslt_builder, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_rslt_builder);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_rslt_builder = start_idx; profile_rslt_builder <= stop_idx; profile_rslt_builder++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_3_set(%u %u %u)\n", tcam_id, profile_rslt_builder,
                ram_line);
            ag_err = ag_drv_tcam_profiles_rslt_builder_3_set(tcam_id, profile_rslt_builder, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_rslt_builder);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_rslt_builder = 0; profile_rslt_builder < max_profile_rslt_builder; profile_rslt_builder++)
        {
            if (profile_rslt_builder < start_idx || profile_rslt_builder > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_rslt_builder_3_get(tcam_id, profile_rslt_builder, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_3_get(%u %u %u)\n", tcam_id, profile_rslt_builder,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_rslt_builder);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_rslt_builder_3 completed. Number of tested entries %u.\n", max_profile_rslt_builder);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_rslt_builder_4:
    {
        uint8_t max_profile_rslt_builder = 4;
        uint8_t profile_rslt_builder = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_rslt_builder) || (stop_idx >= max_profile_rslt_builder))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_rslt_builder);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_rslt_builder = 0; profile_rslt_builder < max_profile_rslt_builder; profile_rslt_builder++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_4_set(%u %u %u)\n", tcam_id, profile_rslt_builder,
                ram_line);
            ag_err = ag_drv_tcam_profiles_rslt_builder_4_set(tcam_id, profile_rslt_builder, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_rslt_builder);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_rslt_builder = start_idx; profile_rslt_builder <= stop_idx; profile_rslt_builder++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_4_set(%u %u %u)\n", tcam_id, profile_rslt_builder,
                ram_line);
            ag_err = ag_drv_tcam_profiles_rslt_builder_4_set(tcam_id, profile_rslt_builder, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_rslt_builder);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_rslt_builder = 0; profile_rslt_builder < max_profile_rslt_builder; profile_rslt_builder++)
        {
            if (profile_rslt_builder < start_idx || profile_rslt_builder > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_rslt_builder_4_get(tcam_id, profile_rslt_builder, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_4_get(%u %u %u)\n", tcam_id, profile_rslt_builder,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_rslt_builder);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_rslt_builder_4 completed. Number of tested entries %u.\n", max_profile_rslt_builder);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_rslt_builder_5:
    {
        uint8_t max_profile_rslt_builder = 4;
        uint8_t profile_rslt_builder = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_rslt_builder) || (stop_idx >= max_profile_rslt_builder))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_rslt_builder);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_rslt_builder = 0; profile_rslt_builder < max_profile_rslt_builder; profile_rslt_builder++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_5_set(%u %u %u)\n", tcam_id, profile_rslt_builder,
                ram_line);
            ag_err = ag_drv_tcam_profiles_rslt_builder_5_set(tcam_id, profile_rslt_builder, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_rslt_builder);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_rslt_builder = start_idx; profile_rslt_builder <= stop_idx; profile_rslt_builder++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_5_set(%u %u %u)\n", tcam_id, profile_rslt_builder,
                ram_line);
            ag_err = ag_drv_tcam_profiles_rslt_builder_5_set(tcam_id, profile_rslt_builder, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_rslt_builder);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_rslt_builder = 0; profile_rslt_builder < max_profile_rslt_builder; profile_rslt_builder++)
        {
            if (profile_rslt_builder < start_idx || profile_rslt_builder > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_rslt_builder_5_get(tcam_id, profile_rslt_builder, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_5_get(%u %u %u)\n", tcam_id, profile_rslt_builder,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_rslt_builder);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_rslt_builder_5 completed. Number of tested entries %u.\n", max_profile_rslt_builder);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_rslt_builder_6:
    {
        uint8_t max_profile_rslt_builder = 4;
        uint8_t profile_rslt_builder = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_rslt_builder) || (stop_idx >= max_profile_rslt_builder))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_rslt_builder);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_rslt_builder = 0; profile_rslt_builder < max_profile_rslt_builder; profile_rslt_builder++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_6_set(%u %u %u)\n", tcam_id, profile_rslt_builder,
                ram_line);
            ag_err = ag_drv_tcam_profiles_rslt_builder_6_set(tcam_id, profile_rslt_builder, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_rslt_builder);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_rslt_builder = start_idx; profile_rslt_builder <= stop_idx; profile_rslt_builder++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_6_set(%u %u %u)\n", tcam_id, profile_rslt_builder,
                ram_line);
            ag_err = ag_drv_tcam_profiles_rslt_builder_6_set(tcam_id, profile_rslt_builder, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_rslt_builder);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_rslt_builder = 0; profile_rslt_builder < max_profile_rslt_builder; profile_rslt_builder++)
        {
            if (profile_rslt_builder < start_idx || profile_rslt_builder > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_rslt_builder_6_get(tcam_id, profile_rslt_builder, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_6_get(%u %u %u)\n", tcam_id, profile_rslt_builder,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_rslt_builder);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_rslt_builder_6 completed. Number of tested entries %u.\n", max_profile_rslt_builder);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_rslt_builder_7:
    {
        uint8_t max_profile_rslt_builder = 4;
        uint8_t profile_rslt_builder = gtmv(m, 2);
        uint32_t ram_line = gtmv(m, 32);

        if ((start_idx >= max_profile_rslt_builder) || (stop_idx >= max_profile_rslt_builder))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_rslt_builder);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_rslt_builder = 0; profile_rslt_builder < max_profile_rslt_builder; profile_rslt_builder++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_7_set(%u %u %u)\n", tcam_id, profile_rslt_builder,
                ram_line);
            ag_err = ag_drv_tcam_profiles_rslt_builder_7_set(tcam_id, profile_rslt_builder, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_rslt_builder);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        ram_line = gtmv(m, 32);

        for (profile_rslt_builder = start_idx; profile_rslt_builder <= stop_idx; profile_rslt_builder++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_7_set(%u %u %u)\n", tcam_id, profile_rslt_builder,
                ram_line);
            ag_err = ag_drv_tcam_profiles_rslt_builder_7_set(tcam_id, profile_rslt_builder, ram_line);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_rslt_builder);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_rslt_builder = 0; profile_rslt_builder < max_profile_rslt_builder; profile_rslt_builder++)
        {
            if (profile_rslt_builder < start_idx || profile_rslt_builder > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_rslt_builder_7_get(tcam_id, profile_rslt_builder, &ram_line);

            bdmf_session_print(session, "ag_drv_tcam_profiles_rslt_builder_7_get(%u %u %u)\n", tcam_id, profile_rslt_builder,
                ram_line);

            if (ram_line != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_rslt_builder);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_rslt_builder_7 completed. Number of tested entries %u.\n", max_profile_rslt_builder);
        bdmf_session_print(session, "SUCCESSES: %u, FAILURES: %u\n", ext_test_success_cnt, ext_test_failure_cnt);
        bdmf_session_print(session, "========================================================================\n\n");

        break;
    }
    case cli_tcam_profiles_tcam_concat:
    {
        uint8_t max_profile_tcam_concat = 4;
        uint8_t profile_tcam_concat = gtmv(m, 2);
        uint16_t en = gtmv(m, 16);

        if ((start_idx >= max_profile_tcam_concat) || (stop_idx >= max_profile_tcam_concat))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_profile_tcam_concat);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (profile_tcam_concat = 0; profile_tcam_concat < max_profile_tcam_concat; profile_tcam_concat++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_tcam_concat_set(%u %u %u)\n", tcam_id, profile_tcam_concat,
                en);
            ag_err = ag_drv_tcam_profiles_tcam_concat_set(tcam_id, profile_tcam_concat, en);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", profile_tcam_concat);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        en = gtmv(m, 16);

        for (profile_tcam_concat = start_idx; profile_tcam_concat <= stop_idx; profile_tcam_concat++)
        {
            bdmf_session_print(session, "ag_drv_tcam_profiles_tcam_concat_set(%u %u %u)\n", tcam_id, profile_tcam_concat,
                en);
            ag_err = ag_drv_tcam_profiles_tcam_concat_set(tcam_id, profile_tcam_concat, en);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", profile_tcam_concat);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (profile_tcam_concat = 0; profile_tcam_concat < max_profile_tcam_concat; profile_tcam_concat++)
        {
            if (profile_tcam_concat < start_idx || profile_tcam_concat > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_tcam_profiles_tcam_concat_get(tcam_id, profile_tcam_concat, &en);

            bdmf_session_print(session, "ag_drv_tcam_profiles_tcam_concat_get(%u %u %u)\n", tcam_id, profile_tcam_concat,
                en);

            if (en != gtmv(m, 16) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", profile_tcam_concat);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of profiles_tcam_concat completed. Number of tested entries %u.\n", max_profile_tcam_concat);
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
static int ag_drv_tcam_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_cfg_dinm: reg = &RU_REG(TCAM, CFG_DINM); blk = &RU_BLK(TCAM); break;
    case bdmf_address_cfg_blksel: reg = &RU_REG(TCAM, CFG_BLKSEL); blk = &RU_BLK(TCAM); break;
    case bdmf_address_cfg_tm: reg = &RU_REG(TCAM, CFG_TM); blk = &RU_BLK(TCAM); break;
    case bdmf_address_cfg_vreset: reg = &RU_REG(TCAM, CFG_VRESET); blk = &RU_BLK(TCAM); break;
    case bdmf_address_counters_searches: reg = &RU_REG(TCAM, COUNTERS_SEARCHES); blk = &RU_BLK(TCAM); break;
    case bdmf_address_counters_hits: reg = &RU_REG(TCAM, COUNTERS_HITS); blk = &RU_BLK(TCAM); break;
    case bdmf_address_tcam_mem_indirect_op: reg = &RU_REG(TCAM, TCAM_MEM_INDIRECT_OP); blk = &RU_BLK(TCAM); break;
    case bdmf_address_tcam_mem_indirect_op_done: reg = &RU_REG(TCAM, TCAM_MEM_INDIRECT_OP_DONE); blk = &RU_BLK(TCAM); break;
    case bdmf_address_tcam_mem_indirect_addr: reg = &RU_REG(TCAM, TCAM_MEM_INDIRECT_ADDR); blk = &RU_BLK(TCAM); break;
    case bdmf_address_tcam_mem_indirect_valid_in: reg = &RU_REG(TCAM, TCAM_MEM_INDIRECT_VALID_IN); blk = &RU_BLK(TCAM); break;
    case bdmf_address_tcam_mem_indirect_valid_out: reg = &RU_REG(TCAM, TCAM_MEM_INDIRECT_VALID_OUT); blk = &RU_BLK(TCAM); break;
    case bdmf_address_tcam_mem_indirect_data_out: reg = &RU_REG(TCAM, TCAM_MEM_INDIRECT_DATA_OUT); blk = &RU_BLK(TCAM); break;
    case bdmf_address_tcam_mem_indirect_data_in: reg = &RU_REG(TCAM, TCAM_MEM_INDIRECT_DATA_IN); blk = &RU_BLK(TCAM); break;
    case bdmf_address_tcam_debug_bus_select: reg = &RU_REG(TCAM, TCAM_DEBUG_BUS_SELECT); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_0: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_0); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_1: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_1); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_2: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_2); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_3: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_3); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_4: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_4); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_5: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_5); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_6: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_6); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_7: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_7); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_8: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_8); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_9: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_9); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_10: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_10); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_11: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_11); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_12: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_12); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_13: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_13); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_14: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_14); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_15: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_15); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_16: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_16); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_17: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_17); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_18: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_18); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_19: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_19); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_20: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_20); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_21: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_21); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_22: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_22); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_23: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_23); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_24: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_24); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_25: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_25); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_26: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_26); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_27: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_27); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_28: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_28); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_29: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_29); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_30: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_30); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_31: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_31); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_32: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_32); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_33: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_33); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_34: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_34); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_35: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_35); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_36: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_36); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_37: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_37); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_38: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_38); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_key_builder_offsets_39: reg = &RU_REG(TCAM, PROFILES_KEY_BUILDER_OFFSETS_39); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_inner_shift_0: reg = &RU_REG(TCAM, PROFILES_INNER_SHIFT_0); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_inner_shift_1: reg = &RU_REG(TCAM, PROFILES_INNER_SHIFT_1); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_inner_shift_2: reg = &RU_REG(TCAM, PROFILES_INNER_SHIFT_2); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_inner_shift_3: reg = &RU_REG(TCAM, PROFILES_INNER_SHIFT_3); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_inner_shift_4: reg = &RU_REG(TCAM, PROFILES_INNER_SHIFT_4); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_inner_shift_5: reg = &RU_REG(TCAM, PROFILES_INNER_SHIFT_5); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_inner_shift_6: reg = &RU_REG(TCAM, PROFILES_INNER_SHIFT_6); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_inner_shift_7: reg = &RU_REG(TCAM, PROFILES_INNER_SHIFT_7); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_inner_shift_8: reg = &RU_REG(TCAM, PROFILES_INNER_SHIFT_8); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_inner_shift_9: reg = &RU_REG(TCAM, PROFILES_INNER_SHIFT_9); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_inner_shift_10: reg = &RU_REG(TCAM, PROFILES_INNER_SHIFT_10); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_inner_shift_11: reg = &RU_REG(TCAM, PROFILES_INNER_SHIFT_11); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_inner_shift_12: reg = &RU_REG(TCAM, PROFILES_INNER_SHIFT_12); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_inner_shift_13: reg = &RU_REG(TCAM, PROFILES_INNER_SHIFT_13); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_inner_shift_14: reg = &RU_REG(TCAM, PROFILES_INNER_SHIFT_14); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_inner_shift_15: reg = &RU_REG(TCAM, PROFILES_INNER_SHIFT_15); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_inner_shift_16: reg = &RU_REG(TCAM, PROFILES_INNER_SHIFT_16); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_inner_shift_17: reg = &RU_REG(TCAM, PROFILES_INNER_SHIFT_17); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_inner_shift_18: reg = &RU_REG(TCAM, PROFILES_INNER_SHIFT_18); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_inner_shift_19: reg = &RU_REG(TCAM, PROFILES_INNER_SHIFT_19); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_inner_shift_20: reg = &RU_REG(TCAM, PROFILES_INNER_SHIFT_20); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_inner_shift_21: reg = &RU_REG(TCAM, PROFILES_INNER_SHIFT_21); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_inner_shift_22: reg = &RU_REG(TCAM, PROFILES_INNER_SHIFT_22); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_inner_shift_23: reg = &RU_REG(TCAM, PROFILES_INNER_SHIFT_23); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_inner_shift_24: reg = &RU_REG(TCAM, PROFILES_INNER_SHIFT_24); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_inner_shift_25: reg = &RU_REG(TCAM, PROFILES_INNER_SHIFT_25); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_inner_shift_26: reg = &RU_REG(TCAM, PROFILES_INNER_SHIFT_26); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_inner_shift_27: reg = &RU_REG(TCAM, PROFILES_INNER_SHIFT_27); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_inner_shift_28: reg = &RU_REG(TCAM, PROFILES_INNER_SHIFT_28); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_inner_shift_29: reg = &RU_REG(TCAM, PROFILES_INNER_SHIFT_29); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_inner_shift_30: reg = &RU_REG(TCAM, PROFILES_INNER_SHIFT_30); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_inner_shift_31: reg = &RU_REG(TCAM, PROFILES_INNER_SHIFT_31); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_cntxin_sp_id_0: reg = &RU_REG(TCAM, PROFILES_CNTXIN_SP_ID_0); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_cntxin_sp_id_1: reg = &RU_REG(TCAM, PROFILES_CNTXIN_SP_ID_1); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_cntxin_mask_0: reg = &RU_REG(TCAM, PROFILES_CNTXIN_MASK_0); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_cntxin_mask_1: reg = &RU_REG(TCAM, PROFILES_CNTXIN_MASK_1); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_cntxin_mask_2: reg = &RU_REG(TCAM, PROFILES_CNTXIN_MASK_2); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_cntxin_mask_3: reg = &RU_REG(TCAM, PROFILES_CNTXIN_MASK_3); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_cntxin_mask_4: reg = &RU_REG(TCAM, PROFILES_CNTXIN_MASK_4); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_cntxin_mask_5: reg = &RU_REG(TCAM, PROFILES_CNTXIN_MASK_5); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_cntxin_mask_6: reg = &RU_REG(TCAM, PROFILES_CNTXIN_MASK_6); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_cntxin_mask_7: reg = &RU_REG(TCAM, PROFILES_CNTXIN_MASK_7); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_sp_mask_0: reg = &RU_REG(TCAM, PROFILES_SP_MASK_0); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_sp_mask_1: reg = &RU_REG(TCAM, PROFILES_SP_MASK_1); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_sp_mask_2: reg = &RU_REG(TCAM, PROFILES_SP_MASK_2); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_sp_mask_3: reg = &RU_REG(TCAM, PROFILES_SP_MASK_3); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_rslt_mux_0: reg = &RU_REG(TCAM, PROFILES_RSLT_MUX_0); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_rslt_mux_1: reg = &RU_REG(TCAM, PROFILES_RSLT_MUX_1); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_rslt_mux_2: reg = &RU_REG(TCAM, PROFILES_RSLT_MUX_2); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_rslt_mux_3: reg = &RU_REG(TCAM, PROFILES_RSLT_MUX_3); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_rslt_builder_0: reg = &RU_REG(TCAM, PROFILES_RSLT_BUILDER_0); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_rslt_builder_1: reg = &RU_REG(TCAM, PROFILES_RSLT_BUILDER_1); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_rslt_builder_2: reg = &RU_REG(TCAM, PROFILES_RSLT_BUILDER_2); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_rslt_builder_3: reg = &RU_REG(TCAM, PROFILES_RSLT_BUILDER_3); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_rslt_builder_4: reg = &RU_REG(TCAM, PROFILES_RSLT_BUILDER_4); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_rslt_builder_5: reg = &RU_REG(TCAM, PROFILES_RSLT_BUILDER_5); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_rslt_builder_6: reg = &RU_REG(TCAM, PROFILES_RSLT_BUILDER_6); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_rslt_builder_7: reg = &RU_REG(TCAM, PROFILES_RSLT_BUILDER_7); blk = &RU_BLK(TCAM); break;
    case bdmf_address_profiles_tcam_concat: reg = &RU_REG(TCAM, PROFILES_TCAM_CONCAT); blk = &RU_BLK(TCAM); break;
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

bdmfmon_handle_t ag_drv_tcam_cli_init(bdmfmon_handle_t root_dir)
{
    bdmfmon_handle_t dir;

    dir = bdmfmon_dir_add(root_dir, "tcam", "tcam", BDMF_ACCESS_ADMIN, NULL);
    BUG_ON(dir == NULL);

    {
        static bdmfmon_cmd_parm_t set_cfg_dinm[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cfg_dinm", "cfg_dinm", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_blksel[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cfg_blksel", "cfg_blksel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_tm[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cfg_tm", "cfg_tm", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cfg_vreset[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("val", "val", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tcam_mem_indirect_op[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cmd", "cmd", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tcam_mem_indirect_addr[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("entry_addr", "entry_addr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mem_id", "mem_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tcam_mem_indirect_valid_in[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("valid", "valid", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tcam_mem_indirect_data_in[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("indirect_data_in", "indirect_data_in", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("value", "value", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tcam_debug_bus_select[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("select_module", "select_module", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_0[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_1[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_2[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_3[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_4[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_5[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_6[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_7[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_8[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_9[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_10[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_11[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_12[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_13[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_14[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_15[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_16[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_17[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_18[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_19[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_20[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_21[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_22[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_23[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_24[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_25[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_26[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_27[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_28[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_29[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_30[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_31[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_32[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_33[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_34[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_35[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_36[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_37[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_38[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_key_builder_offsets_39[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_inner_shift_0[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_inner_shift_1[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_inner_shift_2[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_inner_shift_3[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_inner_shift_4[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_inner_shift_5[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_inner_shift_6[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_inner_shift_7[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_inner_shift_8[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_inner_shift_9[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_inner_shift_10[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_inner_shift_11[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_inner_shift_12[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_inner_shift_13[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_inner_shift_14[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_inner_shift_15[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_inner_shift_16[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_inner_shift_17[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_inner_shift_18[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_inner_shift_19[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_inner_shift_20[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_inner_shift_21[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_inner_shift_22[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_inner_shift_23[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_inner_shift_24[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_inner_shift_25[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_inner_shift_26[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_inner_shift_27[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_inner_shift_28[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_inner_shift_29[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_inner_shift_30[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_inner_shift_31[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_cntxin_sp_id_0[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_sp_id", "profile_sp_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_cntxin_sp_id_1[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_sp_id", "profile_sp_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_cntxin_mask_0[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_cntxin_mask", "profile_cntxin_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_cntxin_mask_1[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_cntxin_mask", "profile_cntxin_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_cntxin_mask_2[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_cntxin_mask", "profile_cntxin_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_cntxin_mask_3[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_cntxin_mask", "profile_cntxin_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_cntxin_mask_4[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_cntxin_mask", "profile_cntxin_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_cntxin_mask_5[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_cntxin_mask", "profile_cntxin_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_cntxin_mask_6[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_cntxin_mask", "profile_cntxin_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_cntxin_mask_7[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_cntxin_mask", "profile_cntxin_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_sp_mask_0[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_sp_mask", "profile_sp_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_sp_mask_1[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_sp_mask", "profile_sp_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_sp_mask_2[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_sp_mask", "profile_sp_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_sp_mask_3[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_sp_mask", "profile_sp_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_rslt_mux_0[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_rslt_mux", "profile_rslt_mux", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_rslt_mux_1[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_rslt_mux", "profile_rslt_mux", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_rslt_mux_2[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_rslt_mux", "profile_rslt_mux", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_rslt_mux_3[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_rslt_mux", "profile_rslt_mux", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_rslt_builder_0[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_rslt_builder", "profile_rslt_builder", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_rslt_builder_1[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_rslt_builder", "profile_rslt_builder", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_rslt_builder_2[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_rslt_builder", "profile_rslt_builder", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_rslt_builder_3[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_rslt_builder", "profile_rslt_builder", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_rslt_builder_4[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_rslt_builder", "profile_rslt_builder", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_rslt_builder_5[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_rslt_builder", "profile_rslt_builder", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_rslt_builder_6[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_rslt_builder", "profile_rslt_builder", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_rslt_builder_7[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_rslt_builder", "profile_rslt_builder", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ram_line", "ram_line", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_profiles_tcam_concat[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_tcam_concat", "profile_tcam_concat", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("en", "en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "cfg_dinm", .val = cli_tcam_cfg_dinm, .parms = set_cfg_dinm },
            { .name = "cfg_blksel", .val = cli_tcam_cfg_blksel, .parms = set_cfg_blksel },
            { .name = "cfg_tm", .val = cli_tcam_cfg_tm, .parms = set_cfg_tm },
            { .name = "cfg_vreset", .val = cli_tcam_cfg_vreset, .parms = set_cfg_vreset },
            { .name = "tcam_mem_indirect_op", .val = cli_tcam_tcam_mem_indirect_op, .parms = set_tcam_mem_indirect_op },
            { .name = "tcam_mem_indirect_addr", .val = cli_tcam_tcam_mem_indirect_addr, .parms = set_tcam_mem_indirect_addr },
            { .name = "tcam_mem_indirect_valid_in", .val = cli_tcam_tcam_mem_indirect_valid_in, .parms = set_tcam_mem_indirect_valid_in },
            { .name = "tcam_mem_indirect_data_in", .val = cli_tcam_tcam_mem_indirect_data_in, .parms = set_tcam_mem_indirect_data_in },
            { .name = "tcam_debug_bus_select", .val = cli_tcam_tcam_debug_bus_select, .parms = set_tcam_debug_bus_select },
            { .name = "profiles_key_builder_offsets_0", .val = cli_tcam_profiles_key_builder_offsets_0, .parms = set_profiles_key_builder_offsets_0 },
            { .name = "profiles_key_builder_offsets_1", .val = cli_tcam_profiles_key_builder_offsets_1, .parms = set_profiles_key_builder_offsets_1 },
            { .name = "profiles_key_builder_offsets_2", .val = cli_tcam_profiles_key_builder_offsets_2, .parms = set_profiles_key_builder_offsets_2 },
            { .name = "profiles_key_builder_offsets_3", .val = cli_tcam_profiles_key_builder_offsets_3, .parms = set_profiles_key_builder_offsets_3 },
            { .name = "profiles_key_builder_offsets_4", .val = cli_tcam_profiles_key_builder_offsets_4, .parms = set_profiles_key_builder_offsets_4 },
            { .name = "profiles_key_builder_offsets_5", .val = cli_tcam_profiles_key_builder_offsets_5, .parms = set_profiles_key_builder_offsets_5 },
            { .name = "profiles_key_builder_offsets_6", .val = cli_tcam_profiles_key_builder_offsets_6, .parms = set_profiles_key_builder_offsets_6 },
            { .name = "profiles_key_builder_offsets_7", .val = cli_tcam_profiles_key_builder_offsets_7, .parms = set_profiles_key_builder_offsets_7 },
            { .name = "profiles_key_builder_offsets_8", .val = cli_tcam_profiles_key_builder_offsets_8, .parms = set_profiles_key_builder_offsets_8 },
            { .name = "profiles_key_builder_offsets_9", .val = cli_tcam_profiles_key_builder_offsets_9, .parms = set_profiles_key_builder_offsets_9 },
            { .name = "profiles_key_builder_offsets_10", .val = cli_tcam_profiles_key_builder_offsets_10, .parms = set_profiles_key_builder_offsets_10 },
            { .name = "profiles_key_builder_offsets_11", .val = cli_tcam_profiles_key_builder_offsets_11, .parms = set_profiles_key_builder_offsets_11 },
            { .name = "profiles_key_builder_offsets_12", .val = cli_tcam_profiles_key_builder_offsets_12, .parms = set_profiles_key_builder_offsets_12 },
            { .name = "profiles_key_builder_offsets_13", .val = cli_tcam_profiles_key_builder_offsets_13, .parms = set_profiles_key_builder_offsets_13 },
            { .name = "profiles_key_builder_offsets_14", .val = cli_tcam_profiles_key_builder_offsets_14, .parms = set_profiles_key_builder_offsets_14 },
            { .name = "profiles_key_builder_offsets_15", .val = cli_tcam_profiles_key_builder_offsets_15, .parms = set_profiles_key_builder_offsets_15 },
            { .name = "profiles_key_builder_offsets_16", .val = cli_tcam_profiles_key_builder_offsets_16, .parms = set_profiles_key_builder_offsets_16 },
            { .name = "profiles_key_builder_offsets_17", .val = cli_tcam_profiles_key_builder_offsets_17, .parms = set_profiles_key_builder_offsets_17 },
            { .name = "profiles_key_builder_offsets_18", .val = cli_tcam_profiles_key_builder_offsets_18, .parms = set_profiles_key_builder_offsets_18 },
            { .name = "profiles_key_builder_offsets_19", .val = cli_tcam_profiles_key_builder_offsets_19, .parms = set_profiles_key_builder_offsets_19 },
            { .name = "profiles_key_builder_offsets_20", .val = cli_tcam_profiles_key_builder_offsets_20, .parms = set_profiles_key_builder_offsets_20 },
            { .name = "profiles_key_builder_offsets_21", .val = cli_tcam_profiles_key_builder_offsets_21, .parms = set_profiles_key_builder_offsets_21 },
            { .name = "profiles_key_builder_offsets_22", .val = cli_tcam_profiles_key_builder_offsets_22, .parms = set_profiles_key_builder_offsets_22 },
            { .name = "profiles_key_builder_offsets_23", .val = cli_tcam_profiles_key_builder_offsets_23, .parms = set_profiles_key_builder_offsets_23 },
            { .name = "profiles_key_builder_offsets_24", .val = cli_tcam_profiles_key_builder_offsets_24, .parms = set_profiles_key_builder_offsets_24 },
            { .name = "profiles_key_builder_offsets_25", .val = cli_tcam_profiles_key_builder_offsets_25, .parms = set_profiles_key_builder_offsets_25 },
            { .name = "profiles_key_builder_offsets_26", .val = cli_tcam_profiles_key_builder_offsets_26, .parms = set_profiles_key_builder_offsets_26 },
            { .name = "profiles_key_builder_offsets_27", .val = cli_tcam_profiles_key_builder_offsets_27, .parms = set_profiles_key_builder_offsets_27 },
            { .name = "profiles_key_builder_offsets_28", .val = cli_tcam_profiles_key_builder_offsets_28, .parms = set_profiles_key_builder_offsets_28 },
            { .name = "profiles_key_builder_offsets_29", .val = cli_tcam_profiles_key_builder_offsets_29, .parms = set_profiles_key_builder_offsets_29 },
            { .name = "profiles_key_builder_offsets_30", .val = cli_tcam_profiles_key_builder_offsets_30, .parms = set_profiles_key_builder_offsets_30 },
            { .name = "profiles_key_builder_offsets_31", .val = cli_tcam_profiles_key_builder_offsets_31, .parms = set_profiles_key_builder_offsets_31 },
            { .name = "profiles_key_builder_offsets_32", .val = cli_tcam_profiles_key_builder_offsets_32, .parms = set_profiles_key_builder_offsets_32 },
            { .name = "profiles_key_builder_offsets_33", .val = cli_tcam_profiles_key_builder_offsets_33, .parms = set_profiles_key_builder_offsets_33 },
            { .name = "profiles_key_builder_offsets_34", .val = cli_tcam_profiles_key_builder_offsets_34, .parms = set_profiles_key_builder_offsets_34 },
            { .name = "profiles_key_builder_offsets_35", .val = cli_tcam_profiles_key_builder_offsets_35, .parms = set_profiles_key_builder_offsets_35 },
            { .name = "profiles_key_builder_offsets_36", .val = cli_tcam_profiles_key_builder_offsets_36, .parms = set_profiles_key_builder_offsets_36 },
            { .name = "profiles_key_builder_offsets_37", .val = cli_tcam_profiles_key_builder_offsets_37, .parms = set_profiles_key_builder_offsets_37 },
            { .name = "profiles_key_builder_offsets_38", .val = cli_tcam_profiles_key_builder_offsets_38, .parms = set_profiles_key_builder_offsets_38 },
            { .name = "profiles_key_builder_offsets_39", .val = cli_tcam_profiles_key_builder_offsets_39, .parms = set_profiles_key_builder_offsets_39 },
            { .name = "profiles_inner_shift_0", .val = cli_tcam_profiles_inner_shift_0, .parms = set_profiles_inner_shift_0 },
            { .name = "profiles_inner_shift_1", .val = cli_tcam_profiles_inner_shift_1, .parms = set_profiles_inner_shift_1 },
            { .name = "profiles_inner_shift_2", .val = cli_tcam_profiles_inner_shift_2, .parms = set_profiles_inner_shift_2 },
            { .name = "profiles_inner_shift_3", .val = cli_tcam_profiles_inner_shift_3, .parms = set_profiles_inner_shift_3 },
            { .name = "profiles_inner_shift_4", .val = cli_tcam_profiles_inner_shift_4, .parms = set_profiles_inner_shift_4 },
            { .name = "profiles_inner_shift_5", .val = cli_tcam_profiles_inner_shift_5, .parms = set_profiles_inner_shift_5 },
            { .name = "profiles_inner_shift_6", .val = cli_tcam_profiles_inner_shift_6, .parms = set_profiles_inner_shift_6 },
            { .name = "profiles_inner_shift_7", .val = cli_tcam_profiles_inner_shift_7, .parms = set_profiles_inner_shift_7 },
            { .name = "profiles_inner_shift_8", .val = cli_tcam_profiles_inner_shift_8, .parms = set_profiles_inner_shift_8 },
            { .name = "profiles_inner_shift_9", .val = cli_tcam_profiles_inner_shift_9, .parms = set_profiles_inner_shift_9 },
            { .name = "profiles_inner_shift_10", .val = cli_tcam_profiles_inner_shift_10, .parms = set_profiles_inner_shift_10 },
            { .name = "profiles_inner_shift_11", .val = cli_tcam_profiles_inner_shift_11, .parms = set_profiles_inner_shift_11 },
            { .name = "profiles_inner_shift_12", .val = cli_tcam_profiles_inner_shift_12, .parms = set_profiles_inner_shift_12 },
            { .name = "profiles_inner_shift_13", .val = cli_tcam_profiles_inner_shift_13, .parms = set_profiles_inner_shift_13 },
            { .name = "profiles_inner_shift_14", .val = cli_tcam_profiles_inner_shift_14, .parms = set_profiles_inner_shift_14 },
            { .name = "profiles_inner_shift_15", .val = cli_tcam_profiles_inner_shift_15, .parms = set_profiles_inner_shift_15 },
            { .name = "profiles_inner_shift_16", .val = cli_tcam_profiles_inner_shift_16, .parms = set_profiles_inner_shift_16 },
            { .name = "profiles_inner_shift_17", .val = cli_tcam_profiles_inner_shift_17, .parms = set_profiles_inner_shift_17 },
            { .name = "profiles_inner_shift_18", .val = cli_tcam_profiles_inner_shift_18, .parms = set_profiles_inner_shift_18 },
            { .name = "profiles_inner_shift_19", .val = cli_tcam_profiles_inner_shift_19, .parms = set_profiles_inner_shift_19 },
            { .name = "profiles_inner_shift_20", .val = cli_tcam_profiles_inner_shift_20, .parms = set_profiles_inner_shift_20 },
            { .name = "profiles_inner_shift_21", .val = cli_tcam_profiles_inner_shift_21, .parms = set_profiles_inner_shift_21 },
            { .name = "profiles_inner_shift_22", .val = cli_tcam_profiles_inner_shift_22, .parms = set_profiles_inner_shift_22 },
            { .name = "profiles_inner_shift_23", .val = cli_tcam_profiles_inner_shift_23, .parms = set_profiles_inner_shift_23 },
            { .name = "profiles_inner_shift_24", .val = cli_tcam_profiles_inner_shift_24, .parms = set_profiles_inner_shift_24 },
            { .name = "profiles_inner_shift_25", .val = cli_tcam_profiles_inner_shift_25, .parms = set_profiles_inner_shift_25 },
            { .name = "profiles_inner_shift_26", .val = cli_tcam_profiles_inner_shift_26, .parms = set_profiles_inner_shift_26 },
            { .name = "profiles_inner_shift_27", .val = cli_tcam_profiles_inner_shift_27, .parms = set_profiles_inner_shift_27 },
            { .name = "profiles_inner_shift_28", .val = cli_tcam_profiles_inner_shift_28, .parms = set_profiles_inner_shift_28 },
            { .name = "profiles_inner_shift_29", .val = cli_tcam_profiles_inner_shift_29, .parms = set_profiles_inner_shift_29 },
            { .name = "profiles_inner_shift_30", .val = cli_tcam_profiles_inner_shift_30, .parms = set_profiles_inner_shift_30 },
            { .name = "profiles_inner_shift_31", .val = cli_tcam_profiles_inner_shift_31, .parms = set_profiles_inner_shift_31 },
            { .name = "profiles_cntxin_sp_id_0", .val = cli_tcam_profiles_cntxin_sp_id_0, .parms = set_profiles_cntxin_sp_id_0 },
            { .name = "profiles_cntxin_sp_id_1", .val = cli_tcam_profiles_cntxin_sp_id_1, .parms = set_profiles_cntxin_sp_id_1 },
            { .name = "profiles_cntxin_mask_0", .val = cli_tcam_profiles_cntxin_mask_0, .parms = set_profiles_cntxin_mask_0 },
            { .name = "profiles_cntxin_mask_1", .val = cli_tcam_profiles_cntxin_mask_1, .parms = set_profiles_cntxin_mask_1 },
            { .name = "profiles_cntxin_mask_2", .val = cli_tcam_profiles_cntxin_mask_2, .parms = set_profiles_cntxin_mask_2 },
            { .name = "profiles_cntxin_mask_3", .val = cli_tcam_profiles_cntxin_mask_3, .parms = set_profiles_cntxin_mask_3 },
            { .name = "profiles_cntxin_mask_4", .val = cli_tcam_profiles_cntxin_mask_4, .parms = set_profiles_cntxin_mask_4 },
            { .name = "profiles_cntxin_mask_5", .val = cli_tcam_profiles_cntxin_mask_5, .parms = set_profiles_cntxin_mask_5 },
            { .name = "profiles_cntxin_mask_6", .val = cli_tcam_profiles_cntxin_mask_6, .parms = set_profiles_cntxin_mask_6 },
            { .name = "profiles_cntxin_mask_7", .val = cli_tcam_profiles_cntxin_mask_7, .parms = set_profiles_cntxin_mask_7 },
            { .name = "profiles_sp_mask_0", .val = cli_tcam_profiles_sp_mask_0, .parms = set_profiles_sp_mask_0 },
            { .name = "profiles_sp_mask_1", .val = cli_tcam_profiles_sp_mask_1, .parms = set_profiles_sp_mask_1 },
            { .name = "profiles_sp_mask_2", .val = cli_tcam_profiles_sp_mask_2, .parms = set_profiles_sp_mask_2 },
            { .name = "profiles_sp_mask_3", .val = cli_tcam_profiles_sp_mask_3, .parms = set_profiles_sp_mask_3 },
            { .name = "profiles_rslt_mux_0", .val = cli_tcam_profiles_rslt_mux_0, .parms = set_profiles_rslt_mux_0 },
            { .name = "profiles_rslt_mux_1", .val = cli_tcam_profiles_rslt_mux_1, .parms = set_profiles_rslt_mux_1 },
            { .name = "profiles_rslt_mux_2", .val = cli_tcam_profiles_rslt_mux_2, .parms = set_profiles_rslt_mux_2 },
            { .name = "profiles_rslt_mux_3", .val = cli_tcam_profiles_rslt_mux_3, .parms = set_profiles_rslt_mux_3 },
            { .name = "profiles_rslt_builder_0", .val = cli_tcam_profiles_rslt_builder_0, .parms = set_profiles_rslt_builder_0 },
            { .name = "profiles_rslt_builder_1", .val = cli_tcam_profiles_rslt_builder_1, .parms = set_profiles_rslt_builder_1 },
            { .name = "profiles_rslt_builder_2", .val = cli_tcam_profiles_rslt_builder_2, .parms = set_profiles_rslt_builder_2 },
            { .name = "profiles_rslt_builder_3", .val = cli_tcam_profiles_rslt_builder_3, .parms = set_profiles_rslt_builder_3 },
            { .name = "profiles_rslt_builder_4", .val = cli_tcam_profiles_rslt_builder_4, .parms = set_profiles_rslt_builder_4 },
            { .name = "profiles_rslt_builder_5", .val = cli_tcam_profiles_rslt_builder_5, .parms = set_profiles_rslt_builder_5 },
            { .name = "profiles_rslt_builder_6", .val = cli_tcam_profiles_rslt_builder_6, .parms = set_profiles_rslt_builder_6 },
            { .name = "profiles_rslt_builder_7", .val = cli_tcam_profiles_rslt_builder_7, .parms = set_profiles_rslt_builder_7 },
            { .name = "profiles_tcam_concat", .val = cli_tcam_profiles_tcam_concat, .parms = set_profiles_tcam_concat },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", ag_drv_tcam_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t get_default[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_cfg_dinm[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cfg_dinm", "cfg_dinm", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_cfg_blksel[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cfg_blksel", "cfg_blksel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_cfg_tm[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cfg_tm", "cfg_tm", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_counters_searches[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("counters_searches", "counters_searches", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_counters_hits[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tcam_idx", "tcam_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_tcam_mem_indirect_data_out[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("indirect_data", "indirect_data", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_tcam_mem_indirect_data_in[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("indirect_data_in", "indirect_data_in", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_0[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_1[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_2[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_3[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_4[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_5[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_6[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_7[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_8[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_9[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_10[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_11[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_12[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_13[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_14[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_15[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_16[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_17[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_18[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_19[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_20[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_21[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_22[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_23[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_24[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_25[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_26[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_27[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_28[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_29[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_30[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_31[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_32[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_33[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_34[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_35[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_36[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_37[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_38[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_key_builder_offsets_39[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_key_builder_offset", "profile_key_builder_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_inner_shift_0[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_inner_shift_1[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_inner_shift_2[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_inner_shift_3[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_inner_shift_4[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_inner_shift_5[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_inner_shift_6[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_inner_shift_7[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_inner_shift_8[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_inner_shift_9[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_inner_shift_10[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_inner_shift_11[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_inner_shift_12[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_inner_shift_13[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_inner_shift_14[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_inner_shift_15[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_inner_shift_16[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_inner_shift_17[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_inner_shift_18[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_inner_shift_19[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_inner_shift_20[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_inner_shift_21[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_inner_shift_22[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_inner_shift_23[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_inner_shift_24[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_inner_shift_25[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_inner_shift_26[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_inner_shift_27[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_inner_shift_28[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_inner_shift_29[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_inner_shift_30[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_inner_shift_31[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_inner_shift", "profile_inner_shift", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_cntxin_sp_id_0[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_sp_id", "profile_sp_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_cntxin_sp_id_1[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_sp_id", "profile_sp_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_cntxin_mask_0[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_cntxin_mask", "profile_cntxin_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_cntxin_mask_1[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_cntxin_mask", "profile_cntxin_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_cntxin_mask_2[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_cntxin_mask", "profile_cntxin_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_cntxin_mask_3[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_cntxin_mask", "profile_cntxin_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_cntxin_mask_4[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_cntxin_mask", "profile_cntxin_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_cntxin_mask_5[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_cntxin_mask", "profile_cntxin_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_cntxin_mask_6[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_cntxin_mask", "profile_cntxin_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_cntxin_mask_7[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_cntxin_mask", "profile_cntxin_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_sp_mask_0[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_sp_mask", "profile_sp_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_sp_mask_1[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_sp_mask", "profile_sp_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_sp_mask_2[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_sp_mask", "profile_sp_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_sp_mask_3[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_sp_mask", "profile_sp_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_rslt_mux_0[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_rslt_mux", "profile_rslt_mux", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_rslt_mux_1[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_rslt_mux", "profile_rslt_mux", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_rslt_mux_2[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_rslt_mux", "profile_rslt_mux", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_rslt_mux_3[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_rslt_mux", "profile_rslt_mux", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_rslt_builder_0[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_rslt_builder", "profile_rslt_builder", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_rslt_builder_1[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_rslt_builder", "profile_rslt_builder", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_rslt_builder_2[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_rslt_builder", "profile_rslt_builder", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_rslt_builder_3[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_rslt_builder", "profile_rslt_builder", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_rslt_builder_4[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_rslt_builder", "profile_rslt_builder", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_rslt_builder_5[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_rslt_builder", "profile_rslt_builder", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_rslt_builder_6[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_rslt_builder", "profile_rslt_builder", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_rslt_builder_7[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_rslt_builder", "profile_rslt_builder", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_profiles_tcam_concat[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("profile_tcam_concat", "profile_tcam_concat", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "cfg_dinm", .val = cli_tcam_cfg_dinm, .parms = get_cfg_dinm },
            { .name = "cfg_blksel", .val = cli_tcam_cfg_blksel, .parms = get_cfg_blksel },
            { .name = "cfg_tm", .val = cli_tcam_cfg_tm, .parms = get_cfg_tm },
            { .name = "cfg_vreset", .val = cli_tcam_cfg_vreset, .parms = get_default },
            { .name = "counters_searches", .val = cli_tcam_counters_searches, .parms = get_counters_searches },
            { .name = "counters_hits", .val = cli_tcam_counters_hits, .parms = get_counters_hits },
            { .name = "tcam_mem_indirect_op", .val = cli_tcam_tcam_mem_indirect_op, .parms = get_default },
            { .name = "tcam_mem_indirect_op_done", .val = cli_tcam_tcam_mem_indirect_op_done, .parms = get_default },
            { .name = "tcam_mem_indirect_addr", .val = cli_tcam_tcam_mem_indirect_addr, .parms = get_default },
            { .name = "tcam_mem_indirect_valid_in", .val = cli_tcam_tcam_mem_indirect_valid_in, .parms = get_default },
            { .name = "tcam_mem_indirect_valid_out", .val = cli_tcam_tcam_mem_indirect_valid_out, .parms = get_default },
            { .name = "tcam_mem_indirect_data_out", .val = cli_tcam_tcam_mem_indirect_data_out, .parms = get_tcam_mem_indirect_data_out },
            { .name = "tcam_mem_indirect_data_in", .val = cli_tcam_tcam_mem_indirect_data_in, .parms = get_tcam_mem_indirect_data_in },
            { .name = "tcam_debug_bus_select", .val = cli_tcam_tcam_debug_bus_select, .parms = get_default },
            { .name = "profiles_key_builder_offsets_0", .val = cli_tcam_profiles_key_builder_offsets_0, .parms = get_profiles_key_builder_offsets_0 },
            { .name = "profiles_key_builder_offsets_1", .val = cli_tcam_profiles_key_builder_offsets_1, .parms = get_profiles_key_builder_offsets_1 },
            { .name = "profiles_key_builder_offsets_2", .val = cli_tcam_profiles_key_builder_offsets_2, .parms = get_profiles_key_builder_offsets_2 },
            { .name = "profiles_key_builder_offsets_3", .val = cli_tcam_profiles_key_builder_offsets_3, .parms = get_profiles_key_builder_offsets_3 },
            { .name = "profiles_key_builder_offsets_4", .val = cli_tcam_profiles_key_builder_offsets_4, .parms = get_profiles_key_builder_offsets_4 },
            { .name = "profiles_key_builder_offsets_5", .val = cli_tcam_profiles_key_builder_offsets_5, .parms = get_profiles_key_builder_offsets_5 },
            { .name = "profiles_key_builder_offsets_6", .val = cli_tcam_profiles_key_builder_offsets_6, .parms = get_profiles_key_builder_offsets_6 },
            { .name = "profiles_key_builder_offsets_7", .val = cli_tcam_profiles_key_builder_offsets_7, .parms = get_profiles_key_builder_offsets_7 },
            { .name = "profiles_key_builder_offsets_8", .val = cli_tcam_profiles_key_builder_offsets_8, .parms = get_profiles_key_builder_offsets_8 },
            { .name = "profiles_key_builder_offsets_9", .val = cli_tcam_profiles_key_builder_offsets_9, .parms = get_profiles_key_builder_offsets_9 },
            { .name = "profiles_key_builder_offsets_10", .val = cli_tcam_profiles_key_builder_offsets_10, .parms = get_profiles_key_builder_offsets_10 },
            { .name = "profiles_key_builder_offsets_11", .val = cli_tcam_profiles_key_builder_offsets_11, .parms = get_profiles_key_builder_offsets_11 },
            { .name = "profiles_key_builder_offsets_12", .val = cli_tcam_profiles_key_builder_offsets_12, .parms = get_profiles_key_builder_offsets_12 },
            { .name = "profiles_key_builder_offsets_13", .val = cli_tcam_profiles_key_builder_offsets_13, .parms = get_profiles_key_builder_offsets_13 },
            { .name = "profiles_key_builder_offsets_14", .val = cli_tcam_profiles_key_builder_offsets_14, .parms = get_profiles_key_builder_offsets_14 },
            { .name = "profiles_key_builder_offsets_15", .val = cli_tcam_profiles_key_builder_offsets_15, .parms = get_profiles_key_builder_offsets_15 },
            { .name = "profiles_key_builder_offsets_16", .val = cli_tcam_profiles_key_builder_offsets_16, .parms = get_profiles_key_builder_offsets_16 },
            { .name = "profiles_key_builder_offsets_17", .val = cli_tcam_profiles_key_builder_offsets_17, .parms = get_profiles_key_builder_offsets_17 },
            { .name = "profiles_key_builder_offsets_18", .val = cli_tcam_profiles_key_builder_offsets_18, .parms = get_profiles_key_builder_offsets_18 },
            { .name = "profiles_key_builder_offsets_19", .val = cli_tcam_profiles_key_builder_offsets_19, .parms = get_profiles_key_builder_offsets_19 },
            { .name = "profiles_key_builder_offsets_20", .val = cli_tcam_profiles_key_builder_offsets_20, .parms = get_profiles_key_builder_offsets_20 },
            { .name = "profiles_key_builder_offsets_21", .val = cli_tcam_profiles_key_builder_offsets_21, .parms = get_profiles_key_builder_offsets_21 },
            { .name = "profiles_key_builder_offsets_22", .val = cli_tcam_profiles_key_builder_offsets_22, .parms = get_profiles_key_builder_offsets_22 },
            { .name = "profiles_key_builder_offsets_23", .val = cli_tcam_profiles_key_builder_offsets_23, .parms = get_profiles_key_builder_offsets_23 },
            { .name = "profiles_key_builder_offsets_24", .val = cli_tcam_profiles_key_builder_offsets_24, .parms = get_profiles_key_builder_offsets_24 },
            { .name = "profiles_key_builder_offsets_25", .val = cli_tcam_profiles_key_builder_offsets_25, .parms = get_profiles_key_builder_offsets_25 },
            { .name = "profiles_key_builder_offsets_26", .val = cli_tcam_profiles_key_builder_offsets_26, .parms = get_profiles_key_builder_offsets_26 },
            { .name = "profiles_key_builder_offsets_27", .val = cli_tcam_profiles_key_builder_offsets_27, .parms = get_profiles_key_builder_offsets_27 },
            { .name = "profiles_key_builder_offsets_28", .val = cli_tcam_profiles_key_builder_offsets_28, .parms = get_profiles_key_builder_offsets_28 },
            { .name = "profiles_key_builder_offsets_29", .val = cli_tcam_profiles_key_builder_offsets_29, .parms = get_profiles_key_builder_offsets_29 },
            { .name = "profiles_key_builder_offsets_30", .val = cli_tcam_profiles_key_builder_offsets_30, .parms = get_profiles_key_builder_offsets_30 },
            { .name = "profiles_key_builder_offsets_31", .val = cli_tcam_profiles_key_builder_offsets_31, .parms = get_profiles_key_builder_offsets_31 },
            { .name = "profiles_key_builder_offsets_32", .val = cli_tcam_profiles_key_builder_offsets_32, .parms = get_profiles_key_builder_offsets_32 },
            { .name = "profiles_key_builder_offsets_33", .val = cli_tcam_profiles_key_builder_offsets_33, .parms = get_profiles_key_builder_offsets_33 },
            { .name = "profiles_key_builder_offsets_34", .val = cli_tcam_profiles_key_builder_offsets_34, .parms = get_profiles_key_builder_offsets_34 },
            { .name = "profiles_key_builder_offsets_35", .val = cli_tcam_profiles_key_builder_offsets_35, .parms = get_profiles_key_builder_offsets_35 },
            { .name = "profiles_key_builder_offsets_36", .val = cli_tcam_profiles_key_builder_offsets_36, .parms = get_profiles_key_builder_offsets_36 },
            { .name = "profiles_key_builder_offsets_37", .val = cli_tcam_profiles_key_builder_offsets_37, .parms = get_profiles_key_builder_offsets_37 },
            { .name = "profiles_key_builder_offsets_38", .val = cli_tcam_profiles_key_builder_offsets_38, .parms = get_profiles_key_builder_offsets_38 },
            { .name = "profiles_key_builder_offsets_39", .val = cli_tcam_profiles_key_builder_offsets_39, .parms = get_profiles_key_builder_offsets_39 },
            { .name = "profiles_inner_shift_0", .val = cli_tcam_profiles_inner_shift_0, .parms = get_profiles_inner_shift_0 },
            { .name = "profiles_inner_shift_1", .val = cli_tcam_profiles_inner_shift_1, .parms = get_profiles_inner_shift_1 },
            { .name = "profiles_inner_shift_2", .val = cli_tcam_profiles_inner_shift_2, .parms = get_profiles_inner_shift_2 },
            { .name = "profiles_inner_shift_3", .val = cli_tcam_profiles_inner_shift_3, .parms = get_profiles_inner_shift_3 },
            { .name = "profiles_inner_shift_4", .val = cli_tcam_profiles_inner_shift_4, .parms = get_profiles_inner_shift_4 },
            { .name = "profiles_inner_shift_5", .val = cli_tcam_profiles_inner_shift_5, .parms = get_profiles_inner_shift_5 },
            { .name = "profiles_inner_shift_6", .val = cli_tcam_profiles_inner_shift_6, .parms = get_profiles_inner_shift_6 },
            { .name = "profiles_inner_shift_7", .val = cli_tcam_profiles_inner_shift_7, .parms = get_profiles_inner_shift_7 },
            { .name = "profiles_inner_shift_8", .val = cli_tcam_profiles_inner_shift_8, .parms = get_profiles_inner_shift_8 },
            { .name = "profiles_inner_shift_9", .val = cli_tcam_profiles_inner_shift_9, .parms = get_profiles_inner_shift_9 },
            { .name = "profiles_inner_shift_10", .val = cli_tcam_profiles_inner_shift_10, .parms = get_profiles_inner_shift_10 },
            { .name = "profiles_inner_shift_11", .val = cli_tcam_profiles_inner_shift_11, .parms = get_profiles_inner_shift_11 },
            { .name = "profiles_inner_shift_12", .val = cli_tcam_profiles_inner_shift_12, .parms = get_profiles_inner_shift_12 },
            { .name = "profiles_inner_shift_13", .val = cli_tcam_profiles_inner_shift_13, .parms = get_profiles_inner_shift_13 },
            { .name = "profiles_inner_shift_14", .val = cli_tcam_profiles_inner_shift_14, .parms = get_profiles_inner_shift_14 },
            { .name = "profiles_inner_shift_15", .val = cli_tcam_profiles_inner_shift_15, .parms = get_profiles_inner_shift_15 },
            { .name = "profiles_inner_shift_16", .val = cli_tcam_profiles_inner_shift_16, .parms = get_profiles_inner_shift_16 },
            { .name = "profiles_inner_shift_17", .val = cli_tcam_profiles_inner_shift_17, .parms = get_profiles_inner_shift_17 },
            { .name = "profiles_inner_shift_18", .val = cli_tcam_profiles_inner_shift_18, .parms = get_profiles_inner_shift_18 },
            { .name = "profiles_inner_shift_19", .val = cli_tcam_profiles_inner_shift_19, .parms = get_profiles_inner_shift_19 },
            { .name = "profiles_inner_shift_20", .val = cli_tcam_profiles_inner_shift_20, .parms = get_profiles_inner_shift_20 },
            { .name = "profiles_inner_shift_21", .val = cli_tcam_profiles_inner_shift_21, .parms = get_profiles_inner_shift_21 },
            { .name = "profiles_inner_shift_22", .val = cli_tcam_profiles_inner_shift_22, .parms = get_profiles_inner_shift_22 },
            { .name = "profiles_inner_shift_23", .val = cli_tcam_profiles_inner_shift_23, .parms = get_profiles_inner_shift_23 },
            { .name = "profiles_inner_shift_24", .val = cli_tcam_profiles_inner_shift_24, .parms = get_profiles_inner_shift_24 },
            { .name = "profiles_inner_shift_25", .val = cli_tcam_profiles_inner_shift_25, .parms = get_profiles_inner_shift_25 },
            { .name = "profiles_inner_shift_26", .val = cli_tcam_profiles_inner_shift_26, .parms = get_profiles_inner_shift_26 },
            { .name = "profiles_inner_shift_27", .val = cli_tcam_profiles_inner_shift_27, .parms = get_profiles_inner_shift_27 },
            { .name = "profiles_inner_shift_28", .val = cli_tcam_profiles_inner_shift_28, .parms = get_profiles_inner_shift_28 },
            { .name = "profiles_inner_shift_29", .val = cli_tcam_profiles_inner_shift_29, .parms = get_profiles_inner_shift_29 },
            { .name = "profiles_inner_shift_30", .val = cli_tcam_profiles_inner_shift_30, .parms = get_profiles_inner_shift_30 },
            { .name = "profiles_inner_shift_31", .val = cli_tcam_profiles_inner_shift_31, .parms = get_profiles_inner_shift_31 },
            { .name = "profiles_cntxin_sp_id_0", .val = cli_tcam_profiles_cntxin_sp_id_0, .parms = get_profiles_cntxin_sp_id_0 },
            { .name = "profiles_cntxin_sp_id_1", .val = cli_tcam_profiles_cntxin_sp_id_1, .parms = get_profiles_cntxin_sp_id_1 },
            { .name = "profiles_cntxin_mask_0", .val = cli_tcam_profiles_cntxin_mask_0, .parms = get_profiles_cntxin_mask_0 },
            { .name = "profiles_cntxin_mask_1", .val = cli_tcam_profiles_cntxin_mask_1, .parms = get_profiles_cntxin_mask_1 },
            { .name = "profiles_cntxin_mask_2", .val = cli_tcam_profiles_cntxin_mask_2, .parms = get_profiles_cntxin_mask_2 },
            { .name = "profiles_cntxin_mask_3", .val = cli_tcam_profiles_cntxin_mask_3, .parms = get_profiles_cntxin_mask_3 },
            { .name = "profiles_cntxin_mask_4", .val = cli_tcam_profiles_cntxin_mask_4, .parms = get_profiles_cntxin_mask_4 },
            { .name = "profiles_cntxin_mask_5", .val = cli_tcam_profiles_cntxin_mask_5, .parms = get_profiles_cntxin_mask_5 },
            { .name = "profiles_cntxin_mask_6", .val = cli_tcam_profiles_cntxin_mask_6, .parms = get_profiles_cntxin_mask_6 },
            { .name = "profiles_cntxin_mask_7", .val = cli_tcam_profiles_cntxin_mask_7, .parms = get_profiles_cntxin_mask_7 },
            { .name = "profiles_sp_mask_0", .val = cli_tcam_profiles_sp_mask_0, .parms = get_profiles_sp_mask_0 },
            { .name = "profiles_sp_mask_1", .val = cli_tcam_profiles_sp_mask_1, .parms = get_profiles_sp_mask_1 },
            { .name = "profiles_sp_mask_2", .val = cli_tcam_profiles_sp_mask_2, .parms = get_profiles_sp_mask_2 },
            { .name = "profiles_sp_mask_3", .val = cli_tcam_profiles_sp_mask_3, .parms = get_profiles_sp_mask_3 },
            { .name = "profiles_rslt_mux_0", .val = cli_tcam_profiles_rslt_mux_0, .parms = get_profiles_rslt_mux_0 },
            { .name = "profiles_rslt_mux_1", .val = cli_tcam_profiles_rslt_mux_1, .parms = get_profiles_rslt_mux_1 },
            { .name = "profiles_rslt_mux_2", .val = cli_tcam_profiles_rslt_mux_2, .parms = get_profiles_rslt_mux_2 },
            { .name = "profiles_rslt_mux_3", .val = cli_tcam_profiles_rslt_mux_3, .parms = get_profiles_rslt_mux_3 },
            { .name = "profiles_rslt_builder_0", .val = cli_tcam_profiles_rslt_builder_0, .parms = get_profiles_rslt_builder_0 },
            { .name = "profiles_rslt_builder_1", .val = cli_tcam_profiles_rslt_builder_1, .parms = get_profiles_rslt_builder_1 },
            { .name = "profiles_rslt_builder_2", .val = cli_tcam_profiles_rslt_builder_2, .parms = get_profiles_rslt_builder_2 },
            { .name = "profiles_rslt_builder_3", .val = cli_tcam_profiles_rslt_builder_3, .parms = get_profiles_rslt_builder_3 },
            { .name = "profiles_rslt_builder_4", .val = cli_tcam_profiles_rslt_builder_4, .parms = get_profiles_rslt_builder_4 },
            { .name = "profiles_rslt_builder_5", .val = cli_tcam_profiles_rslt_builder_5, .parms = get_profiles_rslt_builder_5 },
            { .name = "profiles_rslt_builder_6", .val = cli_tcam_profiles_rslt_builder_6, .parms = get_profiles_rslt_builder_6 },
            { .name = "profiles_rslt_builder_7", .val = cli_tcam_profiles_rslt_builder_7, .parms = get_profiles_rslt_builder_7 },
            { .name = "profiles_tcam_concat", .val = cli_tcam_profiles_tcam_concat, .parms = get_profiles_tcam_concat },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_tcam_cli_get,
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
        BDMFMON_MAKE_PARM(dir, "test", "test", ag_drv_tcam_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0)            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_cmd_parm_t ext_test_default[] = {
            BDMFMON_MAKE_PARM("tcam_id", "tcam_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "cfg_dinm", .val = cli_tcam_cfg_dinm, .parms = ext_test_default},
            { .name = "cfg_blksel", .val = cli_tcam_cfg_blksel, .parms = ext_test_default},
            { .name = "cfg_tm", .val = cli_tcam_cfg_tm, .parms = ext_test_default},
            { .name = "tcam_mem_indirect_data_in", .val = cli_tcam_tcam_mem_indirect_data_in, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_0", .val = cli_tcam_profiles_key_builder_offsets_0, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_1", .val = cli_tcam_profiles_key_builder_offsets_1, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_2", .val = cli_tcam_profiles_key_builder_offsets_2, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_3", .val = cli_tcam_profiles_key_builder_offsets_3, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_4", .val = cli_tcam_profiles_key_builder_offsets_4, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_5", .val = cli_tcam_profiles_key_builder_offsets_5, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_6", .val = cli_tcam_profiles_key_builder_offsets_6, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_7", .val = cli_tcam_profiles_key_builder_offsets_7, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_8", .val = cli_tcam_profiles_key_builder_offsets_8, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_9", .val = cli_tcam_profiles_key_builder_offsets_9, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_10", .val = cli_tcam_profiles_key_builder_offsets_10, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_11", .val = cli_tcam_profiles_key_builder_offsets_11, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_12", .val = cli_tcam_profiles_key_builder_offsets_12, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_13", .val = cli_tcam_profiles_key_builder_offsets_13, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_14", .val = cli_tcam_profiles_key_builder_offsets_14, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_15", .val = cli_tcam_profiles_key_builder_offsets_15, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_16", .val = cli_tcam_profiles_key_builder_offsets_16, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_17", .val = cli_tcam_profiles_key_builder_offsets_17, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_18", .val = cli_tcam_profiles_key_builder_offsets_18, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_19", .val = cli_tcam_profiles_key_builder_offsets_19, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_20", .val = cli_tcam_profiles_key_builder_offsets_20, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_21", .val = cli_tcam_profiles_key_builder_offsets_21, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_22", .val = cli_tcam_profiles_key_builder_offsets_22, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_23", .val = cli_tcam_profiles_key_builder_offsets_23, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_24", .val = cli_tcam_profiles_key_builder_offsets_24, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_25", .val = cli_tcam_profiles_key_builder_offsets_25, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_26", .val = cli_tcam_profiles_key_builder_offsets_26, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_27", .val = cli_tcam_profiles_key_builder_offsets_27, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_28", .val = cli_tcam_profiles_key_builder_offsets_28, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_29", .val = cli_tcam_profiles_key_builder_offsets_29, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_30", .val = cli_tcam_profiles_key_builder_offsets_30, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_31", .val = cli_tcam_profiles_key_builder_offsets_31, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_32", .val = cli_tcam_profiles_key_builder_offsets_32, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_33", .val = cli_tcam_profiles_key_builder_offsets_33, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_34", .val = cli_tcam_profiles_key_builder_offsets_34, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_35", .val = cli_tcam_profiles_key_builder_offsets_35, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_36", .val = cli_tcam_profiles_key_builder_offsets_36, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_37", .val = cli_tcam_profiles_key_builder_offsets_37, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_38", .val = cli_tcam_profiles_key_builder_offsets_38, .parms = ext_test_default},
            { .name = "profiles_key_builder_offsets_39", .val = cli_tcam_profiles_key_builder_offsets_39, .parms = ext_test_default},
            { .name = "profiles_inner_shift_0", .val = cli_tcam_profiles_inner_shift_0, .parms = ext_test_default},
            { .name = "profiles_inner_shift_1", .val = cli_tcam_profiles_inner_shift_1, .parms = ext_test_default},
            { .name = "profiles_inner_shift_2", .val = cli_tcam_profiles_inner_shift_2, .parms = ext_test_default},
            { .name = "profiles_inner_shift_3", .val = cli_tcam_profiles_inner_shift_3, .parms = ext_test_default},
            { .name = "profiles_inner_shift_4", .val = cli_tcam_profiles_inner_shift_4, .parms = ext_test_default},
            { .name = "profiles_inner_shift_5", .val = cli_tcam_profiles_inner_shift_5, .parms = ext_test_default},
            { .name = "profiles_inner_shift_6", .val = cli_tcam_profiles_inner_shift_6, .parms = ext_test_default},
            { .name = "profiles_inner_shift_7", .val = cli_tcam_profiles_inner_shift_7, .parms = ext_test_default},
            { .name = "profiles_inner_shift_8", .val = cli_tcam_profiles_inner_shift_8, .parms = ext_test_default},
            { .name = "profiles_inner_shift_9", .val = cli_tcam_profiles_inner_shift_9, .parms = ext_test_default},
            { .name = "profiles_inner_shift_10", .val = cli_tcam_profiles_inner_shift_10, .parms = ext_test_default},
            { .name = "profiles_inner_shift_11", .val = cli_tcam_profiles_inner_shift_11, .parms = ext_test_default},
            { .name = "profiles_inner_shift_12", .val = cli_tcam_profiles_inner_shift_12, .parms = ext_test_default},
            { .name = "profiles_inner_shift_13", .val = cli_tcam_profiles_inner_shift_13, .parms = ext_test_default},
            { .name = "profiles_inner_shift_14", .val = cli_tcam_profiles_inner_shift_14, .parms = ext_test_default},
            { .name = "profiles_inner_shift_15", .val = cli_tcam_profiles_inner_shift_15, .parms = ext_test_default},
            { .name = "profiles_inner_shift_16", .val = cli_tcam_profiles_inner_shift_16, .parms = ext_test_default},
            { .name = "profiles_inner_shift_17", .val = cli_tcam_profiles_inner_shift_17, .parms = ext_test_default},
            { .name = "profiles_inner_shift_18", .val = cli_tcam_profiles_inner_shift_18, .parms = ext_test_default},
            { .name = "profiles_inner_shift_19", .val = cli_tcam_profiles_inner_shift_19, .parms = ext_test_default},
            { .name = "profiles_inner_shift_20", .val = cli_tcam_profiles_inner_shift_20, .parms = ext_test_default},
            { .name = "profiles_inner_shift_21", .val = cli_tcam_profiles_inner_shift_21, .parms = ext_test_default},
            { .name = "profiles_inner_shift_22", .val = cli_tcam_profiles_inner_shift_22, .parms = ext_test_default},
            { .name = "profiles_inner_shift_23", .val = cli_tcam_profiles_inner_shift_23, .parms = ext_test_default},
            { .name = "profiles_inner_shift_24", .val = cli_tcam_profiles_inner_shift_24, .parms = ext_test_default},
            { .name = "profiles_inner_shift_25", .val = cli_tcam_profiles_inner_shift_25, .parms = ext_test_default},
            { .name = "profiles_inner_shift_26", .val = cli_tcam_profiles_inner_shift_26, .parms = ext_test_default},
            { .name = "profiles_inner_shift_27", .val = cli_tcam_profiles_inner_shift_27, .parms = ext_test_default},
            { .name = "profiles_inner_shift_28", .val = cli_tcam_profiles_inner_shift_28, .parms = ext_test_default},
            { .name = "profiles_inner_shift_29", .val = cli_tcam_profiles_inner_shift_29, .parms = ext_test_default},
            { .name = "profiles_inner_shift_30", .val = cli_tcam_profiles_inner_shift_30, .parms = ext_test_default},
            { .name = "profiles_inner_shift_31", .val = cli_tcam_profiles_inner_shift_31, .parms = ext_test_default},
            { .name = "profiles_cntxin_sp_id_0", .val = cli_tcam_profiles_cntxin_sp_id_0, .parms = ext_test_default},
            { .name = "profiles_cntxin_sp_id_1", .val = cli_tcam_profiles_cntxin_sp_id_1, .parms = ext_test_default},
            { .name = "profiles_cntxin_mask_0", .val = cli_tcam_profiles_cntxin_mask_0, .parms = ext_test_default},
            { .name = "profiles_cntxin_mask_1", .val = cli_tcam_profiles_cntxin_mask_1, .parms = ext_test_default},
            { .name = "profiles_cntxin_mask_2", .val = cli_tcam_profiles_cntxin_mask_2, .parms = ext_test_default},
            { .name = "profiles_cntxin_mask_3", .val = cli_tcam_profiles_cntxin_mask_3, .parms = ext_test_default},
            { .name = "profiles_cntxin_mask_4", .val = cli_tcam_profiles_cntxin_mask_4, .parms = ext_test_default},
            { .name = "profiles_cntxin_mask_5", .val = cli_tcam_profiles_cntxin_mask_5, .parms = ext_test_default},
            { .name = "profiles_cntxin_mask_6", .val = cli_tcam_profiles_cntxin_mask_6, .parms = ext_test_default},
            { .name = "profiles_cntxin_mask_7", .val = cli_tcam_profiles_cntxin_mask_7, .parms = ext_test_default},
            { .name = "profiles_sp_mask_0", .val = cli_tcam_profiles_sp_mask_0, .parms = ext_test_default},
            { .name = "profiles_sp_mask_1", .val = cli_tcam_profiles_sp_mask_1, .parms = ext_test_default},
            { .name = "profiles_sp_mask_2", .val = cli_tcam_profiles_sp_mask_2, .parms = ext_test_default},
            { .name = "profiles_sp_mask_3", .val = cli_tcam_profiles_sp_mask_3, .parms = ext_test_default},
            { .name = "profiles_rslt_mux_0", .val = cli_tcam_profiles_rslt_mux_0, .parms = ext_test_default},
            { .name = "profiles_rslt_mux_1", .val = cli_tcam_profiles_rslt_mux_1, .parms = ext_test_default},
            { .name = "profiles_rslt_mux_2", .val = cli_tcam_profiles_rslt_mux_2, .parms = ext_test_default},
            { .name = "profiles_rslt_mux_3", .val = cli_tcam_profiles_rslt_mux_3, .parms = ext_test_default},
            { .name = "profiles_rslt_builder_0", .val = cli_tcam_profiles_rslt_builder_0, .parms = ext_test_default},
            { .name = "profiles_rslt_builder_1", .val = cli_tcam_profiles_rslt_builder_1, .parms = ext_test_default},
            { .name = "profiles_rslt_builder_2", .val = cli_tcam_profiles_rslt_builder_2, .parms = ext_test_default},
            { .name = "profiles_rslt_builder_3", .val = cli_tcam_profiles_rslt_builder_3, .parms = ext_test_default},
            { .name = "profiles_rslt_builder_4", .val = cli_tcam_profiles_rslt_builder_4, .parms = ext_test_default},
            { .name = "profiles_rslt_builder_5", .val = cli_tcam_profiles_rslt_builder_5, .parms = ext_test_default},
            { .name = "profiles_rslt_builder_6", .val = cli_tcam_profiles_rslt_builder_6, .parms = ext_test_default},
            { .name = "profiles_rslt_builder_7", .val = cli_tcam_profiles_rslt_builder_7, .parms = ext_test_default},
            { .name = "profiles_tcam_concat", .val = cli_tcam_profiles_tcam_concat, .parms = ext_test_default},
            BDMFMON_ENUM_LAST
        };
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name = "low", .val = ag_drv_cli_test_method_low },
            { .name = "mid", .val = ag_drv_cli_test_method_mid },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "ext_test", "ext_test", ag_drv_tcam_cli_ext_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0),
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0),
            BDMFMON_MAKE_PARM("start_idx", "array index to start test", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL),
            BDMFMON_MAKE_PARM("stop_idx", "array index to stop test", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name = "CFG_DINM", .val = bdmf_address_cfg_dinm },
            { .name = "CFG_BLKSEL", .val = bdmf_address_cfg_blksel },
            { .name = "CFG_TM", .val = bdmf_address_cfg_tm },
            { .name = "CFG_VRESET", .val = bdmf_address_cfg_vreset },
            { .name = "COUNTERS_SEARCHES", .val = bdmf_address_counters_searches },
            { .name = "COUNTERS_HITS", .val = bdmf_address_counters_hits },
            { .name = "TCAM_MEM_INDIRECT_OP", .val = bdmf_address_tcam_mem_indirect_op },
            { .name = "TCAM_MEM_INDIRECT_OP_DONE", .val = bdmf_address_tcam_mem_indirect_op_done },
            { .name = "TCAM_MEM_INDIRECT_ADDR", .val = bdmf_address_tcam_mem_indirect_addr },
            { .name = "TCAM_MEM_INDIRECT_VALID_IN", .val = bdmf_address_tcam_mem_indirect_valid_in },
            { .name = "TCAM_MEM_INDIRECT_VALID_OUT", .val = bdmf_address_tcam_mem_indirect_valid_out },
            { .name = "TCAM_MEM_INDIRECT_DATA_OUT", .val = bdmf_address_tcam_mem_indirect_data_out },
            { .name = "TCAM_MEM_INDIRECT_DATA_IN", .val = bdmf_address_tcam_mem_indirect_data_in },
            { .name = "TCAM_DEBUG_BUS_SELECT", .val = bdmf_address_tcam_debug_bus_select },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_0", .val = bdmf_address_profiles_key_builder_offsets_0 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_1", .val = bdmf_address_profiles_key_builder_offsets_1 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_2", .val = bdmf_address_profiles_key_builder_offsets_2 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_3", .val = bdmf_address_profiles_key_builder_offsets_3 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_4", .val = bdmf_address_profiles_key_builder_offsets_4 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_5", .val = bdmf_address_profiles_key_builder_offsets_5 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_6", .val = bdmf_address_profiles_key_builder_offsets_6 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_7", .val = bdmf_address_profiles_key_builder_offsets_7 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_8", .val = bdmf_address_profiles_key_builder_offsets_8 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_9", .val = bdmf_address_profiles_key_builder_offsets_9 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_10", .val = bdmf_address_profiles_key_builder_offsets_10 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_11", .val = bdmf_address_profiles_key_builder_offsets_11 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_12", .val = bdmf_address_profiles_key_builder_offsets_12 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_13", .val = bdmf_address_profiles_key_builder_offsets_13 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_14", .val = bdmf_address_profiles_key_builder_offsets_14 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_15", .val = bdmf_address_profiles_key_builder_offsets_15 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_16", .val = bdmf_address_profiles_key_builder_offsets_16 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_17", .val = bdmf_address_profiles_key_builder_offsets_17 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_18", .val = bdmf_address_profiles_key_builder_offsets_18 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_19", .val = bdmf_address_profiles_key_builder_offsets_19 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_20", .val = bdmf_address_profiles_key_builder_offsets_20 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_21", .val = bdmf_address_profiles_key_builder_offsets_21 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_22", .val = bdmf_address_profiles_key_builder_offsets_22 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_23", .val = bdmf_address_profiles_key_builder_offsets_23 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_24", .val = bdmf_address_profiles_key_builder_offsets_24 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_25", .val = bdmf_address_profiles_key_builder_offsets_25 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_26", .val = bdmf_address_profiles_key_builder_offsets_26 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_27", .val = bdmf_address_profiles_key_builder_offsets_27 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_28", .val = bdmf_address_profiles_key_builder_offsets_28 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_29", .val = bdmf_address_profiles_key_builder_offsets_29 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_30", .val = bdmf_address_profiles_key_builder_offsets_30 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_31", .val = bdmf_address_profiles_key_builder_offsets_31 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_32", .val = bdmf_address_profiles_key_builder_offsets_32 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_33", .val = bdmf_address_profiles_key_builder_offsets_33 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_34", .val = bdmf_address_profiles_key_builder_offsets_34 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_35", .val = bdmf_address_profiles_key_builder_offsets_35 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_36", .val = bdmf_address_profiles_key_builder_offsets_36 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_37", .val = bdmf_address_profiles_key_builder_offsets_37 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_38", .val = bdmf_address_profiles_key_builder_offsets_38 },
            { .name = "PROFILES_KEY_BUILDER_OFFSETS_39", .val = bdmf_address_profiles_key_builder_offsets_39 },
            { .name = "PROFILES_INNER_SHIFT_0", .val = bdmf_address_profiles_inner_shift_0 },
            { .name = "PROFILES_INNER_SHIFT_1", .val = bdmf_address_profiles_inner_shift_1 },
            { .name = "PROFILES_INNER_SHIFT_2", .val = bdmf_address_profiles_inner_shift_2 },
            { .name = "PROFILES_INNER_SHIFT_3", .val = bdmf_address_profiles_inner_shift_3 },
            { .name = "PROFILES_INNER_SHIFT_4", .val = bdmf_address_profiles_inner_shift_4 },
            { .name = "PROFILES_INNER_SHIFT_5", .val = bdmf_address_profiles_inner_shift_5 },
            { .name = "PROFILES_INNER_SHIFT_6", .val = bdmf_address_profiles_inner_shift_6 },
            { .name = "PROFILES_INNER_SHIFT_7", .val = bdmf_address_profiles_inner_shift_7 },
            { .name = "PROFILES_INNER_SHIFT_8", .val = bdmf_address_profiles_inner_shift_8 },
            { .name = "PROFILES_INNER_SHIFT_9", .val = bdmf_address_profiles_inner_shift_9 },
            { .name = "PROFILES_INNER_SHIFT_10", .val = bdmf_address_profiles_inner_shift_10 },
            { .name = "PROFILES_INNER_SHIFT_11", .val = bdmf_address_profiles_inner_shift_11 },
            { .name = "PROFILES_INNER_SHIFT_12", .val = bdmf_address_profiles_inner_shift_12 },
            { .name = "PROFILES_INNER_SHIFT_13", .val = bdmf_address_profiles_inner_shift_13 },
            { .name = "PROFILES_INNER_SHIFT_14", .val = bdmf_address_profiles_inner_shift_14 },
            { .name = "PROFILES_INNER_SHIFT_15", .val = bdmf_address_profiles_inner_shift_15 },
            { .name = "PROFILES_INNER_SHIFT_16", .val = bdmf_address_profiles_inner_shift_16 },
            { .name = "PROFILES_INNER_SHIFT_17", .val = bdmf_address_profiles_inner_shift_17 },
            { .name = "PROFILES_INNER_SHIFT_18", .val = bdmf_address_profiles_inner_shift_18 },
            { .name = "PROFILES_INNER_SHIFT_19", .val = bdmf_address_profiles_inner_shift_19 },
            { .name = "PROFILES_INNER_SHIFT_20", .val = bdmf_address_profiles_inner_shift_20 },
            { .name = "PROFILES_INNER_SHIFT_21", .val = bdmf_address_profiles_inner_shift_21 },
            { .name = "PROFILES_INNER_SHIFT_22", .val = bdmf_address_profiles_inner_shift_22 },
            { .name = "PROFILES_INNER_SHIFT_23", .val = bdmf_address_profiles_inner_shift_23 },
            { .name = "PROFILES_INNER_SHIFT_24", .val = bdmf_address_profiles_inner_shift_24 },
            { .name = "PROFILES_INNER_SHIFT_25", .val = bdmf_address_profiles_inner_shift_25 },
            { .name = "PROFILES_INNER_SHIFT_26", .val = bdmf_address_profiles_inner_shift_26 },
            { .name = "PROFILES_INNER_SHIFT_27", .val = bdmf_address_profiles_inner_shift_27 },
            { .name = "PROFILES_INNER_SHIFT_28", .val = bdmf_address_profiles_inner_shift_28 },
            { .name = "PROFILES_INNER_SHIFT_29", .val = bdmf_address_profiles_inner_shift_29 },
            { .name = "PROFILES_INNER_SHIFT_30", .val = bdmf_address_profiles_inner_shift_30 },
            { .name = "PROFILES_INNER_SHIFT_31", .val = bdmf_address_profiles_inner_shift_31 },
            { .name = "PROFILES_CNTXIN_SP_ID_0", .val = bdmf_address_profiles_cntxin_sp_id_0 },
            { .name = "PROFILES_CNTXIN_SP_ID_1", .val = bdmf_address_profiles_cntxin_sp_id_1 },
            { .name = "PROFILES_CNTXIN_MASK_0", .val = bdmf_address_profiles_cntxin_mask_0 },
            { .name = "PROFILES_CNTXIN_MASK_1", .val = bdmf_address_profiles_cntxin_mask_1 },
            { .name = "PROFILES_CNTXIN_MASK_2", .val = bdmf_address_profiles_cntxin_mask_2 },
            { .name = "PROFILES_CNTXIN_MASK_3", .val = bdmf_address_profiles_cntxin_mask_3 },
            { .name = "PROFILES_CNTXIN_MASK_4", .val = bdmf_address_profiles_cntxin_mask_4 },
            { .name = "PROFILES_CNTXIN_MASK_5", .val = bdmf_address_profiles_cntxin_mask_5 },
            { .name = "PROFILES_CNTXIN_MASK_6", .val = bdmf_address_profiles_cntxin_mask_6 },
            { .name = "PROFILES_CNTXIN_MASK_7", .val = bdmf_address_profiles_cntxin_mask_7 },
            { .name = "PROFILES_SP_MASK_0", .val = bdmf_address_profiles_sp_mask_0 },
            { .name = "PROFILES_SP_MASK_1", .val = bdmf_address_profiles_sp_mask_1 },
            { .name = "PROFILES_SP_MASK_2", .val = bdmf_address_profiles_sp_mask_2 },
            { .name = "PROFILES_SP_MASK_3", .val = bdmf_address_profiles_sp_mask_3 },
            { .name = "PROFILES_RSLT_MUX_0", .val = bdmf_address_profiles_rslt_mux_0 },
            { .name = "PROFILES_RSLT_MUX_1", .val = bdmf_address_profiles_rslt_mux_1 },
            { .name = "PROFILES_RSLT_MUX_2", .val = bdmf_address_profiles_rslt_mux_2 },
            { .name = "PROFILES_RSLT_MUX_3", .val = bdmf_address_profiles_rslt_mux_3 },
            { .name = "PROFILES_RSLT_BUILDER_0", .val = bdmf_address_profiles_rslt_builder_0 },
            { .name = "PROFILES_RSLT_BUILDER_1", .val = bdmf_address_profiles_rslt_builder_1 },
            { .name = "PROFILES_RSLT_BUILDER_2", .val = bdmf_address_profiles_rslt_builder_2 },
            { .name = "PROFILES_RSLT_BUILDER_3", .val = bdmf_address_profiles_rslt_builder_3 },
            { .name = "PROFILES_RSLT_BUILDER_4", .val = bdmf_address_profiles_rslt_builder_4 },
            { .name = "PROFILES_RSLT_BUILDER_5", .val = bdmf_address_profiles_rslt_builder_5 },
            { .name = "PROFILES_RSLT_BUILDER_6", .val = bdmf_address_profiles_rslt_builder_6 },
            { .name = "PROFILES_RSLT_BUILDER_7", .val = bdmf_address_profiles_rslt_builder_7 },
            { .name = "PROFILES_TCAM_CONCAT", .val = bdmf_address_profiles_tcam_concat },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "address", "address", bcm_tcam_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index1", "tcam_id", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

    return dir;
}
#endif /* #ifdef USE_BDMF_SHELL */
