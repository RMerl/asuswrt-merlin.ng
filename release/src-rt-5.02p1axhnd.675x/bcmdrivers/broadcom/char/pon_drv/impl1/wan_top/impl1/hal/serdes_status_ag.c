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
#include "serdes_status_ag.h"
bdmf_error_t ag_drv_serdes_status_status_get(serdes_status_status *status)
{
    uint32_t reg_status=0;

#ifdef VALIDATE_PARMS
    if(!status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, SERDES_STATUS, STATUS, reg_status);

    status->pmd_pll1_lock = RU_FIELD_GET(0, SERDES_STATUS, STATUS, PMD_PLL1_LOCK, reg_status);
    status->o_laser_burst_en = RU_FIELD_GET(0, SERDES_STATUS, STATUS, O_LASER_BURST_EN, reg_status);
    status->pmi_lp_error = RU_FIELD_GET(0, SERDES_STATUS, STATUS, PMI_LP_ERROR, reg_status);
    status->pmi_lp_acknowledge = RU_FIELD_GET(0, SERDES_STATUS, STATUS, PMI_LP_ACKNOWLEDGE, reg_status);
    status->pmd_signal_detect_0 = RU_FIELD_GET(0, SERDES_STATUS, STATUS, PMD_SIGNAL_DETECT_0, reg_status);
    status->pmd_energy_detect_0 = RU_FIELD_GET(0, SERDES_STATUS, STATUS, PMD_ENERGY_DETECT_0, reg_status);
    status->pmd_rx_lock_0 = RU_FIELD_GET(0, SERDES_STATUS, STATUS, PMD_RX_LOCK_0, reg_status);
    status->pmd_tx_clk_vld = RU_FIELD_GET(0, SERDES_STATUS, STATUS, PMD_TX_CLK_VLD, reg_status);
    status->pmd_rx_clk_vld_0 = RU_FIELD_GET(0, SERDES_STATUS, STATUS, PMD_RX_CLK_VLD_0, reg_status);
    status->pmd_rx_lock_0_invert = RU_FIELD_GET(0, SERDES_STATUS, STATUS, PMD_RX_LOCK_0_INVERT, reg_status);
    status->pmd_pll0_lock = RU_FIELD_GET(0, SERDES_STATUS, STATUS, PMD_PLL0_LOCK, reg_status);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
enum
{
    BDMF_status,
};

typedef enum
{
    bdmf_address_status,
}
bdmf_address;

static int bcm_serdes_status_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err;

    switch(parm[0].value.unumber)
    {
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_serdes_status_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err;

    switch(parm[0].value.unumber)
    {
    case BDMF_status:
    {
        serdes_status_status status;
        err = ag_drv_serdes_status_status_get(&status);
        bdmf_session_print(session, "pmd_pll1_lock = %u = 0x%x\n", status.pmd_pll1_lock, status.pmd_pll1_lock);
        bdmf_session_print(session, "o_laser_burst_en = %u = 0x%x\n", status.o_laser_burst_en, status.o_laser_burst_en);
        bdmf_session_print(session, "pmi_lp_error = %u = 0x%x\n", status.pmi_lp_error, status.pmi_lp_error);
        bdmf_session_print(session, "pmi_lp_acknowledge = %u = 0x%x\n", status.pmi_lp_acknowledge, status.pmi_lp_acknowledge);
        bdmf_session_print(session, "pmd_signal_detect_0 = %u = 0x%x\n", status.pmd_signal_detect_0, status.pmd_signal_detect_0);
        bdmf_session_print(session, "pmd_energy_detect_0 = %u = 0x%x\n", status.pmd_energy_detect_0, status.pmd_energy_detect_0);
        bdmf_session_print(session, "pmd_rx_lock_0 = %u = 0x%x\n", status.pmd_rx_lock_0, status.pmd_rx_lock_0);
        bdmf_session_print(session, "pmd_tx_clk_vld = %u = 0x%x\n", status.pmd_tx_clk_vld, status.pmd_tx_clk_vld);
        bdmf_session_print(session, "pmd_rx_clk_vld_0 = %u = 0x%x\n", status.pmd_rx_clk_vld_0, status.pmd_rx_clk_vld_0);
        bdmf_session_print(session, "pmd_rx_lock_0_invert = %u = 0x%x\n", status.pmd_rx_lock_0_invert, status.pmd_rx_lock_0_invert);
        bdmf_session_print(session, "pmd_pll0_lock = %u = 0x%x\n", status.pmd_pll0_lock, status.pmd_pll0_lock);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_serdes_status_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        serdes_status_status status = {.pmd_pll1_lock=gtmv(m, 1), .o_laser_burst_en=gtmv(m, 1), .pmi_lp_error=gtmv(m, 1), .pmi_lp_acknowledge=gtmv(m, 1), .pmd_signal_detect_0=gtmv(m, 1), .pmd_energy_detect_0=gtmv(m, 1), .pmd_rx_lock_0=gtmv(m, 1), .pmd_tx_clk_vld=gtmv(m, 1), .pmd_rx_clk_vld_0=gtmv(m, 1), .pmd_rx_lock_0_invert=gtmv(m, 1), .pmd_pll0_lock=gtmv(m, 1)};
        if(!err) ag_drv_serdes_status_status_get( &status);
        if(!err) bdmf_session_print(session, "ag_drv_serdes_status_status_get( %u %u %u %u %u %u %u %u %u %u %u)\n", status.pmd_pll1_lock, status.o_laser_burst_en, status.pmi_lp_error, status.pmi_lp_acknowledge, status.pmd_signal_detect_0, status.pmd_energy_detect_0, status.pmd_rx_lock_0, status.pmd_tx_clk_vld, status.pmd_rx_clk_vld_0, status.pmd_rx_lock_0_invert, status.pmd_pll0_lock);
    }
    return err;
}

static int bcm_serdes_status_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_status : reg = &RU_REG(SERDES_STATUS, STATUS); blk = &RU_BLK(SERDES_STATUS); break;
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

bdmfmon_handle_t ag_drv_serdes_status_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "serdes_status"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "serdes_status", "serdes_status", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_enum_val_t selector_table[] = {
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_serdes_status_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="status", .val=BDMF_status, .parms=set_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_serdes_status_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_serdes_status_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="STATUS" , .val=bdmf_address_status },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_serdes_status_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

