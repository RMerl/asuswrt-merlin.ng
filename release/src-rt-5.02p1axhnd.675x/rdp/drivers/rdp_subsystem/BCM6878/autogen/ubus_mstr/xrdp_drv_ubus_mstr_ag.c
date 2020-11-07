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
#include "xrdp_drv_ubus_mstr_ag.h"

#define BLOCK_ADDR_COUNT_BITS 0
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_ubus_mstr_en_set(uint8_t ubus_mstr_id, bdmf_boolean en)
{
    uint32_t reg_en=0;

#ifdef VALIDATE_PARMS
    if((ubus_mstr_id >= BLOCK_ADDR_COUNT) ||
       (en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_en = RU_FIELD_SET(ubus_mstr_id, UBUS_MSTR, EN, EN, reg_en, en);

    RU_REG_WRITE(ubus_mstr_id, UBUS_MSTR, EN, reg_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_mstr_en_get(uint8_t ubus_mstr_id, bdmf_boolean *en)
{
    uint32_t reg_en;

#ifdef VALIDATE_PARMS
    if(!en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((ubus_mstr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(ubus_mstr_id, UBUS_MSTR, EN, reg_en);

    *en = RU_FIELD_GET(ubus_mstr_id, UBUS_MSTR, EN, EN, reg_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_mstr_hyst_ctrl_set(uint8_t ubus_mstr_id, uint16_t cmd_space, uint16_t data_space)
{
    uint32_t reg_hyst_ctrl=0;

#ifdef VALIDATE_PARMS
    if((ubus_mstr_id >= BLOCK_ADDR_COUNT) ||
       (cmd_space >= _10BITS_MAX_VAL_) ||
       (data_space >= _10BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_hyst_ctrl = RU_FIELD_SET(ubus_mstr_id, UBUS_MSTR, HYST_CTRL, CMD_SPACE, reg_hyst_ctrl, cmd_space);
    reg_hyst_ctrl = RU_FIELD_SET(ubus_mstr_id, UBUS_MSTR, HYST_CTRL, DATA_SPACE, reg_hyst_ctrl, data_space);

    RU_REG_WRITE(ubus_mstr_id, UBUS_MSTR, HYST_CTRL, reg_hyst_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_mstr_hyst_ctrl_get(uint8_t ubus_mstr_id, uint16_t *cmd_space, uint16_t *data_space)
{
    uint32_t reg_hyst_ctrl;

#ifdef VALIDATE_PARMS
    if(!cmd_space || !data_space)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((ubus_mstr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(ubus_mstr_id, UBUS_MSTR, HYST_CTRL, reg_hyst_ctrl);

    *cmd_space = RU_FIELD_GET(ubus_mstr_id, UBUS_MSTR, HYST_CTRL, CMD_SPACE, reg_hyst_ctrl);
    *data_space = RU_FIELD_GET(ubus_mstr_id, UBUS_MSTR, HYST_CTRL, DATA_SPACE, reg_hyst_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_mstr_hp_set(uint8_t ubus_mstr_id, bdmf_boolean hp_en)
{
    uint32_t reg_hp=0;

#ifdef VALIDATE_PARMS
    if((ubus_mstr_id >= BLOCK_ADDR_COUNT) ||
       (hp_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_hp = RU_FIELD_SET(ubus_mstr_id, UBUS_MSTR, HP, HP_EN, reg_hp, hp_en);

    RU_REG_WRITE(ubus_mstr_id, UBUS_MSTR, HP, reg_hp);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_ubus_mstr_hp_get(uint8_t ubus_mstr_id, bdmf_boolean *hp_en)
{
    uint32_t reg_hp;

#ifdef VALIDATE_PARMS
    if(!hp_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((ubus_mstr_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(ubus_mstr_id, UBUS_MSTR, HP, reg_hp);

    *hp_en = RU_FIELD_GET(ubus_mstr_id, UBUS_MSTR, HP, HP_EN, reg_hp);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
typedef enum
{
    bdmf_address_en,
    bdmf_address_hyst_ctrl,
    bdmf_address_hp,
}
bdmf_address;

static int bcm_ubus_mstr_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_ubus_mstr_en:
        err = ag_drv_ubus_mstr_en_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_ubus_mstr_hyst_ctrl:
        err = ag_drv_ubus_mstr_hyst_ctrl_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_ubus_mstr_hp:
        err = ag_drv_ubus_mstr_hp_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

int bcm_ubus_mstr_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_ubus_mstr_en:
    {
        bdmf_boolean en;
        err = ag_drv_ubus_mstr_en_get(parm[1].value.unumber, &en);
        bdmf_session_print(session, "en = %u (0x%x)\n", en, en);
        break;
    }
    case cli_ubus_mstr_hyst_ctrl:
    {
        uint16_t cmd_space;
        uint16_t data_space;
        err = ag_drv_ubus_mstr_hyst_ctrl_get(parm[1].value.unumber, &cmd_space, &data_space);
        bdmf_session_print(session, "cmd_space = %u (0x%x)\n", cmd_space, cmd_space);
        bdmf_session_print(session, "data_space = %u (0x%x)\n", data_space, data_space);
        break;
    }
    case cli_ubus_mstr_hp:
    {
        bdmf_boolean hp_en;
        err = ag_drv_ubus_mstr_hp_get(parm[1].value.unumber, &hp_en);
        bdmf_session_print(session, "hp_en = %u (0x%x)\n", hp_en, hp_en);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_ubus_mstr_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    uint8_t ubus_mstr_id = parm[1].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        bdmf_boolean en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_mstr_en_set(%u %u)\n", ubus_mstr_id, en);
        if(!err) ag_drv_ubus_mstr_en_set(ubus_mstr_id, en);
        if(!err) ag_drv_ubus_mstr_en_get( ubus_mstr_id, &en);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_mstr_en_get(%u %u)\n", ubus_mstr_id, en);
        if(err || en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t cmd_space=gtmv(m, 10);
        uint16_t data_space=gtmv(m, 10);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_mstr_hyst_ctrl_set(%u %u %u)\n", ubus_mstr_id, cmd_space, data_space);
        if(!err) ag_drv_ubus_mstr_hyst_ctrl_set(ubus_mstr_id, cmd_space, data_space);
        if(!err) ag_drv_ubus_mstr_hyst_ctrl_get( ubus_mstr_id, &cmd_space, &data_space);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_mstr_hyst_ctrl_get(%u %u %u)\n", ubus_mstr_id, cmd_space, data_space);
        if(err || cmd_space!=gtmv(m, 10) || data_space!=gtmv(m, 10))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean hp_en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_mstr_hp_set(%u %u)\n", ubus_mstr_id, hp_en);
        if(!err) ag_drv_ubus_mstr_hp_set(ubus_mstr_id, hp_en);
        if(!err) ag_drv_ubus_mstr_hp_get( ubus_mstr_id, &hp_en);
        if(!err) bdmf_session_print(session, "ag_drv_ubus_mstr_hp_get(%u %u)\n", ubus_mstr_id, hp_en);
        if(err || hp_en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    return err;
}

static int bcm_ubus_mstr_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_en : reg = &RU_REG(UBUS_MSTR, EN); blk = &RU_BLK(UBUS_MSTR); break;
    case bdmf_address_hyst_ctrl : reg = &RU_REG(UBUS_MSTR, HYST_CTRL); blk = &RU_BLK(UBUS_MSTR); break;
    case bdmf_address_hp : reg = &RU_REG(UBUS_MSTR, HP); blk = &RU_BLK(UBUS_MSTR); break;
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

bdmfmon_handle_t ag_drv_ubus_mstr_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "ubus_mstr"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "ubus_mstr", "ubus_mstr", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_en[]={
            BDMFMON_MAKE_PARM_ENUM("ubus_mstr_id", "ubus_mstr_id", ubus_mstr_id_enum_table, 0),
            BDMFMON_MAKE_PARM("en", "en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_hyst_ctrl[]={
            BDMFMON_MAKE_PARM_ENUM("ubus_mstr_id", "ubus_mstr_id", ubus_mstr_id_enum_table, 0),
            BDMFMON_MAKE_PARM("cmd_space", "cmd_space", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("data_space", "data_space", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_hp[]={
            BDMFMON_MAKE_PARM_ENUM("ubus_mstr_id", "ubus_mstr_id", ubus_mstr_id_enum_table, 0),
            BDMFMON_MAKE_PARM("hp_en", "hp_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="en", .val=cli_ubus_mstr_en, .parms=set_en },
            { .name="hyst_ctrl", .val=cli_ubus_mstr_hyst_ctrl, .parms=set_hyst_ctrl },
            { .name="hp", .val=cli_ubus_mstr_hp, .parms=set_hp },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_ubus_mstr_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_MAKE_PARM_ENUM("ubus_mstr_id", "ubus_mstr_id", ubus_mstr_id_enum_table, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="en", .val=cli_ubus_mstr_en, .parms=set_default },
            { .name="hyst_ctrl", .val=cli_ubus_mstr_hyst_ctrl, .parms=set_default },
            { .name="hp", .val=cli_ubus_mstr_hp, .parms=set_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_ubus_mstr_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_ubus_mstr_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0),
            BDMFMON_MAKE_PARM_ENUM("ubus_mstr_id", "ubus_mstr_id", ubus_mstr_id_enum_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="EN" , .val=bdmf_address_en },
            { .name="HYST_CTRL" , .val=bdmf_address_hyst_ctrl },
            { .name="HP" , .val=bdmf_address_hp },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_ubus_mstr_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM_ENUM("index1", "ubus_mstr_id", ubus_mstr_id_enum_table, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

