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
#include "xrdp_drv_acb_if_ag.h"

bdmf_error_t ag_drv_acb_if_conf_set(bdmf_boolean crc_add, uint8_t val_loc)
{
    uint32_t reg_acbif_block_acbif_config_conf0=0;

#ifdef VALIDATE_PARMS
    if((crc_add >= _1BITS_MAX_VAL_) ||
       (val_loc >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_acbif_block_acbif_config_conf0 = RU_FIELD_SET(0, ACB_IF, ACBIF_BLOCK_ACBIF_CONFIG_CONF0, CRC_ADD, reg_acbif_block_acbif_config_conf0, crc_add);
    reg_acbif_block_acbif_config_conf0 = RU_FIELD_SET(0, ACB_IF, ACBIF_BLOCK_ACBIF_CONFIG_CONF0, VAL_LOC, reg_acbif_block_acbif_config_conf0, val_loc);

    RU_REG_WRITE(0, ACB_IF, ACBIF_BLOCK_ACBIF_CONFIG_CONF0, reg_acbif_block_acbif_config_conf0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_acb_if_conf_get(bdmf_boolean *crc_add, uint8_t *val_loc)
{
    uint32_t reg_acbif_block_acbif_config_conf0;

#ifdef VALIDATE_PARMS
    if(!crc_add || !val_loc)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, ACB_IF, ACBIF_BLOCK_ACBIF_CONFIG_CONF0, reg_acbif_block_acbif_config_conf0);

    *crc_add = RU_FIELD_GET(0, ACB_IF, ACBIF_BLOCK_ACBIF_CONFIG_CONF0, CRC_ADD, reg_acbif_block_acbif_config_conf0);
    *val_loc = RU_FIELD_GET(0, ACB_IF, ACBIF_BLOCK_ACBIF_CONFIG_CONF0, VAL_LOC, reg_acbif_block_acbif_config_conf0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_acb_if_acbif_block_acbif_pm_counters_cmd_type_get(uint32_t idx, uint32_t *val)
{
    uint32_t reg_acbif_block_acbif_pm_counters_cmd_type;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((idx >= 3))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, idx, ACB_IF, ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_TYPE, reg_acbif_block_acbif_pm_counters_cmd_type);

    *val = RU_FIELD_GET(0, ACB_IF, ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_TYPE, VAL, reg_acbif_block_acbif_pm_counters_cmd_type);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_acb_if_acbif_block_acbif_pm_counters_cmd_imp_get(uint32_t idx, uint32_t *val)
{
    uint32_t reg_acbif_block_acbif_pm_counters_cmd_imp;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((idx >= 3))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, idx, ACB_IF, ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_IMP, reg_acbif_block_acbif_pm_counters_cmd_imp);

    *val = RU_FIELD_GET(0, ACB_IF, ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_IMP, VAL, reg_acbif_block_acbif_pm_counters_cmd_imp);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_acb_if_acbif_block_acbif_pm_counters_agg_get(uint32_t idx, uint32_t *val)
{
    uint32_t reg_acbif_block_acbif_pm_counters_agg;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((idx >= 2))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, idx, ACB_IF, ACBIF_BLOCK_ACBIF_PM_COUNTERS_AGG, reg_acbif_block_acbif_pm_counters_agg);

    *val = RU_FIELD_GET(0, ACB_IF, ACBIF_BLOCK_ACBIF_PM_COUNTERS_AGG, VAL, reg_acbif_block_acbif_pm_counters_agg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_acb_if_acbif_block_acbif_pm_counters_buffs_get(uint32_t idx, uint32_t *val)
{
    uint32_t reg_acbif_block_acbif_pm_counters_buffs;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((idx >= 2))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, idx, ACB_IF, ACBIF_BLOCK_ACBIF_PM_COUNTERS_BUFFS, reg_acbif_block_acbif_pm_counters_buffs);

    *val = RU_FIELD_GET(0, ACB_IF, ACBIF_BLOCK_ACBIF_PM_COUNTERS_BUFFS, VAL, reg_acbif_block_acbif_pm_counters_buffs);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_acb_if_acbif_block_acbif_pm_counters_gen_cfg_set(bdmf_boolean rd_clr, bdmf_boolean wrap)
{
    uint32_t reg_acbif_block_acbif_pm_counters_gen_cfg=0;

#ifdef VALIDATE_PARMS
    if((rd_clr >= _1BITS_MAX_VAL_) ||
       (wrap >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_acbif_block_acbif_pm_counters_gen_cfg = RU_FIELD_SET(0, ACB_IF, ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG, RD_CLR, reg_acbif_block_acbif_pm_counters_gen_cfg, rd_clr);
    reg_acbif_block_acbif_pm_counters_gen_cfg = RU_FIELD_SET(0, ACB_IF, ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG, WRAP, reg_acbif_block_acbif_pm_counters_gen_cfg, wrap);

    RU_REG_WRITE(0, ACB_IF, ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG, reg_acbif_block_acbif_pm_counters_gen_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_acb_if_acbif_block_acbif_pm_counters_gen_cfg_get(bdmf_boolean *rd_clr, bdmf_boolean *wrap)
{
    uint32_t reg_acbif_block_acbif_pm_counters_gen_cfg;

#ifdef VALIDATE_PARMS
    if(!rd_clr || !wrap)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, ACB_IF, ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG, reg_acbif_block_acbif_pm_counters_gen_cfg);

    *rd_clr = RU_FIELD_GET(0, ACB_IF, ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG, RD_CLR, reg_acbif_block_acbif_pm_counters_gen_cfg);
    *wrap = RU_FIELD_GET(0, ACB_IF, ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG, WRAP, reg_acbif_block_acbif_pm_counters_gen_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_acb_if_acbif_block_acbif_debug_dbgsel_set(uint8_t vs)
{
    uint32_t reg_acbif_block_acbif_debug_dbgsel=0;

#ifdef VALIDATE_PARMS
    if((vs >= _7BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_acbif_block_acbif_debug_dbgsel = RU_FIELD_SET(0, ACB_IF, ACBIF_BLOCK_ACBIF_DEBUG_DBGSEL, VS, reg_acbif_block_acbif_debug_dbgsel, vs);

    RU_REG_WRITE(0, ACB_IF, ACBIF_BLOCK_ACBIF_DEBUG_DBGSEL, reg_acbif_block_acbif_debug_dbgsel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_acb_if_acbif_block_acbif_debug_dbgsel_get(uint8_t *vs)
{
    uint32_t reg_acbif_block_acbif_debug_dbgsel;

#ifdef VALIDATE_PARMS
    if(!vs)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, ACB_IF, ACBIF_BLOCK_ACBIF_DEBUG_DBGSEL, reg_acbif_block_acbif_debug_dbgsel);

    *vs = RU_FIELD_GET(0, ACB_IF, ACBIF_BLOCK_ACBIF_DEBUG_DBGSEL, VS, reg_acbif_block_acbif_debug_dbgsel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_acb_if_acbif_block_acbif_debug_dbgbus_get(uint32_t *val)
{
    uint32_t reg_acbif_block_acbif_debug_dbgbus;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, ACB_IF, ACBIF_BLOCK_ACBIF_DEBUG_DBGBUS, reg_acbif_block_acbif_debug_dbgbus);

    *val = RU_FIELD_GET(0, ACB_IF, ACBIF_BLOCK_ACBIF_DEBUG_DBGBUS, VAL, reg_acbif_block_acbif_debug_dbgbus);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_acb_if_acbif_block_acbif_debug_stat_get(uint32_t idx, uint32_t *val)
{
    uint32_t reg_acbif_block_acbif_debug_stat;

#ifdef VALIDATE_PARMS
    if(!val)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((idx >= 2))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_RAM_READ(0, idx, ACB_IF, ACBIF_BLOCK_ACBIF_DEBUG_STAT, reg_acbif_block_acbif_debug_stat);

    *val = RU_FIELD_GET(0, ACB_IF, ACBIF_BLOCK_ACBIF_DEBUG_STAT, VAL, reg_acbif_block_acbif_debug_stat);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
typedef enum
{
    bdmf_address_acbif_block_acbif_config_conf0,
    bdmf_address_acbif_block_acbif_pm_counters_cmd_type,
    bdmf_address_acbif_block_acbif_pm_counters_cmd_imp,
    bdmf_address_acbif_block_acbif_pm_counters_agg,
    bdmf_address_acbif_block_acbif_pm_counters_buffs,
    bdmf_address_acbif_block_acbif_pm_counters_gen_cfg,
    bdmf_address_acbif_block_acbif_debug_dbgsel,
    bdmf_address_acbif_block_acbif_debug_dbgbus,
    bdmf_address_acbif_block_acbif_debug_stat,
}
bdmf_address;

static int bcm_acb_if_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_acb_if_conf:
        err = ag_drv_acb_if_conf_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_acb_if_acbif_block_acbif_pm_counters_gen_cfg:
        err = ag_drv_acb_if_acbif_block_acbif_pm_counters_gen_cfg_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_acb_if_acbif_block_acbif_debug_dbgsel:
        err = ag_drv_acb_if_acbif_block_acbif_debug_dbgsel_set(parm[1].value.unumber);
        break;
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

int bcm_acb_if_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_acb_if_conf:
    {
        bdmf_boolean crc_add;
        uint8_t val_loc;
        err = ag_drv_acb_if_conf_get(&crc_add, &val_loc);
        bdmf_session_print(session, "crc_add = %u (0x%x)\n", crc_add, crc_add);
        bdmf_session_print(session, "val_loc = %u (0x%x)\n", val_loc, val_loc);
        break;
    }
    case cli_acb_if_acbif_block_acbif_pm_counters_cmd_type:
    {
        uint32_t val;
        err = ag_drv_acb_if_acbif_block_acbif_pm_counters_cmd_type_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_acb_if_acbif_block_acbif_pm_counters_cmd_imp:
    {
        uint32_t val;
        err = ag_drv_acb_if_acbif_block_acbif_pm_counters_cmd_imp_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_acb_if_acbif_block_acbif_pm_counters_agg:
    {
        uint32_t val;
        err = ag_drv_acb_if_acbif_block_acbif_pm_counters_agg_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_acb_if_acbif_block_acbif_pm_counters_buffs:
    {
        uint32_t val;
        err = ag_drv_acb_if_acbif_block_acbif_pm_counters_buffs_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_acb_if_acbif_block_acbif_pm_counters_gen_cfg:
    {
        bdmf_boolean rd_clr;
        bdmf_boolean wrap;
        err = ag_drv_acb_if_acbif_block_acbif_pm_counters_gen_cfg_get(&rd_clr, &wrap);
        bdmf_session_print(session, "rd_clr = %u (0x%x)\n", rd_clr, rd_clr);
        bdmf_session_print(session, "wrap = %u (0x%x)\n", wrap, wrap);
        break;
    }
    case cli_acb_if_acbif_block_acbif_debug_dbgsel:
    {
        uint8_t vs;
        err = ag_drv_acb_if_acbif_block_acbif_debug_dbgsel_get(&vs);
        bdmf_session_print(session, "vs = %u (0x%x)\n", vs, vs);
        break;
    }
    case cli_acb_if_acbif_block_acbif_debug_dbgbus:
    {
        uint32_t val;
        err = ag_drv_acb_if_acbif_block_acbif_debug_dbgbus_get(&val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    case cli_acb_if_acbif_block_acbif_debug_stat:
    {
        uint32_t val;
        err = ag_drv_acb_if_acbif_block_acbif_debug_stat_get(parm[1].value.unumber, &val);
        bdmf_session_print(session, "val = %u (0x%x)\n", val, val);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_acb_if_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        bdmf_boolean crc_add=gtmv(m, 1);
        uint8_t val_loc=gtmv(m, 3);
        if(!err) bdmf_session_print(session, "ag_drv_acb_if_conf_set( %u %u)\n", crc_add, val_loc);
        if(!err) ag_drv_acb_if_conf_set(crc_add, val_loc);
        if(!err) ag_drv_acb_if_conf_get( &crc_add, &val_loc);
        if(!err) bdmf_session_print(session, "ag_drv_acb_if_conf_get( %u %u)\n", crc_add, val_loc);
        if(err || crc_add!=gtmv(m, 1) || val_loc!=gtmv(m, 3))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t idx=gtmv(m, 0);
        uint32_t val=gtmv(m, 32);
        if(!err) ag_drv_acb_if_acbif_block_acbif_pm_counters_cmd_type_get( idx, &val);
        if(!err) bdmf_session_print(session, "ag_drv_acb_if_acbif_block_acbif_pm_counters_cmd_type_get( %u %u)\n", idx, val);
    }
    {
        uint32_t idx=gtmv(m, 0);
        uint32_t val=gtmv(m, 32);
        if(!err) ag_drv_acb_if_acbif_block_acbif_pm_counters_cmd_imp_get( idx, &val);
        if(!err) bdmf_session_print(session, "ag_drv_acb_if_acbif_block_acbif_pm_counters_cmd_imp_get( %u %u)\n", idx, val);
    }
    {
        uint32_t idx=gtmv(m, 1);
        uint32_t val=gtmv(m, 32);
        if(!err) ag_drv_acb_if_acbif_block_acbif_pm_counters_agg_get( idx, &val);
        if(!err) bdmf_session_print(session, "ag_drv_acb_if_acbif_block_acbif_pm_counters_agg_get( %u %u)\n", idx, val);
    }
    {
        uint32_t idx=gtmv(m, 1);
        uint32_t val=gtmv(m, 32);
        if(!err) ag_drv_acb_if_acbif_block_acbif_pm_counters_buffs_get( idx, &val);
        if(!err) bdmf_session_print(session, "ag_drv_acb_if_acbif_block_acbif_pm_counters_buffs_get( %u %u)\n", idx, val);
    }
    {
        bdmf_boolean rd_clr=gtmv(m, 1);
        bdmf_boolean wrap=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_acb_if_acbif_block_acbif_pm_counters_gen_cfg_set( %u %u)\n", rd_clr, wrap);
        if(!err) ag_drv_acb_if_acbif_block_acbif_pm_counters_gen_cfg_set(rd_clr, wrap);
        if(!err) ag_drv_acb_if_acbif_block_acbif_pm_counters_gen_cfg_get( &rd_clr, &wrap);
        if(!err) bdmf_session_print(session, "ag_drv_acb_if_acbif_block_acbif_pm_counters_gen_cfg_get( %u %u)\n", rd_clr, wrap);
        if(err || rd_clr!=gtmv(m, 1) || wrap!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t vs=gtmv(m, 7);
        if(!err) bdmf_session_print(session, "ag_drv_acb_if_acbif_block_acbif_debug_dbgsel_set( %u)\n", vs);
        if(!err) ag_drv_acb_if_acbif_block_acbif_debug_dbgsel_set(vs);
        if(!err) ag_drv_acb_if_acbif_block_acbif_debug_dbgsel_get( &vs);
        if(!err) bdmf_session_print(session, "ag_drv_acb_if_acbif_block_acbif_debug_dbgsel_get( %u)\n", vs);
        if(err || vs!=gtmv(m, 7))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t val=gtmv(m, 21);
        if(!err) ag_drv_acb_if_acbif_block_acbif_debug_dbgbus_get( &val);
        if(!err) bdmf_session_print(session, "ag_drv_acb_if_acbif_block_acbif_debug_dbgbus_get( %u)\n", val);
    }
    {
        uint32_t idx=gtmv(m, 1);
        uint32_t val=gtmv(m, 32);
        if(!err) ag_drv_acb_if_acbif_block_acbif_debug_stat_get( idx, &val);
        if(!err) bdmf_session_print(session, "ag_drv_acb_if_acbif_block_acbif_debug_stat_get( %u %u)\n", idx, val);
    }
    return err;
}

static int bcm_acb_if_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_acbif_block_acbif_config_conf0 : reg = &RU_REG(ACB_IF, ACBIF_BLOCK_ACBIF_CONFIG_CONF0); blk = &RU_BLK(ACB_IF); break;
    case bdmf_address_acbif_block_acbif_pm_counters_cmd_type : reg = &RU_REG(ACB_IF, ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_TYPE); blk = &RU_BLK(ACB_IF); break;
    case bdmf_address_acbif_block_acbif_pm_counters_cmd_imp : reg = &RU_REG(ACB_IF, ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_IMP); blk = &RU_BLK(ACB_IF); break;
    case bdmf_address_acbif_block_acbif_pm_counters_agg : reg = &RU_REG(ACB_IF, ACBIF_BLOCK_ACBIF_PM_COUNTERS_AGG); blk = &RU_BLK(ACB_IF); break;
    case bdmf_address_acbif_block_acbif_pm_counters_buffs : reg = &RU_REG(ACB_IF, ACBIF_BLOCK_ACBIF_PM_COUNTERS_BUFFS); blk = &RU_BLK(ACB_IF); break;
    case bdmf_address_acbif_block_acbif_pm_counters_gen_cfg : reg = &RU_REG(ACB_IF, ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG); blk = &RU_BLK(ACB_IF); break;
    case bdmf_address_acbif_block_acbif_debug_dbgsel : reg = &RU_REG(ACB_IF, ACBIF_BLOCK_ACBIF_DEBUG_DBGSEL); blk = &RU_BLK(ACB_IF); break;
    case bdmf_address_acbif_block_acbif_debug_dbgbus : reg = &RU_REG(ACB_IF, ACBIF_BLOCK_ACBIF_DEBUG_DBGBUS); blk = &RU_BLK(ACB_IF); break;
    case bdmf_address_acbif_block_acbif_debug_stat : reg = &RU_REG(ACB_IF, ACBIF_BLOCK_ACBIF_DEBUG_STAT); blk = &RU_BLK(ACB_IF); break;
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

bdmfmon_handle_t ag_drv_acb_if_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "acb_if"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "acb_if", "acb_if", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_conf[]={
            BDMFMON_MAKE_PARM("crc_add", "crc_add", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("val_loc", "val_loc", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_acbif_block_acbif_pm_counters_gen_cfg[]={
            BDMFMON_MAKE_PARM("rd_clr", "rd_clr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("wrap", "wrap", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_acbif_block_acbif_debug_dbgsel[]={
            BDMFMON_MAKE_PARM("vs", "vs", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="conf", .val=cli_acb_if_conf, .parms=set_conf },
            { .name="acbif_block_acbif_pm_counters_gen_cfg", .val=cli_acb_if_acbif_block_acbif_pm_counters_gen_cfg, .parms=set_acbif_block_acbif_pm_counters_gen_cfg },
            { .name="acbif_block_acbif_debug_dbgsel", .val=cli_acb_if_acbif_block_acbif_debug_dbgsel, .parms=set_acbif_block_acbif_debug_dbgsel },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_acb_if_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_acbif_block_acbif_pm_counters_cmd_type[]={
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_acbif_block_acbif_pm_counters_cmd_imp[]={
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_acbif_block_acbif_pm_counters_agg[]={
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_acbif_block_acbif_pm_counters_buffs[]={
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_acbif_block_acbif_debug_stat[]={
            BDMFMON_MAKE_PARM("idx", "idx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="conf", .val=cli_acb_if_conf, .parms=set_default },
            { .name="acbif_block_acbif_pm_counters_cmd_type", .val=cli_acb_if_acbif_block_acbif_pm_counters_cmd_type, .parms=set_acbif_block_acbif_pm_counters_cmd_type },
            { .name="acbif_block_acbif_pm_counters_cmd_imp", .val=cli_acb_if_acbif_block_acbif_pm_counters_cmd_imp, .parms=set_acbif_block_acbif_pm_counters_cmd_imp },
            { .name="acbif_block_acbif_pm_counters_agg", .val=cli_acb_if_acbif_block_acbif_pm_counters_agg, .parms=set_acbif_block_acbif_pm_counters_agg },
            { .name="acbif_block_acbif_pm_counters_buffs", .val=cli_acb_if_acbif_block_acbif_pm_counters_buffs, .parms=set_acbif_block_acbif_pm_counters_buffs },
            { .name="acbif_block_acbif_pm_counters_gen_cfg", .val=cli_acb_if_acbif_block_acbif_pm_counters_gen_cfg, .parms=set_default },
            { .name="acbif_block_acbif_debug_dbgsel", .val=cli_acb_if_acbif_block_acbif_debug_dbgsel, .parms=set_default },
            { .name="acbif_block_acbif_debug_dbgbus", .val=cli_acb_if_acbif_block_acbif_debug_dbgbus, .parms=set_default },
            { .name="acbif_block_acbif_debug_stat", .val=cli_acb_if_acbif_block_acbif_debug_stat, .parms=set_acbif_block_acbif_debug_stat },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_acb_if_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_acb_if_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="ACBIF_BLOCK_ACBIF_CONFIG_CONF0" , .val=bdmf_address_acbif_block_acbif_config_conf0 },
            { .name="ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_TYPE" , .val=bdmf_address_acbif_block_acbif_pm_counters_cmd_type },
            { .name="ACBIF_BLOCK_ACBIF_PM_COUNTERS_CMD_IMP" , .val=bdmf_address_acbif_block_acbif_pm_counters_cmd_imp },
            { .name="ACBIF_BLOCK_ACBIF_PM_COUNTERS_AGG" , .val=bdmf_address_acbif_block_acbif_pm_counters_agg },
            { .name="ACBIF_BLOCK_ACBIF_PM_COUNTERS_BUFFS" , .val=bdmf_address_acbif_block_acbif_pm_counters_buffs },
            { .name="ACBIF_BLOCK_ACBIF_PM_COUNTERS_GEN_CFG" , .val=bdmf_address_acbif_block_acbif_pm_counters_gen_cfg },
            { .name="ACBIF_BLOCK_ACBIF_DEBUG_DBGSEL" , .val=bdmf_address_acbif_block_acbif_debug_dbgsel },
            { .name="ACBIF_BLOCK_ACBIF_DEBUG_DBGBUS" , .val=bdmf_address_acbif_block_acbif_debug_dbgbus },
            { .name="ACBIF_BLOCK_ACBIF_DEBUG_STAT" , .val=bdmf_address_acbif_block_acbif_debug_stat },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_acb_if_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

