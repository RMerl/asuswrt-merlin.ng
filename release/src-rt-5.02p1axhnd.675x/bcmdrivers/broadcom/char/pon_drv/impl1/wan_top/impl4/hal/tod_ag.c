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
#include "tod_ag.h"
int ag_drv_tod_config_0_set(const tod_config_0 *config_0)
{
    uint32_t reg_config_0=0;

#ifdef VALIDATE_PARMS
    if(!config_0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 1);
        return 1;
    }
    if((config_0->tod_read_busy >= _1BITS_MAX_VAL_) ||
       (config_0->cfg_ts48_pre_sync_fifo_disable >= _1BITS_MAX_VAL_) ||
       (config_0->cfg_ts48_pre_sync_fifo_load_rate >= _5BITS_MAX_VAL_) ||
       (config_0->cfg_tod_pps_clear >= _1BITS_MAX_VAL_) ||
       (config_0->cfg_tod_read >= _1BITS_MAX_VAL_) ||
       (config_0->cfg_ts48_offset >= _10BITS_MAX_VAL_) ||
       (config_0->cfg_ts48_mac_select >= _3BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_config_0 = RU_FIELD_SET(0, TOD, CONFIG_0, TOD_READ_BUSY, reg_config_0, config_0->tod_read_busy);
    reg_config_0 = RU_FIELD_SET(0, TOD, CONFIG_0, CFG_TS48_PRE_SYNC_FIFO_DISABLE, reg_config_0, config_0->cfg_ts48_pre_sync_fifo_disable);
    reg_config_0 = RU_FIELD_SET(0, TOD, CONFIG_0, CFG_TS48_PRE_SYNC_FIFO_LOAD_RATE, reg_config_0, config_0->cfg_ts48_pre_sync_fifo_load_rate);
    reg_config_0 = RU_FIELD_SET(0, TOD, CONFIG_0, CFG_TOD_PPS_CLEAR, reg_config_0, config_0->cfg_tod_pps_clear);
    reg_config_0 = RU_FIELD_SET(0, TOD, CONFIG_0, CFG_TOD_READ, reg_config_0, config_0->cfg_tod_read);
    reg_config_0 = RU_FIELD_SET(0, TOD, CONFIG_0, CFG_TS48_OFFSET, reg_config_0, config_0->cfg_ts48_offset);
    reg_config_0 = RU_FIELD_SET(0, TOD, CONFIG_0, CFG_TS48_MAC_SELECT, reg_config_0, config_0->cfg_ts48_mac_select);

    RU_REG_WRITE(0, TOD, CONFIG_0, reg_config_0);

    return 0;
}

int ag_drv_tod_config_0_get(tod_config_0 *config_0)
{
    uint32_t reg_config_0=0;

#ifdef VALIDATE_PARMS
    if(!config_0)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, TOD, CONFIG_0, reg_config_0);

    config_0->tod_read_busy = RU_FIELD_GET(0, TOD, CONFIG_0, TOD_READ_BUSY, reg_config_0);
    config_0->cfg_ts48_pre_sync_fifo_disable = RU_FIELD_GET(0, TOD, CONFIG_0, CFG_TS48_PRE_SYNC_FIFO_DISABLE, reg_config_0);
    config_0->cfg_ts48_pre_sync_fifo_load_rate = RU_FIELD_GET(0, TOD, CONFIG_0, CFG_TS48_PRE_SYNC_FIFO_LOAD_RATE, reg_config_0);
    config_0->cfg_tod_pps_clear = RU_FIELD_GET(0, TOD, CONFIG_0, CFG_TOD_PPS_CLEAR, reg_config_0);
    config_0->cfg_tod_read = RU_FIELD_GET(0, TOD, CONFIG_0, CFG_TOD_READ, reg_config_0);
    config_0->cfg_ts48_offset = RU_FIELD_GET(0, TOD, CONFIG_0, CFG_TS48_OFFSET, reg_config_0);
    config_0->cfg_ts48_mac_select = RU_FIELD_GET(0, TOD, CONFIG_0, CFG_TS48_MAC_SELECT, reg_config_0);

    return 0;
}

