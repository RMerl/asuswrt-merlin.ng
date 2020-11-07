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
#include "wan_serdes_ag.h"
bdmf_error_t ag_drv_wan_serdes_pll_ctl_set(const wan_serdes_pll_ctl *pll_ctl)
{
    uint32_t reg_pll_ctl=0;

#ifdef VALIDATE_PARMS
    if(!pll_ctl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((pll_ctl->cfg_pll1_lcref_sel >= _1BITS_MAX_VAL_) ||
       (pll_ctl->cfg_pll1_refout_en >= _1BITS_MAX_VAL_) ||
       (pll_ctl->cfg_pll1_refin_en >= _1BITS_MAX_VAL_) ||
       (pll_ctl->cfg_pll0_lcref_sel >= _1BITS_MAX_VAL_) ||
       (pll_ctl->cfg_pll0_refout_en >= _1BITS_MAX_VAL_) ||
       (pll_ctl->cfg_pll0_refin_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pll_ctl = RU_FIELD_SET(0, WAN_SERDES, PLL_CTL, CFG_PLL1_LCREF_SEL, reg_pll_ctl, pll_ctl->cfg_pll1_lcref_sel);
    reg_pll_ctl = RU_FIELD_SET(0, WAN_SERDES, PLL_CTL, CFG_PLL1_REFOUT_EN, reg_pll_ctl, pll_ctl->cfg_pll1_refout_en);
    reg_pll_ctl = RU_FIELD_SET(0, WAN_SERDES, PLL_CTL, CFG_PLL1_REFIN_EN, reg_pll_ctl, pll_ctl->cfg_pll1_refin_en);
    reg_pll_ctl = RU_FIELD_SET(0, WAN_SERDES, PLL_CTL, CFG_PLL0_LCREF_SEL, reg_pll_ctl, pll_ctl->cfg_pll0_lcref_sel);
    reg_pll_ctl = RU_FIELD_SET(0, WAN_SERDES, PLL_CTL, CFG_PLL0_REFOUT_EN, reg_pll_ctl, pll_ctl->cfg_pll0_refout_en);
    reg_pll_ctl = RU_FIELD_SET(0, WAN_SERDES, PLL_CTL, CFG_PLL0_REFIN_EN, reg_pll_ctl, pll_ctl->cfg_pll0_refin_en);

    RU_REG_WRITE(0, WAN_SERDES, PLL_CTL, reg_pll_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_wan_serdes_pll_ctl_get(wan_serdes_pll_ctl *pll_ctl)
{
    uint32_t reg_pll_ctl=0;

#ifdef VALIDATE_PARMS
    if(!pll_ctl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, WAN_SERDES, PLL_CTL, reg_pll_ctl);

    pll_ctl->cfg_pll1_lcref_sel = RU_FIELD_GET(0, WAN_SERDES, PLL_CTL, CFG_PLL1_LCREF_SEL, reg_pll_ctl);
    pll_ctl->cfg_pll1_refout_en = RU_FIELD_GET(0, WAN_SERDES, PLL_CTL, CFG_PLL1_REFOUT_EN, reg_pll_ctl);
    pll_ctl->cfg_pll1_refin_en = RU_FIELD_GET(0, WAN_SERDES, PLL_CTL, CFG_PLL1_REFIN_EN, reg_pll_ctl);
    pll_ctl->cfg_pll0_lcref_sel = RU_FIELD_GET(0, WAN_SERDES, PLL_CTL, CFG_PLL0_LCREF_SEL, reg_pll_ctl);
    pll_ctl->cfg_pll0_refout_en = RU_FIELD_GET(0, WAN_SERDES, PLL_CTL, CFG_PLL0_REFOUT_EN, reg_pll_ctl);
    pll_ctl->cfg_pll0_refin_en = RU_FIELD_GET(0, WAN_SERDES, PLL_CTL, CFG_PLL0_REFIN_EN, reg_pll_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_wan_serdes_temp_ctl_get(uint16_t *wan_temperature_data)
{
    uint32_t reg_temp_ctl=0;

#ifdef VALIDATE_PARMS
    if(!wan_temperature_data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, WAN_SERDES, TEMP_CTL, reg_temp_ctl);

    *wan_temperature_data = RU_FIELD_GET(0, WAN_SERDES, TEMP_CTL, WAN_TEMPERATURE_DATA, reg_temp_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_wan_serdes_pram_ctl_set(const wan_serdes_pram_ctl *pram_ctl)
{
    uint32_t reg_pram_ctl=0;

#ifdef VALIDATE_PARMS
    if(!pram_ctl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((pram_ctl->cfg_pram_we >= _1BITS_MAX_VAL_) ||
       (pram_ctl->cfg_pram_cs >= _1BITS_MAX_VAL_) ||
       (pram_ctl->cfg_pram_ability >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pram_ctl = RU_FIELD_SET(0, WAN_SERDES, PRAM_CTL, CFG_PRAM_WE, reg_pram_ctl, pram_ctl->cfg_pram_we);
    reg_pram_ctl = RU_FIELD_SET(0, WAN_SERDES, PRAM_CTL, CFG_PRAM_CS, reg_pram_ctl, pram_ctl->cfg_pram_cs);
    reg_pram_ctl = RU_FIELD_SET(0, WAN_SERDES, PRAM_CTL, CFG_PRAM_ABILITY, reg_pram_ctl, pram_ctl->cfg_pram_ability);
    reg_pram_ctl = RU_FIELD_SET(0, WAN_SERDES, PRAM_CTL, CFG_PRAM_DATAIN, reg_pram_ctl, pram_ctl->cfg_pram_datain);
    reg_pram_ctl = RU_FIELD_SET(0, WAN_SERDES, PRAM_CTL, CFG_PRAM_ADDR, reg_pram_ctl, pram_ctl->cfg_pram_addr);

    RU_REG_WRITE(0, WAN_SERDES, PRAM_CTL, reg_pram_ctl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_wan_serdes_pram_ctl_get(wan_serdes_pram_ctl *pram_ctl)
{
    uint32_t reg_pram_ctl=0;

#ifdef VALIDATE_PARMS
    if(!pram_ctl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, WAN_SERDES, PRAM_CTL, reg_pram_ctl);

    pram_ctl->cfg_pram_we = RU_FIELD_GET(0, WAN_SERDES, PRAM_CTL, CFG_PRAM_WE, reg_pram_ctl);
    pram_ctl->cfg_pram_cs = RU_FIELD_GET(0, WAN_SERDES, PRAM_CTL, CFG_PRAM_CS, reg_pram_ctl);
    pram_ctl->cfg_pram_ability = RU_FIELD_GET(0, WAN_SERDES, PRAM_CTL, CFG_PRAM_ABILITY, reg_pram_ctl);
    pram_ctl->cfg_pram_datain = RU_FIELD_GET(0, WAN_SERDES, PRAM_CTL, CFG_PRAM_DATAIN, reg_pram_ctl);
    pram_ctl->cfg_pram_addr = RU_FIELD_GET(0, WAN_SERDES, PRAM_CTL, CFG_PRAM_ADDR, reg_pram_ctl);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
enum
{
    BDMF_pll_ctl,
    BDMF_temp_ctl,
    BDMF_pram_ctl,
};

typedef enum
{
    bdmf_address_pll_ctl,
    bdmf_address_temp_ctl,
    bdmf_address_pram_ctl,
}
bdmf_address;

static int bcm_wan_serdes_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err;

    switch(parm[0].value.unumber)
    {
    case BDMF_pll_ctl:
    {
        wan_serdes_pll_ctl pll_ctl = { .cfg_pll1_lcref_sel=parm[1].value.unumber, .cfg_pll1_refout_en=parm[2].value.unumber, .cfg_pll1_refin_en=parm[3].value.unumber, .cfg_pll0_lcref_sel=parm[4].value.unumber, .cfg_pll0_refout_en=parm[5].value.unumber, .cfg_pll0_refin_en=parm[6].value.unumber};
        err = ag_drv_wan_serdes_pll_ctl_set(&pll_ctl);
        break;
    }
    case BDMF_pram_ctl:
    {
        wan_serdes_pram_ctl pram_ctl = { .cfg_pram_we=parm[1].value.unumber, .cfg_pram_cs=parm[2].value.unumber, .cfg_pram_ability=parm[3].value.unumber, .cfg_pram_datain=parm[4].value.unumber, .cfg_pram_addr=parm[5].value.unumber};
        err = ag_drv_wan_serdes_pram_ctl_set(&pram_ctl);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_wan_serdes_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err;

    switch(parm[0].value.unumber)
    {
    case BDMF_pll_ctl:
    {
        wan_serdes_pll_ctl pll_ctl;
        err = ag_drv_wan_serdes_pll_ctl_get(&pll_ctl);
        bdmf_session_print(session, "cfg_pll1_lcref_sel = %u = 0x%x\n", pll_ctl.cfg_pll1_lcref_sel, pll_ctl.cfg_pll1_lcref_sel);
        bdmf_session_print(session, "cfg_pll1_refout_en = %u = 0x%x\n", pll_ctl.cfg_pll1_refout_en, pll_ctl.cfg_pll1_refout_en);
        bdmf_session_print(session, "cfg_pll1_refin_en = %u = 0x%x\n", pll_ctl.cfg_pll1_refin_en, pll_ctl.cfg_pll1_refin_en);
        bdmf_session_print(session, "cfg_pll0_lcref_sel = %u = 0x%x\n", pll_ctl.cfg_pll0_lcref_sel, pll_ctl.cfg_pll0_lcref_sel);
        bdmf_session_print(session, "cfg_pll0_refout_en = %u = 0x%x\n", pll_ctl.cfg_pll0_refout_en, pll_ctl.cfg_pll0_refout_en);
        bdmf_session_print(session, "cfg_pll0_refin_en = %u = 0x%x\n", pll_ctl.cfg_pll0_refin_en, pll_ctl.cfg_pll0_refin_en);
        break;
    }
    case BDMF_temp_ctl:
    {
        uint16_t wan_temperature_data;
        err = ag_drv_wan_serdes_temp_ctl_get(&wan_temperature_data);
        bdmf_session_print(session, "wan_temperature_data = %u = 0x%x\n", wan_temperature_data, wan_temperature_data);
        break;
    }
    case BDMF_pram_ctl:
    {
        wan_serdes_pram_ctl pram_ctl;
        err = ag_drv_wan_serdes_pram_ctl_get(&pram_ctl);
        bdmf_session_print(session, "cfg_pram_we = %u = 0x%x\n", pram_ctl.cfg_pram_we, pram_ctl.cfg_pram_we);
        bdmf_session_print(session, "cfg_pram_cs = %u = 0x%x\n", pram_ctl.cfg_pram_cs, pram_ctl.cfg_pram_cs);
        bdmf_session_print(session, "cfg_pram_ability = %u = 0x%x\n", pram_ctl.cfg_pram_ability, pram_ctl.cfg_pram_ability);
        bdmf_session_print(session, "cfg_pram_datain = %u = 0x%x\n", pram_ctl.cfg_pram_datain, pram_ctl.cfg_pram_datain);
        bdmf_session_print(session, "cfg_pram_addr = %u = 0x%x\n", pram_ctl.cfg_pram_addr, pram_ctl.cfg_pram_addr);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_wan_serdes_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        wan_serdes_pll_ctl pll_ctl = {.cfg_pll1_lcref_sel=gtmv(m, 1), .cfg_pll1_refout_en=gtmv(m, 1), .cfg_pll1_refin_en=gtmv(m, 1), .cfg_pll0_lcref_sel=gtmv(m, 1), .cfg_pll0_refout_en=gtmv(m, 1), .cfg_pll0_refin_en=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_wan_serdes_pll_ctl_set( %u %u %u %u %u %u)\n", pll_ctl.cfg_pll1_lcref_sel, pll_ctl.cfg_pll1_refout_en, pll_ctl.cfg_pll1_refin_en, pll_ctl.cfg_pll0_lcref_sel, pll_ctl.cfg_pll0_refout_en, pll_ctl.cfg_pll0_refin_en);
        if(!err) ag_drv_wan_serdes_pll_ctl_set(&pll_ctl);
        if(!err) ag_drv_wan_serdes_pll_ctl_get( &pll_ctl);
        if(!err) bdmf_session_print(session, "ag_drv_wan_serdes_pll_ctl_get( %u %u %u %u %u %u)\n", pll_ctl.cfg_pll1_lcref_sel, pll_ctl.cfg_pll1_refout_en, pll_ctl.cfg_pll1_refin_en, pll_ctl.cfg_pll0_lcref_sel, pll_ctl.cfg_pll0_refout_en, pll_ctl.cfg_pll0_refin_en);
        if(err || pll_ctl.cfg_pll1_lcref_sel!=gtmv(m, 1) || pll_ctl.cfg_pll1_refout_en!=gtmv(m, 1) || pll_ctl.cfg_pll1_refin_en!=gtmv(m, 1) || pll_ctl.cfg_pll0_lcref_sel!=gtmv(m, 1) || pll_ctl.cfg_pll0_refout_en!=gtmv(m, 1) || pll_ctl.cfg_pll0_refin_en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t wan_temperature_data=gtmv(m, 10);
        if(!err) ag_drv_wan_serdes_temp_ctl_get( &wan_temperature_data);
        if(!err) bdmf_session_print(session, "ag_drv_wan_serdes_temp_ctl_get( %u)\n", wan_temperature_data);
    }
    {
        wan_serdes_pram_ctl pram_ctl = {.cfg_pram_we=gtmv(m, 1), .cfg_pram_cs=gtmv(m, 1), .cfg_pram_ability=gtmv(m, 1), .cfg_pram_datain=gtmv(m, 8), .cfg_pram_addr=gtmv(m, 16)};
        if(!err) bdmf_session_print(session, "ag_drv_wan_serdes_pram_ctl_set( %u %u %u %u %u)\n", pram_ctl.cfg_pram_we, pram_ctl.cfg_pram_cs, pram_ctl.cfg_pram_ability, pram_ctl.cfg_pram_datain, pram_ctl.cfg_pram_addr);
        if(!err) ag_drv_wan_serdes_pram_ctl_set(&pram_ctl);
        if(!err) ag_drv_wan_serdes_pram_ctl_get( &pram_ctl);
        if(!err) bdmf_session_print(session, "ag_drv_wan_serdes_pram_ctl_get( %u %u %u %u %u)\n", pram_ctl.cfg_pram_we, pram_ctl.cfg_pram_cs, pram_ctl.cfg_pram_ability, pram_ctl.cfg_pram_datain, pram_ctl.cfg_pram_addr);
        if(err || pram_ctl.cfg_pram_we!=gtmv(m, 1) || pram_ctl.cfg_pram_cs!=gtmv(m, 1) || pram_ctl.cfg_pram_ability!=gtmv(m, 1) || pram_ctl.cfg_pram_datain!=gtmv(m, 8) || pram_ctl.cfg_pram_addr!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    return err;
}

static int bcm_wan_serdes_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_pll_ctl : reg = &RU_REG(WAN_SERDES, PLL_CTL); blk = &RU_BLK(WAN_SERDES); break;
    case bdmf_address_temp_ctl : reg = &RU_REG(WAN_SERDES, TEMP_CTL); blk = &RU_BLK(WAN_SERDES); break;
    case bdmf_address_pram_ctl : reg = &RU_REG(WAN_SERDES, PRAM_CTL); blk = &RU_BLK(WAN_SERDES); break;
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

bdmfmon_handle_t ag_drv_wan_serdes_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "wan_serdes"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "wan_serdes", "wan_serdes", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_pll_ctl[]={
            BDMFMON_MAKE_PARM("cfg_pll1_lcref_sel", "cfg_pll1_lcref_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfg_pll1_refout_en", "cfg_pll1_refout_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfg_pll1_refin_en", "cfg_pll1_refin_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfg_pll0_lcref_sel", "cfg_pll0_lcref_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfg_pll0_refout_en", "cfg_pll0_refout_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfg_pll0_refin_en", "cfg_pll0_refin_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pram_ctl[]={
            BDMFMON_MAKE_PARM("cfg_pram_we", "cfg_pram_we", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfg_pram_cs", "cfg_pram_cs", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfg_pram_ability", "cfg_pram_ability", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfg_pram_datain", "cfg_pram_datain", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfg_pram_addr", "cfg_pram_addr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="pll_ctl", .val=BDMF_pll_ctl, .parms=set_pll_ctl },
            { .name="pram_ctl", .val=BDMF_pram_ctl, .parms=set_pram_ctl },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_wan_serdes_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="pll_ctl", .val=BDMF_pll_ctl, .parms=set_default },
            { .name="temp_ctl", .val=BDMF_temp_ctl, .parms=set_default },
            { .name="pram_ctl", .val=BDMF_pram_ctl, .parms=set_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_wan_serdes_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_wan_serdes_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="PLL_CTL" , .val=bdmf_address_pll_ctl },
            { .name="TEMP_CTL" , .val=bdmf_address_temp_ctl },
            { .name="PRAM_CTL" , .val=bdmf_address_pram_ctl },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_wan_serdes_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

