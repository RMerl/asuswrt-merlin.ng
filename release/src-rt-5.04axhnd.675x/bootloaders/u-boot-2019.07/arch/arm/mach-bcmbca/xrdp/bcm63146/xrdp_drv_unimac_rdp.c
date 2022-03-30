// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    
*/

#include "rdp_common.h"
#include "xrdp_drv_drivers_common_ag.h"
#include "xrdp_drv_unimac_rdp.h"

#define BLOCK_ADDR_COUNT_BITS 3
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

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
       (command_config->oob_efc_disab >= _1BITS_MAX_VAL_) ||
       (command_config->ignore_tx_pause >= _1BITS_MAX_VAL_) ||
       (command_config->fd_tx_urun_fix_en >= _1BITS_MAX_VAL_) ||
       (command_config->line_loopback >= _1BITS_MAX_VAL_) ||
       (command_config->no_lgth_check >= _1BITS_MAX_VAL_) ||
       (command_config->cntl_frm_ena >= _1BITS_MAX_VAL_) ||
       (command_config->ena_ext_config >= _1BITS_MAX_VAL_) ||
       (command_config->en_internal_tx_crs >= _1BITS_MAX_VAL_) ||
       (command_config->bypass_oob_efc_synchronizer >= _1BITS_MAX_VAL_) ||
       (command_config->oob_efc_mode >= _1BITS_MAX_VAL_) ||
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
    reg_command_config = RU_FIELD_SET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, OOB_EFC_DISAB, reg_command_config, command_config->oob_efc_disab);
    reg_command_config = RU_FIELD_SET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, IGNORE_TX_PAUSE, reg_command_config, command_config->ignore_tx_pause);
    reg_command_config = RU_FIELD_SET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, FD_TX_URUN_FIX_EN, reg_command_config, command_config->fd_tx_urun_fix_en);
    reg_command_config = RU_FIELD_SET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, LINE_LOOPBACK, reg_command_config, command_config->line_loopback);
    reg_command_config = RU_FIELD_SET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, NO_LGTH_CHECK, reg_command_config, command_config->no_lgth_check);
    reg_command_config = RU_FIELD_SET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, CNTL_FRM_ENA, reg_command_config, command_config->cntl_frm_ena);
    reg_command_config = RU_FIELD_SET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, ENA_EXT_CONFIG, reg_command_config, command_config->ena_ext_config);
    reg_command_config = RU_FIELD_SET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, EN_INTERNAL_TX_CRS, reg_command_config, command_config->en_internal_tx_crs);
    reg_command_config = RU_FIELD_SET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, BYPASS_OOB_EFC_SYNCHRONIZER, reg_command_config, command_config->bypass_oob_efc_synchronizer);
    reg_command_config = RU_FIELD_SET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, OOB_EFC_MODE, reg_command_config, command_config->oob_efc_mode);
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
    command_config->oob_efc_disab = RU_FIELD_GET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, OOB_EFC_DISAB, reg_command_config);
    command_config->ignore_tx_pause = RU_FIELD_GET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, IGNORE_TX_PAUSE, reg_command_config);
    command_config->fd_tx_urun_fix_en = RU_FIELD_GET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, FD_TX_URUN_FIX_EN, reg_command_config);
    command_config->line_loopback = RU_FIELD_GET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, LINE_LOOPBACK, reg_command_config);
    command_config->no_lgth_check = RU_FIELD_GET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, NO_LGTH_CHECK, reg_command_config);
    command_config->cntl_frm_ena = RU_FIELD_GET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, CNTL_FRM_ENA, reg_command_config);
    command_config->ena_ext_config = RU_FIELD_GET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, ENA_EXT_CONFIG, reg_command_config);
    command_config->en_internal_tx_crs = RU_FIELD_GET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, EN_INTERNAL_TX_CRS, reg_command_config);
    command_config->bypass_oob_efc_synchronizer = RU_FIELD_GET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, BYPASS_OOB_EFC_SYNCHRONIZER, reg_command_config);
    command_config->oob_efc_mode = RU_FIELD_GET(umac_id, UNIMAC_RDP, COMMAND_CONFIG, OOB_EFC_MODE, reg_command_config);
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

