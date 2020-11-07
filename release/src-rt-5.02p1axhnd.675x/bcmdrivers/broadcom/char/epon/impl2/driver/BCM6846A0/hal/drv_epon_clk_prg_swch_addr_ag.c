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

#include "drivers_epon_ag.h"
#include "drv_epon_clk_prg_swch_addr_ag.h"
bdmf_error_t ag_drv_clk_prg_swch_addr_clk_prg_config_set(uint8_t cfgprgclksel, uint32_t cfgprgclkdivide)
{
    uint32_t reg_clk_prg_config=0;

#ifdef VALIDATE_PARMS
    if((cfgprgclksel >= _3BITS_MAX_VAL_) ||
       (cfgprgclkdivide >= _28BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_clk_prg_config = RU_FIELD_SET(0, CLK_PRG_SWCH_ADDR, CLK_PRG_CONFIG, CFGPRGCLKSEL, reg_clk_prg_config, cfgprgclksel);
    reg_clk_prg_config = RU_FIELD_SET(0, CLK_PRG_SWCH_ADDR, CLK_PRG_CONFIG, CFGPRGCLKDIVIDE, reg_clk_prg_config, cfgprgclkdivide);

    RU_REG_WRITE(0, CLK_PRG_SWCH_ADDR, CLK_PRG_CONFIG, reg_clk_prg_config);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_clk_prg_swch_addr_clk_prg_config_get(uint8_t *cfgprgclksel, uint32_t *cfgprgclkdivide)
{
    uint32_t reg_clk_prg_config=0;

#ifdef VALIDATE_PARMS
    if(!cfgprgclksel || !cfgprgclkdivide)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, CLK_PRG_SWCH_ADDR, CLK_PRG_CONFIG, reg_clk_prg_config);

    *cfgprgclksel = RU_FIELD_GET(0, CLK_PRG_SWCH_ADDR, CLK_PRG_CONFIG, CFGPRGCLKSEL, reg_clk_prg_config);
    *cfgprgclkdivide = RU_FIELD_GET(0, CLK_PRG_SWCH_ADDR, CLK_PRG_CONFIG, CFGPRGCLKDIVIDE, reg_clk_prg_config);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_clk_prg_swch_addr_clk_prg_config2_set(uint16_t cfgprgclkdenom)
{
    uint32_t reg_clk_prg_config2=0;

#ifdef VALIDATE_PARMS
    if((cfgprgclkdenom >= _13BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_clk_prg_config2 = RU_FIELD_SET(0, CLK_PRG_SWCH_ADDR, CLK_PRG_CONFIG2, CFGPRGCLKDENOM, reg_clk_prg_config2, cfgprgclkdenom);

    RU_REG_WRITE(0, CLK_PRG_SWCH_ADDR, CLK_PRG_CONFIG2, reg_clk_prg_config2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_clk_prg_swch_addr_clk_prg_config2_get(uint16_t *cfgprgclkdenom)
{
    uint32_t reg_clk_prg_config2=0;

#ifdef VALIDATE_PARMS
    if(!cfgprgclkdenom)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, CLK_PRG_SWCH_ADDR, CLK_PRG_CONFIG2, reg_clk_prg_config2);

    *cfgprgclkdenom = RU_FIELD_GET(0, CLK_PRG_SWCH_ADDR, CLK_PRG_CONFIG2, CFGPRGCLKDENOM, reg_clk_prg_config2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_clk_prg_swch_addr_clk_prg_config_1_set(uint8_t cfgprgclksel_1, uint32_t cfgprgclkdivide_1)
{
    uint32_t reg_clk_prg_config_1=0;

#ifdef VALIDATE_PARMS
    if((cfgprgclksel_1 >= _3BITS_MAX_VAL_) ||
       (cfgprgclkdivide_1 >= _28BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_clk_prg_config_1 = RU_FIELD_SET(0, CLK_PRG_SWCH_ADDR, CLK_PRG_CONFIG_1, CFGPRGCLKSEL_1, reg_clk_prg_config_1, cfgprgclksel_1);
    reg_clk_prg_config_1 = RU_FIELD_SET(0, CLK_PRG_SWCH_ADDR, CLK_PRG_CONFIG_1, CFGPRGCLKDIVIDE_1, reg_clk_prg_config_1, cfgprgclkdivide_1);

    RU_REG_WRITE(0, CLK_PRG_SWCH_ADDR, CLK_PRG_CONFIG_1, reg_clk_prg_config_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_clk_prg_swch_addr_clk_prg_config_1_get(uint8_t *cfgprgclksel_1, uint32_t *cfgprgclkdivide_1)
{
    uint32_t reg_clk_prg_config_1=0;

#ifdef VALIDATE_PARMS
    if(!cfgprgclksel_1 || !cfgprgclkdivide_1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, CLK_PRG_SWCH_ADDR, CLK_PRG_CONFIG_1, reg_clk_prg_config_1);

    *cfgprgclksel_1 = RU_FIELD_GET(0, CLK_PRG_SWCH_ADDR, CLK_PRG_CONFIG_1, CFGPRGCLKSEL_1, reg_clk_prg_config_1);
    *cfgprgclkdivide_1 = RU_FIELD_GET(0, CLK_PRG_SWCH_ADDR, CLK_PRG_CONFIG_1, CFGPRGCLKDIVIDE_1, reg_clk_prg_config_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_clk_prg_swch_addr_clk_prg_config2_1_set(uint16_t cfgprgclkdenom_1)
{
    uint32_t reg_clk_prg_config2_1=0;

#ifdef VALIDATE_PARMS
    if((cfgprgclkdenom_1 >= _13BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_clk_prg_config2_1 = RU_FIELD_SET(0, CLK_PRG_SWCH_ADDR, CLK_PRG_CONFIG2_1, CFGPRGCLKDENOM_1, reg_clk_prg_config2_1, cfgprgclkdenom_1);

    RU_REG_WRITE(0, CLK_PRG_SWCH_ADDR, CLK_PRG_CONFIG2_1, reg_clk_prg_config2_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_clk_prg_swch_addr_clk_prg_config2_1_get(uint16_t *cfgprgclkdenom_1)
{
    uint32_t reg_clk_prg_config2_1=0;

#ifdef VALIDATE_PARMS
    if(!cfgprgclkdenom_1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, CLK_PRG_SWCH_ADDR, CLK_PRG_CONFIG2_1, reg_clk_prg_config2_1);

    *cfgprgclkdenom_1 = RU_FIELD_GET(0, CLK_PRG_SWCH_ADDR, CLK_PRG_CONFIG2_1, CFGPRGCLKDENOM_1, reg_clk_prg_config2_1);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
enum
{
    BDMF_clk_prg_config,
    BDMF_clk_prg_config2,
    BDMF_clk_prg_config_1,
    BDMF_clk_prg_config2_1,
};

typedef enum
{
    bdmf_address_clk_prg_config,
    bdmf_address_clk_prg_config2,
    bdmf_address_clk_prg_config_1,
    bdmf_address_clk_prg_config2_1,
}
bdmf_address;

static int bcm_clk_prg_swch_addr_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err;

    switch(parm[0].value.unumber)
    {
    case BDMF_clk_prg_config:
        err = ag_drv_clk_prg_swch_addr_clk_prg_config_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case BDMF_clk_prg_config2:
        err = ag_drv_clk_prg_swch_addr_clk_prg_config2_set(parm[1].value.unumber);
        break;
    case BDMF_clk_prg_config_1:
        err = ag_drv_clk_prg_swch_addr_clk_prg_config_1_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case BDMF_clk_prg_config2_1:
        err = ag_drv_clk_prg_swch_addr_clk_prg_config2_1_set(parm[1].value.unumber);
        break;
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_clk_prg_swch_addr_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err;

    switch(parm[0].value.unumber)
    {
    case BDMF_clk_prg_config:
    {
        uint8_t cfgprgclksel;
        uint32_t cfgprgclkdivide;
        err = ag_drv_clk_prg_swch_addr_clk_prg_config_get(&cfgprgclksel, &cfgprgclkdivide);
        bdmf_session_print(session, "cfgprgclksel = %u = 0x%x\n", cfgprgclksel, cfgprgclksel);
        bdmf_session_print(session, "cfgprgclkdivide = %u = 0x%x\n", cfgprgclkdivide, cfgprgclkdivide);
        break;
    }
    case BDMF_clk_prg_config2:
    {
        uint16_t cfgprgclkdenom;
        err = ag_drv_clk_prg_swch_addr_clk_prg_config2_get(&cfgprgclkdenom);
        bdmf_session_print(session, "cfgprgclkdenom = %u = 0x%x\n", cfgprgclkdenom, cfgprgclkdenom);
        break;
    }
    case BDMF_clk_prg_config_1:
    {
        uint8_t cfgprgclksel_1;
        uint32_t cfgprgclkdivide_1;
        err = ag_drv_clk_prg_swch_addr_clk_prg_config_1_get(&cfgprgclksel_1, &cfgprgclkdivide_1);
        bdmf_session_print(session, "cfgprgclksel_1 = %u = 0x%x\n", cfgprgclksel_1, cfgprgclksel_1);
        bdmf_session_print(session, "cfgprgclkdivide_1 = %u = 0x%x\n", cfgprgclkdivide_1, cfgprgclkdivide_1);
        break;
    }
    case BDMF_clk_prg_config2_1:
    {
        uint16_t cfgprgclkdenom_1;
        err = ag_drv_clk_prg_swch_addr_clk_prg_config2_1_get(&cfgprgclkdenom_1);
        bdmf_session_print(session, "cfgprgclkdenom_1 = %u = 0x%x\n", cfgprgclkdenom_1, cfgprgclkdenom_1);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_clk_prg_swch_addr_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        uint8_t cfgprgclksel=gtmv(m, 3);
        uint32_t cfgprgclkdivide=gtmv(m, 28);
        if(!err) bdmf_session_print(session, "ag_drv_clk_prg_swch_addr_clk_prg_config_set( %u %u)\n", cfgprgclksel, cfgprgclkdivide);
        if(!err) ag_drv_clk_prg_swch_addr_clk_prg_config_set(cfgprgclksel, cfgprgclkdivide);
        if(!err) ag_drv_clk_prg_swch_addr_clk_prg_config_get( &cfgprgclksel, &cfgprgclkdivide);
        if(!err) bdmf_session_print(session, "ag_drv_clk_prg_swch_addr_clk_prg_config_get( %u %u)\n", cfgprgclksel, cfgprgclkdivide);
        if(err || cfgprgclksel!=gtmv(m, 3) || cfgprgclkdivide!=gtmv(m, 28))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cfgprgclkdenom=gtmv(m, 13);
        if(!err) bdmf_session_print(session, "ag_drv_clk_prg_swch_addr_clk_prg_config2_set( %u)\n", cfgprgclkdenom);
        if(!err) ag_drv_clk_prg_swch_addr_clk_prg_config2_set(cfgprgclkdenom);
        if(!err) ag_drv_clk_prg_swch_addr_clk_prg_config2_get( &cfgprgclkdenom);
        if(!err) bdmf_session_print(session, "ag_drv_clk_prg_swch_addr_clk_prg_config2_get( %u)\n", cfgprgclkdenom);
        if(err || cfgprgclkdenom!=gtmv(m, 13))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t cfgprgclksel_1=gtmv(m, 3);
        uint32_t cfgprgclkdivide_1=gtmv(m, 28);
        if(!err) bdmf_session_print(session, "ag_drv_clk_prg_swch_addr_clk_prg_config_1_set( %u %u)\n", cfgprgclksel_1, cfgprgclkdivide_1);
        if(!err) ag_drv_clk_prg_swch_addr_clk_prg_config_1_set(cfgprgclksel_1, cfgprgclkdivide_1);
        if(!err) ag_drv_clk_prg_swch_addr_clk_prg_config_1_get( &cfgprgclksel_1, &cfgprgclkdivide_1);
        if(!err) bdmf_session_print(session, "ag_drv_clk_prg_swch_addr_clk_prg_config_1_get( %u %u)\n", cfgprgclksel_1, cfgprgclkdivide_1);
        if(err || cfgprgclksel_1!=gtmv(m, 3) || cfgprgclkdivide_1!=gtmv(m, 28))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cfgprgclkdenom_1=gtmv(m, 13);
        if(!err) bdmf_session_print(session, "ag_drv_clk_prg_swch_addr_clk_prg_config2_1_set( %u)\n", cfgprgclkdenom_1);
        if(!err) ag_drv_clk_prg_swch_addr_clk_prg_config2_1_set(cfgprgclkdenom_1);
        if(!err) ag_drv_clk_prg_swch_addr_clk_prg_config2_1_get( &cfgprgclkdenom_1);
        if(!err) bdmf_session_print(session, "ag_drv_clk_prg_swch_addr_clk_prg_config2_1_get( %u)\n", cfgprgclkdenom_1);
        if(err || cfgprgclkdenom_1!=gtmv(m, 13))
            return err ? err : BDMF_ERR_IO;
    }
    return err;
}

static int bcm_clk_prg_swch_addr_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_clk_prg_config : reg = &RU_REG(CLK_PRG_SWCH_ADDR, CLK_PRG_CONFIG); blk = &RU_BLK(CLK_PRG_SWCH_ADDR); break;
    case bdmf_address_clk_prg_config2 : reg = &RU_REG(CLK_PRG_SWCH_ADDR, CLK_PRG_CONFIG2); blk = &RU_BLK(CLK_PRG_SWCH_ADDR); break;
    case bdmf_address_clk_prg_config_1 : reg = &RU_REG(CLK_PRG_SWCH_ADDR, CLK_PRG_CONFIG_1); blk = &RU_BLK(CLK_PRG_SWCH_ADDR); break;
    case bdmf_address_clk_prg_config2_1 : reg = &RU_REG(CLK_PRG_SWCH_ADDR, CLK_PRG_CONFIG2_1); blk = &RU_BLK(CLK_PRG_SWCH_ADDR); break;
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
            bdmf_session_print(session, "(%3u) 0x%08X\n", i, (uint32_t)((uint32_t*)(blk->addr[i] + reg->addr)));
    return 0;
}

bdmfmon_handle_t ag_drv_clk_prg_swch_addr_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "clk_prg_swch_addr"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "clk_prg_swch_addr", "clk_prg_swch_addr", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_clk_prg_config[]={
            BDMFMON_MAKE_PARM("cfgprgclksel", "cfgprgclksel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgprgclkdivide", "cfgprgclkdivide", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_clk_prg_config2[]={
            BDMFMON_MAKE_PARM("cfgprgclkdenom", "cfgprgclkdenom", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_clk_prg_config_1[]={
            BDMFMON_MAKE_PARM("cfgprgclksel_1", "cfgprgclksel_1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgprgclkdivide_1", "cfgprgclkdivide_1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_clk_prg_config2_1[]={
            BDMFMON_MAKE_PARM("cfgprgclkdenom_1", "cfgprgclkdenom_1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="clk_prg_config", .val=BDMF_clk_prg_config, .parms=set_clk_prg_config },
            { .name="clk_prg_config2", .val=BDMF_clk_prg_config2, .parms=set_clk_prg_config2 },
            { .name="clk_prg_config_1", .val=BDMF_clk_prg_config_1, .parms=set_clk_prg_config_1 },
            { .name="clk_prg_config2_1", .val=BDMF_clk_prg_config2_1, .parms=set_clk_prg_config2_1 },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_clk_prg_swch_addr_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="clk_prg_config", .val=BDMF_clk_prg_config, .parms=set_default },
            { .name="clk_prg_config2", .val=BDMF_clk_prg_config2, .parms=set_default },
            { .name="clk_prg_config_1", .val=BDMF_clk_prg_config_1, .parms=set_default },
            { .name="clk_prg_config2_1", .val=BDMF_clk_prg_config2_1, .parms=set_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_clk_prg_swch_addr_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_clk_prg_swch_addr_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="CLK_PRG_CONFIG" , .val=bdmf_address_clk_prg_config },
            { .name="CLK_PRG_CONFIG2" , .val=bdmf_address_clk_prg_config2 },
            { .name="CLK_PRG_CONFIG_1" , .val=bdmf_address_clk_prg_config_1 },
            { .name="CLK_PRG_CONFIG2_1" , .val=bdmf_address_clk_prg_config2_1 },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_clk_prg_swch_addr_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

