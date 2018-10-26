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
#include "xrdp_drv_unimac_rdp_ag.h"

#define BLOCK_ADDR_COUNT_BITS 3
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_unimac_rdp_ipg_hd_bkp_cntl_set(uint8_t umac_id, uint8_t ipg_config_rx, bdmf_boolean hd_fc_bkoff_ok, bdmf_boolean hd_fc_ena)
{
    uint32_t reg_ipg_hd_bkp_cntl=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT) ||
       (ipg_config_rx >= _5BITS_MAX_VAL_) ||
       (hd_fc_bkoff_ok >= _1BITS_MAX_VAL_) ||
       (hd_fc_ena >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_ipg_hd_bkp_cntl = RU_FIELD_SET(umac_id, UNIMAC_RDP, IPG_HD_BKP_CNTL, IPG_CONFIG_RX, reg_ipg_hd_bkp_cntl, ipg_config_rx);
    reg_ipg_hd_bkp_cntl = RU_FIELD_SET(umac_id, UNIMAC_RDP, IPG_HD_BKP_CNTL, HD_FC_BKOFF_OK, reg_ipg_hd_bkp_cntl, hd_fc_bkoff_ok);
    reg_ipg_hd_bkp_cntl = RU_FIELD_SET(umac_id, UNIMAC_RDP, IPG_HD_BKP_CNTL, HD_FC_ENA, reg_ipg_hd_bkp_cntl, hd_fc_ena);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, IPG_HD_BKP_CNTL, reg_ipg_hd_bkp_cntl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_ipg_hd_bkp_cntl_get(uint8_t umac_id, uint8_t *ipg_config_rx, bdmf_boolean *hd_fc_bkoff_ok, bdmf_boolean *hd_fc_ena)
{
    uint32_t reg_ipg_hd_bkp_cntl;

#ifdef VALIDATE_PARMS
    if(!ipg_config_rx || !hd_fc_bkoff_ok || !hd_fc_ena)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, IPG_HD_BKP_CNTL, reg_ipg_hd_bkp_cntl);

    *ipg_config_rx = RU_FIELD_GET(umac_id, UNIMAC_RDP, IPG_HD_BKP_CNTL, IPG_CONFIG_RX, reg_ipg_hd_bkp_cntl);
    *hd_fc_bkoff_ok = RU_FIELD_GET(umac_id, UNIMAC_RDP, IPG_HD_BKP_CNTL, HD_FC_BKOFF_OK, reg_ipg_hd_bkp_cntl);
    *hd_fc_ena = RU_FIELD_GET(umac_id, UNIMAC_RDP, IPG_HD_BKP_CNTL, HD_FC_ENA, reg_ipg_hd_bkp_cntl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_command_config_set(uint8_t umac_id, const unimac_rdp_command_config *command_config)
{
    uint32_t reg_command_config=0;

#ifdef VALIDATE_PARMS
    if(!command_config)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT) ||
       (command_config->runt_filter_dis >= _1BITS_MAX_VAL_) ||
       (command_config->oob_efc_en >= _1BITS_MAX_VAL_) ||
       (command_config->ignore_tx_pause >= _1BITS_MAX_VAL_) ||
       (command_config->fd_tx_urun_fix_en >= _1BITS_MAX_VAL_) ||
       (command_config->line_loopback >= _1BITS_MAX_VAL_) ||
       (command_config->no_lgth_check >= _1BITS_MAX_VAL_) ||
       (command_config->cntl_frm_ena >= _1BITS_MAX_VAL_) ||
       (command_config->ena_ext_config >= _1BITS_MAX_VAL_) ||
       (command_config->en_internal_tx_crs >= _1BITS_MAX_VAL_) ||
       (command_config->sw_override_rx >= _1BITS_MAX_VAL_) ||
       (command_config->sw_override_tx >= _1BITS_MAX_VAL_) ||
       (command_config->mac_loop_con >= _1BITS_MAX_VAL_) ||
       (command_config->loop_ena >= _1BITS_MAX_VAL_) ||
       (command_config->fcs_corrupt_urun_en >= _1BITS_MAX_VAL_) ||
       (command_config->sw_reset >= _1BITS_MAX_VAL_) ||
       (command_config->overflow_en >= _1BITS_MAX_VAL_) ||
       (command_config->rx_low_latency_en >= _1BITS_MAX_VAL_) ||
       (command_config->hd_ena >= _1BITS_MAX_VAL_) ||
       (command_config->tx_addr_ins >= _1BITS_MAX_VAL_) ||
       (command_config->pause_ignore >= _1BITS_MAX_VAL_) ||
       (command_config->pause_fwd >= _1BITS_MAX_VAL_) ||
       (command_config->crc_fwd >= _1BITS_MAX_VAL_) ||
       (command_config->pad_en >= _1BITS_MAX_VAL_) ||
       (command_config->promis_en >= _1BITS_MAX_VAL_) ||
       (command_config->eth_speed >= _2BITS_MAX_VAL_) ||
       (command_config->rx_ena >= _1BITS_MAX_VAL_) ||
       (command_config->tx_ena >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_command_config = RU_FIELD_SET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, RUNT_FILTER_DIS, reg_command_config, command_config->runt_filter_dis);
    reg_command_config = RU_FIELD_SET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, OOB_EFC_EN, reg_command_config, command_config->oob_efc_en);
    reg_command_config = RU_FIELD_SET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, IGNORE_TX_PAUSE, reg_command_config, command_config->ignore_tx_pause);
    reg_command_config = RU_FIELD_SET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, FD_TX_URUN_FIX_EN, reg_command_config, command_config->fd_tx_urun_fix_en);
    reg_command_config = RU_FIELD_SET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, LINE_LOOPBACK, reg_command_config, command_config->line_loopback);
    reg_command_config = RU_FIELD_SET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, NO_LGTH_CHECK, reg_command_config, command_config->no_lgth_check);
    reg_command_config = RU_FIELD_SET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, CNTL_FRM_ENA, reg_command_config, command_config->cntl_frm_ena);
    reg_command_config = RU_FIELD_SET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, ENA_EXT_CONFIG, reg_command_config, command_config->ena_ext_config);
    reg_command_config = RU_FIELD_SET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, EN_INTERNAL_TX_CRS, reg_command_config, command_config->en_internal_tx_crs);
    reg_command_config = RU_FIELD_SET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, SW_OVERRIDE_RX, reg_command_config, command_config->sw_override_rx);
    reg_command_config = RU_FIELD_SET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, SW_OVERRIDE_TX, reg_command_config, command_config->sw_override_tx);
    reg_command_config = RU_FIELD_SET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, MAC_LOOP_CON, reg_command_config, command_config->mac_loop_con);
    reg_command_config = RU_FIELD_SET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, LOOP_ENA, reg_command_config, command_config->loop_ena);
    reg_command_config = RU_FIELD_SET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, FCS_CORRUPT_URUN_EN, reg_command_config, command_config->fcs_corrupt_urun_en);
    reg_command_config = RU_FIELD_SET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, SW_RESET, reg_command_config, command_config->sw_reset);
    reg_command_config = RU_FIELD_SET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, OVERFLOW_EN, reg_command_config, command_config->overflow_en);
    reg_command_config = RU_FIELD_SET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, RX_LOW_LATENCY_EN, reg_command_config, command_config->rx_low_latency_en);
    reg_command_config = RU_FIELD_SET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, HD_ENA, reg_command_config, command_config->hd_ena);
    reg_command_config = RU_FIELD_SET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, TX_ADDR_INS, reg_command_config, command_config->tx_addr_ins);
    reg_command_config = RU_FIELD_SET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, PAUSE_IGNORE, reg_command_config, command_config->pause_ignore);
    reg_command_config = RU_FIELD_SET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, PAUSE_FWD, reg_command_config, command_config->pause_fwd);
    reg_command_config = RU_FIELD_SET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, CRC_FWD, reg_command_config, command_config->crc_fwd);
    reg_command_config = RU_FIELD_SET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, PAD_EN, reg_command_config, command_config->pad_en);
    reg_command_config = RU_FIELD_SET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, PROMIS_EN, reg_command_config, command_config->promis_en);
    reg_command_config = RU_FIELD_SET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, ETH_SPEED, reg_command_config, command_config->eth_speed);
    reg_command_config = RU_FIELD_SET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, RX_ENA, reg_command_config, command_config->rx_ena);
    reg_command_config = RU_FIELD_SET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, TX_ENA, reg_command_config, command_config->tx_ena);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, COMMAND_CONFIG, reg_command_config);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_command_config_get(uint8_t umac_id, unimac_rdp_command_config *command_config)
{
    uint32_t reg_command_config;

#ifdef VALIDATE_PARMS
    if(!command_config)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, COMMAND_CONFIG, reg_command_config);

    command_config->runt_filter_dis = RU_FIELD_GET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, RUNT_FILTER_DIS, reg_command_config);
    command_config->oob_efc_en = RU_FIELD_GET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, OOB_EFC_EN, reg_command_config);
    command_config->ignore_tx_pause = RU_FIELD_GET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, IGNORE_TX_PAUSE, reg_command_config);
    command_config->fd_tx_urun_fix_en = RU_FIELD_GET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, FD_TX_URUN_FIX_EN, reg_command_config);
    command_config->line_loopback = RU_FIELD_GET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, LINE_LOOPBACK, reg_command_config);
    command_config->no_lgth_check = RU_FIELD_GET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, NO_LGTH_CHECK, reg_command_config);
    command_config->cntl_frm_ena = RU_FIELD_GET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, CNTL_FRM_ENA, reg_command_config);
    command_config->ena_ext_config = RU_FIELD_GET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, ENA_EXT_CONFIG, reg_command_config);
    command_config->en_internal_tx_crs = RU_FIELD_GET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, EN_INTERNAL_TX_CRS, reg_command_config);
    command_config->sw_override_rx = RU_FIELD_GET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, SW_OVERRIDE_RX, reg_command_config);
    command_config->sw_override_tx = RU_FIELD_GET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, SW_OVERRIDE_TX, reg_command_config);
    command_config->mac_loop_con = RU_FIELD_GET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, MAC_LOOP_CON, reg_command_config);
    command_config->loop_ena = RU_FIELD_GET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, LOOP_ENA, reg_command_config);
    command_config->fcs_corrupt_urun_en = RU_FIELD_GET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, FCS_CORRUPT_URUN_EN, reg_command_config);
    command_config->sw_reset = RU_FIELD_GET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, SW_RESET, reg_command_config);
    command_config->overflow_en = RU_FIELD_GET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, OVERFLOW_EN, reg_command_config);
    command_config->rx_low_latency_en = RU_FIELD_GET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, RX_LOW_LATENCY_EN, reg_command_config);
    command_config->hd_ena = RU_FIELD_GET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, HD_ENA, reg_command_config);
    command_config->tx_addr_ins = RU_FIELD_GET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, TX_ADDR_INS, reg_command_config);
    command_config->pause_ignore = RU_FIELD_GET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, PAUSE_IGNORE, reg_command_config);
    command_config->pause_fwd = RU_FIELD_GET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, PAUSE_FWD, reg_command_config);
    command_config->crc_fwd = RU_FIELD_GET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, CRC_FWD, reg_command_config);
    command_config->pad_en = RU_FIELD_GET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, PAD_EN, reg_command_config);
    command_config->promis_en = RU_FIELD_GET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, PROMIS_EN, reg_command_config);
    command_config->eth_speed = RU_FIELD_GET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, ETH_SPEED, reg_command_config);
    command_config->rx_ena = RU_FIELD_GET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, RX_ENA, reg_command_config);
    command_config->tx_ena = RU_FIELD_GET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, TX_ENA, reg_command_config);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_mac_0_set(uint8_t umac_id, uint32_t mac_addr0)
{
    uint32_t reg_mac_0=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_mac_0 = RU_FIELD_SET(umac_id, UNIMAC_RDP, MAC_0, MAC_ADDR0, reg_mac_0, mac_addr0);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, MAC_0, reg_mac_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_mac_0_get(uint8_t umac_id, uint32_t *mac_addr0)
{
    uint32_t reg_mac_0;

#ifdef VALIDATE_PARMS
    if(!mac_addr0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, MAC_0, reg_mac_0);

    *mac_addr0 = RU_FIELD_GET(umac_id, UNIMAC_RDP, MAC_0, MAC_ADDR0, reg_mac_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_mac_1_set(uint8_t umac_id, uint16_t mac_addr1)
{
    uint32_t reg_mac_1=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_mac_1 = RU_FIELD_SET(umac_id, UNIMAC_RDP, MAC_1, MAC_ADDR1, reg_mac_1, mac_addr1);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, MAC_1, reg_mac_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_mac_1_get(uint8_t umac_id, uint16_t *mac_addr1)
{
    uint32_t reg_mac_1;

#ifdef VALIDATE_PARMS
    if(!mac_addr1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, MAC_1, reg_mac_1);

    *mac_addr1 = RU_FIELD_GET(umac_id, UNIMAC_RDP, MAC_1, MAC_ADDR1, reg_mac_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_frm_length_set(uint8_t umac_id, uint16_t maxfr)
{
    uint32_t reg_frm_length=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT) ||
       (maxfr >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_frm_length = RU_FIELD_SET(umac_id, UNIMAC_RDP, FRM_LENGTH, MAXFR, reg_frm_length, maxfr);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, FRM_LENGTH, reg_frm_length);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_frm_length_get(uint8_t umac_id, uint16_t *maxfr)
{
    uint32_t reg_frm_length;

#ifdef VALIDATE_PARMS
    if(!maxfr)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, FRM_LENGTH, reg_frm_length);

    *maxfr = RU_FIELD_GET(umac_id, UNIMAC_RDP, FRM_LENGTH, MAXFR, reg_frm_length);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_pause_quant_set(uint8_t umac_id, uint16_t pause_quant)
{
    uint32_t reg_pause_quant=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pause_quant = RU_FIELD_SET(umac_id, UNIMAC_RDP, PAUSE_QUANT, PAUSE_QUANT, reg_pause_quant, pause_quant);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, PAUSE_QUANT, reg_pause_quant);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_pause_quant_get(uint8_t umac_id, uint16_t *pause_quant)
{
    uint32_t reg_pause_quant;

#ifdef VALIDATE_PARMS
    if(!pause_quant)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, PAUSE_QUANT, reg_pause_quant);

    *pause_quant = RU_FIELD_GET(umac_id, UNIMAC_RDP, PAUSE_QUANT, PAUSE_QUANT, reg_pause_quant);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_tx_ts_seq_id_get(uint8_t umac_id, bdmf_boolean *tsts_valid, uint16_t *tsts_seq_id)
{
    uint32_t reg_tx_ts_seq_id;

#ifdef VALIDATE_PARMS
    if(!tsts_valid || !tsts_seq_id)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, TX_TS_SEQ_ID, reg_tx_ts_seq_id);

    *tsts_valid = RU_FIELD_GET(umac_id, UNIMAC_RDP, TX_TS_SEQ_ID, TSTS_VALID, reg_tx_ts_seq_id);
    *tsts_seq_id = RU_FIELD_GET(umac_id, UNIMAC_RDP, TX_TS_SEQ_ID, TSTS_SEQ_ID, reg_tx_ts_seq_id);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_sfd_offset_set(uint8_t umac_id, uint8_t sfd_offset)
{
    uint32_t reg_sfd_offset=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT) ||
       (sfd_offset >= _4BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_sfd_offset = RU_FIELD_SET(umac_id, UNIMAC_RDP, SFD_OFFSET, SFD_OFFSET, reg_sfd_offset, sfd_offset);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, SFD_OFFSET, reg_sfd_offset);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_sfd_offset_get(uint8_t umac_id, uint8_t *sfd_offset)
{
    uint32_t reg_sfd_offset;

#ifdef VALIDATE_PARMS
    if(!sfd_offset)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, SFD_OFFSET, reg_sfd_offset);

    *sfd_offset = RU_FIELD_GET(umac_id, UNIMAC_RDP, SFD_OFFSET, SFD_OFFSET, reg_sfd_offset);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_mac_mode_get(uint8_t umac_id, unimac_rdp_mac_mode *mac_mode)
{
    uint32_t reg_mac_mode;

#ifdef VALIDATE_PARMS
    if(!mac_mode)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, MAC_MODE, reg_mac_mode);

    mac_mode->link_status = RU_FIELD_GET(umac_id, UNIMAC_RDP, MAC_MODE, LINK_STATUS, reg_mac_mode);
    mac_mode->mac_tx_pause = RU_FIELD_GET(umac_id, UNIMAC_RDP, MAC_MODE, MAC_TX_PAUSE, reg_mac_mode);
    mac_mode->mac_rx_pause = RU_FIELD_GET(umac_id, UNIMAC_RDP, MAC_MODE, MAC_RX_PAUSE, reg_mac_mode);
    mac_mode->mac_duplex = RU_FIELD_GET(umac_id, UNIMAC_RDP, MAC_MODE, MAC_DUPLEX, reg_mac_mode);
    mac_mode->mac_speed = RU_FIELD_GET(umac_id, UNIMAC_RDP, MAC_MODE, MAC_SPEED, reg_mac_mode);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_tag_0_set(uint8_t umac_id, bdmf_boolean config_outer_tpid_enable, uint16_t frm_tag_0)
{
    uint32_t reg_tag_0=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT) ||
       (config_outer_tpid_enable >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tag_0 = RU_FIELD_SET(umac_id, UNIMAC_RDP, TAG_0, CONFIG_OUTER_TPID_ENABLE, reg_tag_0, config_outer_tpid_enable);
    reg_tag_0 = RU_FIELD_SET(umac_id, UNIMAC_RDP, TAG_0, FRM_TAG_0, reg_tag_0, frm_tag_0);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, TAG_0, reg_tag_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_tag_0_get(uint8_t umac_id, bdmf_boolean *config_outer_tpid_enable, uint16_t *frm_tag_0)
{
    uint32_t reg_tag_0;

#ifdef VALIDATE_PARMS
    if(!config_outer_tpid_enable || !frm_tag_0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, TAG_0, reg_tag_0);

    *config_outer_tpid_enable = RU_FIELD_GET(umac_id, UNIMAC_RDP, TAG_0, CONFIG_OUTER_TPID_ENABLE, reg_tag_0);
    *frm_tag_0 = RU_FIELD_GET(umac_id, UNIMAC_RDP, TAG_0, FRM_TAG_0, reg_tag_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_tag_1_set(uint8_t umac_id, bdmf_boolean config_inner_tpid_enable, uint16_t frm_tag_1)
{
    uint32_t reg_tag_1=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT) ||
       (config_inner_tpid_enable >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tag_1 = RU_FIELD_SET(umac_id, UNIMAC_RDP, TAG_1, CONFIG_INNER_TPID_ENABLE, reg_tag_1, config_inner_tpid_enable);
    reg_tag_1 = RU_FIELD_SET(umac_id, UNIMAC_RDP, TAG_1, FRM_TAG_1, reg_tag_1, frm_tag_1);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, TAG_1, reg_tag_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_tag_1_get(uint8_t umac_id, bdmf_boolean *config_inner_tpid_enable, uint16_t *frm_tag_1)
{
    uint32_t reg_tag_1;

#ifdef VALIDATE_PARMS
    if(!config_inner_tpid_enable || !frm_tag_1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, TAG_1, reg_tag_1);

    *config_inner_tpid_enable = RU_FIELD_GET(umac_id, UNIMAC_RDP, TAG_1, CONFIG_INNER_TPID_ENABLE, reg_tag_1);
    *frm_tag_1 = RU_FIELD_GET(umac_id, UNIMAC_RDP, TAG_1, FRM_TAG_1, reg_tag_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_rx_pause_quanta_scale_set(uint8_t umac_id, bdmf_boolean scale_fix, bdmf_boolean scale_control, uint16_t scale_value)
{
    uint32_t reg_rx_pause_quanta_scale=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT) ||
       (scale_fix >= _1BITS_MAX_VAL_) ||
       (scale_control >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_pause_quanta_scale = RU_FIELD_SET(umac_id, UNIMAC_RDP, RX_PAUSE_QUANTA_SCALE, SCALE_FIX, reg_rx_pause_quanta_scale, scale_fix);
    reg_rx_pause_quanta_scale = RU_FIELD_SET(umac_id, UNIMAC_RDP, RX_PAUSE_QUANTA_SCALE, SCALE_CONTROL, reg_rx_pause_quanta_scale, scale_control);
    reg_rx_pause_quanta_scale = RU_FIELD_SET(umac_id, UNIMAC_RDP, RX_PAUSE_QUANTA_SCALE, SCALE_VALUE, reg_rx_pause_quanta_scale, scale_value);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, RX_PAUSE_QUANTA_SCALE, reg_rx_pause_quanta_scale);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_rx_pause_quanta_scale_get(uint8_t umac_id, bdmf_boolean *scale_fix, bdmf_boolean *scale_control, uint16_t *scale_value)
{
    uint32_t reg_rx_pause_quanta_scale;

#ifdef VALIDATE_PARMS
    if(!scale_fix || !scale_control || !scale_value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, RX_PAUSE_QUANTA_SCALE, reg_rx_pause_quanta_scale);

    *scale_fix = RU_FIELD_GET(umac_id, UNIMAC_RDP, RX_PAUSE_QUANTA_SCALE, SCALE_FIX, reg_rx_pause_quanta_scale);
    *scale_control = RU_FIELD_GET(umac_id, UNIMAC_RDP, RX_PAUSE_QUANTA_SCALE, SCALE_CONTROL, reg_rx_pause_quanta_scale);
    *scale_value = RU_FIELD_GET(umac_id, UNIMAC_RDP, RX_PAUSE_QUANTA_SCALE, SCALE_VALUE, reg_rx_pause_quanta_scale);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_tx_preamble_set(uint8_t umac_id, uint8_t tx_preamble)
{
    uint32_t reg_tx_preamble=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT) ||
       (tx_preamble >= _3BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tx_preamble = RU_FIELD_SET(umac_id, UNIMAC_RDP, TX_PREAMBLE, TX_PREAMBLE, reg_tx_preamble, tx_preamble);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, TX_PREAMBLE, reg_tx_preamble);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_tx_preamble_get(uint8_t umac_id, uint8_t *tx_preamble)
{
    uint32_t reg_tx_preamble;

#ifdef VALIDATE_PARMS
    if(!tx_preamble)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, TX_PREAMBLE, reg_tx_preamble);

    *tx_preamble = RU_FIELD_GET(umac_id, UNIMAC_RDP, TX_PREAMBLE, TX_PREAMBLE, reg_tx_preamble);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_tx_ipg_length_set(uint8_t umac_id, uint8_t tx_min_pkt_size, uint8_t tx_ipg_length)
{
    uint32_t reg_tx_ipg_length=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT) ||
       (tx_min_pkt_size >= _7BITS_MAX_VAL_) ||
       (tx_ipg_length >= _7BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tx_ipg_length = RU_FIELD_SET(umac_id, UNIMAC_RDP, TX_IPG_LENGTH, TX_MIN_PKT_SIZE, reg_tx_ipg_length, tx_min_pkt_size);
    reg_tx_ipg_length = RU_FIELD_SET(umac_id, UNIMAC_RDP, TX_IPG_LENGTH, TX_IPG_LENGTH, reg_tx_ipg_length, tx_ipg_length);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, TX_IPG_LENGTH, reg_tx_ipg_length);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_tx_ipg_length_get(uint8_t umac_id, uint8_t *tx_min_pkt_size, uint8_t *tx_ipg_length)
{
    uint32_t reg_tx_ipg_length;

#ifdef VALIDATE_PARMS
    if(!tx_min_pkt_size || !tx_ipg_length)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, TX_IPG_LENGTH, reg_tx_ipg_length);

    *tx_min_pkt_size = RU_FIELD_GET(umac_id, UNIMAC_RDP, TX_IPG_LENGTH, TX_MIN_PKT_SIZE, reg_tx_ipg_length);
    *tx_ipg_length = RU_FIELD_GET(umac_id, UNIMAC_RDP, TX_IPG_LENGTH, TX_IPG_LENGTH, reg_tx_ipg_length);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_pfc_xoff_timer_set(uint8_t umac_id, uint16_t pfc_xoff_timer)
{
    uint32_t reg_pfc_xoff_timer=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pfc_xoff_timer = RU_FIELD_SET(umac_id, UNIMAC_RDP, PFC_XOFF_TIMER, PFC_XOFF_TIMER, reg_pfc_xoff_timer, pfc_xoff_timer);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, PFC_XOFF_TIMER, reg_pfc_xoff_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_pfc_xoff_timer_get(uint8_t umac_id, uint16_t *pfc_xoff_timer)
{
    uint32_t reg_pfc_xoff_timer;

#ifdef VALIDATE_PARMS
    if(!pfc_xoff_timer)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, PFC_XOFF_TIMER, reg_pfc_xoff_timer);

    *pfc_xoff_timer = RU_FIELD_GET(umac_id, UNIMAC_RDP, PFC_XOFF_TIMER, PFC_XOFF_TIMER, reg_pfc_xoff_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_umac_eee_ctrl_set(uint8_t umac_id, const unimac_rdp_umac_eee_ctrl *umac_eee_ctrl)
{
    uint32_t reg_umac_eee_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!umac_eee_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT) ||
       (umac_eee_ctrl->lp_idle_prediction_mode >= _1BITS_MAX_VAL_) ||
       (umac_eee_ctrl->dis_eee_10m >= _1BITS_MAX_VAL_) ||
       (umac_eee_ctrl->eee_txclk_dis >= _1BITS_MAX_VAL_) ||
       (umac_eee_ctrl->rx_fifo_check >= _1BITS_MAX_VAL_) ||
       (umac_eee_ctrl->eee_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_umac_eee_ctrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, UMAC_EEE_CTRL, LP_IDLE_PREDICTION_MODE, reg_umac_eee_ctrl, umac_eee_ctrl->lp_idle_prediction_mode);
    reg_umac_eee_ctrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, UMAC_EEE_CTRL, DIS_EEE_10M, reg_umac_eee_ctrl, umac_eee_ctrl->dis_eee_10m);
    reg_umac_eee_ctrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, UMAC_EEE_CTRL, EEE_TXCLK_DIS, reg_umac_eee_ctrl, umac_eee_ctrl->eee_txclk_dis);
    reg_umac_eee_ctrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, UMAC_EEE_CTRL, RX_FIFO_CHECK, reg_umac_eee_ctrl, umac_eee_ctrl->rx_fifo_check);
    reg_umac_eee_ctrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, UMAC_EEE_CTRL, EEE_EN, reg_umac_eee_ctrl, umac_eee_ctrl->eee_en);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, UMAC_EEE_CTRL, reg_umac_eee_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_umac_eee_ctrl_get(uint8_t umac_id, unimac_rdp_umac_eee_ctrl *umac_eee_ctrl)
{
    uint32_t reg_umac_eee_ctrl;

#ifdef VALIDATE_PARMS
    if(!umac_eee_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, UMAC_EEE_CTRL, reg_umac_eee_ctrl);

    umac_eee_ctrl->lp_idle_prediction_mode = RU_FIELD_GET(umac_id, UNIMAC_RDP, UMAC_EEE_CTRL, LP_IDLE_PREDICTION_MODE, reg_umac_eee_ctrl);
    umac_eee_ctrl->dis_eee_10m = RU_FIELD_GET(umac_id, UNIMAC_RDP, UMAC_EEE_CTRL, DIS_EEE_10M, reg_umac_eee_ctrl);
    umac_eee_ctrl->eee_txclk_dis = RU_FIELD_GET(umac_id, UNIMAC_RDP, UMAC_EEE_CTRL, EEE_TXCLK_DIS, reg_umac_eee_ctrl);
    umac_eee_ctrl->rx_fifo_check = RU_FIELD_GET(umac_id, UNIMAC_RDP, UMAC_EEE_CTRL, RX_FIFO_CHECK, reg_umac_eee_ctrl);
    umac_eee_ctrl->eee_en = RU_FIELD_GET(umac_id, UNIMAC_RDP, UMAC_EEE_CTRL, EEE_EN, reg_umac_eee_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_mii_eee_delay_entry_timer_set(uint8_t umac_id, uint32_t mii_eee_lpi_timer)
{
    uint32_t reg_mii_eee_delay_entry_timer=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_mii_eee_delay_entry_timer = RU_FIELD_SET(umac_id, UNIMAC_RDP, MII_EEE_DELAY_ENTRY_TIMER, MII_EEE_LPI_TIMER, reg_mii_eee_delay_entry_timer, mii_eee_lpi_timer);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, MII_EEE_DELAY_ENTRY_TIMER, reg_mii_eee_delay_entry_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_mii_eee_delay_entry_timer_get(uint8_t umac_id, uint32_t *mii_eee_lpi_timer)
{
    uint32_t reg_mii_eee_delay_entry_timer;

#ifdef VALIDATE_PARMS
    if(!mii_eee_lpi_timer)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, MII_EEE_DELAY_ENTRY_TIMER, reg_mii_eee_delay_entry_timer);

    *mii_eee_lpi_timer = RU_FIELD_GET(umac_id, UNIMAC_RDP, MII_EEE_DELAY_ENTRY_TIMER, MII_EEE_LPI_TIMER, reg_mii_eee_delay_entry_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_gmii_eee_delay_entry_timer_set(uint8_t umac_id, uint32_t gmii_eee_lpi_timer)
{
    uint32_t reg_gmii_eee_delay_entry_timer=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gmii_eee_delay_entry_timer = RU_FIELD_SET(umac_id, UNIMAC_RDP, GMII_EEE_DELAY_ENTRY_TIMER, GMII_EEE_LPI_TIMER, reg_gmii_eee_delay_entry_timer, gmii_eee_lpi_timer);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, GMII_EEE_DELAY_ENTRY_TIMER, reg_gmii_eee_delay_entry_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_gmii_eee_delay_entry_timer_get(uint8_t umac_id, uint32_t *gmii_eee_lpi_timer)
{
    uint32_t reg_gmii_eee_delay_entry_timer;

#ifdef VALIDATE_PARMS
    if(!gmii_eee_lpi_timer)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, GMII_EEE_DELAY_ENTRY_TIMER, reg_gmii_eee_delay_entry_timer);

    *gmii_eee_lpi_timer = RU_FIELD_GET(umac_id, UNIMAC_RDP, GMII_EEE_DELAY_ENTRY_TIMER, GMII_EEE_LPI_TIMER, reg_gmii_eee_delay_entry_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_umac_eee_ref_count_set(uint8_t umac_id, uint16_t eee_ref_count)
{
    uint32_t reg_umac_eee_ref_count=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_umac_eee_ref_count = RU_FIELD_SET(umac_id, UNIMAC_RDP, UMAC_EEE_REF_COUNT, EEE_REF_COUNT, reg_umac_eee_ref_count, eee_ref_count);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, UMAC_EEE_REF_COUNT, reg_umac_eee_ref_count);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_umac_eee_ref_count_get(uint8_t umac_id, uint16_t *eee_ref_count)
{
    uint32_t reg_umac_eee_ref_count;

#ifdef VALIDATE_PARMS
    if(!eee_ref_count)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, UMAC_EEE_REF_COUNT, reg_umac_eee_ref_count);

    *eee_ref_count = RU_FIELD_GET(umac_id, UNIMAC_RDP, UMAC_EEE_REF_COUNT, EEE_REF_COUNT, reg_umac_eee_ref_count);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_umac_timestamp_adjust_set(uint8_t umac_id, bdmf_boolean auto_adjust, bdmf_boolean en_1588, uint16_t adjust)
{
    uint32_t reg_umac_timestamp_adjust=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT) ||
       (auto_adjust >= _1BITS_MAX_VAL_) ||
       (en_1588 >= _1BITS_MAX_VAL_) ||
       (adjust >= _9BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_umac_timestamp_adjust = RU_FIELD_SET(umac_id, UNIMAC_RDP, UMAC_TIMESTAMP_ADJUST, AUTO_ADJUST, reg_umac_timestamp_adjust, auto_adjust);
    reg_umac_timestamp_adjust = RU_FIELD_SET(umac_id, UNIMAC_RDP, UMAC_TIMESTAMP_ADJUST, EN_1588, reg_umac_timestamp_adjust, en_1588);
    reg_umac_timestamp_adjust = RU_FIELD_SET(umac_id, UNIMAC_RDP, UMAC_TIMESTAMP_ADJUST, ADJUST, reg_umac_timestamp_adjust, adjust);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, UMAC_TIMESTAMP_ADJUST, reg_umac_timestamp_adjust);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_umac_timestamp_adjust_get(uint8_t umac_id, bdmf_boolean *auto_adjust, bdmf_boolean *en_1588, uint16_t *adjust)
{
    uint32_t reg_umac_timestamp_adjust;

#ifdef VALIDATE_PARMS
    if(!auto_adjust || !en_1588 || !adjust)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, UMAC_TIMESTAMP_ADJUST, reg_umac_timestamp_adjust);

    *auto_adjust = RU_FIELD_GET(umac_id, UNIMAC_RDP, UMAC_TIMESTAMP_ADJUST, AUTO_ADJUST, reg_umac_timestamp_adjust);
    *en_1588 = RU_FIELD_GET(umac_id, UNIMAC_RDP, UMAC_TIMESTAMP_ADJUST, EN_1588, reg_umac_timestamp_adjust);
    *adjust = RU_FIELD_GET(umac_id, UNIMAC_RDP, UMAC_TIMESTAMP_ADJUST, ADJUST, reg_umac_timestamp_adjust);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_umac_rx_pkt_drop_status_set(uint8_t umac_id, bdmf_boolean rx_ipg_inval)
{
    uint32_t reg_umac_rx_pkt_drop_status=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT) ||
       (rx_ipg_inval >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_umac_rx_pkt_drop_status = RU_FIELD_SET(umac_id, UNIMAC_RDP, UMAC_RX_PKT_DROP_STATUS, RX_IPG_INVAL, reg_umac_rx_pkt_drop_status, rx_ipg_inval);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, UMAC_RX_PKT_DROP_STATUS, reg_umac_rx_pkt_drop_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_umac_rx_pkt_drop_status_get(uint8_t umac_id, bdmf_boolean *rx_ipg_inval)
{
    uint32_t reg_umac_rx_pkt_drop_status;

#ifdef VALIDATE_PARMS
    if(!rx_ipg_inval)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, UMAC_RX_PKT_DROP_STATUS, reg_umac_rx_pkt_drop_status);

    *rx_ipg_inval = RU_FIELD_GET(umac_id, UNIMAC_RDP, UMAC_RX_PKT_DROP_STATUS, RX_IPG_INVAL, reg_umac_rx_pkt_drop_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_umac_symmetric_idle_threshold_set(uint8_t umac_id, uint16_t threshold_value)
{
    uint32_t reg_umac_symmetric_idle_threshold=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_umac_symmetric_idle_threshold = RU_FIELD_SET(umac_id, UNIMAC_RDP, UMAC_SYMMETRIC_IDLE_THRESHOLD, THRESHOLD_VALUE, reg_umac_symmetric_idle_threshold, threshold_value);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, UMAC_SYMMETRIC_IDLE_THRESHOLD, reg_umac_symmetric_idle_threshold);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_umac_symmetric_idle_threshold_get(uint8_t umac_id, uint16_t *threshold_value)
{
    uint32_t reg_umac_symmetric_idle_threshold;

#ifdef VALIDATE_PARMS
    if(!threshold_value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, UMAC_SYMMETRIC_IDLE_THRESHOLD, reg_umac_symmetric_idle_threshold);

    *threshold_value = RU_FIELD_GET(umac_id, UNIMAC_RDP, UMAC_SYMMETRIC_IDLE_THRESHOLD, THRESHOLD_VALUE, reg_umac_symmetric_idle_threshold);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_mii_eee_wake_timer_set(uint8_t umac_id, uint16_t mii_eee_wake_timer)
{
    uint32_t reg_mii_eee_wake_timer=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_mii_eee_wake_timer = RU_FIELD_SET(umac_id, UNIMAC_RDP, MII_EEE_WAKE_TIMER, MII_EEE_WAKE_TIMER, reg_mii_eee_wake_timer, mii_eee_wake_timer);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, MII_EEE_WAKE_TIMER, reg_mii_eee_wake_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_mii_eee_wake_timer_get(uint8_t umac_id, uint16_t *mii_eee_wake_timer)
{
    uint32_t reg_mii_eee_wake_timer;

#ifdef VALIDATE_PARMS
    if(!mii_eee_wake_timer)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, MII_EEE_WAKE_TIMER, reg_mii_eee_wake_timer);

    *mii_eee_wake_timer = RU_FIELD_GET(umac_id, UNIMAC_RDP, MII_EEE_WAKE_TIMER, MII_EEE_WAKE_TIMER, reg_mii_eee_wake_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_gmii_eee_wake_timer_set(uint8_t umac_id, uint16_t gmii_eee_wake_timer)
{
    uint32_t reg_gmii_eee_wake_timer=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_gmii_eee_wake_timer = RU_FIELD_SET(umac_id, UNIMAC_RDP, GMII_EEE_WAKE_TIMER, GMII_EEE_WAKE_TIMER, reg_gmii_eee_wake_timer, gmii_eee_wake_timer);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, GMII_EEE_WAKE_TIMER, reg_gmii_eee_wake_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_gmii_eee_wake_timer_get(uint8_t umac_id, uint16_t *gmii_eee_wake_timer)
{
    uint32_t reg_gmii_eee_wake_timer;

#ifdef VALIDATE_PARMS
    if(!gmii_eee_wake_timer)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, GMII_EEE_WAKE_TIMER, reg_gmii_eee_wake_timer);

    *gmii_eee_wake_timer = RU_FIELD_GET(umac_id, UNIMAC_RDP, GMII_EEE_WAKE_TIMER, GMII_EEE_WAKE_TIMER, reg_gmii_eee_wake_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_umac_rev_id_get(uint8_t umac_id, uint8_t *revision_id_major, uint8_t *revision_id_minor, uint8_t *patch)
{
    uint32_t reg_umac_rev_id;

#ifdef VALIDATE_PARMS
    if(!revision_id_major || !revision_id_minor || !patch)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, UMAC_REV_ID, reg_umac_rev_id);

    *revision_id_major = RU_FIELD_GET(umac_id, UNIMAC_RDP, UMAC_REV_ID, REVISION_ID_MAJOR, reg_umac_rev_id);
    *revision_id_minor = RU_FIELD_GET(umac_id, UNIMAC_RDP, UMAC_REV_ID, REVISION_ID_MINOR, reg_umac_rev_id);
    *patch = RU_FIELD_GET(umac_id, UNIMAC_RDP, UMAC_REV_ID, PATCH, reg_umac_rev_id);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_mac_pfc_type_set(uint8_t umac_id, uint16_t pfc_eth_type)
{
    uint32_t reg_mac_pfc_type=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_mac_pfc_type = RU_FIELD_SET(umac_id, UNIMAC_RDP, MAC_PFC_TYPE, PFC_ETH_TYPE, reg_mac_pfc_type, pfc_eth_type);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, MAC_PFC_TYPE, reg_mac_pfc_type);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_mac_pfc_type_get(uint8_t umac_id, uint16_t *pfc_eth_type)
{
    uint32_t reg_mac_pfc_type;

#ifdef VALIDATE_PARMS
    if(!pfc_eth_type)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, MAC_PFC_TYPE, reg_mac_pfc_type);

    *pfc_eth_type = RU_FIELD_GET(umac_id, UNIMAC_RDP, MAC_PFC_TYPE, PFC_ETH_TYPE, reg_mac_pfc_type);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_mac_pfc_opcode_set(uint8_t umac_id, uint16_t pfc_opcode)
{
    uint32_t reg_mac_pfc_opcode=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_mac_pfc_opcode = RU_FIELD_SET(umac_id, UNIMAC_RDP, MAC_PFC_OPCODE, PFC_OPCODE, reg_mac_pfc_opcode, pfc_opcode);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, MAC_PFC_OPCODE, reg_mac_pfc_opcode);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_mac_pfc_opcode_get(uint8_t umac_id, uint16_t *pfc_opcode)
{
    uint32_t reg_mac_pfc_opcode;

#ifdef VALIDATE_PARMS
    if(!pfc_opcode)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, MAC_PFC_OPCODE, reg_mac_pfc_opcode);

    *pfc_opcode = RU_FIELD_GET(umac_id, UNIMAC_RDP, MAC_PFC_OPCODE, PFC_OPCODE, reg_mac_pfc_opcode);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_mac_pfc_da_0_set(uint8_t umac_id, uint32_t pfc_macda_0)
{
    uint32_t reg_mac_pfc_da_0=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_mac_pfc_da_0 = RU_FIELD_SET(umac_id, UNIMAC_RDP, MAC_PFC_DA_0, PFC_MACDA_0, reg_mac_pfc_da_0, pfc_macda_0);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, MAC_PFC_DA_0, reg_mac_pfc_da_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_mac_pfc_da_0_get(uint8_t umac_id, uint32_t *pfc_macda_0)
{
    uint32_t reg_mac_pfc_da_0;

#ifdef VALIDATE_PARMS
    if(!pfc_macda_0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, MAC_PFC_DA_0, reg_mac_pfc_da_0);

    *pfc_macda_0 = RU_FIELD_GET(umac_id, UNIMAC_RDP, MAC_PFC_DA_0, PFC_MACDA_0, reg_mac_pfc_da_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_mac_pfc_da_1_set(uint8_t umac_id, uint16_t pfc_macda_1)
{
    uint32_t reg_mac_pfc_da_1=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_mac_pfc_da_1 = RU_FIELD_SET(umac_id, UNIMAC_RDP, MAC_PFC_DA_1, PFC_MACDA_1, reg_mac_pfc_da_1, pfc_macda_1);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, MAC_PFC_DA_1, reg_mac_pfc_da_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_mac_pfc_da_1_get(uint8_t umac_id, uint16_t *pfc_macda_1)
{
    uint32_t reg_mac_pfc_da_1;

#ifdef VALIDATE_PARMS
    if(!pfc_macda_1)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, MAC_PFC_DA_1, reg_mac_pfc_da_1);

    *pfc_macda_1 = RU_FIELD_GET(umac_id, UNIMAC_RDP, MAC_PFC_DA_1, PFC_MACDA_1, reg_mac_pfc_da_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_macsec_prog_tx_crc_set(uint8_t umac_id, uint32_t macsec_prog_tx_crc)
{
    uint32_t reg_macsec_prog_tx_crc=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_macsec_prog_tx_crc = RU_FIELD_SET(umac_id, UNIMAC_RDP, MACSEC_PROG_TX_CRC, MACSEC_PROG_TX_CRC, reg_macsec_prog_tx_crc, macsec_prog_tx_crc);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, MACSEC_PROG_TX_CRC, reg_macsec_prog_tx_crc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_macsec_prog_tx_crc_get(uint8_t umac_id, uint32_t *macsec_prog_tx_crc)
{
    uint32_t reg_macsec_prog_tx_crc;

#ifdef VALIDATE_PARMS
    if(!macsec_prog_tx_crc)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, MACSEC_PROG_TX_CRC, reg_macsec_prog_tx_crc);

    *macsec_prog_tx_crc = RU_FIELD_GET(umac_id, UNIMAC_RDP, MACSEC_PROG_TX_CRC, MACSEC_PROG_TX_CRC, reg_macsec_prog_tx_crc);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_macsec_cntrl_set(uint8_t umac_id, bdmf_boolean dis_pause_data_var_ipg, bdmf_boolean tx_crc_program, bdmf_boolean tx_crc_corupt_en, bdmf_boolean tx_launch_en)
{
    uint32_t reg_macsec_cntrl=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT) ||
       (dis_pause_data_var_ipg >= _1BITS_MAX_VAL_) ||
       (tx_crc_program >= _1BITS_MAX_VAL_) ||
       (tx_crc_corupt_en >= _1BITS_MAX_VAL_) ||
       (tx_launch_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_macsec_cntrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, MACSEC_CNTRL, DIS_PAUSE_DATA_VAR_IPG, reg_macsec_cntrl, dis_pause_data_var_ipg);
    reg_macsec_cntrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, MACSEC_CNTRL, TX_CRC_PROGRAM, reg_macsec_cntrl, tx_crc_program);
    reg_macsec_cntrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, MACSEC_CNTRL, TX_CRC_CORUPT_EN, reg_macsec_cntrl, tx_crc_corupt_en);
    reg_macsec_cntrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, MACSEC_CNTRL, TX_LAUNCH_EN, reg_macsec_cntrl, tx_launch_en);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, MACSEC_CNTRL, reg_macsec_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_macsec_cntrl_get(uint8_t umac_id, bdmf_boolean *dis_pause_data_var_ipg, bdmf_boolean *tx_crc_program, bdmf_boolean *tx_crc_corupt_en, bdmf_boolean *tx_launch_en)
{
    uint32_t reg_macsec_cntrl;

#ifdef VALIDATE_PARMS
    if(!dis_pause_data_var_ipg || !tx_crc_program || !tx_crc_corupt_en || !tx_launch_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, MACSEC_CNTRL, reg_macsec_cntrl);

    *dis_pause_data_var_ipg = RU_FIELD_GET(umac_id, UNIMAC_RDP, MACSEC_CNTRL, DIS_PAUSE_DATA_VAR_IPG, reg_macsec_cntrl);
    *tx_crc_program = RU_FIELD_GET(umac_id, UNIMAC_RDP, MACSEC_CNTRL, TX_CRC_PROGRAM, reg_macsec_cntrl);
    *tx_crc_corupt_en = RU_FIELD_GET(umac_id, UNIMAC_RDP, MACSEC_CNTRL, TX_CRC_CORUPT_EN, reg_macsec_cntrl);
    *tx_launch_en = RU_FIELD_GET(umac_id, UNIMAC_RDP, MACSEC_CNTRL, TX_LAUNCH_EN, reg_macsec_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_ts_status_get(uint8_t umac_id, uint8_t *word_avail, bdmf_boolean *tx_ts_fifo_empty, bdmf_boolean *tx_ts_fifo_full)
{
    uint32_t reg_ts_status;

#ifdef VALIDATE_PARMS
    if(!word_avail || !tx_ts_fifo_empty || !tx_ts_fifo_full)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, TS_STATUS, reg_ts_status);

    *word_avail = RU_FIELD_GET(umac_id, UNIMAC_RDP, TS_STATUS, WORD_AVAIL, reg_ts_status);
    *tx_ts_fifo_empty = RU_FIELD_GET(umac_id, UNIMAC_RDP, TS_STATUS, TX_TS_FIFO_EMPTY, reg_ts_status);
    *tx_ts_fifo_full = RU_FIELD_GET(umac_id, UNIMAC_RDP, TS_STATUS, TX_TS_FIFO_FULL, reg_ts_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_tx_ts_data_get(uint8_t umac_id, uint32_t *tx_ts_data)
{
    uint32_t reg_tx_ts_data;

#ifdef VALIDATE_PARMS
    if(!tx_ts_data)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, TX_TS_DATA, reg_tx_ts_data);

    *tx_ts_data = RU_FIELD_GET(umac_id, UNIMAC_RDP, TX_TS_DATA, TX_TS_DATA, reg_tx_ts_data);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_pause_refresh_ctrl_set(uint8_t umac_id, bdmf_boolean enable, uint32_t refresh_timer)
{
    uint32_t reg_pause_refresh_ctrl=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT) ||
       (enable >= _1BITS_MAX_VAL_) ||
       (refresh_timer >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pause_refresh_ctrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, PAUSE_REFRESH_CTRL, ENABLE, reg_pause_refresh_ctrl, enable);
    reg_pause_refresh_ctrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, PAUSE_REFRESH_CTRL, REFRESH_TIMER, reg_pause_refresh_ctrl, refresh_timer);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, PAUSE_REFRESH_CTRL, reg_pause_refresh_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_pause_refresh_ctrl_get(uint8_t umac_id, bdmf_boolean *enable, uint32_t *refresh_timer)
{
    uint32_t reg_pause_refresh_ctrl;

#ifdef VALIDATE_PARMS
    if(!enable || !refresh_timer)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, PAUSE_REFRESH_CTRL, reg_pause_refresh_ctrl);

    *enable = RU_FIELD_GET(umac_id, UNIMAC_RDP, PAUSE_REFRESH_CTRL, ENABLE, reg_pause_refresh_ctrl);
    *refresh_timer = RU_FIELD_GET(umac_id, UNIMAC_RDP, PAUSE_REFRESH_CTRL, REFRESH_TIMER, reg_pause_refresh_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_flush_control_set(uint8_t umac_id, bdmf_boolean flush)
{
    uint32_t reg_flush_control=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT) ||
       (flush >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_flush_control = RU_FIELD_SET(umac_id, UNIMAC_RDP, FLUSH_CONTROL, FLUSH, reg_flush_control, flush);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, FLUSH_CONTROL, reg_flush_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_flush_control_get(uint8_t umac_id, bdmf_boolean *flush)
{
    uint32_t reg_flush_control;

#ifdef VALIDATE_PARMS
    if(!flush)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, FLUSH_CONTROL, reg_flush_control);

    *flush = RU_FIELD_GET(umac_id, UNIMAC_RDP, FLUSH_CONTROL, FLUSH, reg_flush_control);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_rxfifo_stat_get(uint8_t umac_id, bdmf_boolean *rxfifo_overrun, bdmf_boolean *rxfifo_underrun)
{
    uint32_t reg_rxfifo_stat;

#ifdef VALIDATE_PARMS
    if(!rxfifo_overrun || !rxfifo_underrun)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, RXFIFO_STAT, reg_rxfifo_stat);

    *rxfifo_overrun = RU_FIELD_GET(umac_id, UNIMAC_RDP, RXFIFO_STAT, RXFIFO_OVERRUN, reg_rxfifo_stat);
    *rxfifo_underrun = RU_FIELD_GET(umac_id, UNIMAC_RDP, RXFIFO_STAT, RXFIFO_UNDERRUN, reg_rxfifo_stat);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_txfifo_stat_get(uint8_t umac_id, bdmf_boolean *txfifo_overrun, bdmf_boolean *txfifo_underrun)
{
    uint32_t reg_txfifo_stat;

#ifdef VALIDATE_PARMS
    if(!txfifo_overrun || !txfifo_underrun)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, TXFIFO_STAT, reg_txfifo_stat);

    *txfifo_overrun = RU_FIELD_GET(umac_id, UNIMAC_RDP, TXFIFO_STAT, TXFIFO_OVERRUN, reg_txfifo_stat);
    *txfifo_underrun = RU_FIELD_GET(umac_id, UNIMAC_RDP, TXFIFO_STAT, TXFIFO_UNDERRUN, reg_txfifo_stat);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_mac_pfc_ctrl_set(uint8_t umac_id, const unimac_rdp_mac_pfc_ctrl *mac_pfc_ctrl)
{
    uint32_t reg_mac_pfc_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!mac_pfc_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT) ||
       (mac_pfc_ctrl->pfc_stats_en >= _1BITS_MAX_VAL_) ||
       (mac_pfc_ctrl->rx_pass_pfc_frm >= _1BITS_MAX_VAL_) ||
       (mac_pfc_ctrl->force_pfc_xon >= _1BITS_MAX_VAL_) ||
       (mac_pfc_ctrl->pfc_rx_enbl >= _1BITS_MAX_VAL_) ||
       (mac_pfc_ctrl->pfc_tx_enbl >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_mac_pfc_ctrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, MAC_PFC_CTRL, PFC_STATS_EN, reg_mac_pfc_ctrl, mac_pfc_ctrl->pfc_stats_en);
    reg_mac_pfc_ctrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, MAC_PFC_CTRL, RX_PASS_PFC_FRM, reg_mac_pfc_ctrl, mac_pfc_ctrl->rx_pass_pfc_frm);
    reg_mac_pfc_ctrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, MAC_PFC_CTRL, FORCE_PFC_XON, reg_mac_pfc_ctrl, mac_pfc_ctrl->force_pfc_xon);
    reg_mac_pfc_ctrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, MAC_PFC_CTRL, PFC_RX_ENBL, reg_mac_pfc_ctrl, mac_pfc_ctrl->pfc_rx_enbl);
    reg_mac_pfc_ctrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, MAC_PFC_CTRL, PFC_TX_ENBL, reg_mac_pfc_ctrl, mac_pfc_ctrl->pfc_tx_enbl);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, MAC_PFC_CTRL, reg_mac_pfc_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_mac_pfc_ctrl_get(uint8_t umac_id, unimac_rdp_mac_pfc_ctrl *mac_pfc_ctrl)
{
    uint32_t reg_mac_pfc_ctrl;

#ifdef VALIDATE_PARMS
    if(!mac_pfc_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, MAC_PFC_CTRL, reg_mac_pfc_ctrl);

    mac_pfc_ctrl->pfc_stats_en = RU_FIELD_GET(umac_id, UNIMAC_RDP, MAC_PFC_CTRL, PFC_STATS_EN, reg_mac_pfc_ctrl);
    mac_pfc_ctrl->rx_pass_pfc_frm = RU_FIELD_GET(umac_id, UNIMAC_RDP, MAC_PFC_CTRL, RX_PASS_PFC_FRM, reg_mac_pfc_ctrl);
    mac_pfc_ctrl->force_pfc_xon = RU_FIELD_GET(umac_id, UNIMAC_RDP, MAC_PFC_CTRL, FORCE_PFC_XON, reg_mac_pfc_ctrl);
    mac_pfc_ctrl->pfc_rx_enbl = RU_FIELD_GET(umac_id, UNIMAC_RDP, MAC_PFC_CTRL, PFC_RX_ENBL, reg_mac_pfc_ctrl);
    mac_pfc_ctrl->pfc_tx_enbl = RU_FIELD_GET(umac_id, UNIMAC_RDP, MAC_PFC_CTRL, PFC_TX_ENBL, reg_mac_pfc_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_mac_pfc_refresh_ctrl_set(uint8_t umac_id, uint16_t pfc_refresh_timer, bdmf_boolean pfc_refresh_en)
{
    uint32_t reg_mac_pfc_refresh_ctrl=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT) ||
       (pfc_refresh_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_mac_pfc_refresh_ctrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, MAC_PFC_REFRESH_CTRL, PFC_REFRESH_TIMER, reg_mac_pfc_refresh_ctrl, pfc_refresh_timer);
    reg_mac_pfc_refresh_ctrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, MAC_PFC_REFRESH_CTRL, PFC_REFRESH_EN, reg_mac_pfc_refresh_ctrl, pfc_refresh_en);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, MAC_PFC_REFRESH_CTRL, reg_mac_pfc_refresh_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_mac_pfc_refresh_ctrl_get(uint8_t umac_id, uint16_t *pfc_refresh_timer, bdmf_boolean *pfc_refresh_en)
{
    uint32_t reg_mac_pfc_refresh_ctrl;

#ifdef VALIDATE_PARMS
    if(!pfc_refresh_timer || !pfc_refresh_en)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_id, UNIMAC_RDP, MAC_PFC_REFRESH_CTRL, reg_mac_pfc_refresh_ctrl);

    *pfc_refresh_timer = RU_FIELD_GET(umac_id, UNIMAC_RDP, MAC_PFC_REFRESH_CTRL, PFC_REFRESH_TIMER, reg_mac_pfc_refresh_ctrl);
    *pfc_refresh_en = RU_FIELD_GET(umac_id, UNIMAC_RDP, MAC_PFC_REFRESH_CTRL, PFC_REFRESH_EN, reg_mac_pfc_refresh_ctrl);

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
}
bdmf_address;

static int bcm_unimac_rdp_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_unimac_rdp_ipg_hd_bkp_cntl:
        err = ag_drv_unimac_rdp_ipg_hd_bkp_cntl_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_unimac_rdp_command_config:
    {
        unimac_rdp_command_config command_config = { .runt_filter_dis=parm[2].value.unumber, .oob_efc_en=parm[3].value.unumber, .ignore_tx_pause=parm[4].value.unumber, .fd_tx_urun_fix_en=parm[5].value.unumber, .line_loopback=parm[6].value.unumber, .no_lgth_check=parm[7].value.unumber, .cntl_frm_ena=parm[8].value.unumber, .ena_ext_config=parm[9].value.unumber, .en_internal_tx_crs=parm[10].value.unumber, .sw_override_rx=parm[11].value.unumber, .sw_override_tx=parm[12].value.unumber, .mac_loop_con=parm[13].value.unumber, .loop_ena=parm[14].value.unumber, .fcs_corrupt_urun_en=parm[15].value.unumber, .sw_reset=parm[16].value.unumber, .overflow_en=parm[17].value.unumber, .rx_low_latency_en=parm[18].value.unumber, .hd_ena=parm[19].value.unumber, .tx_addr_ins=parm[20].value.unumber, .pause_ignore=parm[21].value.unumber, .pause_fwd=parm[22].value.unumber, .crc_fwd=parm[23].value.unumber, .pad_en=parm[24].value.unumber, .promis_en=parm[25].value.unumber, .eth_speed=parm[26].value.unumber, .rx_ena=parm[27].value.unumber, .tx_ena=parm[28].value.unumber};
        err = ag_drv_unimac_rdp_command_config_set(parm[1].value.unumber, &command_config);
        break;
    }
    case cli_unimac_rdp_mac_0:
        err = ag_drv_unimac_rdp_mac_0_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_rdp_mac_1:
        err = ag_drv_unimac_rdp_mac_1_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_rdp_frm_length:
        err = ag_drv_unimac_rdp_frm_length_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_rdp_pause_quant:
        err = ag_drv_unimac_rdp_pause_quant_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_rdp_sfd_offset:
        err = ag_drv_unimac_rdp_sfd_offset_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_rdp_tag_0:
        err = ag_drv_unimac_rdp_tag_0_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_unimac_rdp_tag_1:
        err = ag_drv_unimac_rdp_tag_1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_unimac_rdp_rx_pause_quanta_scale:
        err = ag_drv_unimac_rdp_rx_pause_quanta_scale_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_unimac_rdp_tx_preamble:
        err = ag_drv_unimac_rdp_tx_preamble_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_rdp_tx_ipg_length:
        err = ag_drv_unimac_rdp_tx_ipg_length_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_unimac_rdp_pfc_xoff_timer:
        err = ag_drv_unimac_rdp_pfc_xoff_timer_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_rdp_umac_eee_ctrl:
    {
        unimac_rdp_umac_eee_ctrl umac_eee_ctrl = { .lp_idle_prediction_mode=parm[2].value.unumber, .dis_eee_10m=parm[3].value.unumber, .eee_txclk_dis=parm[4].value.unumber, .rx_fifo_check=parm[5].value.unumber, .eee_en=parm[6].value.unumber};
        err = ag_drv_unimac_rdp_umac_eee_ctrl_set(parm[1].value.unumber, &umac_eee_ctrl);
        break;
    }
    case cli_unimac_rdp_mii_eee_delay_entry_timer:
        err = ag_drv_unimac_rdp_mii_eee_delay_entry_timer_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_rdp_gmii_eee_delay_entry_timer:
        err = ag_drv_unimac_rdp_gmii_eee_delay_entry_timer_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_rdp_umac_eee_ref_count:
        err = ag_drv_unimac_rdp_umac_eee_ref_count_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_rdp_umac_timestamp_adjust:
        err = ag_drv_unimac_rdp_umac_timestamp_adjust_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_unimac_rdp_umac_rx_pkt_drop_status:
        err = ag_drv_unimac_rdp_umac_rx_pkt_drop_status_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_rdp_umac_symmetric_idle_threshold:
        err = ag_drv_unimac_rdp_umac_symmetric_idle_threshold_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_rdp_mii_eee_wake_timer:
        err = ag_drv_unimac_rdp_mii_eee_wake_timer_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_rdp_gmii_eee_wake_timer:
        err = ag_drv_unimac_rdp_gmii_eee_wake_timer_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_rdp_mac_pfc_type:
        err = ag_drv_unimac_rdp_mac_pfc_type_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_rdp_mac_pfc_opcode:
        err = ag_drv_unimac_rdp_mac_pfc_opcode_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_rdp_mac_pfc_da_0:
        err = ag_drv_unimac_rdp_mac_pfc_da_0_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_rdp_mac_pfc_da_1:
        err = ag_drv_unimac_rdp_mac_pfc_da_1_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_rdp_macsec_prog_tx_crc:
        err = ag_drv_unimac_rdp_macsec_prog_tx_crc_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_rdp_macsec_cntrl:
        err = ag_drv_unimac_rdp_macsec_cntrl_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber, parm[5].value.unumber);
        break;
    case cli_unimac_rdp_pause_refresh_ctrl:
        err = ag_drv_unimac_rdp_pause_refresh_ctrl_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_unimac_rdp_flush_control:
        err = ag_drv_unimac_rdp_flush_control_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_rdp_mac_pfc_ctrl:
    {
        unimac_rdp_mac_pfc_ctrl mac_pfc_ctrl = { .pfc_stats_en=parm[2].value.unumber, .rx_pass_pfc_frm=parm[3].value.unumber, .force_pfc_xon=parm[4].value.unumber, .pfc_rx_enbl=parm[5].value.unumber, .pfc_tx_enbl=parm[6].value.unumber};
        err = ag_drv_unimac_rdp_mac_pfc_ctrl_set(parm[1].value.unumber, &mac_pfc_ctrl);
        break;
    }
    case cli_unimac_rdp_mac_pfc_refresh_ctrl:
        err = ag_drv_unimac_rdp_mac_pfc_refresh_ctrl_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

int bcm_unimac_rdp_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_unimac_rdp_ipg_hd_bkp_cntl:
    {
        uint8_t ipg_config_rx;
        bdmf_boolean hd_fc_bkoff_ok;
        bdmf_boolean hd_fc_ena;
        err = ag_drv_unimac_rdp_ipg_hd_bkp_cntl_get(parm[1].value.unumber, &ipg_config_rx, &hd_fc_bkoff_ok, &hd_fc_ena);
        bdmf_session_print(session, "ipg_config_rx = %u (0x%x)\n", ipg_config_rx, ipg_config_rx);
        bdmf_session_print(session, "hd_fc_bkoff_ok = %u (0x%x)\n", hd_fc_bkoff_ok, hd_fc_bkoff_ok);
        bdmf_session_print(session, "hd_fc_ena = %u (0x%x)\n", hd_fc_ena, hd_fc_ena);
        break;
    }
    case cli_unimac_rdp_command_config:
    {
        unimac_rdp_command_config command_config;
        err = ag_drv_unimac_rdp_command_config_get(parm[1].value.unumber, &command_config);
        bdmf_session_print(session, "runt_filter_dis = %u (0x%x)\n", command_config.runt_filter_dis, command_config.runt_filter_dis);
        bdmf_session_print(session, "oob_efc_en = %u (0x%x)\n", command_config.oob_efc_en, command_config.oob_efc_en);
        bdmf_session_print(session, "ignore_tx_pause = %u (0x%x)\n", command_config.ignore_tx_pause, command_config.ignore_tx_pause);
        bdmf_session_print(session, "fd_tx_urun_fix_en = %u (0x%x)\n", command_config.fd_tx_urun_fix_en, command_config.fd_tx_urun_fix_en);
        bdmf_session_print(session, "line_loopback = %u (0x%x)\n", command_config.line_loopback, command_config.line_loopback);
        bdmf_session_print(session, "no_lgth_check = %u (0x%x)\n", command_config.no_lgth_check, command_config.no_lgth_check);
        bdmf_session_print(session, "cntl_frm_ena = %u (0x%x)\n", command_config.cntl_frm_ena, command_config.cntl_frm_ena);
        bdmf_session_print(session, "ena_ext_config = %u (0x%x)\n", command_config.ena_ext_config, command_config.ena_ext_config);
        bdmf_session_print(session, "en_internal_tx_crs = %u (0x%x)\n", command_config.en_internal_tx_crs, command_config.en_internal_tx_crs);
        bdmf_session_print(session, "sw_override_rx = %u (0x%x)\n", command_config.sw_override_rx, command_config.sw_override_rx);
        bdmf_session_print(session, "sw_override_tx = %u (0x%x)\n", command_config.sw_override_tx, command_config.sw_override_tx);
        bdmf_session_print(session, "mac_loop_con = %u (0x%x)\n", command_config.mac_loop_con, command_config.mac_loop_con);
        bdmf_session_print(session, "loop_ena = %u (0x%x)\n", command_config.loop_ena, command_config.loop_ena);
        bdmf_session_print(session, "fcs_corrupt_urun_en = %u (0x%x)\n", command_config.fcs_corrupt_urun_en, command_config.fcs_corrupt_urun_en);
        bdmf_session_print(session, "sw_reset = %u (0x%x)\n", command_config.sw_reset, command_config.sw_reset);
        bdmf_session_print(session, "overflow_en = %u (0x%x)\n", command_config.overflow_en, command_config.overflow_en);
        bdmf_session_print(session, "rx_low_latency_en = %u (0x%x)\n", command_config.rx_low_latency_en, command_config.rx_low_latency_en);
        bdmf_session_print(session, "hd_ena = %u (0x%x)\n", command_config.hd_ena, command_config.hd_ena);
        bdmf_session_print(session, "tx_addr_ins = %u (0x%x)\n", command_config.tx_addr_ins, command_config.tx_addr_ins);
        bdmf_session_print(session, "pause_ignore = %u (0x%x)\n", command_config.pause_ignore, command_config.pause_ignore);
        bdmf_session_print(session, "pause_fwd = %u (0x%x)\n", command_config.pause_fwd, command_config.pause_fwd);
        bdmf_session_print(session, "crc_fwd = %u (0x%x)\n", command_config.crc_fwd, command_config.crc_fwd);
        bdmf_session_print(session, "pad_en = %u (0x%x)\n", command_config.pad_en, command_config.pad_en);
        bdmf_session_print(session, "promis_en = %u (0x%x)\n", command_config.promis_en, command_config.promis_en);
        bdmf_session_print(session, "eth_speed = %u (0x%x)\n", command_config.eth_speed, command_config.eth_speed);
        bdmf_session_print(session, "rx_ena = %u (0x%x)\n", command_config.rx_ena, command_config.rx_ena);
        bdmf_session_print(session, "tx_ena = %u (0x%x)\n", command_config.tx_ena, command_config.tx_ena);
        break;
    }
    case cli_unimac_rdp_mac_0:
    {
        uint32_t mac_addr0;
        err = ag_drv_unimac_rdp_mac_0_get(parm[1].value.unumber, &mac_addr0);
        bdmf_session_print(session, "mac_addr0 = %u (0x%x)\n", mac_addr0, mac_addr0);
        break;
    }
    case cli_unimac_rdp_mac_1:
    {
        uint16_t mac_addr1;
        err = ag_drv_unimac_rdp_mac_1_get(parm[1].value.unumber, &mac_addr1);
        bdmf_session_print(session, "mac_addr1 = %u (0x%x)\n", mac_addr1, mac_addr1);
        break;
    }
    case cli_unimac_rdp_frm_length:
    {
        uint16_t maxfr;
        err = ag_drv_unimac_rdp_frm_length_get(parm[1].value.unumber, &maxfr);
        bdmf_session_print(session, "maxfr = %u (0x%x)\n", maxfr, maxfr);
        break;
    }
    case cli_unimac_rdp_pause_quant:
    {
        uint16_t pause_quant;
        err = ag_drv_unimac_rdp_pause_quant_get(parm[1].value.unumber, &pause_quant);
        bdmf_session_print(session, "pause_quant = %u (0x%x)\n", pause_quant, pause_quant);
        break;
    }
    case cli_unimac_rdp_tx_ts_seq_id:
    {
        bdmf_boolean tsts_valid;
        uint16_t tsts_seq_id;
        err = ag_drv_unimac_rdp_tx_ts_seq_id_get(parm[1].value.unumber, &tsts_valid, &tsts_seq_id);
        bdmf_session_print(session, "tsts_valid = %u (0x%x)\n", tsts_valid, tsts_valid);
        bdmf_session_print(session, "tsts_seq_id = %u (0x%x)\n", tsts_seq_id, tsts_seq_id);
        break;
    }
    case cli_unimac_rdp_sfd_offset:
    {
        uint8_t sfd_offset;
        err = ag_drv_unimac_rdp_sfd_offset_get(parm[1].value.unumber, &sfd_offset);
        bdmf_session_print(session, "sfd_offset = %u (0x%x)\n", sfd_offset, sfd_offset);
        break;
    }
    case cli_unimac_rdp_mac_mode:
    {
        unimac_rdp_mac_mode mac_mode;
        err = ag_drv_unimac_rdp_mac_mode_get(parm[1].value.unumber, &mac_mode);
        bdmf_session_print(session, "link_status = %u (0x%x)\n", mac_mode.link_status, mac_mode.link_status);
        bdmf_session_print(session, "mac_tx_pause = %u (0x%x)\n", mac_mode.mac_tx_pause, mac_mode.mac_tx_pause);
        bdmf_session_print(session, "mac_rx_pause = %u (0x%x)\n", mac_mode.mac_rx_pause, mac_mode.mac_rx_pause);
        bdmf_session_print(session, "mac_duplex = %u (0x%x)\n", mac_mode.mac_duplex, mac_mode.mac_duplex);
        bdmf_session_print(session, "mac_speed = %u (0x%x)\n", mac_mode.mac_speed, mac_mode.mac_speed);
        break;
    }
    case cli_unimac_rdp_tag_0:
    {
        bdmf_boolean config_outer_tpid_enable;
        uint16_t frm_tag_0;
        err = ag_drv_unimac_rdp_tag_0_get(parm[1].value.unumber, &config_outer_tpid_enable, &frm_tag_0);
        bdmf_session_print(session, "config_outer_tpid_enable = %u (0x%x)\n", config_outer_tpid_enable, config_outer_tpid_enable);
        bdmf_session_print(session, "frm_tag_0 = %u (0x%x)\n", frm_tag_0, frm_tag_0);
        break;
    }
    case cli_unimac_rdp_tag_1:
    {
        bdmf_boolean config_inner_tpid_enable;
        uint16_t frm_tag_1;
        err = ag_drv_unimac_rdp_tag_1_get(parm[1].value.unumber, &config_inner_tpid_enable, &frm_tag_1);
        bdmf_session_print(session, "config_inner_tpid_enable = %u (0x%x)\n", config_inner_tpid_enable, config_inner_tpid_enable);
        bdmf_session_print(session, "frm_tag_1 = %u (0x%x)\n", frm_tag_1, frm_tag_1);
        break;
    }
    case cli_unimac_rdp_rx_pause_quanta_scale:
    {
        bdmf_boolean scale_fix;
        bdmf_boolean scale_control;
        uint16_t scale_value;
        err = ag_drv_unimac_rdp_rx_pause_quanta_scale_get(parm[1].value.unumber, &scale_fix, &scale_control, &scale_value);
        bdmf_session_print(session, "scale_fix = %u (0x%x)\n", scale_fix, scale_fix);
        bdmf_session_print(session, "scale_control = %u (0x%x)\n", scale_control, scale_control);
        bdmf_session_print(session, "scale_value = %u (0x%x)\n", scale_value, scale_value);
        break;
    }
    case cli_unimac_rdp_tx_preamble:
    {
        uint8_t tx_preamble;
        err = ag_drv_unimac_rdp_tx_preamble_get(parm[1].value.unumber, &tx_preamble);
        bdmf_session_print(session, "tx_preamble = %u (0x%x)\n", tx_preamble, tx_preamble);
        break;
    }
    case cli_unimac_rdp_tx_ipg_length:
    {
        uint8_t tx_min_pkt_size;
        uint8_t tx_ipg_length;
        err = ag_drv_unimac_rdp_tx_ipg_length_get(parm[1].value.unumber, &tx_min_pkt_size, &tx_ipg_length);
        bdmf_session_print(session, "tx_min_pkt_size = %u (0x%x)\n", tx_min_pkt_size, tx_min_pkt_size);
        bdmf_session_print(session, "tx_ipg_length = %u (0x%x)\n", tx_ipg_length, tx_ipg_length);
        break;
    }
    case cli_unimac_rdp_pfc_xoff_timer:
    {
        uint16_t pfc_xoff_timer;
        err = ag_drv_unimac_rdp_pfc_xoff_timer_get(parm[1].value.unumber, &pfc_xoff_timer);
        bdmf_session_print(session, "pfc_xoff_timer = %u (0x%x)\n", pfc_xoff_timer, pfc_xoff_timer);
        break;
    }
    case cli_unimac_rdp_umac_eee_ctrl:
    {
        unimac_rdp_umac_eee_ctrl umac_eee_ctrl;
        err = ag_drv_unimac_rdp_umac_eee_ctrl_get(parm[1].value.unumber, &umac_eee_ctrl);
        bdmf_session_print(session, "lp_idle_prediction_mode = %u (0x%x)\n", umac_eee_ctrl.lp_idle_prediction_mode, umac_eee_ctrl.lp_idle_prediction_mode);
        bdmf_session_print(session, "dis_eee_10m = %u (0x%x)\n", umac_eee_ctrl.dis_eee_10m, umac_eee_ctrl.dis_eee_10m);
        bdmf_session_print(session, "eee_txclk_dis = %u (0x%x)\n", umac_eee_ctrl.eee_txclk_dis, umac_eee_ctrl.eee_txclk_dis);
        bdmf_session_print(session, "rx_fifo_check = %u (0x%x)\n", umac_eee_ctrl.rx_fifo_check, umac_eee_ctrl.rx_fifo_check);
        bdmf_session_print(session, "eee_en = %u (0x%x)\n", umac_eee_ctrl.eee_en, umac_eee_ctrl.eee_en);
        break;
    }
    case cli_unimac_rdp_mii_eee_delay_entry_timer:
    {
        uint32_t mii_eee_lpi_timer;
        err = ag_drv_unimac_rdp_mii_eee_delay_entry_timer_get(parm[1].value.unumber, &mii_eee_lpi_timer);
        bdmf_session_print(session, "mii_eee_lpi_timer = %u (0x%x)\n", mii_eee_lpi_timer, mii_eee_lpi_timer);
        break;
    }
    case cli_unimac_rdp_gmii_eee_delay_entry_timer:
    {
        uint32_t gmii_eee_lpi_timer;
        err = ag_drv_unimac_rdp_gmii_eee_delay_entry_timer_get(parm[1].value.unumber, &gmii_eee_lpi_timer);
        bdmf_session_print(session, "gmii_eee_lpi_timer = %u (0x%x)\n", gmii_eee_lpi_timer, gmii_eee_lpi_timer);
        break;
    }
    case cli_unimac_rdp_umac_eee_ref_count:
    {
        uint16_t eee_ref_count;
        err = ag_drv_unimac_rdp_umac_eee_ref_count_get(parm[1].value.unumber, &eee_ref_count);
        bdmf_session_print(session, "eee_ref_count = %u (0x%x)\n", eee_ref_count, eee_ref_count);
        break;
    }
    case cli_unimac_rdp_umac_timestamp_adjust:
    {
        bdmf_boolean auto_adjust;
        bdmf_boolean en_1588;
        uint16_t adjust;
        err = ag_drv_unimac_rdp_umac_timestamp_adjust_get(parm[1].value.unumber, &auto_adjust, &en_1588, &adjust);
        bdmf_session_print(session, "auto_adjust = %u (0x%x)\n", auto_adjust, auto_adjust);
        bdmf_session_print(session, "en_1588 = %u (0x%x)\n", en_1588, en_1588);
        bdmf_session_print(session, "adjust = %u (0x%x)\n", adjust, adjust);
        break;
    }
    case cli_unimac_rdp_umac_rx_pkt_drop_status:
    {
        bdmf_boolean rx_ipg_inval;
        err = ag_drv_unimac_rdp_umac_rx_pkt_drop_status_get(parm[1].value.unumber, &rx_ipg_inval);
        bdmf_session_print(session, "rx_ipg_inval = %u (0x%x)\n", rx_ipg_inval, rx_ipg_inval);
        break;
    }
    case cli_unimac_rdp_umac_symmetric_idle_threshold:
    {
        uint16_t threshold_value;
        err = ag_drv_unimac_rdp_umac_symmetric_idle_threshold_get(parm[1].value.unumber, &threshold_value);
        bdmf_session_print(session, "threshold_value = %u (0x%x)\n", threshold_value, threshold_value);
        break;
    }
    case cli_unimac_rdp_mii_eee_wake_timer:
    {
        uint16_t mii_eee_wake_timer;
        err = ag_drv_unimac_rdp_mii_eee_wake_timer_get(parm[1].value.unumber, &mii_eee_wake_timer);
        bdmf_session_print(session, "mii_eee_wake_timer = %u (0x%x)\n", mii_eee_wake_timer, mii_eee_wake_timer);
        break;
    }
    case cli_unimac_rdp_gmii_eee_wake_timer:
    {
        uint16_t gmii_eee_wake_timer;
        err = ag_drv_unimac_rdp_gmii_eee_wake_timer_get(parm[1].value.unumber, &gmii_eee_wake_timer);
        bdmf_session_print(session, "gmii_eee_wake_timer = %u (0x%x)\n", gmii_eee_wake_timer, gmii_eee_wake_timer);
        break;
    }
    case cli_unimac_rdp_umac_rev_id:
    {
        uint8_t revision_id_major;
        uint8_t revision_id_minor;
        uint8_t patch;
        err = ag_drv_unimac_rdp_umac_rev_id_get(parm[1].value.unumber, &revision_id_major, &revision_id_minor, &patch);
        bdmf_session_print(session, "revision_id_major = %u (0x%x)\n", revision_id_major, revision_id_major);
        bdmf_session_print(session, "revision_id_minor = %u (0x%x)\n", revision_id_minor, revision_id_minor);
        bdmf_session_print(session, "patch = %u (0x%x)\n", patch, patch);
        break;
    }
    case cli_unimac_rdp_mac_pfc_type:
    {
        uint16_t pfc_eth_type;
        err = ag_drv_unimac_rdp_mac_pfc_type_get(parm[1].value.unumber, &pfc_eth_type);
        bdmf_session_print(session, "pfc_eth_type = %u (0x%x)\n", pfc_eth_type, pfc_eth_type);
        break;
    }
    case cli_unimac_rdp_mac_pfc_opcode:
    {
        uint16_t pfc_opcode;
        err = ag_drv_unimac_rdp_mac_pfc_opcode_get(parm[1].value.unumber, &pfc_opcode);
        bdmf_session_print(session, "pfc_opcode = %u (0x%x)\n", pfc_opcode, pfc_opcode);
        break;
    }
    case cli_unimac_rdp_mac_pfc_da_0:
    {
        uint32_t pfc_macda_0;
        err = ag_drv_unimac_rdp_mac_pfc_da_0_get(parm[1].value.unumber, &pfc_macda_0);
        bdmf_session_print(session, "pfc_macda_0 = %u (0x%x)\n", pfc_macda_0, pfc_macda_0);
        break;
    }
    case cli_unimac_rdp_mac_pfc_da_1:
    {
        uint16_t pfc_macda_1;
        err = ag_drv_unimac_rdp_mac_pfc_da_1_get(parm[1].value.unumber, &pfc_macda_1);
        bdmf_session_print(session, "pfc_macda_1 = %u (0x%x)\n", pfc_macda_1, pfc_macda_1);
        break;
    }
    case cli_unimac_rdp_macsec_prog_tx_crc:
    {
        uint32_t macsec_prog_tx_crc;
        err = ag_drv_unimac_rdp_macsec_prog_tx_crc_get(parm[1].value.unumber, &macsec_prog_tx_crc);
        bdmf_session_print(session, "macsec_prog_tx_crc = %u (0x%x)\n", macsec_prog_tx_crc, macsec_prog_tx_crc);
        break;
    }
    case cli_unimac_rdp_macsec_cntrl:
    {
        bdmf_boolean dis_pause_data_var_ipg;
        bdmf_boolean tx_crc_program;
        bdmf_boolean tx_crc_corupt_en;
        bdmf_boolean tx_launch_en;
        err = ag_drv_unimac_rdp_macsec_cntrl_get(parm[1].value.unumber, &dis_pause_data_var_ipg, &tx_crc_program, &tx_crc_corupt_en, &tx_launch_en);
        bdmf_session_print(session, "dis_pause_data_var_ipg = %u (0x%x)\n", dis_pause_data_var_ipg, dis_pause_data_var_ipg);
        bdmf_session_print(session, "tx_crc_program = %u (0x%x)\n", tx_crc_program, tx_crc_program);
        bdmf_session_print(session, "tx_crc_corupt_en = %u (0x%x)\n", tx_crc_corupt_en, tx_crc_corupt_en);
        bdmf_session_print(session, "tx_launch_en = %u (0x%x)\n", tx_launch_en, tx_launch_en);
        break;
    }
    case cli_unimac_rdp_ts_status:
    {
        uint8_t word_avail;
        bdmf_boolean tx_ts_fifo_empty;
        bdmf_boolean tx_ts_fifo_full;
        err = ag_drv_unimac_rdp_ts_status_get(parm[1].value.unumber, &word_avail, &tx_ts_fifo_empty, &tx_ts_fifo_full);
        bdmf_session_print(session, "word_avail = %u (0x%x)\n", word_avail, word_avail);
        bdmf_session_print(session, "tx_ts_fifo_empty = %u (0x%x)\n", tx_ts_fifo_empty, tx_ts_fifo_empty);
        bdmf_session_print(session, "tx_ts_fifo_full = %u (0x%x)\n", tx_ts_fifo_full, tx_ts_fifo_full);
        break;
    }
    case cli_unimac_rdp_tx_ts_data:
    {
        uint32_t tx_ts_data;
        err = ag_drv_unimac_rdp_tx_ts_data_get(parm[1].value.unumber, &tx_ts_data);
        bdmf_session_print(session, "tx_ts_data = %u (0x%x)\n", tx_ts_data, tx_ts_data);
        break;
    }
    case cli_unimac_rdp_pause_refresh_ctrl:
    {
        bdmf_boolean enable;
        uint32_t refresh_timer;
        err = ag_drv_unimac_rdp_pause_refresh_ctrl_get(parm[1].value.unumber, &enable, &refresh_timer);
        bdmf_session_print(session, "enable = %u (0x%x)\n", enable, enable);
        bdmf_session_print(session, "refresh_timer = %u (0x%x)\n", refresh_timer, refresh_timer);
        break;
    }
    case cli_unimac_rdp_flush_control:
    {
        bdmf_boolean flush;
        err = ag_drv_unimac_rdp_flush_control_get(parm[1].value.unumber, &flush);
        bdmf_session_print(session, "flush = %u (0x%x)\n", flush, flush);
        break;
    }
    case cli_unimac_rdp_rxfifo_stat:
    {
        bdmf_boolean rxfifo_overrun;
        bdmf_boolean rxfifo_underrun;
        err = ag_drv_unimac_rdp_rxfifo_stat_get(parm[1].value.unumber, &rxfifo_overrun, &rxfifo_underrun);
        bdmf_session_print(session, "rxfifo_overrun = %u (0x%x)\n", rxfifo_overrun, rxfifo_overrun);
        bdmf_session_print(session, "rxfifo_underrun = %u (0x%x)\n", rxfifo_underrun, rxfifo_underrun);
        break;
    }
    case cli_unimac_rdp_txfifo_stat:
    {
        bdmf_boolean txfifo_overrun;
        bdmf_boolean txfifo_underrun;
        err = ag_drv_unimac_rdp_txfifo_stat_get(parm[1].value.unumber, &txfifo_overrun, &txfifo_underrun);
        bdmf_session_print(session, "txfifo_overrun = %u (0x%x)\n", txfifo_overrun, txfifo_overrun);
        bdmf_session_print(session, "txfifo_underrun = %u (0x%x)\n", txfifo_underrun, txfifo_underrun);
        break;
    }
    case cli_unimac_rdp_mac_pfc_ctrl:
    {
        unimac_rdp_mac_pfc_ctrl mac_pfc_ctrl;
        err = ag_drv_unimac_rdp_mac_pfc_ctrl_get(parm[1].value.unumber, &mac_pfc_ctrl);
        bdmf_session_print(session, "pfc_stats_en = %u (0x%x)\n", mac_pfc_ctrl.pfc_stats_en, mac_pfc_ctrl.pfc_stats_en);
        bdmf_session_print(session, "rx_pass_pfc_frm = %u (0x%x)\n", mac_pfc_ctrl.rx_pass_pfc_frm, mac_pfc_ctrl.rx_pass_pfc_frm);
        bdmf_session_print(session, "force_pfc_xon = %u (0x%x)\n", mac_pfc_ctrl.force_pfc_xon, mac_pfc_ctrl.force_pfc_xon);
        bdmf_session_print(session, "pfc_rx_enbl = %u (0x%x)\n", mac_pfc_ctrl.pfc_rx_enbl, mac_pfc_ctrl.pfc_rx_enbl);
        bdmf_session_print(session, "pfc_tx_enbl = %u (0x%x)\n", mac_pfc_ctrl.pfc_tx_enbl, mac_pfc_ctrl.pfc_tx_enbl);
        break;
    }
    case cli_unimac_rdp_mac_pfc_refresh_ctrl:
    {
        uint16_t pfc_refresh_timer;
        bdmf_boolean pfc_refresh_en;
        err = ag_drv_unimac_rdp_mac_pfc_refresh_ctrl_get(parm[1].value.unumber, &pfc_refresh_timer, &pfc_refresh_en);
        bdmf_session_print(session, "pfc_refresh_timer = %u (0x%x)\n", pfc_refresh_timer, pfc_refresh_timer);
        bdmf_session_print(session, "pfc_refresh_en = %u (0x%x)\n", pfc_refresh_en, pfc_refresh_en);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_unimac_rdp_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    uint8_t umac_id = parm[1].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        uint8_t ipg_config_rx=gtmv(m, 5);
        bdmf_boolean hd_fc_bkoff_ok=gtmv(m, 1);
        bdmf_boolean hd_fc_ena=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_ipg_hd_bkp_cntl_set(%u %u %u %u)\n", umac_id, ipg_config_rx, hd_fc_bkoff_ok, hd_fc_ena);
        if(!err) ag_drv_unimac_rdp_ipg_hd_bkp_cntl_set(umac_id, ipg_config_rx, hd_fc_bkoff_ok, hd_fc_ena);
        if(!err) ag_drv_unimac_rdp_ipg_hd_bkp_cntl_get( umac_id, &ipg_config_rx, &hd_fc_bkoff_ok, &hd_fc_ena);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_ipg_hd_bkp_cntl_get(%u %u %u %u)\n", umac_id, ipg_config_rx, hd_fc_bkoff_ok, hd_fc_ena);
        if(err || ipg_config_rx!=gtmv(m, 5) || hd_fc_bkoff_ok!=gtmv(m, 1) || hd_fc_ena!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        unimac_rdp_command_config command_config = {.runt_filter_dis=gtmv(m, 1), .oob_efc_en=gtmv(m, 1), .ignore_tx_pause=gtmv(m, 1), .fd_tx_urun_fix_en=gtmv(m, 1), .line_loopback=gtmv(m, 1), .no_lgth_check=gtmv(m, 1), .cntl_frm_ena=gtmv(m, 1), .ena_ext_config=gtmv(m, 1), .en_internal_tx_crs=gtmv(m, 1), .sw_override_rx=gtmv(m, 1), .sw_override_tx=gtmv(m, 1), .mac_loop_con=gtmv(m, 1), .loop_ena=gtmv(m, 1), .fcs_corrupt_urun_en=gtmv(m, 1), .sw_reset=gtmv(m, 1), .overflow_en=gtmv(m, 1), .rx_low_latency_en=gtmv(m, 1), .hd_ena=gtmv(m, 1), .tx_addr_ins=gtmv(m, 1), .pause_ignore=gtmv(m, 1), .pause_fwd=gtmv(m, 1), .crc_fwd=gtmv(m, 1), .pad_en=gtmv(m, 1), .promis_en=gtmv(m, 1), .eth_speed=gtmv(m, 2), .rx_ena=gtmv(m, 1), .tx_ena=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_command_config_set(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", umac_id, command_config.runt_filter_dis, command_config.oob_efc_en, command_config.ignore_tx_pause, command_config.fd_tx_urun_fix_en, command_config.line_loopback, command_config.no_lgth_check, command_config.cntl_frm_ena, command_config.ena_ext_config, command_config.en_internal_tx_crs, command_config.sw_override_rx, command_config.sw_override_tx, command_config.mac_loop_con, command_config.loop_ena, command_config.fcs_corrupt_urun_en, command_config.sw_reset, command_config.overflow_en, command_config.rx_low_latency_en, command_config.hd_ena, command_config.tx_addr_ins, command_config.pause_ignore, command_config.pause_fwd, command_config.crc_fwd, command_config.pad_en, command_config.promis_en, command_config.eth_speed, command_config.rx_ena, command_config.tx_ena);
        if(!err) ag_drv_unimac_rdp_command_config_set(umac_id, &command_config);
        if(!err) ag_drv_unimac_rdp_command_config_get( umac_id, &command_config);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_command_config_get(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", umac_id, command_config.runt_filter_dis, command_config.oob_efc_en, command_config.ignore_tx_pause, command_config.fd_tx_urun_fix_en, command_config.line_loopback, command_config.no_lgth_check, command_config.cntl_frm_ena, command_config.ena_ext_config, command_config.en_internal_tx_crs, command_config.sw_override_rx, command_config.sw_override_tx, command_config.mac_loop_con, command_config.loop_ena, command_config.fcs_corrupt_urun_en, command_config.sw_reset, command_config.overflow_en, command_config.rx_low_latency_en, command_config.hd_ena, command_config.tx_addr_ins, command_config.pause_ignore, command_config.pause_fwd, command_config.crc_fwd, command_config.pad_en, command_config.promis_en, command_config.eth_speed, command_config.rx_ena, command_config.tx_ena);
        if(err || command_config.runt_filter_dis!=gtmv(m, 1) || command_config.oob_efc_en!=gtmv(m, 1) || command_config.ignore_tx_pause!=gtmv(m, 1) || command_config.fd_tx_urun_fix_en!=gtmv(m, 1) || command_config.line_loopback!=gtmv(m, 1) || command_config.no_lgth_check!=gtmv(m, 1) || command_config.cntl_frm_ena!=gtmv(m, 1) || command_config.ena_ext_config!=gtmv(m, 1) || command_config.en_internal_tx_crs!=gtmv(m, 1) || command_config.sw_override_rx!=gtmv(m, 1) || command_config.sw_override_tx!=gtmv(m, 1) || command_config.mac_loop_con!=gtmv(m, 1) || command_config.loop_ena!=gtmv(m, 1) || command_config.fcs_corrupt_urun_en!=gtmv(m, 1) || command_config.sw_reset!=gtmv(m, 1) || command_config.overflow_en!=gtmv(m, 1) || command_config.rx_low_latency_en!=gtmv(m, 1) || command_config.hd_ena!=gtmv(m, 1) || command_config.tx_addr_ins!=gtmv(m, 1) || command_config.pause_ignore!=gtmv(m, 1) || command_config.pause_fwd!=gtmv(m, 1) || command_config.crc_fwd!=gtmv(m, 1) || command_config.pad_en!=gtmv(m, 1) || command_config.promis_en!=gtmv(m, 1) || command_config.eth_speed!=gtmv(m, 2) || command_config.rx_ena!=gtmv(m, 1) || command_config.tx_ena!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t mac_addr0=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_mac_0_set(%u %u)\n", umac_id, mac_addr0);
        if(!err) ag_drv_unimac_rdp_mac_0_set(umac_id, mac_addr0);
        if(!err) ag_drv_unimac_rdp_mac_0_get( umac_id, &mac_addr0);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_mac_0_get(%u %u)\n", umac_id, mac_addr0);
        if(err || mac_addr0!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t mac_addr1=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_mac_1_set(%u %u)\n", umac_id, mac_addr1);
        if(!err) ag_drv_unimac_rdp_mac_1_set(umac_id, mac_addr1);
        if(!err) ag_drv_unimac_rdp_mac_1_get( umac_id, &mac_addr1);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_mac_1_get(%u %u)\n", umac_id, mac_addr1);
        if(err || mac_addr1!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t maxfr=gtmv(m, 14);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_frm_length_set(%u %u)\n", umac_id, maxfr);
        if(!err) ag_drv_unimac_rdp_frm_length_set(umac_id, maxfr);
        if(!err) ag_drv_unimac_rdp_frm_length_get( umac_id, &maxfr);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_frm_length_get(%u %u)\n", umac_id, maxfr);
        if(err || maxfr!=gtmv(m, 14))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t pause_quant=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_pause_quant_set(%u %u)\n", umac_id, pause_quant);
        if(!err) ag_drv_unimac_rdp_pause_quant_set(umac_id, pause_quant);
        if(!err) ag_drv_unimac_rdp_pause_quant_get( umac_id, &pause_quant);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_pause_quant_get(%u %u)\n", umac_id, pause_quant);
        if(err || pause_quant!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean tsts_valid=gtmv(m, 1);
        uint16_t tsts_seq_id=gtmv(m, 16);
        if(!err) ag_drv_unimac_rdp_tx_ts_seq_id_get( umac_id, &tsts_valid, &tsts_seq_id);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_tx_ts_seq_id_get(%u %u %u)\n", umac_id, tsts_valid, tsts_seq_id);
    }
    {
        uint8_t sfd_offset=gtmv(m, 4);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_sfd_offset_set(%u %u)\n", umac_id, sfd_offset);
        if(!err) ag_drv_unimac_rdp_sfd_offset_set(umac_id, sfd_offset);
        if(!err) ag_drv_unimac_rdp_sfd_offset_get( umac_id, &sfd_offset);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_sfd_offset_get(%u %u)\n", umac_id, sfd_offset);
        if(err || sfd_offset!=gtmv(m, 4))
            return err ? err : BDMF_ERR_IO;
    }
    {
        unimac_rdp_mac_mode mac_mode = {.link_status=gtmv(m, 1), .mac_tx_pause=gtmv(m, 1), .mac_rx_pause=gtmv(m, 1), .mac_duplex=gtmv(m, 1), .mac_speed=gtmv(m, 2)};
        if(!err) ag_drv_unimac_rdp_mac_mode_get( umac_id, &mac_mode);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_mac_mode_get(%u %u %u %u %u %u)\n", umac_id, mac_mode.link_status, mac_mode.mac_tx_pause, mac_mode.mac_rx_pause, mac_mode.mac_duplex, mac_mode.mac_speed);
    }
    {
        bdmf_boolean config_outer_tpid_enable=gtmv(m, 1);
        uint16_t frm_tag_0=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_tag_0_set(%u %u %u)\n", umac_id, config_outer_tpid_enable, frm_tag_0);
        if(!err) ag_drv_unimac_rdp_tag_0_set(umac_id, config_outer_tpid_enable, frm_tag_0);
        if(!err) ag_drv_unimac_rdp_tag_0_get( umac_id, &config_outer_tpid_enable, &frm_tag_0);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_tag_0_get(%u %u %u)\n", umac_id, config_outer_tpid_enable, frm_tag_0);
        if(err || config_outer_tpid_enable!=gtmv(m, 1) || frm_tag_0!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean config_inner_tpid_enable=gtmv(m, 1);
        uint16_t frm_tag_1=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_tag_1_set(%u %u %u)\n", umac_id, config_inner_tpid_enable, frm_tag_1);
        if(!err) ag_drv_unimac_rdp_tag_1_set(umac_id, config_inner_tpid_enable, frm_tag_1);
        if(!err) ag_drv_unimac_rdp_tag_1_get( umac_id, &config_inner_tpid_enable, &frm_tag_1);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_tag_1_get(%u %u %u)\n", umac_id, config_inner_tpid_enable, frm_tag_1);
        if(err || config_inner_tpid_enable!=gtmv(m, 1) || frm_tag_1!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean scale_fix=gtmv(m, 1);
        bdmf_boolean scale_control=gtmv(m, 1);
        uint16_t scale_value=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_rx_pause_quanta_scale_set(%u %u %u %u)\n", umac_id, scale_fix, scale_control, scale_value);
        if(!err) ag_drv_unimac_rdp_rx_pause_quanta_scale_set(umac_id, scale_fix, scale_control, scale_value);
        if(!err) ag_drv_unimac_rdp_rx_pause_quanta_scale_get( umac_id, &scale_fix, &scale_control, &scale_value);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_rx_pause_quanta_scale_get(%u %u %u %u)\n", umac_id, scale_fix, scale_control, scale_value);
        if(err || scale_fix!=gtmv(m, 1) || scale_control!=gtmv(m, 1) || scale_value!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t tx_preamble=gtmv(m, 3);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_tx_preamble_set(%u %u)\n", umac_id, tx_preamble);
        if(!err) ag_drv_unimac_rdp_tx_preamble_set(umac_id, tx_preamble);
        if(!err) ag_drv_unimac_rdp_tx_preamble_get( umac_id, &tx_preamble);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_tx_preamble_get(%u %u)\n", umac_id, tx_preamble);
        if(err || tx_preamble!=gtmv(m, 3))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t tx_min_pkt_size=gtmv(m, 7);
        uint8_t tx_ipg_length=gtmv(m, 7);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_tx_ipg_length_set(%u %u %u)\n", umac_id, tx_min_pkt_size, tx_ipg_length);
        if(!err) ag_drv_unimac_rdp_tx_ipg_length_set(umac_id, tx_min_pkt_size, tx_ipg_length);
        if(!err) ag_drv_unimac_rdp_tx_ipg_length_get( umac_id, &tx_min_pkt_size, &tx_ipg_length);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_tx_ipg_length_get(%u %u %u)\n", umac_id, tx_min_pkt_size, tx_ipg_length);
        if(err || tx_min_pkt_size!=gtmv(m, 7) || tx_ipg_length!=gtmv(m, 7))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t pfc_xoff_timer=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_pfc_xoff_timer_set(%u %u)\n", umac_id, pfc_xoff_timer);
        if(!err) ag_drv_unimac_rdp_pfc_xoff_timer_set(umac_id, pfc_xoff_timer);
        if(!err) ag_drv_unimac_rdp_pfc_xoff_timer_get( umac_id, &pfc_xoff_timer);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_pfc_xoff_timer_get(%u %u)\n", umac_id, pfc_xoff_timer);
        if(err || pfc_xoff_timer!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        unimac_rdp_umac_eee_ctrl umac_eee_ctrl = {.lp_idle_prediction_mode=gtmv(m, 1), .dis_eee_10m=gtmv(m, 1), .eee_txclk_dis=gtmv(m, 1), .rx_fifo_check=gtmv(m, 1), .eee_en=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_umac_eee_ctrl_set(%u %u %u %u %u %u)\n", umac_id, umac_eee_ctrl.lp_idle_prediction_mode, umac_eee_ctrl.dis_eee_10m, umac_eee_ctrl.eee_txclk_dis, umac_eee_ctrl.rx_fifo_check, umac_eee_ctrl.eee_en);
        if(!err) ag_drv_unimac_rdp_umac_eee_ctrl_set(umac_id, &umac_eee_ctrl);
        if(!err) ag_drv_unimac_rdp_umac_eee_ctrl_get( umac_id, &umac_eee_ctrl);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_umac_eee_ctrl_get(%u %u %u %u %u %u)\n", umac_id, umac_eee_ctrl.lp_idle_prediction_mode, umac_eee_ctrl.dis_eee_10m, umac_eee_ctrl.eee_txclk_dis, umac_eee_ctrl.rx_fifo_check, umac_eee_ctrl.eee_en);
        if(err || umac_eee_ctrl.lp_idle_prediction_mode!=gtmv(m, 1) || umac_eee_ctrl.dis_eee_10m!=gtmv(m, 1) || umac_eee_ctrl.eee_txclk_dis!=gtmv(m, 1) || umac_eee_ctrl.rx_fifo_check!=gtmv(m, 1) || umac_eee_ctrl.eee_en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t mii_eee_lpi_timer=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_mii_eee_delay_entry_timer_set(%u %u)\n", umac_id, mii_eee_lpi_timer);
        if(!err) ag_drv_unimac_rdp_mii_eee_delay_entry_timer_set(umac_id, mii_eee_lpi_timer);
        if(!err) ag_drv_unimac_rdp_mii_eee_delay_entry_timer_get( umac_id, &mii_eee_lpi_timer);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_mii_eee_delay_entry_timer_get(%u %u)\n", umac_id, mii_eee_lpi_timer);
        if(err || mii_eee_lpi_timer!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gmii_eee_lpi_timer=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_gmii_eee_delay_entry_timer_set(%u %u)\n", umac_id, gmii_eee_lpi_timer);
        if(!err) ag_drv_unimac_rdp_gmii_eee_delay_entry_timer_set(umac_id, gmii_eee_lpi_timer);
        if(!err) ag_drv_unimac_rdp_gmii_eee_delay_entry_timer_get( umac_id, &gmii_eee_lpi_timer);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_gmii_eee_delay_entry_timer_get(%u %u)\n", umac_id, gmii_eee_lpi_timer);
        if(err || gmii_eee_lpi_timer!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t eee_ref_count=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_umac_eee_ref_count_set(%u %u)\n", umac_id, eee_ref_count);
        if(!err) ag_drv_unimac_rdp_umac_eee_ref_count_set(umac_id, eee_ref_count);
        if(!err) ag_drv_unimac_rdp_umac_eee_ref_count_get( umac_id, &eee_ref_count);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_umac_eee_ref_count_get(%u %u)\n", umac_id, eee_ref_count);
        if(err || eee_ref_count!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean auto_adjust=gtmv(m, 1);
        bdmf_boolean en_1588=gtmv(m, 1);
        uint16_t adjust=gtmv(m, 9);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_umac_timestamp_adjust_set(%u %u %u %u)\n", umac_id, auto_adjust, en_1588, adjust);
        if(!err) ag_drv_unimac_rdp_umac_timestamp_adjust_set(umac_id, auto_adjust, en_1588, adjust);
        if(!err) ag_drv_unimac_rdp_umac_timestamp_adjust_get( umac_id, &auto_adjust, &en_1588, &adjust);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_umac_timestamp_adjust_get(%u %u %u %u)\n", umac_id, auto_adjust, en_1588, adjust);
        if(err || auto_adjust!=gtmv(m, 1) || en_1588!=gtmv(m, 1) || adjust!=gtmv(m, 9))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean rx_ipg_inval=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_umac_rx_pkt_drop_status_set(%u %u)\n", umac_id, rx_ipg_inval);
        if(!err) ag_drv_unimac_rdp_umac_rx_pkt_drop_status_set(umac_id, rx_ipg_inval);
        if(!err) ag_drv_unimac_rdp_umac_rx_pkt_drop_status_get( umac_id, &rx_ipg_inval);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_umac_rx_pkt_drop_status_get(%u %u)\n", umac_id, rx_ipg_inval);
        if(err || rx_ipg_inval!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t threshold_value=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_umac_symmetric_idle_threshold_set(%u %u)\n", umac_id, threshold_value);
        if(!err) ag_drv_unimac_rdp_umac_symmetric_idle_threshold_set(umac_id, threshold_value);
        if(!err) ag_drv_unimac_rdp_umac_symmetric_idle_threshold_get( umac_id, &threshold_value);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_umac_symmetric_idle_threshold_get(%u %u)\n", umac_id, threshold_value);
        if(err || threshold_value!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t mii_eee_wake_timer=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_mii_eee_wake_timer_set(%u %u)\n", umac_id, mii_eee_wake_timer);
        if(!err) ag_drv_unimac_rdp_mii_eee_wake_timer_set(umac_id, mii_eee_wake_timer);
        if(!err) ag_drv_unimac_rdp_mii_eee_wake_timer_get( umac_id, &mii_eee_wake_timer);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_mii_eee_wake_timer_get(%u %u)\n", umac_id, mii_eee_wake_timer);
        if(err || mii_eee_wake_timer!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t gmii_eee_wake_timer=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_gmii_eee_wake_timer_set(%u %u)\n", umac_id, gmii_eee_wake_timer);
        if(!err) ag_drv_unimac_rdp_gmii_eee_wake_timer_set(umac_id, gmii_eee_wake_timer);
        if(!err) ag_drv_unimac_rdp_gmii_eee_wake_timer_get( umac_id, &gmii_eee_wake_timer);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_gmii_eee_wake_timer_get(%u %u)\n", umac_id, gmii_eee_wake_timer);
        if(err || gmii_eee_wake_timer!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t revision_id_major=gtmv(m, 8);
        uint8_t revision_id_minor=gtmv(m, 8);
        uint8_t patch=gtmv(m, 8);
        if(!err) ag_drv_unimac_rdp_umac_rev_id_get( umac_id, &revision_id_major, &revision_id_minor, &patch);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_umac_rev_id_get(%u %u %u %u)\n", umac_id, revision_id_major, revision_id_minor, patch);
    }
    {
        uint16_t pfc_eth_type=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_mac_pfc_type_set(%u %u)\n", umac_id, pfc_eth_type);
        if(!err) ag_drv_unimac_rdp_mac_pfc_type_set(umac_id, pfc_eth_type);
        if(!err) ag_drv_unimac_rdp_mac_pfc_type_get( umac_id, &pfc_eth_type);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_mac_pfc_type_get(%u %u)\n", umac_id, pfc_eth_type);
        if(err || pfc_eth_type!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t pfc_opcode=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_mac_pfc_opcode_set(%u %u)\n", umac_id, pfc_opcode);
        if(!err) ag_drv_unimac_rdp_mac_pfc_opcode_set(umac_id, pfc_opcode);
        if(!err) ag_drv_unimac_rdp_mac_pfc_opcode_get( umac_id, &pfc_opcode);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_mac_pfc_opcode_get(%u %u)\n", umac_id, pfc_opcode);
        if(err || pfc_opcode!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t pfc_macda_0=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_mac_pfc_da_0_set(%u %u)\n", umac_id, pfc_macda_0);
        if(!err) ag_drv_unimac_rdp_mac_pfc_da_0_set(umac_id, pfc_macda_0);
        if(!err) ag_drv_unimac_rdp_mac_pfc_da_0_get( umac_id, &pfc_macda_0);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_mac_pfc_da_0_get(%u %u)\n", umac_id, pfc_macda_0);
        if(err || pfc_macda_0!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t pfc_macda_1=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_mac_pfc_da_1_set(%u %u)\n", umac_id, pfc_macda_1);
        if(!err) ag_drv_unimac_rdp_mac_pfc_da_1_set(umac_id, pfc_macda_1);
        if(!err) ag_drv_unimac_rdp_mac_pfc_da_1_get( umac_id, &pfc_macda_1);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_mac_pfc_da_1_get(%u %u)\n", umac_id, pfc_macda_1);
        if(err || pfc_macda_1!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t macsec_prog_tx_crc=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_macsec_prog_tx_crc_set(%u %u)\n", umac_id, macsec_prog_tx_crc);
        if(!err) ag_drv_unimac_rdp_macsec_prog_tx_crc_set(umac_id, macsec_prog_tx_crc);
        if(!err) ag_drv_unimac_rdp_macsec_prog_tx_crc_get( umac_id, &macsec_prog_tx_crc);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_macsec_prog_tx_crc_get(%u %u)\n", umac_id, macsec_prog_tx_crc);
        if(err || macsec_prog_tx_crc!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean dis_pause_data_var_ipg=gtmv(m, 1);
        bdmf_boolean tx_crc_program=gtmv(m, 1);
        bdmf_boolean tx_crc_corupt_en=gtmv(m, 1);
        bdmf_boolean tx_launch_en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_macsec_cntrl_set(%u %u %u %u %u)\n", umac_id, dis_pause_data_var_ipg, tx_crc_program, tx_crc_corupt_en, tx_launch_en);
        if(!err) ag_drv_unimac_rdp_macsec_cntrl_set(umac_id, dis_pause_data_var_ipg, tx_crc_program, tx_crc_corupt_en, tx_launch_en);
        if(!err) ag_drv_unimac_rdp_macsec_cntrl_get( umac_id, &dis_pause_data_var_ipg, &tx_crc_program, &tx_crc_corupt_en, &tx_launch_en);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_macsec_cntrl_get(%u %u %u %u %u)\n", umac_id, dis_pause_data_var_ipg, tx_crc_program, tx_crc_corupt_en, tx_launch_en);
        if(err || dis_pause_data_var_ipg!=gtmv(m, 1) || tx_crc_program!=gtmv(m, 1) || tx_crc_corupt_en!=gtmv(m, 1) || tx_launch_en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t word_avail=gtmv(m, 3);
        bdmf_boolean tx_ts_fifo_empty=gtmv(m, 1);
        bdmf_boolean tx_ts_fifo_full=gtmv(m, 1);
        if(!err) ag_drv_unimac_rdp_ts_status_get( umac_id, &word_avail, &tx_ts_fifo_empty, &tx_ts_fifo_full);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_ts_status_get(%u %u %u %u)\n", umac_id, word_avail, tx_ts_fifo_empty, tx_ts_fifo_full);
    }
    {
        uint32_t tx_ts_data=gtmv(m, 32);
        if(!err) ag_drv_unimac_rdp_tx_ts_data_get( umac_id, &tx_ts_data);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_tx_ts_data_get(%u %u)\n", umac_id, tx_ts_data);
    }
    {
        bdmf_boolean enable=gtmv(m, 1);
        uint32_t refresh_timer=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_pause_refresh_ctrl_set(%u %u %u)\n", umac_id, enable, refresh_timer);
        if(!err) ag_drv_unimac_rdp_pause_refresh_ctrl_set(umac_id, enable, refresh_timer);
        if(!err) ag_drv_unimac_rdp_pause_refresh_ctrl_get( umac_id, &enable, &refresh_timer);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_pause_refresh_ctrl_get(%u %u %u)\n", umac_id, enable, refresh_timer);
        if(err || enable!=gtmv(m, 1) || refresh_timer!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean flush=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_flush_control_set(%u %u)\n", umac_id, flush);
        if(!err) ag_drv_unimac_rdp_flush_control_set(umac_id, flush);
        if(!err) ag_drv_unimac_rdp_flush_control_get( umac_id, &flush);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_flush_control_get(%u %u)\n", umac_id, flush);
        if(err || flush!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean rxfifo_overrun=gtmv(m, 1);
        bdmf_boolean rxfifo_underrun=gtmv(m, 1);
        if(!err) ag_drv_unimac_rdp_rxfifo_stat_get( umac_id, &rxfifo_overrun, &rxfifo_underrun);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_rxfifo_stat_get(%u %u %u)\n", umac_id, rxfifo_overrun, rxfifo_underrun);
    }
    {
        bdmf_boolean txfifo_overrun=gtmv(m, 1);
        bdmf_boolean txfifo_underrun=gtmv(m, 1);
        if(!err) ag_drv_unimac_rdp_txfifo_stat_get( umac_id, &txfifo_overrun, &txfifo_underrun);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_txfifo_stat_get(%u %u %u)\n", umac_id, txfifo_overrun, txfifo_underrun);
    }
    {
        unimac_rdp_mac_pfc_ctrl mac_pfc_ctrl = {.pfc_stats_en=gtmv(m, 1), .rx_pass_pfc_frm=gtmv(m, 1), .force_pfc_xon=gtmv(m, 1), .pfc_rx_enbl=gtmv(m, 1), .pfc_tx_enbl=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_mac_pfc_ctrl_set(%u %u %u %u %u %u)\n", umac_id, mac_pfc_ctrl.pfc_stats_en, mac_pfc_ctrl.rx_pass_pfc_frm, mac_pfc_ctrl.force_pfc_xon, mac_pfc_ctrl.pfc_rx_enbl, mac_pfc_ctrl.pfc_tx_enbl);
        if(!err) ag_drv_unimac_rdp_mac_pfc_ctrl_set(umac_id, &mac_pfc_ctrl);
        if(!err) ag_drv_unimac_rdp_mac_pfc_ctrl_get( umac_id, &mac_pfc_ctrl);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_mac_pfc_ctrl_get(%u %u %u %u %u %u)\n", umac_id, mac_pfc_ctrl.pfc_stats_en, mac_pfc_ctrl.rx_pass_pfc_frm, mac_pfc_ctrl.force_pfc_xon, mac_pfc_ctrl.pfc_rx_enbl, mac_pfc_ctrl.pfc_tx_enbl);
        if(err || mac_pfc_ctrl.pfc_stats_en!=gtmv(m, 1) || mac_pfc_ctrl.rx_pass_pfc_frm!=gtmv(m, 1) || mac_pfc_ctrl.force_pfc_xon!=gtmv(m, 1) || mac_pfc_ctrl.pfc_rx_enbl!=gtmv(m, 1) || mac_pfc_ctrl.pfc_tx_enbl!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t pfc_refresh_timer=gtmv(m, 16);
        bdmf_boolean pfc_refresh_en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_mac_pfc_refresh_ctrl_set(%u %u %u)\n", umac_id, pfc_refresh_timer, pfc_refresh_en);
        if(!err) ag_drv_unimac_rdp_mac_pfc_refresh_ctrl_set(umac_id, pfc_refresh_timer, pfc_refresh_en);
        if(!err) ag_drv_unimac_rdp_mac_pfc_refresh_ctrl_get( umac_id, &pfc_refresh_timer, &pfc_refresh_en);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_mac_pfc_refresh_ctrl_get(%u %u %u)\n", umac_id, pfc_refresh_timer, pfc_refresh_en);
        if(err || pfc_refresh_timer!=gtmv(m, 16) || pfc_refresh_en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    return err;
}

static int bcm_unimac_rdp_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_ipg_hd_bkp_cntl : reg = &RU_REG(UNIMAC_RDP, IPG_HD_BKP_CNTL); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_command_config : reg = &RU_REG(UNIMAC_RDP, COMMAND_CONFIG); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_mac_0 : reg = &RU_REG(UNIMAC_RDP, MAC_0); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_mac_1 : reg = &RU_REG(UNIMAC_RDP, MAC_1); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_frm_length : reg = &RU_REG(UNIMAC_RDP, FRM_LENGTH); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_pause_quant : reg = &RU_REG(UNIMAC_RDP, PAUSE_QUANT); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_tx_ts_seq_id : reg = &RU_REG(UNIMAC_RDP, TX_TS_SEQ_ID); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_sfd_offset : reg = &RU_REG(UNIMAC_RDP, SFD_OFFSET); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_mac_mode : reg = &RU_REG(UNIMAC_RDP, MAC_MODE); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_tag_0 : reg = &RU_REG(UNIMAC_RDP, TAG_0); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_tag_1 : reg = &RU_REG(UNIMAC_RDP, TAG_1); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_rx_pause_quanta_scale : reg = &RU_REG(UNIMAC_RDP, RX_PAUSE_QUANTA_SCALE); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_tx_preamble : reg = &RU_REG(UNIMAC_RDP, TX_PREAMBLE); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_tx_ipg_length : reg = &RU_REG(UNIMAC_RDP, TX_IPG_LENGTH); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_pfc_xoff_timer : reg = &RU_REG(UNIMAC_RDP, PFC_XOFF_TIMER); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_umac_eee_ctrl : reg = &RU_REG(UNIMAC_RDP, UMAC_EEE_CTRL); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_mii_eee_delay_entry_timer : reg = &RU_REG(UNIMAC_RDP, MII_EEE_DELAY_ENTRY_TIMER); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_gmii_eee_delay_entry_timer : reg = &RU_REG(UNIMAC_RDP, GMII_EEE_DELAY_ENTRY_TIMER); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_umac_eee_ref_count : reg = &RU_REG(UNIMAC_RDP, UMAC_EEE_REF_COUNT); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_umac_timestamp_adjust : reg = &RU_REG(UNIMAC_RDP, UMAC_TIMESTAMP_ADJUST); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_umac_rx_pkt_drop_status : reg = &RU_REG(UNIMAC_RDP, UMAC_RX_PKT_DROP_STATUS); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_umac_symmetric_idle_threshold : reg = &RU_REG(UNIMAC_RDP, UMAC_SYMMETRIC_IDLE_THRESHOLD); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_mii_eee_wake_timer : reg = &RU_REG(UNIMAC_RDP, MII_EEE_WAKE_TIMER); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_gmii_eee_wake_timer : reg = &RU_REG(UNIMAC_RDP, GMII_EEE_WAKE_TIMER); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_umac_rev_id : reg = &RU_REG(UNIMAC_RDP, UMAC_REV_ID); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_mac_pfc_type : reg = &RU_REG(UNIMAC_RDP, MAC_PFC_TYPE); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_mac_pfc_opcode : reg = &RU_REG(UNIMAC_RDP, MAC_PFC_OPCODE); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_mac_pfc_da_0 : reg = &RU_REG(UNIMAC_RDP, MAC_PFC_DA_0); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_mac_pfc_da_1 : reg = &RU_REG(UNIMAC_RDP, MAC_PFC_DA_1); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_macsec_prog_tx_crc : reg = &RU_REG(UNIMAC_RDP, MACSEC_PROG_TX_CRC); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_macsec_cntrl : reg = &RU_REG(UNIMAC_RDP, MACSEC_CNTRL); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_ts_status : reg = &RU_REG(UNIMAC_RDP, TS_STATUS); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_tx_ts_data : reg = &RU_REG(UNIMAC_RDP, TX_TS_DATA); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_pause_refresh_ctrl : reg = &RU_REG(UNIMAC_RDP, PAUSE_REFRESH_CTRL); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_flush_control : reg = &RU_REG(UNIMAC_RDP, FLUSH_CONTROL); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_rxfifo_stat : reg = &RU_REG(UNIMAC_RDP, RXFIFO_STAT); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_txfifo_stat : reg = &RU_REG(UNIMAC_RDP, TXFIFO_STAT); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_mac_pfc_ctrl : reg = &RU_REG(UNIMAC_RDP, MAC_PFC_CTRL); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_mac_pfc_refresh_ctrl : reg = &RU_REG(UNIMAC_RDP, MAC_PFC_REFRESH_CTRL); blk = &RU_BLK(UNIMAC_RDP); break;
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

bdmfmon_handle_t ag_drv_unimac_rdp_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "unimac_rdp"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "unimac_rdp", "unimac_rdp", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_ipg_hd_bkp_cntl[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("ipg_config_rx", "ipg_config_rx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("hd_fc_bkoff_ok", "hd_fc_bkoff_ok", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("hd_fc_ena", "hd_fc_ena", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_command_config[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("runt_filter_dis", "runt_filter_dis", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("oob_efc_en", "oob_efc_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ignore_tx_pause", "ignore_tx_pause", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fd_tx_urun_fix_en", "fd_tx_urun_fix_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("line_loopback", "line_loopback", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("no_lgth_check", "no_lgth_check", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cntl_frm_ena", "cntl_frm_ena", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ena_ext_config", "ena_ext_config", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("en_internal_tx_crs", "en_internal_tx_crs", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("sw_override_rx", "sw_override_rx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("sw_override_tx", "sw_override_tx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mac_loop_con", "mac_loop_con", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("loop_ena", "loop_ena", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fcs_corrupt_urun_en", "fcs_corrupt_urun_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("sw_reset", "sw_reset", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("overflow_en", "overflow_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rx_low_latency_en", "rx_low_latency_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("hd_ena", "hd_ena", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tx_addr_ins", "tx_addr_ins", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pause_ignore", "pause_ignore", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pause_fwd", "pause_fwd", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("crc_fwd", "crc_fwd", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pad_en", "pad_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("promis_en", "promis_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("eth_speed", "eth_speed", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rx_ena", "rx_ena", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tx_ena", "tx_ena", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mac_0[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("mac_addr0", "mac_addr0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mac_1[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("mac_addr1", "mac_addr1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_frm_length[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("maxfr", "maxfr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pause_quant[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("pause_quant", "pause_quant", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_sfd_offset[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("sfd_offset", "sfd_offset", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tag_0[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("config_outer_tpid_enable", "config_outer_tpid_enable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("frm_tag_0", "frm_tag_0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tag_1[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("config_inner_tpid_enable", "config_inner_tpid_enable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("frm_tag_1", "frm_tag_1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_pause_quanta_scale[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("scale_fix", "scale_fix", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("scale_control", "scale_control", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("scale_value", "scale_value", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_preamble[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("tx_preamble", "tx_preamble", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_ipg_length[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("tx_min_pkt_size", "tx_min_pkt_size", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tx_ipg_length", "tx_ipg_length", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pfc_xoff_timer[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("pfc_xoff_timer", "pfc_xoff_timer", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_umac_eee_ctrl[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("lp_idle_prediction_mode", "lp_idle_prediction_mode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("dis_eee_10m", "dis_eee_10m", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("eee_txclk_dis", "eee_txclk_dis", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rx_fifo_check", "rx_fifo_check", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("eee_en", "eee_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mii_eee_delay_entry_timer[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("mii_eee_lpi_timer", "mii_eee_lpi_timer", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gmii_eee_delay_entry_timer[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gmii_eee_lpi_timer", "gmii_eee_lpi_timer", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_umac_eee_ref_count[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("eee_ref_count", "eee_ref_count", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_umac_timestamp_adjust[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("auto_adjust", "auto_adjust", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("en_1588", "en_1588", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("adjust", "adjust", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_umac_rx_pkt_drop_status[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("rx_ipg_inval", "rx_ipg_inval", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_umac_symmetric_idle_threshold[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("threshold_value", "threshold_value", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mii_eee_wake_timer[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("mii_eee_wake_timer", "mii_eee_wake_timer", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_gmii_eee_wake_timer[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gmii_eee_wake_timer", "gmii_eee_wake_timer", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mac_pfc_type[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("pfc_eth_type", "pfc_eth_type", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mac_pfc_opcode[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("pfc_opcode", "pfc_opcode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mac_pfc_da_0[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("pfc_macda_0", "pfc_macda_0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mac_pfc_da_1[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("pfc_macda_1", "pfc_macda_1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_macsec_prog_tx_crc[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("macsec_prog_tx_crc", "macsec_prog_tx_crc", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_macsec_cntrl[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("dis_pause_data_var_ipg", "dis_pause_data_var_ipg", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tx_crc_program", "tx_crc_program", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tx_crc_corupt_en", "tx_crc_corupt_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tx_launch_en", "tx_launch_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pause_refresh_ctrl[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("enable", "enable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("refresh_timer", "refresh_timer", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_flush_control[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("flush", "flush", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mac_pfc_ctrl[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("pfc_stats_en", "pfc_stats_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rx_pass_pfc_frm", "rx_pass_pfc_frm", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("force_pfc_xon", "force_pfc_xon", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pfc_rx_enbl", "pfc_rx_enbl", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pfc_tx_enbl", "pfc_tx_enbl", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mac_pfc_refresh_ctrl[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("pfc_refresh_timer", "pfc_refresh_timer", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pfc_refresh_en", "pfc_refresh_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="ipg_hd_bkp_cntl", .val=cli_unimac_rdp_ipg_hd_bkp_cntl, .parms=set_ipg_hd_bkp_cntl },
            { .name="command_config", .val=cli_unimac_rdp_command_config, .parms=set_command_config },
            { .name="mac_0", .val=cli_unimac_rdp_mac_0, .parms=set_mac_0 },
            { .name="mac_1", .val=cli_unimac_rdp_mac_1, .parms=set_mac_1 },
            { .name="frm_length", .val=cli_unimac_rdp_frm_length, .parms=set_frm_length },
            { .name="pause_quant", .val=cli_unimac_rdp_pause_quant, .parms=set_pause_quant },
            { .name="sfd_offset", .val=cli_unimac_rdp_sfd_offset, .parms=set_sfd_offset },
            { .name="tag_0", .val=cli_unimac_rdp_tag_0, .parms=set_tag_0 },
            { .name="tag_1", .val=cli_unimac_rdp_tag_1, .parms=set_tag_1 },
            { .name="rx_pause_quanta_scale", .val=cli_unimac_rdp_rx_pause_quanta_scale, .parms=set_rx_pause_quanta_scale },
            { .name="tx_preamble", .val=cli_unimac_rdp_tx_preamble, .parms=set_tx_preamble },
            { .name="tx_ipg_length", .val=cli_unimac_rdp_tx_ipg_length, .parms=set_tx_ipg_length },
            { .name="pfc_xoff_timer", .val=cli_unimac_rdp_pfc_xoff_timer, .parms=set_pfc_xoff_timer },
            { .name="umac_eee_ctrl", .val=cli_unimac_rdp_umac_eee_ctrl, .parms=set_umac_eee_ctrl },
            { .name="mii_eee_delay_entry_timer", .val=cli_unimac_rdp_mii_eee_delay_entry_timer, .parms=set_mii_eee_delay_entry_timer },
            { .name="gmii_eee_delay_entry_timer", .val=cli_unimac_rdp_gmii_eee_delay_entry_timer, .parms=set_gmii_eee_delay_entry_timer },
            { .name="umac_eee_ref_count", .val=cli_unimac_rdp_umac_eee_ref_count, .parms=set_umac_eee_ref_count },
            { .name="umac_timestamp_adjust", .val=cli_unimac_rdp_umac_timestamp_adjust, .parms=set_umac_timestamp_adjust },
            { .name="umac_rx_pkt_drop_status", .val=cli_unimac_rdp_umac_rx_pkt_drop_status, .parms=set_umac_rx_pkt_drop_status },
            { .name="umac_symmetric_idle_threshold", .val=cli_unimac_rdp_umac_symmetric_idle_threshold, .parms=set_umac_symmetric_idle_threshold },
            { .name="mii_eee_wake_timer", .val=cli_unimac_rdp_mii_eee_wake_timer, .parms=set_mii_eee_wake_timer },
            { .name="gmii_eee_wake_timer", .val=cli_unimac_rdp_gmii_eee_wake_timer, .parms=set_gmii_eee_wake_timer },
            { .name="mac_pfc_type", .val=cli_unimac_rdp_mac_pfc_type, .parms=set_mac_pfc_type },
            { .name="mac_pfc_opcode", .val=cli_unimac_rdp_mac_pfc_opcode, .parms=set_mac_pfc_opcode },
            { .name="mac_pfc_da_0", .val=cli_unimac_rdp_mac_pfc_da_0, .parms=set_mac_pfc_da_0 },
            { .name="mac_pfc_da_1", .val=cli_unimac_rdp_mac_pfc_da_1, .parms=set_mac_pfc_da_1 },
            { .name="macsec_prog_tx_crc", .val=cli_unimac_rdp_macsec_prog_tx_crc, .parms=set_macsec_prog_tx_crc },
            { .name="macsec_cntrl", .val=cli_unimac_rdp_macsec_cntrl, .parms=set_macsec_cntrl },
            { .name="pause_refresh_ctrl", .val=cli_unimac_rdp_pause_refresh_ctrl, .parms=set_pause_refresh_ctrl },
            { .name="flush_control", .val=cli_unimac_rdp_flush_control, .parms=set_flush_control },
            { .name="mac_pfc_ctrl", .val=cli_unimac_rdp_mac_pfc_ctrl, .parms=set_mac_pfc_ctrl },
            { .name="mac_pfc_refresh_ctrl", .val=cli_unimac_rdp_mac_pfc_refresh_ctrl, .parms=set_mac_pfc_refresh_ctrl },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_unimac_rdp_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="ipg_hd_bkp_cntl", .val=cli_unimac_rdp_ipg_hd_bkp_cntl, .parms=set_default },
            { .name="command_config", .val=cli_unimac_rdp_command_config, .parms=set_default },
            { .name="mac_0", .val=cli_unimac_rdp_mac_0, .parms=set_default },
            { .name="mac_1", .val=cli_unimac_rdp_mac_1, .parms=set_default },
            { .name="frm_length", .val=cli_unimac_rdp_frm_length, .parms=set_default },
            { .name="pause_quant", .val=cli_unimac_rdp_pause_quant, .parms=set_default },
            { .name="tx_ts_seq_id", .val=cli_unimac_rdp_tx_ts_seq_id, .parms=set_default },
            { .name="sfd_offset", .val=cli_unimac_rdp_sfd_offset, .parms=set_default },
            { .name="mac_mode", .val=cli_unimac_rdp_mac_mode, .parms=set_default },
            { .name="tag_0", .val=cli_unimac_rdp_tag_0, .parms=set_default },
            { .name="tag_1", .val=cli_unimac_rdp_tag_1, .parms=set_default },
            { .name="rx_pause_quanta_scale", .val=cli_unimac_rdp_rx_pause_quanta_scale, .parms=set_default },
            { .name="tx_preamble", .val=cli_unimac_rdp_tx_preamble, .parms=set_default },
            { .name="tx_ipg_length", .val=cli_unimac_rdp_tx_ipg_length, .parms=set_default },
            { .name="pfc_xoff_timer", .val=cli_unimac_rdp_pfc_xoff_timer, .parms=set_default },
            { .name="umac_eee_ctrl", .val=cli_unimac_rdp_umac_eee_ctrl, .parms=set_default },
            { .name="mii_eee_delay_entry_timer", .val=cli_unimac_rdp_mii_eee_delay_entry_timer, .parms=set_default },
            { .name="gmii_eee_delay_entry_timer", .val=cli_unimac_rdp_gmii_eee_delay_entry_timer, .parms=set_default },
            { .name="umac_eee_ref_count", .val=cli_unimac_rdp_umac_eee_ref_count, .parms=set_default },
            { .name="umac_timestamp_adjust", .val=cli_unimac_rdp_umac_timestamp_adjust, .parms=set_default },
            { .name="umac_rx_pkt_drop_status", .val=cli_unimac_rdp_umac_rx_pkt_drop_status, .parms=set_default },
            { .name="umac_symmetric_idle_threshold", .val=cli_unimac_rdp_umac_symmetric_idle_threshold, .parms=set_default },
            { .name="mii_eee_wake_timer", .val=cli_unimac_rdp_mii_eee_wake_timer, .parms=set_default },
            { .name="gmii_eee_wake_timer", .val=cli_unimac_rdp_gmii_eee_wake_timer, .parms=set_default },
            { .name="umac_rev_id", .val=cli_unimac_rdp_umac_rev_id, .parms=set_default },
            { .name="mac_pfc_type", .val=cli_unimac_rdp_mac_pfc_type, .parms=set_default },
            { .name="mac_pfc_opcode", .val=cli_unimac_rdp_mac_pfc_opcode, .parms=set_default },
            { .name="mac_pfc_da_0", .val=cli_unimac_rdp_mac_pfc_da_0, .parms=set_default },
            { .name="mac_pfc_da_1", .val=cli_unimac_rdp_mac_pfc_da_1, .parms=set_default },
            { .name="macsec_prog_tx_crc", .val=cli_unimac_rdp_macsec_prog_tx_crc, .parms=set_default },
            { .name="macsec_cntrl", .val=cli_unimac_rdp_macsec_cntrl, .parms=set_default },
            { .name="ts_status", .val=cli_unimac_rdp_ts_status, .parms=set_default },
            { .name="tx_ts_data", .val=cli_unimac_rdp_tx_ts_data, .parms=set_default },
            { .name="pause_refresh_ctrl", .val=cli_unimac_rdp_pause_refresh_ctrl, .parms=set_default },
            { .name="flush_control", .val=cli_unimac_rdp_flush_control, .parms=set_default },
            { .name="rxfifo_stat", .val=cli_unimac_rdp_rxfifo_stat, .parms=set_default },
            { .name="txfifo_stat", .val=cli_unimac_rdp_txfifo_stat, .parms=set_default },
            { .name="mac_pfc_ctrl", .val=cli_unimac_rdp_mac_pfc_ctrl, .parms=set_default },
            { .name="mac_pfc_refresh_ctrl", .val=cli_unimac_rdp_mac_pfc_refresh_ctrl, .parms=set_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_unimac_rdp_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_unimac_rdp_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0),
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="IPG_HD_BKP_CNTL" , .val=bdmf_address_ipg_hd_bkp_cntl },
            { .name="COMMAND_CONFIG" , .val=bdmf_address_command_config },
            { .name="MAC_0" , .val=bdmf_address_mac_0 },
            { .name="MAC_1" , .val=bdmf_address_mac_1 },
            { .name="FRM_LENGTH" , .val=bdmf_address_frm_length },
            { .name="PAUSE_QUANT" , .val=bdmf_address_pause_quant },
            { .name="TX_TS_SEQ_ID" , .val=bdmf_address_tx_ts_seq_id },
            { .name="SFD_OFFSET" , .val=bdmf_address_sfd_offset },
            { .name="MAC_MODE" , .val=bdmf_address_mac_mode },
            { .name="TAG_0" , .val=bdmf_address_tag_0 },
            { .name="TAG_1" , .val=bdmf_address_tag_1 },
            { .name="RX_PAUSE_QUANTA_SCALE" , .val=bdmf_address_rx_pause_quanta_scale },
            { .name="TX_PREAMBLE" , .val=bdmf_address_tx_preamble },
            { .name="TX_IPG_LENGTH" , .val=bdmf_address_tx_ipg_length },
            { .name="PFC_XOFF_TIMER" , .val=bdmf_address_pfc_xoff_timer },
            { .name="UMAC_EEE_CTRL" , .val=bdmf_address_umac_eee_ctrl },
            { .name="MII_EEE_DELAY_ENTRY_TIMER" , .val=bdmf_address_mii_eee_delay_entry_timer },
            { .name="GMII_EEE_DELAY_ENTRY_TIMER" , .val=bdmf_address_gmii_eee_delay_entry_timer },
            { .name="UMAC_EEE_REF_COUNT" , .val=bdmf_address_umac_eee_ref_count },
            { .name="UMAC_TIMESTAMP_ADJUST" , .val=bdmf_address_umac_timestamp_adjust },
            { .name="UMAC_RX_PKT_DROP_STATUS" , .val=bdmf_address_umac_rx_pkt_drop_status },
            { .name="UMAC_SYMMETRIC_IDLE_THRESHOLD" , .val=bdmf_address_umac_symmetric_idle_threshold },
            { .name="MII_EEE_WAKE_TIMER" , .val=bdmf_address_mii_eee_wake_timer },
            { .name="GMII_EEE_WAKE_TIMER" , .val=bdmf_address_gmii_eee_wake_timer },
            { .name="UMAC_REV_ID" , .val=bdmf_address_umac_rev_id },
            { .name="MAC_PFC_TYPE" , .val=bdmf_address_mac_pfc_type },
            { .name="MAC_PFC_OPCODE" , .val=bdmf_address_mac_pfc_opcode },
            { .name="MAC_PFC_DA_0" , .val=bdmf_address_mac_pfc_da_0 },
            { .name="MAC_PFC_DA_1" , .val=bdmf_address_mac_pfc_da_1 },
            { .name="MACSEC_PROG_TX_CRC" , .val=bdmf_address_macsec_prog_tx_crc },
            { .name="MACSEC_CNTRL" , .val=bdmf_address_macsec_cntrl },
            { .name="TS_STATUS" , .val=bdmf_address_ts_status },
            { .name="TX_TS_DATA" , .val=bdmf_address_tx_ts_data },
            { .name="PAUSE_REFRESH_CTRL" , .val=bdmf_address_pause_refresh_ctrl },
            { .name="FLUSH_CONTROL" , .val=bdmf_address_flush_control },
            { .name="RXFIFO_STAT" , .val=bdmf_address_rxfifo_stat },
            { .name="TXFIFO_STAT" , .val=bdmf_address_txfifo_stat },
            { .name="MAC_PFC_CTRL" , .val=bdmf_address_mac_pfc_ctrl },
            { .name="MAC_PFC_REFRESH_CTRL" , .val=bdmf_address_mac_pfc_refresh_ctrl },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_unimac_rdp_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM_ENUM("index1", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

