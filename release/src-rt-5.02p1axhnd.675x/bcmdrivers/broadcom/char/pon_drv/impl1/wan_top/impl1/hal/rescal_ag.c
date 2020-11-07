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
#include "rescal_ag.h"
bdmf_error_t ag_drv_rescal_cfg_set(bdmf_boolean cfg_wan_rescal_rstb, bdmf_boolean cfg_wan_rescal_diag_on, bdmf_boolean cfg_wan_rescal_pwrdn, uint16_t cfg_wan_rescal_ctrl)
{
    uint32_t reg_cfg=0;

#ifdef VALIDATE_PARMS
    if((cfg_wan_rescal_rstb >= _1BITS_MAX_VAL_) ||
       (cfg_wan_rescal_diag_on >= _1BITS_MAX_VAL_) ||
       (cfg_wan_rescal_pwrdn >= _1BITS_MAX_VAL_) ||
       (cfg_wan_rescal_ctrl >= _13BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cfg = RU_FIELD_SET(0, RESCAL, CFG, CFG_WAN_RESCAL_RSTB, reg_cfg, cfg_wan_rescal_rstb);
    reg_cfg = RU_FIELD_SET(0, RESCAL, CFG, CFG_WAN_RESCAL_DIAG_ON, reg_cfg, cfg_wan_rescal_diag_on);
    reg_cfg = RU_FIELD_SET(0, RESCAL, CFG, CFG_WAN_RESCAL_PWRDN, reg_cfg, cfg_wan_rescal_pwrdn);
    reg_cfg = RU_FIELD_SET(0, RESCAL, CFG, CFG_WAN_RESCAL_CTRL, reg_cfg, cfg_wan_rescal_ctrl);

    RU_REG_WRITE(0, RESCAL, CFG, reg_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rescal_cfg_get(bdmf_boolean *cfg_wan_rescal_rstb, bdmf_boolean *cfg_wan_rescal_diag_on, bdmf_boolean *cfg_wan_rescal_pwrdn, uint16_t *cfg_wan_rescal_ctrl)
{
    uint32_t reg_cfg=0;

#ifdef VALIDATE_PARMS
    if(!cfg_wan_rescal_rstb || !cfg_wan_rescal_diag_on || !cfg_wan_rescal_pwrdn || !cfg_wan_rescal_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, RESCAL, CFG, reg_cfg);

    *cfg_wan_rescal_rstb = RU_FIELD_GET(0, RESCAL, CFG, CFG_WAN_RESCAL_RSTB, reg_cfg);
    *cfg_wan_rescal_diag_on = RU_FIELD_GET(0, RESCAL, CFG, CFG_WAN_RESCAL_DIAG_ON, reg_cfg);
    *cfg_wan_rescal_pwrdn = RU_FIELD_GET(0, RESCAL, CFG, CFG_WAN_RESCAL_PWRDN, reg_cfg);
    *cfg_wan_rescal_ctrl = RU_FIELD_GET(0, RESCAL, CFG, CFG_WAN_RESCAL_CTRL, reg_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rescal_status_0_get(rescal_status_0 *status_0)
{
    uint32_t reg_status_0=0;

#ifdef VALIDATE_PARMS
    if(!status_0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, RESCAL, STATUS_0, reg_status_0);

    status_0->wan_rescal_done = RU_FIELD_GET(0, RESCAL, STATUS_0, WAN_RESCAL_DONE, reg_status_0);
    status_0->wan_rescal_pon = RU_FIELD_GET(0, RESCAL, STATUS_0, WAN_RESCAL_PON, reg_status_0);
    status_0->wan_rescal_prev_comp_cnt = RU_FIELD_GET(0, RESCAL, STATUS_0, WAN_RESCAL_PREV_COMP_CNT, reg_status_0);
    status_0->wan_rescal_ctrl_dfs = RU_FIELD_GET(0, RESCAL, STATUS_0, WAN_RESCAL_CTRL_DFS, reg_status_0);
    status_0->wan_rescal_state = RU_FIELD_GET(0, RESCAL, STATUS_0, WAN_RESCAL_STATE, reg_status_0);
    status_0->wan_rescal_comp = RU_FIELD_GET(0, RESCAL, STATUS_0, WAN_RESCAL_COMP, reg_status_0);
    status_0->wan_rescal_valid = RU_FIELD_GET(0, RESCAL, STATUS_0, WAN_RESCAL_VALID, reg_status_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_rescal_status_1_get(uint8_t *wan_rescal_curr_comp_cnt)
{
    uint32_t reg_status_1=0;

#ifdef VALIDATE_PARMS
    if(!wan_rescal_curr_comp_cnt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, RESCAL, STATUS_1, reg_status_1);

    *wan_rescal_curr_comp_cnt = RU_FIELD_GET(0, RESCAL, STATUS_1, WAN_RESCAL_CURR_COMP_CNT, reg_status_1);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
enum
{
    BDMF_cfg,
    BDMF_status_0,
    BDMF_status_1,
};

typedef enum
{
    bdmf_address_cfg,
    bdmf_address_status_0,
    bdmf_address_status_1,
}
bdmf_address;

static int bcm_rescal_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err;

    switch(parm[0].value.unumber)
    {
    case BDMF_cfg:
        err = ag_drv_rescal_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_rescal_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err;

    switch(parm[0].value.unumber)
    {
    case BDMF_cfg:
    {
        bdmf_boolean cfg_wan_rescal_rstb;
        bdmf_boolean cfg_wan_rescal_diag_on;
        bdmf_boolean cfg_wan_rescal_pwrdn;
        uint16_t cfg_wan_rescal_ctrl;
        err = ag_drv_rescal_cfg_get(&cfg_wan_rescal_rstb, &cfg_wan_rescal_diag_on, &cfg_wan_rescal_pwrdn, &cfg_wan_rescal_ctrl);
        bdmf_session_print(session, "cfg_wan_rescal_rstb = %u = 0x%x\n", cfg_wan_rescal_rstb, cfg_wan_rescal_rstb);
        bdmf_session_print(session, "cfg_wan_rescal_diag_on = %u = 0x%x\n", cfg_wan_rescal_diag_on, cfg_wan_rescal_diag_on);
        bdmf_session_print(session, "cfg_wan_rescal_pwrdn = %u = 0x%x\n", cfg_wan_rescal_pwrdn, cfg_wan_rescal_pwrdn);
        bdmf_session_print(session, "cfg_wan_rescal_ctrl = %u = 0x%x\n", cfg_wan_rescal_ctrl, cfg_wan_rescal_ctrl);
        break;
    }
    case BDMF_status_0:
    {
        rescal_status_0 status_0;
        err = ag_drv_rescal_status_0_get(&status_0);
        bdmf_session_print(session, "wan_rescal_done = %u = 0x%x\n", status_0.wan_rescal_done, status_0.wan_rescal_done);
        bdmf_session_print(session, "wan_rescal_pon = %u = 0x%x\n", status_0.wan_rescal_pon, status_0.wan_rescal_pon);
        bdmf_session_print(session, "wan_rescal_prev_comp_cnt = %u = 0x%x\n", status_0.wan_rescal_prev_comp_cnt, status_0.wan_rescal_prev_comp_cnt);
        bdmf_session_print(session, "wan_rescal_ctrl_dfs = %u = 0x%x\n", status_0.wan_rescal_ctrl_dfs, status_0.wan_rescal_ctrl_dfs);
        bdmf_session_print(session, "wan_rescal_state = %u = 0x%x\n", status_0.wan_rescal_state, status_0.wan_rescal_state);
        bdmf_session_print(session, "wan_rescal_comp = %u = 0x%x\n", status_0.wan_rescal_comp, status_0.wan_rescal_comp);
        bdmf_session_print(session, "wan_rescal_valid = %u = 0x%x\n", status_0.wan_rescal_valid, status_0.wan_rescal_valid);
        break;
    }
    case BDMF_status_1:
    {
        uint8_t wan_rescal_curr_comp_cnt;
        err = ag_drv_rescal_status_1_get(&wan_rescal_curr_comp_cnt);
        bdmf_session_print(session, "wan_rescal_curr_comp_cnt = %u = 0x%x\n", wan_rescal_curr_comp_cnt, wan_rescal_curr_comp_cnt);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_rescal_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        bdmf_boolean cfg_wan_rescal_rstb=gtmv(m, 1);
        bdmf_boolean cfg_wan_rescal_diag_on=gtmv(m, 1);
        bdmf_boolean cfg_wan_rescal_pwrdn=gtmv(m, 1);
        uint16_t cfg_wan_rescal_ctrl=gtmv(m, 13);
        if(!err) bdmf_session_print(session, "ag_drv_rescal_cfg_set( %u %u %u %u)\n", cfg_wan_rescal_rstb, cfg_wan_rescal_diag_on, cfg_wan_rescal_pwrdn, cfg_wan_rescal_ctrl);
        if(!err) ag_drv_rescal_cfg_set(cfg_wan_rescal_rstb, cfg_wan_rescal_diag_on, cfg_wan_rescal_pwrdn, cfg_wan_rescal_ctrl);
        if(!err) ag_drv_rescal_cfg_get( &cfg_wan_rescal_rstb, &cfg_wan_rescal_diag_on, &cfg_wan_rescal_pwrdn, &cfg_wan_rescal_ctrl);
        if(!err) bdmf_session_print(session, "ag_drv_rescal_cfg_get( %u %u %u %u)\n", cfg_wan_rescal_rstb, cfg_wan_rescal_diag_on, cfg_wan_rescal_pwrdn, cfg_wan_rescal_ctrl);
        if(err || cfg_wan_rescal_rstb!=gtmv(m, 1) || cfg_wan_rescal_diag_on!=gtmv(m, 1) || cfg_wan_rescal_pwrdn!=gtmv(m, 1) || cfg_wan_rescal_ctrl!=gtmv(m, 13))
            return err ? err : BDMF_ERR_IO;
    }
    {
        rescal_status_0 status_0 = {.wan_rescal_done=gtmv(m, 1), .wan_rescal_pon=gtmv(m, 4), .wan_rescal_prev_comp_cnt=gtmv(m, 4), .wan_rescal_ctrl_dfs=gtmv(m, 13), .wan_rescal_state=gtmv(m, 3), .wan_rescal_comp=gtmv(m, 1), .wan_rescal_valid=gtmv(m, 1)};
        if(!err) ag_drv_rescal_status_0_get( &status_0);
        if(!err) bdmf_session_print(session, "ag_drv_rescal_status_0_get( %u %u %u %u %u %u %u)\n", status_0.wan_rescal_done, status_0.wan_rescal_pon, status_0.wan_rescal_prev_comp_cnt, status_0.wan_rescal_ctrl_dfs, status_0.wan_rescal_state, status_0.wan_rescal_comp, status_0.wan_rescal_valid);
    }
    {
        uint8_t wan_rescal_curr_comp_cnt=gtmv(m, 6);
        if(!err) ag_drv_rescal_status_1_get( &wan_rescal_curr_comp_cnt);
        if(!err) bdmf_session_print(session, "ag_drv_rescal_status_1_get( %u)\n", wan_rescal_curr_comp_cnt);
    }
    return err;
}

static int bcm_rescal_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_cfg : reg = &RU_REG(RESCAL, CFG); blk = &RU_BLK(RESCAL); break;
    case bdmf_address_status_0 : reg = &RU_REG(RESCAL, STATUS_0); blk = &RU_BLK(RESCAL); break;
    case bdmf_address_status_1 : reg = &RU_REG(RESCAL, STATUS_1); blk = &RU_BLK(RESCAL); break;
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

bdmfmon_handle_t ag_drv_rescal_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "rescal"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "rescal", "rescal", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_cfg[]={
            BDMFMON_MAKE_PARM("cfg_wan_rescal_rstb", "cfg_wan_rescal_rstb", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfg_wan_rescal_diag_on", "cfg_wan_rescal_diag_on", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfg_wan_rescal_pwrdn", "cfg_wan_rescal_pwrdn", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfg_wan_rescal_ctrl", "cfg_wan_rescal_ctrl", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="cfg", .val=BDMF_cfg, .parms=set_cfg },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_rescal_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="cfg", .val=BDMF_cfg, .parms=set_default },
            { .name="status_0", .val=BDMF_status_0, .parms=set_default },
            { .name="status_1", .val=BDMF_status_1, .parms=set_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_rescal_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_rescal_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="CFG" , .val=bdmf_address_cfg },
            { .name="STATUS_0" , .val=bdmf_address_status_0 },
            { .name="STATUS_1" , .val=bdmf_address_status_1 },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_rescal_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

