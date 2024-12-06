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
#include "xrdp_drv_natc_indir_ag.h"

#define BLOCK_ADDR_COUNT_BITS 0
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_natc_indir_addr_reg_set(uint16_t natc_entry, bdmf_boolean w_r)
{
    uint32_t reg_addr_reg = 0;

#ifdef VALIDATE_PARMS
    if ((natc_entry >= _10BITS_MAX_VAL_) ||
       (w_r >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_addr_reg = RU_FIELD_SET(0, NATC_INDIR, ADDR_REG, NATC_ENTRY, reg_addr_reg, natc_entry);
    reg_addr_reg = RU_FIELD_SET(0, NATC_INDIR, ADDR_REG, W_R, reg_addr_reg, w_r);

    RU_REG_WRITE(0, NATC_INDIR, ADDR_REG, reg_addr_reg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_indir_addr_reg_get(uint16_t *natc_entry, bdmf_boolean *w_r)
{
    uint32_t reg_addr_reg;

#ifdef VALIDATE_PARMS
    if (!natc_entry || !w_r)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NATC_INDIR, ADDR_REG, reg_addr_reg);

    *natc_entry = RU_FIELD_GET(0, NATC_INDIR, ADDR_REG, NATC_ENTRY, reg_addr_reg);
    *w_r = RU_FIELD_GET(0, NATC_INDIR, ADDR_REG, W_R, reg_addr_reg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_indir_natc_flow_cntr_indir_addr_reg_set(uint8_t flow_cntr_entry, bdmf_boolean w_r)
{
    uint32_t reg_natc_flow_cntr_indir_addr_reg = 0;

#ifdef VALIDATE_PARMS
    if ((flow_cntr_entry >= _7BITS_MAX_VAL_) ||
       (w_r >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_flow_cntr_indir_addr_reg = RU_FIELD_SET(0, NATC_INDIR, NATC_FLOW_CNTR_INDIR_ADDR_REG, FLOW_CNTR_ENTRY, reg_natc_flow_cntr_indir_addr_reg, flow_cntr_entry);
    reg_natc_flow_cntr_indir_addr_reg = RU_FIELD_SET(0, NATC_INDIR, NATC_FLOW_CNTR_INDIR_ADDR_REG, W_R, reg_natc_flow_cntr_indir_addr_reg, w_r);

    RU_REG_WRITE(0, NATC_INDIR, NATC_FLOW_CNTR_INDIR_ADDR_REG, reg_natc_flow_cntr_indir_addr_reg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_indir_natc_flow_cntr_indir_addr_reg_get(uint8_t *flow_cntr_entry, bdmf_boolean *w_r)
{
    uint32_t reg_natc_flow_cntr_indir_addr_reg;

#ifdef VALIDATE_PARMS
    if (!flow_cntr_entry || !w_r)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NATC_INDIR, NATC_FLOW_CNTR_INDIR_ADDR_REG, reg_natc_flow_cntr_indir_addr_reg);

    *flow_cntr_entry = RU_FIELD_GET(0, NATC_INDIR, NATC_FLOW_CNTR_INDIR_ADDR_REG, FLOW_CNTR_ENTRY, reg_natc_flow_cntr_indir_addr_reg);
    *w_r = RU_FIELD_GET(0, NATC_INDIR, NATC_FLOW_CNTR_INDIR_ADDR_REG, W_R, reg_natc_flow_cntr_indir_addr_reg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_indir_natc_flow_cntr_indir_data_reg_set(uint32_t index, uint32_t data)
{
    uint32_t reg_natc_flow_cntr_indir_data_reg = 0;

#ifdef VALIDATE_PARMS
    if ((index >= 2))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_natc_flow_cntr_indir_data_reg = RU_FIELD_SET(0, NATC_INDIR, NATC_FLOW_CNTR_INDIR_DATA_REG, DATA, reg_natc_flow_cntr_indir_data_reg, data);

    RU_REG_RAM_WRITE(0, index, NATC_INDIR, NATC_FLOW_CNTR_INDIR_DATA_REG, reg_natc_flow_cntr_indir_data_reg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_indir_natc_flow_cntr_indir_data_reg_get(uint32_t index, uint32_t *data)
{
    uint32_t reg_natc_flow_cntr_indir_data_reg;

#ifdef VALIDATE_PARMS
    if (!data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((index >= 2))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, index, NATC_INDIR, NATC_FLOW_CNTR_INDIR_DATA_REG, reg_natc_flow_cntr_indir_data_reg);

    *data = RU_FIELD_GET(0, NATC_INDIR, NATC_FLOW_CNTR_INDIR_DATA_REG, DATA, reg_natc_flow_cntr_indir_data_reg);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL

typedef enum
{
    bdmf_address_addr_reg,
    bdmf_address_data_reg,
    bdmf_address_natc_flow_cntr_indir_addr_reg,
    bdmf_address_natc_flow_cntr_indir_data_reg,
}
bdmf_address;

static int ag_drv_natc_indir_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_natc_indir_addr_reg:
        ag_err = ag_drv_natc_indir_addr_reg_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_natc_indir_natc_flow_cntr_indir_addr_reg:
        ag_err = ag_drv_natc_indir_natc_flow_cntr_indir_addr_reg_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_natc_indir_natc_flow_cntr_indir_data_reg:
        ag_err = ag_drv_natc_indir_natc_flow_cntr_indir_data_reg_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

int bcm_natc_indir_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_natc_indir_addr_reg:
    {
        uint16_t natc_entry;
        bdmf_boolean w_r;
        ag_err = ag_drv_natc_indir_addr_reg_get(&natc_entry, &w_r);
        bdmf_session_print(session, "natc_entry = %u = 0x%x\n", natc_entry, natc_entry);
        bdmf_session_print(session, "w_r = %u = 0x%x\n", w_r, w_r);
        break;
    }
    case cli_natc_indir_natc_flow_cntr_indir_addr_reg:
    {
        uint8_t flow_cntr_entry;
        bdmf_boolean w_r;
        ag_err = ag_drv_natc_indir_natc_flow_cntr_indir_addr_reg_get(&flow_cntr_entry, &w_r);
        bdmf_session_print(session, "flow_cntr_entry = %u = 0x%x\n", flow_cntr_entry, flow_cntr_entry);
        bdmf_session_print(session, "w_r = %u = 0x%x\n", w_r, w_r);
        break;
    }
    case cli_natc_indir_natc_flow_cntr_indir_data_reg:
    {
        uint32_t data;
        ag_err = ag_drv_natc_indir_natc_flow_cntr_indir_data_reg_get(parm[1].value.unumber, &data);
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
static int ag_drv_natc_indir_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t ag_err = BDMF_ERR_OK;
    uint32_t test_success_cnt = 0;
    uint32_t test_failure_cnt = 0;

    {
        uint16_t natc_entry = gtmv(m, 10);
        bdmf_boolean w_r = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_natc_indir_addr_reg_set( %u %u)\n",
            natc_entry, w_r);
        ag_err = ag_drv_natc_indir_addr_reg_set(natc_entry, w_r);
        if (!ag_err)
            ag_err = ag_drv_natc_indir_addr_reg_get(&natc_entry, &w_r);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_natc_indir_addr_reg_get( %u %u)\n",
                natc_entry, w_r);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (natc_entry != gtmv(m, 10) || w_r != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t flow_cntr_entry = gtmv(m, 7);
        bdmf_boolean w_r = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_natc_indir_natc_flow_cntr_indir_addr_reg_set( %u %u)\n",
            flow_cntr_entry, w_r);
        ag_err = ag_drv_natc_indir_natc_flow_cntr_indir_addr_reg_set(flow_cntr_entry, w_r);
        if (!ag_err)
            ag_err = ag_drv_natc_indir_natc_flow_cntr_indir_addr_reg_get(&flow_cntr_entry, &w_r);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_natc_indir_natc_flow_cntr_indir_addr_reg_get( %u %u)\n",
                flow_cntr_entry, w_r);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (flow_cntr_entry != gtmv(m, 7) || w_r != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t index = gtmv(m, 1);
        uint32_t data = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_natc_indir_natc_flow_cntr_indir_data_reg_set( %u %u)\n", index,
            data);
        ag_err = ag_drv_natc_indir_natc_flow_cntr_indir_data_reg_set(index, data);
        if (!ag_err)
            ag_err = ag_drv_natc_indir_natc_flow_cntr_indir_data_reg_get(index, &data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_natc_indir_natc_flow_cntr_indir_data_reg_get( %u %u)\n", index,
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
static int ag_drv_natc_indir_cli_ext_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case cli_natc_indir_natc_flow_cntr_indir_data_reg:
    {
        uint32_t max_index = 2;
        uint32_t index = gtmv(m, 1);
        uint32_t data = gtmv(m, 32);

        if ((start_idx >= max_index) || (stop_idx >= max_index))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_index);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (index = 0; index < max_index; index++)
        {
            bdmf_session_print(session, "ag_drv_natc_indir_natc_flow_cntr_indir_data_reg_set( %u %u)\n", index,
                data);
            ag_err = ag_drv_natc_indir_natc_flow_cntr_indir_data_reg_set(index, data);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        data = gtmv(m, 32);

        for (index = start_idx; index <= stop_idx; index++)
        {
            bdmf_session_print(session, "ag_drv_natc_indir_natc_flow_cntr_indir_data_reg_set( %u %u)\n", index,
                data);
            ag_err = ag_drv_natc_indir_natc_flow_cntr_indir_data_reg_set(index, data);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", index);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (index = 0; index < max_index; index++)
        {
            if (index < start_idx || index > stop_idx)
                m = ag_drv_cli_test_method_high; /* method for checking "unchanged" entries */
            else
                m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_natc_indir_natc_flow_cntr_indir_data_reg_get(index, &data);

            bdmf_session_print(session, "ag_drv_natc_indir_natc_flow_cntr_indir_data_reg_get( %u %u)\n", index,
                data);

            if (data != gtmv(m, 32) || 0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", index);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of natc_flow_cntr_indir_data_reg completed. Number of tested entries %u.\n", max_index);
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
static int ag_drv_natc_indir_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_addr_reg: reg = &RU_REG(NATC_INDIR, ADDR_REG); blk = &RU_BLK(NATC_INDIR); break;
    case bdmf_address_data_reg: reg = &RU_REG(NATC_INDIR, DATA_REG); blk = &RU_BLK(NATC_INDIR); break;
    case bdmf_address_natc_flow_cntr_indir_addr_reg: reg = &RU_REG(NATC_INDIR, NATC_FLOW_CNTR_INDIR_ADDR_REG); blk = &RU_BLK(NATC_INDIR); break;
    case bdmf_address_natc_flow_cntr_indir_data_reg: reg = &RU_REG(NATC_INDIR, NATC_FLOW_CNTR_INDIR_DATA_REG); blk = &RU_BLK(NATC_INDIR); break;
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

bdmfmon_handle_t ag_drv_natc_indir_cli_init(bdmfmon_handle_t root_dir)
{
    bdmfmon_handle_t dir;

    dir = bdmfmon_dir_add(root_dir, "natc_indir", "natc_indir", BDMF_ACCESS_ADMIN, NULL);
    BUG_ON(dir == NULL);

    {
        static bdmfmon_cmd_parm_t set_addr_reg[] = {
            BDMFMON_MAKE_PARM("natc_entry", "natc_entry", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("w_r", "w_r", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_flow_cntr_indir_addr_reg[] = {
            BDMFMON_MAKE_PARM("flow_cntr_entry", "flow_cntr_entry", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("w_r", "w_r", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_natc_flow_cntr_indir_data_reg[] = {
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("data", "data", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "addr_reg", .val = cli_natc_indir_addr_reg, .parms = set_addr_reg },
            { .name = "natc_flow_cntr_indir_addr_reg", .val = cli_natc_indir_natc_flow_cntr_indir_addr_reg, .parms = set_natc_flow_cntr_indir_addr_reg },
            { .name = "natc_flow_cntr_indir_data_reg", .val = cli_natc_indir_natc_flow_cntr_indir_data_reg, .parms = set_natc_flow_cntr_indir_data_reg },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", ag_drv_natc_indir_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t get_default[] = {
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t get_natc_flow_cntr_indir_data_reg[] = {
            BDMFMON_MAKE_PARM("index", "index", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "addr_reg", .val = cli_natc_indir_addr_reg, .parms = get_default },
            { .name = "natc_flow_cntr_indir_addr_reg", .val = cli_natc_indir_natc_flow_cntr_indir_addr_reg, .parms = get_default },
            { .name = "natc_flow_cntr_indir_data_reg", .val = cli_natc_indir_natc_flow_cntr_indir_data_reg, .parms = get_natc_flow_cntr_indir_data_reg },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_natc_indir_cli_get,
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
        BDMFMON_MAKE_PARM(dir, "test", "test", ag_drv_natc_indir_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_cmd_parm_t ext_test_default[] = {
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "natc_flow_cntr_indir_data_reg", .val = cli_natc_indir_natc_flow_cntr_indir_data_reg, .parms = ext_test_default},
            BDMFMON_ENUM_LAST
        };
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name = "low", .val = ag_drv_cli_test_method_low },
            { .name = "mid", .val = ag_drv_cli_test_method_mid },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "ext_test", "ext_test", ag_drv_natc_indir_cli_ext_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0),
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0),
            BDMFMON_MAKE_PARM("start_idx", "array index to start test", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL),
            BDMFMON_MAKE_PARM("stop_idx", "array index to stop test", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name = "ADDR_REG", .val = bdmf_address_addr_reg },
            { .name = "DATA_REG", .val = bdmf_address_data_reg },
            { .name = "NATC_FLOW_CNTR_INDIR_ADDR_REG", .val = bdmf_address_natc_flow_cntr_indir_addr_reg },
            { .name = "NATC_FLOW_CNTR_INDIR_DATA_REG", .val = bdmf_address_natc_flow_cntr_indir_data_reg },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "address", "address", bcm_natc_indir_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

    return dir;
}
#endif /* #ifdef USE_BDMF_SHELL */
