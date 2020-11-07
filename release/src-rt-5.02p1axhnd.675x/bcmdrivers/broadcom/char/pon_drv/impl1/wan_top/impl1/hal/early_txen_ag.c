/*
   Copyright (c) 2015 Broadcom Corporation
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

#include "drivers_common_ag.h"
#include "early_txen_ag.h"
bdmf_error_t ag_drv_early_txen_txen_set(const early_txen_txen *txen)
{
    uint32_t reg_txen=0;

#ifdef VALIDATE_PARMS
    if(!txen)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((txen->cr_xgwan_top_wan_misc_early_txen_cfg_early_txen_bypass >= _1BITS_MAX_VAL_) ||
       (txen->cr_xgwan_top_wan_misc_early_txen_cfg_input_txen_polarity >= _1BITS_MAX_VAL_) ||
       (txen->cr_xgwan_top_wan_misc_early_txen_cfg_output_txen_polarity >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_txen = RU_FIELD_SET(0, EARLY_TXEN, TXEN, CR_XGWAN_TOP_WAN_MISC_EARLY_TXEN_CFG_EARLY_TXEN_BYPASS, reg_txen, txen->cr_xgwan_top_wan_misc_early_txen_cfg_early_txen_bypass);
    reg_txen = RU_FIELD_SET(0, EARLY_TXEN, TXEN, CR_XGWAN_TOP_WAN_MISC_EARLY_TXEN_CFG_INPUT_TXEN_POLARITY, reg_txen, txen->cr_xgwan_top_wan_misc_early_txen_cfg_input_txen_polarity);
    reg_txen = RU_FIELD_SET(0, EARLY_TXEN, TXEN, CR_XGWAN_TOP_WAN_MISC_EARLY_TXEN_CFG_OUTPUT_TXEN_POLARITY, reg_txen, txen->cr_xgwan_top_wan_misc_early_txen_cfg_output_txen_polarity);
    reg_txen = RU_FIELD_SET(0, EARLY_TXEN, TXEN, CR_XGWAN_TOP_WAN_MISC_EARLY_TXEN_CFG_TOFF_TIME, reg_txen, txen->cr_xgwan_top_wan_misc_early_txen_cfg_toff_time);
    reg_txen = RU_FIELD_SET(0, EARLY_TXEN, TXEN, CR_XGWAN_TOP_WAN_MISC_EARLY_TXEN_CFG_SETUP_TIME, reg_txen, txen->cr_xgwan_top_wan_misc_early_txen_cfg_setup_time);
    reg_txen = RU_FIELD_SET(0, EARLY_TXEN, TXEN, CR_XGWAN_TOP_WAN_MISC_EARLY_TXEN_CFG_HOLD_TIME, reg_txen, txen->cr_xgwan_top_wan_misc_early_txen_cfg_hold_time);

    RU_REG_WRITE(0, EARLY_TXEN, TXEN, reg_txen);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_early_txen_txen_get(early_txen_txen *txen)
{
    uint32_t reg_txen=0;

#ifdef VALIDATE_PARMS
    if(!txen)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, EARLY_TXEN, TXEN, reg_txen);

    txen->cr_xgwan_top_wan_misc_early_txen_cfg_early_txen_bypass = RU_FIELD_GET(0, EARLY_TXEN, TXEN, CR_XGWAN_TOP_WAN_MISC_EARLY_TXEN_CFG_EARLY_TXEN_BYPASS, reg_txen);
    txen->cr_xgwan_top_wan_misc_early_txen_cfg_input_txen_polarity = RU_FIELD_GET(0, EARLY_TXEN, TXEN, CR_XGWAN_TOP_WAN_MISC_EARLY_TXEN_CFG_INPUT_TXEN_POLARITY, reg_txen);
    txen->cr_xgwan_top_wan_misc_early_txen_cfg_output_txen_polarity = RU_FIELD_GET(0, EARLY_TXEN, TXEN, CR_XGWAN_TOP_WAN_MISC_EARLY_TXEN_CFG_OUTPUT_TXEN_POLARITY, reg_txen);
    txen->cr_xgwan_top_wan_misc_early_txen_cfg_toff_time = RU_FIELD_GET(0, EARLY_TXEN, TXEN, CR_XGWAN_TOP_WAN_MISC_EARLY_TXEN_CFG_TOFF_TIME, reg_txen);
    txen->cr_xgwan_top_wan_misc_early_txen_cfg_setup_time = RU_FIELD_GET(0, EARLY_TXEN, TXEN, CR_XGWAN_TOP_WAN_MISC_EARLY_TXEN_CFG_SETUP_TIME, reg_txen);
    txen->cr_xgwan_top_wan_misc_early_txen_cfg_hold_time = RU_FIELD_GET(0, EARLY_TXEN, TXEN, CR_XGWAN_TOP_WAN_MISC_EARLY_TXEN_CFG_HOLD_TIME, reg_txen);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
enum
{
    BDMF_txen,
};

typedef enum
{
    bdmf_address_txen,
}
bdmf_address;

static int bcm_early_txen_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err;

    switch(parm[0].value.unumber)
    {
    case BDMF_txen:
    {
        early_txen_txen txen = { .cr_xgwan_top_wan_misc_early_txen_cfg_early_txen_bypass=parm[1].value.unumber, .cr_xgwan_top_wan_misc_early_txen_cfg_input_txen_polarity=parm[2].value.unumber, .cr_xgwan_top_wan_misc_early_txen_cfg_output_txen_polarity=parm[3].value.unumber, .cr_xgwan_top_wan_misc_early_txen_cfg_toff_time=parm[4].value.unumber, .cr_xgwan_top_wan_misc_early_txen_cfg_setup_time=parm[5].value.unumber, .cr_xgwan_top_wan_misc_early_txen_cfg_hold_time=parm[6].value.unumber};
        err = ag_drv_early_txen_txen_set(&txen);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_early_txen_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err;

    switch(parm[0].value.unumber)
    {
    case BDMF_txen:
    {
        early_txen_txen txen;
        err = ag_drv_early_txen_txen_get(&txen);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_early_txen_cfg_early_txen_bypass = %u = 0x%x\n", txen.cr_xgwan_top_wan_misc_early_txen_cfg_early_txen_bypass, txen.cr_xgwan_top_wan_misc_early_txen_cfg_early_txen_bypass);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_early_txen_cfg_input_txen_polarity = %u = 0x%x\n", txen.cr_xgwan_top_wan_misc_early_txen_cfg_input_txen_polarity, txen.cr_xgwan_top_wan_misc_early_txen_cfg_input_txen_polarity);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_early_txen_cfg_output_txen_polarity = %u = 0x%x\n", txen.cr_xgwan_top_wan_misc_early_txen_cfg_output_txen_polarity, txen.cr_xgwan_top_wan_misc_early_txen_cfg_output_txen_polarity);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_early_txen_cfg_toff_time = %u = 0x%x\n", txen.cr_xgwan_top_wan_misc_early_txen_cfg_toff_time, txen.cr_xgwan_top_wan_misc_early_txen_cfg_toff_time);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_early_txen_cfg_setup_time = %u = 0x%x\n", txen.cr_xgwan_top_wan_misc_early_txen_cfg_setup_time, txen.cr_xgwan_top_wan_misc_early_txen_cfg_setup_time);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_early_txen_cfg_hold_time = %u = 0x%x\n", txen.cr_xgwan_top_wan_misc_early_txen_cfg_hold_time, txen.cr_xgwan_top_wan_misc_early_txen_cfg_hold_time);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_early_txen_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        early_txen_txen txen = {.cr_xgwan_top_wan_misc_early_txen_cfg_early_txen_bypass=gtmv(m, 1), .cr_xgwan_top_wan_misc_early_txen_cfg_input_txen_polarity=gtmv(m, 1), .cr_xgwan_top_wan_misc_early_txen_cfg_output_txen_polarity=gtmv(m, 1), .cr_xgwan_top_wan_misc_early_txen_cfg_toff_time=gtmv(m, 8), .cr_xgwan_top_wan_misc_early_txen_cfg_setup_time=gtmv(m, 8), .cr_xgwan_top_wan_misc_early_txen_cfg_hold_time=gtmv(m, 8)};
        if(!err) bdmf_session_print(session, "ag_drv_early_txen_txen_set( %u %u %u %u %u %u)\n", txen.cr_xgwan_top_wan_misc_early_txen_cfg_early_txen_bypass, txen.cr_xgwan_top_wan_misc_early_txen_cfg_input_txen_polarity, txen.cr_xgwan_top_wan_misc_early_txen_cfg_output_txen_polarity, txen.cr_xgwan_top_wan_misc_early_txen_cfg_toff_time, txen.cr_xgwan_top_wan_misc_early_txen_cfg_setup_time, txen.cr_xgwan_top_wan_misc_early_txen_cfg_hold_time);
        if(!err) ag_drv_early_txen_txen_set(&txen);
        if(!err) ag_drv_early_txen_txen_get( &txen);
        if(!err) bdmf_session_print(session, "ag_drv_early_txen_txen_get( %u %u %u %u %u %u)\n", txen.cr_xgwan_top_wan_misc_early_txen_cfg_early_txen_bypass, txen.cr_xgwan_top_wan_misc_early_txen_cfg_input_txen_polarity, txen.cr_xgwan_top_wan_misc_early_txen_cfg_output_txen_polarity, txen.cr_xgwan_top_wan_misc_early_txen_cfg_toff_time, txen.cr_xgwan_top_wan_misc_early_txen_cfg_setup_time, txen.cr_xgwan_top_wan_misc_early_txen_cfg_hold_time);
        if(err || txen.cr_xgwan_top_wan_misc_early_txen_cfg_early_txen_bypass!=gtmv(m, 1) || txen.cr_xgwan_top_wan_misc_early_txen_cfg_input_txen_polarity!=gtmv(m, 1) || txen.cr_xgwan_top_wan_misc_early_txen_cfg_output_txen_polarity!=gtmv(m, 1) || txen.cr_xgwan_top_wan_misc_early_txen_cfg_toff_time!=gtmv(m, 8) || txen.cr_xgwan_top_wan_misc_early_txen_cfg_setup_time!=gtmv(m, 8) || txen.cr_xgwan_top_wan_misc_early_txen_cfg_hold_time!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    return err;
}

static int bcm_early_txen_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_txen : reg = &RU_REG(EARLY_TXEN, TXEN); blk = &RU_BLK(EARLY_TXEN); break;
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
                bdmf_session_print(session, 	 "(%5u) 0x%08X\n", j, (uint32_t)((uint32_t*)(blk->addr[i] + reg->addr) + j));
        }
    else
        for (i = index1_start; i < index1_stop; i++)
            bdmf_session_print(session, "(%3u) 0x%08X\n", i, blk->addr[i]+reg->addr);
    return 0;
}

bdmfmon_handle_t ag_drv_early_txen_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "early_txen"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "early_txen", "early_txen", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_txen[]={
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_early_txen_cfg_early_txen_bypass", "cr_xgwan_top_wan_misc_early_txen_cfg_early_txen_bypass", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_early_txen_cfg_input_txen_polarity", "cr_xgwan_top_wan_misc_early_txen_cfg_input_txen_polarity", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_early_txen_cfg_output_txen_polarity", "cr_xgwan_top_wan_misc_early_txen_cfg_output_txen_polarity", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_early_txen_cfg_toff_time", "cr_xgwan_top_wan_misc_early_txen_cfg_toff_time", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_early_txen_cfg_setup_time", "cr_xgwan_top_wan_misc_early_txen_cfg_setup_time", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_early_txen_cfg_hold_time", "cr_xgwan_top_wan_misc_early_txen_cfg_hold_time", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="txen", .val=BDMF_txen, .parms=set_txen },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_early_txen_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="txen", .val=BDMF_txen, .parms=set_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_early_txen_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_early_txen_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="TXEN" , .val=bdmf_address_txen },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_early_txen_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

