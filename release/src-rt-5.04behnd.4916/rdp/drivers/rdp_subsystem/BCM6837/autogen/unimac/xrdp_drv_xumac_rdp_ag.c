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


#include "xrdp_drv_drivers_common_ag.h"
#include "xrdp_drv_xumac_rdp_ag.h"

#define BLOCK_ADDR_COUNT_BITS 2
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_xumac_rdp_ipg_hd_bkp_cntl_set(uint8_t xumac_id, bdmf_boolean hd_fc_ena, bdmf_boolean hd_fc_bkoff_ok, uint8_t ipg_config_rx)
{
    uint32_t reg_ipg_hd_bkp_cntl = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT) ||
       (hd_fc_ena >= _1BITS_MAX_VAL_) ||
       (hd_fc_bkoff_ok >= _1BITS_MAX_VAL_) ||
       (ipg_config_rx >= _5BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_ipg_hd_bkp_cntl = RU_FIELD_SET(xumac_id, XUMAC_RDP, IPG_HD_BKP_CNTL, HD_FC_ENA, reg_ipg_hd_bkp_cntl, hd_fc_ena);
    reg_ipg_hd_bkp_cntl = RU_FIELD_SET(xumac_id, XUMAC_RDP, IPG_HD_BKP_CNTL, HD_FC_BKOFF_OK, reg_ipg_hd_bkp_cntl, hd_fc_bkoff_ok);
    reg_ipg_hd_bkp_cntl = RU_FIELD_SET(xumac_id, XUMAC_RDP, IPG_HD_BKP_CNTL, IPG_CONFIG_RX, reg_ipg_hd_bkp_cntl, ipg_config_rx);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, IPG_HD_BKP_CNTL, reg_ipg_hd_bkp_cntl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_ipg_hd_bkp_cntl_get(uint8_t xumac_id, bdmf_boolean *hd_fc_ena, bdmf_boolean *hd_fc_bkoff_ok, uint8_t *ipg_config_rx)
{
    uint32_t reg_ipg_hd_bkp_cntl;

#ifdef VALIDATE_PARMS
    if (!hd_fc_ena || !hd_fc_bkoff_ok || !ipg_config_rx)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, IPG_HD_BKP_CNTL, reg_ipg_hd_bkp_cntl);

    *hd_fc_ena = RU_FIELD_GET(xumac_id, XUMAC_RDP, IPG_HD_BKP_CNTL, HD_FC_ENA, reg_ipg_hd_bkp_cntl);
    *hd_fc_bkoff_ok = RU_FIELD_GET(xumac_id, XUMAC_RDP, IPG_HD_BKP_CNTL, HD_FC_BKOFF_OK, reg_ipg_hd_bkp_cntl);
    *ipg_config_rx = RU_FIELD_GET(xumac_id, XUMAC_RDP, IPG_HD_BKP_CNTL, IPG_CONFIG_RX, reg_ipg_hd_bkp_cntl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_command_config_set(uint8_t xumac_id, const xumac_rdp_command_config *command_config)
{
    uint32_t reg_command_config = 0;

#ifdef VALIDATE_PARMS
    if(!command_config)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT) ||
       (command_config->tx_ena >= _1BITS_MAX_VAL_) ||
       (command_config->rx_ena >= _1BITS_MAX_VAL_) ||
       (command_config->eth_speed >= _2BITS_MAX_VAL_) ||
       (command_config->promis_en >= _1BITS_MAX_VAL_) ||
       (command_config->pad_en >= _1BITS_MAX_VAL_) ||
       (command_config->crc_fwd >= _1BITS_MAX_VAL_) ||
       (command_config->pause_fwd >= _1BITS_MAX_VAL_) ||
       (command_config->pause_ignore >= _1BITS_MAX_VAL_) ||
       (command_config->tx_addr_ins >= _1BITS_MAX_VAL_) ||
       (command_config->hd_ena >= _1BITS_MAX_VAL_) ||
       (command_config->rx_low_latency_en >= _1BITS_MAX_VAL_) ||
       (command_config->overflow_en >= _1BITS_MAX_VAL_) ||
       (command_config->sw_reset >= _1BITS_MAX_VAL_) ||
       (command_config->fcs_corrupt_urun_en >= _1BITS_MAX_VAL_) ||
       (command_config->loop_ena >= _1BITS_MAX_VAL_) ||
       (command_config->mac_loop_con >= _1BITS_MAX_VAL_) ||
       (command_config->sw_override_tx >= _1BITS_MAX_VAL_) ||
       (command_config->sw_override_rx >= _1BITS_MAX_VAL_) ||
       (command_config->oob_efc_mode >= _1BITS_MAX_VAL_) ||
       (command_config->bypass_oob_efc_synchronizer >= _1BITS_MAX_VAL_) ||
       (command_config->en_internal_tx_crs >= _1BITS_MAX_VAL_) ||
       (command_config->ena_ext_config >= _1BITS_MAX_VAL_) ||
       (command_config->cntl_frm_ena >= _1BITS_MAX_VAL_) ||
       (command_config->no_lgth_check >= _1BITS_MAX_VAL_) ||
       (command_config->line_loopback >= _1BITS_MAX_VAL_) ||
       (command_config->fd_tx_urun_fix_en >= _1BITS_MAX_VAL_) ||
       (command_config->ignore_tx_pause >= _1BITS_MAX_VAL_) ||
       (command_config->oob_efc_disab >= _1BITS_MAX_VAL_) ||
       (command_config->runt_filter_dis >= _1BITS_MAX_VAL_) ||
       (command_config->eth_speed_bit2 >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_command_config = RU_FIELD_SET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, TX_ENA, reg_command_config, command_config->tx_ena);
    reg_command_config = RU_FIELD_SET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, RX_ENA, reg_command_config, command_config->rx_ena);
    reg_command_config = RU_FIELD_SET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, ETH_SPEED, reg_command_config, command_config->eth_speed);
    reg_command_config = RU_FIELD_SET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, PROMIS_EN, reg_command_config, command_config->promis_en);
    reg_command_config = RU_FIELD_SET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, PAD_EN, reg_command_config, command_config->pad_en);
    reg_command_config = RU_FIELD_SET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, CRC_FWD, reg_command_config, command_config->crc_fwd);
    reg_command_config = RU_FIELD_SET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, PAUSE_FWD, reg_command_config, command_config->pause_fwd);
    reg_command_config = RU_FIELD_SET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, PAUSE_IGNORE, reg_command_config, command_config->pause_ignore);
    reg_command_config = RU_FIELD_SET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, TX_ADDR_INS, reg_command_config, command_config->tx_addr_ins);
    reg_command_config = RU_FIELD_SET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, HD_ENA, reg_command_config, command_config->hd_ena);
    reg_command_config = RU_FIELD_SET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, RX_LOW_LATENCY_EN, reg_command_config, command_config->rx_low_latency_en);
    reg_command_config = RU_FIELD_SET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, OVERFLOW_EN, reg_command_config, command_config->overflow_en);
    reg_command_config = RU_FIELD_SET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, SW_RESET, reg_command_config, command_config->sw_reset);
    reg_command_config = RU_FIELD_SET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, FCS_CORRUPT_URUN_EN, reg_command_config, command_config->fcs_corrupt_urun_en);
    reg_command_config = RU_FIELD_SET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, LOOP_ENA, reg_command_config, command_config->loop_ena);
    reg_command_config = RU_FIELD_SET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, MAC_LOOP_CON, reg_command_config, command_config->mac_loop_con);
    reg_command_config = RU_FIELD_SET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, SW_OVERRIDE_TX, reg_command_config, command_config->sw_override_tx);
    reg_command_config = RU_FIELD_SET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, SW_OVERRIDE_RX, reg_command_config, command_config->sw_override_rx);
    reg_command_config = RU_FIELD_SET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, OOB_EFC_MODE, reg_command_config, command_config->oob_efc_mode);
    reg_command_config = RU_FIELD_SET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, BYPASS_OOB_EFC_SYNCHRONIZER, reg_command_config, command_config->bypass_oob_efc_synchronizer);
    reg_command_config = RU_FIELD_SET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, EN_INTERNAL_TX_CRS, reg_command_config, command_config->en_internal_tx_crs);
    reg_command_config = RU_FIELD_SET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, ENA_EXT_CONFIG, reg_command_config, command_config->ena_ext_config);
    reg_command_config = RU_FIELD_SET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, CNTL_FRM_ENA, reg_command_config, command_config->cntl_frm_ena);
    reg_command_config = RU_FIELD_SET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, NO_LGTH_CHECK, reg_command_config, command_config->no_lgth_check);
    reg_command_config = RU_FIELD_SET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, LINE_LOOPBACK, reg_command_config, command_config->line_loopback);
    reg_command_config = RU_FIELD_SET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, FD_TX_URUN_FIX_EN, reg_command_config, command_config->fd_tx_urun_fix_en);
    reg_command_config = RU_FIELD_SET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, IGNORE_TX_PAUSE, reg_command_config, command_config->ignore_tx_pause);
    reg_command_config = RU_FIELD_SET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, OOB_EFC_DISAB, reg_command_config, command_config->oob_efc_disab);
    reg_command_config = RU_FIELD_SET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, RUNT_FILTER_DIS, reg_command_config, command_config->runt_filter_dis);
    reg_command_config = RU_FIELD_SET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, ETH_SPEED_BIT2, reg_command_config, command_config->eth_speed_bit2);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, COMMAND_CONFIG, reg_command_config);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_command_config_get(uint8_t xumac_id, xumac_rdp_command_config *command_config)
{
    uint32_t reg_command_config;

#ifdef VALIDATE_PARMS
    if (!command_config)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, COMMAND_CONFIG, reg_command_config);

    command_config->tx_ena = RU_FIELD_GET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, TX_ENA, reg_command_config);
    command_config->rx_ena = RU_FIELD_GET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, RX_ENA, reg_command_config);
    command_config->eth_speed = RU_FIELD_GET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, ETH_SPEED, reg_command_config);
    command_config->promis_en = RU_FIELD_GET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, PROMIS_EN, reg_command_config);
    command_config->pad_en = RU_FIELD_GET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, PAD_EN, reg_command_config);
    command_config->crc_fwd = RU_FIELD_GET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, CRC_FWD, reg_command_config);
    command_config->pause_fwd = RU_FIELD_GET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, PAUSE_FWD, reg_command_config);
    command_config->pause_ignore = RU_FIELD_GET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, PAUSE_IGNORE, reg_command_config);
    command_config->tx_addr_ins = RU_FIELD_GET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, TX_ADDR_INS, reg_command_config);
    command_config->hd_ena = RU_FIELD_GET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, HD_ENA, reg_command_config);
    command_config->rx_low_latency_en = RU_FIELD_GET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, RX_LOW_LATENCY_EN, reg_command_config);
    command_config->overflow_en = RU_FIELD_GET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, OVERFLOW_EN, reg_command_config);
    command_config->sw_reset = RU_FIELD_GET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, SW_RESET, reg_command_config);
    command_config->fcs_corrupt_urun_en = RU_FIELD_GET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, FCS_CORRUPT_URUN_EN, reg_command_config);
    command_config->loop_ena = RU_FIELD_GET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, LOOP_ENA, reg_command_config);
    command_config->mac_loop_con = RU_FIELD_GET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, MAC_LOOP_CON, reg_command_config);
    command_config->sw_override_tx = RU_FIELD_GET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, SW_OVERRIDE_TX, reg_command_config);
    command_config->sw_override_rx = RU_FIELD_GET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, SW_OVERRIDE_RX, reg_command_config);
    command_config->oob_efc_mode = RU_FIELD_GET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, OOB_EFC_MODE, reg_command_config);
    command_config->bypass_oob_efc_synchronizer = RU_FIELD_GET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, BYPASS_OOB_EFC_SYNCHRONIZER, reg_command_config);
    command_config->en_internal_tx_crs = RU_FIELD_GET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, EN_INTERNAL_TX_CRS, reg_command_config);
    command_config->ena_ext_config = RU_FIELD_GET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, ENA_EXT_CONFIG, reg_command_config);
    command_config->cntl_frm_ena = RU_FIELD_GET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, CNTL_FRM_ENA, reg_command_config);
    command_config->no_lgth_check = RU_FIELD_GET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, NO_LGTH_CHECK, reg_command_config);
    command_config->line_loopback = RU_FIELD_GET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, LINE_LOOPBACK, reg_command_config);
    command_config->fd_tx_urun_fix_en = RU_FIELD_GET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, FD_TX_URUN_FIX_EN, reg_command_config);
    command_config->ignore_tx_pause = RU_FIELD_GET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, IGNORE_TX_PAUSE, reg_command_config);
    command_config->oob_efc_disab = RU_FIELD_GET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, OOB_EFC_DISAB, reg_command_config);
    command_config->runt_filter_dis = RU_FIELD_GET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, RUNT_FILTER_DIS, reg_command_config);
    command_config->eth_speed_bit2 = RU_FIELD_GET(xumac_id, XUMAC_RDP, COMMAND_CONFIG, ETH_SPEED_BIT2, reg_command_config);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_mac_0_set(uint8_t xumac_id, uint32_t mac_addr0)
{
    uint32_t reg_mac_0 = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_mac_0 = RU_FIELD_SET(xumac_id, XUMAC_RDP, MAC_0, MAC_ADDR0, reg_mac_0, mac_addr0);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, MAC_0, reg_mac_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_mac_0_get(uint8_t xumac_id, uint32_t *mac_addr0)
{
    uint32_t reg_mac_0;

#ifdef VALIDATE_PARMS
    if (!mac_addr0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, MAC_0, reg_mac_0);

    *mac_addr0 = RU_FIELD_GET(xumac_id, XUMAC_RDP, MAC_0, MAC_ADDR0, reg_mac_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_mac_1_set(uint8_t xumac_id, uint16_t mac_addr1)
{
    uint32_t reg_mac_1 = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_mac_1 = RU_FIELD_SET(xumac_id, XUMAC_RDP, MAC_1, MAC_ADDR1, reg_mac_1, mac_addr1);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, MAC_1, reg_mac_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_mac_1_get(uint8_t xumac_id, uint16_t *mac_addr1)
{
    uint32_t reg_mac_1;

#ifdef VALIDATE_PARMS
    if (!mac_addr1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, MAC_1, reg_mac_1);

    *mac_addr1 = RU_FIELD_GET(xumac_id, XUMAC_RDP, MAC_1, MAC_ADDR1, reg_mac_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_frm_length_set(uint8_t xumac_id, uint16_t maxfr)
{
    uint32_t reg_frm_length = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT) ||
       (maxfr >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_frm_length = RU_FIELD_SET(xumac_id, XUMAC_RDP, FRM_LENGTH, MAXFR, reg_frm_length, maxfr);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, FRM_LENGTH, reg_frm_length);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_frm_length_get(uint8_t xumac_id, uint16_t *maxfr)
{
    uint32_t reg_frm_length;

#ifdef VALIDATE_PARMS
    if (!maxfr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, FRM_LENGTH, reg_frm_length);

    *maxfr = RU_FIELD_GET(xumac_id, XUMAC_RDP, FRM_LENGTH, MAXFR, reg_frm_length);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_pause_quant_set(uint8_t xumac_id, uint16_t pause_quant)
{
    uint32_t reg_pause_quant = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pause_quant = RU_FIELD_SET(xumac_id, XUMAC_RDP, PAUSE_QUANT, PAUSE_QUANT, reg_pause_quant, pause_quant);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, PAUSE_QUANT, reg_pause_quant);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_pause_quant_get(uint8_t xumac_id, uint16_t *pause_quant)
{
    uint32_t reg_pause_quant;

#ifdef VALIDATE_PARMS
    if (!pause_quant)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, PAUSE_QUANT, reg_pause_quant);

    *pause_quant = RU_FIELD_GET(xumac_id, XUMAC_RDP, PAUSE_QUANT, PAUSE_QUANT, reg_pause_quant);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tx_ts_seq_id_get(uint8_t xumac_id, uint16_t *tsts_seq_id, bdmf_boolean *tsts_valid)
{
    uint32_t reg_tx_ts_seq_id;

#ifdef VALIDATE_PARMS
    if (!tsts_seq_id || !tsts_valid)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, TX_TS_SEQ_ID, reg_tx_ts_seq_id);

    *tsts_seq_id = RU_FIELD_GET(xumac_id, XUMAC_RDP, TX_TS_SEQ_ID, TSTS_SEQ_ID, reg_tx_ts_seq_id);
    *tsts_valid = RU_FIELD_GET(xumac_id, XUMAC_RDP, TX_TS_SEQ_ID, TSTS_VALID, reg_tx_ts_seq_id);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_sfd_offset_set(uint8_t xumac_id, uint8_t sfd_offset)
{
    uint32_t reg_sfd_offset = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT) ||
       (sfd_offset >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_sfd_offset = RU_FIELD_SET(xumac_id, XUMAC_RDP, SFD_OFFSET, SFD_OFFSET, reg_sfd_offset, sfd_offset);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, SFD_OFFSET, reg_sfd_offset);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_sfd_offset_get(uint8_t xumac_id, uint8_t *sfd_offset)
{
    uint32_t reg_sfd_offset;

#ifdef VALIDATE_PARMS
    if (!sfd_offset)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, SFD_OFFSET, reg_sfd_offset);

    *sfd_offset = RU_FIELD_GET(xumac_id, XUMAC_RDP, SFD_OFFSET, SFD_OFFSET, reg_sfd_offset);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_mac_mode_get(uint8_t xumac_id, xumac_rdp_mac_mode *mac_mode)
{
    uint32_t reg_mac_mode;

#ifdef VALIDATE_PARMS
    if (!mac_mode)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, MAC_MODE, reg_mac_mode);

    mac_mode->mac_speed = RU_FIELD_GET(xumac_id, XUMAC_RDP, MAC_MODE, MAC_SPEED, reg_mac_mode);
    mac_mode->mac_duplex = RU_FIELD_GET(xumac_id, XUMAC_RDP, MAC_MODE, MAC_DUPLEX, reg_mac_mode);
    mac_mode->mac_rx_pause = RU_FIELD_GET(xumac_id, XUMAC_RDP, MAC_MODE, MAC_RX_PAUSE, reg_mac_mode);
    mac_mode->mac_tx_pause = RU_FIELD_GET(xumac_id, XUMAC_RDP, MAC_MODE, MAC_TX_PAUSE, reg_mac_mode);
    mac_mode->link_status = RU_FIELD_GET(xumac_id, XUMAC_RDP, MAC_MODE, LINK_STATUS, reg_mac_mode);
    mac_mode->mac_speed_bit2 = RU_FIELD_GET(xumac_id, XUMAC_RDP, MAC_MODE, MAC_SPEED_BIT2, reg_mac_mode);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tag_0_set(uint8_t xumac_id, uint16_t frm_tag_0, bdmf_boolean config_outer_tpid_enable)
{
    uint32_t reg_tag_0 = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT) ||
       (config_outer_tpid_enable >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tag_0 = RU_FIELD_SET(xumac_id, XUMAC_RDP, TAG_0, FRM_TAG_0, reg_tag_0, frm_tag_0);
    reg_tag_0 = RU_FIELD_SET(xumac_id, XUMAC_RDP, TAG_0, CONFIG_OUTER_TPID_ENABLE, reg_tag_0, config_outer_tpid_enable);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, TAG_0, reg_tag_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tag_0_get(uint8_t xumac_id, uint16_t *frm_tag_0, bdmf_boolean *config_outer_tpid_enable)
{
    uint32_t reg_tag_0;

#ifdef VALIDATE_PARMS
    if (!frm_tag_0 || !config_outer_tpid_enable)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, TAG_0, reg_tag_0);

    *frm_tag_0 = RU_FIELD_GET(xumac_id, XUMAC_RDP, TAG_0, FRM_TAG_0, reg_tag_0);
    *config_outer_tpid_enable = RU_FIELD_GET(xumac_id, XUMAC_RDP, TAG_0, CONFIG_OUTER_TPID_ENABLE, reg_tag_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tag_1_set(uint8_t xumac_id, uint16_t frm_tag_1, bdmf_boolean config_inner_tpid_enable)
{
    uint32_t reg_tag_1 = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT) ||
       (config_inner_tpid_enable >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tag_1 = RU_FIELD_SET(xumac_id, XUMAC_RDP, TAG_1, FRM_TAG_1, reg_tag_1, frm_tag_1);
    reg_tag_1 = RU_FIELD_SET(xumac_id, XUMAC_RDP, TAG_1, CONFIG_INNER_TPID_ENABLE, reg_tag_1, config_inner_tpid_enable);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, TAG_1, reg_tag_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tag_1_get(uint8_t xumac_id, uint16_t *frm_tag_1, bdmf_boolean *config_inner_tpid_enable)
{
    uint32_t reg_tag_1;

#ifdef VALIDATE_PARMS
    if (!frm_tag_1 || !config_inner_tpid_enable)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, TAG_1, reg_tag_1);

    *frm_tag_1 = RU_FIELD_GET(xumac_id, XUMAC_RDP, TAG_1, FRM_TAG_1, reg_tag_1);
    *config_inner_tpid_enable = RU_FIELD_GET(xumac_id, XUMAC_RDP, TAG_1, CONFIG_INNER_TPID_ENABLE, reg_tag_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_rx_pause_quanta_scale_set(uint8_t xumac_id, uint16_t scale_value, bdmf_boolean scale_control, bdmf_boolean scale_fix)
{
    uint32_t reg_rx_pause_quanta_scale = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT) ||
       (scale_control >= _1BITS_MAX_VAL_) ||
       (scale_fix >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_pause_quanta_scale = RU_FIELD_SET(xumac_id, XUMAC_RDP, RX_PAUSE_QUANTA_SCALE, SCALE_VALUE, reg_rx_pause_quanta_scale, scale_value);
    reg_rx_pause_quanta_scale = RU_FIELD_SET(xumac_id, XUMAC_RDP, RX_PAUSE_QUANTA_SCALE, SCALE_CONTROL, reg_rx_pause_quanta_scale, scale_control);
    reg_rx_pause_quanta_scale = RU_FIELD_SET(xumac_id, XUMAC_RDP, RX_PAUSE_QUANTA_SCALE, SCALE_FIX, reg_rx_pause_quanta_scale, scale_fix);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, RX_PAUSE_QUANTA_SCALE, reg_rx_pause_quanta_scale);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_rx_pause_quanta_scale_get(uint8_t xumac_id, uint16_t *scale_value, bdmf_boolean *scale_control, bdmf_boolean *scale_fix)
{
    uint32_t reg_rx_pause_quanta_scale;

#ifdef VALIDATE_PARMS
    if (!scale_value || !scale_control || !scale_fix)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, RX_PAUSE_QUANTA_SCALE, reg_rx_pause_quanta_scale);

    *scale_value = RU_FIELD_GET(xumac_id, XUMAC_RDP, RX_PAUSE_QUANTA_SCALE, SCALE_VALUE, reg_rx_pause_quanta_scale);
    *scale_control = RU_FIELD_GET(xumac_id, XUMAC_RDP, RX_PAUSE_QUANTA_SCALE, SCALE_CONTROL, reg_rx_pause_quanta_scale);
    *scale_fix = RU_FIELD_GET(xumac_id, XUMAC_RDP, RX_PAUSE_QUANTA_SCALE, SCALE_FIX, reg_rx_pause_quanta_scale);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tx_preamble_set(uint8_t xumac_id, uint8_t tx_preamble)
{
    uint32_t reg_tx_preamble = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT) ||
       (tx_preamble >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tx_preamble = RU_FIELD_SET(xumac_id, XUMAC_RDP, TX_PREAMBLE, TX_PREAMBLE, reg_tx_preamble, tx_preamble);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, TX_PREAMBLE, reg_tx_preamble);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tx_preamble_get(uint8_t xumac_id, uint8_t *tx_preamble)
{
    uint32_t reg_tx_preamble;

#ifdef VALIDATE_PARMS
    if (!tx_preamble)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, TX_PREAMBLE, reg_tx_preamble);

    *tx_preamble = RU_FIELD_GET(xumac_id, XUMAC_RDP, TX_PREAMBLE, TX_PREAMBLE, reg_tx_preamble);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tx_ipg_length_set(uint8_t xumac_id, uint8_t tx_ipg_length, uint8_t tx_min_pkt_size)
{
    uint32_t reg_tx_ipg_length = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT) ||
       (tx_ipg_length >= _7BITS_MAX_VAL_) ||
       (tx_min_pkt_size >= _7BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tx_ipg_length = RU_FIELD_SET(xumac_id, XUMAC_RDP, TX_IPG_LENGTH, TX_IPG_LENGTH, reg_tx_ipg_length, tx_ipg_length);
    reg_tx_ipg_length = RU_FIELD_SET(xumac_id, XUMAC_RDP, TX_IPG_LENGTH, TX_MIN_PKT_SIZE, reg_tx_ipg_length, tx_min_pkt_size);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, TX_IPG_LENGTH, reg_tx_ipg_length);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tx_ipg_length_get(uint8_t xumac_id, uint8_t *tx_ipg_length, uint8_t *tx_min_pkt_size)
{
    uint32_t reg_tx_ipg_length;

#ifdef VALIDATE_PARMS
    if (!tx_ipg_length || !tx_min_pkt_size)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, TX_IPG_LENGTH, reg_tx_ipg_length);

    *tx_ipg_length = RU_FIELD_GET(xumac_id, XUMAC_RDP, TX_IPG_LENGTH, TX_IPG_LENGTH, reg_tx_ipg_length);
    *tx_min_pkt_size = RU_FIELD_GET(xumac_id, XUMAC_RDP, TX_IPG_LENGTH, TX_MIN_PKT_SIZE, reg_tx_ipg_length);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_pfc_xoff_timer_set(uint8_t xumac_id, uint16_t pfc_xoff_timer)
{
    uint32_t reg_pfc_xoff_timer = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pfc_xoff_timer = RU_FIELD_SET(xumac_id, XUMAC_RDP, PFC_XOFF_TIMER, PFC_XOFF_TIMER, reg_pfc_xoff_timer, pfc_xoff_timer);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, PFC_XOFF_TIMER, reg_pfc_xoff_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_pfc_xoff_timer_get(uint8_t xumac_id, uint16_t *pfc_xoff_timer)
{
    uint32_t reg_pfc_xoff_timer;

#ifdef VALIDATE_PARMS
    if (!pfc_xoff_timer)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, PFC_XOFF_TIMER, reg_pfc_xoff_timer);

    *pfc_xoff_timer = RU_FIELD_GET(xumac_id, XUMAC_RDP, PFC_XOFF_TIMER, PFC_XOFF_TIMER, reg_pfc_xoff_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_umac_eee_ctrl_set(uint8_t xumac_id, const xumac_rdp_umac_eee_ctrl *umac_eee_ctrl)
{
    uint32_t reg_umac_eee_ctrl = 0;

#ifdef VALIDATE_PARMS
    if(!umac_eee_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT) ||
       (umac_eee_ctrl->eee_en >= _1BITS_MAX_VAL_) ||
       (umac_eee_ctrl->rx_fifo_check >= _1BITS_MAX_VAL_) ||
       (umac_eee_ctrl->eee_txclk_dis >= _1BITS_MAX_VAL_) ||
       (umac_eee_ctrl->dis_eee_10m >= _1BITS_MAX_VAL_) ||
       (umac_eee_ctrl->lp_idle_prediction_mode >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_umac_eee_ctrl = RU_FIELD_SET(xumac_id, XUMAC_RDP, UMAC_EEE_CTRL, EEE_EN, reg_umac_eee_ctrl, umac_eee_ctrl->eee_en);
    reg_umac_eee_ctrl = RU_FIELD_SET(xumac_id, XUMAC_RDP, UMAC_EEE_CTRL, RX_FIFO_CHECK, reg_umac_eee_ctrl, umac_eee_ctrl->rx_fifo_check);
    reg_umac_eee_ctrl = RU_FIELD_SET(xumac_id, XUMAC_RDP, UMAC_EEE_CTRL, EEE_TXCLK_DIS, reg_umac_eee_ctrl, umac_eee_ctrl->eee_txclk_dis);
    reg_umac_eee_ctrl = RU_FIELD_SET(xumac_id, XUMAC_RDP, UMAC_EEE_CTRL, DIS_EEE_10M, reg_umac_eee_ctrl, umac_eee_ctrl->dis_eee_10m);
    reg_umac_eee_ctrl = RU_FIELD_SET(xumac_id, XUMAC_RDP, UMAC_EEE_CTRL, LP_IDLE_PREDICTION_MODE, reg_umac_eee_ctrl, umac_eee_ctrl->lp_idle_prediction_mode);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, UMAC_EEE_CTRL, reg_umac_eee_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_umac_eee_ctrl_get(uint8_t xumac_id, xumac_rdp_umac_eee_ctrl *umac_eee_ctrl)
{
    uint32_t reg_umac_eee_ctrl;

#ifdef VALIDATE_PARMS
    if (!umac_eee_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, UMAC_EEE_CTRL, reg_umac_eee_ctrl);

    umac_eee_ctrl->eee_en = RU_FIELD_GET(xumac_id, XUMAC_RDP, UMAC_EEE_CTRL, EEE_EN, reg_umac_eee_ctrl);
    umac_eee_ctrl->rx_fifo_check = RU_FIELD_GET(xumac_id, XUMAC_RDP, UMAC_EEE_CTRL, RX_FIFO_CHECK, reg_umac_eee_ctrl);
    umac_eee_ctrl->eee_txclk_dis = RU_FIELD_GET(xumac_id, XUMAC_RDP, UMAC_EEE_CTRL, EEE_TXCLK_DIS, reg_umac_eee_ctrl);
    umac_eee_ctrl->dis_eee_10m = RU_FIELD_GET(xumac_id, XUMAC_RDP, UMAC_EEE_CTRL, DIS_EEE_10M, reg_umac_eee_ctrl);
    umac_eee_ctrl->lp_idle_prediction_mode = RU_FIELD_GET(xumac_id, XUMAC_RDP, UMAC_EEE_CTRL, LP_IDLE_PREDICTION_MODE, reg_umac_eee_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_mii_eee_delay_entry_timer_set(uint8_t xumac_id, uint32_t mii_eee_lpi_timer)
{
    uint32_t reg_mii_eee_delay_entry_timer = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_mii_eee_delay_entry_timer = RU_FIELD_SET(xumac_id, XUMAC_RDP, MII_EEE_DELAY_ENTRY_TIMER, MII_EEE_LPI_TIMER, reg_mii_eee_delay_entry_timer, mii_eee_lpi_timer);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, MII_EEE_DELAY_ENTRY_TIMER, reg_mii_eee_delay_entry_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_mii_eee_delay_entry_timer_get(uint8_t xumac_id, uint32_t *mii_eee_lpi_timer)
{
    uint32_t reg_mii_eee_delay_entry_timer;

#ifdef VALIDATE_PARMS
    if (!mii_eee_lpi_timer)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, MII_EEE_DELAY_ENTRY_TIMER, reg_mii_eee_delay_entry_timer);

    *mii_eee_lpi_timer = RU_FIELD_GET(xumac_id, XUMAC_RDP, MII_EEE_DELAY_ENTRY_TIMER, MII_EEE_LPI_TIMER, reg_mii_eee_delay_entry_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gmii_eee_delay_entry_timer_set(uint8_t xumac_id, uint32_t gmii_eee_lpi_timer)
{
    uint32_t reg_gmii_eee_delay_entry_timer = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gmii_eee_delay_entry_timer = RU_FIELD_SET(xumac_id, XUMAC_RDP, GMII_EEE_DELAY_ENTRY_TIMER, GMII_EEE_LPI_TIMER, reg_gmii_eee_delay_entry_timer, gmii_eee_lpi_timer);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GMII_EEE_DELAY_ENTRY_TIMER, reg_gmii_eee_delay_entry_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gmii_eee_delay_entry_timer_get(uint8_t xumac_id, uint32_t *gmii_eee_lpi_timer)
{
    uint32_t reg_gmii_eee_delay_entry_timer;

#ifdef VALIDATE_PARMS
    if (!gmii_eee_lpi_timer)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GMII_EEE_DELAY_ENTRY_TIMER, reg_gmii_eee_delay_entry_timer);

    *gmii_eee_lpi_timer = RU_FIELD_GET(xumac_id, XUMAC_RDP, GMII_EEE_DELAY_ENTRY_TIMER, GMII_EEE_LPI_TIMER, reg_gmii_eee_delay_entry_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_umac_eee_ref_count_set(uint8_t xumac_id, uint16_t eee_ref_count)
{
    uint32_t reg_umac_eee_ref_count = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_umac_eee_ref_count = RU_FIELD_SET(xumac_id, XUMAC_RDP, UMAC_EEE_REF_COUNT, EEE_REF_COUNT, reg_umac_eee_ref_count, eee_ref_count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, UMAC_EEE_REF_COUNT, reg_umac_eee_ref_count);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_umac_eee_ref_count_get(uint8_t xumac_id, uint16_t *eee_ref_count)
{
    uint32_t reg_umac_eee_ref_count;

#ifdef VALIDATE_PARMS
    if (!eee_ref_count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, UMAC_EEE_REF_COUNT, reg_umac_eee_ref_count);

    *eee_ref_count = RU_FIELD_GET(xumac_id, XUMAC_RDP, UMAC_EEE_REF_COUNT, EEE_REF_COUNT, reg_umac_eee_ref_count);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_umac_timestamp_adjust_set(uint8_t xumac_id, uint16_t adjust, bdmf_boolean en_1588, bdmf_boolean auto_adjust)
{
    uint32_t reg_umac_timestamp_adjust = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT) ||
       (adjust >= _9BITS_MAX_VAL_) ||
       (en_1588 >= _1BITS_MAX_VAL_) ||
       (auto_adjust >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_umac_timestamp_adjust = RU_FIELD_SET(xumac_id, XUMAC_RDP, UMAC_TIMESTAMP_ADJUST, ADJUST, reg_umac_timestamp_adjust, adjust);
    reg_umac_timestamp_adjust = RU_FIELD_SET(xumac_id, XUMAC_RDP, UMAC_TIMESTAMP_ADJUST, EN_1588, reg_umac_timestamp_adjust, en_1588);
    reg_umac_timestamp_adjust = RU_FIELD_SET(xumac_id, XUMAC_RDP, UMAC_TIMESTAMP_ADJUST, AUTO_ADJUST, reg_umac_timestamp_adjust, auto_adjust);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, UMAC_TIMESTAMP_ADJUST, reg_umac_timestamp_adjust);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_umac_timestamp_adjust_get(uint8_t xumac_id, uint16_t *adjust, bdmf_boolean *en_1588, bdmf_boolean *auto_adjust)
{
    uint32_t reg_umac_timestamp_adjust;

#ifdef VALIDATE_PARMS
    if (!adjust || !en_1588 || !auto_adjust)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, UMAC_TIMESTAMP_ADJUST, reg_umac_timestamp_adjust);

    *adjust = RU_FIELD_GET(xumac_id, XUMAC_RDP, UMAC_TIMESTAMP_ADJUST, ADJUST, reg_umac_timestamp_adjust);
    *en_1588 = RU_FIELD_GET(xumac_id, XUMAC_RDP, UMAC_TIMESTAMP_ADJUST, EN_1588, reg_umac_timestamp_adjust);
    *auto_adjust = RU_FIELD_GET(xumac_id, XUMAC_RDP, UMAC_TIMESTAMP_ADJUST, AUTO_ADJUST, reg_umac_timestamp_adjust);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_umac_rx_pkt_drop_status_set(uint8_t xumac_id, bdmf_boolean rx_ipg_inval)
{
    uint32_t reg_umac_rx_pkt_drop_status = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT) ||
       (rx_ipg_inval >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_umac_rx_pkt_drop_status = RU_FIELD_SET(xumac_id, XUMAC_RDP, UMAC_RX_PKT_DROP_STATUS, RX_IPG_INVAL, reg_umac_rx_pkt_drop_status, rx_ipg_inval);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, UMAC_RX_PKT_DROP_STATUS, reg_umac_rx_pkt_drop_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_umac_rx_pkt_drop_status_get(uint8_t xumac_id, bdmf_boolean *rx_ipg_inval)
{
    uint32_t reg_umac_rx_pkt_drop_status;

#ifdef VALIDATE_PARMS
    if (!rx_ipg_inval)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, UMAC_RX_PKT_DROP_STATUS, reg_umac_rx_pkt_drop_status);

    *rx_ipg_inval = RU_FIELD_GET(xumac_id, XUMAC_RDP, UMAC_RX_PKT_DROP_STATUS, RX_IPG_INVAL, reg_umac_rx_pkt_drop_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_umac_symmetric_idle_threshold_set(uint8_t xumac_id, uint16_t threshold_value)
{
    uint32_t reg_umac_symmetric_idle_threshold = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_umac_symmetric_idle_threshold = RU_FIELD_SET(xumac_id, XUMAC_RDP, UMAC_SYMMETRIC_IDLE_THRESHOLD, THRESHOLD_VALUE, reg_umac_symmetric_idle_threshold, threshold_value);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, UMAC_SYMMETRIC_IDLE_THRESHOLD, reg_umac_symmetric_idle_threshold);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_umac_symmetric_idle_threshold_get(uint8_t xumac_id, uint16_t *threshold_value)
{
    uint32_t reg_umac_symmetric_idle_threshold;

#ifdef VALIDATE_PARMS
    if (!threshold_value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, UMAC_SYMMETRIC_IDLE_THRESHOLD, reg_umac_symmetric_idle_threshold);

    *threshold_value = RU_FIELD_GET(xumac_id, XUMAC_RDP, UMAC_SYMMETRIC_IDLE_THRESHOLD, THRESHOLD_VALUE, reg_umac_symmetric_idle_threshold);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_mii_eee_wake_timer_set(uint8_t xumac_id, uint16_t mii_eee_wake_timer)
{
    uint32_t reg_mii_eee_wake_timer = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_mii_eee_wake_timer = RU_FIELD_SET(xumac_id, XUMAC_RDP, MII_EEE_WAKE_TIMER, MII_EEE_WAKE_TIMER, reg_mii_eee_wake_timer, mii_eee_wake_timer);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, MII_EEE_WAKE_TIMER, reg_mii_eee_wake_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_mii_eee_wake_timer_get(uint8_t xumac_id, uint16_t *mii_eee_wake_timer)
{
    uint32_t reg_mii_eee_wake_timer;

#ifdef VALIDATE_PARMS
    if (!mii_eee_wake_timer)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, MII_EEE_WAKE_TIMER, reg_mii_eee_wake_timer);

    *mii_eee_wake_timer = RU_FIELD_GET(xumac_id, XUMAC_RDP, MII_EEE_WAKE_TIMER, MII_EEE_WAKE_TIMER, reg_mii_eee_wake_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gmii_eee_wake_timer_set(uint8_t xumac_id, uint16_t gmii_eee_wake_timer)
{
    uint32_t reg_gmii_eee_wake_timer = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gmii_eee_wake_timer = RU_FIELD_SET(xumac_id, XUMAC_RDP, GMII_EEE_WAKE_TIMER, GMII_EEE_WAKE_TIMER, reg_gmii_eee_wake_timer, gmii_eee_wake_timer);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GMII_EEE_WAKE_TIMER, reg_gmii_eee_wake_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gmii_eee_wake_timer_get(uint8_t xumac_id, uint16_t *gmii_eee_wake_timer)
{
    uint32_t reg_gmii_eee_wake_timer;

#ifdef VALIDATE_PARMS
    if (!gmii_eee_wake_timer)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GMII_EEE_WAKE_TIMER, reg_gmii_eee_wake_timer);

    *gmii_eee_wake_timer = RU_FIELD_GET(xumac_id, XUMAC_RDP, GMII_EEE_WAKE_TIMER, GMII_EEE_WAKE_TIMER, reg_gmii_eee_wake_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_umac_rev_id_get(uint8_t xumac_id, uint8_t *patch, uint8_t *revision_id_minor, uint8_t *revision_id_major)
{
    uint32_t reg_umac_rev_id;

#ifdef VALIDATE_PARMS
    if (!patch || !revision_id_minor || !revision_id_major)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, UMAC_REV_ID, reg_umac_rev_id);

    *patch = RU_FIELD_GET(xumac_id, XUMAC_RDP, UMAC_REV_ID, PATCH, reg_umac_rev_id);
    *revision_id_minor = RU_FIELD_GET(xumac_id, XUMAC_RDP, UMAC_REV_ID, REVISION_ID_MINOR, reg_umac_rev_id);
    *revision_id_major = RU_FIELD_GET(xumac_id, XUMAC_RDP, UMAC_REV_ID, REVISION_ID_MAJOR, reg_umac_rev_id);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gmii_2p5g_eee_delay_entry_timer_set(uint8_t xumac_id, uint32_t gmii_2p5g_eee_lpi_timer)
{
    uint32_t reg_gmii_2p5g_eee_delay_entry_timer = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gmii_2p5g_eee_delay_entry_timer = RU_FIELD_SET(xumac_id, XUMAC_RDP, GMII_2P5G_EEE_DELAY_ENTRY_TIMER, GMII_2P5G_EEE_LPI_TIMER, reg_gmii_2p5g_eee_delay_entry_timer, gmii_2p5g_eee_lpi_timer);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GMII_2P5G_EEE_DELAY_ENTRY_TIMER, reg_gmii_2p5g_eee_delay_entry_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gmii_2p5g_eee_delay_entry_timer_get(uint8_t xumac_id, uint32_t *gmii_2p5g_eee_lpi_timer)
{
    uint32_t reg_gmii_2p5g_eee_delay_entry_timer;

#ifdef VALIDATE_PARMS
    if (!gmii_2p5g_eee_lpi_timer)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GMII_2P5G_EEE_DELAY_ENTRY_TIMER, reg_gmii_2p5g_eee_delay_entry_timer);

    *gmii_2p5g_eee_lpi_timer = RU_FIELD_GET(xumac_id, XUMAC_RDP, GMII_2P5G_EEE_DELAY_ENTRY_TIMER, GMII_2P5G_EEE_LPI_TIMER, reg_gmii_2p5g_eee_delay_entry_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gmii_5g_eee_delay_entry_timer_set(uint8_t xumac_id, uint32_t gmii_5g_eee_lpi_timer)
{
    uint32_t reg_gmii_5g_eee_delay_entry_timer = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gmii_5g_eee_delay_entry_timer = RU_FIELD_SET(xumac_id, XUMAC_RDP, GMII_5G_EEE_DELAY_ENTRY_TIMER, GMII_5G_EEE_LPI_TIMER, reg_gmii_5g_eee_delay_entry_timer, gmii_5g_eee_lpi_timer);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GMII_5G_EEE_DELAY_ENTRY_TIMER, reg_gmii_5g_eee_delay_entry_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gmii_5g_eee_delay_entry_timer_get(uint8_t xumac_id, uint32_t *gmii_5g_eee_lpi_timer)
{
    uint32_t reg_gmii_5g_eee_delay_entry_timer;

#ifdef VALIDATE_PARMS
    if (!gmii_5g_eee_lpi_timer)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GMII_5G_EEE_DELAY_ENTRY_TIMER, reg_gmii_5g_eee_delay_entry_timer);

    *gmii_5g_eee_lpi_timer = RU_FIELD_GET(xumac_id, XUMAC_RDP, GMII_5G_EEE_DELAY_ENTRY_TIMER, GMII_5G_EEE_LPI_TIMER, reg_gmii_5g_eee_delay_entry_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gmii_10g_eee_delay_entry_timer_set(uint8_t xumac_id, uint32_t gmii_10g_eee_lpi_timer)
{
    uint32_t reg_gmii_10g_eee_delay_entry_timer = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gmii_10g_eee_delay_entry_timer = RU_FIELD_SET(xumac_id, XUMAC_RDP, GMII_10G_EEE_DELAY_ENTRY_TIMER, GMII_10G_EEE_LPI_TIMER, reg_gmii_10g_eee_delay_entry_timer, gmii_10g_eee_lpi_timer);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GMII_10G_EEE_DELAY_ENTRY_TIMER, reg_gmii_10g_eee_delay_entry_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gmii_10g_eee_delay_entry_timer_get(uint8_t xumac_id, uint32_t *gmii_10g_eee_lpi_timer)
{
    uint32_t reg_gmii_10g_eee_delay_entry_timer;

#ifdef VALIDATE_PARMS
    if (!gmii_10g_eee_lpi_timer)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GMII_10G_EEE_DELAY_ENTRY_TIMER, reg_gmii_10g_eee_delay_entry_timer);

    *gmii_10g_eee_lpi_timer = RU_FIELD_GET(xumac_id, XUMAC_RDP, GMII_10G_EEE_DELAY_ENTRY_TIMER, GMII_10G_EEE_LPI_TIMER, reg_gmii_10g_eee_delay_entry_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gmii_2p5g_eee_wake_timer_set(uint8_t xumac_id, uint16_t gmii_2p5g_eee_wake_timer)
{
    uint32_t reg_gmii_2p5g_eee_wake_timer = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gmii_2p5g_eee_wake_timer = RU_FIELD_SET(xumac_id, XUMAC_RDP, GMII_2P5G_EEE_WAKE_TIMER, GMII_2P5G_EEE_WAKE_TIMER, reg_gmii_2p5g_eee_wake_timer, gmii_2p5g_eee_wake_timer);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GMII_2P5G_EEE_WAKE_TIMER, reg_gmii_2p5g_eee_wake_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gmii_2p5g_eee_wake_timer_get(uint8_t xumac_id, uint16_t *gmii_2p5g_eee_wake_timer)
{
    uint32_t reg_gmii_2p5g_eee_wake_timer;

#ifdef VALIDATE_PARMS
    if (!gmii_2p5g_eee_wake_timer)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GMII_2P5G_EEE_WAKE_TIMER, reg_gmii_2p5g_eee_wake_timer);

    *gmii_2p5g_eee_wake_timer = RU_FIELD_GET(xumac_id, XUMAC_RDP, GMII_2P5G_EEE_WAKE_TIMER, GMII_2P5G_EEE_WAKE_TIMER, reg_gmii_2p5g_eee_wake_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gmii_5g_eee_wake_timer_set(uint8_t xumac_id, uint16_t gmii_5g_eee_wake_timer)
{
    uint32_t reg_gmii_5g_eee_wake_timer = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gmii_5g_eee_wake_timer = RU_FIELD_SET(xumac_id, XUMAC_RDP, GMII_5G_EEE_WAKE_TIMER, GMII_5G_EEE_WAKE_TIMER, reg_gmii_5g_eee_wake_timer, gmii_5g_eee_wake_timer);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GMII_5G_EEE_WAKE_TIMER, reg_gmii_5g_eee_wake_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gmii_5g_eee_wake_timer_get(uint8_t xumac_id, uint16_t *gmii_5g_eee_wake_timer)
{
    uint32_t reg_gmii_5g_eee_wake_timer;

#ifdef VALIDATE_PARMS
    if (!gmii_5g_eee_wake_timer)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GMII_5G_EEE_WAKE_TIMER, reg_gmii_5g_eee_wake_timer);

    *gmii_5g_eee_wake_timer = RU_FIELD_GET(xumac_id, XUMAC_RDP, GMII_5G_EEE_WAKE_TIMER, GMII_5G_EEE_WAKE_TIMER, reg_gmii_5g_eee_wake_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gmii_10g_eee_wake_timer_set(uint8_t xumac_id, uint16_t gmii_10g_eee_wake_timer)
{
    uint32_t reg_gmii_10g_eee_wake_timer = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gmii_10g_eee_wake_timer = RU_FIELD_SET(xumac_id, XUMAC_RDP, GMII_10G_EEE_WAKE_TIMER, GMII_10G_EEE_WAKE_TIMER, reg_gmii_10g_eee_wake_timer, gmii_10g_eee_wake_timer);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GMII_10G_EEE_WAKE_TIMER, reg_gmii_10g_eee_wake_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gmii_10g_eee_wake_timer_get(uint8_t xumac_id, uint16_t *gmii_10g_eee_wake_timer)
{
    uint32_t reg_gmii_10g_eee_wake_timer;

#ifdef VALIDATE_PARMS
    if (!gmii_10g_eee_wake_timer)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GMII_10G_EEE_WAKE_TIMER, reg_gmii_10g_eee_wake_timer);

    *gmii_10g_eee_wake_timer = RU_FIELD_GET(xumac_id, XUMAC_RDP, GMII_10G_EEE_WAKE_TIMER, GMII_10G_EEE_WAKE_TIMER, reg_gmii_10g_eee_wake_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_active_eee_delay_entry_timer_get(uint8_t xumac_id, uint32_t *active_eee_lpi_timer)
{
    uint32_t reg_active_eee_delay_entry_timer;

#ifdef VALIDATE_PARMS
    if (!active_eee_lpi_timer)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, ACTIVE_EEE_DELAY_ENTRY_TIMER, reg_active_eee_delay_entry_timer);

    *active_eee_lpi_timer = RU_FIELD_GET(xumac_id, XUMAC_RDP, ACTIVE_EEE_DELAY_ENTRY_TIMER, ACTIVE_EEE_LPI_TIMER, reg_active_eee_delay_entry_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_active_eee_wake_timer_get(uint8_t xumac_id, uint16_t *active_eee_wake_time)
{
    uint32_t reg_active_eee_wake_timer;

#ifdef VALIDATE_PARMS
    if (!active_eee_wake_time)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, ACTIVE_EEE_WAKE_TIMER, reg_active_eee_wake_timer);

    *active_eee_wake_time = RU_FIELD_GET(xumac_id, XUMAC_RDP, ACTIVE_EEE_WAKE_TIMER, ACTIVE_EEE_WAKE_TIME, reg_active_eee_wake_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_mac_pfc_type_set(uint8_t xumac_id, uint16_t pfc_eth_type)
{
    uint32_t reg_mac_pfc_type = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_mac_pfc_type = RU_FIELD_SET(xumac_id, XUMAC_RDP, MAC_PFC_TYPE, PFC_ETH_TYPE, reg_mac_pfc_type, pfc_eth_type);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, MAC_PFC_TYPE, reg_mac_pfc_type);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_mac_pfc_type_get(uint8_t xumac_id, uint16_t *pfc_eth_type)
{
    uint32_t reg_mac_pfc_type;

#ifdef VALIDATE_PARMS
    if (!pfc_eth_type)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, MAC_PFC_TYPE, reg_mac_pfc_type);

    *pfc_eth_type = RU_FIELD_GET(xumac_id, XUMAC_RDP, MAC_PFC_TYPE, PFC_ETH_TYPE, reg_mac_pfc_type);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_mac_pfc_opcode_set(uint8_t xumac_id, uint16_t pfc_opcode)
{
    uint32_t reg_mac_pfc_opcode = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_mac_pfc_opcode = RU_FIELD_SET(xumac_id, XUMAC_RDP, MAC_PFC_OPCODE, PFC_OPCODE, reg_mac_pfc_opcode, pfc_opcode);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, MAC_PFC_OPCODE, reg_mac_pfc_opcode);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_mac_pfc_opcode_get(uint8_t xumac_id, uint16_t *pfc_opcode)
{
    uint32_t reg_mac_pfc_opcode;

#ifdef VALIDATE_PARMS
    if (!pfc_opcode)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, MAC_PFC_OPCODE, reg_mac_pfc_opcode);

    *pfc_opcode = RU_FIELD_GET(xumac_id, XUMAC_RDP, MAC_PFC_OPCODE, PFC_OPCODE, reg_mac_pfc_opcode);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_mac_pfc_da_0_set(uint8_t xumac_id, uint32_t pfc_macda_0)
{
    uint32_t reg_mac_pfc_da_0 = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_mac_pfc_da_0 = RU_FIELD_SET(xumac_id, XUMAC_RDP, MAC_PFC_DA_0, PFC_MACDA_0, reg_mac_pfc_da_0, pfc_macda_0);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, MAC_PFC_DA_0, reg_mac_pfc_da_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_mac_pfc_da_0_get(uint8_t xumac_id, uint32_t *pfc_macda_0)
{
    uint32_t reg_mac_pfc_da_0;

#ifdef VALIDATE_PARMS
    if (!pfc_macda_0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, MAC_PFC_DA_0, reg_mac_pfc_da_0);

    *pfc_macda_0 = RU_FIELD_GET(xumac_id, XUMAC_RDP, MAC_PFC_DA_0, PFC_MACDA_0, reg_mac_pfc_da_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_mac_pfc_da_1_set(uint8_t xumac_id, uint16_t pfc_macda_1)
{
    uint32_t reg_mac_pfc_da_1 = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_mac_pfc_da_1 = RU_FIELD_SET(xumac_id, XUMAC_RDP, MAC_PFC_DA_1, PFC_MACDA_1, reg_mac_pfc_da_1, pfc_macda_1);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, MAC_PFC_DA_1, reg_mac_pfc_da_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_mac_pfc_da_1_get(uint8_t xumac_id, uint16_t *pfc_macda_1)
{
    uint32_t reg_mac_pfc_da_1;

#ifdef VALIDATE_PARMS
    if (!pfc_macda_1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, MAC_PFC_DA_1, reg_mac_pfc_da_1);

    *pfc_macda_1 = RU_FIELD_GET(xumac_id, XUMAC_RDP, MAC_PFC_DA_1, PFC_MACDA_1, reg_mac_pfc_da_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_macsec_prog_tx_crc_set(uint8_t xumac_id, uint32_t macsec_prog_tx_crc)
{
    uint32_t reg_macsec_prog_tx_crc = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_macsec_prog_tx_crc = RU_FIELD_SET(xumac_id, XUMAC_RDP, MACSEC_PROG_TX_CRC, MACSEC_PROG_TX_CRC, reg_macsec_prog_tx_crc, macsec_prog_tx_crc);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, MACSEC_PROG_TX_CRC, reg_macsec_prog_tx_crc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_macsec_prog_tx_crc_get(uint8_t xumac_id, uint32_t *macsec_prog_tx_crc)
{
    uint32_t reg_macsec_prog_tx_crc;

#ifdef VALIDATE_PARMS
    if (!macsec_prog_tx_crc)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, MACSEC_PROG_TX_CRC, reg_macsec_prog_tx_crc);

    *macsec_prog_tx_crc = RU_FIELD_GET(xumac_id, XUMAC_RDP, MACSEC_PROG_TX_CRC, MACSEC_PROG_TX_CRC, reg_macsec_prog_tx_crc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_macsec_cntrl_set(uint8_t xumac_id, bdmf_boolean tx_launch_en, bdmf_boolean tx_crc_corupt_en, bdmf_boolean tx_crc_program, bdmf_boolean dis_pause_data_var_ipg)
{
    uint32_t reg_macsec_cntrl = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT) ||
       (tx_launch_en >= _1BITS_MAX_VAL_) ||
       (tx_crc_corupt_en >= _1BITS_MAX_VAL_) ||
       (tx_crc_program >= _1BITS_MAX_VAL_) ||
       (dis_pause_data_var_ipg >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_macsec_cntrl = RU_FIELD_SET(xumac_id, XUMAC_RDP, MACSEC_CNTRL, TX_LAUNCH_EN, reg_macsec_cntrl, tx_launch_en);
    reg_macsec_cntrl = RU_FIELD_SET(xumac_id, XUMAC_RDP, MACSEC_CNTRL, TX_CRC_CORUPT_EN, reg_macsec_cntrl, tx_crc_corupt_en);
    reg_macsec_cntrl = RU_FIELD_SET(xumac_id, XUMAC_RDP, MACSEC_CNTRL, TX_CRC_PROGRAM, reg_macsec_cntrl, tx_crc_program);
    reg_macsec_cntrl = RU_FIELD_SET(xumac_id, XUMAC_RDP, MACSEC_CNTRL, DIS_PAUSE_DATA_VAR_IPG, reg_macsec_cntrl, dis_pause_data_var_ipg);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, MACSEC_CNTRL, reg_macsec_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_macsec_cntrl_get(uint8_t xumac_id, bdmf_boolean *tx_launch_en, bdmf_boolean *tx_crc_corupt_en, bdmf_boolean *tx_crc_program, bdmf_boolean *dis_pause_data_var_ipg)
{
    uint32_t reg_macsec_cntrl;

#ifdef VALIDATE_PARMS
    if (!tx_launch_en || !tx_crc_corupt_en || !tx_crc_program || !dis_pause_data_var_ipg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, MACSEC_CNTRL, reg_macsec_cntrl);

    *tx_launch_en = RU_FIELD_GET(xumac_id, XUMAC_RDP, MACSEC_CNTRL, TX_LAUNCH_EN, reg_macsec_cntrl);
    *tx_crc_corupt_en = RU_FIELD_GET(xumac_id, XUMAC_RDP, MACSEC_CNTRL, TX_CRC_CORUPT_EN, reg_macsec_cntrl);
    *tx_crc_program = RU_FIELD_GET(xumac_id, XUMAC_RDP, MACSEC_CNTRL, TX_CRC_PROGRAM, reg_macsec_cntrl);
    *dis_pause_data_var_ipg = RU_FIELD_GET(xumac_id, XUMAC_RDP, MACSEC_CNTRL, DIS_PAUSE_DATA_VAR_IPG, reg_macsec_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_ts_status_get(uint8_t xumac_id, bdmf_boolean *tx_ts_fifo_full, bdmf_boolean *tx_ts_fifo_empty, uint8_t *word_avail)
{
    uint32_t reg_ts_status;

#ifdef VALIDATE_PARMS
    if (!tx_ts_fifo_full || !tx_ts_fifo_empty || !word_avail)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, TS_STATUS, reg_ts_status);

    *tx_ts_fifo_full = RU_FIELD_GET(xumac_id, XUMAC_RDP, TS_STATUS, TX_TS_FIFO_FULL, reg_ts_status);
    *tx_ts_fifo_empty = RU_FIELD_GET(xumac_id, XUMAC_RDP, TS_STATUS, TX_TS_FIFO_EMPTY, reg_ts_status);
    *word_avail = RU_FIELD_GET(xumac_id, XUMAC_RDP, TS_STATUS, WORD_AVAIL, reg_ts_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tx_ts_data_get(uint8_t xumac_id, uint32_t *tx_ts_data)
{
    uint32_t reg_tx_ts_data;

#ifdef VALIDATE_PARMS
    if (!tx_ts_data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, TX_TS_DATA, reg_tx_ts_data);

    *tx_ts_data = RU_FIELD_GET(xumac_id, XUMAC_RDP, TX_TS_DATA, TX_TS_DATA, reg_tx_ts_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_pause_refresh_ctrl_set(uint8_t xumac_id, uint32_t refresh_timer, bdmf_boolean enable)
{
    uint32_t reg_pause_refresh_ctrl = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT) ||
       (refresh_timer >= _17BITS_MAX_VAL_) ||
       (enable >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pause_refresh_ctrl = RU_FIELD_SET(xumac_id, XUMAC_RDP, PAUSE_REFRESH_CTRL, REFRESH_TIMER, reg_pause_refresh_ctrl, refresh_timer);
    reg_pause_refresh_ctrl = RU_FIELD_SET(xumac_id, XUMAC_RDP, PAUSE_REFRESH_CTRL, ENABLE, reg_pause_refresh_ctrl, enable);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, PAUSE_REFRESH_CTRL, reg_pause_refresh_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_pause_refresh_ctrl_get(uint8_t xumac_id, uint32_t *refresh_timer, bdmf_boolean *enable)
{
    uint32_t reg_pause_refresh_ctrl;

#ifdef VALIDATE_PARMS
    if (!refresh_timer || !enable)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, PAUSE_REFRESH_CTRL, reg_pause_refresh_ctrl);

    *refresh_timer = RU_FIELD_GET(xumac_id, XUMAC_RDP, PAUSE_REFRESH_CTRL, REFRESH_TIMER, reg_pause_refresh_ctrl);
    *enable = RU_FIELD_GET(xumac_id, XUMAC_RDP, PAUSE_REFRESH_CTRL, ENABLE, reg_pause_refresh_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_flush_control_set(uint8_t xumac_id, bdmf_boolean flush)
{
    uint32_t reg_flush_control = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT) ||
       (flush >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_flush_control = RU_FIELD_SET(xumac_id, XUMAC_RDP, FLUSH_CONTROL, FLUSH, reg_flush_control, flush);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, FLUSH_CONTROL, reg_flush_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_flush_control_get(uint8_t xumac_id, bdmf_boolean *flush)
{
    uint32_t reg_flush_control;

#ifdef VALIDATE_PARMS
    if (!flush)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, FLUSH_CONTROL, reg_flush_control);

    *flush = RU_FIELD_GET(xumac_id, XUMAC_RDP, FLUSH_CONTROL, FLUSH, reg_flush_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_rxfifo_stat_get(uint8_t xumac_id, bdmf_boolean *rxfifo_underrun, bdmf_boolean *rxfifo_overrun)
{
    uint32_t reg_rxfifo_stat;

#ifdef VALIDATE_PARMS
    if (!rxfifo_underrun || !rxfifo_overrun)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, RXFIFO_STAT, reg_rxfifo_stat);

    *rxfifo_underrun = RU_FIELD_GET(xumac_id, XUMAC_RDP, RXFIFO_STAT, RXFIFO_UNDERRUN, reg_rxfifo_stat);
    *rxfifo_overrun = RU_FIELD_GET(xumac_id, XUMAC_RDP, RXFIFO_STAT, RXFIFO_OVERRUN, reg_rxfifo_stat);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_txfifo_stat_get(uint8_t xumac_id, bdmf_boolean *txfifo_underrun, bdmf_boolean *txfifo_overrun)
{
    uint32_t reg_txfifo_stat;

#ifdef VALIDATE_PARMS
    if (!txfifo_underrun || !txfifo_overrun)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, TXFIFO_STAT, reg_txfifo_stat);

    *txfifo_underrun = RU_FIELD_GET(xumac_id, XUMAC_RDP, TXFIFO_STAT, TXFIFO_UNDERRUN, reg_txfifo_stat);
    *txfifo_overrun = RU_FIELD_GET(xumac_id, XUMAC_RDP, TXFIFO_STAT, TXFIFO_OVERRUN, reg_txfifo_stat);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_mac_pfc_ctrl_set(uint8_t xumac_id, const xumac_rdp_mac_pfc_ctrl *mac_pfc_ctrl)
{
    uint32_t reg_mac_pfc_ctrl = 0;

#ifdef VALIDATE_PARMS
    if(!mac_pfc_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT) ||
       (mac_pfc_ctrl->pfc_tx_enbl >= _1BITS_MAX_VAL_) ||
       (mac_pfc_ctrl->pfc_rx_enbl >= _1BITS_MAX_VAL_) ||
       (mac_pfc_ctrl->force_pfc_xon >= _1BITS_MAX_VAL_) ||
       (mac_pfc_ctrl->rx_pass_pfc_frm >= _1BITS_MAX_VAL_) ||
       (mac_pfc_ctrl->pfc_stats_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_mac_pfc_ctrl = RU_FIELD_SET(xumac_id, XUMAC_RDP, MAC_PFC_CTRL, PFC_TX_ENBL, reg_mac_pfc_ctrl, mac_pfc_ctrl->pfc_tx_enbl);
    reg_mac_pfc_ctrl = RU_FIELD_SET(xumac_id, XUMAC_RDP, MAC_PFC_CTRL, PFC_RX_ENBL, reg_mac_pfc_ctrl, mac_pfc_ctrl->pfc_rx_enbl);
    reg_mac_pfc_ctrl = RU_FIELD_SET(xumac_id, XUMAC_RDP, MAC_PFC_CTRL, FORCE_PFC_XON, reg_mac_pfc_ctrl, mac_pfc_ctrl->force_pfc_xon);
    reg_mac_pfc_ctrl = RU_FIELD_SET(xumac_id, XUMAC_RDP, MAC_PFC_CTRL, RX_PASS_PFC_FRM, reg_mac_pfc_ctrl, mac_pfc_ctrl->rx_pass_pfc_frm);
    reg_mac_pfc_ctrl = RU_FIELD_SET(xumac_id, XUMAC_RDP, MAC_PFC_CTRL, PFC_STATS_EN, reg_mac_pfc_ctrl, mac_pfc_ctrl->pfc_stats_en);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, MAC_PFC_CTRL, reg_mac_pfc_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_mac_pfc_ctrl_get(uint8_t xumac_id, xumac_rdp_mac_pfc_ctrl *mac_pfc_ctrl)
{
    uint32_t reg_mac_pfc_ctrl;

#ifdef VALIDATE_PARMS
    if (!mac_pfc_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, MAC_PFC_CTRL, reg_mac_pfc_ctrl);

    mac_pfc_ctrl->pfc_tx_enbl = RU_FIELD_GET(xumac_id, XUMAC_RDP, MAC_PFC_CTRL, PFC_TX_ENBL, reg_mac_pfc_ctrl);
    mac_pfc_ctrl->pfc_rx_enbl = RU_FIELD_GET(xumac_id, XUMAC_RDP, MAC_PFC_CTRL, PFC_RX_ENBL, reg_mac_pfc_ctrl);
    mac_pfc_ctrl->force_pfc_xon = RU_FIELD_GET(xumac_id, XUMAC_RDP, MAC_PFC_CTRL, FORCE_PFC_XON, reg_mac_pfc_ctrl);
    mac_pfc_ctrl->rx_pass_pfc_frm = RU_FIELD_GET(xumac_id, XUMAC_RDP, MAC_PFC_CTRL, RX_PASS_PFC_FRM, reg_mac_pfc_ctrl);
    mac_pfc_ctrl->pfc_stats_en = RU_FIELD_GET(xumac_id, XUMAC_RDP, MAC_PFC_CTRL, PFC_STATS_EN, reg_mac_pfc_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_mac_pfc_refresh_ctrl_set(uint8_t xumac_id, bdmf_boolean pfc_refresh_en, uint16_t pfc_refresh_timer)
{
    uint32_t reg_mac_pfc_refresh_ctrl = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT) ||
       (pfc_refresh_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_mac_pfc_refresh_ctrl = RU_FIELD_SET(xumac_id, XUMAC_RDP, MAC_PFC_REFRESH_CTRL, PFC_REFRESH_EN, reg_mac_pfc_refresh_ctrl, pfc_refresh_en);
    reg_mac_pfc_refresh_ctrl = RU_FIELD_SET(xumac_id, XUMAC_RDP, MAC_PFC_REFRESH_CTRL, PFC_REFRESH_TIMER, reg_mac_pfc_refresh_ctrl, pfc_refresh_timer);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, MAC_PFC_REFRESH_CTRL, reg_mac_pfc_refresh_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_mac_pfc_refresh_ctrl_get(uint8_t xumac_id, bdmf_boolean *pfc_refresh_en, uint16_t *pfc_refresh_timer)
{
    uint32_t reg_mac_pfc_refresh_ctrl;

#ifdef VALIDATE_PARMS
    if (!pfc_refresh_en || !pfc_refresh_timer)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, MAC_PFC_REFRESH_CTRL, reg_mac_pfc_refresh_ctrl);

    *pfc_refresh_en = RU_FIELD_GET(xumac_id, XUMAC_RDP, MAC_PFC_REFRESH_CTRL, PFC_REFRESH_EN, reg_mac_pfc_refresh_ctrl);
    *pfc_refresh_timer = RU_FIELD_GET(xumac_id, XUMAC_RDP, MAC_PFC_REFRESH_CTRL, PFC_REFRESH_TIMER, reg_mac_pfc_refresh_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gr64_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_gr64 = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gr64 = RU_FIELD_SET(xumac_id, XUMAC_RDP, GR64, COUNT, reg_gr64, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GR64, reg_gr64);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gr64_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_gr64;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GR64, reg_gr64);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GR64, COUNT, reg_gr64);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gr64_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_gr64_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gr64_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GR64_UPPER, COUNT_U8, reg_gr64_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GR64_UPPER, reg_gr64_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gr64_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_gr64_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GR64_UPPER, reg_gr64_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GR64_UPPER, COUNT_U8, reg_gr64_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gr127_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_gr127 = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gr127 = RU_FIELD_SET(xumac_id, XUMAC_RDP, GR127, COUNT, reg_gr127, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GR127, reg_gr127);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gr127_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_gr127;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GR127, reg_gr127);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GR127, COUNT, reg_gr127);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gr127_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_gr127_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gr127_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GR127_UPPER, COUNT_U8, reg_gr127_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GR127_UPPER, reg_gr127_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gr127_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_gr127_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GR127_UPPER, reg_gr127_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GR127_UPPER, COUNT_U8, reg_gr127_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gr255_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_gr255 = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gr255 = RU_FIELD_SET(xumac_id, XUMAC_RDP, GR255, COUNT, reg_gr255, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GR255, reg_gr255);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gr255_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_gr255;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GR255, reg_gr255);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GR255, COUNT, reg_gr255);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gr255_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_gr255_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gr255_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GR255_UPPER, COUNT_U8, reg_gr255_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GR255_UPPER, reg_gr255_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gr255_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_gr255_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GR255_UPPER, reg_gr255_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GR255_UPPER, COUNT_U8, reg_gr255_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gr511_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_gr511 = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gr511 = RU_FIELD_SET(xumac_id, XUMAC_RDP, GR511, COUNT, reg_gr511, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GR511, reg_gr511);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gr511_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_gr511;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GR511, reg_gr511);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GR511, COUNT, reg_gr511);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gr511_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_gr511_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gr511_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GR511_UPPER, COUNT_U8, reg_gr511_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GR511_UPPER, reg_gr511_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gr511_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_gr511_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GR511_UPPER, reg_gr511_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GR511_UPPER, COUNT_U8, reg_gr511_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gr1023_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_gr1023 = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gr1023 = RU_FIELD_SET(xumac_id, XUMAC_RDP, GR1023, COUNT, reg_gr1023, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GR1023, reg_gr1023);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gr1023_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_gr1023;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GR1023, reg_gr1023);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GR1023, COUNT, reg_gr1023);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gr1023_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_gr1023_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gr1023_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GR1023_UPPER, COUNT_U8, reg_gr1023_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GR1023_UPPER, reg_gr1023_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gr1023_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_gr1023_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GR1023_UPPER, reg_gr1023_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GR1023_UPPER, COUNT_U8, reg_gr1023_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gr1518_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_gr1518 = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gr1518 = RU_FIELD_SET(xumac_id, XUMAC_RDP, GR1518, COUNT, reg_gr1518, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GR1518, reg_gr1518);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gr1518_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_gr1518;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GR1518, reg_gr1518);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GR1518, COUNT, reg_gr1518);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gr1518_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_gr1518_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gr1518_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GR1518_UPPER, COUNT_U8, reg_gr1518_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GR1518_UPPER, reg_gr1518_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gr1518_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_gr1518_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GR1518_UPPER, reg_gr1518_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GR1518_UPPER, COUNT_U8, reg_gr1518_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grmgv_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_grmgv = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grmgv = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRMGV, COUNT, reg_grmgv, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRMGV, reg_grmgv);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grmgv_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_grmgv;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRMGV, reg_grmgv);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRMGV, COUNT, reg_grmgv);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grmgv_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_grmgv_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grmgv_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRMGV_UPPER, COUNT_U8, reg_grmgv_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRMGV_UPPER, reg_grmgv_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grmgv_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_grmgv_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRMGV_UPPER, reg_grmgv_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRMGV_UPPER, COUNT_U8, reg_grmgv_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gr2047_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_gr2047 = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gr2047 = RU_FIELD_SET(xumac_id, XUMAC_RDP, GR2047, COUNT, reg_gr2047, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GR2047, reg_gr2047);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gr2047_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_gr2047;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GR2047, reg_gr2047);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GR2047, COUNT, reg_gr2047);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gr2047_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_gr2047_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gr2047_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GR2047_UPPER, COUNT_U8, reg_gr2047_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GR2047_UPPER, reg_gr2047_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gr2047_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_gr2047_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GR2047_UPPER, reg_gr2047_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GR2047_UPPER, COUNT_U8, reg_gr2047_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gr4095_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_gr4095 = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gr4095 = RU_FIELD_SET(xumac_id, XUMAC_RDP, GR4095, COUNT, reg_gr4095, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GR4095, reg_gr4095);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gr4095_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_gr4095;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GR4095, reg_gr4095);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GR4095, COUNT, reg_gr4095);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gr4095_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_gr4095_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gr4095_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GR4095_UPPER, COUNT_U8, reg_gr4095_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GR4095_UPPER, reg_gr4095_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gr4095_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_gr4095_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GR4095_UPPER, reg_gr4095_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GR4095_UPPER, COUNT_U8, reg_gr4095_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gr9216_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_gr9216 = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gr9216 = RU_FIELD_SET(xumac_id, XUMAC_RDP, GR9216, COUNT, reg_gr9216, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GR9216, reg_gr9216);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gr9216_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_gr9216;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GR9216, reg_gr9216);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GR9216, COUNT, reg_gr9216);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gr9216_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_gr9216_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gr9216_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GR9216_UPPER, COUNT_U8, reg_gr9216_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GR9216_UPPER, reg_gr9216_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gr9216_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_gr9216_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GR9216_UPPER, reg_gr9216_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GR9216_UPPER, COUNT_U8, reg_gr9216_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grpkt_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_grpkt = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grpkt = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRPKT, COUNT, reg_grpkt, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRPKT, reg_grpkt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grpkt_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_grpkt;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRPKT, reg_grpkt);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRPKT, COUNT, reg_grpkt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grpkt_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_grpkt_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grpkt_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRPKT_UPPER, COUNT_U8, reg_grpkt_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRPKT_UPPER, reg_grpkt_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grpkt_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_grpkt_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRPKT_UPPER, reg_grpkt_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRPKT_UPPER, COUNT_U8, reg_grpkt_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grbyt_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_grbyt = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grbyt = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRBYT, COUNT, reg_grbyt, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRBYT, reg_grbyt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grbyt_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_grbyt;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRBYT, reg_grbyt);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRBYT, COUNT, reg_grbyt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grbyt_upper_set(uint8_t xumac_id, uint16_t count_u16)
{
    uint32_t reg_grbyt_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grbyt_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRBYT_UPPER, COUNT_U16, reg_grbyt_upper, count_u16);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRBYT_UPPER, reg_grbyt_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grbyt_upper_get(uint8_t xumac_id, uint16_t *count_u16)
{
    uint32_t reg_grbyt_upper;

#ifdef VALIDATE_PARMS
    if (!count_u16)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRBYT_UPPER, reg_grbyt_upper);

    *count_u16 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRBYT_UPPER, COUNT_U16, reg_grbyt_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grmca_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_grmca = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grmca = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRMCA, COUNT, reg_grmca, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRMCA, reg_grmca);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grmca_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_grmca;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRMCA, reg_grmca);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRMCA, COUNT, reg_grmca);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grmca_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_grmca_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grmca_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRMCA_UPPER, COUNT_U8, reg_grmca_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRMCA_UPPER, reg_grmca_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grmca_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_grmca_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRMCA_UPPER, reg_grmca_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRMCA_UPPER, COUNT_U8, reg_grmca_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grbca_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_grbca = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grbca = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRBCA, COUNT, reg_grbca, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRBCA, reg_grbca);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grbca_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_grbca;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRBCA, reg_grbca);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRBCA, COUNT, reg_grbca);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grbca_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_grbca_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grbca_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRBCA_UPPER, COUNT_U8, reg_grbca_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRBCA_UPPER, reg_grbca_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grbca_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_grbca_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRBCA_UPPER, reg_grbca_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRBCA_UPPER, COUNT_U8, reg_grbca_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grfcs_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_grfcs = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grfcs = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRFCS, COUNT, reg_grfcs, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRFCS, reg_grfcs);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grfcs_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_grfcs;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRFCS, reg_grfcs);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRFCS, COUNT, reg_grfcs);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grfcs_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_grfcs_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grfcs_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRFCS_UPPER, COUNT_U8, reg_grfcs_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRFCS_UPPER, reg_grfcs_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grfcs_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_grfcs_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRFCS_UPPER, reg_grfcs_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRFCS_UPPER, COUNT_U8, reg_grfcs_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grxcf_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_grxcf = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grxcf = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRXCF, COUNT, reg_grxcf, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRXCF, reg_grxcf);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grxcf_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_grxcf;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRXCF, reg_grxcf);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRXCF, COUNT, reg_grxcf);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grxcf_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_grxcf_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grxcf_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRXCF_UPPER, COUNT_U8, reg_grxcf_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRXCF_UPPER, reg_grxcf_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grxcf_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_grxcf_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRXCF_UPPER, reg_grxcf_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRXCF_UPPER, COUNT_U8, reg_grxcf_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grxpf_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_grxpf = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grxpf = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRXPF, COUNT, reg_grxpf, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRXPF, reg_grxpf);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grxpf_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_grxpf;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRXPF, reg_grxpf);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRXPF, COUNT, reg_grxpf);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grxpf_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_grxpf_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grxpf_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRXPF_UPPER, COUNT_U8, reg_grxpf_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRXPF_UPPER, reg_grxpf_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grxpf_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_grxpf_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRXPF_UPPER, reg_grxpf_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRXPF_UPPER, COUNT_U8, reg_grxpf_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grxuo_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_grxuo = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grxuo = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRXUO, COUNT, reg_grxuo, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRXUO, reg_grxuo);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grxuo_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_grxuo;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRXUO, reg_grxuo);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRXUO, COUNT, reg_grxuo);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grxuo_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_grxuo_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grxuo_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRXUO_UPPER, COUNT_U8, reg_grxuo_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRXUO_UPPER, reg_grxuo_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grxuo_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_grxuo_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRXUO_UPPER, reg_grxuo_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRXUO_UPPER, COUNT_U8, reg_grxuo_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_graln_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_graln = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_graln = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRALN, COUNT, reg_graln, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRALN, reg_graln);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_graln_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_graln;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRALN, reg_graln);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRALN, COUNT, reg_graln);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_graln_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_graln_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_graln_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRALN_UPPER, COUNT_U8, reg_graln_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRALN_UPPER, reg_graln_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_graln_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_graln_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRALN_UPPER, reg_graln_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRALN_UPPER, COUNT_U8, reg_graln_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grflr_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_grflr = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grflr = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRFLR, COUNT, reg_grflr, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRFLR, reg_grflr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grflr_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_grflr;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRFLR, reg_grflr);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRFLR, COUNT, reg_grflr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grflr_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_grflr_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grflr_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRFLR_UPPER, COUNT_U8, reg_grflr_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRFLR_UPPER, reg_grflr_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grflr_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_grflr_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRFLR_UPPER, reg_grflr_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRFLR_UPPER, COUNT_U8, reg_grflr_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grcde_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_grcde = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grcde = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRCDE, COUNT, reg_grcde, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRCDE, reg_grcde);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grcde_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_grcde;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRCDE, reg_grcde);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRCDE, COUNT, reg_grcde);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grcde_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_grcde_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grcde_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRCDE_UPPER, COUNT_U8, reg_grcde_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRCDE_UPPER, reg_grcde_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grcde_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_grcde_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRCDE_UPPER, reg_grcde_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRCDE_UPPER, COUNT_U8, reg_grcde_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grfcr_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_grfcr = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grfcr = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRFCR, COUNT, reg_grfcr, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRFCR, reg_grfcr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grfcr_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_grfcr;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRFCR, reg_grfcr);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRFCR, COUNT, reg_grfcr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grfcr_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_grfcr_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grfcr_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRFCR_UPPER, COUNT_U8, reg_grfcr_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRFCR_UPPER, reg_grfcr_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grfcr_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_grfcr_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRFCR_UPPER, reg_grfcr_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRFCR_UPPER, COUNT_U8, reg_grfcr_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grovr_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_grovr = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grovr = RU_FIELD_SET(xumac_id, XUMAC_RDP, GROVR, COUNT, reg_grovr, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GROVR, reg_grovr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grovr_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_grovr;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GROVR, reg_grovr);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GROVR, COUNT, reg_grovr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grovr_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_grovr_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grovr_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GROVR_UPPER, COUNT_U8, reg_grovr_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GROVR_UPPER, reg_grovr_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grovr_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_grovr_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GROVR_UPPER, reg_grovr_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GROVR_UPPER, COUNT_U8, reg_grovr_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grjbr_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_grjbr = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grjbr = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRJBR, COUNT, reg_grjbr, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRJBR, reg_grjbr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grjbr_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_grjbr;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRJBR, reg_grjbr);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRJBR, COUNT, reg_grjbr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grjbr_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_grjbr_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grjbr_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRJBR_UPPER, COUNT_U8, reg_grjbr_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRJBR_UPPER, reg_grjbr_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grjbr_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_grjbr_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRJBR_UPPER, reg_grjbr_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRJBR_UPPER, COUNT_U8, reg_grjbr_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grmtue_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_grmtue = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grmtue = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRMTUE, COUNT, reg_grmtue, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRMTUE, reg_grmtue);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grmtue_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_grmtue;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRMTUE, reg_grmtue);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRMTUE, COUNT, reg_grmtue);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grmtue_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_grmtue_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grmtue_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRMTUE_UPPER, COUNT_U8, reg_grmtue_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRMTUE_UPPER, reg_grmtue_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grmtue_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_grmtue_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRMTUE_UPPER, reg_grmtue_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRMTUE_UPPER, COUNT_U8, reg_grmtue_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grpok_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_grpok = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grpok = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRPOK, COUNT, reg_grpok, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRPOK, reg_grpok);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grpok_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_grpok;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRPOK, reg_grpok);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRPOK, COUNT, reg_grpok);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grpok_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_grpok_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grpok_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRPOK_UPPER, COUNT_U8, reg_grpok_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRPOK_UPPER, reg_grpok_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grpok_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_grpok_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRPOK_UPPER, reg_grpok_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRPOK_UPPER, COUNT_U8, reg_grpok_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gruc_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_gruc = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gruc = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRUC, COUNT, reg_gruc, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRUC, reg_gruc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gruc_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_gruc;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRUC, reg_gruc);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRUC, COUNT, reg_gruc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gruc_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_gruc_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gruc_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRUC_UPPER, COUNT_U8, reg_gruc_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRUC_UPPER, reg_gruc_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gruc_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_gruc_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRUC_UPPER, reg_gruc_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRUC_UPPER, COUNT_U8, reg_gruc_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grppp_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_grppp = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grppp = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRPPP, COUNT, reg_grppp, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRPPP, reg_grppp);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grppp_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_grppp;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRPPP, reg_grppp);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRPPP, COUNT, reg_grppp);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grppp_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_grppp_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grppp_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRPPP_UPPER, COUNT_U8, reg_grppp_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRPPP_UPPER, reg_grppp_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grppp_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_grppp_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRPPP_UPPER, reg_grppp_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRPPP_UPPER, COUNT_U8, reg_grppp_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grcrc_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_grcrc = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grcrc = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRCRC, COUNT, reg_grcrc, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRCRC, reg_grcrc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grcrc_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_grcrc;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRCRC, reg_grcrc);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRCRC, COUNT, reg_grcrc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grcrc_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_grcrc_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_grcrc_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GRCRC_UPPER, COUNT_U8, reg_grcrc_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GRCRC_UPPER, reg_grcrc_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_grcrc_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_grcrc_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GRCRC_UPPER, reg_grcrc_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GRCRC_UPPER, COUNT_U8, reg_grcrc_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tr64_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_tr64 = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tr64 = RU_FIELD_SET(xumac_id, XUMAC_RDP, TR64, COUNT, reg_tr64, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, TR64, reg_tr64);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tr64_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_tr64;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, TR64, reg_tr64);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, TR64, COUNT, reg_tr64);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tr64_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_tr64_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tr64_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, TR64_UPPER, COUNT_U8, reg_tr64_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, TR64_UPPER, reg_tr64_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tr64_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_tr64_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, TR64_UPPER, reg_tr64_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, TR64_UPPER, COUNT_U8, reg_tr64_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tr127_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_tr127 = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tr127 = RU_FIELD_SET(xumac_id, XUMAC_RDP, TR127, COUNT, reg_tr127, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, TR127, reg_tr127);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tr127_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_tr127;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, TR127, reg_tr127);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, TR127, COUNT, reg_tr127);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tr127_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_tr127_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tr127_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, TR127_UPPER, COUNT_U8, reg_tr127_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, TR127_UPPER, reg_tr127_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tr127_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_tr127_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, TR127_UPPER, reg_tr127_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, TR127_UPPER, COUNT_U8, reg_tr127_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tr255_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_tr255 = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tr255 = RU_FIELD_SET(xumac_id, XUMAC_RDP, TR255, COUNT, reg_tr255, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, TR255, reg_tr255);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tr255_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_tr255;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, TR255, reg_tr255);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, TR255, COUNT, reg_tr255);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tr255_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_tr255_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tr255_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, TR255_UPPER, COUNT_U8, reg_tr255_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, TR255_UPPER, reg_tr255_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tr255_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_tr255_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, TR255_UPPER, reg_tr255_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, TR255_UPPER, COUNT_U8, reg_tr255_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tr511_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_tr511 = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tr511 = RU_FIELD_SET(xumac_id, XUMAC_RDP, TR511, COUNT, reg_tr511, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, TR511, reg_tr511);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tr511_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_tr511;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, TR511, reg_tr511);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, TR511, COUNT, reg_tr511);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tr511_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_tr511_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tr511_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, TR511_UPPER, COUNT_U8, reg_tr511_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, TR511_UPPER, reg_tr511_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tr511_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_tr511_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, TR511_UPPER, reg_tr511_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, TR511_UPPER, COUNT_U8, reg_tr511_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tr1023_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_tr1023 = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tr1023 = RU_FIELD_SET(xumac_id, XUMAC_RDP, TR1023, COUNT, reg_tr1023, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, TR1023, reg_tr1023);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tr1023_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_tr1023;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, TR1023, reg_tr1023);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, TR1023, COUNT, reg_tr1023);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tr1023_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_tr1023_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tr1023_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, TR1023_UPPER, COUNT_U8, reg_tr1023_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, TR1023_UPPER, reg_tr1023_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tr1023_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_tr1023_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, TR1023_UPPER, reg_tr1023_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, TR1023_UPPER, COUNT_U8, reg_tr1023_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tr1518_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_tr1518 = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tr1518 = RU_FIELD_SET(xumac_id, XUMAC_RDP, TR1518, COUNT, reg_tr1518, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, TR1518, reg_tr1518);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tr1518_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_tr1518;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, TR1518, reg_tr1518);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, TR1518, COUNT, reg_tr1518);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tr1518_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_tr1518_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tr1518_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, TR1518_UPPER, COUNT_U8, reg_tr1518_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, TR1518_UPPER, reg_tr1518_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tr1518_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_tr1518_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, TR1518_UPPER, reg_tr1518_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, TR1518_UPPER, COUNT_U8, reg_tr1518_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_trmgv_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_trmgv = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_trmgv = RU_FIELD_SET(xumac_id, XUMAC_RDP, TRMGV, COUNT, reg_trmgv, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, TRMGV, reg_trmgv);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_trmgv_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_trmgv;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, TRMGV, reg_trmgv);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, TRMGV, COUNT, reg_trmgv);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_trmgv_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_trmgv_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_trmgv_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, TRMGV_UPPER, COUNT_U8, reg_trmgv_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, TRMGV_UPPER, reg_trmgv_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_trmgv_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_trmgv_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, TRMGV_UPPER, reg_trmgv_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, TRMGV_UPPER, COUNT_U8, reg_trmgv_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tr2047_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_tr2047 = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tr2047 = RU_FIELD_SET(xumac_id, XUMAC_RDP, TR2047, COUNT, reg_tr2047, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, TR2047, reg_tr2047);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tr2047_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_tr2047;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, TR2047, reg_tr2047);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, TR2047, COUNT, reg_tr2047);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tr2047_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_tr2047_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tr2047_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, TR2047_UPPER, COUNT_U8, reg_tr2047_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, TR2047_UPPER, reg_tr2047_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tr2047_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_tr2047_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, TR2047_UPPER, reg_tr2047_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, TR2047_UPPER, COUNT_U8, reg_tr2047_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tr4095_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_tr4095 = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tr4095 = RU_FIELD_SET(xumac_id, XUMAC_RDP, TR4095, COUNT, reg_tr4095, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, TR4095, reg_tr4095);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tr4095_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_tr4095;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, TR4095, reg_tr4095);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, TR4095, COUNT, reg_tr4095);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tr4095_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_tr4095_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tr4095_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, TR4095_UPPER, COUNT_U8, reg_tr4095_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, TR4095_UPPER, reg_tr4095_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tr4095_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_tr4095_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, TR4095_UPPER, reg_tr4095_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, TR4095_UPPER, COUNT_U8, reg_tr4095_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tr9216_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_tr9216 = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tr9216 = RU_FIELD_SET(xumac_id, XUMAC_RDP, TR9216, COUNT, reg_tr9216, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, TR9216, reg_tr9216);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tr9216_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_tr9216;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, TR9216, reg_tr9216);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, TR9216, COUNT, reg_tr9216);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tr9216_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_tr9216_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tr9216_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, TR9216_UPPER, COUNT_U8, reg_tr9216_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, TR9216_UPPER, reg_tr9216_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tr9216_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_tr9216_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, TR9216_UPPER, reg_tr9216_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, TR9216_UPPER, COUNT_U8, reg_tr9216_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtpkt_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_gtpkt = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtpkt = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTPKT, COUNT, reg_gtpkt, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTPKT, reg_gtpkt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtpkt_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_gtpkt;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTPKT, reg_gtpkt);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTPKT, COUNT, reg_gtpkt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtpkt_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_gtpkt_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtpkt_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTPKT_UPPER, COUNT_U8, reg_gtpkt_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTPKT_UPPER, reg_gtpkt_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtpkt_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_gtpkt_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTPKT_UPPER, reg_gtpkt_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTPKT_UPPER, COUNT_U8, reg_gtpkt_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtmca_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_gtmca = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtmca = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTMCA, COUNT, reg_gtmca, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTMCA, reg_gtmca);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtmca_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_gtmca;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTMCA, reg_gtmca);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTMCA, COUNT, reg_gtmca);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtmca_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_gtmca_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtmca_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTMCA_UPPER, COUNT_U8, reg_gtmca_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTMCA_UPPER, reg_gtmca_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtmca_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_gtmca_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTMCA_UPPER, reg_gtmca_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTMCA_UPPER, COUNT_U8, reg_gtmca_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtbca_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_gtbca = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtbca = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTBCA, COUNT, reg_gtbca, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTBCA, reg_gtbca);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtbca_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_gtbca;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTBCA, reg_gtbca);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTBCA, COUNT, reg_gtbca);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtbca_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_gtbca_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtbca_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTBCA_UPPER, COUNT_U8, reg_gtbca_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTBCA_UPPER, reg_gtbca_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtbca_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_gtbca_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTBCA_UPPER, reg_gtbca_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTBCA_UPPER, COUNT_U8, reg_gtbca_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtxpf_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_gtxpf = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtxpf = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTXPF, COUNT, reg_gtxpf, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTXPF, reg_gtxpf);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtxpf_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_gtxpf;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTXPF, reg_gtxpf);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTXPF, COUNT, reg_gtxpf);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtxpf_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_gtxpf_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtxpf_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTXPF_UPPER, COUNT_U8, reg_gtxpf_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTXPF_UPPER, reg_gtxpf_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtxpf_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_gtxpf_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTXPF_UPPER, reg_gtxpf_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTXPF_UPPER, COUNT_U8, reg_gtxpf_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtxcf_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_gtxcf = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtxcf = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTXCF, COUNT, reg_gtxcf, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTXCF, reg_gtxcf);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtxcf_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_gtxcf;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTXCF, reg_gtxcf);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTXCF, COUNT, reg_gtxcf);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtxcf_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_gtxcf_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtxcf_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTXCF_UPPER, COUNT_U8, reg_gtxcf_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTXCF_UPPER, reg_gtxcf_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtxcf_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_gtxcf_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTXCF_UPPER, reg_gtxcf_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTXCF_UPPER, COUNT_U8, reg_gtxcf_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtfcs_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_gtfcs = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtfcs = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTFCS, COUNT, reg_gtfcs, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTFCS, reg_gtfcs);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtfcs_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_gtfcs;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTFCS, reg_gtfcs);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTFCS, COUNT, reg_gtfcs);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtfcs_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_gtfcs_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtfcs_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTFCS_UPPER, COUNT_U8, reg_gtfcs_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTFCS_UPPER, reg_gtfcs_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtfcs_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_gtfcs_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTFCS_UPPER, reg_gtfcs_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTFCS_UPPER, COUNT_U8, reg_gtfcs_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtovr_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_gtovr = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtovr = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTOVR, COUNT, reg_gtovr, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTOVR, reg_gtovr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtovr_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_gtovr;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTOVR, reg_gtovr);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTOVR, COUNT, reg_gtovr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtovr_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_gtovr_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtovr_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTOVR_UPPER, COUNT_U8, reg_gtovr_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTOVR_UPPER, reg_gtovr_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtovr_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_gtovr_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTOVR_UPPER, reg_gtovr_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTOVR_UPPER, COUNT_U8, reg_gtovr_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtdrf_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_gtdrf = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtdrf = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTDRF, COUNT, reg_gtdrf, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTDRF, reg_gtdrf);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtdrf_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_gtdrf;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTDRF, reg_gtdrf);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTDRF, COUNT, reg_gtdrf);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtdrf_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_gtdrf_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtdrf_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTDRF_UPPER, COUNT_U8, reg_gtdrf_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTDRF_UPPER, reg_gtdrf_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtdrf_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_gtdrf_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTDRF_UPPER, reg_gtdrf_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTDRF_UPPER, COUNT_U8, reg_gtdrf_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtedf_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_gtedf = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtedf = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTEDF, COUNT, reg_gtedf, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTEDF, reg_gtedf);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtedf_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_gtedf;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTEDF, reg_gtedf);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTEDF, COUNT, reg_gtedf);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtedf_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_gtedf_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtedf_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTEDF_UPPER, COUNT_U8, reg_gtedf_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTEDF_UPPER, reg_gtedf_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtedf_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_gtedf_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTEDF_UPPER, reg_gtedf_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTEDF_UPPER, COUNT_U8, reg_gtedf_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtscl_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_gtscl = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtscl = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTSCL, COUNT, reg_gtscl, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTSCL, reg_gtscl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtscl_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_gtscl;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTSCL, reg_gtscl);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTSCL, COUNT, reg_gtscl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtscl_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_gtscl_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtscl_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTSCL_UPPER, COUNT_U8, reg_gtscl_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTSCL_UPPER, reg_gtscl_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtscl_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_gtscl_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTSCL_UPPER, reg_gtscl_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTSCL_UPPER, COUNT_U8, reg_gtscl_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtmcl_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_gtmcl = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtmcl = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTMCL, COUNT, reg_gtmcl, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTMCL, reg_gtmcl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtmcl_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_gtmcl;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTMCL, reg_gtmcl);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTMCL, COUNT, reg_gtmcl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtmcl_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_gtmcl_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtmcl_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTMCL_UPPER, COUNT_U8, reg_gtmcl_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTMCL_UPPER, reg_gtmcl_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtmcl_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_gtmcl_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTMCL_UPPER, reg_gtmcl_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTMCL_UPPER, COUNT_U8, reg_gtmcl_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtlcl_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_gtlcl = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtlcl = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTLCL, COUNT, reg_gtlcl, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTLCL, reg_gtlcl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtlcl_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_gtlcl;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTLCL, reg_gtlcl);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTLCL, COUNT, reg_gtlcl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtlcl_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_gtlcl_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtlcl_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTLCL_UPPER, COUNT_U8, reg_gtlcl_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTLCL_UPPER, reg_gtlcl_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtlcl_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_gtlcl_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTLCL_UPPER, reg_gtlcl_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTLCL_UPPER, COUNT_U8, reg_gtlcl_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtxcl_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_gtxcl = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtxcl = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTXCL, COUNT, reg_gtxcl, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTXCL, reg_gtxcl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtxcl_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_gtxcl;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTXCL, reg_gtxcl);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTXCL, COUNT, reg_gtxcl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtxcl_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_gtxcl_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtxcl_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTXCL_UPPER, COUNT_U8, reg_gtxcl_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTXCL_UPPER, reg_gtxcl_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtxcl_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_gtxcl_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTXCL_UPPER, reg_gtxcl_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTXCL_UPPER, COUNT_U8, reg_gtxcl_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtfrg_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_gtfrg = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtfrg = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTFRG, COUNT, reg_gtfrg, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTFRG, reg_gtfrg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtfrg_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_gtfrg;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTFRG, reg_gtfrg);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTFRG, COUNT, reg_gtfrg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtfrg_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_gtfrg_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtfrg_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTFRG_UPPER, COUNT_U8, reg_gtfrg_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTFRG_UPPER, reg_gtfrg_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtfrg_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_gtfrg_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTFRG_UPPER, reg_gtfrg_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTFRG_UPPER, COUNT_U8, reg_gtfrg_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtncl_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_gtncl = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtncl = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTNCL, COUNT, reg_gtncl, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTNCL, reg_gtncl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtncl_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_gtncl;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTNCL, reg_gtncl);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTNCL, COUNT, reg_gtncl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtncl_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_gtncl_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtncl_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTNCL_UPPER, COUNT_U8, reg_gtncl_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTNCL_UPPER, reg_gtncl_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtncl_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_gtncl_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTNCL_UPPER, reg_gtncl_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTNCL_UPPER, COUNT_U8, reg_gtncl_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtjbr_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_gtjbr = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtjbr = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTJBR, COUNT, reg_gtjbr, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTJBR, reg_gtjbr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtjbr_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_gtjbr;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTJBR, reg_gtjbr);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTJBR, COUNT, reg_gtjbr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtjbr_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_gtjbr_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtjbr_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTJBR_UPPER, COUNT_U8, reg_gtjbr_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTJBR_UPPER, reg_gtjbr_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtjbr_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_gtjbr_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTJBR_UPPER, reg_gtjbr_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTJBR_UPPER, COUNT_U8, reg_gtjbr_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtbyt_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_gtbyt = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtbyt = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTBYT, COUNT, reg_gtbyt, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTBYT, reg_gtbyt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtbyt_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_gtbyt;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTBYT, reg_gtbyt);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTBYT, COUNT, reg_gtbyt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtbyt_upper_set(uint8_t xumac_id, uint16_t count_u16)
{
    uint32_t reg_gtbyt_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtbyt_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTBYT_UPPER, COUNT_U16, reg_gtbyt_upper, count_u16);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTBYT_UPPER, reg_gtbyt_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtbyt_upper_get(uint8_t xumac_id, uint16_t *count_u16)
{
    uint32_t reg_gtbyt_upper;

#ifdef VALIDATE_PARMS
    if (!count_u16)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTBYT_UPPER, reg_gtbyt_upper);

    *count_u16 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTBYT_UPPER, COUNT_U16, reg_gtbyt_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtpok_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_gtpok = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtpok = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTPOK, COUNT, reg_gtpok, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTPOK, reg_gtpok);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtpok_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_gtpok;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTPOK, reg_gtpok);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTPOK, COUNT, reg_gtpok);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtpok_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_gtpok_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtpok_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTPOK_UPPER, COUNT_U8, reg_gtpok_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTPOK_UPPER, reg_gtpok_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtpok_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_gtpok_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTPOK_UPPER, reg_gtpok_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTPOK_UPPER, COUNT_U8, reg_gtpok_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtuc_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_gtuc = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtuc = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTUC, COUNT, reg_gtuc, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTUC, reg_gtuc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtuc_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_gtuc;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTUC, reg_gtuc);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTUC, COUNT, reg_gtuc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtuc_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_gtuc_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gtuc_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, GTUC_UPPER, COUNT_U8, reg_gtuc_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GTUC_UPPER, reg_gtuc_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gtuc_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_gtuc_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GTUC_UPPER, reg_gtuc_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, GTUC_UPPER, COUNT_U8, reg_gtuc_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_rrpkt_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_rrpkt = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rrpkt = RU_FIELD_SET(xumac_id, XUMAC_RDP, RRPKT, COUNT, reg_rrpkt, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, RRPKT, reg_rrpkt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_rrpkt_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_rrpkt;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, RRPKT, reg_rrpkt);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, RRPKT, COUNT, reg_rrpkt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_rrpkt_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_rrpkt_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rrpkt_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, RRPKT_UPPER, COUNT_U8, reg_rrpkt_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, RRPKT_UPPER, reg_rrpkt_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_rrpkt_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_rrpkt_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, RRPKT_UPPER, reg_rrpkt_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, RRPKT_UPPER, COUNT_U8, reg_rrpkt_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_rrund_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_rrund = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rrund = RU_FIELD_SET(xumac_id, XUMAC_RDP, RRUND, COUNT, reg_rrund, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, RRUND, reg_rrund);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_rrund_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_rrund;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, RRUND, reg_rrund);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, RRUND, COUNT, reg_rrund);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_rrund_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_rrund_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rrund_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, RRUND_UPPER, COUNT_U8, reg_rrund_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, RRUND_UPPER, reg_rrund_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_rrund_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_rrund_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, RRUND_UPPER, reg_rrund_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, RRUND_UPPER, COUNT_U8, reg_rrund_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_rrfrg_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_rrfrg = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rrfrg = RU_FIELD_SET(xumac_id, XUMAC_RDP, RRFRG, COUNT, reg_rrfrg, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, RRFRG, reg_rrfrg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_rrfrg_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_rrfrg;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, RRFRG, reg_rrfrg);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, RRFRG, COUNT, reg_rrfrg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_rrfrg_upper_set(uint8_t xumac_id, uint8_t count_u8)
{
    uint32_t reg_rrfrg_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rrfrg_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, RRFRG_UPPER, COUNT_U8, reg_rrfrg_upper, count_u8);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, RRFRG_UPPER, reg_rrfrg_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_rrfrg_upper_get(uint8_t xumac_id, uint8_t *count_u8)
{
    uint32_t reg_rrfrg_upper;

#ifdef VALIDATE_PARMS
    if (!count_u8)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, RRFRG_UPPER, reg_rrfrg_upper);

    *count_u8 = RU_FIELD_GET(xumac_id, XUMAC_RDP, RRFRG_UPPER, COUNT_U8, reg_rrfrg_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_rrbyt_set(uint8_t xumac_id, uint32_t count)
{
    uint32_t reg_rrbyt = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rrbyt = RU_FIELD_SET(xumac_id, XUMAC_RDP, RRBYT, COUNT, reg_rrbyt, count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, RRBYT, reg_rrbyt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_rrbyt_get(uint8_t xumac_id, uint32_t *count)
{
    uint32_t reg_rrbyt;

#ifdef VALIDATE_PARMS
    if (!count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, RRBYT, reg_rrbyt);

    *count = RU_FIELD_GET(xumac_id, XUMAC_RDP, RRBYT, COUNT, reg_rrbyt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_rrbyt_upper_set(uint8_t xumac_id, uint16_t count_u16)
{
    uint32_t reg_rrbyt_upper = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rrbyt_upper = RU_FIELD_SET(xumac_id, XUMAC_RDP, RRBYT_UPPER, COUNT_U16, reg_rrbyt_upper, count_u16);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, RRBYT_UPPER, reg_rrbyt_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_rrbyt_upper_get(uint8_t xumac_id, uint16_t *count_u16)
{
    uint32_t reg_rrbyt_upper;

#ifdef VALIDATE_PARMS
    if (!count_u16)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, RRBYT_UPPER, reg_rrbyt_upper);

    *count_u16 = RU_FIELD_GET(xumac_id, XUMAC_RDP, RRBYT_UPPER, COUNT_U16, reg_rrbyt_upper);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_mib_cntrl_set(uint8_t xumac_id, bdmf_boolean rx_cnt_rst, bdmf_boolean runt_cnt_rst, bdmf_boolean tx_cnt_rst)
{
    uint32_t reg_mib_cntrl = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT) ||
       (rx_cnt_rst >= _1BITS_MAX_VAL_) ||
       (runt_cnt_rst >= _1BITS_MAX_VAL_) ||
       (tx_cnt_rst >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_mib_cntrl = RU_FIELD_SET(xumac_id, XUMAC_RDP, MIB_CNTRL, RX_CNT_RST, reg_mib_cntrl, rx_cnt_rst);
    reg_mib_cntrl = RU_FIELD_SET(xumac_id, XUMAC_RDP, MIB_CNTRL, RUNT_CNT_RST, reg_mib_cntrl, runt_cnt_rst);
    reg_mib_cntrl = RU_FIELD_SET(xumac_id, XUMAC_RDP, MIB_CNTRL, TX_CNT_RST, reg_mib_cntrl, tx_cnt_rst);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, MIB_CNTRL, reg_mib_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_mib_cntrl_get(uint8_t xumac_id, bdmf_boolean *rx_cnt_rst, bdmf_boolean *runt_cnt_rst, bdmf_boolean *tx_cnt_rst)
{
    uint32_t reg_mib_cntrl;

#ifdef VALIDATE_PARMS
    if (!rx_cnt_rst || !runt_cnt_rst || !tx_cnt_rst)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, MIB_CNTRL, reg_mib_cntrl);

    *rx_cnt_rst = RU_FIELD_GET(xumac_id, XUMAC_RDP, MIB_CNTRL, RX_CNT_RST, reg_mib_cntrl);
    *runt_cnt_rst = RU_FIELD_GET(xumac_id, XUMAC_RDP, MIB_CNTRL, RUNT_CNT_RST, reg_mib_cntrl);
    *tx_cnt_rst = RU_FIELD_GET(xumac_id, XUMAC_RDP, MIB_CNTRL, TX_CNT_RST, reg_mib_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_mib_read_data_set(uint8_t xumac_id, uint32_t data32)
{
    uint32_t reg_mib_read_data = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_mib_read_data = RU_FIELD_SET(xumac_id, XUMAC_RDP, MIB_READ_DATA, DATA32, reg_mib_read_data, data32);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, MIB_READ_DATA, reg_mib_read_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_mib_read_data_get(uint8_t xumac_id, uint32_t *data32)
{
    uint32_t reg_mib_read_data;

#ifdef VALIDATE_PARMS
    if (!data32)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, MIB_READ_DATA, reg_mib_read_data);

    *data32 = RU_FIELD_GET(xumac_id, XUMAC_RDP, MIB_READ_DATA, DATA32, reg_mib_read_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_mib_write_data_set(uint8_t xumac_id, uint32_t data32)
{
    uint32_t reg_mib_write_data = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_mib_write_data = RU_FIELD_SET(xumac_id, XUMAC_RDP, MIB_WRITE_DATA, DATA32, reg_mib_write_data, data32);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, MIB_WRITE_DATA, reg_mib_write_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_mib_write_data_get(uint8_t xumac_id, uint32_t *data32)
{
    uint32_t reg_mib_write_data;

#ifdef VALIDATE_PARMS
    if (!data32)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, MIB_WRITE_DATA, reg_mib_write_data);

    *data32 = RU_FIELD_GET(xumac_id, XUMAC_RDP, MIB_WRITE_DATA, DATA32, reg_mib_write_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_control_set(uint8_t xumac_id, bdmf_boolean mpd_en, uint8_t mseq_len, bdmf_boolean psw_en)
{
    uint32_t reg_control = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT) ||
       (mpd_en >= _1BITS_MAX_VAL_) ||
       (psw_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_control = RU_FIELD_SET(xumac_id, XUMAC_RDP, CONTROL, MPD_EN, reg_control, mpd_en);
    reg_control = RU_FIELD_SET(xumac_id, XUMAC_RDP, CONTROL, MSEQ_LEN, reg_control, mseq_len);
    reg_control = RU_FIELD_SET(xumac_id, XUMAC_RDP, CONTROL, PSW_EN, reg_control, psw_en);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, CONTROL, reg_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_control_get(uint8_t xumac_id, bdmf_boolean *mpd_en, uint8_t *mseq_len, bdmf_boolean *psw_en)
{
    uint32_t reg_control;

#ifdef VALIDATE_PARMS
    if (!mpd_en || !mseq_len || !psw_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, CONTROL, reg_control);

    *mpd_en = RU_FIELD_GET(xumac_id, XUMAC_RDP, CONTROL, MPD_EN, reg_control);
    *mseq_len = RU_FIELD_GET(xumac_id, XUMAC_RDP, CONTROL, MSEQ_LEN, reg_control);
    *psw_en = RU_FIELD_GET(xumac_id, XUMAC_RDP, CONTROL, PSW_EN, reg_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_psw_ms_set(uint8_t xumac_id, uint16_t psw_47_32)
{
    uint32_t reg_psw_ms = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_psw_ms = RU_FIELD_SET(xumac_id, XUMAC_RDP, PSW_MS, PSW_47_32, reg_psw_ms, psw_47_32);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, PSW_MS, reg_psw_ms);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_psw_ms_get(uint8_t xumac_id, uint16_t *psw_47_32)
{
    uint32_t reg_psw_ms;

#ifdef VALIDATE_PARMS
    if (!psw_47_32)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, PSW_MS, reg_psw_ms);

    *psw_47_32 = RU_FIELD_GET(xumac_id, XUMAC_RDP, PSW_MS, PSW_47_32, reg_psw_ms);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_psw_ls_set(uint8_t xumac_id, uint32_t psw_31_0)
{
    uint32_t reg_psw_ls = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_psw_ls = RU_FIELD_SET(xumac_id, XUMAC_RDP, PSW_LS, PSW_31_0, reg_psw_ls, psw_31_0);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, PSW_LS, reg_psw_ls);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_psw_ls_get(uint8_t xumac_id, uint32_t *psw_31_0)
{
    uint32_t reg_psw_ls;

#ifdef VALIDATE_PARMS
    if (!psw_31_0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, PSW_LS, reg_psw_ls);

    *psw_31_0 = RU_FIELD_GET(xumac_id, XUMAC_RDP, PSW_LS, PSW_31_0, reg_psw_ls);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_control_1_set(uint8_t xumac_id, const xumac_rdp_control_1 *control_1)
{
    uint32_t reg_control_1 = 0;

#ifdef VALIDATE_PARMS
    if(!control_1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT) ||
       (control_1->xib_rx_en >= _1BITS_MAX_VAL_) ||
       (control_1->xib_tx_en >= _1BITS_MAX_VAL_) ||
       (control_1->rx_flush_en >= _1BITS_MAX_VAL_) ||
       (control_1->tx_flush_en >= _1BITS_MAX_VAL_) ||
       (control_1->link_down_rst_en >= _1BITS_MAX_VAL_) ||
       (control_1->standard_mux_en >= _1BITS_MAX_VAL_) ||
       (control_1->xgmii_sel >= _1BITS_MAX_VAL_) ||
       (control_1->dic_dis >= _1BITS_MAX_VAL_) ||
       (control_1->rx_start_threshold >= _9BITS_MAX_VAL_) ||
       (control_1->gmii_rx_clk_gate_en >= _1BITS_MAX_VAL_) ||
       (control_1->strict_preamble_dis >= _1BITS_MAX_VAL_) ||
       (control_1->tx_ipg >= _5BITS_MAX_VAL_) ||
       (control_1->min_rx_ipg >= _5BITS_MAX_VAL_) ||
       (control_1->xgmii_sel_ovrd >= _1BITS_MAX_VAL_) ||
       (control_1->gmii_tx_clk_gate_en >= _1BITS_MAX_VAL_) ||
       (control_1->autoconfig_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_control_1 = RU_FIELD_SET(xumac_id, XUMAC_RDP, CONTROL_1, XIB_RX_EN, reg_control_1, control_1->xib_rx_en);
    reg_control_1 = RU_FIELD_SET(xumac_id, XUMAC_RDP, CONTROL_1, XIB_TX_EN, reg_control_1, control_1->xib_tx_en);
    reg_control_1 = RU_FIELD_SET(xumac_id, XUMAC_RDP, CONTROL_1, RX_FLUSH_EN, reg_control_1, control_1->rx_flush_en);
    reg_control_1 = RU_FIELD_SET(xumac_id, XUMAC_RDP, CONTROL_1, TX_FLUSH_EN, reg_control_1, control_1->tx_flush_en);
    reg_control_1 = RU_FIELD_SET(xumac_id, XUMAC_RDP, CONTROL_1, LINK_DOWN_RST_EN, reg_control_1, control_1->link_down_rst_en);
    reg_control_1 = RU_FIELD_SET(xumac_id, XUMAC_RDP, CONTROL_1, STANDARD_MUX_EN, reg_control_1, control_1->standard_mux_en);
    reg_control_1 = RU_FIELD_SET(xumac_id, XUMAC_RDP, CONTROL_1, XGMII_SEL, reg_control_1, control_1->xgmii_sel);
    reg_control_1 = RU_FIELD_SET(xumac_id, XUMAC_RDP, CONTROL_1, DIC_DIS, reg_control_1, control_1->dic_dis);
    reg_control_1 = RU_FIELD_SET(xumac_id, XUMAC_RDP, CONTROL_1, RX_START_THRESHOLD, reg_control_1, control_1->rx_start_threshold);
    reg_control_1 = RU_FIELD_SET(xumac_id, XUMAC_RDP, CONTROL_1, GMII_RX_CLK_GATE_EN, reg_control_1, control_1->gmii_rx_clk_gate_en);
    reg_control_1 = RU_FIELD_SET(xumac_id, XUMAC_RDP, CONTROL_1, STRICT_PREAMBLE_DIS, reg_control_1, control_1->strict_preamble_dis);
    reg_control_1 = RU_FIELD_SET(xumac_id, XUMAC_RDP, CONTROL_1, TX_IPG, reg_control_1, control_1->tx_ipg);
    reg_control_1 = RU_FIELD_SET(xumac_id, XUMAC_RDP, CONTROL_1, MIN_RX_IPG, reg_control_1, control_1->min_rx_ipg);
    reg_control_1 = RU_FIELD_SET(xumac_id, XUMAC_RDP, CONTROL_1, XGMII_SEL_OVRD, reg_control_1, control_1->xgmii_sel_ovrd);
    reg_control_1 = RU_FIELD_SET(xumac_id, XUMAC_RDP, CONTROL_1, GMII_TX_CLK_GATE_EN, reg_control_1, control_1->gmii_tx_clk_gate_en);
    reg_control_1 = RU_FIELD_SET(xumac_id, XUMAC_RDP, CONTROL_1, AUTOCONFIG_EN, reg_control_1, control_1->autoconfig_en);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, CONTROL_1, reg_control_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_control_1_get(uint8_t xumac_id, xumac_rdp_control_1 *control_1)
{
    uint32_t reg_control_1;

#ifdef VALIDATE_PARMS
    if (!control_1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, CONTROL_1, reg_control_1);

    control_1->xib_rx_en = RU_FIELD_GET(xumac_id, XUMAC_RDP, CONTROL_1, XIB_RX_EN, reg_control_1);
    control_1->xib_tx_en = RU_FIELD_GET(xumac_id, XUMAC_RDP, CONTROL_1, XIB_TX_EN, reg_control_1);
    control_1->rx_flush_en = RU_FIELD_GET(xumac_id, XUMAC_RDP, CONTROL_1, RX_FLUSH_EN, reg_control_1);
    control_1->tx_flush_en = RU_FIELD_GET(xumac_id, XUMAC_RDP, CONTROL_1, TX_FLUSH_EN, reg_control_1);
    control_1->link_down_rst_en = RU_FIELD_GET(xumac_id, XUMAC_RDP, CONTROL_1, LINK_DOWN_RST_EN, reg_control_1);
    control_1->standard_mux_en = RU_FIELD_GET(xumac_id, XUMAC_RDP, CONTROL_1, STANDARD_MUX_EN, reg_control_1);
    control_1->xgmii_sel = RU_FIELD_GET(xumac_id, XUMAC_RDP, CONTROL_1, XGMII_SEL, reg_control_1);
    control_1->dic_dis = RU_FIELD_GET(xumac_id, XUMAC_RDP, CONTROL_1, DIC_DIS, reg_control_1);
    control_1->rx_start_threshold = RU_FIELD_GET(xumac_id, XUMAC_RDP, CONTROL_1, RX_START_THRESHOLD, reg_control_1);
    control_1->gmii_rx_clk_gate_en = RU_FIELD_GET(xumac_id, XUMAC_RDP, CONTROL_1, GMII_RX_CLK_GATE_EN, reg_control_1);
    control_1->strict_preamble_dis = RU_FIELD_GET(xumac_id, XUMAC_RDP, CONTROL_1, STRICT_PREAMBLE_DIS, reg_control_1);
    control_1->tx_ipg = RU_FIELD_GET(xumac_id, XUMAC_RDP, CONTROL_1, TX_IPG, reg_control_1);
    control_1->min_rx_ipg = RU_FIELD_GET(xumac_id, XUMAC_RDP, CONTROL_1, MIN_RX_IPG, reg_control_1);
    control_1->xgmii_sel_ovrd = RU_FIELD_GET(xumac_id, XUMAC_RDP, CONTROL_1, XGMII_SEL_OVRD, reg_control_1);
    control_1->gmii_tx_clk_gate_en = RU_FIELD_GET(xumac_id, XUMAC_RDP, CONTROL_1, GMII_TX_CLK_GATE_EN, reg_control_1);
    control_1->autoconfig_en = RU_FIELD_GET(xumac_id, XUMAC_RDP, CONTROL_1, AUTOCONFIG_EN, reg_control_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_extended_control_set(uint8_t xumac_id, uint16_t tx_start_threshold, uint16_t tx_xoff_threshold, uint16_t tx_xon_threshold, bdmf_boolean tx_backpressure_en)
{
    uint32_t reg_extended_control = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT) ||
       (tx_start_threshold >= _9BITS_MAX_VAL_) ||
       (tx_xoff_threshold >= _9BITS_MAX_VAL_) ||
       (tx_xon_threshold >= _9BITS_MAX_VAL_) ||
       (tx_backpressure_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_extended_control = RU_FIELD_SET(xumac_id, XUMAC_RDP, EXTENDED_CONTROL, TX_START_THRESHOLD, reg_extended_control, tx_start_threshold);
    reg_extended_control = RU_FIELD_SET(xumac_id, XUMAC_RDP, EXTENDED_CONTROL, TX_XOFF_THRESHOLD, reg_extended_control, tx_xoff_threshold);
    reg_extended_control = RU_FIELD_SET(xumac_id, XUMAC_RDP, EXTENDED_CONTROL, TX_XON_THRESHOLD, reg_extended_control, tx_xon_threshold);
    reg_extended_control = RU_FIELD_SET(xumac_id, XUMAC_RDP, EXTENDED_CONTROL, TX_BACKPRESSURE_EN, reg_extended_control, tx_backpressure_en);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, EXTENDED_CONTROL, reg_extended_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_extended_control_get(uint8_t xumac_id, uint16_t *tx_start_threshold, uint16_t *tx_xoff_threshold, uint16_t *tx_xon_threshold, bdmf_boolean *tx_backpressure_en)
{
    uint32_t reg_extended_control;

#ifdef VALIDATE_PARMS
    if (!tx_start_threshold || !tx_xoff_threshold || !tx_xon_threshold || !tx_backpressure_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, EXTENDED_CONTROL, reg_extended_control);

    *tx_start_threshold = RU_FIELD_GET(xumac_id, XUMAC_RDP, EXTENDED_CONTROL, TX_START_THRESHOLD, reg_extended_control);
    *tx_xoff_threshold = RU_FIELD_GET(xumac_id, XUMAC_RDP, EXTENDED_CONTROL, TX_XOFF_THRESHOLD, reg_extended_control);
    *tx_xon_threshold = RU_FIELD_GET(xumac_id, XUMAC_RDP, EXTENDED_CONTROL, TX_XON_THRESHOLD, reg_extended_control);
    *tx_backpressure_en = RU_FIELD_GET(xumac_id, XUMAC_RDP, EXTENDED_CONTROL, TX_BACKPRESSURE_EN, reg_extended_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tx_idle_stuffing_control_set(uint8_t xumac_id, uint8_t tx_idle_stuffing_ctrl)
{
    uint32_t reg_tx_idle_stuffing_control = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT) ||
       (tx_idle_stuffing_ctrl >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tx_idle_stuffing_control = RU_FIELD_SET(xumac_id, XUMAC_RDP, TX_IDLE_STUFFING_CONTROL, TX_IDLE_STUFFING_CTRL, reg_tx_idle_stuffing_control, tx_idle_stuffing_ctrl);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, TX_IDLE_STUFFING_CONTROL, reg_tx_idle_stuffing_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tx_idle_stuffing_control_get(uint8_t xumac_id, uint8_t *tx_idle_stuffing_ctrl)
{
    uint32_t reg_tx_idle_stuffing_control;

#ifdef VALIDATE_PARMS
    if (!tx_idle_stuffing_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, TX_IDLE_STUFFING_CONTROL, reg_tx_idle_stuffing_control);

    *tx_idle_stuffing_ctrl = RU_FIELD_GET(xumac_id, XUMAC_RDP, TX_IDLE_STUFFING_CONTROL, TX_IDLE_STUFFING_CTRL, reg_tx_idle_stuffing_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_actual_data_rate_set(uint8_t xumac_id, uint8_t actual_data_rate)
{
    uint32_t reg_actual_data_rate = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT) ||
       (actual_data_rate >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_actual_data_rate = RU_FIELD_SET(xumac_id, XUMAC_RDP, ACTUAL_DATA_RATE, ACTUAL_DATA_RATE, reg_actual_data_rate, actual_data_rate);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, ACTUAL_DATA_RATE, reg_actual_data_rate);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_actual_data_rate_get(uint8_t xumac_id, uint8_t *actual_data_rate)
{
    uint32_t reg_actual_data_rate;

#ifdef VALIDATE_PARMS
    if (!actual_data_rate)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, ACTUAL_DATA_RATE, reg_actual_data_rate);

    *actual_data_rate = RU_FIELD_GET(xumac_id, XUMAC_RDP, ACTUAL_DATA_RATE, ACTUAL_DATA_RATE, reg_actual_data_rate);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gmii_clock_swallower_control_set(uint8_t xumac_id, uint8_t ndiv, uint8_t mdiv)
{
    uint32_t reg_gmii_clock_swallower_control = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gmii_clock_swallower_control = RU_FIELD_SET(xumac_id, XUMAC_RDP, GMII_CLOCK_SWALLOWER_CONTROL, NDIV, reg_gmii_clock_swallower_control, ndiv);
    reg_gmii_clock_swallower_control = RU_FIELD_SET(xumac_id, XUMAC_RDP, GMII_CLOCK_SWALLOWER_CONTROL, MDIV, reg_gmii_clock_swallower_control, mdiv);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, GMII_CLOCK_SWALLOWER_CONTROL, reg_gmii_clock_swallower_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_gmii_clock_swallower_control_get(uint8_t xumac_id, uint8_t *ndiv, uint8_t *mdiv)
{
    uint32_t reg_gmii_clock_swallower_control;

#ifdef VALIDATE_PARMS
    if (!ndiv || !mdiv)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, GMII_CLOCK_SWALLOWER_CONTROL, reg_gmii_clock_swallower_control);

    *ndiv = RU_FIELD_GET(xumac_id, XUMAC_RDP, GMII_CLOCK_SWALLOWER_CONTROL, NDIV, reg_gmii_clock_swallower_control);
    *mdiv = RU_FIELD_GET(xumac_id, XUMAC_RDP, GMII_CLOCK_SWALLOWER_CONTROL, MDIV, reg_gmii_clock_swallower_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_xgmii_data_rate_status_get(uint8_t xumac_id, uint8_t *xgmii_data_rate)
{
    uint32_t reg_xgmii_data_rate_status;

#ifdef VALIDATE_PARMS
    if (!xgmii_data_rate)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, XGMII_DATA_RATE_STATUS, reg_xgmii_data_rate_status);

    *xgmii_data_rate = RU_FIELD_GET(xumac_id, XUMAC_RDP, XGMII_DATA_RATE_STATUS, XGMII_DATA_RATE, reg_xgmii_data_rate_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_status_get(uint8_t xumac_id, xumac_rdp_status *status)
{
    uint32_t reg_status;

#ifdef VALIDATE_PARMS
    if (!status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, STATUS, reg_status);

    status->rx_fifo_overrun = RU_FIELD_GET(xumac_id, XUMAC_RDP, STATUS, RX_FIFO_OVERRUN, reg_status);
    status->rx_fifo_underrun = RU_FIELD_GET(xumac_id, XUMAC_RDP, STATUS, RX_FIFO_UNDERRUN, reg_status);
    status->tx_fifo_underrun = RU_FIELD_GET(xumac_id, XUMAC_RDP, STATUS, TX_FIFO_UNDERRUN, reg_status);
    status->tx_fifo_overrun = RU_FIELD_GET(xumac_id, XUMAC_RDP, STATUS, TX_FIFO_OVERRUN, reg_status);
    status->rx_fault_status = RU_FIELD_GET(xumac_id, XUMAC_RDP, STATUS, RX_FAULT_STATUS, reg_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_rx_discard_packet_counter_set(uint8_t xumac_id, uint32_t pkt_count)
{
    uint32_t reg_rx_discard_packet_counter = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_discard_packet_counter = RU_FIELD_SET(xumac_id, XUMAC_RDP, RX_DISCARD_PACKET_COUNTER, PKT_COUNT, reg_rx_discard_packet_counter, pkt_count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, RX_DISCARD_PACKET_COUNTER, reg_rx_discard_packet_counter);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_rx_discard_packet_counter_get(uint8_t xumac_id, uint32_t *pkt_count)
{
    uint32_t reg_rx_discard_packet_counter;

#ifdef VALIDATE_PARMS
    if (!pkt_count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, RX_DISCARD_PACKET_COUNTER, reg_rx_discard_packet_counter);

    *pkt_count = RU_FIELD_GET(xumac_id, XUMAC_RDP, RX_DISCARD_PACKET_COUNTER, PKT_COUNT, reg_rx_discard_packet_counter);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tx_discard_packet_counter_set(uint8_t xumac_id, uint32_t pkt_count)
{
    uint32_t reg_tx_discard_packet_counter = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tx_discard_packet_counter = RU_FIELD_SET(xumac_id, XUMAC_RDP, TX_DISCARD_PACKET_COUNTER, PKT_COUNT, reg_tx_discard_packet_counter, pkt_count);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, TX_DISCARD_PACKET_COUNTER, reg_tx_discard_packet_counter);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_tx_discard_packet_counter_get(uint8_t xumac_id, uint32_t *pkt_count)
{
    uint32_t reg_tx_discard_packet_counter;

#ifdef VALIDATE_PARMS
    if (!pkt_count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, TX_DISCARD_PACKET_COUNTER, reg_tx_discard_packet_counter);

    *pkt_count = RU_FIELD_GET(xumac_id, XUMAC_RDP, TX_DISCARD_PACKET_COUNTER, PKT_COUNT, reg_tx_discard_packet_counter);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_rev_get(uint8_t xumac_id, uint16_t *sys_port_rev)
{
    uint32_t reg_rev;

#ifdef VALIDATE_PARMS
    if (!sys_port_rev)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, REV, reg_rev);

    *sys_port_rev = RU_FIELD_GET(xumac_id, XUMAC_RDP, REV, SYS_PORT_REV, reg_rev);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_umac_rxerr_mask_set(uint8_t xumac_id, uint32_t mac_rxerr_mask)
{
    uint32_t reg_umac_rxerr_mask = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT) ||
       (mac_rxerr_mask >= _18BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_umac_rxerr_mask = RU_FIELD_SET(xumac_id, XUMAC_RDP, UMAC_RXERR_MASK, MAC_RXERR_MASK, reg_umac_rxerr_mask, mac_rxerr_mask);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, UMAC_RXERR_MASK, reg_umac_rxerr_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_umac_rxerr_mask_get(uint8_t xumac_id, uint32_t *mac_rxerr_mask)
{
    uint32_t reg_umac_rxerr_mask;

#ifdef VALIDATE_PARMS
    if (!mac_rxerr_mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, UMAC_RXERR_MASK, reg_umac_rxerr_mask);

    *mac_rxerr_mask = RU_FIELD_GET(xumac_id, XUMAC_RDP, UMAC_RXERR_MASK, MAC_RXERR_MASK, reg_umac_rxerr_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_mib_max_pkt_size_set(uint8_t xumac_id, uint16_t max_pkt_size)
{
    uint32_t reg_mib_max_pkt_size = 0;

#ifdef VALIDATE_PARMS
    if ((xumac_id >= BLOCK_ADDR_COUNT) ||
       (max_pkt_size >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_mib_max_pkt_size = RU_FIELD_SET(xumac_id, XUMAC_RDP, MIB_MAX_PKT_SIZE, MAX_PKT_SIZE, reg_mib_max_pkt_size, max_pkt_size);

    RU_REG_WRITE(xumac_id, XUMAC_RDP, MIB_MAX_PKT_SIZE, reg_mib_max_pkt_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_xumac_rdp_mib_max_pkt_size_get(uint8_t xumac_id, uint16_t *max_pkt_size)
{
    uint32_t reg_mib_max_pkt_size;

#ifdef VALIDATE_PARMS
    if (!max_pkt_size)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((xumac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(xumac_id, XUMAC_RDP, MIB_MAX_PKT_SIZE, reg_mib_max_pkt_size);

    *max_pkt_size = RU_FIELD_GET(xumac_id, XUMAC_RDP, MIB_MAX_PKT_SIZE, MAX_PKT_SIZE, reg_mib_max_pkt_size);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL

typedef enum
{
    bdmf_address_ipg_hd_bkp_cntl,
    bdmf_address_command_config,
    bdmf_address_mac_0,
    bdmf_address_mac_1,
    bdmf_address_frm_length,
    bdmf_address_pause_quant,
    bdmf_address_tx_ts_seq_id,
    bdmf_address_sfd_offset,
    bdmf_address_mac_mode,
    bdmf_address_tag_0,
    bdmf_address_tag_1,
    bdmf_address_rx_pause_quanta_scale,
    bdmf_address_tx_preamble,
    bdmf_address_tx_ipg_length,
    bdmf_address_pfc_xoff_timer,
    bdmf_address_umac_eee_ctrl,
    bdmf_address_mii_eee_delay_entry_timer,
    bdmf_address_gmii_eee_delay_entry_timer,
    bdmf_address_umac_eee_ref_count,
    bdmf_address_umac_timestamp_adjust,
    bdmf_address_umac_rx_pkt_drop_status,
    bdmf_address_umac_symmetric_idle_threshold,
    bdmf_address_mii_eee_wake_timer,
    bdmf_address_gmii_eee_wake_timer,
    bdmf_address_umac_rev_id,
    bdmf_address_gmii_2p5g_eee_delay_entry_timer,
    bdmf_address_gmii_5g_eee_delay_entry_timer,
    bdmf_address_gmii_10g_eee_delay_entry_timer,
    bdmf_address_gmii_2p5g_eee_wake_timer,
    bdmf_address_gmii_5g_eee_wake_timer,
    bdmf_address_gmii_10g_eee_wake_timer,
    bdmf_address_active_eee_delay_entry_timer,
    bdmf_address_active_eee_wake_timer,
    bdmf_address_mac_pfc_type,
    bdmf_address_mac_pfc_opcode,
    bdmf_address_mac_pfc_da_0,
    bdmf_address_mac_pfc_da_1,
    bdmf_address_macsec_prog_tx_crc,
    bdmf_address_macsec_cntrl,
    bdmf_address_ts_status,
    bdmf_address_tx_ts_data,
    bdmf_address_pause_refresh_ctrl,
    bdmf_address_flush_control,
    bdmf_address_rxfifo_stat,
    bdmf_address_txfifo_stat,
    bdmf_address_mac_pfc_ctrl,
    bdmf_address_mac_pfc_refresh_ctrl,
    bdmf_address_gr64,
    bdmf_address_gr64_upper,
    bdmf_address_gr127,
    bdmf_address_gr127_upper,
    bdmf_address_gr255,
    bdmf_address_gr255_upper,
    bdmf_address_gr511,
    bdmf_address_gr511_upper,
    bdmf_address_gr1023,
    bdmf_address_gr1023_upper,
    bdmf_address_gr1518,
    bdmf_address_gr1518_upper,
    bdmf_address_grmgv,
    bdmf_address_grmgv_upper,
    bdmf_address_gr2047,
    bdmf_address_gr2047_upper,
    bdmf_address_gr4095,
    bdmf_address_gr4095_upper,
    bdmf_address_gr9216,
    bdmf_address_gr9216_upper,
    bdmf_address_grpkt,
    bdmf_address_grpkt_upper,
    bdmf_address_grbyt,
    bdmf_address_grbyt_upper,
    bdmf_address_grmca,
    bdmf_address_grmca_upper,
    bdmf_address_grbca,
    bdmf_address_grbca_upper,
    bdmf_address_grfcs,
    bdmf_address_grfcs_upper,
    bdmf_address_grxcf,
    bdmf_address_grxcf_upper,
    bdmf_address_grxpf,
    bdmf_address_grxpf_upper,
    bdmf_address_grxuo,
    bdmf_address_grxuo_upper,
    bdmf_address_graln,
    bdmf_address_graln_upper,
    bdmf_address_grflr,
    bdmf_address_grflr_upper,
    bdmf_address_grcde,
    bdmf_address_grcde_upper,
    bdmf_address_grfcr,
    bdmf_address_grfcr_upper,
    bdmf_address_grovr,
    bdmf_address_grovr_upper,
    bdmf_address_grjbr,
    bdmf_address_grjbr_upper,
    bdmf_address_grmtue,
    bdmf_address_grmtue_upper,
    bdmf_address_grpok,
    bdmf_address_grpok_upper,
    bdmf_address_gruc,
    bdmf_address_gruc_upper,
    bdmf_address_grppp,
    bdmf_address_grppp_upper,
    bdmf_address_grcrc,
    bdmf_address_grcrc_upper,
    bdmf_address_tr64,
    bdmf_address_tr64_upper,
    bdmf_address_tr127,
    bdmf_address_tr127_upper,
    bdmf_address_tr255,
    bdmf_address_tr255_upper,
    bdmf_address_tr511,
    bdmf_address_tr511_upper,
    bdmf_address_tr1023,
    bdmf_address_tr1023_upper,
    bdmf_address_tr1518,
    bdmf_address_tr1518_upper,
    bdmf_address_trmgv,
    bdmf_address_trmgv_upper,
    bdmf_address_tr2047,
    bdmf_address_tr2047_upper,
    bdmf_address_tr4095,
    bdmf_address_tr4095_upper,
    bdmf_address_tr9216,
    bdmf_address_tr9216_upper,
    bdmf_address_gtpkt,
    bdmf_address_gtpkt_upper,
    bdmf_address_gtmca,
    bdmf_address_gtmca_upper,
    bdmf_address_gtbca,
    bdmf_address_gtbca_upper,
    bdmf_address_gtxpf,
    bdmf_address_gtxpf_upper,
    bdmf_address_gtxcf,
    bdmf_address_gtxcf_upper,
    bdmf_address_gtfcs,
    bdmf_address_gtfcs_upper,
    bdmf_address_gtovr,
    bdmf_address_gtovr_upper,
    bdmf_address_gtdrf,
    bdmf_address_gtdrf_upper,
    bdmf_address_gtedf,
    bdmf_address_gtedf_upper,
    bdmf_address_gtscl,
    bdmf_address_gtscl_upper,
    bdmf_address_gtmcl,
    bdmf_address_gtmcl_upper,
    bdmf_address_gtlcl,
    bdmf_address_gtlcl_upper,
    bdmf_address_gtxcl,
    bdmf_address_gtxcl_upper,
    bdmf_address_gtfrg,
    bdmf_address_gtfrg_upper,
    bdmf_address_gtncl,
    bdmf_address_gtncl_upper,
    bdmf_address_gtjbr,
    bdmf_address_gtjbr_upper,
    bdmf_address_gtbyt,
    bdmf_address_gtbyt_upper,
    bdmf_address_gtpok,
    bdmf_address_gtpok_upper,
    bdmf_address_gtuc,
    bdmf_address_gtuc_upper,
    bdmf_address_rrpkt,
    bdmf_address_rrpkt_upper,
    bdmf_address_rrund,
    bdmf_address_rrund_upper,
    bdmf_address_rrfrg,
    bdmf_address_rrfrg_upper,
    bdmf_address_rrbyt,
    bdmf_address_rrbyt_upper,
    bdmf_address_mib_cntrl,
    bdmf_address_mib_read_data,
    bdmf_address_mib_write_data,
    bdmf_address_control,
    bdmf_address_psw_ms,
    bdmf_address_psw_ls,
    bdmf_address_control_1,
    bdmf_address_extended_control,
    bdmf_address_tx_idle_stuffing_control,
    bdmf_address_actual_data_rate,
    bdmf_address_gmii_clock_swallower_control,
    bdmf_address_xgmii_data_rate_status,
    bdmf_address_status,
    bdmf_address_rx_discard_packet_counter,
    bdmf_address_tx_discard_packet_counter,
    bdmf_address_rev,
    bdmf_address_umac_rxerr_mask,
    bdmf_address_mib_max_pkt_size,
}
bdmf_address;

static int ag_drv_xumac_rdp_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_xumac_rdp_ipg_hd_bkp_cntl:
        ag_err = ag_drv_xumac_rdp_ipg_hd_bkp_cntl_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_xumac_rdp_command_config:
    {
        xumac_rdp_command_config command_config = { .tx_ena = parm[2].value.unumber, .rx_ena = parm[3].value.unumber, .eth_speed = parm[4].value.unumber, .promis_en = parm[5].value.unumber, .pad_en = parm[6].value.unumber, .crc_fwd = parm[7].value.unumber, .pause_fwd = parm[8].value.unumber, .pause_ignore = parm[9].value.unumber, .tx_addr_ins = parm[10].value.unumber, .hd_ena = parm[11].value.unumber, .rx_low_latency_en = parm[12].value.unumber, .overflow_en = parm[13].value.unumber, .sw_reset = parm[14].value.unumber, .fcs_corrupt_urun_en = parm[15].value.unumber, .loop_ena = parm[16].value.unumber, .mac_loop_con = parm[17].value.unumber, .sw_override_tx = parm[18].value.unumber, .sw_override_rx = parm[19].value.unumber, .oob_efc_mode = parm[20].value.unumber, .bypass_oob_efc_synchronizer = parm[21].value.unumber, .en_internal_tx_crs = parm[22].value.unumber, .ena_ext_config = parm[23].value.unumber, .cntl_frm_ena = parm[24].value.unumber, .no_lgth_check = parm[25].value.unumber, .line_loopback = parm[26].value.unumber, .fd_tx_urun_fix_en = parm[27].value.unumber, .ignore_tx_pause = parm[28].value.unumber, .oob_efc_disab = parm[29].value.unumber, .runt_filter_dis = parm[30].value.unumber, .eth_speed_bit2 = parm[31].value.unumber};
        ag_err = ag_drv_xumac_rdp_command_config_set(parm[1].value.unumber, &command_config);
        break;
    }
    case cli_xumac_rdp_mac_0:
        ag_err = ag_drv_xumac_rdp_mac_0_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_mac_1:
        ag_err = ag_drv_xumac_rdp_mac_1_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_frm_length:
        ag_err = ag_drv_xumac_rdp_frm_length_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_pause_quant:
        ag_err = ag_drv_xumac_rdp_pause_quant_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_sfd_offset:
        ag_err = ag_drv_xumac_rdp_sfd_offset_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_tag_0:
        ag_err = ag_drv_xumac_rdp_tag_0_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_xumac_rdp_tag_1:
        ag_err = ag_drv_xumac_rdp_tag_1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_xumac_rdp_rx_pause_quanta_scale:
        ag_err = ag_drv_xumac_rdp_rx_pause_quanta_scale_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_xumac_rdp_tx_preamble:
        ag_err = ag_drv_xumac_rdp_tx_preamble_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_tx_ipg_length:
        ag_err = ag_drv_xumac_rdp_tx_ipg_length_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_xumac_rdp_pfc_xoff_timer:
        ag_err = ag_drv_xumac_rdp_pfc_xoff_timer_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_umac_eee_ctrl:
    {
        xumac_rdp_umac_eee_ctrl umac_eee_ctrl = { .eee_en = parm[2].value.unumber, .rx_fifo_check = parm[3].value.unumber, .eee_txclk_dis = parm[4].value.unumber, .dis_eee_10m = parm[5].value.unumber, .lp_idle_prediction_mode = parm[6].value.unumber};
        ag_err = ag_drv_xumac_rdp_umac_eee_ctrl_set(parm[1].value.unumber, &umac_eee_ctrl);
        break;
    }
    case cli_xumac_rdp_mii_eee_delay_entry_timer:
        ag_err = ag_drv_xumac_rdp_mii_eee_delay_entry_timer_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gmii_eee_delay_entry_timer:
        ag_err = ag_drv_xumac_rdp_gmii_eee_delay_entry_timer_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_umac_eee_ref_count:
        ag_err = ag_drv_xumac_rdp_umac_eee_ref_count_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_umac_timestamp_adjust:
        ag_err = ag_drv_xumac_rdp_umac_timestamp_adjust_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_xumac_rdp_umac_rx_pkt_drop_status:
        ag_err = ag_drv_xumac_rdp_umac_rx_pkt_drop_status_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_umac_symmetric_idle_threshold:
        ag_err = ag_drv_xumac_rdp_umac_symmetric_idle_threshold_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_mii_eee_wake_timer:
        ag_err = ag_drv_xumac_rdp_mii_eee_wake_timer_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gmii_eee_wake_timer:
        ag_err = ag_drv_xumac_rdp_gmii_eee_wake_timer_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gmii_2p5g_eee_delay_entry_timer:
        ag_err = ag_drv_xumac_rdp_gmii_2p5g_eee_delay_entry_timer_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gmii_5g_eee_delay_entry_timer:
        ag_err = ag_drv_xumac_rdp_gmii_5g_eee_delay_entry_timer_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gmii_10g_eee_delay_entry_timer:
        ag_err = ag_drv_xumac_rdp_gmii_10g_eee_delay_entry_timer_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gmii_2p5g_eee_wake_timer:
        ag_err = ag_drv_xumac_rdp_gmii_2p5g_eee_wake_timer_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gmii_5g_eee_wake_timer:
        ag_err = ag_drv_xumac_rdp_gmii_5g_eee_wake_timer_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gmii_10g_eee_wake_timer:
        ag_err = ag_drv_xumac_rdp_gmii_10g_eee_wake_timer_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_mac_pfc_type:
        ag_err = ag_drv_xumac_rdp_mac_pfc_type_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_mac_pfc_opcode:
        ag_err = ag_drv_xumac_rdp_mac_pfc_opcode_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_mac_pfc_da_0:
        ag_err = ag_drv_xumac_rdp_mac_pfc_da_0_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_mac_pfc_da_1:
        ag_err = ag_drv_xumac_rdp_mac_pfc_da_1_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_macsec_prog_tx_crc:
        ag_err = ag_drv_xumac_rdp_macsec_prog_tx_crc_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_macsec_cntrl:
        ag_err = ag_drv_xumac_rdp_macsec_cntrl_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_xumac_rdp_pause_refresh_ctrl:
        ag_err = ag_drv_xumac_rdp_pause_refresh_ctrl_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_xumac_rdp_flush_control:
        ag_err = ag_drv_xumac_rdp_flush_control_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_mac_pfc_ctrl:
    {
        xumac_rdp_mac_pfc_ctrl mac_pfc_ctrl = { .pfc_tx_enbl = parm[2].value.unumber, .pfc_rx_enbl = parm[3].value.unumber, .force_pfc_xon = parm[4].value.unumber, .rx_pass_pfc_frm = parm[5].value.unumber, .pfc_stats_en = parm[6].value.unumber};
        ag_err = ag_drv_xumac_rdp_mac_pfc_ctrl_set(parm[1].value.unumber, &mac_pfc_ctrl);
        break;
    }
    case cli_xumac_rdp_mac_pfc_refresh_ctrl:
        ag_err = ag_drv_xumac_rdp_mac_pfc_refresh_ctrl_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_xumac_rdp_gr64:
        ag_err = ag_drv_xumac_rdp_gr64_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gr64_upper:
        ag_err = ag_drv_xumac_rdp_gr64_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gr127:
        ag_err = ag_drv_xumac_rdp_gr127_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gr127_upper:
        ag_err = ag_drv_xumac_rdp_gr127_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gr255:
        ag_err = ag_drv_xumac_rdp_gr255_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gr255_upper:
        ag_err = ag_drv_xumac_rdp_gr255_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gr511:
        ag_err = ag_drv_xumac_rdp_gr511_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gr511_upper:
        ag_err = ag_drv_xumac_rdp_gr511_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gr1023:
        ag_err = ag_drv_xumac_rdp_gr1023_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gr1023_upper:
        ag_err = ag_drv_xumac_rdp_gr1023_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gr1518:
        ag_err = ag_drv_xumac_rdp_gr1518_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gr1518_upper:
        ag_err = ag_drv_xumac_rdp_gr1518_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_grmgv:
        ag_err = ag_drv_xumac_rdp_grmgv_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_grmgv_upper:
        ag_err = ag_drv_xumac_rdp_grmgv_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gr2047:
        ag_err = ag_drv_xumac_rdp_gr2047_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gr2047_upper:
        ag_err = ag_drv_xumac_rdp_gr2047_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gr4095:
        ag_err = ag_drv_xumac_rdp_gr4095_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gr4095_upper:
        ag_err = ag_drv_xumac_rdp_gr4095_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gr9216:
        ag_err = ag_drv_xumac_rdp_gr9216_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gr9216_upper:
        ag_err = ag_drv_xumac_rdp_gr9216_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_grpkt:
        ag_err = ag_drv_xumac_rdp_grpkt_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_grpkt_upper:
        ag_err = ag_drv_xumac_rdp_grpkt_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_grbyt:
        ag_err = ag_drv_xumac_rdp_grbyt_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_grbyt_upper:
        ag_err = ag_drv_xumac_rdp_grbyt_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_grmca:
        ag_err = ag_drv_xumac_rdp_grmca_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_grmca_upper:
        ag_err = ag_drv_xumac_rdp_grmca_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_grbca:
        ag_err = ag_drv_xumac_rdp_grbca_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_grbca_upper:
        ag_err = ag_drv_xumac_rdp_grbca_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_grfcs:
        ag_err = ag_drv_xumac_rdp_grfcs_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_grfcs_upper:
        ag_err = ag_drv_xumac_rdp_grfcs_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_grxcf:
        ag_err = ag_drv_xumac_rdp_grxcf_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_grxcf_upper:
        ag_err = ag_drv_xumac_rdp_grxcf_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_grxpf:
        ag_err = ag_drv_xumac_rdp_grxpf_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_grxpf_upper:
        ag_err = ag_drv_xumac_rdp_grxpf_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_grxuo:
        ag_err = ag_drv_xumac_rdp_grxuo_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_grxuo_upper:
        ag_err = ag_drv_xumac_rdp_grxuo_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_graln:
        ag_err = ag_drv_xumac_rdp_graln_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_graln_upper:
        ag_err = ag_drv_xumac_rdp_graln_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_grflr:
        ag_err = ag_drv_xumac_rdp_grflr_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_grflr_upper:
        ag_err = ag_drv_xumac_rdp_grflr_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_grcde:
        ag_err = ag_drv_xumac_rdp_grcde_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_grcde_upper:
        ag_err = ag_drv_xumac_rdp_grcde_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_grfcr:
        ag_err = ag_drv_xumac_rdp_grfcr_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_grfcr_upper:
        ag_err = ag_drv_xumac_rdp_grfcr_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_grovr:
        ag_err = ag_drv_xumac_rdp_grovr_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_grovr_upper:
        ag_err = ag_drv_xumac_rdp_grovr_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_grjbr:
        ag_err = ag_drv_xumac_rdp_grjbr_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_grjbr_upper:
        ag_err = ag_drv_xumac_rdp_grjbr_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_grmtue:
        ag_err = ag_drv_xumac_rdp_grmtue_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_grmtue_upper:
        ag_err = ag_drv_xumac_rdp_grmtue_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_grpok:
        ag_err = ag_drv_xumac_rdp_grpok_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_grpok_upper:
        ag_err = ag_drv_xumac_rdp_grpok_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gruc:
        ag_err = ag_drv_xumac_rdp_gruc_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gruc_upper:
        ag_err = ag_drv_xumac_rdp_gruc_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_grppp:
        ag_err = ag_drv_xumac_rdp_grppp_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_grppp_upper:
        ag_err = ag_drv_xumac_rdp_grppp_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_grcrc:
        ag_err = ag_drv_xumac_rdp_grcrc_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_grcrc_upper:
        ag_err = ag_drv_xumac_rdp_grcrc_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_tr64:
        ag_err = ag_drv_xumac_rdp_tr64_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_tr64_upper:
        ag_err = ag_drv_xumac_rdp_tr64_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_tr127:
        ag_err = ag_drv_xumac_rdp_tr127_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_tr127_upper:
        ag_err = ag_drv_xumac_rdp_tr127_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_tr255:
        ag_err = ag_drv_xumac_rdp_tr255_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_tr255_upper:
        ag_err = ag_drv_xumac_rdp_tr255_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_tr511:
        ag_err = ag_drv_xumac_rdp_tr511_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_tr511_upper:
        ag_err = ag_drv_xumac_rdp_tr511_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_tr1023:
        ag_err = ag_drv_xumac_rdp_tr1023_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_tr1023_upper:
        ag_err = ag_drv_xumac_rdp_tr1023_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_tr1518:
        ag_err = ag_drv_xumac_rdp_tr1518_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_tr1518_upper:
        ag_err = ag_drv_xumac_rdp_tr1518_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_trmgv:
        ag_err = ag_drv_xumac_rdp_trmgv_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_trmgv_upper:
        ag_err = ag_drv_xumac_rdp_trmgv_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_tr2047:
        ag_err = ag_drv_xumac_rdp_tr2047_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_tr2047_upper:
        ag_err = ag_drv_xumac_rdp_tr2047_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_tr4095:
        ag_err = ag_drv_xumac_rdp_tr4095_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_tr4095_upper:
        ag_err = ag_drv_xumac_rdp_tr4095_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_tr9216:
        ag_err = ag_drv_xumac_rdp_tr9216_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_tr9216_upper:
        ag_err = ag_drv_xumac_rdp_tr9216_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtpkt:
        ag_err = ag_drv_xumac_rdp_gtpkt_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtpkt_upper:
        ag_err = ag_drv_xumac_rdp_gtpkt_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtmca:
        ag_err = ag_drv_xumac_rdp_gtmca_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtmca_upper:
        ag_err = ag_drv_xumac_rdp_gtmca_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtbca:
        ag_err = ag_drv_xumac_rdp_gtbca_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtbca_upper:
        ag_err = ag_drv_xumac_rdp_gtbca_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtxpf:
        ag_err = ag_drv_xumac_rdp_gtxpf_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtxpf_upper:
        ag_err = ag_drv_xumac_rdp_gtxpf_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtxcf:
        ag_err = ag_drv_xumac_rdp_gtxcf_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtxcf_upper:
        ag_err = ag_drv_xumac_rdp_gtxcf_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtfcs:
        ag_err = ag_drv_xumac_rdp_gtfcs_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtfcs_upper:
        ag_err = ag_drv_xumac_rdp_gtfcs_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtovr:
        ag_err = ag_drv_xumac_rdp_gtovr_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtovr_upper:
        ag_err = ag_drv_xumac_rdp_gtovr_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtdrf:
        ag_err = ag_drv_xumac_rdp_gtdrf_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtdrf_upper:
        ag_err = ag_drv_xumac_rdp_gtdrf_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtedf:
        ag_err = ag_drv_xumac_rdp_gtedf_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtedf_upper:
        ag_err = ag_drv_xumac_rdp_gtedf_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtscl:
        ag_err = ag_drv_xumac_rdp_gtscl_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtscl_upper:
        ag_err = ag_drv_xumac_rdp_gtscl_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtmcl:
        ag_err = ag_drv_xumac_rdp_gtmcl_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtmcl_upper:
        ag_err = ag_drv_xumac_rdp_gtmcl_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtlcl:
        ag_err = ag_drv_xumac_rdp_gtlcl_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtlcl_upper:
        ag_err = ag_drv_xumac_rdp_gtlcl_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtxcl:
        ag_err = ag_drv_xumac_rdp_gtxcl_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtxcl_upper:
        ag_err = ag_drv_xumac_rdp_gtxcl_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtfrg:
        ag_err = ag_drv_xumac_rdp_gtfrg_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtfrg_upper:
        ag_err = ag_drv_xumac_rdp_gtfrg_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtncl:
        ag_err = ag_drv_xumac_rdp_gtncl_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtncl_upper:
        ag_err = ag_drv_xumac_rdp_gtncl_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtjbr:
        ag_err = ag_drv_xumac_rdp_gtjbr_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtjbr_upper:
        ag_err = ag_drv_xumac_rdp_gtjbr_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtbyt:
        ag_err = ag_drv_xumac_rdp_gtbyt_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtbyt_upper:
        ag_err = ag_drv_xumac_rdp_gtbyt_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtpok:
        ag_err = ag_drv_xumac_rdp_gtpok_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtpok_upper:
        ag_err = ag_drv_xumac_rdp_gtpok_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtuc:
        ag_err = ag_drv_xumac_rdp_gtuc_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gtuc_upper:
        ag_err = ag_drv_xumac_rdp_gtuc_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_rrpkt:
        ag_err = ag_drv_xumac_rdp_rrpkt_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_rrpkt_upper:
        ag_err = ag_drv_xumac_rdp_rrpkt_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_rrund:
        ag_err = ag_drv_xumac_rdp_rrund_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_rrund_upper:
        ag_err = ag_drv_xumac_rdp_rrund_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_rrfrg:
        ag_err = ag_drv_xumac_rdp_rrfrg_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_rrfrg_upper:
        ag_err = ag_drv_xumac_rdp_rrfrg_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_rrbyt:
        ag_err = ag_drv_xumac_rdp_rrbyt_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_rrbyt_upper:
        ag_err = ag_drv_xumac_rdp_rrbyt_upper_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_mib_cntrl:
        ag_err = ag_drv_xumac_rdp_mib_cntrl_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_xumac_rdp_mib_read_data:
        ag_err = ag_drv_xumac_rdp_mib_read_data_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_mib_write_data:
        ag_err = ag_drv_xumac_rdp_mib_write_data_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_control:
        ag_err = ag_drv_xumac_rdp_control_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_xumac_rdp_psw_ms:
        ag_err = ag_drv_xumac_rdp_psw_ms_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_psw_ls:
        ag_err = ag_drv_xumac_rdp_psw_ls_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_control_1:
    {
        xumac_rdp_control_1 control_1 = { .xib_rx_en = parm[2].value.unumber, .xib_tx_en = parm[3].value.unumber, .rx_flush_en = parm[4].value.unumber, .tx_flush_en = parm[5].value.unumber, .link_down_rst_en = parm[6].value.unumber, .standard_mux_en = parm[7].value.unumber, .xgmii_sel = parm[8].value.unumber, .dic_dis = parm[9].value.unumber, .rx_start_threshold = parm[10].value.unumber, .gmii_rx_clk_gate_en = parm[11].value.unumber, .strict_preamble_dis = parm[12].value.unumber, .tx_ipg = parm[13].value.unumber, .min_rx_ipg = parm[14].value.unumber, .xgmii_sel_ovrd = parm[15].value.unumber, .gmii_tx_clk_gate_en = parm[16].value.unumber, .autoconfig_en = parm[17].value.unumber};
        ag_err = ag_drv_xumac_rdp_control_1_set(parm[1].value.unumber, &control_1);
        break;
    }
    case cli_xumac_rdp_extended_control:
        ag_err = ag_drv_xumac_rdp_extended_control_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_xumac_rdp_tx_idle_stuffing_control:
        ag_err = ag_drv_xumac_rdp_tx_idle_stuffing_control_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_actual_data_rate:
        ag_err = ag_drv_xumac_rdp_actual_data_rate_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_gmii_clock_swallower_control:
        ag_err = ag_drv_xumac_rdp_gmii_clock_swallower_control_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_xumac_rdp_rx_discard_packet_counter:
        ag_err = ag_drv_xumac_rdp_rx_discard_packet_counter_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_tx_discard_packet_counter:
        ag_err = ag_drv_xumac_rdp_tx_discard_packet_counter_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_umac_rxerr_mask:
        ag_err = ag_drv_xumac_rdp_umac_rxerr_mask_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_xumac_rdp_mib_max_pkt_size:
        ag_err = ag_drv_xumac_rdp_mib_max_pkt_size_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

int bcm_xumac_rdp_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_xumac_rdp_ipg_hd_bkp_cntl:
    {
        bdmf_boolean hd_fc_ena;
        bdmf_boolean hd_fc_bkoff_ok;
        uint8_t ipg_config_rx;
        ag_err = ag_drv_xumac_rdp_ipg_hd_bkp_cntl_get(parm[1].value.unumber, &hd_fc_ena, &hd_fc_bkoff_ok, &ipg_config_rx);
        bdmf_session_print(session, "hd_fc_ena = %u = 0x%x\n", hd_fc_ena, hd_fc_ena);
        bdmf_session_print(session, "hd_fc_bkoff_ok = %u = 0x%x\n", hd_fc_bkoff_ok, hd_fc_bkoff_ok);
        bdmf_session_print(session, "ipg_config_rx = %u = 0x%x\n", ipg_config_rx, ipg_config_rx);
        break;
    }
    case cli_xumac_rdp_command_config:
    {
        xumac_rdp_command_config command_config;
        ag_err = ag_drv_xumac_rdp_command_config_get(parm[1].value.unumber, &command_config);
        bdmf_session_print(session, "tx_ena = %u = 0x%x\n", command_config.tx_ena, command_config.tx_ena);
        bdmf_session_print(session, "rx_ena = %u = 0x%x\n", command_config.rx_ena, command_config.rx_ena);
        bdmf_session_print(session, "eth_speed = %u = 0x%x\n", command_config.eth_speed, command_config.eth_speed);
        bdmf_session_print(session, "promis_en = %u = 0x%x\n", command_config.promis_en, command_config.promis_en);
        bdmf_session_print(session, "pad_en = %u = 0x%x\n", command_config.pad_en, command_config.pad_en);
        bdmf_session_print(session, "crc_fwd = %u = 0x%x\n", command_config.crc_fwd, command_config.crc_fwd);
        bdmf_session_print(session, "pause_fwd = %u = 0x%x\n", command_config.pause_fwd, command_config.pause_fwd);
        bdmf_session_print(session, "pause_ignore = %u = 0x%x\n", command_config.pause_ignore, command_config.pause_ignore);
        bdmf_session_print(session, "tx_addr_ins = %u = 0x%x\n", command_config.tx_addr_ins, command_config.tx_addr_ins);
        bdmf_session_print(session, "hd_ena = %u = 0x%x\n", command_config.hd_ena, command_config.hd_ena);
        bdmf_session_print(session, "rx_low_latency_en = %u = 0x%x\n", command_config.rx_low_latency_en, command_config.rx_low_latency_en);
        bdmf_session_print(session, "overflow_en = %u = 0x%x\n", command_config.overflow_en, command_config.overflow_en);
        bdmf_session_print(session, "sw_reset = %u = 0x%x\n", command_config.sw_reset, command_config.sw_reset);
        bdmf_session_print(session, "fcs_corrupt_urun_en = %u = 0x%x\n", command_config.fcs_corrupt_urun_en, command_config.fcs_corrupt_urun_en);
        bdmf_session_print(session, "loop_ena = %u = 0x%x\n", command_config.loop_ena, command_config.loop_ena);
        bdmf_session_print(session, "mac_loop_con = %u = 0x%x\n", command_config.mac_loop_con, command_config.mac_loop_con);
        bdmf_session_print(session, "sw_override_tx = %u = 0x%x\n", command_config.sw_override_tx, command_config.sw_override_tx);
        bdmf_session_print(session, "sw_override_rx = %u = 0x%x\n", command_config.sw_override_rx, command_config.sw_override_rx);
        bdmf_session_print(session, "oob_efc_mode = %u = 0x%x\n", command_config.oob_efc_mode, command_config.oob_efc_mode);
        bdmf_session_print(session, "bypass_oob_efc_synchronizer = %u = 0x%x\n", command_config.bypass_oob_efc_synchronizer, command_config.bypass_oob_efc_synchronizer);
        bdmf_session_print(session, "en_internal_tx_crs = %u = 0x%x\n", command_config.en_internal_tx_crs, command_config.en_internal_tx_crs);
        bdmf_session_print(session, "ena_ext_config = %u = 0x%x\n", command_config.ena_ext_config, command_config.ena_ext_config);
        bdmf_session_print(session, "cntl_frm_ena = %u = 0x%x\n", command_config.cntl_frm_ena, command_config.cntl_frm_ena);
        bdmf_session_print(session, "no_lgth_check = %u = 0x%x\n", command_config.no_lgth_check, command_config.no_lgth_check);
        bdmf_session_print(session, "line_loopback = %u = 0x%x\n", command_config.line_loopback, command_config.line_loopback);
        bdmf_session_print(session, "fd_tx_urun_fix_en = %u = 0x%x\n", command_config.fd_tx_urun_fix_en, command_config.fd_tx_urun_fix_en);
        bdmf_session_print(session, "ignore_tx_pause = %u = 0x%x\n", command_config.ignore_tx_pause, command_config.ignore_tx_pause);
        bdmf_session_print(session, "oob_efc_disab = %u = 0x%x\n", command_config.oob_efc_disab, command_config.oob_efc_disab);
        bdmf_session_print(session, "runt_filter_dis = %u = 0x%x\n", command_config.runt_filter_dis, command_config.runt_filter_dis);
        bdmf_session_print(session, "eth_speed_bit2 = %u = 0x%x\n", command_config.eth_speed_bit2, command_config.eth_speed_bit2);
        break;
    }
    case cli_xumac_rdp_mac_0:
    {
        uint32_t mac_addr0;
        ag_err = ag_drv_xumac_rdp_mac_0_get(parm[1].value.unumber, &mac_addr0);
        bdmf_session_print(session, "mac_addr0 = %u = 0x%x\n", mac_addr0, mac_addr0);
        break;
    }
    case cli_xumac_rdp_mac_1:
    {
        uint16_t mac_addr1;
        ag_err = ag_drv_xumac_rdp_mac_1_get(parm[1].value.unumber, &mac_addr1);
        bdmf_session_print(session, "mac_addr1 = %u = 0x%x\n", mac_addr1, mac_addr1);
        break;
    }
    case cli_xumac_rdp_frm_length:
    {
        uint16_t maxfr;
        ag_err = ag_drv_xumac_rdp_frm_length_get(parm[1].value.unumber, &maxfr);
        bdmf_session_print(session, "maxfr = %u = 0x%x\n", maxfr, maxfr);
        break;
    }
    case cli_xumac_rdp_pause_quant:
    {
        uint16_t pause_quant;
        ag_err = ag_drv_xumac_rdp_pause_quant_get(parm[1].value.unumber, &pause_quant);
        bdmf_session_print(session, "pause_quant = %u = 0x%x\n", pause_quant, pause_quant);
        break;
    }
    case cli_xumac_rdp_tx_ts_seq_id:
    {
        uint16_t tsts_seq_id;
        bdmf_boolean tsts_valid;
        ag_err = ag_drv_xumac_rdp_tx_ts_seq_id_get(parm[1].value.unumber, &tsts_seq_id, &tsts_valid);
        bdmf_session_print(session, "tsts_seq_id = %u = 0x%x\n", tsts_seq_id, tsts_seq_id);
        bdmf_session_print(session, "tsts_valid = %u = 0x%x\n", tsts_valid, tsts_valid);
        break;
    }
    case cli_xumac_rdp_sfd_offset:
    {
        uint8_t sfd_offset;
        ag_err = ag_drv_xumac_rdp_sfd_offset_get(parm[1].value.unumber, &sfd_offset);
        bdmf_session_print(session, "sfd_offset = %u = 0x%x\n", sfd_offset, sfd_offset);
        break;
    }
    case cli_xumac_rdp_mac_mode:
    {
        xumac_rdp_mac_mode mac_mode;
        ag_err = ag_drv_xumac_rdp_mac_mode_get(parm[1].value.unumber, &mac_mode);
        bdmf_session_print(session, "mac_speed = %u = 0x%x\n", mac_mode.mac_speed, mac_mode.mac_speed);
        bdmf_session_print(session, "mac_duplex = %u = 0x%x\n", mac_mode.mac_duplex, mac_mode.mac_duplex);
        bdmf_session_print(session, "mac_rx_pause = %u = 0x%x\n", mac_mode.mac_rx_pause, mac_mode.mac_rx_pause);
        bdmf_session_print(session, "mac_tx_pause = %u = 0x%x\n", mac_mode.mac_tx_pause, mac_mode.mac_tx_pause);
        bdmf_session_print(session, "link_status = %u = 0x%x\n", mac_mode.link_status, mac_mode.link_status);
        bdmf_session_print(session, "mac_speed_bit2 = %u = 0x%x\n", mac_mode.mac_speed_bit2, mac_mode.mac_speed_bit2);
        break;
    }
    case cli_xumac_rdp_tag_0:
    {
        uint16_t frm_tag_0;
        bdmf_boolean config_outer_tpid_enable;
        ag_err = ag_drv_xumac_rdp_tag_0_get(parm[1].value.unumber, &frm_tag_0, &config_outer_tpid_enable);
        bdmf_session_print(session, "frm_tag_0 = %u = 0x%x\n", frm_tag_0, frm_tag_0);
        bdmf_session_print(session, "config_outer_tpid_enable = %u = 0x%x\n", config_outer_tpid_enable, config_outer_tpid_enable);
        break;
    }
    case cli_xumac_rdp_tag_1:
    {
        uint16_t frm_tag_1;
        bdmf_boolean config_inner_tpid_enable;
        ag_err = ag_drv_xumac_rdp_tag_1_get(parm[1].value.unumber, &frm_tag_1, &config_inner_tpid_enable);
        bdmf_session_print(session, "frm_tag_1 = %u = 0x%x\n", frm_tag_1, frm_tag_1);
        bdmf_session_print(session, "config_inner_tpid_enable = %u = 0x%x\n", config_inner_tpid_enable, config_inner_tpid_enable);
        break;
    }
    case cli_xumac_rdp_rx_pause_quanta_scale:
    {
        uint16_t scale_value;
        bdmf_boolean scale_control;
        bdmf_boolean scale_fix;
        ag_err = ag_drv_xumac_rdp_rx_pause_quanta_scale_get(parm[1].value.unumber, &scale_value, &scale_control, &scale_fix);
        bdmf_session_print(session, "scale_value = %u = 0x%x\n", scale_value, scale_value);
        bdmf_session_print(session, "scale_control = %u = 0x%x\n", scale_control, scale_control);
        bdmf_session_print(session, "scale_fix = %u = 0x%x\n", scale_fix, scale_fix);
        break;
    }
    case cli_xumac_rdp_tx_preamble:
    {
        uint8_t tx_preamble;
        ag_err = ag_drv_xumac_rdp_tx_preamble_get(parm[1].value.unumber, &tx_preamble);
        bdmf_session_print(session, "tx_preamble = %u = 0x%x\n", tx_preamble, tx_preamble);
        break;
    }
    case cli_xumac_rdp_tx_ipg_length:
    {
        uint8_t tx_ipg_length;
        uint8_t tx_min_pkt_size;
        ag_err = ag_drv_xumac_rdp_tx_ipg_length_get(parm[1].value.unumber, &tx_ipg_length, &tx_min_pkt_size);
        bdmf_session_print(session, "tx_ipg_length = %u = 0x%x\n", tx_ipg_length, tx_ipg_length);
        bdmf_session_print(session, "tx_min_pkt_size = %u = 0x%x\n", tx_min_pkt_size, tx_min_pkt_size);
        break;
    }
    case cli_xumac_rdp_pfc_xoff_timer:
    {
        uint16_t pfc_xoff_timer;
        ag_err = ag_drv_xumac_rdp_pfc_xoff_timer_get(parm[1].value.unumber, &pfc_xoff_timer);
        bdmf_session_print(session, "pfc_xoff_timer = %u = 0x%x\n", pfc_xoff_timer, pfc_xoff_timer);
        break;
    }
    case cli_xumac_rdp_umac_eee_ctrl:
    {
        xumac_rdp_umac_eee_ctrl umac_eee_ctrl;
        ag_err = ag_drv_xumac_rdp_umac_eee_ctrl_get(parm[1].value.unumber, &umac_eee_ctrl);
        bdmf_session_print(session, "eee_en = %u = 0x%x\n", umac_eee_ctrl.eee_en, umac_eee_ctrl.eee_en);
        bdmf_session_print(session, "rx_fifo_check = %u = 0x%x\n", umac_eee_ctrl.rx_fifo_check, umac_eee_ctrl.rx_fifo_check);
        bdmf_session_print(session, "eee_txclk_dis = %u = 0x%x\n", umac_eee_ctrl.eee_txclk_dis, umac_eee_ctrl.eee_txclk_dis);
        bdmf_session_print(session, "dis_eee_10m = %u = 0x%x\n", umac_eee_ctrl.dis_eee_10m, umac_eee_ctrl.dis_eee_10m);
        bdmf_session_print(session, "lp_idle_prediction_mode = %u = 0x%x\n", umac_eee_ctrl.lp_idle_prediction_mode, umac_eee_ctrl.lp_idle_prediction_mode);
        break;
    }
    case cli_xumac_rdp_mii_eee_delay_entry_timer:
    {
        uint32_t mii_eee_lpi_timer;
        ag_err = ag_drv_xumac_rdp_mii_eee_delay_entry_timer_get(parm[1].value.unumber, &mii_eee_lpi_timer);
        bdmf_session_print(session, "mii_eee_lpi_timer = %u = 0x%x\n", mii_eee_lpi_timer, mii_eee_lpi_timer);
        break;
    }
    case cli_xumac_rdp_gmii_eee_delay_entry_timer:
    {
        uint32_t gmii_eee_lpi_timer;
        ag_err = ag_drv_xumac_rdp_gmii_eee_delay_entry_timer_get(parm[1].value.unumber, &gmii_eee_lpi_timer);
        bdmf_session_print(session, "gmii_eee_lpi_timer = %u = 0x%x\n", gmii_eee_lpi_timer, gmii_eee_lpi_timer);
        break;
    }
    case cli_xumac_rdp_umac_eee_ref_count:
    {
        uint16_t eee_ref_count;
        ag_err = ag_drv_xumac_rdp_umac_eee_ref_count_get(parm[1].value.unumber, &eee_ref_count);
        bdmf_session_print(session, "eee_ref_count = %u = 0x%x\n", eee_ref_count, eee_ref_count);
        break;
    }
    case cli_xumac_rdp_umac_timestamp_adjust:
    {
        uint16_t adjust;
        bdmf_boolean en_1588;
        bdmf_boolean auto_adjust;
        ag_err = ag_drv_xumac_rdp_umac_timestamp_adjust_get(parm[1].value.unumber, &adjust, &en_1588, &auto_adjust);
        bdmf_session_print(session, "adjust = %u = 0x%x\n", adjust, adjust);
        bdmf_session_print(session, "en_1588 = %u = 0x%x\n", en_1588, en_1588);
        bdmf_session_print(session, "auto_adjust = %u = 0x%x\n", auto_adjust, auto_adjust);
        break;
    }
    case cli_xumac_rdp_umac_rx_pkt_drop_status:
    {
        bdmf_boolean rx_ipg_inval;
        ag_err = ag_drv_xumac_rdp_umac_rx_pkt_drop_status_get(parm[1].value.unumber, &rx_ipg_inval);
        bdmf_session_print(session, "rx_ipg_inval = %u = 0x%x\n", rx_ipg_inval, rx_ipg_inval);
        break;
    }
    case cli_xumac_rdp_umac_symmetric_idle_threshold:
    {
        uint16_t threshold_value;
        ag_err = ag_drv_xumac_rdp_umac_symmetric_idle_threshold_get(parm[1].value.unumber, &threshold_value);
        bdmf_session_print(session, "threshold_value = %u = 0x%x\n", threshold_value, threshold_value);
        break;
    }
    case cli_xumac_rdp_mii_eee_wake_timer:
    {
        uint16_t mii_eee_wake_timer;
        ag_err = ag_drv_xumac_rdp_mii_eee_wake_timer_get(parm[1].value.unumber, &mii_eee_wake_timer);
        bdmf_session_print(session, "mii_eee_wake_timer = %u = 0x%x\n", mii_eee_wake_timer, mii_eee_wake_timer);
        break;
    }
    case cli_xumac_rdp_gmii_eee_wake_timer:
    {
        uint16_t gmii_eee_wake_timer;
        ag_err = ag_drv_xumac_rdp_gmii_eee_wake_timer_get(parm[1].value.unumber, &gmii_eee_wake_timer);
        bdmf_session_print(session, "gmii_eee_wake_timer = %u = 0x%x\n", gmii_eee_wake_timer, gmii_eee_wake_timer);
        break;
    }
    case cli_xumac_rdp_umac_rev_id:
    {
        uint8_t patch;
        uint8_t revision_id_minor;
        uint8_t revision_id_major;
        ag_err = ag_drv_xumac_rdp_umac_rev_id_get(parm[1].value.unumber, &patch, &revision_id_minor, &revision_id_major);
        bdmf_session_print(session, "patch = %u = 0x%x\n", patch, patch);
        bdmf_session_print(session, "revision_id_minor = %u = 0x%x\n", revision_id_minor, revision_id_minor);
        bdmf_session_print(session, "revision_id_major = %u = 0x%x\n", revision_id_major, revision_id_major);
        break;
    }
    case cli_xumac_rdp_gmii_2p5g_eee_delay_entry_timer:
    {
        uint32_t gmii_2p5g_eee_lpi_timer;
        ag_err = ag_drv_xumac_rdp_gmii_2p5g_eee_delay_entry_timer_get(parm[1].value.unumber, &gmii_2p5g_eee_lpi_timer);
        bdmf_session_print(session, "gmii_2p5g_eee_lpi_timer = %u = 0x%x\n", gmii_2p5g_eee_lpi_timer, gmii_2p5g_eee_lpi_timer);
        break;
    }
    case cli_xumac_rdp_gmii_5g_eee_delay_entry_timer:
    {
        uint32_t gmii_5g_eee_lpi_timer;
        ag_err = ag_drv_xumac_rdp_gmii_5g_eee_delay_entry_timer_get(parm[1].value.unumber, &gmii_5g_eee_lpi_timer);
        bdmf_session_print(session, "gmii_5g_eee_lpi_timer = %u = 0x%x\n", gmii_5g_eee_lpi_timer, gmii_5g_eee_lpi_timer);
        break;
    }
    case cli_xumac_rdp_gmii_10g_eee_delay_entry_timer:
    {
        uint32_t gmii_10g_eee_lpi_timer;
        ag_err = ag_drv_xumac_rdp_gmii_10g_eee_delay_entry_timer_get(parm[1].value.unumber, &gmii_10g_eee_lpi_timer);
        bdmf_session_print(session, "gmii_10g_eee_lpi_timer = %u = 0x%x\n", gmii_10g_eee_lpi_timer, gmii_10g_eee_lpi_timer);
        break;
    }
    case cli_xumac_rdp_gmii_2p5g_eee_wake_timer:
    {
        uint16_t gmii_2p5g_eee_wake_timer;
        ag_err = ag_drv_xumac_rdp_gmii_2p5g_eee_wake_timer_get(parm[1].value.unumber, &gmii_2p5g_eee_wake_timer);
        bdmf_session_print(session, "gmii_2p5g_eee_wake_timer = %u = 0x%x\n", gmii_2p5g_eee_wake_timer, gmii_2p5g_eee_wake_timer);
        break;
    }
    case cli_xumac_rdp_gmii_5g_eee_wake_timer:
    {
        uint16_t gmii_5g_eee_wake_timer;
        ag_err = ag_drv_xumac_rdp_gmii_5g_eee_wake_timer_get(parm[1].value.unumber, &gmii_5g_eee_wake_timer);
        bdmf_session_print(session, "gmii_5g_eee_wake_timer = %u = 0x%x\n", gmii_5g_eee_wake_timer, gmii_5g_eee_wake_timer);
        break;
    }
    case cli_xumac_rdp_gmii_10g_eee_wake_timer:
    {
        uint16_t gmii_10g_eee_wake_timer;
        ag_err = ag_drv_xumac_rdp_gmii_10g_eee_wake_timer_get(parm[1].value.unumber, &gmii_10g_eee_wake_timer);
        bdmf_session_print(session, "gmii_10g_eee_wake_timer = %u = 0x%x\n", gmii_10g_eee_wake_timer, gmii_10g_eee_wake_timer);
        break;
    }
    case cli_xumac_rdp_active_eee_delay_entry_timer:
    {
        uint32_t active_eee_lpi_timer;
        ag_err = ag_drv_xumac_rdp_active_eee_delay_entry_timer_get(parm[1].value.unumber, &active_eee_lpi_timer);
        bdmf_session_print(session, "active_eee_lpi_timer = %u = 0x%x\n", active_eee_lpi_timer, active_eee_lpi_timer);
        break;
    }
    case cli_xumac_rdp_active_eee_wake_timer:
    {
        uint16_t active_eee_wake_time;
        ag_err = ag_drv_xumac_rdp_active_eee_wake_timer_get(parm[1].value.unumber, &active_eee_wake_time);
        bdmf_session_print(session, "active_eee_wake_time = %u = 0x%x\n", active_eee_wake_time, active_eee_wake_time);
        break;
    }
    case cli_xumac_rdp_mac_pfc_type:
    {
        uint16_t pfc_eth_type;
        ag_err = ag_drv_xumac_rdp_mac_pfc_type_get(parm[1].value.unumber, &pfc_eth_type);
        bdmf_session_print(session, "pfc_eth_type = %u = 0x%x\n", pfc_eth_type, pfc_eth_type);
        break;
    }
    case cli_xumac_rdp_mac_pfc_opcode:
    {
        uint16_t pfc_opcode;
        ag_err = ag_drv_xumac_rdp_mac_pfc_opcode_get(parm[1].value.unumber, &pfc_opcode);
        bdmf_session_print(session, "pfc_opcode = %u = 0x%x\n", pfc_opcode, pfc_opcode);
        break;
    }
    case cli_xumac_rdp_mac_pfc_da_0:
    {
        uint32_t pfc_macda_0;
        ag_err = ag_drv_xumac_rdp_mac_pfc_da_0_get(parm[1].value.unumber, &pfc_macda_0);
        bdmf_session_print(session, "pfc_macda_0 = %u = 0x%x\n", pfc_macda_0, pfc_macda_0);
        break;
    }
    case cli_xumac_rdp_mac_pfc_da_1:
    {
        uint16_t pfc_macda_1;
        ag_err = ag_drv_xumac_rdp_mac_pfc_da_1_get(parm[1].value.unumber, &pfc_macda_1);
        bdmf_session_print(session, "pfc_macda_1 = %u = 0x%x\n", pfc_macda_1, pfc_macda_1);
        break;
    }
    case cli_xumac_rdp_macsec_prog_tx_crc:
    {
        uint32_t macsec_prog_tx_crc;
        ag_err = ag_drv_xumac_rdp_macsec_prog_tx_crc_get(parm[1].value.unumber, &macsec_prog_tx_crc);
        bdmf_session_print(session, "macsec_prog_tx_crc = %u = 0x%x\n", macsec_prog_tx_crc, macsec_prog_tx_crc);
        break;
    }
    case cli_xumac_rdp_macsec_cntrl:
    {
        bdmf_boolean tx_launch_en;
        bdmf_boolean tx_crc_corupt_en;
        bdmf_boolean tx_crc_program;
        bdmf_boolean dis_pause_data_var_ipg;
        ag_err = ag_drv_xumac_rdp_macsec_cntrl_get(parm[1].value.unumber, &tx_launch_en, &tx_crc_corupt_en, &tx_crc_program, &dis_pause_data_var_ipg);
        bdmf_session_print(session, "tx_launch_en = %u = 0x%x\n", tx_launch_en, tx_launch_en);
        bdmf_session_print(session, "tx_crc_corupt_en = %u = 0x%x\n", tx_crc_corupt_en, tx_crc_corupt_en);
        bdmf_session_print(session, "tx_crc_program = %u = 0x%x\n", tx_crc_program, tx_crc_program);
        bdmf_session_print(session, "dis_pause_data_var_ipg = %u = 0x%x\n", dis_pause_data_var_ipg, dis_pause_data_var_ipg);
        break;
    }
    case cli_xumac_rdp_ts_status:
    {
        bdmf_boolean tx_ts_fifo_full;
        bdmf_boolean tx_ts_fifo_empty;
        uint8_t word_avail;
        ag_err = ag_drv_xumac_rdp_ts_status_get(parm[1].value.unumber, &tx_ts_fifo_full, &tx_ts_fifo_empty, &word_avail);
        bdmf_session_print(session, "tx_ts_fifo_full = %u = 0x%x\n", tx_ts_fifo_full, tx_ts_fifo_full);
        bdmf_session_print(session, "tx_ts_fifo_empty = %u = 0x%x\n", tx_ts_fifo_empty, tx_ts_fifo_empty);
        bdmf_session_print(session, "word_avail = %u = 0x%x\n", word_avail, word_avail);
        break;
    }
    case cli_xumac_rdp_tx_ts_data:
    {
        uint32_t tx_ts_data;
        ag_err = ag_drv_xumac_rdp_tx_ts_data_get(parm[1].value.unumber, &tx_ts_data);
        bdmf_session_print(session, "tx_ts_data = %u = 0x%x\n", tx_ts_data, tx_ts_data);
        break;
    }
    case cli_xumac_rdp_pause_refresh_ctrl:
    {
        uint32_t refresh_timer;
        bdmf_boolean enable;
        ag_err = ag_drv_xumac_rdp_pause_refresh_ctrl_get(parm[1].value.unumber, &refresh_timer, &enable);
        bdmf_session_print(session, "refresh_timer = %u = 0x%x\n", refresh_timer, refresh_timer);
        bdmf_session_print(session, "enable = %u = 0x%x\n", enable, enable);
        break;
    }
    case cli_xumac_rdp_flush_control:
    {
        bdmf_boolean flush;
        ag_err = ag_drv_xumac_rdp_flush_control_get(parm[1].value.unumber, &flush);
        bdmf_session_print(session, "flush = %u = 0x%x\n", flush, flush);
        break;
    }
    case cli_xumac_rdp_rxfifo_stat:
    {
        bdmf_boolean rxfifo_underrun;
        bdmf_boolean rxfifo_overrun;
        ag_err = ag_drv_xumac_rdp_rxfifo_stat_get(parm[1].value.unumber, &rxfifo_underrun, &rxfifo_overrun);
        bdmf_session_print(session, "rxfifo_underrun = %u = 0x%x\n", rxfifo_underrun, rxfifo_underrun);
        bdmf_session_print(session, "rxfifo_overrun = %u = 0x%x\n", rxfifo_overrun, rxfifo_overrun);
        break;
    }
    case cli_xumac_rdp_txfifo_stat:
    {
        bdmf_boolean txfifo_underrun;
        bdmf_boolean txfifo_overrun;
        ag_err = ag_drv_xumac_rdp_txfifo_stat_get(parm[1].value.unumber, &txfifo_underrun, &txfifo_overrun);
        bdmf_session_print(session, "txfifo_underrun = %u = 0x%x\n", txfifo_underrun, txfifo_underrun);
        bdmf_session_print(session, "txfifo_overrun = %u = 0x%x\n", txfifo_overrun, txfifo_overrun);
        break;
    }
    case cli_xumac_rdp_mac_pfc_ctrl:
    {
        xumac_rdp_mac_pfc_ctrl mac_pfc_ctrl;
        ag_err = ag_drv_xumac_rdp_mac_pfc_ctrl_get(parm[1].value.unumber, &mac_pfc_ctrl);
        bdmf_session_print(session, "pfc_tx_enbl = %u = 0x%x\n", mac_pfc_ctrl.pfc_tx_enbl, mac_pfc_ctrl.pfc_tx_enbl);
        bdmf_session_print(session, "pfc_rx_enbl = %u = 0x%x\n", mac_pfc_ctrl.pfc_rx_enbl, mac_pfc_ctrl.pfc_rx_enbl);
        bdmf_session_print(session, "force_pfc_xon = %u = 0x%x\n", mac_pfc_ctrl.force_pfc_xon, mac_pfc_ctrl.force_pfc_xon);
        bdmf_session_print(session, "rx_pass_pfc_frm = %u = 0x%x\n", mac_pfc_ctrl.rx_pass_pfc_frm, mac_pfc_ctrl.rx_pass_pfc_frm);
        bdmf_session_print(session, "pfc_stats_en = %u = 0x%x\n", mac_pfc_ctrl.pfc_stats_en, mac_pfc_ctrl.pfc_stats_en);
        break;
    }
    case cli_xumac_rdp_mac_pfc_refresh_ctrl:
    {
        bdmf_boolean pfc_refresh_en;
        uint16_t pfc_refresh_timer;
        ag_err = ag_drv_xumac_rdp_mac_pfc_refresh_ctrl_get(parm[1].value.unumber, &pfc_refresh_en, &pfc_refresh_timer);
        bdmf_session_print(session, "pfc_refresh_en = %u = 0x%x\n", pfc_refresh_en, pfc_refresh_en);
        bdmf_session_print(session, "pfc_refresh_timer = %u = 0x%x\n", pfc_refresh_timer, pfc_refresh_timer);
        break;
    }
    case cli_xumac_rdp_gr64:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_gr64_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_gr64_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_gr64_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_gr127:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_gr127_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_gr127_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_gr127_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_gr255:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_gr255_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_gr255_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_gr255_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_gr511:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_gr511_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_gr511_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_gr511_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_gr1023:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_gr1023_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_gr1023_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_gr1023_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_gr1518:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_gr1518_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_gr1518_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_gr1518_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_grmgv:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_grmgv_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_grmgv_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_grmgv_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_gr2047:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_gr2047_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_gr2047_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_gr2047_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_gr4095:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_gr4095_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_gr4095_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_gr4095_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_gr9216:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_gr9216_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_gr9216_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_gr9216_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_grpkt:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_grpkt_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_grpkt_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_grpkt_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_grbyt:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_grbyt_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_grbyt_upper:
    {
        uint16_t count_u16;
        ag_err = ag_drv_xumac_rdp_grbyt_upper_get(parm[1].value.unumber, &count_u16);
        bdmf_session_print(session, "count_u16 = %u = 0x%x\n", count_u16, count_u16);
        break;
    }
    case cli_xumac_rdp_grmca:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_grmca_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_grmca_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_grmca_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_grbca:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_grbca_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_grbca_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_grbca_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_grfcs:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_grfcs_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_grfcs_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_grfcs_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_grxcf:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_grxcf_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_grxcf_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_grxcf_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_grxpf:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_grxpf_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_grxpf_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_grxpf_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_grxuo:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_grxuo_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_grxuo_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_grxuo_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_graln:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_graln_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_graln_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_graln_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_grflr:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_grflr_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_grflr_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_grflr_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_grcde:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_grcde_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_grcde_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_grcde_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_grfcr:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_grfcr_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_grfcr_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_grfcr_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_grovr:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_grovr_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_grovr_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_grovr_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_grjbr:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_grjbr_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_grjbr_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_grjbr_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_grmtue:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_grmtue_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_grmtue_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_grmtue_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_grpok:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_grpok_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_grpok_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_grpok_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_gruc:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_gruc_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_gruc_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_gruc_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_grppp:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_grppp_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_grppp_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_grppp_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_grcrc:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_grcrc_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_grcrc_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_grcrc_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_tr64:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_tr64_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_tr64_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_tr64_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_tr127:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_tr127_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_tr127_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_tr127_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_tr255:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_tr255_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_tr255_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_tr255_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_tr511:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_tr511_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_tr511_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_tr511_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_tr1023:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_tr1023_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_tr1023_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_tr1023_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_tr1518:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_tr1518_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_tr1518_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_tr1518_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_trmgv:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_trmgv_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_trmgv_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_trmgv_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_tr2047:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_tr2047_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_tr2047_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_tr2047_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_tr4095:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_tr4095_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_tr4095_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_tr4095_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_tr9216:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_tr9216_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_tr9216_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_tr9216_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_gtpkt:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_gtpkt_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_gtpkt_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_gtpkt_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_gtmca:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_gtmca_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_gtmca_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_gtmca_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_gtbca:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_gtbca_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_gtbca_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_gtbca_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_gtxpf:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_gtxpf_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_gtxpf_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_gtxpf_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_gtxcf:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_gtxcf_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_gtxcf_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_gtxcf_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_gtfcs:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_gtfcs_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_gtfcs_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_gtfcs_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_gtovr:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_gtovr_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_gtovr_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_gtovr_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_gtdrf:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_gtdrf_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_gtdrf_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_gtdrf_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_gtedf:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_gtedf_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_gtedf_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_gtedf_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_gtscl:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_gtscl_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_gtscl_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_gtscl_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_gtmcl:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_gtmcl_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_gtmcl_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_gtmcl_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_gtlcl:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_gtlcl_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_gtlcl_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_gtlcl_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_gtxcl:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_gtxcl_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_gtxcl_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_gtxcl_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_gtfrg:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_gtfrg_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_gtfrg_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_gtfrg_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_gtncl:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_gtncl_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_gtncl_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_gtncl_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_gtjbr:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_gtjbr_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_gtjbr_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_gtjbr_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_gtbyt:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_gtbyt_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_gtbyt_upper:
    {
        uint16_t count_u16;
        ag_err = ag_drv_xumac_rdp_gtbyt_upper_get(parm[1].value.unumber, &count_u16);
        bdmf_session_print(session, "count_u16 = %u = 0x%x\n", count_u16, count_u16);
        break;
    }
    case cli_xumac_rdp_gtpok:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_gtpok_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_gtpok_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_gtpok_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_gtuc:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_gtuc_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_gtuc_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_gtuc_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_rrpkt:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_rrpkt_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_rrpkt_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_rrpkt_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_rrund:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_rrund_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_rrund_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_rrund_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_rrfrg:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_rrfrg_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_rrfrg_upper:
    {
        uint8_t count_u8;
        ag_err = ag_drv_xumac_rdp_rrfrg_upper_get(parm[1].value.unumber, &count_u8);
        bdmf_session_print(session, "count_u8 = %u = 0x%x\n", count_u8, count_u8);
        break;
    }
    case cli_xumac_rdp_rrbyt:
    {
        uint32_t count;
        ag_err = ag_drv_xumac_rdp_rrbyt_get(parm[1].value.unumber, &count);
        bdmf_session_print(session, "count = %u = 0x%x\n", count, count);
        break;
    }
    case cli_xumac_rdp_rrbyt_upper:
    {
        uint16_t count_u16;
        ag_err = ag_drv_xumac_rdp_rrbyt_upper_get(parm[1].value.unumber, &count_u16);
        bdmf_session_print(session, "count_u16 = %u = 0x%x\n", count_u16, count_u16);
        break;
    }
    case cli_xumac_rdp_mib_cntrl:
    {
        bdmf_boolean rx_cnt_rst;
        bdmf_boolean runt_cnt_rst;
        bdmf_boolean tx_cnt_rst;
        ag_err = ag_drv_xumac_rdp_mib_cntrl_get(parm[1].value.unumber, &rx_cnt_rst, &runt_cnt_rst, &tx_cnt_rst);
        bdmf_session_print(session, "rx_cnt_rst = %u = 0x%x\n", rx_cnt_rst, rx_cnt_rst);
        bdmf_session_print(session, "runt_cnt_rst = %u = 0x%x\n", runt_cnt_rst, runt_cnt_rst);
        bdmf_session_print(session, "tx_cnt_rst = %u = 0x%x\n", tx_cnt_rst, tx_cnt_rst);
        break;
    }
    case cli_xumac_rdp_mib_read_data:
    {
        uint32_t data32;
        ag_err = ag_drv_xumac_rdp_mib_read_data_get(parm[1].value.unumber, &data32);
        bdmf_session_print(session, "data32 = %u = 0x%x\n", data32, data32);
        break;
    }
    case cli_xumac_rdp_mib_write_data:
    {
        uint32_t data32;
        ag_err = ag_drv_xumac_rdp_mib_write_data_get(parm[1].value.unumber, &data32);
        bdmf_session_print(session, "data32 = %u = 0x%x\n", data32, data32);
        break;
    }
    case cli_xumac_rdp_control:
    {
        bdmf_boolean mpd_en;
        uint8_t mseq_len;
        bdmf_boolean psw_en;
        ag_err = ag_drv_xumac_rdp_control_get(parm[1].value.unumber, &mpd_en, &mseq_len, &psw_en);
        bdmf_session_print(session, "mpd_en = %u = 0x%x\n", mpd_en, mpd_en);
        bdmf_session_print(session, "mseq_len = %u = 0x%x\n", mseq_len, mseq_len);
        bdmf_session_print(session, "psw_en = %u = 0x%x\n", psw_en, psw_en);
        break;
    }
    case cli_xumac_rdp_psw_ms:
    {
        uint16_t psw_47_32;
        ag_err = ag_drv_xumac_rdp_psw_ms_get(parm[1].value.unumber, &psw_47_32);
        bdmf_session_print(session, "psw_47_32 = %u = 0x%x\n", psw_47_32, psw_47_32);
        break;
    }
    case cli_xumac_rdp_psw_ls:
    {
        uint32_t psw_31_0;
        ag_err = ag_drv_xumac_rdp_psw_ls_get(parm[1].value.unumber, &psw_31_0);
        bdmf_session_print(session, "psw_31_0 = %u = 0x%x\n", psw_31_0, psw_31_0);
        break;
    }
    case cli_xumac_rdp_control_1:
    {
        xumac_rdp_control_1 control_1;
        ag_err = ag_drv_xumac_rdp_control_1_get(parm[1].value.unumber, &control_1);
        bdmf_session_print(session, "xib_rx_en = %u = 0x%x\n", control_1.xib_rx_en, control_1.xib_rx_en);
        bdmf_session_print(session, "xib_tx_en = %u = 0x%x\n", control_1.xib_tx_en, control_1.xib_tx_en);
        bdmf_session_print(session, "rx_flush_en = %u = 0x%x\n", control_1.rx_flush_en, control_1.rx_flush_en);
        bdmf_session_print(session, "tx_flush_en = %u = 0x%x\n", control_1.tx_flush_en, control_1.tx_flush_en);
        bdmf_session_print(session, "link_down_rst_en = %u = 0x%x\n", control_1.link_down_rst_en, control_1.link_down_rst_en);
        bdmf_session_print(session, "standard_mux_en = %u = 0x%x\n", control_1.standard_mux_en, control_1.standard_mux_en);
        bdmf_session_print(session, "xgmii_sel = %u = 0x%x\n", control_1.xgmii_sel, control_1.xgmii_sel);
        bdmf_session_print(session, "dic_dis = %u = 0x%x\n", control_1.dic_dis, control_1.dic_dis);
        bdmf_session_print(session, "rx_start_threshold = %u = 0x%x\n", control_1.rx_start_threshold, control_1.rx_start_threshold);
        bdmf_session_print(session, "gmii_rx_clk_gate_en = %u = 0x%x\n", control_1.gmii_rx_clk_gate_en, control_1.gmii_rx_clk_gate_en);
        bdmf_session_print(session, "strict_preamble_dis = %u = 0x%x\n", control_1.strict_preamble_dis, control_1.strict_preamble_dis);
        bdmf_session_print(session, "tx_ipg = %u = 0x%x\n", control_1.tx_ipg, control_1.tx_ipg);
        bdmf_session_print(session, "min_rx_ipg = %u = 0x%x\n", control_1.min_rx_ipg, control_1.min_rx_ipg);
        bdmf_session_print(session, "xgmii_sel_ovrd = %u = 0x%x\n", control_1.xgmii_sel_ovrd, control_1.xgmii_sel_ovrd);
        bdmf_session_print(session, "gmii_tx_clk_gate_en = %u = 0x%x\n", control_1.gmii_tx_clk_gate_en, control_1.gmii_tx_clk_gate_en);
        bdmf_session_print(session, "autoconfig_en = %u = 0x%x\n", control_1.autoconfig_en, control_1.autoconfig_en);
        break;
    }
    case cli_xumac_rdp_extended_control:
    {
        uint16_t tx_start_threshold;
        uint16_t tx_xoff_threshold;
        uint16_t tx_xon_threshold;
        bdmf_boolean tx_backpressure_en;
        ag_err = ag_drv_xumac_rdp_extended_control_get(parm[1].value.unumber, &tx_start_threshold, &tx_xoff_threshold, &tx_xon_threshold, &tx_backpressure_en);
        bdmf_session_print(session, "tx_start_threshold = %u = 0x%x\n", tx_start_threshold, tx_start_threshold);
        bdmf_session_print(session, "tx_xoff_threshold = %u = 0x%x\n", tx_xoff_threshold, tx_xoff_threshold);
        bdmf_session_print(session, "tx_xon_threshold = %u = 0x%x\n", tx_xon_threshold, tx_xon_threshold);
        bdmf_session_print(session, "tx_backpressure_en = %u = 0x%x\n", tx_backpressure_en, tx_backpressure_en);
        break;
    }
    case cli_xumac_rdp_tx_idle_stuffing_control:
    {
        uint8_t tx_idle_stuffing_ctrl;
        ag_err = ag_drv_xumac_rdp_tx_idle_stuffing_control_get(parm[1].value.unumber, &tx_idle_stuffing_ctrl);
        bdmf_session_print(session, "tx_idle_stuffing_ctrl = %u = 0x%x\n", tx_idle_stuffing_ctrl, tx_idle_stuffing_ctrl);
        break;
    }
    case cli_xumac_rdp_actual_data_rate:
    {
        uint8_t actual_data_rate;
        ag_err = ag_drv_xumac_rdp_actual_data_rate_get(parm[1].value.unumber, &actual_data_rate);
        bdmf_session_print(session, "actual_data_rate = %u = 0x%x\n", actual_data_rate, actual_data_rate);
        break;
    }
    case cli_xumac_rdp_gmii_clock_swallower_control:
    {
        uint8_t ndiv;
        uint8_t mdiv;
        ag_err = ag_drv_xumac_rdp_gmii_clock_swallower_control_get(parm[1].value.unumber, &ndiv, &mdiv);
        bdmf_session_print(session, "ndiv = %u = 0x%x\n", ndiv, ndiv);
        bdmf_session_print(session, "mdiv = %u = 0x%x\n", mdiv, mdiv);
        break;
    }
    case cli_xumac_rdp_xgmii_data_rate_status:
    {
        uint8_t xgmii_data_rate;
        ag_err = ag_drv_xumac_rdp_xgmii_data_rate_status_get(parm[1].value.unumber, &xgmii_data_rate);
        bdmf_session_print(session, "xgmii_data_rate = %u = 0x%x\n", xgmii_data_rate, xgmii_data_rate);
        break;
    }
    case cli_xumac_rdp_status:
    {
        xumac_rdp_status status;
        ag_err = ag_drv_xumac_rdp_status_get(parm[1].value.unumber, &status);
        bdmf_session_print(session, "rx_fifo_overrun = %u = 0x%x\n", status.rx_fifo_overrun, status.rx_fifo_overrun);
        bdmf_session_print(session, "rx_fifo_underrun = %u = 0x%x\n", status.rx_fifo_underrun, status.rx_fifo_underrun);
        bdmf_session_print(session, "tx_fifo_underrun = %u = 0x%x\n", status.tx_fifo_underrun, status.tx_fifo_underrun);
        bdmf_session_print(session, "tx_fifo_overrun = %u = 0x%x\n", status.tx_fifo_overrun, status.tx_fifo_overrun);
        bdmf_session_print(session, "rx_fault_status = %u = 0x%x\n", status.rx_fault_status, status.rx_fault_status);
        break;
    }
    case cli_xumac_rdp_rx_discard_packet_counter:
    {
        uint32_t pkt_count;
        ag_err = ag_drv_xumac_rdp_rx_discard_packet_counter_get(parm[1].value.unumber, &pkt_count);
        bdmf_session_print(session, "pkt_count = %u = 0x%x\n", pkt_count, pkt_count);
        break;
    }
    case cli_xumac_rdp_tx_discard_packet_counter:
    {
        uint32_t pkt_count;
        ag_err = ag_drv_xumac_rdp_tx_discard_packet_counter_get(parm[1].value.unumber, &pkt_count);
        bdmf_session_print(session, "pkt_count = %u = 0x%x\n", pkt_count, pkt_count);
        break;
    }
    case cli_xumac_rdp_rev:
    {
        uint16_t sys_port_rev;
        ag_err = ag_drv_xumac_rdp_rev_get(parm[1].value.unumber, &sys_port_rev);
        bdmf_session_print(session, "sys_port_rev = %u = 0x%x\n", sys_port_rev, sys_port_rev);
        break;
    }
    case cli_xumac_rdp_umac_rxerr_mask:
    {
        uint32_t mac_rxerr_mask;
        ag_err = ag_drv_xumac_rdp_umac_rxerr_mask_get(parm[1].value.unumber, &mac_rxerr_mask);
        bdmf_session_print(session, "mac_rxerr_mask = %u = 0x%x\n", mac_rxerr_mask, mac_rxerr_mask);
        break;
    }
    case cli_xumac_rdp_mib_max_pkt_size:
    {
        uint16_t max_pkt_size;
        ag_err = ag_drv_xumac_rdp_mib_max_pkt_size_get(parm[1].value.unumber, &max_pkt_size);
        bdmf_session_print(session, "max_pkt_size = %u = 0x%x\n", max_pkt_size, max_pkt_size);
        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_xumac_rdp_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    uint8_t xumac_id = parm[1].value.unumber;
    bdmf_error_t ag_err = BDMF_ERR_OK;
    uint32_t test_success_cnt = 0;
    uint32_t test_failure_cnt = 0;

    {
        bdmf_boolean hd_fc_ena = gtmv(m, 1);
        bdmf_boolean hd_fc_bkoff_ok = gtmv(m, 1);
        uint8_t ipg_config_rx = gtmv(m, 5);
        bdmf_session_print(session, "ag_drv_xumac_rdp_ipg_hd_bkp_cntl_set(%u %u %u %u)\n", xumac_id,
            hd_fc_ena, hd_fc_bkoff_ok, ipg_config_rx);
        ag_err = ag_drv_xumac_rdp_ipg_hd_bkp_cntl_set(xumac_id, hd_fc_ena, hd_fc_bkoff_ok, ipg_config_rx);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_ipg_hd_bkp_cntl_get(xumac_id, &hd_fc_ena, &hd_fc_bkoff_ok, &ipg_config_rx);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_ipg_hd_bkp_cntl_get(%u %u %u %u)\n", xumac_id,
                hd_fc_ena, hd_fc_bkoff_ok, ipg_config_rx);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (hd_fc_ena != gtmv(m, 1) || hd_fc_bkoff_ok != gtmv(m, 1) || ipg_config_rx != gtmv(m, 5))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        xumac_rdp_command_config command_config = {.tx_ena = gtmv(m, 1), .rx_ena = gtmv(m, 1), .eth_speed = gtmv(m, 2), .promis_en = gtmv(m, 1), .pad_en = gtmv(m, 1), .crc_fwd = gtmv(m, 1), .pause_fwd = gtmv(m, 1), .pause_ignore = gtmv(m, 1), .tx_addr_ins = gtmv(m, 1), .hd_ena = gtmv(m, 1), .rx_low_latency_en = gtmv(m, 1), .overflow_en = gtmv(m, 1), .sw_reset = gtmv(m, 1), .fcs_corrupt_urun_en = gtmv(m, 1), .loop_ena = gtmv(m, 1), .mac_loop_con = gtmv(m, 1), .sw_override_tx = gtmv(m, 1), .sw_override_rx = gtmv(m, 1), .oob_efc_mode = gtmv(m, 1), .bypass_oob_efc_synchronizer = gtmv(m, 1), .en_internal_tx_crs = gtmv(m, 1), .ena_ext_config = gtmv(m, 1), .cntl_frm_ena = gtmv(m, 1), .no_lgth_check = gtmv(m, 1), .line_loopback = gtmv(m, 1), .fd_tx_urun_fix_en = gtmv(m, 1), .ignore_tx_pause = gtmv(m, 1), .oob_efc_disab = gtmv(m, 1), .runt_filter_dis = gtmv(m, 1), .eth_speed_bit2 = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_xumac_rdp_command_config_set(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", xumac_id,
            command_config.tx_ena, command_config.rx_ena, command_config.eth_speed, command_config.promis_en, 
            command_config.pad_en, command_config.crc_fwd, command_config.pause_fwd, command_config.pause_ignore, 
            command_config.tx_addr_ins, command_config.hd_ena, command_config.rx_low_latency_en, command_config.overflow_en, 
            command_config.sw_reset, command_config.fcs_corrupt_urun_en, command_config.loop_ena, command_config.mac_loop_con, 
            command_config.sw_override_tx, command_config.sw_override_rx, command_config.oob_efc_mode, command_config.bypass_oob_efc_synchronizer, 
            command_config.en_internal_tx_crs, command_config.ena_ext_config, command_config.cntl_frm_ena, command_config.no_lgth_check, 
            command_config.line_loopback, command_config.fd_tx_urun_fix_en, command_config.ignore_tx_pause, command_config.oob_efc_disab, 
            command_config.runt_filter_dis, command_config.eth_speed_bit2);
        ag_err = ag_drv_xumac_rdp_command_config_set(xumac_id, &command_config);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_command_config_get(xumac_id, &command_config);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_command_config_get(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", xumac_id,
                command_config.tx_ena, command_config.rx_ena, command_config.eth_speed, command_config.promis_en, 
                command_config.pad_en, command_config.crc_fwd, command_config.pause_fwd, command_config.pause_ignore, 
                command_config.tx_addr_ins, command_config.hd_ena, command_config.rx_low_latency_en, command_config.overflow_en, 
                command_config.sw_reset, command_config.fcs_corrupt_urun_en, command_config.loop_ena, command_config.mac_loop_con, 
                command_config.sw_override_tx, command_config.sw_override_rx, command_config.oob_efc_mode, command_config.bypass_oob_efc_synchronizer, 
                command_config.en_internal_tx_crs, command_config.ena_ext_config, command_config.cntl_frm_ena, command_config.no_lgth_check, 
                command_config.line_loopback, command_config.fd_tx_urun_fix_en, command_config.ignore_tx_pause, command_config.oob_efc_disab, 
                command_config.runt_filter_dis, command_config.eth_speed_bit2);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (command_config.tx_ena != gtmv(m, 1) || command_config.rx_ena != gtmv(m, 1) || command_config.eth_speed != gtmv(m, 2) || command_config.promis_en != gtmv(m, 1) || command_config.pad_en != gtmv(m, 1) || command_config.crc_fwd != gtmv(m, 1) || command_config.pause_fwd != gtmv(m, 1) || command_config.pause_ignore != gtmv(m, 1) || command_config.tx_addr_ins != gtmv(m, 1) || command_config.hd_ena != gtmv(m, 1) || command_config.rx_low_latency_en != gtmv(m, 1) || command_config.overflow_en != gtmv(m, 1) || command_config.sw_reset != gtmv(m, 1) || command_config.fcs_corrupt_urun_en != gtmv(m, 1) || command_config.loop_ena != gtmv(m, 1) || command_config.mac_loop_con != gtmv(m, 1) || command_config.sw_override_tx != gtmv(m, 1) || command_config.sw_override_rx != gtmv(m, 1) || command_config.oob_efc_mode != gtmv(m, 1) || command_config.bypass_oob_efc_synchronizer != gtmv(m, 1) || command_config.en_internal_tx_crs != gtmv(m, 1) || command_config.ena_ext_config != gtmv(m, 1) || command_config.cntl_frm_ena != gtmv(m, 1) || command_config.no_lgth_check != gtmv(m, 1) || command_config.line_loopback != gtmv(m, 1) || command_config.fd_tx_urun_fix_en != gtmv(m, 1) || command_config.ignore_tx_pause != gtmv(m, 1) || command_config.oob_efc_disab != gtmv(m, 1) || command_config.runt_filter_dis != gtmv(m, 1) || command_config.eth_speed_bit2 != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t mac_addr0 = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_mac_0_set(%u %u)\n", xumac_id,
            mac_addr0);
        ag_err = ag_drv_xumac_rdp_mac_0_set(xumac_id, mac_addr0);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_mac_0_get(xumac_id, &mac_addr0);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_mac_0_get(%u %u)\n", xumac_id,
                mac_addr0);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (mac_addr0 != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t mac_addr1 = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_xumac_rdp_mac_1_set(%u %u)\n", xumac_id,
            mac_addr1);
        ag_err = ag_drv_xumac_rdp_mac_1_set(xumac_id, mac_addr1);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_mac_1_get(xumac_id, &mac_addr1);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_mac_1_get(%u %u)\n", xumac_id,
                mac_addr1);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (mac_addr1 != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t maxfr = gtmv(m, 14);
        bdmf_session_print(session, "ag_drv_xumac_rdp_frm_length_set(%u %u)\n", xumac_id,
            maxfr);
        ag_err = ag_drv_xumac_rdp_frm_length_set(xumac_id, maxfr);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_frm_length_get(xumac_id, &maxfr);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_frm_length_get(%u %u)\n", xumac_id,
                maxfr);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (maxfr != gtmv(m, 14))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t pause_quant = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_xumac_rdp_pause_quant_set(%u %u)\n", xumac_id,
            pause_quant);
        ag_err = ag_drv_xumac_rdp_pause_quant_set(xumac_id, pause_quant);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_pause_quant_get(xumac_id, &pause_quant);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_pause_quant_get(%u %u)\n", xumac_id,
                pause_quant);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pause_quant != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t tsts_seq_id = gtmv(m, 16);
        bdmf_boolean tsts_valid = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_tx_ts_seq_id_get(xumac_id, &tsts_seq_id, &tsts_valid);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_tx_ts_seq_id_get(%u %u %u)\n", xumac_id,
                tsts_seq_id, tsts_valid);
        }
    }

    {
        uint8_t sfd_offset = gtmv(m, 4);
        bdmf_session_print(session, "ag_drv_xumac_rdp_sfd_offset_set(%u %u)\n", xumac_id,
            sfd_offset);
        ag_err = ag_drv_xumac_rdp_sfd_offset_set(xumac_id, sfd_offset);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_sfd_offset_get(xumac_id, &sfd_offset);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_sfd_offset_get(%u %u)\n", xumac_id,
                sfd_offset);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (sfd_offset != gtmv(m, 4))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        xumac_rdp_mac_mode mac_mode = {.mac_speed = gtmv(m, 2), .mac_duplex = gtmv(m, 1), .mac_rx_pause = gtmv(m, 1), .mac_tx_pause = gtmv(m, 1), .link_status = gtmv(m, 1), .mac_speed_bit2 = gtmv(m, 1)};
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_mac_mode_get(xumac_id, &mac_mode);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_mac_mode_get(%u %u %u %u %u %u %u)\n", xumac_id,
                mac_mode.mac_speed, mac_mode.mac_duplex, mac_mode.mac_rx_pause, mac_mode.mac_tx_pause, 
                mac_mode.link_status, mac_mode.mac_speed_bit2);
        }
    }

    {
        uint16_t frm_tag_0 = gtmv(m, 16);
        bdmf_boolean config_outer_tpid_enable = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_xumac_rdp_tag_0_set(%u %u %u)\n", xumac_id,
            frm_tag_0, config_outer_tpid_enable);
        ag_err = ag_drv_xumac_rdp_tag_0_set(xumac_id, frm_tag_0, config_outer_tpid_enable);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_tag_0_get(xumac_id, &frm_tag_0, &config_outer_tpid_enable);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_tag_0_get(%u %u %u)\n", xumac_id,
                frm_tag_0, config_outer_tpid_enable);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (frm_tag_0 != gtmv(m, 16) || config_outer_tpid_enable != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t frm_tag_1 = gtmv(m, 16);
        bdmf_boolean config_inner_tpid_enable = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_xumac_rdp_tag_1_set(%u %u %u)\n", xumac_id,
            frm_tag_1, config_inner_tpid_enable);
        ag_err = ag_drv_xumac_rdp_tag_1_set(xumac_id, frm_tag_1, config_inner_tpid_enable);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_tag_1_get(xumac_id, &frm_tag_1, &config_inner_tpid_enable);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_tag_1_get(%u %u %u)\n", xumac_id,
                frm_tag_1, config_inner_tpid_enable);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (frm_tag_1 != gtmv(m, 16) || config_inner_tpid_enable != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t scale_value = gtmv(m, 16);
        bdmf_boolean scale_control = gtmv(m, 1);
        bdmf_boolean scale_fix = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_xumac_rdp_rx_pause_quanta_scale_set(%u %u %u %u)\n", xumac_id,
            scale_value, scale_control, scale_fix);
        ag_err = ag_drv_xumac_rdp_rx_pause_quanta_scale_set(xumac_id, scale_value, scale_control, scale_fix);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_rx_pause_quanta_scale_get(xumac_id, &scale_value, &scale_control, &scale_fix);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_rx_pause_quanta_scale_get(%u %u %u %u)\n", xumac_id,
                scale_value, scale_control, scale_fix);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (scale_value != gtmv(m, 16) || scale_control != gtmv(m, 1) || scale_fix != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t tx_preamble = gtmv(m, 3);
        bdmf_session_print(session, "ag_drv_xumac_rdp_tx_preamble_set(%u %u)\n", xumac_id,
            tx_preamble);
        ag_err = ag_drv_xumac_rdp_tx_preamble_set(xumac_id, tx_preamble);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_tx_preamble_get(xumac_id, &tx_preamble);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_tx_preamble_get(%u %u)\n", xumac_id,
                tx_preamble);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (tx_preamble != gtmv(m, 3))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t tx_ipg_length = gtmv(m, 7);
        uint8_t tx_min_pkt_size = gtmv(m, 7);
        bdmf_session_print(session, "ag_drv_xumac_rdp_tx_ipg_length_set(%u %u %u)\n", xumac_id,
            tx_ipg_length, tx_min_pkt_size);
        ag_err = ag_drv_xumac_rdp_tx_ipg_length_set(xumac_id, tx_ipg_length, tx_min_pkt_size);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_tx_ipg_length_get(xumac_id, &tx_ipg_length, &tx_min_pkt_size);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_tx_ipg_length_get(%u %u %u)\n", xumac_id,
                tx_ipg_length, tx_min_pkt_size);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (tx_ipg_length != gtmv(m, 7) || tx_min_pkt_size != gtmv(m, 7))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t pfc_xoff_timer = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_xumac_rdp_pfc_xoff_timer_set(%u %u)\n", xumac_id,
            pfc_xoff_timer);
        ag_err = ag_drv_xumac_rdp_pfc_xoff_timer_set(xumac_id, pfc_xoff_timer);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_pfc_xoff_timer_get(xumac_id, &pfc_xoff_timer);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_pfc_xoff_timer_get(%u %u)\n", xumac_id,
                pfc_xoff_timer);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pfc_xoff_timer != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        xumac_rdp_umac_eee_ctrl umac_eee_ctrl = {.eee_en = gtmv(m, 1), .rx_fifo_check = gtmv(m, 1), .eee_txclk_dis = gtmv(m, 1), .dis_eee_10m = gtmv(m, 1), .lp_idle_prediction_mode = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_xumac_rdp_umac_eee_ctrl_set(%u %u %u %u %u %u)\n", xumac_id,
            umac_eee_ctrl.eee_en, umac_eee_ctrl.rx_fifo_check, umac_eee_ctrl.eee_txclk_dis, umac_eee_ctrl.dis_eee_10m, 
            umac_eee_ctrl.lp_idle_prediction_mode);
        ag_err = ag_drv_xumac_rdp_umac_eee_ctrl_set(xumac_id, &umac_eee_ctrl);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_umac_eee_ctrl_get(xumac_id, &umac_eee_ctrl);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_umac_eee_ctrl_get(%u %u %u %u %u %u)\n", xumac_id,
                umac_eee_ctrl.eee_en, umac_eee_ctrl.rx_fifo_check, umac_eee_ctrl.eee_txclk_dis, umac_eee_ctrl.dis_eee_10m, 
                umac_eee_ctrl.lp_idle_prediction_mode);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (umac_eee_ctrl.eee_en != gtmv(m, 1) || umac_eee_ctrl.rx_fifo_check != gtmv(m, 1) || umac_eee_ctrl.eee_txclk_dis != gtmv(m, 1) || umac_eee_ctrl.dis_eee_10m != gtmv(m, 1) || umac_eee_ctrl.lp_idle_prediction_mode != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t mii_eee_lpi_timer = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_mii_eee_delay_entry_timer_set(%u %u)\n", xumac_id,
            mii_eee_lpi_timer);
        ag_err = ag_drv_xumac_rdp_mii_eee_delay_entry_timer_set(xumac_id, mii_eee_lpi_timer);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_mii_eee_delay_entry_timer_get(xumac_id, &mii_eee_lpi_timer);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_mii_eee_delay_entry_timer_get(%u %u)\n", xumac_id,
                mii_eee_lpi_timer);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (mii_eee_lpi_timer != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t gmii_eee_lpi_timer = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gmii_eee_delay_entry_timer_set(%u %u)\n", xumac_id,
            gmii_eee_lpi_timer);
        ag_err = ag_drv_xumac_rdp_gmii_eee_delay_entry_timer_set(xumac_id, gmii_eee_lpi_timer);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gmii_eee_delay_entry_timer_get(xumac_id, &gmii_eee_lpi_timer);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gmii_eee_delay_entry_timer_get(%u %u)\n", xumac_id,
                gmii_eee_lpi_timer);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (gmii_eee_lpi_timer != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t eee_ref_count = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_xumac_rdp_umac_eee_ref_count_set(%u %u)\n", xumac_id,
            eee_ref_count);
        ag_err = ag_drv_xumac_rdp_umac_eee_ref_count_set(xumac_id, eee_ref_count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_umac_eee_ref_count_get(xumac_id, &eee_ref_count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_umac_eee_ref_count_get(%u %u)\n", xumac_id,
                eee_ref_count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (eee_ref_count != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t adjust = gtmv(m, 9);
        bdmf_boolean en_1588 = gtmv(m, 1);
        bdmf_boolean auto_adjust = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_xumac_rdp_umac_timestamp_adjust_set(%u %u %u %u)\n", xumac_id,
            adjust, en_1588, auto_adjust);
        ag_err = ag_drv_xumac_rdp_umac_timestamp_adjust_set(xumac_id, adjust, en_1588, auto_adjust);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_umac_timestamp_adjust_get(xumac_id, &adjust, &en_1588, &auto_adjust);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_umac_timestamp_adjust_get(%u %u %u %u)\n", xumac_id,
                adjust, en_1588, auto_adjust);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (adjust != gtmv(m, 9) || en_1588 != gtmv(m, 1) || auto_adjust != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean rx_ipg_inval = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_xumac_rdp_umac_rx_pkt_drop_status_set(%u %u)\n", xumac_id,
            rx_ipg_inval);
        ag_err = ag_drv_xumac_rdp_umac_rx_pkt_drop_status_set(xumac_id, rx_ipg_inval);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_umac_rx_pkt_drop_status_get(xumac_id, &rx_ipg_inval);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_umac_rx_pkt_drop_status_get(%u %u)\n", xumac_id,
                rx_ipg_inval);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (rx_ipg_inval != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t threshold_value = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_xumac_rdp_umac_symmetric_idle_threshold_set(%u %u)\n", xumac_id,
            threshold_value);
        ag_err = ag_drv_xumac_rdp_umac_symmetric_idle_threshold_set(xumac_id, threshold_value);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_umac_symmetric_idle_threshold_get(xumac_id, &threshold_value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_umac_symmetric_idle_threshold_get(%u %u)\n", xumac_id,
                threshold_value);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (threshold_value != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t mii_eee_wake_timer = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_xumac_rdp_mii_eee_wake_timer_set(%u %u)\n", xumac_id,
            mii_eee_wake_timer);
        ag_err = ag_drv_xumac_rdp_mii_eee_wake_timer_set(xumac_id, mii_eee_wake_timer);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_mii_eee_wake_timer_get(xumac_id, &mii_eee_wake_timer);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_mii_eee_wake_timer_get(%u %u)\n", xumac_id,
                mii_eee_wake_timer);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (mii_eee_wake_timer != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t gmii_eee_wake_timer = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gmii_eee_wake_timer_set(%u %u)\n", xumac_id,
            gmii_eee_wake_timer);
        ag_err = ag_drv_xumac_rdp_gmii_eee_wake_timer_set(xumac_id, gmii_eee_wake_timer);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gmii_eee_wake_timer_get(xumac_id, &gmii_eee_wake_timer);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gmii_eee_wake_timer_get(%u %u)\n", xumac_id,
                gmii_eee_wake_timer);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (gmii_eee_wake_timer != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t patch = gtmv(m, 8);
        uint8_t revision_id_minor = gtmv(m, 8);
        uint8_t revision_id_major = gtmv(m, 8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_umac_rev_id_get(xumac_id, &patch, &revision_id_minor, &revision_id_major);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_umac_rev_id_get(%u %u %u %u)\n", xumac_id,
                patch, revision_id_minor, revision_id_major);
        }
    }

    {
        uint32_t gmii_2p5g_eee_lpi_timer = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gmii_2p5g_eee_delay_entry_timer_set(%u %u)\n", xumac_id,
            gmii_2p5g_eee_lpi_timer);
        ag_err = ag_drv_xumac_rdp_gmii_2p5g_eee_delay_entry_timer_set(xumac_id, gmii_2p5g_eee_lpi_timer);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gmii_2p5g_eee_delay_entry_timer_get(xumac_id, &gmii_2p5g_eee_lpi_timer);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gmii_2p5g_eee_delay_entry_timer_get(%u %u)\n", xumac_id,
                gmii_2p5g_eee_lpi_timer);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (gmii_2p5g_eee_lpi_timer != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t gmii_5g_eee_lpi_timer = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gmii_5g_eee_delay_entry_timer_set(%u %u)\n", xumac_id,
            gmii_5g_eee_lpi_timer);
        ag_err = ag_drv_xumac_rdp_gmii_5g_eee_delay_entry_timer_set(xumac_id, gmii_5g_eee_lpi_timer);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gmii_5g_eee_delay_entry_timer_get(xumac_id, &gmii_5g_eee_lpi_timer);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gmii_5g_eee_delay_entry_timer_get(%u %u)\n", xumac_id,
                gmii_5g_eee_lpi_timer);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (gmii_5g_eee_lpi_timer != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t gmii_10g_eee_lpi_timer = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gmii_10g_eee_delay_entry_timer_set(%u %u)\n", xumac_id,
            gmii_10g_eee_lpi_timer);
        ag_err = ag_drv_xumac_rdp_gmii_10g_eee_delay_entry_timer_set(xumac_id, gmii_10g_eee_lpi_timer);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gmii_10g_eee_delay_entry_timer_get(xumac_id, &gmii_10g_eee_lpi_timer);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gmii_10g_eee_delay_entry_timer_get(%u %u)\n", xumac_id,
                gmii_10g_eee_lpi_timer);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (gmii_10g_eee_lpi_timer != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t gmii_2p5g_eee_wake_timer = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gmii_2p5g_eee_wake_timer_set(%u %u)\n", xumac_id,
            gmii_2p5g_eee_wake_timer);
        ag_err = ag_drv_xumac_rdp_gmii_2p5g_eee_wake_timer_set(xumac_id, gmii_2p5g_eee_wake_timer);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gmii_2p5g_eee_wake_timer_get(xumac_id, &gmii_2p5g_eee_wake_timer);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gmii_2p5g_eee_wake_timer_get(%u %u)\n", xumac_id,
                gmii_2p5g_eee_wake_timer);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (gmii_2p5g_eee_wake_timer != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t gmii_5g_eee_wake_timer = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gmii_5g_eee_wake_timer_set(%u %u)\n", xumac_id,
            gmii_5g_eee_wake_timer);
        ag_err = ag_drv_xumac_rdp_gmii_5g_eee_wake_timer_set(xumac_id, gmii_5g_eee_wake_timer);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gmii_5g_eee_wake_timer_get(xumac_id, &gmii_5g_eee_wake_timer);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gmii_5g_eee_wake_timer_get(%u %u)\n", xumac_id,
                gmii_5g_eee_wake_timer);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (gmii_5g_eee_wake_timer != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t gmii_10g_eee_wake_timer = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gmii_10g_eee_wake_timer_set(%u %u)\n", xumac_id,
            gmii_10g_eee_wake_timer);
        ag_err = ag_drv_xumac_rdp_gmii_10g_eee_wake_timer_set(xumac_id, gmii_10g_eee_wake_timer);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gmii_10g_eee_wake_timer_get(xumac_id, &gmii_10g_eee_wake_timer);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gmii_10g_eee_wake_timer_get(%u %u)\n", xumac_id,
                gmii_10g_eee_wake_timer);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (gmii_10g_eee_wake_timer != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t active_eee_lpi_timer = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_active_eee_delay_entry_timer_get(xumac_id, &active_eee_lpi_timer);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_active_eee_delay_entry_timer_get(%u %u)\n", xumac_id,
                active_eee_lpi_timer);
        }
    }

    {
        uint16_t active_eee_wake_time = gtmv(m, 16);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_active_eee_wake_timer_get(xumac_id, &active_eee_wake_time);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_active_eee_wake_timer_get(%u %u)\n", xumac_id,
                active_eee_wake_time);
        }
    }

    {
        uint16_t pfc_eth_type = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_xumac_rdp_mac_pfc_type_set(%u %u)\n", xumac_id,
            pfc_eth_type);
        ag_err = ag_drv_xumac_rdp_mac_pfc_type_set(xumac_id, pfc_eth_type);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_mac_pfc_type_get(xumac_id, &pfc_eth_type);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_mac_pfc_type_get(%u %u)\n", xumac_id,
                pfc_eth_type);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pfc_eth_type != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t pfc_opcode = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_xumac_rdp_mac_pfc_opcode_set(%u %u)\n", xumac_id,
            pfc_opcode);
        ag_err = ag_drv_xumac_rdp_mac_pfc_opcode_set(xumac_id, pfc_opcode);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_mac_pfc_opcode_get(xumac_id, &pfc_opcode);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_mac_pfc_opcode_get(%u %u)\n", xumac_id,
                pfc_opcode);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pfc_opcode != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t pfc_macda_0 = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_mac_pfc_da_0_set(%u %u)\n", xumac_id,
            pfc_macda_0);
        ag_err = ag_drv_xumac_rdp_mac_pfc_da_0_set(xumac_id, pfc_macda_0);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_mac_pfc_da_0_get(xumac_id, &pfc_macda_0);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_mac_pfc_da_0_get(%u %u)\n", xumac_id,
                pfc_macda_0);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pfc_macda_0 != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t pfc_macda_1 = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_xumac_rdp_mac_pfc_da_1_set(%u %u)\n", xumac_id,
            pfc_macda_1);
        ag_err = ag_drv_xumac_rdp_mac_pfc_da_1_set(xumac_id, pfc_macda_1);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_mac_pfc_da_1_get(xumac_id, &pfc_macda_1);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_mac_pfc_da_1_get(%u %u)\n", xumac_id,
                pfc_macda_1);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pfc_macda_1 != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t macsec_prog_tx_crc = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_macsec_prog_tx_crc_set(%u %u)\n", xumac_id,
            macsec_prog_tx_crc);
        ag_err = ag_drv_xumac_rdp_macsec_prog_tx_crc_set(xumac_id, macsec_prog_tx_crc);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_macsec_prog_tx_crc_get(xumac_id, &macsec_prog_tx_crc);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_macsec_prog_tx_crc_get(%u %u)\n", xumac_id,
                macsec_prog_tx_crc);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (macsec_prog_tx_crc != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean tx_launch_en = gtmv(m, 1);
        bdmf_boolean tx_crc_corupt_en = gtmv(m, 1);
        bdmf_boolean tx_crc_program = gtmv(m, 1);
        bdmf_boolean dis_pause_data_var_ipg = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_xumac_rdp_macsec_cntrl_set(%u %u %u %u %u)\n", xumac_id,
            tx_launch_en, tx_crc_corupt_en, tx_crc_program, dis_pause_data_var_ipg);
        ag_err = ag_drv_xumac_rdp_macsec_cntrl_set(xumac_id, tx_launch_en, tx_crc_corupt_en, tx_crc_program, dis_pause_data_var_ipg);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_macsec_cntrl_get(xumac_id, &tx_launch_en, &tx_crc_corupt_en, &tx_crc_program, &dis_pause_data_var_ipg);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_macsec_cntrl_get(%u %u %u %u %u)\n", xumac_id,
                tx_launch_en, tx_crc_corupt_en, tx_crc_program, dis_pause_data_var_ipg);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (tx_launch_en != gtmv(m, 1) || tx_crc_corupt_en != gtmv(m, 1) || tx_crc_program != gtmv(m, 1) || dis_pause_data_var_ipg != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean tx_ts_fifo_full = gtmv(m, 1);
        bdmf_boolean tx_ts_fifo_empty = gtmv(m, 1);
        uint8_t word_avail = gtmv(m, 3);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_ts_status_get(xumac_id, &tx_ts_fifo_full, &tx_ts_fifo_empty, &word_avail);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_ts_status_get(%u %u %u %u)\n", xumac_id,
                tx_ts_fifo_full, tx_ts_fifo_empty, word_avail);
        }
    }

    {
        uint32_t tx_ts_data = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_tx_ts_data_get(xumac_id, &tx_ts_data);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_tx_ts_data_get(%u %u)\n", xumac_id,
                tx_ts_data);
        }
    }

    {
        uint32_t refresh_timer = gtmv(m, 17);
        bdmf_boolean enable = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_xumac_rdp_pause_refresh_ctrl_set(%u %u %u)\n", xumac_id,
            refresh_timer, enable);
        ag_err = ag_drv_xumac_rdp_pause_refresh_ctrl_set(xumac_id, refresh_timer, enable);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_pause_refresh_ctrl_get(xumac_id, &refresh_timer, &enable);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_pause_refresh_ctrl_get(%u %u %u)\n", xumac_id,
                refresh_timer, enable);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (refresh_timer != gtmv(m, 17) || enable != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean flush = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_xumac_rdp_flush_control_set(%u %u)\n", xumac_id,
            flush);
        ag_err = ag_drv_xumac_rdp_flush_control_set(xumac_id, flush);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_flush_control_get(xumac_id, &flush);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_flush_control_get(%u %u)\n", xumac_id,
                flush);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (flush != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean rxfifo_underrun = gtmv(m, 1);
        bdmf_boolean rxfifo_overrun = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_rxfifo_stat_get(xumac_id, &rxfifo_underrun, &rxfifo_overrun);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_rxfifo_stat_get(%u %u %u)\n", xumac_id,
                rxfifo_underrun, rxfifo_overrun);
        }
    }

    {
        bdmf_boolean txfifo_underrun = gtmv(m, 1);
        bdmf_boolean txfifo_overrun = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_txfifo_stat_get(xumac_id, &txfifo_underrun, &txfifo_overrun);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_txfifo_stat_get(%u %u %u)\n", xumac_id,
                txfifo_underrun, txfifo_overrun);
        }
    }

    {
        xumac_rdp_mac_pfc_ctrl mac_pfc_ctrl = {.pfc_tx_enbl = gtmv(m, 1), .pfc_rx_enbl = gtmv(m, 1), .force_pfc_xon = gtmv(m, 1), .rx_pass_pfc_frm = gtmv(m, 1), .pfc_stats_en = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_xumac_rdp_mac_pfc_ctrl_set(%u %u %u %u %u %u)\n", xumac_id,
            mac_pfc_ctrl.pfc_tx_enbl, mac_pfc_ctrl.pfc_rx_enbl, mac_pfc_ctrl.force_pfc_xon, mac_pfc_ctrl.rx_pass_pfc_frm, 
            mac_pfc_ctrl.pfc_stats_en);
        ag_err = ag_drv_xumac_rdp_mac_pfc_ctrl_set(xumac_id, &mac_pfc_ctrl);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_mac_pfc_ctrl_get(xumac_id, &mac_pfc_ctrl);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_mac_pfc_ctrl_get(%u %u %u %u %u %u)\n", xumac_id,
                mac_pfc_ctrl.pfc_tx_enbl, mac_pfc_ctrl.pfc_rx_enbl, mac_pfc_ctrl.force_pfc_xon, mac_pfc_ctrl.rx_pass_pfc_frm, 
                mac_pfc_ctrl.pfc_stats_en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (mac_pfc_ctrl.pfc_tx_enbl != gtmv(m, 1) || mac_pfc_ctrl.pfc_rx_enbl != gtmv(m, 1) || mac_pfc_ctrl.force_pfc_xon != gtmv(m, 1) || mac_pfc_ctrl.rx_pass_pfc_frm != gtmv(m, 1) || mac_pfc_ctrl.pfc_stats_en != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean pfc_refresh_en = gtmv(m, 1);
        uint16_t pfc_refresh_timer = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_xumac_rdp_mac_pfc_refresh_ctrl_set(%u %u %u)\n", xumac_id,
            pfc_refresh_en, pfc_refresh_timer);
        ag_err = ag_drv_xumac_rdp_mac_pfc_refresh_ctrl_set(xumac_id, pfc_refresh_en, pfc_refresh_timer);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_mac_pfc_refresh_ctrl_get(xumac_id, &pfc_refresh_en, &pfc_refresh_timer);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_mac_pfc_refresh_ctrl_get(%u %u %u)\n", xumac_id,
                pfc_refresh_en, pfc_refresh_timer);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pfc_refresh_en != gtmv(m, 1) || pfc_refresh_timer != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gr64_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_gr64_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gr64_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gr64_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gr64_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_gr64_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gr64_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gr64_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gr127_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_gr127_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gr127_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gr127_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gr127_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_gr127_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gr127_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gr127_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gr255_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_gr255_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gr255_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gr255_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gr255_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_gr255_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gr255_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gr255_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gr511_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_gr511_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gr511_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gr511_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gr511_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_gr511_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gr511_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gr511_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gr1023_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_gr1023_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gr1023_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gr1023_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gr1023_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_gr1023_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gr1023_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gr1023_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gr1518_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_gr1518_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gr1518_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gr1518_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gr1518_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_gr1518_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gr1518_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gr1518_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_grmgv_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_grmgv_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_grmgv_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_grmgv_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_grmgv_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_grmgv_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_grmgv_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_grmgv_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gr2047_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_gr2047_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gr2047_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gr2047_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gr2047_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_gr2047_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gr2047_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gr2047_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gr4095_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_gr4095_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gr4095_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gr4095_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gr4095_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_gr4095_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gr4095_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gr4095_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gr9216_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_gr9216_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gr9216_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gr9216_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gr9216_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_gr9216_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gr9216_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gr9216_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_grpkt_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_grpkt_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_grpkt_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_grpkt_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_grpkt_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_grpkt_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_grpkt_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_grpkt_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_grbyt_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_grbyt_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_grbyt_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_grbyt_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t count_u16 = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_xumac_rdp_grbyt_upper_set(%u %u)\n", xumac_id,
            count_u16);
        ag_err = ag_drv_xumac_rdp_grbyt_upper_set(xumac_id, count_u16);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_grbyt_upper_get(xumac_id, &count_u16);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_grbyt_upper_get(%u %u)\n", xumac_id,
                count_u16);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u16 != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_grmca_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_grmca_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_grmca_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_grmca_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_grmca_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_grmca_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_grmca_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_grmca_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_grbca_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_grbca_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_grbca_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_grbca_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_grbca_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_grbca_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_grbca_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_grbca_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_grfcs_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_grfcs_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_grfcs_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_grfcs_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_grfcs_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_grfcs_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_grfcs_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_grfcs_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_grxcf_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_grxcf_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_grxcf_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_grxcf_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_grxcf_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_grxcf_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_grxcf_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_grxcf_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_grxpf_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_grxpf_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_grxpf_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_grxpf_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_grxpf_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_grxpf_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_grxpf_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_grxpf_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_grxuo_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_grxuo_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_grxuo_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_grxuo_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_grxuo_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_grxuo_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_grxuo_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_grxuo_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_graln_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_graln_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_graln_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_graln_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_graln_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_graln_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_graln_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_graln_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_grflr_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_grflr_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_grflr_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_grflr_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_grflr_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_grflr_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_grflr_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_grflr_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_grcde_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_grcde_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_grcde_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_grcde_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_grcde_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_grcde_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_grcde_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_grcde_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_grfcr_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_grfcr_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_grfcr_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_grfcr_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_grfcr_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_grfcr_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_grfcr_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_grfcr_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_grovr_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_grovr_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_grovr_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_grovr_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_grovr_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_grovr_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_grovr_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_grovr_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_grjbr_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_grjbr_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_grjbr_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_grjbr_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_grjbr_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_grjbr_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_grjbr_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_grjbr_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_grmtue_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_grmtue_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_grmtue_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_grmtue_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_grmtue_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_grmtue_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_grmtue_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_grmtue_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_grpok_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_grpok_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_grpok_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_grpok_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_grpok_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_grpok_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_grpok_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_grpok_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gruc_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_gruc_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gruc_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gruc_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gruc_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_gruc_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gruc_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gruc_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_grppp_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_grppp_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_grppp_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_grppp_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_grppp_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_grppp_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_grppp_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_grppp_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_grcrc_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_grcrc_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_grcrc_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_grcrc_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_grcrc_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_grcrc_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_grcrc_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_grcrc_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_tr64_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_tr64_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_tr64_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_tr64_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_tr64_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_tr64_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_tr64_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_tr64_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_tr127_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_tr127_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_tr127_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_tr127_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_tr127_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_tr127_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_tr127_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_tr127_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_tr255_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_tr255_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_tr255_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_tr255_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_tr255_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_tr255_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_tr255_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_tr255_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_tr511_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_tr511_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_tr511_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_tr511_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_tr511_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_tr511_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_tr511_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_tr511_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_tr1023_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_tr1023_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_tr1023_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_tr1023_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_tr1023_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_tr1023_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_tr1023_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_tr1023_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_tr1518_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_tr1518_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_tr1518_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_tr1518_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_tr1518_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_tr1518_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_tr1518_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_tr1518_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_trmgv_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_trmgv_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_trmgv_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_trmgv_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_trmgv_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_trmgv_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_trmgv_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_trmgv_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_tr2047_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_tr2047_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_tr2047_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_tr2047_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_tr2047_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_tr2047_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_tr2047_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_tr2047_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_tr4095_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_tr4095_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_tr4095_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_tr4095_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_tr4095_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_tr4095_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_tr4095_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_tr4095_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_tr9216_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_tr9216_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_tr9216_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_tr9216_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_tr9216_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_tr9216_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_tr9216_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_tr9216_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtpkt_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_gtpkt_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtpkt_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtpkt_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtpkt_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_gtpkt_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtpkt_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtpkt_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtmca_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_gtmca_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtmca_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtmca_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtmca_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_gtmca_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtmca_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtmca_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtbca_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_gtbca_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtbca_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtbca_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtbca_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_gtbca_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtbca_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtbca_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtxpf_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_gtxpf_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtxpf_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtxpf_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtxpf_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_gtxpf_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtxpf_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtxpf_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtxcf_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_gtxcf_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtxcf_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtxcf_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtxcf_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_gtxcf_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtxcf_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtxcf_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtfcs_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_gtfcs_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtfcs_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtfcs_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtfcs_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_gtfcs_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtfcs_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtfcs_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtovr_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_gtovr_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtovr_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtovr_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtovr_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_gtovr_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtovr_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtovr_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtdrf_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_gtdrf_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtdrf_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtdrf_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtdrf_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_gtdrf_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtdrf_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtdrf_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtedf_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_gtedf_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtedf_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtedf_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtedf_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_gtedf_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtedf_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtedf_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtscl_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_gtscl_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtscl_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtscl_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtscl_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_gtscl_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtscl_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtscl_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtmcl_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_gtmcl_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtmcl_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtmcl_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtmcl_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_gtmcl_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtmcl_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtmcl_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtlcl_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_gtlcl_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtlcl_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtlcl_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtlcl_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_gtlcl_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtlcl_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtlcl_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtxcl_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_gtxcl_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtxcl_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtxcl_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtxcl_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_gtxcl_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtxcl_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtxcl_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtfrg_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_gtfrg_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtfrg_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtfrg_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtfrg_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_gtfrg_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtfrg_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtfrg_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtncl_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_gtncl_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtncl_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtncl_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtncl_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_gtncl_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtncl_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtncl_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtjbr_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_gtjbr_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtjbr_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtjbr_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtjbr_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_gtjbr_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtjbr_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtjbr_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtbyt_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_gtbyt_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtbyt_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtbyt_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t count_u16 = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtbyt_upper_set(%u %u)\n", xumac_id,
            count_u16);
        ag_err = ag_drv_xumac_rdp_gtbyt_upper_set(xumac_id, count_u16);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtbyt_upper_get(xumac_id, &count_u16);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtbyt_upper_get(%u %u)\n", xumac_id,
                count_u16);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u16 != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtpok_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_gtpok_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtpok_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtpok_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtpok_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_gtpok_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtpok_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtpok_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtuc_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_gtuc_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtuc_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtuc_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gtuc_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_gtuc_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gtuc_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gtuc_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_rrpkt_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_rrpkt_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_rrpkt_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_rrpkt_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_rrpkt_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_rrpkt_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_rrpkt_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_rrpkt_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_rrund_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_rrund_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_rrund_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_rrund_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_rrund_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_rrund_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_rrund_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_rrund_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_rrfrg_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_rrfrg_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_rrfrg_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_rrfrg_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t count_u8 = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_rrfrg_upper_set(%u %u)\n", xumac_id,
            count_u8);
        ag_err = ag_drv_xumac_rdp_rrfrg_upper_set(xumac_id, count_u8);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_rrfrg_upper_get(xumac_id, &count_u8);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_rrfrg_upper_get(%u %u)\n", xumac_id,
                count_u8);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u8 != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_rrbyt_set(%u %u)\n", xumac_id,
            count);
        ag_err = ag_drv_xumac_rdp_rrbyt_set(xumac_id, count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_rrbyt_get(xumac_id, &count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_rrbyt_get(%u %u)\n", xumac_id,
                count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t count_u16 = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_xumac_rdp_rrbyt_upper_set(%u %u)\n", xumac_id,
            count_u16);
        ag_err = ag_drv_xumac_rdp_rrbyt_upper_set(xumac_id, count_u16);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_rrbyt_upper_get(xumac_id, &count_u16);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_rrbyt_upper_get(%u %u)\n", xumac_id,
                count_u16);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (count_u16 != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean rx_cnt_rst = gtmv(m, 1);
        bdmf_boolean runt_cnt_rst = gtmv(m, 1);
        bdmf_boolean tx_cnt_rst = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_xumac_rdp_mib_cntrl_set(%u %u %u %u)\n", xumac_id,
            rx_cnt_rst, runt_cnt_rst, tx_cnt_rst);
        ag_err = ag_drv_xumac_rdp_mib_cntrl_set(xumac_id, rx_cnt_rst, runt_cnt_rst, tx_cnt_rst);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_mib_cntrl_get(xumac_id, &rx_cnt_rst, &runt_cnt_rst, &tx_cnt_rst);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_mib_cntrl_get(%u %u %u %u)\n", xumac_id,
                rx_cnt_rst, runt_cnt_rst, tx_cnt_rst);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (rx_cnt_rst != gtmv(m, 1) || runt_cnt_rst != gtmv(m, 1) || tx_cnt_rst != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t data32 = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_mib_read_data_set(%u %u)\n", xumac_id,
            data32);
        ag_err = ag_drv_xumac_rdp_mib_read_data_set(xumac_id, data32);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_mib_read_data_get(xumac_id, &data32);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_mib_read_data_get(%u %u)\n", xumac_id,
                data32);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (data32 != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t data32 = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_mib_write_data_set(%u %u)\n", xumac_id,
            data32);
        ag_err = ag_drv_xumac_rdp_mib_write_data_set(xumac_id, data32);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_mib_write_data_get(xumac_id, &data32);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_mib_write_data_get(%u %u)\n", xumac_id,
                data32);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (data32 != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean mpd_en = gtmv(m, 1);
        uint8_t mseq_len = gtmv(m, 8);
        bdmf_boolean psw_en = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_xumac_rdp_control_set(%u %u %u %u)\n", xumac_id,
            mpd_en, mseq_len, psw_en);
        ag_err = ag_drv_xumac_rdp_control_set(xumac_id, mpd_en, mseq_len, psw_en);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_control_get(xumac_id, &mpd_en, &mseq_len, &psw_en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_control_get(%u %u %u %u)\n", xumac_id,
                mpd_en, mseq_len, psw_en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (mpd_en != gtmv(m, 1) || mseq_len != gtmv(m, 8) || psw_en != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t psw_47_32 = gtmv(m, 16);
        bdmf_session_print(session, "ag_drv_xumac_rdp_psw_ms_set(%u %u)\n", xumac_id,
            psw_47_32);
        ag_err = ag_drv_xumac_rdp_psw_ms_set(xumac_id, psw_47_32);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_psw_ms_get(xumac_id, &psw_47_32);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_psw_ms_get(%u %u)\n", xumac_id,
                psw_47_32);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (psw_47_32 != gtmv(m, 16))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t psw_31_0 = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_psw_ls_set(%u %u)\n", xumac_id,
            psw_31_0);
        ag_err = ag_drv_xumac_rdp_psw_ls_set(xumac_id, psw_31_0);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_psw_ls_get(xumac_id, &psw_31_0);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_psw_ls_get(%u %u)\n", xumac_id,
                psw_31_0);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (psw_31_0 != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        xumac_rdp_control_1 control_1 = {.xib_rx_en = gtmv(m, 1), .xib_tx_en = gtmv(m, 1), .rx_flush_en = gtmv(m, 1), .tx_flush_en = gtmv(m, 1), .link_down_rst_en = gtmv(m, 1), .standard_mux_en = gtmv(m, 1), .xgmii_sel = gtmv(m, 1), .dic_dis = gtmv(m, 1), .rx_start_threshold = gtmv(m, 9), .gmii_rx_clk_gate_en = gtmv(m, 1), .strict_preamble_dis = gtmv(m, 1), .tx_ipg = gtmv(m, 5), .min_rx_ipg = gtmv(m, 5), .xgmii_sel_ovrd = gtmv(m, 1), .gmii_tx_clk_gate_en = gtmv(m, 1), .autoconfig_en = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_xumac_rdp_control_1_set(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", xumac_id,
            control_1.xib_rx_en, control_1.xib_tx_en, control_1.rx_flush_en, control_1.tx_flush_en, 
            control_1.link_down_rst_en, control_1.standard_mux_en, control_1.xgmii_sel, control_1.dic_dis, 
            control_1.rx_start_threshold, control_1.gmii_rx_clk_gate_en, control_1.strict_preamble_dis, control_1.tx_ipg, 
            control_1.min_rx_ipg, control_1.xgmii_sel_ovrd, control_1.gmii_tx_clk_gate_en, control_1.autoconfig_en);
        ag_err = ag_drv_xumac_rdp_control_1_set(xumac_id, &control_1);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_control_1_get(xumac_id, &control_1);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_control_1_get(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", xumac_id,
                control_1.xib_rx_en, control_1.xib_tx_en, control_1.rx_flush_en, control_1.tx_flush_en, 
                control_1.link_down_rst_en, control_1.standard_mux_en, control_1.xgmii_sel, control_1.dic_dis, 
                control_1.rx_start_threshold, control_1.gmii_rx_clk_gate_en, control_1.strict_preamble_dis, control_1.tx_ipg, 
                control_1.min_rx_ipg, control_1.xgmii_sel_ovrd, control_1.gmii_tx_clk_gate_en, control_1.autoconfig_en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (control_1.xib_rx_en != gtmv(m, 1) || control_1.xib_tx_en != gtmv(m, 1) || control_1.rx_flush_en != gtmv(m, 1) || control_1.tx_flush_en != gtmv(m, 1) || control_1.link_down_rst_en != gtmv(m, 1) || control_1.standard_mux_en != gtmv(m, 1) || control_1.xgmii_sel != gtmv(m, 1) || control_1.dic_dis != gtmv(m, 1) || control_1.rx_start_threshold != gtmv(m, 9) || control_1.gmii_rx_clk_gate_en != gtmv(m, 1) || control_1.strict_preamble_dis != gtmv(m, 1) || control_1.tx_ipg != gtmv(m, 5) || control_1.min_rx_ipg != gtmv(m, 5) || control_1.xgmii_sel_ovrd != gtmv(m, 1) || control_1.gmii_tx_clk_gate_en != gtmv(m, 1) || control_1.autoconfig_en != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t tx_start_threshold = gtmv(m, 9);
        uint16_t tx_xoff_threshold = gtmv(m, 9);
        uint16_t tx_xon_threshold = gtmv(m, 9);
        bdmf_boolean tx_backpressure_en = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_xumac_rdp_extended_control_set(%u %u %u %u %u)\n", xumac_id,
            tx_start_threshold, tx_xoff_threshold, tx_xon_threshold, tx_backpressure_en);
        ag_err = ag_drv_xumac_rdp_extended_control_set(xumac_id, tx_start_threshold, tx_xoff_threshold, tx_xon_threshold, tx_backpressure_en);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_extended_control_get(xumac_id, &tx_start_threshold, &tx_xoff_threshold, &tx_xon_threshold, &tx_backpressure_en);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_extended_control_get(%u %u %u %u %u)\n", xumac_id,
                tx_start_threshold, tx_xoff_threshold, tx_xon_threshold, tx_backpressure_en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (tx_start_threshold != gtmv(m, 9) || tx_xoff_threshold != gtmv(m, 9) || tx_xon_threshold != gtmv(m, 9) || tx_backpressure_en != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t tx_idle_stuffing_ctrl = gtmv(m, 4);
        bdmf_session_print(session, "ag_drv_xumac_rdp_tx_idle_stuffing_control_set(%u %u)\n", xumac_id,
            tx_idle_stuffing_ctrl);
        ag_err = ag_drv_xumac_rdp_tx_idle_stuffing_control_set(xumac_id, tx_idle_stuffing_ctrl);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_tx_idle_stuffing_control_get(xumac_id, &tx_idle_stuffing_ctrl);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_tx_idle_stuffing_control_get(%u %u)\n", xumac_id,
                tx_idle_stuffing_ctrl);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (tx_idle_stuffing_ctrl != gtmv(m, 4))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t actual_data_rate = gtmv(m, 3);
        bdmf_session_print(session, "ag_drv_xumac_rdp_actual_data_rate_set(%u %u)\n", xumac_id,
            actual_data_rate);
        ag_err = ag_drv_xumac_rdp_actual_data_rate_set(xumac_id, actual_data_rate);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_actual_data_rate_get(xumac_id, &actual_data_rate);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_actual_data_rate_get(%u %u)\n", xumac_id,
                actual_data_rate);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (actual_data_rate != gtmv(m, 3))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t ndiv = gtmv(m, 8);
        uint8_t mdiv = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_xumac_rdp_gmii_clock_swallower_control_set(%u %u %u)\n", xumac_id,
            ndiv, mdiv);
        ag_err = ag_drv_xumac_rdp_gmii_clock_swallower_control_set(xumac_id, ndiv, mdiv);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_gmii_clock_swallower_control_get(xumac_id, &ndiv, &mdiv);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_gmii_clock_swallower_control_get(%u %u %u)\n", xumac_id,
                ndiv, mdiv);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (ndiv != gtmv(m, 8) || mdiv != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint8_t xgmii_data_rate = gtmv(m, 2);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_xgmii_data_rate_status_get(xumac_id, &xgmii_data_rate);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_xgmii_data_rate_status_get(%u %u)\n", xumac_id,
                xgmii_data_rate);
        }
    }

    {
        xumac_rdp_status status = {.rx_fifo_overrun = gtmv(m, 1), .rx_fifo_underrun = gtmv(m, 1), .tx_fifo_underrun = gtmv(m, 1), .tx_fifo_overrun = gtmv(m, 1), .rx_fault_status = gtmv(m, 2)};
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_status_get(xumac_id, &status);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_status_get(%u %u %u %u %u %u)\n", xumac_id,
                status.rx_fifo_overrun, status.rx_fifo_underrun, status.tx_fifo_underrun, status.tx_fifo_overrun, 
                status.rx_fault_status);
        }
    }

    {
        uint32_t pkt_count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_rx_discard_packet_counter_set(%u %u)\n", xumac_id,
            pkt_count);
        ag_err = ag_drv_xumac_rdp_rx_discard_packet_counter_set(xumac_id, pkt_count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_rx_discard_packet_counter_get(xumac_id, &pkt_count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_rx_discard_packet_counter_get(%u %u)\n", xumac_id,
                pkt_count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pkt_count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t pkt_count = gtmv(m, 32);
        bdmf_session_print(session, "ag_drv_xumac_rdp_tx_discard_packet_counter_set(%u %u)\n", xumac_id,
            pkt_count);
        ag_err = ag_drv_xumac_rdp_tx_discard_packet_counter_set(xumac_id, pkt_count);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_tx_discard_packet_counter_get(xumac_id, &pkt_count);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_tx_discard_packet_counter_get(%u %u)\n", xumac_id,
                pkt_count);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (pkt_count != gtmv(m, 32))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t sys_port_rev = gtmv(m, 16);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_rev_get(xumac_id, &sys_port_rev);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_rev_get(%u %u)\n", xumac_id,
                sys_port_rev);
        }
    }

    {
        uint32_t mac_rxerr_mask = gtmv(m, 18);
        bdmf_session_print(session, "ag_drv_xumac_rdp_umac_rxerr_mask_set(%u %u)\n", xumac_id,
            mac_rxerr_mask);
        ag_err = ag_drv_xumac_rdp_umac_rxerr_mask_set(xumac_id, mac_rxerr_mask);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_umac_rxerr_mask_get(xumac_id, &mac_rxerr_mask);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_umac_rxerr_mask_get(%u %u)\n", xumac_id,
                mac_rxerr_mask);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (mac_rxerr_mask != gtmv(m, 18))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint16_t max_pkt_size = gtmv(m, 14);
        bdmf_session_print(session, "ag_drv_xumac_rdp_mib_max_pkt_size_set(%u %u)\n", xumac_id,
            max_pkt_size);
        ag_err = ag_drv_xumac_rdp_mib_max_pkt_size_set(xumac_id, max_pkt_size);
        if (!ag_err)
            ag_err = ag_drv_xumac_rdp_mib_max_pkt_size_get(xumac_id, &max_pkt_size);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_xumac_rdp_mib_max_pkt_size_get(%u %u)\n", xumac_id,
                max_pkt_size);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (max_pkt_size != gtmv(m, 14))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    bdmf_session_print(session, "successes=%u failures=%u\n", test_success_cnt, test_failure_cnt);

    return ag_err;
}
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_xumac_rdp_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    int chip_rev_idx = RU_CHIP_REV_IDX_GET();
    uint32_t i;
    uint32_t j;
    uint32_t index1_start = 0;
    uint32_t index1_stop;
    uint32_t index2_start = 0;
    uint32_t index2_stop;
    bdmfmon_cmd_parm_t *cliparm;
    const ru_reg_rec *reg;
    const ru_block_rec *blk;
    const char *enum_string = bdmfmon_enum_parm_stringval(session, &parm[0]);

    if(!enum_string)
        return BDMF_ERR_INTERNAL;

    switch (parm[0].value.unumber)
    {
    case bdmf_address_ipg_hd_bkp_cntl: reg = &RU_REG(XUMAC_RDP, IPG_HD_BKP_CNTL); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_command_config: reg = &RU_REG(XUMAC_RDP, COMMAND_CONFIG); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_mac_0: reg = &RU_REG(XUMAC_RDP, MAC_0); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_mac_1: reg = &RU_REG(XUMAC_RDP, MAC_1); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_frm_length: reg = &RU_REG(XUMAC_RDP, FRM_LENGTH); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_pause_quant: reg = &RU_REG(XUMAC_RDP, PAUSE_QUANT); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_tx_ts_seq_id: reg = &RU_REG(XUMAC_RDP, TX_TS_SEQ_ID); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_sfd_offset: reg = &RU_REG(XUMAC_RDP, SFD_OFFSET); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_mac_mode: reg = &RU_REG(XUMAC_RDP, MAC_MODE); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_tag_0: reg = &RU_REG(XUMAC_RDP, TAG_0); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_tag_1: reg = &RU_REG(XUMAC_RDP, TAG_1); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_rx_pause_quanta_scale: reg = &RU_REG(XUMAC_RDP, RX_PAUSE_QUANTA_SCALE); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_tx_preamble: reg = &RU_REG(XUMAC_RDP, TX_PREAMBLE); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_tx_ipg_length: reg = &RU_REG(XUMAC_RDP, TX_IPG_LENGTH); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_pfc_xoff_timer: reg = &RU_REG(XUMAC_RDP, PFC_XOFF_TIMER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_umac_eee_ctrl: reg = &RU_REG(XUMAC_RDP, UMAC_EEE_CTRL); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_mii_eee_delay_entry_timer: reg = &RU_REG(XUMAC_RDP, MII_EEE_DELAY_ENTRY_TIMER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gmii_eee_delay_entry_timer: reg = &RU_REG(XUMAC_RDP, GMII_EEE_DELAY_ENTRY_TIMER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_umac_eee_ref_count: reg = &RU_REG(XUMAC_RDP, UMAC_EEE_REF_COUNT); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_umac_timestamp_adjust: reg = &RU_REG(XUMAC_RDP, UMAC_TIMESTAMP_ADJUST); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_umac_rx_pkt_drop_status: reg = &RU_REG(XUMAC_RDP, UMAC_RX_PKT_DROP_STATUS); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_umac_symmetric_idle_threshold: reg = &RU_REG(XUMAC_RDP, UMAC_SYMMETRIC_IDLE_THRESHOLD); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_mii_eee_wake_timer: reg = &RU_REG(XUMAC_RDP, MII_EEE_WAKE_TIMER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gmii_eee_wake_timer: reg = &RU_REG(XUMAC_RDP, GMII_EEE_WAKE_TIMER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_umac_rev_id: reg = &RU_REG(XUMAC_RDP, UMAC_REV_ID); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gmii_2p5g_eee_delay_entry_timer: reg = &RU_REG(XUMAC_RDP, GMII_2P5G_EEE_DELAY_ENTRY_TIMER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gmii_5g_eee_delay_entry_timer: reg = &RU_REG(XUMAC_RDP, GMII_5G_EEE_DELAY_ENTRY_TIMER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gmii_10g_eee_delay_entry_timer: reg = &RU_REG(XUMAC_RDP, GMII_10G_EEE_DELAY_ENTRY_TIMER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gmii_2p5g_eee_wake_timer: reg = &RU_REG(XUMAC_RDP, GMII_2P5G_EEE_WAKE_TIMER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gmii_5g_eee_wake_timer: reg = &RU_REG(XUMAC_RDP, GMII_5G_EEE_WAKE_TIMER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gmii_10g_eee_wake_timer: reg = &RU_REG(XUMAC_RDP, GMII_10G_EEE_WAKE_TIMER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_active_eee_delay_entry_timer: reg = &RU_REG(XUMAC_RDP, ACTIVE_EEE_DELAY_ENTRY_TIMER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_active_eee_wake_timer: reg = &RU_REG(XUMAC_RDP, ACTIVE_EEE_WAKE_TIMER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_mac_pfc_type: reg = &RU_REG(XUMAC_RDP, MAC_PFC_TYPE); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_mac_pfc_opcode: reg = &RU_REG(XUMAC_RDP, MAC_PFC_OPCODE); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_mac_pfc_da_0: reg = &RU_REG(XUMAC_RDP, MAC_PFC_DA_0); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_mac_pfc_da_1: reg = &RU_REG(XUMAC_RDP, MAC_PFC_DA_1); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_macsec_prog_tx_crc: reg = &RU_REG(XUMAC_RDP, MACSEC_PROG_TX_CRC); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_macsec_cntrl: reg = &RU_REG(XUMAC_RDP, MACSEC_CNTRL); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_ts_status: reg = &RU_REG(XUMAC_RDP, TS_STATUS); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_tx_ts_data: reg = &RU_REG(XUMAC_RDP, TX_TS_DATA); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_pause_refresh_ctrl: reg = &RU_REG(XUMAC_RDP, PAUSE_REFRESH_CTRL); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_flush_control: reg = &RU_REG(XUMAC_RDP, FLUSH_CONTROL); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_rxfifo_stat: reg = &RU_REG(XUMAC_RDP, RXFIFO_STAT); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_txfifo_stat: reg = &RU_REG(XUMAC_RDP, TXFIFO_STAT); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_mac_pfc_ctrl: reg = &RU_REG(XUMAC_RDP, MAC_PFC_CTRL); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_mac_pfc_refresh_ctrl: reg = &RU_REG(XUMAC_RDP, MAC_PFC_REFRESH_CTRL); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gr64: reg = &RU_REG(XUMAC_RDP, GR64); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gr64_upper: reg = &RU_REG(XUMAC_RDP, GR64_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gr127: reg = &RU_REG(XUMAC_RDP, GR127); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gr127_upper: reg = &RU_REG(XUMAC_RDP, GR127_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gr255: reg = &RU_REG(XUMAC_RDP, GR255); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gr255_upper: reg = &RU_REG(XUMAC_RDP, GR255_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gr511: reg = &RU_REG(XUMAC_RDP, GR511); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gr511_upper: reg = &RU_REG(XUMAC_RDP, GR511_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gr1023: reg = &RU_REG(XUMAC_RDP, GR1023); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gr1023_upper: reg = &RU_REG(XUMAC_RDP, GR1023_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gr1518: reg = &RU_REG(XUMAC_RDP, GR1518); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gr1518_upper: reg = &RU_REG(XUMAC_RDP, GR1518_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_grmgv: reg = &RU_REG(XUMAC_RDP, GRMGV); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_grmgv_upper: reg = &RU_REG(XUMAC_RDP, GRMGV_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gr2047: reg = &RU_REG(XUMAC_RDP, GR2047); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gr2047_upper: reg = &RU_REG(XUMAC_RDP, GR2047_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gr4095: reg = &RU_REG(XUMAC_RDP, GR4095); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gr4095_upper: reg = &RU_REG(XUMAC_RDP, GR4095_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gr9216: reg = &RU_REG(XUMAC_RDP, GR9216); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gr9216_upper: reg = &RU_REG(XUMAC_RDP, GR9216_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_grpkt: reg = &RU_REG(XUMAC_RDP, GRPKT); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_grpkt_upper: reg = &RU_REG(XUMAC_RDP, GRPKT_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_grbyt: reg = &RU_REG(XUMAC_RDP, GRBYT); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_grbyt_upper: reg = &RU_REG(XUMAC_RDP, GRBYT_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_grmca: reg = &RU_REG(XUMAC_RDP, GRMCA); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_grmca_upper: reg = &RU_REG(XUMAC_RDP, GRMCA_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_grbca: reg = &RU_REG(XUMAC_RDP, GRBCA); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_grbca_upper: reg = &RU_REG(XUMAC_RDP, GRBCA_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_grfcs: reg = &RU_REG(XUMAC_RDP, GRFCS); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_grfcs_upper: reg = &RU_REG(XUMAC_RDP, GRFCS_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_grxcf: reg = &RU_REG(XUMAC_RDP, GRXCF); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_grxcf_upper: reg = &RU_REG(XUMAC_RDP, GRXCF_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_grxpf: reg = &RU_REG(XUMAC_RDP, GRXPF); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_grxpf_upper: reg = &RU_REG(XUMAC_RDP, GRXPF_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_grxuo: reg = &RU_REG(XUMAC_RDP, GRXUO); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_grxuo_upper: reg = &RU_REG(XUMAC_RDP, GRXUO_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_graln: reg = &RU_REG(XUMAC_RDP, GRALN); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_graln_upper: reg = &RU_REG(XUMAC_RDP, GRALN_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_grflr: reg = &RU_REG(XUMAC_RDP, GRFLR); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_grflr_upper: reg = &RU_REG(XUMAC_RDP, GRFLR_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_grcde: reg = &RU_REG(XUMAC_RDP, GRCDE); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_grcde_upper: reg = &RU_REG(XUMAC_RDP, GRCDE_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_grfcr: reg = &RU_REG(XUMAC_RDP, GRFCR); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_grfcr_upper: reg = &RU_REG(XUMAC_RDP, GRFCR_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_grovr: reg = &RU_REG(XUMAC_RDP, GROVR); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_grovr_upper: reg = &RU_REG(XUMAC_RDP, GROVR_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_grjbr: reg = &RU_REG(XUMAC_RDP, GRJBR); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_grjbr_upper: reg = &RU_REG(XUMAC_RDP, GRJBR_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_grmtue: reg = &RU_REG(XUMAC_RDP, GRMTUE); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_grmtue_upper: reg = &RU_REG(XUMAC_RDP, GRMTUE_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_grpok: reg = &RU_REG(XUMAC_RDP, GRPOK); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_grpok_upper: reg = &RU_REG(XUMAC_RDP, GRPOK_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gruc: reg = &RU_REG(XUMAC_RDP, GRUC); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gruc_upper: reg = &RU_REG(XUMAC_RDP, GRUC_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_grppp: reg = &RU_REG(XUMAC_RDP, GRPPP); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_grppp_upper: reg = &RU_REG(XUMAC_RDP, GRPPP_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_grcrc: reg = &RU_REG(XUMAC_RDP, GRCRC); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_grcrc_upper: reg = &RU_REG(XUMAC_RDP, GRCRC_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_tr64: reg = &RU_REG(XUMAC_RDP, TR64); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_tr64_upper: reg = &RU_REG(XUMAC_RDP, TR64_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_tr127: reg = &RU_REG(XUMAC_RDP, TR127); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_tr127_upper: reg = &RU_REG(XUMAC_RDP, TR127_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_tr255: reg = &RU_REG(XUMAC_RDP, TR255); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_tr255_upper: reg = &RU_REG(XUMAC_RDP, TR255_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_tr511: reg = &RU_REG(XUMAC_RDP, TR511); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_tr511_upper: reg = &RU_REG(XUMAC_RDP, TR511_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_tr1023: reg = &RU_REG(XUMAC_RDP, TR1023); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_tr1023_upper: reg = &RU_REG(XUMAC_RDP, TR1023_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_tr1518: reg = &RU_REG(XUMAC_RDP, TR1518); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_tr1518_upper: reg = &RU_REG(XUMAC_RDP, TR1518_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_trmgv: reg = &RU_REG(XUMAC_RDP, TRMGV); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_trmgv_upper: reg = &RU_REG(XUMAC_RDP, TRMGV_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_tr2047: reg = &RU_REG(XUMAC_RDP, TR2047); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_tr2047_upper: reg = &RU_REG(XUMAC_RDP, TR2047_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_tr4095: reg = &RU_REG(XUMAC_RDP, TR4095); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_tr4095_upper: reg = &RU_REG(XUMAC_RDP, TR4095_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_tr9216: reg = &RU_REG(XUMAC_RDP, TR9216); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_tr9216_upper: reg = &RU_REG(XUMAC_RDP, TR9216_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtpkt: reg = &RU_REG(XUMAC_RDP, GTPKT); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtpkt_upper: reg = &RU_REG(XUMAC_RDP, GTPKT_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtmca: reg = &RU_REG(XUMAC_RDP, GTMCA); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtmca_upper: reg = &RU_REG(XUMAC_RDP, GTMCA_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtbca: reg = &RU_REG(XUMAC_RDP, GTBCA); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtbca_upper: reg = &RU_REG(XUMAC_RDP, GTBCA_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtxpf: reg = &RU_REG(XUMAC_RDP, GTXPF); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtxpf_upper: reg = &RU_REG(XUMAC_RDP, GTXPF_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtxcf: reg = &RU_REG(XUMAC_RDP, GTXCF); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtxcf_upper: reg = &RU_REG(XUMAC_RDP, GTXCF_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtfcs: reg = &RU_REG(XUMAC_RDP, GTFCS); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtfcs_upper: reg = &RU_REG(XUMAC_RDP, GTFCS_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtovr: reg = &RU_REG(XUMAC_RDP, GTOVR); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtovr_upper: reg = &RU_REG(XUMAC_RDP, GTOVR_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtdrf: reg = &RU_REG(XUMAC_RDP, GTDRF); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtdrf_upper: reg = &RU_REG(XUMAC_RDP, GTDRF_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtedf: reg = &RU_REG(XUMAC_RDP, GTEDF); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtedf_upper: reg = &RU_REG(XUMAC_RDP, GTEDF_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtscl: reg = &RU_REG(XUMAC_RDP, GTSCL); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtscl_upper: reg = &RU_REG(XUMAC_RDP, GTSCL_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtmcl: reg = &RU_REG(XUMAC_RDP, GTMCL); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtmcl_upper: reg = &RU_REG(XUMAC_RDP, GTMCL_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtlcl: reg = &RU_REG(XUMAC_RDP, GTLCL); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtlcl_upper: reg = &RU_REG(XUMAC_RDP, GTLCL_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtxcl: reg = &RU_REG(XUMAC_RDP, GTXCL); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtxcl_upper: reg = &RU_REG(XUMAC_RDP, GTXCL_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtfrg: reg = &RU_REG(XUMAC_RDP, GTFRG); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtfrg_upper: reg = &RU_REG(XUMAC_RDP, GTFRG_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtncl: reg = &RU_REG(XUMAC_RDP, GTNCL); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtncl_upper: reg = &RU_REG(XUMAC_RDP, GTNCL_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtjbr: reg = &RU_REG(XUMAC_RDP, GTJBR); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtjbr_upper: reg = &RU_REG(XUMAC_RDP, GTJBR_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtbyt: reg = &RU_REG(XUMAC_RDP, GTBYT); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtbyt_upper: reg = &RU_REG(XUMAC_RDP, GTBYT_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtpok: reg = &RU_REG(XUMAC_RDP, GTPOK); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtpok_upper: reg = &RU_REG(XUMAC_RDP, GTPOK_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtuc: reg = &RU_REG(XUMAC_RDP, GTUC); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gtuc_upper: reg = &RU_REG(XUMAC_RDP, GTUC_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_rrpkt: reg = &RU_REG(XUMAC_RDP, RRPKT); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_rrpkt_upper: reg = &RU_REG(XUMAC_RDP, RRPKT_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_rrund: reg = &RU_REG(XUMAC_RDP, RRUND); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_rrund_upper: reg = &RU_REG(XUMAC_RDP, RRUND_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_rrfrg: reg = &RU_REG(XUMAC_RDP, RRFRG); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_rrfrg_upper: reg = &RU_REG(XUMAC_RDP, RRFRG_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_rrbyt: reg = &RU_REG(XUMAC_RDP, RRBYT); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_rrbyt_upper: reg = &RU_REG(XUMAC_RDP, RRBYT_UPPER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_mib_cntrl: reg = &RU_REG(XUMAC_RDP, MIB_CNTRL); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_mib_read_data: reg = &RU_REG(XUMAC_RDP, MIB_READ_DATA); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_mib_write_data: reg = &RU_REG(XUMAC_RDP, MIB_WRITE_DATA); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_control: reg = &RU_REG(XUMAC_RDP, CONTROL); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_psw_ms: reg = &RU_REG(XUMAC_RDP, PSW_MS); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_psw_ls: reg = &RU_REG(XUMAC_RDP, PSW_LS); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_control_1: reg = &RU_REG(XUMAC_RDP, CONTROL_1); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_extended_control: reg = &RU_REG(XUMAC_RDP, EXTENDED_CONTROL); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_tx_idle_stuffing_control: reg = &RU_REG(XUMAC_RDP, TX_IDLE_STUFFING_CONTROL); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_actual_data_rate: reg = &RU_REG(XUMAC_RDP, ACTUAL_DATA_RATE); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_gmii_clock_swallower_control: reg = &RU_REG(XUMAC_RDP, GMII_CLOCK_SWALLOWER_CONTROL); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_xgmii_data_rate_status: reg = &RU_REG(XUMAC_RDP, XGMII_DATA_RATE_STATUS); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_status: reg = &RU_REG(XUMAC_RDP, STATUS); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_rx_discard_packet_counter: reg = &RU_REG(XUMAC_RDP, RX_DISCARD_PACKET_COUNTER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_tx_discard_packet_counter: reg = &RU_REG(XUMAC_RDP, TX_DISCARD_PACKET_COUNTER); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_rev: reg = &RU_REG(XUMAC_RDP, REV); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_umac_rxerr_mask: reg = &RU_REG(XUMAC_RDP, UMAC_RXERR_MASK); blk = &RU_BLK(XUMAC_RDP); break;
    case bdmf_address_mib_max_pkt_size: reg = &RU_REG(XUMAC_RDP, MIB_MAX_PKT_SIZE); blk = &RU_BLK(XUMAC_RDP); break;
    default :
        return BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    if ((cliparm = bdmfmon_cmd_find(session, "index1")))
    {
        index1_start = cliparm->value.unumber;
        index1_stop = index1_start + 1;
    }
    else
        index1_stop = blk->addr_count;
    if ((cliparm = bdmfmon_cmd_find(session, "index2")))
    {
        index2_start = cliparm->value.unumber;
        index2_stop = index2_start + 1;
    }
    else
        index2_stop = reg->ram_count;
    if (index1_stop > blk->addr_count)
    {
        bdmf_session_print(session, "index1 (%u) is out of range (%u).\n", index1_stop, blk->addr_count);
        return BDMF_ERR_RANGE;
    }
    if (index2_stop > (reg->ram_count))
    {
        bdmf_session_print(session, "index2 (%u) is out of range (%u).\n", index2_stop, reg->ram_count);
        return BDMF_ERR_RANGE;
    }
    if (reg->ram_count)
        for (i = index1_start; i < index1_stop; i++)
        {
            bdmf_session_print(session, "index1 = %u\n", i);
            for (j = index2_start; j < index2_stop; j++)
                bdmf_session_print(session, TAB "(%5u) 0x%08X\n", j, ((blk->addr[i] + reg->addr[chip_rev_idx]) + j));
        }
    else
        for (i = index1_start; i < index1_stop; i++)
            bdmf_session_print(session, "(%3u) 0x%08X\n", i, blk->addr[i]+reg->addr[chip_rev_idx]);
    return 0;
}
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

bdmfmon_handle_t ag_drv_xumac_rdp_cli_init(bdmfmon_handle_t root_dir)
{
    bdmfmon_handle_t dir;

    dir = bdmfmon_dir_add(root_dir, "xumac_rdp", "xumac_rdp", BDMF_ACCESS_ADMIN, NULL);
    BUG_ON(dir == NULL);

    {
        static bdmfmon_cmd_parm_t set_ipg_hd_bkp_cntl[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("hd_fc_ena", "hd_fc_ena", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("hd_fc_bkoff_ok", "hd_fc_bkoff_ok", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ipg_config_rx", "ipg_config_rx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_command_config[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tx_ena", "tx_ena", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rx_ena", "rx_ena", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("eth_speed", "eth_speed", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("promis_en", "promis_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pad_en", "pad_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("crc_fwd", "crc_fwd", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pause_fwd", "pause_fwd", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pause_ignore", "pause_ignore", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tx_addr_ins", "tx_addr_ins", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("hd_ena", "hd_ena", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rx_low_latency_en", "rx_low_latency_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("overflow_en", "overflow_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("sw_reset", "sw_reset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fcs_corrupt_urun_en", "fcs_corrupt_urun_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("loop_ena", "loop_ena", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mac_loop_con", "mac_loop_con", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("sw_override_tx", "sw_override_tx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("sw_override_rx", "sw_override_rx", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("oob_efc_mode", "oob_efc_mode", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("bypass_oob_efc_synchronizer", "bypass_oob_efc_synchronizer", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("en_internal_tx_crs", "en_internal_tx_crs", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ena_ext_config", "ena_ext_config", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("cntl_frm_ena", "cntl_frm_ena", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("no_lgth_check", "no_lgth_check", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("line_loopback", "line_loopback", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fd_tx_urun_fix_en", "fd_tx_urun_fix_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ignore_tx_pause", "ignore_tx_pause", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("oob_efc_disab", "oob_efc_disab", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("runt_filter_dis", "runt_filter_dis", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("eth_speed_bit2", "eth_speed_bit2", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mac_0[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mac_addr0", "mac_addr0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mac_1[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mac_addr1", "mac_addr1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_frm_length[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("maxfr", "maxfr", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pause_quant[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pause_quant", "pause_quant", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_sfd_offset[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("sfd_offset", "sfd_offset", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tag_0[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("frm_tag_0", "frm_tag_0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("config_outer_tpid_enable", "config_outer_tpid_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tag_1[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("frm_tag_1", "frm_tag_1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("config_inner_tpid_enable", "config_inner_tpid_enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_pause_quanta_scale[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("scale_value", "scale_value", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("scale_control", "scale_control", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("scale_fix", "scale_fix", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_preamble[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tx_preamble", "tx_preamble", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_ipg_length[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tx_ipg_length", "tx_ipg_length", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tx_min_pkt_size", "tx_min_pkt_size", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pfc_xoff_timer[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pfc_xoff_timer", "pfc_xoff_timer", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_umac_eee_ctrl[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("eee_en", "eee_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rx_fifo_check", "rx_fifo_check", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("eee_txclk_dis", "eee_txclk_dis", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dis_eee_10m", "dis_eee_10m", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("lp_idle_prediction_mode", "lp_idle_prediction_mode", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mii_eee_delay_entry_timer[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mii_eee_lpi_timer", "mii_eee_lpi_timer", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gmii_eee_delay_entry_timer[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("gmii_eee_lpi_timer", "gmii_eee_lpi_timer", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_umac_eee_ref_count[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("eee_ref_count", "eee_ref_count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_umac_timestamp_adjust[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("adjust", "adjust", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("en_1588", "en_1588", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("auto_adjust", "auto_adjust", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_umac_rx_pkt_drop_status[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rx_ipg_inval", "rx_ipg_inval", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_umac_symmetric_idle_threshold[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("threshold_value", "threshold_value", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mii_eee_wake_timer[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mii_eee_wake_timer", "mii_eee_wake_timer", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gmii_eee_wake_timer[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("gmii_eee_wake_timer", "gmii_eee_wake_timer", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gmii_2p5g_eee_delay_entry_timer[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("gmii_2p5g_eee_lpi_timer", "gmii_2p5g_eee_lpi_timer", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gmii_5g_eee_delay_entry_timer[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("gmii_5g_eee_lpi_timer", "gmii_5g_eee_lpi_timer", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gmii_10g_eee_delay_entry_timer[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("gmii_10g_eee_lpi_timer", "gmii_10g_eee_lpi_timer", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gmii_2p5g_eee_wake_timer[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("gmii_2p5g_eee_wake_timer", "gmii_2p5g_eee_wake_timer", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gmii_5g_eee_wake_timer[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("gmii_5g_eee_wake_timer", "gmii_5g_eee_wake_timer", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gmii_10g_eee_wake_timer[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("gmii_10g_eee_wake_timer", "gmii_10g_eee_wake_timer", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mac_pfc_type[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pfc_eth_type", "pfc_eth_type", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mac_pfc_opcode[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pfc_opcode", "pfc_opcode", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mac_pfc_da_0[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pfc_macda_0", "pfc_macda_0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mac_pfc_da_1[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pfc_macda_1", "pfc_macda_1", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_macsec_prog_tx_crc[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("macsec_prog_tx_crc", "macsec_prog_tx_crc", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_macsec_cntrl[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tx_launch_en", "tx_launch_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tx_crc_corupt_en", "tx_crc_corupt_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tx_crc_program", "tx_crc_program", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dis_pause_data_var_ipg", "dis_pause_data_var_ipg", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pause_refresh_ctrl[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("refresh_timer", "refresh_timer", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("enable", "enable", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_flush_control[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("flush", "flush", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mac_pfc_ctrl[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pfc_tx_enbl", "pfc_tx_enbl", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pfc_rx_enbl", "pfc_rx_enbl", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("force_pfc_xon", "force_pfc_xon", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rx_pass_pfc_frm", "rx_pass_pfc_frm", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pfc_stats_en", "pfc_stats_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mac_pfc_refresh_ctrl[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pfc_refresh_en", "pfc_refresh_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pfc_refresh_timer", "pfc_refresh_timer", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gr64[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gr64_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gr127[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gr127_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gr255[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gr255_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gr511[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gr511_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gr1023[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gr1023_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gr1518[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gr1518_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grmgv[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grmgv_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gr2047[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gr2047_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gr4095[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gr4095_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gr9216[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gr9216_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grpkt[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grpkt_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grbyt[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grbyt_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u16", "count_u16", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grmca[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grmca_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grbca[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grbca_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grfcs[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grfcs_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grxcf[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grxcf_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grxpf[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grxpf_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grxuo[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grxuo_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_graln[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_graln_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grflr[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grflr_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grcde[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grcde_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grfcr[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grfcr_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grovr[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grovr_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grjbr[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grjbr_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grmtue[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grmtue_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grpok[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grpok_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gruc[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gruc_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grppp[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grppp_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grcrc[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_grcrc_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tr64[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tr64_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tr127[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tr127_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tr255[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tr255_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tr511[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tr511_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tr1023[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tr1023_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tr1518[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tr1518_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_trmgv[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_trmgv_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tr2047[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tr2047_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tr4095[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tr4095_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tr9216[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tr9216_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtpkt[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtpkt_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtmca[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtmca_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtbca[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtbca_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtxpf[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtxpf_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtxcf[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtxcf_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtfcs[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtfcs_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtovr[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtovr_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtdrf[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtdrf_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtedf[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtedf_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtscl[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtscl_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtmcl[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtmcl_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtlcl[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtlcl_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtxcl[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtxcl_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtfrg[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtfrg_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtncl[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtncl_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtjbr[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtjbr_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtbyt[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtbyt_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u16", "count_u16", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtpok[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtpok_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtuc[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gtuc_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rrpkt[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rrpkt_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rrund[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rrund_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rrfrg[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rrfrg_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u8", "count_u8", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rrbyt[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count", "count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rrbyt_upper[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("count_u16", "count_u16", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mib_cntrl[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rx_cnt_rst", "rx_cnt_rst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("runt_cnt_rst", "runt_cnt_rst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tx_cnt_rst", "tx_cnt_rst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mib_read_data[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("data32", "data32", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mib_write_data[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("data32", "data32", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_control[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mpd_en", "mpd_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mseq_len", "mseq_len", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("psw_en", "psw_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_psw_ms[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("psw_47_32", "psw_47_32", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_psw_ls[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("psw_31_0", "psw_31_0", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_control_1[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("xib_rx_en", "xib_rx_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("xib_tx_en", "xib_tx_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rx_flush_en", "rx_flush_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tx_flush_en", "tx_flush_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("link_down_rst_en", "link_down_rst_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("standard_mux_en", "standard_mux_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("xgmii_sel", "xgmii_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("dic_dis", "dic_dis", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rx_start_threshold", "rx_start_threshold", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("gmii_rx_clk_gate_en", "gmii_rx_clk_gate_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("strict_preamble_dis", "strict_preamble_dis", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tx_ipg", "tx_ipg", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("min_rx_ipg", "min_rx_ipg", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("xgmii_sel_ovrd", "xgmii_sel_ovrd", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("gmii_tx_clk_gate_en", "gmii_tx_clk_gate_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("autoconfig_en", "autoconfig_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_extended_control[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tx_start_threshold", "tx_start_threshold", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tx_xoff_threshold", "tx_xoff_threshold", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tx_xon_threshold", "tx_xon_threshold", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tx_backpressure_en", "tx_backpressure_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_idle_stuffing_control[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tx_idle_stuffing_ctrl", "tx_idle_stuffing_ctrl", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_actual_data_rate[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("actual_data_rate", "actual_data_rate", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gmii_clock_swallower_control[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("ndiv", "ndiv", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mdiv", "mdiv", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_discard_packet_counter[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pkt_count", "pkt_count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_discard_packet_counter[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("pkt_count", "pkt_count", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_umac_rxerr_mask[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mac_rxerr_mask", "mac_rxerr_mask", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mib_max_pkt_size[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("max_pkt_size", "max_pkt_size", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "ipg_hd_bkp_cntl", .val = cli_xumac_rdp_ipg_hd_bkp_cntl, .parms = set_ipg_hd_bkp_cntl },
            { .name = "command_config", .val = cli_xumac_rdp_command_config, .parms = set_command_config },
            { .name = "mac_0", .val = cli_xumac_rdp_mac_0, .parms = set_mac_0 },
            { .name = "mac_1", .val = cli_xumac_rdp_mac_1, .parms = set_mac_1 },
            { .name = "frm_length", .val = cli_xumac_rdp_frm_length, .parms = set_frm_length },
            { .name = "pause_quant", .val = cli_xumac_rdp_pause_quant, .parms = set_pause_quant },
            { .name = "sfd_offset", .val = cli_xumac_rdp_sfd_offset, .parms = set_sfd_offset },
            { .name = "tag_0", .val = cli_xumac_rdp_tag_0, .parms = set_tag_0 },
            { .name = "tag_1", .val = cli_xumac_rdp_tag_1, .parms = set_tag_1 },
            { .name = "rx_pause_quanta_scale", .val = cli_xumac_rdp_rx_pause_quanta_scale, .parms = set_rx_pause_quanta_scale },
            { .name = "tx_preamble", .val = cli_xumac_rdp_tx_preamble, .parms = set_tx_preamble },
            { .name = "tx_ipg_length", .val = cli_xumac_rdp_tx_ipg_length, .parms = set_tx_ipg_length },
            { .name = "pfc_xoff_timer", .val = cli_xumac_rdp_pfc_xoff_timer, .parms = set_pfc_xoff_timer },
            { .name = "umac_eee_ctrl", .val = cli_xumac_rdp_umac_eee_ctrl, .parms = set_umac_eee_ctrl },
            { .name = "mii_eee_delay_entry_timer", .val = cli_xumac_rdp_mii_eee_delay_entry_timer, .parms = set_mii_eee_delay_entry_timer },
            { .name = "gmii_eee_delay_entry_timer", .val = cli_xumac_rdp_gmii_eee_delay_entry_timer, .parms = set_gmii_eee_delay_entry_timer },
            { .name = "umac_eee_ref_count", .val = cli_xumac_rdp_umac_eee_ref_count, .parms = set_umac_eee_ref_count },
            { .name = "umac_timestamp_adjust", .val = cli_xumac_rdp_umac_timestamp_adjust, .parms = set_umac_timestamp_adjust },
            { .name = "umac_rx_pkt_drop_status", .val = cli_xumac_rdp_umac_rx_pkt_drop_status, .parms = set_umac_rx_pkt_drop_status },
            { .name = "umac_symmetric_idle_threshold", .val = cli_xumac_rdp_umac_symmetric_idle_threshold, .parms = set_umac_symmetric_idle_threshold },
            { .name = "mii_eee_wake_timer", .val = cli_xumac_rdp_mii_eee_wake_timer, .parms = set_mii_eee_wake_timer },
            { .name = "gmii_eee_wake_timer", .val = cli_xumac_rdp_gmii_eee_wake_timer, .parms = set_gmii_eee_wake_timer },
            { .name = "gmii_2p5g_eee_delay_entry_timer", .val = cli_xumac_rdp_gmii_2p5g_eee_delay_entry_timer, .parms = set_gmii_2p5g_eee_delay_entry_timer },
            { .name = "gmii_5g_eee_delay_entry_timer", .val = cli_xumac_rdp_gmii_5g_eee_delay_entry_timer, .parms = set_gmii_5g_eee_delay_entry_timer },
            { .name = "gmii_10g_eee_delay_entry_timer", .val = cli_xumac_rdp_gmii_10g_eee_delay_entry_timer, .parms = set_gmii_10g_eee_delay_entry_timer },
            { .name = "gmii_2p5g_eee_wake_timer", .val = cli_xumac_rdp_gmii_2p5g_eee_wake_timer, .parms = set_gmii_2p5g_eee_wake_timer },
            { .name = "gmii_5g_eee_wake_timer", .val = cli_xumac_rdp_gmii_5g_eee_wake_timer, .parms = set_gmii_5g_eee_wake_timer },
            { .name = "gmii_10g_eee_wake_timer", .val = cli_xumac_rdp_gmii_10g_eee_wake_timer, .parms = set_gmii_10g_eee_wake_timer },
            { .name = "mac_pfc_type", .val = cli_xumac_rdp_mac_pfc_type, .parms = set_mac_pfc_type },
            { .name = "mac_pfc_opcode", .val = cli_xumac_rdp_mac_pfc_opcode, .parms = set_mac_pfc_opcode },
            { .name = "mac_pfc_da_0", .val = cli_xumac_rdp_mac_pfc_da_0, .parms = set_mac_pfc_da_0 },
            { .name = "mac_pfc_da_1", .val = cli_xumac_rdp_mac_pfc_da_1, .parms = set_mac_pfc_da_1 },
            { .name = "macsec_prog_tx_crc", .val = cli_xumac_rdp_macsec_prog_tx_crc, .parms = set_macsec_prog_tx_crc },
            { .name = "macsec_cntrl", .val = cli_xumac_rdp_macsec_cntrl, .parms = set_macsec_cntrl },
            { .name = "pause_refresh_ctrl", .val = cli_xumac_rdp_pause_refresh_ctrl, .parms = set_pause_refresh_ctrl },
            { .name = "flush_control", .val = cli_xumac_rdp_flush_control, .parms = set_flush_control },
            { .name = "mac_pfc_ctrl", .val = cli_xumac_rdp_mac_pfc_ctrl, .parms = set_mac_pfc_ctrl },
            { .name = "mac_pfc_refresh_ctrl", .val = cli_xumac_rdp_mac_pfc_refresh_ctrl, .parms = set_mac_pfc_refresh_ctrl },
            { .name = "gr64", .val = cli_xumac_rdp_gr64, .parms = set_gr64 },
            { .name = "gr64_upper", .val = cli_xumac_rdp_gr64_upper, .parms = set_gr64_upper },
            { .name = "gr127", .val = cli_xumac_rdp_gr127, .parms = set_gr127 },
            { .name = "gr127_upper", .val = cli_xumac_rdp_gr127_upper, .parms = set_gr127_upper },
            { .name = "gr255", .val = cli_xumac_rdp_gr255, .parms = set_gr255 },
            { .name = "gr255_upper", .val = cli_xumac_rdp_gr255_upper, .parms = set_gr255_upper },
            { .name = "gr511", .val = cli_xumac_rdp_gr511, .parms = set_gr511 },
            { .name = "gr511_upper", .val = cli_xumac_rdp_gr511_upper, .parms = set_gr511_upper },
            { .name = "gr1023", .val = cli_xumac_rdp_gr1023, .parms = set_gr1023 },
            { .name = "gr1023_upper", .val = cli_xumac_rdp_gr1023_upper, .parms = set_gr1023_upper },
            { .name = "gr1518", .val = cli_xumac_rdp_gr1518, .parms = set_gr1518 },
            { .name = "gr1518_upper", .val = cli_xumac_rdp_gr1518_upper, .parms = set_gr1518_upper },
            { .name = "grmgv", .val = cli_xumac_rdp_grmgv, .parms = set_grmgv },
            { .name = "grmgv_upper", .val = cli_xumac_rdp_grmgv_upper, .parms = set_grmgv_upper },
            { .name = "gr2047", .val = cli_xumac_rdp_gr2047, .parms = set_gr2047 },
            { .name = "gr2047_upper", .val = cli_xumac_rdp_gr2047_upper, .parms = set_gr2047_upper },
            { .name = "gr4095", .val = cli_xumac_rdp_gr4095, .parms = set_gr4095 },
            { .name = "gr4095_upper", .val = cli_xumac_rdp_gr4095_upper, .parms = set_gr4095_upper },
            { .name = "gr9216", .val = cli_xumac_rdp_gr9216, .parms = set_gr9216 },
            { .name = "gr9216_upper", .val = cli_xumac_rdp_gr9216_upper, .parms = set_gr9216_upper },
            { .name = "grpkt", .val = cli_xumac_rdp_grpkt, .parms = set_grpkt },
            { .name = "grpkt_upper", .val = cli_xumac_rdp_grpkt_upper, .parms = set_grpkt_upper },
            { .name = "grbyt", .val = cli_xumac_rdp_grbyt, .parms = set_grbyt },
            { .name = "grbyt_upper", .val = cli_xumac_rdp_grbyt_upper, .parms = set_grbyt_upper },
            { .name = "grmca", .val = cli_xumac_rdp_grmca, .parms = set_grmca },
            { .name = "grmca_upper", .val = cli_xumac_rdp_grmca_upper, .parms = set_grmca_upper },
            { .name = "grbca", .val = cli_xumac_rdp_grbca, .parms = set_grbca },
            { .name = "grbca_upper", .val = cli_xumac_rdp_grbca_upper, .parms = set_grbca_upper },
            { .name = "grfcs", .val = cli_xumac_rdp_grfcs, .parms = set_grfcs },
            { .name = "grfcs_upper", .val = cli_xumac_rdp_grfcs_upper, .parms = set_grfcs_upper },
            { .name = "grxcf", .val = cli_xumac_rdp_grxcf, .parms = set_grxcf },
            { .name = "grxcf_upper", .val = cli_xumac_rdp_grxcf_upper, .parms = set_grxcf_upper },
            { .name = "grxpf", .val = cli_xumac_rdp_grxpf, .parms = set_grxpf },
            { .name = "grxpf_upper", .val = cli_xumac_rdp_grxpf_upper, .parms = set_grxpf_upper },
            { .name = "grxuo", .val = cli_xumac_rdp_grxuo, .parms = set_grxuo },
            { .name = "grxuo_upper", .val = cli_xumac_rdp_grxuo_upper, .parms = set_grxuo_upper },
            { .name = "graln", .val = cli_xumac_rdp_graln, .parms = set_graln },
            { .name = "graln_upper", .val = cli_xumac_rdp_graln_upper, .parms = set_graln_upper },
            { .name = "grflr", .val = cli_xumac_rdp_grflr, .parms = set_grflr },
            { .name = "grflr_upper", .val = cli_xumac_rdp_grflr_upper, .parms = set_grflr_upper },
            { .name = "grcde", .val = cli_xumac_rdp_grcde, .parms = set_grcde },
            { .name = "grcde_upper", .val = cli_xumac_rdp_grcde_upper, .parms = set_grcde_upper },
            { .name = "grfcr", .val = cli_xumac_rdp_grfcr, .parms = set_grfcr },
            { .name = "grfcr_upper", .val = cli_xumac_rdp_grfcr_upper, .parms = set_grfcr_upper },
            { .name = "grovr", .val = cli_xumac_rdp_grovr, .parms = set_grovr },
            { .name = "grovr_upper", .val = cli_xumac_rdp_grovr_upper, .parms = set_grovr_upper },
            { .name = "grjbr", .val = cli_xumac_rdp_grjbr, .parms = set_grjbr },
            { .name = "grjbr_upper", .val = cli_xumac_rdp_grjbr_upper, .parms = set_grjbr_upper },
            { .name = "grmtue", .val = cli_xumac_rdp_grmtue, .parms = set_grmtue },
            { .name = "grmtue_upper", .val = cli_xumac_rdp_grmtue_upper, .parms = set_grmtue_upper },
            { .name = "grpok", .val = cli_xumac_rdp_grpok, .parms = set_grpok },
            { .name = "grpok_upper", .val = cli_xumac_rdp_grpok_upper, .parms = set_grpok_upper },
            { .name = "gruc", .val = cli_xumac_rdp_gruc, .parms = set_gruc },
            { .name = "gruc_upper", .val = cli_xumac_rdp_gruc_upper, .parms = set_gruc_upper },
            { .name = "grppp", .val = cli_xumac_rdp_grppp, .parms = set_grppp },
            { .name = "grppp_upper", .val = cli_xumac_rdp_grppp_upper, .parms = set_grppp_upper },
            { .name = "grcrc", .val = cli_xumac_rdp_grcrc, .parms = set_grcrc },
            { .name = "grcrc_upper", .val = cli_xumac_rdp_grcrc_upper, .parms = set_grcrc_upper },
            { .name = "tr64", .val = cli_xumac_rdp_tr64, .parms = set_tr64 },
            { .name = "tr64_upper", .val = cli_xumac_rdp_tr64_upper, .parms = set_tr64_upper },
            { .name = "tr127", .val = cli_xumac_rdp_tr127, .parms = set_tr127 },
            { .name = "tr127_upper", .val = cli_xumac_rdp_tr127_upper, .parms = set_tr127_upper },
            { .name = "tr255", .val = cli_xumac_rdp_tr255, .parms = set_tr255 },
            { .name = "tr255_upper", .val = cli_xumac_rdp_tr255_upper, .parms = set_tr255_upper },
            { .name = "tr511", .val = cli_xumac_rdp_tr511, .parms = set_tr511 },
            { .name = "tr511_upper", .val = cli_xumac_rdp_tr511_upper, .parms = set_tr511_upper },
            { .name = "tr1023", .val = cli_xumac_rdp_tr1023, .parms = set_tr1023 },
            { .name = "tr1023_upper", .val = cli_xumac_rdp_tr1023_upper, .parms = set_tr1023_upper },
            { .name = "tr1518", .val = cli_xumac_rdp_tr1518, .parms = set_tr1518 },
            { .name = "tr1518_upper", .val = cli_xumac_rdp_tr1518_upper, .parms = set_tr1518_upper },
            { .name = "trmgv", .val = cli_xumac_rdp_trmgv, .parms = set_trmgv },
            { .name = "trmgv_upper", .val = cli_xumac_rdp_trmgv_upper, .parms = set_trmgv_upper },
            { .name = "tr2047", .val = cli_xumac_rdp_tr2047, .parms = set_tr2047 },
            { .name = "tr2047_upper", .val = cli_xumac_rdp_tr2047_upper, .parms = set_tr2047_upper },
            { .name = "tr4095", .val = cli_xumac_rdp_tr4095, .parms = set_tr4095 },
            { .name = "tr4095_upper", .val = cli_xumac_rdp_tr4095_upper, .parms = set_tr4095_upper },
            { .name = "tr9216", .val = cli_xumac_rdp_tr9216, .parms = set_tr9216 },
            { .name = "tr9216_upper", .val = cli_xumac_rdp_tr9216_upper, .parms = set_tr9216_upper },
            { .name = "gtpkt", .val = cli_xumac_rdp_gtpkt, .parms = set_gtpkt },
            { .name = "gtpkt_upper", .val = cli_xumac_rdp_gtpkt_upper, .parms = set_gtpkt_upper },
            { .name = "gtmca", .val = cli_xumac_rdp_gtmca, .parms = set_gtmca },
            { .name = "gtmca_upper", .val = cli_xumac_rdp_gtmca_upper, .parms = set_gtmca_upper },
            { .name = "gtbca", .val = cli_xumac_rdp_gtbca, .parms = set_gtbca },
            { .name = "gtbca_upper", .val = cli_xumac_rdp_gtbca_upper, .parms = set_gtbca_upper },
            { .name = "gtxpf", .val = cli_xumac_rdp_gtxpf, .parms = set_gtxpf },
            { .name = "gtxpf_upper", .val = cli_xumac_rdp_gtxpf_upper, .parms = set_gtxpf_upper },
            { .name = "gtxcf", .val = cli_xumac_rdp_gtxcf, .parms = set_gtxcf },
            { .name = "gtxcf_upper", .val = cli_xumac_rdp_gtxcf_upper, .parms = set_gtxcf_upper },
            { .name = "gtfcs", .val = cli_xumac_rdp_gtfcs, .parms = set_gtfcs },
            { .name = "gtfcs_upper", .val = cli_xumac_rdp_gtfcs_upper, .parms = set_gtfcs_upper },
            { .name = "gtovr", .val = cli_xumac_rdp_gtovr, .parms = set_gtovr },
            { .name = "gtovr_upper", .val = cli_xumac_rdp_gtovr_upper, .parms = set_gtovr_upper },
            { .name = "gtdrf", .val = cli_xumac_rdp_gtdrf, .parms = set_gtdrf },
            { .name = "gtdrf_upper", .val = cli_xumac_rdp_gtdrf_upper, .parms = set_gtdrf_upper },
            { .name = "gtedf", .val = cli_xumac_rdp_gtedf, .parms = set_gtedf },
            { .name = "gtedf_upper", .val = cli_xumac_rdp_gtedf_upper, .parms = set_gtedf_upper },
            { .name = "gtscl", .val = cli_xumac_rdp_gtscl, .parms = set_gtscl },
            { .name = "gtscl_upper", .val = cli_xumac_rdp_gtscl_upper, .parms = set_gtscl_upper },
            { .name = "gtmcl", .val = cli_xumac_rdp_gtmcl, .parms = set_gtmcl },
            { .name = "gtmcl_upper", .val = cli_xumac_rdp_gtmcl_upper, .parms = set_gtmcl_upper },
            { .name = "gtlcl", .val = cli_xumac_rdp_gtlcl, .parms = set_gtlcl },
            { .name = "gtlcl_upper", .val = cli_xumac_rdp_gtlcl_upper, .parms = set_gtlcl_upper },
            { .name = "gtxcl", .val = cli_xumac_rdp_gtxcl, .parms = set_gtxcl },
            { .name = "gtxcl_upper", .val = cli_xumac_rdp_gtxcl_upper, .parms = set_gtxcl_upper },
            { .name = "gtfrg", .val = cli_xumac_rdp_gtfrg, .parms = set_gtfrg },
            { .name = "gtfrg_upper", .val = cli_xumac_rdp_gtfrg_upper, .parms = set_gtfrg_upper },
            { .name = "gtncl", .val = cli_xumac_rdp_gtncl, .parms = set_gtncl },
            { .name = "gtncl_upper", .val = cli_xumac_rdp_gtncl_upper, .parms = set_gtncl_upper },
            { .name = "gtjbr", .val = cli_xumac_rdp_gtjbr, .parms = set_gtjbr },
            { .name = "gtjbr_upper", .val = cli_xumac_rdp_gtjbr_upper, .parms = set_gtjbr_upper },
            { .name = "gtbyt", .val = cli_xumac_rdp_gtbyt, .parms = set_gtbyt },
            { .name = "gtbyt_upper", .val = cli_xumac_rdp_gtbyt_upper, .parms = set_gtbyt_upper },
            { .name = "gtpok", .val = cli_xumac_rdp_gtpok, .parms = set_gtpok },
            { .name = "gtpok_upper", .val = cli_xumac_rdp_gtpok_upper, .parms = set_gtpok_upper },
            { .name = "gtuc", .val = cli_xumac_rdp_gtuc, .parms = set_gtuc },
            { .name = "gtuc_upper", .val = cli_xumac_rdp_gtuc_upper, .parms = set_gtuc_upper },
            { .name = "rrpkt", .val = cli_xumac_rdp_rrpkt, .parms = set_rrpkt },
            { .name = "rrpkt_upper", .val = cli_xumac_rdp_rrpkt_upper, .parms = set_rrpkt_upper },
            { .name = "rrund", .val = cli_xumac_rdp_rrund, .parms = set_rrund },
            { .name = "rrund_upper", .val = cli_xumac_rdp_rrund_upper, .parms = set_rrund_upper },
            { .name = "rrfrg", .val = cli_xumac_rdp_rrfrg, .parms = set_rrfrg },
            { .name = "rrfrg_upper", .val = cli_xumac_rdp_rrfrg_upper, .parms = set_rrfrg_upper },
            { .name = "rrbyt", .val = cli_xumac_rdp_rrbyt, .parms = set_rrbyt },
            { .name = "rrbyt_upper", .val = cli_xumac_rdp_rrbyt_upper, .parms = set_rrbyt_upper },
            { .name = "mib_cntrl", .val = cli_xumac_rdp_mib_cntrl, .parms = set_mib_cntrl },
            { .name = "mib_read_data", .val = cli_xumac_rdp_mib_read_data, .parms = set_mib_read_data },
            { .name = "mib_write_data", .val = cli_xumac_rdp_mib_write_data, .parms = set_mib_write_data },
            { .name = "control", .val = cli_xumac_rdp_control, .parms = set_control },
            { .name = "psw_ms", .val = cli_xumac_rdp_psw_ms, .parms = set_psw_ms },
            { .name = "psw_ls", .val = cli_xumac_rdp_psw_ls, .parms = set_psw_ls },
            { .name = "control_1", .val = cli_xumac_rdp_control_1, .parms = set_control_1 },
            { .name = "extended_control", .val = cli_xumac_rdp_extended_control, .parms = set_extended_control },
            { .name = "tx_idle_stuffing_control", .val = cli_xumac_rdp_tx_idle_stuffing_control, .parms = set_tx_idle_stuffing_control },
            { .name = "actual_data_rate", .val = cli_xumac_rdp_actual_data_rate, .parms = set_actual_data_rate },
            { .name = "gmii_clock_swallower_control", .val = cli_xumac_rdp_gmii_clock_swallower_control, .parms = set_gmii_clock_swallower_control },
            { .name = "rx_discard_packet_counter", .val = cli_xumac_rdp_rx_discard_packet_counter, .parms = set_rx_discard_packet_counter },
            { .name = "tx_discard_packet_counter", .val = cli_xumac_rdp_tx_discard_packet_counter, .parms = set_tx_discard_packet_counter },
            { .name = "umac_rxerr_mask", .val = cli_xumac_rdp_umac_rxerr_mask, .parms = set_umac_rxerr_mask },
            { .name = "mib_max_pkt_size", .val = cli_xumac_rdp_mib_max_pkt_size, .parms = set_mib_max_pkt_size },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", ag_drv_xumac_rdp_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t get_default[] = {
            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "ipg_hd_bkp_cntl", .val = cli_xumac_rdp_ipg_hd_bkp_cntl, .parms = get_default },
            { .name = "command_config", .val = cli_xumac_rdp_command_config, .parms = get_default },
            { .name = "mac_0", .val = cli_xumac_rdp_mac_0, .parms = get_default },
            { .name = "mac_1", .val = cli_xumac_rdp_mac_1, .parms = get_default },
            { .name = "frm_length", .val = cli_xumac_rdp_frm_length, .parms = get_default },
            { .name = "pause_quant", .val = cli_xumac_rdp_pause_quant, .parms = get_default },
            { .name = "tx_ts_seq_id", .val = cli_xumac_rdp_tx_ts_seq_id, .parms = get_default },
            { .name = "sfd_offset", .val = cli_xumac_rdp_sfd_offset, .parms = get_default },
            { .name = "mac_mode", .val = cli_xumac_rdp_mac_mode, .parms = get_default },
            { .name = "tag_0", .val = cli_xumac_rdp_tag_0, .parms = get_default },
            { .name = "tag_1", .val = cli_xumac_rdp_tag_1, .parms = get_default },
            { .name = "rx_pause_quanta_scale", .val = cli_xumac_rdp_rx_pause_quanta_scale, .parms = get_default },
            { .name = "tx_preamble", .val = cli_xumac_rdp_tx_preamble, .parms = get_default },
            { .name = "tx_ipg_length", .val = cli_xumac_rdp_tx_ipg_length, .parms = get_default },
            { .name = "pfc_xoff_timer", .val = cli_xumac_rdp_pfc_xoff_timer, .parms = get_default },
            { .name = "umac_eee_ctrl", .val = cli_xumac_rdp_umac_eee_ctrl, .parms = get_default },
            { .name = "mii_eee_delay_entry_timer", .val = cli_xumac_rdp_mii_eee_delay_entry_timer, .parms = get_default },
            { .name = "gmii_eee_delay_entry_timer", .val = cli_xumac_rdp_gmii_eee_delay_entry_timer, .parms = get_default },
            { .name = "umac_eee_ref_count", .val = cli_xumac_rdp_umac_eee_ref_count, .parms = get_default },
            { .name = "umac_timestamp_adjust", .val = cli_xumac_rdp_umac_timestamp_adjust, .parms = get_default },
            { .name = "umac_rx_pkt_drop_status", .val = cli_xumac_rdp_umac_rx_pkt_drop_status, .parms = get_default },
            { .name = "umac_symmetric_idle_threshold", .val = cli_xumac_rdp_umac_symmetric_idle_threshold, .parms = get_default },
            { .name = "mii_eee_wake_timer", .val = cli_xumac_rdp_mii_eee_wake_timer, .parms = get_default },
            { .name = "gmii_eee_wake_timer", .val = cli_xumac_rdp_gmii_eee_wake_timer, .parms = get_default },
            { .name = "umac_rev_id", .val = cli_xumac_rdp_umac_rev_id, .parms = get_default },
            { .name = "gmii_2p5g_eee_delay_entry_timer", .val = cli_xumac_rdp_gmii_2p5g_eee_delay_entry_timer, .parms = get_default },
            { .name = "gmii_5g_eee_delay_entry_timer", .val = cli_xumac_rdp_gmii_5g_eee_delay_entry_timer, .parms = get_default },
            { .name = "gmii_10g_eee_delay_entry_timer", .val = cli_xumac_rdp_gmii_10g_eee_delay_entry_timer, .parms = get_default },
            { .name = "gmii_2p5g_eee_wake_timer", .val = cli_xumac_rdp_gmii_2p5g_eee_wake_timer, .parms = get_default },
            { .name = "gmii_5g_eee_wake_timer", .val = cli_xumac_rdp_gmii_5g_eee_wake_timer, .parms = get_default },
            { .name = "gmii_10g_eee_wake_timer", .val = cli_xumac_rdp_gmii_10g_eee_wake_timer, .parms = get_default },
            { .name = "active_eee_delay_entry_timer", .val = cli_xumac_rdp_active_eee_delay_entry_timer, .parms = get_default },
            { .name = "active_eee_wake_timer", .val = cli_xumac_rdp_active_eee_wake_timer, .parms = get_default },
            { .name = "mac_pfc_type", .val = cli_xumac_rdp_mac_pfc_type, .parms = get_default },
            { .name = "mac_pfc_opcode", .val = cli_xumac_rdp_mac_pfc_opcode, .parms = get_default },
            { .name = "mac_pfc_da_0", .val = cli_xumac_rdp_mac_pfc_da_0, .parms = get_default },
            { .name = "mac_pfc_da_1", .val = cli_xumac_rdp_mac_pfc_da_1, .parms = get_default },
            { .name = "macsec_prog_tx_crc", .val = cli_xumac_rdp_macsec_prog_tx_crc, .parms = get_default },
            { .name = "macsec_cntrl", .val = cli_xumac_rdp_macsec_cntrl, .parms = get_default },
            { .name = "ts_status", .val = cli_xumac_rdp_ts_status, .parms = get_default },
            { .name = "tx_ts_data", .val = cli_xumac_rdp_tx_ts_data, .parms = get_default },
            { .name = "pause_refresh_ctrl", .val = cli_xumac_rdp_pause_refresh_ctrl, .parms = get_default },
            { .name = "flush_control", .val = cli_xumac_rdp_flush_control, .parms = get_default },
            { .name = "rxfifo_stat", .val = cli_xumac_rdp_rxfifo_stat, .parms = get_default },
            { .name = "txfifo_stat", .val = cli_xumac_rdp_txfifo_stat, .parms = get_default },
            { .name = "mac_pfc_ctrl", .val = cli_xumac_rdp_mac_pfc_ctrl, .parms = get_default },
            { .name = "mac_pfc_refresh_ctrl", .val = cli_xumac_rdp_mac_pfc_refresh_ctrl, .parms = get_default },
            { .name = "gr64", .val = cli_xumac_rdp_gr64, .parms = get_default },
            { .name = "gr64_upper", .val = cli_xumac_rdp_gr64_upper, .parms = get_default },
            { .name = "gr127", .val = cli_xumac_rdp_gr127, .parms = get_default },
            { .name = "gr127_upper", .val = cli_xumac_rdp_gr127_upper, .parms = get_default },
            { .name = "gr255", .val = cli_xumac_rdp_gr255, .parms = get_default },
            { .name = "gr255_upper", .val = cli_xumac_rdp_gr255_upper, .parms = get_default },
            { .name = "gr511", .val = cli_xumac_rdp_gr511, .parms = get_default },
            { .name = "gr511_upper", .val = cli_xumac_rdp_gr511_upper, .parms = get_default },
            { .name = "gr1023", .val = cli_xumac_rdp_gr1023, .parms = get_default },
            { .name = "gr1023_upper", .val = cli_xumac_rdp_gr1023_upper, .parms = get_default },
            { .name = "gr1518", .val = cli_xumac_rdp_gr1518, .parms = get_default },
            { .name = "gr1518_upper", .val = cli_xumac_rdp_gr1518_upper, .parms = get_default },
            { .name = "grmgv", .val = cli_xumac_rdp_grmgv, .parms = get_default },
            { .name = "grmgv_upper", .val = cli_xumac_rdp_grmgv_upper, .parms = get_default },
            { .name = "gr2047", .val = cli_xumac_rdp_gr2047, .parms = get_default },
            { .name = "gr2047_upper", .val = cli_xumac_rdp_gr2047_upper, .parms = get_default },
            { .name = "gr4095", .val = cli_xumac_rdp_gr4095, .parms = get_default },
            { .name = "gr4095_upper", .val = cli_xumac_rdp_gr4095_upper, .parms = get_default },
            { .name = "gr9216", .val = cli_xumac_rdp_gr9216, .parms = get_default },
            { .name = "gr9216_upper", .val = cli_xumac_rdp_gr9216_upper, .parms = get_default },
            { .name = "grpkt", .val = cli_xumac_rdp_grpkt, .parms = get_default },
            { .name = "grpkt_upper", .val = cli_xumac_rdp_grpkt_upper, .parms = get_default },
            { .name = "grbyt", .val = cli_xumac_rdp_grbyt, .parms = get_default },
            { .name = "grbyt_upper", .val = cli_xumac_rdp_grbyt_upper, .parms = get_default },
            { .name = "grmca", .val = cli_xumac_rdp_grmca, .parms = get_default },
            { .name = "grmca_upper", .val = cli_xumac_rdp_grmca_upper, .parms = get_default },
            { .name = "grbca", .val = cli_xumac_rdp_grbca, .parms = get_default },
            { .name = "grbca_upper", .val = cli_xumac_rdp_grbca_upper, .parms = get_default },
            { .name = "grfcs", .val = cli_xumac_rdp_grfcs, .parms = get_default },
            { .name = "grfcs_upper", .val = cli_xumac_rdp_grfcs_upper, .parms = get_default },
            { .name = "grxcf", .val = cli_xumac_rdp_grxcf, .parms = get_default },
            { .name = "grxcf_upper", .val = cli_xumac_rdp_grxcf_upper, .parms = get_default },
            { .name = "grxpf", .val = cli_xumac_rdp_grxpf, .parms = get_default },
            { .name = "grxpf_upper", .val = cli_xumac_rdp_grxpf_upper, .parms = get_default },
            { .name = "grxuo", .val = cli_xumac_rdp_grxuo, .parms = get_default },
            { .name = "grxuo_upper", .val = cli_xumac_rdp_grxuo_upper, .parms = get_default },
            { .name = "graln", .val = cli_xumac_rdp_graln, .parms = get_default },
            { .name = "graln_upper", .val = cli_xumac_rdp_graln_upper, .parms = get_default },
            { .name = "grflr", .val = cli_xumac_rdp_grflr, .parms = get_default },
            { .name = "grflr_upper", .val = cli_xumac_rdp_grflr_upper, .parms = get_default },
            { .name = "grcde", .val = cli_xumac_rdp_grcde, .parms = get_default },
            { .name = "grcde_upper", .val = cli_xumac_rdp_grcde_upper, .parms = get_default },
            { .name = "grfcr", .val = cli_xumac_rdp_grfcr, .parms = get_default },
            { .name = "grfcr_upper", .val = cli_xumac_rdp_grfcr_upper, .parms = get_default },
            { .name = "grovr", .val = cli_xumac_rdp_grovr, .parms = get_default },
            { .name = "grovr_upper", .val = cli_xumac_rdp_grovr_upper, .parms = get_default },
            { .name = "grjbr", .val = cli_xumac_rdp_grjbr, .parms = get_default },
            { .name = "grjbr_upper", .val = cli_xumac_rdp_grjbr_upper, .parms = get_default },
            { .name = "grmtue", .val = cli_xumac_rdp_grmtue, .parms = get_default },
            { .name = "grmtue_upper", .val = cli_xumac_rdp_grmtue_upper, .parms = get_default },
            { .name = "grpok", .val = cli_xumac_rdp_grpok, .parms = get_default },
            { .name = "grpok_upper", .val = cli_xumac_rdp_grpok_upper, .parms = get_default },
            { .name = "gruc", .val = cli_xumac_rdp_gruc, .parms = get_default },
            { .name = "gruc_upper", .val = cli_xumac_rdp_gruc_upper, .parms = get_default },
            { .name = "grppp", .val = cli_xumac_rdp_grppp, .parms = get_default },
            { .name = "grppp_upper", .val = cli_xumac_rdp_grppp_upper, .parms = get_default },
            { .name = "grcrc", .val = cli_xumac_rdp_grcrc, .parms = get_default },
            { .name = "grcrc_upper", .val = cli_xumac_rdp_grcrc_upper, .parms = get_default },
            { .name = "tr64", .val = cli_xumac_rdp_tr64, .parms = get_default },
            { .name = "tr64_upper", .val = cli_xumac_rdp_tr64_upper, .parms = get_default },
            { .name = "tr127", .val = cli_xumac_rdp_tr127, .parms = get_default },
            { .name = "tr127_upper", .val = cli_xumac_rdp_tr127_upper, .parms = get_default },
            { .name = "tr255", .val = cli_xumac_rdp_tr255, .parms = get_default },
            { .name = "tr255_upper", .val = cli_xumac_rdp_tr255_upper, .parms = get_default },
            { .name = "tr511", .val = cli_xumac_rdp_tr511, .parms = get_default },
            { .name = "tr511_upper", .val = cli_xumac_rdp_tr511_upper, .parms = get_default },
            { .name = "tr1023", .val = cli_xumac_rdp_tr1023, .parms = get_default },
            { .name = "tr1023_upper", .val = cli_xumac_rdp_tr1023_upper, .parms = get_default },
            { .name = "tr1518", .val = cli_xumac_rdp_tr1518, .parms = get_default },
            { .name = "tr1518_upper", .val = cli_xumac_rdp_tr1518_upper, .parms = get_default },
            { .name = "trmgv", .val = cli_xumac_rdp_trmgv, .parms = get_default },
            { .name = "trmgv_upper", .val = cli_xumac_rdp_trmgv_upper, .parms = get_default },
            { .name = "tr2047", .val = cli_xumac_rdp_tr2047, .parms = get_default },
            { .name = "tr2047_upper", .val = cli_xumac_rdp_tr2047_upper, .parms = get_default },
            { .name = "tr4095", .val = cli_xumac_rdp_tr4095, .parms = get_default },
            { .name = "tr4095_upper", .val = cli_xumac_rdp_tr4095_upper, .parms = get_default },
            { .name = "tr9216", .val = cli_xumac_rdp_tr9216, .parms = get_default },
            { .name = "tr9216_upper", .val = cli_xumac_rdp_tr9216_upper, .parms = get_default },
            { .name = "gtpkt", .val = cli_xumac_rdp_gtpkt, .parms = get_default },
            { .name = "gtpkt_upper", .val = cli_xumac_rdp_gtpkt_upper, .parms = get_default },
            { .name = "gtmca", .val = cli_xumac_rdp_gtmca, .parms = get_default },
            { .name = "gtmca_upper", .val = cli_xumac_rdp_gtmca_upper, .parms = get_default },
            { .name = "gtbca", .val = cli_xumac_rdp_gtbca, .parms = get_default },
            { .name = "gtbca_upper", .val = cli_xumac_rdp_gtbca_upper, .parms = get_default },
            { .name = "gtxpf", .val = cli_xumac_rdp_gtxpf, .parms = get_default },
            { .name = "gtxpf_upper", .val = cli_xumac_rdp_gtxpf_upper, .parms = get_default },
            { .name = "gtxcf", .val = cli_xumac_rdp_gtxcf, .parms = get_default },
            { .name = "gtxcf_upper", .val = cli_xumac_rdp_gtxcf_upper, .parms = get_default },
            { .name = "gtfcs", .val = cli_xumac_rdp_gtfcs, .parms = get_default },
            { .name = "gtfcs_upper", .val = cli_xumac_rdp_gtfcs_upper, .parms = get_default },
            { .name = "gtovr", .val = cli_xumac_rdp_gtovr, .parms = get_default },
            { .name = "gtovr_upper", .val = cli_xumac_rdp_gtovr_upper, .parms = get_default },
            { .name = "gtdrf", .val = cli_xumac_rdp_gtdrf, .parms = get_default },
            { .name = "gtdrf_upper", .val = cli_xumac_rdp_gtdrf_upper, .parms = get_default },
            { .name = "gtedf", .val = cli_xumac_rdp_gtedf, .parms = get_default },
            { .name = "gtedf_upper", .val = cli_xumac_rdp_gtedf_upper, .parms = get_default },
            { .name = "gtscl", .val = cli_xumac_rdp_gtscl, .parms = get_default },
            { .name = "gtscl_upper", .val = cli_xumac_rdp_gtscl_upper, .parms = get_default },
            { .name = "gtmcl", .val = cli_xumac_rdp_gtmcl, .parms = get_default },
            { .name = "gtmcl_upper", .val = cli_xumac_rdp_gtmcl_upper, .parms = get_default },
            { .name = "gtlcl", .val = cli_xumac_rdp_gtlcl, .parms = get_default },
            { .name = "gtlcl_upper", .val = cli_xumac_rdp_gtlcl_upper, .parms = get_default },
            { .name = "gtxcl", .val = cli_xumac_rdp_gtxcl, .parms = get_default },
            { .name = "gtxcl_upper", .val = cli_xumac_rdp_gtxcl_upper, .parms = get_default },
            { .name = "gtfrg", .val = cli_xumac_rdp_gtfrg, .parms = get_default },
            { .name = "gtfrg_upper", .val = cli_xumac_rdp_gtfrg_upper, .parms = get_default },
            { .name = "gtncl", .val = cli_xumac_rdp_gtncl, .parms = get_default },
            { .name = "gtncl_upper", .val = cli_xumac_rdp_gtncl_upper, .parms = get_default },
            { .name = "gtjbr", .val = cli_xumac_rdp_gtjbr, .parms = get_default },
            { .name = "gtjbr_upper", .val = cli_xumac_rdp_gtjbr_upper, .parms = get_default },
            { .name = "gtbyt", .val = cli_xumac_rdp_gtbyt, .parms = get_default },
            { .name = "gtbyt_upper", .val = cli_xumac_rdp_gtbyt_upper, .parms = get_default },
            { .name = "gtpok", .val = cli_xumac_rdp_gtpok, .parms = get_default },
            { .name = "gtpok_upper", .val = cli_xumac_rdp_gtpok_upper, .parms = get_default },
            { .name = "gtuc", .val = cli_xumac_rdp_gtuc, .parms = get_default },
            { .name = "gtuc_upper", .val = cli_xumac_rdp_gtuc_upper, .parms = get_default },
            { .name = "rrpkt", .val = cli_xumac_rdp_rrpkt, .parms = get_default },
            { .name = "rrpkt_upper", .val = cli_xumac_rdp_rrpkt_upper, .parms = get_default },
            { .name = "rrund", .val = cli_xumac_rdp_rrund, .parms = get_default },
            { .name = "rrund_upper", .val = cli_xumac_rdp_rrund_upper, .parms = get_default },
            { .name = "rrfrg", .val = cli_xumac_rdp_rrfrg, .parms = get_default },
            { .name = "rrfrg_upper", .val = cli_xumac_rdp_rrfrg_upper, .parms = get_default },
            { .name = "rrbyt", .val = cli_xumac_rdp_rrbyt, .parms = get_default },
            { .name = "rrbyt_upper", .val = cli_xumac_rdp_rrbyt_upper, .parms = get_default },
            { .name = "mib_cntrl", .val = cli_xumac_rdp_mib_cntrl, .parms = get_default },
            { .name = "mib_read_data", .val = cli_xumac_rdp_mib_read_data, .parms = get_default },
            { .name = "mib_write_data", .val = cli_xumac_rdp_mib_write_data, .parms = get_default },
            { .name = "control", .val = cli_xumac_rdp_control, .parms = get_default },
            { .name = "psw_ms", .val = cli_xumac_rdp_psw_ms, .parms = get_default },
            { .name = "psw_ls", .val = cli_xumac_rdp_psw_ls, .parms = get_default },
            { .name = "control_1", .val = cli_xumac_rdp_control_1, .parms = get_default },
            { .name = "extended_control", .val = cli_xumac_rdp_extended_control, .parms = get_default },
            { .name = "tx_idle_stuffing_control", .val = cli_xumac_rdp_tx_idle_stuffing_control, .parms = get_default },
            { .name = "actual_data_rate", .val = cli_xumac_rdp_actual_data_rate, .parms = get_default },
            { .name = "gmii_clock_swallower_control", .val = cli_xumac_rdp_gmii_clock_swallower_control, .parms = get_default },
            { .name = "xgmii_data_rate_status", .val = cli_xumac_rdp_xgmii_data_rate_status, .parms = get_default },
            { .name = "status", .val = cli_xumac_rdp_status, .parms = get_default },
            { .name = "rx_discard_packet_counter", .val = cli_xumac_rdp_rx_discard_packet_counter, .parms = get_default },
            { .name = "tx_discard_packet_counter", .val = cli_xumac_rdp_tx_discard_packet_counter, .parms = get_default },
            { .name = "rev", .val = cli_xumac_rdp_rev, .parms = get_default },
            { .name = "umac_rxerr_mask", .val = cli_xumac_rdp_umac_rxerr_mask, .parms = get_default },
            { .name = "mib_max_pkt_size", .val = cli_xumac_rdp_mib_max_pkt_size, .parms = get_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_xumac_rdp_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name = "low", .val = ag_drv_cli_test_method_low },
            { .name = "mid", .val = ag_drv_cli_test_method_mid },
            { .name = "high", .val = ag_drv_cli_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "test", "test", ag_drv_xumac_rdp_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0)            BDMFMON_MAKE_PARM("xumac_id", "xumac_id", BDMFMON_PARM_UNUMBER, 0));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name = "IPG_HD_BKP_CNTL", .val = bdmf_address_ipg_hd_bkp_cntl },
            { .name = "COMMAND_CONFIG", .val = bdmf_address_command_config },
            { .name = "MAC_0", .val = bdmf_address_mac_0 },
            { .name = "MAC_1", .val = bdmf_address_mac_1 },
            { .name = "FRM_LENGTH", .val = bdmf_address_frm_length },
            { .name = "PAUSE_QUANT", .val = bdmf_address_pause_quant },
            { .name = "TX_TS_SEQ_ID", .val = bdmf_address_tx_ts_seq_id },
            { .name = "SFD_OFFSET", .val = bdmf_address_sfd_offset },
            { .name = "MAC_MODE", .val = bdmf_address_mac_mode },
            { .name = "TAG_0", .val = bdmf_address_tag_0 },
            { .name = "TAG_1", .val = bdmf_address_tag_1 },
            { .name = "RX_PAUSE_QUANTA_SCALE", .val = bdmf_address_rx_pause_quanta_scale },
            { .name = "TX_PREAMBLE", .val = bdmf_address_tx_preamble },
            { .name = "TX_IPG_LENGTH", .val = bdmf_address_tx_ipg_length },
            { .name = "PFC_XOFF_TIMER", .val = bdmf_address_pfc_xoff_timer },
            { .name = "UMAC_EEE_CTRL", .val = bdmf_address_umac_eee_ctrl },
            { .name = "MII_EEE_DELAY_ENTRY_TIMER", .val = bdmf_address_mii_eee_delay_entry_timer },
            { .name = "GMII_EEE_DELAY_ENTRY_TIMER", .val = bdmf_address_gmii_eee_delay_entry_timer },
            { .name = "UMAC_EEE_REF_COUNT", .val = bdmf_address_umac_eee_ref_count },
            { .name = "UMAC_TIMESTAMP_ADJUST", .val = bdmf_address_umac_timestamp_adjust },
            { .name = "UMAC_RX_PKT_DROP_STATUS", .val = bdmf_address_umac_rx_pkt_drop_status },
            { .name = "UMAC_SYMMETRIC_IDLE_THRESHOLD", .val = bdmf_address_umac_symmetric_idle_threshold },
            { .name = "MII_EEE_WAKE_TIMER", .val = bdmf_address_mii_eee_wake_timer },
            { .name = "GMII_EEE_WAKE_TIMER", .val = bdmf_address_gmii_eee_wake_timer },
            { .name = "UMAC_REV_ID", .val = bdmf_address_umac_rev_id },
            { .name = "GMII_2P5G_EEE_DELAY_ENTRY_TIMER", .val = bdmf_address_gmii_2p5g_eee_delay_entry_timer },
            { .name = "GMII_5G_EEE_DELAY_ENTRY_TIMER", .val = bdmf_address_gmii_5g_eee_delay_entry_timer },
            { .name = "GMII_10G_EEE_DELAY_ENTRY_TIMER", .val = bdmf_address_gmii_10g_eee_delay_entry_timer },
            { .name = "GMII_2P5G_EEE_WAKE_TIMER", .val = bdmf_address_gmii_2p5g_eee_wake_timer },
            { .name = "GMII_5G_EEE_WAKE_TIMER", .val = bdmf_address_gmii_5g_eee_wake_timer },
            { .name = "GMII_10G_EEE_WAKE_TIMER", .val = bdmf_address_gmii_10g_eee_wake_timer },
            { .name = "ACTIVE_EEE_DELAY_ENTRY_TIMER", .val = bdmf_address_active_eee_delay_entry_timer },
            { .name = "ACTIVE_EEE_WAKE_TIMER", .val = bdmf_address_active_eee_wake_timer },
            { .name = "MAC_PFC_TYPE", .val = bdmf_address_mac_pfc_type },
            { .name = "MAC_PFC_OPCODE", .val = bdmf_address_mac_pfc_opcode },
            { .name = "MAC_PFC_DA_0", .val = bdmf_address_mac_pfc_da_0 },
            { .name = "MAC_PFC_DA_1", .val = bdmf_address_mac_pfc_da_1 },
            { .name = "MACSEC_PROG_TX_CRC", .val = bdmf_address_macsec_prog_tx_crc },
            { .name = "MACSEC_CNTRL", .val = bdmf_address_macsec_cntrl },
            { .name = "TS_STATUS", .val = bdmf_address_ts_status },
            { .name = "TX_TS_DATA", .val = bdmf_address_tx_ts_data },
            { .name = "PAUSE_REFRESH_CTRL", .val = bdmf_address_pause_refresh_ctrl },
            { .name = "FLUSH_CONTROL", .val = bdmf_address_flush_control },
            { .name = "RXFIFO_STAT", .val = bdmf_address_rxfifo_stat },
            { .name = "TXFIFO_STAT", .val = bdmf_address_txfifo_stat },
            { .name = "MAC_PFC_CTRL", .val = bdmf_address_mac_pfc_ctrl },
            { .name = "MAC_PFC_REFRESH_CTRL", .val = bdmf_address_mac_pfc_refresh_ctrl },
            { .name = "GR64", .val = bdmf_address_gr64 },
            { .name = "GR64_UPPER", .val = bdmf_address_gr64_upper },
            { .name = "GR127", .val = bdmf_address_gr127 },
            { .name = "GR127_UPPER", .val = bdmf_address_gr127_upper },
            { .name = "GR255", .val = bdmf_address_gr255 },
            { .name = "GR255_UPPER", .val = bdmf_address_gr255_upper },
            { .name = "GR511", .val = bdmf_address_gr511 },
            { .name = "GR511_UPPER", .val = bdmf_address_gr511_upper },
            { .name = "GR1023", .val = bdmf_address_gr1023 },
            { .name = "GR1023_UPPER", .val = bdmf_address_gr1023_upper },
            { .name = "GR1518", .val = bdmf_address_gr1518 },
            { .name = "GR1518_UPPER", .val = bdmf_address_gr1518_upper },
            { .name = "GRMGV", .val = bdmf_address_grmgv },
            { .name = "GRMGV_UPPER", .val = bdmf_address_grmgv_upper },
            { .name = "GR2047", .val = bdmf_address_gr2047 },
            { .name = "GR2047_UPPER", .val = bdmf_address_gr2047_upper },
            { .name = "GR4095", .val = bdmf_address_gr4095 },
            { .name = "GR4095_UPPER", .val = bdmf_address_gr4095_upper },
            { .name = "GR9216", .val = bdmf_address_gr9216 },
            { .name = "GR9216_UPPER", .val = bdmf_address_gr9216_upper },
            { .name = "GRPKT", .val = bdmf_address_grpkt },
            { .name = "GRPKT_UPPER", .val = bdmf_address_grpkt_upper },
            { .name = "GRBYT", .val = bdmf_address_grbyt },
            { .name = "GRBYT_UPPER", .val = bdmf_address_grbyt_upper },
            { .name = "GRMCA", .val = bdmf_address_grmca },
            { .name = "GRMCA_UPPER", .val = bdmf_address_grmca_upper },
            { .name = "GRBCA", .val = bdmf_address_grbca },
            { .name = "GRBCA_UPPER", .val = bdmf_address_grbca_upper },
            { .name = "GRFCS", .val = bdmf_address_grfcs },
            { .name = "GRFCS_UPPER", .val = bdmf_address_grfcs_upper },
            { .name = "GRXCF", .val = bdmf_address_grxcf },
            { .name = "GRXCF_UPPER", .val = bdmf_address_grxcf_upper },
            { .name = "GRXPF", .val = bdmf_address_grxpf },
            { .name = "GRXPF_UPPER", .val = bdmf_address_grxpf_upper },
            { .name = "GRXUO", .val = bdmf_address_grxuo },
            { .name = "GRXUO_UPPER", .val = bdmf_address_grxuo_upper },
            { .name = "GRALN", .val = bdmf_address_graln },
            { .name = "GRALN_UPPER", .val = bdmf_address_graln_upper },
            { .name = "GRFLR", .val = bdmf_address_grflr },
            { .name = "GRFLR_UPPER", .val = bdmf_address_grflr_upper },
            { .name = "GRCDE", .val = bdmf_address_grcde },
            { .name = "GRCDE_UPPER", .val = bdmf_address_grcde_upper },
            { .name = "GRFCR", .val = bdmf_address_grfcr },
            { .name = "GRFCR_UPPER", .val = bdmf_address_grfcr_upper },
            { .name = "GROVR", .val = bdmf_address_grovr },
            { .name = "GROVR_UPPER", .val = bdmf_address_grovr_upper },
            { .name = "GRJBR", .val = bdmf_address_grjbr },
            { .name = "GRJBR_UPPER", .val = bdmf_address_grjbr_upper },
            { .name = "GRMTUE", .val = bdmf_address_grmtue },
            { .name = "GRMTUE_UPPER", .val = bdmf_address_grmtue_upper },
            { .name = "GRPOK", .val = bdmf_address_grpok },
            { .name = "GRPOK_UPPER", .val = bdmf_address_grpok_upper },
            { .name = "GRUC", .val = bdmf_address_gruc },
            { .name = "GRUC_UPPER", .val = bdmf_address_gruc_upper },
            { .name = "GRPPP", .val = bdmf_address_grppp },
            { .name = "GRPPP_UPPER", .val = bdmf_address_grppp_upper },
            { .name = "GRCRC", .val = bdmf_address_grcrc },
            { .name = "GRCRC_UPPER", .val = bdmf_address_grcrc_upper },
            { .name = "TR64", .val = bdmf_address_tr64 },
            { .name = "TR64_UPPER", .val = bdmf_address_tr64_upper },
            { .name = "TR127", .val = bdmf_address_tr127 },
            { .name = "TR127_UPPER", .val = bdmf_address_tr127_upper },
            { .name = "TR255", .val = bdmf_address_tr255 },
            { .name = "TR255_UPPER", .val = bdmf_address_tr255_upper },
            { .name = "TR511", .val = bdmf_address_tr511 },
            { .name = "TR511_UPPER", .val = bdmf_address_tr511_upper },
            { .name = "TR1023", .val = bdmf_address_tr1023 },
            { .name = "TR1023_UPPER", .val = bdmf_address_tr1023_upper },
            { .name = "TR1518", .val = bdmf_address_tr1518 },
            { .name = "TR1518_UPPER", .val = bdmf_address_tr1518_upper },
            { .name = "TRMGV", .val = bdmf_address_trmgv },
            { .name = "TRMGV_UPPER", .val = bdmf_address_trmgv_upper },
            { .name = "TR2047", .val = bdmf_address_tr2047 },
            { .name = "TR2047_UPPER", .val = bdmf_address_tr2047_upper },
            { .name = "TR4095", .val = bdmf_address_tr4095 },
            { .name = "TR4095_UPPER", .val = bdmf_address_tr4095_upper },
            { .name = "TR9216", .val = bdmf_address_tr9216 },
            { .name = "TR9216_UPPER", .val = bdmf_address_tr9216_upper },
            { .name = "GTPKT", .val = bdmf_address_gtpkt },
            { .name = "GTPKT_UPPER", .val = bdmf_address_gtpkt_upper },
            { .name = "GTMCA", .val = bdmf_address_gtmca },
            { .name = "GTMCA_UPPER", .val = bdmf_address_gtmca_upper },
            { .name = "GTBCA", .val = bdmf_address_gtbca },
            { .name = "GTBCA_UPPER", .val = bdmf_address_gtbca_upper },
            { .name = "GTXPF", .val = bdmf_address_gtxpf },
            { .name = "GTXPF_UPPER", .val = bdmf_address_gtxpf_upper },
            { .name = "GTXCF", .val = bdmf_address_gtxcf },
            { .name = "GTXCF_UPPER", .val = bdmf_address_gtxcf_upper },
            { .name = "GTFCS", .val = bdmf_address_gtfcs },
            { .name = "GTFCS_UPPER", .val = bdmf_address_gtfcs_upper },
            { .name = "GTOVR", .val = bdmf_address_gtovr },
            { .name = "GTOVR_UPPER", .val = bdmf_address_gtovr_upper },
            { .name = "GTDRF", .val = bdmf_address_gtdrf },
            { .name = "GTDRF_UPPER", .val = bdmf_address_gtdrf_upper },
            { .name = "GTEDF", .val = bdmf_address_gtedf },
            { .name = "GTEDF_UPPER", .val = bdmf_address_gtedf_upper },
            { .name = "GTSCL", .val = bdmf_address_gtscl },
            { .name = "GTSCL_UPPER", .val = bdmf_address_gtscl_upper },
            { .name = "GTMCL", .val = bdmf_address_gtmcl },
            { .name = "GTMCL_UPPER", .val = bdmf_address_gtmcl_upper },
            { .name = "GTLCL", .val = bdmf_address_gtlcl },
            { .name = "GTLCL_UPPER", .val = bdmf_address_gtlcl_upper },
            { .name = "GTXCL", .val = bdmf_address_gtxcl },
            { .name = "GTXCL_UPPER", .val = bdmf_address_gtxcl_upper },
            { .name = "GTFRG", .val = bdmf_address_gtfrg },
            { .name = "GTFRG_UPPER", .val = bdmf_address_gtfrg_upper },
            { .name = "GTNCL", .val = bdmf_address_gtncl },
            { .name = "GTNCL_UPPER", .val = bdmf_address_gtncl_upper },
            { .name = "GTJBR", .val = bdmf_address_gtjbr },
            { .name = "GTJBR_UPPER", .val = bdmf_address_gtjbr_upper },
            { .name = "GTBYT", .val = bdmf_address_gtbyt },
            { .name = "GTBYT_UPPER", .val = bdmf_address_gtbyt_upper },
            { .name = "GTPOK", .val = bdmf_address_gtpok },
            { .name = "GTPOK_UPPER", .val = bdmf_address_gtpok_upper },
            { .name = "GTUC", .val = bdmf_address_gtuc },
            { .name = "GTUC_UPPER", .val = bdmf_address_gtuc_upper },
            { .name = "RRPKT", .val = bdmf_address_rrpkt },
            { .name = "RRPKT_UPPER", .val = bdmf_address_rrpkt_upper },
            { .name = "RRUND", .val = bdmf_address_rrund },
            { .name = "RRUND_UPPER", .val = bdmf_address_rrund_upper },
            { .name = "RRFRG", .val = bdmf_address_rrfrg },
            { .name = "RRFRG_UPPER", .val = bdmf_address_rrfrg_upper },
            { .name = "RRBYT", .val = bdmf_address_rrbyt },
            { .name = "RRBYT_UPPER", .val = bdmf_address_rrbyt_upper },
            { .name = "MIB_CNTRL", .val = bdmf_address_mib_cntrl },
            { .name = "MIB_READ_DATA", .val = bdmf_address_mib_read_data },
            { .name = "MIB_WRITE_DATA", .val = bdmf_address_mib_write_data },
            { .name = "CONTROL", .val = bdmf_address_control },
            { .name = "PSW_MS", .val = bdmf_address_psw_ms },
            { .name = "PSW_LS", .val = bdmf_address_psw_ls },
            { .name = "CONTROL_1", .val = bdmf_address_control_1 },
            { .name = "EXTENDED_CONTROL", .val = bdmf_address_extended_control },
            { .name = "TX_IDLE_STUFFING_CONTROL", .val = bdmf_address_tx_idle_stuffing_control },
            { .name = "ACTUAL_DATA_RATE", .val = bdmf_address_actual_data_rate },
            { .name = "GMII_CLOCK_SWALLOWER_CONTROL", .val = bdmf_address_gmii_clock_swallower_control },
            { .name = "XGMII_DATA_RATE_STATUS", .val = bdmf_address_xgmii_data_rate_status },
            { .name = "STATUS", .val = bdmf_address_status },
            { .name = "RX_DISCARD_PACKET_COUNTER", .val = bdmf_address_rx_discard_packet_counter },
            { .name = "TX_DISCARD_PACKET_COUNTER", .val = bdmf_address_tx_discard_packet_counter },
            { .name = "REV", .val = bdmf_address_rev },
            { .name = "UMAC_RXERR_MASK", .val = bdmf_address_umac_rxerr_mask },
            { .name = "MIB_MAX_PKT_SIZE", .val = bdmf_address_mib_max_pkt_size },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "address", "address", bcm_xumac_rdp_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index1", "xumac_id", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

    return dir;
}
#endif /* #ifdef USE_BDMF_SHELL */
