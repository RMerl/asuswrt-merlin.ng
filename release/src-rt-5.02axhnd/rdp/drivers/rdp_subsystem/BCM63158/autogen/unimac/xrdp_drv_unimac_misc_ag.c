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
#include "xrdp_drv_unimac_misc_ag.h"

#define BLOCK_ADDR_COUNT_BITS 2
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_cfg_set(uint8_t umac_misc_id, const unimac_misc_unimac_top_unimac_misc_unimac_cfg *unimac_top_unimac_misc_unimac_cfg)
{
    uint32_t reg_unimac_top_unimac_misc_unimac_cfg=0;

#ifdef VALIDATE_PARMS
    if(!unimac_top_unimac_misc_unimac_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_misc_id >= BLOCK_ADDR_COUNT) ||
       (unimac_top_unimac_misc_unimac_cfg->direct_gmii >= _1BITS_MAX_VAL_) ||
       (unimac_top_unimac_misc_unimac_cfg->gport_mode >= _1BITS_MAX_VAL_) ||
       (unimac_top_unimac_misc_unimac_cfg->ss_mode_mii >= _1BITS_MAX_VAL_) ||
       (unimac_top_unimac_misc_unimac_cfg->txcrcer >= _1BITS_MAX_VAL_) ||
       (unimac_top_unimac_misc_unimac_cfg->mac_crc_fwd >= _1BITS_MAX_VAL_) ||
       (unimac_top_unimac_misc_unimac_cfg->mac_crc_owrt >= _1BITS_MAX_VAL_) ||
       (unimac_top_unimac_misc_unimac_cfg->ext_tx_flow_control >= _1BITS_MAX_VAL_) ||
       (unimac_top_unimac_misc_unimac_cfg->launch_enable >= _1BITS_MAX_VAL_) ||
       (unimac_top_unimac_misc_unimac_cfg->pp_gen >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_unimac_top_unimac_misc_unimac_cfg = RU_FIELD_SET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG, DIRECT_GMII, reg_unimac_top_unimac_misc_unimac_cfg, unimac_top_unimac_misc_unimac_cfg->direct_gmii);
    reg_unimac_top_unimac_misc_unimac_cfg = RU_FIELD_SET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG, GPORT_MODE, reg_unimac_top_unimac_misc_unimac_cfg, unimac_top_unimac_misc_unimac_cfg->gport_mode);
    reg_unimac_top_unimac_misc_unimac_cfg = RU_FIELD_SET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG, SS_MODE_MII, reg_unimac_top_unimac_misc_unimac_cfg, unimac_top_unimac_misc_unimac_cfg->ss_mode_mii);
    reg_unimac_top_unimac_misc_unimac_cfg = RU_FIELD_SET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG, TXCRCER, reg_unimac_top_unimac_misc_unimac_cfg, unimac_top_unimac_misc_unimac_cfg->txcrcer);
    reg_unimac_top_unimac_misc_unimac_cfg = RU_FIELD_SET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG, MAC_CRC_FWD, reg_unimac_top_unimac_misc_unimac_cfg, unimac_top_unimac_misc_unimac_cfg->mac_crc_fwd);
    reg_unimac_top_unimac_misc_unimac_cfg = RU_FIELD_SET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG, MAC_CRC_OWRT, reg_unimac_top_unimac_misc_unimac_cfg, unimac_top_unimac_misc_unimac_cfg->mac_crc_owrt);
    reg_unimac_top_unimac_misc_unimac_cfg = RU_FIELD_SET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG, EXT_TX_FLOW_CONTROL, reg_unimac_top_unimac_misc_unimac_cfg, unimac_top_unimac_misc_unimac_cfg->ext_tx_flow_control);
    reg_unimac_top_unimac_misc_unimac_cfg = RU_FIELD_SET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG, LAUNCH_ENABLE, reg_unimac_top_unimac_misc_unimac_cfg, unimac_top_unimac_misc_unimac_cfg->launch_enable);
    reg_unimac_top_unimac_misc_unimac_cfg = RU_FIELD_SET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG, PP_PSE_EN, reg_unimac_top_unimac_misc_unimac_cfg, unimac_top_unimac_misc_unimac_cfg->pp_pse_en);
    reg_unimac_top_unimac_misc_unimac_cfg = RU_FIELD_SET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG, PP_GEN, reg_unimac_top_unimac_misc_unimac_cfg, unimac_top_unimac_misc_unimac_cfg->pp_gen);

    RU_REG_WRITE(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG, reg_unimac_top_unimac_misc_unimac_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_cfg_get(uint8_t umac_misc_id, unimac_misc_unimac_top_unimac_misc_unimac_cfg *unimac_top_unimac_misc_unimac_cfg)
{
    uint32_t reg_unimac_top_unimac_misc_unimac_cfg;

#ifdef VALIDATE_PARMS
    if(!unimac_top_unimac_misc_unimac_cfg)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_misc_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG, reg_unimac_top_unimac_misc_unimac_cfg);

    unimac_top_unimac_misc_unimac_cfg->direct_gmii = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG, DIRECT_GMII, reg_unimac_top_unimac_misc_unimac_cfg);
    unimac_top_unimac_misc_unimac_cfg->gport_mode = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG, GPORT_MODE, reg_unimac_top_unimac_misc_unimac_cfg);
    unimac_top_unimac_misc_unimac_cfg->ss_mode_mii = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG, SS_MODE_MII, reg_unimac_top_unimac_misc_unimac_cfg);
    unimac_top_unimac_misc_unimac_cfg->txcrcer = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG, TXCRCER, reg_unimac_top_unimac_misc_unimac_cfg);
    unimac_top_unimac_misc_unimac_cfg->mac_crc_fwd = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG, MAC_CRC_FWD, reg_unimac_top_unimac_misc_unimac_cfg);
    unimac_top_unimac_misc_unimac_cfg->mac_crc_owrt = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG, MAC_CRC_OWRT, reg_unimac_top_unimac_misc_unimac_cfg);
    unimac_top_unimac_misc_unimac_cfg->ext_tx_flow_control = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG, EXT_TX_FLOW_CONTROL, reg_unimac_top_unimac_misc_unimac_cfg);
    unimac_top_unimac_misc_unimac_cfg->launch_enable = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG, LAUNCH_ENABLE, reg_unimac_top_unimac_misc_unimac_cfg);
    unimac_top_unimac_misc_unimac_cfg->pp_pse_en = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG, PP_PSE_EN, reg_unimac_top_unimac_misc_unimac_cfg);
    unimac_top_unimac_misc_unimac_cfg->pp_gen = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG, PP_GEN, reg_unimac_top_unimac_misc_unimac_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg1_set(uint8_t umac_misc_id, uint16_t max_pkt_size, uint16_t rxfifo_congestion_threshold)
{
    uint32_t reg_unimac_top_unimac_misc_unimac_ext_cfg1=0;

#ifdef VALIDATE_PARMS
    if((umac_misc_id >= BLOCK_ADDR_COUNT) ||
       (max_pkt_size >= _14BITS_MAX_VAL_) ||
       (rxfifo_congestion_threshold >= _9BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_unimac_top_unimac_misc_unimac_ext_cfg1 = RU_FIELD_SET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1, MAX_PKT_SIZE, reg_unimac_top_unimac_misc_unimac_ext_cfg1, max_pkt_size);
    reg_unimac_top_unimac_misc_unimac_ext_cfg1 = RU_FIELD_SET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1, RXFIFO_CONGESTION_THRESHOLD, reg_unimac_top_unimac_misc_unimac_ext_cfg1, rxfifo_congestion_threshold);

    RU_REG_WRITE(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1, reg_unimac_top_unimac_misc_unimac_ext_cfg1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg1_get(uint8_t umac_misc_id, uint16_t *max_pkt_size, uint16_t *rxfifo_congestion_threshold)
{
    uint32_t reg_unimac_top_unimac_misc_unimac_ext_cfg1;

#ifdef VALIDATE_PARMS
    if(!max_pkt_size || !rxfifo_congestion_threshold)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_misc_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1, reg_unimac_top_unimac_misc_unimac_ext_cfg1);

    *max_pkt_size = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1, MAX_PKT_SIZE, reg_unimac_top_unimac_misc_unimac_ext_cfg1);
    *rxfifo_congestion_threshold = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1, RXFIFO_CONGESTION_THRESHOLD, reg_unimac_top_unimac_misc_unimac_ext_cfg1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2_set(uint8_t umac_misc_id, const unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2 *unimac_top_unimac_misc_unimac_ext_cfg2)
{
    uint32_t reg_unimac_top_unimac_misc_unimac_ext_cfg2=0;

#ifdef VALIDATE_PARMS
    if(!unimac_top_unimac_misc_unimac_ext_cfg2)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_misc_id >= BLOCK_ADDR_COUNT) ||
       (unimac_top_unimac_misc_unimac_ext_cfg2->rxfifo_pause_threshold >= _9BITS_MAX_VAL_) ||
       (unimac_top_unimac_misc_unimac_ext_cfg2->backpressure_enable_int >= _1BITS_MAX_VAL_) ||
       (unimac_top_unimac_misc_unimac_ext_cfg2->backpressure_enable_ext >= _1BITS_MAX_VAL_) ||
       (unimac_top_unimac_misc_unimac_ext_cfg2->fifo_overrun_ctl_en >= _1BITS_MAX_VAL_) ||
       (unimac_top_unimac_misc_unimac_ext_cfg2->remote_loopback_en >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_unimac_top_unimac_misc_unimac_ext_cfg2 = RU_FIELD_SET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2, RXFIFO_PAUSE_THRESHOLD, reg_unimac_top_unimac_misc_unimac_ext_cfg2, unimac_top_unimac_misc_unimac_ext_cfg2->rxfifo_pause_threshold);
    reg_unimac_top_unimac_misc_unimac_ext_cfg2 = RU_FIELD_SET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2, BACKPRESSURE_ENABLE_INT, reg_unimac_top_unimac_misc_unimac_ext_cfg2, unimac_top_unimac_misc_unimac_ext_cfg2->backpressure_enable_int);
    reg_unimac_top_unimac_misc_unimac_ext_cfg2 = RU_FIELD_SET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2, BACKPRESSURE_ENABLE_EXT, reg_unimac_top_unimac_misc_unimac_ext_cfg2, unimac_top_unimac_misc_unimac_ext_cfg2->backpressure_enable_ext);
    reg_unimac_top_unimac_misc_unimac_ext_cfg2 = RU_FIELD_SET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2, FIFO_OVERRUN_CTL_EN, reg_unimac_top_unimac_misc_unimac_ext_cfg2, unimac_top_unimac_misc_unimac_ext_cfg2->fifo_overrun_ctl_en);
    reg_unimac_top_unimac_misc_unimac_ext_cfg2 = RU_FIELD_SET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2, REMOTE_LOOPBACK_EN, reg_unimac_top_unimac_misc_unimac_ext_cfg2, unimac_top_unimac_misc_unimac_ext_cfg2->remote_loopback_en);

    RU_REG_WRITE(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2, reg_unimac_top_unimac_misc_unimac_ext_cfg2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2_get(uint8_t umac_misc_id, unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2 *unimac_top_unimac_misc_unimac_ext_cfg2)
{
    uint32_t reg_unimac_top_unimac_misc_unimac_ext_cfg2;

#ifdef VALIDATE_PARMS
    if(!unimac_top_unimac_misc_unimac_ext_cfg2)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_misc_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2, reg_unimac_top_unimac_misc_unimac_ext_cfg2);

    unimac_top_unimac_misc_unimac_ext_cfg2->rxfifo_pause_threshold = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2, RXFIFO_PAUSE_THRESHOLD, reg_unimac_top_unimac_misc_unimac_ext_cfg2);
    unimac_top_unimac_misc_unimac_ext_cfg2->backpressure_enable_int = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2, BACKPRESSURE_ENABLE_INT, reg_unimac_top_unimac_misc_unimac_ext_cfg2);
    unimac_top_unimac_misc_unimac_ext_cfg2->backpressure_enable_ext = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2, BACKPRESSURE_ENABLE_EXT, reg_unimac_top_unimac_misc_unimac_ext_cfg2);
    unimac_top_unimac_misc_unimac_ext_cfg2->fifo_overrun_ctl_en = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2, FIFO_OVERRUN_CTL_EN, reg_unimac_top_unimac_misc_unimac_ext_cfg2);
    unimac_top_unimac_misc_unimac_ext_cfg2->remote_loopback_en = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2, REMOTE_LOOPBACK_EN, reg_unimac_top_unimac_misc_unimac_ext_cfg2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_stat_update_mask_set(uint8_t umac_misc_id, uint32_t gport_stat_update_mask)
{
    uint32_t reg_unimac_top_unimac_misc_unimac_stat_update_mask=0;

#ifdef VALIDATE_PARMS
    if((umac_misc_id >= BLOCK_ADDR_COUNT) ||
       (gport_stat_update_mask >= _18BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_unimac_top_unimac_misc_unimac_stat_update_mask = RU_FIELD_SET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_UPDATE_MASK, GPORT_STAT_UPDATE_MASK, reg_unimac_top_unimac_misc_unimac_stat_update_mask, gport_stat_update_mask);

    RU_REG_WRITE(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_UPDATE_MASK, reg_unimac_top_unimac_misc_unimac_stat_update_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_stat_update_mask_get(uint8_t umac_misc_id, uint32_t *gport_stat_update_mask)
{
    uint32_t reg_unimac_top_unimac_misc_unimac_stat_update_mask;

#ifdef VALIDATE_PARMS
    if(!gport_stat_update_mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_misc_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_UPDATE_MASK, reg_unimac_top_unimac_misc_unimac_stat_update_mask);

    *gport_stat_update_mask = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_UPDATE_MASK, GPORT_STAT_UPDATE_MASK, reg_unimac_top_unimac_misc_unimac_stat_update_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_stat_get(uint8_t umac_misc_id, unimac_misc_unimac_top_unimac_misc_unimac_stat *unimac_top_unimac_misc_unimac_stat)
{
    uint32_t reg_unimac_top_unimac_misc_unimac_stat;

#ifdef VALIDATE_PARMS
    if(!unimac_top_unimac_misc_unimac_stat)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_misc_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT, reg_unimac_top_unimac_misc_unimac_stat);

    unimac_top_unimac_misc_unimac_stat->auto_config_en = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT, AUTO_CONFIG_EN, reg_unimac_top_unimac_misc_unimac_stat);
    unimac_top_unimac_misc_unimac_stat->eth_duplex = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT, ETH_DUPLEX, reg_unimac_top_unimac_misc_unimac_stat);
    unimac_top_unimac_misc_unimac_stat->eth_speed = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT, ETH_SPEED, reg_unimac_top_unimac_misc_unimac_stat);
    unimac_top_unimac_misc_unimac_stat->rx_fifo_dat_avl = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT, RX_FIFO_DAT_AVL, reg_unimac_top_unimac_misc_unimac_stat);
    unimac_top_unimac_misc_unimac_stat->mac_in_pause = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT, MAC_IN_PAUSE, reg_unimac_top_unimac_misc_unimac_stat);
    unimac_top_unimac_misc_unimac_stat->mac_tx_empty = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT, MAC_TX_EMPTY, reg_unimac_top_unimac_misc_unimac_stat);
    unimac_top_unimac_misc_unimac_stat->launch_ack = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT, LAUNCH_ACK, reg_unimac_top_unimac_misc_unimac_stat);
    unimac_top_unimac_misc_unimac_stat->lpi_tx_detect = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT, LPI_TX_DETECT, reg_unimac_top_unimac_misc_unimac_stat);
    unimac_top_unimac_misc_unimac_stat->lpi_rx_detect = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT, LPI_RX_DETECT, reg_unimac_top_unimac_misc_unimac_stat);
    unimac_top_unimac_misc_unimac_stat->rx_sop_out = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT, RX_SOP_OUT, reg_unimac_top_unimac_misc_unimac_stat);
    unimac_top_unimac_misc_unimac_stat->rx_sop_delete = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT, RX_SOP_DELETE, reg_unimac_top_unimac_misc_unimac_stat);
    unimac_top_unimac_misc_unimac_stat->mac_stats_update = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT, MAC_STATS_UPDATE, reg_unimac_top_unimac_misc_unimac_stat);
    unimac_top_unimac_misc_unimac_stat->pp_stats = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT, PP_STATS, reg_unimac_top_unimac_misc_unimac_stat);
    unimac_top_unimac_misc_unimac_stat->pp_stats_valid = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT, PP_STATS_VALID, reg_unimac_top_unimac_misc_unimac_stat);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_debug_set(uint8_t umac_misc_id, uint8_t debug_sel)
{
    uint32_t reg_unimac_top_unimac_misc_unimac_debug=0;

#ifdef VALIDATE_PARMS
    if((umac_misc_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_unimac_top_unimac_misc_unimac_debug = RU_FIELD_SET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG, DEBUG_SEL, reg_unimac_top_unimac_misc_unimac_debug, debug_sel);

    RU_REG_WRITE(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG, reg_unimac_top_unimac_misc_unimac_debug);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_debug_get(uint8_t umac_misc_id, uint8_t *debug_sel)
{
    uint32_t reg_unimac_top_unimac_misc_unimac_debug;

#ifdef VALIDATE_PARMS
    if(!debug_sel)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_misc_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG, reg_unimac_top_unimac_misc_unimac_debug);

    *debug_sel = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG, DEBUG_SEL, reg_unimac_top_unimac_misc_unimac_debug);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_rst_set(uint8_t umac_misc_id, bdmf_boolean unimac_rst)
{
    uint32_t reg_unimac_top_unimac_misc_unimac_rst=0;

#ifdef VALIDATE_PARMS
    if((umac_misc_id >= BLOCK_ADDR_COUNT) ||
       (unimac_rst >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_unimac_top_unimac_misc_unimac_rst = RU_FIELD_SET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST, UNIMAC_RST, reg_unimac_top_unimac_misc_unimac_rst, unimac_rst);

    RU_REG_WRITE(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST, reg_unimac_top_unimac_misc_unimac_rst);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_rst_get(uint8_t umac_misc_id, bdmf_boolean *unimac_rst)
{
    uint32_t reg_unimac_top_unimac_misc_unimac_rst;

#ifdef VALIDATE_PARMS
    if(!unimac_rst)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_misc_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST, reg_unimac_top_unimac_misc_unimac_rst);

    *unimac_rst = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST, UNIMAC_RST, reg_unimac_top_unimac_misc_unimac_rst);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_rsv_mask_set(uint8_t umac_misc_id, uint32_t gport_rsv_mask)
{
    uint32_t reg_unimac_top_unimac_misc_unimac_rsv_mask=0;

#ifdef VALIDATE_PARMS
    if((umac_misc_id >= BLOCK_ADDR_COUNT) ||
       (gport_rsv_mask >= _18BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_unimac_top_unimac_misc_unimac_rsv_mask = RU_FIELD_SET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RSV_MASK, GPORT_RSV_MASK, reg_unimac_top_unimac_misc_unimac_rsv_mask, gport_rsv_mask);

    RU_REG_WRITE(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RSV_MASK, reg_unimac_top_unimac_misc_unimac_rsv_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_rsv_mask_get(uint8_t umac_misc_id, uint32_t *gport_rsv_mask)
{
    uint32_t reg_unimac_top_unimac_misc_unimac_rsv_mask;

#ifdef VALIDATE_PARMS
    if(!gport_rsv_mask)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_misc_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RSV_MASK, reg_unimac_top_unimac_misc_unimac_rsv_mask);

    *gport_rsv_mask = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RSV_MASK, GPORT_RSV_MASK, reg_unimac_top_unimac_misc_unimac_rsv_mask);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_overrun_counter_get(uint8_t umac_misc_id, uint32_t *overrun_counter)
{
    uint32_t reg_unimac_top_unimac_misc_unimac_overrun_counter;

#ifdef VALIDATE_PARMS
    if(!overrun_counter)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_misc_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER, reg_unimac_top_unimac_misc_unimac_overrun_counter);

    *overrun_counter = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER, OVERRUN_COUNTER, reg_unimac_top_unimac_misc_unimac_overrun_counter);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_1588_set(uint8_t umac_misc_id, bdmf_boolean tsi_sign_ext, bdmf_boolean osts_timer_dis, bdmf_boolean egr_1588_ts_mode)
{
    uint32_t reg_unimac_top_unimac_misc_unimac_1588=0;

#ifdef VALIDATE_PARMS
    if((umac_misc_id >= BLOCK_ADDR_COUNT) ||
       (tsi_sign_ext >= _1BITS_MAX_VAL_) ||
       (osts_timer_dis >= _1BITS_MAX_VAL_) ||
       (egr_1588_ts_mode >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_unimac_top_unimac_misc_unimac_1588 = RU_FIELD_SET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588, TSI_SIGN_EXT, reg_unimac_top_unimac_misc_unimac_1588, tsi_sign_ext);
    reg_unimac_top_unimac_misc_unimac_1588 = RU_FIELD_SET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588, OSTS_TIMER_DIS, reg_unimac_top_unimac_misc_unimac_1588, osts_timer_dis);
    reg_unimac_top_unimac_misc_unimac_1588 = RU_FIELD_SET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588, EGR_1588_TS_MODE, reg_unimac_top_unimac_misc_unimac_1588, egr_1588_ts_mode);

    RU_REG_WRITE(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588, reg_unimac_top_unimac_misc_unimac_1588);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_1588_get(uint8_t umac_misc_id, bdmf_boolean *tsi_sign_ext, bdmf_boolean *osts_timer_dis, bdmf_boolean *egr_1588_ts_mode)
{
    uint32_t reg_unimac_top_unimac_misc_unimac_1588;

#ifdef VALIDATE_PARMS
    if(!tsi_sign_ext || !osts_timer_dis || !egr_1588_ts_mode)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_misc_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588, reg_unimac_top_unimac_misc_unimac_1588);

    *tsi_sign_ext = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588, TSI_SIGN_EXT, reg_unimac_top_unimac_misc_unimac_1588);
    *osts_timer_dis = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588, OSTS_TIMER_DIS, reg_unimac_top_unimac_misc_unimac_1588);
    *egr_1588_ts_mode = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588, EGR_1588_TS_MODE, reg_unimac_top_unimac_misc_unimac_1588);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_ints_isr_set(uint8_t umac_misc_id, bdmf_boolean gen_int)
{
    uint32_t reg_unimac_top_unimac_ints_isr=0;

#ifdef VALIDATE_PARMS
    if((umac_misc_id >= BLOCK_ADDR_COUNT) ||
       (gen_int >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_unimac_top_unimac_ints_isr = RU_FIELD_SET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_INTS_ISR, GEN_INT, reg_unimac_top_unimac_ints_isr, gen_int);

    RU_REG_WRITE(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_INTS_ISR, reg_unimac_top_unimac_ints_isr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_ints_isr_get(uint8_t umac_misc_id, bdmf_boolean *gen_int)
{
    uint32_t reg_unimac_top_unimac_ints_isr;

#ifdef VALIDATE_PARMS
    if(!gen_int)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_misc_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_INTS_ISR, reg_unimac_top_unimac_ints_isr);

    *gen_int = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_INTS_ISR, GEN_INT, reg_unimac_top_unimac_ints_isr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_ints_ier_set(uint8_t umac_misc_id, uint32_t value)
{
    uint32_t reg_unimac_top_unimac_ints_ier=0;

#ifdef VALIDATE_PARMS
    if((umac_misc_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_unimac_top_unimac_ints_ier = RU_FIELD_SET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_INTS_IER, VALUE, reg_unimac_top_unimac_ints_ier, value);

    RU_REG_WRITE(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_INTS_IER, reg_unimac_top_unimac_ints_ier);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_ints_ier_get(uint8_t umac_misc_id, uint32_t *value)
{
    uint32_t reg_unimac_top_unimac_ints_ier;

#ifdef VALIDATE_PARMS
    if(!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_misc_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_INTS_IER, reg_unimac_top_unimac_ints_ier);

    *value = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_INTS_IER, VALUE, reg_unimac_top_unimac_ints_ier);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_ints_itr_set(uint8_t umac_misc_id, uint32_t value)
{
    uint32_t reg_unimac_top_unimac_ints_itr=0;

#ifdef VALIDATE_PARMS
    if((umac_misc_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_unimac_top_unimac_ints_itr = RU_FIELD_SET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_INTS_ITR, VALUE, reg_unimac_top_unimac_ints_itr, value);

    RU_REG_WRITE(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_INTS_ITR, reg_unimac_top_unimac_ints_itr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_ints_itr_get(uint8_t umac_misc_id, uint32_t *value)
{
    uint32_t reg_unimac_top_unimac_ints_itr;

#ifdef VALIDATE_PARMS
    if(!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_misc_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_INTS_ITR, reg_unimac_top_unimac_ints_itr);

    *value = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_INTS_ITR, VALUE, reg_unimac_top_unimac_ints_itr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_ints_ism_get(uint8_t umac_misc_id, uint32_t *value)
{
    uint32_t reg_unimac_top_unimac_ints_ism;

#ifdef VALIDATE_PARMS
    if(!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((umac_misc_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_INTS_ISM, reg_unimac_top_unimac_ints_ism);

    *value = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_INTS_ISM, VALUE, reg_unimac_top_unimac_ints_ism);

    return BDMF_ERR_OK;
}

#ifdef USE_BDMF_SHELL
typedef enum
{
    bdmf_address_unimac_top_unimac_misc_unimac_cfg,
    bdmf_address_unimac_top_unimac_misc_unimac_ext_cfg1,
    bdmf_address_unimac_top_unimac_misc_unimac_ext_cfg2,
    bdmf_address_unimac_top_unimac_misc_unimac_stat_update_mask,
    bdmf_address_unimac_top_unimac_misc_unimac_stat,
    bdmf_address_unimac_top_unimac_misc_unimac_debug,
    bdmf_address_unimac_top_unimac_misc_unimac_rst,
    bdmf_address_unimac_top_unimac_misc_unimac_rsv_mask,
    bdmf_address_unimac_top_unimac_misc_unimac_overrun_counter,
    bdmf_address_unimac_top_unimac_misc_unimac_1588,
    bdmf_address_unimac_top_unimac_ints_isr,
    bdmf_address_unimac_top_unimac_ints_ier,
    bdmf_address_unimac_top_unimac_ints_itr,
    bdmf_address_unimac_top_unimac_ints_ism,
}
bdmf_address;

static int bcm_unimac_misc_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_unimac_misc_unimac_top_unimac_misc_unimac_cfg:
    {
        unimac_misc_unimac_top_unimac_misc_unimac_cfg unimac_top_unimac_misc_unimac_cfg = { .direct_gmii=parm[2].value.unumber, .gport_mode=parm[3].value.unumber, .ss_mode_mii=parm[4].value.unumber, .txcrcer=parm[5].value.unumber, .mac_crc_fwd=parm[6].value.unumber, .mac_crc_owrt=parm[7].value.unumber, .ext_tx_flow_control=parm[8].value.unumber, .launch_enable=parm[9].value.unumber, .pp_pse_en=parm[10].value.unumber, .pp_gen=parm[11].value.unumber};
        err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_cfg_set(parm[1].value.unumber, &unimac_top_unimac_misc_unimac_cfg);
        break;
    }
    case cli_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg1:
        err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2:
    {
        unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2 unimac_top_unimac_misc_unimac_ext_cfg2 = { .rxfifo_pause_threshold=parm[2].value.unumber, .backpressure_enable_int=parm[3].value.unumber, .backpressure_enable_ext=parm[4].value.unumber, .fifo_overrun_ctl_en=parm[5].value.unumber, .remote_loopback_en=parm[6].value.unumber};
        err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2_set(parm[1].value.unumber, &unimac_top_unimac_misc_unimac_ext_cfg2);
        break;
    }
    case cli_unimac_misc_unimac_top_unimac_misc_unimac_stat_update_mask:
        err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_stat_update_mask_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_misc_unimac_top_unimac_misc_unimac_debug:
        err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_debug_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_misc_unimac_top_unimac_misc_unimac_rst:
        err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_rst_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_misc_unimac_top_unimac_misc_unimac_rsv_mask:
        err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_rsv_mask_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_misc_unimac_top_unimac_misc_unimac_1588:
        err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_1588_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_unimac_misc_unimac_top_unimac_ints_isr:
        err = ag_drv_unimac_misc_unimac_top_unimac_ints_isr_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_misc_unimac_top_unimac_ints_ier:
        err = ag_drv_unimac_misc_unimac_top_unimac_ints_ier_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_misc_unimac_top_unimac_ints_itr:
        err = ag_drv_unimac_misc_unimac_top_unimac_ints_itr_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

int bcm_unimac_misc_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t err = BDMF_ERR_OK;

    switch(parm[0].value.unumber)
    {
    case cli_unimac_misc_unimac_top_unimac_misc_unimac_cfg:
    {
        unimac_misc_unimac_top_unimac_misc_unimac_cfg unimac_top_unimac_misc_unimac_cfg;
        err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_cfg_get(parm[1].value.unumber, &unimac_top_unimac_misc_unimac_cfg);
        bdmf_session_print(session, "direct_gmii = %u (0x%x)\n", unimac_top_unimac_misc_unimac_cfg.direct_gmii, unimac_top_unimac_misc_unimac_cfg.direct_gmii);
        bdmf_session_print(session, "gport_mode = %u (0x%x)\n", unimac_top_unimac_misc_unimac_cfg.gport_mode, unimac_top_unimac_misc_unimac_cfg.gport_mode);
        bdmf_session_print(session, "ss_mode_mii = %u (0x%x)\n", unimac_top_unimac_misc_unimac_cfg.ss_mode_mii, unimac_top_unimac_misc_unimac_cfg.ss_mode_mii);
        bdmf_session_print(session, "txcrcer = %u (0x%x)\n", unimac_top_unimac_misc_unimac_cfg.txcrcer, unimac_top_unimac_misc_unimac_cfg.txcrcer);
        bdmf_session_print(session, "mac_crc_fwd = %u (0x%x)\n", unimac_top_unimac_misc_unimac_cfg.mac_crc_fwd, unimac_top_unimac_misc_unimac_cfg.mac_crc_fwd);
        bdmf_session_print(session, "mac_crc_owrt = %u (0x%x)\n", unimac_top_unimac_misc_unimac_cfg.mac_crc_owrt, unimac_top_unimac_misc_unimac_cfg.mac_crc_owrt);
        bdmf_session_print(session, "ext_tx_flow_control = %u (0x%x)\n", unimac_top_unimac_misc_unimac_cfg.ext_tx_flow_control, unimac_top_unimac_misc_unimac_cfg.ext_tx_flow_control);
        bdmf_session_print(session, "launch_enable = %u (0x%x)\n", unimac_top_unimac_misc_unimac_cfg.launch_enable, unimac_top_unimac_misc_unimac_cfg.launch_enable);
        bdmf_session_print(session, "pp_pse_en = %u (0x%x)\n", unimac_top_unimac_misc_unimac_cfg.pp_pse_en, unimac_top_unimac_misc_unimac_cfg.pp_pse_en);
        bdmf_session_print(session, "pp_gen = %u (0x%x)\n", unimac_top_unimac_misc_unimac_cfg.pp_gen, unimac_top_unimac_misc_unimac_cfg.pp_gen);
        break;
    }
    case cli_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg1:
    {
        uint16_t max_pkt_size;
        uint16_t rxfifo_congestion_threshold;
        err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg1_get(parm[1].value.unumber, &max_pkt_size, &rxfifo_congestion_threshold);
        bdmf_session_print(session, "max_pkt_size = %u (0x%x)\n", max_pkt_size, max_pkt_size);
        bdmf_session_print(session, "rxfifo_congestion_threshold = %u (0x%x)\n", rxfifo_congestion_threshold, rxfifo_congestion_threshold);
        break;
    }
    case cli_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2:
    {
        unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2 unimac_top_unimac_misc_unimac_ext_cfg2;
        err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2_get(parm[1].value.unumber, &unimac_top_unimac_misc_unimac_ext_cfg2);
        bdmf_session_print(session, "rxfifo_pause_threshold = %u (0x%x)\n", unimac_top_unimac_misc_unimac_ext_cfg2.rxfifo_pause_threshold, unimac_top_unimac_misc_unimac_ext_cfg2.rxfifo_pause_threshold);
        bdmf_session_print(session, "backpressure_enable_int = %u (0x%x)\n", unimac_top_unimac_misc_unimac_ext_cfg2.backpressure_enable_int, unimac_top_unimac_misc_unimac_ext_cfg2.backpressure_enable_int);
        bdmf_session_print(session, "backpressure_enable_ext = %u (0x%x)\n", unimac_top_unimac_misc_unimac_ext_cfg2.backpressure_enable_ext, unimac_top_unimac_misc_unimac_ext_cfg2.backpressure_enable_ext);
        bdmf_session_print(session, "fifo_overrun_ctl_en = %u (0x%x)\n", unimac_top_unimac_misc_unimac_ext_cfg2.fifo_overrun_ctl_en, unimac_top_unimac_misc_unimac_ext_cfg2.fifo_overrun_ctl_en);
        bdmf_session_print(session, "remote_loopback_en = %u (0x%x)\n", unimac_top_unimac_misc_unimac_ext_cfg2.remote_loopback_en, unimac_top_unimac_misc_unimac_ext_cfg2.remote_loopback_en);
        break;
    }
    case cli_unimac_misc_unimac_top_unimac_misc_unimac_stat_update_mask:
    {
        uint32_t gport_stat_update_mask;
        err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_stat_update_mask_get(parm[1].value.unumber, &gport_stat_update_mask);
        bdmf_session_print(session, "gport_stat_update_mask = %u (0x%x)\n", gport_stat_update_mask, gport_stat_update_mask);
        break;
    }
    case cli_unimac_misc_unimac_top_unimac_misc_unimac_stat:
    {
        unimac_misc_unimac_top_unimac_misc_unimac_stat unimac_top_unimac_misc_unimac_stat;
        err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_stat_get(parm[1].value.unumber, &unimac_top_unimac_misc_unimac_stat);
        bdmf_session_print(session, "auto_config_en = %u (0x%x)\n", unimac_top_unimac_misc_unimac_stat.auto_config_en, unimac_top_unimac_misc_unimac_stat.auto_config_en);
        bdmf_session_print(session, "eth_duplex = %u (0x%x)\n", unimac_top_unimac_misc_unimac_stat.eth_duplex, unimac_top_unimac_misc_unimac_stat.eth_duplex);
        bdmf_session_print(session, "eth_speed = %u (0x%x)\n", unimac_top_unimac_misc_unimac_stat.eth_speed, unimac_top_unimac_misc_unimac_stat.eth_speed);
        bdmf_session_print(session, "rx_fifo_dat_avl = %u (0x%x)\n", unimac_top_unimac_misc_unimac_stat.rx_fifo_dat_avl, unimac_top_unimac_misc_unimac_stat.rx_fifo_dat_avl);
        bdmf_session_print(session, "mac_in_pause = %u (0x%x)\n", unimac_top_unimac_misc_unimac_stat.mac_in_pause, unimac_top_unimac_misc_unimac_stat.mac_in_pause);
        bdmf_session_print(session, "mac_tx_empty = %u (0x%x)\n", unimac_top_unimac_misc_unimac_stat.mac_tx_empty, unimac_top_unimac_misc_unimac_stat.mac_tx_empty);
        bdmf_session_print(session, "launch_ack = %u (0x%x)\n", unimac_top_unimac_misc_unimac_stat.launch_ack, unimac_top_unimac_misc_unimac_stat.launch_ack);
        bdmf_session_print(session, "lpi_tx_detect = %u (0x%x)\n", unimac_top_unimac_misc_unimac_stat.lpi_tx_detect, unimac_top_unimac_misc_unimac_stat.lpi_tx_detect);
        bdmf_session_print(session, "lpi_rx_detect = %u (0x%x)\n", unimac_top_unimac_misc_unimac_stat.lpi_rx_detect, unimac_top_unimac_misc_unimac_stat.lpi_rx_detect);
        bdmf_session_print(session, "rx_sop_out = %u (0x%x)\n", unimac_top_unimac_misc_unimac_stat.rx_sop_out, unimac_top_unimac_misc_unimac_stat.rx_sop_out);
        bdmf_session_print(session, "rx_sop_delete = %u (0x%x)\n", unimac_top_unimac_misc_unimac_stat.rx_sop_delete, unimac_top_unimac_misc_unimac_stat.rx_sop_delete);
        bdmf_session_print(session, "mac_stats_update = %u (0x%x)\n", unimac_top_unimac_misc_unimac_stat.mac_stats_update, unimac_top_unimac_misc_unimac_stat.mac_stats_update);
        bdmf_session_print(session, "pp_stats = %u (0x%x)\n", unimac_top_unimac_misc_unimac_stat.pp_stats, unimac_top_unimac_misc_unimac_stat.pp_stats);
        bdmf_session_print(session, "pp_stats_valid = %u (0x%x)\n", unimac_top_unimac_misc_unimac_stat.pp_stats_valid, unimac_top_unimac_misc_unimac_stat.pp_stats_valid);
        break;
    }
    case cli_unimac_misc_unimac_top_unimac_misc_unimac_debug:
    {
        uint8_t debug_sel;
        err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_debug_get(parm[1].value.unumber, &debug_sel);
        bdmf_session_print(session, "debug_sel = %u (0x%x)\n", debug_sel, debug_sel);
        break;
    }
    case cli_unimac_misc_unimac_top_unimac_misc_unimac_rst:
    {
        bdmf_boolean unimac_rst;
        err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_rst_get(parm[1].value.unumber, &unimac_rst);
        bdmf_session_print(session, "unimac_rst = %u (0x%x)\n", unimac_rst, unimac_rst);
        break;
    }
    case cli_unimac_misc_unimac_top_unimac_misc_unimac_rsv_mask:
    {
        uint32_t gport_rsv_mask;
        err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_rsv_mask_get(parm[1].value.unumber, &gport_rsv_mask);
        bdmf_session_print(session, "gport_rsv_mask = %u (0x%x)\n", gport_rsv_mask, gport_rsv_mask);
        break;
    }
    case cli_unimac_misc_unimac_top_unimac_misc_unimac_overrun_counter:
    {
        uint32_t overrun_counter;
        err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_overrun_counter_get(parm[1].value.unumber, &overrun_counter);
        bdmf_session_print(session, "overrun_counter = %u (0x%x)\n", overrun_counter, overrun_counter);
        break;
    }
    case cli_unimac_misc_unimac_top_unimac_misc_unimac_1588:
    {
        bdmf_boolean tsi_sign_ext;
        bdmf_boolean osts_timer_dis;
        bdmf_boolean egr_1588_ts_mode;
        err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_1588_get(parm[1].value.unumber, &tsi_sign_ext, &osts_timer_dis, &egr_1588_ts_mode);
        bdmf_session_print(session, "tsi_sign_ext = %u (0x%x)\n", tsi_sign_ext, tsi_sign_ext);
        bdmf_session_print(session, "osts_timer_dis = %u (0x%x)\n", osts_timer_dis, osts_timer_dis);
        bdmf_session_print(session, "egr_1588_ts_mode = %u (0x%x)\n", egr_1588_ts_mode, egr_1588_ts_mode);
        break;
    }
    case cli_unimac_misc_unimac_top_unimac_ints_isr:
    {
        bdmf_boolean gen_int;
        err = ag_drv_unimac_misc_unimac_top_unimac_ints_isr_get(parm[1].value.unumber, &gen_int);
        bdmf_session_print(session, "gen_int = %u (0x%x)\n", gen_int, gen_int);
        break;
    }
    case cli_unimac_misc_unimac_top_unimac_ints_ier:
    {
        uint32_t value;
        err = ag_drv_unimac_misc_unimac_top_unimac_ints_ier_get(parm[1].value.unumber, &value);
        bdmf_session_print(session, "value = %u (0x%x)\n", value, value);
        break;
    }
    case cli_unimac_misc_unimac_top_unimac_ints_itr:
    {
        uint32_t value;
        err = ag_drv_unimac_misc_unimac_top_unimac_ints_itr_get(parm[1].value.unumber, &value);
        bdmf_session_print(session, "value = %u (0x%x)\n", value, value);
        break;
    }
    case cli_unimac_misc_unimac_top_unimac_ints_ism:
    {
        uint32_t value;
        err = ag_drv_unimac_misc_unimac_top_unimac_ints_ism_get(parm[1].value.unumber, &value);
        bdmf_session_print(session, "value = %u (0x%x)\n", value, value);
        break;
    }
    default:
        err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return err;
}

static int bcm_unimac_misc_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    uint8_t umac_misc_id = parm[1].value.unumber;
    bdmf_error_t err = BDMF_ERR_OK;

    {
        unimac_misc_unimac_top_unimac_misc_unimac_cfg unimac_top_unimac_misc_unimac_cfg = {.direct_gmii=gtmv(m, 1), .gport_mode=gtmv(m, 1), .ss_mode_mii=gtmv(m, 1), .txcrcer=gtmv(m, 1), .mac_crc_fwd=gtmv(m, 1), .mac_crc_owrt=gtmv(m, 1), .ext_tx_flow_control=gtmv(m, 1), .launch_enable=gtmv(m, 1), .pp_pse_en=gtmv(m, 8), .pp_gen=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_cfg_set(%u %u %u %u %u %u %u %u %u %u %u)\n", umac_misc_id, unimac_top_unimac_misc_unimac_cfg.direct_gmii, unimac_top_unimac_misc_unimac_cfg.gport_mode, unimac_top_unimac_misc_unimac_cfg.ss_mode_mii, unimac_top_unimac_misc_unimac_cfg.txcrcer, unimac_top_unimac_misc_unimac_cfg.mac_crc_fwd, unimac_top_unimac_misc_unimac_cfg.mac_crc_owrt, unimac_top_unimac_misc_unimac_cfg.ext_tx_flow_control, unimac_top_unimac_misc_unimac_cfg.launch_enable, unimac_top_unimac_misc_unimac_cfg.pp_pse_en, unimac_top_unimac_misc_unimac_cfg.pp_gen);
        if(!err) ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_cfg_set(umac_misc_id, &unimac_top_unimac_misc_unimac_cfg);
        if(!err) ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_cfg_get( umac_misc_id, &unimac_top_unimac_misc_unimac_cfg);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_cfg_get(%u %u %u %u %u %u %u %u %u %u %u)\n", umac_misc_id, unimac_top_unimac_misc_unimac_cfg.direct_gmii, unimac_top_unimac_misc_unimac_cfg.gport_mode, unimac_top_unimac_misc_unimac_cfg.ss_mode_mii, unimac_top_unimac_misc_unimac_cfg.txcrcer, unimac_top_unimac_misc_unimac_cfg.mac_crc_fwd, unimac_top_unimac_misc_unimac_cfg.mac_crc_owrt, unimac_top_unimac_misc_unimac_cfg.ext_tx_flow_control, unimac_top_unimac_misc_unimac_cfg.launch_enable, unimac_top_unimac_misc_unimac_cfg.pp_pse_en, unimac_top_unimac_misc_unimac_cfg.pp_gen);
        if(err || unimac_top_unimac_misc_unimac_cfg.direct_gmii!=gtmv(m, 1) || unimac_top_unimac_misc_unimac_cfg.gport_mode!=gtmv(m, 1) || unimac_top_unimac_misc_unimac_cfg.ss_mode_mii!=gtmv(m, 1) || unimac_top_unimac_misc_unimac_cfg.txcrcer!=gtmv(m, 1) || unimac_top_unimac_misc_unimac_cfg.mac_crc_fwd!=gtmv(m, 1) || unimac_top_unimac_misc_unimac_cfg.mac_crc_owrt!=gtmv(m, 1) || unimac_top_unimac_misc_unimac_cfg.ext_tx_flow_control!=gtmv(m, 1) || unimac_top_unimac_misc_unimac_cfg.launch_enable!=gtmv(m, 1) || unimac_top_unimac_misc_unimac_cfg.pp_pse_en!=gtmv(m, 8) || unimac_top_unimac_misc_unimac_cfg.pp_gen!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint16_t max_pkt_size=gtmv(m, 14);
        uint16_t rxfifo_congestion_threshold=gtmv(m, 9);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg1_set(%u %u %u)\n", umac_misc_id, max_pkt_size, rxfifo_congestion_threshold);
        if(!err) ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg1_set(umac_misc_id, max_pkt_size, rxfifo_congestion_threshold);
        if(!err) ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg1_get( umac_misc_id, &max_pkt_size, &rxfifo_congestion_threshold);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg1_get(%u %u %u)\n", umac_misc_id, max_pkt_size, rxfifo_congestion_threshold);
        if(err || max_pkt_size!=gtmv(m, 14) || rxfifo_congestion_threshold!=gtmv(m, 9))
            return err ? err : BDMF_ERR_IO;
    }
    {
        unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2 unimac_top_unimac_misc_unimac_ext_cfg2 = {.rxfifo_pause_threshold=gtmv(m, 9), .backpressure_enable_int=gtmv(m, 1), .backpressure_enable_ext=gtmv(m, 1), .fifo_overrun_ctl_en=gtmv(m, 1), .remote_loopback_en=gtmv(m, 1)};
        if(!err) bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2_set(%u %u %u %u %u %u)\n", umac_misc_id, unimac_top_unimac_misc_unimac_ext_cfg2.rxfifo_pause_threshold, unimac_top_unimac_misc_unimac_ext_cfg2.backpressure_enable_int, unimac_top_unimac_misc_unimac_ext_cfg2.backpressure_enable_ext, unimac_top_unimac_misc_unimac_ext_cfg2.fifo_overrun_ctl_en, unimac_top_unimac_misc_unimac_ext_cfg2.remote_loopback_en);
        if(!err) ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2_set(umac_misc_id, &unimac_top_unimac_misc_unimac_ext_cfg2);
        if(!err) ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2_get( umac_misc_id, &unimac_top_unimac_misc_unimac_ext_cfg2);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2_get(%u %u %u %u %u %u)\n", umac_misc_id, unimac_top_unimac_misc_unimac_ext_cfg2.rxfifo_pause_threshold, unimac_top_unimac_misc_unimac_ext_cfg2.backpressure_enable_int, unimac_top_unimac_misc_unimac_ext_cfg2.backpressure_enable_ext, unimac_top_unimac_misc_unimac_ext_cfg2.fifo_overrun_ctl_en, unimac_top_unimac_misc_unimac_ext_cfg2.remote_loopback_en);
        if(err || unimac_top_unimac_misc_unimac_ext_cfg2.rxfifo_pause_threshold!=gtmv(m, 9) || unimac_top_unimac_misc_unimac_ext_cfg2.backpressure_enable_int!=gtmv(m, 1) || unimac_top_unimac_misc_unimac_ext_cfg2.backpressure_enable_ext!=gtmv(m, 1) || unimac_top_unimac_misc_unimac_ext_cfg2.fifo_overrun_ctl_en!=gtmv(m, 1) || unimac_top_unimac_misc_unimac_ext_cfg2.remote_loopback_en!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gport_stat_update_mask=gtmv(m, 18);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_stat_update_mask_set(%u %u)\n", umac_misc_id, gport_stat_update_mask);
        if(!err) ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_stat_update_mask_set(umac_misc_id, gport_stat_update_mask);
        if(!err) ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_stat_update_mask_get( umac_misc_id, &gport_stat_update_mask);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_stat_update_mask_get(%u %u)\n", umac_misc_id, gport_stat_update_mask);
        if(err || gport_stat_update_mask!=gtmv(m, 18))
            return err ? err : BDMF_ERR_IO;
    }
    {
        unimac_misc_unimac_top_unimac_misc_unimac_stat unimac_top_unimac_misc_unimac_stat = {.auto_config_en=gtmv(m, 1), .eth_duplex=gtmv(m, 1), .eth_speed=gtmv(m, 2), .rx_fifo_dat_avl=gtmv(m, 1), .mac_in_pause=gtmv(m, 1), .mac_tx_empty=gtmv(m, 1), .launch_ack=gtmv(m, 1), .lpi_tx_detect=gtmv(m, 1), .lpi_rx_detect=gtmv(m, 1), .rx_sop_out=gtmv(m, 1), .rx_sop_delete=gtmv(m, 1), .mac_stats_update=gtmv(m, 1), .pp_stats=gtmv(m, 8), .pp_stats_valid=gtmv(m, 1)};
        if(!err) ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_stat_get( umac_misc_id, &unimac_top_unimac_misc_unimac_stat);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_stat_get(%u %u %u %u %u %u %u %u %u %u %u %u %u %u %u)\n", umac_misc_id, unimac_top_unimac_misc_unimac_stat.auto_config_en, unimac_top_unimac_misc_unimac_stat.eth_duplex, unimac_top_unimac_misc_unimac_stat.eth_speed, unimac_top_unimac_misc_unimac_stat.rx_fifo_dat_avl, unimac_top_unimac_misc_unimac_stat.mac_in_pause, unimac_top_unimac_misc_unimac_stat.mac_tx_empty, unimac_top_unimac_misc_unimac_stat.launch_ack, unimac_top_unimac_misc_unimac_stat.lpi_tx_detect, unimac_top_unimac_misc_unimac_stat.lpi_rx_detect, unimac_top_unimac_misc_unimac_stat.rx_sop_out, unimac_top_unimac_misc_unimac_stat.rx_sop_delete, unimac_top_unimac_misc_unimac_stat.mac_stats_update, unimac_top_unimac_misc_unimac_stat.pp_stats, unimac_top_unimac_misc_unimac_stat.pp_stats_valid);
    }
    {
        uint8_t debug_sel=gtmv(m, 8);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_debug_set(%u %u)\n", umac_misc_id, debug_sel);
        if(!err) ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_debug_set(umac_misc_id, debug_sel);
        if(!err) ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_debug_get( umac_misc_id, &debug_sel);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_debug_get(%u %u)\n", umac_misc_id, debug_sel);
        if(err || debug_sel!=gtmv(m, 8))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean unimac_rst=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_rst_set(%u %u)\n", umac_misc_id, unimac_rst);
        if(!err) ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_rst_set(umac_misc_id, unimac_rst);
        if(!err) ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_rst_get( umac_misc_id, &unimac_rst);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_rst_get(%u %u)\n", umac_misc_id, unimac_rst);
        if(err || unimac_rst!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t gport_rsv_mask=gtmv(m, 18);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_rsv_mask_set(%u %u)\n", umac_misc_id, gport_rsv_mask);
        if(!err) ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_rsv_mask_set(umac_misc_id, gport_rsv_mask);
        if(!err) ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_rsv_mask_get( umac_misc_id, &gport_rsv_mask);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_rsv_mask_get(%u %u)\n", umac_misc_id, gport_rsv_mask);
        if(err || gport_rsv_mask!=gtmv(m, 18))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t overrun_counter=gtmv(m, 32);
        if(!err) ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_overrun_counter_get( umac_misc_id, &overrun_counter);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_overrun_counter_get(%u %u)\n", umac_misc_id, overrun_counter);
    }
    {
        bdmf_boolean tsi_sign_ext=gtmv(m, 1);
        bdmf_boolean osts_timer_dis=gtmv(m, 1);
        bdmf_boolean egr_1588_ts_mode=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_1588_set(%u %u %u %u)\n", umac_misc_id, tsi_sign_ext, osts_timer_dis, egr_1588_ts_mode);
        if(!err) ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_1588_set(umac_misc_id, tsi_sign_ext, osts_timer_dis, egr_1588_ts_mode);
        if(!err) ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_1588_get( umac_misc_id, &tsi_sign_ext, &osts_timer_dis, &egr_1588_ts_mode);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_1588_get(%u %u %u %u)\n", umac_misc_id, tsi_sign_ext, osts_timer_dis, egr_1588_ts_mode);
        if(err || tsi_sign_ext!=gtmv(m, 1) || osts_timer_dis!=gtmv(m, 1) || egr_1588_ts_mode!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        bdmf_boolean gen_int=gtmv(m, 1);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_ints_isr_set(%u %u)\n", umac_misc_id, gen_int);
        if(!err) ag_drv_unimac_misc_unimac_top_unimac_ints_isr_set(umac_misc_id, gen_int);
        if(!err) ag_drv_unimac_misc_unimac_top_unimac_ints_isr_get( umac_misc_id, &gen_int);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_ints_isr_get(%u %u)\n", umac_misc_id, gen_int);
        if(err || gen_int!=gtmv(m, 1))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t value=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_ints_ier_set(%u %u)\n", umac_misc_id, value);
        if(!err) ag_drv_unimac_misc_unimac_top_unimac_ints_ier_set(umac_misc_id, value);
        if(!err) ag_drv_unimac_misc_unimac_top_unimac_ints_ier_get( umac_misc_id, &value);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_ints_ier_get(%u %u)\n", umac_misc_id, value);
        if(err || value!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t value=gtmv(m, 32);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_ints_itr_set(%u %u)\n", umac_misc_id, value);
        if(!err) ag_drv_unimac_misc_unimac_top_unimac_ints_itr_set(umac_misc_id, value);
        if(!err) ag_drv_unimac_misc_unimac_top_unimac_ints_itr_get( umac_misc_id, &value);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_ints_itr_get(%u %u)\n", umac_misc_id, value);
        if(err || value!=gtmv(m, 32))
            return err ? err : BDMF_ERR_IO;
    }
    {
        uint32_t value=gtmv(m, 32);
        if(!err) ag_drv_unimac_misc_unimac_top_unimac_ints_ism_get( umac_misc_id, &value);
        if(!err) bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_ints_ism_get(%u %u)\n", umac_misc_id, value);
    }
    return err;
}

static int bcm_unimac_misc_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_unimac_top_unimac_misc_unimac_cfg : reg = &RU_REG(UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG); blk = &RU_BLK(UNIMAC_MISC); break;
    case bdmf_address_unimac_top_unimac_misc_unimac_ext_cfg1 : reg = &RU_REG(UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1); blk = &RU_BLK(UNIMAC_MISC); break;
    case bdmf_address_unimac_top_unimac_misc_unimac_ext_cfg2 : reg = &RU_REG(UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2); blk = &RU_BLK(UNIMAC_MISC); break;
    case bdmf_address_unimac_top_unimac_misc_unimac_stat_update_mask : reg = &RU_REG(UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_UPDATE_MASK); blk = &RU_BLK(UNIMAC_MISC); break;
    case bdmf_address_unimac_top_unimac_misc_unimac_stat : reg = &RU_REG(UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT); blk = &RU_BLK(UNIMAC_MISC); break;
    case bdmf_address_unimac_top_unimac_misc_unimac_debug : reg = &RU_REG(UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG); blk = &RU_BLK(UNIMAC_MISC); break;
    case bdmf_address_unimac_top_unimac_misc_unimac_rst : reg = &RU_REG(UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST); blk = &RU_BLK(UNIMAC_MISC); break;
    case bdmf_address_unimac_top_unimac_misc_unimac_rsv_mask : reg = &RU_REG(UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RSV_MASK); blk = &RU_BLK(UNIMAC_MISC); break;
    case bdmf_address_unimac_top_unimac_misc_unimac_overrun_counter : reg = &RU_REG(UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER); blk = &RU_BLK(UNIMAC_MISC); break;
    case bdmf_address_unimac_top_unimac_misc_unimac_1588 : reg = &RU_REG(UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588); blk = &RU_BLK(UNIMAC_MISC); break;
    case bdmf_address_unimac_top_unimac_ints_isr : reg = &RU_REG(UNIMAC_MISC, UNIMAC_TOP_UNIMAC_INTS_ISR); blk = &RU_BLK(UNIMAC_MISC); break;
    case bdmf_address_unimac_top_unimac_ints_ier : reg = &RU_REG(UNIMAC_MISC, UNIMAC_TOP_UNIMAC_INTS_IER); blk = &RU_BLK(UNIMAC_MISC); break;
    case bdmf_address_unimac_top_unimac_ints_itr : reg = &RU_REG(UNIMAC_MISC, UNIMAC_TOP_UNIMAC_INTS_ITR); blk = &RU_BLK(UNIMAC_MISC); break;
    case bdmf_address_unimac_top_unimac_ints_ism : reg = &RU_REG(UNIMAC_MISC, UNIMAC_TOP_UNIMAC_INTS_ISM); blk = &RU_BLK(UNIMAC_MISC); break;
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

bdmfmon_handle_t ag_drv_unimac_misc_cli_init(bdmfmon_handle_t driver_dir)
{
    bdmfmon_handle_t dir;

    if ((dir = bdmfmon_dir_find(driver_dir, "unimac_misc"))!=NULL)
        return dir;
    dir = bdmfmon_dir_add(driver_dir, "unimac_misc", "unimac_misc", BDMF_ACCESS_ADMIN, NULL);

    {
        static bdmfmon_cmd_parm_t set_unimac_top_unimac_misc_unimac_cfg[]={
            BDMFMON_MAKE_PARM_ENUM("umac_misc_id", "umac_misc_id", umac_misc_id_enum_table, 0),
            BDMFMON_MAKE_PARM("direct_gmii", "direct_gmii", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("gport_mode", "gport_mode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ss_mode_mii", "ss_mode_mii", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("txcrcer", "txcrcer", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mac_crc_fwd", "mac_crc_fwd", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("mac_crc_owrt", "mac_crc_owrt", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("ext_tx_flow_control", "ext_tx_flow_control", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("launch_enable", "launch_enable", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pp_pse_en", "pp_pse_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("pp_gen", "pp_gen", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unimac_top_unimac_misc_unimac_ext_cfg1[]={
            BDMFMON_MAKE_PARM_ENUM("umac_misc_id", "umac_misc_id", umac_misc_id_enum_table, 0),
            BDMFMON_MAKE_PARM("max_pkt_size", "max_pkt_size", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("rxfifo_congestion_threshold", "rxfifo_congestion_threshold", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unimac_top_unimac_misc_unimac_ext_cfg2[]={
            BDMFMON_MAKE_PARM_ENUM("umac_misc_id", "umac_misc_id", umac_misc_id_enum_table, 0),
            BDMFMON_MAKE_PARM("rxfifo_pause_threshold", "rxfifo_pause_threshold", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("backpressure_enable_int", "backpressure_enable_int", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("backpressure_enable_ext", "backpressure_enable_ext", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("fifo_overrun_ctl_en", "fifo_overrun_ctl_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("remote_loopback_en", "remote_loopback_en", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unimac_top_unimac_misc_unimac_stat_update_mask[]={
            BDMFMON_MAKE_PARM_ENUM("umac_misc_id", "umac_misc_id", umac_misc_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gport_stat_update_mask", "gport_stat_update_mask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unimac_top_unimac_misc_unimac_debug[]={
            BDMFMON_MAKE_PARM_ENUM("umac_misc_id", "umac_misc_id", umac_misc_id_enum_table, 0),
            BDMFMON_MAKE_PARM("debug_sel", "debug_sel", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unimac_top_unimac_misc_unimac_rst[]={
            BDMFMON_MAKE_PARM_ENUM("umac_misc_id", "umac_misc_id", umac_misc_id_enum_table, 0),
            BDMFMON_MAKE_PARM("unimac_rst", "unimac_rst", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unimac_top_unimac_misc_unimac_rsv_mask[]={
            BDMFMON_MAKE_PARM_ENUM("umac_misc_id", "umac_misc_id", umac_misc_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gport_rsv_mask", "gport_rsv_mask", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unimac_top_unimac_misc_unimac_1588[]={
            BDMFMON_MAKE_PARM_ENUM("umac_misc_id", "umac_misc_id", umac_misc_id_enum_table, 0),
            BDMFMON_MAKE_PARM("tsi_sign_ext", "tsi_sign_ext", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("osts_timer_dis", "osts_timer_dis", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_MAKE_PARM("egr_1588_ts_mode", "egr_1588_ts_mode", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unimac_top_unimac_ints_isr[]={
            BDMFMON_MAKE_PARM_ENUM("umac_misc_id", "umac_misc_id", umac_misc_id_enum_table, 0),
            BDMFMON_MAKE_PARM("gen_int", "gen_int", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unimac_top_unimac_ints_ier[]={
            BDMFMON_MAKE_PARM_ENUM("umac_misc_id", "umac_misc_id", umac_misc_id_enum_table, 0),
            BDMFMON_MAKE_PARM("value", "value", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unimac_top_unimac_ints_itr[]={
            BDMFMON_MAKE_PARM_ENUM("umac_misc_id", "umac_misc_id", umac_misc_id_enum_table, 0),
            BDMFMON_MAKE_PARM("value", "value", BDMFMON_PARM_NUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="unimac_top_unimac_misc_unimac_cfg", .val=cli_unimac_misc_unimac_top_unimac_misc_unimac_cfg, .parms=set_unimac_top_unimac_misc_unimac_cfg },
            { .name="unimac_top_unimac_misc_unimac_ext_cfg1", .val=cli_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg1, .parms=set_unimac_top_unimac_misc_unimac_ext_cfg1 },
            { .name="unimac_top_unimac_misc_unimac_ext_cfg2", .val=cli_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2, .parms=set_unimac_top_unimac_misc_unimac_ext_cfg2 },
            { .name="unimac_top_unimac_misc_unimac_stat_update_mask", .val=cli_unimac_misc_unimac_top_unimac_misc_unimac_stat_update_mask, .parms=set_unimac_top_unimac_misc_unimac_stat_update_mask },
            { .name="unimac_top_unimac_misc_unimac_debug", .val=cli_unimac_misc_unimac_top_unimac_misc_unimac_debug, .parms=set_unimac_top_unimac_misc_unimac_debug },
            { .name="unimac_top_unimac_misc_unimac_rst", .val=cli_unimac_misc_unimac_top_unimac_misc_unimac_rst, .parms=set_unimac_top_unimac_misc_unimac_rst },
            { .name="unimac_top_unimac_misc_unimac_rsv_mask", .val=cli_unimac_misc_unimac_top_unimac_misc_unimac_rsv_mask, .parms=set_unimac_top_unimac_misc_unimac_rsv_mask },
            { .name="unimac_top_unimac_misc_unimac_1588", .val=cli_unimac_misc_unimac_top_unimac_misc_unimac_1588, .parms=set_unimac_top_unimac_misc_unimac_1588 },
            { .name="unimac_top_unimac_ints_isr", .val=cli_unimac_misc_unimac_top_unimac_ints_isr, .parms=set_unimac_top_unimac_ints_isr },
            { .name="unimac_top_unimac_ints_ier", .val=cli_unimac_misc_unimac_top_unimac_ints_ier, .parms=set_unimac_top_unimac_ints_ier },
            { .name="unimac_top_unimac_ints_itr", .val=cli_unimac_misc_unimac_top_unimac_ints_itr, .parms=set_unimac_top_unimac_ints_itr },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", bcm_unimac_misc_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t set_default[]={
            BDMFMON_MAKE_PARM_ENUM("umac_misc_id", "umac_misc_id", umac_misc_id_enum_table, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name="unimac_top_unimac_misc_unimac_cfg", .val=cli_unimac_misc_unimac_top_unimac_misc_unimac_cfg, .parms=set_default },
            { .name="unimac_top_unimac_misc_unimac_ext_cfg1", .val=cli_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg1, .parms=set_default },
            { .name="unimac_top_unimac_misc_unimac_ext_cfg2", .val=cli_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2, .parms=set_default },
            { .name="unimac_top_unimac_misc_unimac_stat_update_mask", .val=cli_unimac_misc_unimac_top_unimac_misc_unimac_stat_update_mask, .parms=set_default },
            { .name="unimac_top_unimac_misc_unimac_stat", .val=cli_unimac_misc_unimac_top_unimac_misc_unimac_stat, .parms=set_default },
            { .name="unimac_top_unimac_misc_unimac_debug", .val=cli_unimac_misc_unimac_top_unimac_misc_unimac_debug, .parms=set_default },
            { .name="unimac_top_unimac_misc_unimac_rst", .val=cli_unimac_misc_unimac_top_unimac_misc_unimac_rst, .parms=set_default },
            { .name="unimac_top_unimac_misc_unimac_rsv_mask", .val=cli_unimac_misc_unimac_top_unimac_misc_unimac_rsv_mask, .parms=set_default },
            { .name="unimac_top_unimac_misc_unimac_overrun_counter", .val=cli_unimac_misc_unimac_top_unimac_misc_unimac_overrun_counter, .parms=set_default },
            { .name="unimac_top_unimac_misc_unimac_1588", .val=cli_unimac_misc_unimac_top_unimac_misc_unimac_1588, .parms=set_default },
            { .name="unimac_top_unimac_ints_isr", .val=cli_unimac_misc_unimac_top_unimac_ints_isr, .parms=set_default },
            { .name="unimac_top_unimac_ints_ier", .val=cli_unimac_misc_unimac_top_unimac_ints_ier, .parms=set_default },
            { .name="unimac_top_unimac_ints_itr", .val=cli_unimac_misc_unimac_top_unimac_ints_itr, .parms=set_default },
            { .name="unimac_top_unimac_ints_ism", .val=cli_unimac_misc_unimac_top_unimac_ints_ism, .parms=set_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_unimac_misc_cli_get,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_test_method[] = {
            { .name="low" , .val=bdmf_test_method_low },
            { .name="mid" , .val=bdmf_test_method_mid },
            { .name="high" , .val=bdmf_test_method_high },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "test", "test", bcm_unimac_misc_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0),
            BDMFMON_MAKE_PARM_ENUM("umac_misc_id", "umac_misc_id", umac_misc_id_enum_table, 0));
    }
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name="UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG" , .val=bdmf_address_unimac_top_unimac_misc_unimac_cfg },
            { .name="UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1" , .val=bdmf_address_unimac_top_unimac_misc_unimac_ext_cfg1 },
            { .name="UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2" , .val=bdmf_address_unimac_top_unimac_misc_unimac_ext_cfg2 },
            { .name="UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT_UPDATE_MASK" , .val=bdmf_address_unimac_top_unimac_misc_unimac_stat_update_mask },
            { .name="UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT" , .val=bdmf_address_unimac_top_unimac_misc_unimac_stat },
            { .name="UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG" , .val=bdmf_address_unimac_top_unimac_misc_unimac_debug },
            { .name="UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST" , .val=bdmf_address_unimac_top_unimac_misc_unimac_rst },
            { .name="UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RSV_MASK" , .val=bdmf_address_unimac_top_unimac_misc_unimac_rsv_mask },
            { .name="UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER" , .val=bdmf_address_unimac_top_unimac_misc_unimac_overrun_counter },
            { .name="UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588" , .val=bdmf_address_unimac_top_unimac_misc_unimac_1588 },
            { .name="UNIMAC_TOP_UNIMAC_INTS_ISR" , .val=bdmf_address_unimac_top_unimac_ints_isr },
            { .name="UNIMAC_TOP_UNIMAC_INTS_IER" , .val=bdmf_address_unimac_top_unimac_ints_ier },
            { .name="UNIMAC_TOP_UNIMAC_INTS_ITR" , .val=bdmf_address_unimac_top_unimac_ints_itr },
            { .name="UNIMAC_TOP_UNIMAC_INTS_ISM" , .val=bdmf_address_unimac_top_unimac_ints_ism },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "address", "address", bcm_unimac_misc_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM_ENUM("index1", "umac_misc_id", umac_misc_id_enum_table, 0),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
    return dir;
}
#endif /* USE_BDMF_SHELL */

