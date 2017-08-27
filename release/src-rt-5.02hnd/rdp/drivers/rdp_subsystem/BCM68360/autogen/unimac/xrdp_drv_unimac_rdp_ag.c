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

bdmf_error_t ag_drv_unimac_rdp_umac_dummy_get(uint8_t umac_id, uint8_t *umac_dummy)
{
    uint32_t reg_umac_dummy;

#ifdef VALIDATE_PARMS
    if(!umac_dummy)
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

    RU_REG_READ(umac_id, UNIMAC_RDP, UMAC_DUMMY, reg_umac_dummy);

    *umac_dummy = RU_FIELD_GET(umac_id, UNIMAC_RDP, UMAC_DUMMY, UMAC_DUMMY, reg_umac_dummy);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_hd_bkp_cntl_set(uint8_t umac_id, uint8_t ipg_config_rx, bdmf_boolean hd_fc_bkoff_ok, bdmf_boolean hd_fc_ena)
{
    uint32_t reg_hd_bkp_cntl=0;

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

    reg_hd_bkp_cntl = RU_FIELD_SET(umac_id, UNIMAC_RDP, HD_BKP_CNTL, IPG_CONFIG_RX, reg_hd_bkp_cntl, ipg_config_rx);
    reg_hd_bkp_cntl = RU_FIELD_SET(umac_id, UNIMAC_RDP, HD_BKP_CNTL, HD_FC_BKOFF_OK, reg_hd_bkp_cntl, hd_fc_bkoff_ok);
    reg_hd_bkp_cntl = RU_FIELD_SET(umac_id, UNIMAC_RDP, HD_BKP_CNTL, HD_FC_ENA, reg_hd_bkp_cntl, hd_fc_ena);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, HD_BKP_CNTL, reg_hd_bkp_cntl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_hd_bkp_cntl_get(uint8_t umac_id, uint8_t *ipg_config_rx, bdmf_boolean *hd_fc_bkoff_ok, bdmf_boolean *hd_fc_ena)
{
    uint32_t reg_hd_bkp_cntl;

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

    RU_REG_READ(umac_id, UNIMAC_RDP, HD_BKP_CNTL, reg_hd_bkp_cntl);

    *ipg_config_rx = RU_FIELD_GET(umac_id, UNIMAC_RDP, HD_BKP_CNTL, IPG_CONFIG_RX, reg_hd_bkp_cntl);
    *hd_fc_bkoff_ok = RU_FIELD_GET(umac_id, UNIMAC_RDP, HD_BKP_CNTL, HD_FC_BKOFF_OK, reg_hd_bkp_cntl);
    *hd_fc_ena = RU_FIELD_GET(umac_id, UNIMAC_RDP, HD_BKP_CNTL, HD_FC_ENA, reg_hd_bkp_cntl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_cmd_set(uint8_t umac_id, const unimac_rdp_cmd *cmd)
{
    uint32_t reg_cmd=0;

#ifdef VALIDATE_PARMS
    if(!cmd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT) ||
       (cmd->runt_filter_dis >= _1BITS_MAX_VAL_) ||
       (cmd->txrx_en_config >= _1BITS_MAX_VAL_) ||
       (cmd->tx_pause_ignore >= _1BITS_MAX_VAL_) ||
       (cmd->prbl_ena >= _1BITS_MAX_VAL_) ||
       (cmd->rx_err_disc >= _1BITS_MAX_VAL_) ||
       (cmd->rmt_loop_ena >= _1BITS_MAX_VAL_) ||
       (cmd->no_lgth_check >= _1BITS_MAX_VAL_) ||
       (cmd->cntl_frm_ena >= _1BITS_MAX_VAL_) ||
       (cmd->ena_ext_config >= _1BITS_MAX_VAL_) ||
       (cmd->lcl_loop_ena >= _1BITS_MAX_VAL_) ||
       (cmd->sw_reset >= _1BITS_MAX_VAL_) ||
       (cmd->hd_ena >= _1BITS_MAX_VAL_) ||
       (cmd->tx_addr_ins >= _1BITS_MAX_VAL_) ||
       (cmd->rx_pause_ignore >= _1BITS_MAX_VAL_) ||
       (cmd->pause_fwd >= _1BITS_MAX_VAL_) ||
       (cmd->crc_fwd >= _1BITS_MAX_VAL_) ||
       (cmd->pad_en >= _1BITS_MAX_VAL_) ||
       (cmd->promis_en >= _1BITS_MAX_VAL_) ||
       (cmd->eth_speed >= _2BITS_MAX_VAL_) ||
       (cmd->rx_ena >= _1BITS_MAX_VAL_) ||
       (cmd->tx_ena >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_cmd = RU_FIELD_SET(umac_id, UNIMAC_RDP, CMD, RUNT_FILTER_DIS, reg_cmd, cmd->runt_filter_dis);
    reg_cmd = RU_FIELD_SET(umac_id, UNIMAC_RDP, CMD, TXRX_EN_CONFIG, reg_cmd, cmd->txrx_en_config);
    reg_cmd = RU_FIELD_SET(umac_id, UNIMAC_RDP, CMD, TX_PAUSE_IGNORE, reg_cmd, cmd->tx_pause_ignore);
    reg_cmd = RU_FIELD_SET(umac_id, UNIMAC_RDP, CMD, PRBL_ENA, reg_cmd, cmd->prbl_ena);
    reg_cmd = RU_FIELD_SET(umac_id, UNIMAC_RDP, CMD, RX_ERR_DISC, reg_cmd, cmd->rx_err_disc);
    reg_cmd = RU_FIELD_SET(umac_id, UNIMAC_RDP, CMD, RMT_LOOP_ENA, reg_cmd, cmd->rmt_loop_ena);
    reg_cmd = RU_FIELD_SET(umac_id, UNIMAC_RDP, CMD, NO_LGTH_CHECK, reg_cmd, cmd->no_lgth_check);
    reg_cmd = RU_FIELD_SET(umac_id, UNIMAC_RDP, CMD, CNTL_FRM_ENA, reg_cmd, cmd->cntl_frm_ena);
    reg_cmd = RU_FIELD_SET(umac_id, UNIMAC_RDP, CMD, ENA_EXT_CONFIG, reg_cmd, cmd->ena_ext_config);
    reg_cmd = RU_FIELD_SET(umac_id, UNIMAC_RDP, CMD, LCL_LOOP_ENA, reg_cmd, cmd->lcl_loop_ena);
    reg_cmd = RU_FIELD_SET(umac_id, UNIMAC_RDP, CMD, SW_RESET, reg_cmd, cmd->sw_reset);
    reg_cmd = RU_FIELD_SET(umac_id, UNIMAC_RDP, CMD, HD_ENA, reg_cmd, cmd->hd_ena);
    reg_cmd = RU_FIELD_SET(umac_id, UNIMAC_RDP, CMD, TX_ADDR_INS, reg_cmd, cmd->tx_addr_ins);
    reg_cmd = RU_FIELD_SET(umac_id, UNIMAC_RDP, CMD, RX_PAUSE_IGNORE, reg_cmd, cmd->rx_pause_ignore);
    reg_cmd = RU_FIELD_SET(umac_id, UNIMAC_RDP, CMD, PAUSE_FWD, reg_cmd, cmd->pause_fwd);
    reg_cmd = RU_FIELD_SET(umac_id, UNIMAC_RDP, CMD, CRC_FWD, reg_cmd, cmd->crc_fwd);
    reg_cmd = RU_FIELD_SET(umac_id, UNIMAC_RDP, CMD, PAD_EN, reg_cmd, cmd->pad_en);
    reg_cmd = RU_FIELD_SET(umac_id, UNIMAC_RDP, CMD, PROMIS_EN, reg_cmd, cmd->promis_en);
    reg_cmd = RU_FIELD_SET(umac_id, UNIMAC_RDP, CMD, ETH_SPEED, reg_cmd, cmd->eth_speed);
    reg_cmd = RU_FIELD_SET(umac_id, UNIMAC_RDP, CMD, RX_ENA, reg_cmd, cmd->rx_ena);
    reg_cmd = RU_FIELD_SET(umac_id, UNIMAC_RDP, CMD, TX_ENA, reg_cmd, cmd->tx_ena);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, CMD, reg_cmd);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_cmd_get(uint8_t umac_id, unimac_rdp_cmd *cmd)
{
    uint32_t reg_cmd;

#ifdef VALIDATE_PARMS
    if(!cmd)
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

    RU_REG_READ(umac_id, UNIMAC_RDP, CMD, reg_cmd);

    cmd->runt_filter_dis = RU_FIELD_GET(umac_id, UNIMAC_RDP, CMD, RUNT_FILTER_DIS, reg_cmd);
    cmd->txrx_en_config = RU_FIELD_GET(umac_id, UNIMAC_RDP, CMD, TXRX_EN_CONFIG, reg_cmd);
    cmd->tx_pause_ignore = RU_FIELD_GET(umac_id, UNIMAC_RDP, CMD, TX_PAUSE_IGNORE, reg_cmd);
    cmd->prbl_ena = RU_FIELD_GET(umac_id, UNIMAC_RDP, CMD, PRBL_ENA, reg_cmd);
    cmd->rx_err_disc = RU_FIELD_GET(umac_id, UNIMAC_RDP, CMD, RX_ERR_DISC, reg_cmd);
    cmd->rmt_loop_ena = RU_FIELD_GET(umac_id, UNIMAC_RDP, CMD, RMT_LOOP_ENA, reg_cmd);
    cmd->no_lgth_check = RU_FIELD_GET(umac_id, UNIMAC_RDP, CMD, NO_LGTH_CHECK, reg_cmd);
    cmd->cntl_frm_ena = RU_FIELD_GET(umac_id, UNIMAC_RDP, CMD, CNTL_FRM_ENA, reg_cmd);
    cmd->ena_ext_config = RU_FIELD_GET(umac_id, UNIMAC_RDP, CMD, ENA_EXT_CONFIG, reg_cmd);
    cmd->lcl_loop_ena = RU_FIELD_GET(umac_id, UNIMAC_RDP, CMD, LCL_LOOP_ENA, reg_cmd);
    cmd->sw_reset = RU_FIELD_GET(umac_id, UNIMAC_RDP, CMD, SW_RESET, reg_cmd);
    cmd->hd_ena = RU_FIELD_GET(umac_id, UNIMAC_RDP, CMD, HD_ENA, reg_cmd);
    cmd->tx_addr_ins = RU_FIELD_GET(umac_id, UNIMAC_RDP, CMD, TX_ADDR_INS, reg_cmd);
    cmd->rx_pause_ignore = RU_FIELD_GET(umac_id, UNIMAC_RDP, CMD, RX_PAUSE_IGNORE, reg_cmd);
    cmd->pause_fwd = RU_FIELD_GET(umac_id, UNIMAC_RDP, CMD, PAUSE_FWD, reg_cmd);
    cmd->crc_fwd = RU_FIELD_GET(umac_id, UNIMAC_RDP, CMD, CRC_FWD, reg_cmd);
    cmd->pad_en = RU_FIELD_GET(umac_id, UNIMAC_RDP, CMD, PAD_EN, reg_cmd);
    cmd->promis_en = RU_FIELD_GET(umac_id, UNIMAC_RDP, CMD, PROMIS_EN, reg_cmd);
    cmd->eth_speed = RU_FIELD_GET(umac_id, UNIMAC_RDP, CMD, ETH_SPEED, reg_cmd);
    cmd->rx_ena = RU_FIELD_GET(umac_id, UNIMAC_RDP, CMD, RX_ENA, reg_cmd);
    cmd->tx_ena = RU_FIELD_GET(umac_id, UNIMAC_RDP, CMD, TX_ENA, reg_cmd);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_mac0_set(uint8_t umac_id, uint32_t mac_0)
{
    uint32_t reg_mac0=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_mac0 = RU_FIELD_SET(umac_id, UNIMAC_RDP, MAC0, MAC_0, reg_mac0, mac_0);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, MAC0, reg_mac0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_mac0_get(uint8_t umac_id, uint32_t *mac_0)
{
    uint32_t reg_mac0;

#ifdef VALIDATE_PARMS
    if(!mac_0)
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

    RU_REG_READ(umac_id, UNIMAC_RDP, MAC0, reg_mac0);

    *mac_0 = RU_FIELD_GET(umac_id, UNIMAC_RDP, MAC0, MAC_0, reg_mac0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_mac1_set(uint8_t umac_id, uint16_t mac_1)
{
    uint32_t reg_mac1=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_mac1 = RU_FIELD_SET(umac_id, UNIMAC_RDP, MAC1, MAC_1, reg_mac1, mac_1);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, MAC1, reg_mac1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_mac1_get(uint8_t umac_id, uint16_t *mac_1)
{
    uint32_t reg_mac1;

#ifdef VALIDATE_PARMS
    if(!mac_1)
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

    RU_REG_READ(umac_id, UNIMAC_RDP, MAC1, reg_mac1);

    *mac_1 = RU_FIELD_GET(umac_id, UNIMAC_RDP, MAC1, MAC_1, reg_mac1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_frm_len_set(uint8_t umac_id, uint16_t frame_length)
{
    uint32_t reg_frm_len=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT) ||
       (frame_length >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_frm_len = RU_FIELD_SET(umac_id, UNIMAC_RDP, FRM_LEN, FRAME_LENGTH, reg_frm_len, frame_length);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, FRM_LEN, reg_frm_len);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_frm_len_get(uint8_t umac_id, uint16_t *frame_length)
{
    uint32_t reg_frm_len;

#ifdef VALIDATE_PARMS
    if(!frame_length)
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

    RU_REG_READ(umac_id, UNIMAC_RDP, FRM_LEN, reg_frm_len);

    *frame_length = RU_FIELD_GET(umac_id, UNIMAC_RDP, FRM_LEN, FRAME_LENGTH, reg_frm_len);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_pause_qunat_set(uint8_t umac_id, uint16_t pause_quant)
{
    uint32_t reg_pause_qunat=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pause_qunat = RU_FIELD_SET(umac_id, UNIMAC_RDP, PAUSE_QUNAT, PAUSE_QUANT, reg_pause_qunat, pause_quant);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, PAUSE_QUNAT, reg_pause_qunat);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_pause_qunat_get(uint8_t umac_id, uint16_t *pause_quant)
{
    uint32_t reg_pause_qunat;

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

    RU_REG_READ(umac_id, UNIMAC_RDP, PAUSE_QUNAT, reg_pause_qunat);

    *pause_quant = RU_FIELD_GET(umac_id, UNIMAC_RDP, PAUSE_QUNAT, PAUSE_QUANT, reg_pause_qunat);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_sfd_offset_get(uint8_t umac_id, uint32_t *temp)
{
    uint32_t reg_sfd_offset;

#ifdef VALIDATE_PARMS
    if(!temp)
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

    *temp = RU_FIELD_GET(umac_id, UNIMAC_RDP, SFD_OFFSET, TEMP, reg_sfd_offset);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_mode_get(uint8_t umac_id, unimac_rdp_mode *mode)
{
    uint32_t reg_mode;

#ifdef VALIDATE_PARMS
    if(!mode)
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

    RU_REG_READ(umac_id, UNIMAC_RDP, MODE, reg_mode);

    mode->mac_link_stat = RU_FIELD_GET(umac_id, UNIMAC_RDP, MODE, MAC_LINK_STAT, reg_mode);
    mode->mac_tx_pause = RU_FIELD_GET(umac_id, UNIMAC_RDP, MODE, MAC_TX_PAUSE, reg_mode);
    mode->mac_rx_pause = RU_FIELD_GET(umac_id, UNIMAC_RDP, MODE, MAC_RX_PAUSE, reg_mode);
    mode->mac_duplex = RU_FIELD_GET(umac_id, UNIMAC_RDP, MODE, MAC_DUPLEX, reg_mode);
    mode->mac_speed = RU_FIELD_GET(umac_id, UNIMAC_RDP, MODE, MAC_SPEED, reg_mode);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_frm_tag0_set(uint8_t umac_id, uint16_t outer_tag)
{
    uint32_t reg_frm_tag0=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_frm_tag0 = RU_FIELD_SET(umac_id, UNIMAC_RDP, FRM_TAG0, OUTER_TAG, reg_frm_tag0, outer_tag);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, FRM_TAG0, reg_frm_tag0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_frm_tag0_get(uint8_t umac_id, uint16_t *outer_tag)
{
    uint32_t reg_frm_tag0;

#ifdef VALIDATE_PARMS
    if(!outer_tag)
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

    RU_REG_READ(umac_id, UNIMAC_RDP, FRM_TAG0, reg_frm_tag0);

    *outer_tag = RU_FIELD_GET(umac_id, UNIMAC_RDP, FRM_TAG0, OUTER_TAG, reg_frm_tag0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_frm_tag1_set(uint8_t umac_id, uint16_t inner_tag)
{
    uint32_t reg_frm_tag1=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_frm_tag1 = RU_FIELD_SET(umac_id, UNIMAC_RDP, FRM_TAG1, INNER_TAG, reg_frm_tag1, inner_tag);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, FRM_TAG1, reg_frm_tag1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_frm_tag1_get(uint8_t umac_id, uint16_t *inner_tag)
{
    uint32_t reg_frm_tag1;

#ifdef VALIDATE_PARMS
    if(!inner_tag)
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

    RU_REG_READ(umac_id, UNIMAC_RDP, FRM_TAG1, reg_frm_tag1);

    *inner_tag = RU_FIELD_GET(umac_id, UNIMAC_RDP, FRM_TAG1, INNER_TAG, reg_frm_tag1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_tx_ipg_len_set(uint8_t umac_id, uint8_t tx_ipg_len)
{
    uint32_t reg_tx_ipg_len=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT) ||
       (tx_ipg_len >= _7BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_tx_ipg_len = RU_FIELD_SET(umac_id, UNIMAC_RDP, TX_IPG_LEN, TX_IPG_LEN, reg_tx_ipg_len, tx_ipg_len);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, TX_IPG_LEN, reg_tx_ipg_len);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_tx_ipg_len_get(uint8_t umac_id, uint8_t *tx_ipg_len)
{
    uint32_t reg_tx_ipg_len;

#ifdef VALIDATE_PARMS
    if(!tx_ipg_len)
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

    RU_REG_READ(umac_id, UNIMAC_RDP, TX_IPG_LEN, reg_tx_ipg_len);

    *tx_ipg_len = RU_FIELD_GET(umac_id, UNIMAC_RDP, TX_IPG_LEN, TX_IPG_LEN, reg_tx_ipg_len);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_eee_ctrl_set(uint8_t umac_id, const unimac_rdp_eee_ctrl *eee_ctrl)
{
    uint32_t reg_eee_ctrl=0;

#ifdef VALIDATE_PARMS
    if(!eee_ctrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT) ||
       (eee_ctrl->lp_idle_prediction_mode >= _1BITS_MAX_VAL_) ||
       (eee_ctrl->dis_eee_10m >= _1BITS_MAX_VAL_) ||
       (eee_ctrl->eee_txclk_dis >= _1BITS_MAX_VAL_) ||
       (eee_ctrl->rx_fifo_check >= _1BITS_MAX_VAL_) ||
       (eee_ctrl->eee_en >= _1BITS_MAX_VAL_) ||
       (eee_ctrl->en_lpi_tx_pause >= _1BITS_MAX_VAL_) ||
       (eee_ctrl->en_lpi_tx_pfc >= _1BITS_MAX_VAL_) ||
       (eee_ctrl->en_lpi_rx_pause >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_eee_ctrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, EEE_CTRL, LP_IDLE_PREDICTION_MODE, reg_eee_ctrl, eee_ctrl->lp_idle_prediction_mode);
    reg_eee_ctrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, EEE_CTRL, DIS_EEE_10M, reg_eee_ctrl, eee_ctrl->dis_eee_10m);
    reg_eee_ctrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, EEE_CTRL, EEE_TXCLK_DIS, reg_eee_ctrl, eee_ctrl->eee_txclk_dis);
    reg_eee_ctrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, EEE_CTRL, RX_FIFO_CHECK, reg_eee_ctrl, eee_ctrl->rx_fifo_check);
    reg_eee_ctrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, EEE_CTRL, EEE_EN, reg_eee_ctrl, eee_ctrl->eee_en);
    reg_eee_ctrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, EEE_CTRL, EN_LPI_TX_PAUSE, reg_eee_ctrl, eee_ctrl->en_lpi_tx_pause);
    reg_eee_ctrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, EEE_CTRL, EN_LPI_TX_PFC, reg_eee_ctrl, eee_ctrl->en_lpi_tx_pfc);
    reg_eee_ctrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, EEE_CTRL, EN_LPI_RX_PAUSE, reg_eee_ctrl, eee_ctrl->en_lpi_rx_pause);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, EEE_CTRL, reg_eee_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_eee_ctrl_get(uint8_t umac_id, unimac_rdp_eee_ctrl *eee_ctrl)
{
    uint32_t reg_eee_ctrl;

#ifdef VALIDATE_PARMS
    if(!eee_ctrl)
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

    RU_REG_READ(umac_id, UNIMAC_RDP, EEE_CTRL, reg_eee_ctrl);

    eee_ctrl->lp_idle_prediction_mode = RU_FIELD_GET(umac_id, UNIMAC_RDP, EEE_CTRL, LP_IDLE_PREDICTION_MODE, reg_eee_ctrl);
    eee_ctrl->dis_eee_10m = RU_FIELD_GET(umac_id, UNIMAC_RDP, EEE_CTRL, DIS_EEE_10M, reg_eee_ctrl);
    eee_ctrl->eee_txclk_dis = RU_FIELD_GET(umac_id, UNIMAC_RDP, EEE_CTRL, EEE_TXCLK_DIS, reg_eee_ctrl);
    eee_ctrl->rx_fifo_check = RU_FIELD_GET(umac_id, UNIMAC_RDP, EEE_CTRL, RX_FIFO_CHECK, reg_eee_ctrl);
    eee_ctrl->eee_en = RU_FIELD_GET(umac_id, UNIMAC_RDP, EEE_CTRL, EEE_EN, reg_eee_ctrl);
    eee_ctrl->en_lpi_tx_pause = RU_FIELD_GET(umac_id, UNIMAC_RDP, EEE_CTRL, EN_LPI_TX_PAUSE, reg_eee_ctrl);
    eee_ctrl->en_lpi_tx_pfc = RU_FIELD_GET(umac_id, UNIMAC_RDP, EEE_CTRL, EN_LPI_TX_PFC, reg_eee_ctrl);
    eee_ctrl->en_lpi_rx_pause = RU_FIELD_GET(umac_id, UNIMAC_RDP, EEE_CTRL, EN_LPI_RX_PAUSE, reg_eee_ctrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_eee_lpi_timer_set(uint8_t umac_id, uint32_t eee_lpi_timer)
{
    uint32_t reg_eee_lpi_timer=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_eee_lpi_timer = RU_FIELD_SET(umac_id, UNIMAC_RDP, EEE_LPI_TIMER, EEE_LPI_TIMER, reg_eee_lpi_timer, eee_lpi_timer);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, EEE_LPI_TIMER, reg_eee_lpi_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_eee_lpi_timer_get(uint8_t umac_id, uint32_t *eee_lpi_timer)
{
    uint32_t reg_eee_lpi_timer;

#ifdef VALIDATE_PARMS
    if(!eee_lpi_timer)
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

    RU_REG_READ(umac_id, UNIMAC_RDP, EEE_LPI_TIMER, reg_eee_lpi_timer);

    *eee_lpi_timer = RU_FIELD_GET(umac_id, UNIMAC_RDP, EEE_LPI_TIMER, EEE_LPI_TIMER, reg_eee_lpi_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_eee_wake_timer_set(uint8_t umac_id, uint16_t eee_wake_timer)
{
    uint32_t reg_eee_wake_timer=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_eee_wake_timer = RU_FIELD_SET(umac_id, UNIMAC_RDP, EEE_WAKE_TIMER, EEE_WAKE_TIMER, reg_eee_wake_timer, eee_wake_timer);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, EEE_WAKE_TIMER, reg_eee_wake_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_eee_wake_timer_get(uint8_t umac_id, uint16_t *eee_wake_timer)
{
    uint32_t reg_eee_wake_timer;

#ifdef VALIDATE_PARMS
    if(!eee_wake_timer)
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

    RU_REG_READ(umac_id, UNIMAC_RDP, EEE_WAKE_TIMER, reg_eee_wake_timer);

    *eee_wake_timer = RU_FIELD_GET(umac_id, UNIMAC_RDP, EEE_WAKE_TIMER, EEE_WAKE_TIMER, reg_eee_wake_timer);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_eee_ref_count_set(uint8_t umac_id, uint16_t eee_reference_count)
{
    uint32_t reg_eee_ref_count=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_eee_ref_count = RU_FIELD_SET(umac_id, UNIMAC_RDP, EEE_REF_COUNT, EEE_REFERENCE_COUNT, reg_eee_ref_count, eee_reference_count);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, EEE_REF_COUNT, reg_eee_ref_count);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_eee_ref_count_get(uint8_t umac_id, uint16_t *eee_reference_count)
{
    uint32_t reg_eee_ref_count;

#ifdef VALIDATE_PARMS
    if(!eee_reference_count)
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

    RU_REG_READ(umac_id, UNIMAC_RDP, EEE_REF_COUNT, reg_eee_ref_count);

    *eee_reference_count = RU_FIELD_GET(umac_id, UNIMAC_RDP, EEE_REF_COUNT, EEE_REFERENCE_COUNT, reg_eee_ref_count);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_rx_pkt_drop_status_set(uint8_t umac_id, bdmf_boolean rx_ipg_invalid)
{
    uint32_t reg_rx_pkt_drop_status=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT) ||
       (rx_ipg_invalid >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_pkt_drop_status = RU_FIELD_SET(umac_id, UNIMAC_RDP, RX_PKT_DROP_STATUS, RX_IPG_INVALID, reg_rx_pkt_drop_status, rx_ipg_invalid);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, RX_PKT_DROP_STATUS, reg_rx_pkt_drop_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_rx_pkt_drop_status_get(uint8_t umac_id, bdmf_boolean *rx_ipg_invalid)
{
    uint32_t reg_rx_pkt_drop_status;

#ifdef VALIDATE_PARMS
    if(!rx_ipg_invalid)
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

    RU_REG_READ(umac_id, UNIMAC_RDP, RX_PKT_DROP_STATUS, reg_rx_pkt_drop_status);

    *rx_ipg_invalid = RU_FIELD_GET(umac_id, UNIMAC_RDP, RX_PKT_DROP_STATUS, RX_IPG_INVALID, reg_rx_pkt_drop_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_symmetric_idle_threshold_set(uint8_t umac_id, uint16_t threshold_value)
{
    uint32_t reg_symmetric_idle_threshold=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_symmetric_idle_threshold = RU_FIELD_SET(umac_id, UNIMAC_RDP, SYMMETRIC_IDLE_THRESHOLD, THRESHOLD_VALUE, reg_symmetric_idle_threshold, threshold_value);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, SYMMETRIC_IDLE_THRESHOLD, reg_symmetric_idle_threshold);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_symmetric_idle_threshold_get(uint8_t umac_id, uint16_t *threshold_value)
{
    uint32_t reg_symmetric_idle_threshold;

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

    RU_REG_READ(umac_id, UNIMAC_RDP, SYMMETRIC_IDLE_THRESHOLD, reg_symmetric_idle_threshold);

    *threshold_value = RU_FIELD_GET(umac_id, UNIMAC_RDP, SYMMETRIC_IDLE_THRESHOLD, THRESHOLD_VALUE, reg_symmetric_idle_threshold);

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

bdmf_error_t ag_drv_unimac_rdp_macsec_cntrl_set(uint8_t umac_id, bdmf_boolean tx_crc_program, bdmf_boolean tx_crc_corrupt_en, bdmf_boolean tx_lanuch_en)
{
    uint32_t reg_macsec_cntrl=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT) ||
       (tx_crc_program >= _1BITS_MAX_VAL_) ||
       (tx_crc_corrupt_en >= _1BITS_MAX_VAL_) ||
       (tx_lanuch_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_macsec_cntrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, MACSEC_CNTRL, TX_CRC_PROGRAM, reg_macsec_cntrl, tx_crc_program);
    reg_macsec_cntrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, MACSEC_CNTRL, TX_CRC_CORRUPT_EN, reg_macsec_cntrl, tx_crc_corrupt_en);
    reg_macsec_cntrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, MACSEC_CNTRL, TX_LANUCH_EN, reg_macsec_cntrl, tx_lanuch_en);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, MACSEC_CNTRL, reg_macsec_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_macsec_cntrl_get(uint8_t umac_id, bdmf_boolean *tx_crc_program, bdmf_boolean *tx_crc_corrupt_en, bdmf_boolean *tx_lanuch_en)
{
    uint32_t reg_macsec_cntrl;

#ifdef VALIDATE_PARMS
    if(!tx_crc_program || !tx_crc_corrupt_en || !tx_lanuch_en)
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

    *tx_crc_program = RU_FIELD_GET(umac_id, UNIMAC_RDP, MACSEC_CNTRL, TX_CRC_PROGRAM, reg_macsec_cntrl);
    *tx_crc_corrupt_en = RU_FIELD_GET(umac_id, UNIMAC_RDP, MACSEC_CNTRL, TX_CRC_CORRUPT_EN, reg_macsec_cntrl);
    *tx_lanuch_en = RU_FIELD_GET(umac_id, UNIMAC_RDP, MACSEC_CNTRL, TX_LANUCH_EN, reg_macsec_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_ts_status_cntrl_get(uint8_t umac_id, uint8_t *word_avail, bdmf_boolean *tx_ts_fifo_empty, bdmf_boolean *tx_ts_fifo_full)
{
    uint32_t reg_ts_status_cntrl;

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

    RU_REG_READ(umac_id, UNIMAC_RDP, TS_STATUS_CNTRL, reg_ts_status_cntrl);

    *word_avail = RU_FIELD_GET(umac_id, UNIMAC_RDP, TS_STATUS_CNTRL, WORD_AVAIL, reg_ts_status_cntrl);
    *tx_ts_fifo_empty = RU_FIELD_GET(umac_id, UNIMAC_RDP, TS_STATUS_CNTRL, TX_TS_FIFO_EMPTY, reg_ts_status_cntrl);
    *tx_ts_fifo_full = RU_FIELD_GET(umac_id, UNIMAC_RDP, TS_STATUS_CNTRL, TX_TS_FIFO_FULL, reg_ts_status_cntrl);

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

bdmf_error_t ag_drv_unimac_rdp_pause_cntrl_set(uint8_t umac_id, bdmf_boolean pause_control_en, uint32_t pause_timer)
{
    uint32_t reg_pause_cntrl=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT) ||
       (pause_control_en >= _1BITS_MAX_VAL_) ||
       (pause_timer >= _17BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_pause_cntrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, PAUSE_CNTRL, PAUSE_CONTROL_EN, reg_pause_cntrl, pause_control_en);
    reg_pause_cntrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, PAUSE_CNTRL, PAUSE_TIMER, reg_pause_cntrl, pause_timer);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, PAUSE_CNTRL, reg_pause_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_pause_cntrl_get(uint8_t umac_id, bdmf_boolean *pause_control_en, uint32_t *pause_timer)
{
    uint32_t reg_pause_cntrl;

#ifdef VALIDATE_PARMS
    if(!pause_control_en || !pause_timer)
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

    RU_REG_READ(umac_id, UNIMAC_RDP, PAUSE_CNTRL, reg_pause_cntrl);

    *pause_control_en = RU_FIELD_GET(umac_id, UNIMAC_RDP, PAUSE_CNTRL, PAUSE_CONTROL_EN, reg_pause_cntrl);
    *pause_timer = RU_FIELD_GET(umac_id, UNIMAC_RDP, PAUSE_CNTRL, PAUSE_TIMER, reg_pause_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_txfifo_flush_set(uint8_t umac_id, bdmf_boolean tx_flush)
{
    uint32_t reg_txfifo_flush=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT) ||
       (tx_flush >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_txfifo_flush = RU_FIELD_SET(umac_id, UNIMAC_RDP, TXFIFO_FLUSH, TX_FLUSH, reg_txfifo_flush, tx_flush);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, TXFIFO_FLUSH, reg_txfifo_flush);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_txfifo_flush_get(uint8_t umac_id, bdmf_boolean *tx_flush)
{
    uint32_t reg_txfifo_flush;

#ifdef VALIDATE_PARMS
    if(!tx_flush)
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

    RU_REG_READ(umac_id, UNIMAC_RDP, TXFIFO_FLUSH, reg_txfifo_flush);

    *tx_flush = RU_FIELD_GET(umac_id, UNIMAC_RDP, TXFIFO_FLUSH, TX_FLUSH, reg_txfifo_flush);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_rxfifo_stat_get(uint8_t umac_id, uint8_t *rxfifo_status)
{
    uint32_t reg_rxfifo_stat;

#ifdef VALIDATE_PARMS
    if(!rxfifo_status)
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

    *rxfifo_status = RU_FIELD_GET(umac_id, UNIMAC_RDP, RXFIFO_STAT, RXFIFO_STATUS, reg_rxfifo_stat);

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

bdmf_error_t ag_drv_unimac_rdp_ppp_cntrl_set(uint8_t umac_id, const unimac_rdp_ppp_cntrl *ppp_cntrl)
{
    uint32_t reg_ppp_cntrl=0;

#ifdef VALIDATE_PARMS
    if(!ppp_cntrl)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT) ||
       (ppp_cntrl->pfc_stats_en >= _1BITS_MAX_VAL_) ||
       (ppp_cntrl->rx_pass_pfc_frm >= _1BITS_MAX_VAL_) ||
       (ppp_cntrl->force_ppp_xon >= _1BITS_MAX_VAL_) ||
       (ppp_cntrl->ppp_en_rx >= _1BITS_MAX_VAL_) ||
       (ppp_cntrl->ppp_en_tx >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_ppp_cntrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, PPP_CNTRL, PFC_STATS_EN, reg_ppp_cntrl, ppp_cntrl->pfc_stats_en);
    reg_ppp_cntrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, PPP_CNTRL, RX_PASS_PFC_FRM, reg_ppp_cntrl, ppp_cntrl->rx_pass_pfc_frm);
    reg_ppp_cntrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, PPP_CNTRL, FORCE_PPP_XON, reg_ppp_cntrl, ppp_cntrl->force_ppp_xon);
    reg_ppp_cntrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, PPP_CNTRL, PPP_EN_RX, reg_ppp_cntrl, ppp_cntrl->ppp_en_rx);
    reg_ppp_cntrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, PPP_CNTRL, PPP_EN_TX, reg_ppp_cntrl, ppp_cntrl->ppp_en_tx);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, PPP_CNTRL, reg_ppp_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_ppp_cntrl_get(uint8_t umac_id, unimac_rdp_ppp_cntrl *ppp_cntrl)
{
    uint32_t reg_ppp_cntrl;

#ifdef VALIDATE_PARMS
    if(!ppp_cntrl)
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

    RU_REG_READ(umac_id, UNIMAC_RDP, PPP_CNTRL, reg_ppp_cntrl);

    ppp_cntrl->pfc_stats_en = RU_FIELD_GET(umac_id, UNIMAC_RDP, PPP_CNTRL, PFC_STATS_EN, reg_ppp_cntrl);
    ppp_cntrl->rx_pass_pfc_frm = RU_FIELD_GET(umac_id, UNIMAC_RDP, PPP_CNTRL, RX_PASS_PFC_FRM, reg_ppp_cntrl);
    ppp_cntrl->force_ppp_xon = RU_FIELD_GET(umac_id, UNIMAC_RDP, PPP_CNTRL, FORCE_PPP_XON, reg_ppp_cntrl);
    ppp_cntrl->ppp_en_rx = RU_FIELD_GET(umac_id, UNIMAC_RDP, PPP_CNTRL, PPP_EN_RX, reg_ppp_cntrl);
    ppp_cntrl->ppp_en_tx = RU_FIELD_GET(umac_id, UNIMAC_RDP, PPP_CNTRL, PPP_EN_TX, reg_ppp_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_ppp_refresh_cntrl_set(uint8_t umac_id, uint16_t ppp_refresh_timer, bdmf_boolean ppp_refresh_en)
{
    uint32_t reg_ppp_refresh_cntrl=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT) ||
       (ppp_refresh_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_ppp_refresh_cntrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, PPP_REFRESH_CNTRL, PPP_REFRESH_TIMER, reg_ppp_refresh_cntrl, ppp_refresh_timer);
    reg_ppp_refresh_cntrl = RU_FIELD_SET(umac_id, UNIMAC_RDP, PPP_REFRESH_CNTRL, PPP_REFRESH_EN, reg_ppp_refresh_cntrl, ppp_refresh_en);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, PPP_REFRESH_CNTRL, reg_ppp_refresh_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_ppp_refresh_cntrl_get(uint8_t umac_id, uint16_t *ppp_refresh_timer, bdmf_boolean *ppp_refresh_en)
{
    uint32_t reg_ppp_refresh_cntrl;

#ifdef VALIDATE_PARMS
    if(!ppp_refresh_timer || !ppp_refresh_en)
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

    RU_REG_READ(umac_id, UNIMAC_RDP, PPP_REFRESH_CNTRL, reg_ppp_refresh_cntrl);

    *ppp_refresh_timer = RU_FIELD_GET(umac_id, UNIMAC_RDP, PPP_REFRESH_CNTRL, PPP_REFRESH_TIMER, reg_ppp_refresh_cntrl);
    *ppp_refresh_en = RU_FIELD_GET(umac_id, UNIMAC_RDP, PPP_REFRESH_CNTRL, PPP_REFRESH_EN, reg_ppp_refresh_cntrl);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_tx_pause_prel0_get(uint8_t umac_id, uint32_t *tx_pause_prb0)
{
    uint32_t reg_tx_pause_prel0;

#ifdef VALIDATE_PARMS
    if(!tx_pause_prb0)
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

    RU_REG_READ(umac_id, UNIMAC_RDP, TX_PAUSE_PREL0, reg_tx_pause_prel0);

    *tx_pause_prb0 = RU_FIELD_GET(umac_id, UNIMAC_RDP, TX_PAUSE_PREL0, TX_PAUSE_PRB0, reg_tx_pause_prel0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_tx_pause_prel1_get(uint8_t umac_id, uint32_t *tx_pause_prb1)
{
    uint32_t reg_tx_pause_prel1;

#ifdef VALIDATE_PARMS
    if(!tx_pause_prb1)
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

    RU_REG_READ(umac_id, UNIMAC_RDP, TX_PAUSE_PREL1, reg_tx_pause_prel1);

    *tx_pause_prb1 = RU_FIELD_GET(umac_id, UNIMAC_RDP, TX_PAUSE_PREL1, TX_PAUSE_PRB1, reg_tx_pause_prel1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_tx_pause_prel2_get(uint8_t umac_id, uint32_t *tx_pause_prb2)
{
    uint32_t reg_tx_pause_prel2;

#ifdef VALIDATE_PARMS
    if(!tx_pause_prb2)
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

    RU_REG_READ(umac_id, UNIMAC_RDP, TX_PAUSE_PREL2, reg_tx_pause_prel2);

    *tx_pause_prb2 = RU_FIELD_GET(umac_id, UNIMAC_RDP, TX_PAUSE_PREL2, TX_PAUSE_PRB2, reg_tx_pause_prel2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_tx_pause_prel3_get(uint8_t umac_id, uint32_t *tx_pause_prb3)
{
    uint32_t reg_tx_pause_prel3;

#ifdef VALIDATE_PARMS
    if(!tx_pause_prb3)
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

    RU_REG_READ(umac_id, UNIMAC_RDP, TX_PAUSE_PREL3, reg_tx_pause_prel3);

    *tx_pause_prb3 = RU_FIELD_GET(umac_id, UNIMAC_RDP, TX_PAUSE_PREL3, TX_PAUSE_PRB3, reg_tx_pause_prel3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_rx_pause_prel0_get(uint8_t umac_id, uint32_t *rx_pause_prb0)
{
    uint32_t reg_rx_pause_prel0;

#ifdef VALIDATE_PARMS
    if(!rx_pause_prb0)
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

    RU_REG_READ(umac_id, UNIMAC_RDP, RX_PAUSE_PREL0, reg_rx_pause_prel0);

    *rx_pause_prb0 = RU_FIELD_GET(umac_id, UNIMAC_RDP, RX_PAUSE_PREL0, RX_PAUSE_PRB0, reg_rx_pause_prel0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_rx_pause_prel1_get(uint8_t umac_id, uint32_t *rx_pause_prb1)
{
    uint32_t reg_rx_pause_prel1;

#ifdef VALIDATE_PARMS
    if(!rx_pause_prb1)
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

    RU_REG_READ(umac_id, UNIMAC_RDP, RX_PAUSE_PREL1, reg_rx_pause_prel1);

    *rx_pause_prb1 = RU_FIELD_GET(umac_id, UNIMAC_RDP, RX_PAUSE_PREL1, RX_PAUSE_PRB1, reg_rx_pause_prel1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_rx_pause_prel2_get(uint8_t umac_id, uint32_t *rx_pause_prb2)
{
    uint32_t reg_rx_pause_prel2;

#ifdef VALIDATE_PARMS
    if(!rx_pause_prb2)
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

    RU_REG_READ(umac_id, UNIMAC_RDP, RX_PAUSE_PREL2, reg_rx_pause_prel2);

    *rx_pause_prb2 = RU_FIELD_GET(umac_id, UNIMAC_RDP, RX_PAUSE_PREL2, RX_PAUSE_PRB2, reg_rx_pause_prel2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_rx_pause_prel3_get(uint8_t umac_id, uint32_t *rx_pause_prb3)
{
    uint32_t reg_rx_pause_prel3;

#ifdef VALIDATE_PARMS
    if(!rx_pause_prb3)
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

    RU_REG_READ(umac_id, UNIMAC_RDP, RX_PAUSE_PREL3, reg_rx_pause_prel3);

    *rx_pause_prb3 = RU_FIELD_GET(umac_id, UNIMAC_RDP, RX_PAUSE_PREL3, RX_PAUSE_PRB3, reg_rx_pause_prel3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_rxerr_mask_set(uint8_t umac_id, uint32_t mac_rxerr_mask)
{
    uint32_t reg_rxerr_mask=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT) ||
       (mac_rxerr_mask >= _18BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rxerr_mask = RU_FIELD_SET(umac_id, UNIMAC_RDP, RXERR_MASK, MAC_RXERR_MASK, reg_rxerr_mask, mac_rxerr_mask);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, RXERR_MASK, reg_rxerr_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_rxerr_mask_get(uint8_t umac_id, uint32_t *mac_rxerr_mask)
{
    uint32_t reg_rxerr_mask;

#ifdef VALIDATE_PARMS
    if(!mac_rxerr_mask)
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

    RU_REG_READ(umac_id, UNIMAC_RDP, RXERR_MASK, reg_rxerr_mask);

    *mac_rxerr_mask = RU_FIELD_GET(umac_id, UNIMAC_RDP, RXERR_MASK, MAC_RXERR_MASK, reg_rxerr_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_rx_max_pkt_size_set(uint8_t umac_id, uint16_t max_pkt_size)
{
    uint32_t reg_rx_max_pkt_size=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT) ||
       (max_pkt_size >= _14BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_rx_max_pkt_size = RU_FIELD_SET(umac_id, UNIMAC_RDP, RX_MAX_PKT_SIZE, MAX_PKT_SIZE, reg_rx_max_pkt_size, max_pkt_size);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, RX_MAX_PKT_SIZE, reg_rx_max_pkt_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_rx_max_pkt_size_get(uint8_t umac_id, uint16_t *max_pkt_size)
{
    uint32_t reg_rx_max_pkt_size;

#ifdef VALIDATE_PARMS
    if(!max_pkt_size)
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

    RU_REG_READ(umac_id, UNIMAC_RDP, RX_MAX_PKT_SIZE, reg_rx_max_pkt_size);

    *max_pkt_size = RU_FIELD_GET(umac_id, UNIMAC_RDP, RX_MAX_PKT_SIZE, MAX_PKT_SIZE, reg_rx_max_pkt_size);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_mdio_cmd_set(uint8_t umac_id, const unimac_rdp_mdio_cmd *mdio_cmd)
{
    uint32_t reg_mdio_cmd=0;

#ifdef VALIDATE_PARMS
    if(!mdio_cmd)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_id >= BLOCK_ADDR_COUNT) ||
       (mdio_cmd->mdio_busy >= _1BITS_MAX_VAL_) ||
       (mdio_cmd->fail >= _1BITS_MAX_VAL_) ||
       (mdio_cmd->op_code >= _2BITS_MAX_VAL_) ||
       (mdio_cmd->phy_prt_addr >= _5BITS_MAX_VAL_) ||
       (mdio_cmd->reg_dec_addr >= _5BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_mdio_cmd = RU_FIELD_SET(umac_id, UNIMAC_RDP, MDIO_CMD, MDIO_BUSY, reg_mdio_cmd, mdio_cmd->mdio_busy);
    reg_mdio_cmd = RU_FIELD_SET(umac_id, UNIMAC_RDP, MDIO_CMD, FAIL, reg_mdio_cmd, mdio_cmd->fail);
    reg_mdio_cmd = RU_FIELD_SET(umac_id, UNIMAC_RDP, MDIO_CMD, OP_CODE, reg_mdio_cmd, mdio_cmd->op_code);
    reg_mdio_cmd = RU_FIELD_SET(umac_id, UNIMAC_RDP, MDIO_CMD, PHY_PRT_ADDR, reg_mdio_cmd, mdio_cmd->phy_prt_addr);
    reg_mdio_cmd = RU_FIELD_SET(umac_id, UNIMAC_RDP, MDIO_CMD, REG_DEC_ADDR, reg_mdio_cmd, mdio_cmd->reg_dec_addr);
    reg_mdio_cmd = RU_FIELD_SET(umac_id, UNIMAC_RDP, MDIO_CMD, DATA_ADDR, reg_mdio_cmd, mdio_cmd->data_addr);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, MDIO_CMD, reg_mdio_cmd);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_mdio_cmd_get(uint8_t umac_id, unimac_rdp_mdio_cmd *mdio_cmd)
{
    uint32_t reg_mdio_cmd;

#ifdef VALIDATE_PARMS
    if(!mdio_cmd)
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

    RU_REG_READ(umac_id, UNIMAC_RDP, MDIO_CMD, reg_mdio_cmd);

    mdio_cmd->mdio_busy = RU_FIELD_GET(umac_id, UNIMAC_RDP, MDIO_CMD, MDIO_BUSY, reg_mdio_cmd);
    mdio_cmd->fail = RU_FIELD_GET(umac_id, UNIMAC_RDP, MDIO_CMD, FAIL, reg_mdio_cmd);
    mdio_cmd->op_code = RU_FIELD_GET(umac_id, UNIMAC_RDP, MDIO_CMD, OP_CODE, reg_mdio_cmd);
    mdio_cmd->phy_prt_addr = RU_FIELD_GET(umac_id, UNIMAC_RDP, MDIO_CMD, PHY_PRT_ADDR, reg_mdio_cmd);
    mdio_cmd->reg_dec_addr = RU_FIELD_GET(umac_id, UNIMAC_RDP, MDIO_CMD, REG_DEC_ADDR, reg_mdio_cmd);
    mdio_cmd->data_addr = RU_FIELD_GET(umac_id, UNIMAC_RDP, MDIO_CMD, DATA_ADDR, reg_mdio_cmd);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_mdio_cfg_set(uint8_t umac_id, uint8_t mdio_clk_divider, bdmf_boolean mdio_clause)
{
    uint32_t reg_mdio_cfg=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT) ||
       (mdio_clk_divider >= _6BITS_MAX_VAL_) ||
       (mdio_clause >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_mdio_cfg = RU_FIELD_SET(umac_id, UNIMAC_RDP, MDIO_CFG, MDIO_CLK_DIVIDER, reg_mdio_cfg, mdio_clk_divider);
    reg_mdio_cfg = RU_FIELD_SET(umac_id, UNIMAC_RDP, MDIO_CFG, MDIO_CLAUSE, reg_mdio_cfg, mdio_clause);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, MDIO_CFG, reg_mdio_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_mdio_cfg_get(uint8_t umac_id, uint8_t *mdio_clk_divider, bdmf_boolean *mdio_clause)
{
    uint32_t reg_mdio_cfg;

#ifdef VALIDATE_PARMS
    if(!mdio_clk_divider || !mdio_clause)
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

    RU_REG_READ(umac_id, UNIMAC_RDP, MDIO_CFG, reg_mdio_cfg);

    *mdio_clk_divider = RU_FIELD_GET(umac_id, UNIMAC_RDP, MDIO_CFG, MDIO_CLK_DIVIDER, reg_mdio_cfg);
    *mdio_clause = RU_FIELD_GET(umac_id, UNIMAC_RDP, MDIO_CFG, MDIO_CLAUSE, reg_mdio_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_mdf_cnt_get(uint8_t umac_id, uint32_t *mdf_packet_counter)
{
    uint32_t reg_mdf_cnt;

#ifdef VALIDATE_PARMS
    if(!mdf_packet_counter)
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

    RU_REG_READ(umac_id, UNIMAC_RDP, MDF_CNT, reg_mdf_cnt);

    *mdf_packet_counter = RU_FIELD_GET(umac_id, UNIMAC_RDP, MDF_CNT, MDF_PACKET_COUNTER, reg_mdf_cnt);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_diag_sel_set(uint8_t umac_id, uint8_t diag_hi_select, uint8_t diag_lo_select)
{
    uint32_t reg_diag_sel=0;

#ifdef VALIDATE_PARMS
    if((umac_id >= BLOCK_ADDR_COUNT) ||
       (diag_hi_select >= _6BITS_MAX_VAL_) ||
       (diag_lo_select >= _6BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_diag_sel = RU_FIELD_SET(umac_id, UNIMAC_RDP, DIAG_SEL, DIAG_HI_SELECT, reg_diag_sel, diag_hi_select);
    reg_diag_sel = RU_FIELD_SET(umac_id, UNIMAC_RDP, DIAG_SEL, DIAG_LO_SELECT, reg_diag_sel, diag_lo_select);

    RU_REG_WRITE(umac_id, UNIMAC_RDP, DIAG_SEL, reg_diag_sel);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_rdp_diag_sel_get(uint8_t umac_id, uint8_t *diag_hi_select, uint8_t *diag_lo_select)
{
    uint32_t reg_diag_sel;

#ifdef VALIDATE_PARMS
    if(!diag_hi_select || !diag_lo_select)
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

    RU_REG_READ(umac_id, UNIMAC_RDP, DIAG_SEL, reg_diag_sel);

    *diag_hi_select = RU_FIELD_GET(umac_id, UNIMAC_RDP, DIAG_SEL, DIAG_HI_SELECT, reg_diag_sel);
    *diag_lo_select = RU_FIELD_GET(umac_id, UNIMAC_RDP, DIAG_SEL, DIAG_LO_SELECT, reg_diag_sel);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
typedef enum
{
    bdmf_address_umac_dummy,
    bdmf_address_hd_bkp_cntl,
    bdmf_address_cmd,
    bdmf_address_mac0,
    bdmf_address_mac1,
    bdmf_address_frm_len,
    bdmf_address_pause_qunat,
    bdmf_address_sfd_offset,
    bdmf_address_mode,
    bdmf_address_frm_tag0,
    bdmf_address_frm_tag1,
    bdmf_address_tx_ipg_len,
    bdmf_address_eee_ctrl,
    bdmf_address_eee_lpi_timer,
    bdmf_address_eee_wake_timer,
    bdmf_address_eee_ref_count,
    bdmf_address_rx_pkt_drop_status,
    bdmf_address_symmetric_idle_threshold,
    bdmf_address_macsec_prog_tx_crc,
    bdmf_address_macsec_cntrl,
    bdmf_address_ts_status_cntrl,
    bdmf_address_tx_ts_data,
    bdmf_address_pause_cntrl,
    bdmf_address_txfifo_flush,
    bdmf_address_rxfifo_stat,
    bdmf_address_txfifo_stat,
    bdmf_address_ppp_cntrl,
    bdmf_address_ppp_refresh_cntrl,
    bdmf_address_tx_pause_prel0,
    bdmf_address_tx_pause_prel1,
    bdmf_address_tx_pause_prel2,
    bdmf_address_tx_pause_prel3,
    bdmf_address_rx_pause_prel0,
    bdmf_address_rx_pause_prel1,
    bdmf_address_rx_pause_prel2,
    bdmf_address_rx_pause_prel3,
    bdmf_address_rxerr_mask,
    bdmf_address_rx_max_pkt_size,
    bdmf_address_mdio_cmd,
    bdmf_address_mdio_cfg,
    bdmf_address_mdf_cnt,
    bdmf_address_diag_sel,
}
bdmf_address;

static int bcm_unimac_rdp_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_unimac_rdp_hd_bkp_cntl:
        err = ag_drv_unimac_rdp_hd_bkp_cntl_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_unimac_rdp_cmd:
    {
        unimac_rdp_cmd cmd = { .runt_filter_dis=parm[2].value.unumber, .txrx_en_config=parm[3].value.unumber, .tx_pause_ignore=parm[4].value.unumber, .prbl_ena=parm[5].value.unumber, .rx_err_disc=parm[6].value.unumber, .rmt_loop_ena=parm[7].value.unumber, .no_lgth_check=parm[8].value.unumber, .cntl_frm_ena=parm[9].value.unumber, .ena_ext_config=parm[10].value.unumber, .lcl_loop_ena=parm[11].value.unumber, .sw_reset=parm[12].value.unumber, .hd_ena=parm[13].value.unumber, .tx_addr_ins=parm[14].value.unumber, .rx_pause_ignore=parm[15].value.unumber, .pause_fwd=parm[16].value.unumber, .crc_fwd=parm[17].value.unumber, .pad_en=parm[18].value.unumber, .promis_en=parm[19].value.unumber, .eth_speed=parm[20].value.unumber, .rx_ena=parm[21].value.unumber, .tx_ena=parm[22].value.unumber};
        err = ag_drv_unimac_rdp_cmd_set(parm[1].value.unumber, &cmd);
        break;
    }
    case cli_unimac_rdp_mac0:
        err = ag_drv_unimac_rdp_mac0_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_rdp_mac1:
        err = ag_drv_unimac_rdp_mac1_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_rdp_frm_len:
        err = ag_drv_unimac_rdp_frm_len_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_rdp_pause_qunat:
        err = ag_drv_unimac_rdp_pause_qunat_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_rdp_frm_tag0:
        err = ag_drv_unimac_rdp_frm_tag0_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_rdp_frm_tag1:
        err = ag_drv_unimac_rdp_frm_tag1_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_rdp_tx_ipg_len:
        err = ag_drv_unimac_rdp_tx_ipg_len_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_rdp_eee_ctrl:
    {
        unimac_rdp_eee_ctrl eee_ctrl = { .lp_idle_prediction_mode=parm[2].value.unumber, .dis_eee_10m=parm[3].value.unumber, .eee_txclk_dis=parm[4].value.unumber, .rx_fifo_check=parm[5].value.unumber, .eee_en=parm[6].value.unumber, .en_lpi_tx_pause=parm[7].value.unumber, .en_lpi_tx_pfc=parm[8].value.unumber, .en_lpi_rx_pause=parm[9].value.unumber};
        err = ag_drv_unimac_rdp_eee_ctrl_set(parm[1].value.unumber, &eee_ctrl);
        break;
    }
    case cli_unimac_rdp_eee_lpi_timer:
        err = ag_drv_unimac_rdp_eee_lpi_timer_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_rdp_eee_wake_timer:
        err = ag_drv_unimac_rdp_eee_wake_timer_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_rdp_eee_ref_count:
        err = ag_drv_unimac_rdp_eee_ref_count_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_rdp_rx_pkt_drop_status:
        err = ag_drv_unimac_rdp_rx_pkt_drop_status_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_rdp_symmetric_idle_threshold:
        err = ag_drv_unimac_rdp_symmetric_idle_threshold_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_rdp_macsec_prog_tx_crc:
        err = ag_drv_unimac_rdp_macsec_prog_tx_crc_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_rdp_macsec_cntrl:
        err = ag_drv_unimac_rdp_macsec_cntrl_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_unimac_rdp_pause_cntrl:
        err = ag_drv_unimac_rdp_pause_cntrl_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_unimac_rdp_txfifo_flush:
        err = ag_drv_unimac_rdp_txfifo_flush_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_rdp_ppp_cntrl:
    {
        unimac_rdp_ppp_cntrl ppp_cntrl = { .pfc_stats_en=parm[2].value.unumber, .rx_pass_pfc_frm=parm[3].value.unumber, .force_ppp_xon=parm[4].value.unumber, .ppp_en_rx=parm[5].value.unumber, .ppp_en_tx=parm[6].value.unumber};
        err = ag_drv_unimac_rdp_ppp_cntrl_set(parm[1].value.unumber, &ppp_cntrl);
        break;
    }
    case cli_unimac_rdp_ppp_refresh_cntrl:
        err = ag_drv_unimac_rdp_ppp_refresh_cntrl_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_unimac_rdp_rxerr_mask:
        err = ag_drv_unimac_rdp_rxerr_mask_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_rdp_rx_max_pkt_size:
        err = ag_drv_unimac_rdp_rx_max_pkt_size_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_rdp_mdio_cmd:
    {
        unimac_rdp_mdio_cmd mdio_cmd = { .mdio_busy=parm[2].value.unumber, .fail=parm[3].value.unumber, .op_code=parm[4].value.unumber, .phy_prt_addr=parm[5].value.unumber, .reg_dec_addr=parm[6].value.unumber, .data_addr=parm[7].value.unumber};
        err = ag_drv_unimac_rdp_mdio_cmd_set(parm[1].value.unumber, &mdio_cmd);
        break;
    }
    case cli_unimac_rdp_mdio_cfg:
        err = ag_drv_unimac_rdp_mdio_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_unimac_rdp_diag_sel:
        err = ag_drv_unimac_rdp_diag_sel_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
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
    case cli_unimac_rdp_umac_dummy:
    {
        uint8_t umac_dummy;
        err = ag_drv_unimac_rdp_umac_dummy_get(parm[1].value.unumber, &umac_dummy);
        bdmf_session_print(session, "umac_dummy = %u (0x%x)\n", umac_dummy, umac_dummy);
        break;
    }
    case cli_unimac_rdp_hd_bkp_cntl:
    {
        uint8_t ipg_config_rx;
        bdmf_boolean hd_fc_bkoff_ok;
        bdmf_boolean hd_fc_ena;
        err = ag_drv_unimac_rdp_hd_bkp_cntl_get(parm[1].value.unumber, &ipg_config_rx, &hd_fc_bkoff_ok, &hd_fc_ena);
        bdmf_session_print(session, "ipg_config_rx = %u (0x%x)\n", ipg_config_rx, ipg_config_rx);
        bdmf_session_print(session, "hd_fc_bkoff_ok = %u (0x%x)\n", hd_fc_bkoff_ok, hd_fc_bkoff_ok);
        bdmf_session_print(session, "hd_fc_ena = %u (0x%x)\n", hd_fc_ena, hd_fc_ena);
        break;
    }
    case cli_unimac_rdp_cmd:
    {
        unimac_rdp_cmd cmd;
        err = ag_drv_unimac_rdp_cmd_get(parm[1].value.unumber, &cmd);
        bdmf_session_print(session, "runt_filter_dis = %u (0x%x)\n", cmd.runt_filter_dis, cmd.runt_filter_dis);
        bdmf_session_print(session, "txrx_en_config = %u (0x%x)\n", cmd.txrx_en_config, cmd.txrx_en_config);
        bdmf_session_print(session, "tx_pause_ignore = %u (0x%x)\n", cmd.tx_pause_ignore, cmd.tx_pause_ignore);
        bdmf_session_print(session, "prbl_ena = %u (0x%x)\n", cmd.prbl_ena, cmd.prbl_ena);
        bdmf_session_print(session, "rx_err_disc = %u (0x%x)\n", cmd.rx_err_disc, cmd.rx_err_disc);
        bdmf_session_print(session, "rmt_loop_ena = %u (0x%x)\n", cmd.rmt_loop_ena, cmd.rmt_loop_ena);
        bdmf_session_print(session, "no_lgth_check = %u (0x%x)\n", cmd.no_lgth_check, cmd.no_lgth_check);
        bdmf_session_print(session, "cntl_frm_ena = %u (0x%x)\n", cmd.cntl_frm_ena, cmd.cntl_frm_ena);
        bdmf_session_print(session, "ena_ext_config = %u (0x%x)\n", cmd.ena_ext_config, cmd.ena_ext_config);
        bdmf_session_print(session, "lcl_loop_ena = %u (0x%x)\n", cmd.lcl_loop_ena, cmd.lcl_loop_ena);
        bdmf_session_print(session, "sw_reset = %u (0x%x)\n", cmd.sw_reset, cmd.sw_reset);
        bdmf_session_print(session, "hd_ena = %u (0x%x)\n", cmd.hd_ena, cmd.hd_ena);
        bdmf_session_print(session, "tx_addr_ins = %u (0x%x)\n", cmd.tx_addr_ins, cmd.tx_addr_ins);
        bdmf_session_print(session, "rx_pause_ignore = %u (0x%x)\n", cmd.rx_pause_ignore, cmd.rx_pause_ignore);
        bdmf_session_print(session, "pause_fwd = %u (0x%x)\n", cmd.pause_fwd, cmd.pause_fwd);
        bdmf_session_print(session, "crc_fwd = %u (0x%x)\n", cmd.crc_fwd, cmd.crc_fwd);
        bdmf_session_print(session, "pad_en = %u (0x%x)\n", cmd.pad_en, cmd.pad_en);
        bdmf_session_print(session, "promis_en = %u (0x%x)\n", cmd.promis_en, cmd.promis_en);
        bdmf_session_print(session, "eth_speed = %u (0x%x)\n", cmd.eth_speed, cmd.eth_speed);
        bdmf_session_print(session, "rx_ena = %u (0x%x)\n", cmd.rx_ena, cmd.rx_ena);
        bdmf_session_print(session, "tx_ena = %u (0x%x)\n", cmd.tx_ena, cmd.tx_ena);
        break;
    }
    case cli_unimac_rdp_mac0:
    {
        uint32_t mac_0;
        err = ag_drv_unimac_rdp_mac0_get(parm[1].value.unumber, &mac_0);
        bdmf_session_print(session, "mac_0 = %u (0x%x)\n", mac_0, mac_0);
        break;
    }
    case cli_unimac_rdp_mac1:
    {
        uint16_t mac_1;
        err = ag_drv_unimac_rdp_mac1_get(parm[1].value.unumber, &mac_1);
        bdmf_session_print(session, "mac_1 = %u (0x%x)\n", mac_1, mac_1);
        break;
    }
    case cli_unimac_rdp_frm_len:
    {
        uint16_t frame_length;
        err = ag_drv_unimac_rdp_frm_len_get(parm[1].value.unumber, &frame_length);
        bdmf_session_print(session, "frame_length = %u (0x%x)\n", frame_length, frame_length);
        break;
    }
    case cli_unimac_rdp_pause_qunat:
    {
        uint16_t pause_quant;
        err = ag_drv_unimac_rdp_pause_qunat_get(parm[1].value.unumber, &pause_quant);
        bdmf_session_print(session, "pause_quant = %u (0x%x)\n", pause_quant, pause_quant);
        break;
    }
    case cli_unimac_rdp_sfd_offset:
    {
        uint32_t temp;
        err = ag_drv_unimac_rdp_sfd_offset_get(parm[1].value.unumber, &temp);
        bdmf_session_print(session, "temp = %u (0x%x)\n", temp, temp);
        break;
    }
    case cli_unimac_rdp_mode:
    {
        unimac_rdp_mode mode;
        err = ag_drv_unimac_rdp_mode_get(parm[1].value.unumber, &mode);
        bdmf_session_print(session, "mac_link_stat = %u (0x%x)\n", mode.mac_link_stat, mode.mac_link_stat);
        bdmf_session_print(session, "mac_tx_pause = %u (0x%x)\n", mode.mac_tx_pause, mode.mac_tx_pause);
        bdmf_session_print(session, "mac_rx_pause = %u (0x%x)\n", mode.mac_rx_pause, mode.mac_rx_pause);
        bdmf_session_print(session, "mac_duplex = %u (0x%x)\n", mode.mac_duplex, mode.mac_duplex);
        bdmf_session_print(session, "mac_speed = %u (0x%x)\n", mode.mac_speed, mode.mac_speed);
        break;
    }
    case cli_unimac_rdp_frm_tag0:
    {
        uint16_t outer_tag;
        err = ag_drv_unimac_rdp_frm_tag0_get(parm[1].value.unumber, &outer_tag);
        bdmf_session_print(session, "outer_tag = %u (0x%x)\n", outer_tag, outer_tag);
        break;
    }
    case cli_unimac_rdp_frm_tag1:
    {
        uint16_t inner_tag;
        err = ag_drv_unimac_rdp_frm_tag1_get(parm[1].value.unumber, &inner_tag);
        bdmf_session_print(session, "inner_tag = %u (0x%x)\n", inner_tag, inner_tag);
        break;
    }
    case cli_unimac_rdp_tx_ipg_len:
    {
        uint8_t tx_ipg_len;
        err = ag_drv_unimac_rdp_tx_ipg_len_get(parm[1].value.unumber, &tx_ipg_len);
        bdmf_session_print(session, "tx_ipg_len = %u (0x%x)\n", tx_ipg_len, tx_ipg_len);
        break;
    }
    case cli_unimac_rdp_eee_ctrl:
    {
        unimac_rdp_eee_ctrl eee_ctrl;
        err = ag_drv_unimac_rdp_eee_ctrl_get(parm[1].value.unumber, &eee_ctrl);
        bdmf_session_print(session, "lp_idle_prediction_mode = %u (0x%x)\n", eee_ctrl.lp_idle_prediction_mode, eee_ctrl.lp_idle_prediction_mode);
        bdmf_session_print(session, "dis_eee_10m = %u (0x%x)\n", eee_ctrl.dis_eee_10m, eee_ctrl.dis_eee_10m);
        bdmf_session_print(session, "eee_txclk_dis = %u (0x%x)\n", eee_ctrl.eee_txclk_dis, eee_ctrl.eee_txclk_dis);
        bdmf_session_print(session, "rx_fifo_check = %u (0x%x)\n", eee_ctrl.rx_fifo_check, eee_ctrl.rx_fifo_check);
        bdmf_session_print(session, "eee_en = %u (0x%x)\n", eee_ctrl.eee_en, eee_ctrl.eee_en);
        bdmf_session_print(session, "en_lpi_tx_pause = %u (0x%x)\n", eee_ctrl.en_lpi_tx_pause, eee_ctrl.en_lpi_tx_pause);
        bdmf_session_print(session, "en_lpi_tx_pfc = %u (0x%x)\n", eee_ctrl.en_lpi_tx_pfc, eee_ctrl.en_lpi_tx_pfc);
        bdmf_session_print(session, "en_lpi_rx_pause = %u (0x%x)\n", eee_ctrl.en_lpi_rx_pause, eee_ctrl.en_lpi_rx_pause);
        break;
    }
    case cli_unimac_rdp_eee_lpi_timer:
    {
        uint32_t eee_lpi_timer;
        err = ag_drv_unimac_rdp_eee_lpi_timer_get(parm[1].value.unumber, &eee_lpi_timer);
        bdmf_session_print(session, "eee_lpi_timer = %u (0x%x)\n", eee_lpi_timer, eee_lpi_timer);
        break;
    }
    case cli_unimac_rdp_eee_wake_timer:
    {
        uint16_t eee_wake_timer;
        err = ag_drv_unimac_rdp_eee_wake_timer_get(parm[1].value.unumber, &eee_wake_timer);
        bdmf_session_print(session, "eee_wake_timer = %u (0x%x)\n", eee_wake_timer, eee_wake_timer);
        break;
    }
    case cli_unimac_rdp_eee_ref_count:
    {
        uint16_t eee_reference_count;
        err = ag_drv_unimac_rdp_eee_ref_count_get(parm[1].value.unumber, &eee_reference_count);
        bdmf_session_print(session, "eee_reference_count = %u (0x%x)\n", eee_reference_count, eee_reference_count);
        break;
    }
    case cli_unimac_rdp_rx_pkt_drop_status:
    {
        bdmf_boolean rx_ipg_invalid;
        err = ag_drv_unimac_rdp_rx_pkt_drop_status_get(parm[1].value.unumber, &rx_ipg_invalid);
        bdmf_session_print(session, "rx_ipg_invalid = %u (0x%x)\n", rx_ipg_invalid, rx_ipg_invalid);
        break;
    }
    case cli_unimac_rdp_symmetric_idle_threshold:
    {
        uint16_t threshold_value;
        err = ag_drv_unimac_rdp_symmetric_idle_threshold_get(parm[1].value.unumber, &threshold_value);
        bdmf_session_print(session, "threshold_value = %u (0x%x)\n", threshold_value, threshold_value);
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
        bdmf_boolean tx_crc_program;
        bdmf_boolean tx_crc_corrupt_en;
        bdmf_boolean tx_lanuch_en;
        err = ag_drv_unimac_rdp_macsec_cntrl_get(parm[1].value.unumber, &tx_crc_program, &tx_crc_corrupt_en, &tx_lanuch_en);
        bdmf_session_print(session, "tx_crc_program = %u (0x%x)\n", tx_crc_program, tx_crc_program);
        bdmf_session_print(session, "tx_crc_corrupt_en = %u (0x%x)\n", tx_crc_corrupt_en, tx_crc_corrupt_en);
        bdmf_session_print(session, "tx_lanuch_en = %u (0x%x)\n", tx_lanuch_en, tx_lanuch_en);
        break;
    }
    case cli_unimac_rdp_ts_status_cntrl:
    {
        uint8_t word_avail;
        bdmf_boolean tx_ts_fifo_empty;
        bdmf_boolean tx_ts_fifo_full;
        err = ag_drv_unimac_rdp_ts_status_cntrl_get(parm[1].value.unumber, &word_avail, &tx_ts_fifo_empty, &tx_ts_fifo_full);
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
    case cli_unimac_rdp_pause_cntrl:
    {
        bdmf_boolean pause_control_en;
        uint32_t pause_timer;
        err = ag_drv_unimac_rdp_pause_cntrl_get(parm[1].value.unumber, &pause_control_en, &pause_timer);
        bdmf_session_print(session, "pause_control_en = %u (0x%x)\n", pause_control_en, pause_control_en);
        bdmf_session_print(session, "pause_timer = %u (0x%x)\n", pause_timer, pause_timer);
        break;
    }
    case cli_unimac_rdp_txfifo_flush:
    {
        bdmf_boolean tx_flush;
        err = ag_drv_unimac_rdp_txfifo_flush_get(parm[1].value.unumber, &tx_flush);
        bdmf_session_print(session, "tx_flush = %u (0x%x)\n", tx_flush, tx_flush);
        break;
    }
    case cli_unimac_rdp_rxfifo_stat:
    {
        uint8_t rxfifo_status;
        err = ag_drv_unimac_rdp_rxfifo_stat_get(parm[1].value.unumber, &rxfifo_status);
        bdmf_session_print(session, "rxfifo_status = %u (0x%x)\n", rxfifo_status, rxfifo_status);
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
    case cli_unimac_rdp_ppp_cntrl:
    {
        unimac_rdp_ppp_cntrl ppp_cntrl;
        err = ag_drv_unimac_rdp_ppp_cntrl_get(parm[1].value.unumber, &ppp_cntrl);
        bdmf_session_print(session, "pfc_stats_en = %u (0x%x)\n", ppp_cntrl.pfc_stats_en, ppp_cntrl.pfc_stats_en);
        bdmf_session_print(session, "rx_pass_pfc_frm = %u (0x%x)\n", ppp_cntrl.rx_pass_pfc_frm, ppp_cntrl.rx_pass_pfc_frm);
        bdmf_session_print(session, "force_ppp_xon = %u (0x%x)\n", ppp_cntrl.force_ppp_xon, ppp_cntrl.force_ppp_xon);
        bdmf_session_print(session, "ppp_en_rx = %u (0x%x)\n", ppp_cntrl.ppp_en_rx, ppp_cntrl.ppp_en_rx);
        bdmf_session_print(session, "ppp_en_tx = %u (0x%x)\n", ppp_cntrl.ppp_en_tx, ppp_cntrl.ppp_en_tx);
        break;
    }
    case cli_unimac_rdp_ppp_refresh_cntrl:
    {
        uint16_t ppp_refresh_timer;
        bdmf_boolean ppp_refresh_en;
        err = ag_drv_unimac_rdp_ppp_refresh_cntrl_get(parm[1].value.unumber, &ppp_refresh_timer, &ppp_refresh_en);
        bdmf_session_print(session, "ppp_refresh_timer = %u (0x%x)\n", ppp_refresh_timer, ppp_refresh_timer);
        bdmf_session_print(session, "ppp_refresh_en = %u (0x%x)\n", ppp_refresh_en, ppp_refresh_en);
        break;
    }
    case cli_unimac_rdp_tx_pause_prel0:
    {
        uint32_t tx_pause_prb0;
        err = ag_drv_unimac_rdp_tx_pause_prel0_get(parm[1].value.unumber, &tx_pause_prb0);
        bdmf_session_print(session, "tx_pause_prb0 = %u (0x%x)\n", tx_pause_prb0, tx_pause_prb0);
        break;
    }
    case cli_unimac_rdp_tx_pause_prel1:
    {
        uint32_t tx_pause_prb1;
        err = ag_drv_unimac_rdp_tx_pause_prel1_get(parm[1].value.unumber, &tx_pause_prb1);
        bdmf_session_print(session, "tx_pause_prb1 = %u (0x%x)\n", tx_pause_prb1, tx_pause_prb1);
        break;
    }
    case cli_unimac_rdp_tx_pause_prel2:
    {
        uint32_t tx_pause_prb2;
        err = ag_drv_unimac_rdp_tx_pause_prel2_get(parm[1].value.unumber, &tx_pause_prb2);
        bdmf_session_print(session, "tx_pause_prb2 = %u (0x%x)\n", tx_pause_prb2, tx_pause_prb2);
        break;
    }
    case cli_unimac_rdp_tx_pause_prel3:
    {
        uint32_t tx_pause_prb3;
        err = ag_drv_unimac_rdp_tx_pause_prel3_get(parm[1].value.unumber, &tx_pause_prb3);
        bdmf_session_print(session, "tx_pause_prb3 = %u (0x%x)\n", tx_pause_prb3, tx_pause_prb3);
        break;
    }
    case cli_unimac_rdp_rx_pause_prel0:
    {
        uint32_t rx_pause_prb0;
        err = ag_drv_unimac_rdp_rx_pause_prel0_get(parm[1].value.unumber, &rx_pause_prb0);
        bdmf_session_print(session, "rx_pause_prb0 = %u (0x%x)\n", rx_pause_prb0, rx_pause_prb0);
        break;
    }
    case cli_unimac_rdp_rx_pause_prel1:
    {
        uint32_t rx_pause_prb1;
        err = ag_drv_unimac_rdp_rx_pause_prel1_get(parm[1].value.unumber, &rx_pause_prb1);
        bdmf_session_print(session, "rx_pause_prb1 = %u (0x%x)\n", rx_pause_prb1, rx_pause_prb1);
        break;
    }
    case cli_unimac_rdp_rx_pause_prel2:
    {
        uint32_t rx_pause_prb2;
        err = ag_drv_unimac_rdp_rx_pause_prel2_get(parm[1].value.unumber, &rx_pause_prb2);
        bdmf_session_print(session, "rx_pause_prb2 = %u (0x%x)\n", rx_pause_prb2, rx_pause_prb2);
        break;
    }
    case cli_unimac_rdp_rx_pause_prel3:
    {
        uint32_t rx_pause_prb3;
        err = ag_drv_unimac_rdp_rx_pause_prel3_get(parm[1].value.unumber, &rx_pause_prb3);
        bdmf_session_print(session, "rx_pause_prb3 = %u (0x%x)\n", rx_pause_prb3, rx_pause_prb3);
        break;
    }
    case cli_unimac_rdp_rxerr_mask:
    {
        uint32_t mac_rxerr_mask;
        err = ag_drv_unimac_rdp_rxerr_mask_get(parm[1].value.unumber, &mac_rxerr_mask);
        bdmf_session_print(session, "mac_rxerr_mask = %u (0x%x)\n", mac_rxerr_mask, mac_rxerr_mask);
        break;
    }
    case cli_unimac_rdp_rx_max_pkt_size:
    {
        uint16_t max_pkt_size;
        err = ag_drv_unimac_rdp_rx_max_pkt_size_get(parm[1].value.unumber, &max_pkt_size);
        bdmf_session_print(session, "max_pkt_size = %u (0x%x)\n", max_pkt_size, max_pkt_size);
        break;
    }
    case cli_unimac_rdp_mdio_cmd:
    {
        unimac_rdp_mdio_cmd mdio_cmd;
        err = ag_drv_unimac_rdp_mdio_cmd_get(parm[1].value.unumber, &mdio_cmd);
        bdmf_session_print(session, "mdio_busy = %u (0x%x)\n", mdio_cmd.mdio_busy, mdio_cmd.mdio_busy);
        bdmf_session_print(session, "fail = %u (0x%x)\n", mdio_cmd.fail, mdio_cmd.fail);
        bdmf_session_print(session, "op_code = %u (0x%x)\n", mdio_cmd.op_code, mdio_cmd.op_code);
        bdmf_session_print(session, "phy_prt_addr = %u (0x%x)\n", mdio_cmd.phy_prt_addr, mdio_cmd.phy_prt_addr);
        bdmf_session_print(session, "reg_dec_addr = %u (0x%x)\n", mdio_cmd.reg_dec_addr, mdio_cmd.reg_dec_addr);
        bdmf_session_print(session, "data_addr = %u (0x%x)\n", mdio_cmd.data_addr, mdio_cmd.data_addr);
        break;
    }
    case cli_unimac_rdp_mdio_cfg:
    {
        uint8_t mdio_clk_divider;
        bdmf_boolean mdio_clause;
        err = ag_drv_unimac_rdp_mdio_cfg_get(parm[1].value.unumber, &mdio_clk_divider, &mdio_clause);
        bdmf_session_print(session, "mdio_clk_divider = %u (0x%x)\n", mdio_clk_divider, mdio_clk_divider);
        bdmf_session_print(session, "mdio_clause = %u (0x%x)\n", mdio_clause, mdio_clause);
        break;
    }
    case cli_unimac_rdp_mdf_cnt:
    {
        uint32_t mdf_packet_counter;
        err = ag_drv_unimac_rdp_mdf_cnt_get(parm[1].value.unumber, &mdf_packet_counter);
        bdmf_session_print(session, "mdf_packet_counter = %u (0x%x)\n", mdf_packet_counter, mdf_packet_counter);
        break;
    }
    case cli_unimac_rdp_diag_sel:
    {
        uint8_t diag_hi_select;
        uint8_t diag_lo_select;
        err = ag_drv_unimac_rdp_diag_sel_get(parm[1].value.unumber, &diag_hi_select, &diag_lo_select);
        bdmf_session_print(session, "diag_hi_select = %u (0x%x)\n", diag_hi_select, diag_hi_select);
        bdmf_session_print(session, "diag_lo_select = %u (0x%x)\n", diag_lo_select, diag_lo_select);
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
        uint8_t umac_dummy=gtmv(m, 8);
        if(!err) ag_drv_unimac_rdp_umac_dummy_get( umac_id, &umac_dummy);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_umac_dummy_get(%u %u)\n", umac_id, umac_dummy);
    }
    {
        uint8_t ipg_config_rx=gtmv(m, 5);
        bdmf_boolean hd_fc_bkoff_ok=gtmv(m, 1);
        bdmf_boolean hd_fc_ena=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_hd_bkp_cntl_set(%u %u %u %u)\n", umac_id, ipg_config_rx, hd_fc_bkoff_ok, hd_fc_ena);
        if(!err) ag_drv_unimac_rdp_hd_bkp_cntl_set(umac_id, ipg_config_rx, hd_fc_bkoff_ok, hd_fc_ena);
        if(!err) ag_drv_unimac_rdp_hd_bkp_cntl_get( umac_id, &ipg_config_rx, &hd_fc_bkoff_ok, &hd_fc_ena);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_hd_bkp_cntl_get(%u %u %u %u)\n", umac_id, ipg_config_rx, hd_fc_bkoff_ok, hd_fc_ena);
        if(err || ipg_config_rx!=gtmv(m, 5) || hd_fc_bkoff_ok!=gtmv(m, 1) || hd_fc_ena!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        unimac_rdp_cmd cmd = {.runt_filter_dis=gtmv(m, 1), .txrx_en_config=gtmv(m, 1), .tx_pause_ignore=gtmv(m, 1), .prbl_ena=gtmv(m, 1), .rx_err_disc=gtmv(m, 1), .rmt_loop_ena=gtmv(m, 1), .no_lgth_check=gtmv(m, 1), .cntl_frm_ena=gtmv(m, 1), .ena_ext_config=gtmv(m, 1), .lcl_loop_ena=gtmv(m, 1), .sw_reset=gtmv(m, 1), .hd_ena=gtmv(m, 1), .tx_addr_ins=gtmv(m, 1), .rx_pause_ignore=gtmv(m, 1), .pause_fwd=gtmv(m, 1), .crc_fwd=gtmv(m, 1), .pad_en=gtmv(m, 1), .promis_en=gtmv(m, 1), .eth_speed=gtmv(m, 2), .rx_ena=gtmv(m, 1), .tx_ena=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_cmd_set(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", umac_id, cmd.runt_filter_dis, cmd.txrx_en_config, cmd.tx_pause_ignore, cmd.prbl_ena, cmd.rx_err_disc, cmd.rmt_loop_ena, cmd.no_lgth_check, cmd.cntl_frm_ena, cmd.ena_ext_config, cmd.lcl_loop_ena, cmd.sw_reset, cmd.hd_ena, cmd.tx_addr_ins, cmd.rx_pause_ignore, cmd.pause_fwd, cmd.crc_fwd, cmd.pad_en, cmd.promis_en, cmd.eth_speed, cmd.rx_ena, cmd.tx_ena);
        if(!err) ag_drv_unimac_rdp_cmd_set(umac_id, &cmd);
        if(!err) ag_drv_unimac_rdp_cmd_get( umac_id, &cmd);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_cmd_get(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", umac_id, cmd.runt_filter_dis, cmd.txrx_en_config, cmd.tx_pause_ignore, cmd.prbl_ena, cmd.rx_err_disc, cmd.rmt_loop_ena, cmd.no_lgth_check, cmd.cntl_frm_ena, cmd.ena_ext_config, cmd.lcl_loop_ena, cmd.sw_reset, cmd.hd_ena, cmd.tx_addr_ins, cmd.rx_pause_ignore, cmd.pause_fwd, cmd.crc_fwd, cmd.pad_en, cmd.promis_en, cmd.eth_speed, cmd.rx_ena, cmd.tx_ena);
        if(err || cmd.runt_filter_dis!=gtmv(m, 1) || cmd.txrx_en_config!=gtmv(m, 1) || cmd.tx_pause_ignore!=gtmv(m, 1) || cmd.prbl_ena!=gtmv(m, 1) || cmd.rx_err_disc!=gtmv(m, 1) || cmd.rmt_loop_ena!=gtmv(m, 1) || cmd.no_lgth_check!=gtmv(m, 1) || cmd.cntl_frm_ena!=gtmv(m, 1) || cmd.ena_ext_config!=gtmv(m, 1) || cmd.lcl_loop_ena!=gtmv(m, 1) || cmd.sw_reset!=gtmv(m, 1) || cmd.hd_ena!=gtmv(m, 1) || cmd.tx_addr_ins!=gtmv(m, 1) || cmd.rx_pause_ignore!=gtmv(m, 1) || cmd.pause_fwd!=gtmv(m, 1) || cmd.crc_fwd!=gtmv(m, 1) || cmd.pad_en!=gtmv(m, 1) || cmd.promis_en!=gtmv(m, 1) || cmd.eth_speed!=gtmv(m, 2) || cmd.rx_ena!=gtmv(m, 1) || cmd.tx_ena!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t mac_0=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_mac0_set(%u %u)\n", umac_id, mac_0);
        if(!err) ag_drv_unimac_rdp_mac0_set(umac_id, mac_0);
        if(!err) ag_drv_unimac_rdp_mac0_get( umac_id, &mac_0);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_mac0_get(%u %u)\n", umac_id, mac_0);
        if(err || mac_0!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t mac_1=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_mac1_set(%u %u)\n", umac_id, mac_1);
        if(!err) ag_drv_unimac_rdp_mac1_set(umac_id, mac_1);
        if(!err) ag_drv_unimac_rdp_mac1_get( umac_id, &mac_1);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_mac1_get(%u %u)\n", umac_id, mac_1);
        if(err || mac_1!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t frame_length=gtmv(m, 14);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_frm_len_set(%u %u)\n", umac_id, frame_length);
        if(!err) ag_drv_unimac_rdp_frm_len_set(umac_id, frame_length);
        if(!err) ag_drv_unimac_rdp_frm_len_get( umac_id, &frame_length);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_frm_len_get(%u %u)\n", umac_id, frame_length);
        if(err || frame_length!=gtmv(m, 14))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t pause_quant=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_pause_qunat_set(%u %u)\n", umac_id, pause_quant);
        if(!err) ag_drv_unimac_rdp_pause_qunat_set(umac_id, pause_quant);
        if(!err) ag_drv_unimac_rdp_pause_qunat_get( umac_id, &pause_quant);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_pause_qunat_get(%u %u)\n", umac_id, pause_quant);
        if(err || pause_quant!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t temp=gtmv(m, 32);
        if(!err) ag_drv_unimac_rdp_sfd_offset_get( umac_id, &temp);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_sfd_offset_get(%u %u)\n", umac_id, temp);
    }
    {
        unimac_rdp_mode mode = {.mac_link_stat=gtmv(m, 1), .mac_tx_pause=gtmv(m, 1), .mac_rx_pause=gtmv(m, 1), .mac_duplex=gtmv(m, 1), .mac_speed=gtmv(m, 2)};
        if(!err) ag_drv_unimac_rdp_mode_get( umac_id, &mode);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_mode_get(%u %u %u %u %u %u)\n", umac_id, mode.mac_link_stat, mode.mac_tx_pause, mode.mac_rx_pause, mode.mac_duplex, mode.mac_speed);
    }
    {
        uint16_t outer_tag=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_frm_tag0_set(%u %u)\n", umac_id, outer_tag);
        if(!err) ag_drv_unimac_rdp_frm_tag0_set(umac_id, outer_tag);
        if(!err) ag_drv_unimac_rdp_frm_tag0_get( umac_id, &outer_tag);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_frm_tag0_get(%u %u)\n", umac_id, outer_tag);
        if(err || outer_tag!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t inner_tag=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_frm_tag1_set(%u %u)\n", umac_id, inner_tag);
        if(!err) ag_drv_unimac_rdp_frm_tag1_set(umac_id, inner_tag);
        if(!err) ag_drv_unimac_rdp_frm_tag1_get( umac_id, &inner_tag);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_frm_tag1_get(%u %u)\n", umac_id, inner_tag);
        if(err || inner_tag!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t tx_ipg_len=gtmv(m, 7);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_tx_ipg_len_set(%u %u)\n", umac_id, tx_ipg_len);
        if(!err) ag_drv_unimac_rdp_tx_ipg_len_set(umac_id, tx_ipg_len);
        if(!err) ag_drv_unimac_rdp_tx_ipg_len_get( umac_id, &tx_ipg_len);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_tx_ipg_len_get(%u %u)\n", umac_id, tx_ipg_len);
        if(err || tx_ipg_len!=gtmv(m, 7))
            return err ? err : BDMF_ERR_IO;
    }
    {
        unimac_rdp_eee_ctrl eee_ctrl = {.lp_idle_prediction_mode=gtmv(m, 1), .dis_eee_10m=gtmv(m, 1), .eee_txclk_dis=gtmv(m, 1), .rx_fifo_check=gtmv(m, 1), .eee_en=gtmv(m, 1), .en_lpi_tx_pause=gtmv(m, 1), .en_lpi_tx_pfc=gtmv(m, 1), .en_lpi_rx_pause=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_eee_ctrl_set(%u %u %u %u %u %u %u %u %u)\n", umac_id, eee_ctrl.lp_idle_prediction_mode, eee_ctrl.dis_eee_10m, eee_ctrl.eee_txclk_dis, eee_ctrl.rx_fifo_check, eee_ctrl.eee_en, eee_ctrl.en_lpi_tx_pause, eee_ctrl.en_lpi_tx_pfc, eee_ctrl.en_lpi_rx_pause);
        if(!err) ag_drv_unimac_rdp_eee_ctrl_set(umac_id, &eee_ctrl);
        if(!err) ag_drv_unimac_rdp_eee_ctrl_get( umac_id, &eee_ctrl);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_eee_ctrl_get(%u %u %u %u %u %u %u %u %u)\n", umac_id, eee_ctrl.lp_idle_prediction_mode, eee_ctrl.dis_eee_10m, eee_ctrl.eee_txclk_dis, eee_ctrl.rx_fifo_check, eee_ctrl.eee_en, eee_ctrl.en_lpi_tx_pause, eee_ctrl.en_lpi_tx_pfc, eee_ctrl.en_lpi_rx_pause);
        if(err || eee_ctrl.lp_idle_prediction_mode!=gtmv(m, 1) || eee_ctrl.dis_eee_10m!=gtmv(m, 1) || eee_ctrl.eee_txclk_dis!=gtmv(m, 1) || eee_ctrl.rx_fifo_check!=gtmv(m, 1) || eee_ctrl.eee_en!=gtmv(m, 1) || eee_ctrl.en_lpi_tx_pause!=gtmv(m, 1) || eee_ctrl.en_lpi_tx_pfc!=gtmv(m, 1) || eee_ctrl.en_lpi_rx_pause!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t eee_lpi_timer=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_eee_lpi_timer_set(%u %u)\n", umac_id, eee_lpi_timer);
        if(!err) ag_drv_unimac_rdp_eee_lpi_timer_set(umac_id, eee_lpi_timer);
        if(!err) ag_drv_unimac_rdp_eee_lpi_timer_get( umac_id, &eee_lpi_timer);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_eee_lpi_timer_get(%u %u)\n", umac_id, eee_lpi_timer);
        if(err || eee_lpi_timer!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t eee_wake_timer=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_eee_wake_timer_set(%u %u)\n", umac_id, eee_wake_timer);
        if(!err) ag_drv_unimac_rdp_eee_wake_timer_set(umac_id, eee_wake_timer);
        if(!err) ag_drv_unimac_rdp_eee_wake_timer_get( umac_id, &eee_wake_timer);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_eee_wake_timer_get(%u %u)\n", umac_id, eee_wake_timer);
        if(err || eee_wake_timer!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t eee_reference_count=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_eee_ref_count_set(%u %u)\n", umac_id, eee_reference_count);
        if(!err) ag_drv_unimac_rdp_eee_ref_count_set(umac_id, eee_reference_count);
        if(!err) ag_drv_unimac_rdp_eee_ref_count_get( umac_id, &eee_reference_count);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_eee_ref_count_get(%u %u)\n", umac_id, eee_reference_count);
        if(err || eee_reference_count!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean rx_ipg_invalid=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_rx_pkt_drop_status_set(%u %u)\n", umac_id, rx_ipg_invalid);
        if(!err) ag_drv_unimac_rdp_rx_pkt_drop_status_set(umac_id, rx_ipg_invalid);
        if(!err) ag_drv_unimac_rdp_rx_pkt_drop_status_get( umac_id, &rx_ipg_invalid);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_rx_pkt_drop_status_get(%u %u)\n", umac_id, rx_ipg_invalid);
        if(err || rx_ipg_invalid!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t threshold_value=gtmv(m, 16);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_symmetric_idle_threshold_set(%u %u)\n", umac_id, threshold_value);
        if(!err) ag_drv_unimac_rdp_symmetric_idle_threshold_set(umac_id, threshold_value);
        if(!err) ag_drv_unimac_rdp_symmetric_idle_threshold_get( umac_id, &threshold_value);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_symmetric_idle_threshold_get(%u %u)\n", umac_id, threshold_value);
        if(err || threshold_value!=gtmv(m, 16))
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
        bdmf_boolean tx_crc_program=gtmv(m, 1);
        bdmf_boolean tx_crc_corrupt_en=gtmv(m, 1);
        bdmf_boolean tx_lanuch_en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_macsec_cntrl_set(%u %u %u %u)\n", umac_id, tx_crc_program, tx_crc_corrupt_en, tx_lanuch_en);
        if(!err) ag_drv_unimac_rdp_macsec_cntrl_set(umac_id, tx_crc_program, tx_crc_corrupt_en, tx_lanuch_en);
        if(!err) ag_drv_unimac_rdp_macsec_cntrl_get( umac_id, &tx_crc_program, &tx_crc_corrupt_en, &tx_lanuch_en);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_macsec_cntrl_get(%u %u %u %u)\n", umac_id, tx_crc_program, tx_crc_corrupt_en, tx_lanuch_en);
        if(err || tx_crc_program!=gtmv(m, 1) || tx_crc_corrupt_en!=gtmv(m, 1) || tx_lanuch_en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t word_avail=gtmv(m, 2);
        bdmf_boolean tx_ts_fifo_empty=gtmv(m, 1);
        bdmf_boolean tx_ts_fifo_full=gtmv(m, 1);
        if(!err) ag_drv_unimac_rdp_ts_status_cntrl_get( umac_id, &word_avail, &tx_ts_fifo_empty, &tx_ts_fifo_full);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_ts_status_cntrl_get(%u %u %u %u)\n", umac_id, word_avail, tx_ts_fifo_empty, tx_ts_fifo_full);
    }
    {
        uint32_t tx_ts_data=gtmv(m, 32);
        if(!err) ag_drv_unimac_rdp_tx_ts_data_get( umac_id, &tx_ts_data);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_tx_ts_data_get(%u %u)\n", umac_id, tx_ts_data);
    }
    {
        bdmf_boolean pause_control_en=gtmv(m, 1);
        uint32_t pause_timer=gtmv(m, 17);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_pause_cntrl_set(%u %u %u)\n", umac_id, pause_control_en, pause_timer);
        if(!err) ag_drv_unimac_rdp_pause_cntrl_set(umac_id, pause_control_en, pause_timer);
        if(!err) ag_drv_unimac_rdp_pause_cntrl_get( umac_id, &pause_control_en, &pause_timer);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_pause_cntrl_get(%u %u %u)\n", umac_id, pause_control_en, pause_timer);
        if(err || pause_control_en!=gtmv(m, 1) || pause_timer!=gtmv(m, 17))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean tx_flush=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_txfifo_flush_set(%u %u)\n", umac_id, tx_flush);
        if(!err) ag_drv_unimac_rdp_txfifo_flush_set(umac_id, tx_flush);
        if(!err) ag_drv_unimac_rdp_txfifo_flush_get( umac_id, &tx_flush);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_txfifo_flush_get(%u %u)\n", umac_id, tx_flush);
        if(err || tx_flush!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t rxfifo_status=gtmv(m, 2);
        if(!err) ag_drv_unimac_rdp_rxfifo_stat_get( umac_id, &rxfifo_status);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_rxfifo_stat_get(%u %u)\n", umac_id, rxfifo_status);
    }
    {
        bdmf_boolean txfifo_overrun=gtmv(m, 1);
        bdmf_boolean txfifo_underrun=gtmv(m, 1);
        if(!err) ag_drv_unimac_rdp_txfifo_stat_get( umac_id, &txfifo_overrun, &txfifo_underrun);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_txfifo_stat_get(%u %u %u)\n", umac_id, txfifo_overrun, txfifo_underrun);
    }
    {
        unimac_rdp_ppp_cntrl ppp_cntrl = {.pfc_stats_en=gtmv(m, 1), .rx_pass_pfc_frm=gtmv(m, 1), .force_ppp_xon=gtmv(m, 1), .ppp_en_rx=gtmv(m, 1), .ppp_en_tx=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_ppp_cntrl_set(%u %u %u %u %u %u)\n", umac_id, ppp_cntrl.pfc_stats_en, ppp_cntrl.rx_pass_pfc_frm, ppp_cntrl.force_ppp_xon, ppp_cntrl.ppp_en_rx, ppp_cntrl.ppp_en_tx);
        if(!err) ag_drv_unimac_rdp_ppp_cntrl_set(umac_id, &ppp_cntrl);
        if(!err) ag_drv_unimac_rdp_ppp_cntrl_get( umac_id, &ppp_cntrl);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_ppp_cntrl_get(%u %u %u %u %u %u)\n", umac_id, ppp_cntrl.pfc_stats_en, ppp_cntrl.rx_pass_pfc_frm, ppp_cntrl.force_ppp_xon, ppp_cntrl.ppp_en_rx, ppp_cntrl.ppp_en_tx);
        if(err || ppp_cntrl.pfc_stats_en!=gtmv(m, 1) || ppp_cntrl.rx_pass_pfc_frm!=gtmv(m, 1) || ppp_cntrl.force_ppp_xon!=gtmv(m, 1) || ppp_cntrl.ppp_en_rx!=gtmv(m, 1) || ppp_cntrl.ppp_en_tx!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t ppp_refresh_timer=gtmv(m, 16);
        bdmf_boolean ppp_refresh_en=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_ppp_refresh_cntrl_set(%u %u %u)\n", umac_id, ppp_refresh_timer, ppp_refresh_en);
        if(!err) ag_drv_unimac_rdp_ppp_refresh_cntrl_set(umac_id, ppp_refresh_timer, ppp_refresh_en);
        if(!err) ag_drv_unimac_rdp_ppp_refresh_cntrl_get( umac_id, &ppp_refresh_timer, &ppp_refresh_en);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_ppp_refresh_cntrl_get(%u %u %u)\n", umac_id, ppp_refresh_timer, ppp_refresh_en);
        if(err || ppp_refresh_timer!=gtmv(m, 16) || ppp_refresh_en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t tx_pause_prb0=gtmv(m, 32);
        if(!err) ag_drv_unimac_rdp_tx_pause_prel0_get( umac_id, &tx_pause_prb0);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_tx_pause_prel0_get(%u %u)\n", umac_id, tx_pause_prb0);
    }
    {
        uint32_t tx_pause_prb1=gtmv(m, 32);
        if(!err) ag_drv_unimac_rdp_tx_pause_prel1_get( umac_id, &tx_pause_prb1);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_tx_pause_prel1_get(%u %u)\n", umac_id, tx_pause_prb1);
    }
    {
        uint32_t tx_pause_prb2=gtmv(m, 32);
        if(!err) ag_drv_unimac_rdp_tx_pause_prel2_get( umac_id, &tx_pause_prb2);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_tx_pause_prel2_get(%u %u)\n", umac_id, tx_pause_prb2);
    }
    {
        uint32_t tx_pause_prb3=gtmv(m, 32);
        if(!err) ag_drv_unimac_rdp_tx_pause_prel3_get( umac_id, &tx_pause_prb3);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_tx_pause_prel3_get(%u %u)\n", umac_id, tx_pause_prb3);
    }
    {
        uint32_t rx_pause_prb0=gtmv(m, 32);
        if(!err) ag_drv_unimac_rdp_rx_pause_prel0_get( umac_id, &rx_pause_prb0);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_rx_pause_prel0_get(%u %u)\n", umac_id, rx_pause_prb0);
    }
    {
        uint32_t rx_pause_prb1=gtmv(m, 32);
        if(!err) ag_drv_unimac_rdp_rx_pause_prel1_get( umac_id, &rx_pause_prb1);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_rx_pause_prel1_get(%u %u)\n", umac_id, rx_pause_prb1);
    }
    {
        uint32_t rx_pause_prb2=gtmv(m, 32);
        if(!err) ag_drv_unimac_rdp_rx_pause_prel2_get( umac_id, &rx_pause_prb2);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_rx_pause_prel2_get(%u %u)\n", umac_id, rx_pause_prb2);
    }
    {
        uint32_t rx_pause_prb3=gtmv(m, 32);
        if(!err) ag_drv_unimac_rdp_rx_pause_prel3_get( umac_id, &rx_pause_prb3);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_rx_pause_prel3_get(%u %u)\n", umac_id, rx_pause_prb3);
    }
    {
        uint32_t mac_rxerr_mask=gtmv(m, 18);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_rxerr_mask_set(%u %u)\n", umac_id, mac_rxerr_mask);
        if(!err) ag_drv_unimac_rdp_rxerr_mask_set(umac_id, mac_rxerr_mask);
        if(!err) ag_drv_unimac_rdp_rxerr_mask_get( umac_id, &mac_rxerr_mask);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_rxerr_mask_get(%u %u)\n", umac_id, mac_rxerr_mask);
        if(err || mac_rxerr_mask!=gtmv(m, 18))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t max_pkt_size=gtmv(m, 14);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_rx_max_pkt_size_set(%u %u)\n", umac_id, max_pkt_size);
        if(!err) ag_drv_unimac_rdp_rx_max_pkt_size_set(umac_id, max_pkt_size);
        if(!err) ag_drv_unimac_rdp_rx_max_pkt_size_get( umac_id, &max_pkt_size);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_rx_max_pkt_size_get(%u %u)\n", umac_id, max_pkt_size);
        if(err || max_pkt_size!=gtmv(m, 14))
            return err ? err : BDMF_ERR_IO;
    }
    {
        unimac_rdp_mdio_cmd mdio_cmd = {.mdio_busy=gtmv(m, 1), .fail=gtmv(m, 1), .op_code=gtmv(m, 2), .phy_prt_addr=gtmv(m, 5), .reg_dec_addr=gtmv(m, 5), .data_addr=gtmv(m, 16)};
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_mdio_cmd_set(%u %u %u %u %u %u %u)\n", umac_id, mdio_cmd.mdio_busy, mdio_cmd.fail, mdio_cmd.op_code, mdio_cmd.phy_prt_addr, mdio_cmd.reg_dec_addr, mdio_cmd.data_addr);
        if(!err) ag_drv_unimac_rdp_mdio_cmd_set(umac_id, &mdio_cmd);
        if(!err) ag_drv_unimac_rdp_mdio_cmd_get( umac_id, &mdio_cmd);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_mdio_cmd_get(%u %u %u %u %u %u %u)\n", umac_id, mdio_cmd.mdio_busy, mdio_cmd.fail, mdio_cmd.op_code, mdio_cmd.phy_prt_addr, mdio_cmd.reg_dec_addr, mdio_cmd.data_addr);
        if(err || mdio_cmd.mdio_busy!=gtmv(m, 1) || mdio_cmd.fail!=gtmv(m, 1) || mdio_cmd.op_code!=gtmv(m, 2) || mdio_cmd.phy_prt_addr!=gtmv(m, 5) || mdio_cmd.reg_dec_addr!=gtmv(m, 5) || mdio_cmd.data_addr!=gtmv(m, 16))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint8_t mdio_clk_divider=gtmv(m, 6);
        bdmf_boolean mdio_clause=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_mdio_cfg_set(%u %u %u)\n", umac_id, mdio_clk_divider, mdio_clause);
        if(!err) ag_drv_unimac_rdp_mdio_cfg_set(umac_id, mdio_clk_divider, mdio_clause);
        if(!err) ag_drv_unimac_rdp_mdio_cfg_get( umac_id, &mdio_clk_divider, &mdio_clause);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_mdio_cfg_get(%u %u %u)\n", umac_id, mdio_clk_divider, mdio_clause);
        if(err || mdio_clk_divider!=gtmv(m, 6) || mdio_clause!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t mdf_packet_counter=gtmv(m, 32);
        if(!err) ag_drv_unimac_rdp_mdf_cnt_get( umac_id, &mdf_packet_counter);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_mdf_cnt_get(%u %u)\n", umac_id, mdf_packet_counter);
    }
    {
        uint8_t diag_hi_select=gtmv(m, 6);
        uint8_t diag_lo_select=gtmv(m, 6);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_diag_sel_set(%u %u %u)\n", umac_id, diag_hi_select, diag_lo_select);
        if(!err) ag_drv_unimac_rdp_diag_sel_set(umac_id, diag_hi_select, diag_lo_select);
        if(!err) ag_drv_unimac_rdp_diag_sel_get( umac_id, &diag_hi_select, &diag_lo_select);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_rdp_diag_sel_get(%u %u %u)\n", umac_id, diag_hi_select, diag_lo_select);
        if(err || diag_hi_select!=gtmv(m, 6) || diag_lo_select!=gtmv(m, 6))
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
    case bdmf_address_umac_dummy : reg = &RU_REG(UNIMAC_RDP, UMAC_DUMMY); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_hd_bkp_cntl : reg = &RU_REG(UNIMAC_RDP, HD_BKP_CNTL); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_cmd : reg = &RU_REG(UNIMAC_RDP, CMD); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_mac0 : reg = &RU_REG(UNIMAC_RDP, MAC0); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_mac1 : reg = &RU_REG(UNIMAC_RDP, MAC1); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_frm_len : reg = &RU_REG(UNIMAC_RDP, FRM_LEN); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_pause_qunat : reg = &RU_REG(UNIMAC_RDP, PAUSE_QUNAT); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_sfd_offset : reg = &RU_REG(UNIMAC_RDP, SFD_OFFSET); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_mode : reg = &RU_REG(UNIMAC_RDP, MODE); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_frm_tag0 : reg = &RU_REG(UNIMAC_RDP, FRM_TAG0); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_frm_tag1 : reg = &RU_REG(UNIMAC_RDP, FRM_TAG1); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_tx_ipg_len : reg = &RU_REG(UNIMAC_RDP, TX_IPG_LEN); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_eee_ctrl : reg = &RU_REG(UNIMAC_RDP, EEE_CTRL); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_eee_lpi_timer : reg = &RU_REG(UNIMAC_RDP, EEE_LPI_TIMER); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_eee_wake_timer : reg = &RU_REG(UNIMAC_RDP, EEE_WAKE_TIMER); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_eee_ref_count : reg = &RU_REG(UNIMAC_RDP, EEE_REF_COUNT); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_rx_pkt_drop_status : reg = &RU_REG(UNIMAC_RDP, RX_PKT_DROP_STATUS); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_symmetric_idle_threshold : reg = &RU_REG(UNIMAC_RDP, SYMMETRIC_IDLE_THRESHOLD); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_macsec_prog_tx_crc : reg = &RU_REG(UNIMAC_RDP, MACSEC_PROG_TX_CRC); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_macsec_cntrl : reg = &RU_REG(UNIMAC_RDP, MACSEC_CNTRL); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_ts_status_cntrl : reg = &RU_REG(UNIMAC_RDP, TS_STATUS_CNTRL); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_tx_ts_data : reg = &RU_REG(UNIMAC_RDP, TX_TS_DATA); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_pause_cntrl : reg = &RU_REG(UNIMAC_RDP, PAUSE_CNTRL); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_txfifo_flush : reg = &RU_REG(UNIMAC_RDP, TXFIFO_FLUSH); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_rxfifo_stat : reg = &RU_REG(UNIMAC_RDP, RXFIFO_STAT); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_txfifo_stat : reg = &RU_REG(UNIMAC_RDP, TXFIFO_STAT); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_ppp_cntrl : reg = &RU_REG(UNIMAC_RDP, PPP_CNTRL); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_ppp_refresh_cntrl : reg = &RU_REG(UNIMAC_RDP, PPP_REFRESH_CNTRL); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_tx_pause_prel0 : reg = &RU_REG(UNIMAC_RDP, TX_PAUSE_PREL0); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_tx_pause_prel1 : reg = &RU_REG(UNIMAC_RDP, TX_PAUSE_PREL1); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_tx_pause_prel2 : reg = &RU_REG(UNIMAC_RDP, TX_PAUSE_PREL2); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_tx_pause_prel3 : reg = &RU_REG(UNIMAC_RDP, TX_PAUSE_PREL3); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_rx_pause_prel0 : reg = &RU_REG(UNIMAC_RDP, RX_PAUSE_PREL0); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_rx_pause_prel1 : reg = &RU_REG(UNIMAC_RDP, RX_PAUSE_PREL1); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_rx_pause_prel2 : reg = &RU_REG(UNIMAC_RDP, RX_PAUSE_PREL2); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_rx_pause_prel3 : reg = &RU_REG(UNIMAC_RDP, RX_PAUSE_PREL3); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_rxerr_mask : reg = &RU_REG(UNIMAC_RDP, RXERR_MASK); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_rx_max_pkt_size : reg = &RU_REG(UNIMAC_RDP, RX_MAX_PKT_SIZE); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_mdio_cmd : reg = &RU_REG(UNIMAC_RDP, MDIO_CMD); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_mdio_cfg : reg = &RU_REG(UNIMAC_RDP, MDIO_CFG); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_mdf_cnt : reg = &RU_REG(UNIMAC_RDP, MDF_CNT); blk = &RU_BLK(UNIMAC_RDP); break;
    case bdmf_address_diag_sel : reg = &RU_REG(UNIMAC_RDP, DIAG_SEL); blk = &RU_BLK(UNIMAC_RDP); break;
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
        static bdmfmon_cmd_parm_t set_hd_bkp_cntl[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("ipg_config_rx", "ipg_config_rx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("hd_fc_bkoff_ok", "hd_fc_bkoff_ok", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("hd_fc_ena", "hd_fc_ena", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_cmd[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("runt_filter_dis", "runt_filter_dis", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("txrx_en_config", "txrx_en_config", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tx_pause_ignore", "tx_pause_ignore", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("prbl_ena", "prbl_ena", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rx_err_disc", "rx_err_disc", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rmt_loop_ena", "rmt_loop_ena", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("no_lgth_check", "no_lgth_check", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("cntl_frm_ena", "cntl_frm_ena", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ena_ext_config", "ena_ext_config", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("lcl_loop_ena", "lcl_loop_ena", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("sw_reset", "sw_reset", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("hd_ena", "hd_ena", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tx_addr_ins", "tx_addr_ins", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rx_pause_ignore", "rx_pause_ignore", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pause_fwd", "pause_fwd", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("crc_fwd", "crc_fwd", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pad_en", "pad_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("promis_en", "promis_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("eth_speed", "eth_speed", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rx_ena", "rx_ena", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tx_ena", "tx_ena", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mac0[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("mac_0", "mac_0", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mac1[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("mac_1", "mac_1", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_frm_len[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("frame_length", "frame_length", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pause_qunat[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("pause_quant", "pause_quant", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_frm_tag0[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("outer_tag", "outer_tag", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_frm_tag1[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("inner_tag", "inner_tag", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_tx_ipg_len[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("tx_ipg_len", "tx_ipg_len", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_eee_ctrl[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("lp_idle_prediction_mode", "lp_idle_prediction_mode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("dis_eee_10m", "dis_eee_10m", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("eee_txclk_dis", "eee_txclk_dis", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rx_fifo_check", "rx_fifo_check", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("eee_en", "eee_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("en_lpi_tx_pause", "en_lpi_tx_pause", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("en_lpi_tx_pfc", "en_lpi_tx_pfc", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("en_lpi_rx_pause", "en_lpi_rx_pause", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_eee_lpi_timer[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("eee_lpi_timer", "eee_lpi_timer", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_eee_wake_timer[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("eee_wake_timer", "eee_wake_timer", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_eee_ref_count[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("eee_reference_count", "eee_reference_count", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_pkt_drop_status[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("rx_ipg_invalid", "rx_ipg_invalid", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_symmetric_idle_threshold[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("threshold_value", "threshold_value", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_macsec_prog_tx_crc[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("macsec_prog_tx_crc", "macsec_prog_tx_crc", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_macsec_cntrl[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("tx_crc_program", "tx_crc_program", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tx_crc_corrupt_en", "tx_crc_corrupt_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("tx_lanuch_en", "tx_lanuch_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_pause_cntrl[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("pause_control_en", "pause_control_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pause_timer", "pause_timer", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_txfifo_flush[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("tx_flush", "tx_flush", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ppp_cntrl[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("pfc_stats_en", "pfc_stats_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rx_pass_pfc_frm", "rx_pass_pfc_frm", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("force_ppp_xon", "force_ppp_xon", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ppp_en_rx", "ppp_en_rx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ppp_en_tx", "ppp_en_tx", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_ppp_refresh_cntrl[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("ppp_refresh_timer", "ppp_refresh_timer", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ppp_refresh_en", "ppp_refresh_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rxerr_mask[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("mac_rxerr_mask", "mac_rxerr_mask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_rx_max_pkt_size[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("max_pkt_size", "max_pkt_size", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mdio_cmd[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("mdio_busy", "mdio_busy", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fail", "fail", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("op_code", "op_code", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("phy_prt_addr", "phy_prt_addr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("reg_dec_addr", "reg_dec_addr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("data_addr", "data_addr", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_mdio_cfg[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("mdio_clk_divider", "mdio_clk_divider", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mdio_clause", "mdio_clause", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_diag_sel[]={
            BDMFMON_MAKE_PARM_ENUM("umac_id", "umac_id", umac_id_enum_table, 0),
            BDMFMON_MAKE_PARM("diag_hi_select", "diag_hi_select", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("diag_lo_select", "diag_lo_select", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="hd_bkp_cntl", .val=cli_unimac_rdp_hd_bkp_cntl, .parms=set_hd_bkp_cntl },
            { .name="cmd", .val=cli_unimac_rdp_cmd, .parms=set_cmd },
            { .name="mac0", .val=cli_unimac_rdp_mac0, .parms=set_mac0 },
            { .name="mac1", .val=cli_unimac_rdp_mac1, .parms=set_mac1 },
            { .name="frm_len", .val=cli_unimac_rdp_frm_len, .parms=set_frm_len },
            { .name="pause_qunat", .val=cli_unimac_rdp_pause_qunat, .parms=set_pause_qunat },
            { .name="frm_tag0", .val=cli_unimac_rdp_frm_tag0, .parms=set_frm_tag0 },
            { .name="frm_tag1", .val=cli_unimac_rdp_frm_tag1, .parms=set_frm_tag1 },
            { .name="tx_ipg_len", .val=cli_unimac_rdp_tx_ipg_len, .parms=set_tx_ipg_len },
            { .name="eee_ctrl", .val=cli_unimac_rdp_eee_ctrl, .parms=set_eee_ctrl },
            { .name="eee_lpi_timer", .val=cli_unimac_rdp_eee_lpi_timer, .parms=set_eee_lpi_timer },
            { .name="eee_wake_timer", .val=cli_unimac_rdp_eee_wake_timer, .parms=set_eee_wake_timer },
            { .name="eee_ref_count", .val=cli_unimac_rdp_eee_ref_count, .parms=set_eee_ref_count },
            { .name="rx_pkt_drop_status", .val=cli_unimac_rdp_rx_pkt_drop_status, .parms=set_rx_pkt_drop_status },
            { .name="symmetric_idle_threshold", .val=cli_unimac_rdp_symmetric_idle_threshold, .parms=set_symmetric_idle_threshold },
            { .name="macsec_prog_tx_crc", .val=cli_unimac_rdp_macsec_prog_tx_crc, .parms=set_macsec_prog_tx_crc },
            { .name="macsec_cntrl", .val=cli_unimac_rdp_macsec_cntrl, .parms=set_macsec_cntrl },
            { .name="pause_cntrl", .val=cli_unimac_rdp_pause_cntrl, .parms=set_pause_cntrl },
            { .name="txfifo_flush", .val=cli_unimac_rdp_txfifo_flush, .parms=set_txfifo_flush },
            { .name="ppp_cntrl", .val=cli_unimac_rdp_ppp_cntrl, .parms=set_ppp_cntrl },
            { .name="ppp_refresh_cntrl", .val=cli_unimac_rdp_ppp_refresh_cntrl, .parms=set_ppp_refresh_cntrl },
            { .name="rxerr_mask", .val=cli_unimac_rdp_rxerr_mask, .parms=set_rxerr_mask },
            { .name="rx_max_pkt_size", .val=cli_unimac_rdp_rx_max_pkt_size, .parms=set_rx_max_pkt_size },
            { .name="mdio_cmd", .val=cli_unimac_rdp_mdio_cmd, .parms=set_mdio_cmd },
            { .name="mdio_cfg", .val=cli_unimac_rdp_mdio_cfg, .parms=set_mdio_cfg },
            { .name="diag_sel", .val=cli_unimac_rdp_diag_sel, .parms=set_diag_sel },
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
            { .name="umac_dummy", .val=cli_unimac_rdp_umac_dummy, .parms=set_default },
            { .name="hd_bkp_cntl", .val=cli_unimac_rdp_hd_bkp_cntl, .parms=set_default },
            { .name="cmd", .val=cli_unimac_rdp_cmd, .parms=set_default },
            { .name="mac0", .val=cli_unimac_rdp_mac0, .parms=set_default },
            { .name="mac1", .val=cli_unimac_rdp_mac1, .parms=set_default },
            { .name="frm_len", .val=cli_unimac_rdp_frm_len, .parms=set_default },
            { .name="pause_qunat", .val=cli_unimac_rdp_pause_qunat, .parms=set_default },
            { .name="sfd_offset", .val=cli_unimac_rdp_sfd_offset, .parms=set_default },
            { .name="mode", .val=cli_unimac_rdp_mode, .parms=set_default },
            { .name="frm_tag0", .val=cli_unimac_rdp_frm_tag0, .parms=set_default },
            { .name="frm_tag1", .val=cli_unimac_rdp_frm_tag1, .parms=set_default },
            { .name="tx_ipg_len", .val=cli_unimac_rdp_tx_ipg_len, .parms=set_default },
            { .name="eee_ctrl", .val=cli_unimac_rdp_eee_ctrl, .parms=set_default },
            { .name="eee_lpi_timer", .val=cli_unimac_rdp_eee_lpi_timer, .parms=set_default },
            { .name="eee_wake_timer", .val=cli_unimac_rdp_eee_wake_timer, .parms=set_default },
            { .name="eee_ref_count", .val=cli_unimac_rdp_eee_ref_count, .parms=set_default },
            { .name="rx_pkt_drop_status", .val=cli_unimac_rdp_rx_pkt_drop_status, .parms=set_default },
            { .name="symmetric_idle_threshold", .val=cli_unimac_rdp_symmetric_idle_threshold, .parms=set_default },
            { .name="macsec_prog_tx_crc", .val=cli_unimac_rdp_macsec_prog_tx_crc, .parms=set_default },
            { .name="macsec_cntrl", .val=cli_unimac_rdp_macsec_cntrl, .parms=set_default },
            { .name="ts_status_cntrl", .val=cli_unimac_rdp_ts_status_cntrl, .parms=set_default },
            { .name="tx_ts_data", .val=cli_unimac_rdp_tx_ts_data, .parms=set_default },
            { .name="pause_cntrl", .val=cli_unimac_rdp_pause_cntrl, .parms=set_default },
            { .name="txfifo_flush", .val=cli_unimac_rdp_txfifo_flush, .parms=set_default },
            { .name="rxfifo_stat", .val=cli_unimac_rdp_rxfifo_stat, .parms=set_default },
            { .name="txfifo_stat", .val=cli_unimac_rdp_txfifo_stat, .parms=set_default },
            { .name="ppp_cntrl", .val=cli_unimac_rdp_ppp_cntrl, .parms=set_default },
            { .name="ppp_refresh_cntrl", .val=cli_unimac_rdp_ppp_refresh_cntrl, .parms=set_default },
            { .name="tx_pause_prel0", .val=cli_unimac_rdp_tx_pause_prel0, .parms=set_default },
            { .name="tx_pause_prel1", .val=cli_unimac_rdp_tx_pause_prel1, .parms=set_default },
            { .name="tx_pause_prel2", .val=cli_unimac_rdp_tx_pause_prel2, .parms=set_default },
            { .name="tx_pause_prel3", .val=cli_unimac_rdp_tx_pause_prel3, .parms=set_default },
            { .name="rx_pause_prel0", .val=cli_unimac_rdp_rx_pause_prel0, .parms=set_default },
            { .name="rx_pause_prel1", .val=cli_unimac_rdp_rx_pause_prel1, .parms=set_default },
            { .name="rx_pause_prel2", .val=cli_unimac_rdp_rx_pause_prel2, .parms=set_default },
            { .name="rx_pause_prel3", .val=cli_unimac_rdp_rx_pause_prel3, .parms=set_default },
            { .name="rxerr_mask", .val=cli_unimac_rdp_rxerr_mask, .parms=set_default },
            { .name="rx_max_pkt_size", .val=cli_unimac_rdp_rx_max_pkt_size, .parms=set_default },
            { .name="mdio_cmd", .val=cli_unimac_rdp_mdio_cmd, .parms=set_default },
            { .name="mdio_cfg", .val=cli_unimac_rdp_mdio_cfg, .parms=set_default },
            { .name="mdf_cnt", .val=cli_unimac_rdp_mdf_cnt, .parms=set_default },
            { .name="diag_sel", .val=cli_unimac_rdp_diag_sel, .parms=set_default },
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
            { .name="UMAC_DUMMY" , .val=bdmf_address_umac_dummy },
            { .name="HD_BKP_CNTL" , .val=bdmf_address_hd_bkp_cntl },
            { .name="CMD" , .val=bdmf_address_cmd },
            { .name="MAC0" , .val=bdmf_address_mac0 },
            { .name="MAC1" , .val=bdmf_address_mac1 },
            { .name="FRM_LEN" , .val=bdmf_address_frm_len },
            { .name="PAUSE_QUNAT" , .val=bdmf_address_pause_qunat },
            { .name="SFD_OFFSET" , .val=bdmf_address_sfd_offset },
            { .name="MODE" , .val=bdmf_address_mode },
            { .name="FRM_TAG0" , .val=bdmf_address_frm_tag0 },
            { .name="FRM_TAG1" , .val=bdmf_address_frm_tag1 },
            { .name="TX_IPG_LEN" , .val=bdmf_address_tx_ipg_len },
            { .name="EEE_CTRL" , .val=bdmf_address_eee_ctrl },
            { .name="EEE_LPI_TIMER" , .val=bdmf_address_eee_lpi_timer },
            { .name="EEE_WAKE_TIMER" , .val=bdmf_address_eee_wake_timer },
            { .name="EEE_REF_COUNT" , .val=bdmf_address_eee_ref_count },
            { .name="RX_PKT_DROP_STATUS" , .val=bdmf_address_rx_pkt_drop_status },
            { .name="SYMMETRIC_IDLE_THRESHOLD" , .val=bdmf_address_symmetric_idle_threshold },
            { .name="MACSEC_PROG_TX_CRC" , .val=bdmf_address_macsec_prog_tx_crc },
            { .name="MACSEC_CNTRL" , .val=bdmf_address_macsec_cntrl },
            { .name="TS_STATUS_CNTRL" , .val=bdmf_address_ts_status_cntrl },
            { .name="TX_TS_DATA" , .val=bdmf_address_tx_ts_data },
            { .name="PAUSE_CNTRL" , .val=bdmf_address_pause_cntrl },
            { .name="TXFIFO_FLUSH" , .val=bdmf_address_txfifo_flush },
            { .name="RXFIFO_STAT" , .val=bdmf_address_rxfifo_stat },
            { .name="TXFIFO_STAT" , .val=bdmf_address_txfifo_stat },
            { .name="PPP_CNTRL" , .val=bdmf_address_ppp_cntrl },
            { .name="PPP_REFRESH_CNTRL" , .val=bdmf_address_ppp_refresh_cntrl },
            { .name="TX_PAUSE_PREL0" , .val=bdmf_address_tx_pause_prel0 },
            { .name="TX_PAUSE_PREL1" , .val=bdmf_address_tx_pause_prel1 },
            { .name="TX_PAUSE_PREL2" , .val=bdmf_address_tx_pause_prel2 },
            { .name="TX_PAUSE_PREL3" , .val=bdmf_address_tx_pause_prel3 },
            { .name="RX_PAUSE_PREL0" , .val=bdmf_address_rx_pause_prel0 },
            { .name="RX_PAUSE_PREL1" , .val=bdmf_address_rx_pause_prel1 },
            { .name="RX_PAUSE_PREL2" , .val=bdmf_address_rx_pause_prel2 },
            { .name="RX_PAUSE_PREL3" , .val=bdmf_address_rx_pause_prel3 },
            { .name="RXERR_MASK" , .val=bdmf_address_rxerr_mask },
            { .name="RX_MAX_PKT_SIZE" , .val=bdmf_address_rx_max_pkt_size },
            { .name="MDIO_CMD" , .val=bdmf_address_mdio_cmd },
            { .name="MDIO_CFG" , .val=bdmf_address_mdio_cfg },
            { .name="MDF_CNT" , .val=bdmf_address_mdf_cnt },
            { .name="DIAG_SEL" , .val=bdmf_address_diag_sel },
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

