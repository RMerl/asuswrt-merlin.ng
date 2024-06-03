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
#include "xrdp_drv_unimac_misc_ag.h"

#define BLOCK_ADDR_COUNT_BITS 3
#define BLOCK_ADDR_COUNT (1<<BLOCK_ADDR_COUNT_BITS)

bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_cfg_set(uint8_t umac_misc_id, bdmf_boolean mac_crc_fwd, bdmf_boolean mac_crc_owrt)
{
    uint32_t reg_unimac_top_unimac_misc_unimac_cfg = 0;

#ifdef VALIDATE_PARMS
    if ((umac_misc_id >= BLOCK_ADDR_COUNT) ||
       (mac_crc_fwd >= _1BITS_MAX_VAL_) ||
       (mac_crc_owrt >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_unimac_top_unimac_misc_unimac_cfg = RU_FIELD_SET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG, MAC_CRC_FWD, reg_unimac_top_unimac_misc_unimac_cfg, mac_crc_fwd);
    reg_unimac_top_unimac_misc_unimac_cfg = RU_FIELD_SET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG, MAC_CRC_OWRT, reg_unimac_top_unimac_misc_unimac_cfg, mac_crc_owrt);

    RU_REG_WRITE(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG, reg_unimac_top_unimac_misc_unimac_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_cfg_get(uint8_t umac_misc_id, bdmf_boolean *mac_crc_fwd, bdmf_boolean *mac_crc_owrt)
{
    uint32_t reg_unimac_top_unimac_misc_unimac_cfg;

#ifdef VALIDATE_PARMS
    if (!mac_crc_fwd || !mac_crc_owrt)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((umac_misc_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG, reg_unimac_top_unimac_misc_unimac_cfg);

    *mac_crc_fwd = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG, MAC_CRC_FWD, reg_unimac_top_unimac_misc_unimac_cfg);
    *mac_crc_owrt = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG, MAC_CRC_OWRT, reg_unimac_top_unimac_misc_unimac_cfg);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg1_set(uint8_t umac_misc_id, uint16_t max_pkt_size, uint16_t rxfifo_congestion_threshold)
{
    uint32_t reg_unimac_top_unimac_misc_unimac_ext_cfg1 = 0;

#ifdef VALIDATE_PARMS
    if ((umac_misc_id >= BLOCK_ADDR_COUNT) ||
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
    if (!max_pkt_size || !rxfifo_congestion_threshold)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((umac_misc_id >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_unimac_top_unimac_misc_unimac_ext_cfg2 = 0;

#ifdef VALIDATE_PARMS
    if(!unimac_top_unimac_misc_unimac_ext_cfg2)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((umac_misc_id >= BLOCK_ADDR_COUNT) ||
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
    if (!unimac_top_unimac_misc_unimac_ext_cfg2)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((umac_misc_id >= BLOCK_ADDR_COUNT))
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

bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_stat_get(uint8_t umac_misc_id, unimac_misc_unimac_top_unimac_misc_unimac_stat *unimac_top_unimac_misc_unimac_stat)
{
    uint32_t reg_unimac_top_unimac_misc_unimac_stat;

#ifdef VALIDATE_PARMS
    if (!unimac_top_unimac_misc_unimac_stat)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((umac_misc_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT, reg_unimac_top_unimac_misc_unimac_stat);

    unimac_top_unimac_misc_unimac_stat->port_rate = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT, PORT_RATE, reg_unimac_top_unimac_misc_unimac_stat);
    unimac_top_unimac_misc_unimac_stat->lpi_tx_detect = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT, LPI_TX_DETECT, reg_unimac_top_unimac_misc_unimac_stat);
    unimac_top_unimac_misc_unimac_stat->lpi_rx_detect = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT, LPI_RX_DETECT, reg_unimac_top_unimac_misc_unimac_stat);
    unimac_top_unimac_misc_unimac_stat->pp_stats = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT, PP_STATS, reg_unimac_top_unimac_misc_unimac_stat);
    unimac_top_unimac_misc_unimac_stat->pp_stats_valid = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT, PP_STATS_VALID, reg_unimac_top_unimac_misc_unimac_stat);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_debug_set(uint8_t umac_misc_id, uint8_t debug_sel)
{
    uint32_t reg_unimac_top_unimac_misc_unimac_debug = 0;

#ifdef VALIDATE_PARMS
    if ((umac_misc_id >= BLOCK_ADDR_COUNT))
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
    if (!debug_sel)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((umac_misc_id >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_unimac_top_unimac_misc_unimac_rst = 0;

#ifdef VALIDATE_PARMS
    if ((umac_misc_id >= BLOCK_ADDR_COUNT) ||
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
    if (!unimac_rst)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((umac_misc_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST, reg_unimac_top_unimac_misc_unimac_rst);

    *unimac_rst = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST, UNIMAC_RST, reg_unimac_top_unimac_misc_unimac_rst);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_overrun_counter_get(uint8_t umac_misc_id, uint32_t *overrun_counter)
{
    uint32_t reg_unimac_top_unimac_misc_unimac_overrun_counter;

#ifdef VALIDATE_PARMS
    if (!overrun_counter)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((umac_misc_id >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_unimac_top_unimac_misc_unimac_1588 = 0;

#ifdef VALIDATE_PARMS
    if ((umac_misc_id >= BLOCK_ADDR_COUNT) ||
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
    if (!tsi_sign_ext || !osts_timer_dis || !egr_1588_ts_mode)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((umac_misc_id >= BLOCK_ADDR_COUNT))
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
    uint32_t reg_unimac_top_unimac_ints_isr = 0;

#ifdef VALIDATE_PARMS
    if ((umac_misc_id >= BLOCK_ADDR_COUNT) ||
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
    if (!gen_int)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((umac_misc_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_INTS_ISR, reg_unimac_top_unimac_ints_isr);

    *gen_int = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_INTS_ISR, GEN_INT, reg_unimac_top_unimac_ints_isr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_ints_ier_set(uint8_t umac_misc_id, bdmf_boolean value)
{
    uint32_t reg_unimac_top_unimac_ints_ier = 0;

#ifdef VALIDATE_PARMS
    if ((umac_misc_id >= BLOCK_ADDR_COUNT) ||
       (value >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_unimac_top_unimac_ints_ier = RU_FIELD_SET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_INTS_IER, VALUE, reg_unimac_top_unimac_ints_ier, value);

    RU_REG_WRITE(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_INTS_IER, reg_unimac_top_unimac_ints_ier);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_ints_ier_get(uint8_t umac_misc_id, bdmf_boolean *value)
{
    uint32_t reg_unimac_top_unimac_ints_ier;

#ifdef VALIDATE_PARMS
    if (!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((umac_misc_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_INTS_IER, reg_unimac_top_unimac_ints_ier);

    *value = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_INTS_IER, VALUE, reg_unimac_top_unimac_ints_ier);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_ints_itr_set(uint8_t umac_misc_id, bdmf_boolean value)
{
    uint32_t reg_unimac_top_unimac_ints_itr = 0;

#ifdef VALIDATE_PARMS
    if ((umac_misc_id >= BLOCK_ADDR_COUNT) ||
       (value >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_unimac_top_unimac_ints_itr = RU_FIELD_SET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_INTS_ITR, VALUE, reg_unimac_top_unimac_ints_itr, value);

    RU_REG_WRITE(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_INTS_ITR, reg_unimac_top_unimac_ints_itr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_ints_itr_get(uint8_t umac_misc_id, bdmf_boolean *value)
{
    uint32_t reg_unimac_top_unimac_ints_itr;

#ifdef VALIDATE_PARMS
    if (!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((umac_misc_id >= BLOCK_ADDR_COUNT))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    RU_REG_READ(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_INTS_ITR, reg_unimac_top_unimac_ints_itr);

    *value = RU_FIELD_GET(umac_misc_id, UNIMAC_MISC, UNIMAC_TOP_UNIMAC_INTS_ITR, VALUE, reg_unimac_top_unimac_ints_itr);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_unimac_misc_unimac_top_unimac_ints_ism_get(uint8_t umac_misc_id, bdmf_boolean *value)
{
    uint32_t reg_unimac_top_unimac_ints_ism;

#ifdef VALIDATE_PARMS
    if (!value)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if ((umac_misc_id >= BLOCK_ADDR_COUNT))
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
    bdmf_address_unimac_top_unimac_misc_unimac_stat,
    bdmf_address_unimac_top_unimac_misc_unimac_debug,
    bdmf_address_unimac_top_unimac_misc_unimac_rst,
    bdmf_address_unimac_top_unimac_misc_unimac_overrun_counter,
    bdmf_address_unimac_top_unimac_misc_unimac_1588,
    bdmf_address_unimac_top_unimac_ints_isr,
    bdmf_address_unimac_top_unimac_ints_ier,
    bdmf_address_unimac_top_unimac_ints_itr,
    bdmf_address_unimac_top_unimac_ints_ism,
}
bdmf_address;

static int ag_drv_unimac_misc_cli_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_unimac_misc_unimac_top_unimac_misc_unimac_cfg:
        ag_err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_cfg_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg1:
        ag_err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg1_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber);
        break;
    case cli_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2:
    {
        unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2 unimac_top_unimac_misc_unimac_ext_cfg2 = { .rxfifo_pause_threshold = parm[2].value.unumber, .backpressure_enable_int = parm[3].value.unumber, .backpressure_enable_ext = parm[4].value.unumber, .fifo_overrun_ctl_en = parm[5].value.unumber, .remote_loopback_en = parm[6].value.unumber};
        ag_err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2_set(parm[1].value.unumber, &unimac_top_unimac_misc_unimac_ext_cfg2);
        break;
    }
    case cli_unimac_misc_unimac_top_unimac_misc_unimac_debug:
        ag_err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_debug_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_misc_unimac_top_unimac_misc_unimac_rst:
        ag_err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_rst_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_misc_unimac_top_unimac_misc_unimac_1588:
        ag_err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_1588_set(parm[1].value.unumber, parm[2].value.unumber, parm[3].value.unumber, parm[4].value.unumber);
        break;
    case cli_unimac_misc_unimac_top_unimac_ints_isr:
        ag_err = ag_drv_unimac_misc_unimac_top_unimac_ints_isr_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_misc_unimac_top_unimac_ints_ier:
        ag_err = ag_drv_unimac_misc_unimac_top_unimac_ints_ier_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    case cli_unimac_misc_unimac_top_unimac_ints_itr:
        ag_err = ag_drv_unimac_misc_unimac_top_unimac_ints_itr_set(parm[1].value.unumber, parm[2].value.unumber);
        break;
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

int bcm_unimac_misc_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t ag_err;

    switch (parm[0].value.unumber)
    {
    case cli_unimac_misc_unimac_top_unimac_misc_unimac_cfg:
    {
        bdmf_boolean mac_crc_fwd;
        bdmf_boolean mac_crc_owrt;
        ag_err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_cfg_get(parm[1].value.unumber, &mac_crc_fwd, &mac_crc_owrt);
        bdmf_session_print(session, "mac_crc_fwd = %u = 0x%x\n", mac_crc_fwd, mac_crc_fwd);
        bdmf_session_print(session, "mac_crc_owrt = %u = 0x%x\n", mac_crc_owrt, mac_crc_owrt);
        break;
    }
    case cli_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg1:
    {
        uint16_t max_pkt_size;
        uint16_t rxfifo_congestion_threshold;
        ag_err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg1_get(parm[1].value.unumber, &max_pkt_size, &rxfifo_congestion_threshold);
        bdmf_session_print(session, "max_pkt_size = %u = 0x%x\n", max_pkt_size, max_pkt_size);
        bdmf_session_print(session, "rxfifo_congestion_threshold = %u = 0x%x\n", rxfifo_congestion_threshold, rxfifo_congestion_threshold);
        break;
    }
    case cli_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2:
    {
        unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2 unimac_top_unimac_misc_unimac_ext_cfg2;
        ag_err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2_get(parm[1].value.unumber, &unimac_top_unimac_misc_unimac_ext_cfg2);
        bdmf_session_print(session, "rxfifo_pause_threshold = %u = 0x%x\n", unimac_top_unimac_misc_unimac_ext_cfg2.rxfifo_pause_threshold, unimac_top_unimac_misc_unimac_ext_cfg2.rxfifo_pause_threshold);
        bdmf_session_print(session, "backpressure_enable_int = %u = 0x%x\n", unimac_top_unimac_misc_unimac_ext_cfg2.backpressure_enable_int, unimac_top_unimac_misc_unimac_ext_cfg2.backpressure_enable_int);
        bdmf_session_print(session, "backpressure_enable_ext = %u = 0x%x\n", unimac_top_unimac_misc_unimac_ext_cfg2.backpressure_enable_ext, unimac_top_unimac_misc_unimac_ext_cfg2.backpressure_enable_ext);
        bdmf_session_print(session, "fifo_overrun_ctl_en = %u = 0x%x\n", unimac_top_unimac_misc_unimac_ext_cfg2.fifo_overrun_ctl_en, unimac_top_unimac_misc_unimac_ext_cfg2.fifo_overrun_ctl_en);
        bdmf_session_print(session, "remote_loopback_en = %u = 0x%x\n", unimac_top_unimac_misc_unimac_ext_cfg2.remote_loopback_en, unimac_top_unimac_misc_unimac_ext_cfg2.remote_loopback_en);
        break;
    }
    case cli_unimac_misc_unimac_top_unimac_misc_unimac_stat:
    {
        unimac_misc_unimac_top_unimac_misc_unimac_stat unimac_top_unimac_misc_unimac_stat;
        ag_err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_stat_get(parm[1].value.unumber, &unimac_top_unimac_misc_unimac_stat);
        bdmf_session_print(session, "port_rate = %u = 0x%x\n", unimac_top_unimac_misc_unimac_stat.port_rate, unimac_top_unimac_misc_unimac_stat.port_rate);
        bdmf_session_print(session, "lpi_tx_detect = %u = 0x%x\n", unimac_top_unimac_misc_unimac_stat.lpi_tx_detect, unimac_top_unimac_misc_unimac_stat.lpi_tx_detect);
        bdmf_session_print(session, "lpi_rx_detect = %u = 0x%x\n", unimac_top_unimac_misc_unimac_stat.lpi_rx_detect, unimac_top_unimac_misc_unimac_stat.lpi_rx_detect);
        bdmf_session_print(session, "pp_stats = %u = 0x%x\n", unimac_top_unimac_misc_unimac_stat.pp_stats, unimac_top_unimac_misc_unimac_stat.pp_stats);
        bdmf_session_print(session, "pp_stats_valid = %u = 0x%x\n", unimac_top_unimac_misc_unimac_stat.pp_stats_valid, unimac_top_unimac_misc_unimac_stat.pp_stats_valid);
        break;
    }
    case cli_unimac_misc_unimac_top_unimac_misc_unimac_debug:
    {
        uint8_t debug_sel;
        ag_err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_debug_get(parm[1].value.unumber, &debug_sel);
        bdmf_session_print(session, "debug_sel = %u = 0x%x\n", debug_sel, debug_sel);
        break;
    }
    case cli_unimac_misc_unimac_top_unimac_misc_unimac_rst:
    {
        bdmf_boolean unimac_rst;
        ag_err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_rst_get(parm[1].value.unumber, &unimac_rst);
        bdmf_session_print(session, "unimac_rst = %u = 0x%x\n", unimac_rst, unimac_rst);
        break;
    }
    case cli_unimac_misc_unimac_top_unimac_misc_unimac_overrun_counter:
    {
        uint32_t overrun_counter;
        ag_err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_overrun_counter_get(parm[1].value.unumber, &overrun_counter);
        bdmf_session_print(session, "overrun_counter = %u = 0x%x\n", overrun_counter, overrun_counter);
        break;
    }
    case cli_unimac_misc_unimac_top_unimac_misc_unimac_1588:
    {
        bdmf_boolean tsi_sign_ext;
        bdmf_boolean osts_timer_dis;
        bdmf_boolean egr_1588_ts_mode;
        ag_err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_1588_get(parm[1].value.unumber, &tsi_sign_ext, &osts_timer_dis, &egr_1588_ts_mode);
        bdmf_session_print(session, "tsi_sign_ext = %u = 0x%x\n", tsi_sign_ext, tsi_sign_ext);
        bdmf_session_print(session, "osts_timer_dis = %u = 0x%x\n", osts_timer_dis, osts_timer_dis);
        bdmf_session_print(session, "egr_1588_ts_mode = %u = 0x%x\n", egr_1588_ts_mode, egr_1588_ts_mode);
        break;
    }
    case cli_unimac_misc_unimac_top_unimac_ints_isr:
    {
        bdmf_boolean gen_int;
        ag_err = ag_drv_unimac_misc_unimac_top_unimac_ints_isr_get(parm[1].value.unumber, &gen_int);
        bdmf_session_print(session, "gen_int = %u = 0x%x\n", gen_int, gen_int);
        break;
    }
    case cli_unimac_misc_unimac_top_unimac_ints_ier:
    {
        bdmf_boolean value;
        ag_err = ag_drv_unimac_misc_unimac_top_unimac_ints_ier_get(parm[1].value.unumber, &value);
        bdmf_session_print(session, "value = %u = 0x%x\n", value, value);
        break;
    }
    case cli_unimac_misc_unimac_top_unimac_ints_itr:
    {
        bdmf_boolean value;
        ag_err = ag_drv_unimac_misc_unimac_top_unimac_ints_itr_get(parm[1].value.unumber, &value);
        bdmf_session_print(session, "value = %u = 0x%x\n", value, value);
        break;
    }
    case cli_unimac_misc_unimac_top_unimac_ints_ism:
    {
        bdmf_boolean value;
        ag_err = ag_drv_unimac_misc_unimac_top_unimac_ints_ism_get(parm[1].value.unumber, &value);
        bdmf_session_print(session, "value = %u = 0x%x\n", value, value);
        break;
    }
    default:
        ag_err = BDMF_ERR_NOT_SUPPORTED;
        break;
    }
    return ag_err;
}

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_unimac_misc_cli_test(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_test_method m = parm[0].value.unumber;
    uint8_t umac_misc_id = parm[1].value.unumber;
    bdmf_error_t ag_err = BDMF_ERR_OK;
    uint32_t test_success_cnt = 0;
    uint32_t test_failure_cnt = 0;

    {
        bdmf_boolean mac_crc_fwd = gtmv(m, 1);
        bdmf_boolean mac_crc_owrt = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_cfg_set(%u %u %u)\n", umac_misc_id,
            mac_crc_fwd, mac_crc_owrt);
        ag_err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_cfg_set(umac_misc_id, mac_crc_fwd, mac_crc_owrt);
        if (!ag_err)
            ag_err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_cfg_get(umac_misc_id, &mac_crc_fwd, &mac_crc_owrt);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_cfg_get(%u %u %u)\n", umac_misc_id,
                mac_crc_fwd, mac_crc_owrt);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (mac_crc_fwd != gtmv(m, 1) || mac_crc_owrt != gtmv(m, 1))
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
        uint16_t rxfifo_congestion_threshold = gtmv(m, 9);
        bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg1_set(%u %u %u)\n", umac_misc_id,
            max_pkt_size, rxfifo_congestion_threshold);
        ag_err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg1_set(umac_misc_id, max_pkt_size, rxfifo_congestion_threshold);
        if (!ag_err)
            ag_err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg1_get(umac_misc_id, &max_pkt_size, &rxfifo_congestion_threshold);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg1_get(%u %u %u)\n", umac_misc_id,
                max_pkt_size, rxfifo_congestion_threshold);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (max_pkt_size != gtmv(m, 14) || rxfifo_congestion_threshold != gtmv(m, 9))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2 unimac_top_unimac_misc_unimac_ext_cfg2 = {.rxfifo_pause_threshold = gtmv(m, 9), .backpressure_enable_int = gtmv(m, 1), .backpressure_enable_ext = gtmv(m, 1), .fifo_overrun_ctl_en = gtmv(m, 1), .remote_loopback_en = gtmv(m, 1)};
        bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2_set(%u %u %u %u %u %u)\n", umac_misc_id,
            unimac_top_unimac_misc_unimac_ext_cfg2.rxfifo_pause_threshold, unimac_top_unimac_misc_unimac_ext_cfg2.backpressure_enable_int, unimac_top_unimac_misc_unimac_ext_cfg2.backpressure_enable_ext, unimac_top_unimac_misc_unimac_ext_cfg2.fifo_overrun_ctl_en, 
            unimac_top_unimac_misc_unimac_ext_cfg2.remote_loopback_en);
        ag_err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2_set(umac_misc_id, &unimac_top_unimac_misc_unimac_ext_cfg2);
        if (!ag_err)
            ag_err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2_get(umac_misc_id, &unimac_top_unimac_misc_unimac_ext_cfg2);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2_get(%u %u %u %u %u %u)\n", umac_misc_id,
                unimac_top_unimac_misc_unimac_ext_cfg2.rxfifo_pause_threshold, unimac_top_unimac_misc_unimac_ext_cfg2.backpressure_enable_int, unimac_top_unimac_misc_unimac_ext_cfg2.backpressure_enable_ext, unimac_top_unimac_misc_unimac_ext_cfg2.fifo_overrun_ctl_en, 
                unimac_top_unimac_misc_unimac_ext_cfg2.remote_loopback_en);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (unimac_top_unimac_misc_unimac_ext_cfg2.rxfifo_pause_threshold != gtmv(m, 9) || unimac_top_unimac_misc_unimac_ext_cfg2.backpressure_enable_int != gtmv(m, 1) || unimac_top_unimac_misc_unimac_ext_cfg2.backpressure_enable_ext != gtmv(m, 1) || unimac_top_unimac_misc_unimac_ext_cfg2.fifo_overrun_ctl_en != gtmv(m, 1) || unimac_top_unimac_misc_unimac_ext_cfg2.remote_loopback_en != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        unimac_misc_unimac_top_unimac_misc_unimac_stat unimac_top_unimac_misc_unimac_stat = {.port_rate = gtmv(m, 3), .lpi_tx_detect = gtmv(m, 1), .lpi_rx_detect = gtmv(m, 1), .pp_stats = gtmv(m, 8), .pp_stats_valid = gtmv(m, 1)};
        if (!ag_err)
            ag_err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_stat_get(umac_misc_id, &unimac_top_unimac_misc_unimac_stat);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_stat_get(%u %u %u %u %u %u)\n", umac_misc_id,
                unimac_top_unimac_misc_unimac_stat.port_rate, unimac_top_unimac_misc_unimac_stat.lpi_tx_detect, unimac_top_unimac_misc_unimac_stat.lpi_rx_detect, unimac_top_unimac_misc_unimac_stat.pp_stats, 
                unimac_top_unimac_misc_unimac_stat.pp_stats_valid);
        }
    }

    {
        uint8_t debug_sel = gtmv(m, 8);
        bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_debug_set(%u %u)\n", umac_misc_id,
            debug_sel);
        ag_err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_debug_set(umac_misc_id, debug_sel);
        if (!ag_err)
            ag_err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_debug_get(umac_misc_id, &debug_sel);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_debug_get(%u %u)\n", umac_misc_id,
                debug_sel);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (debug_sel != gtmv(m, 8))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean unimac_rst = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_rst_set(%u %u)\n", umac_misc_id,
            unimac_rst);
        ag_err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_rst_set(umac_misc_id, unimac_rst);
        if (!ag_err)
            ag_err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_rst_get(umac_misc_id, &unimac_rst);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_rst_get(%u %u)\n", umac_misc_id,
                unimac_rst);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (unimac_rst != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        uint32_t overrun_counter = gtmv(m, 32);
        if (!ag_err)
            ag_err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_overrun_counter_get(umac_misc_id, &overrun_counter);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_overrun_counter_get(%u %u)\n", umac_misc_id,
                overrun_counter);
        }
    }

    {
        bdmf_boolean tsi_sign_ext = gtmv(m, 1);
        bdmf_boolean osts_timer_dis = gtmv(m, 1);
        bdmf_boolean egr_1588_ts_mode = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_1588_set(%u %u %u %u)\n", umac_misc_id,
            tsi_sign_ext, osts_timer_dis, egr_1588_ts_mode);
        ag_err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_1588_set(umac_misc_id, tsi_sign_ext, osts_timer_dis, egr_1588_ts_mode);
        if (!ag_err)
            ag_err = ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_1588_get(umac_misc_id, &tsi_sign_ext, &osts_timer_dis, &egr_1588_ts_mode);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_1588_get(%u %u %u %u)\n", umac_misc_id,
                tsi_sign_ext, osts_timer_dis, egr_1588_ts_mode);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (tsi_sign_ext != gtmv(m, 1) || osts_timer_dis != gtmv(m, 1) || egr_1588_ts_mode != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean gen_int = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_ints_isr_set(%u %u)\n", umac_misc_id,
            gen_int);
        ag_err = ag_drv_unimac_misc_unimac_top_unimac_ints_isr_set(umac_misc_id, gen_int);
        if (!ag_err)
            ag_err = ag_drv_unimac_misc_unimac_top_unimac_ints_isr_get(umac_misc_id, &gen_int);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_ints_isr_get(%u %u)\n", umac_misc_id,
                gen_int);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (gen_int != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean value = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_ints_ier_set(%u %u)\n", umac_misc_id,
            value);
        ag_err = ag_drv_unimac_misc_unimac_top_unimac_ints_ier_set(umac_misc_id, value);
        if (!ag_err)
            ag_err = ag_drv_unimac_misc_unimac_top_unimac_ints_ier_get(umac_misc_id, &value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_ints_ier_get(%u %u)\n", umac_misc_id,
                value);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (value != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean value = gtmv(m, 1);
        bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_ints_itr_set(%u %u)\n", umac_misc_id,
            value);
        ag_err = ag_drv_unimac_misc_unimac_top_unimac_ints_itr_set(umac_misc_id, value);
        if (!ag_err)
            ag_err = ag_drv_unimac_misc_unimac_top_unimac_ints_itr_get(umac_misc_id, &value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_ints_itr_get(%u %u)\n", umac_misc_id,
                value);
        }
        else
        {
            return BDMF_ERR_IO;
        }

        if (value != gtmv(m, 1))
        {
            test_failure_cnt++;
            bdmf_session_print(session, "  register failed match; failures=%u\n", test_failure_cnt);        }
        else
        {
            test_success_cnt++;
        }
    }

    {
        bdmf_boolean value = gtmv(m, 1);
        if (!ag_err)
            ag_err = ag_drv_unimac_misc_unimac_top_unimac_ints_ism_get(umac_misc_id, &value);

        if (!ag_err)
        {
            bdmf_session_print(session, "ag_drv_unimac_misc_unimac_top_unimac_ints_ism_get(%u %u)\n", umac_misc_id,
                value);
        }
    }

    bdmf_session_print(session, "successes=%u failures=%u\n", test_success_cnt, test_failure_cnt);

    return ag_err;
}
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
static int ag_drv_unimac_misc_cli_address(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
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
    case bdmf_address_unimac_top_unimac_misc_unimac_cfg: reg = &RU_REG(UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG); blk = &RU_BLK(UNIMAC_MISC); break;
    case bdmf_address_unimac_top_unimac_misc_unimac_ext_cfg1: reg = &RU_REG(UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1); blk = &RU_BLK(UNIMAC_MISC); break;
    case bdmf_address_unimac_top_unimac_misc_unimac_ext_cfg2: reg = &RU_REG(UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2); blk = &RU_BLK(UNIMAC_MISC); break;
    case bdmf_address_unimac_top_unimac_misc_unimac_stat: reg = &RU_REG(UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT); blk = &RU_BLK(UNIMAC_MISC); break;
    case bdmf_address_unimac_top_unimac_misc_unimac_debug: reg = &RU_REG(UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG); blk = &RU_BLK(UNIMAC_MISC); break;
    case bdmf_address_unimac_top_unimac_misc_unimac_rst: reg = &RU_REG(UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST); blk = &RU_BLK(UNIMAC_MISC); break;
    case bdmf_address_unimac_top_unimac_misc_unimac_overrun_counter: reg = &RU_REG(UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER); blk = &RU_BLK(UNIMAC_MISC); break;
    case bdmf_address_unimac_top_unimac_misc_unimac_1588: reg = &RU_REG(UNIMAC_MISC, UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588); blk = &RU_BLK(UNIMAC_MISC); break;
    case bdmf_address_unimac_top_unimac_ints_isr: reg = &RU_REG(UNIMAC_MISC, UNIMAC_TOP_UNIMAC_INTS_ISR); blk = &RU_BLK(UNIMAC_MISC); break;
    case bdmf_address_unimac_top_unimac_ints_ier: reg = &RU_REG(UNIMAC_MISC, UNIMAC_TOP_UNIMAC_INTS_IER); blk = &RU_BLK(UNIMAC_MISC); break;
    case bdmf_address_unimac_top_unimac_ints_itr: reg = &RU_REG(UNIMAC_MISC, UNIMAC_TOP_UNIMAC_INTS_ITR); blk = &RU_BLK(UNIMAC_MISC); break;
    case bdmf_address_unimac_top_unimac_ints_ism: reg = &RU_REG(UNIMAC_MISC, UNIMAC_TOP_UNIMAC_INTS_ISM); blk = &RU_BLK(UNIMAC_MISC); break;
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

bdmfmon_handle_t ag_drv_unimac_misc_cli_init(bdmfmon_handle_t root_dir)
{
    bdmfmon_handle_t dir;

    dir = bdmfmon_dir_add(root_dir, "unimac_misc", "unimac_misc", BDMF_ACCESS_ADMIN, NULL);
    BUG_ON(dir == NULL);

    {
        static bdmfmon_cmd_parm_t set_unimac_top_unimac_misc_unimac_cfg[] = {
            BDMFMON_MAKE_PARM("umac_misc_id", "umac_misc_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mac_crc_fwd", "mac_crc_fwd", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("mac_crc_owrt", "mac_crc_owrt", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unimac_top_unimac_misc_unimac_ext_cfg1[] = {
            BDMFMON_MAKE_PARM("umac_misc_id", "umac_misc_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("max_pkt_size", "max_pkt_size", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rxfifo_congestion_threshold", "rxfifo_congestion_threshold", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unimac_top_unimac_misc_unimac_ext_cfg2[] = {
            BDMFMON_MAKE_PARM("umac_misc_id", "umac_misc_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("rxfifo_pause_threshold", "rxfifo_pause_threshold", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("backpressure_enable_int", "backpressure_enable_int", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("backpressure_enable_ext", "backpressure_enable_ext", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("fifo_overrun_ctl_en", "fifo_overrun_ctl_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("remote_loopback_en", "remote_loopback_en", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unimac_top_unimac_misc_unimac_debug[] = {
            BDMFMON_MAKE_PARM("umac_misc_id", "umac_misc_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("debug_sel", "debug_sel", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unimac_top_unimac_misc_unimac_rst[] = {
            BDMFMON_MAKE_PARM("umac_misc_id", "umac_misc_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("unimac_rst", "unimac_rst", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unimac_top_unimac_misc_unimac_1588[] = {
            BDMFMON_MAKE_PARM("umac_misc_id", "umac_misc_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("tsi_sign_ext", "tsi_sign_ext", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("osts_timer_dis", "osts_timer_dis", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("egr_1588_ts_mode", "egr_1588_ts_mode", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unimac_top_unimac_ints_isr[] = {
            BDMFMON_MAKE_PARM("umac_misc_id", "umac_misc_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("gen_int", "gen_int", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unimac_top_unimac_ints_ier[] = {
            BDMFMON_MAKE_PARM("umac_misc_id", "umac_misc_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("value", "value", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_cmd_parm_t set_unimac_top_unimac_ints_itr[] = {
            BDMFMON_MAKE_PARM("umac_misc_id", "umac_misc_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_MAKE_PARM("value", "value", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "unimac_top_unimac_misc_unimac_cfg", .val = cli_unimac_misc_unimac_top_unimac_misc_unimac_cfg, .parms = set_unimac_top_unimac_misc_unimac_cfg },
            { .name = "unimac_top_unimac_misc_unimac_ext_cfg1", .val = cli_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg1, .parms = set_unimac_top_unimac_misc_unimac_ext_cfg1 },
            { .name = "unimac_top_unimac_misc_unimac_ext_cfg2", .val = cli_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2, .parms = set_unimac_top_unimac_misc_unimac_ext_cfg2 },
            { .name = "unimac_top_unimac_misc_unimac_debug", .val = cli_unimac_misc_unimac_top_unimac_misc_unimac_debug, .parms = set_unimac_top_unimac_misc_unimac_debug },
            { .name = "unimac_top_unimac_misc_unimac_rst", .val = cli_unimac_misc_unimac_top_unimac_misc_unimac_rst, .parms = set_unimac_top_unimac_misc_unimac_rst },
            { .name = "unimac_top_unimac_misc_unimac_1588", .val = cli_unimac_misc_unimac_top_unimac_misc_unimac_1588, .parms = set_unimac_top_unimac_misc_unimac_1588 },
            { .name = "unimac_top_unimac_ints_isr", .val = cli_unimac_misc_unimac_top_unimac_ints_isr, .parms = set_unimac_top_unimac_ints_isr },
            { .name = "unimac_top_unimac_ints_ier", .val = cli_unimac_misc_unimac_top_unimac_ints_ier, .parms = set_unimac_top_unimac_ints_ier },
            { .name = "unimac_top_unimac_ints_itr", .val = cli_unimac_misc_unimac_top_unimac_ints_itr, .parms = set_unimac_top_unimac_ints_itr },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "set", "set", ag_drv_unimac_misc_cli_set,
            BDMFMON_MAKE_PARM_SELECTOR("purpose", "purpose", selector_table, 0));
    }
    {
        static bdmfmon_cmd_parm_t get_default[] = {
            BDMFMON_MAKE_PARM("umac_misc_id", "umac_misc_id", BDMFMON_PARM_UNUMBER, 0),
            BDMFMON_PARM_LIST_TERMINATOR
        };
        static bdmfmon_enum_val_t selector_table[] = {
            { .name = "unimac_top_unimac_misc_unimac_cfg", .val = cli_unimac_misc_unimac_top_unimac_misc_unimac_cfg, .parms = get_default },
            { .name = "unimac_top_unimac_misc_unimac_ext_cfg1", .val = cli_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg1, .parms = get_default },
            { .name = "unimac_top_unimac_misc_unimac_ext_cfg2", .val = cli_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg2, .parms = get_default },
            { .name = "unimac_top_unimac_misc_unimac_stat", .val = cli_unimac_misc_unimac_top_unimac_misc_unimac_stat, .parms = get_default },
            { .name = "unimac_top_unimac_misc_unimac_debug", .val = cli_unimac_misc_unimac_top_unimac_misc_unimac_debug, .parms = get_default },
            { .name = "unimac_top_unimac_misc_unimac_rst", .val = cli_unimac_misc_unimac_top_unimac_misc_unimac_rst, .parms = get_default },
            { .name = "unimac_top_unimac_misc_unimac_overrun_counter", .val = cli_unimac_misc_unimac_top_unimac_misc_unimac_overrun_counter, .parms = get_default },
            { .name = "unimac_top_unimac_misc_unimac_1588", .val = cli_unimac_misc_unimac_top_unimac_misc_unimac_1588, .parms = get_default },
            { .name = "unimac_top_unimac_ints_isr", .val = cli_unimac_misc_unimac_top_unimac_ints_isr, .parms = get_default },
            { .name = "unimac_top_unimac_ints_ier", .val = cli_unimac_misc_unimac_top_unimac_ints_ier, .parms = get_default },
            { .name = "unimac_top_unimac_ints_itr", .val = cli_unimac_misc_unimac_top_unimac_ints_itr, .parms = get_default },
            { .name = "unimac_top_unimac_ints_ism", .val = cli_unimac_misc_unimac_top_unimac_ints_ism, .parms = get_default },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_CMD(dir, "get", "get", bcm_unimac_misc_cli_get,
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
        BDMFMON_MAKE_PARM(dir, "test", "test", ag_drv_unimac_misc_cli_test,
            BDMFMON_MAKE_PARM_ENUM("method", "low: 0000, mid: 1000, high: 1111", enum_table_test_method, 0)            BDMFMON_MAKE_PARM("umac_misc_id", "umac_misc_id", BDMFMON_PARM_UNUMBER, 0));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

#ifdef HAL_DRV_TEST_ENABLE
    {
        static bdmfmon_enum_val_t enum_table_address[] = {
            { .name = "UNIMAC_TOP_UNIMAC_MISC_UNIMAC_CFG", .val = bdmf_address_unimac_top_unimac_misc_unimac_cfg },
            { .name = "UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG1", .val = bdmf_address_unimac_top_unimac_misc_unimac_ext_cfg1 },
            { .name = "UNIMAC_TOP_UNIMAC_MISC_UNIMAC_EXT_CFG2", .val = bdmf_address_unimac_top_unimac_misc_unimac_ext_cfg2 },
            { .name = "UNIMAC_TOP_UNIMAC_MISC_UNIMAC_STAT", .val = bdmf_address_unimac_top_unimac_misc_unimac_stat },
            { .name = "UNIMAC_TOP_UNIMAC_MISC_UNIMAC_DEBUG", .val = bdmf_address_unimac_top_unimac_misc_unimac_debug },
            { .name = "UNIMAC_TOP_UNIMAC_MISC_UNIMAC_RST", .val = bdmf_address_unimac_top_unimac_misc_unimac_rst },
            { .name = "UNIMAC_TOP_UNIMAC_MISC_UNIMAC_OVERRUN_COUNTER", .val = bdmf_address_unimac_top_unimac_misc_unimac_overrun_counter },
            { .name = "UNIMAC_TOP_UNIMAC_MISC_UNIMAC_1588", .val = bdmf_address_unimac_top_unimac_misc_unimac_1588 },
            { .name = "UNIMAC_TOP_UNIMAC_INTS_ISR", .val = bdmf_address_unimac_top_unimac_ints_isr },
            { .name = "UNIMAC_TOP_UNIMAC_INTS_IER", .val = bdmf_address_unimac_top_unimac_ints_ier },
            { .name = "UNIMAC_TOP_UNIMAC_INTS_ITR", .val = bdmf_address_unimac_top_unimac_ints_itr },
            { .name = "UNIMAC_TOP_UNIMAC_INTS_ISM", .val = bdmf_address_unimac_top_unimac_ints_ism },
            BDMFMON_ENUM_LAST
        };
        BDMFMON_MAKE_PARM(dir, "address", "address", bcm_unimac_misc_cli_address,
            BDMFMON_MAKE_PARM_ENUM("method", "method", enum_table_address, 0),
            BDMFMON_MAKE_PARM("index1", "umac_misc_id", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL),
            BDMFMON_MAKE_PARM("index2", "onu_id/alloc_id/port_id/etc...", BDMFMON_PARM_UNUMBER, BDMFMON_PARM_FLAG_OPTIONAL));
    }
#endif /* #ifdef HAL_DRV_TEST_ENABLE */

    return dir;
}
#endif /* #ifdef USE_BDMF_SHELL */
