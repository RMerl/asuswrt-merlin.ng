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
#include "drv_epon_epn_onu_mac_addr_ag.h"
#define BLOCK_ADDR_COUNT_BITS 5
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_epn_onu_mac_addr_lo_set(uint8_t link_idx, uint8_t mfgaddrreglo, uint32_t onuaddrreg)
{
    uint32_t reg_lo=0;

#ifdef VALIDATE_PARMS
    if((link_idx >= BLOCK_ADDR_COUNT) ||
       (onuaddrreg >= _24BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_lo = RU_FIELD_SET(link_idx, EPN_ONU_MAC_ADDR, LO, MFGADDRREGLO, reg_lo, mfgaddrreglo);
    reg_lo = RU_FIELD_SET(link_idx, EPN_ONU_MAC_ADDR, LO, ONUADDRREG, reg_lo, onuaddrreg);

    RU_REG_WRITE(link_idx, EPN_ONU_MAC_ADDR, LO, reg_lo);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_onu_mac_addr_lo_get(uint8_t link_idx, uint8_t *mfgaddrreglo, uint32_t *onuaddrreg)
{
    uint32_t reg_lo=0;

#ifdef VALIDATE_PARMS
    if(!mfgaddrreglo || !onuaddrreg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((link_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(link_idx, EPN_ONU_MAC_ADDR, LO, reg_lo);

    *mfgaddrreglo = RU_FIELD_GET(link_idx, EPN_ONU_MAC_ADDR, LO, MFGADDRREGLO, reg_lo);
    *onuaddrreg = RU_FIELD_GET(link_idx, EPN_ONU_MAC_ADDR, LO, ONUADDRREG, reg_lo);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_onu_mac_addr_hi_set(uint8_t link_idx, uint16_t mfgaddrreghi)
{
    uint32_t reg_hi=0;

#ifdef VALIDATE_PARMS
    if((link_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_hi = RU_FIELD_SET(link_idx, EPN_ONU_MAC_ADDR, HI, MFGADDRREGHI, reg_hi, mfgaddrreghi);

    RU_REG_WRITE(link_idx, EPN_ONU_MAC_ADDR, HI, reg_hi);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_onu_mac_addr_hi_get(uint8_t link_idx, uint16_t *mfgaddrreghi)
{
    uint32_t reg_hi=0;

#ifdef VALIDATE_PARMS
    if(!mfgaddrreghi)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((link_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(link_idx, EPN_ONU_MAC_ADDR, HI, reg_hi);

    *mfgaddrreghi = RU_FIELD_GET(link_idx, EPN_ONU_MAC_ADDR, HI, MFGADDRREGHI, reg_hi);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
enum
{
    BDMF_lo,
    BDMF_hi,
};

typedef enum
{
    bdmf_address_lo,
    bdmf_address_hi,
}
bdmf_address;

typedef enum LinkIndexEnum
{
    DRV_EPON_LINK0 = 0,
    DRV_EPON_LINK1,
    DRV_EPON_LINK2,
    DRV_EPON_LINK3,
    DRV_EPON_LINK4,
    DRV_EPON_LINK5,
    DRV_EPON_LINK6,
    DRV_EPON_LINK8,
    DRV_EPON_LINK9,
    DRV_EPON_LINK10,
    DRV_EPON_LINK11,
    DRV_EPON_LINK12,
    DRV_EPON_LINK13,
    DRV_EPON_LINK14,
    DRV_EPON_LINK15,
    DRV_EPON_LINK16,
    DRV_EPON_LINK17,
    DRV_EPON_LINK18,
    DRV_EPON_LINK19,
    DRV_EPON_LINK20,
    DRV_EPON_LINK21,
    DRV_EPON_LINK22,
    DRV_EPON_LINK23,
    DRV_EPON_LINK24,
    DRV_EPON_LINK25,
    DRV_EPON_LINK26,
    DRV_EPON_LINK27,
    DRV_EPON_LINK28,
    DRV_EPON_LINK29,
    DRV_EPON_LINK30,
    DRV_EPON_LINK31,
    DRV_EPON_LINK_MAX = DRV_EPON_LINK31
} LinkIndexEnum;

struct bdmfmon_enum_val link_idx_enum_table[] = {
    {"LINK0 ", DRV_EPON_LINK0 },
    {"LINK1 ", DRV_EPON_LINK1 },
    {"LINK2 ", DRV_EPON_LINK2 },
    {"LINK3 ", DRV_EPON_LINK3 },
    {"LINK4 ", DRV_EPON_LINK4 },
    {"LINK5 ", DRV_EPON_LINK5 },
    {"LINK6 ", DRV_EPON_LINK6 },
    {"LINK8 ", DRV_EPON_LINK8 },
    {"LINK9 ", DRV_EPON_LINK9 },
    {"LINK10", DRV_EPON_LINK10},
    {"LINK11", DRV_EPON_LINK11},
    {"LINK12", DRV_EPON_LINK12},
    {"LINK13", DRV_EPON_LINK13},
    {"LINK14", DRV_EPON_LINK14},
    {"LINK15", DRV_EPON_LINK15},
    {"LINK16", DRV_EPON_LINK16},
    {"LINK17", DRV_EPON_LINK17},
    {"LINK18", DRV_EPON_LINK18},
    {"LINK19", DRV_EPON_LINK19},
    {"LINK20", DRV_EPON_LINK20},
    {"LINK21", DRV_EPON_LINK21},
    {"LINK22", DRV_EPON_LINK22},
    {"LINK23", DRV_EPON_LINK23},
    {"LINK24", DRV_EPON_LINK24},
    {"LINK25", DRV_EPON_LINK25},
    {"LINK26", DRV_EPON_LINK26},
    {"LINK27", DRV_EPON_LINK27},
    {"LINK28", DRV_EPON_LINK28},
    {"LINK29", DRV_EPON_LINK29},
    {"LINK30", DRV_EPON_LINK30},
    {"LINK31", DRV_EPON_LINK31},
    {NULL, 0},
};


static int bcm_epn_onu_mac_addr_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err;

    switch(parm[0].value.unumber)
    {
    case BDMF_lo:
        err = ag_drv_epn_onu_mac_addr_lo_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case BDMF_hi:
        err = ag_drv_epn_onu_mac_addr_hi_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_epn_onu_mac_addr_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err;

    switch(parm[0].value.unumber)
    {
    case BDMF_lo:
    {
        uint8_t mfgaddrreglo;
        uint32_t onuaddrreg;
        err = ag_drv_epn_onu_mac_addr_lo_get(parm[1].value.unumber, &mfgaddrreglo, &onuaddrreg);
        bdmf_session_print(session, "mfgaddrreglo = %u = 0x%x\n", mfgaddrreglo, mfgaddrreglo);
        bdmf_session_print(session, "onuaddrreg = %u = 0x%x\n", onuaddrreg, onuaddrreg);
        break;
    }
    case BDMF_hi:
    {
        uint16_t mfgaddrreghi;
        err = ag_drv_epn_onu_mac_addr_hi_get(parm[1].value.unumber, &mfgaddrreghi);
        bdmf_session_print(session, "mfgaddrreghi = %u = 0x%x\n", mfgaddrreghi, mfgaddrreghi);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_epn_onu_mac_addr_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    uint8_t link_idx = parm[1].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        uint8_t mfgaddrreglo=gtmv(m, 8);
        uint32_t onuaddrreg=gtmv(m, 24);
        if(!err) bdmf_session_print(session, "ag_drv_epn_onu_mac_addr_lo_set(%u %u %u)\n", link_idx, mfgaddrreglo, onuaddrreg);
        if(!err) ag_drv_epn_onu_mac_addr_lo_set(link_idx, mfgaddrreglo, onuaddrreg);
        if(!err) ag_drv_epn_onu_mac_addr_lo_get( link_idx, &mfgaddrreglo, &onuaddrreg);
        if(!err) bdmf_session_print(session, "ag_drv_epn_onu_mac_addr_lo_get(%u %u %u)\n", link_idx, mfgaddrreglo, onuaddrreg);
        if(err || mfgaddrreglo!=gtmv(m, 8) || onuaddrreg!=gtmv(m, 24))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t mfgaddrreghi=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_epn_onu_mac_addr_hi_set(%u %u)\n", link_idx, mfgaddrreghi);
        if(!err) ag_drv_epn_onu_mac_addr_hi_set(link_idx, mfgaddrreghi);
        if(!err) ag_drv_epn_onu_mac_addr_hi_get( link_idx, &mfgaddrreghi);
        if(!err) bdmf_session_print(session, "ag_drv_epn_onu_mac_addr_hi_get(%u %u)\n", link_idx, mfgaddrreghi);
        if(err || mfgaddrreghi!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    return err;
}

static int bcm_epn_onu_mac_addr_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_lo : reg = &RU_REG(EPN_ONU_MAC_ADDR, LO); blk = &RU_BLK(EPN_ONU_MAC_ADDR); break;
    case bdmf_address_hi : reg = &RU_REG(EPN_ONU_MAC_ADDR, HI); blk = &RU_BLK(EPN_ONU_MAC_ADDR); break;
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

bdmfmon_handle_t ag_drv_epn_onu_mac_addr_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "epn_onu_mac_addr"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "epn_onu_mac_addr", "epn_onu_mac_addr", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_lo[]={
            BDMFMON_MAKE_PARM_ENUM("link_idx", "link_idx", link_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("mfgaddrreglo", "mfgaddrreglo", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("onuaddrreg", "onuaddrreg", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_hi[]={
            BDMFMON_MAKE_PARM_ENUM("link_idx", "link_idx", link_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("mfgaddrreghi", "mfgaddrreghi", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="lo", .val=BDMF_lo, .parms=set_lo },
            { .name="hi", .val=BDMF_hi, .parms=set_hi },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_epn_onu_mac_addr_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_MAKE_PARM_ENUM("link_idx", "link_idx", link_idx_enum_table, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="lo", .val=BDMF_lo, .parms=set_default },
            { .name="hi", .val=BDMF_hi, .parms=set_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_epn_onu_mac_addr_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_epn_onu_mac_addr_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0),
            BDMFMON_MAKE_PARM_ENUM("link_idx", "link_idx", link_idx_enum_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="LO" , .val=bdmf_address_lo },
            { .name="HI" , .val=bdmf_address_hi },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_epn_onu_mac_addr_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM_ENUM("index1", "link_idx", link_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

