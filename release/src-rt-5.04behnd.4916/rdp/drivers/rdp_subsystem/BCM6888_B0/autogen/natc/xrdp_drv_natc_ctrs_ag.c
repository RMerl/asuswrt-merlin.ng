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
#include "xrdp_drv_natc_ctrs_ag.h"

#define BLOCK_ADDR_COUNT_BITS 0
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_natc_ctrs_natc_ctrs_set(const natc_ctrs_natc_ctrs *natc_ctrs)
{
    uint32_t reg_cache_hit_count = 0;
    uint32_t reg_cache_miss_count = 0;
    uint32_t reg_ddr_request_count = 0;
    uint32_t reg_ddr_evict_count = 0;
    uint32_t reg_ddr_block_count = 0;

#ifdef VALIDATE_PARMS
    if(!natc_ctrs)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    reg_cache_hit_count = RU_FIELD_SET(0, NATC_CTRS, CACHE_HIT_COUNT, CACHE_HIT_COUNT, reg_cache_hit_count, natc_ctrs->cache_hit_count);
    reg_cache_miss_count = RU_FIELD_SET(0, NATC_CTRS, CACHE_MISS_COUNT, CACHE_MISS_COUNT, reg_cache_miss_count, natc_ctrs->cache_miss_count);
    reg_ddr_request_count = RU_FIELD_SET(0, NATC_CTRS, DDR_REQUEST_COUNT, DDR_REQUEST_COUNT, reg_ddr_request_count, natc_ctrs->ddr_request_count);
    reg_ddr_evict_count = RU_FIELD_SET(0, NATC_CTRS, DDR_EVICT_COUNT, DDR_EVICT_COUNT, reg_ddr_evict_count, natc_ctrs->ddr_evict_count);
    reg_ddr_block_count = RU_FIELD_SET(0, NATC_CTRS, DDR_BLOCK_COUNT, DDR_BLOCK_COUNT, reg_ddr_block_count, natc_ctrs->ddr_block_count);

    RU_REG_WRITE(0, NATC_CTRS, CACHE_HIT_COUNT, reg_cache_hit_count);
    RU_REG_WRITE(0, NATC_CTRS, CACHE_MISS_COUNT, reg_cache_miss_count);
    RU_REG_WRITE(0, NATC_CTRS, DDR_REQUEST_COUNT, reg_ddr_request_count);
    RU_REG_WRITE(0, NATC_CTRS, DDR_EVICT_COUNT, reg_ddr_evict_count);
    RU_REG_WRITE(0, NATC_CTRS, DDR_BLOCK_COUNT, reg_ddr_block_count);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_natc_ctrs_natc_ctrs_get(natc_ctrs_natc_ctrs *natc_ctrs)
{
    uint32_t reg_cache_hit_count;
    uint32_t reg_cache_miss_count;
    uint32_t reg_ddr_request_count;
    uint32_t reg_ddr_evict_count;
    uint32_t reg_ddr_block_count;

#ifdef VALIDATE_PARMS
    if (!natc_ctrs)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, NATC_CTRS, CACHE_HIT_COUNT, reg_cache_hit_count);
    RU_REG_READ(0, NATC_CTRS, CACHE_MISS_COUNT, reg_cache_miss_count);
    RU_REG_READ(0, NATC_CTRS, DDR_REQUEST_COUNT, reg_ddr_request_count);
    RU_REG_READ(0, NATC_CTRS, DDR_EVICT_COUNT, reg_ddr_evict_count);
    RU_REG_READ(0, NATC_CTRS, DDR_BLOCK_COUNT, reg_ddr_block_count);

    natc_ctrs->cache_hit_count = RU_FIELD_GET(0, NATC_CTRS, CACHE_HIT_COUNT, CACHE_HIT_COUNT, reg_cache_hit_count);
    natc_ctrs->cache_miss_count = RU_FIELD_GET(0, NATC_CTRS, CACHE_MISS_COUNT, CACHE_MISS_COUNT, reg_cache_miss_count);
    natc_ctrs->ddr_request_count = RU_FIELD_GET(0, NATC_CTRS, DDR_REQUEST_COUNT, DDR_REQUEST_COUNT, reg_ddr_request_count);
    natc_ctrs->ddr_evict_count = RU_FIELD_GET(0, NATC_CTRS, DDR_EVICT_COUNT, DDR_EVICT_COUNT, reg_ddr_evict_count);
    natc_ctrs->ddr_block_count = RU_FIELD_GET(0, NATC_CTRS, DDR_BLOCK_COUNT, DDR_BLOCK_COUNT, reg_ddr_block_count);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL

typedef enum
{
    bdmf_address_cache_hit_count,
    bdmf_address_cache_miss_count,
    bdmf_address_ddr_request_count,
    bdmf_address_ddr_evict_count,
    bdmf_address_ddr_block_count,
}
bdmf_address;

static int ag_drv_natc_ctrs_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_natc_ctrs_natc_ctrs:
    {
        natc_ctrs_natc_ctrs natc_ctrs = { .cache_hit_count = parm[1].value.unumber, .cache_miss_count = parm[2].value.unumber, .ddr_request_count = parm[3].value.unumber, .ddr_evict_count = parm[4].value.unumber, .ddr_block_count = parm[5].value.unumber};
        ag_err = ag_drv_natc_ctrs_natc_ctrs_set(&natc_ctrs);
        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

int bcm_natc_ctrs_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_natc_ctrs_natc_ctrs:
    {
        natc_ctrs_natc_ctrs natc_ctrs;
        ag_err = ag_drv_natc_ctrs_natc_ctrs_get(&natc_ctrs);
        bdmf_session_print(session, "cache_hit_count = %u = 0x%x\n", natc_ctrs.cache_hit_count, natc_ctrs.cache_hit_count);
        bdmf_session_print(session, "cache_miss_count = %u = 0x%x\n", natc_ctrs.cache_miss_count, natc_ctrs.cache_miss_count);
        bdmf_session_print(session, "ddr_request_count = %u = 0x%x\n", natc_ctrs.ddr_request_count, natc_ctrs.ddr_request_count);
        bdmf_session_print(session, "ddr_evict_count = %u = 0x%x\n", natc_ctrs.ddr_evict_count, natc_ctrs.ddr_evict_count);
        bdmf_session_print(session, "ddr_block_count = %u = 0x%x\n", natc_ctrs.ddr_block_count, natc_ctrs.ddr_block_count);
        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_natc_ctrs_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t ag_err = BDMF_ERR_OK;
    uint32_t test_success_cnt = 0;
    uint32_t test_failure_cnt = 0;

    {
        natc_ctrs_natc_ctrs natc_ctrs = {.cache_hit_count = gtmv(m, 32), .cache_miss_count = gtmv(m, 32), .ddr_request_count = gtmv(m, 32), .ddr_evict_count = gtmv(m, 32), .ddr_block_count = gtmv(m, 32)};
        bdmf_session_print(session, "ag_drv_natc_ctrs_natc_ctrs_set( %u %u %u %u %u)\n",
            natc_ctrs.cache_hit_count, natc_ctrs.cache_miss_count, natc_ctrs.ddr_request_count, natc_ctrs.ddr_evict_count, 
            natc_ctrs.ddr_block_count);
        ag_err = ag_drv_natc_ctrs_natc_ctrs_set(&natc_ctrs);
        if (!ag_err)
            ag_err = ag_drv_natc_ctrs_natc_ctrs_get(&natc_ctrs);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_natc_ctrs_natc_ctrs_get( %u %u %u %u %u)\n",
                natc_ctrs.cache_hit_count, natc_ctrs.cache_miss_count, natc_ctrs.ddr_request_count, natc_ctrs.ddr_evict_count, 
                natc_ctrs.ddr_block_count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (natc_ctrs.cache_hit_count != gtmv(m, 32) || natc_ctrs.cache_miss_count != gtmv(m, 32) || natc_ctrs.ddr_request_count != gtmv(m, 32) || natc_ctrs.ddr_evict_count != gtmv(m, 32) || natc_ctrs.ddr_block_count != gtmv(m, 32))
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
static int ag_drv_natc_ctrs_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_cache_hit_count: reg = &RU_REG(NATC_CTRS, CACHE_HIT_COUNT); blk = &RU_BLK(NATC_CTRS); break;
    case bdmf_address_cache_miss_count: reg = &RU_REG(NATC_CTRS, CACHE_MISS_COUNT); blk = &RU_BLK(NATC_CTRS); break;
    case bdmf_address_ddr_request_count: reg = &RU_REG(NATC_CTRS, DDR_REQUEST_COUNT); blk = &RU_BLK(NATC_CTRS); break;
    case bdmf_address_ddr_evict_count: reg = &RU_REG(NATC_CTRS, DDR_EVICT_COUNT); blk = &RU_BLK(NATC_CTRS); break;
    case bdmf_address_ddr_block_count: reg = &RU_REG(NATC_CTRS, DDR_BLOCK_COUNT); blk = &RU_BLK(NATC_CTRS); break;
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

bdmfmon_handle_t ag_drv_natc_ctrs_cli_init(bdmfmon_handle_t root_dir)
{
    bdmfmon_handle_t dir;

    dir = bdmfmon_dir_add(root_dir, "natc_ctrs", "natc_ctrs", BDMF_ACCESS_ADMIN, NULL);
    BUG_ON(dir == NULL);

    {
        static bdmfmon_cmd_parm_t set_natc_ctrs[] = {
            BDMFMON_MAKE_PARM("cache_hit_count", "cache_hit_count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cache_miss_count", "cache_miss_count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_request_count", "ddr_request_count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_evict_count", "ddr_evict_count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ddr_block_count", "ddr_block_count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "natc_ctrs", .val = cli_natc_ctrs_natc_ctrs, .parms = set_natc_ctrs },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", ag_drv_natc_ctrs_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t get_default[] = {
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "natc_ctrs", .val = cli_natc_ctrs_natc_ctrs, .parms = get_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_natc_ctrs_cli_get,
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
        BDMFMON_MAKE_PARM(dir, "test", "test", ag_drv_natc_ctrs_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name = "CACHE_HIT_COUNT", .val = bdmf_address_cache_hit_count },
            { .name = "CACHE_MISS_COUNT", .val = bdmf_address_cache_miss_count },
            { .name = "DDR_REQUEST_COUNT", .val = bdmf_address_ddr_request_count },
            { .name = "DDR_EVICT_COUNT", .val = bdmf_address_ddr_evict_count },
            { .name = "DDR_BLOCK_COUNT", .val = bdmf_address_ddr_block_count },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "address", "address", bcm_natc_ctrs_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

    return dir;
}
#endif /* #ifdef USE_BDMF_SHELL */
