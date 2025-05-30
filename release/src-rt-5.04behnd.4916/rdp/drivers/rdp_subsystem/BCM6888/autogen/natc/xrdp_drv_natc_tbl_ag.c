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
#include "xrdp_drv_natc_tbl_ag.h"

#define BLOCK_ADDR_COUNT_BITS 3
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_natc_tbl_key_addr_set(uint8_t tbl_idx, uint32_t key_lo, uint8_t key_hi)
{
    uint32_t reg_ddr_key_base_address_lower = 0;
    uint32_t reg_ddr_key_base_address_upper = 0;

#ifdef VALIDATE_PARMS
    if ((tbl_idx >= BLOCK_ADDR_COUNT) ||
       (key_lo >= _29BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(tbl_idx, NATC_TBL, DDR_KEY_BASE_ADDRESS_LOWER, reg_ddr_key_base_address_lower);
    RU_REG_READ(tbl_idx, NATC_TBL, DDR_KEY_BASE_ADDRESS_UPPER, reg_ddr_key_base_address_upper);

    reg_ddr_key_base_address_lower = RU_FIELD_SET(tbl_idx, NATC_TBL, DDR_KEY_BASE_ADDRESS_LOWER, BAR, reg_ddr_key_base_address_lower, key_lo);
    reg_ddr_key_base_address_upper = RU_FIELD_SET(tbl_idx, NATC_TBL, DDR_KEY_BASE_ADDRESS_UPPER, BAR, reg_ddr_key_base_address_upper, key_hi);

    RU_REG_WRITE(tbl_idx, NATC_TBL, DDR_KEY_BASE_ADDRESS_LOWER, reg_ddr_key_base_address_lower);
    RU_REG_WRITE(tbl_idx, NATC_TBL, DDR_KEY_BASE_ADDRESS_UPPER, reg_ddr_key_base_address_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_tbl_key_addr_get(uint8_t tbl_idx, uint32_t *key_lo, uint8_t *key_hi)
{
    uint32_t reg_ddr_key_base_address_lower;
    uint32_t reg_ddr_key_base_address_upper;

#ifdef VALIDATE_PARMS
    if (!key_lo || !key_hi)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tbl_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(tbl_idx, NATC_TBL, DDR_KEY_BASE_ADDRESS_LOWER, reg_ddr_key_base_address_lower);
    RU_REG_READ(tbl_idx, NATC_TBL, DDR_KEY_BASE_ADDRESS_UPPER, reg_ddr_key_base_address_upper);

    *key_lo = RU_FIELD_GET(tbl_idx, NATC_TBL, DDR_KEY_BASE_ADDRESS_LOWER, BAR, reg_ddr_key_base_address_lower);
    *key_hi = RU_FIELD_GET(tbl_idx, NATC_TBL, DDR_KEY_BASE_ADDRESS_UPPER, BAR, reg_ddr_key_base_address_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_tbl_res_addr_set(uint8_t tbl_idx, uint32_t res_lo, uint8_t res_hi)
{
    uint32_t reg_ddr_result_base_address_lower = 0;
    uint32_t reg_ddr_result_base_address_upper = 0;

#ifdef VALIDATE_PARMS
    if ((tbl_idx >= BLOCK_ADDR_COUNT) ||
       (res_lo >= _29BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(tbl_idx, NATC_TBL, DDR_RESULT_BASE_ADDRESS_LOWER, reg_ddr_result_base_address_lower);
    RU_REG_READ(tbl_idx, NATC_TBL, DDR_RESULT_BASE_ADDRESS_UPPER, reg_ddr_result_base_address_upper);

    reg_ddr_result_base_address_lower = RU_FIELD_SET(tbl_idx, NATC_TBL, DDR_RESULT_BASE_ADDRESS_LOWER, BAR, reg_ddr_result_base_address_lower, res_lo);
    reg_ddr_result_base_address_upper = RU_FIELD_SET(tbl_idx, NATC_TBL, DDR_RESULT_BASE_ADDRESS_UPPER, BAR, reg_ddr_result_base_address_upper, res_hi);

    RU_REG_WRITE(tbl_idx, NATC_TBL, DDR_RESULT_BASE_ADDRESS_LOWER, reg_ddr_result_base_address_lower);
    RU_REG_WRITE(tbl_idx, NATC_TBL, DDR_RESULT_BASE_ADDRESS_UPPER, reg_ddr_result_base_address_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_tbl_res_addr_get(uint8_t tbl_idx, uint32_t *res_lo, uint8_t *res_hi)
{
    uint32_t reg_ddr_result_base_address_lower;
    uint32_t reg_ddr_result_base_address_upper;

#ifdef VALIDATE_PARMS
    if (!res_lo || !res_hi)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((tbl_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(tbl_idx, NATC_TBL, DDR_RESULT_BASE_ADDRESS_LOWER, reg_ddr_result_base_address_lower);
    RU_REG_READ(tbl_idx, NATC_TBL, DDR_RESULT_BASE_ADDRESS_UPPER, reg_ddr_result_base_address_upper);

    *res_lo = RU_FIELD_GET(tbl_idx, NATC_TBL, DDR_RESULT_BASE_ADDRESS_LOWER, BAR, reg_ddr_result_base_address_lower);
    *res_hi = RU_FIELD_GET(tbl_idx, NATC_TBL, DDR_RESULT_BASE_ADDRESS_UPPER, BAR, reg_ddr_result_base_address_upper);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL

typedef enum
{
    bdmf_address_ddr_key_base_address_lower,
    bdmf_address_ddr_key_base_address_upper,
    bdmf_address_ddr_result_base_address_lower,
    bdmf_address_ddr_result_base_address_upper,
}
bdmf_address;

static int ag_drv_natc_tbl_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_natc_tbl_key_addr:
        ag_err = ag_drv_natc_tbl_key_addr_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_natc_tbl_res_addr:
        ag_err = ag_drv_natc_tbl_res_addr_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

int bcm_natc_tbl_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_natc_tbl_key_addr:
    {
        uint32_t key_lo;
        uint8_t key_hi;
        ag_err = ag_drv_natc_tbl_key_addr_get(parm[1].value.unumber, &key_lo, &key_hi);
        bdmf_session_print(session, "key_lo = %u = 0x%x\n", key_lo, key_lo);
        bdmf_session_print(session, "key_hi = %u = 0x%x\n", key_hi, key_hi);
        break;
    }
    case cli_natc_tbl_res_addr:
    {
        uint32_t res_lo;
        uint8_t res_hi;
        ag_err = ag_drv_natc_tbl_res_addr_get(parm[1].value.unumber, &res_lo, &res_hi);
        bdmf_session_print(session, "res_lo = %u = 0x%x\n", res_lo, res_lo);
        bdmf_session_print(session, "res_hi = %u = 0x%x\n", res_hi, res_hi);
        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_natc_tbl_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    uint8_t tbl_idx = parm[1].value.unumber;
    bdmf_error_t ag_err = BDMF_ERR_OK;
    uint32_t test_success_cnt = 0;
    uint32_t test_failure_cnt = 0;

    {
        uint32_t key_lo = gtmv(m, 29);
        uint8_t key_hi = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_natc_tbl_key_addr_set(%u %u %u)\n", tbl_idx,
            key_lo, key_hi);
        ag_err = ag_drv_natc_tbl_key_addr_set(tbl_idx, key_lo, key_hi);
        if (!ag_err)
            ag_err = ag_drv_natc_tbl_key_addr_get(tbl_idx, &key_lo, &key_hi);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_natc_tbl_key_addr_get(%u %u %u)\n", tbl_idx,
                key_lo, key_hi);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (key_lo != gtmv(m, 29) || key_hi != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t res_lo = gtmv(m, 29);
        uint8_t res_hi = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_natc_tbl_res_addr_set(%u %u %u)\n", tbl_idx,
            res_lo, res_hi);
        ag_err = ag_drv_natc_tbl_res_addr_set(tbl_idx, res_lo, res_hi);
        if (!ag_err)
            ag_err = ag_drv_natc_tbl_res_addr_get(tbl_idx, &res_lo, &res_hi);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_natc_tbl_res_addr_get(%u %u %u)\n", tbl_idx,
                res_lo, res_hi);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (res_lo != gtmv(m, 29) || res_hi != gtmv(m, 8))
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
static int ag_drv_natc_tbl_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_ddr_key_base_address_lower: reg = &RU_REG(NATC_TBL, DDR_KEY_BASE_ADDRESS_LOWER); blk = &RU_BLK(NATC_TBL); break;
    case bdmf_address_ddr_key_base_address_upper: reg = &RU_REG(NATC_TBL, DDR_KEY_BASE_ADDRESS_UPPER); blk = &RU_BLK(NATC_TBL); break;
    case bdmf_address_ddr_result_base_address_lower: reg = &RU_REG(NATC_TBL, DDR_RESULT_BASE_ADDRESS_LOWER); blk = &RU_BLK(NATC_TBL); break;
    case bdmf_address_ddr_result_base_address_upper: reg = &RU_REG(NATC_TBL, DDR_RESULT_BASE_ADDRESS_UPPER); blk = &RU_BLK(NATC_TBL); break;
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

bdmfmon_handle_t ag_drv_natc_tbl_cli_init(bdmfmon_handle_t root_dir)
{
    bdmfmon_handle_t dir;

    dir = bdmfmon_dir_add(root_dir, "natc_tbl", "natc_tbl", BDMF_ACCESS_ADMIN, NULL);
    BUG_ON(dir == NULL);

    {
        static bdmfmon_cmd_parm_t set_key_addr[] = {
            BDMFMON_MAKE_PARM("tbl_idx", "tbl_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("key_lo", "key_lo", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("key_hi", "key_hi", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_res_addr[] = {
            BDMFMON_MAKE_PARM("tbl_idx", "tbl_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("res_lo", "res_lo", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("res_hi", "res_hi", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "key_addr", .val = cli_natc_tbl_key_addr, .parms = set_key_addr },
            { .name = "res_addr", .val = cli_natc_tbl_res_addr, .parms = set_res_addr },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", ag_drv_natc_tbl_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t get_default[] = {
            BDMFMON_MAKE_PARM("tbl_idx", "tbl_idx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "key_addr", .val = cli_natc_tbl_key_addr, .parms = get_default },
            { .name = "res_addr", .val = cli_natc_tbl_res_addr, .parms = get_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_natc_tbl_cli_get,
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
        BDMFMON_MAKE_PARM(dir, "test", "test", ag_drv_natc_tbl_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0)            BDMFMON_MAKE_PARM("tbl_idx", "tbl_idx", BDMFMON_PARM_UNUMBER, 0));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name = "DDR_KEY_BASE_ADDRESS_LOWER", .val = bdmf_address_ddr_key_base_address_lower },
            { .name = "DDR_KEY_BASE_ADDRESS_UPPER", .val = bdmf_address_ddr_key_base_address_upper },
            { .name = "DDR_RESULT_BASE_ADDRESS_LOWER", .val = bdmf_address_ddr_result_base_address_lower },
            { .name = "DDR_RESULT_BASE_ADDRESS_UPPER", .val = bdmf_address_ddr_result_base_address_upper },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "address", "address", bcm_natc_tbl_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index1", "tbl_idx", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

    return dir;
}
#endif /* #ifdef USE_BDMF_SHELL */
