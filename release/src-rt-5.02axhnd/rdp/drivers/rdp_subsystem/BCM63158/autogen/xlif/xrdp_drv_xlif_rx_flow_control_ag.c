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
#include "xrdp_drv_xlif_rx_flow_control_ag.h"

#define BLOCK_ADDR_COUNT_BITS 2
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_xlif_rx_flow_control_cosmap_en_get(uint8_t channel_id, bdmf_boolean *pfc_en, bdmf_boolean *llfc_en)
{
    uint32_t reg_cosmap_en;

#ifdef VALIDATE_PARMS
    if(!pfc_en || !llfc_en)
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

    RU_REG_READ(channel_id, XLIF_RX_FLOW_CONTROL, COSMAP_EN, reg_cosmap_en);

    *pfc_en = RU_FIELD_GET(channel_id, XLIF_RX_FLOW_CONTROL, COSMAP_EN, PFC_EN, reg_cosmap_en);
    *llfc_en = RU_FIELD_GET(channel_id, XLIF_RX_FLOW_CONTROL, COSMAP_EN, LLFC_EN, reg_cosmap_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xlif_rx_flow_control_cosmap_get(uint8_t channel_id, uint16_t *value)
{
    uint32_t reg_cosmap;

#ifdef VALIDATE_PARMS
    if(!value)
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

    RU_REG_READ(channel_id, XLIF_RX_FLOW_CONTROL, COSMAP, reg_cosmap);

    *value = RU_FIELD_GET(channel_id, XLIF_RX_FLOW_CONTROL, COSMAP, VALUE, reg_cosmap);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
typedef enum
{
    bdmf_address_cosmap_en,
    bdmf_address_cosmap,
}
bdmf_address;

static int bcm_xlif_rx_flow_control_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

int bcm_xlif_rx_flow_control_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_xlif_rx_flow_control_cosmap_en:
    {
        bdmf_boolean pfc_en;
        bdmf_boolean llfc_en;
        err = ag_drv_xlif_rx_flow_control_cosmap_en_get(parm[1].value.unumber, &pfc_en, &llfc_en);
        bdmf_session_print(session, "pfc_en = %u (0x%x)\n", pfc_en, pfc_en);
        bdmf_session_print(session, "llfc_en = %u (0x%x)\n", llfc_en, llfc_en);
        break;
    }
    case cli_xlif_rx_flow_control_cosmap:
    {
        uint16_t value;
        err = ag_drv_xlif_rx_flow_control_cosmap_get(parm[1].value.unumber, &value);
        bdmf_session_print(session, "value = %u (0x%x)\n", value, value);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_xlif_rx_flow_control_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    uint8_t channel_id = parm[1].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        bdmf_boolean pfc_en=gtmv(m, 1);
        bdmf_boolean llfc_en=gtmv(m, 1);
        if(!err) ag_drv_xlif_rx_flow_control_cosmap_en_get( channel_id, &pfc_en, &llfc_en);
        if(!err) bdmf_session_print(session, "ag_drv_xlif_rx_flow_control_cosmap_en_get(%u %u %u)\n", channel_id, pfc_en, llfc_en);
    }
    {
        uint16_t value=gtmv(m, 16);
        if(!err) ag_drv_xlif_rx_flow_control_cosmap_get( channel_id, &value);
        if(!err) bdmf_session_print(session, "ag_drv_xlif_rx_flow_control_cosmap_get(%u %u)\n", channel_id, value);
    }
    return err;
}

static int bcm_xlif_rx_flow_control_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_cosmap_en : reg = &RU_REG(XLIF_RX_FLOW_CONTROL, COSMAP_EN); blk = &RU_BLK(XLIF_RX_FLOW_CONTROL); break;
    case bdmf_address_cosmap : reg = &RU_REG(XLIF_RX_FLOW_CONTROL, COSMAP); blk = &RU_BLK(XLIF_RX_FLOW_CONTROL); break;
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

bdmfmon_handle_t ag_drv_xlif_rx_flow_control_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "xlif_rx_flow_control"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "xlif_rx_flow_control", "xlif_rx_flow_control", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_enum_val_t selector_table[] = {
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_xlif_rx_flow_control_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_MAKE_PARM_ENUM("channel_id", "channel_id", channel_id_enum_table, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="cosmap_en", .val=cli_xlif_rx_flow_control_cosmap_en, .parms=set_default },
            { .name="cosmap", .val=cli_xlif_rx_flow_control_cosmap, .parms=set_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_xlif_rx_flow_control_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_xlif_rx_flow_control_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0),
            BDMFMON_MAKE_PARM_ENUM("channel_id", "channel_id", channel_id_enum_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="COSMAP_EN" , .val=bdmf_address_cosmap_en },
            { .name="COSMAP" , .val=bdmf_address_cosmap },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_xlif_rx_flow_control_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM_ENUM("index1", "channel_id", channel_id_enum_table, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

