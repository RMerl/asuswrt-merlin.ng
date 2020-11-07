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
#include "pmi_ag.h"
bdmf_error_t ag_drv_pmi_lp_0_set(bdmf_boolean cr_xgwan_top_wan_misc_pmi_lp_en, bdmf_boolean cr_xgwan_top_wan_misc_pmi_lp_write)
{
    uint32_t reg_lp_0=0;

#ifdef VALIDATE_PARMS
    if((cr_xgwan_top_wan_misc_pmi_lp_en >= _1BITS_MAX_VAL_) ||
       (cr_xgwan_top_wan_misc_pmi_lp_write >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_lp_0 = RU_FIELD_SET(0, PMI, LP_0, CR_XGWAN_TOP_WAN_MISC_PMI_LP_EN, reg_lp_0, cr_xgwan_top_wan_misc_pmi_lp_en);
    reg_lp_0 = RU_FIELD_SET(0, PMI, LP_0, CR_XGWAN_TOP_WAN_MISC_PMI_LP_WRITE, reg_lp_0, cr_xgwan_top_wan_misc_pmi_lp_write);

    RU_REG_WRITE(0, PMI, LP_0, reg_lp_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_pmi_lp_0_get(bdmf_boolean *cr_xgwan_top_wan_misc_pmi_lp_en, bdmf_boolean *cr_xgwan_top_wan_misc_pmi_lp_write)
{
    uint32_t reg_lp_0=0;

#ifdef VALIDATE_PARMS
    if(!cr_xgwan_top_wan_misc_pmi_lp_en || !cr_xgwan_top_wan_misc_pmi_lp_write)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, PMI, LP_0, reg_lp_0);

    *cr_xgwan_top_wan_misc_pmi_lp_en = RU_FIELD_GET(0, PMI, LP_0, CR_XGWAN_TOP_WAN_MISC_PMI_LP_EN, reg_lp_0);
    *cr_xgwan_top_wan_misc_pmi_lp_write = RU_FIELD_GET(0, PMI, LP_0, CR_XGWAN_TOP_WAN_MISC_PMI_LP_WRITE, reg_lp_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_pmi_lp_1_set(uint32_t cr_xgwan_top_wan_misc_pmi_lp_addr)
{
    uint32_t reg_lp_1=0;

#ifdef VALIDATE_PARMS
#endif

    reg_lp_1 = RU_FIELD_SET(0, PMI, LP_1, CR_XGWAN_TOP_WAN_MISC_PMI_LP_ADDR, reg_lp_1, cr_xgwan_top_wan_misc_pmi_lp_addr);

    RU_REG_WRITE(0, PMI, LP_1, reg_lp_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_pmi_lp_1_get(uint32_t *cr_xgwan_top_wan_misc_pmi_lp_addr)
{
    uint32_t reg_lp_1=0;

#ifdef VALIDATE_PARMS
    if(!cr_xgwan_top_wan_misc_pmi_lp_addr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, PMI, LP_1, reg_lp_1);

    *cr_xgwan_top_wan_misc_pmi_lp_addr = RU_FIELD_GET(0, PMI, LP_1, CR_XGWAN_TOP_WAN_MISC_PMI_LP_ADDR, reg_lp_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_pmi_lp_2_set(uint16_t cr_xgwan_top_wan_misc_pmi_lp_wrdata, uint16_t cr_xgwan_top_wan_misc_pmi_lp_maskdata)
{
    uint32_t reg_lp_2=0;

#ifdef VALIDATE_PARMS
#endif

    reg_lp_2 = RU_FIELD_SET(0, PMI, LP_2, CR_XGWAN_TOP_WAN_MISC_PMI_LP_WRDATA, reg_lp_2, cr_xgwan_top_wan_misc_pmi_lp_wrdata);
    reg_lp_2 = RU_FIELD_SET(0, PMI, LP_2, CR_XGWAN_TOP_WAN_MISC_PMI_LP_MASKDATA, reg_lp_2, cr_xgwan_top_wan_misc_pmi_lp_maskdata);

    RU_REG_WRITE(0, PMI, LP_2, reg_lp_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_pmi_lp_2_get(uint16_t *cr_xgwan_top_wan_misc_pmi_lp_wrdata, uint16_t *cr_xgwan_top_wan_misc_pmi_lp_maskdata)
{
    uint32_t reg_lp_2=0;

#ifdef VALIDATE_PARMS
    if(!cr_xgwan_top_wan_misc_pmi_lp_wrdata || !cr_xgwan_top_wan_misc_pmi_lp_maskdata)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, PMI, LP_2, reg_lp_2);

    *cr_xgwan_top_wan_misc_pmi_lp_wrdata = RU_FIELD_GET(0, PMI, LP_2, CR_XGWAN_TOP_WAN_MISC_PMI_LP_WRDATA, reg_lp_2);
    *cr_xgwan_top_wan_misc_pmi_lp_maskdata = RU_FIELD_GET(0, PMI, LP_2, CR_XGWAN_TOP_WAN_MISC_PMI_LP_MASKDATA, reg_lp_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_pmi_lp_3_get(bdmf_boolean *pmi_lp_err, bdmf_boolean *pmi_lp_ack, uint16_t *pmi_lp_rddata)
{
    uint32_t reg_lp_3=0;

#ifdef VALIDATE_PARMS
    if(!pmi_lp_err || !pmi_lp_ack || !pmi_lp_rddata)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, PMI, LP_3, reg_lp_3);

    *pmi_lp_err = RU_FIELD_GET(0, PMI, LP_3, PMI_LP_ERR, reg_lp_3);
    *pmi_lp_ack = RU_FIELD_GET(0, PMI, LP_3, PMI_LP_ACK, reg_lp_3);
    *pmi_lp_rddata = RU_FIELD_GET(0, PMI, LP_3, PMI_LP_RDDATA, reg_lp_3);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
enum
{
    BDMF_lp_0,
    BDMF_lp_1,
    BDMF_lp_2,
    BDMF_lp_3,
};

typedef enum
{
    bdmf_address_lp_0,
    bdmf_address_lp_1,
    bdmf_address_lp_2,
    bdmf_address_lp_3,
}
bdmf_address;

static int bcm_pmi_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err;

    switch(parm[0].value.unumber)
    {
    case BDMF_lp_0:
        err = ag_drv_pmi_lp_0_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case BDMF_lp_1:
        err = ag_drv_pmi_lp_1_set(parm[1].value.unumber);
        break;
    case BDMF_lp_2:
        err = ag_drv_pmi_lp_2_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_pmi_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err;

    switch(parm[0].value.unumber)
    {
    case BDMF_lp_0:
    {
        bdmf_boolean cr_xgwan_top_wan_misc_pmi_lp_en;
        bdmf_boolean cr_xgwan_top_wan_misc_pmi_lp_write;
        err = ag_drv_pmi_lp_0_get(&cr_xgwan_top_wan_misc_pmi_lp_en, &cr_xgwan_top_wan_misc_pmi_lp_write);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_pmi_lp_en = %u = 0x%x\n", cr_xgwan_top_wan_misc_pmi_lp_en, cr_xgwan_top_wan_misc_pmi_lp_en);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_pmi_lp_write = %u = 0x%x\n", cr_xgwan_top_wan_misc_pmi_lp_write, cr_xgwan_top_wan_misc_pmi_lp_write);
        break;
    }
    case BDMF_lp_1:
    {
        uint32_t cr_xgwan_top_wan_misc_pmi_lp_addr;
        err = ag_drv_pmi_lp_1_get(&cr_xgwan_top_wan_misc_pmi_lp_addr);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_pmi_lp_addr = %u = 0x%x\n", cr_xgwan_top_wan_misc_pmi_lp_addr, cr_xgwan_top_wan_misc_pmi_lp_addr);
        break;
    }
    case BDMF_lp_2:
    {
        uint16_t cr_xgwan_top_wan_misc_pmi_lp_wrdata;
        uint16_t cr_xgwan_top_wan_misc_pmi_lp_maskdata;
        err = ag_drv_pmi_lp_2_get(&cr_xgwan_top_wan_misc_pmi_lp_wrdata, &cr_xgwan_top_wan_misc_pmi_lp_maskdata);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_pmi_lp_wrdata = %u = 0x%x\n", cr_xgwan_top_wan_misc_pmi_lp_wrdata, cr_xgwan_top_wan_misc_pmi_lp_wrdata);
        bdmf_session_print(session, "cr_xgwan_top_wan_misc_pmi_lp_maskdata = %u = 0x%x\n", cr_xgwan_top_wan_misc_pmi_lp_maskdata, cr_xgwan_top_wan_misc_pmi_lp_maskdata);
        break;
    }
    case BDMF_lp_3:
    {
        bdmf_boolean pmi_lp_err;
        bdmf_boolean pmi_lp_ack;
        uint16_t pmi_lp_rddata;
        err = ag_drv_pmi_lp_3_get(&pmi_lp_err, &pmi_lp_ack, &pmi_lp_rddata);
        bdmf_session_print(session, "pmi_lp_err = %u = 0x%x\n", pmi_lp_err, pmi_lp_err);
        bdmf_session_print(session, "pmi_lp_ack = %u = 0x%x\n", pmi_lp_ack, pmi_lp_ack);
        bdmf_session_print(session, "pmi_lp_rddata = %u = 0x%x\n", pmi_lp_rddata, pmi_lp_rddata);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_pmi_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        bdmf_boolean cr_xgwan_top_wan_misc_pmi_lp_en=gtmv(m, 1);
        bdmf_boolean cr_xgwan_top_wan_misc_pmi_lp_write=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_pmi_lp_0_set( %u %u)\n", cr_xgwan_top_wan_misc_pmi_lp_en, cr_xgwan_top_wan_misc_pmi_lp_write);
        if(!err) ag_drv_pmi_lp_0_set(cr_xgwan_top_wan_misc_pmi_lp_en, cr_xgwan_top_wan_misc_pmi_lp_write);
        if(!err) ag_drv_pmi_lp_0_get( &cr_xgwan_top_wan_misc_pmi_lp_en, &cr_xgwan_top_wan_misc_pmi_lp_write);
        if(!err) bdmf_session_print(session, "ag_drv_pmi_lp_0_get( %u %u)\n", cr_xgwan_top_wan_misc_pmi_lp_en, cr_xgwan_top_wan_misc_pmi_lp_write);
        if(err || cr_xgwan_top_wan_misc_pmi_lp_en!=gtmv(m, 1) || cr_xgwan_top_wan_misc_pmi_lp_write!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cr_xgwan_top_wan_misc_pmi_lp_addr=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_pmi_lp_1_set( %u)\n", cr_xgwan_top_wan_misc_pmi_lp_addr);
        if(!err) ag_drv_pmi_lp_1_set(cr_xgwan_top_wan_misc_pmi_lp_addr);
        if(!err) ag_drv_pmi_lp_1_get( &cr_xgwan_top_wan_misc_pmi_lp_addr);
        if(!err) bdmf_session_print(session, "ag_drv_pmi_lp_1_get( %u)\n", cr_xgwan_top_wan_misc_pmi_lp_addr);
        if(err || cr_xgwan_top_wan_misc_pmi_lp_addr!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cr_xgwan_top_wan_misc_pmi_lp_wrdata=gtmv(m, 16);
        uint16_t cr_xgwan_top_wan_misc_pmi_lp_maskdata=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_pmi_lp_2_set( %u %u)\n", cr_xgwan_top_wan_misc_pmi_lp_wrdata, cr_xgwan_top_wan_misc_pmi_lp_maskdata);
        if(!err) ag_drv_pmi_lp_2_set(cr_xgwan_top_wan_misc_pmi_lp_wrdata, cr_xgwan_top_wan_misc_pmi_lp_maskdata);
        if(!err) ag_drv_pmi_lp_2_get( &cr_xgwan_top_wan_misc_pmi_lp_wrdata, &cr_xgwan_top_wan_misc_pmi_lp_maskdata);
        if(!err) bdmf_session_print(session, "ag_drv_pmi_lp_2_get( %u %u)\n", cr_xgwan_top_wan_misc_pmi_lp_wrdata, cr_xgwan_top_wan_misc_pmi_lp_maskdata);
        if(err || cr_xgwan_top_wan_misc_pmi_lp_wrdata!=gtmv(m, 16) || cr_xgwan_top_wan_misc_pmi_lp_maskdata!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean pmi_lp_err=gtmv(m, 1);
        bdmf_boolean pmi_lp_ack=gtmv(m, 1);
        uint16_t pmi_lp_rddata=gtmv(m, 16);
        if(!err) ag_drv_pmi_lp_3_get( &pmi_lp_err, &pmi_lp_ack, &pmi_lp_rddata);
        if(!err) bdmf_session_print(session, "ag_drv_pmi_lp_3_get( %u %u %u)\n", pmi_lp_err, pmi_lp_ack, pmi_lp_rddata);
    }
    return err;
}

static int bcm_pmi_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_lp_0 : reg = &RU_REG(PMI, LP_0); blk = &RU_BLK(PMI); break;
    case bdmf_address_lp_1 : reg = &RU_REG(PMI, LP_1); blk = &RU_BLK(PMI); break;
    case bdmf_address_lp_2 : reg = &RU_REG(PMI, LP_2); blk = &RU_BLK(PMI); break;
    case bdmf_address_lp_3 : reg = &RU_REG(PMI, LP_3); blk = &RU_BLK(PMI); break;
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

bdmfmon_handle_t ag_drv_pmi_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "pmi"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "pmi", "pmi", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_lp_0[]={
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_pmi_lp_en", "cr_xgwan_top_wan_misc_pmi_lp_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_pmi_lp_write", "cr_xgwan_top_wan_misc_pmi_lp_write", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_lp_1[]={
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_pmi_lp_addr", "cr_xgwan_top_wan_misc_pmi_lp_addr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_lp_2[]={
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_pmi_lp_wrdata", "cr_xgwan_top_wan_misc_pmi_lp_wrdata", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cr_xgwan_top_wan_misc_pmi_lp_maskdata", "cr_xgwan_top_wan_misc_pmi_lp_maskdata", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="lp_0", .val=BDMF_lp_0, .parms=set_lp_0 },
            { .name="lp_1", .val=BDMF_lp_1, .parms=set_lp_1 },
            { .name="lp_2", .val=BDMF_lp_2, .parms=set_lp_2 },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_pmi_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="lp_0", .val=BDMF_lp_0, .parms=set_default },
            { .name="lp_1", .val=BDMF_lp_1, .parms=set_default },
            { .name="lp_2", .val=BDMF_lp_2, .parms=set_default },
            { .name="lp_3", .val=BDMF_lp_3, .parms=set_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_pmi_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_pmi_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="LP_0" , .val=bdmf_address_lp_0 },
            { .name="LP_1" , .val=bdmf_address_lp_1 },
            { .name="LP_2" , .val=bdmf_address_lp_2 },
            { .name="LP_3" , .val=bdmf_address_lp_3 },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_pmi_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

