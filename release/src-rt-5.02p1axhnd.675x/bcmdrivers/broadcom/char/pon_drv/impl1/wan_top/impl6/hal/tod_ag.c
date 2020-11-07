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
bdmf_error_t ag_drv_tod_config_0_set(const tod_config_0 *config_0)
{
    uint32_t reg_config_0=0;

#ifdef VALIDATE_PARMS
    if(!config_0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((config_0->cfg_ts48_mac_select >= _3BITS_MAX_VAL_) ||
       (config_0->cfg_ts48_enable >= _1BITS_MAX_VAL_) ||
       (config_0->ts48_offset >= _10BITS_MAX_VAL_) ||
       (config_0->cfg_ts48_read >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_config_0 = RU_FIELD_SET(0, TOD, CONFIG_0, TS48_MAC_SELECT, reg_config_0, config_0->cfg_ts48_mac_select);
    reg_config_0 = RU_FIELD_SET(0, TOD, CONFIG_0, TS48_ENABLE, reg_config_0, config_0->cfg_ts48_enable);
    reg_config_0 = RU_FIELD_SET(0, TOD, CONFIG_0, TS48_OFFSET, reg_config_0, config_0->ts48_offset);
    reg_config_0 = RU_FIELD_SET(0, TOD, CONFIG_0, TS48_READ, reg_config_0, config_0->cfg_ts48_read);

    RU_REG_WRITE(0, TOD, CONFIG_0, reg_config_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tod_config_0_get(tod_config_0 *config_0)
{
    uint32_t reg_config_0=0;

#ifdef VALIDATE_PARMS
    if(!config_0)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, TOD, CONFIG_0, reg_config_0);

    config_0->cfg_ts48_mac_select = RU_FIELD_GET(0, TOD, CONFIG_0, TS48_MAC_SELECT, reg_config_0);
    config_0->cfg_ts48_enable = RU_FIELD_GET(0, TOD, CONFIG_0, TS48_ENABLE, reg_config_0);
    config_0->ts48_offset = RU_FIELD_GET(0, TOD, CONFIG_0, TS48_OFFSET, reg_config_0);
    config_0->cfg_ts48_read = RU_FIELD_GET(0, TOD, CONFIG_0, TS48_READ, reg_config_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tod_msb_get(uint16_t *ts48_wan_read_msb)
{
    uint32_t reg_msb=0;

#ifdef VALIDATE_PARMS
    if(!ts48_wan_read_msb)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, TOD, MSB, reg_msb);

    *ts48_wan_read_msb = RU_FIELD_GET(0, TOD, MSB, TS48_WAN_READ_MSB, reg_msb);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tod_lsb_get(uint32_t *ts48_wan_read_lsb)
{
    uint32_t reg_lsb=0;

#ifdef VALIDATE_PARMS
    if(!ts48_wan_read_lsb)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, TOD, LSB, reg_lsb);

    *ts48_wan_read_lsb = RU_FIELD_GET(0, TOD, LSB, TS48_WAN_READ_LSB, reg_lsb);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tod_tod_config_2_set(uint16_t tx_offset, uint16_t rx_offset)
{
    uint32_t reg_config_2=0;

#ifdef VALIDATE_PARMS
    if((rx_offset >= _10BITS_MAX_VAL_) ||
       (tx_offset >= _10BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_config_2 = RU_FIELD_SET(0, TOD, CONFIG_2, RX_OFFSET, reg_config_2, rx_offset);
    reg_config_2 = RU_FIELD_SET(0, TOD, CONFIG_2, TX_OFFSET, reg_config_2, tx_offset);

    RU_REG_WRITE(0, TOD, CONFIG_2, reg_config_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tod_tod_config_2_get(uint16_t *tx_offset, uint16_t *rx_offset)
{
    uint32_t reg_config_2=0;

#ifdef VALIDATE_PARMS
    if(!rx_offset || !tx_offset)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, TOD, CONFIG_2, reg_config_2);

    *rx_offset = RU_FIELD_GET(0, TOD, CONFIG_2, RX_OFFSET, reg_config_2);
    *tx_offset = RU_FIELD_GET(0, TOD, CONFIG_2, TX_OFFSET, reg_config_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tod_tod_config_3_set(bdmf_boolean ts48_fifo_dis, uint8_t ts48_fifo_ld_rate, uint16_t ref_offset)
{
    uint32_t reg_config_3=0;

#ifdef VALIDATE_PARMS
    if((ref_offset >= _10BITS_MAX_VAL_) ||
       (ts48_fifo_ld_rate >= _5BITS_MAX_VAL_) ||
       (ts48_fifo_dis >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_config_3 = RU_FIELD_SET(0, TOD, CONFIG_3, REF_OFFSET, reg_config_3, ref_offset);
    reg_config_3 = RU_FIELD_SET(0, TOD, CONFIG_3, TS48_FIFO_LD_RATE, reg_config_3, ts48_fifo_ld_rate);
    reg_config_3 = RU_FIELD_SET(0, TOD, CONFIG_3, TS48_FIFO_DIS, reg_config_3, ts48_fifo_dis);

    RU_REG_WRITE(0, TOD, CONFIG_3, reg_config_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tod_tod_config_3_get(bdmf_boolean *ts48_fifo_dis, uint8_t *ts48_fifo_ld_rate, uint16_t *ref_offset)
{
    uint32_t reg_config_3=0;

#ifdef VALIDATE_PARMS
    if(!ref_offset || !ts48_fifo_ld_rate || !ts48_fifo_dis)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, TOD, CONFIG_3, reg_config_3);

    *ref_offset = RU_FIELD_GET(0, TOD, CONFIG_3, REF_OFFSET, reg_config_3);
    *ts48_fifo_ld_rate = RU_FIELD_GET(0, TOD, CONFIG_3, TS48_FIFO_LD_RATE, reg_config_3);
    *ts48_fifo_dis = RU_FIELD_GET(0, TOD, CONFIG_3, TS48_FIFO_DIS, reg_config_3);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tod_tod_status_0_get(uint16_t *ts16_tx_read, uint16_t *ts16_rx_read)
{
    uint32_t reg_status_0=0;

#ifdef VALIDATE_PARMS
    if(!ts16_rx_read || !ts16_tx_read)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, TOD, STATUS_0, reg_status_0);

    *ts16_rx_read = RU_FIELD_GET(0, TOD, STATUS_0, TS16_RX_READ, reg_status_0);
    *ts16_tx_read = RU_FIELD_GET(0, TOD, STATUS_0, TS16_TX_READ, reg_status_0);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tod_tod_status_1_get(uint16_t *ts16_rx_synce_read, uint16_t *ts16_ref_synce_read)
{
    uint32_t reg_status_1=0;

#ifdef VALIDATE_PARMS
    if(!ts16_ref_synce_read || !ts16_rx_synce_read)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, TOD, STATUS_1, reg_status_1);

    *ts16_ref_synce_read = RU_FIELD_GET(0, TOD, STATUS_1, TS16_REF_SYNCE_READ, reg_status_1);
    *ts16_rx_synce_read = RU_FIELD_GET(0, TOD, STATUS_1, TS16_RX_SYNCE_READ, reg_status_1);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tod_tod_status_2_get(uint16_t *ts16_mac_tx_read, uint16_t *ts16_mac_rx_read)
{
    uint32_t reg_status_2=0;

#ifdef VALIDATE_PARMS
    if(!ts16_mac_rx_read || !ts16_mac_tx_read)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, TOD, STATUS_2, reg_status_2);

    *ts16_mac_rx_read = RU_FIELD_GET(0, TOD, STATUS_2, TS16_MAC_RX_READ, reg_status_2);
    *ts16_mac_tx_read = RU_FIELD_GET(0, TOD, STATUS_2, TS16_MAC_TX_READ, reg_status_2);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tod_tod_fifo_status_set(const tod_tod_fifo_status *tod_fifo_status)
{
    uint32_t reg_fifo_status=0;

#ifdef VALIDATE_PARMS
    if(!tod_fifo_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
    if((tod_fifo_status->rempty_gpon >= _1BITS_MAX_VAL_) ||
       (tod_fifo_status->rempty_epon >= _1BITS_MAX_VAL_) ||
       (tod_fifo_status->rempty_ngpon >= _1BITS_MAX_VAL_) ||
       (tod_fifo_status->rempty_10g_epon >= _1BITS_MAX_VAL_) ||
       (tod_fifo_status->rempty_ae >= _1BITS_MAX_VAL_) ||
       (tod_fifo_status->rempty_rx >= _1BITS_MAX_VAL_) ||
       (tod_fifo_status->rempty_tx >= _1BITS_MAX_VAL_) ||
       (tod_fifo_status->rempty_ref >= _1BITS_MAX_VAL_) ||
       (tod_fifo_status->rempty_mac_rx >= _1BITS_MAX_VAL_) ||
       (tod_fifo_status->rempty_mac_tx >= _1BITS_MAX_VAL_) ||
       (tod_fifo_status->wfull_gpon >= _1BITS_MAX_VAL_) ||
       (tod_fifo_status->wfull_epon >= _1BITS_MAX_VAL_) ||
       (tod_fifo_status->wfull_ngpon >= _1BITS_MAX_VAL_) ||
       (tod_fifo_status->wfull_10g_epon >= _1BITS_MAX_VAL_) ||
       (tod_fifo_status->wfull_ae >= _1BITS_MAX_VAL_) ||
       (tod_fifo_status->wfull_rx >= _1BITS_MAX_VAL_) ||
       (tod_fifo_status->wfull_tx >= _1BITS_MAX_VAL_) ||
       (tod_fifo_status->wfull_ref >= _1BITS_MAX_VAL_) ||
       (tod_fifo_status->wfull_mac_rx >= _1BITS_MAX_VAL_) ||
       (tod_fifo_status->wfull_mac_tx >= _1BITS_MAX_VAL_))
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_RANGE), BDMF_ERR_RANGE);
        return BDMF_ERR_RANGE;
    }
#endif

    reg_fifo_status = RU_FIELD_SET(0, TOD, FIFO_STATUS, REMPTY_GPON, reg_fifo_status, tod_fifo_status->rempty_gpon);
    reg_fifo_status = RU_FIELD_SET(0, TOD, FIFO_STATUS, REMPTY_EPON, reg_fifo_status, tod_fifo_status->rempty_epon);
    reg_fifo_status = RU_FIELD_SET(0, TOD, FIFO_STATUS, REMPTY_NGPON, reg_fifo_status, tod_fifo_status->rempty_ngpon);
    reg_fifo_status = RU_FIELD_SET(0, TOD, FIFO_STATUS, REMPTY_10G_EPON, reg_fifo_status, tod_fifo_status->rempty_10g_epon);
    reg_fifo_status = RU_FIELD_SET(0, TOD, FIFO_STATUS, REMPTY_AE, reg_fifo_status, tod_fifo_status->rempty_ae);
    reg_fifo_status = RU_FIELD_SET(0, TOD, FIFO_STATUS, REMPTY_RX, reg_fifo_status, tod_fifo_status->rempty_rx);
    reg_fifo_status = RU_FIELD_SET(0, TOD, FIFO_STATUS, REMPTY_TX, reg_fifo_status, tod_fifo_status->rempty_tx);
    reg_fifo_status = RU_FIELD_SET(0, TOD, FIFO_STATUS, REMPTY_REF, reg_fifo_status, tod_fifo_status->rempty_ref);
    reg_fifo_status = RU_FIELD_SET(0, TOD, FIFO_STATUS, REMPTY_MAC_RX, reg_fifo_status, tod_fifo_status->rempty_mac_rx);
    reg_fifo_status = RU_FIELD_SET(0, TOD, FIFO_STATUS, REMPTY_MAC_TX, reg_fifo_status, tod_fifo_status->rempty_mac_tx);
    reg_fifo_status = RU_FIELD_SET(0, TOD, FIFO_STATUS, WFULL_GPON, reg_fifo_status, tod_fifo_status->wfull_gpon);
    reg_fifo_status = RU_FIELD_SET(0, TOD, FIFO_STATUS, WFULL_EPON, reg_fifo_status, tod_fifo_status->wfull_epon);
    reg_fifo_status = RU_FIELD_SET(0, TOD, FIFO_STATUS, WFULL_NGPON, reg_fifo_status, tod_fifo_status->wfull_ngpon);
    reg_fifo_status = RU_FIELD_SET(0, TOD, FIFO_STATUS, WFULL_10G_EPON, reg_fifo_status, tod_fifo_status->wfull_10g_epon);
    reg_fifo_status = RU_FIELD_SET(0, TOD, FIFO_STATUS, WFULL_AE, reg_fifo_status, tod_fifo_status->wfull_ae);
    reg_fifo_status = RU_FIELD_SET(0, TOD, FIFO_STATUS, WFULL_RX, reg_fifo_status, tod_fifo_status->wfull_rx);
    reg_fifo_status = RU_FIELD_SET(0, TOD, FIFO_STATUS, WFULL_TX, reg_fifo_status, tod_fifo_status->wfull_tx);
    reg_fifo_status = RU_FIELD_SET(0, TOD, FIFO_STATUS, WFULL_REF, reg_fifo_status, tod_fifo_status->wfull_ref);
    reg_fifo_status = RU_FIELD_SET(0, TOD, FIFO_STATUS, WFULL_MAC_RX, reg_fifo_status, tod_fifo_status->wfull_mac_rx);
    reg_fifo_status = RU_FIELD_SET(0, TOD, FIFO_STATUS, WFULL_MAC_TX, reg_fifo_status, tod_fifo_status->wfull_mac_tx);

    RU_REG_WRITE(0, TOD, FIFO_STATUS, reg_fifo_status);

    return BDMF_ERR_OK;
}

bdmf_error_t ag_drv_tod_tod_fifo_status_get(tod_tod_fifo_status *tod_fifo_status)
{
    uint32_t reg_fifo_status=0;

#ifdef VALIDATE_PARMS
    if(!tod_fifo_status)
    {
        bdmf_trace("ERROR driver %s:%u| err=%s (%d)\n", __FILE__, __LINE__, bdmf_strerror(BDMF_ERR_PARM), BDMF_ERR_PARM);
        return BDMF_ERR_PARM;
    }
#endif

    RU_REG_READ(0, TOD, FIFO_STATUS, reg_fifo_status);

    tod_fifo_status->rempty_gpon = RU_FIELD_GET(0, TOD, FIFO_STATUS, REMPTY_GPON, reg_fifo_status);
    tod_fifo_status->rempty_epon = RU_FIELD_GET(0, TOD, FIFO_STATUS, REMPTY_EPON, reg_fifo_status);
    tod_fifo_status->rempty_ngpon = RU_FIELD_GET(0, TOD, FIFO_STATUS, REMPTY_NGPON, reg_fifo_status);
    tod_fifo_status->rempty_10g_epon = RU_FIELD_GET(0, TOD, FIFO_STATUS, REMPTY_10G_EPON, reg_fifo_status);
    tod_fifo_status->rempty_ae = RU_FIELD_GET(0, TOD, FIFO_STATUS, REMPTY_AE, reg_fifo_status);
    tod_fifo_status->rempty_rx = RU_FIELD_GET(0, TOD, FIFO_STATUS, REMPTY_RX, reg_fifo_status);
    tod_fifo_status->rempty_tx = RU_FIELD_GET(0, TOD, FIFO_STATUS, REMPTY_TX, reg_fifo_status);
    tod_fifo_status->rempty_ref = RU_FIELD_GET(0, TOD, FIFO_STATUS, REMPTY_REF, reg_fifo_status);
    tod_fifo_status->rempty_mac_rx = RU_FIELD_GET(0, TOD, FIFO_STATUS, REMPTY_MAC_RX, reg_fifo_status);
    tod_fifo_status->rempty_mac_tx = RU_FIELD_GET(0, TOD, FIFO_STATUS, REMPTY_MAC_TX, reg_fifo_status);
    tod_fifo_status->wfull_gpon = RU_FIELD_GET(0, TOD, FIFO_STATUS, WFULL_GPON, reg_fifo_status);
    tod_fifo_status->wfull_epon = RU_FIELD_GET(0, TOD, FIFO_STATUS, WFULL_EPON, reg_fifo_status);
    tod_fifo_status->wfull_ngpon = RU_FIELD_GET(0, TOD, FIFO_STATUS, WFULL_NGPON, reg_fifo_status);
    tod_fifo_status->wfull_10g_epon = RU_FIELD_GET(0, TOD, FIFO_STATUS, WFULL_10G_EPON, reg_fifo_status);
    tod_fifo_status->wfull_ae = RU_FIELD_GET(0, TOD, FIFO_STATUS, WFULL_AE, reg_fifo_status);
    tod_fifo_status->wfull_rx = RU_FIELD_GET(0, TOD, FIFO_STATUS, WFULL_RX, reg_fifo_status);
    tod_fifo_status->wfull_tx = RU_FIELD_GET(0, TOD, FIFO_STATUS, WFULL_TX, reg_fifo_status);
    tod_fifo_status->wfull_ref = RU_FIELD_GET(0, TOD, FIFO_STATUS, WFULL_REF, reg_fifo_status);
    tod_fifo_status->wfull_mac_rx = RU_FIELD_GET(0, TOD, FIFO_STATUS, WFULL_MAC_RX, reg_fifo_status);
    tod_fifo_status->wfull_mac_tx = RU_FIELD_GET(0, TOD, FIFO_STATUS, WFULL_MAC_TX, reg_fifo_status);

    return BDMF_ERR_OK;
}