int ag_drv_tod_config_1_set(uint8_t cfg_tod_load_ts48_offset, uint8_t cfg_tod_load, uint32_t cfg_tod_seconds)
{
    uint32_t reg_config_1=0;

#ifdef VALIDATE_PARMS
    if((cfg_tod_load_ts48_offset >= _1BITS_MAX_VAL_) ||
       (cfg_tod_load >= _1BITS_MAX_VAL_) ||
       (cfg_tod_seconds >= _19BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_config_1 = RU_FIELD_SET(0, TOD, CONFIG_1, CFG_TOD_LOAD_TS48_OFFSET, reg_config_1, cfg_tod_load_ts48_offset);
    reg_config_1 = RU_FIELD_SET(0, TOD, CONFIG_1, CFG_TOD_LOAD, reg_config_1, cfg_tod_load);
    reg_config_1 = RU_FIELD_SET(0, TOD, CONFIG_1, CFG_TOD_SECONDS, reg_config_1, cfg_tod_seconds);

    RU_REG_WRITE(0, TOD, CONFIG_1, reg_config_1);

    return 0;
}

int ag_drv_tod_config_1_get(uint8_t *cfg_tod_load_ts48_offset, uint8_t *cfg_tod_load, uint32_t *cfg_tod_seconds)
{
    uint32_t reg_config_1=0;

#ifdef VALIDATE_PARMS
    if(!cfg_tod_load_ts48_offset || !cfg_tod_load || !cfg_tod_seconds)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, TOD, CONFIG_1, reg_config_1);

    *cfg_tod_load_ts48_offset = RU_FIELD_GET(0, TOD, CONFIG_1, CFG_TOD_LOAD_TS48_OFFSET, reg_config_1);
    *cfg_tod_load = RU_FIELD_GET(0, TOD, CONFIG_1, CFG_TOD_LOAD, reg_config_1);
    *cfg_tod_seconds = RU_FIELD_GET(0, TOD, CONFIG_1, CFG_TOD_SECONDS, reg_config_1);

    return 0;
}

int ag_drv_tod_config_2_set(uint16_t cfg_tx_offset, uint16_t cfg_rx_offset)
{
    uint32_t reg_config_2=0;

#ifdef VALIDATE_PARMS
    if((cfg_tx_offset >= _10BITS_MAX_VAL_) ||
       (cfg_rx_offset >= _10BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_config_2 = RU_FIELD_SET(0, TOD, CONFIG_2, CFG_TX_OFFSET, reg_config_2, cfg_tx_offset);
    reg_config_2 = RU_FIELD_SET(0, TOD, CONFIG_2, CFG_RX_OFFSET, reg_config_2, cfg_rx_offset);

    RU_REG_WRITE(0, TOD, CONFIG_2, reg_config_2);

    return 0;
}

int ag_drv_tod_config_2_get(uint16_t *cfg_tx_offset, uint16_t *cfg_rx_offset)
{
    uint32_t reg_config_2=0;

#ifdef VALIDATE_PARMS
    if(!cfg_tx_offset || !cfg_rx_offset)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, TOD, CONFIG_2, reg_config_2);

    *cfg_tx_offset = RU_FIELD_GET(0, TOD, CONFIG_2, CFG_TX_OFFSET, reg_config_2);
    *cfg_rx_offset = RU_FIELD_GET(0, TOD, CONFIG_2, CFG_RX_OFFSET, reg_config_2);

    return 0;
}

int ag_drv_tod_config_3_set(uint16_t cfg_ref_offset)
{
    uint32_t reg_config_3=0;

#ifdef VALIDATE_PARMS
    if((cfg_ref_offset >= _10BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_config_3 = RU_FIELD_SET(0, TOD, CONFIG_3, CFG_REF_OFFSET, reg_config_3, cfg_ref_offset);

    RU_REG_WRITE(0, TOD, CONFIG_3, reg_config_3);

    return 0;
}

int ag_drv_tod_config_3_get(uint16_t *cfg_ref_offset)
{
    uint32_t reg_config_3=0;

#ifdef VALIDATE_PARMS
    if(!cfg_ref_offset)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, TOD, CONFIG_3, reg_config_3);

    *cfg_ref_offset = RU_FIELD_GET(0, TOD, CONFIG_3, CFG_REF_OFFSET, reg_config_3);

    return 0;
}

int ag_drv_tod_config_4_set(uint32_t cfg_tod_1pps_ns_offset)
{
    uint32_t reg_config_4=0;

#ifdef VALIDATE_PARMS
    if((cfg_tod_1pps_ns_offset >= _30BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_config_4 = RU_FIELD_SET(0, TOD, CONFIG_4, CFG_TOD_1PPS_NS_OFFSET, reg_config_4, cfg_tod_1pps_ns_offset);

    RU_REG_WRITE(0, TOD, CONFIG_4, reg_config_4);

    return 0;
}

int ag_drv_tod_config_4_get(uint32_t *cfg_tod_1pps_ns_offset)
{
    uint32_t reg_config_4=0;

#ifdef VALIDATE_PARMS
    if(!cfg_tod_1pps_ns_offset)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, TOD, CONFIG_4, reg_config_4);

    *cfg_tod_1pps_ns_offset = RU_FIELD_GET(0, TOD, CONFIG_4, CFG_TOD_1PPS_NS_OFFSET, reg_config_4);

    return 0;
}

int ag_drv_tod_config_5_set(uint8_t cfg_tod_load_ns_offset, uint32_t cfg_tod_ns_offset)
{
    uint32_t reg_config_5=0;

#ifdef VALIDATE_PARMS
    if((cfg_tod_load_ns_offset >= _1BITS_MAX_VAL_) ||
       (cfg_tod_ns_offset >= _30BITS_MAX_VAL_))
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 0);
        return 0;
    }
#endif

    reg_config_5 = RU_FIELD_SET(0, TOD, CONFIG_5, CFG_TOD_LOAD_NS_OFFSET, reg_config_5, cfg_tod_load_ns_offset);
    reg_config_5 = RU_FIELD_SET(0, TOD, CONFIG_5, CFG_TOD_NS_OFFSET, reg_config_5, cfg_tod_ns_offset);

    RU_REG_WRITE(0, TOD, CONFIG_5, reg_config_5);

    return 0;
}

int ag_drv_tod_config_5_get(uint8_t *cfg_tod_load_ns_offset, uint32_t *cfg_tod_ns_offset)
{
    uint32_t reg_config_5=0;

#ifdef VALIDATE_PARMS
    if(!cfg_tod_load_ns_offset || !cfg_tod_ns_offset)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, TOD, CONFIG_5, reg_config_5);

    *cfg_tod_load_ns_offset = RU_FIELD_GET(0, TOD, CONFIG_5, CFG_TOD_LOAD_NS_OFFSET, reg_config_5);
    *cfg_tod_ns_offset = RU_FIELD_GET(0, TOD, CONFIG_5, CFG_TOD_NS_OFFSET, reg_config_5);

    return 0;
}

int ag_drv_tod_ts48_msb_get(uint16_t *ts48_wan_read_msb)
{
    uint32_t reg_ts48_msb=0;

#ifdef VALIDATE_PARMS
    if(!ts48_wan_read_msb)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, TOD, TS48_MSB, reg_ts48_msb);

    *ts48_wan_read_msb = RU_FIELD_GET(0, TOD, TS48_MSB, TS48_WAN_READ_MSB, reg_ts48_msb);

    return 0;
}

