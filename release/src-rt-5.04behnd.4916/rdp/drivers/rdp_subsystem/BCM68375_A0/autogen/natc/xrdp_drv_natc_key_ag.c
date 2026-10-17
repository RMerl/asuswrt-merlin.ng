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
#include "xrdp_drv_natc_key_ag.h"

#define BLOCK_ADDR_COUNT_BITS 3
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_natc_key_mask_set(uint8_t tbl_idx, uint8_t zero, const natc_key_mask *mask)
{
#ifdef VALIDATE_PARMS
    if(!mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tbl_idx >= BLOCK_ADDR_COUNT) ||
       (zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_WRITE(tbl_idx, zero * 8 + 0, NATC_KEY, MASK, mask->data[0]);
    RU_REG_RAM_WRITE(tbl_idx, zero * 8 + 1, NATC_KEY, MASK, mask->data[1]);
    RU_REG_RAM_WRITE(tbl_idx, zero * 8 + 2, NATC_KEY, MASK, mask->data[2]);
    RU_REG_RAM_WRITE(tbl_idx, zero * 8 + 3, NATC_KEY, MASK, mask->data[3]);
    RU_REG_RAM_WRITE(tbl_idx, zero * 8 + 4, NATC_KEY, MASK, mask->data[4]);
    RU_REG_RAM_WRITE(tbl_idx, zero * 8 + 5, NATC_KEY, MASK, mask->data[5]);
    RU_REG_RAM_WRITE(tbl_idx, zero * 8 + 6, NATC_KEY, MASK, mask->data[6]);
    RU_REG_RAM_WRITE(tbl_idx, zero * 8 + 7, NATC_KEY, MASK, mask->data[7]);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_key_mask_get(uint8_t tbl_idx, uint8_t zero, natc_key_mask *mask)
{
#ifdef VALIDATE_PARMS
    if (!mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tbl_idx >= BLOCK_ADDR_COUNT) ||
       (zero >= 1))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(tbl_idx, zero * 8 + 0, NATC_KEY, MASK, mask->data[0]);
    RU_REG_RAM_READ(tbl_idx, zero * 8 + 1, NATC_KEY, MASK, mask->data[1]);
    RU_REG_RAM_READ(tbl_idx, zero * 8 + 2, NATC_KEY, MASK, mask->data[2]);
    RU_REG_RAM_READ(tbl_idx, zero * 8 + 3, NATC_KEY, MASK, mask->data[3]);
    RU_REG_RAM_READ(tbl_idx, zero * 8 + 4, NATC_KEY, MASK, mask->data[4]);
    RU_REG_RAM_READ(tbl_idx, zero * 8 + 5, NATC_KEY, MASK, mask->data[5]);
    RU_REG_RAM_READ(tbl_idx, zero * 8 + 6, NATC_KEY, MASK, mask->data[6]);
    RU_REG_RAM_READ(tbl_idx, zero * 8 + 7, NATC_KEY, MASK, mask->data[7]);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL

typedef enum
{
    bdmf_address_mask,
}
bdmf_address;

static int ag_drv_natc_key_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_natc_key_mask:
    {
        natc_key_mask mask = { .data = { parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber, parm[6].value.unumber, parm[7].value.unumber, parm[8].value.unumber, parm[9].value.unumber, parm[10].value.unumber}};
        ag_err = ag_drv_natc_key_mask_set(parm[1].value.unumber, parm[2].value.unumber, &mask);
        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

int bcm_natc_key_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_natc_key_mask:
    {
        natc_key_mask mask;
        ag_err = ag_drv_natc_key_mask_get(parm[1].value.unumber, parm[2].value.unumber, &mask);
        bdmf_session_print(session, "data[0] = %u = 0x%x\n", mask.data[0], mask.data[0]);
        bdmf_session_print(session, "data[1] = %u = 0x%x\n", mask.data[1], mask.data[1]);
        bdmf_session_print(session, "data[2] = %u = 0x%x\n", mask.data[2], mask.data[2]);
        bdmf_session_print(session, "data[3] = %u = 0x%x\n", mask.data[3], mask.data[3]);
        bdmf_session_print(session, "data[4] = %u = 0x%x\n", mask.data[4], mask.data[4]);
        bdmf_session_print(session, "data[5] = %u = 0x%x\n", mask.data[5], mask.data[5]);
        bdmf_session_print(session, "data[6] = %u = 0x%x\n", mask.data[6], mask.data[6]);
        bdmf_session_print(session, "data[7] = %u = 0x%x\n", mask.data[7], mask.data[7]);
        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_natc_key_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    uint8_t tbl_idx = parm[1].value.unumber;
    bdmf_error_t ag_err = BDMF_ERR_OK;
    uint32_t test_success_cnt = 0;
    uint32_t test_failure_cnt = 0;

    {
        uint8_t zero = gtmv(m, 0);
        natc_key_mask mask = {.data = {gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32)}};
        bdmf_session_print(session, "ag_drv_natc_key_mask_set(%u %u %u %u %u %u %u %u %u %u)\n", tbl_idx, zero,
            mask.data[0], mask.data[1], mask.data[2], mask.data[3], 
            mask.data[4], mask.data[5], mask.data[6], mask.data[7]);
        ag_err = ag_drv_natc_key_mask_set(tbl_idx, zero, &mask);
        if (!ag_err)
            ag_err = ag_drv_natc_key_mask_get(tbl_idx, zero, &mask);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_natc_key_mask_get(%u %u %u %u %u %u %u %u %u %u)\n", tbl_idx, zero,
                mask.data[0], mask.data[1], mask.data[2], mask.data[3], 
                mask.data[4], mask.data[5], mask.data[6], mask.data[7]);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (mask.data[0] != gtmv(m, 32) || mask.data[1] != gtmv(m, 32) || mask.data[2] != gtmv(m, 32) || mask.data[3] != gtmv(m, 32) || mask.data[4] != gtmv(m, 32) || mask.data[5] != gtmv(m, 32) || mask.data[6] != gtmv(m, 32) || mask.data[7] != gtmv(m, 32))
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
static int ag_drv_natc_key_cli_ext_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m, input_method = parm[0].value.unumber;
    uint8_t tbl_idx = parm[2].value.unumber;
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
    case cli_natc_key_mask:
    {
        uint8_t max_zero = 1;
        uint8_t zero = gtmv(m, 0);
        natc_key_mask mask = {.data = {gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32), gtmv(m, 32) } };

        if ((start_idx >= max_zero) || (stop_idx >= max_zero))
        {
            bdmf_session_print(session, "ERROR: start_idx and stop_idx must be < %u\n", max_zero);
            return BDMF_ERR_PARM;
        }

        bdmf_session_print(session, "\n ======== Set all array entries to the init values ========\n");
        for (zero = 0; zero < max_zero; zero++)
        {
            bdmf_session_print(session, "ag_drv_natc_key_mask_set(%u %u %u %u %u %u %u %u %u %u)\n", tbl_idx, zero,
                mask.data[0], mask.data[1], mask.data[2], mask.data[3], 
                mask.data[4], mask.data[5], mask.data[6], mask.data[7]);
            ag_err = ag_drv_natc_key_mask_set(tbl_idx, zero, &mask);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting init value to array entry %u\n", zero);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Set the specified entries [%u..%u] ========\n", start_idx, stop_idx);
        /* Set the specified entries to values according to the method */
        m = input_method;
        mask.data[0] = gtmv(m, 32);
        mask.data[1] = gtmv(m, 32);
        mask.data[2] = gtmv(m, 32);
        mask.data[3] = gtmv(m, 32);
        mask.data[4] = gtmv(m, 32);
        mask.data[5] = gtmv(m, 32);
        mask.data[6] = gtmv(m, 32);
        mask.data[7] = gtmv(m, 32);

        for (zero = start_idx; zero <= stop_idx; zero++)
        {
            bdmf_session_print(session, "ag_drv_natc_key_mask_set(%u %u %u %u %u %u %u %u %u %u)\n", tbl_idx, zero,
                mask.data[0], mask.data[1], mask.data[2], mask.data[3], 
                mask.data[4], mask.data[5], mask.data[6], mask.data[7]);
            ag_err = ag_drv_natc_key_mask_set(tbl_idx, zero, &mask);
            if (ag_err)
            {
                bdmf_session_print(session, "Error on setting value to array entry %u\n", zero);
                return BDMF_ERR_IO;
            }
        }

        bdmf_session_print(session, "\n ======== Read and check all array entries ========\n");
        input_method = m; 
        for (zero = 0; zero < max_zero; zero++)
        {
            m = input_method; /* method for checking entries [start_idx..stop_idx] */

            ag_err = ag_drv_natc_key_mask_get(tbl_idx, zero, &mask);

            bdmf_session_print(session, "ag_drv_natc_key_mask_get(%u %u %u %u %u %u %u %u %u %u)\n", tbl_idx, zero,
                mask.data[0], mask.data[1], mask.data[2], mask.data[3], 
                mask.data[4], mask.data[5], mask.data[6], mask.data[7]);

            if (mask.data[0] != gtmv(m, 32) || 
                mask.data[1] != gtmv(m, 32) || 
                mask.data[2] != gtmv(m, 32) || 
                mask.data[3] != gtmv(m, 32) || 
                mask.data[4] != gtmv(m, 32) || 
                mask.data[5] != gtmv(m, 32) || 
                mask.data[6] != gtmv(m, 32) || 
                mask.data[7] != gtmv(m, 32) || 
                0)
            {
                bdmf_session_print(session, "Test failed on comparing set and get results of entry %u\n", zero);
                ext_test_failure_cnt++;
            }
            else
            {
                ext_test_success_cnt++;
            }
        }

        bdmf_session_print(session, "========================================================================\n");
        bdmf_session_print(session, "Test of mask completed. Number of tested entries %u.\n", max_zero);
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
static int ag_drv_natc_key_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_mask: reg = &RU_REG(NATC_KEY, MASK); blk = &RU_BLK(NATC_KEY); break;
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

bdmfmon_handle_t ag_drv_natc_key_cli_init(bdmfmon_handle_t root_dir)
{
    bdmfmon_handle_t dir;

    dir = bdmfmon_dir_add(root_dir, "natc_key", "natc_key", BDMF_ACCESS_ADMIN, NULL);
    BUG_ON(dir == NULL);

    {
        static bdmfmon_cmd_parm_t set_mask[] = {
            BDMFMON_MAKE_PARM("tbl_idx", "tbl_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("data0", "data0", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data1", "data1", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data2", "data2", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data3", "data3", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data4", "data4", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data5", "data5", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data6", "data6", BDMFMON_PARM_HEX, 0),
            BDMFMON_MAKE_PARM("data7", "data7", BDMFMON_PARM_HEX, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "mask", .val = cli_natc_key_mask, .parms = set_mask },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", ag_drv_natc_key_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t get_mask[] = {
            BDMFMON_MAKE_PARM("tbl_idx", "tbl_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("zero", "zero", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "mask", .val = cli_natc_key_mask, .parms = get_mask },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_natc_key_cli_get,
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
        BDMFMON_MAKE_PARM(dir, "test", "test", ag_drv_natc_key_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0)            BDMFMON_MAKE_PARM("tbl_idx", "tbl_idx", BDMFMON_PARM_UNUMBER, 0));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_cmd_parm_t ext_test_default[] = {
            BDMFMON_MAKE_PARM("tbl_idx", "tbl_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "mask", .val = cli_natc_key_mask, .parms = ext_test_default},
            BDMFMON_ENUM_LAST
        };
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name = "low", .val = ag_drv_cli_test_method_low },
            { .name = "mid", .val = ag_drv_cli_test_method_mid },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "ext_test", "ext_test", ag_drv_natc_key_cli_ext_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0),
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0),
            BDMFMON_MAKE_PARM("start_idx", "array index to start test", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL),
            BDMFMON_MAKE_PARM("stop_idx", "array index to stop test", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name = "MASK", .val = bdmf_address_mask },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "address", "address", bcm_natc_key_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index1", "tbl_idx", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

    return dir;
}
#endif /* #ifdef USE_BDMF_SHELL */
