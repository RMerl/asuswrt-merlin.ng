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
#include "drv_epon_epn_tx_l1s_shp_ag.h"
#define BLOCK_ADDR_COUNT_BITS 5
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_epn_tx_l1s_shp_config_set(uint8_t shaper_idx, bdmf_boolean cfgshpincovr, uint32_t cfgshprate, uint8_t cfgshpbstsize)
{
    uint32_t reg_config=0;

#ifdef VALIDATE_PARMS
    if((shaper_idx >= BLOCK_ADDR_COUNT) ||
       (cfgshpincovr >= _1BITS_MAX_VAL_) ||
       (cfgshprate >= _23BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_config = RU_FIELD_SET(shaper_idx, EPN_TX_L1S_SHP, CONFIG, CFGSHPINCOVR, reg_config, cfgshpincovr);
    reg_config = RU_FIELD_SET(shaper_idx, EPN_TX_L1S_SHP, CONFIG, CFGSHPRATE, reg_config, cfgshprate);
    reg_config = RU_FIELD_SET(shaper_idx, EPN_TX_L1S_SHP, CONFIG, CFGSHPBSTSIZE, reg_config, cfgshpbstsize);

    RU_REG_WRITE(shaper_idx, EPN_TX_L1S_SHP, CONFIG, reg_config);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_tx_l1s_shp_config_get(uint8_t shaper_idx, bdmf_boolean *cfgshpincovr, uint32_t *cfgshprate, uint8_t *cfgshpbstsize)
{
    uint32_t reg_config=0;

#ifdef VALIDATE_PARMS
    if(!cfgshpincovr || !cfgshprate || !cfgshpbstsize)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((shaper_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(shaper_idx, EPN_TX_L1S_SHP, CONFIG, reg_config);

    *cfgshpincovr = RU_FIELD_GET(shaper_idx, EPN_TX_L1S_SHP, CONFIG, CFGSHPINCOVR, reg_config);
    *cfgshprate = RU_FIELD_GET(shaper_idx, EPN_TX_L1S_SHP, CONFIG, CFGSHPRATE, reg_config);
    *cfgshpbstsize = RU_FIELD_GET(shaper_idx, EPN_TX_L1S_SHP, CONFIG, CFGSHPBSTSIZE, reg_config);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_tx_l1s_shp_que_en_set(uint8_t shaper_idx, uint32_t cfgshpen)
{
    uint32_t reg_que_en=0;

#ifdef VALIDATE_PARMS
    if((shaper_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_que_en = RU_FIELD_SET(shaper_idx, EPN_TX_L1S_SHP, QUE_EN, CFGSHPEN, reg_que_en, cfgshpen);

    RU_REG_WRITE(shaper_idx, EPN_TX_L1S_SHP, QUE_EN, reg_que_en);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_epn_tx_l1s_shp_que_en_get(uint8_t shaper_idx, uint32_t *cfgshpen)
{
    uint32_t reg_que_en=0;

#ifdef VALIDATE_PARMS
    if(!cfgshpen)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((shaper_idx >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(shaper_idx, EPN_TX_L1S_SHP, QUE_EN, reg_que_en);

    *cfgshpen = RU_FIELD_GET(shaper_idx, EPN_TX_L1S_SHP, QUE_EN, CFGSHPEN, reg_que_en);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
enum
{
    BDMF_config,
    BDMF_que_en,
};

typedef enum
{
    bdmf_address_config,
    bdmf_address_que_en,
}
bdmf_address;

typedef enum ShaperIdEnum
{
    DRV_EPON_SHAPER0 = 0,
    DRV_EPON_SHAPER1,
    DRV_EPON_SHAPER2,
    DRV_EPON_SHAPER3,
    DRV_EPON_SHAPER4,
    DRV_EPON_SHAPER5,
    DRV_EPON_SHAPER6,
    DRV_EPON_SHAPER8,
    DRV_EPON_SHAPER9,
    DRV_EPON_SHAPER10,
    DRV_EPON_SHAPER11,
    DRV_EPON_SHAPER12,
    DRV_EPON_SHAPER13,
    DRV_EPON_SHAPER14,
    DRV_EPON_SHAPER15,
    DRV_EPON_SHAPER16,
    DRV_EPON_SHAPER17,
    DRV_EPON_SHAPER18,
    DRV_EPON_SHAPER19,
    DRV_EPON_SHAPER20,
    DRV_EPON_SHAPER21,
    DRV_EPON_SHAPER22,
    DRV_EPON_SHAPER23,
    DRV_EPON_SHAPER24,
    DRV_EPON_SHAPER25,
    DRV_EPON_SHAPER26,
    DRV_EPON_SHAPER27,
    DRV_EPON_SHAPER28,
    DRV_EPON_SHAPER29,
    DRV_EPON_SHAPER30,
    DRV_EPON_SHAPER31,
    DRV_EPON_SHAPER_MAX = DRV_EPON_SHAPER31
} ShaperIdEnum;

struct bdmfmon_enum_val shaper_idx_enum_table[] = {
    {"SHAPER0 ", DRV_EPON_SHAPER0 },
    {"SHAPER1 ", DRV_EPON_SHAPER1 },
    {"SHAPER2 ", DRV_EPON_SHAPER2 },
    {"SHAPER3 ", DRV_EPON_SHAPER3 },
    {"SHAPER4 ", DRV_EPON_SHAPER4 },
    {"SHAPER5 ", DRV_EPON_SHAPER5 },
    {"SHAPER6 ", DRV_EPON_SHAPER6 },
    {"SHAPER8 ", DRV_EPON_SHAPER8 },
    {"SHAPER9 ", DRV_EPON_SHAPER9 },
    {"SHAPER10", DRV_EPON_SHAPER10},
    {"SHAPER11", DRV_EPON_SHAPER11},
    {"SHAPER12", DRV_EPON_SHAPER12},
    {"SHAPER13", DRV_EPON_SHAPER13},
    {"SHAPER14", DRV_EPON_SHAPER14},
    {"SHAPER15", DRV_EPON_SHAPER15},
    {"SHAPER16", DRV_EPON_SHAPER16},
    {"SHAPER17", DRV_EPON_SHAPER17},
    {"SHAPER18", DRV_EPON_SHAPER18},
    {"SHAPER19", DRV_EPON_SHAPER19},
    {"SHAPER20", DRV_EPON_SHAPER20},
    {"SHAPER21", DRV_EPON_SHAPER21},
    {"SHAPER22", DRV_EPON_SHAPER22},
    {"SHAPER23", DRV_EPON_SHAPER23},
    {"SHAPER24", DRV_EPON_SHAPER24},
    {"SHAPER25", DRV_EPON_SHAPER25},
    {"SHAPER26", DRV_EPON_SHAPER26},
    {"SHAPER27", DRV_EPON_SHAPER27},
    {"SHAPER28", DRV_EPON_SHAPER28},
    {"SHAPER29", DRV_EPON_SHAPER29},
    {"SHAPER30", DRV_EPON_SHAPER30},
    {"SHAPER31", DRV_EPON_SHAPER31},
    {NULL, 0},
};

static int bcm_epn_tx_l1s_shp_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err;

    switch(parm[0].value.unumber)
    {
    case BDMF_config:
        err = ag_drv_epn_tx_l1s_shp_config_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case BDMF_que_en:
        err = ag_drv_epn_tx_l1s_shp_que_en_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_epn_tx_l1s_shp_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err;

    switch(parm[0].value.unumber)
    {
    case BDMF_config:
    {
        bdmf_boolean cfgshpincovr;
        uint32_t cfgshprate;
        uint8_t cfgshpbstsize;
        err = ag_drv_epn_tx_l1s_shp_config_get(parm[1].value.unumber, &cfgshpincovr, &cfgshprate, &cfgshpbstsize);
        bdmf_session_print(session, "cfgshpincovr = %u = 0x%x\n", cfgshpincovr, cfgshpincovr);
        bdmf_session_print(session, "cfgshprate = %u = 0x%x\n", cfgshprate, cfgshprate);
        bdmf_session_print(session, "cfgshpbstsize = %u = 0x%x\n", cfgshpbstsize, cfgshpbstsize);
        break;
    }
    case BDMF_que_en:
    {
        uint32_t cfgshpen;
        err = ag_drv_epn_tx_l1s_shp_que_en_get(parm[1].value.unumber, &cfgshpen);
        bdmf_session_print(session, "cfgshpen = %u = 0x%x\n", cfgshpen, cfgshpen);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_epn_tx_l1s_shp_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    uint8_t shaper_idx = parm[1].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        bdmf_boolean cfgshpincovr=gtmv(m, 1);
        uint32_t cfgshprate=gtmv(m, 23);
        uint8_t cfgshpbstsize=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_epn_tx_l1s_shp_config_set(%u %u %u %u)\n", shaper_idx, cfgshpincovr, cfgshprate, cfgshpbstsize);
        if(!err) ag_drv_epn_tx_l1s_shp_config_set(shaper_idx, cfgshpincovr, cfgshprate, cfgshpbstsize);
        if(!err) ag_drv_epn_tx_l1s_shp_config_get( shaper_idx, &cfgshpincovr, &cfgshprate, &cfgshpbstsize);
        if(!err) bdmf_session_print(session, "ag_drv_epn_tx_l1s_shp_config_get(%u %u %u %u)\n", shaper_idx, cfgshpincovr, cfgshprate, cfgshpbstsize);
        if(err || cfgshpincovr!=gtmv(m, 1) || cfgshprate!=gtmv(m, 23) || cfgshpbstsize!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t cfgshpen=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_epn_tx_l1s_shp_que_en_set(%u %u)\n", shaper_idx, cfgshpen);
        if(!err) ag_drv_epn_tx_l1s_shp_que_en_set(shaper_idx, cfgshpen);
        if(!err) ag_drv_epn_tx_l1s_shp_que_en_get( shaper_idx, &cfgshpen);
        if(!err) bdmf_session_print(session, "ag_drv_epn_tx_l1s_shp_que_en_get(%u %u)\n", shaper_idx, cfgshpen);
        if(err || cfgshpen!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    return err;
}

static int bcm_epn_tx_l1s_shp_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_config : reg = &RU_REG(EPN_TX_L1S_SHP, CONFIG); blk = &RU_BLK(EPN_TX_L1S_SHP); break;
    case bdmf_address_que_en : reg = &RU_REG(EPN_TX_L1S_SHP, QUE_EN); blk = &RU_BLK(EPN_TX_L1S_SHP); break;
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

bdmfmon_handle_t ag_drv_epn_tx_l1s_shp_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "epn_tx_l1s_shp"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "epn_tx_l1s_shp", "epn_tx_l1s_shp", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_config[]={
            BDMFMON_MAKE_PARM_ENUM("shaper_idx", "shaper_idx", shaper_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("cfgshpincovr", "cfgshpincovr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgshprate", "cfgshprate", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cfgshpbstsize", "cfgshpbstsize", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_que_en[]={
            BDMFMON_MAKE_PARM_ENUM("shaper_idx", "shaper_idx", shaper_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("cfgshpen", "cfgshpen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="config", .val=BDMF_config, .parms=set_config },
            { .name="que_en", .val=BDMF_que_en, .parms=set_que_en },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_epn_tx_l1s_shp_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_MAKE_PARM_ENUM("shaper_idx", "shaper_idx", shaper_idx_enum_table, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="config", .val=BDMF_config, .parms=set_default },
            { .name="que_en", .val=BDMF_que_en, .parms=set_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_epn_tx_l1s_shp_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_epn_tx_l1s_shp_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0),
            BDMFMON_MAKE_PARM_ENUM("shaper_idx", "shaper_idx", shaper_idx_enum_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="CONFIG" , .val=bdmf_address_config },
            { .name="QUE_EN" , .val=bdmf_address_que_en },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_epn_tx_l1s_shp_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM_ENUM("index1", "shaper_idx", shaper_idx_enum_table, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

