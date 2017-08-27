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
#include "xrdp_drv_xlif_rx_if_ag.h"

#define BLOCK_ADDR_COUNT_BITS 3
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_xlif_rx_if_if_dis_set(uint8_t channel_id, bdmf_boolean disable)
{
    uint32_t reg_if_dis=0;

#ifdef VALIDATE_PARMS
    if((channel_id >= BLOCK_ADDR_COUNT) ||
       (disable >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_if_dis = RU_FIELD_SET(channel_id, XLIF_RX_IF, IF_DIS, DISABLE, reg_if_dis, disable);

    RU_REG_WRITE(channel_id, XLIF_RX_IF, IF_DIS, reg_if_dis);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_rx_if_if_dis_get(uint8_t channel_id, bdmf_boolean *disable)
{
    uint32_t reg_if_dis;

#ifdef VALIDATE_PARMS
    if(!disable)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((channel_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(channel_id, XLIF_RX_IF, IF_DIS, reg_if_dis);

    *disable = RU_FIELD_GET(channel_id, XLIF_RX_IF, IF_DIS, DISABLE, reg_if_dis);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_rx_if_oflw_flag_get(uint8_t channel_id, bdmf_boolean *oflw)
{
    uint32_t reg_oflw_flag;

#ifdef VALIDATE_PARMS
    if(!oflw)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((channel_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(channel_id, XLIF_RX_IF, OFLW_FLAG, reg_oflw_flag);

    *oflw = RU_FIELD_GET(channel_id, XLIF_RX_IF, OFLW_FLAG, OFLW, reg_oflw_flag);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_rx_if_err_flag_get(uint8_t channel_id, bdmf_boolean *err)
{
    uint32_t reg_err_flag;

#ifdef VALIDATE_PARMS
    if(!err)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((channel_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(channel_id, XLIF_RX_IF, ERR_FLAG, reg_err_flag);

    *err = RU_FIELD_GET(channel_id, XLIF_RX_IF, ERR_FLAG, ERR, reg_err_flag);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
typedef enum
{
    bdmf_address_if_dis,
    bdmf_address_oflw_flag,
    bdmf_address_err_flag,
}
bdmf_address;

static int bcm_xlif_rx_if_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_xlif_rx_if_if_dis:
        err = ag_drv_xlif_rx_if_if_dis_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

int bcm_xlif_rx_if_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_xlif_rx_if_if_dis:
    {
        bdmf_boolean disable;
        err = ag_drv_xlif_rx_if_if_dis_get(parm[1].value.unumber, &disable);
        bdmf_session_print(session, "disable = %u (0x%x)\n", disable, disable);
        break;
    }
    case cli_xlif_rx_if_oflw_flag:
    {
        bdmf_boolean oflw;
        err = ag_drv_xlif_rx_if_oflw_flag_get(parm[1].value.unumber, &oflw);
        bdmf_session_print(session, "oflw = %u (0x%x)\n", oflw, oflw);
        break;
    }
    case cli_xlif_rx_if_err_flag:
    {
        bdmf_boolean err;
        err = ag_drv_xlif_rx_if_err_flag_get(parm[1].value.unumber, &err);
        bdmf_session_print(session, "err = %u (0x%x)\n", err, err);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_xlif_rx_if_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    uint8_t channel_id = parm[1].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        bdmf_boolean disable=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_xlif_rx_if_if_dis_set(%u %u)\n", channel_id, disable);
        if(!err) ag_drv_xlif_rx_if_if_dis_set(channel_id, disable);
        if(!err) ag_drv_xlif_rx_if_if_dis_get( channel_id, &disable);
        if(!err) bdmf_session_print(session, "ag_drv_xlif_rx_if_if_dis_get(%u %u)\n", channel_id, disable);
        if(err || disable!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean oflw=gtmv(m, 1);
        if(!err) ag_drv_xlif_rx_if_oflw_flag_get( channel_id, &oflw);
        if(!err) bdmf_session_print(session, "ag_drv_xlif_rx_if_oflw_flag_get(%u %u)\n", channel_id, oflw);
    }
    {
        bdmf_boolean err=gtmv(m, 1);
        if(!err) ag_drv_xlif_rx_if_err_flag_get( channel_id, &err);
        if(!err) bdmf_session_print(session, "ag_drv_xlif_rx_if_err_flag_get(%u %u)\n", channel_id, err);
    }
    return err;
}

static int bcm_xlif_rx_if_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_if_dis : reg = &RU_REG(XLIF_RX_IF, IF_DIS); blk = &RU_BLK(XLIF_RX_IF); break;
    case bdmf_address_oflw_flag : reg = &RU_REG(XLIF_RX_IF, OFLW_FLAG); blk = &RU_BLK(XLIF_RX_IF); break;
    case bdmf_address_err_flag : reg = &RU_REG(XLIF_RX_IF, ERR_FLAG); blk = &RU_BLK(XLIF_RX_IF); break;
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

bdmfmon_handle_t ag_drv_xlif_rx_if_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "xlif_rx_if"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "xlif_rx_if", "xlif_rx_if", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_if_dis[]={
            BDMFMON_MAKE_PARM_ENUM("channel_id", "channel_id", channel_id_enum_table, 0),
            BDMFMON_MAKE_PARM("disable", "disable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="if_dis", .val=cli_xlif_rx_if_if_dis, .parms=set_if_dis },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_xlif_rx_if_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_MAKE_PARM_ENUM("channel_id", "channel_id", channel_id_enum_table, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="if_dis", .val=cli_xlif_rx_if_if_dis, .parms=set_default },
            { .name="oflw_flag", .val=cli_xlif_rx_if_oflw_flag, .parms=set_default },
            { .name="err_flag", .val=cli_xlif_rx_if_err_flag, .parms=set_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_xlif_rx_if_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_xlif_rx_if_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0),
            BDMFMON_MAKE_PARM_ENUM("channel_id", "channel_id", channel_id_enum_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="IF_DIS" , .val=bdmf_address_if_dis },
            { .name="OFLW_FLAG" , .val=bdmf_address_oflw_flag },
            { .name="ERR_FLAG" , .val=bdmf_address_err_flag },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_xlif_rx_if_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM_ENUM("index1", "channel_id", channel_id_enum_table, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