int ag_drv_tod_ts48_lsb_get(uint32_t *ts48_wan_read_lsb)
{
    uint32_t reg_ts48_lsb=0;

#ifdef VALIDATE_PARMS
    if(!ts48_wan_read_lsb)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, TOD, TS48_LSB, reg_ts48_lsb);

    *ts48_wan_read_lsb = RU_FIELD_GET(0, TOD, TS48_LSB, TS48_WAN_READ_LSB, reg_ts48_lsb);

    return 0;
}

int ag_drv_tod_ts64_msb_get(uint32_t *ts64_wan_read_msb)
{
    uint32_t reg_ts64_msb=0;

#ifdef VALIDATE_PARMS
    if(!ts64_wan_read_msb)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, TOD, TS64_MSB, reg_ts64_msb);

    *ts64_wan_read_msb = RU_FIELD_GET(0, TOD, TS64_MSB, TS64_WAN_READ_MSB, reg_ts64_msb);

    return 0;
}

int ag_drv_tod_ts64_lsb_get(uint32_t *ts64_wan_read_lsb)
{
    uint32_t reg_ts64_lsb=0;

#ifdef VALIDATE_PARMS
    if(!ts64_wan_read_lsb)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, TOD, TS64_LSB, reg_ts64_lsb);

    *ts64_wan_read_lsb = RU_FIELD_GET(0, TOD, TS64_LSB, TS64_WAN_READ_LSB, reg_ts64_lsb);

    return 0;
}

int ag_drv_tod_status_0_get(uint16_t *ts16_ref_synce_read)
{
    uint32_t reg_status_0=0;

#ifdef VALIDATE_PARMS
    if(!ts16_ref_synce_read)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, TOD, STATUS_0, reg_status_0);

    *ts16_ref_synce_read = RU_FIELD_GET(0, TOD, STATUS_0, TS16_REF_SYNCE_READ, reg_status_0);

    return 0;
}

int ag_drv_tod_status_1_get(uint16_t *ts16_mac_tx_read, uint16_t *ts16_mac_rx_read)
{
    uint32_t reg_status_1=0;

#ifdef VALIDATE_PARMS
    if(!ts16_mac_tx_read || !ts16_mac_rx_read)
    {
        pr_err("ERROR driver %s:%u|(%d)\n", __FILE__, __LINE__, 2);
        return 2;
    }
#endif

    RU_REG_READ(0, TOD, STATUS_1, reg_status_1);

    *ts16_mac_tx_read = RU_FIELD_GET(0, TOD, STATUS_1, TS16_MAC_TX_READ, reg_status_1);
    *ts16_mac_rx_read = RU_FIELD_GET(0, TOD, STATUS_1, TS16_MAC_RX_READ, reg_status_1);

    return 0;
}

